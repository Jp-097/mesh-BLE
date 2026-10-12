#pragma once
#include <Arduino.h>
#include <Preferences.h>

class SetupWizard {
public:
    static bool isConfigured();
    static void saveName(const char* name);
};   