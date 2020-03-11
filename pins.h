#if !defined(_NEODMA_PINS_H_)
#define _NEODMA_PINS_H_

// clang-format off

/*
The tables declared here identify compatible pins and their corresponding
SERCOMs and DMA-related registers and such. SAMD21/51 can actually handle
SPI DMA on many more pins than are indicated here, but the code design
INTENTIONALLY limits it to specific pins -- one "curated" pin per SERCOM.
Not for technical reasons, but as a matter of documentation and clarity.
Although each SERCOM could work with a choice of MOSI pins, it becomes
extremely verbose to try to explain "if you choose pin X for output, then
you can't use pins Y or Z" (and then repeating this for every SERCOM,
times every supported board). It's INFINITELY SIMPLER to explain and use
if one "good pin" has been preselected per SERCOM for each board. The user
then has a list of valid pins for any given board, they can use any one
(or more, if multiple Adafruit_NeoPixel_ZeroDMA instances) in whatever
combination, no complicated if/else/else/else explanations needed.
I tried to pick pins that are nicely spaced around the board and don't
knock out other vital peripherals. SERCOM pin selection is NOT a fun
process, believe me, it's SO MUCH EASIER this way. Most programs will be
using only one output anyway, maybe a couple (if you need lots, consider
the NeoPXL8 library instead). 
*/
struct {
  SERCOM        *sercom;
  Sercom        *sercomBase;
  uint8_t        dmacID;
  uint8_t        mosi;
  SercomSpiTXPad padTX;
  EPioType       pinFunc;
} sercomTable[] = {
// sercom   base     dmacID              mosi  padTX            pinFunc

#if defined(ARDUINO_GEMMA_M0)
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,    0, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
#elif defined(ARDUINO_TRINKET_M0)
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,    4, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
#elif defined(ADAFRUIT_CIRCUITPLAYGROUND_M0)
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A2, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   A7, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
#elif defined(__SAMD51__) // Metro M4
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX, MOSI, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    6, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    3, SPI_PAD_0_SCK_1, PIO_SERCOM,
#else // Metro M0
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,   11, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    5, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   23, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   A5, SPI_PAD_0_SCK_3, PIO_SERCOM_ALT,
#endif

};

#define N_SERCOMS (sizeof sercomTable / sizeof sercomTable[0])

#endif // _NEODMA_PINS_H_


#if 0 // WIP

// NOTE TO FUTURE SELF: avoiding pins used by essential SERCOM peripherals
// (I2C, Serial) was a design decision to make documentation easier (not
// having to explain "if you have DMA NeoPixels on this pin, you can't use
// I2C devices") (exception being the SPI MOSI pin, because the older
// library handled that on Metro boards and it's not as widely used as I2C).
// HOWEVER, on many boards where there's only an "external" I2C bus (no
// onboard sensors or such sharing the bus), it mmmmight be sensible to
// allow DMA NeoPixels on either the SDA or SCL pins, since the NeoPixels
// at that point physically block I2C (ditto for the Serial1 TX/RX pins) --
// it's sort of implied that the peripheral can't be used at the same time,
// but as implemented right now, it additionally *enforces* not using DMA
// NeoPixels on those pins (at all, not just when not-using-peripheral).
// Maybe that's too strict and not necessary. Or maybe the selection of
// pins here, as-is, adequately covers most situations. Just saying there
// might be a possibility of having to revisit these tables to add 1-2 more
// pin options that overlap I2C or Serial1 on boards where those are
// physically exposed and not shared with onboard peripherals. It's no fun,
// requires using the pinfinder.py script (in extras directory) and looking
// for the right missing items to add on a per-board basis.

#if defined(ADAFRUIT_FEATHER_M0)
  // Serial1 (TX/RX) is on SERCOM0, do not use
  // SERCOM1,2 are 100% in the clear
  // I2C is on SERCOM3, do not use
  // SPI is on SERCOM4, but OK to use (as SPI MOSI)
  // Serial5 is on SERCOM5, but OK to use (Arduino core detritus)
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,   12, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    5, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX, MOSI, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    6, SPI_PAD_2_SCK_3, PIO_SERCOM,
#endif

