// This is a PARED-DOWN NeoPixel example for the Adafruit_NeoPixel_SERCOM
// library, demonstrating pin declarations, etc.  For more complete examples
// of NeoPixel operations, see the examples included with the 'regular'
// Adafruit_NeoPixel library.

// Also requires LATEST Adafruit_NeoPixel and Adafruit_ZeroDMA libraries. SPI/SERCOM
// selects DMA or non-DMA transport internally.

#include <Adafruit_NeoPixel_SERCOM.h>

/*
  This branch discovers SERCOM routes at runtime. A pin works if the board's
  variant data maps it to a valid SERCOM MOSI route (PAD 0, 2, or 3).

  begin() returns false when a pin has no compatible route.

  Note: If you use a board's MOSI pin, that SPI peripheral is not available
  for other devices.
*/

#define PIN        12
#define NUM_PIXELS 30

Adafruit_NeoPixel_SERCOM strip(NUM_PIXELS, PIN, NEO_GRB);

void setup() {
  if (!strip.begin())
    while (1)
      ;

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
