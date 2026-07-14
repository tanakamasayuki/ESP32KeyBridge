#pragma once

#include <ESP32KeyBridge.h>
#include <EspUsbHost.h>

// USB Host input adapters backed by the EspUsbHost library.
//
// Ownership model: the sketch owns the EspUsbHost stack and starts it with
// the library's own configuration (port selection, etc.). Adapters take a
// reference and subscribe to events lazily on their first update(), so they
// have no begin() and no ordering against the bridge or the stack.
//
// EspUsbHost callbacks are single-slot (each on*() call replaces the
// previous callback) and fire from the library's internal task. The
// adapters therefore share one subscription per hook through an internal
// hub and hand the data to the update() context behind a critical section.
// Sketches must not set the hooks the adapters use themselves (onMouse,
// onDeviceDisconnected; onKeyboard and onConsumerControl once the keyboard
// adapter is real).

namespace esp32keybridge
{

namespace detail
{

// One hub per EspUsbHost stack: owns the single-slot callbacks and fans
// events out to the adapters registered on that stack.
class EspUsbHostHub
{
public:
  static constexpr size_t MaxStacks = 2; // ESP32-P4 can host on HS and FS
  static constexpr size_t MaxDisconnectSinks = 4;

  using MouseSink = void (*)(void *context, const EspUsbHostMouseEvent &event);
  using DisconnectSink = void (*)(void *context, uint8_t address);

  static EspUsbHostHub &forStack(EspUsbHost &usb)
  {
    static EspUsbHostHub hubs[MaxStacks];
    for (size_t i = 0; i < MaxStacks; ++i)
    {
      if (hubs[i].stack_ == &usb)
      {
        return hubs[i];
      }
    }
    for (size_t i = 0; i < MaxStacks; ++i)
    {
      if (hubs[i].stack_ == nullptr)
      {
        hubs[i].bind(usb);
        return hubs[i];
      }
    }
    return hubs[0]; // more stacks than MaxStacks; unreachable on real chips
  }

  void setMouseSink(MouseSink sink, void *context)
  {
    mouseContext_ = context;
    mouseSink_ = sink;
  }

  void addDisconnectSink(DisconnectSink sink, void *context)
  {
    for (size_t i = 0; i < MaxDisconnectSinks; ++i)
    {
      if (disconnectSinks_[i] == nullptr)
      {
        disconnectContexts_[i] = context;
        disconnectSinks_[i] = sink;
        return;
      }
    }
  }

private:
  void bind(EspUsbHost &usb)
  {
    stack_ = &usb;
    usb.onMouse(
        [this](const EspUsbHostMouseEvent &event)
        {
          if (mouseSink_ != nullptr)
          {
            mouseSink_(mouseContext_, event);
          }
        });
    usb.onDeviceDisconnected(
        [this](const EspUsbHostDeviceInfo &info)
        {
          for (size_t i = 0; i < MaxDisconnectSinks; ++i)
          {
            if (disconnectSinks_[i] != nullptr)
            {
              disconnectSinks_[i](disconnectContexts_[i], info.address);
            }
          }
        });
  }

  EspUsbHost *stack_ = nullptr;
  MouseSink mouseSink_ = nullptr;
  void *mouseContext_ = nullptr;
  DisconnectSink disconnectSinks_[MaxDisconnectSinks] = {};
  void *disconnectContexts_[MaxDisconnectSinks] = {};
};

} // namespace detail

// USB keyboard input (consumer keys of the same device included). Keyboard
// reports become presses/releases in keys(); the bridge lock state is
// forwarded to the keyboard LEDs.
//
// MOCK: build-only skeleton fixing the sketch-facing API. Waiting on an
// EspUsbHost change (modifier-only presses do not produce onKeyboard
// events yet).
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
// wheel accumulate as relative axis deltas. All mice on the stack are
// merged (buttons union, deltas summed); a disconnect releases that
// mouse's buttons only.
class EspUsbHostMouseInputAdapter : public InputAdapter
{
public:
  // Mice tracked simultaneously (per USB address); reports from further
  // addresses are ignored.
  static constexpr size_t MaxMice = 4;

  explicit EspUsbHostMouseInputAdapter(EspUsbHost &usb) : usb_(usb) {}

