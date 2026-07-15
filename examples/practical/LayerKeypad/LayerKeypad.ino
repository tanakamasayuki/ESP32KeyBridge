// A small GPIO keypad with two functions per key: normally the four keys
// are document/slide navigation (Page Up/Down, Home, End); while the Fn key
// is held they become media keys (volume and track skip). One physical
// pad, two banks - the classic Fn layer, built from plain buttons.
//
// How the layer works: the Fn key is a virtual key that triggers a layer.
// While it is held, the layer's remaps apply (here the navigation keys turn
// into consumer/media usages - a layer can remap across key kinds); with Fn
// released, every key passes through as its base function. Resolution is
// fixed at press time, so a key held across an Fn release keeps the value it
// was pressed as (no stuck keys).
//
// Hardware: one ESP32-S3 (its single USB OTG port is the keyboard device
// plugged into the PC) and momentary switches between the pins below and
// GND. The pins use the internal pull-up; closed = pressed.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeGpio.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbDevice.h>

EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::GpioKeyInputAdapter keypad;
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

// The Fn key is a virtual key: it lives only inside the bridge (never sent
// to the PC) and here only serves as the layer trigger.
const esp32keybridge::Key kFn = esp32keybridge::VirtualUsage::V1;

void setup()
{
  // 1) Start the hardware.
  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "LayerKeypad";
  usbDevice.begin(deviceConfig);

  // 2) Describe the switches and wire the bridge: addKey(pin, key,
  //    activeLow, pullUp), pins adjusted to your wiring. The base function
  //    of each key is what it sends with Fn released.
  keypad.addKey(GPIO_NUM_4, kFn, /*activeLow=*/true, /*pullUp=*/true);
  keypad.addKey(GPIO_NUM_5, esp32keybridge::KeyboardUsage::PageUp,
                /*activeLow=*/true, /*pullUp=*/true);
  keypad.addKey(GPIO_NUM_6, esp32keybridge::KeyboardUsage::PageDown,
                /*activeLow=*/true, /*pullUp=*/true);
  keypad.addKey(GPIO_NUM_7, esp32keybridge::KeyboardUsage::Home,
                /*activeLow=*/true, /*pullUp=*/true);
  keypad.addKey(GPIO_NUM_8, esp32keybridge::KeyboardUsage::End,
                /*activeLow=*/true, /*pullUp=*/true);
  bridge.addInput(keypad);
  bridge.addOutput(pc);

  // 3) Build and apply the configuration. The layer triggered by the Fn
  //    key remaps the four navigation keys to media keys while Fn is held;
  //    a layer remap can cross key kinds (keyboard -> consumer).
  esp32keybridge::ESP32KeyBridgeConfig config;
  esp32keybridge::LayerConfig &fn = config.addLayer(kFn);
  fn.remap(esp32keybridge::KeyboardUsage::PageUp,
           esp32keybridge::ConsumerUsage::VolumeIncrement);
  fn.remap(esp32keybridge::KeyboardUsage::PageDown,
           esp32keybridge::ConsumerUsage::VolumeDecrement);
  fn.remap(esp32keybridge::KeyboardUsage::Home,
           esp32keybridge::ConsumerUsage::ScanPreviousTrack);
  fn.remap(esp32keybridge::KeyboardUsage::End,
           esp32keybridge::ConsumerUsage::ScanNextTrack);
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
