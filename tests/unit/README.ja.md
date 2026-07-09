# Unit Tests

このディレクトリには、ハードウェアなしで実行できる core のテストを置きます。

`core_smoke/` は `lang-ship:host:host` profile を使い、Arduino CLI 経由で host 向けにコンパイルして実行します。pytest から環境の `g++` を直接呼ばず、Arduino ライブラリとしての include / build 経路も確認します。

対象候補:

- 共通キーイベント。
- keymap と usage / keycode 変換。
- layer 切り替え。
- macro 実行。
- 設定 object の適用。
