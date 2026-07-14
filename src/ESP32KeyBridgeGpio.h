#pragma once

#include <ESP32KeyBridge.h>

#include <Arduino.h>

#include <initializer_list>

#if __has_include(<soc/soc_caps.h>)
#include <soc/soc_caps.h>
#endif

#if defined(SOC_PCNT_SUPPORTED) && SOC_PCNT_SUPPORTED
#include <driver/gpio.h>
#include <driver/pulse_cnt.h>
#endif

// GPIO input adapters (no external library; Arduino GPIO and ESP-IDF
// peripherals only).

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
// Scanning, debouncing, and ghost blocking run in a dedicated 1 kHz task
// started on the first update(), so short taps are caught even when the
// sketch blocks the loop; update() only snapshots the debounced state.
// Configure the pins before the first update().
class GpioMatrixInputAdapter : public InputAdapter
{
public:
  static constexpr size_t MaxRows = 16;
  static constexpr size_t MaxCols = 16;

  // Scan output lines, top row first. Returns false when too many or
  // when the scan task already started.
  bool setRowPins(std::initializer_list<uint8_t> pins)
  {
    if (pins.size() > MaxRows || taskStarted_)
    {
      return false;
    }
    rowCount_ = 0;
    for (uint8_t pin : pins)
    {
      rowPins_[rowCount_++] = pin;
    }
    return true;
  }

  // Input lines (internal pull-up), leftmost column first. Returns false
  // when too many or when the scan task already started.
  bool setColPins(std::initializer_list<uint8_t> pins)
  {
    if (pins.size() > MaxCols || taskStarted_)
    {
      return false;
    }
    colCount_ = 0;
    for (uint8_t pin : pins)
    {
      colPins_[colCount_++] = pin;
    }
    return true;
  }

  // The keymap in row-major order (top-left key first) — write it as one
  // row per line and it reads like the physical layout. Call after the
  // pins are set; exactly rows x cols entries are required (returns false
  // otherwise). Put Key() (invalid = no key) on unused positions.
  bool setKeys(std::initializer_list<Key> layout)
  {
    if (rowCount_ == 0 || colCount_ == 0 || layout.size() != rowCount_ * colCount_)
    {
      return false;
    }
    size_t index = 0;
    for (const Key &key : layout)
    {
      grid_[index / colCount_][index % colCount_] = key;
      ++index;
    }
    keysValid_ = true;
    return true;
  }

  size_t keyCount() const { return keysValid_ ? rowCount_ * colCount_ : 0; }

  // A state change is accepted after it stays stable for this long.
  void setDebounceMillis(uint32_t ms) { debounceMillis_ = ms; }

  // Declares that every switch has a diode: disables ghost blocking, so
  // any number of keys can be held at once.
  void setHasDiodes(bool hasDiodes) { hasDiodes_ = hasDiodes; }

  void update() override
  {
    if (rowCount_ == 0 || colCount_ == 0 || !keysValid_)
    {
      return;
    }
    if (!taskStarted_)
    {
      taskStarted_ = true;
      // Idle rows stay high-Z so simultaneous presses never drive two
      // row lines against each other.
      for (size_t r = 0; r < rowCount_; ++r)
      {
        pinMode(rowPins_[r], INPUT);
      }
      for (size_t c = 0; c < colCount_; ++c)
      {
        pinMode(colPins_[c], INPUT_PULLUP);
      }
      xTaskCreate(&GpioMatrixInputAdapter::scanTask, "kbmatrix", 2048, this, 2, nullptr);
    }

    // Snapshot the debounced state fed by the scan task, then rebuild
    // outside the lock.
    uint16_t pressed[MaxRows];
    portENTER_CRITICAL(&mux_);
    for (size_t r = 0; r < rowCount_; ++r)
    {
      pressed[r] = pressedBits_[r];
    }
    portEXIT_CRITICAL(&mux_);

    keys_.clear();
    for (size_t r = 0; r < rowCount_; ++r)
    {
      for (size_t c = 0; c < colCount_; ++c)
      {
        if ((pressed[r] & (1u << c)) != 0 && grid_[r][c].kind != KeyKind::None)
        {
          keys_.press(grid_[r][c]);
        }
      }
    }
  }

