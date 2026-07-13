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
  assert(bridge.addInput(keyboard)); // config slot 0
  assert(bridge.addInput(scanner));  // config slot 1

  esp32keybridge::Key enter = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Enter);
  esp32keybridge::Key tab = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Tab);

  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.input(1).remap(enter, tab));
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

static void test_explicit_config_index_binding()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter scanner;
  assert(bridge.addInput(scanner, 3));

  esp32keybridge::Key enter = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Enter);
  esp32keybridge::Key tab = esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Tab);

  esp32keybridge::ESP32KeyBridgeConfig config;
  assert(config.input(3).remap(enter, tab));
  bridge.applyConfig(config);

  assert(scanner.press(enter));
  bridge.update();
  assert(bridge.outputKeys().contains(tab));
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

  esp32keybridge::HostLayout enUs = esp32keybridge::HostLayout::enUs();
  assert(enUs.encode(U'a', stroke) && stroke.key.code == 0x04 && !stroke.shift);
  assert(enUs.encode(U'A', stroke) && stroke.key.code == 0x04 && stroke.shift);
  assert(enUs.encode(U'@', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Digit2) &&
         stroke.shift);
  assert(enUs.encode(U'"', stroke) &&
         stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::Quote) &&
         stroke.shift);
  assert(!enUs.encode(U'¥', stroke)); // Yen is not typable on en_us

  esp32keybridge::HostLayout jaJp = esp32keybridge::HostLayout::jaJp();
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
  assert(esp32keybridge::HostLayout::byName("ja_jp", &found).encode(U'@', stroke));
  assert(found);
  assert(stroke.key == esp32keybridge::keyboardKey(esp32keybridge::KeyboardUsage::LeftBracket));
  esp32keybridge::HostLayout::byName("xx_xx", &found);
  assert(!found);
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
  assert(bridge.typeText("a\r\nb"));
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

// UC5 setup: US-engraved keyboard on a ja_jp host.
static void configureUsOnJa(esp32keybridge::ESP32KeyBridgeConfig &config)
{
  config.convertLayout(0, esp32keybridge::HostLayout::enUs());
  config.hostLayout = esp32keybridge::HostLayout::jaJp();
}

static void test_layout_conversion_suppresses_shift()
{
  esp32keybridge::ESP32KeyBridge bridge;
  esp32keybridge::ManualInputAdapter keyboard;
  assert(bridge.addInput(keyboard));

  esp32keybridge::ESP32KeyBridgeConfig config;
  configureUsOnJa(config);
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
  assert(bridge.addInput(keyboard));

  esp32keybridge::ESP32KeyBridgeConfig config;
  configureUsOnJa(config);
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
  assert(bridge.addInput(keyboard));

  esp32keybridge::ESP32KeyBridgeConfig config;
  configureUsOnJa(config);
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
  assert(bridge.addInput(keyboard));

  esp32keybridge::ESP32KeyBridgeConfig config;
  configureUsOnJa(config);
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
  assert(bridge.addInput(keyboard));

  esp32keybridge::ESP32KeyBridgeConfig config;
  configureUsOnJa(config);
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
  assert(bridge.addInput(keyboard));

  // Reverse direction: JIS-engraved keyboard on an en_us host. Yen cannot
  // be typed on en_us: the press is dropped and counted.
  esp32keybridge::ESP32KeyBridgeConfig config;
  config.convertLayout(0, esp32keybridge::HostLayout::jaJp());
  config.hostLayout = esp32keybridge::HostLayout::enUs();
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
  assert(bridge.addInput(keyboard));

  esp32keybridge::ESP32KeyBridgeConfig config;
  configureUsOnJa(config);
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
  run("global_remap_swaps_without_chaining", test_global_remap_swaps_without_chaining);
  run("remap_crosses_key_kinds", test_remap_crosses_key_kinds);
  run("global_disable_drops_key", test_global_disable_drops_key);
  run("per_input_remap_applies_to_one_input", test_per_input_remap_applies_to_one_input);
  run("explicit_config_index_binding", test_explicit_config_index_binding);
  run("momentary_layer_with_press_time_resolution", test_momentary_layer_with_press_time_resolution);
  run("layer_trigger_and_key_in_same_update", test_layer_trigger_and_key_in_same_update);
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
  run("typing_produces_atomic_frames", test_typing_produces_atomic_frames);
  run("typing_parks_user_modifiers", test_typing_parks_user_modifiers);
  run("typing_defer_option_waits_for_modifiers", test_typing_defer_option_waits_for_modifiers);
  run("typing_caps_lock_compensation", test_typing_caps_lock_compensation);
  run("typing_control_chars_and_crlf", test_typing_control_chars_and_crlf);
  run("typing_unencodable_chars_are_counted", test_typing_unencodable_chars_are_counted);
  run("text_macro_types_on_trigger", test_text_macro_types_on_trigger);
  run("relative_axes_scale_and_drain", test_relative_axes_scale_and_drain);
  run("layout_conversion_suppresses_shift", test_layout_conversion_suppresses_shift);
  run("layout_conversion_synthesizes_shift", test_layout_conversion_synthesizes_shift);
  run("layout_conversion_letters_stay_identity", test_layout_conversion_letters_stay_identity);
  run("layout_conversion_shortcut_passthrough", test_layout_conversion_shortcut_passthrough);
  run("layout_conversion_shift_passes_for_nonprintable", test_layout_conversion_shift_passes_for_nonprintable);
  run("layout_conversion_untypable_is_dropped", test_layout_conversion_untypable_is_dropped);
  run("layout_conversion_toggle_key", test_layout_conversion_toggle_key);
  Serial.print("TEST done ");
  Serial.print(g_total);
  Serial.print("/");
  Serial.println(g_total);
}

void loop() {}
