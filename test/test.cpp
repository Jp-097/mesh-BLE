#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3 funcionando");
}

void loop() {
  Serial.println("Hola Mundo");
  delay(1000);
}