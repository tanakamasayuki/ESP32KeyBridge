# 設定と WebSerial

`ESP32KeyBridge` core は、キーマップやレイヤーを C++ コードで決め打ちする使い方と、外部から読み込んだ設定を適用する使い方の両方を許容します。

ライブラリ本体は、設定をどこから取得するか、どこへ保存するか、どの UI で編集するかを規定しません。WebSerial から入力デバイス設定、キーマップ、出力デバイス設定を編集する画面は、リファレンス実装として examples 側で提供する方針です。
外部設定適用の最小例は [../examples/RuntimeConfigApply/README.ja.md](../examples/RuntimeConfigApply/README.ja.md) に置きます。

## 基本構成

```text
Browser / CLI
  ├─ USB HID keyboard / consumer control  入力デバイスとして認識
  └─ USB CDC serial                       WebSerial / CLI で設定変更

Reference example
  ├─ WebSerial / CDC transport
  ├─ config parser / optional validator
  ├─ optional storage
  └─ ESP32KeyBridge coreへ適用
```

## 設定で変更できるもの

- キーマップ
- レイヤー
- ワンショット、モーメンタリ、トグルなどの layer 動作
- キー無効化、キー入れ替え
- マクロ
- Consumer Control 割り当て
- USB device 情報
- Bluetooth device 情報
- 有効にする入力 adapter / 出力 adapter

これらは「外部設定で変更できなければならない」という意味ではありません。同じ内容を C++ コードで構築し、外部設定なしで動かす使い方もサポートします。

## 設定チャネル

WebSerial / CDC serial はリファレンス実装の主な候補です。

- WebSerial 対応ブラウザから設定画面を開く。
- CDC serial へ設定取得、設定更新、保存、再読み込みのコマンドを送る。
- example 側は設定を検証し、必要に応じて永続 storage へ保存する。
- 変換 engine は C++ API または設定 object として受け取った内容を適用する。

UART、BLE、TCP などは同じ設定モデルへ別 transport で接続する候補として扱います。これらは core 必須機能ではありません。

## ESP32SerialCtl との関係

`ESP32SerialCtl` は WebSerial コントロールパネル、serial CLI、NVS 設定、ファイル操作などの近い用途を持つ既存ライブラリです。

`ESP32KeyBridge` の WebSerial リファレンス実装では、設定チャネルや UI の設計で `ESP32SerialCtl` の以下の考え方を参考にします。

- 人間と機械の両方が扱いやすい serial protocol。
- `OK` / `ERR` のような予測可能な応答。
- `conf get` / `conf set` / `conf list` に近い設定操作。
- WebSerial UI からの接続、コマンド送信、設定表示。
- NVS や filesystem を使った永続設定。

ただし `ESP32KeyBridge` core は `ESP32SerialCtl` に依存しません。設定対象は keyboard bridge 固有であり、キーマップ、layer、macro、入力 adapter、出力 adapter などを扱います。

## 初期コマンド案

確定 API ではありません。

```text
kb info
kb status
kb config get
kb config set <json>
kb config save
kb config load
kb keymap get
kb keymap set <json>
kb layer list
kb layer get <index>
kb layer set <index> <json>
kb macro list
kb macro get <name>
kb macro set <name> <json>
kb reset
```

JSON を使う場合でも、転送単位、サイズ制限、部分更新、検証エラーの返し方は別途設計します。

## 永続化方針

初期候補:

- 小さい設定: NVS
- 大きいキーマップ / macro: LittleFS または SPIFFS
- 大容量またはユーザー交換前提: SD card

保存形式は core では固定しません。リファレンス実装では、バージョン管理、互換性確認、破損検出、容量制限への対応を優先します。WebSerial UI 側で編集しやすい形式と、デバイス内部で扱いやすい形式を分ける可能性があります。
