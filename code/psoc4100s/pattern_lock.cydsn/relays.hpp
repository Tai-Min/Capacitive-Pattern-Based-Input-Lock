#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <cstdint>
#ifdef __cplusplus
}
#endif

#include "pattern.hpp"

namespace relays {
void init();
bool setRelayCode(uint8_t relay, const pattern::PatternBuffer buf);
bool matchPatternAndOpen(const pattern::PatternBuffer buf);
void closeAll();
};