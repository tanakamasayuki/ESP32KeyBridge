#pragma once

#include <stddef.h>
#include <stdint.h>

namespace esp32keybridge
{

enum class Key : uint16_t
{
  None = 0,
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
  Num1 = 0x1e,
  Num2 = 0x1f,
  Num3 = 0x20,
  Num4 = 0x21,
  Num5 = 0x22,
  Num6 = 0x23,
  Num7 = 0x24,
  Num8 = 0x25,
  Num9 = 0x26,
  Num0 = 0x27,
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
  Dot = 0x37,
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
  NonUsBackslash = 0x64,
  International1 = 0x87,
  International2 = 0x88,
  International3 = 0x89,
  International4 = 0x8a,
  International5 = 0x8b,
  International6 = 0x8c,
  International7 = 0x8d,
  International8 = 0x8e,
  International9 = 0x8f,
  Lang1 = 0x90,
  Lang2 = 0x91,
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
  Fn1 = 0x0100,
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

InputCode keyboardCode(Key key);
InputCode consumerCode(uint16_t code);
InputCode consumerCode(ConsumerUsage usage);
InputCode pointerButtonCode(uint16_t code);
InputCode pointerAxisCode(uint16_t code);
InputCode pointerAxisCode(PointerAxis axis);
InputCode vendorCode(uint16_t code);
uint16_t hidUsageFromKey(Key key);
Key keyFromHidUsage(uint16_t usage);
bool isHidKeyboardKey(Key key);
Key keyFromCode(InputCode code);
const char *inputDomainName(InputDomain domain);
bool isValid(InputCode code);
InputEvent inputEvent(InputCode code, bool pressed, uint32_t timestampMs = 0);
InputEvent keyEvent(Key key, bool pressed, uint32_t timestampMs = 0);
InputValueEvent inputValueEvent(InputCode code, int16_t value, uint32_t timestampMs = 0);
InputValueEvent pointerAxisValueEvent(PointerAxis axis, int16_t value, uint32_t timestampMs = 0);

bool isModifierKey(Key key);
const char *keyName(Key key);
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

struct HidConsumerReport
{
  static constexpr size_t ReportSize = 2;

  uint16_t usage = 0;
  bool overflow = false;

  void clear();
  bool empty() const;
  bool writeReport(uint8_t *buffer, size_t size) const;
};

class InputState
{
public:
  static constexpr size_t MaxCodes = 32;

  void clear();
  bool press(Key key);
  bool press(InputCode code);
  bool mergeFrom(const InputState &other);
  bool release(Key key);
  bool release(InputCode code);
  bool contains(Key key) const;
  bool contains(InputCode code) const;
  bool isPressed(Key key) const;
  bool isPressed(InputCode code) const;
  bool apply(InputEvent event);
  size_t codeCount() const;
  Key keyAt(size_t index) const;
  InputCode codeAt(size_t index) const;

private:
  InputCode codes_[MaxCodes] = {};
  size_t codeCount_ = 0;
};

HidKeyboardReport buildHidKeyboardReport(const InputState &state);
HidConsumerReport buildHidConsumerReport(const InputState &state);

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

struct CodeRemap
{
  InputCode from;
  InputCode to;
};

struct KeyMacro
{
  static constexpr size_t MaxKeys = 8;

  Key trigger = Key::None;
  Key keys[MaxKeys] = {};
  size_t keyCount = 0;
};

class TransformConfig
{
public:
  static constexpr size_t MaxRemaps = 32;
  static constexpr size_t MaxDisabledKeys = 32;
  static constexpr size_t MaxMacros = 16;

  bool remap(Key from, Key to);
  bool remap(InputCode from, InputCode to);
  bool disable(Key key);
  bool disable(InputCode code);
  bool macro(Key trigger, const Key *keys, size_t keyCount);
  void clear();
  Key map(Key key) const;
  InputCode map(InputCode code) const;
  bool isDisabled(Key key) const;
  bool isDisabled(InputCode code) const;
  const KeyMacro *findMacro(Key trigger) const;
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
  void setMomentary(Key trigger);
  bool remap(Key from, Key to);
  void clear();
  bool enabled() const;
  Key trigger() const;
  Key map(Key key) const;

private:
  bool enabled_ = false;
  Key trigger_ = Key::None;
  TransformConfig transform_;
};

class LayoutConfig
{
public:
  static constexpr size_t MaxMappings = 64;

  bool map(Key from, Key to);
  void clear();
  Key convert(Key key) const;

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
