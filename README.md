# Adafruit_NeoPixel_ZeroDMA
DMA-based NeoPixel library for SAMD21 and SAMD51 microcontrollers
(Feather M0, M4, etc.) Doesn't require stopping interrupts, so millis() and
micros() don't lose time, soft PWM (for servos, etc.) still operate normally.

Requires LATEST Adafruit_NeoPixel and Adafruit_ZeroDMA libraries also be
installed (Adafruit SAMD board support automatically includes
Adafruit_ZeroDMA).

THIS ONLY WORKS ON CERTAIN PINS. THIS IS NORMAL. Library uses SERCOM
peripherals for SPI output, and the hardware only supports this on specific
pins (plus, some SERCOMs are in use for Serial, I2C, etc.).

AS OF VERSION 1.2.0: the selection of pins is more restrictive than before,
to better avoid collisions with other peripherals (I2C, etc.)...with few
exceptions, you can use DMA NeoPixels with impunity. And more boards are
supported now.

COMPATIBLE BOARDS AND PINS:

* Feather M0: pins 5, 6, 12 and MOSI\*.
* Feather M0 Express: pins 6, 12 and MOSI\*.
* Feather M4: pins 12, A2, A4 and MOSI\*.
* ItsyBitsy M0: pins 5, 12 and MOSI\*.
* ItsyBitsy M4: pins 2, 5, 12 and MOSI\*.
* Metro M0 or Arduino Zero: pins 5, 12 and MOSI\*.
* Metro M4: pins 6, 11, A3 and MOSI\*.
* Metro M4 AirLift: pins 6, 11 and MOSI\*.
* Grand Central: pins 11, 14, 23 and MOSI\*.
* HalloWing M0: pins 4 (NEOPIX), 6 and MOSI\*.
* HalloWing M4: pins 6, 8, A5 and MOSI\*.
* MONSTER M4SK: pin 2.
* PyPortal, PyPortal Titano: pin 3 (SENSE connector).
* PyGamer, PyGamer Advance: pins 12 and A4.
* PyBadge, PyBadge AirLift: pins A4, MOSI\*.
* Crickit M0: pins 8, 11, A8 and A11.
* Trellis M4: pin 10 (keypad NeoPixels).
* Circuit Playground M0: pin A2.
* Trinket M0: pin 4 (can't use simultaneously with I2C, SPI or Serial1).
* Gemma M0: pin D0 (can't use simultaneously with I2C, SPI or Serial1).
* QT Py: MOSI\* and pin 16 (underside pad, can't use w/optional SPI flash).

\* If using the MOSI pin on these boards, the corresponding SPI peripheral is
not usable. A few add-ons (usually TFT display shields/wings) rely on SPI,
so avoid NeoPixeling from this pin in such situations. MOSI is really only
offered anymore to maintain partial compatibility with older projects that
might've used the earlier library, which was less selective about such things.

OTHER THINGS TO KNOW:

DMA NeoPixels use a LOT of RAM: 12 bytes/pixel for RGB, 16 bytes/pixel for
RGBW, about 4X as much as regular NeoPixel library (plus a little bit extra
for structures & stuff).

0/1 bit timing does not precisely match NeoPixel/WS2812/SK6812 datasheet
specs, but it seems to work well enough. Use at your own peril.

Have not tested this yet with multiple instances (DMA-driven NeoPixels on
multiple pins), but in theory it should work. Should also be OK mixing DMA
and non-DMA NeoPixels in same sketch (just use different constructor and
pins for each).

Currently this only supports strip declaration with length & pin known at
compile time, so it's not a 100% drop-in replacement for all NeoPixel code
right now. But probably 99%+ of all sketches are written that way, so it's
perfectly usable for most. The stock NeoPixel library has the option of
setting the length & pin number at run-time (so these can be stored in a
config file or in EEPROM)...but those functions are now considered
deprecated, so we should be OK going forward.
