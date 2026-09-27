# NeoPixel lifecycle tests

These host tests compile the checkout's actual `Adafruit_NeoPixel_SERCOM.cpp`
through `lifecycle.cpp`. They replace Arduino and SPI with observable
fakes. No firmware source is copied or generated.

Run from the library root using the SimIO Backend interpreter:

```sh
/Users/crabel/Documents/src/SimIO/SimIOBackend/.venv/bin/python extras/native/run.py --register-style legacy --report legacy-after.json
/Users/crabel/Documents/src/SimIO/SimIOBackend/.venv/bin/python extras/native/run.py --register-style native --report native-after.json
```

The runner stores compiler scratch files, executable, compiler output, source
hashes and case results in `build/native-lifecycle/`. It sets `TMPDIR` to that
workspace build directory. The compiler is the host `/usr/bin/c++`, not a
replacement for PlatformIO firmware builds.

The legacy fake exposes `Sercom`, `SERCOMn` and `SPI.DATA.reg`. The native fake
exposes `sercom_registers_t`, `SERCOMn_REGS` and `SPIM.SERCOM_DATA`. Both modes
execute the same lifecycle cases.

## Behavior checked

- A successful begin allocates two frame buffers without submitting SPI work.
  Show queues a finite frame; completion submits the newest pending frame.
- Destruction ends SPI before freeing an active source buffer.
- Destruction before begin or after invalid-pin rejection releases nothing.
- Failed SPI begin and either frame-buffer allocation release partial resources.
- Borrowed SPI is not deleted. Repeated `begin()` calls retain one allocation.
- Repeated active lifetimes cancel SPI before their frame buffers are released.

For buffer accounting the production translation unit's `malloc` and `free`
calls are redirected to tracked functions. The SPI fake retains a submitted
source pointer, supports partial reads, and invokes completion only when the
test finishes that source.

## Evidence and limits

`spi-pipeline-before.json` records the required red assertion against the old
implementation. It failed because `show()` did not submit through SPI.

The fakes do not reproduce physical SERCOM timing, queue contention, flash ECC,
or the SAME54 cold-start fault. PlatformIO compilation and hardware acceptance
remain separate requirements.

## Pending-frame regression

`pending_frames` submits A, B, and C before completing A. The newest pending
frame C follows A. `completion_during_publish` completes A at the final masked
publication boundary in `show(C)`. `in_flight_buffer` reads part of A before
preparing B and C, then confirms SPI consumes A and C unchanged.
