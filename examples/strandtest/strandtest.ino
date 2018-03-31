// This is a PARED-DOWN NeoPixel example for the Adafruit_NeoPixel_ZeroDMA
// library, demonstrating pin declarations, etc.  For more complete examples
// of NeoPixel operations, see the examples included with the 'regular'
// Adafruit_NeoPixel library.

// Also requires LATEST Adafruit_NeoPixel and Adafruit_ZeroDMA libraries.

#include <Adafruit_NeoPixel_ZeroDMA.h>

// DMA NeoPixels work ONLY on SPECIFIC PINS.
// On Circuit Playground Express: 8, A2 and A7 (TX) are valid.
// On Feather M0, Arduino Zero, etc.: 5, 11, A5 and 23 (SPI MOSI).
// On GEMMA M0: pin 0.
// On Trinket M0: pin 4.
// On Metro M4: 3, 6, 8, 11, A3 and MOSI
#define PIN         8
#define NUM_PIXELS 10

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
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((uint8_t)(
        (elapsed * 256 / 1000000) + i * 256 / strip.numPixels())));
    }
    strip.show();
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
