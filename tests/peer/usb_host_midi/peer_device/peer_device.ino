// Peer smoke test, device board: acts as the USB MIDI device the host
// board's bridge listens to. Serial commands send MIDI note on/off messages
// on channel 0.
//
// Commands: p=ping (reports the mount state), a=note 60 on, A=note 60 off,
// b=note 62 on, B=note 62 off.

#include <EspUsbDevice.h>

EspUsbDevice device;
EspUsbDeviceMidi midi(device);

static void report(bool ok, const char *label)
{
  Serial.printf(ok ? "SEND %s\n" : "SEND_FAILED %s\n", label);
}

void setup()
{
  Serial.begin(115200);
  delay(5000); // let the host board finish booting before enumeration

  EspUsbDeviceConfig config;
  config.product = "KeyBridge Peer MIDI";
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
      report(midi.noteOn(0, 60, 100), "NOTE60_ON");
      break;
    case 'A':
      report(midi.noteOff(0, 60, 0), "NOTE60_OFF");
      break;
    case 'b':
      report(midi.noteOn(0, 62, 100), "NOTE62_ON");
      break;
    case 'B':
      report(midi.noteOff(0, 62, 0), "NOTE62_OFF");
      break;
    default:
      break;
    }
  }
  delay(1);
}
