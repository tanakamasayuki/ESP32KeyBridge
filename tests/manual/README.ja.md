# 手動テスト

このディレクトリには、自動化しにくい確認手順を置きます。

初期候補:

- GPIO matrix の配線確認。
- 実 USB keyboard / barcode reader を USB Host adapter で読み取れること。
- USB HID keyboard として Host OS に認識されること。
- USB hub 経由の複数 keyboard 入力を統合できること。
- WebSerial reference UI から設定を読み書きできること。
- BLE HID pairing と再接続。
- 複数入力または複数出力を同時に使う構成の実操作確認。

手順を追加するときは、必要なボード、配線、使用する example、期待される観測結果を明記します。

manual 用の ESP32-S3 は常時接続前提ではありません。`.env` では既存プロジェクトと同じ `TEST_SERIAL_PORT_ESP32S3` を使います。

