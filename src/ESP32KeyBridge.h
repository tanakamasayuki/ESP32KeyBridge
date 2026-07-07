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

inline bool isModifierKey(Key key)
{
  return key == Key::LeftCtrl || key == Key::LeftShift;
}

inline const char *keyName(Key key)
{
  switch (key)
  {
  case Key::None:
    return "None";
  case Key::A:
    return "A";
  case Key::B:
    return "B";
  case Key::C:
    return "C";
  case Key::Enter:
    return "Enter";
  case Key::Tab:
    return "Tab";
  case Key::Insert:
    return "Insert";
  case Key::CapsLock:
    return "CapsLock";
  case Key::LeftCtrl:
    return "LeftCtrl";
  case Key::LeftShift:
    return "LeftShift";
  case Key::Fn1:
    return "Fn1";
  }
  return "Unknown";
}

class KeyboardState
{
public:
  static constexpr size_t MaxKeys = 32;

  void clear()
  {
    keyCount_ = 0;
  }

  bool press(Key key)
  {
    if (key == Key::None)
    {
      return false;
    }
    if (isPressed(key))
    {
      return true;
    }
    if (keyCount_ >= MaxKeys)
    {
      return false;
    }
    keys_[keyCount_++] = key;
    return true;
  }

  bool release(Key key)
  {
    for (size_t i = 0; i < keyCount_; ++i)
    {
      if (keys_[i] == key)
      {
        keys_[i] = keys_[keyCount_ - 1];
        --keyCount_;
        return true;
      }
    }
    return false;
  }

  bool isPressed(Key key) const
  {
    for (size_t i = 0; i < keyCount_; ++i)
    {
      if (keys_[i] == key)
      {
        return true;
      }
    }
    return false;
  }

  size_t keyCount() const
  {
    return keyCount_;
  }

  Key keyAt(size_t index) const
  {
    return index < keyCount_ ? keys_[index] : Key::None;
  }

private:
  Key keys_[MaxKeys] = {};
  size_t keyCount_ = 0;
};

class InputAdapter
{
public:
  virtual ~InputAdapter() = default;
  virtual void update() = 0;
  virtual const KeyboardState &state() const = 0;
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

  bool remap(Key from, Key to)
  {
    if (from == Key::None || to == Key::None)
    {
      return false;
    }
    for (size_t i = 0; i < remapCount_; ++i)
    {
      if (remaps_[i].from == from)
      {
        remaps_[i].to = to;
        return true;
      }
    }
    if (remapCount_ >= MaxRemaps)
    {
      return false;
    }
    remaps_[remapCount_++] = {from, to};
    return true;
  }

  bool disable(Key key)
  {
    if (key == Key::None || isDisabled(key))
    {
      return key != Key::None;
    }
    if (disabledKeyCount_ >= MaxDisabledKeys)
    {
      return false;
    }
    disabledKeys_[disabledKeyCount_++] = key;
    return true;
  }

  bool macro(Key trigger, const Key *keys, size_t keyCount)
  {
    if (trigger == Key::None || keys == nullptr || keyCount == 0 || keyCount > KeyMacro::MaxKeys)
    {
      return false;
    }
    for (size_t i = 0; i < keyCount; ++i)
    {
      if (keys[i] == Key::None)
      {
        return false;
      }
    }

    for (size_t i = 0; i < macroCount_; ++i)
    {
      if (macros_[i].trigger == trigger)
      {
        macros_[i].keyCount = keyCount;
        for (size_t j = 0; j < keyCount; ++j)
        {
          macros_[i].keys[j] = keys[j];
        }
        return true;
      }
    }

    if (macroCount_ >= MaxMacros)
    {
      return false;
    }

    macros_[macroCount_].trigger = trigger;
    macros_[macroCount_].keyCount = keyCount;
    for (size_t i = 0; i < keyCount; ++i)
    {
      macros_[macroCount_].keys[i] = keys[i];
    }
    ++macroCount_;
    return true;
  }

  Key map(Key key) const
  {
    for (size_t i = 0; i < remapCount_; ++i)
    {
      if (remaps_[i].from == key)
      {
        return remaps_[i].to;
      }
    }
    return key;
  }

  bool isDisabled(Key key) const
  {
    for (size_t i = 0; i < disabledKeyCount_; ++i)
    {
      if (disabledKeys_[i] == key)
      {
        return true;
      }
    }
    return false;
  }

  const KeyMacro *findMacro(Key trigger) const
  {
    for (size_t i = 0; i < macroCount_; ++i)
    {
      if (macros_[i].trigger == trigger)
      {
        return &macros_[i];
      }
    }
    return nullptr;
  }

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
  void setMomentary(Key trigger)
  {
    trigger_ = trigger;
    enabled_ = trigger != Key::None;
  }

  bool remap(Key from, Key to)
  {
    return transform_.remap(from, to);
  }

  bool enabled() const
  {
    return enabled_;
  }

  Key trigger() const
  {
    return trigger_;
  }

  Key map(Key key) const
  {
    return transform_.map(key);
  }

private:
  bool enabled_ = false;
  Key trigger_ = Key::None;
  TransformConfig transform_;
};

