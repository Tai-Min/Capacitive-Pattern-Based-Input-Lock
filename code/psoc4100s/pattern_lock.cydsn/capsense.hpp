#pragma once

#ifdef __cplusplus
extern "C" {
#include <cstdint>
}
#endif

#include "conf.hpp"

namespace capsense {
enum class ScanState
{
    SCAN_READY,
    SCAN_WIP,
    SCAN_DONE,
    PROCESSING_DONE,
    NO_WIDGET_SELECTED
};

enum class ProximityState {
    DETECTED,
    LOST
};

enum class TouchpadState {
    STARTED_TOUCH,
    STOPPED_TOUCH
};

enum class ButtonState 
{
    PRESSED,
    RELEASED
};

using TouchpadBuffer = uint8_t(&)[CAPSENSE_BUFFER_SIZE];
using ProximityCallback = void(*)(ProximityState state);
using TouchpadCallback = void(*)(TouchpadState state);
using ButtonCallback = void(*)(ButtonState state);

void start();
void selectWidget(uint32_t widget);
ScanState processSelectedWidget();
bool isTouchpadClear();
void clearTouchpad();
const TouchpadBuffer getTouchpadBuf();
bool getPixelValue(uint8_t x, uint8_t y);
void registerOnProximityStateChangeCallback(ProximityCallback cb);
void registerTouchpadStateChangeCallback(TouchpadCallback cb);
void registerAcceptBtnCallback(ButtonCallback cb);
void registerCancelBtnCallback(ButtonCallback cb);
};