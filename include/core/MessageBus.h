//message bus (transfer messages)//
#pragma once
#include <Arduino.h>

class MessageBus {
public:
    static void init();
    static void process();
    static void fromMesh(const char* payload);
    static void fromBle(const char* payload);
};   