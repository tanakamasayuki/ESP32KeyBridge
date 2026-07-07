# EventInput

`esp32keybridge::InputEvent` を `esp32keybridge::EventInputAdapter` に投入して keyboard state を作る例です。

この example は実 USB / GPIO adapter を使いません。2 秒ごとに `A` の press / release を切り替え、`esp32keybridge::ESP32KeyBridge` の出力 state を serial に表示します。

将来の USB Host adapter、GPIO adapter、UART adapter などは、raw input を `esp32keybridge::InputEvent` に変換して同じ考え方で state に反映できます。

