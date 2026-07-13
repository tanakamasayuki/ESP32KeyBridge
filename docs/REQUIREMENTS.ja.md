# 要件

`ESP32KeyBridge` は、ESP32 シリーズを用いて様々な入力デバイスから取得したキーボード系入力を共通形式で処理し、任意の出力デバイスへ変換するための Arduino 向けライブラリです。

## 目的

本ライブラリは、個々のデバイス制御ではなく「入力 adapter」「入力ごとの処理」「入力統合」「全体変換」「出力 adapter」を分離し、ESP32 シリーズで共通に利用できるキーボード入力変換エンジンを提供します。

core は特定の入出力方式、設定形式、保存先、設定 UI に依存しません。USB Host / USB Device は入出力 adapter の一種であり、USB adapter の実装では `EspUsbHost` や `EspUsbDevice` を利用できます。

代表的なユースケースは、USB HID keyboard、GPIO matrix、barcode reader などの入力を受け取り、キーマップ、レイヤー、キー入れ替え、入力統合を適用して USB HID keyboard や BLE HID などへ出力する構成です。WebSerial からの設定変更や USB HID + CDC 複合デバイスは、core ではなくリファレンス実装や examples 側で扱います。

## 対応プラットフォーム

- ESP32
- ESP32-S2
- ESP32-S3
- ESP32-C3
- ESP32-C6
- ESP32-H2
- ESP32-P4
- 将来追加される ESP32 シリーズ

各 SoC が持つ機能に応じて、利用可能な入出力 adapter を選択できるものとします。

## ライブラリの役割

- 入力デバイスからキーイベントを取得する。
- 入力ごとの表現を共通キーイベント(種別 + 値)へ変換する。
- キーコード、修飾キー、レイヤー、マクロなどを変換する。
- 物理 keyboard の刻印 layout と host OS の layout 設定が異なる構成で、記号を含めて刻印どおりに入力できるようにする(レイアウト変換。例: US 刻印 keyboard を日本語設定の host で使う)。
- barcode reader、serial、macro などの文字由来入力を、出力先 layout に応じた打鍵列として送信する。
- Caps Lock / Num Lock などの lock 状態を「正本 = 解釈する側」の原則で扱い、host からの LED report を全入力デバイスへ転送する。正本不在時はブリッジが終端ホストとして管理する。
- マウス(ボタン + 相対移動・ホイール)を入力・変換・出力する。
- 変換済みイベントを出力デバイスへ送信する。
- C++ コードで構築した設定、または外部で読み込んだ設定 object を適用し、キーマップやレイヤーを切り替える。

ハードウェア依存部分は adapter 層へ分離します。

## 中間データモデル

キー・ボタンの同一性は「種別 + 値」で表し(keyboard / consumer / mouse_button / virtual)、HID 由来の種別は Usage ID と数値恒等にします。トラフィックは「継続(状態系: 押下集合)」と「単発(イベント系: 文字・キー操作指示・相対値)」の 2 大分類で扱います。パススルーとリマップは変換テーブル不要で、レイアウト変換と文字列打鍵だけが変換表を必要とします。詳細は [DATA_MODEL.ja.md](DATA_MODEL.ja.md) を参照してください。

## 入力インターフェース

GPIO:

- 単体スイッチ
- 行列キーボード
- キーパッド
- ロータリーエンコーダ
- フットスイッチ
- 抵抗ラダー式キー

USB Host:

- USB HID Keyboard
- USB Numeric Keypad
- USB Barcode Scanner
- USB Consumer Control
- USB Gamepad

Bluetooth:

- BLE Keyboard
- BLE Consumer Device
- BLE Remote
- Bluetooth Classic HID Keyboard
- Bluetooth Classic HID Consumer Device

UART / Serial:

- 外部マイコンからのキーイベント
- 独自プロトコル
- デバッグ入力

I2C:

- GPIO Expander
- キーパッドユニット
- タッチ入力デバイス

SPI:

- GPIO Expander
- キーボードコントローラ

ネットワーク:

- ESP-NOW
- TCP
- UDP
- WebSocket

赤外線:

- IR Remote
- 学習リモコン
- Consumer Control

## 出力インターフェース

USB Device:

- HID Keyboard
- Consumer Control
- Mouse
- CDC Serial for reference configuration examples

Bluetooth:

- BLE HID Keyboard
- BLE HID Consumer Control
- Bluetooth Classic HID Keyboard

UART:

- シリアルイベント出力
- デバッグ出力

ネットワーク:

- ESP-NOW
- TCP
- UDP
- MQTT

GPIO:

- LED
- ブザー
- リレー
- 外部マイコン制御

## キー変換機能

