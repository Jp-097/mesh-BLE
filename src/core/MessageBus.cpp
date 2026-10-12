//Conect both//

#include "core/MessageBus.h"
#include "mesh/MeshNode.h"
#include "ble/BleManager.h"

void MessageBus::init() {}

void MessageBus::process() {}

// Llega del teléfono por BLE → enviar al mesh
void MessageBus::fromBle(const char* payload) {
    MeshNode::send(payload);
}

// Llega de otro nodo por ESP-NOW → reenviar al teléfono
void MessageBus::fromMesh(const char* payload) {
    BleManager::sendToPhone(payload);
}   