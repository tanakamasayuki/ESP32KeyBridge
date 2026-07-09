#pragma once

#include <ESP32KeyBridge.h>
#include <EspUsbHost.h>

namespace esp32keybridge
{

class EspUsbHostKeyboardInputAdapter : public InputAdapter
{
public:
  explicit EspUsbHostKeyboardInputAdapter(EspUsbHost &usb)
      : usb_(usb)
  {
  }

  void setLayout(KeyboardLayout layout)
  {
    layout_ = layout;
  }

  void begin()
  {
    usb_.onKeyboard([this](const EspUsbHostKeyboardEvent &event)
                    {
                      const KeySymbol key = layout_.decode(hidUsage(event.keycode));
                      if (key != KeySymbol::None)
                      {
                        state_.apply(keyEvent(key, event.pressed, millis()));
                      }
                    });
  }

  void update() override
  {
  }

  const InputState &state() const override
  {
    return state_;
  }

  void clear()
  {
    state_.clear();
  }

private:
  EspUsbHost &usb_;
  KeyboardLayout layout_ = KeyboardLayout::us();
  InputState state_;
};

} // namespace esp32keybridge
