# Examples

examples は「実用例」と「使い方の例」を明示的に分けています。

- `practical/`: 実用例。実ハードウェア構成でそのまま書き込んで使うスケッチ。中身は目的の設定 + `update()` だけで、Serial 表示や押下注入は含みません。
- `usage/`: 使い方の例。`ManualInputAdapter` / `ManualOutputAdapter` で押下を注入し、core の挙動(変換結果や report バイト列)を Serial で確認する API デモです。

## Practical Examples

実装順 7 の実機アダプタ(USB Host / USB Device など)が載った時点で追加します。最初の候補は SwapCtrlCapsLock の実用版(USB Host keyboard 入力 + USB Device keyboard 出力 + remap 2 エントリのみ)です。

## Usage Examples

- `FootSwitchLayer`: フットスイッチを virtual キーに remap し、踏んでいる間だけのレイヤーで J/K を Down/Up にする(UC7)。
- `LayoutConversion`: US 刻印キーボードを ja_jp ホストで刻印どおり打てるようにする(Shift 消費/合成・ショートカット素通し・on/off 切替)(UC5)。
- `MediaKeyRemap`: 種別またぎ remap で F13/F14 を音量 Up / Play-Pause にし、consumer report の組み立てを見る(UC9)。
- `MergeKeyboards`: 複数入力の和集合統合。片側の Shift を反対側のキーに効かせ、切断時の全 release を見る(UC2)。
- `SwapCtrlCapsLock`: global remap 2 エントリで Ctrl と CapsLock を入れ替える(UC1)。
- `TextMacroTyping`: 文字列マクロと `typeText()` の打鍵展開を 1 update 1 フェーズで見る(UC10)。

このほか、マウス相対値(UC11)の usage example を今後追加します。
