"""Run the library checks with the pinned SimIO Framework."""
import argparse
import subprocess
import sys
from pathlib import Path


FRAMEWORK_SHA = "76492a7993f63a2c4687fcdeb5a57853a2272598"
ROOT = Path(__file__).resolve().parents[2]


def run(command, cwd=ROOT):
    subprocess.run(command, cwd=cwd, check=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--framework", required=True, type=Path)
    parser.add_argument("--base-ref", required=True)
    parser.add_argument("--pio", default="pio")
    parser.add_argument("--clang-format", default="clang-format")
    parser.add_argument("--doxygen", default="doxygen")
    args = parser.parse_args()
    run(["git", "diff", "--check", args.base_ref])
    framework = args.framework.resolve()
    actual = subprocess.check_output(
        ["git", "-C", str(framework), "rev-parse", "HEAD"], text=True
    ).strip()
    if actual != FRAMEWORK_SHA:
        raise SystemExit(f"SimIOFramework must be {FRAMEWORK_SHA}, got {actual}")

    build = ROOT / "build" / "ci"
    build.mkdir(parents=True, exist_ok=True)
    config = build / "platformio.ini"
    config.write_text(
        "[platformio]\n"
        "packages_dir = packages\n"
        "default_envs = metro_m0, metro_m4, metro_m0_irq, metro_m4_irq\n"
        "src_dir = ../../examples/strandtest\n"
        "\n[env]\n"
        "platform = atmelsam@8.3.0\n"
        "framework = arduino\n"
        "platform_packages =\n"
        f"  framework-arduino-samd-adafruit@symlink://{framework}\n"
        "  framework-cmsis-atmel@https://github.com/crabel99/ArduinoModule-CMSIS-Atmel.git#e3072fc23b6ebbd54f0e05a3f65d1e58497aba7d\n"
        "lib_deps =\n"
        f"  symlink://{ROOT}\n"
        f"  symlink://{framework / 'libraries' / 'Adafruit_ZeroDMA'}\n"
        "  adafruit/Adafruit NeoPixel@1.15.5\n"
        "build_flags =\n"
        "  -DARDUINO_SAMD_ADAFRUIT\n"
        "  -DUSE_TINYUSB\n"
        f"  -I{framework / 'libraries' / 'Adafruit_ZeroDMA'}\n"
        "\n[env:metro_m0]\n"
        "board = adafruit_metro_m0\n"
        "build_flags = ${env.build_flags} -DUSE_ZERODMA\n"
        "\n[env:metro_m4]\n"
        "board = adafruit_metro_m4\n"
        "build_flags = ${env.build_flags} -DUSE_ZERODMA\n"
        "\n[env:metro_m0_irq]\n"
        "board = adafruit_metro_m0\n"
        "\n[env:metro_m4_irq]\n"
        "board = adafruit_metro_m4\n"
    )
    run([sys.executable, "-m", "unittest", "discover", "-s", "extras"])
    for style in ("legacy", "native"):
        run(
            [
                sys.executable,
                "extras/native/run.py",
                "--register-style",
                style,
                "--report",
                f"ci-native-{style}.json",
            ]
        )
    for board in ("metro_m0", "metro_m4", "metro_m0_irq", "metro_m4_irq"):
        run([args.pio, "run", "-d", str(build), "-e", board, "-t", "clean"])
        run([args.pio, "run", "-d", str(build), "-e", board])
    sources = subprocess.check_output(
        [
            "git",
            "ls-files",
            "*.[ch]",
            "*.cc",
            "*.cpp",
            "*.cxx",
            "*.hpp",
        ],
        cwd=ROOT,
        text=True,
    ).splitlines()
    sources = [name for name in sources if name != "pins.inc" and (ROOT / name).is_file()]
    run([args.clang_format, "--dry-run", "--Werror", *sources])
    run([args.doxygen, "Doxyfile"])


if __name__ == "__main__":
    main()
