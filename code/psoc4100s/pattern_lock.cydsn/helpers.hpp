#pragma once

#define INFOS
#ifdef INFOS
#define INFO(s) UART_UartPutString("[INFO] "  s "\r\n")
#else
#define INFO(s) (0)
#endif

#define WARNS
#ifdef WARNS
#define WARN(s) UART_UartPutString("[WARN] "  s "\r\n")
#else
#define WARN(s) (0)
#endif

#define DBGS
#ifdef DBGS
#define DBG(s) UART_UartPutString("[DBG] "  s "\r\n")
#else
#define DBG(s) (0)
#endif

namespace helpers {
void writeVal(int val);
};
