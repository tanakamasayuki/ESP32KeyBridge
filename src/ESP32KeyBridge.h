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

// Predefined virtual key slots. Virtual keys exist only inside the bridge
// (layer triggers, macro triggers, mode toggles) and carry no meaning of
// their own — alias a slot with a name in the sketch. virtualKey(n)
// accepts any code from 1 to 65535 when the predefined slots are not
// enough.
enum class VirtualUsage : uint16_t
{
  None = 0,
  V1 = 1,
  V2 = 2,
  V3 = 3,
  V4 = 4,
  V5 = 5,
  V6 = 6,
  V7 = 7,
  V8 = 8,
  V9 = 9,
  V10 = 10,
  V11 = 11,
  V12 = 12,
  V13 = 13,
  V14 = 14,
  V15 = 15,
  V16 = 16,
};

// HID button page usage IDs as mouse buttons (1-origin, matching the
// MouseButton key kind). mouseButtonKey(n) accepts any button number when
// a name is not defined here.
enum class MouseUsage : uint16_t
{
  None = 0,
  Left = 1,
  Right = 2,
  Middle = 3,
  Back = 4,
  Forward = 5,
  Button6 = 6,
  Button7 = 7,
  Button8 = 8,
};

struct Key
{
  KeyKind kind = KeyKind::None;
  uint16_t code = 0;

  constexpr Key() = default;
  constexpr Key(KeyKind keyKind, uint16_t keyCode) : kind(keyKind), code(keyCode) {}

  // The usage enums determine the kind, so they convert to Key directly.
  // Raw numbers beyond the named values use mouseButtonKey() /
  // virtualKey() instead.
  constexpr Key(KeyboardUsage usage)
      : kind(KeyKind::Keyboard), code(static_cast<uint16_t>(usage))
  {
  }

  constexpr Key(ConsumerUsage usage)
      : kind(KeyKind::Consumer), code(static_cast<uint16_t>(usage))
  {
  }

  constexpr Key(MouseUsage usage)
      : kind(KeyKind::MouseButton), code(static_cast<uint16_t>(usage))
  {
  }

  constexpr Key(VirtualUsage usage)
      : kind(KeyKind::Virtual), code(static_cast<uint16_t>(usage))
  {
  }

  constexpr bool operator==(const Key &other) const
  {
    return kind == other.kind && code == other.code;
  }

