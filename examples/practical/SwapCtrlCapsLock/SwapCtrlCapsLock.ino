// Sits between a keyboard and a PC and swaps CapsLock and Ctrl. No settings
// are changed on the PC or on the keyboard, and the Caps Lock LED follows
// the swapped key correctly.
//
// Hardware: one ESP32-P4.
//   - USB Host port: plug the keyboard in here.
//   - USB Device port: plug this into the PC.
//
// NOTE: the USB adapters are build-only mocks until implementation step 7.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbHost.h>
#include <EspUsbDevice.h>

EspUsbHost usbHost;     // real keyboard side
EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostKeyboardInputAdapter keyboard(usbHost);
esp32keybridge::EspUsbDeviceKeyboardOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware. Stack options (ports on multi-port chips,
  //    VID/PID, ...) use the EspUsbHost / EspUsbDevice configs directly.
  usbHost.begin();

  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "SwapCtrlCapsLock";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge. A single-step remap never chains, so two entries
  //    swap the keys.
  bridge.addInput(keyboard);
  bridge.addOutput(pc);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.global.remap(esp32keybridge::KeyboardUsage::CapsLock,
                      esp32keybridge::KeyboardUsage::LeftCtrl);
  config.global.remap(esp32keybridge::KeyboardUsage::LeftCtrl,
                      esp32keybridge::KeyboardUsage::CapsLock);
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
