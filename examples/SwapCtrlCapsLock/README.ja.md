# SwapCtrlCapsLock

ユースケース 1([../../docs/USE_CASES.ja.md](../../docs/USE_CASES.ja.md))の最小 example です。キーボードと OS の設定を変更せずに、ブリッジの global remap 2 エントリで Ctrl と CapsLock を入れ替えます。

```cpp
config.global.remap(kCapsLock, kLeftCtrl);
config.global.remap(kLeftCtrl, kCapsLock);
```

remap は 1 段の lookup で連鎖しないため、2 エントリの登録がそのまま「入れ替え」になります。変換テーブル・レイアウト・Lock 状態は一切関与せず、長押し・同時押し・ホスト側オートリピートは保たれます。

この example は実ハードウェアの代わりに `esp32keybridge::ManualInputAdapter` で押下を注入し、変換結果(`bridge.outputKeys()`)を Serial に表示します。実構成では入力を USB Host keyboard adapter、出力を USB Device keyboard adapter に差し替えます。
