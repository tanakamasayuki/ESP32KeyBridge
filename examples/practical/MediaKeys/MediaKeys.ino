// Turns unused keys (F13-F15) into volume and play/pause keys, for
// keyboards that have no media keys of their own.
//
// Hardware: one ESP32-P4.
//   - USB Host port: plug the keyboard in here.
//   - USB Device port: plug this into the PC (enumerates as a composite
//     keyboard + consumer device).
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
// Composite HID output: consumer (and mouse) reports exist on the PC side
// because this output is registered.
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware.
  usbHost.begin();

  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "MediaKeys";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge. Remap crosses key kinds: keyboard keys become
  //    consumer keys. Holding F14 keeps Volume Up pressed; the PC decides
  //    how it repeats.
  bridge.addInput(keyboard);
  bridge.addOutput(pc);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.global.remap(esp32keybridge::KeyboardUsage::F13,
                      esp32keybridge::ConsumerUsage::VolumeDecrement);
  config.global.remap(esp32keybridge::KeyboardUsage::F14,
                      esp32keybridge::ConsumerUsage::VolumeIncrement);
  config.global.remap(esp32keybridge::KeyboardUsage::F15,
                      esp32keybridge::ConsumerUsage::PlayPause);
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
