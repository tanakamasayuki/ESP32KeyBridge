#pragma once

#include <stddef.h>
#include <stdint.h>

// ESP32KeyBridge core.
//
// The core is portable C++ and depends only on <stdint.h> / <stddef.h>.
// It never reads a clock, never allocates dynamically, and is driven from a
// single execution context via update(). Platform specific code lives in
// adapters. See docs/DATA_MODEL.ja.md and docs/CORE_DESIGN.ja.md.

namespace esp32keybridge
{

// ---------------------------------------------------------------------------
// Key identity: kind + code
// ---------------------------------------------------------------------------

enum class KeyKind : uint8_t
{
  None = 0,
  Keyboard = 1,    // HID keyboard page (0x07). code is numerically the Usage ID.
  Consumer = 2,    // HID consumer page (0x0C). code is numerically the Usage ID.
  MouseButton = 3, // Mouse button number, 1-origin.
  Virtual = 4,     // Bridge internal keys. Never emitted to outputs.
};

struct Key
{
  KeyKind kind = KeyKind::None;
  uint16_t code = 0;

  constexpr Key() = default;
  constexpr Key(KeyKind keyKind, uint16_t keyCode) : kind(keyKind), code(keyCode) {}

  constexpr bool operator==(const Key &other) const
  {
    return kind == other.kind && code == other.code;
  }

