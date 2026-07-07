# ESP32KeyBridge

ESP32KeyBridge is an early-stage Arduino library for bridging keyboard-oriented input across ESP32 devices. It receives events from input adapters, normalizes them into common state, applies transformations, merges multiple inputs when needed, and sends the result to one or more output adapters.

The design is documented in [docs/REQUIREMENTS.ja.md](docs/REQUIREMENTS.ja.md). The repository is structured so implementation, documentation, tests, and release checks can follow the same project management style as the related ESP32 Arduino libraries.

## Scope

The library is intended to separate:

- input adapters, such as GPIO, USB Host, Bluetooth, UART, I2C, SPI, network, and IR;
- transformation logic, such as keycode mapping, layers, macros, disabled keys, swapped keys, and Consumer Key handling;
- output adapters, such as USB Device, Bluetooth, UART, network, and GPIO.

The core does not depend on a specific transport, storage backend, configuration format, or UI. USB adapters may use `EspUsbHost` and `EspUsbDevice`, and WebSerial configuration is planned as a reference example rather than a core requirement.

Initial development should start with small core APIs that can be tested without hardware, then add hardware-backed adapters incrementally.

Core design and API sketches are in [docs/CORE_DESIGN.ja.md](docs/CORE_DESIGN.ja.md) and [docs/API_SKETCHES.ja.md](docs/API_SKETCHES.ja.md). Configuration design notes are in [docs/CONFIGURATION.ja.md](docs/CONFIGURATION.ja.md).

## Tests

Tests are organized under `tests/` with the same style as the related Arduino library projects.

```sh
cd tests
uv run pytest
```

See [tests/README.ja.md](tests/README.ja.md) and [tests/TEST_PLAN.ja.md](tests/TEST_PLAN.ja.md).

## Release

Release automation and GitHub Actions are shared across projects. This repository should not rely on copied, per-project customized workflows.

See [docs/RELEASE_CHECKLIST.ja.md](docs/RELEASE_CHECKLIST.ja.md).
