#pragma once

#include <ESP32KeyBridge.h>

#if __has_include(<soc/soc_caps.h>)
#include <soc/soc_caps.h>
#endif

// GPIO input adapters (no external library; Arduino GPIO only).
//
// MOCK: this header currently contains build-only skeletons that fix the
// sketch-facing API. The real implementations land with implementation
// step 7.

namespace esp32keybridge
{

// GPIO pins pressed as keys (foot switches, panel buttons, a few
// direct-wired keys). Register each pin with addKey(); the pins are
// configured on the first update() and debounced inside update(). No
// begin(). Matrix scanning is a separate adapter.
class GpioKeyInputAdapter : public InputAdapter
{
public:
  // Bounded by the chip's GPIO count (an entry costs ~a dozen bytes, so
  // this is not a memory concern). Note that more than KeySet::MaxKeys
  // (32) keys held down at the same time overflows the pressed set.
#if defined(SOC_GPIO_PIN_COUNT)
  static constexpr size_t MaxKeys = SOC_GPIO_PIN_COUNT;
#else
  static constexpr size_t MaxKeys = 32; // host build fallback
#endif

  // Maps a pin to the key it presses. activeLow: LOW = pressed (switch to
  // GND); pullUp: enable the internal pull-up. Returns false when the
  // table is full.
  bool addKey(uint8_t pin, Key key, bool activeLow = true, bool pullUp = true)
  {
    if (count_ >= MaxKeys)
    {
      return false;
    }
    entries_[count_] = Entry{pin, key, activeLow, pullUp};
    ++count_;
    return true;
  }

  size_t keyCount() const { return count_; }

  void update() override {}
  const KeySet &keys() const override { return keys_; }

private:
  struct Entry
  {
    uint8_t pin;
    Key key;
    bool activeLow;
    bool pullUp;
  };

  Entry entries_[MaxKeys] = {};
  size_t count_ = 0;
  KeySet keys_;
};

} // namespace esp32keybridge