#if defined(ADAFRUIT_FEATHER_M0_EXPRESS)
  // Serial1 (TX/RX) is on SERCOM0, do not use
  // SERCOM1,5 are 100% in the clear
  // SPI FLASH is on SERCOM2, do not use
  // I2C is on SERCOM3, do not use
  // SPI is on SERCOM4, but OK to use (as SPI MOSI)
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,   12, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX, MOSI, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    6, SPI_PAD_2_SCK_3, PIO_SERCOM,
#endif

#if defined(ADAFRUIT_FEATHER_M4_EXPRESS)
  // SERCOM0,3,4 are 100% clear to use
  // SPI is on SERCOM1, but OK to use (as SPI MOSI)
  // I2C is on SERCOM2, do not use
  // Serial1 (TX/RX) is on SERCOM5, do not use
  // Feather M4 uses QSPI flash, not on a SERCOM
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A4, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   12, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   A2, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_ITSYBITSY_M0)
  // Serial1 (TX/RX) is on SERCOM0, do not use
  // SERCOM1 is 100% OK to use!
  // I2C is on SERCOM3, do not use
  // SPI is on SERCOM4, but OK to use (as SPI MOSI)
  // SPI FLASH (SPI1) is on SERCOM5, do not use
  // Pin 5 is the magic level-shifted pin on ItsyBitsy, enable if possible!
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,   12, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    5, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX, MOSI, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS)
  // SPI is on SERCOM1, but OK to use (as SPI MOSI)
  // I2C is on SERCOM2, do not use
  // Serial1 (TX/RX) is on SERCOM3, do not use
  // ItsyBitsy M4 uses QSPI flash, not on a SERCOM
  // Pin 5 is the magic level-shifted pin on ItsyBitsy, enable if possible!
  // SERCOM0,4,5 are 100% clear to use!
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,    2, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX, MOSI, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    5, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   12, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_METRO_M0_EXPRESS)
  // Serial1 (TX/RX) is on SERCOM0, do not use
  // SERCOM1,2 are 100% in the clear
  // I2C is on SERCOM3, do not use
  // SPI is on SERCOM4, but OK to use (as SPI MOSI)
  // SPI FLASH (SPI1) is on SERCOM5, do not use
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,   12, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    5, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX, MOSI, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
#endif


#if defined(ADAFRUIT_METRO_M4_EXPRESS)
  // SERCOM0,1,4 are 100% in the clear
  // SPI is on SERCOM2, but OK to use (as SPI MOSI)
  // Serial1 (TX/RX) is on SERCOM3, do not use
  // I2C is on SERCOM5, do not use
  // Metro M4 uses QSPI flash, not on a SERCOM
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX, MOSI, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    6, SPI_PAD_3_SCK_1, PIO_SERCOM,
#endif

#if defined(ADAFRUIT_METRO_M4_AIRLIFT_LITE)
  // Serial2 (to ESP32) is on SERCOM0, do not use
  // SERCOM1,4 are 100% in the clear
  // SPI is on SERCOM2, but OK to use (as SPI MOSI)
  // Serial1 (TX/RX) is on SERCOM3, do not use
  // I2C is on SERCOM5, do not use
  // Metro M4 uses QSPI flash, not on a SERCOM
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX, MOSI, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    6, SPI_PAD_3_SCK_1, PIO_SERCOM,
#endif

#if defined(ADAFRUIT_GRAND_CENTRAL_M4)
  // SERCOM1,4,5 are 100% in the clear
  // Serial1 (TX/RX) is on SERCOM0, do not use
  // SPI1 (SD card) is on SERCOM2, do not use
  // I2C is on SERCOM3, do not use
  // I2C2 is on SERCOM6, do not use
  // SPI is on SERCOM7, but OK to use (as SPI MOSI)
  // Grand Central uses QSPI flash, not on a SERCOM
  // SERCOMs 1, 4 and 5 are mentioned in the board's variant.h file but
  // are not actually instantiated as Serial peripherals...probably a
  // carryover from an earlier board design, which had multiple TX/RX
  // selections. Consider these SERCOMs safe to use for now.
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   23, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,  A12, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom7, SERCOM7, SERCOM7_DMAC_ID_TX, MOSI, SPI_PAD_0_SCK_1, PIO_SERCOM,
#endif

