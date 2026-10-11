#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(9000);
  Serial.println("Esp32 iniciando");

  WiFi.mode(WIFI_STA);

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
}
