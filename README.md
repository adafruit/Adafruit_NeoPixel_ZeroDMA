# Adafruit_NeoPixel_ZeroDMA
DMA-based NeoPixel library for M0 microcontrollers (Feather M0, Arduino Zero, etc.) Doesn't require stopping interrupts, so millis() & micros() don't lose time, soft PWM (for servos, etc.) still operate normally, etc.

Requires LATEST Adafruit_NeoPixel and Adafruit_ZeroDMA libraries also be installed.

THIS IS A WORK-IN-PROGRESS AND NOT 100% THERE YET.

THIS ONLY WORKS ON CERTAIN PINS. THIS IS NORMAL. Library uses SERCOM peripherals for SPI output, and the hardware only supports this on specific pins (plus, some SERCOMs are in use for Serial, I2C, etc.).

On Circuit Playground Express: 8, A2 and A7 (TX) are valid.
On Feather M0, Arduino Zero, etc.: 5, 11, A5 and 23 (SPI MOSI).
On GEMMA M0: pin 0.

OTHER THINGS TO KNOW:

DMA NeoPixels use a LOT of RAM: 12 bytes/pixel for RGB, 16 bytes/pixel for RGBW, about 4X as much as regular NeoPixel library (plus a little bit extra for structures & stuff).

0/1 bit timing does not precisely match NeoPixel/WS2812/SK6812 datasheet specs, but it seems to work well enough. Use at your own peril.

Have not tested this yet with multiple instances (DMA-driven NeoPixels on multiple pins), but in theory it should work. Should also be OK mixing DMA and non-DMA NeoPixels in same sketch (just use different constructor and pins for each).

Currently this only supports strip declaration with length & pin known at compile time, so it's not a 100% drop-in replacement for all NeoPixel code right now. But probably 99%+ of all sketches are written that way, so it's perfectly usable for most. The stock NeoPixel library has the option of setting the length & pin number at run-time (so these can be stored in a config file or in EEPROM)...this is entirely possible here, just hasn't been written yet.
