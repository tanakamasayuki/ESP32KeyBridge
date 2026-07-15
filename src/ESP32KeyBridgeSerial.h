#pragma once

#include <ESP32KeyBridge.h>

#include <Arduino.h>

// Output adapters that write to an Arduino Print stream (USB CDC Serial, a
// hardware UART such as Serial1, SoftwareSerial, ...). No external library.

namespace esp32keybridge
{

// Text output: emits characters, not HID reports. It only consumes the
// text stream (writeText()) — the characters injected with
// typeText()/typeChar() and, when an input sets convertLayout(), the
// characters its keystrokes decode to. That decoding uses the input's
// engraving layout, so this output carries no layout of its own. Turning a
// USB HID barcode reader into a stream of text is the same path: give the
// scanner input convertLayout() and this output writes the decoded text.
class TextOutputAdapter : public OutputAdapter
{
public:
  explicit TextOutputAdapter(Print &out) : out_(out) {}

  // The key set is a HID concern; a text output ignores it.
  void write(const KeySet &keys) override { (void)keys; }

  // Characters from the text stream, written as UTF-8.
  void writeText(char32_t codepoint) override
  {
    uint8_t bytes[4];
    const size_t n = encodeUtf8(codepoint, bytes);
    for (size_t i = 0; i < n; ++i)
    {
      out_.write(bytes[i]);
    }
  }

  bool connected() const override { return true; }

private:
  static size_t encodeUtf8(char32_t cp, uint8_t *out)
  {
    if (cp < 0x80)
    {
      out[0] = static_cast<uint8_t>(cp);
      return 1;
    }
    if (cp < 0x800)
    {
      out[0] = static_cast<uint8_t>(0xc0 | (cp >> 6));
      out[1] = static_cast<uint8_t>(0x80 | (cp & 0x3f));
      return 2;
    }
    if (cp < 0x10000)
    {
      out[0] = static_cast<uint8_t>(0xe0 | (cp >> 12));
      out[1] = static_cast<uint8_t>(0x80 | ((cp >> 6) & 0x3f));
      out[2] = static_cast<uint8_t>(0x80 | (cp & 0x3f));
      return 3;
    }
    out[0] = static_cast<uint8_t>(0xf0 | (cp >> 18));
    out[1] = static_cast<uint8_t>(0x80 | ((cp >> 12) & 0x3f));
    out[2] = static_cast<uint8_t>(0x80 | ((cp >> 6) & 0x3f));
    out[3] = static_cast<uint8_t>(0x80 | (cp & 0x3f));
    return 4;
  }

  Print &out_;
};

// Diagnostic monitor: writes a human-readable line whenever the output key
// set changes, a character is typed, or a relative axis moves. Add it
// alongside a real output to watch what the bridge produces, or on its own
// to bring up input hardware before wiring the real output. Never acts as
// the lock authority.
class LogOutputAdapter : public OutputAdapter
{
public:
  explicit LogOutputAdapter(Print &out) : out_(out) {}

  void write(const KeySet &keys) override
  {
    if (sameAsLast(keys))
    {
      return;
    }
    last_ = keys;
    out_.print("KEYS");
    if (keys.count() == 0)
    {
      out_.print(" (empty)");
    }
    for (size_t i = 0; i < keys.count(); ++i)
    {
      const Key key = keys.at(i);
      out_.print(' ');
      out_.print(keyKindName(key.kind));
      out_.print(':');
      out_.print(key.code, HEX);
    }
    out_.println();
  }

  void writeText(char32_t codepoint) override
  {
    out_.print("TEXT ");
    out_.println(static_cast<uint32_t>(codepoint), HEX);
  }

  void writeAxisDelta(Axis axis, int32_t delta) override
  {
    out_.print("AXIS ");
    out_.print(axisName(axis));
    out_.print(' ');
    if (delta > 0)
    {
      out_.print('+');
    }
    out_.println(delta);
  }

  bool connected() const override { return true; }

private:
  static const char *axisName(Axis axis)
  {
    switch (axis)
    {
    case Axis::X:
      return "x";
    case Axis::Y:
      return "y";
    case Axis::Wheel:
      return "wheel";
    case Axis::Pan:
      return "pan";
    }
    return "?";
  }

  bool sameAsLast(const KeySet &keys) const
  {
    if (keys.count() != last_.count())
    {
      return false;
    }
    for (size_t i = 0; i < keys.count(); ++i)
    {
      if (!last_.contains(keys.at(i)))
      {
        return false;
      }
    }
    return true;
  }

  Print &out_;
  KeySet last_;
};

} // namespace esp32keybridge