  constexpr bool operator!=(const Key &other) const
  {
    return !(*this == other);
  }
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

constexpr Key mouseButtonKey(MouseUsage usage)
{
  return Key(KeyKind::MouseButton, static_cast<uint16_t>(usage));
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
// Relative axes (one-shot events with a delta payload)
// ---------------------------------------------------------------------------

enum class Axis : uint8_t
{
  X = 0,
  Y = 1,
  Wheel = 2,
  Pan = 3,
};

constexpr size_t kAxisCount = 4;

// ---------------------------------------------------------------------------
// Keyboard layout (character <-> key + shift table). Role-neutral: the same
// description serves as an input device's engraving (InputConfig) and as
// the declaration of how a host interprets keys (config.hostLayout).
// ---------------------------------------------------------------------------
//
// Used to synthesize keystrokes from characters (text macros, serial text).
// Digits map to the main row only, so encoded strokes never depend on
// NumLock. Caps Lock compensation is applied by the typing engine, not here.

struct KeyStroke
{
  Key key;
  bool shift = false;
};

struct KeyboardLayoutEntry
{
  uint16_t usage; // KeyboardUsage value
  uint16_t base;  // codepoint without shift (0 = none)
  uint16_t shift; // codepoint with shift (0 = none)
  bool capsAffects;
};

class KeyboardLayout
{
public:
  KeyboardLayout(); // defaults to en_us

  static KeyboardLayout enUs();
  static KeyboardLayout jaJp();
  // Lookup by locale name ("en_us", "ja_jp"). Returns en_us when unknown and
  // sets *found to false if provided.
  static KeyboardLayout byName(const char *name, bool *found = nullptr);

  // Encodes a printable character. Returns false when this layout cannot
  // type the character. Control characters are handled by the typing engine.
  bool encode(char32_t codepoint, KeyStroke &stroke) const;

  // Decodes a key press under this layout (as engraved on a keyboard, or as
  // interpreted by a host). Returns 0 when the key produces no character in
  // that plane.
  char32_t decode(Key key, bool shift) const;

  // True when Caps Lock inverts the shift requirement of this stroke's key.
  bool capsAffects(Key key) const;

  const char *name() const;

private:
  KeyboardLayout(const KeyboardLayoutEntry *entries, size_t count, const char *layoutName);

  const KeyboardLayoutEntry *entries_;
  size_t count_;
  const char *name_;
};

// ---------------------------------------------------------------------------
// Lock state
// ---------------------------------------------------------------------------
//
// Lock state is owned by whoever interprets it. While a lock-reporting output
// (USB device / BLE HID connected to a host) is present, that host is the
// authority and the bridge only relays. Without one, the bridge acts as the
// terminal host and toggles Caps/Num/Scroll on lock key presses itself. The
// bridge always keeps one internal shadow copy and notifies every input
// adapter (keyboard LEDs, GPIO pins) when it changes.

struct LockState
{
  bool numLock = false;
  bool capsLock = false;
  bool scrollLock = false;
  bool kana = false; // JIS Kana LED. Forwarded only; never toggled by the bridge.

  constexpr bool operator==(const LockState &other) const
  {
    return numLock == other.numLock && capsLock == other.capsLock &&
           scrollLock == other.scrollLock && kana == other.kana;
  }

  constexpr bool operator!=(const LockState &other) const
  {
    return !(*this == other);
  }
};

// ---------------------------------------------------------------------------
// HID report builders (pure functions; descriptors and transmission are the
// output adapter's business)
// ---------------------------------------------------------------------------

// USB boot keyboard report: modifier byte + up to 6 key usages.
struct HidKeyboardReport
{
  static constexpr size_t MaxKeys = 6;
  static constexpr size_t BootReportSize = 8;

  uint8_t modifiers = 0;
  uint8_t keys[MaxKeys] = {};
  size_t keyCount = 0;
  bool overflow = false;

  void clear();
  bool empty() const;
  bool writeBootReport(uint8_t *buffer, size_t size) const;
};

// Rollover keyboard report: modifier byte + up to 32 key usages.
struct HidKeyboardRolloverReport
{
  static constexpr size_t MaxKeys = 32;
  static constexpr size_t ReportSize = 33;

  uint8_t modifiers = 0;
  uint8_t keys[MaxKeys] = {};
  size_t keyCount = 0;
  bool overflow = false;

  void clear();
  bool empty() const;
  bool writeReport(uint8_t *buffer, size_t size) const;
};

// Minimal consumer report: one 16-bit usage (little endian). Multiple
// simultaneous consumer keys set overflow; richer report formats are an
// output adapter extension.
struct HidConsumerReport
{
  static constexpr size_t ReportSize = 2;

  uint16_t usage = 0;
  bool overflow = false;

  void clear();
  bool empty() const;
  bool writeReport(uint8_t *buffer, size_t size) const;
};

// Relative mouse report: 8 button bits + X/Y/Wheel/Pan deltas.
struct HidMouseReport
{
  static constexpr size_t MaxButtons = 8;
  static constexpr size_t ReportSize = 5;

  uint8_t buttons = 0;
  int8_t x = 0;
  int8_t y = 0;
  int8_t wheel = 0;
  int8_t pan = 0;
  bool overflow = false;

  void clear();
  bool empty() const;

  // Adds a delta to an axis field, saturating to the int8 report range, and
  // returns the remainder to carry into the next report.
  int32_t applyAxisDelta(Axis axis, int32_t delta);

  bool writeReport(uint8_t *buffer, size_t size) const;
};

// Builders skip keys their report cannot represent (other kinds, virtual
// keys) per the "emit what you can represent" rule.
HidKeyboardReport buildHidKeyboardReport(const KeySet &keys);
HidKeyboardRolloverReport buildHidKeyboardRolloverReport(const KeySet &keys);
HidConsumerReport buildHidConsumerReport(const KeySet &keys);
HidMouseReport buildHidMouseReport(const KeySet &keys);

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

  // Called when the bridge's lock state changes and once after this input is
  // added or reconnects. Adapters reflect it to their device (USB LED report,
  // GPIO pins, BLE output report). Default: ignore.
  virtual void setLockState(const LockState &state) { (void)state; }

  // Returns the accumulated relative delta of an axis since the last call
  // and resets it (mouse movement, wheel, encoder rotation). Default: none.
  virtual int32_t takeAxisDelta(Axis axis)
  {
    (void)axis;
    return 0;
  }
};

// Minimal input adapter driven by direct press/release calls. Used by unit
// tests, examples, and sketch-level custom inputs (buttons, pedals).
class ManualInputAdapter : public InputAdapter
{
public:
  void update() override {}
  const KeySet &keys() const override { return keys_; }
  bool connected() const override { return connected_; }
  void setLockState(const LockState &state) override
  {
    lockState_ = state;
    ++lockStateCount_;
  }