#if defined(ARDUINO_SAMD_HALLOWING_M0)
  // SERCOM0,1 are 100% in the clear
  // Serial1 (TX/RX) is on SERCOM2, do not use
  // I2C is on SERCOM3, do not use
  // SPI FLASH is on SERCOM4, do not use
  // SPI (incl. screen) is on SERCOM5, but OK to use (as SPI MOSI)
  // NEOPIX jack is pin 4, SENSE is 3, backlight is 7 (avoid)
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,    4, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    6, SPI_PAD_2_SCK_3, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX, MOSI, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_HALLOWING_M4_EXPRESS)
  // SERCOM0,3 are 100% in the clear
  // TFT (SPI1) is on SERCOM1, do not use
  // I2C is on SERCOM2, do not use
  // Serial1 (TX/RX) is on SERCOM4, do not use
  // SPI is on SERCOM5, but OK to use (as SPI MOSI)
  // HalloWing M4 uses QSPI flash, not on a SERCOM
  // NEOPIX jack is pin D3 (PB02) -- this is a SERCOM5 pin and will
  // interfere with SPI (but not the TFT, on its own bus). Since this
  // is the ONLY option for DMA'ing to NEOPIX, it's allowed here, with
  // the understanding that external SPI is then unavailable.
  // Onboard NeoPixels are on D8 (PB16), also a SERCOM5 pin with the
  // same concern. THEREFORE, you get a choice: SPI interface to hardware on
  // the FeatherWing header -or- DMA out on MOSI pin -or- DMA NEOPIX jack
  // -or- DMA onboard pixels. ONLY ONE OF THESE.
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A5, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,    6, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    8, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_MONSTER_M4SK_EXPRESS)
  // SERCOM0,4 are 100% in the clear (but no external pins to SERCOM0)
  // I2C is on SERCOM1, do not use
  // Right TFT (SPI) is on SERCOM2, do not use
  // PDM mic (SPI2) is on SERCOM3, do not use
  // Left TFT (SPI1) is on SERCOM5, do not use
  // Monster M4sk uses QSPI flash, not on a SERCOM
  // 3-pin JST is pin D2 (PB08)
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    2, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_PYPORTAL)
  // SERCOM0,1 are 100% in the clear, but few pins are exposed
  // SPI (SD card) is on SERCOM2, no pins exposed, do not use
  // Serial1 (TX/RX) is on SERCOM4, used for WiFi, do not use
  // I2C on SERCOM5, do not use
  // NEOPIX connector is pin 4 (PA05) -- the only SERCOM/PAD combo there
  // is SERCOM0/PAD[1], but PAD[1] can't be used for MOSI, so DMA is not
  // available on this pin.
  // SENSE connector is pin 3 (PA04) -- this DOES allow DMA, and is one of
  // the few exposed pins, so let's enable using that even though it's not
  // the canonical NeoPixel connector.
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,    3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_PYPORTAL_M4_TITANO)
  // Same rules and oddness as PYPORTAL_M4 above
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,    3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_CRICKIT_M0)
  // I2C on SERCOM1, do not use
  // Serial1 (TX/RX) is on SERCOM5, do not use
  // A11 = Captouch 3, A8 = Signal 8, 11 = NeoPixel, D8 = Servo 3
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,  A11, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   A8, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   11, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
#endif




// WIPs and notes below this line...as items are resolved, move them up
// -------------------------------------------------------------------------


