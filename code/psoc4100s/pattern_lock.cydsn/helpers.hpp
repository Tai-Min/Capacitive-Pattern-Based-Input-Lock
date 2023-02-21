#pragma once
#include "serial.hpp"

#define INFOS
#ifdef INFOS
#define INFO(s) serial::send("[INFO] "  s "\r\n")
#else
#define INFO(s) (0)
#endif

#define WARNS
#ifdef WARNS
#define WARN(s) serial::send("[WARN] "  s "\r\n")
#else
#define WARN(s) (0)
#endif

#define DBGS
#ifdef DBGS
#define DBG(s) serial::send("[DBG] "  s "\r\n")
#else
#define DBG(s) (0)
#endif

#define COMM(s) serial::send("[COMM] " s "\r\n")

namespace helpers {
void writeVal(int val);
};
