#pragma once
#include "variant.h"
using neoPixelType = uint16_t;
constexpr neoPixelType NEO_GRB = 1;
class Adafruit_NeoPixel {
public:
  Adafruit_NeoPixel(uint16_t n = 0, uint8_t p = 6, neoPixelType = NEO_GRB)
      : numLEDs(n), numBytes(n * 3), pin(p), pixels(static_cast<uint8_t *>(calloc(numBytes, 1))) {}
  ~Adafruit_NeoPixel() { free(pixels); }
  void begin() {}

protected:
  uint16_t numLEDs, numBytes;
  int16_t pin;
  uint8_t *pixels;
  uint8_t wOffset = 1, rOffset = 1;
};
