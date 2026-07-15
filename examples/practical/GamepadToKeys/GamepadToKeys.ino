// Use a USB gamepad to control the PC as a keyboard: the D-pad becomes the
// arrow keys and the face buttons become Space/Enter/Escape. Handy for
// kiosks, a media PC on the couch, retro emulators that only take a
// keyboard, or as an accessible input. The PC sees an ordinary keyboard,
// so nothing is installed.
//
// The gamepad adapter maps controls to keys (any key kind); the bridge then
// forwards them like any other keyboard input, so you can also remap or add
// layers on top. Only mapped controls do anything.
//
// Button numbers differ between controllers. If a button does the wrong
// thing, press each button and adjust the mapButton() numbers below (or read
// the raw numbers with EspUsbHost's gamepad example).
//
// Hardware: one ESP32-P4 (two USB roles are needed, so single-USB chips like
// the ESP32-S3 cannot run this sketch alone).
//   - USB Device = HS port: plug this into the PC.
//   - USB Host = FS port: plug the gamepad in here.
// The P4 USB port details are documented in the SwapCtrlCapsLock example.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbHost.h>
#include <EspUsbDevice.h>

EspUsbHost usbHost;     // gamepad side (FS)
EspUsbDevice usbDevice; // PC side (HS)

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostGamepadInputAdapter gamepad(usbHost);
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware. The USB Device is fixed to the HS port, so the
  //    host explicitly takes the FS side.
  EspUsbHostConfig hostConfig;
  hostConfig.port = ESP_USB_HOST_PORT_FULL_SPEED;
  usbHost.begin(hostConfig);

  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "GamepadToKeys";
  usbDevice.begin(deviceConfig);

  // 2) Map the controls, then wire the bridge. The hat/D-pad is the arrow
  //    keys; the first four buttons are common keys. Button numbers are
  //    1-based as the controller reports them - adjust to your gamepad.
  gamepad.mapHat(esp32keybridge::KeyboardUsage::Up, esp32keybridge::KeyboardUsage::Down,
                 esp32keybridge::KeyboardUsage::Left, esp32keybridge::KeyboardUsage::Right);
  gamepad.mapButton(1, esp32keybridge::KeyboardUsage::Space);
  gamepad.mapButton(2, esp32keybridge::KeyboardUsage::Enter);
  gamepad.mapButton(3, esp32keybridge::KeyboardUsage::Escape);
  gamepad.mapButton(4, esp32keybridge::KeyboardUsage::Backspace);
  bridge.addInput(gamepad);
  bridge.addOutput(pc);

  // 3) No transform needed: the mapped keys pass straight through.
  esp32keybridge::ESP32KeyBridgeConfig config;
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
