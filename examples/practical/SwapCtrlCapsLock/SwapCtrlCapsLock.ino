// Sits between a keyboard and a PC and swaps CapsLock and Ctrl. No settings
// are changed on the PC or on the keyboard, and the Caps Lock LED follows
// the swapped key correctly.
//
// Hardware: one ESP32-P4 (two USB roles are needed, so single-USB chips
// like the ESP32-S3 cannot run this sketch alone).
//   - USB Device = HS port: plug this into the PC. Fixed to HS by the
//     arduino-esp32 (3.3.10) library.
//   - USB Host = FS port: plug the keyboard in here. Which connector the
//     FS PHY reaches (OTG pins GPIO26/27 or CDC pins GPIO24/25) depends on
//     the board; see README.ja.md and the swap line in setup().
//
// NOTE: the USB adapters are build-only mocks until implementation step 7.

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
esp32keybridge::EspUsbDeviceKeyboardOutputAdapter pc(usbDevice);

void setup()
{
  // If your board has no FS OTG port (GPIO26/27) and the keyboard must go
  // into the CDC-side port (GPIO24/25), uncomment the next line. It swaps
  // the CDC/OTG mapping of the FS PHY and must run before any USB
  // initialization. Example: on the M5Stack Tab5 the Type-C port is
  // labeled OTG but is actually FS CDC, so it needs this swap. Check the
  // actual wiring of your board, not just the labels.
  // usb_wrap_ll_phy_select(&USB_WRAP, 0);

  // 1) Start the hardware. The USB Device is fixed to the HS port, so the
  //    host explicitly takes the FS side.
  EspUsbHostConfig hostConfig;
  hostConfig.port = ESP_USB_HOST_PORT_FULL_SPEED;
  usbHost.begin(hostConfig);

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
