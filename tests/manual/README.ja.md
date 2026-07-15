# 手動テスト

このディレクトリには、自動化しにくい確認手順を置きます。

## 複合 HID 出力の lock 正本(LED output report 受信)

peer テストでは確認できない項目です(EspUsbHost 2.2.0 が boot 宣言キーボードにしか LED を送れないため skip 中。実 PC の OS は report ID を扱えるので手動で確認する)。

- 必要なもの: ESP32-S3 1台、実 PC(Windows / Linux / macOS いずれか)、実キーボード。
- example: `FootSwitch`(または `SerialTextTyper`)をそのまま書き込み、S3 の USB OTG ポートを PC へ挿す。
- 手順: PC につながっている実キーボードで NumLock を押して PC の lock 状態を変える。
- 期待: PC が複合デバイスへ LED output report を送り、ブリッジの lock 正本がホスト追随になる。確認は `SerialTextTyper` に数字を含む文字列を送り、NumLock の状態に関係なく正しい文字が出ること(タイピングエンジンが lock 状態を参照して補正するのは CapsLock。NumLock はテンキー未使用のため影響しないので、確認は CapsLock 側が簡単: PC 側で CapsLock を ON にして `abc` を送信し、`abc` と入力される = CapsLock 補正が効いている = LED 受信が機能している)。

## 初期候補:

- GPIO matrix の配線確認。
- 実 USB keyboard / barcode reader を USB Host adapter で読み取れること。
- USB HID keyboard として Host OS に認識されること。
- USB hub 経由の複数 keyboard 入力を統合できること。
- WebSerial reference UI から設定を読み書きできること。
- BLE HID pairing と再接続。
- 複数入力または複数出力を同時に使う構成の実操作確認。

手順を追加するときは、必要なボード、配線、使用する example、期待される観測結果を明記します。

manual 用の ESP32-S3 は常時接続前提ではありません。`.env` では既存プロジェクトと同じ `TEST_SERIAL_PORT_ESP32S3` を使います。

