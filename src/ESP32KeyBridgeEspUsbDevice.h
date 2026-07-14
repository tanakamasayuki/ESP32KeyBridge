#pragma once

#include <ESP32KeyBridge.h>
#include <EspUsbDevice.h>

#include <string.h>

// USB Device output adapters backed by the EspUsbDevice library.
//
// Ownership model: the sketch owns the EspUsbDevice stack and starts it with
// the library's own configuration (VID/PID, product name). Adapters take a
// reference; they register their HID interfaces at construction time, so
// construct them before calling usbDevice.begin() (the descriptor is fixed
// there). They have no begin().

namespace esp32keybridge
{

// USB keyboard device output. Packs the output key set into boot keyboard
// reports (6KRO) and receives LED output reports from the host, which makes
// the host the lock state authority.
class EspUsbDeviceKeyboardOutputAdapter : public OutputAdapter
{
public:
  explicit EspUsbDeviceKeyboardOutputAdapter(EspUsbDevice &usb)
      : usb_(usb), keyboard_(usb)
  {
    // Fires from the TinyUSB task: publish the raw LED byte for the
    // update() context to decode in getLockState().
    keyboard_.onOutputReport(
        [this](const EspUsbDeviceHidKeyboardOutputReport &report)
        {
          hostLeds_ = report.leds;
          ledSeen_ = true;
        });
  }

  // Sends a boot report when the key state changed (or when the previous
  // send could not go out because the endpoint was busy).
  void write(const KeySet &keys) override
  {
    const HidKeyboardReport report = buildHidKeyboardReport(keys);
    EspUsbDeviceBootKeyboardReport bootReport;
    bootReport.modifiers = report.modifiers;
    for (size_t i = 0; i < HidKeyboardReport::MaxKeys; ++i)
    {
      bootReport.keys[i] = i < report.keyCount ? report.keys[i] : 0;
    }

    if (!sendPending_ && memcmp(&bootReport, &lastReport_, sizeof(bootReport)) == 0)
    {
      return;
    }
    lastReport_ = bootReport;
    // sendReport is non-blocking; when it fails (endpoint busy), retry on
    // the next update instead of dropping the state change.
    sendPending_ = !keyboard_.sendReport(bootReport, 0);
  }

  // True while a host has mounted the device.
  bool connected() const override { return usb_.ready(); }

  // LED output reports from the host provide the lock state. False until
  // the first report arrives; the bridge keeps its shadow meanwhile.
  bool reportsLockState() const override { return true; }
  bool getLockState(LockState &out) const override
  {
    if (!ledSeen_)
    {
      return false;
    }
    const uint8_t leds = hostLeds_;
    out.numLock = (leds & ESP_USB_DEVICE_KEYBOARD_LED_NUM_LOCK) != 0;
    out.capsLock = (leds & ESP_USB_DEVICE_KEYBOARD_LED_CAPS_LOCK) != 0;
    out.scrollLock = (leds & ESP_USB_DEVICE_KEYBOARD_LED_SCROLL_LOCK) != 0;
    out.kana = (leds & ESP_USB_DEVICE_KEYBOARD_LED_KANA) != 0;
    return true;
  }

private:
  EspUsbDevice &usb_;
  EspUsbDeviceHidKeyboard keyboard_;
  EspUsbDeviceBootKeyboardReport lastReport_;
  bool sendPending_ = false;
  volatile uint8_t hostLeds_ = 0;
  volatile bool ledSeen_ = false;
};

// Composite HID device output: keyboard + consumer + mouse in one device.
// USB interfaces are fixed when usbDevice.begin() runs and stay visible to
// the host even while unused (adding or removing one would require
// re-enumeration); choosing this adapter over the keyboard-only one is
// what decides that consumer/mouse reports exist on the host.
//
// MOCK: build-only skeleton fixing the sketch-facing API. The real
// implementation lands with implementation step 7.
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
