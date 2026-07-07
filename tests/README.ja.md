# テスト

このディレクトリには `ESP32KeyBridge` のテスト仕様と自動テストを置きます。

初期段階では、ハードウェアに依存しない core の unit test を優先します。GPIO、USB、Bluetooth などの adapter は、build-only、実機自動、manual を分けて追加します。

## 必要なもの

- `uv`
- Arduino CLI
- 対象ボード用の ESP32 board package
- 実機テストに使う ESP32 ボード

## 構成

- `unit/`: 共通キーイベント、keymap、layer、macro など、ホスト不要のテスト。
- `examples_compile/`: examples sketch の build-only smoke test。
- `hardware/`: pytest-embedded で実機に upload して確認する自動テスト。
- `manual/`: 配線、Host OS、Bluetooth pairing、目視確認など人の操作が必要なテスト。

## 実行

```sh
uv run pytest
uv run pytest unit/
uv run pytest examples_compile/
```

現在のカバレッジと追加予定は [TEST_PLAN.ja.md](TEST_PLAN.ja.md) を参照してください。

