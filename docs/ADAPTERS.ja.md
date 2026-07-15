# アダプタ一覧

アダプタの一覧・実装状況・PC からどう見えるか(USB デバイス構成)をまとめます。実装の進行([DEVELOPMENT_PLAN.ja.md](DEVELOPMENT_PLAN.ja.md) 実装順 7)に合わせて更新します。

状況の凡例: **実装済み** / **モック**(スケッチ向け API は確定、ビルドのみ可) / **構想**(ヘッダ未作成)。

## 入力アダプタ

| クラス | ヘッダ | 依存 | 状況 | 機能 |
|---|---|---|---|---|
| `ManualInputAdapter` | `ESP32KeyBridge.h`(core) | なし | **実装済み** | `press()` / `release()` で論理押下を注入。相対値(`addAxisDelta`)、切断模擬、LockState 受信の記録。自作入力の最小リファレンス |
| `EspUsbHostKeyboardInputAdapter` | `ESP32KeyBridgeEspUsbHost.h` | EspUsbHost 2.2.0+ | **実装済み(実機検証待ち)** | USB キーボード(スタック上の全キーボードを合算、最大 4 台。consumer キー = メディアキーやリモコンも合算、同時 8 usage)。`onKeyboardState` の 256-bit スナップショット(boot / report-ID boot / NKRO 共通、修飾キー単独変化含む)を KeySet に写像。boot のロールオーバーエラーコード(usage 0x01〜0x03)は除外。LockState を全キーボードの LED へ転送(num/caps/scroll。kana は EspUsbHost に転送手段なし。**既知の制限**: EspUsbHost 2.2.0 は boot 宣言キーボードにしか LED を送れず、report-ID/NKRO 専用キーボードへは届かない = 上流依頼中)。presence は最初の report から、切断でそのデバイスのキーだけ解放 |
| `EspUsbHostMouseInputAdapter` | `ESP32KeyBridgeEspUsbHost.h` | EspUsbHost | **実装済み(実機検証待ち)** | USB マウス(スタック上の全マウスを合算、最大 4 台)。ボタン = MouseButton キー(union)、移動・ホイール = 相対軸(合算)。presence は最初の report から(列挙照会 API が無いため)、切断でそのマウスのボタンだけ解放 |
| `GpioKeyInputAdapter` | `ESP32KeyBridgeGpio.h` | Arduino GPIO のみ | **実装済み(実機検証待ち)** | `addKey(pin, key, activeLow, pullUp)` で最大 `SOC_GPIO_PIN_COUNT` キー。デバウンス内蔵(既定 5ms、`setDebounceMillis()` で調整)。ピン設定は登録後最初の `update()`。loop レート(≈1kHz)のポーリングで足りる |
| BLE キーボード / マウス入力 | - | 専用 BLE ライブラリ(作成予定) | **構想(別ライブラリ完成待ち)** | BLE キーボード / マウス(HID over GATT central)。専用 BLE ライブラリ(central + peripheral 両役を 1 本、NimBLE ベース、クラシック非対応)を別途作成し、完成までこのライブラリでは BLE 非対応 |
| `GpioMatrixInputAdapter` | `ESP32KeyBridgeGpio.h` | Arduino GPIO のみ | **実装済み(実機検証待ち)** | キーマトリクス(最大 16 行 × 16 列。スキャン・デバウンス・ゴースト判定は専用タスク 1kHz = loop がブロックしても取りこぼさない。初回 `update()` で起動、非アクティブ行は high-Z)。配線は `setRowPins()` / `setColPins()`(行 = スキャン出力、列 = プルアップ入力)、キーマップは `setKeys()` に行優先の並びで渡す(見た目 = 物理配置、座標番号なし。歯抜けは無効キー `Key()`)。ダイオード任意(無しはゴーストブロック = 確実な同時押し 2 キーまで。`setHasDiodes(true)` で無制限)。デバウンス内蔵 |
| `RotaryEncoderInputAdapter` | `ESP32KeyBridgeGpio.h` | PCNT(ESP-IDF 内蔵。`SOC_PCNT_SUPPORTED` のチップのみ) | **実装済み(実機検証待ち)** | ロータリーエンコーダ(PCNT ×4 直交デコード + グリッチフィルタ、割り込みなし、1 アダプタ 1 エンコーダ = チップの PCNT ユニット数まで)。1 ノッチ → キータップ(`mapToKeys(cw, ccw)`)または相対軸 ±1(`mapToAxis(axis)`)。`setPulsesPerDetent()`(既定 4 = EC11)。押し込みスイッチは GpioKeyInputAdapter で |
| BLE マウス / IR / I2C・SPI エキスパンダ | - | - | **構想** | [DEVELOPMENT_PLAN.ja.md](DEVELOPMENT_PLAN.ja.md) の対応予定 |

