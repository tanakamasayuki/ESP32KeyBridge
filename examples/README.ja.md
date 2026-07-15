# Examples

examples はすべて**実用例**です。実ハードウェア構成でそのまま書き込んで使うスケッチで、中身は「1) ハードウェア起動 → 2) ブリッジ配線 → 3) 設定を組んで適用」の 3 段構成に統一しています。

> **注意**: examples が使うアダプタはすべて実装済みです(実機検証は進行中)。一覧は [../docs/ADAPTERS.ja.md](../docs/ADAPTERS.ja.md) を参照。

## Practical Examples

- `BarcodeToText`: USB HID バーコードリーダーをシリアルの文字列に変換したいとき。ESP32-S3 + USB Host。
- `FootSwitch`: フットスイッチや自作ボタンで定型文入力・メディア操作・ページ送りをしたいとき。ESP32-S3 + GPIO。
- `GamepadToKeys`: USB ゲームパッドで PC をキーボード操作(十字=矢印、ボタン=Space/Enter など)したいとき。ESP32-P4 + USB Host/Device。
- `KeyMonitor`: 入力ハードのブリングアップ・監視用に、ブリッジ出力をシリアルに表示したいとき。ESP32-S3 + GPIO。
- `LayerKeypad`: 自作キーパッドで「1 キー 2 機能」(Fn 長押しでナビ→メディアに切替)をやりたいとき。ESP32-S3 + GPIO。
- `MediaKeys`: 使っていないキー(F13 など)を音量・再生/停止キーにしたいとき。ESP32-P4。
- `MergeKeyboards`: 複数の入力(USB キーボード + GPIO 追加ボタンなど)を 1 つのキーボードとして PC に見せたいとき。ESP32-P4 + USB Host/Device + GPIO。
- `MidiToKeys`: USB MIDI コントローラのパッド/鍵盤をキーボードショートカットにしたいとき。ESP32-P4 + USB Host/Device。
- `NavLayer`: どんな USB キーボードにも「CapsLock 長押しで HJKL が矢印」のナビ(Fn)レイヤーを足したいとき。ESP32-P4 + USB Host/Device。
- `NumPad`: 自作テンキー・マクロパッド・自作キーボードを作りたいとき(スイッチマトリクスを GPIO 直結)。ESP32-S3 + GPIO。
- `NaturalScroll`: マウスのホイール方向を PC の設定に手を入れずに反転したいとき(左利き用のボタン入れ替えも)。ESP32-P4。
- `ScrollDial`: 手元に物理スクロールダイヤル(回すとホイール、押すと中クリック。ジョグにも)が欲しいとき。ESP32-S3 + GPIO(エンコーダ)。
- `SerialTextTyper`: シリアルで送った文字列を PC への打鍵にしたいとき(テスト自動化など)。ESP32-S3。
- `SwapCtrlCapsLock`: キーボードと PC の間に挟んで、両方の設定を変えずに CapsLock と Ctrl を入れ替えたいとき。ESP32-P4。**P4 の USB ポート事情の説明はここが基準です。**
- `TouchButtons`: 自作インプットアダプタの書き方を知りたいとき(題材 = ESP32-S3 内蔵の静電容量タッチをボタンに)。ESP32-S3。
- `UsKeyboardOnJapanesePc`: US 配列キーボードやバーコードリーダーを、日本語設定のままの PC で刻印どおり打ちたいとき。ESP32-P4。
- `VolumeKnob`: 手元に物理的な音量つまみが欲しいとき(ロータリーエンコーダ。スクロールダイヤルにも)。ESP32-S3 + GPIO。

## 自作の入出力をつなぎたいとき

次を入口にしてください。

- **自作インプットアダプタの書き方の実例: `TouchButtons`**(`InputAdapter` を継承して `update()` でハードを読み、`keys()` で押下集合を返す最小テンプレート。I2C・センサ・ネットワーク等も同じ型で書けます)。
- 最小のリファレンス実装: `ManualInputAdapter` / `ManualOutputAdapter`(`press()` / `release()` を呼ぶだけで入力になる・受け取った押下集合や文字を記録する)。
- アダプタの責務と API: `InputAdapter` / `OutputAdapter` のヘッダコメントと、各アダプタヘッダ(`ESP32KeyBridgeEspUsbHost.h` など)。
- report の組み立て: `buildHidKeyboardReport()` 等の builder 群。

## 追加候補

- BLE⇄USB 変換(BleToUsb・BLE マウス): BLE は専用ライブラリ(central + peripheral 両役、NimBLE ベース、クラシック非対応)を別途作成し、完成までこのライブラリでは対応しない方針。
