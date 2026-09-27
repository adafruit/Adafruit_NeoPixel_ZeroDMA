#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

enum EPioType { PIO_SERCOM = 2, PIO_SERCOM_ALT = 3 };
enum SercomSpiTXPad { SPI_PAD_0_SCK_1, SPI_PAD_2_SCK_3, SPI_PAD_3_SCK_1 };
enum SercomRXPad { SERCOM_RX_PAD_1 = 1 };
struct SERCOM {
  unsigned index;
};
#ifdef FAKE_NATIVE_REGISTERS
struct sercom_registers_t {
  struct {
    uint32_t SERCOM_DATA;
  } SPIM;
};
#else
struct Sercom {
  struct {
    struct {
      uint32_t reg;
    } DATA;
  } SPI;
};
using sercom_registers_t = Sercom;
#endif
extern sercom_registers_t registers[8];
#ifdef FAKE_NATIVE_REGISTERS
#define SERCOM0_REGS (&registers[0])
#define SERCOM1_REGS (&registers[1])
#define SERCOM2_REGS (&registers[2])
#define SERCOM3_REGS (&registers[3])
#define SERCOM4_REGS (&registers[4])
#define SERCOM5_REGS (&registers[5])
#define SERCOM6_REGS (&registers[6])
#define SERCOM7_REGS (&registers[7])
#endif
#define SERCOM_INST_NUM 8
#define PINS_COUNT 34
#define SPI_INTERFACES_COUNT 1
#define PIN_SPI_MOSI 9
#define PAD_SPI_TX SPI_PAD_0_SCK_1
struct PinDescription {
  uint8_t ulPort, ulPin;
};
extern PinDescription g_APinDescription[PINS_COUNT];
extern SERCOM sercom0, sercom1, sercom2, sercom3, sercom4, sercom5, sercom6, sercom7;

namespace observed {
extern bool spiBeginResult;
extern bool spiTransferFails;
extern unsigned spiBegins, spiTransactions, spiTransfers, spiEnds, pinMuxes;
extern const uint8_t *spiSource;
extern size_t spiLength, spiOffset;
extern void (*spiCallback)(void *, int);
extern void *spiUser;
extern std::vector<std::vector<uint8_t>> spiSubmitted, spiCompleted;
extern unsigned spiConstructed, spiDestroyed;
} // namespace observed

#ifndef FAKE_NATIVE_REGISTERS
#define SERCOM0 (&registers[0])

#define SERCOM1 (&registers[1])

#define SERCOM2 (&registers[2])

#define SERCOM3 (&registers[3])

#define SERCOM4 (&registers[4])

#define SERCOM5 (&registers[5])

#define SERCOM6 (&registers[6])

#define SERCOM7 (&registers[7])
#endif

namespace observed {
extern uint32_t primask;
extern void (*beforeMaskRead)();
} // namespace observed
inline uint32_t __get_PRIMASK() {
  if (!observed::primask && observed::beforeMaskRead) {
    auto interrupt = observed::beforeMaskRead;
    observed::beforeMaskRead = nullptr;
    interrupt();
  }
  return observed::primask;
}
inline void __disable_irq() { observed::primask = 1; }
inline void __set_PRIMASK(uint32_t value) { observed::primask = value; }
namespace observed {
extern unsigned transactionEnds, activeTransactions;
}
