#include "capsense.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "project.h"
#ifdef __cplusplus
}
#endif

#include "timing.hpp"

namespace capsense {
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

namespace {
ScanState doScan();
ScanState checkScanCompleted();
ScanState processScanResult();
void setProximityState(bool state);
void setTouchpadState(bool state);
void setAcceptBtnState(bool state);
void setCancelBtnState(bool state);
};

namespace {
    ScanState scanState = ScanState::NO_WIDGET_SELECTED;
    uint32_t currentWidget = CAPSENSE_BAD_WIDGET_ID;
    
    ProximityCallback proximityChangeCb = nullptr;
    TouchpadCallback touchpadChangeCb = nullptr;
    ButtonCallback acceptBtnCb = nullptr;
    ButtonCallback cancelBtnCb = nullptr;

    bool proximityState = false;
    bool touchpadState = false;
    bool acceptBtnState = false;
    bool cancelBtnState = false;
    
    bool touchpadClear = true;
    uint8_t touchpadPattern[CAPSENSE_BUFFER_SIZE] = {0};
};

void start()
{
    CapSense_Start();
    CapSense_ISR_Disable();
    
    EZI2C_Start();
    EZI2C_EzI2CSetBuffer1(sizeof(CapSense_dsRam), sizeof(CapSense_dsRam), (uint8*)&CapSense_dsRam);
}

void selectWidget(uint32_t widget)
{
    scanState = ScanState::SCAN_READY;
    currentWidget = widget;
}

ScanState processSelectedWidget()
{
    if(currentWidget >= CAPSENSE_BAD_WIDGET_ID)
        return ScanState::NO_WIDGET_SELECTED;
    
    switch(scanState)
    {
        case ScanState::SCAN_READY:
            scanState = doScan();
            break;
        case ScanState::SCAN_WIP:
            scanState = checkScanCompleted();
            break;
        case ScanState::SCAN_DONE:
            scanState = processScanResult();
            break;
        case ScanState::PROCESSING_DONE:
            scanState = ScanState::SCAN_READY;
            break;
        case ScanState::NO_WIDGET_SELECTED:
            break;
    }
    return scanState;
}

bool isTouchpadClear()
{
    return touchpadClear;
}

void clearTouchpad()
{
    for(uint8_t i = 0; i < CAPSENSE_BUFFER_SIZE; i++)
    {
        touchpadPattern[i] = 0;
    }
    touchpadClear = true;
}

const TouchpadBuffer getTouchpadBuf()
{
    return touchpadPattern;
}

bool getPixelValue(uint8_t x, uint8_t y)
{
    uint16_t bitToSet = y * CAPSENSE_NUM_COLS + x;
        
    uint8_t arrIdx = bitToSet / 8;
    uint8_t shift = bitToSet % 8;
        
    return touchpadPattern[arrIdx] & (1 << shift);
}

void registerOnProximityStateChangeCallback(ProximityCallback cb)
{
    proximityChangeCb = cb;
}

void registerTouchpadStateChangeCallback(TouchpadCallback cb)
{
    touchpadChangeCb = cb;
}

void registerAcceptBtnCallback(ButtonCallback cb)
{
    acceptBtnCb = cb;
}

void registerCancelBtnCallback(ButtonCallback cb)
{
    cancelBtnCb = cb;
}

namespace {
ScanState doScan()
{
    ScanState state;
    
    if(CapSense_NOT_BUSY != CapSense_IsBusy())
    {
        //timing::doSleep(1, false);
        state = ScanState::SCAN_READY;
    }
    else
    { 
        CapSense_SetupWidget(currentWidget);
        CapSense_CSDScan();
        state = ScanState::SCAN_WIP;
    }
    
    return state;
}

ScanState checkScanCompleted()
{       
    ScanState state;
    
    if(CapSense_NOT_BUSY != CapSense_IsBusy())
    {
        //timing::doSleep(1, false);
        state = ScanState::SCAN_WIP;
    }
    else
    {
        state = ScanState::SCAN_DONE;
    }
    
    return state;
}

ScanState processScanResult()
{
    CapSense_ProcessWidget(currentWidget);
    
    bool state = CapSense_IsWidgetActive(currentWidget);
    
    switch(currentWidget)
    {
        case CapSense_PROXIMITY_WDGT_ID:
            setProximityState(state);
            break;
        case CapSense_ACCEPT_WDGT_ID:
            setAcceptBtnState(state);
            break;
        case CapSense_CANCEL_WDGT_ID: 
            setCancelBtnState(state);
            break;
        case CapSense_TOUCHPAD_WDGT_ID:
            setTouchpadState(state);
            break;
        default:
            break;
    }
    
    return ScanState::PROCESSING_DONE;
}

void setProximityState(bool state)
{
    if(proximityChangeCb)
    {
        if(state && !proximityState)
        {
            proximityChangeCb(ProximityState::DETECTED);
        }
        else if(!state && proximityState)
        {
            proximityChangeCb(ProximityState::LOST);
        }
    }
    proximityState = state;
}

void setTouchpadState(bool state)
{
    if(state)
    {
        
        uint32 coords = CapSense_GetXYCoordinates(CapSense_TOUCHPAD_WDGT_ID);
        
        // Ignore last pixel for easier computation
        uint16_t x = LO16(coords);
        if(x >= 100)
        {
            x = 99;
        }
        x /= CAPSENSE_COLS_DIVIDER; // Yields value between 0 and CAPSENSE_NUM_COLS - 1
        
        uint16_t y = HI16(coords);
        y = 100 - y;
        if(y >= 100)
        {
            y = 99;
        }
        y /= CAPSENSE_ROWS_DIVIDER; // Yields value between 0 and CAPSENSE_NUM_ROWS - 1
        
        uint16_t bitToSet = y * CAPSENSE_NUM_COLS + x;
        
        uint8_t arrIdx = bitToSet / 8;
        uint8_t shift = bitToSet % 8;
        
        touchpadPattern[arrIdx] |= (1 << shift);
    }
    if(touchpadChangeCb)
    {
        if(state && !touchpadState)
        {
            touchpadChangeCb(TouchpadState::STARTED_TOUCH);
            touchpadClear = false;
        }
        else if (!state && touchpadState)
        {
            touchpadChangeCb(TouchpadState::STOPPED_TOUCH);
        }
    }
    touchpadState = state;
}

void setAcceptBtnState(bool state)
{
    if(acceptBtnCb)
    {
        if(state && !acceptBtnState)
        {
            acceptBtnCb(ButtonState::PRESSED);
        }
        else if (!state && acceptBtnState)
        {
            acceptBtnCb(ButtonState::RELEASED);
        }
    }
    acceptBtnState = state;
}

void setCancelBtnState(bool state)
{
    if(cancelBtnCb)
    {
        if (state && !cancelBtnState)
        {
            cancelBtnCb(ButtonState::PRESSED);
        }
        else if (!state && cancelBtnState)
        {
            cancelBtnCb(ButtonState::RELEASED);
        }
    }
    cancelBtnState = state;
}
};
};