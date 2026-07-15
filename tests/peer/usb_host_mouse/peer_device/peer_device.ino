// Peer smoke test, device board: acts as the USB mouse the host board's
// bridge listens to. Serial commands drive boot mouse reports; movement
// commands preserve the currently held buttons so button and motion tests
// stay independent.
//
// Commands: p=ping (reports the mount state), x=move X +10, y=move Y +7,
// w=wheel +3, l/L=left button down/up, r/R=right button down/up.

#include <EspUsbDevice.h>

EspUsbDevice device;
EspUsbDeviceHidMouse mouse(device);

// Retry the send for up to a second: right after enumeration the endpoint
// can briefly refuse a report even though the device is mounted. The
// callbacks are non-capturing lambdas, so they decay to a plain function
// pointer (a template would trip the .ino auto-prototype generator).
static void sendRetry(bool (*send)(), const char *label)
{
  const uint32_t start = millis();
  while (millis() - start < 1000)
  {
    if (send())
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
  config.product = "KeyBridge Peer Mouse";
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
    case 'x':
      sendRetry([] { return mouse.move(10, 0, 0, mouse.buttons()); }, "MOVE_X");
      break;
    case 'y':
      sendRetry([] { return mouse.move(0, 7, 0, mouse.buttons()); }, "MOVE_Y");
      break;
    case 'w':
      sendRetry([] { return mouse.move(0, 0, 3, mouse.buttons()); }, "WHEEL_3");
      break;
    case 'l':
      sendRetry([] { return mouse.press(ESP_USB_DEVICE_MOUSE_LEFT); }, "LBTN_DOWN");
      break;
    case 'L':
      sendRetry([] { return mouse.release(ESP_USB_DEVICE_MOUSE_LEFT); }, "LBTN_UP");
      break;
    case 'r':
      sendRetry([] { return mouse.press(ESP_USB_DEVICE_MOUSE_RIGHT); }, "RBTN_DOWN");
      break;
    case 'R':
      sendRetry([] { return mouse.release(ESP_USB_DEVICE_MOUSE_RIGHT); }, "RBTN_UP");
      break;
    default:
      break;
    }
  }
  delay(1);
}