  constexpr bool operator!=(const Key &other) const
  {
    return !(*this == other);
  }
};

// HID keyboard page usage IDs. Names follow the US legends used by the HID
// usage table, but a value always identifies the physical key position and
// never a character.
enum class KeyboardUsage : uint16_t
{
  None = 0x00,
  A = 0x04,
  B = 0x05,
  C = 0x06,
  D = 0x07,
  E = 0x08,
  F = 0x09,
  G = 0x0a,
  H = 0x0b,
  I = 0x0c,
  J = 0x0d,
  K = 0x0e,
  L = 0x0f,
  M = 0x10,
  N = 0x11,
  O = 0x12,
  P = 0x13,
  Q = 0x14,
  R = 0x15,
  S = 0x16,
  T = 0x17,
  U = 0x18,
  V = 0x19,
  W = 0x1a,
  X = 0x1b,
  Y = 0x1c,
  Z = 0x1d,
  Digit1 = 0x1e,
  Digit2 = 0x1f,
  Digit3 = 0x20,
  Digit4 = 0x21,
  Digit5 = 0x22,
  Digit6 = 0x23,
  Digit7 = 0x24,
  Digit8 = 0x25,
  Digit9 = 0x26,
  Digit0 = 0x27,
  Enter = 0x28,
  Escape = 0x29,
  Backspace = 0x2a,
  Tab = 0x2b,
  Space = 0x2c,
  Minus = 0x2d,
  Equal = 0x2e,
  LeftBracket = 0x2f,
  RightBracket = 0x30,
  Backslash = 0x31,
  NonUsHash = 0x32,
  Semicolon = 0x33,
  Quote = 0x34,
  Grave = 0x35,
  Comma = 0x36,
  Period = 0x37,
  Slash = 0x38,
  CapsLock = 0x39,
  F1 = 0x3a,
  F2 = 0x3b,
  F3 = 0x3c,
  F4 = 0x3d,
  F5 = 0x3e,
  F6 = 0x3f,
  F7 = 0x40,
  F8 = 0x41,
  F9 = 0x42,
  F10 = 0x43,
  F11 = 0x44,
  F12 = 0x45,
  PrintScreen = 0x46,
  ScrollLock = 0x47,
  Pause = 0x48,
  Insert = 0x49,
  Home = 0x4a,
  PageUp = 0x4b,
  Delete = 0x4c,
  End = 0x4d,
  PageDown = 0x4e,
  Right = 0x4f,
  Left = 0x50,
  Down = 0x51,
  Up = 0x52,
  NumLock = 0x53,
  KeypadDivide = 0x54,
  KeypadMultiply = 0x55,
  KeypadMinus = 0x56,
  KeypadPlus = 0x57,
  KeypadEnter = 0x58,
  Keypad1 = 0x59,
  Keypad2 = 0x5a,
  Keypad3 = 0x5b,
  Keypad4 = 0x5c,
  Keypad5 = 0x5d,
  Keypad6 = 0x5e,
  Keypad7 = 0x5f,
  Keypad8 = 0x60,
  Keypad9 = 0x61,
  Keypad0 = 0x62,
  KeypadPeriod = 0x63,
  NonUsBackslash = 0x64,
  Application = 0x65,
  Power = 0x66,
  KeypadEqual = 0x67,
  F13 = 0x68,
  F14 = 0x69,
  F15 = 0x6a,
  F16 = 0x6b,
  F17 = 0x6c,
  F18 = 0x6d,
  F19 = 0x6e,
  F20 = 0x6f,
  F21 = 0x70,
  F22 = 0x71,
  F23 = 0x72,
  F24 = 0x73,
  International1 = 0x87, // JIS Ro
  International2 = 0x88, // JIS Katakana/Hiragana
  International3 = 0x89, // JIS Yen
  International4 = 0x8a, // JIS Henkan
  International5 = 0x8b, // JIS Muhenkan
  International6 = 0x8c,
  International7 = 0x8d,
  International8 = 0x8e,
  International9 = 0x8f,
  Lang1 = 0x90, // Korean Hangul/English, Mac JIS Kana
  Lang2 = 0x91, // Korean Hanja, Mac JIS Eisu
  Lang3 = 0x92,
  Lang4 = 0x93,
  Lang5 = 0x94,
  Lang6 = 0x95,
  Lang7 = 0x96,
  Lang8 = 0x97,
  Lang9 = 0x98,
  LeftCtrl = 0xe0,
  LeftShift = 0xe1,
  LeftAlt = 0xe2,
  LeftGui = 0xe3,
  RightCtrl = 0xe4,
  RightShift = 0xe5,
  RightAlt = 0xe6,
  RightGui = 0xe7,
};

// HID consumer page usage IDs.
enum class ConsumerUsage : uint16_t
{
  None = 0x0000,
  Power = 0x0030,
  Sleep = 0x0032,
  Menu = 0x0040,
  BrightnessIncrement = 0x006f,
  BrightnessDecrement = 0x0070,
  Play = 0x00b0,
  Pause = 0x00b1,
  Record = 0x00b2,
  FastForward = 0x00b3,
  Rewind = 0x00b4,
  ScanNextTrack = 0x00b5,
  ScanPreviousTrack = 0x00b6,
  Stop = 0x00b7,
  Eject = 0x00b8,
  PlayPause = 0x00cd,
  Mute = 0x00e2,
  VolumeIncrement = 0x00e9,
  VolumeDecrement = 0x00ea,
  BrowserSearch = 0x0221,
  BrowserHome = 0x0223,
  BrowserBack = 0x0224,
  BrowserForward = 0x0225,
  BrowserRefresh = 0x0227,
  BrowserBookmarks = 0x022a,
};

constexpr Key keyboardKey(KeyboardUsage usage)
{
  return Key(KeyKind::Keyboard, static_cast<uint16_t>(usage));
}

constexpr Key keyboardKey(uint16_t usage)
{
  return Key(KeyKind::Keyboard, usage);
}

constexpr Key consumerKey(ConsumerUsage usage)
{
  return Key(KeyKind::Consumer, static_cast<uint16_t>(usage));
}

constexpr Key consumerKey(uint16_t usage)
{
  return Key(KeyKind::Consumer, usage);
}

constexpr Key mouseButtonKey(uint16_t button)
{
  return Key(KeyKind::MouseButton, button);
}

constexpr Key virtualKey(uint16_t code)
{
  return Key(KeyKind::Virtual, code);
}

bool isValid(Key key);
const char *keyKindName(KeyKind kind);

// ---------------------------------------------------------------------------
// Keyboard modifier normalization
// ---------------------------------------------------------------------------
//
// Modifiers are ordinary keyboard keys (0xE0-0xE7) inside the core. These
// helpers convert between the HID report modifier bitmask and Key values so
// adapters can normalize reports on input and rebuild them on output.

bool isKeyboardModifier(Key key);

// Returns the modifier bitmask bit (0x01..0x80) for a modifier key, 0 otherwise.
uint8_t keyboardModifierMask(Key key);

// Returns the modifier key for a bit index (0..7 -> LeftCtrl..RightGui).
Key keyboardModifierFromBitIndex(uint8_t bitIndex);

// ---------------------------------------------------------------------------
// KeySet: set of currently pressed keys (held state)
// ---------------------------------------------------------------------------

class KeySet
{
public:
  static constexpr size_t MaxKeys = 32;

