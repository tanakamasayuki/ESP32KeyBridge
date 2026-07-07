# リリースチェックリスト

`ESP32KeyBridge` のリリース前に確認する項目です。リリース、バージョン更新、GitHub Actions は全プロジェクト共通の運用に従い、このリポジトリ固有の workflow は編集しません。

## 事前確認

- `README.ja.md` / `README.md` のスコープが実装済み範囲と合っている。
- `docs/REQUIREMENTS.ja.md` の要件と実装済み範囲の差分が説明できる。
- `docs/CONFIGURATION.ja.md` の core 非依存方針と WebSerial / CDC reference example 方針が実装と合っている。
- `docs/DEVELOPMENT_PLAN.ja.md` は現在の方針と残作業を示している。
- `tests/README.ja.md` / `tests/TEST_PLAN.ja.md` は現状のテスト構成と合っている。

## メタデータ

- `library.properties` の `name`、`sentence`、`paragraph`、`architectures`、`includes` が公開内容と合っている。
- `keywords.txt` に公開 class、method、constant が入っている。
- version と changelog は共通 bump / release 運用に従って更新する。

## テスト

```sh
cd tests
uv run pytest
```

実機が必要なテストは `tests/manual/README.ja.md` に手順を残し、自動テストの合格条件と混ぜないようにします。

## リリース作業

- 共通 bump / release 手順で version と changelog を更新する。
- 最終 diff に build artifact、cache、local profile 固有の変更が入っていないことを確認する。
- tag / GitHub release は共通 Action の前提に従って作成する。
