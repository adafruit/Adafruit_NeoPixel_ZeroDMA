# Adafruit_NeoPixel_SERCOM

Queued-SPI NeoPixel library for SAMD21, SAMD51, and SAME5x microcontrollers.
Frames are encoded into a staging buffer and submitted through the core SPI
queue. The active frame stays unchanged until its completion callback.

Requires Adafruit_NeoPixel and Adafruit Zero DMA Library. SPI/SERCOM selects
DMA or non-DMA transport internally; this library does not select or manage DMA.
The core must provide the completion-callback SPI API proposed in
[ArduinoCore-samd #395](https://github.com/adafruit/ArduinoCore-samd/pull/395).
Stock cores without that API are not supported.

Include `<Adafruit_NeoPixel_SERCOM.h>` and construct an
`Adafruit_NeoPixel_SERCOM` object. The repository URL retains its original name.

THIS ONLY WORKS ON CERTAIN PINS. THIS IS NORMAL.

This branch discovers pin support at runtime using the board's pin mux data and
generated SAMD21/SAMD51/SAME5x SERCOM route tables. `begin()` succeeds only
when the selected pin has a valid SERCOM MOSI route (SERCOM PAD 0, 2, or 3).
If no compatible route exists, `begin()` returns `false`.

If using a board's MOSI pin for NeoPixels, that SPI peripheral is not usable
for other devices.

OTHER THINGS TO KNOW:

Queued SPI NeoPixels use 21 bytes per RGB pixel or 28 bytes per RGBW pixel,
plus 180 reset bytes. The base pixel buffer and two expanded frame buffers are
kept at the same time.

0/1 bit timing does not precisely match NeoPixel/WS2812/SK6812 datasheet
specs, but it seems to work well enough. Use at your own peril.

The selected SPI peripheral is dedicated to the strip. Calls to `show()`
coalesce while a frame is active, so the newest pending frame follows it.

Length/pin/type can be provided at run time when constructing the strip
object. What is not supported as a drop-in workflow is changing those values
after `begin()` has initialized SPI resources.

If your project needs configurable strip settings, construct a new
Adafruit_NeoPixel_SERCOM instance with the desired values before calling
`begin()`.

## Verification

CI runs the Python and native lifecycle checks and compiles `strandtest` for
Adafruit Metro M0 and Metro M4 with both DMA and interrupt-driven SPI against
the pinned Framework PR revision.
It also checks C++ formatting with clang-format 18.1.8 and generates Doxygen
output in `build/doxygen`.

Run the same checks locally with
`python extras/ci/run.py --framework <checkout> --base-ref upstream/master`.
The base reference must identify the target branch of the pull request so the
whitespace check covers the whole proposed change.
The checkout must be at the revision recorded by the script. PlatformIO 6.1.19,
clang-format 18.1.8, Doxygen, and Graphviz must be available on the command path; the script
also accepts explicit `--pio`, `--clang-format`, and `--doxygen` paths.
