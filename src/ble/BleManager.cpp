#include "ble/BleManager.h"
#include "core/MessageBus.h"
#include "setup/SetupWizard.h"
#include <Preferences.h>

// UUIDs del Nordic UART Service
#define NUS_SERVICE   "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define NUS_RX        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define NUS_TX        "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer*         BleManager::server  = nullptr;
BLECharacteristic* BleManager::txChar  = nullptr;
BLECharacteristic* BleManager::rxChar  = nullptr;
bool               BleManager::connected = false;

class ServerCB : public BLEServerCallbacks {
    void onConnect(BLEServer*) override {
        BleManager::connected = true;
        Serial.println("[BLE] Phone conectado");
    }
    void onDisconnect(BLEServer*) override {
        BleManager::connected = false;
        Serial.println("[BLE] Phone desconectado, re-advirtiendo...");
        delay(500);
        BleManager::server->startAdvertising();
    }
};

class RxCharCB : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* c) override {
        BleManager::onPhoneWrite(c);
    }
};

void BleManager::onPhoneWrite(BLECharacteristic* c) {
    String data = c->getValue().c_str();
    data.trim();
    if (data.length() == 0) return;

    // Primer uso: el texto escrito = nombre del nodo
    if (!SetupWizard::isConfigured()) {
        Serial.printf("[SETUP] Nombre: %s\n", data.c_str());
        SetupWizard::saveName(data.c_str());
        delay(300);
        ESP.restart();
        return;
    }

    Serial.printf("[BLE RX] %s\n", data.c_str());
    MessageBus::fromBle(data.c_str());
}

void BleManager::init() {
    String name;
    if (SetupWizard::isConfigured()) {
        Preferences prefs;
        prefs.begin("cfg", true);
        name = prefs.getString("name", "MeshNode");
        prefs.end();
    } else {
        name = "MeshNode-SETUP";
    }

    BLEDevice::init(name.c_str());

    server = BLEDevice::createServer();
    server->setCallbacks(new ServerCB());

    BLEService* service = server->createService(NUS_SERVICE);

    rxChar = service->createCharacteristic(
        NUS_RX,
        BLECharacteristic::PROPERTY_WRITE
    );
    rxChar->setCallbacks(new RxCharCB());

    txChar = service->createCharacteristic(
        NUS_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    txChar->addDescriptor(new BLE2902());

    service->start();
    server->getAdvertising()->start();

    Serial.printf("[BLE] Advertiendo como '%s'\n", name.c_str());
}

void BleManager::loop() {}

void BleManager::sendToPhone(const char* msg) {
    if (!connected) return;
    txChar->setValue(std::string(msg));
    txChar->notify();
}