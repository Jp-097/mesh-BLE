//Callback ajustment//
#include "mesh/MeshNode.h"
#include "core/MessageBus.h"
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// ─── Estructura de paquete ───────────────────────────────────
struct MeshPacket {
    uint8_t  type;
    uint32_t msgId;
    uint64_t senderId;
    char     payload[128];
};

// ─── Constantes ──────────────────────────────────────────────
#define MSG_DATA   0x01
#define MSG_HELLO  0x02
#define MAX_PEERS  8
#define HELLO_INTERVAL_MS  5000

// ─── Estado ──────────────────────────────────────────────────
static MeshPacket packet;
static uint32_t localMsgCounter = 0;
static uint32_t lastSeenMsgId = 0;
static unsigned long lastHelloTime = 0;
static uint8_t knownPeers[MAX_PEERS][6];
static int peerCount = 0;

// ─── Helpers ─────────────────────────────────────────────────
static bool isKnownPeer(const uint8_t* mac) {
    for (int i = 0; i < peerCount; i++) {
        if (memcmp(knownPeers[i], mac, 6) == 0) return true;
    }
    return false;
}

static void addPeer(const uint8_t* mac) {
    if (peerCount >= MAX_PEERS) return;
    if (isKnownPeer(mac)) return;
    for (int i = 0; i < 6; i++) {
        if (mac[i] != 0xFF) {
            memcpy(knownPeers[peerCount], mac, 6);
            peerCount++;
            esp_now_peer_info_t info = {};
            memcpy(info.peer_addr, mac, 6);
            info.channel = 0;
            info.encrypt = false;
            esp_now_add_peer(&info);
            Serial.printf("[DISCOVER] Nuevo peer #%d\n", peerCount);
            break;
        }
    }
}

// ─── Callbacks ESP-NOW ───────────────────────────────────────
void MeshNode::onSent(const uint8_t* mac, esp_now_send_status_t status) {
    Serial.printf("[TX] %s\n",
        status == ESP_NOW_SEND_SUCCESS ? "OK" : "ERR");
}

void MeshNode::onRecv(const uint8_t* mac, const uint8_t* data, int len) {
    if (len < (int)sizeof(MeshPacket)) return;

    MeshPacket rx;
    memcpy(&rx, data, sizeof(rx));

    // Auto-registrar peer desconocido
    addPeer(mac);

    // Ignorar nuestros propios paquetes (rebote)
    if (rx.senderId == ESP.getEfuseMac()) return;

    // HELLO: silencioso, solo descubre
    if (rx.type == MSG_HELLO) {
        return;
    }

    // Detección de duplicados
    if (rx.msgId == lastSeenMsgId) {
        Serial.println("[DROP] Duplicado");
        return;
    }
    lastSeenMsgId = rx.msgId;

    Serial.println();
    Serial.println("===== RX =====");
    Serial.printf("ID: %lu\n", rx.msgId);
    Serial.printf("Nodo: %lu\n", (uint32_t)rx.senderId);
    Serial.printf("Texto: %s\n", rx.payload);
    Serial.println("==============");

    // Reenviar al teléfono si está conectado
    MessageBus::fromMesh(rx.payload);
}

// ─── Hello ───────────────────────────────────────────────────
void MeshNode::sendHello() {
    packet.type = MSG_HELLO;
    packet.msgId = ++localMsgCounter;
    packet.senderId = ESP.getEfuseMac();
    packet.payload[0] = '\0';
    esp_now_send(nullptr, (uint8_t*)&packet, sizeof(packet));
}

// ─── Init ────────────────────────────────────────────────────
void MeshNode::init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

        // Forzar canal 1 (trick: promiscuous on → set channel → promiscuous off)
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    Serial.println("Nodo Universal v2 (auto-discovery)");
    Serial.printf("NodeID: %lu\n", (uint32_t)ESP.getEfuseMac());
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());

    if (esp_now_init() != ESP_OK) {
        Serial.println("ERROR ESP-NOW init");
        return;
    }

    // Registrar broadcast
    uint8_t broadcastMac[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    esp_now_peer_info_t bInfo = {};
    memcpy(bInfo.peer_addr, broadcastMac, 6);
    bInfo.channel = 0;
    bInfo.encrypt = false;
    esp_now_add_peer(&bInfo);

    esp_now_register_send_cb(onSent);
    esp_now_register_recv_cb(onRecv);

    Serial.println("ESP-NOW listo. Enviando hello inicial...");
    sendHello();
}

// ─── Loop ────────────────────────────────────────────────────
void MeshNode::loop() {
    if (millis() - lastHelloTime >= HELLO_INTERVAL_MS) {
        lastHelloTime = millis();
        sendHello();
    }

    // Fallback: Serial (OTG) como 2da opción
    if (Serial.available()) {
        String text = Serial.readStringUntil('\n');
        text.trim();
        if (text.length() > 0) send(text.c_str());
    }
}   

// ─── Send ────────────────────────────────────────────────────
void MeshNode::send(const char* msg) {
    packet.type = MSG_DATA;
    packet.msgId = ++localMsgCounter;
    packet.senderId = ESP.getEfuseMac();
    strncpy(packet.payload, msg, sizeof(packet.payload) - 1);
    packet.payload[sizeof(packet.payload) - 1] = '\0';

    esp_now_send(nullptr, (uint8_t*)&packet, sizeof(packet));
    Serial.printf("[LOCAL TX] %s\n", msg);
}   