  int32_t takeAxisDelta(Axis axis) override
  {
    const size_t index = static_cast<size_t>(axis);
    const int32_t delta = axisDeltas_[index];
    axisDeltas_[index] = 0;
    return delta;
  }

  bool press(Key key) { return keys_.press(key); }
  bool release(Key key) { return keys_.release(key); }
  void clear() { keys_.clear(); }
  void setConnected(bool connected) { connected_ = connected; }
  void addAxisDelta(Axis axis, int32_t delta) { axisDeltas_[static_cast<size_t>(axis)] += delta; }

  // Last lock state received from the bridge (LED view of this input).
  const LockState &lockState() const { return lockState_; }
  size_t lockStateCount() const { return lockStateCount_; }

private:
  KeySet keys_;
  bool connected_ = true;
  LockState lockState_;
  size_t lockStateCount_ = 0;
  int32_t axisDeltas_[kAxisCount] = {};
};

// ---------------------------------------------------------------------------
// Output adapters
// ---------------------------------------------------------------------------

class OutputAdapter
{
public:
  virtual ~OutputAdapter() = default;

  // Receives the transformed key set once per bridge update. Adapters emit
  // what they can represent and silently drop the rest (e.g. virtual keys).
  virtual void write(const KeySet &keys) = 0;

  // While false, the bridge does not write to this output and it cannot act
  // as the lock authority.
  virtual bool connected() const { return true; }

  // Lock-reporting outputs (USB device / BLE HID receiving LED output
  // reports from a host) return true and provide the host's lock state.
  virtual bool reportsLockState() const { return false; }
  virtual bool getLockState(LockState &out) const
  {
    (void)out;
    return false;
  }

  // Receives every character consumed from the text stream, including ones
  // the host layout cannot type. Text-native outputs (UART log, network)
  // take them directly; HID outputs rely on the synthesized keystrokes in
  // write(). Default: ignore.
  virtual void writeText(char32_t codepoint) { (void)codepoint; }

  // Receives non-zero relative axis totals once per update (after scaling).
  // Report packing, saturation, and carry are the adapter's business.
  virtual void writeAxisDelta(Axis axis, int32_t delta)
  {
    (void)axis;
    (void)delta;
  }
};

// Minimal output adapter that records what the bridge writes and lets tests
// or sketches simulate a lock-reporting host. Counterpart of
// ManualInputAdapter.
class ManualOutputAdapter : public OutputAdapter
{
public:
  void write(const KeySet &keys) override
  {
    keys_ = keys;
    ++writeCount_;
  }
  bool connected() const override { return connected_; }
  bool reportsLockState() const override { return reportsLockState_; }
  bool getLockState(LockState &out) const override
  {
    if (!reportsLockState_)
    {
      return false;
    }
    out = hostLockState_;
    return true;
  }

