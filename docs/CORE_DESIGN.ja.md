# Core Design

`ESP32KeyBridge` core は、特定の入出力方式に依存しないキーボード入力の統合・変換エンジンです。

USB Host / USB Device、GPIO、BLE、UART、WebSerial、NVS、JSON などは core の外側に置きます。必要な依存は adapter または example が持ちます。

ドキュメント内の C++ 例では、`esp32keybridge::` などの namespace を省略しません。`using namespace`、type alias、短い wrapper を使うかどうかはユーザー側の選択とします。

## データフロー

```text
Physical Input
  -> Input Adapter
  -> Per-Input Pipeline
  -> DeviceState
  -> Merge Layer
  -> MergedState
  -> Global Pipeline
  -> OutputState / ActionQueue
  -> Output Adapter
```

## Input Adapter

入力 adapter は物理入力や通信入力を読み、core が扱える event / state に変換します。

例:

- GPIO matrix adapter
- USB HID keyboard adapter
- BLE HID keyboard adapter
- UART event adapter
- barcode reader adapter

責務:

- raw input を読む。
- press / release などの差分を作る。
- 入力デバイス ID を付ける。
- 入力ごとの現在 state を更新する。

USB adapter が `EspUsbHost` を使う場合でも、その依存は adapter 内に閉じ込めます。

## Per-Input Pipeline

per-input pipeline は、入力デバイスごとの処理を行います。

例:

- 特定 keyboard の CapsLock を LeftCtrl にする。
- barcode reader の Enter suffix を Tab にする。
- 特定 input profile の layout decode を行う。
- 物理入力 adapter が必要とする補正を行う。

同じ処理名でも、per-input と global では意味が異なります。

```text
per-input remap:
  この入力デバイスから来た CapsLock だけを LeftCtrl にする。

global remap:
  どの入力から来ても、統合後の CapsLock を LeftCtrl にする。
```

## DeviceState

`DeviceState` は入力デバイスごとの現在状態です。

最初は keyboard state を主対象にします。

```text
DeviceState
  deviceId
  keyboard modifiers
  keyboard keys
  consumer keys
  future pointer state
```

将来、mouse、trackpad、wheel などを扱えるように event domain を拡張できる余地を残します。ただし MVP は keyboard / consumer control を優先します。

core には `esp32keybridge::InputCode` を置き、keyboard、consumer control、pointer button、pointer axis、vendor などの domain を表せるようにします。`esp32keybridge::InputState` は `InputCode` の集合を保持し、keyboard 以外の code も同じ state として扱います。
`esp32keybridge::Key` は keyboard domain の convenience enum で、基本キーは USB HID keyboard usage ID に寄せた値を持ちます。adapter は raw HID usage と `esp32keybridge::Key` / `esp32keybridge::InputCode` の対応を単純に扱えるようにします。言語・地域依存の物理キーは `NonUsHash`、`NonUsBackslash`、`International1`-`International9`、`Lang1`-`Lang9` として HID usage に沿って表します。
HID adapter は `esp32keybridge::keyFromHidUsage()` と `esp32keybridge::hidUsageFromKey()` を使って raw HID usage と `esp32keybridge::Key` を変換できます。実 HID keyboard usage として出せるかは `esp32keybridge::isHidKeyboardKey()` で確認できます。`esp32keybridge::Fn1` のような core 内 symbolic key は HID usage ではないため、`esp32keybridge::hidUsageFromKey()` では `0` になります。

`esp32keybridge::TransformConfig` は `InputCode` ベースの remap / disable を持ちます。`esp32keybridge::Key` を受け取る API は keyboard domain 用の convenience です。これにより Consumer Control のような keyboard 以外の code も、同じ transform pipeline で扱えます。

layout conversion と state macro は現時点では keyboard domain 中心です。keyboard 以外の domain は、明示的に remap / disable されない限り pipeline を通過します。

press / release の差分には `esp32keybridge::InputEvent` を使います。`esp32keybridge::InputState::apply(event)` で event を state に反映できます。debounce、hold/tap、event macro sequence などはこの event 表現を使って後から追加する想定です。
pointer axis のような値付き入力は、押下状態とは別に `esp32keybridge::InputValueEvent` で表します。`esp32keybridge::PointerAxis` は `X`、`Y`、`Wheel`、`Pan` を持ち、`esp32keybridge::pointerAxisValueEvent()` で delta を表現できます。値付き入力を state pipeline に統合するか、output adapter / example 側で別 queue として扱うかは、mouse / trackpad の実装時にユースケースを見て決めます。
`esp32keybridge::EventInputAdapter` は、この event 表現を使って state を作る最小 input adapter です。実 adapter は raw input を `InputEvent` に変換し、同じ state 更新の考え方を使えます。
`esp32keybridge::RecordingOutputAdapter` は最後に出力された state と write count を保持する最小 output adapter です。`esp32keybridge::RecordingHidKeyboardOutputAdapter`、`esp32keybridge::RecordingHidKeyboardRolloverOutputAdapter`、`esp32keybridge::RecordingHidConsumerOutputAdapter`、`esp32keybridge::RecordingHidPointerOutputAdapter` は最後に生成された report と write count を保持します。いずれも実出力を持たない test / smoke / debug で使います。

## Merge Layer

merge layer は複数の `DeviceState` を統合します。

例:

```text
keyboardA: LeftShift down
keyboardB: A down
merged: LeftShift + A
```

初期方針:

- modifier を共有できる。
- 通常キーを共有できる。
- Consumer Control、pointer button、pointer axis、vendor code を domain ごとに共有できる。
- 入力デバイスを単一 keyboard として扱える。
- 競合や rollover 制限は output adapter の都合と分けて扱う。

