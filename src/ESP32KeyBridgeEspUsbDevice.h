#pragma once

#include <ESP32KeyBridge.h>
#include <EspUsbDevice.h>

#include <string.h>

// USB Device output adapter backed by the EspUsbDevice library.
//
// Ownership model: the sketch owns the EspUsbDevice stack and starts it with
// the library's own configuration (VID/PID, product name). The adapter takes
// a reference; it registers its HID interfaces at construction time, so
// construct it before calling usbDevice.begin() (the descriptor is fixed
// there). It has no begin().

namespace esp32keybridge
{

// Composite HID device output: keyboard + consumer + mouse in one device.
// The full composition is always registered — an interface that never sends
// a report is harmless on the USB side, so there is no keyboard-only
// variant to choose from. The keyboard interface still speaks the boot
// protocol, which covers BIOS/UEFI use.
class EspUsbDeviceHidOutputAdapter : public OutputAdapter
{
public:
  explicit EspUsbDeviceHidOutputAdapter(EspUsbDevice &usb)
      : usb_(usb), keyboard_(usb), consumer_(usb), mouse_(usb)
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

  // Packs the key set into keyboard, consumer, and mouse-button reports.
  // Each report is sent when its state changed (or when the previous send
  // could not go out because the endpoint was busy).
  void write(const KeySet &keys) override
  {
    writeKeyboard(keys);
    writeConsumer(keys);
    writeMouse(keys);
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

  // Relative axis totals accumulate here; the bridge delivers them before
  // write() in the same update, so they ride the mouse report of the same
  // cycle (int8 saturation with the remainder carried to the next update).
  // The boot mouse report has no pan field, so Pan is dropped.
  void writeAxisDelta(Axis axis, int32_t delta) override
  {
    if (axis == Axis::Pan)
    {
      return;
    }
    pendingAxis_[static_cast<size_t>(axis)] += delta;
  }

private:
  void writeKeyboard(const KeySet &keys)
  {
    const HidKeyboardReport report = buildHidKeyboardReport(keys);
    EspUsbDeviceBootKeyboardReport bootReport;
    bootReport.modifiers = report.modifiers;
    for (size_t i = 0; i < HidKeyboardReport::MaxKeys; ++i)
    {
      bootReport.keys[i] = i < report.keyCount ? report.keys[i] : 0;
    }

    if (!keyboardPending_ && memcmp(&bootReport, &lastKeyboardReport_, sizeof(bootReport)) == 0)
    {
      return;
    }
    lastKeyboardReport_ = bootReport;
    // sendReport is non-blocking; when it fails (endpoint busy), retry on
    // the next update instead of dropping the state change.
    keyboardPending_ = !keyboard_.sendReport(bootReport, 0);
  }

  void writeConsumer(const KeySet &keys)
  {
    const HidConsumerReport report = buildHidConsumerReport(keys);
    if (!consumerPending_ && report.usage == lastConsumerUsage_)
    {
      return;
    }
    lastConsumerUsage_ = report.usage;
    consumerPending_ = !consumer_.sendUsage(report.usage, 0);
  }

  void writeMouse(const KeySet &keys)
  {
    HidMouseReport report = buildHidMouseReport(keys);
    for (size_t axis = 0; axis < kAxisCount; ++axis)
    {
      pendingAxis_[axis] = report.applyAxisDelta(static_cast<Axis>(axis), pendingAxis_[axis]);
    }
    const bool moved = report.x != 0 || report.y != 0 || report.wheel != 0;
    // lastMouseButtons_ tracks the last successful send, so a failed
    // button change keeps mismatching and retries on the next update.
    if (!moved && report.buttons == lastMouseButtons_)
    {
      return;
    }

    EspUsbDeviceBootMouseReport bootReport;
    bootReport.buttons = report.buttons;
    bootReport.x = report.x;
    bootReport.y = report.y;
    bootReport.wheel = report.wheel;
    if (mouse_.sendReport(bootReport, 0))
    {
      lastMouseButtons_ = report.buttons;
    }
    else
    {
      // Endpoint busy: put the movement back so nothing is lost.
      pendingAxis_[static_cast<size_t>(Axis::X)] += report.x;
      pendingAxis_[static_cast<size_t>(Axis::Y)] += report.y;
      pendingAxis_[static_cast<size_t>(Axis::Wheel)] += report.wheel;
    }
  }

  EspUsbDevice &usb_;
  EspUsbDeviceHidKeyboard keyboard_;
  EspUsbDeviceHidConsumerControl consumer_;
  EspUsbDeviceHidMouse mouse_;
  EspUsbDeviceBootKeyboardReport lastKeyboardReport_;
  bool keyboardPending_ = false;
  uint16_t lastConsumerUsage_ = 0;
  bool consumerPending_ = false;
  uint8_t lastMouseButtons_ = 0;
  int32_t pendingAxis_[kAxisCount] = {};
  volatile uint8_t hostLeds_ = 0;
  volatile bool ledSeen_ = false;
};

} // namespace esp32keybridge
