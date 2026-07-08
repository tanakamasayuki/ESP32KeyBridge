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
InputCode consumerCode(uint16_t code);
InputCode pointerButtonCode(uint16_t code);
InputCode pointerAxisCode(uint16_t code);
InputCode vendorCode(uint16_t code);
Key keyFromCode(InputCode code);
const char *inputDomainName(InputDomain domain);
InputEvent inputEvent(InputCode code, bool pressed, uint32_t timestampMs = 0);
InputEvent keyEvent(Key key, bool pressed, uint32_t timestampMs = 0);

bool isModifierKey(Key key);
const char *keyName(Key key);

class InputState
{
public:
  static constexpr size_t MaxCodes = 32;

  void clear();
  bool press(Key key);
  bool press(InputCode code);
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