  const KeySet &keys() const override { return keys_; }

private:
  // Runs at 1 kHz regardless of the loop rate: scans the matrix, applies
  // the debounce and ghost rules, and publishes pressedBits_.
  static void scanTask(void *context)
  {
    GpioMatrixInputAdapter &self = *static_cast<GpioMatrixInputAdapter *>(context);
    TickType_t lastWake = xTaskGetTickCount();
    for (;;)
    {
      vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1));
      self.scanOnce();
    }
  }

  void scanOnce()
  {
    const uint32_t now = millis();
    for (size_t r = 0; r < rowCount_; ++r)
    {
      // Set the output latch LOW before enabling the driver to avoid a
      // HIGH glitch, read the columns, then release the line to high-Z.
      digitalWrite(rowPins_[r], LOW);
      pinMode(rowPins_[r], OUTPUT);
      delayMicroseconds(3);
      uint16_t rowRaw = 0;
      for (size_t c = 0; c < colCount_; ++c)
      {
        if (digitalRead(colPins_[c]) == LOW)
        {
          rowRaw |= static_cast<uint16_t>(1u << c);
        }
      }
      pinMode(rowPins_[r], INPUT);

      for (size_t c = 0; c < colCount_; ++c)
      {
        const uint16_t bit = static_cast<uint16_t>(1u << c);
        const bool raw = (rowRaw & bit) != 0;
        if (raw != ((rawBits_[r] & bit) != 0))
        {
          rawBits_[r] = raw ? (rawBits_[r] | bit) : (rawBits_[r] & ~bit);
          lastChangeMs_[r][c] = now;
        }
        const bool pressed = (pressedBits_[r] & bit) != 0;
        if (pressed == raw || (now - lastChangeMs_[r][c]) < debounceMillis_)
        {
          continue;
        }
        if (raw && !hasDiodes_ && isGhost(r, c))
        {
          // Ambiguous 2x2 rectangle: hold the press back; it commits on a
          // later scan once the rectangle resolves.
          continue;
        }
        portENTER_CRITICAL(&mux_);
        pressedBits_[r] = raw ? (pressedBits_[r] | bit) : (pressedBits_[r] & ~bit);
        portEXIT_CRITICAL(&mux_);
      }
    }
  }

  // Without diodes, three pressed corners of a rectangle make the fourth
  // switch read as pressed too — the new press cannot be told apart from
  // its phantom, so it is blocked while the rectangle exists.
  bool isGhost(size_t row, size_t col) const
  {
    for (size_t r = 0; r < rowCount_; ++r)
    {
      if (r == row || (pressedBits_[r] & (1u << col)) == 0)
      {
        continue;
      }
      for (size_t c = 0; c < colCount_; ++c)
      {
        if (c == col || (pressedBits_[row] & (1u << c)) == 0)
        {
          continue;
        }
        if ((pressedBits_[r] & (1u << c)) != 0)
        {
          return true;
        }
      }
    }
    return false;
  }

  uint8_t rowPins_[MaxRows] = {};
  uint8_t colPins_[MaxCols] = {};
  size_t rowCount_ = 0;
  size_t colCount_ = 0;
  Key grid_[MaxRows][MaxCols] = {};
  bool keysValid_ = false;
  bool taskStarted_ = false;
  bool hasDiodes_ = false;
  uint32_t debounceMillis_ = 5;
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  uint16_t rawBits_[MaxRows] = {};      // scan task only
  uint16_t pressedBits_[MaxRows] = {};  // shared via mux_
  uint32_t lastChangeMs_[MaxRows][MaxCols] = {}; // scan task only
  KeySet keys_;
};

#if defined(SOC_PCNT_SUPPORTED) && SOC_PCNT_SUPPORTED

// Rotary encoder (EC11 style) counted by the PCNT peripheral — hardware
// quadrature counting, no interrupts; update() reads the counter
// difference. One adapter per encoder, up to the chip's PCNT unit count
// (the push switch of the encoder is a plain GpioKeyInputAdapter key).
class RotaryEncoderInputAdapter : public InputAdapter
{
public:
  RotaryEncoderInputAdapter(uint8_t pinA, uint8_t pinB) : pinA_(pinA), pinB_(pinB) {}

  // Each detent taps the mapped key once (press on one update, release on
  // the next). Alternative to mapToAxis(); the last call wins. Swap the
  // arguments if your encoder turns out reversed.
  void mapToKeys(Key clockwise, Key counterClockwise)
  {
    clockwiseKey_ = clockwise;
    counterClockwiseKey_ = counterClockwise;
    useAxis_ = false;
  }

  // Each detent adds ±1 to the axis instead (scroll dials, jog wheels).
  // Direction and speed are then shaped by config.setAxisScale().
  void mapToAxis(Axis axis)
  {
    axis_ = axis;
    useAxis_ = true;
  }

