// Peer smoke test, device board: the bridge output pipeline behind the
// composite USB Device adapter. A ManualInputAdapter is driven by serial
// commands; the host board observes the resulting reports and sets the
// keyboard LEDs to test the lock authority path.
//
// Commands: p=ping (reports the mount state), a/r=key A down/up,
// s/S=LeftShift down/up, v/V=consumer VolumeUp down/up, m/M=mouse left
// button down/up, w=wheel +3, x=mouse X +10, y=mouse Y +7, 6/R=keys
// A..F down/up (6-key rollover), z/Z=press/release keyboard A + consumer
// VolumeUp + mouse left + X move in one frame (composite output),
// q=print the lock state seen by the input.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbDevice.h>

EspUsbDevice usbDevice;

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::ManualInputAdapter input;
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  Serial.begin(115200);
  delay(5000); // let the host board finish booting before enumeration

  EspUsbDeviceConfig config;
  config.product = "KeyBridge Peer Bridge";
  Serial.printf("DEVICE_BEGIN %u\n", usbDevice.begin(config) ? 1 : 0);

  bridge.addInput(input);
  bridge.addOutput(pc);
  esp32keybridge::ESP32KeyBridgeConfig bridgeConfig;
  bridge.applyConfig(bridgeConfig);
}

void loop()
{
  while (Serial.available() > 0)
  {
    switch (Serial.read())
    {
    case 'p':
      Serial.printf("PONG ready=%u\n", usbDevice.ready() ? 1 : 0);
      break;
    case 'a':
      input.press(esp32keybridge::KeyboardUsage::A);
      Serial.println("CMD A_DOWN");
      break;
    case 'r':
      input.release(esp32keybridge::KeyboardUsage::A);
      Serial.println("CMD A_UP");
      break;
    case 's':
      input.press(esp32keybridge::KeyboardUsage::LeftShift);
      Serial.println("CMD SHIFT_DOWN");
      break;
    case 'S':
      input.release(esp32keybridge::KeyboardUsage::LeftShift);
      Serial.println("CMD SHIFT_UP");
      break;
    case 'v':
      input.press(esp32keybridge::ConsumerUsage::VolumeIncrement);
      Serial.println("CMD VOLUP_DOWN");
      break;
    case 'V':
      input.release(esp32keybridge::ConsumerUsage::VolumeIncrement);
      Serial.println("CMD VOLUP_UP");
      break;
    case 'm':
      input.press(esp32keybridge::MouseUsage::Left);
      Serial.println("CMD LBUTTON_DOWN");
      break;
    case 'M':
      input.release(esp32keybridge::MouseUsage::Left);
      Serial.println("CMD LBUTTON_UP");
      break;
    case 'w':
      input.addAxisDelta(esp32keybridge::Axis::Wheel, 3);
      Serial.println("CMD WHEEL_3");
      break;
    case 'x':
      input.addAxisDelta(esp32keybridge::Axis::X, 10);
      Serial.println("CMD MOVE_X");
      break;
    case 'y':
      input.addAxisDelta(esp32keybridge::Axis::Y, 7);
      Serial.println("CMD MOVE_Y");
      break;
    case '6':
      input.press(esp32keybridge::KeyboardUsage::A);
      input.press(esp32keybridge::KeyboardUsage::B);
      input.press(esp32keybridge::KeyboardUsage::C);
      input.press(esp32keybridge::KeyboardUsage::D);
      input.press(esp32keybridge::KeyboardUsage::E);
      input.press(esp32keybridge::KeyboardUsage::F);
      Serial.println("CMD SIX_DOWN");
      break;
    case 'R':
      input.release(esp32keybridge::KeyboardUsage::A);
      input.release(esp32keybridge::KeyboardUsage::B);
      input.release(esp32keybridge::KeyboardUsage::C);
      input.release(esp32keybridge::KeyboardUsage::D);
      input.release(esp32keybridge::KeyboardUsage::E);
      input.release(esp32keybridge::KeyboardUsage::F);
      Serial.println("CMD SIX_UP");
      break;
    case 'z':
      // All three HID classes change in one bridge frame: the composite
      // device emits keyboard, consumer, and mouse reports (in that order).
      input.press(esp32keybridge::KeyboardUsage::A);
      input.press(esp32keybridge::ConsumerUsage::VolumeIncrement);
      input.press(esp32keybridge::MouseUsage::Left);
      input.addAxisDelta(esp32keybridge::Axis::X, 5);
      Serial.println("CMD COMBO_DOWN");
      break;
    case 'Z':
      input.release(esp32keybridge::KeyboardUsage::A);
      input.release(esp32keybridge::ConsumerUsage::VolumeIncrement);
      input.release(esp32keybridge::MouseUsage::Left);
      Serial.println("CMD COMBO_UP");
      break;
    case 'q':
    {
      const esp32keybridge::LockState &lock = input.lockState();
      Serial.printf("LOCK num=%u caps=%u scroll=%u kana=%u\n", lock.numLock ? 1 : 0,
                    lock.capsLock ? 1 : 0, lock.scrollLock ? 1 : 0, lock.kana ? 1 : 0);
      break;
    }
    default:
      break;
    }
  }
  bridge.update();
  delay(1);
}