補足: 文字列入力(シリアル指令、シリアル型バーコードリーダー)はアダプタではなく、ブリッジの一級入力 `typeText()` / `typeChar()` を使います(example: SerialTextTyper)。

## 出力アダプタ

| クラス | ヘッダ | 依存 | 状況 | lock 報告 | 機能 |
|---|---|---|---|---|---|
| `ManualOutputAdapter` | `ESP32KeyBridge.h`(core) | なし | **実装済み** | 模擬可 | 受け取った押下集合・文字・相対値を記録。ホストの LED report を模擬できる(テスト・自作出力のリファレンス) |
| `EspUsbDeviceHidOutputAdapter` | `ESP32KeyBridgeEspUsbDevice.h` | EspUsbDevice | **実装済み(実機検証待ち)** | あり | 複合 HID デバイス(keyboard + consumer + mouse)。keyboard = boot 6KRO、consumer = 16bit usage × 1、相対軸 → mouse report(int8 飽和 + 繰り越し、Pan は非対応で破棄)。各 report は変化時のみ送信 + busy 時は次 update で再送。LED output report 受信 = lock 正本になれる(kana 含む)。`connected()` = ホストに mount 中 |
| BLE HID 出力 | - | 専用 BLE ライブラリ(作成予定) | **構想(別ライブラリ完成待ち)** | あり(予定) | BLE HID peripheral。入力側と同じ専用 BLE ライブラリに載せる |
| UART 出力(イベント・ログ・文字) | - | なし | **構想** | なし | `writeText()` を直接受ける文字対応出力 |

## PC からどう見えるか(USB デバイス構成)

USB Device 出力は 1 種類で、常に**複合 HID デバイス**として PC に見えます。

| 出力アダプタ | PC から見える構成 | 送る report |
|---|---|---|
| `EspUsbDeviceHidOutputAdapter` | 複合 HID デバイス(keyboard + consumer + mouse) | keyboard(boot 6KRO)/ consumer(16bit usage × 1)/ mouse(ボタン 8 + X/Y/Wheel、int8 飽和 + 繰り越し。Pan は boot mouse report に無く破棄) |

規則:

- **常に全部入りで見せる**方針です。使わない interface は report を送らないだけで USB 的に無害なので、キーボード単体版は用意しません(古い BIOS 向けの最小構成が必要なら実キーボードを直結すればよい、という割り切り)。keyboard interface は boot protocol 対応なので BIOS/UEFI でも動作します。将来 interface の無効化オプションが要る場合はコンストラクタ引数(既定値付き)で非破壊に追加できます。
- **interface 構成は `usbDevice.begin()` 時に確定**し、以後変更できません(追加・削除には再 enumerate = 抜き挿し相当が必要)。設定にマウスやメディアキーを後から足しても、descriptor は最初から揃っているので再列挙は起きません。
- VID / PID / 製品名などのデバイス素性は `EspUsbDeviceConfig` で(スタックはスケッチ所有)。
- **LED output report を受信**し、接続中は lock 状態の正本(の連鎖の候補)になります([DATA_MODEL.ja.md](DATA_MODEL.ja.md) の Lock 状態)。
- rollover(32 キー)report builder は core にありますが、v1 の USB Device 出力は boot keyboard を基本とします(NKRO descriptor は将来のアダプタ拡張)。

## update() 頻度と取りこぼし耐性

