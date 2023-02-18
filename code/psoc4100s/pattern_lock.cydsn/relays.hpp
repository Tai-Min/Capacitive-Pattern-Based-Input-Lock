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
enum class Relays {
    RELAY_1 = 0,
    RELAY_2,
    RELAY_3,
    RELAY_4,
    NONE
};

bool matchPatternAndOpen(const pattern::PatternBuffer buf);
void closeAll();
};