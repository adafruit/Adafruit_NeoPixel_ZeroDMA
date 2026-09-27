#pragma once
#include "variant.h"
#include <vector>
constexpr uint8_t MSBFIRST = 1, SPI_MODE0 = 0;
struct SPISettings {
  SPISettings(uint32_t, uint8_t, uint8_t) {}
};
class SPIClass {
public:
  SPIClass(SERCOM *, uint8_t, uint8_t, uint8_t, SercomSpiTXPad, SercomRXPad) {
    ++observed::spiConstructed;
  }
  ~SPIClass() { ++observed::spiDestroyed; }
  bool begin() {
    ++observed::spiBegins;
    return observed::spiBeginResult;
  }
  void beginTransaction(SPISettings) {
    ++observed::spiTransactions;
    ++observed::activeTransactions;
  }
  void endTransaction() {
    ++observed::transactionEnds;
    --observed::activeTransactions;
  }
  void transfer(const void *source, void *, size_t length, bool, void (*callback)(void *, int),
                void *user) {
    ++observed::spiTransfers;
    if (observed::spiTransferFails) {
      callback(user, -1);
      return;
    }
    observed::spiSource = static_cast<const uint8_t *>(source);
    observed::spiLength = length;
    observed::spiOffset = 0;
    observed::spiCallback = callback;
    observed::spiUser = user;
    observed::spiSubmitted.emplace_back(observed::spiSource, observed::spiSource + length);
    observed::spiCompleted.emplace_back();
  }
  void end() {
    ++observed::spiEnds;
    observed::spiSource = nullptr;
    observed::spiCallback = nullptr;
    observed::spiUser = nullptr;
  }
};
extern SPIClass SPI;
