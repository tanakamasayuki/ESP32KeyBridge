#include <Arduino.h>
#include <ESP32KeyBridge.h>

class FixedKeyboardInput : public esp32keybridge::InputAdapter
{
public:
  void update() override
  {
    state_.clear();
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
    Serial.print("runtime:");
    for (size_t i = 0; i < state.keyCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(esp32keybridge::keyName(state.keyAt(i)));
    }
    Serial.println();
  }
};

class ExampleConfigService
{
public:
  bool available() const
  {
    return !applied_ && millis() > 5000;
  }

  esp32keybridge::ESP32KeyBridgeConfig readConfig()
  {
    applied_ = true;

    esp32keybridge::ESP32KeyBridgeConfig config;
    config.global.remap(esp32keybridge::Key::A, esp32keybridge::Key::B);
    return config;
  }

private:
  bool applied_ = false;
};

esp32keybridge::ESP32KeyBridge bridge;
FixedKeyboardInput input;
SerialKeyboardOutput output;
ExampleConfigService configService;

void setup()
{
  Serial.begin(115200);

  bridge.addInput(input);
  bridge.addOutput(output);
  bridge.begin();
}

void loop()
{
  if (configService.available())
  {
    esp32keybridge::ESP32KeyBridgeConfig next = configService.readConfig();
    esp32keybridge::ESP32KeyBridgeConfigError error;

    if (bridge.validateConfig(next, error))
    {
      bridge.applyConfig(next);
      Serial.println("config applied");
    }
    else
    {
      Serial.println(error.message != nullptr ? error.message : "config error");
    }
  }

  bridge.update();
  delay(1000);
}

