#include "relays.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "project.h"
#ifdef __cplusplus
}
#endif

#include "conf.hpp"
#include "timing.hpp"
#include "helpers.hpp"

namespace relays {
void init();
bool setRelayCode(uint8_t relay, const pattern::PatternBuffer buf);
bool matchPatternAndOpen(const pattern::PatternBuffer buf);
void closeAll();

namespace {
struct relay {
    void(*write)(uint8_t);
    void(*timerCb)();
    timing::Timer timer;
    pattern::PatternShape buf[PATTERN_MAX_LENGTH];
};
    
bool comparePatterns(const pattern::PatternBuffer patternBuf, const pattern::PatternBuffer relayPatternBuf);
void openRelay(relay &r, uint32_t time);
};

void r1Off(){relay_1_Write(false);}
void r2Off(){relay_2_Write(false);}
void r3Off(){relay_3_Write(false);}
void r4Off(){relay_4_Write(false);}

namespace {
    relay relays[RELAYS_COUNT] = {
        {relay_1_Write, r1Off},
        {relay_2_Write, r2Off},
        {relay_3_Write, r3Off},
        {relay_4_Write, r4Off}
    };
};

void init()
{
    for(uint8_t i = 0; i < RELAYS_COUNT; i++)
    {
        relays[i].timer.cb = relays[i].timerCb;
        timing::add(relays[i].timer);
        
        for(uint8_t j = 0; j < PATTERN_MAX_LENGTH; j++)
        {
            relays[i].buf[j] = pattern::PatternShape::TERMINATOR;
        }
    }
}

bool setRelayCode(uint8_t relay, const pattern::PatternBuffer buf)
{
    if(relay >= RELAYS_COUNT)
    {
        return false;
    }
    
    for(uint8_t i = 0; i < PATTERN_MAX_LENGTH; i++)
    {
        relays[relay].buf[i] = buf[i];
    }
    
    return true;
}

bool matchPatternAndOpen(const pattern::PatternBuffer buf)
{
    bool res = false;
    for(uint8_t i = 0; i < RELAYS_COUNT; i++)
    {
        if (comparePatterns(buf, relays[i].buf))
        {
            res = true;
            openRelay(relays[i], RELAY_OPEN_TIME_MS);
        }
    }
    return res;
}

void closeAll()
{
    for(uint8_t i = 0; i < RELAYS_COUNT; i++)
    {
        relays[i].write(false);
        timing::stop(relays[i].timer);
    }
}

namespace {
bool comparePatterns(const pattern::PatternBuffer patternBuf, const pattern::PatternBuffer relayPatternBuf)
{
    for(uint8_t i = 0; i < PATTERN_MAX_LENGTH; i++)
    {
        if (relayPatternBuf[i] != patternBuf[i])
        {
            return false;
        }
        else if (patternBuf[i] == pattern::PatternShape::TERMINATOR)
        {
            return true;
        }
    }
    return true;
}

void openRelay(relay &r, uint32_t time)
{
    r.write(true);
    r.timer.timerThreshold = RELAY_OPEN_TIME_MS;
    timing::restart(r.timer);
}
};
};