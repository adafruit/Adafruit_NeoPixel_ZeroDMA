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
};

#endif // _ADAFRUIT_NEOPIXEL_ZERODMA_H_
