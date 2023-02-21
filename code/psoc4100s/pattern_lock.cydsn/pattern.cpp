#include "pattern.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "project.h"
#ifdef __cplusplus
}
#endif

#include "timing.hpp"
#include "ssie.hpp"

namespace pattern {
bool isFull();
bool insertSymbol(PatternShape symbol);
bool removeLastSymbol();
void clear();
const PatternBuffer getPatternBuf();
PatternShape classify(const capsense::TouchpadBuffer buf);
void updatePatternBlinker(bool active);
void blinkErrSignalAndClearPattern();

namespace {
    void writePatternBit(bool b);
    void writePattern(uint8_t ptrn);
}

namespace {
    uint8_t patternCount = 0;
    PatternShape patternBuf[PATTERN_MAX_LENGTH] = {PatternShape::TERMINATOR};
};

bool isFull()
{
    return (patternCount >= PATTERN_MAX_LENGTH);
}

bool insertSymbol(PatternShape symbol)
{
    if (patternCount >= PATTERN_MAX_LENGTH)
    {
        return false;
    }
    
    patternBuf[patternCount++] = symbol;
    writePattern(patternCount);
    return true;
}

bool removeLastSymbol()
{
    if (!patternCount)
    {
        return false;
    }
    
    patternBuf[--patternCount] = PatternShape::TERMINATOR;
    
    writePattern(patternCount);
    return true;
}

void clear()
{
    while (patternCount)
    {
        patternBuf[--patternCount] = PatternShape::TERMINATOR;
    }
    writePattern(patternCount);
}

const PatternBuffer getPatternBuf()
{
    return patternBuf;
}

PatternShape classify(const capsense::TouchpadBuffer buf)
{
    return static_cast<PatternShape>(ssie::run(buf));
}

void updatePatternBlinker(bool active)
{
    static bool flipper = false;
    
    if (active)
    {
        if (flipper && patternCount < PATTERN_MAX_LENGTH)
        {
            writePattern(patternCount + 1);
        }
        else 
        {
            writePattern(patternCount);
        }
        flipper = !flipper;
    }
    else
    {
        flipper = false;
        writePattern(patternCount);
    }
}

void blinkErrSignalAndClearPattern()
{
    writePattern(PATTERN_MAX_LENGTH);
    timing::doSleep(PATTERN_ERR_BLINK_DELAY_MS);
    writePattern(0);
    timing::doSleep(PATTERN_ERR_BLINK_DELAY_MS);
    writePattern(PATTERN_MAX_LENGTH);
    timing::doSleep(PATTERN_ERR_BLINK_DELAY_MS);
    clear();
}

namespace 
{
    void writePatternBit(bool b)
    {
        pattern_clk_Write(false);
        pattern_data_Write(b);
        pattern_clk_Write(true);
    }
    
    void writePattern(uint8_t ptrn)
    {
        pattern_latch_Write(false);
        for(uint8_t i = 0; i < PATTERN_MAX_LENGTH - ptrn; i++)
        {
            writePatternBit(false);
        }
        for(uint8_t i = 0; i < ptrn; i++)
        {
            writePatternBit(true);
        }
        pattern_latch_Write(true);
    }
}

};