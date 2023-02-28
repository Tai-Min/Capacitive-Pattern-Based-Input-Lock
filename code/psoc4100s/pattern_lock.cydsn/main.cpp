#ifdef __cplusplus
extern "C" {
#endif
#include "project.h"
#ifdef __cplusplus
}
#endif

#include "wdt.hpp"
#include "capsense.hpp"
#include "timing.hpp"
#include "pattern.hpp"
#include "relays.hpp"
#include "helpers.hpp"
#include "serial.hpp"

//#define SKIP_ADD_PATTERN
//#define PRINT_RAW_BUFFER
//#define PRINT_PATTERN
//#define PRINT_PATTERN_NEWLINE

enum class AppState {
    STANDBY,
    NORMAL,
    PATTERN_WIP
};

void registerCallbacks();
void registerTimers();
void hardwareInit();
void readSensors(const uint8_t *selectedWidgets, uint8_t numSelectedWidgets);

void handleProximityStateChange(capsense::ProximityState state);
void handleTouchpadStateChange(capsense::TouchpadState state);
void handleAcceptBtnStateChange(capsense::ButtonState state);
void handleCancelBtnStatechange(capsense::ButtonState state);

void handleProximityActivityTimeout();
void handleTouchpadActivityTimeout();
void handleCancelBtnHoldTimeout();
void handlePatternBlinkTimeout();

void printAppState();

namespace {
    AppState appState = AppState::STANDBY;
    
    timing::Timer proximityTimer;
    timing::Timer touchpadTimer;
    timing::Timer cancelTimer;
    timing::Timer patternBlinkTimer;
    
    constexpr uint8_t numWidgets = 4;
    constexpr uint8_t widgets[numWidgets] = {CapSense_PROXIMITY_WDGT_ID, CapSense_ACCEPT_WDGT_ID, CapSense_CANCEL_WDGT_ID, CapSense_TOUCHPAD_WDGT_ID};
};

int main()
{
    CyGlobalIntEnable;

    registerCallbacks();
    registerTimers();
    hardwareInit();
    
    INFO("Application started");
    
    while (true)
    {
        serial::update();
        
        if (appState == AppState::STANDBY)
        {
            uint8_t w = CapSense_PROXIMITY_WDGT_ID;
            readSensors(&w, 1);
        }
        else
        {
            readSensors(widgets, numWidgets);
        }
        
        uint32_t sleepTime = 0;
        switch(appState)
        {
        case AppState::STANDBY:
            sleepTime = SLEEP_STANDBY_MS;
            break;
        case AppState::NORMAL:
            sleepTime = SLEEP_NORMAL_MS;
            break;
        case AppState::PATTERN_WIP:
            sleepTime = SLEEP_PATTERN_WIP_MS;
            break;
        }
        timing::doSleep(sleepTime, false);
    }
}

void registerCallbacks()
{
    capsense::registerOnProximityStateChangeCallback(handleProximityStateChange);
    capsense::registerTouchpadStateChangeCallback(handleTouchpadStateChange);
    capsense::registerAcceptBtnCallback(handleAcceptBtnStateChange);
    capsense::registerCancelBtnCallback(handleCancelBtnStatechange);
    
    proximityTimer.cb = handleProximityActivityTimeout;
    touchpadTimer.cb = handleTouchpadActivityTimeout;
    cancelTimer.cb = handleCancelBtnHoldTimeout;
    patternBlinkTimer.cb = handlePatternBlinkTimeout;
    
    serial::setRelayCodeCallback(relays::setRelayCode);
}

void registerTimers()
{
    proximityTimer.timerThreshold = TIMER_PROXIMITY_THRESHOLD_MS;
    touchpadTimer.timerThreshold = TIMER_TOUCHPAD_THRESHOLD_MS;
    cancelTimer.timerThreshold = TIMER_CANCEL_THRESHOLD_MS;
    patternBlinkTimer.timerThreshold = TIMER_PATTERN_BLINKER_THRESHOLD_MS;
    
    timing::add(proximityTimer);
    timing::add(touchpadTimer);
    timing::add(cancelTimer);
    timing::add(patternBlinkTimer);
    
    timing::restart(patternBlinkTimer);
}

void hardwareInit()
{
    watchdog::init();
    serial::init();
    capsense::start();
    relays::init();
}

void readSensors(const uint8_t *selectedWidgets, uint8_t numSelectedWidgets)
{
    for(uint8_t i = 0; i < numSelectedWidgets; i++)
    {
        capsense::selectWidget(selectedWidgets[i]);
        
        capsense::ScanState scanState;
        do
        {
            scanState = capsense::processSelectedWidget();
        } while(scanState != capsense::ScanState::PROCESSING_DONE);
    }
}

void handleProximityStateChange(capsense::ProximityState state)
{
    if(state == capsense::ProximityState::DETECTED)
    {
        if(!timing::isRunning(proximityTimer))
        {
            INFO("Presence detected near lock, speeding up app at least to NORMAL");
        }
        
        if (appState < AppState::NORMAL)
        {
            appState = AppState::NORMAL;
        }
        printAppState();
        
        appState = AppState::NORMAL;
        timing::stop(proximityTimer);
        DBG("Proximity detected!");
        
    }
    else if(state == capsense::ProximityState::LOST)
    {
        timing::restart(proximityTimer);
        DBG("Proximity lost!");
    }
}

