#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <cstdint>
#ifdef __cplusplus
}
#endif

namespace timing {
using TimeoutCallback = void(*)();

struct Timer {
    uint32_t timerThreshold = 0;
    bool _isRunning = false;
    uint32_t _timerVal = 0;
    TimeoutCallback cb = nullptr;
};

bool add(Timer &t);
void restart(Timer &t);
void stop(Timer &t);
bool isRunning(Timer &t);
void doSleep(uint32_t delayMs, bool deep = true);
};