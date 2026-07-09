# テスト計画

## テスト方針

`ESP32KeyBridge` は入力、変換、出力を分離するライブラリです。テストも同じ分け方にします。

**unit test** は、共通 event / state と変換処理をホスト上で検証します。キー変換、入力統合、layer、macro、設定 object の適用など、ハードウェアが不要な仕様はここで固定します。

**build-only test** は、Arduino examples が対象ボード向けにコンパイルできることを確認します。

**single-board test** は、ESP32-S3 1台へ upload し、Arduino runtime 上で core や reference example が動くことを serial assertion で確認します。manual / single 用 S3 は必要なときだけ接続する前提です。

**peer test** は、常時接続された ESP32-S3 2台を使い、USB adapter の境界だけを確認します。片方を USB Host 役、もう片方を USB Device 役として直結します。core の merge / remap / layer は peer ではなく unit test を主戦場にします。

**manual test** は、配線、Host OS の認識、Bluetooth pairing、目視や実操作が検証の本質に含まれる場合だけに使います。

```text
tests/
  unit/              自動 - 共通キーイベントと変換処理。
  examples_compile/  自動 - examples sketch の build-only smoke。
  single/            自動 - ESP32-S3 1台の runtime smoke。
  peer/              自動 - ESP32-S3 2台の USB adapter smoke。
  manual/            手動 - OS 認識、pairing、配線、目視確認。
```

## テスト環境

通常の自動テスト環境は、ローカル PC と常時接続された ESP32-S3 peer 2台を前提にします。

- Local: core unit test と examples compile。
- S3 single: 必要時に接続する ESP32-S3 1台。core runtime smoke と reference example smoke。default profile で実行し、`.env` では `TEST_SERIAL_PORT_S3_SINGLE` を使う。
- S3 peer: 常時接続された ESP32-S3 2台。USB data pin を直結し、USB Host adapter / USB Device adapter の境界を確認する。default profile で実行し、`.env` では `TEST_SERIAL_PORT_S3_PEER_HOST` と `TEST_SERIAL_PORT_PEER_DEVICE_S3_PEER_DEVICE` を使う。
- Manual S3: GPIO matrix、実 USB keyboard、barcode reader、WebSerial UI など人の操作が必要な確認に使う。`.env` では既存プロジェクトと同じ `TEST_SERIAL_PORT_ESP32S3` を使う。

ESP32-P4 は常時接続の自動テスト環境には含めません。P4 は USB Host / Device bridge の example で登場する可能性がありますが、loopback 自動テストの前提にはしません。

## 初期カバレッジ計画

| 機能 | unit | examples_compile | single | peer | manual |
|------|------|------------------|--------|------|--------|
| 共通キーイベント | 実装済み | | | | |
| 入力ごとの state | 実装済み | | | | |
| 6KRO を超える 32 code までの state | 実装済み | | 実装済み | | |
| 複数入力 merge | 実装済み | | | | |
| input / processor / output interface | 実装済み | | | | |
| キーコード変換 | 実装済み | | | | |
| キー無効化 / 入れ替え | 実装済み | | 実装済み | | |
| layer 基本動作 | 実装済み | 実装済み | | | |
| macro 基本動作 | 実装済み | 実装済み | | | |
| layout conversion | 実装済み | 実装済み | | | |
| HID keyboard boot report | 実装済み | 実装済み | 実装済み | | |
| HID keyboard 32 code rollover report | 実装済み | 実装済み | 実装済み | | |
| HID consumer / pointer report | 実装済み | 実装済み | | | |
| 設定 object の適用 | 実装済み | 実装済み | 実装済み | | |
| Arduino runtime core smoke | | | 実装済み | | |
| hardcoded config example | | 実装済み | | | |
| 設定の保存 / 読み込み reference example | | 予定 | 予定 | | storage 差分確認 |
| CDC serial 設定 reference example | | 予定 | 予定 | | WebSerial 確認 |
| USB Host keyboard input adapter | 予定 | 予定 | | 予定 | 実 keyboard 確認 |
| USB Device keyboard output adapter | 予定 | 予定 | | 予定 | Host OS 認識 |
| USB HID + CDC 複合デバイス example | | 予定 | | 予定 | Host OS / WebSerial 確認 |
| GPIO matrix input | 予定 | 予定 | | | 配線確認 |
| USB hub 複数 keyboard 統合 | 予定 | 予定 | | 任意 | 実 hub / keyboard 確認 |
| BLE HID output | 予定 | 予定 | | | pairing 確認 |

## Peer テストの位置付け

peer テストは、USB adapter が実 USB 経由で期待どおり event / state を受け渡しできることを確認する smoke test に限定します。`ESP32KeyBridge` core の正しさは peer ではなく unit test で固定します。

初期候補:

- `usb_host_keyboard`: Device 役 S3 が HID keyboard report を送信し、Host 役 S3 の USB Host adapter が event / state として受け取る。
- `usb_device_keyboard`: Bridge 役 S3 が USB Device keyboard report を出力し、Host 役 S3 が USB Host で観測する。
- `usb_hid_cdc_config`: HID 出力と CDC 設定 reference example の最低限の共存を確認する。複雑な WebSerial UI 操作は manual に残す。

USB-to-USB bridge 全体の完全な end-to-end 自動化は、入力 source、bridge、output observer の 3 役が必要になりやすいため初期必須にしません。2台 peer では adapter 境界を優先します。

## 追加時の判断基準

- 変換仕様は、できるだけ `unit/` で先に固定する。
- adapter は、まず build-only test を追加して API の破壊を検出する。
- S3 single / peer の実機自動テストは、serial log だけで期待値を assert できる範囲に限定する。
- 人の判断が必要な確認は `manual/` に隔離し、自動テストの合格条件に混ぜない。
