# Peer Tests

`usb_host_keyboard`: the device board exposes a boot-declared HID keyboard with `EspUsbDevice` and the host board's bridge input pipeline is verified end to end — remap (A→B), modifier-only press/release, multi-key rollover (A+C+E, with the remap applied inside the chord), key-with-modifier (LeftShift+A), the terminal-host CapsLock toggle forwarded back to the keyboard LEDs, and a held key released when the device drops off the bus.

`usb_host_consumer`: consumer control press/release from a consumer-only device merges into the bridge key set through the keyboard input adapter.

`usb_host_mouse`: the device board acts as a USB mouse and the host board's `EspUsbHostMouseInputAdapter` is verified — relative X/Y movement, wheel, and left/right buttons (which arrive as MouseButton keys in the output set).

`usb_device_keyboard_output` checks the opposite adapter boundary: the peer device board drives the bridge output pipeline (manual input → composite HID output) and the host DUT observes the reports with `EspUsbHost` — single key, modifier-only, six-key rollover (A–F), consumer usage, mouse button + wheel, relative mouse X/Y movement, a composite frame where keyboard, consumer, and mouse reports are emitted together (observed in KEY_STATE→CONSUMER→MOUSE order), and the LED-to-lock path (the host sends an LED output report, the composite output adapter becomes the lock authority and propagates it to the input). The LED-to-lock test was skipped on EspUsbHost 2.2.0 (`setKeyboardLeds()` reached only boot-declared keyboard interfaces, and the composite device merges its HID classes into one report-ID interface); 2.3.0 detects the LED output report in the report descriptor and sends a report-ID Set_Report, so it is now enabled.

Two-board ESP32-S3 hardware tests.

Core logic is covered by `tests/unit/`. Peer tests are reserved for USB adapter boundary smoke tests.

Run with the default profile:

```sh
uv run --env-file .env pytest peer/
```

Serial port variables follow the existing project convention:

```sh
TEST_SERIAL_PORT_S3_PEER_HOST=/dev/ttyUSB0
TEST_SERIAL_PORT_PEER_DEVICE_S3_PEER_DEVICE=/dev/ttyUSB1
```
