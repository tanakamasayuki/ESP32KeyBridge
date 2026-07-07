# PerInputRemap

入力デバイスごとの remap と、merge 後の global remap の違いを示す example です。

この example は実 USB adapter を使いません。

- input 0: 通常 keyboard として `Enter` を押す。
- input 1: barcode scanner 相当として `Enter` と `A` を押す。
- per-input 設定: input 1 だけ `Enter -> Tab`。
- global 設定: 統合後に `A -> B`。

期待する考え方:

```text
input 0 Enter -> output Enter
input 1 Enter -> output Tab
input 1 A     -> output B
```

同じ `Enter` でも、どの入力デバイスから来たかによって処理を変えられます。
出力は serial に key state を名前で表示します。
