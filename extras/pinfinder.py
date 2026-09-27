#!/usr/bin/env python3
"""Generate pins.inc from parsed SAMD21/DA1 and SAMD51/E5x mux tables.

Canonical location: extras/pinfinder.py

This follows the same pattern used by the Wire pinmux generator:
- Define device-series macros from datasheet device tables.
- Emit pin routing in pin-count tiers.
- Gate each tier with series-macro expressions (not per-pin guards).

Only SPI MOSI-capable pads are emitted: PAD 0, 2, and 3.
PAD 1 is intentionally excluded because it is clock-only for SPI TX.
"""
from __future__ import annotations

import csv
import re
from pathlib import Path
from typing import Dict, List, Sequence, Tuple

ROOT = Path(__file__).resolve().parent.parent
DATA_DIR = ROOT / "extras" / "spipinmux"
OUTPUT = ROOT / "pins.inc"

SAMD21_TABLE_7_1 = DATA_DIR / "SAMD21_Table_7-1.csv"
SAMD51_TABLE_6_1 = DATA_DIR / "SAMD51_Table_6-1.csv"
SAMD21_TABLE_2_1 = DATA_DIR / "SAMD21_Table_2-1.csv"
SAMD21_TABLE_2_2 = DATA_DIR / "SAMD21_Table_2-2.csv"
SAMD51_TABLE_1_1 = DATA_DIR / "SAMD51_Table_1-1.csv"
SAMD51_TABLE_1_2 = DATA_DIR / "SAMD51_Table_1-2.csv"

SERCOM_PAD_RE = re.compile(r"SERCOM(\d+)/PAD\[(\d+)\]")
PIN_RE = re.compile(r"^P([A-D])(\d{2})$")
SPI_TX_PADS = {0, 2, 3}
PORT_NUM = {"A": 0, "B": 1, "C": 2, "D": 3}
MUX_INFO = (("C", 2), ("D", 3))


Entry = Tuple[int, int, int, int, int]

# Wire-style series macros grouped by family/pin-count buckets.
SERIES_ORDER = [
    "SAMD21_DA1_32_SERIES",
    "SAMD21_DA1_48_SERIES",
    "SAMD21_DA1_64_SERIES",
    "SAMD5X_E5X_48_SERIES",
    "SAMD5X_E5X_64_SERIES",
    "SAMD5X_E5X_100_SERIES",
    "SAMD5X_E5X_120_SERIES",
]


def _strip_at_prefix(device: str) -> str:
    dev = device.strip().upper()
    if dev.startswith("AT"):
        dev = dev[2:]
    return dev


def macro_candidates(device: str) -> List[str]:
    dev = _strip_at_prefix(device)
    # The SAM D5x/E5x datasheet tables omit the silicon-revision suffix, while
    # Microchip's CMSIS headers use full part macros such as __SAME54P20A__.
    if dev.startswith(("SAMD51", "SAME51", "SAME53", "SAME54")) and dev[-1].isdigit():
        dev += "A"
    return [f"__{dev}__"]


def load_csv(path: Path, skip_rows: int = 0) -> List[Dict[str, str]]:
    with path.open(newline="", encoding="utf-8-sig") as handle:
        for _ in range(skip_rows):
            next(handle)
        return list(csv.DictReader(handle))


def load_device_pin_counts() -> Dict[str, int]:
    tables = [
        (SAMD21_TABLE_2_1, 2),
        (SAMD21_TABLE_2_2, 1),
        (SAMD51_TABLE_1_1, 2),
        (SAMD51_TABLE_1_2, 2),
    ]
    counts: Dict[str, int] = {}
    for path, skip_rows in tables:
        for row in load_csv(path, skip_rows=skip_rows):
            device = row.get("Device", "").strip()
            pins = row.get("Pins", "").strip()
            if not device or not pins:
                continue
            try:
                counts[device] = int(pins)
            except ValueError:
                continue
    return counts


def parse_sercom_pad(text: str) -> Tuple[int, int] | None:
    if not text:
        return None
    match = SERCOM_PAD_RE.search(text)
    if not match:
        return None
    return int(match.group(1)), int(match.group(2))