  // Quadrature pulses per detent; 4 (a full cycle, EC11 default) unless
  // the datasheet says otherwise.
  void setPulsesPerDetent(uint8_t pulses)
  {
    if (pulses > 0)
    {
      pulsesPerDetent_ = pulses;
    }
  }

  void update() override
  {
    if (!initialized_)
    {
      initialized_ = true;
      ready_ = initPcnt();
    }
    if (!ready_)
    {
      return;
    }

    int count = 0;
    pcnt_unit_get_count(unit_, &count);
    pulseAccumulator_ += count - lastCount_;
    lastCount_ = count;
    // Re-center long before the hardware limits so the counter never
    // saturates between updates.
    if (count > RecenterThreshold || count < -RecenterThreshold)
    {
      pcnt_unit_clear_count(unit_);
      lastCount_ = 0;
    }

    const int32_t detents = pulseAccumulator_ / pulsesPerDetent_;
    pulseAccumulator_ -= detents * pulsesPerDetent_;

    if (useAxis_)
    {
      axisDeltas_[static_cast<size_t>(axis_)] += detents;
      return;
    }

    pendingTaps_ += detents;
    if (tapActive_)
    {
      // Release last update's tap first so consecutive detents stay
      // distinct presses.
      keys_.release(tapKey_);
      tapActive_ = false;
    }
    else if (pendingTaps_ != 0)
    {
      tapKey_ = pendingTaps_ > 0 ? clockwiseKey_ : counterClockwiseKey_;
      pendingTaps_ += pendingTaps_ > 0 ? -1 : 1;
      if (tapKey_.kind != KeyKind::None)
      {
        keys_.press(tapKey_);
        tapActive_ = true;
      }
    }
  }

  const KeySet &keys() const override { return keys_; }

  int32_t takeAxisDelta(Axis axis) override
  {
    const size_t index = static_cast<size_t>(axis);
    const int32_t delta = axisDeltas_[index];
    axisDeltas_[index] = 0;
    return delta;
  }

private:
  static constexpr int CounterLimit = 32000;
  static constexpr int RecenterThreshold = 16000;

  bool initPcnt()
  {
    pcnt_unit_config_t unitConfig = {};
    unitConfig.low_limit = -CounterLimit;
    unitConfig.high_limit = CounterLimit;
    if (pcnt_new_unit(&unitConfig, &unit_) != ESP_OK)
    {
      return false;
    }

    pcnt_glitch_filter_config_t filterConfig = {};
    filterConfig.max_glitch_ns = 1000;
    pcnt_unit_set_glitch_filter(unit_, &filterConfig);

    // Two channels observing each other's phase give full x4 quadrature
    // decoding.
    pcnt_chan_config_t channelAConfig = {};
    channelAConfig.edge_gpio_num = pinA_;
    channelAConfig.level_gpio_num = pinB_;
    pcnt_channel_handle_t channelA = nullptr;
    if (pcnt_new_channel(unit_, &channelAConfig, &channelA) != ESP_OK)
    {
      return false;
    }
    pcnt_chan_config_t channelBConfig = {};
    channelBConfig.edge_gpio_num = pinB_;
    channelBConfig.level_gpio_num = pinA_;
    pcnt_channel_handle_t channelB = nullptr;
    if (pcnt_new_channel(unit_, &channelBConfig, &channelB) != ESP_OK)
    {
      return false;
    }
    pcnt_channel_set_edge_action(channelA, PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                                 PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(channelA, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                  PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    pcnt_channel_set_edge_action(channelB, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                 PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(channelB, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                  PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

    // Mechanical encoders switch to GND; enable the pull-ups after the
    // channels have claimed the pins.
    gpio_set_pull_mode(static_cast<gpio_num_t>(pinA_), GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(static_cast<gpio_num_t>(pinB_), GPIO_PULLUP_ONLY);

    pcnt_unit_enable(unit_);
    pcnt_unit_clear_count(unit_);
    pcnt_unit_start(unit_);
    return true;
  }

  uint8_t pinA_;
  uint8_t pinB_;
  Key clockwiseKey_;
  Key counterClockwiseKey_;
  Axis axis_ = Axis::Wheel;
  bool useAxis_ = false;
  uint8_t pulsesPerDetent_ = 4;
  bool initialized_ = false;
  bool ready_ = false;
  pcnt_unit_handle_t unit_ = nullptr;
  int lastCount_ = 0;
  int32_t pulseAccumulator_ = 0;
  int32_t pendingTaps_ = 0;
  Key tapKey_;
  bool tapActive_ = false;
  int32_t axisDeltas_[kAxisCount] = {};
  KeySet keys_;
};

#endif // SOC_PCNT_SUPPORTED

} // namespace esp32keybridge