サンプリングの時間精度はアダプタが担保するのが原則です([CORE_DESIGN.ja.md](CORE_DESIGN.ja.md) の「時間・並行性の境界」)。`update()` の呼び出し間隔が影響するのは「反映(変換・出力)の遅れ」だけで、入力自体の取りこぼしは起きません — 例外は `GpioKeyInputAdapter` です。

| アダプタ | サンプリング機構 | loop が長時間ブロックしたとき |
|---|---|---|
| `GpioKeyInputAdapter` | `update()` 内で読む(**loop レート依存**) | **ブロック中より短い押下は取りこぼす**。ペダル・ボタンのような押下の長い入力専用の割り切り |
| `GpioMatrixInputAdapter` | 専用タスク 1kHz(スキャン・デバウンス・ゴースト判定込み。初回 `update()` で起動) | 取りこぼさない(押下状態はタスク側で確定済み。反映が遅れるだけ) |
| `RotaryEncoderInputAdapter` | PCNT ペリフェラルのハードウェア計数 | 取りこぼさない(カウンタが溜めている。反映が遅れるだけ) |
| `EspUsbHostKeyboardInputAdapter` / `EspUsbHostMouseInputAdapter` | EspUsbHost のタスクからコールバック受け(critical section 越しの共有状態) | 取りこぼさない(キーボードは状態スナップショット保持。マウスの移動量は合算して保持) |
| `ManualInputAdapter` | スケッチが直接 `press()`/`release()` | スケッチ次第(呼んだものはそのまま残る) |

出力側(`EspUsbDeviceHidOutputAdapter` 等)は状態スナップショット方式なので、`update()` が遅れても「最後の状態」が送られるだけで壊れません(タイピング・マクロの打鍵速度は update 間隔に比例して遅くなります)。

## 追加依存ライブラリ

core(`ESP32KeyBridge.h`)は依存ゼロ(純粋 C++)です。アダプタヘッダは**ヘッダのみ**なので、include したアダプタの分だけ依存ライブラリが必要になります(include しなければ不要)。

| アダプタヘッダ | 依存ライブラリ | 入手 | sketch.yaml での指定例 |
|---|---|---|---|
| `ESP32KeyBridgeEspUsbHost.h` | [EspUsbHost](https://github.com/tanakamasayuki/EspUsbHost) 2.2.0 以上(`onKeyboardState`) | Arduino Library Manager | `- EspUsbHost` |
| `ESP32KeyBridgeEspUsbDevice.h` | [EspUsbDevice](https://github.com/tanakamasayuki/EspUsbDevice) | Arduino Library Manager | `- EspUsbDevice` |
| `ESP32KeyBridgeGpio.h` | なし(Arduino GPIO のみ) | - | - |

- ボードサポートは arduino-esp32(examples は 3.3.10 の profile を同梱。ESP32-P4 の USB Device は HS 固定などの制約は [SwapCtrlCapsLock の README](../examples/practical/SwapCtrlCapsLock/README.ja.md) を参照)。
- リポジトリを隣に checkout して開発する場合は、sketch.yaml の `libraries:` に `- dir: ../../../../EspUsbHost` のようなディレクトリ参照も使えます。

## 共通規則(全アダプタ)

- スタック(EspUsbHost / EspUsbDevice 等)は**スケッチが所有**し、スタック自身の config で起動する。アダプタはコンストラクタで参照を受けるだけで **`begin()` を持たない**(購読は初回 `update()`。USB device の interface 登録のみアダプタ構築時)。
- 時間・割り込み・タスクはアダプタ内部に閉じ、`update()` の呼び出しコンテキストへスレッドセーフに引き渡す([CORE_DESIGN.ja.md](CORE_DESIGN.ja.md) の「時間・並行性の境界」)。
- EspUsbHost のコールバックは単一スロット(後から設定した方で上書き)なので、アダプタは内部ハブ経由でスタックごとに 1 購読を共有する。**スケッチ側で `onKeyboardState` / `onConsumerControl` / `onMouse` / `onDeviceDisconnected` を設定しないこと**(アダプタのイベントが止まります)。イベント単位の `onKeyboard` はアダプタが使わないので、スケッチから自由に使えます。
- 出力アダプタは表現できるものだけを出し、他は黙って捨てる(virtual キーは出力に出ない)。