def parse_pin(pin_name: str) -> Tuple[int, int] | None:
    match = PIN_RE.match(pin_name.strip())
    if not match:
        return None
    port = PORT_NUM.get(match.group(1))
    if port is None:
        return None
    return port, int(match.group(2))


def _first_present_tier(row: Dict[str, str], package_columns: Sequence[str]) -> int | None:
    for index, column in enumerate(package_columns):
        if row.get(column, "").strip():
            return index
    return None


def load_mux_entries(csv_path: Path, pin_column: str, package_columns: Sequence[str]) -> Dict[int, List[Entry]]:
    entries_by_tier: Dict[int, List[Entry]] = {index: [] for index in range(len(package_columns))}
    for row in load_csv(csv_path):
        tier = _first_present_tier(row, package_columns)
        if tier is None:
            continue
        pin_name = row.get(pin_column, "").strip()
        parsed_pin = parse_pin(pin_name)
        if not parsed_pin:
            continue
        port, port_pin = parsed_pin
        for column_name, mux in MUX_INFO:
            parsed = parse_sercom_pad(row.get(column_name, "").strip())
            if not parsed:
                continue
            sercom_num, pad = parsed
            if pad not in SPI_TX_PADS:
                continue
            entries_by_tier[tier].append((port, port_pin, sercom_num, pad, mux))

    sorted_entries: Dict[int, List[Entry]] = {}
    for tier, tier_entries in entries_by_tier.items():
        sorted_entries[tier] = sorted(
            tier_entries,
            key=lambda entry: (entry[2], entry[4], entry[0], entry[1], entry[3]),
        )
    return sorted_entries


def merge_tiers(tier_entries: Dict[int, List[Entry]], max_tier: int) -> List[Entry]:
    merged: List[Entry] = []
    for tier in range(0, max_tier + 1):
        merged.extend(tier_entries.get(tier, []))
    return merged


def series_for_device(device: str, pin_count: int) -> str | None:
    dev = _strip_at_prefix(device)

    if dev.startswith("SAMD21E") or dev.startswith("SAMDA1E"):
        return "SAMD21_DA1_32_SERIES"
    if dev.startswith("SAMD21G") or dev.startswith("SAMDA1G"):
        return "SAMD21_DA1_48_SERIES"
    if dev.startswith("SAMD21J") or dev.startswith("SAMDA1J"):
        return "SAMD21_DA1_64_SERIES"

    if dev.startswith("SAMD51G") or dev.startswith("SAME51G"):
        return "SAMD5X_E5X_48_SERIES"
    if dev.startswith("SAMD51J") or dev.startswith("SAME51J") or dev.startswith("SAME53J"):
        return "SAMD5X_E5X_64_SERIES"
    if (
        dev.startswith("SAMD51N")
        or dev.startswith("SAME51N")
        or dev.startswith("SAME53N")
        or dev.startswith("SAME54N")
    ):
        return "SAMD5X_E5X_100_SERIES"
    if dev.startswith("SAMD51P") or dev.startswith("SAME54P") or pin_count >= 120:
        return "SAMD5X_E5X_120_SERIES"

    return None


def format_series_macro(series_name: str, checks: List[str]) -> List[str]:
    lines: List[str] = [f"#ifndef {series_name}"]
    if not checks:
        lines.append(f"#define {series_name} 0")
        lines.append("#endif")
        return lines

    expr = " || ".join(f"defined({check})" for check in sorted(set(checks)))
    lines.append(f"#if {expr}")
    lines.append(f"#define {series_name} 1")
    lines.append("#else")
    lines.append(f"#define {series_name} 0")
    lines.append("#endif")
    lines.append("#endif")
    return lines


def emit_entries_block(comment: str, guard_expr: str, entries: Sequence[Entry]) -> List[str]:
    lines: List[str] = [f"// {comment}"]
    lines.append(f"#if {guard_expr}")

    current_group: Tuple[int, int] | None = None
    for entry in entries:
        group = (entry[2], entry[4])
        if group != current_group:
            if current_group is not None:
                lines.append("")
            lines.append(f"    // SERCOM{group[0]} / MUX {'C' if group[1] == 2 else 'D'}")
            current_group = group
        lines.append(format_entry(entry))

    lines.append("#endif")
    lines.append("")
    return lines


