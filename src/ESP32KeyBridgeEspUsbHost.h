#pragma once

#include <ESP32KeyBridge.h>
#include <EspUsbHost.h>

#include <string.h>

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
// Sketches must not set the hooks the adapters use themselves
// (onKeyboardState, onConsumerControl, onMouse, onDeviceDisconnected).
// The per-event onKeyboard hook stays free for sketches.

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

  using KeyboardStateSink = void (*)(void *context, const EspUsbHostKeyboardState &state);
  using ConsumerSink = void (*)(void *context, const EspUsbHostConsumerControlEvent &event);
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

  void setKeyboardStateSink(KeyboardStateSink sink, void *context)
  {
    keyboardContext_ = context;
    keyboardSink_ = sink;
  }

  void setConsumerSink(ConsumerSink sink, void *context)
  {
    consumerContext_ = context;
    consumerSink_ = sink;
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
    usb.onKeyboardState(
        [this](const EspUsbHostKeyboardState &state)
        {
          if (keyboardSink_ != nullptr)
          {
            keyboardSink_(keyboardContext_, state);
          }
        });
    usb.onConsumerControl(
        [this](const EspUsbHostConsumerControlEvent &event)
        {
          if (consumerSink_ != nullptr)
          {
            consumerSink_(consumerContext_, event);
          }
        });
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
  KeyboardStateSink keyboardSink_ = nullptr;
  void *keyboardContext_ = nullptr;
  ConsumerSink consumerSink_ = nullptr;
  void *consumerContext_ = nullptr;
  MouseSink mouseSink_ = nullptr;
  void *mouseContext_ = nullptr;
  DisconnectSink disconnectSinks_[MaxDisconnectSinks] = {};
  void *disconnectContexts_[MaxDisconnectSinks] = {};
};

} // namespace detail

// USB keyboard input (consumer keys included). All keyboards on the stack
// are merged (key union across devices); a disconnect releases only that
// device's keys. The bridge lock state is forwarded to every keyboard's
// LEDs.
//
// Keyboard state comes from onKeyboardState (EspUsbHost 2.2.0+): one
// format-independent 256-bit snapshot per changed report — boot, report-ID
// boot, and NKRO alike, modifier-only changes included — which maps
// directly onto the key set. Consumer keys (media keys on keyboards,
// remotes) arrive as onConsumerControl press/release events.
class EspUsbHostKeyboardInputAdapter : public InputAdapter
{
public:
  // Keyboard interfaces tracked simultaneously (per USB address +
  // interface); reports from further ones are ignored.
  static constexpr size_t MaxKeyboards = 4;
  // Simultaneously held consumer usages across all devices.
  static constexpr size_t MaxConsumerKeys = 8;

  explicit EspUsbHostKeyboardInputAdapter(EspUsbHost &usb) : usb_(usb) {}

  void update() override
  {
    if (!subscribed_)
    {
      subscribed_ = true;
      detail::EspUsbHostHub &hub = detail::EspUsbHostHub::forStack(usb_);
      hub.setKeyboardStateSink(&EspUsbHostKeyboardInputAdapter::handleKeyboardState, this);
      hub.setConsumerSink(&EspUsbHostKeyboardInputAdapter::handleConsumer, this);
      hub.addDisconnectSink(&EspUsbHostKeyboardInputAdapter::handleDisconnect, this);
    }

    // Snapshot the callback-fed state, then rebuild outside the lock.
    uint8_t bitmaps[MaxKeyboards][ESP_USB_HOST_KEYBOARD_BITMAP_SIZE];
    size_t keyboardCount;
    ConsumerEntry consumer[MaxConsumerKeys];
    size_t consumerCount;
    portENTER_CRITICAL(&mux_);
    keyboardCount = keyboardCount_;
    for (size_t i = 0; i < keyboardCount_; ++i)
    {
      memcpy(bitmaps[i], keyboards_[i].bitmap, ESP_USB_HOST_KEYBOARD_BITMAP_SIZE);
    }
    consumerCount = consumerCount_;
    for (size_t i = 0; i < consumerCount_; ++i)
    {
      consumer[i] = consumer_[i];
    }
    portEXIT_CRITICAL(&mux_);

    keys_.clear();
    for (size_t i = 0; i < keyboardCount; ++i)
    {
      for (size_t byte = 0; byte < ESP_USB_HOST_KEYBOARD_BITMAP_SIZE; ++byte)
      {
        uint8_t bits = bitmaps[i][byte];
        while (bits != 0)
        {
          const int bit = __builtin_ctz(bits);
          bits = static_cast<uint8_t>(bits & (bits - 1));
          const uint16_t usage = static_cast<uint16_t>(byte * 8 + bit);
          // Usages 0x01-0x03 are boot rollover/POST error codes, not keys.
          if (usage >= 0x04)
          {
            keys_.press(keyboardKey(usage));
          }
        }
      }
    }
    for (size_t i = 0; i < consumerCount; ++i)
    {
      keys_.press(consumerKey(consumer[i].usage));
    }
    connected_ = keyboardCount > 0 || consumerCount > 0;
  }

