#include <iostream>
#include <string>
#include <unordered_set>
#include "Adafruit_NeoPixel.h"
#include "SPI.h"
namespace observed {
uint32_t primask = 0;
void (*beforeMaskRead)() = nullptr;
bool spiBeginResult = true;
bool spiTransferFails = false;
unsigned spiBegins = 0, spiTransactions = 0, spiTransfers = 0, spiEnds = 0, pinMuxes = 0;
const uint8_t *spiSource = nullptr;
size_t spiLength = 0, spiOffset = 0;
void (*spiCallback)(void *, int) = nullptr;
void *spiUser = nullptr;
std::vector<std::vector<uint8_t>> spiSubmitted, spiCompleted;
unsigned spiConstructed = 0, spiDestroyed = 0, transactionEnds = 0, activeTransactions = 0;
std::unordered_set<void *> buffers;
unsigned mallocCalls = 0, failMallocCall = 0, freedActiveSources = 0;
} // namespace observed
void *trackedMalloc(size_t n) {
  if (++observed::mallocCalls == observed::failMallocCall)
    return nullptr;
  void *p = std::malloc(n);
  if (p)
    observed::buffers.insert(p);
  return p;
}
void trackedFree(void *p) {
  if (p != nullptr && p == observed::spiSource)
    ++observed::freedActiveSources;
  observed::buffers.erase(p);
  std::free(p);
}
#define malloc trackedMalloc
#define free(...) FREE_DISPATCH(__VA_ARGS__)
#define FREE_DISPATCH(...) trackedFree(__VA_ARGS__)
#include "../../Adafruit_NeoPixel_SERCOM.cpp"
#undef malloc
#undef free
SERCOM sercom0{0}, sercom1{1}, sercom2{2}, sercom3{3}, sercom4{4}, sercom5{5}, sercom6{6},
    sercom7{7};
