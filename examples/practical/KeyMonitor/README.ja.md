# KeyMonitor

ブリッジが出力する内容をシリアルコンソールに表示する、入力ハードウェアのブリングアップ用ツール。ボタンを配線したら、押下・離しが流れるのを見て、実際の出力をつなぐ前に配線を確認できます。実出力と併設して、流れているトラフィックを監視する使い方も同じです。

- `LogOutputAdapter` は、押下集合が変化したとき・文字がタイプされたとき・相対軸が動いたときに人間可読の行を出します。lock 正本にはなりません。
- この例では実 HID 出力を使わず、`Serial`(ボードの USB-serial)にログだけを出します。

シリアル出力の例:

```text
KEYS keyboard:04
KEYS keyboard:04 keyboard:05
KEYS (empty)
KEYS consumer:00cd
AXIS wheel +3
```

## 応用: 実出力と併設する

任意の example にモニタを 1 つ足すだけで、実出力と並行して監視できます:

```cpp
esp32keybridge::LogOutputAdapter monitor(Serial1);
bridge.addOutput(pc);       // 実出力
bridge.addOutput(monitor);  // 監視(別ストリームへ)
```

## ハードウェア構成(ESP32-S3)

- ボタンを各ピンと GND の間に接続します(この例: GPIO4/5/6。配線に合わせて変更)。
- ログは `Serial`(ボードの USB-serial)に出ます。

> **注意**: 使用するアダプタ(GPIO 入力 / `LogOutputAdapter`)は実装済みです(実機検証はこれから)。
