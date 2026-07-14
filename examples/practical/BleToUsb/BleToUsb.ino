// Turns a BLE keyboard into a wired USB keyboard. For PCs without
// Bluetooth, BIOS/recovery screens, KVM switches, or machines where
// pairing is not allowed.
//
// Hardware: one ESP32-S3. Its single USB OTG port is the keyboard device
// plugged into the PC; the BLE keyboard pairs with the ESP32.
//
// NOTE: the BLE input adapter is a build-only mock (the BLE library is
// not decided yet).

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeBle.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbDevice.h>

EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
// Scanning, pairing, and reconnection live inside the adapter. If the
// keyboard's battery dies mid-press, the disconnect releases all of its
// keys — nothing gets stuck.
esp32keybridge::BleKeyboardInputAdapter keyboard;
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware.
  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "BleToUsb";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge. No configuration: keys pass through unchanged, and
  //    lock LED reports from the PC are forwarded back to the BLE keyboard.
  bridge.addInput(keyboard);
  bridge.addOutput(pc);
}

void loop()
{
  bridge.update();
  delay(1);
}
