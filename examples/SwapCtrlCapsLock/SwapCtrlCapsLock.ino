// UC1: Swap Ctrl and CapsLock without changing the keyboard or the OS.
//
// This example uses ManualInputAdapter as a stand-in for a real keyboard
// input adapter, and prints the transformed output keys over Serial. With a
// USB Host input adapter and a USB Device output adapter, the same
// configuration swaps the keys between a physical keyboard and the PC.

#include <ESP32KeyBridge.h>

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::ManualInputAdapter keyboard;

const esp32keybridge::Key kCapsLock =
    esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock);
const esp32keybridge::Key kLeftCtrl =
    esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftCtrl);

void printOutputKeys()
{
  Serial.print("output:");
  for (size_t i = 0; i < bridge.outputKeys().count(); ++i)
  {
    esp32keybridge::Key key = bridge.outputKeys().at(i);
    Serial.print(' ');
    Serial.print(esp32keybridge::keyKindName(key.kind));
    Serial.print(":0x");
    Serial.print(key.code, HEX);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);

  bridge.addInput(keyboard);

  // A single-step remap never chains, so two entries swap the keys.
  esp32keybridge::ESP32KeyBridgeConfig config;
  config.global.remap(kCapsLock, kLeftCtrl);
  config.global.remap(kLeftCtrl, kCapsLock);
  bridge.applyConfig(config);

  bridge.begin();

  // Pressing the physical CapsLock produces LeftCtrl (0xE0).
  keyboard.press(kCapsLock);
  bridge.update();
  printOutputKeys();

  // Pressing the physical LeftCtrl produces CapsLock (0x39).
  keyboard.press(kLeftCtrl);
  bridge.update();
  printOutputKeys();

  keyboard.release(kCapsLock);
  keyboard.release(kLeftCtrl);
  bridge.update();
  printOutputKeys();
}

void loop()
{
  bridge.update();
}
