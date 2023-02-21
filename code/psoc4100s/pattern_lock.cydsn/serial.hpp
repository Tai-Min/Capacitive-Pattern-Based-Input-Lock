#pragma once

#include "pattern.hpp"

namespace serial {
using relayCodeCallback = bool(*)(uint8_t, const pattern::PatternBuffer);

void init();
void setRelayCodeCallback(relayCodeCallback cb);
void sendChar(char c);
void send(const char *str);
bool update();
};