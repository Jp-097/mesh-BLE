//mesh//
#pragma once
#include <Arduino.h>
#include <esp_now.h>

class MeshNode {
public:
    static void init();
    static void loop();
    static void send(const char* msg);

private:
    static void onRecv(const uint8_t* mac, const uint8_t* data, int len);
    static void onSent(const uint8_t* mac, esp_now_send_status_t status);
    static void sendHello();
};   