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

- `usb_host_keyboard`: Device board(EspUsbDevice の keyboard **単体** = boot 宣言 interface)が HID keyboard report を送り、Host board のブリッジ入力パイプラインを検証する。内容: remap(A→B)、修飾キー単独 press/release、**多キー同時押し(A+C+E をリマップ込みで)**、**キー+修飾同時(LeftShift+A)**、終端ホストの CapsLock トグル → キーボード LED 転送、**デバイス切断でホールド中のキーが解放される**こと。
- `usb_host_consumer`: Device board(consumer control 単体)の press/release がキーボード入力アダプタ経由で押下集合に統合されることを検証する。
- `usb_host_mouse`: Device board(USB マウス)が boot mouse report を送り、Host board の `EspUsbHostMouseInputAdapter` が処理することを検証する。内容: X/Y 相対移動、ホイール、左/右ボタン(押下集合に MouseButton キーとして到達)。
- `usb_device_keyboard_output`: Device board 側でブリッジ出力パイプライン(ManualInput → 複合 HID 出力)から keyboard / consumer / mouse report を出力し、Host board が素の EspUsbHost で観測する。内容: 単キー、修飾単独、**6キーロールオーバー(A〜F)**、consumer usage、マウスボタン+ホイール、**マウス X/Y 相対移動**、**複合同時(キーボード+consumer+マウスを 1 フレームで出力し KEY_STATE→CONSUMER→MOUSE の順で観測)**、**LED → lock 正本(ホストが LED output report を送り、複合出力アダプタが lock 正本になって入力へ伝播)**。

> **補足**: LED → lock 正本テストは EspUsbHost 2.2.0 では skip していた(`setKeyboardLeds()` が boot 宣言 interface にしか届かず、複合 HID は 1 interface に report ID でマージされ boot 宣言を持たないため)。**2.3.0 で report descriptor の LED output report を検出し report ID 付き SET_REPORT を送れるようになり解消**、有効化した。

## 追加候補

- `usb_hid_cdc_config`: HID 出力と CDC 設定 reference example の最低限の共存を確認する。

USB-to-USB bridge 全体の完全な end-to-end 自動化は、入力 source、bridge、output observer の 3 役が必要になりやすいため初期必須にしません。2台 peer では adapter 境界を優先します。
