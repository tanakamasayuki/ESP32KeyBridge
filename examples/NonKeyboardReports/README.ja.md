# NonKeyboardReports

keyboard 以外の domain を含む `esp32keybridge::InputState` から、HID report builder を使って keyboard / consumer / pointer button の report を作る例です。

この example は USB 出力を行いません。`esp32keybridge::buildHidKeyboardReport()`、`esp32keybridge::buildHidConsumerReport()`、`esp32keybridge::buildHidPointerReport()` の結果を Serial に表示します。

pointer axis のような値付き入力は `esp32keybridge::InputState` ではなく `esp32keybridge::InputValueEvent` として扱い、`esp32keybridge::HidPointerReport::apply()` で report に反映します。
