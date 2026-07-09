#include <Arduino.h>
#include <ESP32KeyBridge.h>

class SerialHidReportOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::InputState &state) override
  {
    const esp32keybridge::HidKeyboardReport keyboard = esp32keybridge::buildHidKeyboardReport(state);
    const esp32keybridge::HidKeyboardRolloverReport rollover =
        esp32keybridge::buildHidKeyboardRolloverReport(state);
    const esp32keybridge::HidConsumerReport consumer = esp32keybridge::buildHidConsumerReport(state);
    const esp32keybridge::HidPointerReport pointer = esp32keybridge::buildHidPointerReport(state);

    Serial.print("keyboard modifiers=");
    Serial.print(keyboard.modifiers);
    Serial.print(" bootKeys=");
    Serial.print(keyboard.keyCount);
    Serial.print(" rolloverKeys=");
    Serial.print(rollover.keyCount);
    Serial.print(" consumer=0x");
    Serial.print(consumer.usage, HEX);
    Serial.print(" pointerButtons=0x");
    Serial.print(pointer.buttons, HEX);
    Serial.print(" overflow=");
    Serial.println(keyboard.overflow || rollover.overflow || consumer.overflow || pointer.overflow);
  }
};

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EventInputAdapter input;
SerialHidReportOutput output;

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

  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::LeftCtrl, pressed, now));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, pressed, now));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::B, pressed, now));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::C, pressed, now));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::D, pressed, now));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::E, pressed, now));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::F, pressed, now));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::G, pressed, now));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::H, pressed, now));
  input.apply(esp32keybridge::inputEvent(
      esp32keybridge::consumerCode(esp32keybridge::ConsumerUsage::VolumeIncrement), pressed, now));
  input.apply(esp32keybridge::inputEvent(esp32keybridge::pointerButtonCode(1), pressed, now));

  bridge.update();

  esp32keybridge::HidPointerReport movement;
  movement.apply(esp32keybridge::pointerAxisValueEvent(esp32keybridge::PointerAxis::X, pressed ? 4 : -4, now));
  movement.apply(esp32keybridge::pointerAxisValueEvent(esp32keybridge::PointerAxis::Wheel, pressed ? 1 : -1, now));
  Serial.print("pointer delta x=");
  Serial.print(movement.x);
  Serial.print(" wheel=");
  Serial.println(movement.wheel);

  delay(1000);
}
