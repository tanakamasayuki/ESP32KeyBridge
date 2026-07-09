#include <EspUsbHost.h>

EspUsbHost usb;
bool sawA = false;

void setup()
{
  Serial.begin(115200);
  delay(500);

  usb.onDeviceConnected([](const EspUsbHostDeviceInfo &device)
                        {
                          Serial.printf("HOST_CONNECTED vid=%04x pid=%04x\n", device.vid, device.pid);
                        });

  usb.onKeyboard([](const EspUsbHostKeyboardEvent &event)
                 {
                   if (event.keycode == 0x04 && event.pressed)
                   {
                     sawA = true;
                     Serial.println("HOST_KEY_A_PRESSED");
                   }
                   else if (event.keycode == 0x04 && event.released && sawA)
                   {
                     Serial.println("HOST_KEY_A_RELEASED");
                   }
                 });

  if (!usb.begin())
  {
    Serial.printf("HOST_BEGIN_FAILED %s\n", usb.lastErrorName());
  }
}

void loop()
{
  delay(1);
}
