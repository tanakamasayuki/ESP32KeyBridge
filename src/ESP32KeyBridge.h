#pragma once

#include <stddef.h>
#include <stdint.h>

namespace esp32keybridge
{

enum class KeySymbol : uint16_t
{
  None = 0,
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  Num1,
  Num2,
  Num3,
  Num4,
  Num5,
  Num6,
  Num7,
  Num8,
  Num9,
  Num0,
  Enter,
  Escape,
  Backspace,
  Tab,
  Space,
  Minus,
  Equal,
  LeftBracket,
  RightBracket,
  Backslash,
  NonUsHash,
  Semicolon,
  Quote,
  Grave,
  Comma,
  Dot,
  Slash,
  CapsLock,
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,
  PrintScreen,
  ScrollLock,
  Pause,
  Insert,
  Home,
  PageUp,
  Delete,
  End,
  PageDown,
  Right,
  Left,
  Down,
  Up,
  NonUsBackslash,
  International1,
  International2,
  International3,
  International4,
  International5,
  International6,
  International7,
  International8,
  International9,
  Lang1,
  Lang2,
  Lang3,
  Lang4,
  Lang5,
  Lang6,
  Lang7,
  Lang8,
  Lang9,
  LeftCtrl,
  LeftShift,
  LeftAlt,
  LeftGui,
  RightCtrl,
  RightShift,
  RightAlt,
  RightGui,
  Fn1 = 0x0100,
};

enum class HidUsage : uint16_t
{
  None = 0,
  Usage04 = 0x04,
  Usage05 = 0x05,
  Usage06 = 0x06,
  Usage07 = 0x07,
  Usage08 = 0x08,
  Usage09 = 0x09,
  Usage0A = 0x0a,
  Usage0B = 0x0b,
  Usage0C = 0x0c,
  Usage0D = 0x0d,
  Usage0E = 0x0e,
  Usage0F = 0x0f,
  Usage10 = 0x10,
  Usage11 = 0x11,
  Usage12 = 0x12,
  Usage13 = 0x13,
  Usage14 = 0x14,
  Usage15 = 0x15,
  Usage16 = 0x16,
  Usage17 = 0x17,
  Usage18 = 0x18,
  Usage19 = 0x19,
  Usage1A = 0x1a,
  Usage1B = 0x1b,
  Usage1C = 0x1c,
  Usage1D = 0x1d,
  Usage1E = 0x1e,
  Usage1F = 0x1f,
  Usage20 = 0x20,
  Usage21 = 0x21,
  Usage22 = 0x22,
  Usage23 = 0x23,
  Usage24 = 0x24,
  Usage25 = 0x25,
  Usage26 = 0x26,
  Usage27 = 0x27,
  Usage28 = 0x28,
  Usage29 = 0x29,
  Usage2A = 0x2a,
  Usage2B = 0x2b,
  Usage2C = 0x2c,
  Usage2D = 0x2d,
  Usage2E = 0x2e,
  Usage2F = 0x2f,
  Usage30 = 0x30,
  Usage31 = 0x31,
  Usage32 = 0x32,
  Usage33 = 0x33,
  Usage34 = 0x34,
  Usage35 = 0x35,
  Usage36 = 0x36,
  Usage37 = 0x37,
  Usage38 = 0x38,
  Usage39 = 0x39,
  Usage3A = 0x3a,
  Usage3B = 0x3b,
  Usage3C = 0x3c,
  Usage3D = 0x3d,
  Usage3E = 0x3e,
  Usage3F = 0x3f,
  Usage40 = 0x40,
  Usage41 = 0x41,
  Usage42 = 0x42,
  Usage43 = 0x43,
  Usage44 = 0x44,
  Usage45 = 0x45,
  Usage46 = 0x46,
  Usage47 = 0x47,
  Usage48 = 0x48,
  Usage49 = 0x49,
  Usage4A = 0x4a,
  Usage4B = 0x4b,
  Usage4C = 0x4c,
  Usage4D = 0x4d,
  Usage4E = 0x4e,
  Usage4F = 0x4f,
  Usage50 = 0x50,
  Usage51 = 0x51,
  Usage52 = 0x52,
  Usage64 = 0x64,
  Usage87 = 0x87,
  Usage88 = 0x88,
  Usage89 = 0x89,
  Usage8A = 0x8a,
  Usage8B = 0x8b,
  Usage8C = 0x8c,
  Usage8D = 0x8d,
  Usage8E = 0x8e,
  Usage8F = 0x8f,
  Usage90 = 0x90,
  Usage91 = 0x91,
  Usage92 = 0x92,
  Usage93 = 0x93,
  Usage94 = 0x94,
  Usage95 = 0x95,
  Usage96 = 0x96,
  Usage97 = 0x97,
  Usage98 = 0x98,
  UsageE0 = 0xe0,
  UsageE1 = 0xe1,
  UsageE2 = 0xe2,
  UsageE3 = 0xe3,
  UsageE4 = 0xe4,
  UsageE5 = 0xe5,
  UsageE6 = 0xe6,
  UsageE7 = 0xe7,
};

enum class KeyboardLayoutId : uint8_t
{
  Us = 1,
  Fr = 2,
};

class KeyboardLayout
{
public:
  static KeyboardLayout us();
  static KeyboardLayout fr();

