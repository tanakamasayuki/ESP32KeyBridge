# Examples

Every example is a **practical** one: a sketch for a real hardware setup, ready to flash as-is. They all share the same structure: 1) start the hardware, 2) wire the bridge, 3) build and apply the configuration.

> **Note**: the hardware adapters are being implemented in step 7. See [../docs/ADAPTERS.ja.md](../docs/ADAPTERS.ja.md) for the current status (the USB Device composite HID output, the GPIO input, and the USB Host mouse input are implemented; the rest are build-only mocks).

## Practical Examples

- `BleToUsb`: use a BLE keyboard as a wired USB keyboard (PCs without Bluetooth, BIOS, KVM). ESP32-S3.
- `FootSwitch`: type canned text, control media, or turn pages with foot switches or homemade buttons. ESP32-S3 + GPIO.
- `MediaKeys`: turn unused keys (F13...) into volume and play/pause keys. ESP32-P4.
- `NaturalScroll`: invert the mouse wheel direction without touching the PC's settings (swapping buttons for left-handed use, too). ESP32-P4.
- `SerialTextTyper`: type text received over serial into the PC (test automation, kiosks). ESP32-S3.
- `SwapCtrlCapsLock`: sit between a keyboard and a PC and swap CapsLock and Ctrl without changing either. ESP32-P4. **The P4 USB port details live in this example's README.**
- `UsKeyboardOnJapanesePc`: type as engraved on a US keyboard or barcode reader connected to a PC that stays on the ja_jp layout. ESP32-P4.

## Connecting your own inputs and outputs

Start from these instead of an example:

- Minimal reference implementations: `ManualInputAdapter` / `ManualOutputAdapter` (press()/release() calls become an input; writes and text are recorded).
- Adapter responsibilities and API: the header comments of `InputAdapter` / `OutputAdapter` and the adapter headers (`ESP32KeyBridgeEspUsbHost.h`, ...).
- Report packing: the `buildHidKeyboardReport()` family of builders.

## Candidates

- Multi-keyboard merge (MergeKeyboards): waiting on the design decision between one adapter merging all keyboards and per-keyboard adapters.
- BLE mouse to USB (waiting on the BLE library choice).
