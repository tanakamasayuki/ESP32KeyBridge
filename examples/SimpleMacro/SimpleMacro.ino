#include <Arduino.h>
#include <ESP32KeyBridge.h>

class MacroDemoInput : public esp32keybridge::InputAdapter
{
public:
  void update() override
  {
    state_.clear();
    state_.press(esp32keybridge::Key::Fn1);
  }

  const esp32keybridge::InputState &state() const override
  {
    return state_;
  }

private:
  esp32keybridge::InputState state_;
};

class SerialKeyboardOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::InputState &state) override
  {
    Serial.print("macro:");
    for (size_t i = 0; i < state.codeCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(esp32keybridge::keyName(state.keyAt(i)));
    }
    Serial.println();
  }
};

esp32keybridge::ESP32KeyBridge bridge;
MacroDemoInput input;
SerialKeyboardOutput output;

void setup()
{
  Serial.begin(115200);

  bridge.addInput(input);
  bridge.addOutput(output);

  esp32keybridge::ESP32KeyBridgeConfig config;
  const esp32keybridge::Key macroKeys[] = {
    esp32keybridge::Key::LeftCtrl,
    esp32keybridge::Key::A,
    esp32keybridge::Key::B,
  };
  config.global.macro(esp32keybridge::Key::Fn1, macroKeys, 3);

  bridge.applyConfig(config);
  bridge.begin();
}

void loop()
{
  bridge.update();
  delay(1000);
}