  void writeText(char32_t codepoint) override
  {
    lastText_ = codepoint;
    ++textCount_;
  }
  void writeAxisDelta(Axis axis, int32_t delta) override
  {
    axisTotals_[static_cast<size_t>(axis)] += delta;
  }

  const KeySet &keys() const { return keys_; }
  size_t writeCount() const { return writeCount_; }
  char32_t lastText() const { return lastText_; }
  size_t textCount() const { return textCount_; }
  int32_t axisTotal(Axis axis) const { return axisTotals_[static_cast<size_t>(axis)]; }
  void clear()
  {
    keys_.clear();
    writeCount_ = 0;
    lastText_ = 0;
    textCount_ = 0;
    for (size_t i = 0; i < kAxisCount; ++i)
    {
      axisTotals_[i] = 0;
    }
  }

  void setConnected(bool connected) { connected_ = connected; }
  // Simulates a host that reports lock state via LED output reports.
  void setHostLockState(const LockState &state)
  {
    reportsLockState_ = true;
    hostLockState_ = state;
  }

private:
  KeySet keys_;
  size_t writeCount_ = 0;
  bool connected_ = true;
  bool reportsLockState_ = false;
  LockState hostLockState_;
  char32_t lastText_ = 0;
  size_t textCount_ = 0;
  int32_t axisTotals_[kAxisCount] = {};
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

// Text macro: pressing the trigger key (after remap) enqueues the stored
// characters into the text stream. The trigger itself is consumed.
struct TextMacro
{
  static constexpr size_t MaxLength = 31;

  Key trigger;
  char32_t text[MaxLength] = {};
  size_t length = 0;
};

// Settings for one input: transforms and keyboard layout conversion.
// Created by ESP32KeyBridgeConfig::addInputConfig() (numbering is
// automatic) and bound to an input with ESP32KeyBridge::addInput(input,
// inputConfig). Binding the same InputConfig to several inputs shares the
// settings.
class InputConfig
{
public:
  // Single-step remap / disable applied to keys of the bound inputs only,
  // before the global transform.
  bool remap(Key from, Key to) { return transform_.remap(from, to); }
  bool disable(Key key) { return transform_.disable(key); }

  // Enables keyboard layout conversion for the bound inputs: printable
  // keys are decoded with the engraving layout (using the input's own
  // Shift, which is consumed) and re-encoded with
  // ESP32KeyBridgeConfig::hostLayout, synthesizing Shift as needed.
  // Non-printable keys and Ctrl/Alt/GUI shortcuts pass through.
  void convertLayout(const KeyboardLayout &engraving)
  {
    engraving_ = engraving;
    convertsLayout_ = true;
  }

  const TransformConfig &transform() const { return transform_; }
  bool convertsLayout() const { return convertsLayout_; }
  const KeyboardLayout &engraving() const { return engraving_; }

  void clear()
  {
    transform_.clear();
    engraving_ = KeyboardLayout();
    convertsLayout_ = false;
  }

private:
  friend class ESP32KeyBridgeConfig;
  friend class ESP32KeyBridge;

  static constexpr uint8_t kUnboundSlot = 0xff;

  TransformConfig transform_;
  KeyboardLayout engraving_;
  bool convertsLayout_ = false;
  uint8_t slot_ = kUnboundSlot; // assigned by addInputConfig()
};

class ESP32KeyBridgeConfig
{
public:
  static constexpr size_t MaxInputConfigs = 8;
  static constexpr size_t MaxLayers = 4;
  static constexpr size_t MaxTextMacros = 4;

  // Creates per-input settings; numbering is automatic. Bind the result
  // with ESP32KeyBridge::addInput(input, inputConfig). When all
  // MaxInputConfigs slots are in use, returns a writable dummy that is
  // never applied.
  InputConfig &addInputConfig();

  // Adds a momentary layer triggered by the given key and returns it for
  // remap registration. Returns a writable dummy that is never applied
  // when all layer slots (MaxLayers) are in use.
  LayerConfig &addLayer(Key trigger);

