# UsbHostKeyboardInput

`EspUsbHost` で受け取った USB HID keyboard event を `esp32keybridge::InputAdapter` として扱う例です。

この example は adapter 実装の最小形を示すためのものです。`EspUsbHostKeyboardEvent::keycode` を `esp32keybridge::keyFromHidUsage()` で `esp32keybridge::Key` へ変換し、`esp32keybridge::InputState` に反映します。

core は `EspUsbHost` に依存しません。この依存は example 内の adapter に閉じ込めます。
