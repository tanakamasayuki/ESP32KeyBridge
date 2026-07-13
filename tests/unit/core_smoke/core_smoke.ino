#include <ESP32KeyBridge.h>
#include <cassert>

namespace
{
int g_total = 0;

static void test_key_identity_is_kind_plus_code()
{
  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);
  assert(a.kind == esp32keybridge::KeyKind::Keyboard);
  assert(a.code == 0x04);

  // Same numeric code in different kinds must be different keys.
  esp32keybridge::Key keyboard30 = esp32keybridge::keyboardKey(0x30);
  esp32keybridge::Key consumer30 = esp32keybridge::consumerKey(0x30);
  assert(keyboard30 != consumer30);
  assert(keyboard30 == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::RightBracket));

  assert(esp32keybridge::isValid(a));
  assert(esp32keybridge::isValid(esp32keybridge::mouseButtonKey(1)));
  assert(esp32keybridge::isValid(esp32keybridge::virtualKey(1)));
  assert(!esp32keybridge::isValid(esp32keybridge::Key()));
  assert(!esp32keybridge::isValid(esp32keybridge::keyboardKey(static_cast<uint16_t>(0))));
}

static void test_key_kind_names()
{
  assert(esp32keybridge::keyKindName(esp32keybridge::KeyKind::Keyboard)[0] == 'k');
  assert(esp32keybridge::keyKindName(esp32keybridge::KeyKind::Consumer)[0] == 'c');
  assert(esp32keybridge::keyKindName(esp32keybridge::KeyKind::MouseButton)[0] == 'm');
  assert(esp32keybridge::keyKindName(esp32keybridge::KeyKind::Virtual)[0] == 'v');
}

static void test_key_set_press_release()
{
  esp32keybridge::KeySet keys;
  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);

  assert(keys.count() == 0);
  assert(keys.press(a));
  assert(keys.contains(a));
  assert(keys.count() == 1);

  // Pressing again does not duplicate.
  assert(keys.press(a));
  assert(keys.count() == 1);

  assert(keys.release(a));
  assert(!keys.contains(a));
  assert(keys.count() == 0);
  assert(!keys.release(a));

  // Invalid keys are rejected.
  assert(!keys.press(esp32keybridge::Key()));
}

static void test_key_set_holds_mixed_kinds()
{
  esp32keybridge::KeySet keys;
  assert(keys.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A)));
  assert(keys.press(esp32keybridge::consumerKey(esp32keybridge::ConsumerUsage::VolumeIncrement)));
  assert(keys.press(esp32keybridge::mouseButtonKey(1)));
  assert(keys.press(esp32keybridge::virtualKey(1)));
  assert(keys.count() == 4);
  assert(keys.contains(esp32keybridge::consumerKey(0x00e9)));
}

static void test_key_set_capacity()
{
  esp32keybridge::KeySet keys;
  for (uint16_t i = 0; i < esp32keybridge::KeySet::MaxKeys; ++i)
  {
    assert(keys.press(esp32keybridge::virtualKey(static_cast<uint16_t>(i + 1))));
  }
  assert(keys.count() == esp32keybridge::KeySet::MaxKeys);
  assert(!keys.press(esp32keybridge::virtualKey(1000)));

  // Already-pressed keys still report success when full.
  assert(keys.press(esp32keybridge::virtualKey(1)));
}

static void test_modifier_normalization()
{
  esp32keybridge::Key leftCtrl = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftCtrl);
  esp32keybridge::Key rightShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::RightShift);
  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);

  assert(esp32keybridge::isKeyboardModifier(leftCtrl));
  assert(!esp32keybridge::isKeyboardModifier(a));
  assert(!esp32keybridge::isKeyboardModifier(esp32keybridge::consumerKey(0x00e0)));

  assert(esp32keybridge::keyboardModifierMask(leftCtrl) == 0x01);
  assert(esp32keybridge::keyboardModifierMask(rightShift) == 0x20);
  assert(esp32keybridge::keyboardModifierMask(a) == 0);

  assert(esp32keybridge::keyboardModifierFromBitIndex(0) == leftCtrl);
  assert(esp32keybridge::keyboardModifierFromBitIndex(5) == rightShift);
  assert(!esp32keybridge::isValid(esp32keybridge::keyboardModifierFromBitIndex(8)));

  // Modifiers are ordinary keys in the set; the mask is a derived view.
  esp32keybridge::KeySet keys;
  assert(keys.press(leftCtrl));
  assert(keys.press(rightShift));
  assert(keys.press(a));
  assert(keys.keyboardModifierMask() == 0x21);

  // Report mask -> keys normalization for input adapters.
  esp32keybridge::KeySet fromMask;
  assert(fromMask.pressKeyboardModifiers(0x21));
  assert(fromMask.count() == 2);
  assert(fromMask.contains(leftCtrl));
  assert(fromMask.contains(rightShift));
}

