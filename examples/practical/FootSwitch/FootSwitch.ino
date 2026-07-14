// Foot switches (or homemade buttons) that work as a keyboard: one pedal
// types a canned text, one is a play/pause media key, one turns the page
// for presentations. Anything hands-free.
//
// Hardware: one ESP32-S3 (its single USB OTG port is the keyboard device
// plugged into the PC) and switches between the pins below and GND. The
// pins use the internal pull-up; closed = pressed.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeGpio.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbDevice.h>

EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::GpioKeyInputAdapter pedals;
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

// Virtual keys exist only inside the bridge (never sent to the PC) and
// carry no meaning of their own: alias a predefined slot (VirtualUsage::V1
// .. V16, or virtualKey(n) for any number) with a name. Here it links the
// first pedal to the text macro below.
const esp32keybridge::Key kEmailPedal = esp32keybridge::VirtualUsage::V1;

void setup()
{
  // 1) Start the hardware.
  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "FootSwitch";
  usbDevice.begin(deviceConfig);

  // 2) Describe the switches and wire the bridge: addKey(pin, key,
  //    activeLow, pullUp), pins adjusted to your wiring. A pedal can press
  //    any key kind directly - a consumer key makes it a media pedal.
  pedals.addKey(GPIO_NUM_4, kEmailPedal, /*activeLow=*/true, /*pullUp=*/true);
  pedals.addKey(GPIO_NUM_5, esp32keybridge::ConsumerUsage::PlayPause,
                /*activeLow=*/true, /*pullUp=*/true);
  pedals.addKey(GPIO_NUM_6, esp32keybridge::KeyboardUsage::PageDown,
                /*activeLow=*/true, /*pullUp=*/true);
  bridge.addInput(pedals);
  bridge.addOutput(pc);

  // 3) Build and apply the configuration. Stepping on the first pedal
  //    types this text (the virtual key is consumed as the trigger).
  //    Characters are expanded into keystrokes with config.hostLayout
  //    (en_us by default); set it to match the PC's layout setting.
  esp32keybridge::ESP32KeyBridgeConfig config;
  config.textMacro(kEmailPedal, "user@example.com");
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
