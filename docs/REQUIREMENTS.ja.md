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
- 入力ごとの表現を共通キーイベントへ変換する。
- キーコード、修飾キー、レイヤー、マクロなどを変換する。
- 変換済みイベントを出力デバイスへ送信する。
- C++ コードで構築した設定、または外部で読み込んだ設定 object を適用し、キーマップやレイヤーを切り替える。

ハードウェア依存部分は adapter 層へ分離します。

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

- キーコード変換
- 修飾キー変換
- レイヤー切替
- ワンショットレイヤー
- モーメンタリレイヤー
- トグルレイヤー
- キー無効化
- キー入れ替え
- マクロ実行
- Consumer Key 変換
- 独自キー定義

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
- WebSerial 設定画面で keymap を変更し、再ビルドなしで HID 出力へ反映する。
- USB hub 経由の複数 keyboard を統合し、片方の Shift / Ctrl をもう片方の keyboard 入力にも効かせる。

## 位置付け

本ライブラリは QMK のようなキーボードファームウェアそのものを目指すものではありません。ESP32 シリーズ上で動作する汎用的な「キーボード入力、統合、変換、出力エンジン」として位置付けます。

主対象は keyboard / consumer control などの HID 系入力ですが、将来的に mouse、trackpad、pointer、wheel などの入力・出力も event domain の拡張として扱える余地を残します。各種入力デバイス、通信方式、出力方式を自由に組み合わせられることを優先し、SoC ごとの機能差は各 adapter 実装で吸収します。
