#include "ESP32KeyBridge.h"

namespace esp32keybridge
{

bool InputCode::operator==(const InputCode &other) const
{
  return domain == other.domain && code == other.code;
}

bool InputCode::operator!=(const InputCode &other) const
{
  return !(*this == other);
}

InputCode keyboardCode(Key key)
{
  return {InputDomain::Keyboard, static_cast<uint16_t>(key)};
}

Key keyFromCode(InputCode code)
{
  if (code.domain != InputDomain::Keyboard)
  {
    return Key::None;
  }
  return static_cast<Key>(code.code);
}

const char *inputDomainName(InputDomain domain)
{
  switch (domain)
  {
  case InputDomain::Keyboard:
    return "Keyboard";
  case InputDomain::Consumer:
    return "Consumer";
  case InputDomain::PointerButton:
    return "PointerButton";
  case InputDomain::PointerAxis:
    return "PointerAxis";
  case InputDomain::Vendor:
    return "Vendor";
  }
  return "Unknown";
}

bool isModifierKey(Key key)
{
  return key == Key::LeftCtrl || key == Key::LeftShift;
}

const char *keyName(Key key)
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

void KeyboardState::clear()
{
  keyCount_ = 0;
}

bool KeyboardState::press(Key key)
{
  return press(keyboardCode(key));
}

bool KeyboardState::press(InputCode code)
{
  if (code.domain != InputDomain::Keyboard || code.code == static_cast<uint16_t>(Key::None))
  {
    return false;
  }
  if (isPressed(code))
  {
    return true;
  }
  if (keyCount_ >= MaxKeys)
  {
    return false;
  }
  codes_[keyCount_++] = code;
  return true;
}

bool KeyboardState::release(Key key)
{
  return release(keyboardCode(key));
}

bool KeyboardState::release(InputCode code)
{
  for (size_t i = 0; i < keyCount_; ++i)
  {
    if (codes_[i] == code)
    {
      codes_[i] = codes_[keyCount_ - 1];
      --keyCount_;
      return true;
    }
  }
  return false;
}

bool KeyboardState::isPressed(Key key) const
{
  return isPressed(keyboardCode(key));
}

bool KeyboardState::isPressed(InputCode code) const
{
  for (size_t i = 0; i < keyCount_; ++i)
  {
    if (codes_[i] == code)
    {
      return true;
    }
  }
  return false;
}

size_t KeyboardState::keyCount() const
{
  return keyCount_;
}

Key KeyboardState::keyAt(size_t index) const
{
  return index < keyCount_ ? keyFromCode(codes_[index]) : Key::None;
}

InputCode KeyboardState::codeAt(size_t index) const
{
  return index < keyCount_ ? codes_[index] : keyboardCode(Key::None);
}

bool TransformConfig::remap(Key from, Key to)
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

bool TransformConfig::disable(Key key)
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

bool TransformConfig::macro(Key trigger, const Key *keys, size_t keyCount)
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

Key TransformConfig::map(Key key) const
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

bool TransformConfig::isDisabled(Key key) const
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

const KeyMacro *TransformConfig::findMacro(Key trigger) const
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

bool TransformConfig::empty() const
{
  return remapCount_ == 0 && disabledKeyCount_ == 0 && macroCount_ == 0;
}

void LayerConfig::setMomentary(Key trigger)
{
  trigger_ = trigger;
  enabled_ = trigger != Key::None;
}

bool LayerConfig::remap(Key from, Key to)
{
  return transform_.remap(from, to);
}

bool LayerConfig::enabled() const
{
  return enabled_;
}

Key LayerConfig::trigger() const
{
  return trigger_;
}

Key LayerConfig::map(Key key) const
{
  return transform_.map(key);
}

bool LayoutConfig::map(Key from, Key to)
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

Key LayoutConfig::convert(Key key) const
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

TransformConfig &ESP32KeyBridgeConfig::input(size_t index)
{
  return index < MaxInputConfigs ? inputTransforms_[index] : invalidInputTransform_;
}

const TransformConfig &ESP32KeyBridgeConfig::input(size_t index) const
{
  return index < MaxInputConfigs ? inputTransforms_[index] : invalidInputTransform_;
}

TransformConfig *ESP32KeyBridgeConfig::tryInput(size_t index)
{
  return index < MaxInputConfigs ? &inputTransforms_[index] : nullptr;
}

const TransformConfig *ESP32KeyBridgeConfig::tryInput(size_t index) const
{
  return index < MaxInputConfigs ? &inputTransforms_[index] : nullptr;
}

bool ESP32KeyBridgeConfig::hasInvalidInputConfig() const
{
  return !invalidInputTransform_.empty();
}

bool ESP32KeyBridge::addInput(InputAdapter &input)
{
  if (inputCount_ >= MaxInputs)
  {
    return false;
  }
  inputs_[inputCount_++] = &input;
  return true;
}

bool ESP32KeyBridge::addOutput(OutputAdapter &output)
{
  if (outputCount_ >= MaxOutputs)
  {
    return false;
  }
  outputs_[outputCount_++] = &output;
  return true;
}

bool ESP32KeyBridge::validateConfig(const ESP32KeyBridgeConfig &config, ESP32KeyBridgeConfigError &error) const
{
  if (config.hasInvalidInputConfig())
  {
    error.message = "input config index out of range";
    return false;
  }
  error.message = nullptr;
  return true;
}

void ESP32KeyBridge::applyConfig(const ESP32KeyBridgeConfig &config)
{
  config_ = config;
}

void ESP32KeyBridge::begin() {}

void ESP32KeyBridge::update()
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

const KeyboardState &ESP32KeyBridge::mergedState() const
{
  return mergedState_;
}

const KeyboardState &ESP32KeyBridge::outputState() const
{
  return outputState_;
}

void ESP32KeyBridge::mergeInput(const KeyboardState &input, KeyboardState &merged) const
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

void ESP32KeyBridge::applyTransform(const KeyboardState &input, const TransformConfig &transform, KeyboardState &output) const
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

void ESP32KeyBridge::applyLayer(const KeyboardState &input, KeyboardState &output) const
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

void ESP32KeyBridge::applyLayout(const KeyboardState &input, KeyboardState &output) const
{
  for (size_t i = 0; i < input.keyCount(); ++i)
  {
    output.press(config_.layout.convert(input.keyAt(i)));
  }
}

} // namespace esp32keybridge
