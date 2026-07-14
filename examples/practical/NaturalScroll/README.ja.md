# NaturalScroll

マウスのホイール方向を、PC の設定に手を入れずに反転したいとき用。設定を変えられない会社 PC・共有 PC でもマウス側(このブリッジ)で完結し、OS をまたいでも同じ操作感になります。

- ホイールは相対軸として流れ、`config.setAxisScale(Axis::Wheel, -1)` の負のスケールで反転します。ボタンとポインタ移動はそのまま素通しです。
- スケールは倍率も兼ねます: `-2` = 反転 + 2 倍速、`2` = 向きそのまま 2 倍速。
- 複数マウスをつないだ場合も全部まとめて同じ変換がかかります(入力アダプタが合算します)。

## 応用: 左利き用にボタンを入れ替える

マウスのボタンもキーの一種(`MouseUsage::Left` / `Right` / `Middle` / `Back` / `Forward`。名前の無い番号は `mouseButtonKey(n)`)なので、remap で入れ替えられます:

```cpp
config.global.remap(esp32keybridge::MouseUsage::Left, esp32keybridge::MouseUsage::Right);
config.global.remap(esp32keybridge::MouseUsage::Right, esp32keybridge::MouseUsage::Left);
```

ポインタ移動の左右反転(`setAxisScale(Axis::X, -1)`)と組み合わせると、トラックボールの向きを変えて置くような使い方もできます。

## ハードウェア構成(ESP32-P4)

USB を 2 系統(Host + Device)使うため ESP32-P4 が必要です。

- **USB Device(PC 側)= HS 固定**(arduino-esp32 3.3.10 の制約)。
- **USB Host(マウス側)= FS 明示指定**。マウスのコネクタが FS の CDC ピン(GPIO24/25)に配線されているボードでは、スケッチ内の PHY スワップ行のコメントアウトを外してください。

P4 の USB ポートの詳細、表記と実配線が違うボードの注意(M5Stack Tab5 の事例)、HS を Host に使う場合の制約は [SwapCtrlCapsLock/README.ja.md](../SwapCtrlCapsLock/README.ja.md) を参照してください。

> **注意**: 使用するアダプタ(USB Host マウス入力 / USB Device 複合 HID 出力)は実装済みです(実機検証はこれから)。
