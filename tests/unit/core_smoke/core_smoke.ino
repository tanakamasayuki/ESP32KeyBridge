#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeSerial.h>
#include <cassert>
#include <cstring>

namespace
{
int g_total = 0;

// A Print sink that captures everything written into a fixed buffer, so the
// serial output adapters can be checked byte-for-byte on the host without a
// real port. Only write(uint8_t) is virtual in Print; print()/println() build
// on it.
class CaptureStream : public Print
{
public:
  size_t write(uint8_t value) override
  {
    if (length_ + 1 < Capacity)
    {
      buffer_[length_++] = static_cast<char>(value);
      buffer_[length_] = '\0';
    }
    return 1;
  }
  using Print::write;

  void reset()
  {
    length_ = 0;
    buffer_[0] = '\0';
  }
  const char *c_str() const { return buffer_; }
  size_t length() const { return length_; }
  uint8_t byteAt(size_t i) const { return static_cast<uint8_t>(buffer_[i]); }
  bool has(const char *needle) const { return std::strstr(buffer_, needle) != nullptr; }

private:
  static constexpr size_t Capacity = 256;
  char buffer_[Capacity] = {};
  size_t length_ = 0;
};

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

static void test_key_converts_implicitly_from_usage_enums()
{
  // The usage enums carry their kind, so they convert to Key directly and
  // can be passed to any Key parameter without keyboardKey()/consumerKey().
  esp32keybridge::Key caps = esp32keybridge::KeyboardUsage::CapsLock;
  assert(caps == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock));

  esp32keybridge::Key volumeUp = esp32keybridge::ConsumerUsage::VolumeIncrement;
  assert(volumeUp == esp32keybridge::consumerKey(esp32keybridge::ConsumerUsage::VolumeIncrement));

  esp32keybridge::Key leftButton = esp32keybridge::MouseUsage::Left;
  assert(leftButton == esp32keybridge::mouseButtonKey(1));
  assert(leftButton == esp32keybridge::mouseButtonKey(esp32keybridge::MouseUsage::Left));

  esp32keybridge::Key layerKey = esp32keybridge::VirtualUsage::V1;
  assert(layerKey == esp32keybridge::virtualKey(1));

  esp32keybridge::TransformConfig transform;
  assert(transform.remap(esp32keybridge::KeyboardUsage::F13,
                         esp32keybridge::ConsumerUsage::VolumeIncrement));
  assert(transform.map(esp32keybridge::KeyboardUsage::F13) == volumeUp);
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

static void test_global_remap_swaps_without_chaining()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key capsLock = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock);
  esp32keybridge::Key leftCtrl = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftCtrl);

  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.global.remap(capsLock, leftCtrl));
  assert(config.global.remap(leftCtrl, capsLock));

  esp32keybridge::ESP32KeyBridgeConfigError error;
  assert(bridge.validateConfig(config, error));
  bridge.applyConfig(config);

  assert(keyboard.press(capsLock));
  assert(keyboard.press(leftCtrl));
  bridge.update();

  // Single-step lookup: both keys swap, neither chains back.
  assert(bridge.outputKeys().contains(leftCtrl));
  assert(bridge.outputKeys().contains(capsLock));
  assert(bridge.outputKeys().count() == 2);
  assert(bridge.mergedKeys().contains(capsLock));

  assert(keyboard.release(capsLock));
  bridge.update();
  assert(bridge.outputKeys().contains(capsLock));
  assert(!bridge.outputKeys().contains(leftCtrl));
}

static void test_remap_crosses_key_kinds()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.global.remap(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::F13),
                             esp32keybridge::consumerKey(esp32keybridge::ConsumerUsage::VolumeIncrement)));
  bridge.applyConfig(config);

  assert(keyboard.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::F13)));
  bridge.update();
  assert(bridge.outputKeys().contains(esp32keybridge::consumerKey(0x00e9)));
  assert(bridge.outputKeys().count() == 1);
}

static void test_global_disable_drops_key()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key insert = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Insert);
  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.global.disable(insert));
  bridge.applyConfig(config);

  assert(keyboard.press(insert));
  assert(keyboard.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A)));
  bridge.update();
  assert(!bridge.outputKeys().contains(insert));
  assert(bridge.outputKeys().count() == 1);
  assert(bridge.mergedKeys().contains(insert));

  assert(keyboard.release(insert));
  bridge.update();
  assert(bridge.outputKeys().count() == 1);
}

static void test_per_input_remap_applies_to_one_input()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ManualInputAdapter scanner;

  esp32keybridge::Key enter = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Enter);
  esp32keybridge::Key tab = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Tab);

  // Per-input settings bind by handle: only the scanner gets the remap.
  esp32keybridge::ESP32KeyBridgeConfig config;
  esp32keybridge::InputConfig &scannerConfig = config.addInputConfig();
  assert(scannerConfig.remap(enter, tab));
  assert(bridge.addInput(keyboard));
  assert(bridge.addInput(scanner, scannerConfig));
  bridge.applyConfig(config);

  assert(keyboard.press(enter));
  bridge.update();
  assert(bridge.outputKeys().contains(enter));
  assert(!bridge.outputKeys().contains(tab));

  assert(keyboard.release(enter));
  assert(scanner.press(enter));
  bridge.update();
  assert(bridge.outputKeys().contains(tab));
  assert(!bridge.outputKeys().contains(enter));
}

static void test_input_config_shared_between_inputs()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter left;
  esp32keybridge::ManualInputAdapter right;

  esp32keybridge::Key enter = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Enter);
  esp32keybridge::Key tab = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Tab);

  // Binding the same InputConfig to two inputs shares the settings.
  esp32keybridge::ESP32KeyBridgeConfig config;
  esp32keybridge::InputConfig &shared = config.addInputConfig();
  assert(shared.remap(enter, tab));
  assert(bridge.addInput(left, shared));
  assert(bridge.addInput(right, shared));
  bridge.applyConfig(config);

  assert(left.press(enter));
  bridge.update();
  assert(bridge.outputKeys().contains(tab));
  assert(left.release(enter));
  bridge.update();

  assert(right.press(enter));
  bridge.update();
  assert(bridge.outputKeys().contains(tab));
  assert(!bridge.outputKeys().contains(enter));
}

static void test_momentary_layer_with_press_time_resolution()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key fn1 = esp32keybridge::virtualKey(1);
  esp32keybridge::Key h = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::H);
  esp32keybridge::Key left = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Left);
  esp32keybridge::Key capsLock = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock);

  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.global.remap(capsLock, fn1)); // donor key -> virtual trigger
  config.layer(0).setTrigger(fn1);
  assert(config.layer(0).remap(h, left));
  bridge.applyConfig(config);

  // Without the layer, H is H.
  assert(keyboard.press(h));
  bridge.update();
  assert(bridge.outputKeys().contains(h));
  assert(keyboard.release(h));
  bridge.update();

  // Trigger held: new press resolves through the layer; trigger is consumed.
  assert(keyboard.press(capsLock));
  bridge.update();
  assert(bridge.outputKeys().count() == 0);

  assert(keyboard.press(h));
  bridge.update();
  assert(bridge.outputKeys().contains(left));
  assert(!bridge.outputKeys().contains(h));

  // Press-time resolution: releasing the trigger keeps the held key as Left.
  assert(keyboard.release(capsLock));
  bridge.update();
  assert(bridge.outputKeys().contains(left));
  assert(!bridge.outputKeys().contains(h));

  // Release uses the press-time value: no stuck key.
  assert(keyboard.release(h));
  bridge.update();
  assert(bridge.outputKeys().count() == 0);

  // After the layer, H resolves to H again.
  assert(keyboard.press(h));
  bridge.update();
  assert(bridge.outputKeys().contains(h));
}

