// Give any USB keyboard a "hold to navigate" layer: while CapsLock is held,
// H/J/K/L become the arrow keys (and U/O become Home/End), like a laptop Fn
// layer or the vim arrows. Nothing is installed on the PC, and CapsLock
// itself no longer toggles caps - it becomes the layer (Fn) key.
//
// How the layer works: CapsLock is remapped to a virtual key that triggers
// a layer; while that trigger is held, the layer's remaps apply. Resolution
// is fixed at press time, so releasing CapsLock first still holds whatever
// arrow was already pressed - no stuck keys.
//
// Hardware: one ESP32-P4 (two USB roles are needed, so single-USB chips
// like the ESP32-S3 cannot run this sketch alone).
//   - USB Device = HS port: plug this into the PC.
//   - USB Host = FS port: plug the keyboard in here.
// The P4 USB port details (and the optional FS PHY swap) are documented in
// the SwapCtrlCapsLock example; this sketch follows the same wiring.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbHost.h>
#include <EspUsbDevice.h>

EspUsbHost usbHost;     // real keyboard side (FS)
EspUsbDevice usbDevice; // PC side (HS)

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostKeyboardInputAdapter keyboard(usbHost);
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware. The USB Device is fixed to the HS port, so the
  //    host explicitly takes the FS side.
  EspUsbHostConfig hostConfig;
  hostConfig.port = ESP_USB_HOST_PORT_FULL_SPEED;
  usbHost.begin(hostConfig);

  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "NavLayer";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge.
  bridge.addInput(keyboard);
  bridge.addOutput(pc);

  // 3) Build and apply the configuration.
  esp32keybridge::ESP32KeyBridgeConfig config;

  // CapsLock becomes the layer (Fn) trigger. A virtual key lives only
  // inside the bridge and is never sent to the PC, so CapsLock no longer
  // reaches the PC at all - pick a different key here if you still want
  // CapsLock. VirtualUsage::V1..V16 are named slots; virtualKey(n) takes
  // any number.
  config.global.remap(esp32keybridge::KeyboardUsage::CapsLock,
                      esp32keybridge::VirtualUsage::V1);

  // The layer triggered by that virtual key. While it is held, these
  // remaps apply; keys with no entry pass through unchanged, so normal
  // typing still works with CapsLock held.
  esp32keybridge::LayerConfig &nav = config.addLayer(esp32keybridge::VirtualUsage::V1);
  nav.remap(esp32keybridge::KeyboardUsage::H, esp32keybridge::KeyboardUsage::Left);
  nav.remap(esp32keybridge::KeyboardUsage::J, esp32keybridge::KeyboardUsage::Down);
  nav.remap(esp32keybridge::KeyboardUsage::K, esp32keybridge::KeyboardUsage::Up);
  nav.remap(esp32keybridge::KeyboardUsage::L, esp32keybridge::KeyboardUsage::Right);
  nav.remap(esp32keybridge::KeyboardUsage::U, esp32keybridge::KeyboardUsage::Home);
  nav.remap(esp32keybridge::KeyboardUsage::O, esp32keybridge::KeyboardUsage::End);

  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