  KeySymbol decode(HidUsage usage) const;
  HidUsage encode(KeySymbol key) const;
  KeyboardLayoutId id() const;

private:
  explicit KeyboardLayout(KeyboardLayoutId id);

  KeyboardLayoutId id_;
};

enum class InputDomain : uint8_t
{
  Keyboard = 1,
  Consumer = 2,
  PointerButton = 3,
  PointerAxis = 4,
  Vendor = 255,
};

enum class ConsumerUsage : uint16_t
{
  None = 0,
  Power = 0x0030,
  Sleep = 0x0032,
  Menu = 0x0040,
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
  BassBoost = 0x00e5,
  VolumeIncrement = 0x00e9,
  VolumeDecrement = 0x00ea,
  BrowserSearch = 0x0221,
  BrowserHome = 0x0223,
  BrowserBack = 0x0224,
  BrowserForward = 0x0225,
  BrowserRefresh = 0x0227,
  BrowserBookmarks = 0x022a,
};

enum class PointerAxis : uint16_t
{
  None = 0,
  X = 1,
  Y = 2,
  Wheel = 3,
  Pan = 4,
};

struct InputCode
{
  InputDomain domain = InputDomain::Keyboard;
  uint16_t code = 0;

  bool operator==(const InputCode &other) const;
  bool operator!=(const InputCode &other) const;
};

struct InputEvent
{
  InputCode code;
  bool pressed = false;
  uint32_t timestampMs = 0;
};

struct InputValueEvent
{
  InputCode code;
  int16_t value = 0;
  uint32_t timestampMs = 0;
};

InputCode keyboardCode(KeySymbol key);
InputCode consumerCode(uint16_t code);
InputCode consumerCode(ConsumerUsage usage);
InputCode pointerButtonCode(uint16_t code);
InputCode pointerAxisCode(uint16_t code);
InputCode pointerAxisCode(PointerAxis axis);
InputCode vendorCode(uint16_t code);
uint16_t hidUsageValue(HidUsage usage);
HidUsage hidUsage(uint16_t usage);
uint16_t hidUsageFromKeySymbol(KeySymbol key);
KeySymbol keySymbolFromHidUsage(uint16_t usage);
bool isHidKeyboardKeySymbol(KeySymbol key);
KeySymbol keySymbolFromCode(InputCode code);
const char *inputDomainName(InputDomain domain);
bool isValid(InputCode code);
InputEvent inputEvent(InputCode code, bool pressed, uint32_t timestampMs = 0);
InputEvent keyEvent(KeySymbol key, bool pressed, uint32_t timestampMs = 0);
InputValueEvent inputValueEvent(InputCode code, int16_t value, uint32_t timestampMs = 0);
InputValueEvent pointerAxisValueEvent(PointerAxis axis, int16_t value, uint32_t timestampMs = 0);

bool isModifierKeySymbol(KeySymbol key);
const char *keySymbolName(KeySymbol key);
const char *consumerUsageName(ConsumerUsage usage);
const char *pointerAxisName(PointerAxis axis);

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

struct HidConsumerReport
{
  static constexpr size_t ReportSize = 2;

