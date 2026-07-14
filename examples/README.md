# Examples

Every example is a **practical** one: a sketch for a real hardware setup, ready to flash as-is. They all share the same structure: 1) start the hardware, 2) wire the bridge, 3) build and apply the configuration.

> **Note**: the adapters used by NumPad (matrix) and VolumeKnob (encoder) are build-only mocks while their API settles; every other example runs on implemented adapters (hardware verification in progress). See [../docs/ADAPTERS.ja.md](../docs/ADAPTERS.ja.md) for the roster.

## Practical Examples

- `FootSwitch`: type canned text, control media, or turn pages with foot switches or homemade buttons. ESP32-S3 + GPIO.
- `MediaKeys`: turn unused keys (F13...) into volume and play/pause keys. ESP32-P4.
- `NumPad`: build a numeric keypad, macro pad, or custom keyboard from a GPIO-wired switch matrix. ESP32-S3 + GPIO.
- `NaturalScroll`: invert the mouse wheel direction without touching the PC's settings (swapping buttons for left-handed use, too). ESP32-P4.
- `SerialTextTyper`: type text received over serial into the PC (test automation, kiosks). ESP32-S3.
- `SwapCtrlCapsLock`: sit between a keyboard and a PC and swap CapsLock and Ctrl without changing either. ESP32-P4. **The P4 USB port details live in this example's README.**
- `UsKeyboardOnJapanesePc`: type as engraved on a US keyboard or barcode reader connected to a PC that stays on the ja_jp layout. ESP32-P4.
- `VolumeKnob`: a physical volume knob on your desk (rotary encoder; works as a scroll dial too). ESP32-S3 + GPIO.

## Connecting your own inputs and outputs

Start from these instead of an example:

- Minimal reference implementations: `ManualInputAdapter` / `ManualOutputAdapter` (press()/release() calls become an input; writes and text are recorded).
- Adapter responsibilities and API: the header comments of `InputAdapter` / `OutputAdapter` and the adapter headers (`ESP32KeyBridgeEspUsbHost.h`, ...).
- Report packing: the `buildHidKeyboardReport()` family of builders.

## Candidates

- Multi-keyboard merge (MergeKeyboards): waiting on the design decision between one adapter merging all keyboards and per-keyboard adapters.
- BLE-to-USB (BleToUsb, BLE mouse): BLE will live in a dedicated library (both central and peripheral roles, NimBLE-based, no Bluetooth Classic); this library stays BLE-free until it exists.
