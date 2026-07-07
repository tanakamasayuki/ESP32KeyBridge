# RuntimeConfigApply

実行中に外部から受け取った設定 object を `esp32keybridge::ESP32KeyBridge` に適用する例です。

この example は実 WebSerial や storage を使いません。`ExampleConfigService` が 5 秒後に新しい `esp32keybridge::ESP32KeyBridgeConfig` を返し、`A -> B` の remap を適用します。

重要な点:

- 設定の取得方法は core の外側にある。
- 設定の保存先も core の外側にある。
- core は `validateConfig()` と `applyConfig()` の入口だけを提供する。

WebSerial、UART、BLE、NVS、LittleFS などを使う場合も、最終的には `esp32keybridge::ESP32KeyBridgeConfig` を作って同じ流れで適用します。

