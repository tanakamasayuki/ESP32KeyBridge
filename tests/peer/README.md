# Peer Tests

`usb_host_keyboard` is the first peer smoke test. The device board exposes an HID keyboard with `EspUsbDevice`, sends `A`, and the host board receives it with `EspUsbHost` before feeding it into the `esp32keybridge::ESP32KeyBridge` input pipeline.

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