static void test_layer_trigger_and_key_in_same_update()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key fn1 = esp32keybridge::virtualKey(1);
  esp32keybridge::Key h = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::H);
  esp32keybridge::Key left = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Left);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.layer(0).setTrigger(fn1);
  assert(config.layer(0).remap(h, left));
  bridge.applyConfig(config);

  // Trigger and target arrive in the same update: trigger-first semantics.
  assert(keyboard.press(fn1));
  assert(keyboard.press(h));
  bridge.update();
  assert(bridge.outputKeys().contains(left));
  assert(bridge.outputKeys().count() == 1);

  // Trigger release and a new press in the same update: layer is off first.
  assert(keyboard.release(fn1));
  assert(keyboard.release(h));
  bridge.update();
  assert(keyboard.press(fn1));
  bridge.update();
  assert(keyboard.release(fn1));
  assert(keyboard.press(h));
  bridge.update();
  assert(bridge.outputKeys().contains(h));
  assert(!bridge.outputKeys().contains(left));
}

static void test_add_layer_by_trigger()
{
  esp32keybridge::ESP32KeyBridgeConfig config;

  esp32keybridge::LayerConfig &nav = config.addLayer(esp32keybridge::VirtualUsage::V1);
  assert(nav.remap(esp32keybridge::KeyboardUsage::J, esp32keybridge::KeyboardUsage::Down));
  assert(config.layer(0).trigger() == esp32keybridge::virtualKey(1));
  assert(config.layer(0).hasMapping(esp32keybridge::KeyboardUsage::J));

  // Slots fill in registration order; when all MaxLayers are in use,
  // addLayer returns a writable dummy that is never applied.
  config.addLayer(esp32keybridge::VirtualUsage::V2);
  config.addLayer(esp32keybridge::VirtualUsage::V3);
  config.addLayer(esp32keybridge::VirtualUsage::V4);
  esp32keybridge::LayerConfig &overflow = config.addLayer(esp32keybridge::VirtualUsage::V5);
  assert(overflow.remap(esp32keybridge::KeyboardUsage::A, esp32keybridge::KeyboardUsage::B));
  for (size_t i = 0; i < esp32keybridge::ESP32KeyBridgeConfig::MaxLayers; ++i)
  {
    assert(config.layer(i).trigger() != esp32keybridge::virtualKey(5));
  }

  // The added layer works end to end through the bridge.
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter input;
  bridge.addInput(input);
  bridge.applyConfig(config);

  input.press(esp32keybridge::VirtualUsage::V1);
  input.press(esp32keybridge::KeyboardUsage::J);
  bridge.update();
  assert(bridge.outputKeys().contains(esp32keybridge::KeyboardUsage::Down));
  assert(!bridge.outputKeys().contains(esp32keybridge::KeyboardUsage::J));
  assert(!bridge.outputKeys().contains(esp32keybridge::VirtualUsage::V1));
}

static void test_layer_works_across_inputs()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter pedal;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(pedal));
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key fn1 = esp32keybridge::virtualKey(1);
  esp32keybridge::Key h = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::H);
  esp32keybridge::Key left = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Left);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.layer(0).setTrigger(fn1);
  assert(config.layer(0).remap(h, left));
  bridge.applyConfig(config);

  // Layer state is managed at the merged level: a pedal trigger affects the
  // keyboard's keys.
  assert(pedal.press(fn1));
  bridge.update();
  assert(keyboard.press(h));
  bridge.update();
  assert(bridge.outputKeys().contains(left));
}

static void test_apply_config_keeps_held_resolution()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);
  esp32keybridge::Key b = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::B);

  assert(keyboard.press(a));
  bridge.update();
  assert(bridge.outputKeys().contains(a));

  // A new remap applied while A is held does not re-resolve the held press.
  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.global.remap(a, b));
  bridge.applyConfig(config);
  bridge.update();
  assert(bridge.outputKeys().contains(a));
  assert(!bridge.outputKeys().contains(b));

  // Re-pressing resolves with the new configuration.
  assert(keyboard.release(a));
  bridge.update();
  assert(keyboard.press(a));
  bridge.update();
  assert(bridge.outputKeys().contains(b));
  assert(!bridge.outputKeys().contains(a));
}

static void test_unconsumed_virtual_key_stays_in_output()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  // No layer uses virtual 9: it passes through to outputKeys(); output
  // adapters are responsible for dropping what they cannot represent.
  assert(keyboard.press(esp32keybridge::virtualKey(9)));
  bridge.update();
  assert(bridge.outputKeys().contains(esp32keybridge::virtualKey(9)));
}

static void test_outputs_receive_output_keys()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ManualOutputAdapter usb;
  assert(bridge.addInput(keyboard));
  assert(bridge.addOutput(usb));
  assert(bridge.outputCount() == 1);

  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.global.remap(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock),
                             esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftCtrl)));
  bridge.applyConfig(config);

  assert(keyboard.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock)));
  bridge.update();
  assert(usb.writeCount() == 1);
  assert(usb.keys().contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftCtrl)));

  // Disconnected outputs are not written.
  usb.setConnected(false);
  bridge.update();
  assert(usb.writeCount() == 1);
}

static void test_terminal_host_toggles_locks_on_press()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key capsLock = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock);
  esp32keybridge::Key numLock = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::NumLock);

  // No lock-reporting output: the bridge is the terminal host.
  bridge.update();
  assert(!bridge.lockAuthorityPresent());
  assert(!bridge.lockState().capsLock);

  // Toggle on press, not on release, and not again while held.
  assert(keyboard.press(capsLock));
  bridge.update();
  assert(bridge.lockState().capsLock);
  bridge.update();
  assert(bridge.lockState().capsLock);
  assert(keyboard.release(capsLock));
  bridge.update();
  assert(bridge.lockState().capsLock);

  assert(keyboard.press(capsLock));
  bridge.update();
  assert(!bridge.lockState().capsLock);
  assert(keyboard.release(capsLock));

  assert(keyboard.press(numLock));
  bridge.update();
  assert(bridge.lockState().numLock);
  assert(!bridge.lockState().capsLock);
  assert(!bridge.lockState().kana);
}

static void test_terminal_host_toggle_uses_resolved_key()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key capsLock = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock);
  esp32keybridge::Key leftCtrl = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftCtrl);

  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.global.remap(capsLock, leftCtrl));
  assert(config.global.remap(leftCtrl, capsLock));
  bridge.applyConfig(config);

  // Physical CapsLock resolves to LeftCtrl: no toggle.
  assert(keyboard.press(capsLock));
  bridge.update();
  assert(!bridge.lockState().capsLock);
  assert(keyboard.release(capsLock));
  bridge.update();

  // Physical LeftCtrl resolves to CapsLock: toggles, as a host would.
  assert(keyboard.press(leftCtrl));
  bridge.update();
  assert(bridge.lockState().capsLock);
}

static void test_lock_state_is_pushed_to_inputs()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  // Newly added inputs receive the current state on the first update.
  bridge.update();
  assert(keyboard.lockStateCount() == 1);

  // Changes are pushed to every connected input.
  assert(keyboard.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock)));
  bridge.update();
  assert(keyboard.lockStateCount() == 2);
  assert(keyboard.lockState().capsLock);

  // No change, no push. Release before the disconnect below so that the
  // reconnect does not replay the press (and toggle again).
  assert(keyboard.release(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock)));
  bridge.update();
  assert(keyboard.lockStateCount() == 2);

  // A reconnecting input gets the current state pushed again.
  keyboard.setConnected(false);
  bridge.update();
  keyboard.setConnected(true);
  bridge.update();
  assert(keyboard.lockStateCount() == 3);

  // A late-joining input gets the state on its first update.
  esp32keybridge::ManualInputAdapter second;
  assert(bridge.addInput(second));
  bridge.update();
  assert(second.lockStateCount() == 1);
  assert(second.lockState().capsLock);
}

