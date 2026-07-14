# SerialTextTyper

シリアルで送った文字列を、PC への打鍵として入力したいとき用。テスト自動化、キオスクの初期設定、「キーボードとしてしか触れないマシン」への文字列送り込みなどに使います。

- 入力アダプタはありません。文字ストリーム(`typeText()` / `typeChar()`)はブリッジの一級入力です。
- 文字は**届いたそばから打鍵**されます(行バッファなし)。UTF-8 の多バイト文字だけ、続きのバイトが揃うまで待ちます。改行(CR / LF / CRLF)は制御文字表で 1 つの Enter になります。
- 文字は `config.hostLayout`(既定 en_us)で打鍵に展開されます。PC 側のレイアウト設定が違う場合は合わせてください:

```cpp
esp32keybridge::ESP32KeyBridgeConfig config;
config.hostLayout = esp32keybridge::KeyboardLayout::jaJp();
bridge.applyConfig(config);
```
- `typeText()` は UTF-8 を受けます。打鍵化できるのは「ホストレイアウト表にある印字文字」(en_us = ASCII 印字文字 / ja_jp = ASCII 印字文字 + `¥`)と制御文字 5 種(TAB / LF / CR / BS / ESC)で、それ以外(かな漢字・絵文字など)は読み捨ててカウントします(`textEncodeFailCount()`)。詳細は [DATA_MODEL.ja.md](../../../docs/DATA_MODEL.ja.md) の「文字ストリーム」を参照してください。
- 打鍵は 1 update に 1 フェーズずつ進みます。loop の周期がそのまま打鍵ペースです(この例では約 1ms/フェーズ)。
- 受信は打鍵よりずっと速いので(115200bps ≒ 1 万文字/秒 vs 約 333 文字/秒)、**背圧**で調整します: テキストキューの空き(`bridge.typeAvailable()`)がある間だけ Serial から読み、埋まっている間は UART の RX バッファに滞留させます。タイムアウトなし・ブロックなしで、文字も欠落しません。

## ハードウェア構成(ESP32-S3)

- ESP32-S3 の USB OTG ポートを PC へ挿します(キーボードデバイスとして見えます)。
- 文字列は UART シリアル(ピン、またはボードの USB シリアル変換チップ)から受けます。OTG ポートは HID に使うため、シリアルは UART 側です。

> **注意**: 使用する USB Device keyboard 出力アダプタは実装済みです(実機検証はこれから)。
