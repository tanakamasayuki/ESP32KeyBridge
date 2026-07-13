# Unit Tests

このディレクトリには、ハードウェアなしで実行できる core のテストを置きます。

`core_smoke/` は `lang-ship:host:host` profile を使い、Arduino CLI 経由で host 向けにコンパイルして実行します。pytest から環境の `g++` を直接呼ばず、Arduino ライブラリとしての include / build 経路も確認します。

対象(実装順に合わせて拡充):

- 種別 + 値のキー同一性と押下集合(実装済み)。
- 和集合統合、切断時の全 release、modifier 正規化(実装済み)。
- remap(種別またぎ)/ 無効化 / virtual キー / レイヤー(press 時確定)(実装済み)。
- LockState の正本規則。
- 文字ストリーム / アクション列 / 相対値とホストレイアウト記述。
- レイアウト変換。
