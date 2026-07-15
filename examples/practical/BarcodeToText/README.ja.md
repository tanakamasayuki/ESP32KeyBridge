# BarcodeToText

USB HID バーコードリーダー(バーコードをキー入力として「打つ」タイプ)を、シリアルに流れる**文字列**に変換したいとき用。PC のシリアルモニタで読む、別の機器へ渡す、といった使い方ができます。キーボードフォーカスや前面アプリは不要です。

- バーコードリーダーは USB Host 入力アダプタで読みます(キーボードモードの HID スキャナならそのまま動きます)。
- **レイアウトは入力側の設定**です。スキャナ入力に `convertLayout(en_us)`(スキャナの刻印)を与えると、bridge が押下を刻印でデコードして文字にし、それが文字ストリームに流れます。`TextOutputAdapter` はその文字を `Print`(ここでは `Serial`)へ書くだけで、出力側にレイアウトは持ちません。
- Shift も反映され、スキャン終端の Enter は改行(Tab はタブ)として出ます。
- JIS 設定のスキャナなら入力側を `convertLayout(esp32keybridge::KeyboardLayout::jaJp())` にします。

## 応用: UART で別機器へ送る / 打鍵文字も流す

`Print` を差し替えるだけで出力先を変えられます。PC ではなく別 MCU や端末へ送るなら `Serial1`(ハードウェア UART)を渡します:

```cpp
esp32keybridge::TextOutputAdapter text(Serial1);
```

`typeText()` で注入した文字も同じ出力に流れます(注入テキストと、convertLayout 入力の押下由来の文字が、同じ文字ストリームとして流れます)。

## ハードウェア構成(ESP32-S3)

- バーコードリーダーを ESP32-S3 の USB OTG ポートに挿します(USB Host として読みます)。
- デコードされた文字は `Serial`(ボードの USB-serial = PC 側シリアルポート)に出ます。

> **注意**: 使用するアダプタ(USB Host キーボード入力 / `TextOutputAdapter`)は実装済みです(実機検証はこれから)。スキャナが 1 文字を極端に速く打つ場合、`update()` 間隔より短い押下は取りこぼす可能性があります(USB Host 入力は状態スナップショット方式のため)。
