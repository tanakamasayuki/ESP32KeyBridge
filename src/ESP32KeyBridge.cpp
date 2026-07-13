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
  const TransformConfig &perInput = config_.input(inputConfigIndexes_[inputIndex]);

  Key key = perInput.map(source);
  bool disabled = perInput.isDisabled(key);

  uint8_t triggerMask = 0;
  if (!disabled)
  {
    key = config_.global.map(key);
    triggerMask = layerTriggerMaskFor(key);
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
  else if (!disabled)
  {
    key = applyLayerOverlay(key);
    if (!config_.global.isDisabled(key))
    {
      entry.resolved = key;
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
  }

  updateLockState();

  for (size_t i = 0; i < outputCount_; ++i)
  {
    if (outputs_[i]->connected())
    {
      outputs_[i]->write(output_);
    }
  }
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
