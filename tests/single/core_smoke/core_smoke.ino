#include <Arduino.h>
#include <ESP32KeyBridge.h>

class VirtualInput : public esp32keybridge::InputAdapter
{
public:
  void update() override
  {
    state_.clear();
    state_.press(esp32keybridge::Key::CapsLock);
    state_.press(esp32keybridge::Key::A);
    state_.press(esp32keybridge::Key::Insert);
  }

  const esp32keybridge::KeyboardState &state() const override
  {
    return state_;
  }

private:
  esp32keybridge::KeyboardState state_;
};

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

VirtualInput input;
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
  bridge.update();

  const bool ok =
      output.writeCount_ == 1 &&
      bridge.mergedState().isPressed(esp32keybridge::Key::CapsLock) &&
      bridge.mergedState().isPressed(esp32keybridge::Key::Insert) &&
      bridge.outputState().isPressed(esp32keybridge::Key::LeftCtrl) &&
      bridge.outputState().isPressed(esp32keybridge::Key::A) &&
      !bridge.outputState().isPressed(esp32keybridge::Key::CapsLock) &&
      !bridge.outputState().isPressed(esp32keybridge::Key::Insert) &&
      output.last_.isPressed(esp32keybridge::Key::LeftCtrl) &&
      output.last_.isPressed(esp32keybridge::Key::A);

  Serial.println("TEST_END");
  Serial.println(ok ? "OK" : "NG");
}

void loop()
{
}