static void test_lock_authority_overrides_and_disables_toggle()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ManualOutputAdapter usb;
  assert(bridge.addInput(keyboard));
  assert(bridge.addOutput(usb));

  esp32keybridge::LockState host;
  host.numLock = true;
  host.kana = true;
  usb.setHostLockState(host);

  bridge.update();
  assert(bridge.lockAuthorityPresent());
  assert(bridge.lockState().numLock);
  assert(bridge.lockState().kana);
  assert(keyboard.lockState().numLock);

  // While an authority is present the bridge never self-toggles: CapsLock
  // press is relayed as a key, and the state follows only the host.
  assert(keyboard.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock)));
  bridge.update();
  assert(!bridge.lockState().capsLock);

  // The host reports the change: shadow and inputs follow.
  host.capsLock = true;
  usb.setHostLockState(host);
  bridge.update();
  assert(bridge.lockState().capsLock);
  assert(keyboard.lockState().capsLock);
}

static void test_lock_authority_wins_over_terminal_state()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  // Terminal mode: caps on.
  assert(keyboard.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock)));
  bridge.update();
  assert(keyboard.release(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock)));
  bridge.update();
  assert(bridge.lockState().capsLock);

  // An authority appears with caps off: external wins.
  esp32keybridge::ManualOutputAdapter usb;
  usb.setHostLockState(esp32keybridge::LockState());
  assert(bridge.addOutput(usb));
  bridge.update();
  assert(bridge.lockAuthorityPresent());
  assert(!bridge.lockState().capsLock);

  // The authority disappears: terminal mode continues from the last value.
  usb.setConnected(false);
  bridge.update();
  assert(!bridge.lockAuthorityPresent());
  assert(!bridge.lockState().capsLock);
  assert(keyboard.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::CapsLock)));
  bridge.update();
  assert(bridge.lockState().capsLock);
}

static void test_first_lock_reporting_output_is_authority()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualOutputAdapter log;   // not lock-reporting
  esp32keybridge::ManualOutputAdapter usb;   // first lock-reporting output
  esp32keybridge::ManualOutputAdapter ble;   // second: state is not followed
  assert(bridge.addOutput(log));
  assert(bridge.addOutput(usb));
  assert(bridge.addOutput(ble));

  esp32keybridge::LockState usbState;
  usbState.capsLock = true;
  usb.setHostLockState(usbState);

  esp32keybridge::LockState bleState;
  bleState.numLock = true;
  ble.setHostLockState(bleState);

  bridge.update();
  assert(bridge.lockAuthorityPresent());
  assert(bridge.lockState().capsLock);
  assert(!bridge.lockState().numLock);

  // When the first authority disconnects, the next lock-reporting output
  // takes over.
  usb.setConnected(false);
  bridge.update();
  assert(bridge.lockAuthorityPresent());
  assert(bridge.lockState().numLock);
}

static void test_host_layout_encode()
{
  esp32keybridge::KeyStroke stroke;

  esp32keybridge::KeyboardLayout enUs = esp32keybridge::KeyboardLayout::enUs();
  assert(enUs.encode(U'a', stroke) && stroke.key.code == 0x04 && !stroke.shift);
  assert(enUs.encode(U'A', stroke) && stroke.key.code == 0x04 && stroke.shift);
  assert(enUs.encode(U'@', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit2) &&
         stroke.shift);
  assert(enUs.encode(U'"', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Quote) &&
         stroke.shift);
  assert(!enUs.encode(U'¥', stroke)); // Yen is not typable on en_us

  esp32keybridge::KeyboardLayout jaJp = esp32keybridge::KeyboardLayout::jaJp();
  assert(jaJp.encode(U'@', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftBracket) &&
         !stroke.shift);
  assert(jaJp.encode(U'"', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit2) &&
         stroke.shift);
  assert(jaJp.encode(U'¥', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::International3) &&
         !stroke.shift);

  bool found = false;
  assert(esp32keybridge::KeyboardLayout::byName("ja_jp", &found).encode(U'@', stroke));
  assert(found);
  assert(stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftBracket));
  esp32keybridge::KeyboardLayout::byName("xx_xx", &found);
  assert(!found);

  // en_us / ja_jp carry no AltGr plane; the flag is always false.
  assert(!enUs.hasAltGr());
  assert(!jaJp.hasAltGr());
  assert(enUs.encode(U'@', stroke) && !stroke.altGr);
}

static void test_de_de_altgr_encode_decode()
{
  esp32keybridge::KeyStroke stroke;
  esp32keybridge::KeyboardLayout de = esp32keybridge::KeyboardLayout::deDe();
  assert(de.hasAltGr());

  // AltGr plane: @ € { [ ] } \ ~ | are reached with the altGr flag set.
  assert(de.encode(U'@', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Q) &&
         !stroke.shift && stroke.altGr);
  assert(de.encode(U'€', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::E) &&
         stroke.altGr);
  assert(de.encode(U'{', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit7) &&
         stroke.altGr);
  assert(de.encode(U'|', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::NonUsBackslash) &&
         stroke.altGr);

  // QWERTZ Y/Z swap and the base/Shift planes clear the altGr flag.
  assert(de.encode(U'z', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Y) && !stroke.altGr);
  assert(de.encode(U'y', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Z) && !stroke.altGr);
  assert(de.encode(U'a', stroke) && !stroke.shift && !stroke.altGr);

  // decode: two-arg reads base/Shift, three-arg selects the AltGr plane.
  esp32keybridge::Key q = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Q);
  assert(de.decode(q, false) == U'q');
  assert(de.decode(q, false, false) == U'q');
  assert(de.decode(q, false, true) == U'@');
  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);
  assert(de.decode(a, false, true) == 0); // A has no AltGr plane

  bool found = false;
  assert(esp32keybridge::KeyboardLayout::byName("de_de", &found).hasAltGr());
  assert(found);
}

static void test_typing_emits_altgr()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualOutputAdapter usb;
  esp32keybridge::ESP32KeyBridgeConfig config;
  config.hostLayout = esp32keybridge::KeyboardLayout::deDe();
  assert(bridge.addOutput(usb));
  bridge.applyConfig(config);

  // '@' on de_DE is AltGr+Q: the key-down phase carries Right Alt, not Shift.
  bridge.typeText("@");
  bridge.update(); // phase 0: modifiers only
  bridge.update(); // phase 1: modifiers + key
  const esp32keybridge::KeySet &k = usb.keys();
  assert(k.contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Q)));
  assert(k.contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::RightAlt)));
  assert(!k.contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift)));
}

