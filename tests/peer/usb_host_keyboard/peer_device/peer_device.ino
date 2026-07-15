// Peer smoke test, device board: acts as the USB keyboard the host
// board's bridge listens to. Keyboard ONLY on purpose: a single HID class
// keeps the boot-declared interface, which EspUsbHost's setKeyboardLeds
// requires (its composite/report-ID limitation is reported upstream).
// Serial commands drive raw HID reports; LED output reports from the
// host are printed.
//
// Commands: p=ping (reports the mount state), a=press A, r=release all
// keys, c/C=LeftCtrl down/up, l=CapsLock tap.

#include <EspUsbDevice.h>

EspUsbDevice device;
EspUsbDeviceHidKeyboard keyboard(device);

static EspUsbDeviceBootKeyboardReport report;

static void sendKeyboard(const char *label)
{
  const uint32_t start = millis();
  while (millis() - start < 1000)
  {
    if (keyboard.sendReport(report, 0))
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

  keyboard.onOutputReport(
      [](const EspUsbDeviceHidKeyboardOutputReport &led)
      {
        Serial.printf("LED numlock=%u capslock=%u scrolllock=%u\n",
                      led.numLock ? 1 : 0, led.capsLock ? 1 : 0, led.scrollLock ? 1 : 0);
      });

  EspUsbDeviceConfig config;
  config.product = "KeyBridge Peer Keyboard";
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
      report.keys[0] = 0x04; // A
      sendKeyboard("A_DOWN");
      break;
    case 'r':
      report = EspUsbDeviceBootKeyboardReport{};
      sendKeyboard("ALL_UP");
      break;
    case 'c':
      report.modifiers |= 0x01; // LeftCtrl
      sendKeyboard("LCTRL_DOWN");
      break;
    case 'C':
      report.modifiers &= static_cast<uint8_t>(~0x01);
      sendKeyboard("LCTRL_UP");
      break;
    case 'l':
      report.keys[0] = 0x39; // CapsLock
      sendKeyboard("CAPS_DOWN");
      report.keys[0] = 0;
      sendKeyboard("CAPS_UP");
      break;
    default:
      break;
    }
  }
  delay(1);
}
