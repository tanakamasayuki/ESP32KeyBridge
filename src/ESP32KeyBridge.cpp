#include "ESP32KeyBridge.h"

namespace esp32keybridge
{

namespace
{
constexpr uint16_t kKeyboardModifierFirst = 0xe0;
constexpr uint16_t kKeyboardModifierLast = 0xe7;
} // namespace

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

bool ESP32KeyBridge::addInput(InputAdapter &input)
{
  if (inputCount_ >= MaxInputs)
  {
    return false;
  }
  inputs_[inputCount_] = &input;
  ++inputCount_;
  return true;
}

void ESP32KeyBridge::clearInputs()
{
  inputCount_ = 0;
  merged_.clear();
  mergedOverflow_ = false;
}

size_t ESP32KeyBridge::inputCount() const
{
  return inputCount_;
}

void ESP32KeyBridge::begin()
{
}

void ESP32KeyBridge::update()
{
  for (size_t i = 0; i < inputCount_; ++i)
  {
    inputs_[i]->update();
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
}

const KeySet &ESP32KeyBridge::mergedKeys() const
{
  return merged_;
}

bool ESP32KeyBridge::mergedOverflow() const
{
  return mergedOverflow_;
}

} // namespace esp32keybridge