static void test_layout_conversion_altgr_both_directions()
{
  // de_DE keyboard -> en_us host: AltGr+Q = '@', which is Shift+2 on en_us.
  {
    esp32keybridge::ESP32KeyBridge bridge;
    esp32keybridge::ManualInputAdapter keyboard;
    esp32keybridge::ESP32KeyBridgeConfig config;
    esp32keybridge::InputConfig &kc = config.addInputConfig();
    kc.convertLayout(esp32keybridge::KeyboardLayout::deDe());
    config.hostLayout = esp32keybridge::KeyboardLayout::enUs();
    assert(bridge.addInput(keyboard, kc));
    bridge.applyConfig(config);

    esp32keybridge::Key rightAlt = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::RightAlt);
    esp32keybridge::Key q = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Q);
    assert(keyboard.press(rightAlt));
    assert(keyboard.press(q));
    bridge.update();
    const esp32keybridge::KeySet &o = bridge.outputKeys();
    assert(o.contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit2)));
    assert(o.contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift)));
    assert(!o.contains(rightAlt)); // physical AltGr consumed by conversion
    assert(!o.contains(q));
  }

  // en_us keyboard -> de_DE host: Shift+2 = '@', which is AltGr+Q on de_DE.
  {
    esp32keybridge::ESP32KeyBridge bridge;
    esp32keybridge::ManualInputAdapter keyboard;
    esp32keybridge::ESP32KeyBridgeConfig config;
    esp32keybridge::InputConfig &kc = config.addInputConfig();
    kc.convertLayout(esp32keybridge::KeyboardLayout::enUs());
    config.hostLayout = esp32keybridge::KeyboardLayout::deDe();
    assert(bridge.addInput(keyboard, kc));
    bridge.applyConfig(config);

    esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);
    esp32keybridge::Key digit2 = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit2);
    assert(keyboard.press(leftShift));
    assert(keyboard.press(digit2));
    bridge.update();
    const esp32keybridge::KeySet &o = bridge.outputKeys();
    assert(o.contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Q)));
    assert(o.contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::RightAlt)));
    assert(!o.contains(leftShift)); // physical Shift consumed by conversion
    assert(!o.contains(digit2));
  }
}

static void test_typing_produces_atomic_frames()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualOutputAdapter usb;
  assert(bridge.addOutput(usb));

  esp32keybridge::Key digit1 = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit1);
  esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);

  // '!' on en_us = Shift + Digit1: modifier-first, then key, then release.
  assert(bridge.typeChar(U'!'));
  bridge.update();
  assert(bridge.typingActive());
  assert(bridge.outputKeys().contains(leftShift));
  assert(!bridge.outputKeys().contains(digit1));
  bridge.update();
  assert(bridge.outputKeys().contains(leftShift));
  assert(bridge.outputKeys().contains(digit1));
  bridge.update();
  assert(bridge.outputKeys().contains(leftShift));
  assert(!bridge.outputKeys().contains(digit1));
  bridge.update();
  assert(!bridge.typingActive());
  assert(bridge.outputKeys().count() == 0);
  assert(usb.textCount() == 1);
  assert(usb.lastText() == U'!');
}

static void test_typing_parks_user_modifiers()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);
  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);

  assert(keyboard.press(leftShift));
  bridge.update();
  assert(bridge.outputKeys().contains(leftShift));

  // Typing 'a' (no shift required) while the user holds Shift: the user's
  // modifier is parked during the frame and restored afterwards.
  assert(bridge.typeChar(U'a'));
  bridge.update();
  assert(!bridge.outputKeys().contains(leftShift));
  bridge.update();
  assert(bridge.outputKeys().contains(a));
  assert(!bridge.outputKeys().contains(leftShift));
  bridge.update();
  bridge.update();
  assert(!bridge.typingActive());
  assert(bridge.outputKeys().contains(leftShift));
}

static void test_typing_defer_option_waits_for_modifiers()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.deferTypingWhileModifiersHeld = true;
  bridge.applyConfig(config);

  esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);

  assert(keyboard.press(leftShift));
  bridge.update();
  assert(bridge.typeChar(U'a'));
  bridge.update();
  bridge.update();
  assert(!bridge.typingActive());
  assert(bridge.textQueueLength() == 1);
  assert(bridge.outputKeys().contains(leftShift));

  assert(keyboard.release(leftShift));
  bridge.update();
  assert(bridge.typingActive());
  bridge.update();
  assert(bridge.outputKeys().contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A)));
}

static void test_typing_caps_lock_compensation()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualOutputAdapter usb;
  assert(bridge.addOutput(usb));

  esp32keybridge::LockState host;
  host.capsLock = true;
  usb.setHostLockState(host);
  bridge.update();
  assert(bridge.lockState().capsLock);

  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);
  esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);

  // Caps on: 'a' needs Shift to come out lowercase.
  assert(bridge.typeChar(U'a'));
  bridge.update();
  bridge.update();
  assert(bridge.outputKeys().contains(a));
  assert(bridge.outputKeys().contains(leftShift));
  bridge.update();
  bridge.update();

  // Caps on: 'A' needs no Shift.
  assert(bridge.typeChar(U'A'));
  bridge.update();
  bridge.update();
  assert(bridge.outputKeys().contains(a));
  assert(!bridge.outputKeys().contains(leftShift));
}

static void test_typing_control_chars_and_crlf()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualOutputAdapter usb;
  assert(bridge.addOutput(usb));

  // CRLF collapses to one Enter at the enqueue edge.
  assert(bridge.typeText("a\r\nb") == 4);
  assert(bridge.textQueueLength() == 3); // 'a', CR, 'b'

  bool sawEnter = false;
  for (int i = 0; i < 12; ++i)
  {
    bridge.update();
    if (bridge.outputKeys().contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Enter)))
    {
      sawEnter = true;
    }
  }
  assert(sawEnter);
  assert(!bridge.typingActive());
  assert(usb.textCount() == 3);
  assert(bridge.textEncodeFailCount() == 0);
}

static void test_typing_unencodable_chars_are_counted()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualOutputAdapter log;
  assert(bridge.addOutput(log));

  // en_us cannot type Yen: dropped, counted, still delivered to writeText.
  assert(bridge.typeChar(U'¥'));
  bridge.update();
  assert(!bridge.typingActive());
  assert(bridge.textEncodeFailCount() == 1);
  assert(log.textCount() == 1);
  assert(log.lastText() == U'¥');
}

static void test_type_available_reports_queue_space()
{
  esp32keybridge::ESP32KeyBridge bridge;
  assert(bridge.typeAvailable() == esp32keybridge::ESP32KeyBridge::MaxTextQueue);

  assert(bridge.typeChar(U'a'));
  assert(bridge.typeAvailable() == esp32keybridge::ESP32KeyBridge::MaxTextQueue - 1);

  // Fill the queue: typeAvailable reaches 0 and further characters drop.
  while (bridge.typeAvailable() > 0)
  {
    assert(bridge.typeChar(U'b'));
  }
  assert(!bridge.typeChar(U'c'));
  assert(bridge.textOverflowCount() == 1);
}

static void test_type_text_stops_without_dropping_when_full()
{
  esp32keybridge::ESP32KeyBridge bridge;

  // Leave two free slots, then send a longer string: exactly two
  // characters are consumed, the rest is neither consumed nor dropped.
  while (bridge.typeAvailable() > 2)
  {
    assert(bridge.typeChar(U'x'));
  }
  assert(bridge.typeText("abcd") == 2);
  assert(bridge.typeAvailable() == 0);
  assert(bridge.textOverflowCount() == 0);

  // Multi-byte characters are consumed whole or not at all: with one free
  // slot, one 2-byte yen sign fits (2 bytes consumed), the second waits.
  esp32keybridge::ESP32KeyBridge bridge2;
  while (bridge2.typeAvailable() > 1)
  {
    assert(bridge2.typeChar(U'x'));
  }
  assert(bridge2.typeText("\xc2\xa5\xc2\xa5") == 2);
  assert(bridge2.typeAvailable() == 0);
  assert(bridge2.textOverflowCount() == 0);
}

static void test_text_macro_types_on_trigger()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ManualOutputAdapter usb;
  assert(bridge.addInput(keyboard));
  assert(bridge.addOutput(usb));

  esp32keybridge::Key fn2 = esp32keybridge::virtualKey(2);

  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.textMacro(fn2, "hi"));
  bridge.applyConfig(config);

  assert(keyboard.press(fn2));
  bridge.update();
  // The trigger is consumed and typing starts.
  assert(!bridge.outputKeys().contains(fn2));
  assert(bridge.typingActive());

  bool sawH = false;
  bool sawI = false;
  for (int i = 0; i < 10; ++i)
  {
    bridge.update();
    if (bridge.outputKeys().contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::H)))
    {
      sawH = true;
    }
    if (bridge.outputKeys().contains(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::I)))
    {
      sawI = true;
    }
  }
  assert(sawH);
  assert(sawI);
  assert(usb.textCount() == 2);

  // Holding the trigger does not retype; a new press does.
  assert(keyboard.release(fn2));
  bridge.update();
  assert(keyboard.press(fn2));
  bridge.update();
  assert(bridge.textQueueLength() + (bridge.typingActive() ? 1 : 0) >= 1);
}

