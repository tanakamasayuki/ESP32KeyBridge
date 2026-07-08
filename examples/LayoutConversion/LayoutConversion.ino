#include <Arduino.h>
#include <ESP32KeyBridge.h>

class LayoutDemoInput : public esp32keybridge::InputAdapter
{
public:
  void update() override
  {
    state_.clear();
    state_.press(esp32keybridge::Key::A);
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
    Serial.print("layout:");
    for (size_t i = 0; i < state.codeCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(esp32keybridge::keyName(state.keyAt(i)));
    }
    Serial.println();
  }
};

esp32keybridge::ESP32KeyBridge bridge;
LayoutDemoInput input;
SerialKeyboardOutput output;

void setup()
{
  Serial.begin(115200);

  bridge.addInput(input);
  bridge.addOutput(output);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.layout.map(esp32keybridge::Key::A, esp32keybridge::Key::B);

  bridge.applyConfig(config);
  bridge.begin();
}

void loop()
{
  bridge.update();
  delay(1000);
}

