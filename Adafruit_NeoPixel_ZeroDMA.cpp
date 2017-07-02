/*
TO DO:
setPin(uint8_t p)
updateLength(uint16_t n)
updateType(neoPixelType t)
*/

#include "Adafruit_NeoPixel_ZeroDMA.h"
#include "wiring_private.h" // pinPeripheral() function
#include "bittable.h"       // Optional, see comments in show()

Adafruit_NeoPixel_ZeroDMA::Adafruit_NeoPixel_ZeroDMA(
  uint16_t n, uint8_t p, neoPixelType t) : Adafruit_NeoPixel(n, p, t),
  brightness(256), dmaBuf(NULL), spi(NULL) {
}

// Might not have this constuctor option...gets nasty
Adafruit_NeoPixel_ZeroDMA::Adafruit_NeoPixel_ZeroDMA(void) :
  Adafruit_NeoPixel(), brightness(256), dmaBuf(NULL), spi(NULL) {
}

Adafruit_NeoPixel_ZeroDMA::~Adafruit_NeoPixel_ZeroDMA() {
  dma.abort();
  if(spi) {
    spi->endTransaction();
#ifdef SPI
    if(spi != &SPI) delete spi;
#endif
  }
  if(dmaBuf) free(dmaBuf);
}

struct {
  SERCOM        *sercom;
  Sercom        *sercomBase;
  uint8_t        dmacID, mosi, miso, sck;
  SercomSpiTXPad padTX;
  SercomRXPad    padRX;
  EPioType       pinFunc;
} sercomTable[] = {
#ifdef ARDUINO_GEMMA_M0
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,  0, 14,  2, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_2, PIO_SERCOM_ALT,
#else
#ifdef ADAFRUIT_CIRCUITPLAYGROUND_M0
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,  8, A5, A4, SPI_PAD_3_SCK_1, SERCOM_RX_PAD_0, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX, A2, A9, A3, SPI_PAD_2_SCK_3, SERCOM_RX_PAD_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX, A7,  5, A6, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_2, PIO_SERCOM_ALT,
#else
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,  5,  3,  2, SPI_PAD_3_SCK_1, SERCOM_RX_PAD_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX, 11, 12, 13, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX, 23, 22, 24, SPI_PAD_2_SCK_3, SERCOM_RX_PAD_0, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX, A5,  6,  7, SPI_PAD_0_SCK_3, SERCOM_RX_PAD_2, PIO_SERCOM_ALT,
#endif // end not ADAFRUIT_CIRCUITPLAYGROUND_M0
#endif // end not ARDUINO_GEMMA_M0
};
#define N_SERCOMS (sizeof(sercomTable) / sizeof(sercomTable[0]))

boolean Adafruit_NeoPixel_ZeroDMA::begin(void) {

  uint8_t i;
  for(i=0; (i<N_SERCOMS) && (sercomTable[i].mosi != pin); i++);
  if(i >= N_SERCOMS) return false; // Invalid pin

  Adafruit_NeoPixel::begin(); // Call base class begin() function 1st
  // TO DO: Check for successful malloc in base class here

  uint8_t  bytesPerPixel = (wOffset = rOffset) ? 3 : 4;
  uint32_t bytesTotal    = (numLEDs * bytesPerPixel * 8 * 3 + 7) / 8 + 90;
  if((dmaBuf = (uint8_t *)malloc(bytesTotal))) {
#ifdef SPI
    spi = (pin == 23) ? &SPI :
#else
    spi =
#endif
      new SPIClass(sercomTable[i].sercom,
        sercomTable[i].miso, sercomTable[i].sck, sercomTable[i].mosi,
        sercomTable[i].padTX, sercomTable[i].padRX);
    if((spi)) {
      spi->begin();
      pinPeripheral(sercomTable[i].mosi, sercomTable[i].pinFunc);
      dma.configure_peripheraltrigger(sercomTable[i].dmacID);
      dma.configure_triggeraction(DMA_TRIGGER_ACTON_BEAT);
      if(STATUS_OK == dma.allocate()) {
        dma.setup_transfer_descriptor(
          dmaBuf,             // move data from here
          (void *)(&sercomTable[i].sercomBase->SPI.DATA.reg), // to here
          bytesTotal,         // this many...
          DMA_BEAT_SIZE_BYTE, // bytes/hword/words
          true,               // increment source addr?
          false);             // increment dest addr?
        if(STATUS_OK == dma.add_descriptor()) {
          dma.loop(true); // DMA transaction loops forever! Latch is built in.
          memset(dmaBuf, 0, bytesTotal); // IMPORTANT - clears latch data at end
          // SPI transaction is started BUT NEVER ENDS.  This is important.
          // 800 khz * 3 = 2.4MHz
          spi->beginTransaction(SPISettings(2400000, MSBFIRST, SPI_MODE0));
          if(STATUS_OK == dma.start_transfer_job()) {
            return true; // SUCCESS
          } // Else various errors, clean up partially-initialized stuff:
          spi->endTransaction();
        }
        dma.free();
      }
#ifdef SPI
      if(spi != &SPI) delete spi;
#endif
      spi = NULL;
    }
    free(dmaBuf);
    dmaBuf = NULL;
  }
  return false;
}

