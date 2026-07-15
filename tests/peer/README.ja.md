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

- `usb_host_keyboard`: Device board(EspUsbDevice の keyboard **単体** = boot 宣言 interface)が HID keyboard report を送り、Host board のブリッジ入力パイプラインを検証する。内容: remap(A→B)、修飾キー単独 press/release、終端ホストの CapsLock トグル → キーボード LED 転送。
- `usb_host_consumer`: Device board(consumer control 単体)の press/release がキーボード入力アダプタ経由で押下集合に統合されることを検証する。
- `usb_device_keyboard_output`: Device board 側でブリッジ出力パイプライン(ManualInput → 複合 HID 出力)から keyboard / consumer / mouse report を出力し、Host board が素の EspUsbHost で観測する。LED → lock 正本のテストは **skip 中**(下記の既知の制限)。

## 既知の制限(上流依頼中)

EspUsbHost 2.2.0 の `setKeyboardLeds()` は **boot 宣言された keyboard interface にしか送れない**。EspUsbDevice の複合 HID は 1 interface に report ID でマージされ boot 宣言を持たないため、テストホストから複合デバイスへ LED を送れない(入力イベントは report descriptor 解析で届くので非対称)。実世界では report-ID/NKRO 専用キーボードへの LED 転送も同じ制限に当たる。修正依頼済み(report ID 付き SET_REPORT 対応)。実 PC は report ID を扱えるため、複合出力アダプタの lock 正本動作は OS での手動確認でカバーする。

## 初期候補

- `usb_hid_cdc_config`: HID 出力と CDC 設定 reference example の最低限の共存を確認する。

USB-to-USB bridge 全体の完全な end-to-end 自動化は、入力 source、bridge、output observer の 3 役が必要になりやすいため初期必須にしません。2台 peer では adapter 境界を優先します。
