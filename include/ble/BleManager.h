//Comunication BLE//

#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BleManager {
public:
    static void init();
    static void loop();
    static void sendToPhone(const char* msg);
    static bool connected;
    static BLEServer* server;
    static void onPhoneWrite(BLECharacteristic* c);   // ← mueve aquí

private:
    static BLECharacteristic*  txChar;
    static BLECharacteristic*  rxChar;
};