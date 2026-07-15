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
// (onKeyboardState, onConsumerControl, onMouse, onGamepad, onMidiMessage,
// onDeviceDisconnected). The per-event onKeyboard hook stays free for
// sketches.

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
  using GamepadSink = void (*)(void *context, const EspUsbHostGamepadEvent &event);
  using MidiSink = void (*)(void *context, const EspUsbHostMidiMessage &message);
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

  void setGamepadSink(GamepadSink sink, void *context)
  {
    gamepadContext_ = context;
    gamepadSink_ = sink;
  }

  void setMidiSink(MidiSink sink, void *context)
  {
    midiContext_ = context;
    midiSink_ = sink;
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
    usb.onGamepad(
        [this](const EspUsbHostGamepadEvent &event)
        {
          if (gamepadSink_ != nullptr)
          {
            gamepadSink_(gamepadContext_, event);
          }
        });
    usb.onMidiMessage(
        [this](const EspUsbHostMidiMessage &message)
        {
          if (midiSink_ != nullptr)
          {
            midiSink_(midiContext_, message);
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
  GamepadSink gamepadSink_ = nullptr;
  void *gamepadContext_ = nullptr;
  MidiSink midiSink_ = nullptr;
  void *midiContext_ = nullptr;
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

// USB gamepad input. Buttons and the hat/D-pad map to keys; only mapped
// controls produce output, so the sketch decides what each control means
// (any Key kind - keyboard, consumer, mouse button, or a virtual key to
// remap/layer later). All gamepads on the stack are merged; a disconnect
// drops only that pad's controls. Analog sticks and triggers are not mapped
// in this version.
//
// Reports arrive as onGamepad (EspUsbHost 2.3.0): decoded HID fields. Button
// page (0x09) fields are buttons (usage = HID button number, 1-based); the
// Generic Desktop hat switch (usage 0x39) is the D-pad. Button numbers vary
// by controller - watch a LogOutputAdapter to find which is which.
class EspUsbHostGamepadInputAdapter : public InputAdapter
{
public:
  static constexpr size_t MaxGamepads = 4;
  static constexpr size_t MaxButtons = 32;

  explicit EspUsbHostGamepadInputAdapter(EspUsbHost &usb) : usb_(usb) {}

  // Map HID button `number` (1-based, as the controller reports it) to any
  // Key. Returns false if the number is out of range; unmapped buttons
  // produce nothing.
  bool mapButton(uint8_t number, Key key)
  {
    if (number < 1 || number > MaxButtons)
    {
      return false;
    }
    buttonKeys_[number - 1] = key;
    return true;
  }

  // Map the four hat/D-pad directions; diagonals press the two neighbours.
  void mapHat(Key up, Key down, Key left, Key right)
  {
    hatUp_ = up;
    hatDown_ = down;
    hatLeft_ = left;
    hatRight_ = right;
  }

  void update() override
  {
    if (!subscribed_)
    {
      subscribed_ = true;
      detail::EspUsbHostHub &hub = detail::EspUsbHostHub::forStack(usb_);
      hub.setGamepadSink(&EspUsbHostGamepadInputAdapter::handleGamepad, this);
      hub.addDisconnectSink(&EspUsbHostGamepadInputAdapter::handleDisconnect, this);
    }

    uint32_t buttons[MaxGamepads];
    int8_t hats[MaxGamepads];
    size_t padCount;
    portENTER_CRITICAL(&mux_);
    padCount = padCount_;
    for (size_t i = 0; i < padCount_; ++i)
    {
      buttons[i] = pads_[i].buttons;
      hats[i] = pads_[i].hat;
    }
    portEXIT_CRITICAL(&mux_);

    keys_.clear();
    bool up = false, down = false, left = false, right = false;
    for (size_t i = 0; i < padCount; ++i)
    {
      for (size_t b = 0; b < MaxButtons; ++b)
      {
        if ((buttons[i] & (1u << b)) != 0 && isValid(buttonKeys_[b]))
        {
          keys_.press(buttonKeys_[b]);
        }
      }
      switch (hats[i])
      {
      case 0: up = true; break;
      case 1: up = true; right = true; break;
      case 2: right = true; break;
      case 3: down = true; right = true; break;
      case 4: down = true; break;
      case 5: down = true; left = true; break;
      case 6: left = true; break;
      case 7: up = true; left = true; break;
      default: break;
      }
    }
    if (up && isValid(hatUp_)) keys_.press(hatUp_);
    if (down && isValid(hatDown_)) keys_.press(hatDown_);
    if (left && isValid(hatLeft_)) keys_.press(hatLeft_);
    if (right && isValid(hatRight_)) keys_.press(hatRight_);
    connected_ = padCount > 0;
  }

  const KeySet &keys() const override { return keys_; }
  bool connected() const override { return connected_; }

private:
  struct Pad
  {
    uint8_t address = 0;
    uint8_t interfaceNumber = 0;
    uint32_t buttons = 0;
    int8_t hat = -1;
  };

  // Fires from the EspUsbHost task.
  static void handleGamepad(void *context, const EspUsbHostGamepadEvent &event)
  {
    EspUsbHostGamepadInputAdapter &self = *static_cast<EspUsbHostGamepadInputAdapter *>(context);
    uint32_t buttons = 0;
    int8_t hat = -1;
    for (size_t i = 0; i < event.fieldCount; ++i)
    {
      const EspUsbHostHIDFieldValue &field = event.fields[i];
      if (field.usagePage == 0x09) // Button page
      {
        if (field.usage >= 1 && field.usage <= MaxButtons && field.value != 0)
        {
          buttons |= (1u << (field.usage - 1));
        }
      }
      else if (field.usagePage == 0x01 && field.usage == 0x39) // Hat switch
      {
        const int32_t dir = field.value - field.logicalMin;
        if (dir >= 0 && dir <= 7)
        {
          hat = static_cast<int8_t>(dir);
        }
      }
    }

    portENTER_CRITICAL(&self.mux_);
    size_t index = self.padCount_;
    for (size_t i = 0; i < self.padCount_; ++i)
    {
      if (self.pads_[i].address == event.address &&
          self.pads_[i].interfaceNumber == event.interfaceNumber)
      {
        index = i;
        break;
      }
    }
    if (index == self.padCount_ && self.padCount_ < MaxGamepads)
    {
      self.pads_[self.padCount_].address = event.address;
      self.pads_[self.padCount_].interfaceNumber = event.interfaceNumber;
      ++self.padCount_;
    }
    if (index < self.padCount_)
    {
      self.pads_[index].buttons = buttons;
      self.pads_[index].hat = hat;
    }
    portEXIT_CRITICAL(&self.mux_);
  }

  // Fires from the EspUsbHost task (any device, not only gamepads).
  static void handleDisconnect(void *context, uint8_t address)
  {
    EspUsbHostGamepadInputAdapter &self = *static_cast<EspUsbHostGamepadInputAdapter *>(context);
    portENTER_CRITICAL(&self.mux_);
    for (size_t i = self.padCount_; i > 0; --i)
    {
      if (self.pads_[i - 1].address == address)
      {
        self.pads_[i - 1] = self.pads_[self.padCount_ - 1];
        --self.padCount_;
      }
    }
    portEXIT_CRITICAL(&self.mux_);
  }

  EspUsbHost &usb_;
  bool subscribed_ = false;
  KeySet keys_;
  bool connected_ = false;
  Key buttonKeys_[MaxButtons] = {};
  Key hatUp_, hatDown_, hatLeft_, hatRight_;
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  Pad pads_[MaxGamepads];
  size_t padCount_ = 0;
};

// USB MIDI input. MIDI notes map to keys: Note On (velocity > 0) presses the
// mapped key, Note Off (or Note On with velocity 0) releases it. Only mapped
// notes produce output, and any Key kind is allowed, so a pad or piano key
// can become a shortcut, a media key, or a virtual key to remap/layer later.
//
// Messages arrive as onMidiMessage (EspUsbHost 2.3.0). Every channel is
// listened to by default; setChannel() narrows it to one. Only note on/off
// are used - control changes, pitch bend, etc. are ignored in this version.
class EspUsbHostMidiInputAdapter : public InputAdapter
{
public:
  explicit EspUsbHostMidiInputAdapter(EspUsbHost &usb) : usb_(usb) {}

  // Map a MIDI note (0-127) to any Key. Returns false if the note is out of
  // range; unmapped notes produce nothing.
  bool mapNote(uint8_t note, Key key)
  {
    if (note > 127)
    {
      return false;
    }
    noteKeys_[note] = key;
    return true;
  }

  // Listen to one MIDI channel (0-15), or all channels with -1 (default).
  void setChannel(int channel = -1) { channel_ = channel; }

  void update() override
  {
    if (!subscribed_)
    {
      subscribed_ = true;
      detail::EspUsbHostHub &hub = detail::EspUsbHostHub::forStack(usb_);
      hub.setMidiSink(&EspUsbHostMidiInputAdapter::handleMidi, this);
      hub.addDisconnectSink(&EspUsbHostMidiInputAdapter::handleDisconnect, this);
    }

    uint8_t onNotes[16];
    size_t deviceCount;
    portENTER_CRITICAL(&mux_);
    memcpy(onNotes, onNotes_, sizeof(onNotes));
    deviceCount = deviceCount_;
    portEXIT_CRITICAL(&mux_);

    keys_.clear();
    for (size_t note = 0; note < 128; ++note)
    {
      if ((onNotes[note >> 3] & (1u << (note & 7))) != 0 && isValid(noteKeys_[note]))
      {
        keys_.press(noteKeys_[note]);
      }
    }
    connected_ = deviceCount > 0;
  }

  const KeySet &keys() const override { return keys_; }
  bool connected() const override { return connected_; }

private:
  static constexpr size_t MaxDevices = 4;

  // Fires from the EspUsbHost task.
  static void handleMidi(void *context, const EspUsbHostMidiMessage &message)
  {
    EspUsbHostMidiInputAdapter &self = *static_cast<EspUsbHostMidiInputAdapter *>(context);
    const uint8_t type = message.status & 0xf0;
    const uint8_t channel = message.status & 0x0f;
    const bool noteOn = (type == 0x90) && (message.data2 > 0);
    const bool noteOff = (type == 0x80) || ((type == 0x90) && (message.data2 == 0));

    portENTER_CRITICAL(&self.mux_);
    // Track the device for presence (and clearing stuck notes on unplug).
    bool known = false;
    for (size_t i = 0; i < self.deviceCount_; ++i)
    {
      if (self.devices_[i] == message.address)
      {
        known = true;
        break;
      }
    }
    if (!known && self.deviceCount_ < MaxDevices)
    {
      self.devices_[self.deviceCount_++] = message.address;
    }

    if ((self.channel_ < 0 || channel == self.channel_) && (noteOn || noteOff))
    {
      const uint8_t note = message.data1 & 0x7f;
      if (noteOn)
      {
        self.onNotes_[note >> 3] |= static_cast<uint8_t>(1u << (note & 7));
      }
      else
      {
        self.onNotes_[note >> 3] &= static_cast<uint8_t>(~(1u << (note & 7)));
      }
    }
    portEXIT_CRITICAL(&self.mux_);
  }

  // Fires from the EspUsbHost task (any device, not only MIDI).
  static void handleDisconnect(void *context, uint8_t address)
  {
    EspUsbHostMidiInputAdapter &self = *static_cast<EspUsbHostMidiInputAdapter *>(context);
    portENTER_CRITICAL(&self.mux_);
    for (size_t i = self.deviceCount_; i > 0; --i)
    {
      if (self.devices_[i - 1] == address)
      {
        self.devices_[i - 1] = self.devices_[self.deviceCount_ - 1];
        --self.deviceCount_;
      }
    }
    // No MIDI device left: drop any notes still marked on, so they do not
    // reappear when a device is plugged back in.
    if (self.deviceCount_ == 0)
    {
      memset(self.onNotes_, 0, sizeof(self.onNotes_));
    }
    portEXIT_CRITICAL(&self.mux_);
  }

  EspUsbHost &usb_;
  bool subscribed_ = false;
  KeySet keys_;
  bool connected_ = false;
  int channel_ = -1;
  Key noteKeys_[128] = {};
  uint8_t onNotes_[16] = {};
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  uint8_t devices_[MaxDevices] = {};
  size_t deviceCount_ = 0;
};

} // namespace esp32keybridge
