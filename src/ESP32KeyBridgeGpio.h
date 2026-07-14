#pragma once

#include <ESP32KeyBridge.h>

// GPIO input adapters (no external library; Arduino GPIO only).
//
// MOCK: this header currently contains build-only skeletons that fix the
// sketch-facing API. The real implementations land with implementation
// step 7.

namespace esp32keybridge
{

// One GPIO pin pressed as one key (foot switch, panel button). The pin is
// configured (pull-up, active-low by default) on the first update();
// debouncing happens inside update(). No begin().
class GpioKeyInputAdapter : public InputAdapter
{
public:
  GpioKeyInputAdapter(uint8_t pin, Key key, bool activeLow = true)
      : pin_(pin), key_(key), activeLow_(activeLow)
  {
  }

  void update() override {}
  const KeySet &keys() const override { return keys_; }

private:
  uint8_t pin_;
  Key key_;
  bool activeLow_;
  KeySet keys_;
};

} // namespace esp32keybridge