static void test_relative_axes_scale_and_drain()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter mouse;
  esp32keybridge::ManualOutputAdapter usb;
  assert(bridge.addInput(mouse));
  assert(bridge.addOutput(usb));

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.setAxisScale(esp32keybridge::Axis::Wheel, -2); // natural scrolling, doubled
  bridge.applyConfig(config);

  mouse.addAxisDelta(esp32keybridge::Axis::X, 5);
  mouse.addAxisDelta(esp32keybridge::Axis::Wheel, 1);
  bridge.update();
  assert(bridge.axisDelta(esp32keybridge::Axis::X) == 5);
  assert(bridge.axisDelta(esp32keybridge::Axis::Wheel) == -2);
  assert(usb.axisTotal(esp32keybridge::Axis::X) == 5);
  assert(usb.axisTotal(esp32keybridge::Axis::Wheel) == -2);

  // Deltas are one-shot: drained after the update.
  bridge.update();
  assert(bridge.axisDelta(esp32keybridge::Axis::X) == 0);
  assert(usb.axisTotal(esp32keybridge::Axis::X) == 5);
}

// UC5 setup: registers a US-engraved keyboard, bound to layout conversion
// toward a ja_jp host.
static void addUsKeyboardOnJa(esp32keybridge::ESP32KeyBridge &bridge,
                              esp32keybridge::ManualInputAdapter &keyboard,
                              esp32keybridge::ESP32KeyBridgeConfig &config)
{
  esp32keybridge::InputConfig &keyboardConfig = config.addInputConfig();
  keyboardConfig.convertLayout(esp32keybridge::KeyboardLayout::enUs());
  config.hostLayout = esp32keybridge::KeyboardLayout::jaJp();
  assert(bridge.addInput(keyboard, keyboardConfig));
}

static void test_layout_conversion_suppresses_shift()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ESP32KeyBridgeConfig config;
  addUsKeyboardOnJa(bridge, keyboard, config);
  bridge.applyConfig(config);

  esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);
  esp32keybridge::Key digit2 = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit2);
  esp32keybridge::Key leftBracket = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftBracket);

  // Engraved '@' = Shift+2 on the US keyboard -> ja_jp '@' key, no Shift.
  assert(keyboard.press(leftShift));
  assert(keyboard.press(digit2));
  bridge.update();
  assert(bridge.outputKeys().contains(leftBracket));
  assert(!bridge.outputKeys().contains(digit2));
  assert(!bridge.outputKeys().contains(leftShift)); // consumed, suppressed

  // Held while the physical Shift is released first: press-time resolution.
  assert(keyboard.release(leftShift));
  bridge.update();
  assert(bridge.outputKeys().contains(leftBracket));

  assert(keyboard.release(digit2));
  bridge.update();
  assert(bridge.outputKeys().count() == 0);
}

static void test_layout_conversion_synthesizes_shift()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ESP32KeyBridgeConfig config;
  addUsKeyboardOnJa(bridge, keyboard, config);
  bridge.applyConfig(config);

  esp32keybridge::Key equal = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Equal);
  esp32keybridge::Key minus = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Minus);
  esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);

  // Engraved '=' (US Equal, no Shift) -> ja_jp '=' is Shift+Minus: Shift is
  // synthesized and held.
  assert(keyboard.press(equal));
  bridge.update();
  assert(bridge.outputKeys().contains(minus));
  assert(bridge.outputKeys().contains(leftShift));
  bridge.update();
  assert(bridge.outputKeys().contains(minus)); // long press is preserved
  assert(bridge.outputKeys().contains(leftShift));

  assert(keyboard.release(equal));
  bridge.update();
  assert(bridge.outputKeys().count() == 0); // synthesized Shift released too
}

static void test_layout_conversion_letters_stay_identity()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ESP32KeyBridgeConfig config;
  addUsKeyboardOnJa(bridge, keyboard, config);
  bridge.applyConfig(config);

  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);
  esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);

  // 'a' passes as A; Shift+'a' decodes to 'A' and re-synthesizes Shift+A.
  assert(keyboard.press(a));
  bridge.update();
  assert(bridge.outputKeys().contains(a));
  assert(!bridge.outputKeys().contains(leftShift));
  assert(keyboard.release(a));
  bridge.update();

  assert(keyboard.press(leftShift));
  assert(keyboard.press(a));
  bridge.update();
  assert(bridge.outputKeys().contains(a));
  assert(bridge.outputKeys().contains(leftShift));
}

static void test_layout_conversion_shortcut_passthrough()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ESP32KeyBridgeConfig config;
  addUsKeyboardOnJa(bridge, keyboard, config);
  bridge.applyConfig(config);

  esp32keybridge::Key leftCtrl = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftCtrl);
  esp32keybridge::Key digit2 = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit2);

  // Ctrl+2 is a shortcut: position semantics, no conversion.
  assert(keyboard.press(leftCtrl));
  assert(keyboard.press(digit2));
  bridge.update();
  assert(bridge.outputKeys().contains(leftCtrl));
  assert(bridge.outputKeys().contains(digit2));
  assert(!bridge.outputKeys().contains(
      esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftBracket)));
}

static void test_layout_conversion_shift_passes_for_nonprintable()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ESP32KeyBridgeConfig config;
  addUsKeyboardOnJa(bridge, keyboard, config);
  bridge.applyConfig(config);

  esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);
  esp32keybridge::Key left = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Left);

  // Shift+Arrow selection: the arrow passes through, and the real Shift is
  // reflected while it is held.
  assert(keyboard.press(leftShift));
  bridge.update();
  assert(!bridge.outputKeys().contains(leftShift)); // consumed while alone

  assert(keyboard.press(left));
  bridge.update();
  assert(bridge.outputKeys().contains(left));
  assert(bridge.outputKeys().contains(leftShift));

  assert(keyboard.release(left));
  bridge.update();
  assert(!bridge.outputKeys().contains(leftShift));
}

static void test_layout_conversion_untypable_is_dropped()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;

  // Reverse direction: JIS-engraved keyboard on an en_us host. Yen cannot
  // be typed on en_us: the press is dropped and counted.
  esp32keybridge::ESP32KeyBridgeConfig config;
  esp32keybridge::InputConfig &keyboardConfig = config.addInputConfig();
  keyboardConfig.convertLayout(esp32keybridge::KeyboardLayout::jaJp());
  config.hostLayout = esp32keybridge::KeyboardLayout::enUs();
  assert(bridge.addInput(keyboard, keyboardConfig));
  bridge.applyConfig(config);

  assert(keyboard.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::International3)));
  bridge.update();
  assert(bridge.outputKeys().count() == 0);
  assert(bridge.layoutConvertFailCount() == 1);
}

