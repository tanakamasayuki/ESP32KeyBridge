#pragma once

#include <ESP32KeyBridge.h>

// USB Device output adapters backed by the EspUsbDevice library.
//
// Ownership model: the sketch owns the EspUsbDevice stack and starts it with
// the library's own configuration (VID/PID, product name, port selection on
// multi-port chips). Adapters take a reference; they register their HID
// interfaces at construction time, so construct them before calling
// usbDevice.begin() (the descriptor is fixed there). They have no begin().
//
// MOCK: build-only skeletons that fix the sketch-facing API. The real
// implementations land with implementation step 7.

class EspUsbDevice; // provided by the EspUsbDevice library

namespace esp32keybridge
{

// USB keyboard device output. Packs the output key set into keyboard
// reports and receives LED output reports from the host, which makes the
// host the lock state authority.
class EspUsbDeviceKeyboardOutputAdapter : public OutputAdapter
{
public:
  explicit EspUsbDeviceKeyboardOutputAdapter(EspUsbDevice &usb) : usb_(usb) {}

  void write(const KeySet &keys) override { (void)keys; }

  // True while the device is mounted by a host.
  bool connected() const override { return false; }

  // LED output reports from the host provide the lock state.
  bool reportsLockState() const override { return true; }
  bool getLockState(LockState &out) const override
  {
    (void)out;
    return false;
  }

private:
  EspUsbDevice &usb_;
};

// Composite HID device output: keyboard + consumer + mouse in one device.
// Registering this output is what makes mouse reports appear on the host.
class EspUsbDeviceHidOutputAdapter : public OutputAdapter
{
public:
  explicit EspUsbDeviceHidOutputAdapter(EspUsbDevice &usb) : usb_(usb) {}

  // Keyboard and consumer keys are packed into their reports; other kinds
  // are skipped.
  void write(const KeySet &keys) override { (void)keys; }

  bool connected() const override { return false; }

  bool reportsLockState() const override { return true; }
  bool getLockState(LockState &out) const override
  {
    (void)out;
    return false;
  }

  // Relative axis totals become mouse report deltas (saturation and carry
  // handled inside).
  void writeAxisDelta(Axis axis, int32_t delta) override
  {
    (void)axis;
    (void)delta;
  }

private:
  EspUsbDevice &usb_;
};

} // namespace esp32keybridge
