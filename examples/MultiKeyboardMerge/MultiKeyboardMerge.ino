#include <Arduino.h>
#include <ESP32KeyBridge.h>

class FixedKeyboardInput : public esp32keybridge::InputAdapter
{
public:
  FixedKeyboardInput(esp32keybridge::Key first, esp32keybridge::Key second = esp32keybridge::Key::None)
    : first_(first), second_(second)
  {
  }

  void update() override
  {
    state_.clear();
    state_.press(first_);
    state_.press(second_);
  }

  const esp32keybridge::KeyboardState &state() const override
  {
    return state_;
  }

private:
  esp32keybridge::Key first_;
  esp32keybridge::Key second_;
  esp32keybridge::KeyboardState state_;
};

class SerialKeyboardOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::KeyboardState &state) override
  {
    Serial.print("merged:");
    for (size_t i = 0; i < state.keyCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(esp32keybridge::keyName(state.keyAt(i)));
    }
    Serial.println();
  }
};

esp32keybridge::ESP32KeyBridge bridge;
FixedKeyboardInput leftKeyboard(esp32keybridge::Key::LeftShift);
FixedKeyboardInput rightKeyboard(esp32keybridge::Key::A);
SerialKeyboardOutput output;

void setup()
{
  Serial.begin(115200);

  bridge.addInput(leftKeyboard);
  bridge.addInput(rightKeyboard);
  bridge.addOutput(output);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.merge.shareModifiers = true;
  config.merge.shareKeys = true;

  bridge.applyConfig(config);
  bridge.begin();
}

void loop()
{
  bridge.update();
  delay(1000);
}

