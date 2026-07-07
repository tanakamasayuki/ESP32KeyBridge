# SimpleMacro

単純な macro の最小例です。

この example は実 USB adapter を使いません。virtual input が `Fn1` を押した状態を作り、global transform で `Fn1` を `LeftCtrl + A + B` の key state に展開します。

現在の macro は、まず state 変換として実装している最小機能です。押下順、release 順、delay、文字列入力などの event 的な macro は今後検討します。
出力は serial に key state を名前で表示します。
