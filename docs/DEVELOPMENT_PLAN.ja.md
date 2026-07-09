# 開発計画

この文書は、`ESP32KeyBridge` の現在の方針と次に実装する範囲をまとめます。時系列ログではなく、リリース判断や次作業に必要な状態だけを残します。

## 基本方針

`ESP32KeyBridge` は、個別デバイス制御を直接束ねるライブラリではなく、入力 adapter、入力ごとの処理、入力統合、全体変換、出力 adapter を分離した汎用キーボード変換ライブラリとして実装します。

中心に置くのは、入力ごとの状態と統合後の状態です。GPIO matrix、USB HID keyboard、BLE keyboard、UART などの入力 adapter は共通 event / state を生成し、per-input pipeline が入力ごとの処理を行い、merge layer が複数入力を統合し、global pipeline が keymap、layer、macro などを処理します。出力 adapter は変換済み state / action を USB HID、BLE HID、UART、ネットワークなどへ送信します。

core は特定の transport、保存先、設定 UI に依存しません。USB Host / USB Device adapter では `EspUsbHost` / `EspUsbDevice` を利用できますが、依存は adapter 側に閉じ込めます。USB HID + CDC 複合デバイスや WebSerial 設定画面のような複雑な構成は、まず examples 側のリファレンス実装として検討します。

## 現在のマイルストーン

最初の core MVP は、実ハードウェア依存を増やす前に host unit test と build-only example で固定します。

完了済み:

- [API_SKETCHES.ja.md](API_SKETCHES.ja.md) でユーザー側 API の初期案を整理する。
- [CORE_DESIGN.ja.md](CORE_DESIGN.ja.md) で data flow、state、adapter、configuration boundary を整理する。
- `esp32keybridge::InputCode` / `esp32keybridge::InputState` と press / release の基本表現を実装する。
- `esp32keybridge::InputValueEvent` で pointer axis のような値付き入力を表現する。
- input adapter / output adapter の最小 interface を実装する。
- virtual input / recording output を使った unit test と example で merge、remap、disable を固定する。
- hardcoded config と外部設定 object の両方で使える設定適用 API を実装する。
- per-input remap、明示 config index、layer、macro、layout conversion を core test で固定する。
- keyboard / consumer / pointer の最小 HID report builder を実装する。

次に検討するもの:

- USB HID keyboard の Host input adapter example。
- USB HID keyboard / consumer / pointer の Device output adapter example。
- 32 code までの rollover keyboard report を扱う Host / Device adapter example。
- GPIO matrix input adapter example。
- WebSerial 設定画面 reference example。
- pointer axis の event queue / output adapter 連携方式。

## 対応予定の入力

- GPIO: 単体スイッチ、行列キーボード、キーパッド、ロータリーエンコーダ、フットスイッチ、抵抗ラダー式キー。
- USB Host: HID keyboard、numeric keypad、barcode scanner、consumer control、gamepad。
- Bluetooth: BLE HID keyboard / consumer device / remote、対応 SoC で Bluetooth Classic HID。
- UART / Serial: 外部マイコンからのキーイベント、独自プロトコル、デバッグ入力。
- I2C / SPI: GPIO expander、キーパッドユニット、キーボードコントローラ。
- Network: ESP-NOW、TCP、UDP、WebSocket。
- IR: remote、学習リモコン、consumer control。

## 対応予定の出力

- USB Device: HID keyboard、consumer control、mouse。
- Bluetooth: BLE HID keyboard / consumer control、Bluetooth Classic HID keyboard。
- UART: シリアルイベント出力、デバッグ。
- Network: ESP-NOW、TCP、UDP、MQTT。
- GPIO: LED、ブザー、リレー、外部マイコン制御。

## 設計判断

- 入力元の scan / receive 処理と、キー変換処理を混ぜない。
- 入力ごとの処理、入力統合、統合後の全体処理を分ける。
- 出力 adapter は複数同時利用できる前提で設計する。
- core の `esp32keybridge::InputState` は 6KRO に限定しない。ただし full NKRO ではなく `esp32keybridge::InputState::MaxCodes` 個までで、現状は 32 code を上限にする。6KRO boot report は output adapter 向けの変換結果であり、32 code までの rollover report は adapter 固有 report として追加できるようにする。
- キーマップ、レイヤー、マクロは C++ コードで決め打ちする使い方と、外部設定を適用する使い方の両方を許容する。
- core は設定転送手段と永続化先を固定しない。NVS、LittleFS、SPIFFS、SD card、USB CDC、BLE、UART、TCP などは examples / adapter 側の選択肢として扱う。
- SoC ごとの USB / Bluetooth / peripheral 差は adapter 層に閉じ込める。
- keyboard / consumer control を主対象にしつつ、mouse、trackpad、pointer 系の event domain を追加しても pipeline が破綻しない設計にする。pointer button は state として扱い、pointer axis は値付き event として扱う。

設定と WebSerial の詳細は [CONFIGURATION.ja.md](CONFIGURATION.ja.md) を参照してください。

## テスト方針

core の変換処理は host 上の unit test で固定します。ハードウェア adapter は、build-only smoke test、実機自動テスト、manual test を分けて管理します。

詳細は [../tests/TEST_PLAN.ja.md](../tests/TEST_PLAN.ja.md) を参照してください。

## リリース運用

リリース、バージョン更新、GitHub Actions は全プロジェクト共通の運用に従います。このリポジトリ内に共通 Action をコピーして個別カスタマイズする方針は取りません。
