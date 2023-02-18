#include "helpers.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "project.h"
#ifdef __cplusplus
}
#endif

namespace helpers {
void writeVal(int val);

void writeVal(int32_t val)
{
    uint8_t buf[11] = {0};
    
    uint16_t divider = 1;
    uint8_t idxCntr = 0;

    // Check negative value.
    if (val < 0)
    {
        buf[idxCntr++] = '-';
        val *= -1;
    }

    int16_t valTmp = val;

    // Get max divider to get num of digits.
    while (valTmp >= 10)
    {
        divider *= 10;
        valTmp /= 10;
    }

    // Convert value starting from highest digit
    // i.e. "1" ... to "3" in 1223.
    while (divider)
    {
        uint8_t digitInt = val / divider;
        val -= digitInt * divider;
        divider /= 10;

        buf[idxCntr++] = digitInt + '0'; // To ASCII.
    }

    for(int i = idxCntr - 1; i >= 0; i--)
    {
        UART_tx_Write(buf[i]);
    }
}
};