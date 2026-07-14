// Turns unused keys (F13-F15) into volume and play/pause keys, for
// keyboards that have no media keys of their own.
//
// Hardware: one ESP32-P4 (two USB roles are needed).
//   - USB Device = HS port: plug this into the PC. Enumerates as a
//     composite keyboard + consumer device. Fixed to HS by the
//     arduino-esp32 (3.3.10) library.
//   - USB Host = FS port: plug the keyboard in here. Port details and
//     board caveats: see SwapCtrlCapsLock/README.ja.md.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbHost.h>
#include <EspUsbDevice.h>

// Used only by the commented-out FS PHY swap below.
#include "hal/usb_wrap_ll.h"
#include "soc/usb_wrap_struct.h"

EspUsbHost usbHost;     // real keyboard side (FS)
EspUsbDevice usbDevice; // PC side (HS)

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostKeyboardInputAdapter keyboard(usbHost);
// The USB Device output is a composite HID device (keyboard + consumer +
// mouse); interfaces this sketch does not use simply never send reports.
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // Uncomment when the keyboard connector is wired to the FS CDC pins
  // (GPIO24/25) instead of the OTG pins (GPIO26/27). Must run before any
  // USB initialization. Check the actual wiring, not just the labels.
  // usb_wrap_ll_phy_select(&USB_WRAP, 0);

  // 1) Start the hardware. The USB Device is fixed to the HS port, so the
  //    host explicitly takes the FS side.
  EspUsbHostConfig hostConfig;
  hostConfig.port = ESP_USB_HOST_PORT_FULL_SPEED;
  usbHost.begin(hostConfig);

  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "MediaKeys";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge.
  bridge.addInput(keyboard);
  bridge.addOutput(pc);

  // 3) Build and apply the configuration. Remap crosses key kinds:
  //    keyboard keys become consumer keys. Holding F14 keeps Volume Up
  //    pressed; the PC decides how it repeats.
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
