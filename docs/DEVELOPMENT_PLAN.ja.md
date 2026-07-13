# 開発計画

この文書は、`ESP32KeyBridge` の現在の方針と次に実装する範囲をまとめます。時系列ログではなく、リリース判断や次作業に必要な状態だけを残します。

## 基本方針

仕様はユースケース駆動のゼロベース検証で確定しました。データモデルは [DATA_MODEL.ja.md](DATA_MODEL.ja.md)、モジュール境界は [CORE_DESIGN.ja.md](CORE_DESIGN.ja.md)、決定の台帳は [DECISIONS.ja.md](DECISIONS.ja.md) を参照してください。

core は特定のトランスポート、保存先、設定 UI に依存しません。USB / BLE / GPIO などの依存はアダプタ側に閉じ込めます(アダプタ層 = HAL)。

## 現在のマイルストーン: 確定仕様の実装

### 旧実装の扱い

ゼロベース検討前の暫定実装は、差分改修ではなく削除して新規に実装し直します(語彙・API が変わるため。ハイブリッド状態を作らない)。

- **旧実装の参照点: commit `4d2d48151c62`**(新仕様ドキュメントと旧 `src/` / `examples/` が両方揃っている最後の地点)。
- 旧実装から部品単位で移植するもの: HID report builder 群(6KRO / rollover / consumer / pointer)、Recording 系アダプタのパターン、押下集合の固定長ストレージ、`EspUsbHostKeyboardInputAdapter` の構造。
- `tests/` のインフラ(pytest ハーネス、sketch.yaml、`.env` のポート設定、peer 配線)は残し、テストの中身だけ実装順に合わせて新規作成する。
- `examples/` は削除し、[USE_CASES.ja.md](USE_CASES.ja.md) のユースケースに対応する形で機能が載るたびに再作成する。

### 実装順

1. **キー表現と状態系の基盤**: 種別 + 値のキー、入力ごとの押下集合、和集合統合、切断クリア、modifier 正規化。host unit test で固定。
2. **キー変換**: remap(1 段・種別またぎ)、無効化、virtual キー、レイヤー(press 時確定)。
3. **Lock / LED**: 内部シャドウ、正本の連鎖(lock 報告出力 → 終端ホストモード)、全入力への通知。
4. **単発系**: 文字ストリーム、アクション列キュー(アトミック性・修飾退避/復元・あふれ)、相対値(合算・繰り越し・反転/倍率)、ホストレイアウト記述(`en_us` / `ja_jp`)と制御文字。
5. **レイアウト変換**: (キー,Shift)→(キー,Shift) 表、Shift 抑制・合成、on/off 切替、修飾コンボ素通し。
6. **マウス**: mouse_button 種別、相対軸、マウス report builder。
7. **アダプタと example**: USB Host keyboard/mouse 入力、USB device(単体 / 複合)出力、GPIO マトリクス、BLE、文字列デバイス(シリアル)。既存の examples / tests を新 API へ移行。

各段階で unit test(host 実行)を先に固定し、実機依存は build-only → single → peer の順で追加します([../tests/TEST_PLAN.ja.md](../tests/TEST_PLAN.ja.md))。

## 対応予定の入出力

- 入力: USB Host(keyboard / mouse / barcode)、GPIO(単独キー・マトリクス・エンコーダ・フットスイッチ)、BLE keyboard / mouse、UART / シリアル(イベント・文字列)、I2C / SPI エキスパンダ、IR。
- 出力: USB Device(keyboard / mouse / consumer、複合)、BLE HID、UART(イベント・ログ・文字)、GPIO。
- 将来メモ(v1 対象外): 無線ペア / TCP トランスポート、出力切替(KVM 的)、絶対座標。詳細は [DECISIONS.ja.md](DECISIONS.ja.md)。

## 設計判断(要約)

- ブリッジは解釈しない / 正本は解釈者が持つ / core は時間を持たない / 移植可能な純粋 C++(依存境界は [CORE_DESIGN.ja.md](CORE_DESIGN.ja.md))。
- トラフィックは継続(状態系)と単発(イベント系)の 2 大分類で扱う。
- 中間の押下集合に出力形式の制限(6KRO 等)を持ち込まない。
- 原理的な不可能性([DATA_MODEL.ja.md](DATA_MODEL.ja.md) 末尾)は隠さず明記する。

## テスト方針

core の変換処理は host 上の unit test で固定します。ハードウェアアダプタは build-only smoke、実機自動テスト、manual test を分けて管理します。詳細は [../tests/TEST_PLAN.ja.md](../tests/TEST_PLAN.ja.md) を参照してください。

## リリース運用

リリース、バージョン更新、GitHub Actions は全プロジェクト共通の運用に従います。このリポジトリ内に共通 Action をコピーして個別カスタマイズする方針は取りません。
