/*
DMA NeoPixel library for M0-based boards (Feather M0, Arduino Zero, etc.).
Doesn't require stopping interrupts, so millis()/micros() don't lose time,
soft PWM (for servos, etc.) still operate normally, etc.
THIS IS A WORK-IN-PROGRESS AND NOT 100% THERE YET.

THIS ONLY WORKS ON CERTAIN PINS.  THIS IS NORMAL.  Library uses SERCOM
peripherals for SPI output, and the hardware only supports this on
specific pins (plus, some SERCOMs are in use for Serial, I2C, etc.).
See example sketch for explanation.

0/1 bit timing does not precisely match NeoPixel/WS2812/SK6812 datasheet
specs, but it seems to work well enough.  Use at your own peril.

Currently this only supports strip declaration with length & pin known at
compile time, so it's not a 100% drop-in replacement for all NeoPixel code
right now.  But probably 99%+ of all sketches are written that way, so it's
perfectly usable for most.  The stock NeoPixel library has the option of
setting the length & pin number at run-time (so these can be stored in a
config file or in EEPROM)...this is entirely possible here, just hasn't
been written yet.  In that regard, TO DO:
setPin(uint8_t p)
updateLength(uint16_t n)
updateType(neoPixelType t)
UPDATE: no, don't.  Please just use the C++ 'new' operator to allocate a
strip (passing length & type) if needed that way.  It's been added to the
NeoPixel library roadmap that these functions are deprecated.

Have not tested this yet with multiple instances (DMA-driven NeoPixels on
multiple pins), but in theory it should work.  Should also be OK mixing
DMA and non-DMA NeoPixels in same sketch (just use different constructor
and pins for each).
*/

#include "Adafruit_NeoPixel_ZeroDMA.h"
#include "bittable.h"       // Optional, see comments in show()
#include "pins.h"           // SPI DMA capable pin tables (per device)
#include "wiring_private.h" // pinPeripheral() function

/** @brief Initialize a NeoPixel strand
    @param n Number of pixels
    @param p Pin to use (we will figure out what Sercom to use
    @param t The color order / type of pixels
*/
Adafruit_NeoPixel_ZeroDMA::Adafruit_NeoPixel_ZeroDMA(uint16_t n, uint8_t p,
                                                     neoPixelType t)
    : Adafruit_NeoPixel(n, p, t), brightness(256), dmaBuf(NULL), spi(NULL) {}

/** @brief Create a NOT FINISHED onject -- need setPin(), updateLength(),
    updateType() for this.
    Will require stopping DMA, reallocating, restarting DMA.  Fun times.
*/
Adafruit_NeoPixel_ZeroDMA::Adafruit_NeoPixel_ZeroDMA(void)
    : Adafruit_NeoPixel(), brightness(256), dmaBuf(NULL), spi(NULL) {}

Adafruit_NeoPixel_ZeroDMA::~Adafruit_NeoPixel_ZeroDMA() {
  dma.abort();
  if (spi) {
    spi->endTransaction();
#ifdef SPI
    if (spi != &SPI)
      delete spi;
#endif
  }
  if (dmaBuf)
    free(dmaBuf);
}

