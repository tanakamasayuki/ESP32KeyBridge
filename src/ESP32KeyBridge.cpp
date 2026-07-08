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

InputEvent keyEvent(Key key, bool pressed, uint32_t timestampMs)
{
  return {keyboardCode(key), pressed, timestampMs};
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

void InputState::clear()
{
  codeCount_ = 0;
}

bool InputState::press(Key key)
{
  return press(keyboardCode(key));
}

bool InputState::press(InputCode code)
{
  if (code.code == 0)
  {
    return false;
  }
  if (isPressed(code))
  {
    return true;
  }
  if (codeCount_ >= MaxCodes)
  {
    return false;
  }
  codes_[codeCount_++] = code;
  return true;
}

bool InputState::release(Key key)
{
  return release(keyboardCode(key));
}

bool InputState::release(InputCode code)
{
  for (size_t i = 0; i < codeCount_; ++i)
  {
    if (codes_[i] == code)
    {
      codes_[i] = codes_[codeCount_ - 1];
      --codeCount_;
      return true;
    }
  }
  return false;
}

bool InputState::isPressed(Key key) const
{
  return isPressed(keyboardCode(key));
}

bool InputState::isPressed(InputCode code) const
{
  for (size_t i = 0; i < codeCount_; ++i)
  {
    if (codes_[i] == code)
    {
      return true;
    }
  }
  return false;
}

bool InputState::apply(InputEvent event)
{
  return event.pressed ? press(event.code) : release(event.code);
}

size_t InputState::codeCount() const
{
  return codeCount_;
}

Key InputState::keyAt(size_t index) const
{
  return index < codeCount_ ? keyFromCode(codes_[index]) : Key::None;
}

InputCode InputState::codeAt(size_t index) const
{
  return index < codeCount_ ? codes_[index] : keyboardCode(Key::None);
}

void EventInputAdapter::update() {}

const InputState &EventInputAdapter::state() const
{
  return state_;
}

bool EventInputAdapter::apply(InputEvent event)
{
  return state_.apply(event);
}

void EventInputAdapter::clear()
{
  state_.clear();
}

void RecordingOutputAdapter::write(const InputState &state)
{
  state_ = state;
  ++writeCount_;
}

const InputState &RecordingOutputAdapter::state() const
{
  return state_;
}

size_t RecordingOutputAdapter::writeCount() const
{
  return writeCount_;
}

void RecordingOutputAdapter::clear()
{
  state_.clear();
  writeCount_ = 0;
}

bool TransformConfig::remap(Key from, Key to)
{
  return remap(keyboardCode(from), keyboardCode(to));
}

bool TransformConfig::remap(InputCode from, InputCode to)
{
  if (from.code == 0 || to.code == 0)
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
  return disable(keyboardCode(key));
}

bool TransformConfig::disable(InputCode code)
{
  if (code.code == 0 || isDisabled(code))
  {
    return code.code != 0;
  }
  if (disabledCodeCount_ >= MaxDisabledKeys)
  {
    return false;
  }
  disabledCodes_[disabledCodeCount_++] = code;
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

void TransformConfig::clear()
{
  remapCount_ = 0;
  disabledCodeCount_ = 0;
  macroCount_ = 0;
}

Key TransformConfig::map(Key key) const
{
  return keyFromCode(map(keyboardCode(key)));
}

InputCode TransformConfig::map(InputCode code) const
{
  for (size_t i = 0; i < remapCount_; ++i)
  {
    if (remaps_[i].from == code)
    {
      return remaps_[i].to;
    }
  }
  return code;
}

bool TransformConfig::isDisabled(Key key) const
{
  return isDisabled(keyboardCode(key));
}

bool TransformConfig::isDisabled(InputCode code) const
{
  for (size_t i = 0; i < disabledCodeCount_; ++i)
  {
    if (disabledCodes_[i] == code)
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
  return remapCount_ == 0 && disabledCodeCount_ == 0 && macroCount_ == 0;
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

void LayerConfig::clear()
{
  enabled_ = false;
  trigger_ = Key::None;
  transform_.clear();
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
    if (mappings_[i].from == keyboardCode(from))
    {
      mappings_[i].to = keyboardCode(to);
      return true;
    }
  }
  if (mappingCount_ >= MaxMappings)
  {
    return false;
  }
  mappings_[mappingCount_++] = {keyboardCode(from), keyboardCode(to)};
  return true;
}

void LayoutConfig::clear()
{
  mappingCount_ = 0;
}

Key LayoutConfig::convert(Key key) const
{
  for (size_t i = 0; i < mappingCount_; ++i)
  {
    if (mappings_[i].from == keyboardCode(key))
    {
      return keyFromCode(mappings_[i].to);
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

void ESP32KeyBridgeConfig::clear()
{
  for (size_t i = 0; i < MaxInputConfigs; ++i)
  {
    inputTransforms_[i].clear();
  }
  invalidInputTransform_.clear();
  global.clear();
  layer.clear();
  layout.clear();
  merge = MergeConfig{};
}

bool ESP32KeyBridge::addInput(InputAdapter &input)
{
  return addInput(input, inputCount_);
}

bool ESP32KeyBridge::addInput(InputAdapter &input, size_t configIndex)
{
  if (inputCount_ >= MaxInputs)
  {
    return false;
  }
  if (configIndex >= ESP32KeyBridgeConfig::MaxInputConfigs)
  {
    return false;
  }
  inputs_[inputCount_++] = &input;
  inputConfigIndexes_[inputCount_ - 1] = configIndex;
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

void ESP32KeyBridge::clearInputs()
{
  for (size_t i = 0; i < inputCount_; ++i)
  {
    inputs_[i] = nullptr;
    inputConfigIndexes_[i] = 0;
  }
  inputCount_ = 0;
  mergedState_.clear();
  outputState_.clear();
}

void ESP32KeyBridge::clearOutputs()
{
  for (size_t i = 0; i < outputCount_; ++i)
  {
    outputs_[i] = nullptr;
  }
  outputCount_ = 0;
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
    InputState deviceState;
    applyTransform(inputs_[i]->state(), config_.input(inputConfigIndexes_[i]), deviceState);
    mergeInput(deviceState, mergedState_);
  }

  outputState_.clear();
  InputState layered;
  applyLayer(mergedState_, layered);
  InputState layoutConverted;
  applyLayout(layered, layoutConverted);
  applyTransform(layoutConverted, config_.global, outputState_);

  for (size_t i = 0; i < outputCount_; ++i)
  {
    outputs_[i]->write(outputState_);
  }
}

const InputState &ESP32KeyBridge::mergedState() const
{
  return mergedState_;
}

const InputState &ESP32KeyBridge::outputState() const
{
  return outputState_;
}

bool ESP32KeyBridge::shouldMerge(InputCode code) const
{
  if (code.domain == InputDomain::Keyboard)
  {
    const Key key = keyFromCode(code);
    return isModifierKey(key) ? config_.merge.shareModifiers : config_.merge.shareKeyboardKeys;
  }

  switch (code.domain)
  {
  case InputDomain::Consumer:
    return config_.merge.shareConsumer;
  case InputDomain::PointerButton:
    return config_.merge.sharePointerButtons;
  case InputDomain::PointerAxis:
    return config_.merge.sharePointerAxes;
  case InputDomain::Vendor:
    return config_.merge.shareVendor;
  case InputDomain::Keyboard:
    break;
  }
  return false;
}

void ESP32KeyBridge::mergeInput(const InputState &input, InputState &merged) const
{
  for (size_t i = 0; i < input.codeCount(); ++i)
  {
    const InputCode code = input.codeAt(i);
    if (shouldMerge(code))
    {
      merged.press(code);
    }
  }
}

void ESP32KeyBridge::applyTransform(const InputState &input, const TransformConfig &transform, InputState &output) const
{
  for (size_t i = 0; i < input.codeCount(); ++i)
  {
    const InputCode code = input.codeAt(i);
    if (transform.isDisabled(code))
    {
      continue;
    }
    if (code.domain == InputDomain::Keyboard)
    {
      const Key key = keyFromCode(code);
      const KeyMacro *macro = transform.findMacro(key);
      if (macro != nullptr)
      {
        for (size_t j = 0; j < macro->keyCount; ++j)
        {
          output.press(macro->keys[j]);
        }
        continue;
      }
    }
    output.press(transform.map(code));
  }
}

void ESP32KeyBridge::applyLayer(const InputState &input, InputState &output) const
{
  const bool layerActive = config_.layer.enabled() && input.isPressed(config_.layer.trigger());
  for (size_t i = 0; i < input.codeCount(); ++i)
  {
    const InputCode code = input.codeAt(i);
    if (code.domain != InputDomain::Keyboard)
    {
      output.press(code);
      continue;
    }
    const Key key = input.keyAt(i);
    if (layerActive && key == config_.layer.trigger())
    {
      continue;
    }
    output.press(layerActive ? config_.layer.map(key) : key);
  }
}

void ESP32KeyBridge::applyLayout(const InputState &input, InputState &output) const
{
  for (size_t i = 0; i < input.codeCount(); ++i)
  {
    const InputCode code = input.codeAt(i);
    if (code.domain == InputDomain::Keyboard)
    {
      output.press(config_.layout.convert(input.keyAt(i)));
    }
    else
    {
      output.press(code);
    }
  }
}

} // namespace esp32keybridge
