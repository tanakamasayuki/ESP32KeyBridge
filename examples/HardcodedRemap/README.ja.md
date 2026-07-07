# HardcodedRemap

外部設定なしで `esp32keybridge::ESP32KeyBridgeConfig` を C++ コードから構築する最小例です。

この example は実 USB adapter を使いません。virtual input が `CapsLock`、`A`、`Insert` を押した状態を作り、global transform で `CapsLock` を `LeftCtrl` へ変換し、`Insert` を無効化します。

出力は serial に現在の key state を表示します。