  uint16_t usage = 0;
  bool overflow = false;

  void clear();
  bool empty() const;
  bool writeReport(uint8_t *buffer, size_t size) const;
};

struct HidPointerReport
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
  bool apply(InputValueEvent event);
  bool writeReport(uint8_t *buffer, size_t size) const;
};

class InputState
{
public:
  static constexpr size_t MaxCodes = 32;

  void clear();
  bool press(KeySymbol key);
  bool press(InputCode code);
  bool mergeFrom(const InputState &other);
  bool release(KeySymbol key);
  bool release(InputCode code);
  bool contains(KeySymbol key) const;
  bool contains(InputCode code) const;
  bool isPressed(KeySymbol key) const;
  bool isPressed(InputCode code) const;
  bool apply(InputEvent event);
  size_t codeCount() const;
  KeySymbol keyAt(size_t index) const;
  InputCode codeAt(size_t index) const;

private:
  InputCode codes_[MaxCodes] = {};
  size_t codeCount_ = 0;
};

HidKeyboardReport buildHidKeyboardReport(const InputState &state);
HidKeyboardRolloverReport buildHidKeyboardRolloverReport(const InputState &state);
HidConsumerReport buildHidConsumerReport(const InputState &state);
HidPointerReport buildHidPointerReport(const InputState &state);

class InputAdapter
{
public:
  virtual ~InputAdapter() = default;
  virtual void update() = 0;
  virtual const InputState &state() const = 0;
};

class EventInputAdapter : public InputAdapter
{
public:
  void update() override;
  const InputState &state() const override;
  bool apply(InputEvent event);
  void clear();

private:
  InputState state_;
};

class OutputAdapter
{
public:
  virtual ~OutputAdapter() = default;
  virtual void write(const InputState &state) = 0;
};

class RecordingOutputAdapter : public OutputAdapter
{
public:
  void write(const InputState &state) override;
  const InputState &state() const;
  size_t writeCount() const;
  void clear();

private:
  InputState state_;
  size_t writeCount_ = 0;
};

class RecordingHidKeyboardOutputAdapter : public OutputAdapter
{
public:
  void write(const InputState &state) override;
  const HidKeyboardReport &report() const;
  size_t writeCount() const;
  void clear();

private:
  HidKeyboardReport report_;
  size_t writeCount_ = 0;
};

class RecordingHidKeyboardRolloverOutputAdapter : public OutputAdapter
{
public:
  void write(const InputState &state) override;
  const HidKeyboardRolloverReport &report() const;
  size_t writeCount() const;
  void clear();

private:
  HidKeyboardRolloverReport report_;
  size_t writeCount_ = 0;
};

class RecordingHidConsumerOutputAdapter : public OutputAdapter
{
public:
  void write(const InputState &state) override;
  const HidConsumerReport &report() const;
  size_t writeCount() const;
  void clear();

private:
  HidConsumerReport report_;
  size_t writeCount_ = 0;
};

class RecordingHidPointerOutputAdapter : public OutputAdapter
{
public:
  void write(const InputState &state) override;
  const HidPointerReport &report() const;
  size_t writeCount() const;
  void clear();

private:
  HidPointerReport report_;
  size_t writeCount_ = 0;
};

struct CodeRemap
{
  InputCode from;
  InputCode to;
};

struct KeyMacro
{
  static constexpr size_t MaxKeys = 8;

  KeySymbol trigger = KeySymbol::None;
  KeySymbol keys[MaxKeys] = {};
  size_t keyCount = 0;
};

