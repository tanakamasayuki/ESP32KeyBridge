// A homemade numeric keypad: 12 switches in a 4x3 matrix wired straight
// to GPIO pins, showing up as a USB keyboard. The same recipe scales to
// macro pads and full custom keyboards.
//
// Hardware: one ESP32-S3 (its single USB OTG port is the keyboard device
// plugged into the PC) and a switch matrix: each switch sits between one
// row line and one column line. Diodes are optional — without them, ghost
// blocking limits reliable chords to two keys.
//
//   Layout            rows (scan outputs)   columns (inputs, pull-up)
//   7 8 9             GPIO4  GPIO5          GPIO10  GPIO11  GPIO12
//   4 5 6             GPIO6  GPIO7
//   1 2 3
//   0 . Enter

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeGpio.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbDevice.h>

EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::GpioMatrixInputAdapter pad;
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware.
  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "NumPad";
  usbDevice.begin(deviceConfig);

  // 2) Describe the matrix and wire the bridge. Wiring and keymap are
  //    separate: the pin lists describe the lines (adjusted to your
  //    wiring), and setKeys() assigns the keys in row-major order — the
  //    code layout below IS the physical layout. Unused positions take
  //    esp32keybridge::Key().
  pad.setRowPins({GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7});
  pad.setColPins({GPIO_NUM_10, GPIO_NUM_11, GPIO_NUM_12});
  // The single-name using declaration (not `using namespace`) keeps the
  // keymap readable as a grid; the clang-format guards keep the grid
  // shape through auto-formatting.
  using esp32keybridge::KeyboardUsage;
  // clang-format off
  pad.setKeys({
      //              GPIO10                  GPIO11                       GPIO12
      /* GPIO4 */     KeyboardUsage::Keypad7, KeyboardUsage::Keypad8,      KeyboardUsage::Keypad9,
      /* GPIO5 */     KeyboardUsage::Keypad4, KeyboardUsage::Keypad5,      KeyboardUsage::Keypad6,
      /* GPIO6 */     KeyboardUsage::Keypad1, KeyboardUsage::Keypad2,      KeyboardUsage::Keypad3,
      /* GPIO7 */     KeyboardUsage::Keypad0, KeyboardUsage::KeypadPeriod, KeyboardUsage::KeypadEnter,
  });
  // clang-format on
  bridge.addInput(pad);
  bridge.addOutput(pc);

  // 3) Build and apply the configuration (nothing to transform here).
  esp32keybridge::ESP32KeyBridgeConfig config;
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
