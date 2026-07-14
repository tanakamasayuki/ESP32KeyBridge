# MediaKeys

メディアキーのないキーボードで、使っていないキー(F13〜F15 など)を音量・再生/停止キーにしたいとき用。

- remap は種別またぎができるので、キーボードキー → Consumer キーの割り当てが 1 行ずつで書けます(F13 → 音量下げ、F14 → 音量上げ、F15 → 再生/停止)。
- 押している間は押しっぱなしとして届き、音量の連続変化などの解釈は PC 側が行います。
- USB Device 出力(`EspUsbDeviceHidOutputAdapter`)は常に keyboard + consumer + mouse の**複合 HID デバイス**として PC に見えます。メディアキーは consumer interface から送られ、使わない interface(この例では mouse)は report を送らないだけで実害はありません。USB の interface 構成は `usbDevice.begin()` 時に確定し、動的な出し入れは USB の仕組み上できません。

## ハードウェア構成(ESP32-P4)

USB を 2 系統(Host + Device)使うため ESP32-P4 が必要です。

- **USB Device(PC 側)= HS 固定**(arduino-esp32 3.3.10 の制約)。
- **USB Host(キーボード側)= FS 明示指定**。キーボードのコネクタが FS の CDC ピン(GPIO24/25)に配線されているボードでは、スケッチ内の PHY スワップ行のコメントアウトを外してください。

P4 の USB ポートの詳細、表記と実配線が違うボードの注意(M5Stack Tab5 の事例)、HS を Host に使う場合の制約は [SwapCtrlCapsLock/README.ja.md](../SwapCtrlCapsLock/README.ja.md) を参照してください。

> **注意**: 使用するアダプタ(USB Host キーボード入力 / USB Device 複合 HID 出力)は実装済みです(実機検証はこれから)。USB Host 入力には EspUsbHost 2.2.0 以上が必要です(`onKeyboardState`)。