  void clear();

  // Adds a key. Returns false only when the set is full (already-pressed keys
  // return true without duplication).
  bool press(Key key);

  // Removes a key. Returns true when the key was present.
  bool release(Key key);

  bool contains(Key key) const;

  // Union with another set. Returns false when keys were dropped because this
  // set became full.
  bool mergeFrom(const KeySet &other);

  size_t count() const;
  Key at(size_t index) const;

  // Builds the HID modifier bitmask from pressed modifier keys.
  uint8_t keyboardModifierMask() const;

  // Presses modifier keys for every bit set in the mask. Returns false when
  // the set became full.
  bool pressKeyboardModifiers(uint8_t mask);

private:
  Key keys_[MaxKeys] = {};
  size_t count_ = 0;
};

// ---------------------------------------------------------------------------
// Input adapters
// ---------------------------------------------------------------------------

class InputAdapter
{
public:
  virtual ~InputAdapter() = default;

  // Called once per bridge update. Adapters read their hardware here.
  virtual void update() = 0;

  // Currently pressed keys of this input (logical presses; adapters may
  // synthesize toggles, one-shots, or pulses that do not match physical
  // switch state).
  virtual const KeySet &keys() const = 0;

  // While false, the bridge treats this input as absent: its keys drop out of
  // the merged state (all keys released). Adapters report disconnection here.
  virtual bool connected() const { return true; }
};

// Minimal input adapter driven by direct press/release calls. Used by unit
// tests, examples, and sketch-level custom inputs (buttons, pedals).
class ManualInputAdapter : public InputAdapter
{
public:
  void update() override {}
  const KeySet &keys() const override { return keys_; }
  bool connected() const override { return connected_; }

  bool press(Key key) { return keys_.press(key); }
  bool release(Key key) { return keys_.release(key); }
  void clear() { keys_.clear(); }
  void setConnected(bool connected) { connected_ = connected; }

private:
  KeySet keys_;
  bool connected_ = true;
};

// ---------------------------------------------------------------------------
// Transform configuration
// ---------------------------------------------------------------------------

struct KeyRemap
{
  Key from;
  Key to;
};

// Remap and disable table. remap() entries are a single-step lookup and never
// chain: registering A->B and B->A swaps the two keys. Entries may cross key
// kinds (keyboard key -> consumer key, any key -> virtual key, ...).
class TransformConfig
{
public:
  static constexpr size_t MaxRemaps = 32;
  static constexpr size_t MaxDisabledKeys = 32;

  bool remap(Key from, Key to);
  bool disable(Key key);
  void clear();

  Key map(Key key) const;
  bool isDisabled(Key key) const;
  bool empty() const;

private:
  KeyRemap remaps_[MaxRemaps] = {};
  size_t remapCount_ = 0;
  Key disabled_[MaxDisabledKeys] = {};
  size_t disabledCount_ = 0;
};

// Momentary layer: while the trigger key is held (after per-input and global
// remap), the layer's remap table overlays the resolution of new presses.
// The trigger key itself is consumed and never emitted.
class LayerConfig
{
public:
  static constexpr size_t MaxRemaps = 16;

  void setTrigger(Key trigger);
  bool remap(Key from, Key to);
  void clear();

  bool enabled() const;
  Key trigger() const;

