# Single-Board Tests

ESP32-S3 1台で実行する自動テストです。

目的は、core が Arduino runtime 上で動くことを serial assertion で確認することです。USB Host / USB Device adapter の実 USB 境界は `tests/peer/` に置きます。

実行例:

```sh
uv run --env-file .env pytest single/
```

`sketch.yaml` の default profile を使うため、通常は `--profile` を指定しません。