  const KeySet &keys() const override { return keys_; }

  // A keyboard counts as present from its first changed report (EspUsbHost
  // has no enumeration query); false again when every known device
  // disconnected. A keyboard that never reported has no state to lose.
  bool connected() const override { return connected_; }

  // Forwards the bridge lock state to the LEDs of every keyboard on the
  // stack. EspUsbHost carries num/caps/scroll; kana is not forwardable.
  void setLockState(const LockState &state) override
  {
    usb_.setKeyboardLeds(state.numLock, state.capsLock, state.scrollLock);
  }

private:
  struct Keyboard
  {
    uint8_t address = 0;
    uint8_t interfaceNumber = 0;
    uint8_t bitmap[ESP_USB_HOST_KEYBOARD_BITMAP_SIZE] = {};
  };

  struct ConsumerEntry
  {
    uint8_t address = 0;
    uint16_t usage = 0;
  };

  // Fires from the EspUsbHost task.
  static void handleKeyboardState(void *context, const EspUsbHostKeyboardState &state)
  {
    EspUsbHostKeyboardInputAdapter &self = *static_cast<EspUsbHostKeyboardInputAdapter *>(context);
    portENTER_CRITICAL(&self.mux_);
    size_t index = self.keyboardCount_;
    for (size_t i = 0; i < self.keyboardCount_; ++i)
    {
      if (self.keyboards_[i].address == state.address &&
          self.keyboards_[i].interfaceNumber == state.interfaceNumber)
      {
        index = i;
        break;
      }
    }
    if (index == self.keyboardCount_ && self.keyboardCount_ < MaxKeyboards)
    {
      self.keyboards_[self.keyboardCount_].address = state.address;
      self.keyboards_[self.keyboardCount_].interfaceNumber = state.interfaceNumber;
      ++self.keyboardCount_;
    }
    if (index < self.keyboardCount_)
    {
      memcpy(self.keyboards_[index].bitmap, state.keys, ESP_USB_HOST_KEYBOARD_BITMAP_SIZE);
    }
    portEXIT_CRITICAL(&self.mux_);
  }

  // Fires from the EspUsbHost task.
  static void handleConsumer(void *context, const EspUsbHostConsumerControlEvent &event)
  {
    EspUsbHostKeyboardInputAdapter &self = *static_cast<EspUsbHostKeyboardInputAdapter *>(context);
    portENTER_CRITICAL(&self.mux_);
    if (event.released)
    {
      for (size_t i = 0; i < self.consumerCount_; ++i)
      {
        if (self.consumer_[i].address == event.address && self.consumer_[i].usage == event.usage)
        {
          self.consumer_[i] = self.consumer_[self.consumerCount_ - 1];
          --self.consumerCount_;
          break;
        }
      }
    }
    if (event.pressed && event.usage != 0 && self.consumerCount_ < MaxConsumerKeys)
    {
      bool held = false;
      for (size_t i = 0; i < self.consumerCount_; ++i)
      {
        if (self.consumer_[i].address == event.address && self.consumer_[i].usage == event.usage)
        {
          held = true;
          break;
        }
      }
      if (!held)
      {
        self.consumer_[self.consumerCount_].address = event.address;
        self.consumer_[self.consumerCount_].usage = event.usage;
        ++self.consumerCount_;
      }
    }
    portEXIT_CRITICAL(&self.mux_);
  }

  // Fires from the EspUsbHost task (any device, not only keyboards).
  static void handleDisconnect(void *context, uint8_t address)
  {
    EspUsbHostKeyboardInputAdapter &self = *static_cast<EspUsbHostKeyboardInputAdapter *>(context);
    portENTER_CRITICAL(&self.mux_);
    for (size_t i = self.keyboardCount_; i > 0; --i)
    {
      if (self.keyboards_[i - 1].address == address)
      {
        self.keyboards_[i - 1] = self.keyboards_[self.keyboardCount_ - 1];
        --self.keyboardCount_;
      }
    }
    for (size_t i = self.consumerCount_; i > 0; --i)
    {
      if (self.consumer_[i - 1].address == address)
      {
        self.consumer_[i - 1] = self.consumer_[self.consumerCount_ - 1];
        --self.consumerCount_;
      }
    }
    portEXIT_CRITICAL(&self.mux_);
  }

  EspUsbHost &usb_;
  bool subscribed_ = false;
  KeySet keys_;
  bool connected_ = false;
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  Keyboard keyboards_[MaxKeyboards];
  size_t keyboardCount_ = 0;
  ConsumerEntry consumer_[MaxConsumerKeys];
  size_t consumerCount_ = 0;
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
