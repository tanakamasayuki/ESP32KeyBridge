# MultiKeyboardMerge

複数の入力 keyboard state を 1 つの出力 keyboard state に統合する最小例です。

この example は実 USB adapter を使いません。左側の virtual keyboard が `LeftShift`、右側の virtual keyboard が `A` を押した状態を作り、`esp32keybridge::ESP32KeyBridge` が統合した state を serial に表示します。

期待する考え方:

```text
left input:  LeftShift
right input: A
output:      LeftShift + A
```

将来の USB Host adapter では、USB hub 経由の複数 keyboard 入力を同じ merge layer に接続します。

