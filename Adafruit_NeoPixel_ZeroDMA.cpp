#include "Adafruit_NeoPixel_ZeroDMA.h"
#include "wiring_private.h" // pinPeripheral() function

Adafruit_NeoPixel_ZeroDMA::Adafruit_NeoPixel_ZeroDMA(
  uint16_t n, uint8_t p, neoPixelType t) : Adafruit_NeoPixel(n, p, t),
  brightness(256), dmaBuf(NULL), spi(NULL) {
}

// Might not have this constuctor option...gets nasty
Adafruit_NeoPixel_ZeroDMA::Adafruit_NeoPixel_ZeroDMA(void) :
  Adafruit_NeoPixel(), brightness(256), dmaBuf(NULL), spi(NULL) {
}

// TO DO: all the pin-change and length-change uglies

Adafruit_NeoPixel_ZeroDMA::~Adafruit_NeoPixel_ZeroDMA() {
  dma.abort();
  if(spi)    spi->endTransaction();
  if(dmaBuf) free(dmaBuf);
}

#define DMA_SERCOM SERCOM1
#define DMA_SERCOM_DMAC_ID_TX SERCOM1_DMAC_ID_TX
boolean Adafruit_NeoPixel_ZeroDMA::begin(void) {
  Adafruit_NeoPixel::begin(); // Call base class begin() function 1st
  uint8_t  bytesPerPixel = (wOffset = rOffset) ? 3 : 4;
  uint32_t bytesTotal    = (numLEDs * bytesPerPixel * 8 * 3 + 7) / 8 + 90;
  if((dmaBuf = (uint8_t *)malloc(bytesTotal))) {
    if((spi = new SPIClass(&sercom1, 12, 13, 11,
      SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3))) {
      spi->begin();
      pinPeripheral(11, PIO_SERCOM);
      dma.configure_peripheraltrigger(DMA_SERCOM_DMAC_ID_TX);
      dma.configure_triggeraction(DMA_TRIGGER_ACTON_BEAT);
      if(STATUS_OK == dma.allocate()) {
        dma.setup_transfer_descriptor(
          dmaBuf,                              // move data from here
          (void *)(&DMA_SERCOM->SPI.DATA.reg), // to here
          bytesTotal,                          // this many...
          DMA_BEAT_SIZE_BYTE,                  // bytes/hword/words
          true,                                // increment source addr?
          false);                              // increment dest addr?
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
      delete spi;
      spi = NULL;
    }
    free(dmaBuf);
    dmaBuf = NULL;
  }
  return false;
}

// TO DO: might change this to use a table for expanding each byte 3X.
// Quicker but table requires 768 bytes flash, might be worth it.
void Adafruit_NeoPixel_ZeroDMA::show(void) {
  uint32_t bitOffset = 0, byteOffset;
  uint8_t  c, b, bitOffsetWithinByte, bitMask, *ptr = pixels;
  for(int p=0; p<numBytes; p++) {
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
}

void Adafruit_NeoPixel_ZeroDMA::setBrightness(uint8_t b) {
  brightness = (uint16_t)b + 1;
}


uint8_t Adafruit_NeoPixel_ZeroDMA::getBrightness(void) const {
  return brightness - 1;
}
