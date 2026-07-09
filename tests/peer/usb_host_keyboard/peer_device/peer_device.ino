#include <EspUsbDevice.h>

EspUsbDevice device;
EspUsbDeviceHidKeyboard keyboard(device);

static bool tapWithTimeout(char key)
{
  const uint32_t startMs = millis();
  while (millis() - startMs < 1000)
  {
    if (keyboard.tapKey(key))
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

  EspUsbDeviceConfig config;
  config.vid = 0x303a;
  config.pid = 0x4001;
  config.manufacturer = "EspUsb";
  config.product = "ESP32KeyBridge Peer Keyboard";
  config.serialNumber = "esp32keybridge-peer-kbd";

  Serial.printf("DEVICE_BEGIN %u\n", device.begin(config) ? 1 : 0);
}

void loop()
{
  while (Serial.available() > 0)
  {
    const char command = static_cast<char>(Serial.read());
    if (command == 'a')
    {
      Serial.printf("DEVICE_SEND_A %u\n", tapWithTimeout('a') ? 1 : 0);
    }
  }
  delay(1);
}