void handleTouchpadStateChange(capsense::TouchpadState state)
{
    if(state == capsense::TouchpadState::STARTED_TOUCH)
    {
        if(!timing::isRunning(touchpadTimer))
        {
            INFO("Touch detected on touchpad, speeding up app at least to PATTERN_WIP");
        }
        
        if (appState < AppState::PATTERN_WIP)
        {
            appState = AppState::PATTERN_WIP;
        }
        printAppState();
        
        appState = AppState::PATTERN_WIP;
        timing::stop(touchpadTimer);
        DBG("Started touch!");
    }
    else if(state == capsense::TouchpadState::STOPPED_TOUCH)
    {
        if(pattern::isFull())
        {
            capsense::clearTouchpad();
        }
        timing::restart(touchpadTimer);
        DBG("Stopped touch!");
    } 
}

void handleAcceptBtnStateChange(capsense::ButtonState state)
{
    if(state == capsense::ButtonState::PRESSED)
    {
        DBG("Accept pressed!");
    }
    else if(state == capsense::ButtonState::RELEASED)
    {
        if(capsense::isTouchpadClear() || pattern::isFull())
        {
            pattern::insertSymbol(pattern::PatternShape::TERMINATOR); 
            
            if (!relays::matchPatternAndOpen(pattern::getPatternBuf()))
            {
                pattern::blinkErrSignalAndClearPattern();
                WARN("Invalid pattern inserted");
            }
            else
            {
                pattern::clear();
                INFO("Correct pattern inserted, at least one relay is active");
            }
        }
        else
        {
#ifdef PRINT_RAW_BUFFER
    for(uint8_t i = 0; i < CAPSENSE_BUFFER_SIZE; i++)
    {
        helpers::writeVal(capsense::getTouchpadBuf()[i]);
        serial::send(", ");
    }
    serial::send("\r\n");
#endif
#ifdef PRINT_PATTERN
            for(int x = 0; x < CAPSENSE_NUM_COLS; x++)
            {
                for(int y = 0; y < CAPSENSE_NUM_ROWS; y++)
                {
                    if(capsense::getPixelValue(x, y))
                    {
                        serial::send("X");
                    }
                    else
                    {
                        serial::send("O");
                    }
                }
#ifdef PRINT_PATTERN_NEWLINE
                serial::send("\r\n");
#endif
            }
            serial::send("\r\n");
#endif

            pattern::PatternShape shape = pattern::classify(capsense::getTouchpadBuf());
            //pattern::PatternShape shape = pattern::PatternShape::X;
            //pattern::PatternShape shape = pattern::classify(buf);
            switch(shape)
            {
            case pattern::PatternShape::X:
                DBG("Inserted: X");
                break;
            case pattern::PatternShape::TRIANGLE:
                DBG("Inserted: triangle");
                break;
            case pattern::PatternShape::HEART:
                DBG("Inserted: heart");
                break;
            case pattern::PatternShape::CIRCLE:
                DBG("Inserted: circle");
                break;
            case pattern::PatternShape::SQUARE:
                DBG("Inserted: square");
                break;
            case pattern::PatternShape::TERMINATOR:
                DBG("Inserted: terminator???");
                break;
            }
#ifndef SKIP_ADD_PATTERN
            pattern::insertSymbol(shape); 
#endif
            capsense::clearTouchpad();
            INFO("Shape inserted to buffer via accept button");

        }
        DBG("Accept released!");
    } 
}

void handleCancelBtnStatechange(capsense::ButtonState state)
{
    if(state == capsense::ButtonState::PRESSED)
    {
        timing::restart(cancelTimer);
        DBG("Cancel pressed!");
    }
    else if(state == capsense::ButtonState::RELEASED)
    {
        timing::stop(cancelTimer);
        
        if(capsense::isTouchpadClear())
        {
            pattern::removeLastSymbol();
            INFO("Removed symbol via cancel button");
        }
        else
        {
            capsense::clearTouchpad();
            INFO("Cleared touchpad via cancel button");
        }
        DBG("Cancel released!");
    } 
}

void handleProximityActivityTimeout()
{
    if (appState > AppState::STANDBY)
    {
        appState = AppState::STANDBY;
    }
    
    pattern::clear();
    capsense::clearTouchpad();
    DBG("Proximity timeout!");
    INFO("No presence detected for a period of time, slowing down app to STANDBY");
    printAppState();
}

void handleTouchpadActivityTimeout()
{
    if (appState > AppState::NORMAL)
    {
        appState = AppState::NORMAL;
    }
    
    DBG("Touchpad timeout!");
    INFO("Touchpad inactive for a period of time, slowing down app to NORMAL");
    capsense::clearTouchpad();
    printAppState();
}

void handleCancelBtnHoldTimeout()
{
    pattern::clear();
    relays::closeAll();
    DBG("Cancel hold timeout!");
    INFO("Pattern cleared and relays locked via cancel button hold");
}

void handlePatternBlinkTimeout()
{
    pattern::updatePatternBlinker(!capsense::isTouchpadClear() && !pattern::isFull());
    timing::restart(patternBlinkTimer);
    //DBG("Patter blink updated!");
}

void printAppState()
{
    switch(appState)
    {
    case AppState::STANDBY:
        INFO("Current application state is STANDBY");
        break;
    case AppState::NORMAL:
        INFO("Current application state is NORMAL");
        break;
    case AppState::PATTERN_WIP:
        INFO("Current application state is PATTERN_WIP");
        break;
    }
}