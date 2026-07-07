# テスト

このディレクトリには `ESP32KeyBridge` のテスト仕様と自動テストを置きます。

初期段階では、ハードウェアに依存しない core の unit test を優先します。GPIO、USB、Bluetooth などの adapter は、build-only、S3 1台、S3 peer、manual を分けて追加します。

## 必要なもの

- `uv`
- Arduino CLI
- 対象ボード用の ESP32 board package
- 常時接続の ESP32-S3 peer 2台
- 必要時に接続する manual / single 用 ESP32-S3

## 構成

- `unit/`: 共通キーイベント、keymap、layer、macro など、ホスト不要のテスト。
- `examples_compile/`: examples sketch の build-only smoke test。
- `single/`: ESP32-S3 1台で Arduino runtime 上の core / reference example を確認する自動テスト。
- `peer/`: 常時接続された ESP32-S3 2台で USB adapter 境界を確認する自動テスト。
- `manual/`: 配線、Host OS、Bluetooth pairing、目視確認など人の操作が必要なテスト。

## 実行

```sh
uv run pytest
uv run pytest unit/
uv run pytest examples_compile/
uv run --env-file .env pytest single/
uv run --env-file .env pytest peer/
```

現在のカバレッジと追加予定は [TEST_PLAN.ja.md](TEST_PLAN.ja.md) を参照してください。
