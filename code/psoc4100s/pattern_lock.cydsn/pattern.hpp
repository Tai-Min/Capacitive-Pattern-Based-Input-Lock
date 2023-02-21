#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <cstdint>
#ifdef __cplusplus
}
#endif

#include "conf.hpp"
#include "capsense.hpp"

namespace pattern {

enum class PatternShape {
    X,
    TRIANGLE,
    HEART,
    CIRCLE,
    SQUARE,
    TERMINATOR
};

using PatternBuffer = PatternShape(&)[PATTERN_MAX_LENGTH];

bool isFull();
bool insertSymbol(PatternShape symbol);
bool removeLastSymbol();
void clear();
const PatternBuffer getPatternBuf();
PatternShape classify(const capsense::TouchpadBuffer buf);
void updatePatternBlinker(bool active);
void blinkErrSignalAndClearPattern();
};