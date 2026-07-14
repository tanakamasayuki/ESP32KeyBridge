#pragma once

#include <ESP32KeyBridge.h>

#include <Arduino.h>

#include <initializer_list>

#if __has_include(<soc/soc_caps.h>)
#include <soc/soc_caps.h>
#endif

// GPIO input adapters (no external library; Arduino GPIO only).

namespace esp32keybridge
{

// GPIO pins pressed as keys (foot switches, panel buttons, a few
// direct-wired keys). Register each pin with addKey(); the pins are
// configured on the first update() after registration and debounced inside
// update(), so polling at the loop rate (~1 kHz with delay(1)) is enough.
// No begin(). Matrix scanning is a separate adapter.
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
    entries_[count_] = Entry{pin, key, activeLow, pullUp, false, false, 0};
    ++count_;
    return true;
  }

  size_t keyCount() const { return count_; }

  // A state change is accepted after it stays stable for this long.
  // Raise it for bouncy switches (big mechanical pedals).
  void setDebounceMillis(uint32_t ms) { debounceMillis_ = ms; }

  void update() override
  {
    // Configure pins registered since the last update.
    while (configured_ < count_)
    {
      Entry &entry = entries_[configured_];
      pinMode(entry.pin, entry.pullUp ? INPUT_PULLUP : INPUT);
      entry.raw = readPressed(entry);
      entry.lastChangeMs = millis();
      ++configured_;
    }

    const uint32_t now = millis();
    for (size_t i = 0; i < configured_; ++i)
    {
      Entry &entry = entries_[i];
      const bool raw = readPressed(entry);
      if (raw != entry.raw)
      {
        entry.raw = raw;
        entry.lastChangeMs = now;
      }
      if (entry.pressed != entry.raw && (now - entry.lastChangeMs) >= debounceMillis_)
      {
        entry.pressed = entry.raw;
        if (entry.pressed)
        {
          keys_.press(entry.key);
        }
        else
        {
          keys_.release(entry.key);
        }
      }
    }
  }

  const KeySet &keys() const override { return keys_; }

private:
  struct Entry
  {
    uint8_t pin;
    Key key;
    bool activeLow;
    bool pullUp;
    bool raw;     // last read level, as "pressed?"
    bool pressed; // debounced logical state
    uint32_t lastChangeMs;
  };

  bool readPressed(const Entry &entry) const
  {
    return (digitalRead(entry.pin) == LOW) == entry.activeLow;
  }

  Entry entries_[MaxKeys] = {};
  size_t count_ = 0;
  size_t configured_ = 0;
  uint32_t debounceMillis_ = 5;
  KeySet keys_;
};

// Key matrix scanned over GPIO (macro pads, numeric keypads, homemade
// keyboards). Wiring and keymap are separate: setRowPins()/setColPins()
// describe the lines (rows are driven one at a time, columns are read
// back through pull-ups), setKeys() assigns the keys in row-major order —
// the code layout mirrors the physical layout, so no coordinates or
// indices appear. Diodes are optional (without them, ghost blocking
// suppresses the phantom key when a 2x2 rectangle is pressed).
//
// MOCK: build-only skeleton fixing the sketch-facing API. The real
// implementation lands with the matrix step.
class GpioMatrixInputAdapter : public InputAdapter
{
public:
  static constexpr size_t MaxRows = 16;
  static constexpr size_t MaxCols = 16;

  // Scan output lines, top row first. Returns false when too many.
  bool setRowPins(std::initializer_list<uint8_t> pins)
  {
    (void)pins;
    return true;
  }

  // Input lines (internal pull-up), leftmost column first. Returns false
  // when too many.
  bool setColPins(std::initializer_list<uint8_t> pins)
  {
    (void)pins;
    return true;
  }

  // The keymap in row-major order (top-left key first) — write it as one
  // row per line and it reads like the physical layout. Call after the
  // pins are set; exactly rows x cols entries are required (returns false
  // otherwise). Put Key() (invalid = no key) on unused positions.
  bool setKeys(std::initializer_list<Key> layout)
  {
    (void)layout;
    return true;
  }

  size_t keyCount() const { return 0; }

  // A state change is accepted after it stays stable for this long.
  void setDebounceMillis(uint32_t ms) { (void)ms; }

  // Declares that every switch has a diode: disables ghost blocking, so
  // any number of keys can be held at once.
  void setHasDiodes(bool hasDiodes) { (void)hasDiodes; }

  void update() override {}
  const KeySet &keys() const override { return keys_; }

private:
  KeySet keys_;
};

// Rotary encoder (EC11 style) counted by the PCNT peripheral — hardware
// counting, no interrupts; update() reads the counter difference. One
// adapter per encoder (the push switch of the encoder is a plain
// GpioKeyInputAdapter key).
//
// MOCK: build-only skeleton fixing the sketch-facing API. The real
// implementation lands with the encoder step.
class RotaryEncoderInputAdapter : public InputAdapter
{
public:
  RotaryEncoderInputAdapter(uint8_t pinA, uint8_t pinB)
  {
    (void)pinA;
    (void)pinB;
  }

  // Each detent taps the mapped key once (press on one update, release on
  // the next). Alternative to mapToAxis().
  void mapToKeys(Key clockwise, Key counterClockwise)
  {
    (void)clockwise;
    (void)counterClockwise;
  }

  // Each detent adds ±1 to the axis instead (scroll dials, jog wheels).
  // Direction and speed are then shaped by config.setAxisScale().
  void mapToAxis(Axis axis) { (void)axis; }

  // Quadrature pulses per detent; 4 (a full cycle, EC11 default) unless
  // the datasheet says otherwise.
  void setPulsesPerDetent(uint8_t pulses) { (void)pulses; }

  void update() override {}
  const KeySet &keys() const override { return keys_; }
  int32_t takeAxisDelta(Axis axis) override
  {
    (void)axis;
    return 0;
  }

private:
  KeySet keys_;
};

} // namespace esp32keybridge
