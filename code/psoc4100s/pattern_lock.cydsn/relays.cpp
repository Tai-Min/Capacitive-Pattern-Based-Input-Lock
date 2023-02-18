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

namespace relays {
bool matchPatternAndOpen(const pattern::PatternBuffer buf);
void closeAll();

namespace {
    struct relay {
        void(*write)(uint8_t);
        timing::Timer timer;
        pattern::PatternShape code[PATTERN_MAX_LENGTH];
    };
    
    relay relays[RELAYS_COUNT] = {
        {.write = relay_1_Write},
        {.write = relay_2_Write},
        {.write = relay_3_Write},
        {.write = relay_4_Write}
    };
};

bool matchPatternAndOpen(const pattern::PatternBuffer buf)
{
    
    return true;
}

void closeAll()
{
    for(uint8_t i = 0; i < RELAYS_COUNT; i++)
    {
        relays[i].write(false);
        timing::stop(relays[i].timer);
    }
}

};