- キーコード変換(1 段 remap、種別またぎ可)
- キー入れ替え、キー無効化
- モーメンタリレイヤー(virtual キートリガ、press 時確定)
- マクロ実行(キーシーケンス macro、文字列タイプ macro)
- キーボードのレイアウト変換((キー, Shift) 変換表 + Shift 抑制・合成、on/off 切替付き)
- ホストレイアウト記述(`en_us` / `ja_jp` を最初に、基本的な言語を同梱)とユーザー定義表
- lock 状態管理(正本 = アクティブな lock 報告出力、正本不在時は終端ホストモード、全入力へ LED 転送)
- Consumer Key 変換
- マウスボタン変換、相対軸の反転・倍率
- 独自キー定義(virtual 種別)

ワンショット・トグル・tap/hold などの時間依存・状態合成系は、アダプタ側の論理押下合成で実現するか、将来検討とします([DECISIONS.ja.md](DECISIONS.ja.md) の将来メモ)。

## 設定方針

キーマップやレイヤーは、スケッチ内で決め打ちする使い方と、外部から読み込んだ設定 object として適用する使い方の両方を許容します。

core は設定の取得方法、転送手段、保存先、ファイル形式、UI を規定しません。WebSerial、CDC serial、NVS、LittleFS などは、リファレンス実装や examples 側の選択肢として扱います。

設定対象:

- キーマップ
- レイヤー
- マクロ
- Bluetooth 情報
- USB VID / PID / product / manufacturer / serial などの USB 情報
- デバイス名
- 入力 adapter / 出力 adapter の有効化
- 永続設定

リファレンス実装で扱う可能性がある設定転送手段:

- USB CDC + WebSerial
- UART
- BLE
- TCP
- 独自プロトコル

USB HID + CDC 複合デバイスの CDC serial で WebSerial 設定画面を提供する構成は、有力なリファレンス実装です。PC からは HID keyboard として見えつつ、ブラウザの WebSerial 画面または serial CLI から設定を読み書きできます。

## 永続化

core は永続化を必須にしません。必要なアプリケーションや example が、任意の保存先へ設定を保存できるものとします。

保存先候補:

- NVS
- LittleFS
- SPIFFS
- SD card

リファレンス実装では、小さい設定は NVS、容量の大きいキーマップや macro 定義は LittleFS / SPIFFS / SD card へ逃がせる設計を検討します。

## 非機能要件

- Arduino から利用可能にする。
- MIT ライセンスで提供する。
- SoC 依存部を最小化する。
- core は移植可能な純粋 C++ とし、Arduino API・ESP-IDF・FreeRTOS などプラットフォーム固有機能に依存しない。プラットフォーム依存は adapter 層(実質の HAL)に閉じ込める。
- core は時刻取得・動的メモリ確保をせず、host PC 上でコンパイル・テストできる。
- 入力 adapter、入力ごとの処理、入力統合、全体変換、出力 adapter を独立したモジュールとして構成する。
- 複数入力の同時利用を可能にする。
- 複数出力の同時利用を可能にする。
- core の依存を小さくし、複雑な利用例や設定 UI は examples / adapter 側で拡張できる設計にする。

## 想定利用例

- USB keyboard から USB keyboard への変換。
- USB keyboard から BLE keyboard への変換。
- BLE keyboard から USB keyboard への変換。
- GPIO key matrix から USB keyboard への変換。
- GPIO key matrix から BLE keyboard への変換。
- USB keyboard から ESP-NOW への変換。
- ESP-NOW から USB keyboard への変換。
- Serial 入力から USB keyboard への変換。
- IR remote から Consumer Control への変換。
- USB barcode reader から serial 出力への変換。
- US 刻印 keyboard を日本語設定の Windows へ接続し、記号を含めて刻印どおりに入力する(レイアウト変換)。
- serial で受け取った文字列を、host の layout 設定に合わせた打鍵として送信する。
- 複数の入力デバイスと USB keyboard + mouse 複合デバイス出力を組み合わせる。
- WebSerial 設定画面で keymap を変更し、再ビルドなしで HID 出力へ反映する。
- USB hub 経由の複数 keyboard を統合し、片方の Shift / Ctrl をもう片方の keyboard 入力にも効かせる。

## 位置付け

本ライブラリは QMK のようなキーボードファームウェアそのものを目指すものではありません。ESP32 シリーズ上で動作する汎用的な「キーボード入力、統合、変換、出力エンジン」として位置付けます。

主対象は keyboard / consumer control などの HID 系入力ですが、将来的に mouse、trackpad、pointer、wheel などの入力・出力も event domain の拡張として扱える余地を残します。各種入力デバイス、通信方式、出力方式を自由に組み合わせられることを優先し、SoC ごとの機能差は各 adapter 実装で吸収します。
