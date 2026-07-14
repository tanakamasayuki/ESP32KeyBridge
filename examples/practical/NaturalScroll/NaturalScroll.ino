// Inverts the mouse wheel direction in hardware: natural scrolling on any
// PC without touching its settings (locked-down office machines, shared
// PCs, or one mouse behaving the same across every OS).
//
// Hardware: one ESP32-P4 (two USB roles are needed).
//   - USB Device = HS port: plug this into the PC. Enumerates as a
//     composite keyboard + consumer + mouse device. Fixed to HS by the
//     arduino-esp32 (3.3.10) library.
//   - USB Host = FS port: plug the mouse in here. Port details and
//     board caveats: see SwapCtrlCapsLock/README.ja.md.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbHost.h>
#include <EspUsbDevice.h>

// Used only by the commented-out FS PHY swap below.
#include "hal/usb_wrap_ll.h"
#include "soc/usb_wrap_struct.h"

EspUsbHost usbHost;     // real mouse side (FS)
EspUsbDevice usbDevice; // PC side (HS)

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostMouseInputAdapter mouse(usbHost);
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // Uncomment when the mouse connector is wired to the FS CDC pins
  // (GPIO24/25) instead of the OTG pins (GPIO26/27). Must run before any
  // USB initialization. Check the actual wiring, not just the labels.
  // usb_wrap_ll_phy_select(&USB_WRAP, 0);

  // 1) Start the hardware. The USB Device is fixed to the HS port, so the
  //    host explicitly takes the FS side.
  EspUsbHostConfig hostConfig;
  hostConfig.port = ESP_USB_HOST_PORT_FULL_SPEED;
  usbHost.begin(hostConfig);

  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "NaturalScroll";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge.
  bridge.addInput(mouse);
  bridge.addOutput(pc);

  // 3) Build and apply the configuration. A negative axis scale inverts
  //    the direction; buttons and pointer movement pass through untouched.
  esp32keybridge::ESP32KeyBridgeConfig config;
  config.setAxisScale(esp32keybridge::Axis::Wheel, -1);
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
