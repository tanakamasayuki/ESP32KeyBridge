# ESP32KeyBridge

ESP32KeyBridge は、ESP32 シリーズで複数の入力デバイスから受け取ったキー入力を共通形式に変換し、任意の出力へ橋渡しするための Arduino 向けライブラリです。

現在は設計初期段階です。[docs/REQUIREMENTS.ja.md](docs/REQUIREMENTS.ja.md) の要件を元に、入力、変換、出力を分離したライブラリとして段階的に実装します。

## 目的

- GPIO、USB Host、Bluetooth、UART、I2C、SPI、ネットワーク、赤外線などの入力 adapter からの入力を共通イベントへ変換する。
- キーコード変換、レイヤー、マクロ、Consumer Key などの変換処理を入力デバイスや出力デバイスから分離する。
- USB Device、Bluetooth、UART、ネットワーク、GPIO などの出力 adapter へ同じ変換結果を送れるようにする。
- core は特定の入出力、保存先、設定 UI に依存しない構成にする。
- core は Arduino / ESP-IDF などプラットフォーム固有機能に依存しない移植可能な純粋 C++ とし、ハードウェア依存は adapter 層に閉じ込める。

## 想定スコープ

初期実装では、すべての入出力を一度に実装しません。まずは小さい自動テストで固定できる core API から始めます。

- 共通キーイベント型。
- 入力 adapter、変換 processor、出力 adapter の境界。
- GPIO matrix などの単純な入力。
- USB HID Keyboard / Consumer Control などの代表的な出力。
- キーマップ、レイヤー、キー入れ替えなどの基本変換。
- USB HID、複数キーボード統合、WebSerial 設定画面などの複雑な構成は adapter と examples 側で扱う。

中間データ表現(種別 + 値のキー、継続 / 単発の 2 大分類、lock 状態、レイアウト変換)は [docs/DATA_MODEL.ja.md](docs/DATA_MODEL.ja.md) で定義します。代表ユースケースの検証は [docs/USE_CASES.ja.md](docs/USE_CASES.ja.md)、設計決定の台帳は [docs/DECISIONS.ja.md](docs/DECISIONS.ja.md) にあります。対応予定の詳細は [docs/REQUIREMENTS.ja.md](docs/REQUIREMENTS.ja.md)、[docs/CORE_DESIGN.ja.md](docs/CORE_DESIGN.ja.md)、[docs/API_SKETCHES.ja.md](docs/API_SKETCHES.ja.md)、[docs/CONFIGURATION.ja.md](docs/CONFIGURATION.ja.md)、[docs/DEVELOPMENT_PLAN.ja.md](docs/DEVELOPMENT_PLAN.ja.md) を参照してください。

## 対応環境

- Arduino-ESP32
- ESP32 / ESP32-S2 / ESP32-S3 / ESP32-C3 / ESP32-C6 / ESP32-H2 / ESP32-P4

各 SoC で利用できる USB、Bluetooth、GPIO などの機能差は、adapter 層で扱います。USB adapter の実装では `EspUsbHost` や `EspUsbDevice` を利用できますが、core はそれらに依存しません。

## テスト

テスト構造は他の ESP32 ライブラリプロジェクトと同じ方針で、`tests/` 以下に仕様、unit、hardware、manual の確認を分けて置きます。

```sh
cd tests
uv run pytest
```

初期段階では、実装済み core API の unit test と build-only smoke test から追加します。詳しくは [tests/README.ja.md](tests/README.ja.md) と [tests/TEST_PLAN.ja.md](tests/TEST_PLAN.ja.md) を参照してください。

## Examples

現在の example は [examples/README.ja.md](examples/README.ja.md) にまとめています。

## リリース

リリース、バージョン更新、GitHub Actions は全プロジェクト共通の運用に従います。このリポジトリ固有の workflow をコピーして個別カスタマイズする前提にはしません。

リリース前確認項目は [docs/RELEASE_CHECKLIST.ja.md](docs/RELEASE_CHECKLIST.ja.md) にまとめます。
