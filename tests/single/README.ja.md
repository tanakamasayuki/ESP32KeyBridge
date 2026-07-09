# Single-Board Tests

ESP32-S3 1台で実行する自動テストです。

目的は、core が Arduino runtime 上で動くことを serial assertion で確認することです。USB Host / USB Device adapter の実 USB 境界は `tests/peer/` に置きます。

## core_smoke

`core_smoke` は、Arduino runtime 上で以下を確認します。

- `esp32keybridge::ESP32KeyBridge` の `begin()` / `update()` が動く。
- `esp32keybridge::EventInputAdapter` から投入した event が state に反映される。
- global remap と disable が適用される。
- `bridge.mergedState()` と `bridge.outputState()` が期待どおりになる。
- 6KRO boot keyboard report は 6 個を超える key で `overflow` する。
- `esp32keybridge::HidKeyboardRolloverReport` は 32 code 上限内の複数 key を保持する。

実行例:

```sh
uv run --env-file .env pytest single/
```

`sketch.yaml` の default profile を使うため、通常は `--profile` を指定しません。
