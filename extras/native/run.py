"""Compile the actual checkout against observable native hardware fakes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--register-style", choices=("legacy", "native"), default="legacy")
parser.add_argument("--report", default="results.json")
args = parser.parse_args()
root = Path(__file__).resolve().parent
source = root.parents[1]
build = source / "build/native-lifecycle"
build.mkdir(parents=True, exist_ok=True)
environment = dict(os.environ, TMPDIR=str(build))
executable = build / "lifecycle"
command = ["/usr/bin/c++", "-std=c++17", "-Wall", "-Wextra", "-DARDUINO_SAMD_ADAFRUIT", "-D__SAME54P20A__", "-I"+str(root/"fakes"), str(root/"lifecycle.cpp"), "-o", str(executable)]
if args.register_style == "native":
    command.insert(1, "-DFAKE_NATIVE_REGISTERS")
result = subprocess.run(command, env=environment, text=True, capture_output=True)
(build/(args.register_style+"-build.log")).write_text(result.stdout+result.stderr)
if result.returncode:
    print(result.stdout+result.stderr)
    raise SystemExit(result.returncode)
cases = ("spi_pipeline_required", "pending_frames", "completion_during_publish", "in_flight_buffer", "transfer_failure", "destroy_active", "never_begun", "invalid_pin", "spi_begin_failure", "buffer_failure_1", "buffer_failure_2", "borrowed_spi", "begin_twice", "repeated_lifetimes", "irq_restore")
records = []
for case in cases:
    result = subprocess.run([str(executable),case],env=environment,text=True,capture_output=True)
    print(result.stdout, end="")
    records.append(dict(case=case,exit_code=result.returncode,stdout=result.stdout,stderr=result.stderr))
report = dict(command=command,source=str(source),sha256={name:hashlib.sha256((source/name).read_bytes()).hexdigest() for name in ("Adafruit_NeoPixel_SERCOM.cpp","Adafruit_NeoPixel_SERCOM.h")}, cases=records)
(build/"source-contract.json").write_text(json.dumps({"forbidden": {name: token not in (source/name).read_text() for name, token in (("Adafruit_NeoPixel_SERCOM.h", "Adafruit_ZeroDMA"), ("Adafruit_NeoPixel_SERCOM.cpp", "Adafruit_ZeroDMA"), ("Adafruit_NeoPixel_SERCOM.cpp", "dma.setTrigger"))}}, indent=2)+"\n")
(build/args.report).write_text(json.dumps(report,indent=2)+"\n")
raise SystemExit(1 if any(r["exit_code"] for r in records) else 0)
