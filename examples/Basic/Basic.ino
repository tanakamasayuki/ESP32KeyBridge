#include <ESP32KeyBridge.h>

ESP32KeyBridge keyBridge;

void setup()
{
  Serial.begin(115200);
  keyBridge.begin();
}

void loop()
{
  keyBridge.update();
}

