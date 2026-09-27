#pragma once
#include "variant.h"
inline void pinPeripheral(uint8_t, EPioType) { ++observed::pinMuxes; }
