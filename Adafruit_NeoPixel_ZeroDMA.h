#ifndef _ADAFRUIT_NEOPIXEL_ZERODMA_H_
#define _ADAFRUIT_NEOPIXEL_ZERODMA_H_

#include <Adafruit_NeoPixel.h>
#include <Adafruit_ZeroDMA.h>
#include <SPI.h>

// For Adafruit SAMD boards, alias SPIClassSAMD to SPIClass so the
// same code works on Arduino or Adafruit SAMD boards (hardware SPI
// is implemented a bit different on each now).
#ifdef ARDUINO_SAMD_ADAFRUIT
typedef SPIClass SPIClassSAMD;
#endif

/** @brief Create a NeoPixel class that uses SPI DMA to write strands in
    a non-blocking manner */
class Adafruit_NeoPixel_ZeroDMA : public Adafruit_NeoPixel {

public:
  Adafruit_NeoPixel_ZeroDMA(uint16_t n, uint8_t p = 6,
                            neoPixelType t = NEO_GRB);
  Adafruit_NeoPixel_ZeroDMA(void);
  ~Adafruit_NeoPixel_ZeroDMA();

  bool begin(void);
  // Although esoteric, there IS a use case for keeping this overloaded
  // begin() variant public, please DO NOT move to the protected section.
  bool begin(SERCOM *sercom, Sercom *sercomBase, uint8_t dmacID, uint8_t mosi,
             SercomSpiTXPad padTX, EPioType pinFunc);
  void show();
  void setBrightness(uint8_t);
  uint8_t getBrightness() const;
  /**
   * @brief Override NeoPixel canShow, this always returns true because we
   * double buffer
   * @returns True always */
  inline bool canShow(void) { return true; }

protected:
  Adafruit_ZeroDMA dma; ///< The DMA manager for the SPI class
  SPIClassSAMD *spi;    ///< Underlying SPI hardware interface we use to DMA
  uint8_t *dmaBuf;      ///< The raw buffer we write to SPI to mimic NeoPixel
  uint16_t brightness;  ///<  1 (off) to 256 (brightest)
#ifdef __SAMD51__
  // Hacky stuff for Trellis M4: PA27 (to NeoPixel matrix) is not on a
  // SERCOM, nor a pattern generator pin (which would work with NeoPXL8),
  // so we use the PORT toggle register to DMA NeoPixel data out. This is
  // not RAM-efficient but we're just looking to control the 32 pixels of
  // that matrix, not arbitrary-length strips, so the waste is localized.
  uint8_t toggleMask; // Port bit to toggle
#endif
};

#endif // _ADAFRUIT_NEOPIXEL_ZERODMA_H_
