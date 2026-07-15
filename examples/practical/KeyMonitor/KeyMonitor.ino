// Prints what the bridge produces to the serial console — a bring-up tool
// for input hardware. Wire your buttons, watch the presses and releases
// scroll by, and confirm the wiring before connecting a real output.
// Works the same next to a real output: add the monitor alongside it to
// see the traffic.
//
// Hardware: one ESP32-S3 with buttons between the pins below and GND. The
// log comes out Serial (the board's USB-serial); no USB HID output is
// used here.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeGpio.h>
#include <ESP32KeyBridgeSerial.h>

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::GpioKeyInputAdapter buttons;
esp32keybridge::LogOutputAdapter monitor(Serial);

void setup()
{
  // 1) Start the hardware.
  Serial.begin(115200);

  // 2) Describe the buttons and wire the bridge.
  buttons.addKey(GPIO_NUM_4, esp32keybridge::KeyboardUsage::A, /*activeLow=*/true, /*pullUp=*/true);
  buttons.addKey(GPIO_NUM_5, esp32keybridge::KeyboardUsage::B, /*activeLow=*/true, /*pullUp=*/true);
  buttons.addKey(GPIO_NUM_6, esp32keybridge::ConsumerUsage::PlayPause, /*activeLow=*/true, /*pullUp=*/true);
  bridge.addInput(buttons);
  bridge.addOutput(monitor);

  // 3) Build and apply the configuration (nothing to transform here).
  esp32keybridge::ESP32KeyBridgeConfig config;
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
