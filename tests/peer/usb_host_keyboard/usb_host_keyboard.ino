// Peer smoke test, host board: the USB Host keyboard input adapter feeds
// the bridge input pipeline (remap included) and forwards the lock state
// back to the keyboard LEDs. The peer device board acts as the USB
// keyboard (peer_device/peer_device.ino).
//
// Prints "OUT <kind>:<code>..." (sorted) whenever the set of keys reaching
// the output adapter changes, so the test can assert the pipeline result.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <EspUsbHost.h>

EspUsbHost usbHost;

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostKeyboardInputAdapter keyboard(usbHost);
esp32keybridge::ManualOutputAdapter pc;

static esp32keybridge::KeySet lastKeys;

static bool sameKeys(const esp32keybridge::KeySet &a, const esp32keybridge::KeySet &b)
{
  if (a.count() != b.count())
  {
    return false;
  }
  for (size_t i = 0; i < a.count(); ++i)
  {
    if (!b.contains(a.at(i)))
    {
      return false;
    }
  }
  return true;
}

static void printKeys(const esp32keybridge::KeySet &keys)
{
  esp32keybridge::Key sorted[esp32keybridge::KeySet::MaxKeys];
  const size_t count = keys.count();
  for (size_t i = 0; i < count; ++i)
  {
    sorted[i] = keys.at(i);
  }
  for (size_t i = 1; i < count; ++i)
  {
    const esp32keybridge::Key key = sorted[i];
    size_t j = i;
    while (j > 0 && (static_cast<uint8_t>(sorted[j - 1].kind) > static_cast<uint8_t>(key.kind) ||
                     (sorted[j - 1].kind == key.kind && sorted[j - 1].code > key.code)))
    {
      sorted[j] = sorted[j - 1];
      --j;
    }
    sorted[j] = key;
  }

  Serial.print("OUT");
  if (count == 0)
  {
    Serial.print(" empty");
  }
  for (size_t i = 0; i < count; ++i)
  {
    Serial.printf(" %s:%04x", esp32keybridge::keyKindName(sorted[i].kind), sorted[i].code);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  usbHost.onDeviceConnected(
      [](const EspUsbHostDeviceInfo &info)
      { Serial.printf("HOST_CONNECTED vid=%04x pid=%04x\n", info.vid, info.pid); });
  usbHost.begin();

  bridge.addInput(keyboard);
  bridge.addOutput(pc);

  // A remap proves the pipeline runs (A arrives as B on the output side).
  esp32keybridge::ESP32KeyBridgeConfig config;
  config.global.remap(esp32keybridge::KeyboardUsage::A, esp32keybridge::KeyboardUsage::B);
  bridge.applyConfig(config);
  Serial.println("BRIDGE_READY");
}

void loop()
{
  bridge.update();
  if (!sameKeys(pc.keys(), lastKeys))
  {
    lastKeys = pc.keys();
    printKeys(lastKeys);
  }
  delay(1);
}