  void update() override
  {
    if (!subscribed_)
    {
      subscribed_ = true;
      detail::EspUsbHostHub &hub = detail::EspUsbHostHub::forStack(usb_);
      hub.setMouseSink(&EspUsbHostMouseInputAdapter::handleMouse, this);
      hub.addDisconnectSink(&EspUsbHostMouseInputAdapter::handleDisconnect, this);
    }

    // Snapshot the callback-fed state, then rebuild outside the lock.
    uint8_t buttons[MaxMice];
    size_t mouseCount;
    int32_t deltas[kAxisCount];
    portENTER_CRITICAL(&mux_);
    mouseCount = mouseCount_;
    for (size_t i = 0; i < mouseCount_; ++i)
    {
      buttons[i] = mice_[i].buttons;
    }
    for (size_t axis = 0; axis < kAxisCount; ++axis)
    {
      deltas[axis] = pendingAxis_[axis];
      pendingAxis_[axis] = 0;
    }
    portEXIT_CRITICAL(&mux_);

    keys_.clear();
    for (size_t i = 0; i < mouseCount; ++i)
    {
      for (size_t bit = 0; bit < 8; ++bit)
      {
        if ((buttons[i] & (1u << bit)) != 0)
        {
          keys_.press(mouseButtonKey(static_cast<uint16_t>(bit + 1)));
        }
      }
    }
    for (size_t axis = 0; axis < kAxisCount; ++axis)
    {
      axisDeltas_[axis] += deltas[axis];
    }
    connected_ = mouseCount > 0;
  }

  const KeySet &keys() const override { return keys_; }

  // A mouse counts as present from its first report (EspUsbHost has no
  // enumeration query); false again when every known mouse disconnected.
  // A mouse that never reported has no state to lose.
  bool connected() const override { return connected_; }

  int32_t takeAxisDelta(Axis axis) override
  {
    const size_t index = static_cast<size_t>(axis);
    const int32_t delta = axisDeltas_[index];
    axisDeltas_[index] = 0;
    return delta;
  }

private:
  struct Mouse
  {
    uint8_t address = 0;
    uint8_t buttons = 0;
  };

  // Fires from the EspUsbHost task.
  static void handleMouse(void *context, const EspUsbHostMouseEvent &event)
  {
    EspUsbHostMouseInputAdapter &self = *static_cast<EspUsbHostMouseInputAdapter *>(context);
    portENTER_CRITICAL(&self.mux_);
    size_t index = self.mouseCount_;
    for (size_t i = 0; i < self.mouseCount_; ++i)
    {
      if (self.mice_[i].address == event.address)
      {
        index = i;
        break;
      }
    }
    if (index == self.mouseCount_ && self.mouseCount_ < MaxMice)
    {
      self.mice_[self.mouseCount_].address = event.address;
      self.mice_[self.mouseCount_].buttons = 0;
      ++self.mouseCount_;
    }
    if (index < self.mouseCount_)
    {
      self.mice_[index].buttons = event.buttons;
      self.pendingAxis_[static_cast<size_t>(Axis::X)] += event.x;
      self.pendingAxis_[static_cast<size_t>(Axis::Y)] += event.y;
      self.pendingAxis_[static_cast<size_t>(Axis::Wheel)] += event.wheel;
    }
    portEXIT_CRITICAL(&self.mux_);
  }

  // Fires from the EspUsbHost task (any device, not only mice).
  static void handleDisconnect(void *context, uint8_t address)
  {
    EspUsbHostMouseInputAdapter &self = *static_cast<EspUsbHostMouseInputAdapter *>(context);
    portENTER_CRITICAL(&self.mux_);
    for (size_t i = 0; i < self.mouseCount_; ++i)
    {
      if (self.mice_[i].address == address)
      {
        self.mice_[i] = self.mice_[self.mouseCount_ - 1];
        --self.mouseCount_;
        break;
      }
    }
    portEXIT_CRITICAL(&self.mux_);
  }

  EspUsbHost &usb_;
  bool subscribed_ = false;
  KeySet keys_;
  bool connected_ = false;
  int32_t axisDeltas_[kAxisCount] = {};
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  Mouse mice_[MaxMice];
  size_t mouseCount_ = 0;
  int32_t pendingAxis_[kAxisCount] = {};
};

} // namespace esp32keybridge
