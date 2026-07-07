#include <ESP32KeyBridge.h>

esp32keybridge::ESP32KeyBridge keyBridge;

void setup()
{
  Serial.begin(115200);
  keyBridge.begin();
}

void loop()
{
  keyBridge.update();
}