static void test_bridge_merges_inputs_as_union()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter left;
  esp32keybridge::ManualInputAdapter right;

  assert(bridge.addInput(left));
  assert(bridge.addInput(right));
  assert(bridge.inputCount() == 2);
  bridge.begin();

  esp32keybridge::Key shift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);
  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);

  assert(left.press(shift));
  assert(right.press(a));
  bridge.update();

  assert(bridge.mergedKeys().count() == 2);
  assert(bridge.mergedKeys().contains(shift));
  assert(bridge.mergedKeys().contains(a));
  assert(bridge.mergedKeys().keyboardModifierMask() == 0x02);
  assert(!bridge.mergedOverflow());
}

static void test_bridge_release_requires_all_inputs()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter left;
  esp32keybridge::ManualInputAdapter right;
  assert(bridge.addInput(left));
  assert(bridge.addInput(right));

  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);
  assert(left.press(a));
  assert(right.press(a));
  bridge.update();
  assert(bridge.mergedKeys().contains(a));

  // One input releases, the other still holds: the key stays pressed.
  assert(left.release(a));
  bridge.update();
  assert(bridge.mergedKeys().contains(a));

  assert(right.release(a));
  bridge.update();
  assert(!bridge.mergedKeys().contains(a));
  assert(bridge.mergedKeys().count() == 0);
}

static void test_bridge_disconnected_input_releases_keys()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key shift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);
  assert(keyboard.press(shift));
  bridge.update();
  assert(bridge.mergedKeys().contains(shift));

  // Disconnect while the key is held: no stuck key in the merged state.
  keyboard.setConnected(false);
  bridge.update();
  assert(!bridge.mergedKeys().contains(shift));
  assert(bridge.mergedKeys().count() == 0);

  // Reconnect restores the adapter's held keys.
  keyboard.setConnected(true);
  bridge.update();
  assert(bridge.mergedKeys().contains(shift));
}

static void test_bridge_merge_overflow_is_reported()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter left;
  esp32keybridge::ManualInputAdapter right;
  assert(bridge.addInput(left));
  assert(bridge.addInput(right));

  for (uint16_t i = 0; i < esp32keybridge::KeySet::MaxKeys; ++i)
  {
    assert(left.press(esp32keybridge::virtualKey(static_cast<uint16_t>(i + 1))));
  }
  assert(right.press(esp32keybridge::virtualKey(1000)));

  bridge.update();
  assert(bridge.mergedOverflow());
  assert(bridge.mergedKeys().count() == esp32keybridge::KeySet::MaxKeys);
}

static void test_bridge_can_clear_inputs()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));
  assert(keyboard.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A)));
  bridge.update();
  assert(bridge.mergedKeys().count() == 1);

  bridge.clearInputs();
  assert(bridge.inputCount() == 0);
  assert(bridge.mergedKeys().count() == 0);
}

static void run(const char *name, void (*test)())
{
  Serial.print("RUN ");
  Serial.println(name);
  ++g_total;
  test();
}

} // namespace

void setup()
{
  Serial.begin(115200);
  run("key_identity_is_kind_plus_code", test_key_identity_is_kind_plus_code);
  run("key_kind_names", test_key_kind_names);
  run("key_set_press_release", test_key_set_press_release);
  run("key_set_holds_mixed_kinds", test_key_set_holds_mixed_kinds);
  run("key_set_capacity", test_key_set_capacity);
  run("modifier_normalization", test_modifier_normalization);
  run("bridge_merges_inputs_as_union", test_bridge_merges_inputs_as_union);
  run("bridge_release_requires_all_inputs", test_bridge_release_requires_all_inputs);
  run("bridge_disconnected_input_releases_keys", test_bridge_disconnected_input_releases_keys);
  run("bridge_merge_overflow_is_reported", test_bridge_merge_overflow_is_reported);
  run("bridge_can_clear_inputs", test_bridge_can_clear_inputs);
  Serial.print("TEST done ");
  Serial.print(g_total);
  Serial.print("/");
  Serial.println(g_total);
}

void loop() {}