  // Slot access, mainly for inspection; addLayer() is the usual entry.
  LayerConfig &layer(size_t index = 0);
  const LayerConfig &layer(size_t index = 0) const;

  // Registers a text macro (UTF-8). Returns false when the macro table is
  // full or the text is too long / invalid.
  bool textMacro(Key trigger, const char *utf8);
  const TextMacro *findTextMacro(Key trigger) const;

  // Relative axis transform: negative scale inverts (natural scrolling),
  // magnitude multiplies. Default 1.
  void setAxisScale(Axis axis, int16_t scale);
  int16_t axisScale(Axis axis) const;

  // Keyboard layout conversion is a per-input setting: see
  // InputConfig::convertLayout(). Pressing this key (after remap) toggles
  // the conversion on/off at runtime and is consumed. Required in
  // practice: environments that interpret keys differently (BIOS,
  // recovery) need a bypass.
  Key layoutConversionToggle;

  void clear();

  TransformConfig global;

  // Layout of the host the output is connected to; used to synthesize
  // keystrokes from characters and as the target of layout conversion.
  // Default: en_us.
  KeyboardLayout hostLayout;

  // When true, text typing waits until no keyboard modifier held by any
  // input is present, instead of temporarily releasing them per character.
  bool deferTypingWhileModifiersHeld = false;

private:
  friend class ESP32KeyBridge;

  InputConfig inputConfigs_[MaxInputConfigs];
  size_t inputConfigCount_ = 0;
  LayerConfig layers_[MaxLayers];
  TextMacro textMacros_[MaxTextMacros];
  size_t textMacroCount_ = 0;
  int16_t axisScales_[kAxisCount] = {1, 1, 1, 1};
  InputConfig dummyInputConfig_;
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
  static constexpr size_t MaxOutputs = 4;
  static constexpr size_t MaxHeldKeys = 64;
  static constexpr size_t MaxTextQueue = 64;

  // Registers an input. The second form binds the input to per-input
  // settings created with ESP32KeyBridgeConfig::addInputConfig(); binding
  // the same InputConfig to several inputs shares the settings. Only the
  // handle's identity is taken here — the settings' contents take effect
  // when applyConfig() copies the configuration, so they may be written
  // before or after this call. The first form registers the input with no
  // per-input settings.
  bool addInput(InputAdapter &input);
  bool addInput(InputAdapter &input, const InputConfig &inputConfig);
  void clearInputs();
  size_t inputCount() const;

  bool addOutput(OutputAdapter &output);
  void clearOutputs();
  size_t outputCount() const;

  bool validateConfig(const ESP32KeyBridgeConfig &config, ESP32KeyBridgeConfigError &error) const;
  void applyConfig(const ESP32KeyBridgeConfig &config);

  // There is no begin() and no ordering between addInput/addOutput,
  // applyConfig, and adapter startup: registration and configuration only
  // store state, and everything starts working once update() is called.

  // Updates all inputs, tracks press/release transitions, rebuilds both key
  // sets, maintains the lock state, and writes the result to all connected
  // outputs. A key stays pressed until every input that holds it releases
  // it; a disconnected input contributes nothing.
  void update();

  // Internal lock state shadow. While a connected lock-reporting output is
  // present (the first registered one is the authority), it mirrors that
  // host. Otherwise the bridge is the terminal host and toggles
  // Caps/Num/Scroll on lock key presses in the output keys.
  const LockState &lockState() const;

  // True while the lock state follows an external authority.
  bool lockAuthorityPresent() const;

  // --- Text stream -------------------------------------------------------
  //
  // Characters are carried as characters and expanded into keystrokes at the
  // output edge using config.hostLayout. The typing engine emits one phase
  // per update (modifiers, key down, key up), keeps each character's frame
  // atomic, temporarily parks user-held keyboard modifiers, and applies Caps
  // Lock compensation from the lock state shadow.

  // Enqueues one character. Returns false when the queue was full (the
  // character is dropped and counted).
  bool typeChar(char32_t codepoint);

