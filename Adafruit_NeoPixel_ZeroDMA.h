#ifndef _ADAFRUIT_NEOPIXEL_ZERODMA_H_
#define _ADAFRUIT_NEOPIXEL_ZERODMA_H_

#include <Adafruit_NeoPixel.h>
#include <Adafruit_ZeroDMA.h>
#include <SPI.h>

class Adafruit_NeoPixel_ZeroDMA : public Adafruit_NeoPixel {

 public:

  Adafruit_NeoPixel_ZeroDMA(uint16_t n, uint8_t p=6, neoPixelType t=NEO_GRB);
  Adafruit_NeoPixel_ZeroDMA(void);
  ~Adafruit_NeoPixel_ZeroDMA();

  boolean     begin(SERCOM *sercom, Sercom *sercomBase, uint8_t dmacID, uint8_t mosi, uint8_t miso, uint8_t sck, SercomSpiTXPad padTX, SercomRXPad padRX, EPioType pinFunc);
  boolean     begin(void);
  void        show(),
              setBrightness(uint8_t);
  uint8_t     getBrightness() const;
  inline bool canShow(void) { return true; }

 protected:

  Adafruit_ZeroDMA dma;
  SPIClass        *spi;
  uint8_t         *dmaBuf;
  uint16_t         brightness;
#ifdef __SAMD51__
  // Hacky stuff for Trellis M4: PA27 (to NeoPixel matrix) is not on a
  // SERCOM, nor a pattern generator pin (which would work with NeoPXL8),
  // so we use the PORT toggle register to DMA NeoPixel data out. This is
  // not RAM-efficient but we're just looking to control the 32 pixels of
  // that matrix, not arbitrary-length strips, so the waste is localized.
  uint8_t          toggleMask; // Port bit to toggle
#endif
};

#endif // _ADAFRUIT_NEOPIXEL_ZERODMA_H_
