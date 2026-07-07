# Examples

Examples for `ESP32KeyBridge`.

Early examples use virtual inputs and serial output to demonstrate the core API without depending on USB, BLE, GPIO, storage, or WebSerial.

- `Basic`: minimal `begin()` / `update()` sketch.
- `HardcodedRemap`: hardcoded C++ remap / disable configuration.
- `MultiKeyboardMerge`: merge multiple virtual keyboard inputs into one keyboard state.
- `PerInputRemap`: apply a remap to only one input, then apply a global remap after merge.

Planned examples include USB keyboard bridge, USB remap, WebSerial configuration, and GPIO matrix input.
