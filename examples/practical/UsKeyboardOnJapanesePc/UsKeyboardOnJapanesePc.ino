// Lets a US-engraved keyboard (or barcode reader) type exactly what its
// keycaps say on a PC that stays configured for the Japanese (ja_jp)
// layout. No settings are changed on the PC.
//
// Hardware: one ESP32-P4 (two USB roles are needed).
//   - USB Device = HS port: plug this into the PC. Fixed to HS by the
//     arduino-esp32 (3.3.10) library.
//   - USB Host = FS port: plug the US keyboard / barcode reader in here.
//     Port details and board caveats: see SwapCtrlCapsLock/README.ja.md.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbHost.h>
#include <EspUsbDevice.h>

// Used only by the commented-out FS PHY swap below.
#include "hal/usb_wrap_ll.h"
#include "soc/usb_wrap_struct.h"

EspUsbHost usbHost;     // US keyboard / barcode reader side (FS)
EspUsbDevice usbDevice; // PC side (HS)

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostKeyboardInputAdapter keyboard(usbHost);
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
  deviceConfig.product = "UsKeyboardOnJapanesePc";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge. Input-side settings live with the input: this
  //    keyboard is engraved as en_us, and its printable keys are converted
  //    to the PC's layout (hostLayout, step 3) with Shift consumed /
  //    synthesized as needed. Ctrl/Alt/GUI shortcuts pass through
  //    unconverted.
  esp32keybridge::ESP32KeyBridgeConfig config;
  esp32keybridge::InputConfig &keyboardConfig = config.addInputConfig();
  keyboardConfig.convertLayout(esp32keybridge::KeyboardLayout::enUs());
  bridge.addInput(keyboard, keyboardConfig);
  bridge.addOutput(pc);

  // 3) Build and apply the configuration.
  //
  // hostLayout declares how the PC already interprets keys (its existing
  // OS setting — nothing is changed on the PC).
  config.hostLayout = esp32keybridge::KeyboardLayout::jaJp();
  // BIOS-like environments interpret keys themselves: this key toggles the
  // conversion off/on at runtime. Pause exists on most keyboards and is
  // rarely used.
  config.layoutConversionToggle = esp32keybridge::KeyboardUsage::Pause;
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
