#include <Arduino.h>
#include <ESP32KeyBridge.h>

class RecordingOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::KeyboardState &state) override
  {
    last_ = state;
    ++writeCount_;
  }

  esp32keybridge::KeyboardState last_;
  int writeCount_ = 0;
};

esp32keybridge::EventInputAdapter input;
RecordingOutput output;
esp32keybridge::ESP32KeyBridge bridge;

void setup()
{
  Serial.begin(115200);
  delay(1500);

  Serial.println("TEST_BEGIN core_smoke");

  bridge.addInput(input);
  bridge.addOutput(output);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.global.remap(esp32keybridge::Key::CapsLock, esp32keybridge::Key::LeftCtrl);
  config.global.disable(esp32keybridge::Key::Insert);
  bridge.applyConfig(config);
  bridge.begin();

  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::CapsLock, true, millis()));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, true, millis()));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::Insert, true, millis()));
  bridge.update();

  const bool firstOk =
      output.writeCount_ == 1 &&
      bridge.mergedState().isPressed(esp32keybridge::Key::CapsLock) &&
      bridge.mergedState().isPressed(esp32keybridge::Key::Insert) &&
      bridge.outputState().isPressed(esp32keybridge::Key::LeftCtrl) &&
      bridge.outputState().isPressed(esp32keybridge::Key::A) &&
      !bridge.outputState().isPressed(esp32keybridge::Key::CapsLock) &&
      !bridge.outputState().isPressed(esp32keybridge::Key::Insert) &&
      output.last_.isPressed(esp32keybridge::Key::LeftCtrl) &&
      output.last_.isPressed(esp32keybridge::Key::A);

  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, false, millis()));
  bridge.update();

  const bool secondOk =
      output.writeCount_ == 2 &&
      bridge.outputState().isPressed(esp32keybridge::Key::LeftCtrl) &&
      !bridge.outputState().isPressed(esp32keybridge::Key::A);

  Serial.println("TEST_END");
  Serial.println(firstOk && secondOk ? "OK" : "NG");
}

void loop()
{
}
