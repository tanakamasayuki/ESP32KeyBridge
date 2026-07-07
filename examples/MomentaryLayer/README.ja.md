# MomentaryLayer

momentary layer の最小例です。

この example は実 USB adapter を使いません。virtual input が `Fn1` と `A` を押した状態を作り、`Fn1` が押されている間だけ `A -> B` の layer remap を適用します。`Fn1` 自体は出力 state には含めません。

期待する考え方:

```text
input:  Fn1 + A
output: B
```

出力は serial に key state を名前で表示します。
