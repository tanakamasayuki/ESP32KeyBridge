# FootSwitch

フットスイッチや自作ボタンをキーボードとして PC につなぎたいとき用。手がふさがっている場面のワンアクション入力に使います。

この example は 3 スイッチ構成です:

| ピン | キー | 用途 |
|---|---|---|
| GPIO4 | virtual キー(→ 文字列マクロ) | 定型文(メールアドレス等)を打つ |
| GPIO5 | Consumer PlayPause | メディアペダル(再生/停止) |
| GPIO6 | PageDown | プレゼンのページ送り |

- スイッチは `addKey(pin, key, activeLow, pullUp)` で登録します(極性・プルアップはピンごとに指定)。上限はチップの GPIO 数(`SOC_GPIO_PIN_COUNT`)そのものです。デバウンスはアダプタ内で行います。
- ペダルはどの種別のキーでも直接押せます。Consumer キーを割り当てればそのままメディアペダルです。
- virtual キーはブリッジ内専用で PC には届きません。事前定義スロット `VirtualUsage::V1`〜`V16`(足りなければ `virtualKey(n)` で 1〜65535 の任意番号)にスケッチ内で名前を付けて使います(この例では `kEmailPedal = VirtualUsage::V1`)。
- 文字は `config.hostLayout`(既定 en_us)で打鍵に展開されます。PC 側のレイアウト設定に合わせてください。

## 応用: momentary layer のペダルを足す

キーボード入力アダプタと併用する場合、ペダルを「踏んでいる間だけ有効なレイヤー」のトリガーにすると、J/K を Down/Up にするような使い方ができます(レイヤーは入力横断で効きます)。ペダルを 1 つ追加し、専用の virtual キーを割り当てます:

```cpp
const esp32keybridge::Key kNavPedal = esp32keybridge::VirtualUsage::V2;

pedals.addKey(GPIO_NUM_7, kNavPedal, /*activeLow=*/true, /*pullUp=*/true);

esp32keybridge::LayerConfig &nav = config.addLayer(kNavPedal);
nav.remap(esp32keybridge::KeyboardUsage::J, esp32keybridge::KeyboardUsage::Down);
nav.remap(esp32keybridge::KeyboardUsage::K, esp32keybridge::KeyboardUsage::Up);
```

## ハードウェア構成(ESP32-S3)

- ESP32-S3 の USB OTG ポートを PC へ挿します(キーボードデバイスとして見えます)。
- スイッチは各ピンと GND の間に接続します(閉で押下)。ピンは配線に合わせて変更してください。

> **注意**: USB Device keyboard 出力アダプタは実装済み(実機検証はこれから)、GPIO 入力アダプタはビルド確認用のモックです(実装順 7 で実装されます)。
