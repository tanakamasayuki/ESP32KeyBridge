#include "ESP32KeyBridge.h"

namespace esp32keybridge
{

namespace
{
int8_t clampReportDelta(int16_t value, bool &overflow)
{
  if (value > 127)
  {
    overflow = true;
    return 127;
  }
  if (value < -127)
  {
    overflow = true;
    return -127;
  }
  return static_cast<int8_t>(value);
}

int8_t addReportDelta(int8_t current, int16_t value, bool &overflow)
{
  return clampReportDelta(static_cast<int16_t>(current) + value, overflow);
}
} // namespace

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

InputCode consumerCode(uint16_t code)
{
  return {InputDomain::Consumer, code};
}

InputCode consumerCode(ConsumerUsage usage)
{
  return consumerCode(static_cast<uint16_t>(usage));
}

InputCode pointerButtonCode(uint16_t code)
{
  return {InputDomain::PointerButton, code};
}

InputCode pointerAxisCode(uint16_t code)
{
  return {InputDomain::PointerAxis, code};
}

InputCode pointerAxisCode(PointerAxis axis)
{
  return pointerAxisCode(static_cast<uint16_t>(axis));
}

InputCode vendorCode(uint16_t code)
{
  return {InputDomain::Vendor, code};
}

uint16_t hidUsageFromKey(Key key)
{
  return isHidKeyboardKey(key) ? static_cast<uint16_t>(key) : 0;
}

Key keyFromHidUsage(uint16_t usage)
{
  return usage == 0 || usage > 0xff ? Key::None : static_cast<Key>(usage);
}

bool isHidKeyboardKey(Key key)
{
  const uint16_t usage = static_cast<uint16_t>(key);
  return usage != 0 && usage <= 0xff;
}

Key keyFromCode(InputCode code)
{
  if (code.domain != InputDomain::Keyboard)
  {
    return Key::None;
  }
  return static_cast<Key>(code.code);
}

InputEvent inputEvent(InputCode code, bool pressed, uint32_t timestampMs)
{
  return {code, pressed, timestampMs};
}

InputEvent keyEvent(Key key, bool pressed, uint32_t timestampMs)
{
  return inputEvent(keyboardCode(key), pressed, timestampMs);
}

InputValueEvent inputValueEvent(InputCode code, int16_t value, uint32_t timestampMs)
{
  return {code, value, timestampMs};
}

InputValueEvent pointerAxisValueEvent(PointerAxis axis, int16_t value, uint32_t timestampMs)
{
  return inputValueEvent(pointerAxisCode(axis), value, timestampMs);
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

bool isValid(InputCode code)
{
  return code.code != 0;
}

bool isModifierKey(Key key)
{
  return key == Key::LeftCtrl || key == Key::LeftShift || key == Key::LeftAlt || key == Key::LeftGui ||
         key == Key::RightCtrl || key == Key::RightShift || key == Key::RightAlt || key == Key::RightGui;
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
  case Key::D:
    return "D";
  case Key::E:
    return "E";
  case Key::F:
    return "F";
  case Key::G:
    return "G";
  case Key::H:
    return "H";
  case Key::I:
    return "I";
  case Key::J:
    return "J";
  case Key::K:
    return "K";
  case Key::L:
    return "L";
  case Key::M:
    return "M";
  case Key::N:
    return "N";
  case Key::O:
    return "O";
  case Key::P:
    return "P";
  case Key::Q:
    return "Q";
  case Key::R:
    return "R";
  case Key::S:
    return "S";
  case Key::T:
    return "T";
  case Key::U:
    return "U";
  case Key::V:
    return "V";
  case Key::W:
    return "W";
  case Key::X:
    return "X";
  case Key::Y:
    return "Y";
  case Key::Z:
    return "Z";
  case Key::Num1:
    return "Num1";
  case Key::Num2:
    return "Num2";
  case Key::Num3:
    return "Num3";
  case Key::Num4:
    return "Num4";
  case Key::Num5:
    return "Num5";
  case Key::Num6:
    return "Num6";
  case Key::Num7:
    return "Num7";
  case Key::Num8:
    return "Num8";
  case Key::Num9:
    return "Num9";
  case Key::Num0:
    return "Num0";
  case Key::Enter:
    return "Enter";
  case Key::Escape:
    return "Escape";
  case Key::Backspace:
    return "Backspace";
  case Key::Tab:
    return "Tab";
  case Key::Space:
    return "Space";
  case Key::Minus:
    return "Minus";
  case Key::Equal:
    return "Equal";
  case Key::LeftBracket:
    return "LeftBracket";
  case Key::RightBracket:
    return "RightBracket";
  case Key::Backslash:
    return "Backslash";
  case Key::NonUsHash:
    return "NonUsHash";
  case Key::Semicolon:
    return "Semicolon";
  case Key::Quote:
    return "Quote";
  case Key::Grave:
    return "Grave";
  case Key::Comma:
    return "Comma";
  case Key::Dot:
    return "Dot";
  case Key::Slash:
    return "Slash";
  case Key::Insert:
    return "Insert";
  case Key::CapsLock:
    return "CapsLock";
  case Key::F1:
    return "F1";
  case Key::F2:
    return "F2";
  case Key::F3:
    return "F3";
  case Key::F4:
    return "F4";
  case Key::F5:
    return "F5";
  case Key::F6:
    return "F6";
  case Key::F7:
    return "F7";
  case Key::F8:
    return "F8";
  case Key::F9:
    return "F9";
  case Key::F10:
    return "F10";
  case Key::F11:
    return "F11";
  case Key::F12:
    return "F12";
  case Key::PrintScreen:
    return "PrintScreen";
  case Key::ScrollLock:
    return "ScrollLock";
  case Key::Pause:
    return "Pause";
  case Key::Home:
    return "Home";
  case Key::PageUp:
    return "PageUp";
  case Key::Delete:
    return "Delete";
  case Key::End:
    return "End";
  case Key::PageDown:
    return "PageDown";
  case Key::Right:
    return "Right";
  case Key::Left:
    return "Left";
  case Key::Down:
    return "Down";
  case Key::Up:
    return "Up";
  case Key::NonUsBackslash:
    return "NonUsBackslash";
  case Key::International1:
    return "International1";
  case Key::International2:
    return "International2";
  case Key::International3:
    return "International3";
  case Key::International4:
    return "International4";
  case Key::International5:
    return "International5";
  case Key::International6:
    return "International6";
  case Key::International7:
    return "International7";
  case Key::International8:
    return "International8";
  case Key::International9:
    return "International9";
  case Key::Lang1:
    return "Lang1";
  case Key::Lang2:
    return "Lang2";
  case Key::Lang3:
    return "Lang3";
  case Key::Lang4:
    return "Lang4";
  case Key::Lang5:
    return "Lang5";
  case Key::Lang6:
    return "Lang6";
  case Key::Lang7:
    return "Lang7";
  case Key::Lang8:
    return "Lang8";
  case Key::Lang9:
    return "Lang9";
  case Key::LeftCtrl:
    return "LeftCtrl";
  case Key::LeftShift:
    return "LeftShift";
  case Key::LeftAlt:
    return "LeftAlt";
  case Key::LeftGui:
    return "LeftGui";
  case Key::RightCtrl:
    return "RightCtrl";
  case Key::RightShift:
    return "RightShift";
  case Key::RightAlt:
    return "RightAlt";
  case Key::RightGui:
    return "RightGui";
  case Key::Fn1:
    return "Fn1";
  }
  return "Unknown";
}

const char *consumerUsageName(ConsumerUsage usage)
{
  switch (usage)
  {
  case ConsumerUsage::None:
    return "None";
  case ConsumerUsage::Power:
    return "Power";
  case ConsumerUsage::Sleep:
    return "Sleep";
  case ConsumerUsage::Menu:
    return "Menu";
  case ConsumerUsage::Play:
    return "Play";
  case ConsumerUsage::Pause:
    return "Pause";
  case ConsumerUsage::Record:
    return "Record";
  case ConsumerUsage::FastForward:
    return "FastForward";
  case ConsumerUsage::Rewind:
    return "Rewind";
  case ConsumerUsage::ScanNextTrack:
    return "ScanNextTrack";
  case ConsumerUsage::ScanPreviousTrack:
    return "ScanPreviousTrack";
  case ConsumerUsage::Stop:
    return "Stop";
  case ConsumerUsage::Eject:
    return "Eject";
  case ConsumerUsage::PlayPause:
    return "PlayPause";
  case ConsumerUsage::Mute:
    return "Mute";
  case ConsumerUsage::BassBoost:
    return "BassBoost";
  case ConsumerUsage::VolumeIncrement:
    return "VolumeIncrement";
  case ConsumerUsage::VolumeDecrement:
    return "VolumeDecrement";
  case ConsumerUsage::BrowserSearch:
    return "BrowserSearch";
  case ConsumerUsage::BrowserHome:
    return "BrowserHome";
  case ConsumerUsage::BrowserBack:
    return "BrowserBack";
  case ConsumerUsage::BrowserForward:
    return "BrowserForward";
  case ConsumerUsage::BrowserRefresh:
    return "BrowserRefresh";
  case ConsumerUsage::BrowserBookmarks:
    return "BrowserBookmarks";
  }
  return "Unknown";
}

const char *pointerAxisName(PointerAxis axis)
{
  switch (axis)
  {
  case PointerAxis::None:
    return "None";
  case PointerAxis::X:
    return "X";
  case PointerAxis::Y:
    return "Y";
  case PointerAxis::Wheel:
    return "Wheel";
  case PointerAxis::Pan:
    return "Pan";
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
  if (!isValid(code))
  {
    return false;
  }
  if (contains(code))
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

bool InputState::mergeFrom(const InputState &other)
{
  bool mergedAll = true;
  for (size_t i = 0; i < other.codeCount(); ++i)
  {
    if (!press(other.codeAt(i)))
    {
      mergedAll = false;
    }
  }
  return mergedAll;
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

bool InputState::contains(Key key) const
{
  return contains(keyboardCode(key));
}

bool InputState::contains(InputCode code) const
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

bool InputState::isPressed(Key key) const
{
  return contains(key);
}

bool InputState::isPressed(InputCode code) const
{
  return contains(code);
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

void HidKeyboardReport::clear()
{
  modifiers = 0;
  for (size_t i = 0; i < MaxKeys; ++i)
  {
    keys[i] = 0;
  }
  keyCount = 0;
  overflow = false;
}

bool HidKeyboardReport::empty() const
{
  return modifiers == 0 && keyCount == 0 && !overflow;
}

bool HidKeyboardReport::writeBootReport(uint8_t *buffer, size_t size) const
{
  if (buffer == nullptr || size < BootReportSize)
  {
    return false;
  }

  buffer[0] = modifiers;
  buffer[1] = 0;
  for (size_t i = 0; i < MaxKeys; ++i)
  {
    buffer[2 + i] = i < keyCount ? keys[i] : 0;
  }
  return true;
}

void HidConsumerReport::clear()
{
  usage = 0;
  overflow = false;
}

bool HidConsumerReport::empty() const
{
  return usage == 0 && !overflow;
}

bool HidConsumerReport::writeReport(uint8_t *buffer, size_t size) const
{
  if (buffer == nullptr || size < ReportSize)
  {
    return false;
  }

  buffer[0] = static_cast<uint8_t>(usage & 0xff);
  buffer[1] = static_cast<uint8_t>((usage >> 8) & 0xff);
  return true;
}

void HidPointerReport::clear()
{
  buttons = 0;
  x = 0;
  y = 0;
  wheel = 0;
  pan = 0;
  overflow = false;
}

bool HidPointerReport::empty() const
{
  return buttons == 0 && x == 0 && y == 0 && wheel == 0 && pan == 0 && !overflow;
}

bool HidPointerReport::apply(InputValueEvent event)
{
  if (event.code.domain != InputDomain::PointerAxis || !isValid(event.code))
  {
    return false;
  }

  switch (static_cast<PointerAxis>(event.code.code))
  {
  case PointerAxis::X:
    x = addReportDelta(x, event.value, overflow);
    return true;
  case PointerAxis::Y:
    y = addReportDelta(y, event.value, overflow);
    return true;
  case PointerAxis::Wheel:
    wheel = addReportDelta(wheel, event.value, overflow);
    return true;
  case PointerAxis::Pan:
    pan = addReportDelta(pan, event.value, overflow);
    return true;
  case PointerAxis::None:
    break;
  }
  return false;
}

bool HidPointerReport::writeReport(uint8_t *buffer, size_t size) const
{
  if (buffer == nullptr || size < ReportSize)
  {
    return false;
  }

  buffer[0] = buttons;
  buffer[1] = static_cast<uint8_t>(x);
  buffer[2] = static_cast<uint8_t>(y);
  buffer[3] = static_cast<uint8_t>(wheel);
  buffer[4] = static_cast<uint8_t>(pan);
  return true;
}

HidKeyboardReport buildHidKeyboardReport(const InputState &state)
{
  HidKeyboardReport report;
  for (size_t i = 0; i < state.codeCount(); ++i)
  {
    const InputCode code = state.codeAt(i);
    if (code.domain != InputDomain::Keyboard)
    {
      continue;
    }

    const Key key = keyFromCode(code);
    const uint16_t usage = hidUsageFromKey(key);
    if (usage == 0)
    {
      continue;
    }

    if (usage >= 0xe0 && usage <= 0xe7)
    {
      report.modifiers |= static_cast<uint8_t>(1u << (usage - 0xe0));
      continue;
    }

    if (report.keyCount >= HidKeyboardReport::MaxKeys)
    {
      report.overflow = true;
      continue;
    }

    report.keys[report.keyCount++] = static_cast<uint8_t>(usage);
  }
  return report;
}

HidConsumerReport buildHidConsumerReport(const InputState &state)
{
  HidConsumerReport report;
  for (size_t i = 0; i < state.codeCount(); ++i)
  {
    const InputCode code = state.codeAt(i);
    if (code.domain != InputDomain::Consumer || !isValid(code))
    {
      continue;
    }

    if (report.usage != 0)
    {
      report.overflow = true;
      continue;
    }

    report.usage = code.code;
  }
  return report;
}

HidPointerReport buildHidPointerReport(const InputState &state)
{
  HidPointerReport report;
  for (size_t i = 0; i < state.codeCount(); ++i)
  {
    const InputCode code = state.codeAt(i);
    if (code.domain != InputDomain::PointerButton || !isValid(code))
    {
      continue;
    }

    if (code.code > HidPointerReport::MaxButtons)
    {
      report.overflow = true;
      continue;
    }

    report.buttons |= static_cast<uint8_t>(1u << (code.code - 1));
  }
  return report;
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

void RecordingHidKeyboardOutputAdapter::write(const InputState &state)
{
  report_ = buildHidKeyboardReport(state);
  ++writeCount_;
}

const HidKeyboardReport &RecordingHidKeyboardOutputAdapter::report() const
{
  return report_;
}

size_t RecordingHidKeyboardOutputAdapter::writeCount() const
{
  return writeCount_;
}

void RecordingHidKeyboardOutputAdapter::clear()
{
  report_.clear();
  writeCount_ = 0;
}

bool TransformConfig::remap(Key from, Key to)
{
  return remap(keyboardCode(from), keyboardCode(to));
}

bool TransformConfig::remap(InputCode from, InputCode to)
{
  if (!isValid(from) || !isValid(to))
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
  if (!isValid(code) || isDisabled(code))
  {
    return isValid(code);
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
