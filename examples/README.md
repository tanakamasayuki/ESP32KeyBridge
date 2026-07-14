# Examples

The examples are explicitly split into practical examples and usage examples.

- `practical/`: practical sketches for real hardware setups, ready to flash as-is. They contain nothing but the intended configuration plus `update()` — no Serial dumps, no injected presses.
- `usage/`: API demos. They inject presses through `ManualInputAdapter` / `ManualOutputAdapter` and print the core's behavior (transformed keys, report bytes) over Serial.

## Practical Examples

Added once the hardware adapters of implementation step 7 (USB Host / USB Device, ...) land. The first candidate is the practical SwapCtrlCapsLock (USB Host keyboard input + USB Device keyboard output + two remap entries, nothing else).

## Usage Examples

- `FootSwitchLayer`: remap a foot switch to a virtual key and use it as a momentary layer turning J/K into Down/Up (UC7).
- `LayoutConversion`: type as engraved on a US keyboard connected to a ja_jp host (Shift consumption/synthesis, shortcut passthrough, on/off toggle) (UC5).
- `MediaKeyRemap`: kind-crossing remap turning F13/F14 into Volume Up / Play-Pause, and the consumer report bytes (UC9).
- `MergeKeyboards`: union merge of multiple inputs — Shift on one half applies to the other, and disconnect releases everything (UC2).
- `SwapCtrlCapsLock`: swap Ctrl and CapsLock with two global remap entries (UC1).
- `TextMacroTyping`: text macros and `typeText()` expanded into keystrokes, one phase per update (UC10).

A usage example for relative mouse axes (UC11) will follow.
