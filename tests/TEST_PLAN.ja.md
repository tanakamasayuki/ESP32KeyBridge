# テスト計画

## テスト方針

`ESP32KeyBridge` は入力、変換、出力を分離するライブラリです。テストも同じ分け方にします。

**unit test** は、共通 event / state と変換処理をホスト上で検証します。キー変換、入力統合、layer、macro、設定 object の適用など、ハードウェアが不要な仕様はここで固定します。

**build-only test** は、Arduino examples が対象ボード向けにコンパイルできることを確認します。

**hardware test** は、pytest-embedded などで実機へ upload し、serial assertion で確認できる範囲だけを自動化します。

**manual test** は、配線、Host OS の認識、Bluetooth pairing、目視や実操作が検証の本質に含まれる場合だけに使います。

```text
tests/
  unit/              自動 - 共通キーイベントと変換処理。
  examples_compile/  自動 - examples sketch の build-only smoke。
  hardware/          自動 - 実機 upload と serial assertion。
  manual/            手動 - OS 認識、pairing、配線、目視確認。
```

## 初期カバレッジ計画

| 機能 | unit | examples_compile | hardware | manual |
|------|------|------------------|----------|--------|
| 共通キーイベント | 予定 | | | |
| 入力ごとの state | 予定 | | | |
| 複数入力 merge | 予定 | | | |
| input / processor / output interface | 予定 | | | |
| キーコード変換 | 予定 | | | |
| キー無効化 / 入れ替え | 予定 | | | |
| layer 基本動作 | 予定 | | | |
| macro 基本動作 | 予定 | | | |
| 設定 object の適用 | 予定 | | | |
| 設定の保存 / 読み込み reference example | | 予定 | 予定 | storage 差分確認 |
| CDC serial 設定 reference example | | 予定 | 予定 | WebSerial 確認 |
| USB HID + CDC 複合デバイス example | | 予定 | 予定 | Host OS 認識 |
| GPIO matrix input | 予定 | 予定 | 予定 | 配線確認 |
| USB HID keyboard output | 予定 | 予定 | 予定 | Host OS 認識 |
| BLE HID output | 予定 | 予定 | 予定 | pairing 確認 |

## 追加時の判断基準

- 変換仕様は、できるだけ `unit/` で先に固定する。
- adapter は、まず build-only test を追加して API の破壊を検出する。
- 実機自動テストは、serial log だけで期待値を assert できる範囲に限定する。
- 人の判断が必要な確認は `manual/` に隔離し、自動テストの合格条件に混ぜない。
