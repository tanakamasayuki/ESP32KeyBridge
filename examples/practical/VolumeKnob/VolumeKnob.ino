// A USB volume knob: turning a rotary encoder adjusts the PC volume,
// pressing it toggles mute.
//
// Hardware: one ESP32-S3 (its single USB OTG port is the device plugged
// into the PC) and an EC11-style rotary encoder: the A/B phases on the
// pins below with the common pin to GND, the push switch to GND. The
// encoder is counted by the PCNT peripheral (hardware counting, no
// interrupts).

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeGpio.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbDevice.h>

EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::RotaryEncoderInputAdapter knob(/*pinA=*/GPIO_NUM_5, /*pinB=*/GPIO_NUM_6);
esp32keybridge::GpioKeyInputAdapter button;
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware.
  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "VolumeKnob";
  usbDevice.begin(deviceConfig);

  // 2) Describe the knob and wire the bridge. Each detent taps the mapped
  //    key once; the encoder's push switch is a plain GPIO key.
  knob.mapToKeys(/*clockwise=*/esp32keybridge::ConsumerUsage::VolumeIncrement,
                 /*counterClockwise=*/esp32keybridge::ConsumerUsage::VolumeDecrement);
  button.addKey(GPIO_NUM_7, esp32keybridge::ConsumerUsage::Mute,
                /*activeLow=*/true, /*pullUp=*/true);
  bridge.addInput(knob);
  bridge.addInput(button);
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
