// Peer smoke test, host board: the USB Host gamepad input adapter feeds the
// bridge input pipeline. Buttons and the hat are mapped to keyboard keys so
// the result shows up as keys in the output set. The peer device board acts
// as the USB gamepad (peer_device/peer_device.ino).
//
// Prints "OUT <kind>:<code>..." (sorted) whenever the set of keys reaching
// the output adapter changes.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <EspUsbHost.h>

EspUsbHost usbHost;

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostGamepadInputAdapter gamepad(usbHost);
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

  // Buttons 1/2 and the four hat directions map to distinct keyboard keys so
  // the test can read them back as OUT lines.
  gamepad.mapButton(1, esp32keybridge::KeyboardUsage::A); // 0x04
  gamepad.mapButton(2, esp32keybridge::KeyboardUsage::B); // 0x05
  gamepad.mapHat(esp32keybridge::KeyboardUsage::Up, esp32keybridge::KeyboardUsage::Down,
                 esp32keybridge::KeyboardUsage::Left, esp32keybridge::KeyboardUsage::Right);
  bridge.addInput(gamepad);
  bridge.addOutput(pc);
  esp32keybridge::ESP32KeyBridgeConfig config;
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
