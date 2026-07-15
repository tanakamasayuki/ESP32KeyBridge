// Peer smoke test, device board: acts as the USB gamepad the host board's
// bridge listens to. Serial commands drive a boot gamepad report (buttons +
// hat), held as persistent state so presses and hat directions can be tested
// independently.
//
// Commands: p=ping (reports the mount state), a=press button 1, b=press
// button 2, A=release all buttons, u=hat up, c=hat center.

#include <EspUsbDevice.h>

EspUsbDevice device;
EspUsbDeviceHidGamepad gamepad(device);

static EspUsbDeviceGamepadReport report;

static void send(const char *label)
{
  const uint32_t start = millis();
  while (millis() - start < 1000)
  {
    if (gamepad.sendReport(report, 0))
    {
      Serial.printf("SEND %s\n", label);
      return;
    }
    delay(5);
  }
  Serial.printf("SEND_FAILED %s\n", label);
}

void setup()
{
  Serial.begin(115200);
  delay(5000); // let the host board finish booting before enumeration

  EspUsbDeviceConfig config;
  config.product = "KeyBridge Peer Gamepad";
  Serial.printf("DEVICE_BEGIN %u\n", device.begin(config) ? 1 : 0);
}

void loop()
{
  while (Serial.available() > 0)
  {
    switch (Serial.read())
    {
    case 'p':
      Serial.printf("PONG ready=%u\n", device.ready() ? 1 : 0);
      break;
    case 'a':
      report.buttons |= (1u << 0); // button 1
      send("BTN1_DOWN");
      break;
    case 'b':
      report.buttons |= (1u << 1); // button 2
      send("BTN2_DOWN");
      break;
    case 'A':
      report.buttons = 0;
      send("BTN_UP");
      break;
    case 'u':
      report.hat = ESP_USB_DEVICE_GAMEPAD_HAT_UP;
      send("HAT_UP");
      break;
    case 'c':
      report.hat = ESP_USB_DEVICE_GAMEPAD_HAT_CENTER;
      send("HAT_CENTER");
      break;
    default:
      break;
    }
  }
  delay(1);
}