/** @brief Initialize the underlying SPI SERCOM for DMA transfers
    @param sercom Pointer to the underlying SERCOM from the Arduino core
    @param sercomBase the 'raw' Sercom register base address
    @param dmacID the DMAC id that matches the TX for the sercom (check DS)
    @param mosi The MOSI pin (where we send data to the neopixel)
    @param padTX the pinmux set up for SPI SERCOM pin config
    @param pinFunc The pinmux setup for which 'type' of pinmux we use
    @returns True or false on success
*/
boolean Adafruit_NeoPixel_ZeroDMA::_begin(SERCOM *sercom, Sercom *sercomBase,
                                          uint8_t dmacID, uint8_t mosi,
                                          SercomSpiTXPad padTX,
                                          EPioType pinFunc) {

  if (mosi != pin)
    return false; // Invalid pin

  Adafruit_NeoPixel::begin(); // Call base class begin() function 1st
  // TO DO: Check for successful malloc in base class here

  // DMA buffer is 3X the NeoPixel buffer size.  Each bit is expanded
  // 3:1 to allow use of SPI peripheral to generate NeoPixel-like timing
  // (0b100 for a zero bit, 0b110 for a one bit).  SPI is clocked at
  // 2.4 MHz, the 3:1 sizing then creates NeoPixel-like 800 KHz bitrate.
  // The extra 90 bytes is the low-level latch at the end of the NeoPixel
  // data stream.  When idle, SPI logic level is normally HIGH, we need
  // LOW for latch.  There is no invert option.  Various tricks like
  // switching the pin to a normal LOW output at end of data don't quite
  // work, there's still small glitches.  So, solution here is to keep
  // the SPI DMA transfer in an endless loop...it actually issues the
  // NeoPixel data over and over again forever (this doesn't cost us
  // anything, since it's 100% DMA, no CPU use)...and those 90 zero
  // bytes at the end provide the 300 microsecond EOD latch.  Hack!

  uint8_t bytesPerPixel = (wOffset == rOffset) ? 3 : 4;
  uint32_t bytesTotal = (numLEDs * bytesPerPixel * 8 * 3 + 7) / 8 + 90;
  if ((dmaBuf = (uint8_t *)malloc(bytesTotal))) {
    spi = NULL; // No SPIClass assigned yet,
                // check MOSI pin against existing defined SPI SERCOMs...
#if SPI_INTERFACES_COUNT > 0
    if (pin == PIN_SPI_MOSI) { // If NeoPixel pin is main SPI MOSI...
      spi = &SPI;              // Use the existing SPIClass object
      padTX = PAD_SPI_TX;
    }
#endif
#if SPI_INTERFACES_COUNT > 1
    else if (pin == PIN_SPI1_MOSI) { // If NeoPixel pin = secondary SPI MOSI...
      spi = &SPI1;                   // Use the SPI1 SPIClass object
      padTX = PAD_SPI1_TX;
    }
#endif
#if SPI_INTERFACES_COUNT > 2
    else if (pin == PIN_SPI2_MOSI) { // Ditto, tertiary SPI
      spi = &SPI2;
      padTX = PAD_SPI2_TX;
    }
#endif
#if SPI_INTERFACES_COUNT > 3
    else if (pin == PIN_SPI3_MOSI) {
      spi = &SPI3;
      padTX = PAD_SPI3_TX;
    }
#endif
#if SPI_INTERFACES_COUNT > 4
    else if (pin == PIN_SPI4_MOSI) {
      spi = &SPI4;
      padTX = PAD_SPI4_TX;
    }
#endif
#if SPI_INTERFACES_COUNT > 5
    else if (pin == PIN_SPI5_MOSI) {
      spi = &SPI5;
      padTX = PAD_SPI5_TX;
    }
#endif
    // If NeoPixel pin is not an existing SPI SERCOM, allocate a new one.
    if (spi == NULL) {
      // DIRTY POOL! The SPIClass constructor expects MISO, SCK and MOSI
      // pins, in that order. Our library only intends to ever use the MOSI
      // output, the others are never even set to SERCOM periph functions.
      // We just give the SPI constructor THE SAME PIN NUMBER for all three.
      // The SPI lib never checks if they're distinct and valid for each of
      // the three. It does set pinPeripheral for each (or in this case,
      // the same for the MOSI pin three times)...but no matter, we set our
      // own pinPeripheral below. The SPI RX PAD also doesn't matter...we
      // always claim it's PAD 1 here, because (by hardware design) the TX
      // pad will always be 0, 2 or 3...this might collide with the SCK PAD
      // value, but we don't care, neither SCK nor MISO is actually used.
      // (This is tested across many SAMD devices and works, but it's
      // conceivable that this could fail spectacularly on some unforseen
      // future device, if the SERCOM pad assignment becomes hardwarily
      // strict.)
      spi = new SPIClass(sercom, mosi, mosi, mosi, padTX, SERCOM_RX_PAD_1);
    }
    if ((spi)) {
      spi->begin();
      pinPeripheral(mosi, pinFunc);
      dma.setTrigger(dmacID);
      dma.setAction(DMA_TRIGGER_ACTON_BEAT);
      if (DMA_STATUS_OK == dma.allocate()) {
        if (dma.addDescriptor(dmaBuf, // move data from here
                              (void *)(&sercomBase->SPI.DATA.reg), // to here
                              bytesTotal,         // this many...
                              DMA_BEAT_SIZE_BYTE, // bytes/hword/words
                              true,               // increment source addr?
                              false)) {           // increment dest addr?
          dma.loop(true); // DMA transaction loops forever! Latch is built in.
          memset(dmaBuf, 0, bytesTotal); // IMPORTANT - clears latch data @ end
          // SPI transaction is started BUT NEVER ENDS.  This is important.
          // 800 khz * 3 = 2.4MHz
          spi->beginTransaction(SPISettings(2400000, MSBFIRST, SPI_MODE0));
          if (DMA_STATUS_OK == dma.startJob())
            return true; // SUCCESS
          // Else various errors, clean up partially-initialized stuff:
          spi->endTransaction();
        }
        dma.free();
      }
      // Delete SPIClass object, UNLESS it's an existing (Arduino-defined) one
#if SPI_INTERFACES_COUNT > 0
      if (spi == &SPI) {
        spi = NULL;
      }
#endif
#if SPI_INTERFACES_COUNT > 1
      else if (spi == &SPI1) {
        spi = NULL;
      }
#endif
#if SPI_INTERFACES_COUNT > 2
      else if (spi == &SPI2) {
        spi = NULL;
      }
#endif
#if SPI_INTERFACES_COUNT > 3
      else if (spi == &SPI3) {
        spi = NULL;
      }
#endif
#if SPI_INTERFACES_COUNT > 4
      else if (spi == &SPI4) {
        spi = NULL;
      }
#endif
#if SPI_INTERFACES_COUNT > 5
      else if (spi == &SPI5) {
        spi = NULL;
      }
#endif

#ifdef SPI
      if (spi != NULL) {
        delete spi;
        spi = NULL;
      }
#endif
    }
    free(dmaBuf);
    dmaBuf = NULL;
  }
  return false;
}

