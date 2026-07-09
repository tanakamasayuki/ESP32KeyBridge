# UsbHostKeyboardInput

`EspUsbHost` で受け取った USB HID keyboard event を `esp32keybridge::InputAdapter` として扱う例です。

この example は `esp32keybridge::EspUsbHostKeyboardInputAdapter` を使います。adapter は `EspUsbHostKeyboardEvent::keycode` を `esp32keybridge::HidUsage` として受け取り、指定した `esp32keybridge::KeyboardLayout` で `esp32keybridge::KeySymbol` へ decode して `esp32keybridge::InputState` に反映します。

`esp32keybridge::EspUsbHostKeyboardInputAdapter` は USB Host 側では `EspUsbHost::setKeyboardLayout()` や `EspUsbHostKeyboardEvent::ascii` を使いません。入力レイアウトは bridge 側で `input.setLayout(esp32keybridge::KeyboardLayout::us())` のように指定します。

US keyboard を日本語 Windows 向けに出すような変換では、単純な key-to-key remap だけでは不十分です。HID usage を入力レイアウト上の意味へ decode し、その意味を出力レイアウト向けの HID usage へ encode する table が必要になります。この example は USB Host 入力 adapter の最小形に留め、出力 layout encode を含む bridge example は別途追加します。

core は `EspUsbHost` に依存しません。`EspUsbHost` 依存は `src/adapters/EspUsbHostKeyboardInputAdapter.h` を include した場合だけ必要になります。
