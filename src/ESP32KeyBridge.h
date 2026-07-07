#pragma once

#include <stddef.h>
#include <stdint.h>

namespace esp32keybridge
{

enum class Key : uint16_t
{
  None = 0,
  A,
  B,
  C,
  Enter,
  Tab,
  Insert,
  CapsLock,
  LeftCtrl,
  LeftShift,
  Fn1,
};

enum class InputDomain : uint8_t
{
  Keyboard = 1,
  Consumer = 2,
  PointerButton = 3,
  PointerAxis = 4,
  Vendor = 255,
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

InputCode keyboardCode(Key key);
Key keyFromCode(InputCode code);
const char *inputDomainName(InputDomain domain);
InputEvent keyEvent(Key key, bool pressed, uint32_t timestampMs = 0);

bool isModifierKey(Key key);
const char *keyName(Key key);

class KeyboardState
{
public:
  static constexpr size_t MaxKeys = 32;

  void clear();
  bool press(Key key);
  bool press(InputCode code);
  bool release(Key key);
  bool release(InputCode code);
  bool isPressed(Key key) const;
  bool isPressed(InputCode code) const;
  bool apply(InputEvent event);
  size_t keyCount() const;
  Key keyAt(size_t index) const;
  InputCode codeAt(size_t index) const;

private:
  InputCode codes_[MaxKeys] = {};
  size_t keyCount_ = 0;
};

class InputAdapter
{
public:
  virtual ~InputAdapter() = default;
  virtual void update() = 0;
  virtual const KeyboardState &state() const = 0;
};

class EventInputAdapter : public InputAdapter
{
public:
  void update() override;
  const KeyboardState &state() const override;
  bool apply(InputEvent event);
  void clear();

private:
  KeyboardState state_;
};

class OutputAdapter
{
public:
  virtual ~OutputAdapter() = default;
  virtual void write(const KeyboardState &state) = 0;
};

struct KeyRemap
{
  Key from = Key::None;
  Key to = Key::None;
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
  bool disable(Key key);
  bool macro(Key trigger, const Key *keys, size_t keyCount);
  Key map(Key key) const;
  bool isDisabled(Key key) const;
  const KeyMacro *findMacro(Key trigger) const;
  bool empty() const;

private:
  KeyRemap remaps_[MaxRemaps] = {};
  size_t remapCount_ = 0;
  Key disabledKeys_[MaxDisabledKeys] = {};
  size_t disabledKeyCount_ = 0;
  KeyMacro macros_[MaxMacros] = {};
  size_t macroCount_ = 0;
};

struct MergeConfig
{
  bool shareModifiers = true;
  bool shareKeys = true;
};

class LayerConfig
{
public:
  void setMomentary(Key trigger);
  bool remap(Key from, Key to);
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
  Key convert(Key key) const;

private:
  KeyRemap mappings_[MaxMappings] = {};
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
  bool addOutput(OutputAdapter &output);
  bool validateConfig(const ESP32KeyBridgeConfig &config, ESP32KeyBridgeConfigError &error) const;
  void applyConfig(const ESP32KeyBridgeConfig &config);
  void begin();
  void update();
  const KeyboardState &mergedState() const;
  const KeyboardState &outputState() const;

private:
  void mergeInput(const KeyboardState &input, KeyboardState &merged) const;
  void applyTransform(const KeyboardState &input, const TransformConfig &transform, KeyboardState &output) const;
  void applyLayer(const KeyboardState &input, KeyboardState &output) const;
  void applyLayout(const KeyboardState &input, KeyboardState &output) const;

  InputAdapter *inputs_[MaxInputs] = {};
  size_t inputCount_ = 0;
  OutputAdapter *outputs_[MaxOutputs] = {};
  size_t outputCount_ = 0;
  ESP32KeyBridgeConfig config_;
  KeyboardState mergedState_;
  KeyboardState outputState_;
};

} // namespace esp32keybridge