`esp32keybridge::MergeConfig` は `shareModifiers`、`shareKeyboardKeys`、`shareConsumer`、`sharePointerButtons`、`sharePointerAxes`、`shareVendor` を持ちます。複数 keyboard の modifier 共有だけ有効にしつつ、pointer や vendor event は merge しない、といった使い分けを core 側で表現できます。

USB HID boot keyboard の 6KRO 制限は output adapter 側の制約です。core の merged state は、可能な限り出力形式に依存しない状態で保持します。`esp32keybridge::InputState` は 6 個を超える key state も保持しますが、full NKRO ではなく `esp32keybridge::InputState::MaxCodes` 個までです。現状の上限は 32 code です。

## Global Pipeline

global pipeline は、統合後の状態に対して全体処理を行います。

例:

- global remap
- layer
- macro
- layout conversion
- key disable
- output routing

macro は event 的、layer は state 的な性質が強いため、processor は event と state の両方を扱える設計にします。

## Output Adapter

出力 adapter は `OutputState` または `ActionQueue` を具体的な出力へ変換します。

例:

- USB HID keyboard output
- BLE HID keyboard output
- UART event output
- GPIO output

USB Device adapter が `EspUsbDevice` を使う場合でも、その依存は adapter 内に閉じ込めます。

USB boot keyboard 形式の 6KRO report に詰める純粋処理として、core は `esp32keybridge::buildHidKeyboardReport()` を提供します。これは `esp32keybridge::InputState` から modifier byte と最大 6 個の key usage を作り、boot report に入りきらない場合は report の `overflow` を立てます。`esp32keybridge::HidKeyboardReport` は `clear()`、`empty()`、8バイト boot report へ書き出す `writeBootReport()` を持ちます。6KRO への変換はあくまで boot keyboard 用の出力形式であり、内部 state の保持上限ではありません。

32 code までの rollover report に詰める純粋処理として、core は `esp32keybridge::buildHidKeyboardRolloverReport()` も提供します。`esp32keybridge::HidKeyboardRolloverReport` は modifier byte と最大 32 個の key usage を持ち、33バイト report へ書き出す `writeReport()` を持ちます。実際の HID descriptor、複合 HID descriptor、USB 送信などは output adapter 側の責務です。

Consumer Control の最小 report として、core は `esp32keybridge::buildHidConsumerReport()` も提供します。これは最初の Consumer usage を 16-bit report として扱い、複数の Consumer usage が同時にある場合は `overflow` を立てます。`esp32keybridge::ConsumerUsage` には volume、media transport、browser navigation など代表的な usage を置き、未登録の usage は `esp32keybridge::consumerCode(uint16_t)` で直接表現します。複数 Consumer usage を同時送信する descriptor や report 形式は output adapter 側で拡張します。

Pointer の最小 report として、core は `esp32keybridge::HidPointerReport` と `esp32keybridge::buildHidPointerReport()` を提供します。`esp32keybridge::InputState` から最大 8 個の pointer button を button bit に変換し、`esp32keybridge::InputValueEvent` で `X`、`Y`、`Wheel`、`Pan` の delta を加算します。axis は 8-bit report 範囲に saturate し、範囲外の button や delta overflow では `overflow` を立てます。descriptor、absolute pointer、multi-touch、より大きい axis report は output adapter 側で拡張します。

`ESP32KeyBridge` は最後の `update()` で作られた merged state と output state を参照できる API を持ちます。これは debug、single-board smoke test、reference example の状態表示に使います。実際の出力は引き続き output adapter の責務です。

## Configuration Boundary

core は設定 object を受け取って適用します。

core が規定しないもの:

- 設定をどこから取得するか。
- 設定をどこへ保存するか。
- JSON、binary、C++ builder などの外部形式。
- WebSerial、UART、BLE、TCP などの転送手段。
- 設定画面 UI。

core が持つもの:

- C++ API で設定を構築する入口。
- 設定 object を検証する入口。
- 設定 object を適用する入口。

想定フロー:

```text
hardcoded C++ config
  -> esp32keybridge::ESP32KeyBridgeConfig
  -> bridge.applyConfig()

external config
  -> user/example parser
  -> esp32keybridge::ESP32KeyBridgeConfig
  -> bridge.validateConfig()
  -> bridge.applyConfig()
  -> optional user/example storage
```

## 実装済み Core MVP

core MVP は実 USB adapter ではなく、virtual input / recording output と host unit test で固定します。

実装済み:

- `esp32keybridge::InputCode` / `esp32keybridge::InputState`
- `esp32keybridge::InputEvent` と `esp32keybridge::EventInputAdapter`
- `esp32keybridge::InputValueEvent` と pointer axis delta 表現
- `esp32keybridge::RecordingOutputAdapter`
- `esp32keybridge::RecordingHidKeyboardOutputAdapter`
- `esp32keybridge::RecordingHidConsumerOutputAdapter`
- `esp32keybridge::RecordingHidPointerOutputAdapter`
- CapsLock -> LeftCtrl のような remap
- key disable
- 複数入力の merge と modifier 共有
- domain 別の merge option
- 入力追加順 index による per-input remap
- 明示 config index による input と per-input config の紐付け
- state 変換としての単純 macro
- key mapping table としての layout conversion
- HID keyboard / consumer / pointer の最小 report builder

次に adapter / example 側で検討するもの:

- USB Host keyboard input adapter
- USB Device keyboard / consumer / pointer output adapter
- GPIO matrix input adapter
- WebSerial reference configuration example