static void test_layout_conversion_toggle_key()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ESP32KeyBridgeConfig config;
  addUsKeyboardOnJa(bridge, keyboard, config);
  config.layoutConversionToggle = esp32keybridge::virtualKey(5);
  assert(config.global.remap(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::F14),
                             esp32keybridge::virtualKey(5)));
  bridge.applyConfig(config);

  esp32keybridge::Key f14 = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::F14);
  esp32keybridge::Key leftShift = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift);
  esp32keybridge::Key digit2 = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit2);

  // Toggle off: the key is consumed.
  assert(bridge.layoutConversionEnabled());
  assert(keyboard.press(f14));
  bridge.update();
  assert(!bridge.layoutConversionEnabled());
  assert(bridge.outputKeys().count() == 0);
  assert(keyboard.release(f14));
  bridge.update();

  // Conversion off (BIOS etc.): raw passthrough.
  assert(keyboard.press(leftShift));
  assert(keyboard.press(digit2));
  bridge.update();
  assert(bridge.outputKeys().contains(leftShift));
  assert(bridge.outputKeys().contains(digit2));
  assert(keyboard.release(leftShift));
  assert(keyboard.release(digit2));
  bridge.update();

  // Toggle back on.
  assert(keyboard.press(f14));
  bridge.update();
  assert(bridge.layoutConversionEnabled());
}

static void test_convert_layout_emits_text_to_outputs()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ManualOutputAdapter out;
  esp32keybridge::ESP32KeyBridgeConfig config;
  esp32keybridge::InputConfig &kb = config.addInputConfig();
  kb.convertLayout(esp32keybridge::KeyboardLayout::enUs());
  assert(bridge.addInput(keyboard, kb));
  assert(bridge.addOutput(out));
  bridge.applyConfig(config);

  const esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);
  const esp32keybridge::Key enter = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Enter);

  // The press edge decodes to 'a' and reaches the text output once.
  assert(keyboard.press(a));
  bridge.update();
  assert(out.textCount() == 1);
  assert(out.lastText() == U'a');
  // Held: no repeat while the key stays down.
  bridge.update();
  assert(out.textCount() == 1);
  assert(keyboard.release(a));
  bridge.update();
  assert(out.textCount() == 1);

  // Enter carries no printable codepoint but maps to a newline.
  assert(keyboard.press(enter));
  bridge.update();
  assert(out.textCount() == 2);
  assert(out.lastText() == U'\n');
}

static void test_no_convert_layout_emits_no_text()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ManualOutputAdapter out;
  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(bridge.addInput(keyboard)); // no convertLayout
  assert(bridge.addOutput(out));
  bridge.applyConfig(config);

  // Without convertLayout a physical press is not decoded to text; it only
  // appears in the output key set.
  const esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);
  assert(keyboard.press(a));
  bridge.update();
  assert(out.textCount() == 0);
  assert(out.keys().contains(a));
}

static void test_hid_keyboard_report_builder()
{
  esp32keybridge::KeySet keys;
  assert(keys.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftShift)));
  assert(keys.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A)));
  assert(keys.press(esp32keybridge::consumerKey(0x00e9)));  // skipped
  assert(keys.press(esp32keybridge::mouseButtonKey(1)));    // skipped
  assert(keys.press(esp32keybridge::virtualKey(1)));        // skipped

  esp32keybridge::HidKeyboardReport report = esp32keybridge::buildHidKeyboardReport(keys);
  assert(report.modifiers == 0x02);
  assert(report.keyCount == 1);
  assert(report.keys[0] == 0x04);
  assert(!report.overflow);
  assert(!report.empty());

  uint8_t buffer[esp32keybridge::HidKeyboardReport::BootReportSize] = {};
  assert(report.writeBootReport(buffer, sizeof(buffer)));
  assert(buffer[0] == 0x02);
  assert(buffer[1] == 0x00);
  assert(buffer[2] == 0x04);
  assert(buffer[3] == 0x00);
  assert(!report.writeBootReport(buffer, 4));

  report.clear();
  assert(report.empty());
}

static void test_hid_keyboard_report_overflow()
{
  esp32keybridge::KeySet keys;
  for (uint16_t i = 0; i < 7; ++i)
  {
    assert(keys.press(esp32keybridge::keyboardKey(static_cast<uint16_t>(0x04 + i))));
  }

  esp32keybridge::HidKeyboardReport report = esp32keybridge::buildHidKeyboardReport(keys);
  assert(report.keyCount == 6);
  assert(report.overflow);

  esp32keybridge::HidKeyboardRolloverReport rollover =
      esp32keybridge::buildHidKeyboardRolloverReport(keys);
  assert(rollover.keyCount == 7);
  assert(!rollover.overflow);

  uint8_t buffer[esp32keybridge::HidKeyboardRolloverReport::ReportSize] = {};
  assert(rollover.writeReport(buffer, sizeof(buffer)));
  assert(buffer[0] == 0x00);
  assert(buffer[1] == 0x04);
  assert(buffer[7] == 0x0a);
}

static void test_hid_consumer_report_builder()
{
  esp32keybridge::KeySet keys;
  assert(keys.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A)));
  assert(keys.press(esp32keybridge::consumerKey(esp32keybridge::ConsumerUsage::VolumeIncrement)));

  esp32keybridge::HidConsumerReport report = esp32keybridge::buildHidConsumerReport(keys);
  assert(report.usage == 0x00e9);
  assert(!report.overflow);

  uint8_t buffer[esp32keybridge::HidConsumerReport::ReportSize] = {};
  assert(report.writeReport(buffer, sizeof(buffer)));
  assert(buffer[0] == 0xe9);
  assert(buffer[1] == 0x00);

  assert(keys.press(esp32keybridge::consumerKey(esp32keybridge::ConsumerUsage::Mute)));
  report = esp32keybridge::buildHidConsumerReport(keys);
  assert(report.overflow);
}

static void test_hid_mouse_report_builder()
{
  esp32keybridge::KeySet keys;
  assert(keys.press(esp32keybridge::mouseButtonKey(1)));
  assert(keys.press(esp32keybridge::mouseButtonKey(3)));
  assert(keys.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A))); // skipped

  esp32keybridge::HidMouseReport report = esp32keybridge::buildHidMouseReport(keys);
  assert(report.buttons == 0x05);
  assert(!report.overflow);

  // Axis saturation with carry.
  int32_t remainder = report.applyAxisDelta(esp32keybridge::Axis::X, 200);
  assert(report.x == 127);
  assert(remainder == 73);
  remainder = report.applyAxisDelta(esp32keybridge::Axis::Wheel, -3);
  assert(report.wheel == -3);
  assert(remainder == 0);

  uint8_t buffer[esp32keybridge::HidMouseReport::ReportSize] = {};
  assert(report.writeReport(buffer, sizeof(buffer)));
  assert(buffer[0] == 0x05);
  assert(buffer[1] == 0x7f);
  assert(buffer[3] == 0xfd);

  // Button 9 cannot be represented.
  assert(keys.press(esp32keybridge::mouseButtonKey(9)));
  report = esp32keybridge::buildHidMouseReport(keys);
  assert(report.overflow);
}

static void test_key_set_iteration_merge_and_clear()
{
  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);
  esp32keybridge::Key b = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::B);
  esp32keybridge::Key c = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::C);

  esp32keybridge::KeySet left;
  assert(left.press(a));
  assert(left.press(b));

  // at() enumerates the pressed keys; every entry is a member.
  bool sawA = false;
  bool sawB = false;
  for (size_t i = 0; i < left.count(); ++i)
  {
    const esp32keybridge::Key key = left.at(i);
    assert(left.contains(key));
    sawA = sawA || (key == a);
    sawB = sawB || (key == b);
  }
  assert(sawA && sawB);
  assert(!esp32keybridge::isValid(left.at(left.count()))); // out of range

  // mergeFrom is a union: overlapping keys are not duplicated.
  esp32keybridge::KeySet right;
  assert(right.press(b));
  assert(right.press(c));
  assert(left.mergeFrom(right));
  assert(left.count() == 3);
  assert(left.contains(a) && left.contains(b) && left.contains(c));

  // Merging past capacity reports the drop but keeps what fit.
  esp32keybridge::KeySet full;
  for (uint16_t i = 0; i < esp32keybridge::KeySet::MaxKeys; ++i)
  {
    assert(full.press(esp32keybridge::virtualKey(static_cast<uint16_t>(i + 1))));
  }
  esp32keybridge::KeySet extra;
  assert(extra.press(esp32keybridge::virtualKey(1000)));
  assert(!full.mergeFrom(extra));
  assert(full.count() == esp32keybridge::KeySet::MaxKeys);

  left.clear();
  assert(left.count() == 0);
  assert(!left.contains(a));
}