#ifdef __SAMD51__
// See notes below about M4 tomfoolery
#define EXTRASTARTBYTES 24 // Empty bytes issued until DMA timing solidifies
#define LATCHTIME 300      // Time, in microseconds, for end-of-data latch

static volatile uint32_t lastBitTime; // micros() when last bit issued

// Called at end of DMA transfer. Notes
// start-of-NeoPixel-latch time.
static void dmaCallback(Adafruit_ZeroDMA *dma) { lastBitTime = micros(); }
#endif

/** @brief Initialize SPI sercom and DMA
    @returns True
 */
boolean Adafruit_NeoPixel_ZeroDMA::begin(void) {

  uint8_t i;
  for (i = 0; (i < N_SERCOMS) && (sercomTable[i].mosi != pin); i++)
    ;
  if (i >= N_SERCOMS) {
#ifndef __SAMD51__
    return false; // Invalid pin
#else
    // Super-hacky thing specifically for the Trellis M4 lets us DMA to a
    // non-SERCOM pin.  Please don't rely on this as a general approach,
    // it's not RAM-efficient and is only practical here because the matrix
    // size on that board is 32 pixels, not too bad (eats ~3K).

    // TO DO: Check for successful malloc in base class here
    Adafruit_NeoPixel::begin(); // Call base class begin() function 1st
    uint8_t bytesPerPixel = (wOffset == rOffset) ? 3 : 4;
    uint32_t bytesTotal = (numLEDs * bytesPerPixel * 32 + EXTRASTARTBYTES);
    if ((dmaBuf = (uint8_t *)malloc(bytesTotal))) {
      int i;

      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);

      dma.setTrigger(TCC0_DMAC_ID_OVF);
      dma.setAction(DMA_TRIGGER_ACTON_BEAT);

      EPortType port = g_APinDescription[pin].ulPort;
      uint8_t bit = g_APinDescription[pin].ulPin; // 0-31
      uint8_t byteOffset = bit / 8;               // 0-3
      volatile uint8_t *dst =
          (volatile uint8_t *)&(PORT->Group[port].OUTTGL.reg) + byteOffset;
      toggleMask = digitalPinToBitMask(pin) >> (byteOffset * 8);

      dma.allocate();
      dma.setPriority(
          DMA_PRIORITY_3);      // highest priority since latency is critical
      dma.addDescriptor(dmaBuf, // source
                        (void *)dst,        // destination
                        bytesTotal,         // count
                        DMA_BEAT_SIZE_BYTE, // size per
                        true,               // increment source
                        false);             // don't increment destination

      dma.setCallback(dmaCallback);

      // Set up generic clock gen 5 as source for TCC0
      // Datasheet recommends setting GENCTRL register in a single write,
      // so a temp value is used here to more easily construct a value.
      GCLK_GENCTRL_Type genctrl;
      genctrl.bit.SRC = GCLK_GENCTRL_SRC_DFLL_Val; // 48 MHz source
      genctrl.bit.GENEN = 1;                       // Enable
      genctrl.bit.OE = 1;
      genctrl.bit.DIVSEL = 0; // Do not divide clock source
      genctrl.bit.DIV = 0;
      GCLK->GENCTRL[5].reg = genctrl.reg;
      while (GCLK->SYNCBUSY.bit.GENCTRL1 == 1)
        ;

      GCLK->PCHCTRL[TCC0_GCLK_ID].bit.CHEN = 0;
      while (GCLK->PCHCTRL[TCC0_GCLK_ID].bit.CHEN)
        ; // Wait for disable
      GCLK_PCHCTRL_Type pchctrl;
      pchctrl.bit.GEN = GCLK_PCHCTRL_GEN_GCLK5_Val;
      pchctrl.bit.CHEN = 1;
      GCLK->PCHCTRL[TCC0_GCLK_ID].reg = pchctrl.reg;
      while (!GCLK->PCHCTRL[TCC0_GCLK_ID].bit.CHEN)
        ; // Wait for enable

      // Disable TCC before configuring it
      TCC0->CTRLA.bit.ENABLE = 0;
      while (TCC0->SYNCBUSY.bit.ENABLE)
        ;

      TCC0->CTRLA.bit.PRESCALER = TCC_CTRLA_PRESCALER_DIV1_Val; // 1:1 Prescale

      TCC0->WAVE.bit.WAVEGEN = TCC_WAVE_WAVEGEN_NPWM_Val; // Normal PWM mode
      while (TCC0->SYNCBUSY.bit.WAVE)
        ;

      TCC0->CC[0].reg = 0; // No PWM out
      while (TCC0->SYNCBUSY.bit.CC0)
        ;

      // 3.2 MHz clock: 4 DMA xfers per NeoPixel bit = 800 KHz
      TCC0->PER.reg = ((48000000 + 1600000) / 3200000) - 1;
      while (TCC0->SYNCBUSY.bit.PER)
        ;

      TCC0->CTRLA.bit.ENABLE = 1;
      while (TCC0->SYNCBUSY.bit.ENABLE)
        ;

      memset(dmaBuf, 0, EXTRASTARTBYTES); // Initialize buf start with zeros

      return true;
    }
#endif
  }
