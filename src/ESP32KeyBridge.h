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

class TransformConfig
{
public:
  static constexpr size_t MaxRemaps = 32;
  static constexpr size_t MaxDisabledKeys = 32;

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

private:
  KeyRemap remaps_[MaxRemaps] = {};
  size_t remapCount_ = 0;
  Key disabledKeys_[MaxDisabledKeys] = {};
  size_t disabledKeyCount_ = 0;
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
    KeyboardState merged;
    for (size_t i = 0; i < inputCount_; ++i)
    {
      inputs_[i]->update();
      KeyboardState deviceState;
      applyTransform(inputs_[i]->state(), config_.input(i), deviceState);
      mergeInput(deviceState, merged);
    }

    KeyboardState output;
    KeyboardState layered;
    applyLayer(merged, layered);
    applyTransform(layered, config_.global, output);

    for (size_t i = 0; i < outputCount_; ++i)
    {
      outputs_[i]->write(output);
    }
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

  InputAdapter *inputs_[MaxInputs] = {};
  size_t inputCount_ = 0;
  OutputAdapter *outputs_[MaxOutputs] = {};
  size_t outputCount_ = 0;
  ESP32KeyBridgeConfig config_;
};

} // namespace esp32keybridge
