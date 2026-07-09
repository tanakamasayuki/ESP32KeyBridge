#include "ESP32KeyBridge.h"

namespace esp32keybridge
{

namespace
{
int8_t clampReportDelta(int16_t value, bool &overflow)
{
  if (value > 127)
  {
    overflow = true;
    return 127;
  }
  if (value < -127)
  {
    overflow = true;
    return -127;
  }
  return static_cast<int8_t>(value);
}

int8_t addReportDelta(int8_t current, int16_t value, bool &overflow)
{
  return clampReportDelta(static_cast<int16_t>(current) + value, overflow);
}

KeySymbol decodeUs(HidUsage usage)
{
  switch (usage)
  {
  case HidUsage::Usage04: return KeySymbol::A;
  case HidUsage::Usage05: return KeySymbol::B;
  case HidUsage::Usage06: return KeySymbol::C;
  case HidUsage::Usage07: return KeySymbol::D;
  case HidUsage::Usage08: return KeySymbol::E;
  case HidUsage::Usage09: return KeySymbol::F;
  case HidUsage::Usage0A: return KeySymbol::G;
  case HidUsage::Usage0B: return KeySymbol::H;
  case HidUsage::Usage0C: return KeySymbol::I;
  case HidUsage::Usage0D: return KeySymbol::J;
  case HidUsage::Usage0E: return KeySymbol::K;
  case HidUsage::Usage0F: return KeySymbol::L;
  case HidUsage::Usage10: return KeySymbol::M;
  case HidUsage::Usage11: return KeySymbol::N;
  case HidUsage::Usage12: return KeySymbol::O;
  case HidUsage::Usage13: return KeySymbol::P;
  case HidUsage::Usage14: return KeySymbol::Q;
  case HidUsage::Usage15: return KeySymbol::R;
  case HidUsage::Usage16: return KeySymbol::S;
  case HidUsage::Usage17: return KeySymbol::T;
  case HidUsage::Usage18: return KeySymbol::U;
  case HidUsage::Usage19: return KeySymbol::V;
  case HidUsage::Usage1A: return KeySymbol::W;
  case HidUsage::Usage1B: return KeySymbol::X;
  case HidUsage::Usage1C: return KeySymbol::Y;
  case HidUsage::Usage1D: return KeySymbol::Z;
  case HidUsage::Usage1E: return KeySymbol::Num1;
  case HidUsage::Usage1F: return KeySymbol::Num2;
  case HidUsage::Usage20: return KeySymbol::Num3;
  case HidUsage::Usage21: return KeySymbol::Num4;
  case HidUsage::Usage22: return KeySymbol::Num5;
  case HidUsage::Usage23: return KeySymbol::Num6;
  case HidUsage::Usage24: return KeySymbol::Num7;
  case HidUsage::Usage25: return KeySymbol::Num8;
  case HidUsage::Usage26: return KeySymbol::Num9;
  case HidUsage::Usage27: return KeySymbol::Num0;
  case HidUsage::Usage28: return KeySymbol::Enter;
  case HidUsage::Usage29: return KeySymbol::Escape;
  case HidUsage::Usage2A: return KeySymbol::Backspace;
  case HidUsage::Usage2B: return KeySymbol::Tab;
  case HidUsage::Usage2C: return KeySymbol::Space;
  case HidUsage::Usage2D: return KeySymbol::Minus;
  case HidUsage::Usage2E: return KeySymbol::Equal;
  case HidUsage::Usage2F: return KeySymbol::LeftBracket;
  case HidUsage::Usage30: return KeySymbol::RightBracket;
  case HidUsage::Usage31: return KeySymbol::Backslash;
  case HidUsage::Usage32: return KeySymbol::NonUsHash;
  case HidUsage::Usage33: return KeySymbol::Semicolon;
  case HidUsage::Usage34: return KeySymbol::Quote;
  case HidUsage::Usage35: return KeySymbol::Grave;
  case HidUsage::Usage36: return KeySymbol::Comma;
  case HidUsage::Usage37: return KeySymbol::Dot;
  case HidUsage::Usage38: return KeySymbol::Slash;
  case HidUsage::Usage39: return KeySymbol::CapsLock;
  case HidUsage::Usage3A: return KeySymbol::F1;
  case HidUsage::Usage3B: return KeySymbol::F2;
  case HidUsage::Usage3C: return KeySymbol::F3;
  case HidUsage::Usage3D: return KeySymbol::F4;
  case HidUsage::Usage3E: return KeySymbol::F5;
  case HidUsage::Usage3F: return KeySymbol::F6;
  case HidUsage::Usage40: return KeySymbol::F7;
  case HidUsage::Usage41: return KeySymbol::F8;
  case HidUsage::Usage42: return KeySymbol::F9;
  case HidUsage::Usage43: return KeySymbol::F10;
  case HidUsage::Usage44: return KeySymbol::F11;
  case HidUsage::Usage45: return KeySymbol::F12;
  case HidUsage::Usage46: return KeySymbol::PrintScreen;
  case HidUsage::Usage47: return KeySymbol::ScrollLock;
  case HidUsage::Usage48: return KeySymbol::Pause;
  case HidUsage::Usage49: return KeySymbol::Insert;
  case HidUsage::Usage4A: return KeySymbol::Home;
  case HidUsage::Usage4B: return KeySymbol::PageUp;
  case HidUsage::Usage4C: return KeySymbol::Delete;
  case HidUsage::Usage4D: return KeySymbol::End;
  case HidUsage::Usage4E: return KeySymbol::PageDown;
  case HidUsage::Usage4F: return KeySymbol::Right;
  case HidUsage::Usage50: return KeySymbol::Left;
  case HidUsage::Usage51: return KeySymbol::Down;
  case HidUsage::Usage52: return KeySymbol::Up;
  case HidUsage::Usage64: return KeySymbol::NonUsBackslash;
  case HidUsage::Usage87: return KeySymbol::International1;
  case HidUsage::Usage88: return KeySymbol::International2;
  case HidUsage::Usage89: return KeySymbol::International3;
  case HidUsage::Usage8A: return KeySymbol::International4;
  case HidUsage::Usage8B: return KeySymbol::International5;
  case HidUsage::Usage8C: return KeySymbol::International6;
  case HidUsage::Usage8D: return KeySymbol::International7;
  case HidUsage::Usage8E: return KeySymbol::International8;
  case HidUsage::Usage8F: return KeySymbol::International9;
  case HidUsage::Usage90: return KeySymbol::Lang1;
  case HidUsage::Usage91: return KeySymbol::Lang2;
  case HidUsage::Usage92: return KeySymbol::Lang3;
  case HidUsage::Usage93: return KeySymbol::Lang4;
  case HidUsage::Usage94: return KeySymbol::Lang5;
  case HidUsage::Usage95: return KeySymbol::Lang6;
  case HidUsage::Usage96: return KeySymbol::Lang7;
  case HidUsage::Usage97: return KeySymbol::Lang8;
  case HidUsage::Usage98: return KeySymbol::Lang9;
  case HidUsage::UsageE0: return KeySymbol::LeftCtrl;
  case HidUsage::UsageE1: return KeySymbol::LeftShift;
  case HidUsage::UsageE2: return KeySymbol::LeftAlt;
  case HidUsage::UsageE3: return KeySymbol::LeftGui;
  case HidUsage::UsageE4: return KeySymbol::RightCtrl;
  case HidUsage::UsageE5: return KeySymbol::RightShift;
  case HidUsage::UsageE6: return KeySymbol::RightAlt;
  case HidUsage::UsageE7: return KeySymbol::RightGui;
  case HidUsage::None: break;
  }
  return KeySymbol::None;
}

HidUsage encodeUs(KeySymbol key)
{
  switch (key)
  {
  case KeySymbol::A: return HidUsage::Usage04;
  case KeySymbol::B: return HidUsage::Usage05;
  case KeySymbol::C: return HidUsage::Usage06;
  case KeySymbol::D: return HidUsage::Usage07;
  case KeySymbol::E: return HidUsage::Usage08;
  case KeySymbol::F: return HidUsage::Usage09;
  case KeySymbol::G: return HidUsage::Usage0A;
  case KeySymbol::H: return HidUsage::Usage0B;
  case KeySymbol::I: return HidUsage::Usage0C;
  case KeySymbol::J: return HidUsage::Usage0D;
  case KeySymbol::K: return HidUsage::Usage0E;
  case KeySymbol::L: return HidUsage::Usage0F;
  case KeySymbol::M: return HidUsage::Usage10;
  case KeySymbol::N: return HidUsage::Usage11;
  case KeySymbol::O: return HidUsage::Usage12;
  case KeySymbol::P: return HidUsage::Usage13;
  case KeySymbol::Q: return HidUsage::Usage14;
  case KeySymbol::R: return HidUsage::Usage15;
  case KeySymbol::S: return HidUsage::Usage16;
  case KeySymbol::T: return HidUsage::Usage17;
  case KeySymbol::U: return HidUsage::Usage18;
  case KeySymbol::V: return HidUsage::Usage19;
  case KeySymbol::W: return HidUsage::Usage1A;
  case KeySymbol::X: return HidUsage::Usage1B;
  case KeySymbol::Y: return HidUsage::Usage1C;
  case KeySymbol::Z: return HidUsage::Usage1D;
  case KeySymbol::Num1: return HidUsage::Usage1E;
  case KeySymbol::Num2: return HidUsage::Usage1F;
  case KeySymbol::Num3: return HidUsage::Usage20;
  case KeySymbol::Num4: return HidUsage::Usage21;
  case KeySymbol::Num5: return HidUsage::Usage22;
  case KeySymbol::Num6: return HidUsage::Usage23;
  case KeySymbol::Num7: return HidUsage::Usage24;
  case KeySymbol::Num8: return HidUsage::Usage25;
  case KeySymbol::Num9: return HidUsage::Usage26;
  case KeySymbol::Num0: return HidUsage::Usage27;
  case KeySymbol::Enter: return HidUsage::Usage28;
  case KeySymbol::Escape: return HidUsage::Usage29;
  case KeySymbol::Backspace: return HidUsage::Usage2A;
  case KeySymbol::Tab: return HidUsage::Usage2B;
  case KeySymbol::Space: return HidUsage::Usage2C;
  case KeySymbol::Minus: return HidUsage::Usage2D;
  case KeySymbol::Equal: return HidUsage::Usage2E;
  case KeySymbol::LeftBracket: return HidUsage::Usage2F;
  case KeySymbol::RightBracket: return HidUsage::Usage30;
  case KeySymbol::Backslash: return HidUsage::Usage31;
  case KeySymbol::NonUsHash: return HidUsage::Usage32;
  case KeySymbol::Semicolon: return HidUsage::Usage33;
  case KeySymbol::Quote: return HidUsage::Usage34;
  case KeySymbol::Grave: return HidUsage::Usage35;
  case KeySymbol::Comma: return HidUsage::Usage36;
  case KeySymbol::Dot: return HidUsage::Usage37;
  case KeySymbol::Slash: return HidUsage::Usage38;
  case KeySymbol::CapsLock: return HidUsage::Usage39;
  case KeySymbol::F1: return HidUsage::Usage3A;
  case KeySymbol::F2: return HidUsage::Usage3B;
  case KeySymbol::F3: return HidUsage::Usage3C;
  case KeySymbol::F4: return HidUsage::Usage3D;
  case KeySymbol::F5: return HidUsage::Usage3E;
  case KeySymbol::F6: return HidUsage::Usage3F;
  case KeySymbol::F7: return HidUsage::Usage40;
  case KeySymbol::F8: return HidUsage::Usage41;
  case KeySymbol::F9: return HidUsage::Usage42;
  case KeySymbol::F10: return HidUsage::Usage43;
  case KeySymbol::F11: return HidUsage::Usage44;
  case KeySymbol::F12: return HidUsage::Usage45;
  case KeySymbol::PrintScreen: return HidUsage::Usage46;
  case KeySymbol::ScrollLock: return HidUsage::Usage47;
  case KeySymbol::Pause: return HidUsage::Usage48;
  case KeySymbol::Insert: return HidUsage::Usage49;
  case KeySymbol::Home: return HidUsage::Usage4A;
  case KeySymbol::PageUp: return HidUsage::Usage4B;
  case KeySymbol::Delete: return HidUsage::Usage4C;
  case KeySymbol::End: return HidUsage::Usage4D;
  case KeySymbol::PageDown: return HidUsage::Usage4E;
  case KeySymbol::Right: return HidUsage::Usage4F;
  case KeySymbol::Left: return HidUsage::Usage50;
  case KeySymbol::Down: return HidUsage::Usage51;
  case KeySymbol::Up: return HidUsage::Usage52;
  case KeySymbol::NonUsBackslash: return HidUsage::Usage64;
  case KeySymbol::International1: return HidUsage::Usage87;
  case KeySymbol::International2: return HidUsage::Usage88;
  case KeySymbol::International3: return HidUsage::Usage89;
  case KeySymbol::International4: return HidUsage::Usage8A;
  case KeySymbol::International5: return HidUsage::Usage8B;
  case KeySymbol::International6: return HidUsage::Usage8C;
  case KeySymbol::International7: return HidUsage::Usage8D;
  case KeySymbol::International8: return HidUsage::Usage8E;
  case KeySymbol::International9: return HidUsage::Usage8F;
  case KeySymbol::Lang1: return HidUsage::Usage90;
  case KeySymbol::Lang2: return HidUsage::Usage91;
  case KeySymbol::Lang3: return HidUsage::Usage92;
  case KeySymbol::Lang4: return HidUsage::Usage93;
  case KeySymbol::Lang5: return HidUsage::Usage94;
  case KeySymbol::Lang6: return HidUsage::Usage95;
  case KeySymbol::Lang7: return HidUsage::Usage96;
  case KeySymbol::Lang8: return HidUsage::Usage97;
  case KeySymbol::Lang9: return HidUsage::Usage98;
  case KeySymbol::LeftCtrl: return HidUsage::UsageE0;
  case KeySymbol::LeftShift: return HidUsage::UsageE1;
  case KeySymbol::LeftAlt: return HidUsage::UsageE2;
  case KeySymbol::LeftGui: return HidUsage::UsageE3;
  case KeySymbol::RightCtrl: return HidUsage::UsageE4;
  case KeySymbol::RightShift: return HidUsage::UsageE5;
  case KeySymbol::RightAlt: return HidUsage::UsageE6;
  case KeySymbol::RightGui: return HidUsage::UsageE7;
  case KeySymbol::None:
  case KeySymbol::Fn1:
    break;
  }
  return HidUsage::None;
}
} // namespace

bool InputCode::operator==(const InputCode &other) const
{
  return domain == other.domain && code == other.code;
}

bool InputCode::operator!=(const InputCode &other) const
{
  return !(*this == other);
}

InputCode keyboardCode(KeySymbol key)
{
  return {InputDomain::Keyboard, static_cast<uint16_t>(key)};
}

KeyboardLayout::KeyboardLayout(KeyboardLayoutId id) : id_(id) {}

KeyboardLayout KeyboardLayout::us()
{
  return KeyboardLayout(KeyboardLayoutId::Us);
}

KeyboardLayout KeyboardLayout::fr()
{
  return KeyboardLayout(KeyboardLayoutId::Fr);
}

KeyboardLayoutId KeyboardLayout::id() const
{
  return id_;
}

KeySymbol KeyboardLayout::decode(HidUsage usage) const
{
  if (usage == HidUsage::None)
  {
    return KeySymbol::None;
  }

  if (id_ == KeyboardLayoutId::Fr)
  {
    switch (usage)
    {
    case HidUsage::Usage04:
      return KeySymbol::Q;
    case HidUsage::Usage14:
      return KeySymbol::A;
    case HidUsage::Usage1A:
      return KeySymbol::Z;
    case HidUsage::Usage1D:
      return KeySymbol::W;
    case HidUsage::Usage33:
      return KeySymbol::M;
    case HidUsage::Usage10:
      return KeySymbol::Semicolon;
    default:
      break;
    }
  }

  return decodeUs(usage);
}

HidUsage KeyboardLayout::encode(KeySymbol key) const
{
  if (key == KeySymbol::None || key == KeySymbol::Fn1)
  {
    return HidUsage::None;
  }

  if (id_ == KeyboardLayoutId::Fr)
  {
    switch (key)
    {
    case KeySymbol::Q:
      return HidUsage::Usage04;
    case KeySymbol::A:
      return HidUsage::Usage14;
    case KeySymbol::Z:
      return HidUsage::Usage1A;
    case KeySymbol::W:
      return HidUsage::Usage1D;
    case KeySymbol::M:
      return HidUsage::Usage33;
    case KeySymbol::Semicolon:
      return HidUsage::Usage10;
    default:
      break;
    }
  }

  return encodeUs(key);
}

InputCode consumerCode(uint16_t code)
{
  return {InputDomain::Consumer, code};
}

InputCode consumerCode(ConsumerUsage usage)
{
  return consumerCode(static_cast<uint16_t>(usage));
}

InputCode pointerButtonCode(uint16_t code)
{
  return {InputDomain::PointerButton, code};
}

InputCode pointerAxisCode(uint16_t code)
{
  return {InputDomain::PointerAxis, code};
}

InputCode pointerAxisCode(PointerAxis axis)
{
  return pointerAxisCode(static_cast<uint16_t>(axis));
}

InputCode vendorCode(uint16_t code)
{
  return {InputDomain::Vendor, code};
}

uint16_t hidUsageValue(HidUsage usage)
{
  return static_cast<uint16_t>(usage);
}

HidUsage hidUsage(uint16_t usage)
{
  return usage == 0 || usage > 0xff ? HidUsage::None : static_cast<HidUsage>(usage);
}

uint16_t hidUsageFromKeySymbol(KeySymbol key)
{
  return hidUsageValue(KeyboardLayout::us().encode(key));
}

KeySymbol keySymbolFromHidUsage(uint16_t usage)
{
  return KeyboardLayout::us().decode(hidUsage(usage));
}

bool isHidKeyboardKeySymbol(KeySymbol key)
{
  return KeyboardLayout::us().encode(key) != HidUsage::None;
}

KeySymbol keySymbolFromCode(InputCode code)
{
  if (code.domain != InputDomain::Keyboard)
  {
    return KeySymbol::None;
  }
  return static_cast<KeySymbol>(code.code);
}

InputEvent inputEvent(InputCode code, bool pressed, uint32_t timestampMs)
{
  return {code, pressed, timestampMs};
}

InputEvent keyEvent(KeySymbol key, bool pressed, uint32_t timestampMs)
{
  return inputEvent(keyboardCode(key), pressed, timestampMs);
}

InputValueEvent inputValueEvent(InputCode code, int16_t value, uint32_t timestampMs)
{
  return {code, value, timestampMs};
}

InputValueEvent pointerAxisValueEvent(PointerAxis axis, int16_t value, uint32_t timestampMs)
{
  return inputValueEvent(pointerAxisCode(axis), value, timestampMs);
}

const char *inputDomainName(InputDomain domain)
{
  switch (domain)
  {
  case InputDomain::Keyboard:
    return "Keyboard";
  case InputDomain::Consumer:
    return "Consumer";
  case InputDomain::PointerButton:
    return "PointerButton";
  case InputDomain::PointerAxis:
    return "PointerAxis";
  case InputDomain::Vendor:
    return "Vendor";
  }
  return "Unknown";
}

bool isValid(InputCode code)
{
  return code.code != 0;
}

bool isModifierKeySymbol(KeySymbol key)
{
  return key == KeySymbol::LeftCtrl || key == KeySymbol::LeftShift || key == KeySymbol::LeftAlt || key == KeySymbol::LeftGui ||
         key == KeySymbol::RightCtrl || key == KeySymbol::RightShift || key == KeySymbol::RightAlt || key == KeySymbol::RightGui;
}

const char *keySymbolName(KeySymbol key)
{
  switch (key)
  {
  case KeySymbol::None:
    return "None";
  case KeySymbol::A:
    return "A";
  case KeySymbol::B:
    return "B";
  case KeySymbol::C:
    return "C";
  case KeySymbol::D:
    return "D";
  case KeySymbol::E:
    return "E";
  case KeySymbol::F:
    return "F";
  case KeySymbol::G:
    return "G";
  case KeySymbol::H:
    return "H";
  case KeySymbol::I:
    return "I";
  case KeySymbol::J:
    return "J";
  case KeySymbol::K:
    return "K";
  case KeySymbol::L:
    return "L";
  case KeySymbol::M:
    return "M";
  case KeySymbol::N:
    return "N";
  case KeySymbol::O:
    return "O";
  case KeySymbol::P:
    return "P";
  case KeySymbol::Q:
    return "Q";
  case KeySymbol::R:
    return "R";
  case KeySymbol::S:
    return "S";
  case KeySymbol::T:
    return "T";
  case KeySymbol::U:
    return "U";
  case KeySymbol::V:
    return "V";
  case KeySymbol::W:
    return "W";
  case KeySymbol::X:
    return "X";
  case KeySymbol::Y:
    return "Y";
  case KeySymbol::Z:
    return "Z";
  case KeySymbol::Num1:
    return "Num1";
  case KeySymbol::Num2:
    return "Num2";
  case KeySymbol::Num3:
    return "Num3";
  case KeySymbol::Num4:
    return "Num4";
  case KeySymbol::Num5:
    return "Num5";
  case KeySymbol::Num6:
    return "Num6";
  case KeySymbol::Num7:
    return "Num7";
  case KeySymbol::Num8:
    return "Num8";
  case KeySymbol::Num9:
    return "Num9";
  case KeySymbol::Num0:
    return "Num0";
  case KeySymbol::Enter:
    return "Enter";
  case KeySymbol::Escape:
    return "Escape";
  case KeySymbol::Backspace:
    return "Backspace";
  case KeySymbol::Tab:
    return "Tab";
  case KeySymbol::Space:
    return "Space";
  case KeySymbol::Minus:
    return "Minus";
  case KeySymbol::Equal:
    return "Equal";
  case KeySymbol::LeftBracket:
    return "LeftBracket";
  case KeySymbol::RightBracket:
    return "RightBracket";
  case KeySymbol::Backslash:
    return "Backslash";
  case KeySymbol::NonUsHash:
    return "NonUsHash";
  case KeySymbol::Semicolon:
    return "Semicolon";
  case KeySymbol::Quote:
    return "Quote";
  case KeySymbol::Grave:
    return "Grave";
  case KeySymbol::Comma:
    return "Comma";
  case KeySymbol::Dot:
    return "Dot";
  case KeySymbol::Slash:
    return "Slash";
  case KeySymbol::Insert:
    return "Insert";
  case KeySymbol::CapsLock:
    return "CapsLock";
  case KeySymbol::F1:
    return "F1";
  case KeySymbol::F2:
    return "F2";
  case KeySymbol::F3:
    return "F3";
  case KeySymbol::F4:
    return "F4";
  case KeySymbol::F5:
    return "F5";
  case KeySymbol::F6:
    return "F6";
  case KeySymbol::F7:
    return "F7";
  case KeySymbol::F8:
    return "F8";
  case KeySymbol::F9:
    return "F9";
  case KeySymbol::F10:
    return "F10";
  case KeySymbol::F11:
    return "F11";
  case KeySymbol::F12:
    return "F12";
  case KeySymbol::PrintScreen:
    return "PrintScreen";
  case KeySymbol::ScrollLock:
    return "ScrollLock";
  case KeySymbol::Pause:
    return "Pause";
  case KeySymbol::Home:
    return "Home";
  case KeySymbol::PageUp:
    return "PageUp";
  case KeySymbol::Delete:
    return "Delete";
  case KeySymbol::End:
    return "End";
  case KeySymbol::PageDown:
    return "PageDown";
  case KeySymbol::Right:
    return "Right";
  case KeySymbol::Left:
    return "Left";
  case KeySymbol::Down:
    return "Down";
  case KeySymbol::Up:
    return "Up";
  case KeySymbol::NonUsBackslash:
    return "NonUsBackslash";
  case KeySymbol::International1:
    return "International1";
  case KeySymbol::International2:
    return "International2";
  case KeySymbol::International3:
    return "International3";
  case KeySymbol::International4:
    return "International4";
  case KeySymbol::International5:
    return "International5";
  case KeySymbol::International6:
    return "International6";
  case KeySymbol::International7:
    return "International7";
  case KeySymbol::International8:
    return "International8";
  case KeySymbol::International9:
    return "International9";
  case KeySymbol::Lang1:
    return "Lang1";
  case KeySymbol::Lang2:
    return "Lang2";
  case KeySymbol::Lang3:
    return "Lang3";
  case KeySymbol::Lang4:
    return "Lang4";
  case KeySymbol::Lang5:
    return "Lang5";
  case KeySymbol::Lang6:
    return "Lang6";
  case KeySymbol::Lang7:
    return "Lang7";
  case KeySymbol::Lang8:
    return "Lang8";
  case KeySymbol::Lang9:
    return "Lang9";
  case KeySymbol::LeftCtrl:
    return "LeftCtrl";
  case KeySymbol::LeftShift:
    return "LeftShift";
  case KeySymbol::LeftAlt:
    return "LeftAlt";
  case KeySymbol::LeftGui:
    return "LeftGui";
  case KeySymbol::RightCtrl:
    return "RightCtrl";
  case KeySymbol::RightShift:
    return "RightShift";
  case KeySymbol::RightAlt:
    return "RightAlt";
  case KeySymbol::RightGui:
    return "RightGui";
  case KeySymbol::Fn1:
    return "Fn1";
  }
  return "Unknown";
}

const char *consumerUsageName(ConsumerUsage usage)
{
  switch (usage)
  {
  case ConsumerUsage::None:
    return "None";
  case ConsumerUsage::Power:
    return "Power";
  case ConsumerUsage::Sleep:
    return "Sleep";
  case ConsumerUsage::Menu:
    return "Menu";
  case ConsumerUsage::Play:
    return "Play";
  case ConsumerUsage::Pause:
    return "Pause";
  case ConsumerUsage::Record:
    return "Record";
  case ConsumerUsage::FastForward:
    return "FastForward";
  case ConsumerUsage::Rewind:
    return "Rewind";
  case ConsumerUsage::ScanNextTrack:
    return "ScanNextTrack";
  case ConsumerUsage::ScanPreviousTrack:
    return "ScanPreviousTrack";
  case ConsumerUsage::Stop:
    return "Stop";
  case ConsumerUsage::Eject:
    return "Eject";
  case ConsumerUsage::PlayPause:
    return "PlayPause";
  case ConsumerUsage::Mute:
    return "Mute";
  case ConsumerUsage::BassBoost:
    return "BassBoost";
  case ConsumerUsage::VolumeIncrement:
    return "VolumeIncrement";
  case ConsumerUsage::VolumeDecrement:
    return "VolumeDecrement";
  case ConsumerUsage::BrowserSearch:
    return "BrowserSearch";
  case ConsumerUsage::BrowserHome:
    return "BrowserHome";
  case ConsumerUsage::BrowserBack:
    return "BrowserBack";
  case ConsumerUsage::BrowserForward:
    return "BrowserForward";
  case ConsumerUsage::BrowserRefresh:
    return "BrowserRefresh";
  case ConsumerUsage::BrowserBookmarks:
    return "BrowserBookmarks";
  }
  return "Unknown";
}

const char *pointerAxisName(PointerAxis axis)
{
  switch (axis)
  {
  case PointerAxis::None:
    return "None";
  case PointerAxis::X:
    return "X";
  case PointerAxis::Y:
    return "Y";
  case PointerAxis::Wheel:
    return "Wheel";
  case PointerAxis::Pan:
    return "Pan";
  }
  return "Unknown";
}

void InputState::clear()
{
  codeCount_ = 0;
}

bool InputState::press(KeySymbol key)
{
  return press(keyboardCode(key));
}

bool InputState::press(InputCode code)
{
  if (!isValid(code))
  {
    return false;
  }
  if (contains(code))
  {
    return true;
  }
  if (codeCount_ >= MaxCodes)
  {
    return false;
  }
  codes_[codeCount_++] = code;
  return true;
}

bool InputState::mergeFrom(const InputState &other)
{
  bool mergedAll = true;
  for (size_t i = 0; i < other.codeCount(); ++i)
  {
    if (!press(other.codeAt(i)))
    {
      mergedAll = false;
    }
  }
  return mergedAll;
}

bool InputState::release(KeySymbol key)
{
  return release(keyboardCode(key));
}

bool InputState::release(InputCode code)
{
  for (size_t i = 0; i < codeCount_; ++i)
  {
    if (codes_[i] == code)
    {
      codes_[i] = codes_[codeCount_ - 1];
      --codeCount_;
      return true;
    }
  }
  return false;
}

bool InputState::contains(KeySymbol key) const
{
  return contains(keyboardCode(key));
}

bool InputState::contains(InputCode code) const
{
  for (size_t i = 0; i < codeCount_; ++i)
  {
    if (codes_[i] == code)
    {
      return true;
    }
  }
  return false;
}

bool InputState::isPressed(KeySymbol key) const
{
  return contains(key);
}

bool InputState::isPressed(InputCode code) const
{
  return contains(code);
}

bool InputState::apply(InputEvent event)
{
  return event.pressed ? press(event.code) : release(event.code);
}

size_t InputState::codeCount() const
{
  return codeCount_;
}

KeySymbol InputState::keyAt(size_t index) const
{
  return index < codeCount_ ? keySymbolFromCode(codes_[index]) : KeySymbol::None;
}

InputCode InputState::codeAt(size_t index) const
{
  return index < codeCount_ ? codes_[index] : keyboardCode(KeySymbol::None);
}

void HidKeyboardReport::clear()
{
  modifiers = 0;
  for (size_t i = 0; i < MaxKeys; ++i)
  {
    keys[i] = 0;
  }
  keyCount = 0;
  overflow = false;
}

bool HidKeyboardReport::empty() const
{
  return modifiers == 0 && keyCount == 0 && !overflow;
}

bool HidKeyboardReport::writeBootReport(uint8_t *buffer, size_t size) const
{
  if (buffer == nullptr || size < BootReportSize)
  {
    return false;
  }

  buffer[0] = modifiers;
  buffer[1] = 0;
  for (size_t i = 0; i < MaxKeys; ++i)
  {
    buffer[2 + i] = i < keyCount ? keys[i] : 0;
  }
  return true;
}

void HidKeyboardRolloverReport::clear()
{
  modifiers = 0;
  for (size_t i = 0; i < MaxKeys; ++i)
  {
    keys[i] = 0;
  }
  keyCount = 0;
  overflow = false;
}

bool HidKeyboardRolloverReport::empty() const
{
  return modifiers == 0 && keyCount == 0 && !overflow;
}

bool HidKeyboardRolloverReport::writeReport(uint8_t *buffer, size_t size) const
{
  if (buffer == nullptr || size < ReportSize)
  {
    return false;
  }

  buffer[0] = modifiers;
  for (size_t i = 0; i < MaxKeys; ++i)
  {
    buffer[1 + i] = i < keyCount ? keys[i] : 0;
  }
  return true;
}

void HidConsumerReport::clear()
{
  usage = 0;
  overflow = false;
}

bool HidConsumerReport::empty() const
{
  return usage == 0 && !overflow;
}

bool HidConsumerReport::writeReport(uint8_t *buffer, size_t size) const
{
  if (buffer == nullptr || size < ReportSize)
  {
    return false;
  }

  buffer[0] = static_cast<uint8_t>(usage & 0xff);
  buffer[1] = static_cast<uint8_t>((usage >> 8) & 0xff);
  return true;
}

void HidPointerReport::clear()
{
  buttons = 0;
  x = 0;
  y = 0;
  wheel = 0;
  pan = 0;
  overflow = false;
}

bool HidPointerReport::empty() const
{
  return buttons == 0 && x == 0 && y == 0 && wheel == 0 && pan == 0 && !overflow;
}

bool HidPointerReport::apply(InputValueEvent event)
{
  if (event.code.domain != InputDomain::PointerAxis || !isValid(event.code))
  {
    return false;
  }

  switch (static_cast<PointerAxis>(event.code.code))
  {
  case PointerAxis::X:
    x = addReportDelta(x, event.value, overflow);
    return true;
  case PointerAxis::Y:
    y = addReportDelta(y, event.value, overflow);
    return true;
  case PointerAxis::Wheel:
    wheel = addReportDelta(wheel, event.value, overflow);
    return true;
  case PointerAxis::Pan:
    pan = addReportDelta(pan, event.value, overflow);
    return true;
  case PointerAxis::None:
    break;
  }
  return false;
}

bool HidPointerReport::writeReport(uint8_t *buffer, size_t size) const
{
  if (buffer == nullptr || size < ReportSize)
  {
    return false;
  }

  buffer[0] = buttons;
  buffer[1] = static_cast<uint8_t>(x);
  buffer[2] = static_cast<uint8_t>(y);
  buffer[3] = static_cast<uint8_t>(wheel);
  buffer[4] = static_cast<uint8_t>(pan);
  return true;
}

HidKeyboardReport buildHidKeyboardReport(const InputState &state)
{
  HidKeyboardReport report;
  for (size_t i = 0; i < state.codeCount(); ++i)
  {
    const InputCode code = state.codeAt(i);
    if (code.domain != InputDomain::Keyboard)
    {
      continue;
    }

    const KeySymbol key = keySymbolFromCode(code);
    const uint16_t usage = hidUsageFromKeySymbol(key);
    if (usage == 0)
    {
      continue;
    }

    if (usage >= 0xe0 && usage <= 0xe7)
    {
      report.modifiers |= static_cast<uint8_t>(1u << (usage - 0xe0));
      continue;
    }

    if (report.keyCount >= HidKeyboardReport::MaxKeys)
    {
      report.overflow = true;
      continue;
    }

    report.keys[report.keyCount++] = static_cast<uint8_t>(usage);
  }
  return report;
}

HidKeyboardRolloverReport buildHidKeyboardRolloverReport(const InputState &state)
{
  HidKeyboardRolloverReport report;
  for (size_t i = 0; i < state.codeCount(); ++i)
  {
    const InputCode code = state.codeAt(i);
    if (code.domain != InputDomain::Keyboard)
    {
      continue;
    }

    const KeySymbol key = keySymbolFromCode(code);
    const uint16_t usage = hidUsageFromKeySymbol(key);
    if (usage == 0)
    {
      continue;
    }

    if (usage >= 0xe0 && usage <= 0xe7)
    {
      report.modifiers |= static_cast<uint8_t>(1u << (usage - 0xe0));
      continue;
    }

    if (report.keyCount >= HidKeyboardRolloverReport::MaxKeys)
    {
      report.overflow = true;
      continue;
    }

    report.keys[report.keyCount++] = static_cast<uint8_t>(usage);
  }
  return report;
}

HidConsumerReport buildHidConsumerReport(const InputState &state)
{
  HidConsumerReport report;
  for (size_t i = 0; i < state.codeCount(); ++i)
  {
    const InputCode code = state.codeAt(i);
    if (code.domain != InputDomain::Consumer || !isValid(code))
    {
      continue;
    }

    if (report.usage != 0)
    {
      report.overflow = true;
      continue;
    }

    report.usage = code.code;
  }
  return report;
}

HidPointerReport buildHidPointerReport(const InputState &state)
{
  HidPointerReport report;
  for (size_t i = 0; i < state.codeCount(); ++i)
  {
    const InputCode code = state.codeAt(i);
    if (code.domain != InputDomain::PointerButton || !isValid(code))
    {
      continue;
    }

    if (code.code > HidPointerReport::MaxButtons)
    {
      report.overflow = true;
      continue;
    }

    report.buttons |= static_cast<uint8_t>(1u << (code.code - 1));
  }
  return report;
}

void EventInputAdapter::update() {}

const InputState &EventInputAdapter::state() const
{
  return state_;
}

bool EventInputAdapter::apply(InputEvent event)
{
  return state_.apply(event);
}

void EventInputAdapter::clear()
{
  state_.clear();
}

void RecordingOutputAdapter::write(const InputState &state)
{
  state_ = state;
  ++writeCount_;
}

const InputState &RecordingOutputAdapter::state() const
{
  return state_;
}

size_t RecordingOutputAdapter::writeCount() const
{
  return writeCount_;
}

void RecordingOutputAdapter::clear()
{
  state_.clear();
  writeCount_ = 0;
}

void RecordingHidKeyboardOutputAdapter::write(const InputState &state)
{
  report_ = buildHidKeyboardReport(state);
  ++writeCount_;
}

const HidKeyboardReport &RecordingHidKeyboardOutputAdapter::report() const
{
  return report_;
}

size_t RecordingHidKeyboardOutputAdapter::writeCount() const
{
  return writeCount_;
}

void RecordingHidKeyboardOutputAdapter::clear()
{
  report_.clear();
  writeCount_ = 0;
}

void RecordingHidKeyboardRolloverOutputAdapter::write(const InputState &state)
{
  report_ = buildHidKeyboardRolloverReport(state);
  ++writeCount_;
}

const HidKeyboardRolloverReport &RecordingHidKeyboardRolloverOutputAdapter::report() const
{
  return report_;
}

size_t RecordingHidKeyboardRolloverOutputAdapter::writeCount() const
{
  return writeCount_;
}

void RecordingHidKeyboardRolloverOutputAdapter::clear()
{
  report_.clear();
  writeCount_ = 0;
}

void RecordingHidConsumerOutputAdapter::write(const InputState &state)
{
  report_ = buildHidConsumerReport(state);
  ++writeCount_;
}

const HidConsumerReport &RecordingHidConsumerOutputAdapter::report() const
{
  return report_;
}

size_t RecordingHidConsumerOutputAdapter::writeCount() const
{
  return writeCount_;
}

void RecordingHidConsumerOutputAdapter::clear()
{
  report_.clear();
  writeCount_ = 0;
}

void RecordingHidPointerOutputAdapter::write(const InputState &state)
{
  report_ = buildHidPointerReport(state);
  ++writeCount_;
}

const HidPointerReport &RecordingHidPointerOutputAdapter::report() const
{
  return report_;
}

size_t RecordingHidPointerOutputAdapter::writeCount() const
{
  return writeCount_;
}

void RecordingHidPointerOutputAdapter::clear()
{
  report_.clear();
  writeCount_ = 0;
}

bool TransformConfig::remap(KeySymbol from, KeySymbol to)
{
  return remap(keyboardCode(from), keyboardCode(to));
}

bool TransformConfig::remap(InputCode from, InputCode to)
{
  if (!isValid(from) || !isValid(to))
  {
    return false;
  }
  for (size_t i = 0; i < remapCount_; ++i)
  {
    if (remaps_[i].from == from)
    {
      remaps_[i].to = to;
      return true;
    }
  }
  if (remapCount_ >= MaxRemaps)
  {
    return false;
  }
  remaps_[remapCount_++] = {from, to};
  return true;
}

bool TransformConfig::disable(KeySymbol key)
{
  return disable(keyboardCode(key));
}

bool TransformConfig::disable(InputCode code)
{
  if (!isValid(code) || isDisabled(code))
  {
    return isValid(code);
  }
  if (disabledCodeCount_ >= MaxDisabledKeys)
  {
    return false;
  }
  disabledCodes_[disabledCodeCount_++] = code;
  return true;
}

bool TransformConfig::macro(KeySymbol trigger, const KeySymbol *keys, size_t keyCount)
{
  if (trigger == KeySymbol::None || keys == nullptr || keyCount == 0 || keyCount > KeyMacro::MaxKeys)
  {
    return false;
  }
  for (size_t i = 0; i < keyCount; ++i)
  {
    if (keys[i] == KeySymbol::None)
    {
      return false;
    }
  }

  for (size_t i = 0; i < macroCount_; ++i)
  {
    if (macros_[i].trigger == trigger)
    {
      macros_[i].keyCount = keyCount;
      for (size_t j = 0; j < keyCount; ++j)
      {
        macros_[i].keys[j] = keys[j];
      }
      return true;
    }
  }

  if (macroCount_ >= MaxMacros)
  {
    return false;
  }

  macros_[macroCount_].trigger = trigger;
  macros_[macroCount_].keyCount = keyCount;
  for (size_t i = 0; i < keyCount; ++i)
  {
    macros_[macroCount_].keys[i] = keys[i];
  }
  ++macroCount_;
  return true;
}

void TransformConfig::clear()
{
  remapCount_ = 0;
  disabledCodeCount_ = 0;
  macroCount_ = 0;
}

KeySymbol TransformConfig::map(KeySymbol key) const
{
  return keySymbolFromCode(map(keyboardCode(key)));
}

InputCode TransformConfig::map(InputCode code) const
{
  for (size_t i = 0; i < remapCount_; ++i)
  {
    if (remaps_[i].from == code)
    {
      return remaps_[i].to;
    }
  }
  return code;
}

bool TransformConfig::isDisabled(KeySymbol key) const
{
  return isDisabled(keyboardCode(key));
}

bool TransformConfig::isDisabled(InputCode code) const
{
  for (size_t i = 0; i < disabledCodeCount_; ++i)
  {
    if (disabledCodes_[i] == code)
    {
      return true;
    }
  }
  return false;
}

const KeyMacro *TransformConfig::findMacro(KeySymbol trigger) const
{
  for (size_t i = 0; i < macroCount_; ++i)
  {
    if (macros_[i].trigger == trigger)
    {
      return &macros_[i];
    }
  }
  return nullptr;
}

bool TransformConfig::empty() const
{
  return remapCount_ == 0 && disabledCodeCount_ == 0 && macroCount_ == 0;
}

void LayerConfig::setMomentary(KeySymbol trigger)
{
  trigger_ = trigger;
  enabled_ = trigger != KeySymbol::None;
}

bool LayerConfig::remap(KeySymbol from, KeySymbol to)
{
  return transform_.remap(from, to);
}

void LayerConfig::clear()
{
  enabled_ = false;
  trigger_ = KeySymbol::None;
  transform_.clear();
}

bool LayerConfig::enabled() const
{
  return enabled_;
}

KeySymbol LayerConfig::trigger() const
{
  return trigger_;
}

KeySymbol LayerConfig::map(KeySymbol key) const
{
  return transform_.map(key);
}

bool LayoutConfig::map(KeySymbol from, KeySymbol to)
{
  if (from == KeySymbol::None || to == KeySymbol::None)
  {
    return false;
  }
  for (size_t i = 0; i < mappingCount_; ++i)
  {
    if (mappings_[i].from == keyboardCode(from))
    {
      mappings_[i].to = keyboardCode(to);
      return true;
    }
  }
  if (mappingCount_ >= MaxMappings)
  {
    return false;
  }
  mappings_[mappingCount_++] = {keyboardCode(from), keyboardCode(to)};
  return true;
}

void LayoutConfig::clear()
{
  mappingCount_ = 0;
}

KeySymbol LayoutConfig::convert(KeySymbol key) const
{
  for (size_t i = 0; i < mappingCount_; ++i)
  {
    if (mappings_[i].from == keyboardCode(key))
    {
      return keySymbolFromCode(mappings_[i].to);
    }
  }
  return key;
}

TransformConfig &ESP32KeyBridgeConfig::input(size_t index)
{
  return index < MaxInputConfigs ? inputTransforms_[index] : invalidInputTransform_;
}

const TransformConfig &ESP32KeyBridgeConfig::input(size_t index) const
{
  return index < MaxInputConfigs ? inputTransforms_[index] : invalidInputTransform_;
}

TransformConfig *ESP32KeyBridgeConfig::tryInput(size_t index)
{
  return index < MaxInputConfigs ? &inputTransforms_[index] : nullptr;
}

const TransformConfig *ESP32KeyBridgeConfig::tryInput(size_t index) const
{
  return index < MaxInputConfigs ? &inputTransforms_[index] : nullptr;
}

bool ESP32KeyBridgeConfig::hasInvalidInputConfig() const
{
  return !invalidInputTransform_.empty();
}

void ESP32KeyBridgeConfig::clear()
{
  for (size_t i = 0; i < MaxInputConfigs; ++i)
  {
    inputTransforms_[i].clear();
  }
  invalidInputTransform_.clear();
  global.clear();
  layer.clear();
  layout.clear();
  merge = MergeConfig{};
}

bool ESP32KeyBridge::addInput(InputAdapter &input)
{
  return addInput(input, inputCount_);
}

bool ESP32KeyBridge::addInput(InputAdapter &input, size_t configIndex)
{
  if (inputCount_ >= MaxInputs)
  {
    return false;
  }
  if (configIndex >= ESP32KeyBridgeConfig::MaxInputConfigs)
  {
    return false;
  }
  inputs_[inputCount_++] = &input;
  inputConfigIndexes_[inputCount_ - 1] = configIndex;
  return true;
}

bool ESP32KeyBridge::addOutput(OutputAdapter &output)
{
  if (outputCount_ >= MaxOutputs)
  {
    return false;
  }
  outputs_[outputCount_++] = &output;
  return true;
}

void ESP32KeyBridge::clearInputs()
{
  for (size_t i = 0; i < inputCount_; ++i)
  {
    inputs_[i] = nullptr;
    inputConfigIndexes_[i] = 0;
  }
  inputCount_ = 0;
  mergedState_.clear();
  outputState_.clear();
}

void ESP32KeyBridge::clearOutputs()
{
  for (size_t i = 0; i < outputCount_; ++i)
  {
    outputs_[i] = nullptr;
  }
  outputCount_ = 0;
}

bool ESP32KeyBridge::validateConfig(const ESP32KeyBridgeConfig &config, ESP32KeyBridgeConfigError &error) const
{
  if (config.hasInvalidInputConfig())
  {
    error.message = "input config index out of range";
    return false;
  }
  error.message = nullptr;
  return true;
}

void ESP32KeyBridge::applyConfig(const ESP32KeyBridgeConfig &config)
{
  config_ = config;
}

void ESP32KeyBridge::begin() {}

void ESP32KeyBridge::update()
{
  mergedState_.clear();
  for (size_t i = 0; i < inputCount_; ++i)
  {
    inputs_[i]->update();
    InputState deviceState;
    applyTransform(inputs_[i]->state(), config_.input(inputConfigIndexes_[i]), deviceState);
    mergeInput(deviceState, mergedState_);
  }

  outputState_.clear();
  InputState layered;
  applyLayer(mergedState_, layered);
  InputState layoutConverted;
  applyLayout(layered, layoutConverted);
  applyTransform(layoutConverted, config_.global, outputState_);

  for (size_t i = 0; i < outputCount_; ++i)
  {
    outputs_[i]->write(outputState_);
  }
}

const InputState &ESP32KeyBridge::mergedState() const
{
  return mergedState_;
}

const InputState &ESP32KeyBridge::outputState() const
{
  return outputState_;
}

bool ESP32KeyBridge::shouldMerge(InputCode code) const
{
  if (code.domain == InputDomain::Keyboard)
  {
    const KeySymbol key = keySymbolFromCode(code);
    return isModifierKeySymbol(key) ? config_.merge.shareModifiers : config_.merge.shareKeyboardKeys;
  }

  switch (code.domain)
  {
  case InputDomain::Consumer:
    return config_.merge.shareConsumer;
  case InputDomain::PointerButton:
    return config_.merge.sharePointerButtons;
  case InputDomain::PointerAxis:
    return config_.merge.sharePointerAxes;
  case InputDomain::Vendor:
    return config_.merge.shareVendor;
  case InputDomain::Keyboard:
    break;
  }
  return false;
}

void ESP32KeyBridge::mergeInput(const InputState &input, InputState &merged) const
{
  for (size_t i = 0; i < input.codeCount(); ++i)
  {
    const InputCode code = input.codeAt(i);
    if (shouldMerge(code))
    {
      merged.press(code);
    }
  }
}

void ESP32KeyBridge::applyTransform(const InputState &input, const TransformConfig &transform, InputState &output) const
{
  for (size_t i = 0; i < input.codeCount(); ++i)
  {
    const InputCode code = input.codeAt(i);
    if (transform.isDisabled(code))
    {
      continue;
    }
    if (code.domain == InputDomain::Keyboard)
    {
      const KeySymbol key = keySymbolFromCode(code);
      const KeyMacro *macro = transform.findMacro(key);
      if (macro != nullptr)
      {
        for (size_t j = 0; j < macro->keyCount; ++j)
        {
          output.press(macro->keys[j]);
        }
        continue;
      }
    }
    output.press(transform.map(code));
  }
}

void ESP32KeyBridge::applyLayer(const InputState &input, InputState &output) const
{
  const bool layerActive = config_.layer.enabled() && input.isPressed(config_.layer.trigger());
  for (size_t i = 0; i < input.codeCount(); ++i)
  {
    const InputCode code = input.codeAt(i);
    if (code.domain != InputDomain::Keyboard)
    {
      output.press(code);
      continue;
    }
    const KeySymbol key = input.keyAt(i);
    if (layerActive && key == config_.layer.trigger())
    {
      continue;
    }
    output.press(layerActive ? config_.layer.map(key) : key);
  }
}

void ESP32KeyBridge::applyLayout(const InputState &input, InputState &output) const
{
  for (size_t i = 0; i < input.codeCount(); ++i)
  {
    const InputCode code = input.codeAt(i);
    if (code.domain == InputDomain::Keyboard)
    {
      output.press(config_.layout.convert(input.keyAt(i)));
    }
    else
    {
      output.press(code);
    }
  }
}

} // namespace esp32keybridge
