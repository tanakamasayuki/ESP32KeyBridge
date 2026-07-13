# Examples

旧実装の examples は、ゼロベース仕様の確定に伴い削除しました(参照点: commit `4d2d48151c62`)。[../docs/USE_CASES.ja.md](../docs/USE_CASES.ja.md) のユースケースに対応する example を、実装の進行([../docs/DEVELOPMENT_PLAN.ja.md](../docs/DEVELOPMENT_PLAN.ja.md) の実装順)に合わせて再作成します。

## Core Examples

- `SwapCtrlCapsLock`: global remap 2 エントリで Ctrl と CapsLock を入れ替える(UC1)。

## 追加予定

- 複数入力の統合(UC2)
- GPIO 単独キー / フットスイッチ(UC7)
- メディアキー remap(UC9)
- レイヤー(Fn 相当の virtual キー)
- 文字列マクロ(UC10)
- レイアウト変換(UC5)
- USB Host / USB Device adapter を使う実機構成
