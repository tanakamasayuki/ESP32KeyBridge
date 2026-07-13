#include "ESP32KeyBridge.h"

namespace esp32keybridge
{

namespace
{
constexpr uint16_t kKeyboardModifierFirst = 0xe0;
constexpr uint16_t kKeyboardModifierLast = 0xe7;

constexpr uint16_t U(KeyboardUsage usage)
{
  return static_cast<uint16_t>(usage);
}

// clang-format off
const HostLayoutEntry kEnUsEntries[] = {
    {U(KeyboardUsage::A), 'a', 'A', true}, {U(KeyboardUsage::B), 'b', 'B', true},
    {U(KeyboardUsage::C), 'c', 'C', true}, {U(KeyboardUsage::D), 'd', 'D', true},
    {U(KeyboardUsage::E), 'e', 'E', true}, {U(KeyboardUsage::F), 'f', 'F', true},
    {U(KeyboardUsage::G), 'g', 'G', true}, {U(KeyboardUsage::H), 'h', 'H', true},
    {U(KeyboardUsage::I), 'i', 'I', true}, {U(KeyboardUsage::J), 'j', 'J', true},
    {U(KeyboardUsage::K), 'k', 'K', true}, {U(KeyboardUsage::L), 'l', 'L', true},
    {U(KeyboardUsage::M), 'm', 'M', true}, {U(KeyboardUsage::N), 'n', 'N', true},
    {U(KeyboardUsage::O), 'o', 'O', true}, {U(KeyboardUsage::P), 'p', 'P', true},
    {U(KeyboardUsage::Q), 'q', 'Q', true}, {U(KeyboardUsage::R), 'r', 'R', true},
    {U(KeyboardUsage::S), 's', 'S', true}, {U(KeyboardUsage::T), 't', 'T', true},
    {U(KeyboardUsage::U), 'u', 'U', true}, {U(KeyboardUsage::V), 'v', 'V', true},
    {U(KeyboardUsage::W), 'w', 'W', true}, {U(KeyboardUsage::X), 'x', 'X', true},
    {U(KeyboardUsage::Y), 'y', 'Y', true}, {U(KeyboardUsage::Z), 'z', 'Z', true},
    {U(KeyboardUsage::Digit1), '1', '!', false}, {U(KeyboardUsage::Digit2), '2', '@', false},
    {U(KeyboardUsage::Digit3), '3', '#', false}, {U(KeyboardUsage::Digit4), '4', '$', false},
    {U(KeyboardUsage::Digit5), '5', '%', false}, {U(KeyboardUsage::Digit6), '6', '^', false},
    {U(KeyboardUsage::Digit7), '7', '&', false}, {U(KeyboardUsage::Digit8), '8', '*', false},
    {U(KeyboardUsage::Digit9), '9', '(', false}, {U(KeyboardUsage::Digit0), '0', ')', false},
    {U(KeyboardUsage::Space), ' ', 0, false},
    {U(KeyboardUsage::Minus), '-', '_', false},
    {U(KeyboardUsage::Equal), '=', '+', false},
    {U(KeyboardUsage::LeftBracket), '[', '{', false},
    {U(KeyboardUsage::RightBracket), ']', '}', false},
    {U(KeyboardUsage::Backslash), '\\', '|', false},
    {U(KeyboardUsage::Semicolon), ';', ':', false},
    {U(KeyboardUsage::Quote), '\'', '"', false},
    {U(KeyboardUsage::Grave), '`', '~', false},
    {U(KeyboardUsage::Comma), ',', '<', false},
    {U(KeyboardUsage::Period), '.', '>', false},
    {U(KeyboardUsage::Slash), '/', '?', false},
};

// JIS layout as interpreted by a ja_jp host (Windows 106/109). The Grave key
// is Zenkaku/Hankaku (no character) and International2/4/5 are IME keys, so
// they are not in the table.
const HostLayoutEntry kJaJpEntries[] = {
    {U(KeyboardUsage::A), 'a', 'A', true}, {U(KeyboardUsage::B), 'b', 'B', true},
    {U(KeyboardUsage::C), 'c', 'C', true}, {U(KeyboardUsage::D), 'd', 'D', true},
    {U(KeyboardUsage::E), 'e', 'E', true}, {U(KeyboardUsage::F), 'f', 'F', true},
    {U(KeyboardUsage::G), 'g', 'G', true}, {U(KeyboardUsage::H), 'h', 'H', true},
    {U(KeyboardUsage::I), 'i', 'I', true}, {U(KeyboardUsage::J), 'j', 'J', true},
    {U(KeyboardUsage::K), 'k', 'K', true}, {U(KeyboardUsage::L), 'l', 'L', true},
    {U(KeyboardUsage::M), 'm', 'M', true}, {U(KeyboardUsage::N), 'n', 'N', true},
    {U(KeyboardUsage::O), 'o', 'O', true}, {U(KeyboardUsage::P), 'p', 'P', true},
    {U(KeyboardUsage::Q), 'q', 'Q', true}, {U(KeyboardUsage::R), 'r', 'R', true},
    {U(KeyboardUsage::S), 's', 'S', true}, {U(KeyboardUsage::T), 't', 'T', true},
    {U(KeyboardUsage::U), 'u', 'U', true}, {U(KeyboardUsage::V), 'v', 'V', true},
    {U(KeyboardUsage::W), 'w', 'W', true}, {U(KeyboardUsage::X), 'x', 'X', true},
    {U(KeyboardUsage::Y), 'y', 'Y', true}, {U(KeyboardUsage::Z), 'z', 'Z', true},
    {U(KeyboardUsage::Digit1), '1', '!', false}, {U(KeyboardUsage::Digit2), '2', '"', false},
    {U(KeyboardUsage::Digit3), '3', '#', false}, {U(KeyboardUsage::Digit4), '4', '$', false},
    {U(KeyboardUsage::Digit5), '5', '%', false}, {U(KeyboardUsage::Digit6), '6', '&', false},
    {U(KeyboardUsage::Digit7), '7', '\'', false}, {U(KeyboardUsage::Digit8), '8', '(', false},
    {U(KeyboardUsage::Digit9), '9', ')', false}, {U(KeyboardUsage::Digit0), '0', 0, false},
    {U(KeyboardUsage::Space), ' ', 0, false},
    {U(KeyboardUsage::Minus), '-', '=', false},
    {U(KeyboardUsage::Equal), '^', '~', false},
    {U(KeyboardUsage::LeftBracket), '@', '`', false},
    {U(KeyboardUsage::RightBracket), '[', '{', false},
    {U(KeyboardUsage::NonUsHash), ']', '}', false},
    {U(KeyboardUsage::Semicolon), ';', '+', false},
    {U(KeyboardUsage::Quote), ':', '*', false},
    {U(KeyboardUsage::Comma), ',', '<', false},
    {U(KeyboardUsage::Period), '.', '>', false},
    {U(KeyboardUsage::Slash), '/', '?', false},
    {U(KeyboardUsage::International1), '\\', '_', false},
    {U(KeyboardUsage::International3), 0x00a5, '|', false}, // Yen sign
};
// clang-format on

bool nameEquals(const char *a, const char *b)
{
  if (a == nullptr || b == nullptr)
  {
    return false;
  }
  size_t i = 0;
  while (a[i] != '\0' && b[i] != '\0')
  {
    if (a[i] != b[i])
    {
      return false;
    }
    ++i;
  }
  return a[i] == b[i];
}
} // namespace

HostLayout::HostLayout()
    : entries_(kEnUsEntries), count_(sizeof(kEnUsEntries) / sizeof(kEnUsEntries[0])),
      name_("en_us")
{
}

HostLayout::HostLayout(const HostLayoutEntry *entries, size_t count, const char *layoutName)
    : entries_(entries), count_(count), name_(layoutName)
{
}

HostLayout HostLayout::enUs()
{
  return HostLayout();
}

HostLayout HostLayout::jaJp()
{
  return HostLayout(kJaJpEntries, sizeof(kJaJpEntries) / sizeof(kJaJpEntries[0]), "ja_jp");
}

HostLayout HostLayout::byName(const char *name, bool *found)
{
  if (nameEquals(name, "ja_jp"))
  {
    if (found != nullptr)
    {
      *found = true;
    }
    return jaJp();
  }
  const bool isEnUs = nameEquals(name, "en_us");
  if (found != nullptr)
  {
    *found = isEnUs;
  }
  return enUs();
}

bool HostLayout::encode(char32_t codepoint, KeyStroke &stroke) const
{
  if (codepoint == 0)
  {
    return false;
  }
  for (size_t i = 0; i < count_; ++i)
  {
    if (entries_[i].base == codepoint)
    {
      stroke.key = keyboardKey(entries_[i].usage);
      stroke.shift = false;
      return true;
    }
  }
  for (size_t i = 0; i < count_; ++i)
  {
    if (entries_[i].shift == codepoint)
    {
      stroke.key = keyboardKey(entries_[i].usage);
      stroke.shift = true;
      return true;
    }
  }
  return false;
}

char32_t HostLayout::decode(Key key, bool shift) const
{
  if (key.kind != KeyKind::Keyboard)
  {
    return 0;
  }
  for (size_t i = 0; i < count_; ++i)
  {
    if (entries_[i].usage == key.code)
    {
      return shift ? entries_[i].shift : entries_[i].base;
    }
  }
  return 0;
}

bool HostLayout::capsAffects(Key key) const
{
  if (key.kind != KeyKind::Keyboard)
  {
    return false;
  }
  for (size_t i = 0; i < count_; ++i)
  {
    if (entries_[i].usage == key.code)
    {
      return entries_[i].capsAffects;
    }
  }
  return false;
}

const char *HostLayout::name() const
{
  return name_;
}

bool isValid(Key key)
{
  if (key.kind == KeyKind::None)
  {
    return false;
  }
  if (key.code == 0)
  {
    return false;
  }
  return true;
}

const char *keyKindName(KeyKind kind)
{
  switch (kind)
  {
  case KeyKind::None:
    return "none";
  case KeyKind::Keyboard:
    return "keyboard";
  case KeyKind::Consumer:
    return "consumer";
  case KeyKind::MouseButton:
    return "mouse_button";
  case KeyKind::Virtual:
    return "virtual";
  }
  return "unknown";
}

bool isKeyboardModifier(Key key)
{
  return key.kind == KeyKind::Keyboard && key.code >= kKeyboardModifierFirst &&
         key.code <= kKeyboardModifierLast;
}

uint8_t keyboardModifierMask(Key key)
{
  if (!isKeyboardModifier(key))
  {
    return 0;
  }
  return static_cast<uint8_t>(1u << (key.code - kKeyboardModifierFirst));
}

Key keyboardModifierFromBitIndex(uint8_t bitIndex)
{
  if (bitIndex > 7)
  {
    return Key();
  }
  return keyboardKey(static_cast<uint16_t>(kKeyboardModifierFirst + bitIndex));
}

void KeySet::clear()
{
  count_ = 0;
}

bool KeySet::press(Key key)
{
  if (!esp32keybridge::isValid(key))
  {
    return false;
  }
  if (contains(key))
  {
    return true;
  }
  if (count_ >= MaxKeys)
  {
    return false;
  }
  keys_[count_] = key;
  ++count_;
  return true;
}

bool KeySet::release(Key key)
{
  for (size_t i = 0; i < count_; ++i)
  {
    if (keys_[i] == key)
    {
      keys_[i] = keys_[count_ - 1];
      --count_;
      return true;
    }
  }
  return false;
}

bool KeySet::contains(Key key) const
{
  for (size_t i = 0; i < count_; ++i)
  {
    if (keys_[i] == key)
    {
      return true;
    }
  }
  return false;
}

bool KeySet::mergeFrom(const KeySet &other)
{
  bool ok = true;
  for (size_t i = 0; i < other.count_; ++i)
  {
    if (!press(other.keys_[i]))
    {
      ok = false;
    }
  }
  return ok;
}

size_t KeySet::count() const
{
  return count_;
}

Key KeySet::at(size_t index) const
{
  if (index >= count_)
  {
    return Key();
  }
  return keys_[index];
}

uint8_t KeySet::keyboardModifierMask() const
{
  uint8_t mask = 0;
  for (size_t i = 0; i < count_; ++i)
  {
    mask |= esp32keybridge::keyboardModifierMask(keys_[i]);
  }
  return mask;
}

bool KeySet::pressKeyboardModifiers(uint8_t mask)
{
  bool ok = true;
  for (uint8_t bit = 0; bit < 8; ++bit)
  {
    if ((mask & (1u << bit)) == 0)
    {
      continue;
    }
    if (!press(keyboardModifierFromBitIndex(bit)))
    {
      ok = false;
    }
  }
  return ok;
}

bool TransformConfig::remap(Key from, Key to)
{
  if (!esp32keybridge::isValid(from) || !esp32keybridge::isValid(to))
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
  remaps_[remapCount_].from = from;
  remaps_[remapCount_].to = to;
  ++remapCount_;
  return true;
}

bool TransformConfig::disable(Key key)
{
  if (!esp32keybridge::isValid(key))
  {
    return false;
  }
  if (isDisabled(key))
  {
    return true;
  }
  if (disabledCount_ >= MaxDisabledKeys)
  {
    return false;
  }
  disabled_[disabledCount_] = key;
  ++disabledCount_;
  return true;
}

void TransformConfig::clear()
{
  remapCount_ = 0;
  disabledCount_ = 0;
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
  for (size_t i = 0; i < disabledCount_; ++i)
  {
    if (disabled_[i] == key)
    {
      return true;
    }
  }
  return false;
}

bool TransformConfig::empty() const
{
  return remapCount_ == 0 && disabledCount_ == 0;
}

void LayerConfig::setTrigger(Key trigger)
{
  trigger_ = trigger;
}

bool LayerConfig::remap(Key from, Key to)
{
  if (!esp32keybridge::isValid(from) || !esp32keybridge::isValid(to))
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
  remaps_[remapCount_].from = from;
  remaps_[remapCount_].to = to;
  ++remapCount_;
  return true;
}

void LayerConfig::clear()
{
  trigger_ = Key();
  remapCount_ = 0;
}

bool LayerConfig::enabled() const
{
  return esp32keybridge::isValid(trigger_);
}

Key LayerConfig::trigger() const
{
  return trigger_;
}

Key LayerConfig::map(Key key) const
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

bool LayerConfig::hasMapping(Key key) const
{
  for (size_t i = 0; i < remapCount_; ++i)
  {
    if (remaps_[i].from == key)
    {
      return true;
    }
  }
  return false;
}

TransformConfig &ESP32KeyBridgeConfig::input(size_t index)
{
  if (index >= MaxInputConfigs)
  {
    dummyInput_.clear();
    return dummyInput_;
  }
  return inputs_[index];
}

const TransformConfig &ESP32KeyBridgeConfig::input(size_t index) const
{
  if (index >= MaxInputConfigs)
  {
    return dummyInput_;
  }
  return inputs_[index];
}

LayerConfig &ESP32KeyBridgeConfig::layer(size_t index)
{
  if (index >= MaxLayers)
  {
    dummyLayer_.clear();
    return dummyLayer_;
  }
  return layers_[index];
}

const LayerConfig &ESP32KeyBridgeConfig::layer(size_t index) const
{
  if (index >= MaxLayers)
  {
    return dummyLayer_;
  }
  return layers_[index];
}

namespace
{
// Minimal UTF-8 decoder. Returns the number of bytes consumed (0 on an
// invalid sequence) and stores the codepoint.
size_t decodeUtf8(const char *utf8, char32_t &codepoint)
{
  const uint8_t first = static_cast<uint8_t>(utf8[0]);
  if (first < 0x80)
  {
    codepoint = first;
    return 1;
  }
  size_t length = 0;
  char32_t value = 0;
  if ((first & 0xe0) == 0xc0)
  {
    length = 2;
    value = first & 0x1f;
  }
  else if ((first & 0xf0) == 0xe0)
  {
    length = 3;
    value = first & 0x0f;
  }
  else if ((first & 0xf8) == 0xf0)
  {
    length = 4;
    value = first & 0x07;
  }
  else
  {
    return 0;
  }
  for (size_t i = 1; i < length; ++i)
  {
    const uint8_t byte = static_cast<uint8_t>(utf8[i]);
    if ((byte & 0xc0) != 0x80)
    {
      return 0;
    }
    value = (value << 6) | (byte & 0x3f);
  }
  codepoint = value;
  return length;
}
} // namespace

bool ESP32KeyBridgeConfig::textMacro(Key trigger, const char *utf8)
{
  if (!esp32keybridge::isValid(trigger) || utf8 == nullptr)
  {
    return false;
  }

  TextMacro *slot = nullptr;
  for (size_t i = 0; i < textMacroCount_; ++i)
  {
    if (textMacros_[i].trigger == trigger)
    {
      slot = &textMacros_[i];
      break;
    }
  }
  if (slot == nullptr)
  {
    if (textMacroCount_ >= MaxTextMacros)
    {
      return false;
    }
    slot = &textMacros_[textMacroCount_];
  }

  TextMacro macro;
  macro.trigger = trigger;
  size_t pos = 0;
  while (utf8[pos] != '\0')
  {
    char32_t codepoint = 0;
    const size_t consumed = decodeUtf8(utf8 + pos, codepoint);
    if (consumed == 0 || macro.length >= TextMacro::MaxLength)
    {
      return false;
    }
    macro.text[macro.length] = codepoint;
    ++macro.length;
    pos += consumed;
  }

  const bool isNew = slot == &textMacros_[textMacroCount_];
  *slot = macro;
  if (isNew)
  {
    ++textMacroCount_;
  }
  return true;
}

const TextMacro *ESP32KeyBridgeConfig::findTextMacro(Key trigger) const
{
  for (size_t i = 0; i < textMacroCount_; ++i)
  {
    if (textMacros_[i].trigger == trigger)
    {
      return &textMacros_[i];
    }
  }
  return nullptr;
}

void ESP32KeyBridgeConfig::convertLayout(size_t inputIndex, const HostLayout &engraving)
{
  if (inputIndex >= MaxInputConfigs)
  {
    return;
  }
  inputLayouts_[inputIndex] = engraving;
  inputLayoutEnabled_[inputIndex] = true;
}

bool ESP32KeyBridgeConfig::inputLayoutEnabled(size_t index) const
{
  if (index >= MaxInputConfigs)
  {
    return false;
  }
  return inputLayoutEnabled_[index];
}

const HostLayout &ESP32KeyBridgeConfig::inputLayout(size_t index) const
{
  if (index >= MaxInputConfigs)
  {
    return inputLayouts_[0];
  }
  return inputLayouts_[index];
}

void ESP32KeyBridgeConfig::setAxisScale(Axis axis, int16_t scale)
{
  axisScales_[static_cast<size_t>(axis)] = scale;
}

int16_t ESP32KeyBridgeConfig::axisScale(Axis axis) const
{
  return axisScales_[static_cast<size_t>(axis)];
}

void ESP32KeyBridgeConfig::clear()
{
  global.clear();
  for (size_t i = 0; i < MaxInputConfigs; ++i)
  {
    inputs_[i].clear();
  }
  for (size_t i = 0; i < MaxLayers; ++i)
  {
    layers_[i].clear();
  }
  for (size_t i = 0; i < MaxTextMacros; ++i)
  {
    textMacros_[i] = TextMacro();
  }
  textMacroCount_ = 0;
  for (size_t i = 0; i < kAxisCount; ++i)
  {
    axisScales_[i] = 1;
  }
  for (size_t i = 0; i < MaxInputConfigs; ++i)
  {
    inputLayouts_[i] = HostLayout::enUs();
    inputLayoutEnabled_[i] = false;
  }
  layoutConversionToggle = Key();
  hostLayout = HostLayout::enUs();
  deferTypingWhileModifiersHeld = false;
}

namespace
{
int8_t saturateToInt8(int32_t value)
{
  if (value > 127)
  {
    return 127;
  }
  if (value < -127)
  {
    return -127;
  }
  return static_cast<int8_t>(value);
}
} // namespace

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
  return modifiers == 0 && keyCount == 0;
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

void HidKeyboardRolloverReport::clear()
{
  modifiers = 0;
  for (size_t i = 0; i < MaxKeys; ++i)
  {
    keys[i] = 0;
  }
  keyCount = 0;
  overflow = false;
}

bool HidKeyboardRolloverReport::empty() const
{
  return modifiers == 0 && keyCount == 0;
}

bool HidKeyboardRolloverReport::writeReport(uint8_t *buffer, size_t size) const
{
  if (buffer == nullptr || size < ReportSize)
  {
    return false;
  }
  buffer[0] = modifiers;
  for (size_t i = 0; i < MaxKeys; ++i)
  {
    buffer[1 + i] = i < keyCount ? keys[i] : 0;
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
  return usage == 0;
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

void HidMouseReport::clear()
{
  buttons = 0;
  x = 0;
  y = 0;
  wheel = 0;
  pan = 0;
  overflow = false;
}

bool HidMouseReport::empty() const
{
  return buttons == 0 && x == 0 && y == 0 && wheel == 0 && pan == 0;
}

int32_t HidMouseReport::applyAxisDelta(Axis axis, int32_t delta)
{
  int8_t *field = nullptr;
  switch (axis)
  {
  case Axis::X:
    field = &x;
    break;
  case Axis::Y:
    field = &y;
    break;
  case Axis::Wheel:
    field = &wheel;
    break;
  case Axis::Pan:
    field = &pan;
    break;
  }
  if (field == nullptr)
  {
    return delta;
  }
  const int32_t total = static_cast<int32_t>(*field) + delta;
  *field = saturateToInt8(total);
  return total - *field;
}

bool HidMouseReport::writeReport(uint8_t *buffer, size_t size) const
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

HidKeyboardReport buildHidKeyboardReport(const KeySet &keys)
{
  HidKeyboardReport report;
  for (size_t i = 0; i < keys.count(); ++i)
  {
    const Key key = keys.at(i);
    if (key.kind != KeyKind::Keyboard)
    {
      continue;
    }
    const uint8_t modifierMask = keyboardModifierMask(key);
    if (modifierMask != 0)
    {
      report.modifiers |= modifierMask;
      continue;
    }
    if (key.code > 0xff)
    {
      continue;
    }
    if (report.keyCount >= HidKeyboardReport::MaxKeys)
    {
      report.overflow = true;
      continue;
    }
    report.keys[report.keyCount] = static_cast<uint8_t>(key.code);
    ++report.keyCount;
  }
  return report;
}

HidKeyboardRolloverReport buildHidKeyboardRolloverReport(const KeySet &keys)
{
  HidKeyboardRolloverReport report;
  for (size_t i = 0; i < keys.count(); ++i)
  {
    const Key key = keys.at(i);
    if (key.kind != KeyKind::Keyboard)
    {
      continue;
    }
    const uint8_t modifierMask = keyboardModifierMask(key);
    if (modifierMask != 0)
    {
      report.modifiers |= modifierMask;
      continue;
    }
    if (key.code > 0xff)
    {
      continue;
    }
    if (report.keyCount >= HidKeyboardRolloverReport::MaxKeys)
    {
      report.overflow = true;
      continue;
    }
    report.keys[report.keyCount] = static_cast<uint8_t>(key.code);
    ++report.keyCount;
  }
  return report;
}

HidConsumerReport buildHidConsumerReport(const KeySet &keys)
{
  HidConsumerReport report;
  for (size_t i = 0; i < keys.count(); ++i)
  {
    const Key key = keys.at(i);
    if (key.kind != KeyKind::Consumer)
    {
      continue;
    }
    if (report.usage != 0)
    {
      report.overflow = true;
      continue;
    }
    report.usage = key.code;
  }
  return report;
}

HidMouseReport buildHidMouseReport(const KeySet &keys)
{
  HidMouseReport report;
  for (size_t i = 0; i < keys.count(); ++i)
  {
    const Key key = keys.at(i);
    if (key.kind != KeyKind::MouseButton)
    {
      continue;
    }
    if (key.code < 1 || key.code > HidMouseReport::MaxButtons)
    {
      report.overflow = true;
      continue;
    }
    report.buttons |= static_cast<uint8_t>(1u << (key.code - 1));
  }
  return report;
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
  inputs_[inputCount_] = &input;
  inputConfigIndexes_[inputCount_] = configIndex;
  inputWasConnected_[inputCount_] = false; // first update pushes the lock state
  ++inputCount_;
  return true;
}

void ESP32KeyBridge::clearInputs()
{
  inputCount_ = 0;
  heldCount_ = 0;
  for (size_t i = 0; i < ESP32KeyBridgeConfig::MaxLayers; ++i)
  {
    layerHoldCounts_[i] = 0;
  }
  merged_.clear();
  output_.clear();
  mergedOverflow_ = false;
  outputOverflow_ = false;
  for (size_t i = 0; i < 3; ++i)
  {
    prevLockKeyPressed_[i] = false;
  }
}

size_t ESP32KeyBridge::inputCount() const
{
  return inputCount_;
}

bool ESP32KeyBridge::addOutput(OutputAdapter &output)
{
  if (outputCount_ >= MaxOutputs)
  {
    return false;
  }
  outputs_[outputCount_] = &output;
  ++outputCount_;
  return true;
}

void ESP32KeyBridge::clearOutputs()
{
  outputCount_ = 0;
  lockAuthorityPresent_ = false;
}

size_t ESP32KeyBridge::outputCount() const
{
  return outputCount_;
}

bool ESP32KeyBridge::validateConfig(const ESP32KeyBridgeConfig &config,
                                    ESP32KeyBridgeConfigError &error) const
{
  (void)config;
  error.message = nullptr;
  return true;
}

void ESP32KeyBridge::applyConfig(const ESP32KeyBridgeConfig &config)
{
  // Held presses keep their press-time resolution; only new presses see the
  // new configuration.
  config_ = config;
}

void ESP32KeyBridge::begin()
{
}

size_t ESP32KeyBridge::findHeld(uint8_t inputIndex, Key source) const
{
  for (size_t i = 0; i < heldCount_; ++i)
  {
    if (held_[i].inputIndex == inputIndex && held_[i].source == source)
    {
      return i;
    }
  }
  return MaxHeldKeys;
}

void ESP32KeyBridge::removeHeld(size_t index)
{
  for (uint8_t layer = 0; layer < ESP32KeyBridgeConfig::MaxLayers; ++layer)
  {
    if ((held_[index].layerTriggerMask & (1u << layer)) != 0 && layerHoldCounts_[layer] > 0)
    {
      --layerHoldCounts_[layer];
    }
  }
  held_[index] = held_[heldCount_ - 1];
  --heldCount_;
}

uint8_t ESP32KeyBridge::layerTriggerMaskFor(Key key) const
{
  uint8_t mask = 0;
  for (uint8_t layer = 0; layer < ESP32KeyBridgeConfig::MaxLayers; ++layer)
  {
    const LayerConfig &config = config_.layer(layer);
    if (config.enabled() && config.trigger() == key)
    {
      mask |= static_cast<uint8_t>(1u << layer);
    }
  }
  return mask;
}

Key ESP32KeyBridge::applyLayerOverlay(Key key) const
{
  for (uint8_t layer = 0; layer < ESP32KeyBridgeConfig::MaxLayers; ++layer)
  {
    if (layerHoldCounts_[layer] == 0)
    {
      continue;
    }
    const LayerConfig &config = config_.layer(layer);
    if (config.hasMapping(key))
    {
      return config.map(key);
    }
  }
  return key;
}

void ESP32KeyBridge::resolvePress(uint8_t inputIndex, Key source, bool triggersOnly)
{
  const size_t configIndex = inputConfigIndexes_[inputIndex];
  const TransformConfig &perInput = config_.input(configIndex);

  Key key = source;
  bool converted = false;
  bool requiresShift = false;
  bool dropped = false;       // conversion produced an untypable character
  bool shiftConsumed = false; // Shift of a converting input, used for decode

  // Layout conversion runs first, on the raw physical key of the engraving.
  if (layoutConversionEnabled_ && config_.inputLayoutEnabled(configIndex) &&
      source.kind == KeyKind::Keyboard)
  {
    const KeySet &inputKeys = inputs_[inputIndex]->keys();
    const Key leftShift = keyboardKey(KeyboardUsage::LeftShift);
    const Key rightShift = keyboardKey(KeyboardUsage::RightShift);
    if (source == leftShift || source == rightShift)
    {
      shiftConsumed = true;
    }
    else
    {
      // Shortcut rule: keys pressed while Ctrl/Alt/GUI is held on the same
      // input pass through unconverted (position semantics).
      const bool shortcut = inputKeys.contains(keyboardKey(KeyboardUsage::LeftCtrl)) ||
                            inputKeys.contains(keyboardKey(KeyboardUsage::RightCtrl)) ||
                            inputKeys.contains(keyboardKey(KeyboardUsage::LeftAlt)) ||
                            inputKeys.contains(keyboardKey(KeyboardUsage::RightAlt)) ||
                            inputKeys.contains(keyboardKey(KeyboardUsage::LeftGui)) ||
                            inputKeys.contains(keyboardKey(KeyboardUsage::RightGui));
      if (!shortcut)
      {
        const bool shifted = inputKeys.contains(leftShift) || inputKeys.contains(rightShift);
        const char32_t codepoint = config_.inputLayout(configIndex).decode(source, shifted);
        if (codepoint != 0)
        {
          KeyStroke stroke;
          if (config_.hostLayout.encode(codepoint, stroke))
          {
            key = stroke.key;
            requiresShift = stroke.shift;
            converted = true;
          }
          else
          {
            // The host layout cannot type this character (e.g. Yen on
            // en_us): drop the press, never emit a wrong key.
            converted = true;
            dropped = true;
          }
        }
      }
    }
  }

  bool disabled = false;
  uint8_t triggerMask = 0;
  if (!shiftConsumed && !dropped)
  {
    if (!converted)
    {
      key = perInput.map(key);
      disabled = perInput.isDisabled(key);
    }
    if (!disabled)
    {
      key = config_.global.map(key);
      triggerMask = layerTriggerMaskFor(key);
    }
  }

  if (triggersOnly && triggerMask == 0)
  {
    return;
  }

  if (heldCount_ >= MaxHeldKeys)
  {
    outputOverflow_ = true;
    return;
  }

  HeldKey &entry = held_[heldCount_];
  entry.inputIndex = inputIndex;
  entry.source = source;
  entry.resolved = Key();
  entry.layerTriggerMask = triggerMask;
  entry.converted = converted;
  entry.requiresShift = converted && requiresShift;

  // Side effects live below the triggersOnly gate so the two press passes
  // cannot double-apply them.
  if (dropped)
  {
    ++layoutConvertFailCount_;
  }

  if (triggerMask != 0)
  {
    // Layer triggers are consumed and never emitted.
    for (uint8_t layer = 0; layer < ESP32KeyBridgeConfig::MaxLayers; ++layer)
    {
      if ((triggerMask & (1u << layer)) != 0)
      {
        ++layerHoldCounts_[layer];
      }
    }
  }
  else if (!disabled && !shiftConsumed && !dropped)
  {
    if (esp32keybridge::isValid(config_.layoutConversionToggle) &&
        key == config_.layoutConversionToggle)
    {
      // Consumed; toggles conversion for new presses only (held keys keep
      // their press-time resolution).
      layoutConversionEnabled_ = !layoutConversionEnabled_;
    }
    else
    {
      const TextMacro *macro = config_.findTextMacro(key);
      if (macro != nullptr)
      {
        // Text macro triggers are consumed; the press enqueues the text.
        enqueueMacro(*macro);
      }
      else
      {
        key = applyLayerOverlay(key);
        if (!config_.global.isDisabled(key))
        {
          entry.resolved = key;
        }
      }
    }
  }

  ++heldCount_;
}

void ESP32KeyBridge::update()
{
  outputOverflow_ = false;

  for (size_t i = 0; i < inputCount_; ++i)
  {
    inputs_[i]->update();
  }

  // Releases first: a trigger released and a key pressed in the same update
  // resolve without the layer.
  for (size_t i = heldCount_; i > 0; --i)
  {
    const HeldKey &entry = held_[i - 1];
    const InputAdapter *input = inputs_[entry.inputIndex];
    if (!input->connected() || !input->keys().contains(entry.source))
    {
      removeHeld(i - 1);
    }
  }

  // Presses in two passes: layer triggers first, then everything else, so a
  // trigger and a target key arriving in the same update apply the layer.
  for (int pass = 0; pass < 2; ++pass)
  {
    const bool triggersOnly = pass == 0;
    for (size_t i = 0; i < inputCount_; ++i)
    {
      if (!inputs_[i]->connected())
      {
        continue;
      }
      const KeySet &keys = inputs_[i]->keys();
      for (size_t k = 0; k < keys.count(); ++k)
      {
        Key source = keys.at(k);
        if (findHeld(static_cast<uint8_t>(i), source) != MaxHeldKeys)
        {
          continue;
        }
        resolvePress(static_cast<uint8_t>(i), source, triggersOnly);
      }
    }
  }

  merged_.clear();
  mergedOverflow_ = false;
  for (size_t i = 0; i < inputCount_; ++i)
  {
    if (!inputs_[i]->connected())
    {
      continue;
    }
    if (!merged_.mergeFrom(inputs_[i]->keys()))
    {
      mergedOverflow_ = true;
    }
  }

  output_.clear();
  bool synthesizeShift = false;
  for (size_t i = 0; i < heldCount_; ++i)
  {
    if (!esp32keybridge::isValid(held_[i].resolved))
    {
      continue;
    }
    if (!output_.press(held_[i].resolved))
    {
      outputOverflow_ = true;
    }
    if (held_[i].requiresShift)
    {
      synthesizeShift = true;
    }
  }
  if (synthesizeShift)
  {
    output_.press(keyboardKey(KeyboardUsage::LeftShift));
  }

  // A converting input's Shift keys are consumed for decoding, but while a
  // non-converted key of that input is held (arrows, shortcuts), its real
  // Shift passes through so Shift+Arrow selection works.
  for (size_t i = 0; i < inputCount_; ++i)
  {
    if (!inputs_[i]->connected() || !layoutConversionEnabled_ ||
        !config_.inputLayoutEnabled(inputConfigIndexes_[i]))
    {
      continue;
    }
    const KeySet &inputKeys = inputs_[i]->keys();
    const bool physLeft = inputKeys.contains(keyboardKey(KeyboardUsage::LeftShift));
    const bool physRight = inputKeys.contains(keyboardKey(KeyboardUsage::RightShift));
    if (!physLeft && !physRight)
    {
      continue;
    }
    for (size_t h = 0; h < heldCount_; ++h)
    {
      const HeldKey &entry = held_[h];
      if (entry.inputIndex != i || entry.converted || !esp32keybridge::isValid(entry.resolved) ||
          entry.resolved.kind != KeyKind::Keyboard || isKeyboardModifier(entry.resolved))
      {
        continue;
      }
      if (physLeft)
      {
        output_.press(keyboardKey(KeyboardUsage::LeftShift));
      }
      if (physRight)
      {
        output_.press(keyboardKey(KeyboardUsage::RightShift));
      }
      break;
    }
  }

  stepTyping();
  updateAxes();
  updateLockState();

  for (size_t i = 0; i < outputCount_; ++i)
  {
    if (outputs_[i]->connected())
    {
      outputs_[i]->write(output_);
    }
  }
}

bool ESP32KeyBridge::enqueueChar(char32_t codepoint)
{
  // CRLF is normalized to a single Enter at the enqueue edge.
  const bool skipLF = codepoint == '\n' && lastEnqueuedCR_;
  lastEnqueuedCR_ = codepoint == '\r';
  if (skipLF)
  {
    return true;
  }
  if (textCount_ >= MaxTextQueue)
  {
    ++textOverflowCount_;
    return false;
  }
  textQueue_[(textHead_ + textCount_) % MaxTextQueue] = codepoint;
  ++textCount_;
  return true;
}

bool ESP32KeyBridge::typeChar(char32_t codepoint)
{
  return enqueueChar(codepoint);
}

bool ESP32KeyBridge::typeText(const char *utf8)
{
  if (utf8 == nullptr)
  {
    return false;
  }
  bool ok = true;
  size_t pos = 0;
  while (utf8[pos] != '\0')
  {
    char32_t codepoint = 0;
    const size_t consumed = decodeUtf8(utf8 + pos, codepoint);
    if (consumed == 0)
    {
      ++textEncodeFailCount_;
      ++pos;
      ok = false;
      continue;
    }
    if (!enqueueChar(codepoint))
    {
      ok = false;
    }
    pos += consumed;
  }
  return ok;
}

void ESP32KeyBridge::enqueueMacro(const TextMacro &macro)
{
  for (size_t i = 0; i < macro.length; ++i)
  {
    enqueueChar(macro.text[i]);
  }
}

bool ESP32KeyBridge::encodeCharForTyping(char32_t codepoint, KeyStroke &stroke) const
{
  switch (codepoint)
  {
  case '\t':
    stroke.key = keyboardKey(KeyboardUsage::Tab);
    stroke.shift = false;
    return true;
  case '\n':
  case '\r':
    stroke.key = keyboardKey(KeyboardUsage::Enter);
    stroke.shift = false;
    return true;
  case 0x08:
    stroke.key = keyboardKey(KeyboardUsage::Backspace);
    stroke.shift = false;
    return true;
  case 0x1b:
    stroke.key = keyboardKey(KeyboardUsage::Escape);
    stroke.shift = false;
    return true;
  default:
    break;
  }
  if (codepoint < 0x20 || codepoint == 0x7f)
  {
    return false;
  }
  if (!config_.hostLayout.encode(codepoint, stroke))
  {
    return false;
  }
  // Caps Lock compensation against the lock state shadow.
  if (lockState_.capsLock && config_.hostLayout.capsAffects(stroke.key))
  {
    stroke.shift = !stroke.shift;
  }
  return true;
}

void ESP32KeyBridge::stepTyping()
{
  // The user's keyboard modifier state before any suppression: used for the
  // defer option and restored automatically once typing goes idle.
  const uint8_t userModifiers = output_.keyboardModifierMask();

  if (!typingActive_)
  {
    if (textCount_ == 0)
    {
      return;
    }
    if (config_.deferTypingWhileModifiersHeld && userModifiers != 0)
    {
      return;
    }
    // Consume characters until one produces a stroke. Every consumed
    // character is delivered to text-native outputs, even unencodable ones.
    while (textCount_ > 0 && !typingActive_)
    {
      const char32_t codepoint = textQueue_[textHead_];
      textHead_ = (textHead_ + 1) % MaxTextQueue;
      --textCount_;
      for (size_t i = 0; i < outputCount_; ++i)
      {
        if (outputs_[i]->connected())
        {
          outputs_[i]->writeText(codepoint);
        }
      }
      KeyStroke stroke;
      if (encodeCharForTyping(codepoint, stroke))
      {
        typingStroke_ = stroke;
        typingPhase_ = 0;
        typingActive_ = true;
      }
      else
      {
        ++textEncodeFailCount_;
      }
    }
    if (!typingActive_)
    {
      return;
    }
  }

  // One atomic character frame, one phase per update: (0) modifiers only,
  // (1) modifiers + key, (2) modifiers only. User-held keyboard modifiers
  // are parked during the frame and reappear when typing goes idle.
  KeySet result;
  for (size_t i = 0; i < output_.count(); ++i)
  {
    const Key key = output_.at(i);
    if (isKeyboardModifier(key))
    {
      continue;
    }
    result.press(key);
  }
  if (typingStroke_.shift)
  {
    result.press(keyboardKey(KeyboardUsage::LeftShift));
  }
  if (typingPhase_ == 1)
  {
    result.press(typingStroke_.key);
  }
  output_ = result;

  ++typingPhase_;
  if (typingPhase_ >= 3)
  {
    typingActive_ = false;
  }
}

void ESP32KeyBridge::updateAxes()
{
  for (size_t axis = 0; axis < kAxisCount; ++axis)
  {
    int32_t total = 0;
    for (size_t i = 0; i < inputCount_; ++i)
    {
      // Drain disconnected inputs too so stale deltas are not replayed on
      // reconnect, but only connected inputs contribute.
      const int32_t delta = inputs_[i]->takeAxisDelta(static_cast<Axis>(axis));
      if (inputs_[i]->connected())
      {
        total += delta;
      }
    }
    total *= config_.axisScale(static_cast<Axis>(axis));
    axisDeltas_[axis] = total;
    if (total == 0)
    {
      continue;
    }
    for (size_t i = 0; i < outputCount_; ++i)
    {
      if (outputs_[i]->connected())
      {
        outputs_[i]->writeAxisDelta(static_cast<Axis>(axis), total);
      }
    }
  }
}

size_t ESP32KeyBridge::textQueueLength() const
{
  return textCount_;
}

bool ESP32KeyBridge::typingActive() const
{
  return typingActive_;
}

uint32_t ESP32KeyBridge::textOverflowCount() const
{
  return textOverflowCount_;
}

uint32_t ESP32KeyBridge::textEncodeFailCount() const
{
  return textEncodeFailCount_;
}

int32_t ESP32KeyBridge::axisDelta(Axis axis) const
{
  return axisDeltas_[static_cast<size_t>(axis)];
}

void ESP32KeyBridge::setLayoutConversionEnabled(bool enabled)
{
  layoutConversionEnabled_ = enabled;
}

bool ESP32KeyBridge::layoutConversionEnabled() const
{
  return layoutConversionEnabled_;
}

uint32_t ESP32KeyBridge::layoutConvertFailCount() const
{
  return layoutConvertFailCount_;
}

void ESP32KeyBridge::updateLockState()
{
  // Authority: the first registered, connected, lock-reporting output.
  const OutputAdapter *authority = nullptr;
  for (size_t i = 0; i < outputCount_; ++i)
  {
    if (outputs_[i]->reportsLockState() && outputs_[i]->connected())
    {
      authority = outputs_[i];
      break;
    }
  }
  lockAuthorityPresent_ = authority != nullptr;

  bool changed = false;

  // Track press transitions of the lock keys in the transformed output. Kept
  // up to date in both modes so that losing the authority while a lock key is
  // held does not cause a spurious toggle.
  const Key lockKeys[3] = {
      keyboardKey(KeyboardUsage::CapsLock),
      keyboardKey(KeyboardUsage::NumLock),
      keyboardKey(KeyboardUsage::ScrollLock),
  };
  bool *lockBits[3] = {&lockState_.capsLock, &lockState_.numLock, &lockState_.scrollLock};
  for (size_t i = 0; i < 3; ++i)
  {
    const bool pressed = output_.contains(lockKeys[i]);
    const bool newPress = pressed && !prevLockKeyPressed_[i];
    prevLockKeyPressed_[i] = pressed;

    // Terminal host mode: the bridge owns the lock state and toggles on
    // press. Kana has no standard toggle key and is never self-toggled.
    if (!lockAuthorityPresent_ && newPress)
    {
      *lockBits[i] = !*lockBits[i];
      changed = true;
    }
  }

  if (authority != nullptr)
  {
    LockState reported;
    if (authority->getLockState(reported) && reported != lockState_)
    {
      lockState_ = reported;
      changed = true;
    }
  }

  // Notify inputs on change, and push the current state to inputs seen
  // connected for the first time (hosts only report changes, so late-joining
  // keyboards need the push).
  for (size_t i = 0; i < inputCount_; ++i)
  {
    const bool connected = inputs_[i]->connected();
    if (connected && (changed || !inputWasConnected_[i]))
    {
      inputs_[i]->setLockState(lockState_);
    }
    inputWasConnected_[i] = connected;
  }
}

const KeySet &ESP32KeyBridge::mergedKeys() const
{
  return merged_;
}

const LockState &ESP32KeyBridge::lockState() const
{
  return lockState_;
}

bool ESP32KeyBridge::lockAuthorityPresent() const
{
  return lockAuthorityPresent_;
}

const KeySet &ESP32KeyBridge::outputKeys() const
{
  return output_;
}

bool ESP32KeyBridge::mergedOverflow() const
{
  return mergedOverflow_;
}

bool ESP32KeyBridge::outputOverflow() const
{
  return outputOverflow_;
}

} // namespace esp32keybridge
