#include <Arduino.h>
#include <ESP32KeyBridge.h>

class LayerDemoInput : public esp32keybridge::InputAdapter
{
public:
  void update() override
  {
    state_.clear();
    state_.press(esp32keybridge::Key::Fn1);
    state_.press(esp32keybridge::Key::A);
  }

  const esp32keybridge::KeyboardState &state() const override
  {
    return state_;
  }

private:
  esp32keybridge::KeyboardState state_;
};

class SerialKeyboardOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::KeyboardState &state) override
  {
    Serial.print("layered:");
    for (size_t i = 0; i < state.keyCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(static_cast<unsigned>(state.keyAt(i)));
    }
    Serial.println();
  }
};

esp32keybridge::ESP32KeyBridge bridge;
LayerDemoInput input;
SerialKeyboardOutput output;

void setup()
{
  Serial.begin(115200);

  bridge.addInput(input);
  bridge.addOutput(output);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.layer.setMomentary(esp32keybridge::Key::Fn1);
  config.layer.remap(esp32keybridge::Key::A, esp32keybridge::Key::B);

  bridge.applyConfig(config);
  bridge.begin();
}

void loop()
{
  bridge.update();
  delay(1000);
}