static void test_mouse_usage_buttons_map_to_report_bits()
{
  // The whole MouseUsage range converts to a Key implicitly and lands on the
  // matching report bit (button N -> bit N-1).
  struct
  {
    esp32keybridge::MouseUsage usage;
    uint8_t bit;
  } cases[] = {
      {esp32keybridge::MouseUsage::Left, 0},    {esp32keybridge::MouseUsage::Right, 1},
      {esp32keybridge::MouseUsage::Middle, 2},  {esp32keybridge::MouseUsage::Back, 3},
      {esp32keybridge::MouseUsage::Forward, 4}, {esp32keybridge::MouseUsage::Button6, 5},
      {esp32keybridge::MouseUsage::Button7, 6}, {esp32keybridge::MouseUsage::Button8, 7},
  };

  for (const auto &c : cases)
  {
    esp32keybridge::Key key = c.usage; // implicit conversion
    esp32keybridge::KeySet keys;
    assert(keys.press(key));
    esp32keybridge::HidMouseReport report = esp32keybridge::buildHidMouseReport(keys);
    assert(report.buttons == static_cast<uint8_t>(1u << c.bit));
    assert(!report.overflow);
  }

  // Back + Forward together set two bits.
  esp32keybridge::KeySet both;
  assert(both.press(esp32keybridge::MouseUsage::Back));
  assert(both.press(esp32keybridge::MouseUsage::Forward));
  esp32keybridge::HidMouseReport report = esp32keybridge::buildHidMouseReport(both);
  assert(report.buttons == 0x18); // bits 3 and 4
}

static void test_all_axes_scale_independently()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter mouse;
  esp32keybridge::ManualOutputAdapter usb;
  assert(bridge.addInput(mouse));
  assert(bridge.addOutput(usb));

  esp32keybridge::ESP32KeyBridgeConfig config;
  // Y left at the default scale of 1; the others get distinct scales.
  assert(config.axisScale(esp32keybridge::Axis::Y) == 1);
  config.setAxisScale(esp32keybridge::Axis::X, 3);
  config.setAxisScale(esp32keybridge::Axis::Wheel, -2);
  config.setAxisScale(esp32keybridge::Axis::Pan, 2);
  assert(config.axisScale(esp32keybridge::Axis::X) == 3);
  bridge.applyConfig(config);

  mouse.addAxisDelta(esp32keybridge::Axis::X, 4);
  mouse.addAxisDelta(esp32keybridge::Axis::Y, 5);
  mouse.addAxisDelta(esp32keybridge::Axis::Wheel, 3);
  mouse.addAxisDelta(esp32keybridge::Axis::Pan, 1);
  bridge.update();

  assert(bridge.axisDelta(esp32keybridge::Axis::X) == 12);
  assert(bridge.axisDelta(esp32keybridge::Axis::Y) == 5); // unscaled
  assert(bridge.axisDelta(esp32keybridge::Axis::Wheel) == -6);
  assert(bridge.axisDelta(esp32keybridge::Axis::Pan) == 2);
  assert(usb.axisTotal(esp32keybridge::Axis::Y) == 5);

  // Every axis drains after the update.
  bridge.update();
  assert(bridge.axisDelta(esp32keybridge::Axis::X) == 0);
  assert(bridge.axisDelta(esp32keybridge::Axis::Y) == 0);
  assert(bridge.axisDelta(esp32keybridge::Axis::Wheel) == 0);
  assert(bridge.axisDelta(esp32keybridge::Axis::Pan) == 0);
}

static void test_text_macro_lookup_and_capacity()
{
  esp32keybridge::ESP32KeyBridgeConfig config;
  esp32keybridge::Key fn1 = esp32keybridge::virtualKey(1);

  // Not registered yet.
  assert(config.findTextMacro(fn1) == nullptr);

  assert(config.textMacro(fn1, "ok"));
  const esp32keybridge::TextMacro *macro = config.findTextMacro(fn1);
  assert(macro != nullptr);
  assert(macro->length == 2);
  assert(macro->text[0] == U'o' && macro->text[1] == U'k');

  // Re-registering the same trigger overwrites in place (no new slot).
  assert(config.textMacro(fn1, "hey"));
  macro = config.findTextMacro(fn1);
  assert(macro != nullptr && macro->length == 3 && macro->text[0] == U'h');

  // Fill the remaining slots, then overflow.
  assert(config.textMacro(esp32keybridge::virtualKey(2), "b"));
  assert(config.textMacro(esp32keybridge::virtualKey(3), "c"));
  assert(config.textMacro(esp32keybridge::virtualKey(4), "d"));
  assert(!config.textMacro(esp32keybridge::virtualKey(5), "e"));
  assert(config.findTextMacro(esp32keybridge::virtualKey(5)) == nullptr);

  // Invalid arguments are rejected.
  assert(!config.textMacro(esp32keybridge::Key(), "x"));
  assert(!config.textMacro(fn1, nullptr));
}

static void test_multiple_outputs_receive_keys_text_axis()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  esp32keybridge::ManualOutputAdapter first;
  esp32keybridge::ManualOutputAdapter second;
  esp32keybridge::ManualOutputAdapter offline;
  assert(bridge.addInput(keyboard));
  assert(bridge.addOutput(first));
  assert(bridge.addOutput(second));
  assert(bridge.addOutput(offline));
  offline.setConnected(false);

  esp32keybridge::Key a = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A);

  // Keys and relative axes fan out to every connected output.
  assert(keyboard.press(a));
  keyboard.addAxisDelta(esp32keybridge::Axis::Wheel, 2);
  bridge.update();
  assert(first.keys().contains(a));
  assert(second.keys().contains(a));
  assert(first.axisTotal(esp32keybridge::Axis::Wheel) == 2);
  assert(second.axisTotal(esp32keybridge::Axis::Wheel) == 2);
  assert(offline.writeCount() == 0);
  assert(offline.axisTotal(esp32keybridge::Axis::Wheel) == 0);

  // The text stream fans out the same way.
  assert(bridge.typeChar(U'z'));
  for (int i = 0; i < 6; ++i)
  {
    bridge.update();
  }
  assert(first.textCount() == 1 && first.lastText() == U'z');
  assert(second.textCount() == 1 && second.lastText() == U'z');
  assert(offline.textCount() == 0);
}

static void test_text_output_adapter_writes_utf8()
{
  CaptureStream stream;
  esp32keybridge::TextOutputAdapter text(stream);

  // ASCII: one byte.
  text.writeText(U'a');
  assert(stream.length() == 1 && stream.byteAt(0) == 0x61);

  // U+00A5 YEN SIGN: two bytes.
  stream.reset();
  text.writeText(U'¥');
  assert(stream.length() == 2 && stream.byteAt(0) == 0xc2 && stream.byteAt(1) == 0xa5);

  // U+3042 HIRAGANA A: three bytes.
  stream.reset();
  text.writeText(U'あ');
  assert(stream.length() == 3 && stream.byteAt(0) == 0xe3 && stream.byteAt(1) == 0x81 &&
         stream.byteAt(2) == 0x82);

  // U+1F600 GRINNING FACE: four bytes.
  stream.reset();
  text.writeText(U'\U0001f600');
  assert(stream.length() == 4 && stream.byteAt(0) == 0xf0 && stream.byteAt(1) == 0x9f &&
         stream.byteAt(2) == 0x98 && stream.byteAt(3) == 0x80);

  // A text output ignores the key set entirely.
  esp32keybridge::KeySet keys;
  assert(keys.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A)));
  stream.reset();
  text.write(keys);
  assert(stream.length() == 0);
  assert(text.connected());
}

