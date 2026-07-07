# Examples

Examples for `ESP32KeyBridge`.

Early examples use virtual inputs and serial output to demonstrate the core API without depending on USB, BLE, GPIO, storage, or WebSerial.

- `Basic`: minimal `begin()` / `update()` sketch.
- `HardcodedRemap`: hardcoded C++ remap / disable configuration.
- `MultiKeyboardMerge`: merge multiple virtual keyboard inputs into one keyboard state.

Planned examples include USB keyboard bridge, USB remap, WebSerial configuration, and GPIO matrix input.

