#pragma once

#include <ESP32KeyBridge.h>

// USB Host input adapters backed by the EspUsbHost library.
//
// Ownership model: the sketch owns the EspUsbHost stack and starts it with
// the library's own configuration (port selection, etc.). Adapters take a
// reference and subscribe to events lazily on their first update(), so they
// have no begin() and no ordering against the bridge or the stack.
//
// MOCK: build-only skeletons that fix the sketch-facing API. The real
// implementations land with implementation step 7.

class EspUsbHost; // provided by the EspUsbHost library

namespace esp32keybridge
{

// USB keyboard input (consumer keys of the same device included). Keyboard
// reports become presses/releases in keys(); the bridge lock state is
// forwarded to the keyboard LEDs.
class EspUsbHostKeyboardInputAdapter : public InputAdapter
{
public:
  explicit EspUsbHostKeyboardInputAdapter(EspUsbHost &usb) : usb_(usb) {}

  // Drains events received since the last update into the key set.
  void update() override {}

  const KeySet &keys() const override { return keys_; }

  // False until a keyboard is enumerated. Disconnect drops all keys.
  bool connected() const override { return false; }

  // Forwards the bridge lock state to the keyboard LEDs.
  void setLockState(const LockState &state) override { (void)state; }

private:
  EspUsbHost &usb_;
  KeySet keys_;
};

// USB mouse input. Buttons become MouseButton keys in keys(); movement and
// wheel accumulate as relative axis deltas.
class EspUsbHostMouseInputAdapter : public InputAdapter
{
public:
  explicit EspUsbHostMouseInputAdapter(EspUsbHost &usb) : usb_(usb) {}

  void update() override {}
  const KeySet &keys() const override { return keys_; }
  bool connected() const override { return false; }

  int32_t takeAxisDelta(Axis axis) override
  {
    (void)axis;
    return 0;
  }

private:
  EspUsbHost &usb_;
  KeySet keys_;
};

} // namespace esp32keybridge