static void test_log_output_adapter_formats_lines()
{
  CaptureStream stream;
  esp32keybridge::LogOutputAdapter log(stream);

  // A key set change prints one KEYS line naming each key by kind and code.
  esp32keybridge::KeySet keys;
  assert(keys.press(esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::A)));
  log.write(keys);
  assert(stream.has("KEYS") && stream.has("keyboard:4"));

  // Clearing prints the empty marker.
  stream.reset();
  esp32keybridge::KeySet empty;
  log.write(empty);
  assert(stream.has("(empty)"));

  // No change, no line.
  stream.reset();
  log.write(empty);
  assert(stream.length() == 0);

  // Text and axis events each get their own line.
  stream.reset();
  log.writeText(U'A'); // 0x41
  assert(stream.has("TEXT 41"));

  stream.reset();
  log.writeAxisDelta(esp32keybridge::Axis::Wheel, -3);
  assert(stream.has("AXIS wheel -3"));

  stream.reset();
  log.writeAxisDelta(esp32keybridge::Axis::X, 5);
  assert(stream.has("AXIS x +5"));
  assert(log.connected());
}

static void run(const char *name, void (*test)())
{
  Serial.print("RUN ");
  Serial.println(name);
  ++g_total;
  test();
}

} // namespace

static void runAllTests()
{
  run("key_identity_is_kind_plus_code", test_key_identity_is_kind_plus_code);
  run("key_converts_implicitly_from_usage_enums", test_key_converts_implicitly_from_usage_enums);
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
  run("global_remap_swaps_without_chaining", test_global_remap_swaps_without_chaining);
  run("remap_crosses_key_kinds", test_remap_crosses_key_kinds);
  run("global_disable_drops_key", test_global_disable_drops_key);
  run("per_input_remap_applies_to_one_input", test_per_input_remap_applies_to_one_input);
  run("input_config_shared_between_inputs", test_input_config_shared_between_inputs);
  run("momentary_layer_with_press_time_resolution", test_momentary_layer_with_press_time_resolution);
  run("layer_trigger_and_key_in_same_update", test_layer_trigger_and_key_in_same_update);
  run("add_layer_by_trigger", test_add_layer_by_trigger);
  run("layer_works_across_inputs", test_layer_works_across_inputs);
  run("apply_config_keeps_held_resolution", test_apply_config_keeps_held_resolution);
  run("unconsumed_virtual_key_stays_in_output", test_unconsumed_virtual_key_stays_in_output);
  run("outputs_receive_output_keys", test_outputs_receive_output_keys);
  run("terminal_host_toggles_locks_on_press", test_terminal_host_toggles_locks_on_press);
  run("terminal_host_toggle_uses_resolved_key", test_terminal_host_toggle_uses_resolved_key);
  run("lock_state_is_pushed_to_inputs", test_lock_state_is_pushed_to_inputs);
  run("lock_authority_overrides_and_disables_toggle", test_lock_authority_overrides_and_disables_toggle);
  run("lock_authority_wins_over_terminal_state", test_lock_authority_wins_over_terminal_state);
  run("first_lock_reporting_output_is_authority", test_first_lock_reporting_output_is_authority);
  run("host_layout_encode", test_host_layout_encode);
  run("de_de_altgr_encode_decode", test_de_de_altgr_encode_decode);
  run("typing_emits_altgr", test_typing_emits_altgr);
  run("typing_produces_atomic_frames", test_typing_produces_atomic_frames);
  run("typing_parks_user_modifiers", test_typing_parks_user_modifiers);
  run("typing_defer_option_waits_for_modifiers", test_typing_defer_option_waits_for_modifiers);
  run("typing_caps_lock_compensation", test_typing_caps_lock_compensation);
  run("typing_control_chars_and_crlf", test_typing_control_chars_and_crlf);
  run("typing_unencodable_chars_are_counted", test_typing_unencodable_chars_are_counted);
  run("type_available_reports_queue_space", test_type_available_reports_queue_space);
  run("type_text_stops_without_dropping_when_full", test_type_text_stops_without_dropping_when_full);
  run("text_macro_types_on_trigger", test_text_macro_types_on_trigger);
  run("relative_axes_scale_and_drain", test_relative_axes_scale_and_drain);
  run("layout_conversion_suppresses_shift", test_layout_conversion_suppresses_shift);
  run("layout_conversion_synthesizes_shift", test_layout_conversion_synthesizes_shift);
  run("layout_conversion_letters_stay_identity", test_layout_conversion_letters_stay_identity);
  run("layout_conversion_shortcut_passthrough", test_layout_conversion_shortcut_passthrough);
  run("layout_conversion_shift_passes_for_nonprintable", test_layout_conversion_shift_passes_for_nonprintable);
  run("layout_conversion_untypable_is_dropped", test_layout_conversion_untypable_is_dropped);
  run("layout_conversion_toggle_key", test_layout_conversion_toggle_key);
  run("layout_conversion_altgr_both_directions", test_layout_conversion_altgr_both_directions);
  run("convert_layout_emits_text_to_outputs", test_convert_layout_emits_text_to_outputs);
  run("no_convert_layout_emits_no_text", test_no_convert_layout_emits_no_text);
  run("key_set_iteration_merge_and_clear", test_key_set_iteration_merge_and_clear);
  run("mouse_usage_buttons_map_to_report_bits", test_mouse_usage_buttons_map_to_report_bits);
  run("all_axes_scale_independently", test_all_axes_scale_independently);
  run("text_macro_lookup_and_capacity", test_text_macro_lookup_and_capacity);
  run("multiple_outputs_receive_keys_text_axis", test_multiple_outputs_receive_keys_text_axis);
  run("text_output_adapter_writes_utf8", test_text_output_adapter_writes_utf8);
  run("log_output_adapter_formats_lines", test_log_output_adapter_formats_lines);
  run("hid_keyboard_report_builder", test_hid_keyboard_report_builder);
  run("hid_keyboard_report_overflow", test_hid_keyboard_report_overflow);
  run("hid_consumer_report_builder", test_hid_consumer_report_builder);
  run("hid_mouse_report_builder", test_hid_mouse_report_builder);
  Serial.print("TEST done ");
  Serial.print(g_total);
  Serial.print("/");
  Serial.println(g_total);
}

// The tests allocate a bridge (~7.3 KB) and a config (~5.6 KB) as locals,
// which overflows the default 8 KB loop task stack on real boards. The
// arduino-esp32 core sizes the loop task with this weak hook (what its
// SET_LOOP_TASK_STACK_SIZE macro expands to); on the host core it is
// simply an uncalled function.
size_t getArduinoLoopTaskStackSize() { return 24 * 1024; }

void setup()
{
  Serial.begin(115200);
  runAllTests();
}

void loop()
{
  // Re-emit the summary until the reader attaches. The test runner starts
  // reading only after the board has booted and run setup(), so a
  // one-shot print at boot is lost in that gap (confirmed empirically on
  // this harness); a fixed pre-print delay would only paper over a
  // gap whose length is environment-dependent. The runner matches the
  // first line it sees and disconnects. If an assertion fails the board
  // panics and never reaches here, so no summary is emitted and the run
  // fails — the intended outcome. No-op on the host, which matches the
  // first print from setup() before loop() runs.
  Serial.print("TEST done ");
  Serial.print(g_total);
  Serial.print("/");
  Serial.println(g_total);
  delay(1000);
}