void Adafruit_NeoPixel_ZeroDMA::show(void) {
#ifdef _BITTABLE_H_
  // If bittable.h is included, 3:1 bit expansion is handled using a table
  // lookup -- each byte of input (from NeoPixel buffer) is replaced with
  // three bytes output (from table to DMA buffer).  This is quick but the
  // table requires about 768 bytes of code space.
  uint8_t *in = pixels, *out = dmaBuf, *table;
  for(uint16_t p=numBytes; p--;) {
    table  = (uint8_t *)&bitExpand[(*in++ * brightness) >> 8];
    *out++ = *table++;
    *out++ = *table++;
    *out++ = *table++;
  }
#else
  // If bittable.h is NOT included, 3:1 bit expansion is done on the fly.
  // More complex, but smaller executable.
  uint32_t bitOffset = 0, byteOffset;
  uint8_t  c, b, bitOffsetWithinByte, bitMask, *ptr = pixels;
  for(uint16_t p=0; p<numBytes; p++) {
    c = (*ptr++ * brightness) >> 8;
    for(b=0x80; b; b >>= 1) {
      byteOffset          = bitOffset / 8;
      bitOffsetWithinByte = bitOffset & 7;
      bitMask = (c & b) ? 0b110 : 0b100;
      if(bitOffsetWithinByte <= 5) {
        dmaBuf[byteOffset] = (dmaBuf[byteOffset]
          & ~(0b111   << (5 - bitOffsetWithinByte)))
          |  (bitMask << (5 - bitOffsetWithinByte));
      } else {
        dmaBuf[byteOffset] = (dmaBuf[byteOffset]
          & ~(0b111   >> (bitOffsetWithinByte - 5)))
          |  (bitMask >> (bitOffsetWithinByte - 5));
        byteOffset++;
        dmaBuf[byteOffset] = dmaBuf[byteOffset]
          & ~(0b111   << (13 - bitOffsetWithinByte))
          |  (bitMask << (13 - bitOffsetWithinByte));
      }
      bitOffset += 3;
    }
  }
#endif // !_BITTABLE_H_
}

// Brightness is stored differently here than in normal NeoPixel library.
// In either case it's *specified* the same: 0 (off) to 255 (brightest).
// Classic NeoPixel rearranges this internally so 0 is max, 1 is off and
// 255 is just below max...it's a decision based on how fixed-point math
// is handled in that code.  Here it's stored internally as 1 (off) to
// 256 (brightest), requiring a 16-bit value.

void Adafruit_NeoPixel_ZeroDMA::setBrightness(uint8_t b) {
  brightness = (uint16_t)b + 1; // 0-255 in, 1-256 out
}

uint8_t Adafruit_NeoPixel_ZeroDMA::getBrightness(void) const {
  return brightness - 1; // 1-256 in, 0-255 out
}