class TransformConfig
{
public:
  static constexpr size_t MaxRemaps = 32;
  static constexpr size_t MaxDisabledKeys = 32;
  static constexpr size_t MaxMacros = 16;

  bool remap(KeySymbol from, KeySymbol to);
  bool remap(InputCode from, InputCode to);
  bool disable(KeySymbol key);
  bool disable(InputCode code);
  bool macro(KeySymbol trigger, const KeySymbol *keys, size_t keyCount);
  void clear();
  KeySymbol map(KeySymbol key) const;
  InputCode map(InputCode code) const;
  bool isDisabled(KeySymbol key) const;
  bool isDisabled(InputCode code) const;
  const KeyMacro *findMacro(KeySymbol trigger) const;
  bool empty() const;

private:
  CodeRemap remaps_[MaxRemaps] = {};
  size_t remapCount_ = 0;
  InputCode disabledCodes_[MaxDisabledKeys] = {};
  size_t disabledCodeCount_ = 0;
  KeyMacro macros_[MaxMacros] = {};
  size_t macroCount_ = 0;
};

struct MergeConfig
{
  bool shareModifiers = true;
  bool shareKeyboardKeys = true;
  bool shareConsumer = true;
  bool sharePointerButtons = true;
  bool sharePointerAxes = true;
  bool shareVendor = true;
};

class LayerConfig
{
public:
  void setMomentary(KeySymbol trigger);
  bool remap(KeySymbol from, KeySymbol to);
  void clear();
  bool enabled() const;
  KeySymbol trigger() const;
  KeySymbol map(KeySymbol key) const;

private:
  bool enabled_ = false;
  KeySymbol trigger_ = KeySymbol::None;
  TransformConfig transform_;
};

class LayoutConfig
{
public:
  static constexpr size_t MaxMappings = 64;

  bool map(KeySymbol from, KeySymbol to);
  void clear();
  KeySymbol convert(KeySymbol key) const;

private:
  CodeRemap mappings_[MaxMappings] = {};
  size_t mappingCount_ = 0;
};

class ESP32KeyBridgeConfig
{
public:
  static constexpr size_t MaxInputConfigs = 8;

  TransformConfig &input(size_t index);
  const TransformConfig &input(size_t index) const;
  TransformConfig *tryInput(size_t index);
  const TransformConfig *tryInput(size_t index) const;
  bool hasInvalidInputConfig() const;
  void clear();

  TransformConfig global;
  LayerConfig layer;
  LayoutConfig layout;
  MergeConfig merge;

private:
  TransformConfig inputTransforms_[MaxInputConfigs] = {};
  TransformConfig invalidInputTransform_;
};

struct ESP32KeyBridgeConfigError
{
  const char *message = nullptr;
};

class ESP32KeyBridge
{
public:
  static constexpr size_t MaxInputs = 8;
  static constexpr size_t MaxOutputs = 4;

  bool addInput(InputAdapter &input);
  bool addInput(InputAdapter &input, size_t configIndex);
  bool addOutput(OutputAdapter &output);
  void clearInputs();
  void clearOutputs();
  bool validateConfig(const ESP32KeyBridgeConfig &config, ESP32KeyBridgeConfigError &error) const;
  void applyConfig(const ESP32KeyBridgeConfig &config);
  void begin();
  void update();
  const InputState &mergedState() const;
  const InputState &outputState() const;

private:
  bool shouldMerge(InputCode code) const;
  void mergeInput(const InputState &input, InputState &merged) const;
  void applyTransform(const InputState &input, const TransformConfig &transform, InputState &output) const;
  void applyLayer(const InputState &input, InputState &output) const;
  void applyLayout(const InputState &input, InputState &output) const;

  InputAdapter *inputs_[MaxInputs] = {};
  size_t inputConfigIndexes_[MaxInputs] = {};
  size_t inputCount_ = 0;
  OutputAdapter *outputs_[MaxOutputs] = {};
  size_t outputCount_ = 0;
  ESP32KeyBridgeConfig config_;
  InputState mergedState_;
  InputState outputState_;
};

} // namespace esp32keybridge
