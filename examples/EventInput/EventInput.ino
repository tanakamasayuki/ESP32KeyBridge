#include <Arduino.h>
#include <ESP32KeyBridge.h>

class SerialKeyboardOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::InputState &state) override
  {
    Serial.print("event:");
    for (size_t i = 0; i < state.codeCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(esp32keybridge::keyName(state.keyAt(i)));
    }
    Serial.println();
  }
};

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EventInputAdapter input;
SerialKeyboardOutput output;

void setup()
{
  Serial.begin(115200);

  bridge.addInput(input);
  bridge.addOutput(output);
  bridge.begin();
}

void loop()
{
  const uint32_t now = millis();
  const bool pressed = (now / 2000) % 2 == 0;

  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, pressed, now));
  bridge.update();
  delay(1000);
}

