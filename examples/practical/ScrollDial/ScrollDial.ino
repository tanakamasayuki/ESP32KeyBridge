// A desktop scroll dial: turning a rotary encoder scrolls the mouse wheel,
// and pressing the encoder's shaft is a middle click. The PC sees an
// ordinary USB mouse (wheel + buttons), so it works everywhere with no
// driver or setting.
//
// This is the mapToAxis() counterpart of VolumeKnob (which uses mapToKeys()
// for media keys): here each detent feeds a relative axis instead of tapping
// a key. Point it at Axis::X instead of Axis::Wheel and it becomes a jog
// dial for video scrubbing; config.setAxisScale() shapes speed and
// direction (negative reverses, like macOS "natural" scrolling).
//
// Hardware: one ESP32-S3 (its single USB OTG port is the mouse device
// plugged into the PC). An EC11-style rotary encoder with a push switch:
// A phase = GPIO5, B phase = GPIO6, common = GND, push switch = GPIO7 and
// GND (adjust to your wiring).

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeGpio.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbDevice.h>

EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::RotaryEncoderInputAdapter dial(GPIO_NUM_5, GPIO_NUM_6);
esp32keybridge::GpioKeyInputAdapter click; // the encoder's push switch
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware.
  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "ScrollDial";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge. The dial feeds the wheel axis; the push switch is a
  //    plain GPIO key mapped to the middle mouse button. Both are inputs
  //    merged into the one mouse the PC sees.
  dial.mapToAxis(esp32keybridge::Axis::Wheel);
  click.addKey(GPIO_NUM_7, esp32keybridge::MouseUsage::Middle,
               /*activeLow=*/true, /*pullUp=*/true);
  bridge.addInput(dial);
  bridge.addInput(click);
  bridge.addOutput(pc);

  // 3) Build and apply the configuration. The axis scale sets scroll speed
  //    and direction: one wheel step per detent here; use a negative value
  //    to reverse, or a larger magnitude for faster scrolling.
  esp32keybridge::ESP32KeyBridgeConfig config;
  config.setAxisScale(esp32keybridge::Axis::Wheel, 1);
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
