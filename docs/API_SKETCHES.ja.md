# API スケッチ

この文書は、`ESP32KeyBridge` を使う側から見た理想的な API を先に書き出すための設計メモです。ここにあるコードは初期案であり、確定 API ではありません。

目的は、core がどの責務を持ち、adapter / example 側に何を残すかを確認することです。

## 方針

- core は入力 adapter、入力ごとの処理、入力統合、全体変換、出力 adapter をつなぐ。
- core は USB、BLE、GPIO、WebSerial、NVS、JSON などに直接依存しない。
- 設定は C++ コードで決め打ちできる。
- 外部から読み込んだ設定 object も適用できる。
- 複雑な設定 UI や保存処理は examples 側で実装する。
- ドキュメント内のコード例では namespace を省略しない。`using namespace` や type alias で短くするかはユーザー側の選択とする。

## Code Domain

keyboard 以外の HID 系入力へ拡張できるように、core には `esp32keybridge::InputCode` を置きます。

```cpp
esp32keybridge::InputCode code = esp32keybridge::keyboardCode(esp32keybridge::Key::A);
```

`esp32keybridge::InputState` は `esp32keybridge::InputCode` の集合を保持します。keyboard、consumer control、pointer、vendor などを同じ state として扱い、keyboard 専用の変換は keyboard domain の code にだけ適用します。
`esp32keybridge::InputCode` は code `0` を無効値として扱い、`esp32keybridge::isValid()` で確認できます。
state に code が含まれるかは `esp32keybridge::InputState::contains()` で確認します。keyboard 用の `isPressed()` は convenience として残します。
複数の state を set 的に合わせる場合は `esp32keybridge::InputState::mergeFrom()` を使えます。

`esp32keybridge::TransformConfig` の remap / disable は `esp32keybridge::InputCode` ベースです。`esp32keybridge::Key` を受け取る API は keyboard domain 用の convenience として扱います。

```cpp
esp32keybridge::InputCode volumeUp = esp32keybridge::consumerCode(0x00e9);
esp32keybridge::InputCode volumeDown = esp32keybridge::consumerCode(0x00ea);

esp32keybridge::ESP32KeyBridgeConfig config;
config.global.remap(volumeUp, volumeDown);
```

press / release の差分は `esp32keybridge::InputEvent` として表せます。

```cpp
esp32keybridge::InputEvent event = esp32keybridge::keyEvent(esp32keybridge::Key::A, true);
```
`esp32keybridge::EventInputAdapter` は event を投入して keyboard state を作る最小 adapter です。
`esp32keybridge::RecordingOutputAdapter` は最後に出力された state を保持する最小 output adapter です。unit test、single-board smoke、debug 用途で使えます。
`esp32keybridge::InputValueEvent` は pointer axis のような値付き入力を表します。押下状態の集合である `esp32keybridge::InputState` には入れず、`esp32keybridge::HidPointerReport::apply()` など値付き入力を扱う側へ渡します。

```cpp
esp32keybridge::InputValueEvent move =
    esp32keybridge::pointerAxisValueEvent(esp32keybridge::PointerAxis::X, 12);
```

## Hardcoded Remap

外部設定なしで、スケッチ内に変換内容を直接書く使い方です。

```cpp
#include <ESP32KeyBridge.h>

esp32keybridge::ESP32KeyBridge bridge;

esp32keybridge::EventInputAdapter input;
esp32keybridge::RecordingOutputAdapter output;

void setup()
{
  bridge.addInput(input);
  bridge.addOutput(output);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.global.remap(esp32keybridge::Key::CapsLock, esp32keybridge::Key::LeftCtrl);
  config.global.disable(esp32keybridge::Key::Insert);

  bridge.applyConfig(config);
  bridge.begin();
}

void loop()
{
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::CapsLock, true));
  bridge.update();
}
```

