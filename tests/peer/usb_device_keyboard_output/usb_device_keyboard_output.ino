// Peer smoke test, host board: plain EspUsbHost observer. Prints what the
// bridge on the peer device board (peer_device/peer_device.ino) actually
// puts on the wire: keyboard state, consumer usages, and mouse reports.
// Serial commands drive the keyboard LEDs to test the lock authority path.
//
// Commands: n=NumLock LED on, 0=all LEDs off.

#include <EspUsbHost.h>

EspUsbHost usb;

void setup()
{
  Serial.begin(115200);
  delay(500);

  usb.onDeviceConnected(
      [](const EspUsbHostDeviceInfo &info)
      { Serial.printf("HOST_CONNECTED vid=%04x pid=%04x\n", info.vid, info.pid); });
  usb.onKeyboardState(
      [](const EspUsbHostKeyboardState &state)
      {
        Serial.printf("KEY_STATE modifiers=%02x a=%u b=%u\n", state.modifiers,
                      state.isDown(0x04) ? 1 : 0, state.isDown(0x05) ? 1 : 0);
      });
  usb.onConsumerControl(
      [](const EspUsbHostConsumerControlEvent &event)
      {
        Serial.printf("CONSUMER usage=%04x pressed=%u released=%u\n", event.usage,
                      event.pressed ? 1 : 0, event.released ? 1 : 0);
      });
  usb.onMouse(
      [](const EspUsbHostMouseEvent &event)
      {
        Serial.printf("MOUSE buttons=%02x x=%d y=%d wheel=%d\n", event.buttons,
                      static_cast<int>(event.x), static_cast<int>(event.y),
                      static_cast<int>(event.wheel));
      });
  usb.begin();
  Serial.println("HOST_READY");
}

void loop()
{
  while (Serial.available() > 0)
  {
    switch (Serial.read())
    {
    case 'n':
      Serial.printf("LED_TX %u\n", usb.setKeyboardLeds(true, false, false) ? 1 : 0);
      break;
    case '0':
      Serial.printf("LED_TX %u\n", usb.setKeyboardLeds(false, false, false) ? 1 : 0);
      break;
    default:
      break;
    }
  }
  delay(1);
}
