#ifndef _ADAFRUIT_NEOPIXEL_SERCOM_H_
#define _ADAFRUIT_NEOPIXEL_SERCOM_H_

#include <Adafruit_NeoPixel.h>
#include <SPI.h>

// For Adafruit SAMD boards, alias SPIClassSAMD to SPIClass so the
// same code works on Arduino or Adafruit SAMD boards (hardware SPI
// is implemented a bit different on each now).
#ifdef ARDUINO_SAMD_ADAFRUIT
typedef SPIClass SPIClassSAMD;
#endif

/** @brief Create a NeoPixel class that uses queued SPI to write strands in
    a non-blocking manner */
class Adafruit_NeoPixel_SERCOM : public Adafruit_NeoPixel {

public:
  Adafruit_NeoPixel_SERCOM(uint16_t n, uint8_t p = 6, neoPixelType t = NEO_GRB);
  // Uses NeoPixel default color order (NEO_GRB).
  Adafruit_NeoPixel_SERCOM(uint16_t n, uint8_t p, neoPixelType t, bool altSercom);
  Adafruit_NeoPixel_SERCOM(void);
  ~Adafruit_NeoPixel_SERCOM();

  bool begin(void);
  bool begin(SERCOM *sercom, uint8_t mosi, SercomSpiTXPad padTX, EPioType pinFunc);
  void show();
  void setBrightness(uint8_t);
  uint8_t getBrightness() const;
  /**
   * @brief Override NeoPixel canShow, this always returns true because we
   * double buffer
   * @returns True always */
  inline bool canShow(void) { return true; }

protected:
  SPIClassSAMD *spi;
  uint8_t *activeBuf;
  uint8_t *stagingBuf; ///< Buffer prepared while the active buffer is in flight
  uint16_t brightness; ///<  1 (off) to 256 (brightest)

private:
  bool _useAltSercom; ///< Prefer ALT SERCOM variant if available
  uint32_t frameBytes;
  volatile bool transferActive;
  volatile bool refreshPending;
  volatile bool stagingEncoding;
  bool ownsSpi;
  void releaseResources();
  static void transferComplete(void *user, int status);
  void handleTransferComplete(int status);
  void encodeInto(uint8_t *buffer);
  bool startTransfer(uint8_t *buffer);
  bool _setupSercomFromPin(SERCOM **outSercom, SercomSpiTXPad *outPadTX, EPioType *outPinFunc);
};

#endif // _ADAFRUIT_NEOPIXEL_SERCOM_H_
