# Peer Tests

ESP32-S3 2台を使う自動テストです。

`ESP32KeyBridge` では core logic は `tests/unit/` を主戦場にします。peer test は USB adapter の境界を確認する smoke test に限定します。

## ハードウェア

常時接続された ESP32-S3 2台を使います。

- Host board: USB Host adapter を実行する ESP32-S3。
- Device board: USB Device adapter または test source device を実行する ESP32-S3。

USB data pin を直結します。

| Host board | Device board |
|------------|--------------|
| GPIO19 (D-) | GPIO19 (D-) |
| GPIO20 (D+) | GPIO20 (D+) |
| GND | GND |

両方のボードを PC などから別々に給電している場合、VBUS は接続しません。

## 実行

`sketch.yaml` の default profile を使うため、通常は `--profile` を指定しません。

```sh
uv run --env-file .env pytest peer/
```

`.env` では `EspUsbDevice` など既存プロジェクトと同じ変数名を使います。

```sh
TEST_SERIAL_PORT_S3_PEER_HOST=/dev/ttyUSB0
TEST_SERIAL_PORT_PEER_DEVICE_S3_PEER_DEVICE=/dev/ttyUSB1
```

## 追加済み

- `usb_host_keyboard`: Device board が HID keyboard report を送信し、Host board の USB Host adapter が受け取った key event を `esp32keybridge::ESP32KeyBridge` の input pipeline へ流せることを確認する。
- `usb_device_keyboard_output`: Device board 側で `esp32keybridge::ESP32KeyBridge` の output pipeline から USB Device keyboard report を出力し、Host board が USB Host で観測する。

## 初期候補

- `usb_hid_cdc_config`: HID 出力と CDC 設定 reference example の最低限の共存を確認する。

USB-to-USB bridge 全体の完全な end-to-end 自動化は、入力 source、bridge、output observer の 3 役が必要になりやすいため初期必須にしません。2台 peer では adapter 境界を優先します。
