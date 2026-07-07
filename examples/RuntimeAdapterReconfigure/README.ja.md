# RuntimeAdapterReconfigure

実行中に input / output adapter の登録を差し替える例です。

この example は実 USB / GPIO adapter を使いません。起動直後は `inputA -> outputA` として `A` を出力し、5 秒後に `bridge.clearInputs()` / `bridge.clearOutputs()` を呼んで `inputB -> outputB` へ差し替えます。

重要な点:

- `clearInputs()` / `clearOutputs()` は bridge の登録を外す API です。
- adapter object 自体の state や寿命はユーザー側で管理します。
- 実 USB / BLE / GPIO adapter の再構成でも同じ考え方を使います。