#ifdef __SAMD51__
  toggleMask = 0; // Using library's normal SERCOM DMA technique
#endif
  return _begin(sercomTable[i].sercom, sercomTable[i].sercomBase,
                sercomTable[i].dmacID, sercomTable[i].mosi,
                sercomTable[i].padTX, sercomTable[i].pinFunc);
}

/** @brief Convert the NeoPixel buffer to larger DMA buffer and start xfer
 */
void Adafruit_NeoPixel_ZeroDMA::show(void) {
#ifdef __SAMD51__
  if (!toggleMask) { // Using normal SERCOM DMA technique?
#endif

    // Expand 8 bits 'abcdefgh' to 24 bits '1a01b01c01d01e01f01g01h0'
#ifdef _BITTABLE_H_
    // If bittable.h is included, 3:1 bit expansion is handled using a table
    // lookup -- each byte of input (from NeoPixel buffer) is replaced with
    // three bytes output (from table to DMA buffer).  This is about twice
    // as quick as math below but the table requires about 1KB of code space.
    uint8_t *in = pixels, *out = dmaBuf;
    uint32_t expanded;
    for (uint16_t p = numBytes; p--;) {
      expanded = bitExpand[(*in++ * brightness) >> 8];
      *out++ = expanded >> 16; // Shifting 32-bit table entry is
      *out++ = expanded >> 8;  // about 11% faster than copying
      *out++ = expanded;       // three values from a uint8_t table.
    }
#else
  // If bittable.h is NOT included, 3:1 bit expansion is done on the fly.
  // More complex, but smaller executable.
  uint8_t *in = pixels, *out = dmaBuf, i, abef, cdgh;
  uint32_t expanded;
  for (uint16_t p = numBytes; p--;) {
    cdgh = (*in++ * brightness) >> 8;
    abef = cdgh & 0b11001100; // ab00ef00
    cdgh &= 0b00110011;       // 00cd00gh
    expanded = ((abef * 0b1010000010100000) & 0b010010000000010010000000) |
               ((cdgh * 0b0000101000001010) & 0b000000010010000000010010) |
               0b100100100100100100100100;
    *out++ = expanded >> 16;
    *out++ = expanded >> 8;
    *out++ = expanded;
  }
#endif // !_BITTABLE_H_

#ifdef __SAMD51__
  } else { // NOT using SERCOM DMA technique, expansion is different...
    uint8_t *src = pixels; // Pixel buffer base address from NeoPixel lib
    uint8_t *dst = dmaBuf + EXTRASTARTBYTES;
    uint32_t count = numLEDs * ((wOffset == rOffset) ? 3 : 4); // Bytes/pixel
    while (dma.isActive())
      ; // Wait for DMA callback, so pixel data isn't corrupted
    while (count--) {
      uint8_t byte = (*src++ * brightness) >> 8;
      for (uint8_t bit = 0x80; bit; bit >>= 1) {
        *dst++ = toggleMask; // Initial toggle high
        if (byte & bit) {
          *dst++ = 0;          // Hold high at 1/4
          *dst++ = 0;          // Hold high at 2/4
          *dst++ = toggleMask; // Toggle low at 3/4
        } else {
          *dst++ = toggleMask; // Toggle low at 1/4
          *dst++ = 0;          // Hold low at 2/4
          *dst++ = 0;          // Hold low at 3/4
        }
      }
    }

    dma.startJob();
    // Wait for latch, factor out EXTRASTARTBYTES transmission time too!
    while ((micros() - lastBitTime) <= (LATCHTIME - (EXTRASTARTBYTES * 5 / 4)))
      ;
    dma.trigger();
  }
#endif
}

/** @brief
    Brightness is stored differently here than in normal NeoPixel library.
    In either case it's *specified* the same: 0 (off) to 255 (brightest).
    Classic NeoPixel rearranges this internally so 0 is max, 1 is off and
    255 is just below max...it's a decision based on how fixed-point math
    is handled in that code.  Here it's stored internally as 1 (off) to
    256 (brightest), requiring a 16-bit value.
    @param b 0 - 255 brightness value
*/
void Adafruit_NeoPixel_ZeroDMA::setBrightness(uint8_t b) {
  brightness = (uint16_t)b + 1; // 0-255 in, 1-256 out
}

/** @brief The brightness, back adjusted to 0-255 standard expectation
    @returns 0 for off, 255 for max brightness */
uint8_t Adafruit_NeoPixel_ZeroDMA::getBrightness(void) const {
  return brightness - 1; // 1-256 in, 0-255 out
}