この使い方では、保存先や設定 UI は存在しません。ビルド時に決めた変換だけを実行します。
`bridge.outputState()` を読むと、最後の `update()` で生成された出力 state を debug や single-board smoke test で確認できます。
USB Host / USB Device を使う場合は、ここで使っている `esp32keybridge::EventInputAdapter` と `esp32keybridge::RecordingOutputAdapter` を adapter 実装に差し替えます。core はその adapter の依存を持ちません。

## Multi Keyboard Merge

複数の入力 keyboard を 1 つの keyboard state として統合する使い方です。

```cpp
#include <ESP32KeyBridge.h>

esp32keybridge::ESP32KeyBridge bridge;

esp32keybridge::EventInputAdapter keyboardA;
esp32keybridge::EventInputAdapter keyboardB;
esp32keybridge::RecordingOutputAdapter output;

void setup()
{
  bridge.addInput(keyboardA);
  bridge.addInput(keyboardB);
  bridge.addOutput(output);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.merge.shareModifiers = true;
  config.merge.shareKeyboardKeys = true;

  bridge.applyConfig(config);
  bridge.begin();
}

void loop()
{
  keyboardA.apply(esp32keybridge::keyEvent(esp32keybridge::Key::LeftShift, true));
  keyboardB.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, true));
  bridge.update();
}
```

期待する挙動:

```text
keyboardA: LeftShift down
keyboardB: A down
output: LeftShift + A
```

このユースケースでは、入力ごとの状態を持ち、merge layer で統合してから出力します。
最小 example は [examples/MultiKeyboardMerge](../examples/MultiKeyboardMerge/README.ja.md) に置きます。

## Per-Input Remap

特定の入力デバイスにだけ remap を適用する使い方です。

```cpp
esp32keybridge::ESP32KeyBridgeConfig config;

auto keyboard = config.input(0);
keyboard.remap(esp32keybridge::Key::CapsLock, esp32keybridge::Key::LeftCtrl);

auto scanner = config.input(1);
scanner.remap(esp32keybridge::Key::Enter, esp32keybridge::Key::Tab);

bridge.applyConfig(config);
```

同じ `Enter` でも、通常 keyboard と barcode reader で異なる扱いにできます。
現在の core MVP では input config slot の index で指定します。`bridge.addInput(input)` は登録順を config slot として使い、`bridge.addInput(input, configIndex)` で明示的に slot を指定できます。将来、device ID や profile matching を追加する可能性があります。
最小 example は [examples/PerInputRemap](../examples/PerInputRemap/README.ja.md) に置きます。

## Global Remap

入力デバイスを問わず、統合後の状態に対して remap を適用する使い方です。

```cpp
esp32keybridge::ESP32KeyBridgeConfig config;

config.global.remap(esp32keybridge::Key::CapsLock, esp32keybridge::Key::LeftCtrl);
config.global.remap(esp32keybridge::Key::LeftCtrl, esp32keybridge::Key::CapsLock);

bridge.applyConfig(config);
```

per-input remap と global remap は意味が異なります。

```text
per-input: 特定デバイスから来たキーだけを変換する。
global: どのデバイスから来ても統合後に変換する。
```

## Momentary Layer

trigger key が押されている間だけ layer remap を適用する使い方です。

```cpp
esp32keybridge::ESP32KeyBridgeConfig config;

config.layer.setMomentary(esp32keybridge::Key::Fn1);
config.layer.remap(esp32keybridge::Key::A, esp32keybridge::Key::B);

bridge.applyConfig(config);
```

期待する挙動:

```text
input:  Fn1 + A
output: B
```

`Fn1` は layer trigger として消費され、出力 state には含めません。最小 example は [examples/MomentaryLayer](../examples/MomentaryLayer/README.ja.md) に置きます。

## Simple Macro

trigger key を複数 key の state に展開する使い方です。

```cpp
esp32keybridge::ESP32KeyBridgeConfig config;

const esp32keybridge::Key macroKeys[] = {
  esp32keybridge::Key::LeftCtrl,
  esp32keybridge::Key::A,
  esp32keybridge::Key::B,
};
config.global.macro(esp32keybridge::Key::Fn1, macroKeys, 3);

bridge.applyConfig(config);
```