class LayoutConfig
{
public:
  static constexpr size_t MaxMappings = 64;

  bool map(Key from, Key to)
  {
    if (from == Key::None || to == Key::None)
    {
      return false;
    }
    for (size_t i = 0; i < mappingCount_; ++i)
    {
      if (mappings_[i].from == from)
      {
        mappings_[i].to = to;
        return true;
      }
    }
    if (mappingCount_ >= MaxMappings)
    {
      return false;
    }
    mappings_[mappingCount_++] = {from, to};
    return true;
  }

  Key convert(Key key) const
  {
    for (size_t i = 0; i < mappingCount_; ++i)
    {
      if (mappings_[i].from == key)
      {
        return mappings_[i].to;
      }
    }
    return key;
  }

private:
  KeyRemap mappings_[MaxMappings] = {};
  size_t mappingCount_ = 0;
};

class ESP32KeyBridgeConfig
{
public:
  static constexpr size_t MaxInputConfigs = 8;

  TransformConfig &input(size_t index)
  {
    return index < MaxInputConfigs ? inputTransforms_[index] : invalidInputTransform_;
  }

  const TransformConfig &input(size_t index) const
  {
    return index < MaxInputConfigs ? inputTransforms_[index] : invalidInputTransform_;
  }

  TransformConfig *tryInput(size_t index)
  {
    return index < MaxInputConfigs ? &inputTransforms_[index] : nullptr;
  }

  const TransformConfig *tryInput(size_t index) const
  {
    return index < MaxInputConfigs ? &inputTransforms_[index] : nullptr;
  }

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

  bool addInput(InputAdapter &input)
  {
    if (inputCount_ >= MaxInputs)
    {
      return false;
    }
    inputs_[inputCount_++] = &input;
    return true;
  }

  bool addOutput(OutputAdapter &output)
  {
    if (outputCount_ >= MaxOutputs)
    {
      return false;
    }
    outputs_[outputCount_++] = &output;
    return true;
  }

  bool validateConfig(const ESP32KeyBridgeConfig &, ESP32KeyBridgeConfigError &error) const
  {
    error.message = nullptr;
    return true;
  }

  void applyConfig(const ESP32KeyBridgeConfig &config)
  {
    config_ = config;
  }

  void begin() {}

  void update()
  {
    mergedState_.clear();
    for (size_t i = 0; i < inputCount_; ++i)
    {
      inputs_[i]->update();
      KeyboardState deviceState;
      applyTransform(inputs_[i]->state(), config_.input(i), deviceState);
      mergeInput(deviceState, mergedState_);
    }

    outputState_.clear();
    KeyboardState layered;
    applyLayer(mergedState_, layered);
    KeyboardState layoutConverted;
    applyLayout(layered, layoutConverted);
    applyTransform(layoutConverted, config_.global, outputState_);

    for (size_t i = 0; i < outputCount_; ++i)
    {
      outputs_[i]->write(outputState_);
    }
  }

  const KeyboardState &mergedState() const
  {
    return mergedState_;
  }

  const KeyboardState &outputState() const
  {
    return outputState_;
  }

private:
  void mergeInput(const KeyboardState &input, KeyboardState &merged) const
  {
    for (size_t i = 0; i < input.keyCount(); ++i)
    {
      const Key key = input.keyAt(i);
      if (isModifierKey(key))
      {
        if (config_.merge.shareModifiers)
        {
          merged.press(key);
        }
      }
      else if (config_.merge.shareKeys)
      {
        merged.press(key);
      }
    }
  }

  void applyTransform(const KeyboardState &input, const TransformConfig &transform, KeyboardState &output) const
  {
    for (size_t i = 0; i < input.keyCount(); ++i)
    {
      const Key key = input.keyAt(i);
      if (transform.isDisabled(key))
      {
        continue;
      }
      const KeyMacro *macro = transform.findMacro(key);
      if (macro != nullptr)
      {
        for (size_t j = 0; j < macro->keyCount; ++j)
        {
          output.press(macro->keys[j]);
        }
        continue;
      }
      output.press(transform.map(key));
    }
  }

  void applyLayer(const KeyboardState &input, KeyboardState &output) const
  {
    const bool layerActive = config_.layer.enabled() && input.isPressed(config_.layer.trigger());
    for (size_t i = 0; i < input.keyCount(); ++i)
    {
      const Key key = input.keyAt(i);
      if (layerActive && key == config_.layer.trigger())
      {
        continue;
      }
      output.press(layerActive ? config_.layer.map(key) : key);
    }
  }

  void applyLayout(const KeyboardState &input, KeyboardState &output) const
  {
    for (size_t i = 0; i < input.keyCount(); ++i)
    {
      output.press(config_.layout.convert(input.keyAt(i)));
    }
  }

  InputAdapter *inputs_[MaxInputs] = {};
  size_t inputCount_ = 0;
  OutputAdapter *outputs_[MaxOutputs] = {};
  size_t outputCount_ = 0;
  ESP32KeyBridgeConfig config_;
  KeyboardState mergedState_;
  KeyboardState outputState_;
};

} // namespace esp32keybridge
