#include <Arduino.h>
#include <ESP32KeyBridge.h>
#include <EspUsbHost.h>

class EspUsbHostKeyboardInputAdapter : public esp32keybridge::InputAdapter
{
public:
  explicit EspUsbHostKeyboardInputAdapter(EspUsbHost &usb)
      : usb_(usb)
  {
  }

  void begin()
  {
    usb_.onKeyboard([this](const EspUsbHostKeyboardEvent &event)
                    {
                      const esp32keybridge::Key key = esp32keybridge::keyFromHidUsage(event.keycode);
                      if (key != esp32keybridge::Key::None)
                      {
                        state_.apply(esp32keybridge::keyEvent(key, event.pressed, millis()));
                      } });
  }

  void update() override
  {
  }

  const esp32keybridge::InputState &state() const override
  {
    return state_;
  }

private:
  EspUsbHost &usb_;
  esp32keybridge::InputState state_;
};

class SerialStateOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::InputState &state) override
  {
    Serial.print("keys:");
    for (size_t i = 0; i < state.codeCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(esp32keybridge::keyName(state.keyAt(i)));
    }
    Serial.println();
  }
};

EspUsbHost usb;
EspUsbHostKeyboardInputAdapter input(usb);
SerialStateOutput output;
esp32keybridge::ESP32KeyBridge bridge;

void setup()
{
  Serial.begin(115200);
  delay(500);

  input.begin();
  bridge.addInput(input);
  bridge.addOutput(output);
  bridge.begin();

  if (!usb.begin())
  {
    Serial.printf("HOST_BEGIN_FAILED %s\n", usb.lastErrorName());
  }
}

void loop()
{
  bridge.update();
  delay(1);
}
