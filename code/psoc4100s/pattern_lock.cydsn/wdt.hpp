#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <cstdint>
#ifdef __cplusplus
}
#endif

namespace watchdog {
void init();
void start(uint32_t delayMs);
bool interruptOccured();
void clearInterrupt();
};