//21 friendliest line's of code help me out//

#include <Arduino.h>
#include "mesh/MeshNode.h"
#include "ble/BleManager.h"
#include "core/MessageBus.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    MessageBus::init();
    MeshNode::init();
    BleManager::init();
    Serial.println("Sistema listo");
}

void loop() {
    MeshNode::loop();
    BleManager::loop();
    MessageBus::process();
}