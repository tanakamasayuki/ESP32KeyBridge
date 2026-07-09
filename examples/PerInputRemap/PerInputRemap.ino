#include <Arduino.h>
#include <ESP32KeyBridge.h>

class FixedKeyboardInput : public esp32keybridge::InputAdapter
{
public:
  FixedKeyboardInput(esp32keybridge::KeySymbol first, esp32keybridge::KeySymbol second = esp32keybridge::KeySymbol::None)
    : first_(first), second_(second)
  {
  }

  void update() override
  {
    state_.clear();
    state_.press(first_);
    state_.press(second_);
  }

  const esp32keybridge::InputState &state() const override
  {
    return state_;
  }

private:
  esp32keybridge::KeySymbol first_;
  esp32keybridge::KeySymbol second_;
  esp32keybridge::InputState state_;
};

class SerialKeyboardOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::InputState &state) override
  {
    Serial.print("output:");
    for (size_t i = 0; i < state.codeCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(esp32keybridge::keySymbolName(state.keyAt(i)));
    }
    Serial.println();
  }
};

esp32keybridge::ESP32KeyBridge bridge;
FixedKeyboardInput keyboard(esp32keybridge::KeySymbol::Enter);
FixedKeyboardInput scanner(esp32keybridge::KeySymbol::Enter, esp32keybridge::KeySymbol::A);
SerialKeyboardOutput output;

void setup()
{
  Serial.begin(115200);

  bridge.addInput(keyboard, 0);
  bridge.addInput(scanner, 1);
  bridge.addOutput(output);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.input(1).remap(esp32keybridge::KeySymbol::Enter, esp32keybridge::KeySymbol::Tab);
  config.global.remap(esp32keybridge::KeySymbol::A, esp32keybridge::KeySymbol::B);

  bridge.applyConfig(config);
  bridge.begin();
}

void loop()
{
  bridge.update();
  delay(1000);
}