  // Returns the overlay mapping, or the key itself when no entry exists.
  Key map(Key key) const;
  bool hasMapping(Key key) const;

private:
  Key trigger_;
  KeyRemap remaps_[MaxRemaps] = {};
  size_t remapCount_ = 0;
};

class ESP32KeyBridgeConfig
{
public:
  static constexpr size_t MaxInputConfigs = 8;
  static constexpr size_t MaxLayers = 4;

  // Per-input transform, addressed by config slot (see addInput()). An
  // out-of-range index returns a writable dummy that is never applied.
  TransformConfig &input(size_t index);
  const TransformConfig &input(size_t index) const;

  LayerConfig &layer(size_t index = 0);
  const LayerConfig &layer(size_t index = 0) const;

  void clear();

  TransformConfig global;

private:
  TransformConfig inputs_[MaxInputConfigs];
  LayerConfig layers_[MaxLayers];
  TransformConfig dummyInput_;
  LayerConfig dummyLayer_;
};

struct ESP32KeyBridgeConfigError
{
  const char *message = nullptr;
};

// ---------------------------------------------------------------------------
// Bridge
// ---------------------------------------------------------------------------
//
// Key resolution is fixed at press time: when a key appears on an input, it is
// resolved once through per-input remap -> global remap -> layer overlay ->
// disable, and the result is held until that key is released. Layer changes or
// applyConfig() while a key is held never re-resolve it, so releases always
// release exactly what was pressed (no stuck keys).
//
// Within one update, releases are processed before presses, and presses that
// resolve to a layer trigger are processed before other presses (a trigger and
// a target key arriving in the same update behave as trigger-first).

class ESP32KeyBridge
{
public:
  static constexpr size_t MaxInputs = 8;
  static constexpr size_t MaxHeldKeys = 64;

  // Registers an input. The config slot defaults to the registration order;
  // the second form binds an explicit slot of ESP32KeyBridgeConfig::input().
  bool addInput(InputAdapter &input);
  bool addInput(InputAdapter &input, size_t configIndex);
  void clearInputs();
  size_t inputCount() const;

  bool validateConfig(const ESP32KeyBridgeConfig &config, ESP32KeyBridgeConfigError &error) const;
  void applyConfig(const ESP32KeyBridgeConfig &config);

  void begin();

  // Updates all inputs, tracks press/release transitions, and rebuilds both
  // key sets. A key stays pressed until every input that holds it releases
  // it; a disconnected input contributes nothing.
  void update();

  // Raw union of connected inputs' keys, before any transform.
  const KeySet &mergedKeys() const;

  // Transformed result: resolved keys of all held presses. Virtual keys that
  // were not consumed as layer triggers remain here; output adapters emit
  // only what they can represent.
  const KeySet &outputKeys() const;

  // True when the last update dropped keys (merged set full).
  bool mergedOverflow() const;

  // True when the last update dropped transformed keys (held table or output
  // set full). Dropped keys are never emitted, so they cannot get stuck.
  bool outputOverflow() const;

private:
  struct HeldKey
  {
    uint8_t inputIndex = 0;
    Key source;
    Key resolved;
    uint8_t layerTriggerMask = 0;
  };

  size_t findHeld(uint8_t inputIndex, Key source) const;
  void removeHeld(size_t index);
  uint8_t layerTriggerMaskFor(Key key) const;
  Key applyLayerOverlay(Key key) const;
  void resolvePress(uint8_t inputIndex, Key source, bool triggersOnly);

  InputAdapter *inputs_[MaxInputs] = {};
  size_t inputConfigIndexes_[MaxInputs] = {};
  size_t inputCount_ = 0;
  ESP32KeyBridgeConfig config_;
  HeldKey held_[MaxHeldKeys] = {};
  size_t heldCount_ = 0;
  uint16_t layerHoldCounts_[ESP32KeyBridgeConfig::MaxLayers] = {};
  KeySet merged_;
  KeySet output_;
  bool mergedOverflow_ = false;
  bool outputOverflow_ = false;
};

} // namespace esp32keybridge
