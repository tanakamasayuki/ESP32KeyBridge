#include <Arduino.h>
#include <ESP32KeyBridge.h>

class FixedKeyboardInput : public esp32keybridge::InputAdapter
{
public:
  explicit FixedKeyboardInput(esp32keybridge::Key key)
    : key_(key)
  {
  }

  void update() override
  {
    state_.clear();
    state_.press(key_);
  }

  const esp32keybridge::KeyboardState &state() const override
  {
    return state_;
  }

private:
  esp32keybridge::Key key_;
  esp32keybridge::KeyboardState state_;
};

class SerialKeyboardOutput : public esp32keybridge::OutputAdapter
{
public:
  explicit SerialKeyboardOutput(const char *name)
    : name_(name)
  {
  }

  void write(const esp32keybridge::KeyboardState &state) override
  {
    Serial.print(name_);
    Serial.print(':');
    for (size_t i = 0; i < state.keyCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(esp32keybridge::keyName(state.keyAt(i)));
    }
    Serial.println();
  }

private:
  const char *name_;
};

esp32keybridge::ESP32KeyBridge bridge;
FixedKeyboardInput inputA(esp32keybridge::Key::A);
FixedKeyboardInput inputB(esp32keybridge::Key::B);
SerialKeyboardOutput outputA("outputA");
SerialKeyboardOutput outputB("outputB");

bool reconfigured = false;

void setup()
{
  Serial.begin(115200);

  bridge.addInput(inputA);
  bridge.addOutput(outputA);
  bridge.begin();
}

void loop()
{
  if (!reconfigured && millis() > 5000)
  {
    bridge.clearInputs();
    bridge.clearOutputs();
    bridge.addInput(inputB);
    bridge.addOutput(outputB);
    reconfigured = true;
    Serial.println("reconfigured");
  }

  bridge.update();
  delay(1000);
}