sercom_registers_t registers[8] = {};
PinDescription g_APinDescription[PINS_COUNT] = {};
SPIClass SPI(&sercom0, 9, 9, 9, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_1);
class InspectableStrip : public Adafruit_NeoPixel_SERCOM {
public:
  using Adafruit_NeoPixel_SERCOM::Adafruit_NeoPixel_SERCOM;
  void frame(uint8_t value) {
    memset(pixels, value, numBytes);
    show();
  }
};
unsigned failures = 0;
void check(bool condition, const char *message) {
  if (!condition) {
    ++failures;
    std::cout << "FAIL " << message << '\n';
  }
}
bool initialize(Adafruit_NeoPixel_SERCOM &strip, uint8_t pin = 33) {
  return strip.begin(&sercom1, pin, SPI_PAD_0_SCK_1, PIO_SERCOM);
}
void completeSpi() {
  if (!observed::spiSource)
    return;
  observed::spiCompleted.back().insert(observed::spiCompleted.back().end(),
                                       observed::spiSource + observed::spiOffset,
                                       observed::spiSource + observed::spiLength);
  auto callback = observed::spiCallback;
  auto user = observed::spiUser;
  observed::spiSource = nullptr;
  observed::spiCallback = nullptr;
  observed::spiUser = nullptr;
  callback(user, 0);
}
void advanceSpi(size_t count) {
  if (!observed::spiSource)
    return;
  const size_t remain = observed::spiLength - observed::spiOffset;
  if (count > remain)
    count = remain;
  observed::spiCompleted.back().insert(observed::spiCompleted.back().end(),
                                       observed::spiSource + observed::spiOffset,
                                       observed::spiSource + observed::spiOffset + count);
  observed::spiOffset += count;
  if (observed::spiOffset == observed::spiLength) {
    auto callback = observed::spiCallback;
    auto user = observed::spiUser;
    observed::spiSource = nullptr;
    observed::spiCallback = nullptr;
    observed::spiUser = nullptr;
    callback(user, 0);
  }
}
std::vector<uint8_t> expectedFrame(uint8_t first, uint8_t second, uint8_t third) {
  std::vector<uint8_t> frame(99, 0);
  for (size_t i = 0; i < 9; i += 3) {
    frame[i] = first;
    frame[i + 1] = second;
    frame[i + 2] = third;
  }
  return frame;
}
void released(unsigned constructed, unsigned destroyed) {
  check(observed::activeTransactions == 0, "SPI transactions balanced");
  check(observed::spiConstructed - constructed == observed::spiDestroyed - destroyed,
        "owned SPI deleted exactly once");
  check(observed::buffers.empty(), "frame buffers released");
  check(observed::freedActiveSources == 0, "SPI releases its source pointer before buffer free");
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  std::string name = argv[1];
  auto constructed = observed::spiConstructed, destroyed = observed::spiDestroyed;
  if (name == "spi_pipeline_required") {
    InspectableStrip strip(1, 33);
    check(initialize(strip), "begin succeeds");
    strip.show();
    check(observed::spiTransfers == 1, "show queues the frame through SPI");
    completeSpi();
  } else if (name == "pending_frames" || name == "completion_during_publish" ||
             name == "in_flight_buffer") {
    InspectableStrip strip(1, 33);
    check(initialize(strip), "begin succeeds");
    strip.frame(0);
    if (name == "in_flight_buffer")
      advanceSpi(3);
    strip.frame(0x80);
    if (name == "completion_during_publish")
      observed::beforeMaskRead = [] { completeSpi(); };
    strip.frame(0xff);
    observed::beforeMaskRead = nullptr;
    if (name == "in_flight_buffer")
      check(observed::spiCompleted.front().size() == 3,
            "active SPI source is retained during staging encode");
    completeSpi();
    completeSpi();
    check(observed::spiSubmitted.size() == 2, "only active and newest pending frames queue");
    check(observed::spiSubmitted ==
              std::vector<std::vector<uint8_t>>{expectedFrame(0x92, 0x49, 0x24),
                                                expectedFrame(0xdb, 0x6d, 0xb6)},
          "SPI submits exact A then C payloads and latch bytes");
    check(observed::spiSubmitted == observed::spiCompleted, "SPI consumes retained source bytes");
  } else if (name == "transfer_failure") {
    InspectableStrip strip(1, 33);
    check(initialize(strip), "begin succeeds");
    observed::spiTransferFails = true;
    strip.show();
    observed::spiTransferFails = false;
    strip.show();
    check(observed::spiSource != nullptr, "synchronous SPI failure clears active state");
    completeSpi();
  } else if (name == "destroy_active") {
    {
      InspectableStrip strip(1, 33);
      check(initialize(strip), "begin succeeds");
      strip.show();
    }
    check(observed::spiSource == nullptr, "destruction cancels SPI before buffers release");
  } else if (name == "never_begun") {
    Adafruit_NeoPixel_SERCOM strip(1, 33);
  } else if (name == "invalid_pin") {
    Adafruit_NeoPixel_SERCOM strip(1, 33);
    check(!initialize(strip, 32), "invalid pin rejected");
  } else if (name == "spi_begin_failure") {
    observed::spiBeginResult = false;
    Adafruit_NeoPixel_SERCOM strip(1, 33);
    check(!initialize(strip), "SPI begin failure propagates");
    check(observed::buffers.empty(), "failed SPI begin releases frame buffers");
    observed::spiBeginResult = true;
    check(initialize(strip), "SPI begin retry succeeds");
    strip.show();
    completeSpi();
  } else if (name == "buffer_failure_1" || name == "buffer_failure_2") {
    observed::failMallocCall = name == "buffer_failure_1" ? 1 : 2;
    Adafruit_NeoPixel_SERCOM strip(1, 33);
    check(!initialize(strip), "buffer allocation failure propagates");
    check(observed::buffers.empty(), "partial frame allocation releases buffers");
    observed::failMallocCall = 0;
    check(initialize(strip), "buffer allocation retry succeeds");
    strip.show();
    completeSpi();
  } else if (name == "borrowed_spi") {
    Adafruit_NeoPixel_SERCOM strip(1, 9);
    check(initialize(strip, 9), "borrowed SPI begins");
    strip.show();
    completeSpi();
    check(observed::spiDestroyed == destroyed, "borrowed SPI is not deleted");
  } else if (name == "begin_twice") {
    InspectableStrip strip(1, 33);
    check(initialize(strip), "first begin succeeds");
    check(initialize(strip), "second begin succeeds");
    strip.show();
    check(observed::spiTransfers == 1, "second begin preserves one frame allocation");
    check(observed::spiBegins == 1 && observed::mallocCalls == 2,
          "second begin does not reinitialize SPI or buffers");
    completeSpi();
  } else if (name == "repeated_lifetimes") {
    for (unsigned i = 0; i < 8; ++i) {
      InspectableStrip strip(1, 33);
      check(initialize(strip), "lifetime begins");
      strip.show();
    }
    check(observed::spiSource == nullptr, "each lifetime cancels its active SPI source");
  } else if (name == "irq_restore") {
    observed::primask = 1;
    InspectableStrip strip(1, 33);
    check(initialize(strip), "begin succeeds");
    strip.show();
    completeSpi();
  } else
    return 2;
  check(observed::primask == (name == "irq_restore" ? 1u : 0u), "interrupt mask restored");
  released(constructed, destroyed);
  std::cout << name << ": " << (failures ? "FAIL" : "PASS") << '\n';
  return failures ? 1 : 0;
}