  // Enqueues a UTF-8 string and returns the number of bytes consumed.
  // When the queue fills up it stops without dropping anything; resume
  // later from utf8 + the returned offset. Nothing ever blocks — to "send
  // it all", pump update() between calls:
  //   const char *p = text;
  //   while (*p != '\0') { p += bridge.typeText(p); bridge.update(); }
  // Invalid UTF-8 bytes are the exception: they are consumed and counted
  // so a resume loop always makes progress.
  size_t typeText(const char *utf8);

  size_t textQueueLength() const;
  // Free space in the text queue: how many characters typeChar/typeText
  // can accept right now without dropping (Serial-style backpressure:
  // while (Serial.available() && bridge.typeAvailable()) ...).
  size_t typeAvailable() const;
  bool typingActive() const;
  // Characters dropped because the queue was full.
  uint32_t textOverflowCount() const;
  // Characters the host layout could not type (skipped, still sent to
  // writeText outputs).
  uint32_t textEncodeFailCount() const;

  // --- Relative axes ------------------------------------------------------

  // Total delta of the last update (sum over inputs, after axis scale).
  int32_t axisDelta(Axis axis) const;

  // --- Layout conversion --------------------------------------------------

  // Master switch (also flipped by config.layoutConversionToggle). Held keys
  // keep their press-time resolution when this changes.
  void setLayoutConversionEnabled(bool enabled);
  bool layoutConversionEnabled() const;

  // Presses whose character the host layout could not type (dropped).
  uint32_t layoutConvertFailCount() const;

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
    bool converted = false;     // produced by layout conversion
    bool requiresShift = false; // synthesized Shift while held (converted only)
  };

  size_t findHeld(uint8_t inputIndex, Key source) const;
  void removeHeld(size_t index);
  uint8_t layerTriggerMaskFor(Key key) const;
  Key applyLayerOverlay(Key key) const;
  void resolvePress(uint8_t inputIndex, Key source, bool triggersOnly);
  // Returns the bound per-input settings, or nullptr when the input has none.
  const InputConfig *perInputConfig(size_t inputIndex) const;
  void updateLockState();
  bool enqueueChar(char32_t codepoint);
  void enqueueMacro(const TextMacro &macro);
  bool encodeCharForTyping(char32_t codepoint, KeyStroke &stroke) const;
  void stepTyping();
  void updateAxes();

  InputAdapter *inputs_[MaxInputs] = {};
  uint8_t inputConfigSlots_[MaxInputs] = {}; // InputConfig::kUnboundSlot = none
  bool inputWasConnected_[MaxInputs] = {};
  size_t inputCount_ = 0;
  OutputAdapter *outputs_[MaxOutputs] = {};
  size_t outputCount_ = 0;
  ESP32KeyBridgeConfig config_;
  HeldKey held_[MaxHeldKeys] = {};
  size_t heldCount_ = 0;
  uint16_t layerHoldCounts_[ESP32KeyBridgeConfig::MaxLayers] = {};
  KeySet merged_;
  KeySet output_;
  bool mergedOverflow_ = false;
  bool outputOverflow_ = false;
  LockState lockState_;
  bool lockAuthorityPresent_ = false;
  bool prevLockKeyPressed_[3] = {}; // Caps, Num, Scroll in outputKeys()

  // Text stream ring buffer and typing engine state.
  char32_t textQueue_[MaxTextQueue] = {};
  size_t textHead_ = 0;
  size_t textCount_ = 0;
  bool lastEnqueuedCR_ = false;
  bool typingActive_ = false;
  uint8_t typingPhase_ = 0; // 0: modifiers, 1: modifiers+key, 2: modifiers only
  KeyStroke typingStroke_;
  uint32_t textOverflowCount_ = 0;
  uint32_t textEncodeFailCount_ = 0;

  int32_t axisDeltas_[kAxisCount] = {};

  bool layoutConversionEnabled_ = true;
  uint32_t layoutConvertFailCount_ = 0;
};

} // namespace esp32keybridge