#if defined(ADAFRUIT_CIRCUITPLAYGROUND_M0)
  // SERCOM0 is allowed, but SD card Gizmo isn't compatible
  // "Internal" I2C (for LIS3DH) is on SERCOM1, do not use
  // SERCOM2 would be in the clear, but all the MOSI-capable pins are
  // assigned to other tasks: 5 = right, 7 = switch, 26 = IR in
  // SPI FLASH (SPI1) is on SERCOM3, do not use
  // Serial1 (TX/RX) is on SERCOM4, do not use
  // I2C is on SERCOM5, do not use
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,    9, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   10, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A2, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A3, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A8, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,  A10, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   34, SPI_PAD_2_SCK_3, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   35, SPI_PAD_0_SCK_1, PIO_SERCOM,

  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    5, SPI_PAD_2_SCK_3, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    7, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   A8, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   26, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   34, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   35, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_GEMMA_M0)
  // Serial1 (TX/RX) is on SERCOM0, do not use
  // SPI is on SERCOM0 also, output-only, can't have both. Mosi is pin 0
// Why is the script not picking that up?
// It's PORTA4, which should function as SERCOM0/PAD[0]
// Pin A0 is 8ul
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,  SDA, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A2, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,

  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,

  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX, MISO, SPI_PAD_2_SCK_3, PIO_SERCOM,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,    7, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX, MISO, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    7, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_TRINKET_M0)
  // Serial1 (TX/RX) is on SERCOM0, do not use
  // SPI also SERCOM0
  // I2C on SERCOM2
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,  SDA, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,  SCK, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX, MOSI, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,    5, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   12, SPI_PAD_2_SCK_3, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   13, SPI_PAD_2_SCK_3, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A2, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A3, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A4, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,

  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    7, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,

  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,  SDA, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    5, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   12, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   13, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   A2, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,

  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   10, SPI_PAD_2_SCK_3, PIO_SERCOM,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   10, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
#endif


#if defined(ADAFRUIT_PYBADGE_M4_EXPRESS)
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A4, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   37, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   40, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    5, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    9, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,  SDA, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   38, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   40, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,

  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,    9, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   12, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,  SCK, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   31, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   A2, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,  SCL, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   36, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,MOSI1, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,MISO1, SPI_PAD_0_SCK_1, PIO_SERCOM,

  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    1, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   13, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   A6, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   A9, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   31, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_PYBADGE_AIRLIFT_M4)
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A4, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   37, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   40, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    5, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    9, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,  SDA, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   38, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   40, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,

  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,    9, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   12, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,  SCK, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   31, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   A2, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,  SCL, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   36, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,MOSI1, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,MISO1, SPI_PAD_0_SCK_1, PIO_SERCOM,

  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    1, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   13, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   A6, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   A9, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   31, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
#endif


#if defined(ADAFRUIT_PYGAMER_M4_EXPRESS)
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A4, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   37, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   40, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    5, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    9, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,  SDA, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   38, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   40, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,

  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,    9, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   12, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,  SCK, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   33, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   A2, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,  SCL, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   36, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,MOSI1, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    1, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   13, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   A6, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   A9, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   33, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
#endif

#if defined(ADAFRUIT_PYGAMER_ADVANCE_M4_EXPRESS)
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   A4, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   37, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom0, SERCOM0, SERCOM0_DMAC_ID_TX,   40, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    5, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX,    9, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom1, SERCOM1, SERCOM1_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,  SDA, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   38, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom2, SERCOM2, SERCOM2_DMAC_ID_TX,   40, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,

  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,    9, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   12, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,  SCK, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom3, SERCOM3, SERCOM3_DMAC_ID_TX,   33, SPI_PAD_3_SCK_1, PIO_SERCOM,

  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,    8, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   A2, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,  SCL, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,   36, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom4, SERCOM4, SERCOM4_DMAC_ID_TX,MOSI1, SPI_PAD_0_SCK_1, PIO_SERCOM,

  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    1, SPI_PAD_0_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,    3, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   11, SPI_PAD_3_SCK_1, PIO_SERCOM,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   13, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   A6, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   A9, SPI_PAD_0_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX, MOSI, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
  &sercom5, SERCOM5, SERCOM5_DMAC_ID_TX,   33, SPI_PAD_3_SCK_1, PIO_SERCOM_ALT,
#endif



#endif // 0


// Need something here to catch Arduino Zero (there's no simple define for
// it), or maybe include that check with the Metro M0 section (same pinout)

