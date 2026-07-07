# LayoutConversion

layout conversion の最小例です。

この example は実 USB adapter を使いません。virtual input が `A` を押した状態を作り、layout conversion で `A -> B` に変換します。

現在の layout conversion は、まず汎用的な key mapping table として実装しています。US/JA などの実 layout 定義、文字意味ベースの decode / encode、修飾キー付き記号変換は今後検討します。