def format_entry(entry: Entry) -> str:
    port, port_pin, sercom_num, pad, mux = entry
    pin_name = f"P{chr(ord('A') + port)}{port_pin:02d}"
    pin_label = f"{pin_name}{'C' if mux == 2 else 'D'}"
    spacing = "  " if port_pin < 10 else " "
    return f"SERCOM_PIN({port}, {port_pin}, {sercom_num}, {pad}, {mux}){spacing}// {pin_label}"


def generate_include() -> str:
    device_pin_counts = load_device_pin_counts()
    series_checks: Dict[str, List[str]] = {name: [] for name in SERIES_ORDER}
    for device, pin_count in device_pin_counts.items():
        series = series_for_device(device, pin_count)
        if series is None:
            continue
        series_checks[series].extend(macro_candidates(device))

    samd21_tiers = load_mux_entries(
        SAMD21_TABLE_7_1,
        "I/O Pin",
        ("SAMD2xE", "SAMD2xG", "SAMD2xJ"),
    )
    samd51_tiers = load_mux_entries(
        SAMD51_TABLE_6_1,
        "Pad Name",
        ("48", "64", "100", "120", "128"),
    )

    samd21_32 = merge_tiers(samd21_tiers, 0)
    samd21_48_only = samd21_tiers.get(1, [])
    samd21_64_only = samd21_tiers.get(2, [])

    samd5x_48 = merge_tiers(samd51_tiers, 0)
    samd5x_64_only = samd51_tiers.get(1, [])
    samd5x_100_only = samd51_tiers.get(2, [])
    samd5x_120_only = samd51_tiers.get(3, []) + samd51_tiers.get(4, [])

    lines: List[str] = []
    lines.append("// Auto-generated by extras/pinfinder.py. Do not edit manually.")
    lines.append("// Source tables: extras/spipinmux/SAMD21_Table_7-1.csv, extras/spipinmux/SAMD51_Table_6-1.csv")
    lines.append("")

    lines.append("// Device-series macros (Wire-style gating pattern).")
    for series_name in SERIES_ORDER:
        lines.extend(format_series_macro(series_name, series_checks.get(series_name, [])))
        lines.append("")

    lines.append("// SAMD21 / DA1: shared routes then pin-count extras.")
    lines.extend(
        emit_entries_block(
            "SAMD21/DA1 shared E(32)+G(48)+J(64) routes",
            "SAMD21_DA1_32_SERIES || SAMD21_DA1_48_SERIES || SAMD21_DA1_64_SERIES",
            samd21_32,
        )
    )
    lines.extend(
        emit_entries_block(
            "SAMD21/DA1 48-pin and larger extra routes",
            "SAMD21_DA1_48_SERIES || SAMD21_DA1_64_SERIES",
            samd21_48_only,
        )
    )
    lines.extend(
        emit_entries_block(
            "SAMD21/DA1 64-pin extra routes",
            "SAMD21_DA1_64_SERIES",
            samd21_64_only,
        )
    )

    lines.append("// SAMD51 / SAME5x: shared routes then pin-count extras.")
    lines.extend(
        emit_entries_block(
            "SAMD5x/E5x shared 48-pin+ routes",
            "SAMD5X_E5X_48_SERIES || SAMD5X_E5X_64_SERIES || SAMD5X_E5X_100_SERIES || SAMD5X_E5X_120_SERIES",
            samd5x_48,
        )
    )
    lines.extend(
        emit_entries_block(
            "SAMD5x/E5x 64-pin and larger extra routes",
            "SAMD5X_E5X_64_SERIES || SAMD5X_E5X_100_SERIES || SAMD5X_E5X_120_SERIES",
            samd5x_64_only,
        )
    )
    lines.extend(
        emit_entries_block(
            "SAMD5x/E5x 100-pin and larger extra routes",
            "SAMD5X_E5X_100_SERIES || SAMD5X_E5X_120_SERIES",
            samd5x_100_only,
        )
    )
    lines.extend(
        emit_entries_block(
            "SAMD5x/E5x 120/128-pin extra routes",
            "SAMD5X_E5X_120_SERIES",
            samd5x_120_only,
        )
    )

    return "\n".join(lines)


def main() -> int:
    OUTPUT.write_text(generate_include(), encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
