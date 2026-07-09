# LayoutConversion

layout conversion の最小例です。

この example は実 USB adapter を使いません。virtual input が意味キーとしての `A` を押した状態を作り、`LayoutConfig` の key mapping table で `A -> B` に変換します。

USB HID usage と意味キーの decode / encode は `esp32keybridge::KeyboardLayout` が担当します。例えば FR layout の `esp32keybridge::HidUsage::Usage04` は `esp32keybridge::KeySymbol::Q` に decode され、US layout で出力すると `esp32keybridge::HidUsage::Usage14` に encode されます。

現在の `esp32keybridge::LayoutConfig` は、layout table そのものではなく、decode 後の `esp32keybridge::KeySymbol` に適用する汎用的な key mapping table として扱います。
出力は serial に key state を名前で表示します。
