// Merge several input devices into one keyboard the PC sees. A USB keyboard
// and a few extra GPIO macro buttons are combined and forwarded to the PC as
// a single HID device: keys from either source appear together, so you can
// bolt extra hardware keys onto an existing keyboard without the PC knowing.
//
// Two kinds of merging happen here:
//   - The USB Host keyboard adapter already merges every keyboard on its
//     stack (plug a USB hub with several keyboards and they all combine, up
//     to 4) - that is one addInput().
//   - Adding a second, different input (the GPIO macro pad) with another
//     addInput() merges across adapters too.
// The merge is a union: a key stays pressed until every source that holds
// it releases it, so the same key on two devices never gets stuck.
//
// Hardware: one ESP32-P4 (two USB roles are needed - USB keyboard input and
// USB device output - so single-USB chips like the ESP32-S3 cannot run this
// sketch alone).
//   - USB Device = HS port: plug this into the PC.
//   - USB Host = FS port: plug the keyboard (or a hub of keyboards) in here.
// The P4 USB port details are documented in the SwapCtrlCapsLock example.
// The GPIO macro buttons are momentary switches between the pins below and
// GND (internal pull-up, closed = pressed).

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeGpio.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbHost.h>
#include <EspUsbDevice.h>

EspUsbHost usbHost;     // keyboard side (FS)
EspUsbDevice usbDevice; // PC side (HS)

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostKeyboardInputAdapter keyboard(usbHost);
esp32keybridge::GpioKeyInputAdapter macroPad;
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware. The USB Device is fixed to the HS port, so the
  //    host explicitly takes the FS side.
  EspUsbHostConfig hostConfig;
  hostConfig.port = ESP_USB_HOST_PORT_FULL_SPEED;
  usbHost.begin(hostConfig);

  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "MergeKeyboards";
  usbDevice.begin(deviceConfig);

  // 2) Wire both inputs into the one bridge. The GPIO buttons send keys the
  //    keyboard may not have - here media/navigation extras.
  macroPad.addKey(GPIO_NUM_4, esp32keybridge::ConsumerUsage::Mute,
                  /*activeLow=*/true, /*pullUp=*/true);
  macroPad.addKey(GPIO_NUM_5, esp32keybridge::ConsumerUsage::PlayPause,
                  /*activeLow=*/true, /*pullUp=*/true);
  macroPad.addKey(GPIO_NUM_6, esp32keybridge::KeyboardUsage::F13,
                  /*activeLow=*/true, /*pullUp=*/true);
  bridge.addInput(keyboard);
  bridge.addInput(macroPad);
  bridge.addOutput(pc);

  // 3) No transform needed: the default configuration passes the merged
  //    keys straight through.
  esp32keybridge::ESP32KeyBridgeConfig config;
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
