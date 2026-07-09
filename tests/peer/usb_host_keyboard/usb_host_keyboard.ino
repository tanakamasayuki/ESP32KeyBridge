#include <ESP32KeyBridge.h>
#include <EspUsbHost.h>

EspUsbHost usb;
esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EventInputAdapter input;
esp32keybridge::RecordingOutputAdapter output;
bool sawA = false;

void setup()
{
  Serial.begin(115200);
  delay(500);

  bridge.addInput(input);
  bridge.addOutput(output);
  bridge.begin();

  usb.onDeviceConnected([](const EspUsbHostDeviceInfo &device)
                        {
                          Serial.printf("HOST_CONNECTED vid=%04x pid=%04x\n", device.vid, device.pid);
                        });

  usb.onKeyboard([](const EspUsbHostKeyboardEvent &event)
                 {
                   const esp32keybridge::Key key = esp32keybridge::keyFromHidUsage(event.keycode);
                   if (key == esp32keybridge::Key::None)
                   {
                     return;
                   }

                   input.apply(esp32keybridge::keyEvent(key, event.pressed, millis()));
                   bridge.update();

                   if (key == esp32keybridge::Key::A && event.pressed &&
                       bridge.outputState().isPressed(esp32keybridge::Key::A) &&
                       output.state().isPressed(esp32keybridge::Key::A))
                   {
                     sawA = true;
                     Serial.println("KEYBRIDGE_KEY_A_PRESSED");
                   }
                   else if (key == esp32keybridge::Key::A && event.released && sawA &&
                            !bridge.outputState().isPressed(esp32keybridge::Key::A) &&
                            !output.state().isPressed(esp32keybridge::Key::A))
                   {
                     Serial.println("KEYBRIDGE_KEY_A_RELEASED");
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
