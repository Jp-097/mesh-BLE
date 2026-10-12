//antes de que se vaya todo a 9999 lineas help//

#include <WiFi.h>
#include <esp_now.h>

struct MeshPacket {
    uint32_t msgId;
    uint64_t senderId;
    char payload[128];
};

MeshPacket packet;

esp_now_peer_info_t peerInfo;

uint32_t localMsgCounter = 0;

uint32_t lastSeenMsgId = 0;

uint8_t peer1[] = {
    0xAA,0xBB,0xCC,0xDD,0xEE,0xFF
};

uint8_t peer2[] = {
    0x11,0x22,0x33,0x44,0x55,0x66
};

void addPeer(uint8_t *mac)
{
    memset(&peerInfo, 0, sizeof(peerInfo));

    memcpy(peerInfo.peer_addr, mac, 6);

    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    esp_now_add_peer(&peerInfo);
}

void onDataSent(
    const uint8_t *mac_addr,
    esp_now_send_status_t status)
{
    Serial.print("[TX] ");

    if(status == ESP_NOW_SEND_SUCCESS)
        Serial.println("OK");
    else
        Serial.println("ERROR");
}

void onDataRecv(
    const uint8_t *mac,
    const uint8_t *incomingData,
    int len)
{
    MeshPacket incoming;

    memcpy(&incoming,
           incomingData,
           sizeof(incoming));

    if(incoming.msgId == lastSeenMsgId)
    {
        Serial.println("[DROP] Duplicado");
        return;
    }

    lastSeenMsgId = incoming.msgId;

    Serial.println();
    Serial.println("===== RX =====");

    Serial.print("ID: ");
    Serial.println(incoming.msgId);

    Serial.print("Nodo: ");
    Serial.println((uint32_t)incoming.senderId);

    Serial.print("Texto: ");
    Serial.println(incoming.payload);

    Serial.println("==============");
}

void setup()
{
    Serial.begin(115200);

    delay(2000);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.println();
    Serial.println("Nodo Universal v1");

    uint64_t myId = ESP.getEfuseMac();

    Serial.print("NodeID: ");
    Serial.println((uint32_t)myId);

    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());

    if(esp_now_init() != ESP_OK)
    {
        Serial.println("ERROR ESP-NOW");
        return;
    }

    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);

    addPeer(peer1);
    addPeer(peer2);

    Serial.println("ESP-NOW listo");
}

void loop()
{
    if(Serial.available())
    {
        String text =
            Serial.readStringUntil('\n');

        text.trim();

        if(text.length() == 0)
            return;

        packet.msgId = ++localMsgCounter;

        packet.senderId =
            ESP.getEfuseMac();

        strncpy(
            packet.payload,
            text.c_str(),
            sizeof(packet.payload) - 1
        );

        packet.payload[
            sizeof(packet.payload)-1
        ] = '\0';

        esp_now_send(
            peer1,
            (uint8_t*)&packet,
            sizeof(packet)
        );

        esp_now_send(
            peer2,
            (uint8_t*)&packet,
            sizeof(packet)
        );

        Serial.println();
        Serial.print("[LOCAL TX] ");
        Serial.println(text);
    }
}
