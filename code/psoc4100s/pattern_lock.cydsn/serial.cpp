#include "serial.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "project.h"
#ifdef __cplusplus
}
#endif

#include "helpers.hpp"
#include "timing.hpp"

namespace serial {
void init();
void setRelayCodeCallback(const relayCodeCallback cb);
void sendChar(char c);
void send(const char *str);
bool update();

namespace {
uint16_t crc(uint8_t * data, uint8_t length);
bool publishPattern(uint8_t *buf, uint8_t len);
}

namespace {
constexpr uint8_t relaySetPatternCmd = 1;
relayCodeCallback relayCb = nullptr;
}

void init()
{
    UART_Start();
}

void setRelayCodeCallback(const relayCodeCallback cb)
{
    relayCb = cb;
}

void sendChar(char c)
{
    UART_UartPutChar(c);
}

void send(const char *str)
{
    UART_UartPutString(str);
}

bool update()
{
    uint8_t bufCursor = 0;
    uint8_t buf[16];
    uint8_t cmd;
    uint16_t checksumComputed;
    uint16_t checksumReceived;
    
    while(UART_SpiUartGetRxBufferSize() && bufCursor < 16)
    {
        buf[bufCursor++] = UART_SpiUartReadRxData();
        timing::doSleep(10, false);
    }
    
    if(!bufCursor)
    {
        return false;
    }

    // command + 2 crc
    if(bufCursor > 0 && bufCursor < 3)
    {
        goto cleanup;
    }
    
    checksumComputed = crc(buf, bufCursor - 2);
    checksumReceived = (buf[bufCursor - 2] << 8) | buf[bufCursor - 1];
    
    if(checksumComputed != checksumReceived)
    {
        goto cleanup;
    }
    
    cmd = buf[0];
    
    switch(cmd)
    {
    case relaySetPatternCmd:
        // start from first byte of command, ignore crc bytes
        if(!publishPattern(buf + 1, bufCursor - 3))
        {
            goto cleanup;
        }
         
        COMM("ok");
        return true;
        break;
    default:
        goto cleanup;
        break;
    }
    
cleanup:
    UART_SpiUartClearRxBuffer();
    COMM("err");
    return false;
}

namespace {
uint16_t crc(uint8_t * data, uint8_t length)
{
   uint8_t sum1 = 0;
   uint8_t sum2 = 0;
   
   for(int i = 0; i < length; i++)
   {
      sum1 = (sum1 + data[i]) % 255;
      sum2 = (sum2 + sum1) % 255;
   }
   return (sum2 << 8) | sum1;
}

bool publishPattern(uint8_t *buf, uint8_t len)
{
    // relay 
    // or relay + max 8 symbols
    if (len < 1 || len > 9)
    {
        return false;
    }
    
    uint8_t relay = buf[0];
    if (relay >= RELAYS_COUNT)
    {
        return false;
    }
    
    pattern::PatternShape shapes[PATTERN_MAX_LENGTH];
    for(uint8_t i = 0; i < PATTERN_MAX_LENGTH; i++)
    {
        if(i >= (len - 1))
        {
            shapes[i] = pattern::PatternShape::TERMINATOR;
        }
        else
        {
            // there are 5 shapes.
            if(buf[i + 1] > 5)
            {
                return false;
            }
            shapes[i] = static_cast<pattern::PatternShape>(buf[i + 1]);
        }
    }
    
    if(relayCb)
    {
        if(!relayCb(relay, shapes))
        {
            return false;
        }
    }
    return true;
}
}
};