現在の macro は state 変換としての最小機能です。押下順、release 順、delay、文字列入力などの event 的な macro は今後検討します。最小 example は [examples/SimpleMacro](../examples/SimpleMacro/README.ja.md) に置きます。

## Layout Conversion

layout conversion 用の key mapping table を適用する使い方です。

```cpp
esp32keybridge::ESP32KeyBridgeConfig config;

config.layout.map(esp32keybridge::Key::A, esp32keybridge::Key::B);

bridge.applyConfig(config);
```

現在の layout conversion は汎用的な key mapping table として扱います。US/JA などの実 layout 定義、文字意味ベースの decode / encode、修飾キー付き記号変換は今後検討します。最小 example は [examples/LayoutConversion](../examples/LayoutConversion/README.ja.md) に置きます。

## Non-Keyboard Reports

keyboard 以外の domain も `esp32keybridge::InputCode` として pipeline を通せます。HID report へ詰める純粋処理は core に置きますが、USB descriptor や実送信は output adapter 側に置きます。

```cpp
esp32keybridge::InputState state;
state.press(esp32keybridge::Key::A);
state.press(esp32keybridge::consumerCode(esp32keybridge::ConsumerUsage::VolumeIncrement));
state.press(esp32keybridge::pointerButtonCode(1));

esp32keybridge::HidKeyboardReport keyboard = esp32keybridge::buildHidKeyboardReport(state);
esp32keybridge::HidConsumerReport consumer = esp32keybridge::buildHidConsumerReport(state);
esp32keybridge::HidPointerReport pointer = esp32keybridge::buildHidPointerReport(state);

pointer.apply(esp32keybridge::pointerAxisValueEvent(esp32keybridge::PointerAxis::Wheel, 1));
```

`esp32keybridge::RecordingHidKeyboardOutputAdapter`、`esp32keybridge::RecordingHidConsumerOutputAdapter`、`esp32keybridge::RecordingHidPointerOutputAdapter` は、実出力なしで report 化結果を確認するための adapter です。最小 example は [examples/NonKeyboardReports](../examples/NonKeyboardReports/README.ja.md) に置きます。

## Runtime Config Apply

設定の取得、parse、保存は core の外側で行い、core には設定 object を渡します。

```cpp
void loop()
{
  if (configService.available())
  {
    esp32keybridge::ESP32KeyBridgeConfig next = configService.readConfig();

    esp32keybridge::ESP32KeyBridgeConfigError error;
    if (bridge.validateConfig(next, error))
    {
      bridge.applyConfig(next);
      configStorage.save(next);
    }
    else
    {
      configService.writeError(error);
    }
  }

  bridge.update();
}
```

`configService` と `configStorage` は example またはユーザー実装です。WebSerial、UART、BLE、NVS、LittleFS、JSON などはここで選択します。
最小 example は [examples/RuntimeConfigApply](../examples/RuntimeConfigApply/README.ja.md) に置きます。
設定 object を再利用する場合は `next.clear()` で既存内容を消してから新しい内容を組み立てます。

## Runtime Adapter Reconfigure

入力や出力 adapter を実行中に差し替える場合は、bridge 側の登録を clear してから再登録します。

```cpp
bridge.clearInputs();
bridge.clearOutputs();

bridge.addInput(nextInput);
bridge.addOutput(nextOutput);
```

`clearInputs()` は bridge の登録を外すだけで、adapter object 自体の state は消しません。必要なら adapter 側の `clear()` などを呼びます。

## Reference WebSerial Config

WebSerial 設定画面は core ではなく reference example として提供します。

```text
Browser WebSerial UI
  -> CDC serial transport
  -> config parser
  -> esp32keybridge::ESP32KeyBridgeConfig
  -> bridge.validateConfig()
  -> bridge.applyConfig()
  -> optional storage save
```

この example では、入力デバイス設定、キーマップ、出力デバイス設定をブラウザから変更できるようにします。ただし、同じ core は外部設定なしの hardcoded sketch でも使える必要があります。
