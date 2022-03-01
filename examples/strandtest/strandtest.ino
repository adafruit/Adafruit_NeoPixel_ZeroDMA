// This is a PARED-DOWN NeoPixel example for the Adafruit_NeoPixel_ZeroDMA
// library, demonstrating pin declarations, etc.  For more complete examples
// of NeoPixel operations, see the examples included with the 'regular'
// Adafruit_NeoPixel library.

// Also requires LATEST Adafruit_NeoPixel and Adafruit_ZeroDMA libraries.

#include <Adafruit_NeoPixel_ZeroDMA.h>

/* DMA NeoPixels work ONLY on SPECIFIC PINS:
   Feather M0: pins 5, 6, 12 and MOSI*.
   Feather M0 Express: pins 6, 12 and MOSI*.
   Feather M4: pins 12, A2, A4 and MOSI*.
   ItsyBitsy M0: pins 5, 12 and MOSI*.
   ItsyBitsy M4: pins 2, 5, 12 and MOSI*.
   Metro M0 or Arduino Zero: pins 5, 12 and MOSI*.
   Metro M4: pins 6, 11, A3 and MOSI*.
   Metro M4 AirLift: pins 6, 11 and MOSI*.
   Grand Central: pins 11, 14, 23 and MOSI*.
   HalloWing M0: pins 4 (NEOPIX), 6 and MOSI*.
   HalloWing M4: pins 6, 8, A5 and MOSI*.
   MONSTER M4SK: pin 2.
   PyPortal, PyPortal Titano: pin 3 (SENSE connector).
   PyGamer, PyGamer Advance: pins 12 and A4.
   PyBadge, PyBadge AirLift: pins A4, MOSI*.
   Crickit M0: pins 8, 11, A8 and A11.
   Trellis M4: pin 10 (keypad NeoPixels).
   Circuit Playground M0: pin A2.
   Trinket M0: pin 4 (can't use with I2C, SPI or Serial1 active).
   Gemma M0: pin D0 (can't use with I2C, SPI or Serial1 active).
   QT Py: MOSI* and pin 16 (underside pad, can't use w/optional SPI flash).
   Arduino NANO 33 IoT: pins 4, 6, 7, A2, A3, MOSI*.
   * If using the MOSI pin on these boards, the corresponding SPI
     peripheral is not usable.
*/

#define PIN        12
#define NUM_PIXELS 30

Adafruit_NeoPixel_ZeroDMA strip(NUM_PIXELS, PIN, NEO_GRB);

void setup() {
  strip.begin();
  strip.setBrightness(32);
  strip.show();
}

void loop() {
  uint16_t i;

  // 'Color wipe' across all pixels
  for(uint32_t c = 0xFF0000; c; c >>= 8) { // Red, green, blue
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(50);
    }
  }

  // Rainbow cycle
  uint32_t elapsed, t, startTime = micros();
  for(;;) {
    t       = micros();
    elapsed = t - startTime;
    if(elapsed > 5000000) break; // Run for 5 seconds
    uint32_t firstPixelHue = elapsed / 32;
    for(i=0; i<strip.numPixels(); i++) {
      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
  }
}
