# Examples

examples はすべて**実用例**です。実ハードウェア構成でそのまま書き込んで使うスケッチで、中身は「1) ハードウェア起動 → 2) ブリッジ配線 → 3) 設定を組んで適用」の 3 段構成に統一しています。

> **注意**: examples が使うアダプタはすべて実装済みです(実機検証は進行中)。実装状況の一覧は [../docs/ADAPTERS.ja.md](../docs/ADAPTERS.ja.md) を参照。

## Practical Examples

- `FootSwitch`: フットスイッチや自作ボタンで定型文入力・メディア操作・ページ送りをしたいとき。ESP32-S3 + GPIO。
- `MediaKeys`: 使っていないキー(F13 など)を音量・再生/停止キーにしたいとき。ESP32-P4。
- `NaturalScroll`: マウスのホイール方向を PC の設定に手を入れずに反転したいとき(左利き用のボタン入れ替えも)。ESP32-P4。
- `SerialTextTyper`: シリアルで送った文字列を PC への打鍵にしたいとき(テスト自動化など)。ESP32-S3。
- `SwapCtrlCapsLock`: キーボードと PC の間に挟んで、両方の設定を変えずに CapsLock と Ctrl を入れ替えたいとき。ESP32-P4。**P4 の USB ポート事情の説明はここが基準です。**
- `UsKeyboardOnJapanesePc`: US 配列キーボードやバーコードリーダーを、日本語設定のままの PC で刻印どおり打ちたいとき。ESP32-P4。

## 自作の入出力をつなぎたいとき

example ではなく、次を入口にしてください。

- 最小のリファレンス実装: `ManualInputAdapter` / `ManualOutputAdapter`(`press()` / `release()` を呼ぶだけで入力になる・受け取った押下集合や文字を記録する)。
- アダプタの責務と API: `InputAdapter` / `OutputAdapter` のヘッダコメントと、各アダプタヘッダ(`ESP32KeyBridgeEspUsbHost.h` など)。
- report の組み立て: `buildHidKeyboardReport()` 等の builder 群。

## 追加候補

- 複数キーボードの統合(MergeKeyboards): 1 アダプタで全キーボードを統合するか、キーボード個別に分けるかの設計決定待ち。
- BLE⇄USB 変換(BleToUsb・BLE マウス): BLE は専用ライブラリ(central + peripheral 両役、NimBLE ベース、クラシック非対応)を別途作成し、完成までこのライブラリでは対応しない方針。
