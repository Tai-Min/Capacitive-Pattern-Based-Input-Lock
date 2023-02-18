#include "wdt.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include <cstdint>
#include "project.h"
#ifdef __cplusplus
}
#endif

#include "conf.hpp"

extern "C" CY_ISR(Timer_Interrupt);

namespace watchdog {
void init();
void start(uint32_t delayMs);
bool interruptOccured();
void clearInterrupt();

namespace {
uint32_t calibrateMatchValue(uint32_t delayMs);
};

namespace {
    constexpr uint32_t MILLI_SEC_TO_MICRO_SEC = 1000;
    volatile bool interruptOccuredFlag = false;
};

void init()
{
    CyIntSetVector(WDT_IRQ_NUMBER, Timer_Interrupt);
    CySysWdtUnmaskInterrupt();
    CyIntEnable(WDT_IRQ_NUMBER);
    CySysWdtDisable();
    CySysClkIloStartMeasurement();
}

void start(uint32_t delayMs)
{
    
    CySysWdtSetMatch(CySysWdtGetCount() + calibrateMatchValue(delayMs)); 
    CySysClkIloStopMeasurement();
    CySysWdtEnable();
}

bool interruptOccured()
{
    return interruptOccuredFlag;
}

void clearInterrupt()
{
    interruptOccuredFlag = false;
    CySysWdtDisable();
    CySysClkIloStartMeasurement();
}

namespace {
uint32_t calibrateMatchValue(uint32_t delayMs)
{    
    uint32_t desiredDelay = (delayMs * MILLI_SEC_TO_MICRO_SEC);  
    uint32_t tempILOCounts;
    cystatus res = CYRET_STARTED;
    while(CYRET_SUCCESS != res)
    {   
        res = CySysClkIloCompensate(desiredDelay, &tempILOCounts);
    }    
    return tempILOCounts;
}
};
};

extern "C" CY_ISR(Timer_Interrupt)
{
    CySysWdtClearInterrupt();    
    watchdog::interruptOccuredFlag = true;   
}