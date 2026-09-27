#pragma once

#include "variant.h"
#include <stdint.h>

/**
 * @brief One SPI-capable SERCOM route for a specific MCU port pin.
 */
struct _SercomPinLookup {
  uint8_t port;      ///< EPortType index: 0=PORTA, 1=PORTB, 2=PORTC, 3=PORTD.
  uint8_t portPin;   ///< Pin index within the selected port (0-31).
  uint8_t sercomNum; ///< SERCOM instance index.
  uint8_t pad;       ///< SERCOM pad number (0-3).
  uint8_t mux;       ///< Pin mux selector: 2=PIO_SERCOM (MUX C), 3=PIO_SERCOM_ALT
                     ///< (MUX D).
};

static const _SercomPinLookup _sercomPinTable[] = {
#define SERCOM_PIN(port, portPin, sercomNum, pad, mux) {port, portPin, sercomNum, pad, mux},
#include "pins.inc"
#undef SERCOM_PIN
};

// SERCOM class instances are defined in each board's variant.cpp.
extern SERCOM sercom0, sercom1, sercom2, sercom3;
#if SERCOM_INST_NUM > 4
extern SERCOM sercom4, sercom5;
#endif
#if SERCOM_INST_NUM > 6
extern SERCOM sercom6, sercom7;
#endif

static SERCOM *const _sercoms[] = {
    &sercom0, &sercom1, &sercom2, &sercom3,
#if SERCOM_INST_NUM > 4
    &sercom4, &sercom5,
#endif
#if SERCOM_INST_NUM > 6
    &sercom6, &sercom7,
#endif
};
