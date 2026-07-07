# Examples

`ESP32KeyBridge` の example です。

初期段階では、実 USB / BLE / GPIO adapter ではなく、virtual input / serial output を使って core の使い方を示します。USB Host、USB Device、WebSerial 設定画面などは core が固まってから adapter / reference example として追加します。

## Core Examples

- `Basic`: `esp32keybridge::ESP32KeyBridge` を生成し、`begin()` / `update()` を呼ぶ最小 sketch。
- `HardcodedRemap`: 外部設定なしで C++ コードから remap / disable を設定する例。
- `MultiKeyboardMerge`: 複数の virtual keyboard 入力を 1 つの keyboard state へ統合する例。
- `PerInputRemap`: 特定の入力デバイスにだけ remap を適用し、merge 後に global remap も適用する例。
- `MomentaryLayer`: `Fn1` を押している間だけ layer remap を適用する例。
- `SimpleMacro`: trigger key を複数 key の state に展開する例。

## 追加予定

- `UsbKeyboardBridge`: USB Host keyboard 入力を USB Device keyboard 出力へ橋渡しする例。
- `UsbKeyboardRemap`: USB keyboard 入力に remap / layout conversion を適用する例。
- `WebSerialConfig`: ブラウザから入力デバイス設定、キーマップ、出力設定を変更する reference example。
- `GpioMatrix`: GPIO matrix 入力 adapter の例。
