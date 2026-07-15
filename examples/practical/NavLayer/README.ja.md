# NavLayer

どんな USB キーボードにも「押している間だけのナビレイヤー」を足したいとき用。**CapsLock を押している間だけ H/J/K/L が矢印キー**(U/O が Home/End)になります。ノート PC の Fn レイヤーや vim の矢印と同じ感覚で、PC には何もインストールしません。

- **レイヤーの仕組み**: CapsLock を仮想キー(`VirtualUsage::V1`)に remap し、その仮想キーで有効になるレイヤーを 1 枚用意します。レイヤーが有効な間だけ `layer.remap()` の差し替えが効き、登録の無いキーはそのまま通るので、CapsLock を押しながらでも普通のタイピングはできます。
- **押下時に確定**: キーは押した瞬間に「何になるか」が決まり、離すまで保持されます。先に CapsLock(トリガ)を離しても、押しっぱなしの矢印はそのまま(スタックしません)。
- CapsLock は**レイヤー(Fn)キー専用**になり、PC には届きません(Caps トグルも無くなります)。CapsLock を残したい場合は、スケッチのトリガキーを別のキーに変えてください。
- レイヤーは複数枚(`MaxLayers` まで)登録でき、トリガを別の仮想キーにすれば重ねられます。

## カスタマイズ

`config.addLayer(...)` に続けて `nav.remap(元キー, 差し替え先)` を足すだけです。差し替え先はキー種別をまたげます(例: レイヤー中の数字段をメディアキーに):

```cpp
nav.remap(esp32keybridge::KeyboardUsage::Digit1,
          esp32keybridge::ConsumerUsage::VolumeDecrement);
```

## ハードウェア構成(ESP32-P4)

USB を 2 系統(Host + Device)使うため ESP32-P4 が必要です(ESP32-S3 など USB 1 系統のチップ単体では動きません)。

- **USB Device = HS ポート**: PC へ挿します。
- **USB Host = FS ポート**: キーボードを挿します。

P4 の USB ポートの詳細(どのコネクタがどのピンに配線されているか、FS PHY のスワップが必要な場合、M5Stack Tab5 の事例など)は [../SwapCtrlCapsLock/README.ja.md](../SwapCtrlCapsLock/README.ja.md) が基準です。本 example も同じ配線に従います。

> **注意**: 使用するアダプタ(USB Host キーボード入力 / USB Device 複合 HID 出力)は実装済みです(実機検証はこれから)。USB Host 入力には EspUsbHost 2.3.0 以上が必要です。
