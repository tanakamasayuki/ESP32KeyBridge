# アダプタ一覧

アダプタの一覧・実装状況・PC からどう見えるか(USB デバイス構成)をまとめます。実装の進行([DEVELOPMENT_PLAN.ja.md](DEVELOPMENT_PLAN.ja.md) 実装順 7)に合わせて更新します。

状況の凡例: **実装済み** / **モック**(スケッチ向け API は確定、ビルドのみ可) / **構想**(ヘッダ未作成)。

## 入力アダプタ

| クラス | ヘッダ | 依存 | 状況 | 機能 |
|---|---|---|---|---|
| `ManualInputAdapter` | `ESP32KeyBridge.h`(core) | なし | **実装済み** | `press()` / `release()` で論理押下を注入。相対値(`addAxisDelta`)、切断模擬、LockState 受信の記録。自作入力の最小リファレンス |
| `EspUsbHostKeyboardInputAdapter` | `ESP32KeyBridgeEspUsbHost.h` | EspUsbHost | **モック** | USB キーボード(同一デバイスの consumer キー含む)。LockState をキーボード LED へ転送(`setKeyboardLeds`)。切断で全 release |
| `EspUsbHostMouseInputAdapter` | `ESP32KeyBridgeEspUsbHost.h` | EspUsbHost | **モック** | USB マウス。ボタン = MouseButton キー、移動・ホイール = 相対軸 |
| `GpioKeyInputAdapter` | `ESP32KeyBridgeGpio.h` | Arduino GPIO のみ | **モック** | `addKey(pin, key, activeLow, pullUp)` で最大 `SOC_GPIO_PIN_COUNT` キー。デバウンス内蔵。ピン設定は初回 `update()` |
| `BleKeyboardInputAdapter` | `ESP32KeyBridgeBle.h` | BLE ライブラリ未定 | **モック** | BLE キーボード(HID over GATT)。ペアリング・再接続はアダプタ内。切断で全 release |
| GPIO マトリクス | - | Arduino GPIO のみ | **構想** | スキャン(専用 task / 周期タイマ ≈1kHz)・ゴースト対策・座標→キー割り当て |
| ロータリーエンコーダ | - | PCNT | **構想** | ハードウェア計数、`update()` でカウンタ差分 → 相対軸 or タップ合成 |
| BLE マウス / IR / I2C・SPI エキスパンダ | - | - | **構想** | [DEVELOPMENT_PLAN.ja.md](DEVELOPMENT_PLAN.ja.md) の対応予定 |

補足: 文字列入力(シリアル指令、シリアル型バーコードリーダー)はアダプタではなく、ブリッジの一級入力 `typeText()` / `typeChar()` を使います(example: SerialTextTyper)。

## 出力アダプタ

| クラス | ヘッダ | 依存 | 状況 | lock 報告 | 機能 |
|---|---|---|---|---|---|
| `ManualOutputAdapter` | `ESP32KeyBridge.h`(core) | なし | **実装済み** | 模擬可 | 受け取った押下集合・文字・相対値を記録。ホストの LED report を模擬できる(テスト・自作出力のリファレンス) |
| `EspUsbDeviceKeyboardOutputAdapter` | `ESP32KeyBridgeEspUsbDevice.h` | EspUsbDevice | **モック(次の実装対象)** | あり | USB キーボードデバイス。LED output report 受信 = lock 正本になれる |
| `EspUsbDeviceHidOutputAdapter` | `ESP32KeyBridgeEspUsbDevice.h` | EspUsbDevice | **モック** | あり | 複合 HID(keyboard + consumer + mouse)。相対軸 → mouse report(飽和 + 繰り越し) |
| BLE HID 出力 | - | BLE ライブラリ未定 | **構想** | あり(予定) | |
| UART 出力(イベント・ログ・文字) | - | なし | **構想** | なし | `writeText()` を直接受ける文字対応出力 |

## PC からどう見えるか(USB デバイス構成)

どの出力アダプタを選ぶかで、PC に見える USB デバイスの構成が決まります。

| 出力アダプタ | PC から見える構成 | 送る report |
|---|---|---|
| `EspUsbDeviceKeyboardOutputAdapter` | HID キーボード単体 | keyboard(boot 6KRO) |
| `EspUsbDeviceHidOutputAdapter` | 複合 HID デバイス(keyboard + consumer + mouse) | keyboard(boot 6KRO)/ consumer(16bit usage × 1)/ mouse(ボタン 8 + X/Y/Wheel/Pan、int8 飽和 + 繰り越し) |

規則:

- **interface 構成は `usbDevice.begin()` 時に確定**し、以後変更できません(追加・削除には再 enumerate = 抜き挿し相当が必要)。未使用の interface も PC からは見え続けます(report が飛ばないだけで実害なし)。
- consumer / mouse の report が PC に存在するのは、複合出力を**登録した場合のみ**です(仕様: マウス report はマウス対応出力を登録した場合のみ)。
- VID / PID / 製品名などのデバイス素性は `EspUsbDeviceConfig` で(スタックはスケッチ所有)。
- どちらの USB Device 出力も **LED output report を受信**し、接続中は lock 状態の正本(の連鎖の候補)になります([DATA_MODEL.ja.md](DATA_MODEL.ja.md) の Lock 状態)。
- rollover(32 キー)report builder は core にありますが、v1 の USB Device 出力は boot keyboard を基本とします(NKRO descriptor は将来のアダプタ拡張)。

## 追加依存ライブラリ

core(`ESP32KeyBridge.h`)は依存ゼロ(純粋 C++)です。アダプタヘッダは**ヘッダのみ**なので、include したアダプタの分だけ依存ライブラリが必要になります(include しなければ不要)。

| アダプタヘッダ | 依存ライブラリ | 入手 | sketch.yaml での指定例 |
|---|---|---|---|
| `ESP32KeyBridgeEspUsbHost.h` | [EspUsbHost](https://github.com/tanakamasayuki/EspUsbHost)(2.1.3 時点) | Arduino Library Manager | `- EspUsbHost (2.1.3)` |
| `ESP32KeyBridgeEspUsbDevice.h` | [EspUsbDevice](https://github.com/tanakamasayuki/EspUsbDevice)(1.2.5 時点) | Arduino Library Manager | `- EspUsbDevice (1.2.5)` |
| `ESP32KeyBridgeGpio.h` | なし(Arduino GPIO のみ) | - | - |
| `ESP32KeyBridgeBle.h` | 未定(BLE ライブラリ選定待ち) | - | - |

- ボードサポートは arduino-esp32(examples は 3.3.10 の profile を同梱。ESP32-P4 の USB Device は HS 固定などの制約は [SwapCtrlCapsLock の README](../examples/practical/SwapCtrlCapsLock/README.ja.md) を参照)。
- リポジトリを隣に checkout して開発する場合は、sketch.yaml の `libraries:` に `- dir: ../../../../EspUsbHost` のようなディレクトリ参照も使えます。

## 共通規則(全アダプタ)

- スタック(EspUsbHost / EspUsbDevice 等)は**スケッチが所有**し、スタック自身の config で起動する。アダプタはコンストラクタで参照を受けるだけで **`begin()` を持たない**(購読は初回 `update()`。USB device の interface 登録のみアダプタ構築時)。
- 時間・割り込み・タスクはアダプタ内部に閉じ、`update()` の呼び出しコンテキストへスレッドセーフに引き渡す([CORE_DESIGN.ja.md](CORE_DESIGN.ja.md) の「時間・並行性の境界」)。
- 出力アダプタは表現できるものだけを出し、他は黙って捨てる(virtual キーは出力に出ない)。
