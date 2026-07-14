# UsKeyboardOnJapanesePc

US 配列のキーボードやバーコードリーダーを、日本語(ja_jp)設定のままの PC で刻印どおり打ちたいとき用。OS 側の設定は変更しません。

- 印字キーを刻印(en_us)で文字に戻し、PC 側レイアウト(ja_jp)の打鍵に再エンコードします。必要な Shift は自動で合成・抑制されます(`'` → Shift+7 など)。
- 設定の意味: 刻印(`keyboardConfig.convertLayout(enUs)`)は**入力デバイスごと**の属性、`config.hostLayout = jaJp()` は「**PC が既にどう設定されているか**」の宣言です(PC 側は何も変更しません)。全出力は同じ押下集合を受けるため、宛先の解釈はブリッジに 1 つだけ持ちます。
- Ctrl/Alt/GUI を伴うショートカットは無変換で素通しします。
- **Pause キーで変換を on/off** できます。BIOS などブリッジと違う解釈をする環境では off にして素のキーを通します。
- 打てない文字は読み捨ててカウントされます(`layoutConvertFailCount()`)。

## ハードウェア構成(ESP32-P4)

USB を 2 系統(Host + Device)使うため ESP32-P4 が必要です。

- **USB Device(PC 側)= HS 固定**(arduino-esp32 3.3.10 の制約)。
- **USB Host(キーボード側)= FS 明示指定**。キーボードのコネクタが FS の CDC ピン(GPIO24/25)に配線されているボードでは、スケッチ内の PHY スワップ行のコメントアウトを外してください。

P4 の USB ポートの詳細、表記と実配線が違うボードの注意(M5Stack Tab5 の事例)、HS を Host に使う場合の制約は [SwapCtrlCapsLock/README.ja.md](../SwapCtrlCapsLock/README.ja.md) を参照してください。

> **注意**: USB アダプタは現在ビルド確認用のモックです(実装順 7 で実装されます)。
