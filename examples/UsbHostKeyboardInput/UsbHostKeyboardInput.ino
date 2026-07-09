#include <Arduino.h>
#include <ESP32KeyBridge.h>
#include <adapters/EspUsbHostKeyboardInputAdapter.h>
#include <EspUsbHost.h>

class SerialStateOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::InputState &state) override
  {
    Serial.print("keys:");
    for (size_t i = 0; i < state.codeCount(); ++i)
    {
      Serial.print(' ');
      Serial.print(esp32keybridge::keySymbolName(state.keyAt(i)));
    }
    Serial.println();
  }
};

EspUsbHost usb;
esp32keybridge::EspUsbHostKeyboardInputAdapter input(usb);
SerialStateOutput output;
esp32keybridge::ESP32KeyBridge bridge;

void setup()
{
  Serial.begin(115200);
  delay(500);

  input.setLayout(esp32keybridge::KeyboardLayout::us());
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
