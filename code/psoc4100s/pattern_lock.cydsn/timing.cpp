#include "timing.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "project.h"
#ifdef __cplusplus
}
#endif

#include "conf.hpp"
#include "wdt.hpp"
#include "helpers.hpp"

namespace timing {
bool add(Timer &t);
void restart(Timer &t);
void stop(Timer &t);
bool isRunning(Timer &t);
void doSleep(uint32_t delayMs);

namespace
{
void update(uint32_t timePassedMs);
}

namespace {
    Timer *timers[TIMER_COUNT] = {nullptr};
};

bool add(Timer &t)
{
    for (uint8_t i = 0; i < TIMER_COUNT; i++)
    {
        if (timers[i] == nullptr)
        {
            timers[i] = &t;
            return true;
        }
    }
    return false;
}

void restart(Timer &t)
{
    t._timerVal = 0;
    t._isRunning = true;
}

void stop(Timer &t)
{
    t._isRunning = false;
}

bool isRunning(Timer &t)
{
    return t._isRunning;
}

void doSleep(uint32_t delayMs, bool deep)
{
    watchdog::start(delayMs);

    if(deep)
    {
        CySysPmDeepSleep();
    }
    else
    {
        CySysPmSleep();
    }
    
    watchdog::clearInterrupt();
    update(delayMs);
    
}

namespace {
void update(uint32_t timePassedMs)
{
    for (uint8_t i = 0; i < TIMER_COUNT; i++)
    {
        if (timers[i] == nullptr)
        {
            continue;
        }
        
        timers[i]->_timerVal += timePassedMs;
            
        if (timers[i]->_isRunning && timers[i]->_timerVal > timers[i]->timerThreshold)
        {
            stop(*timers[i]);
            
            if(timers[i]->cb)
            {   
                timers[i]->cb();
            }
        }
        
    }
}
}

};