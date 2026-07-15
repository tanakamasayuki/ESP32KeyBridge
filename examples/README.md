# Examples

Every example is a **practical** one: a sketch for a real hardware setup, ready to flash as-is. They all share the same structure: 1) start the hardware, 2) wire the bridge, 3) build and apply the configuration.

> **Note**: every adapter used by the examples is implemented (hardware verification in progress). See [../docs/ADAPTERS.ja.md](../docs/ADAPTERS.ja.md) for the roster.

## Practical Examples

- `BarcodeToText`: turn a USB HID barcode reader into a stream of text on the serial port. ESP32-S3 + USB Host.
- `FootSwitch`: type canned text, control media, or turn pages with foot switches or homemade buttons. ESP32-S3 + GPIO.
- `GamepadToKeys`: control the PC as a keyboard with a USB gamepad (D-pad = arrows, buttons = Space/Enter/...). ESP32-P4 + USB Host/Device.
- `KeyMonitor`: print the bridge output to the serial console to bring up or monitor input hardware. ESP32-S3 + GPIO.
- `LayerKeypad`: build a keypad where every key has two functions — hold Fn to switch the pad from navigation to media keys. ESP32-S3 + GPIO.
- `MediaKeys`: turn unused keys (F13...) into volume and play/pause keys. ESP32-P4.
- `MergeKeyboards`: combine several inputs (a USB keyboard plus extra GPIO buttons) into one keyboard the PC sees. ESP32-P4 + USB Host/Device + GPIO.
- `MidiToKeys`: turn a USB MIDI controller's pads/keys into keyboard shortcuts. ESP32-P4 + USB Host/Device.
- `NavLayer`: give any USB keyboard a hold-to-navigate (Fn) layer — hold CapsLock and H/J/K/L become the arrow keys. ESP32-P4 + USB Host/Device.
- `NumPad`: build a numeric keypad, macro pad, or custom keyboard from a GPIO-wired switch matrix. ESP32-S3 + GPIO.
- `NaturalScroll`: invert the mouse wheel direction without touching the PC's settings (swapping buttons for left-handed use, too). ESP32-P4.
- `ScrollDial`: a physical scroll dial on your desk (turn to scroll the wheel, press for a middle click; also works as a jog dial). ESP32-S3 + GPIO (rotary encoder).
- `SerialTextTyper`: type text received over serial into the PC (test automation, kiosks). ESP32-S3.
- `SwapCtrlCapsLock`: sit between a keyboard and a PC and swap CapsLock and Ctrl without changing either. ESP32-P4. **The P4 USB port details live in this example's README.**
- `TouchButtons`: learn how to write your own input adapter (worked example: the ESP32-S3's built-in capacitive touch pads as buttons). ESP32-S3.
- `UsKeyboardOnJapanesePc`: type as engraved on a US keyboard or barcode reader connected to a PC that stays on the ja_jp layout. ESP32-P4.
- `VolumeKnob`: a physical volume knob on your desk (rotary encoder; works as a scroll dial too). ESP32-S3 + GPIO.

## Connecting your own inputs and outputs

Start here:

- **A worked custom input adapter: `TouchButtons`** (subclass `InputAdapter`, read hardware in `update()`, return the pressed keys from `keys()` — the minimal template; an I2C expander, a sensor, or a network source follows the same shape).
- Minimal reference implementations: `ManualInputAdapter` / `ManualOutputAdapter` (press()/release() calls become an input; writes and text are recorded).
- Adapter responsibilities and API: the header comments of `InputAdapter` / `OutputAdapter` and the adapter headers (`ESP32KeyBridgeEspUsbHost.h`, ...).
- Report packing: the `buildHidKeyboardReport()` family of builders.

## Candidates

- BLE-to-USB (BleToUsb, BLE mouse): BLE will live in a dedicated library (both central and peripheral roles, NimBLE-based, no Bluetooth Classic); this library stays BLE-free until it exists.
