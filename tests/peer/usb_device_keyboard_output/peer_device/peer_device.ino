#include <ESP32KeyBridge.h>
#include <EspUsbDevice.h>

EspUsbDevice device;
EspUsbDeviceHidKeyboard keyboard(device);
esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EventInputAdapter input;

class UsbKeyboardOutput : public esp32keybridge::OutputAdapter
{
public:
  void write(const esp32keybridge::InputState &state) override
  {
    if (sent_ || !state.isPressed(esp32keybridge::KeySymbol::A))
    {
      return;
    }
    sent_ = keyboard.tapKey('a');
  }

  bool sent() const
  {
    return sent_;
  }

  void clear()
  {
    sent_ = false;
  }

private:
  bool sent_ = false;
};

UsbKeyboardOutput output;

static bool sendAThroughBridge()
{
  output.clear();
  const uint32_t startMs = millis();
  while (millis() - startMs < 1000)
  {
    input.apply(esp32keybridge::keyEvent(esp32keybridge::KeySymbol::A, true, millis()));
    bridge.update();
    input.apply(esp32keybridge::keyEvent(esp32keybridge::KeySymbol::A, false, millis()));
    bridge.update();
    if (output.sent())
    {
      return true;
    }
    delay(5);
  }
  return false;
}

void setup()
{
  Serial.begin(115200);
  delay(1500);

  keyboard.setLayout(ESP_USB_DEVICE_KEYBOARD_LAYOUT_EN_US);

  bridge.addInput(input);
  bridge.addOutput(output);
  bridge.begin();

  EspUsbDeviceConfig config;
  config.vid = 0x303a;
  config.pid = 0x4002;
  config.manufacturer = "ESP32KeyBridge";
  config.product = "ESP32KeyBridge Output Keyboard";
  config.serialNumber = "esp32keybridge-output-kbd";

  Serial.printf("DEVICE_BEGIN %u\n", device.begin(config) ? 1 : 0);
}

void loop()
{
  while (Serial.available() > 0)
  {
    const char command = static_cast<char>(Serial.read());
    if (command == 'a')
    {
      Serial.printf("BRIDGE_SEND_A %u\n", sendAThroughBridge() ? 1 : 0);
    }
  }
  delay(1);
}
