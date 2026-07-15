// Peer smoke test, device board: acts as a consumer control device (media
// remote) for the host board's bridge.
//
// Commands: p=ping (reports the mount state), v/V=VolumeUp press/release.

#include <EspUsbDevice.h>

EspUsbDevice device;
EspUsbDeviceHidConsumerControl consumer(device);

static void sendConsumer(uint16_t usage, const char *label)
{
  const uint32_t start = millis();
  while (millis() - start < 1000)
  {
    if (consumer.sendUsage(usage, 0))
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
  config.product = "KeyBridge Peer Consumer";
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
    case 'v':
      sendConsumer(0x00e9, "VOLUP_DOWN"); // Volume Increment
      break;
    case 'V':
      sendConsumer(0x0000, "VOLUP_UP");
      break;
    default:
      break;
    }
  }
  delay(1);
}
