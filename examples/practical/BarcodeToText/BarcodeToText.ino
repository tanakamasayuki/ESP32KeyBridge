// Turns a USB HID barcode reader (the kind that "types" the barcode as
// keystrokes) into a stream of text on the serial port. Read the barcodes
// from a PC serial monitor, or feed them to another device — no keyboard
// focus or foreground app needed.
//
// Hardware: one ESP32-S3. The barcode reader plugs into the USB OTG port
// (read as a USB Host); the decoded text comes out Serial (the board's
// USB-serial, i.e. the PC-facing serial port). Any keyboard-mode HID
// scanner works.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeSerial.h>
#include <EspUsbHost.h>

EspUsbHost usbHost; // barcode reader side

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostKeyboardInputAdapter scanner(usbHost);
esp32keybridge::TextOutputAdapter text(Serial);

void setup()
{
  // 1) Start the hardware.
  Serial.begin(115200);
  usbHost.begin();

  // 2) Wire the bridge. The layout lives with the input: the scanner is
  //    engraved as en_us (what most scanners send), and convertLayout
  //    decodes its keystrokes to characters. The decoded text — including
  //    the scan-terminating Enter as a newline — flows to the text
  //    output. Set convertLayout(KeyboardLayout::jaJp()) if the scanner is
  //    configured for JIS.
  esp32keybridge::ESP32KeyBridgeConfig config;
  esp32keybridge::InputConfig &scannerConfig = config.addInputConfig();
  scannerConfig.convertLayout(esp32keybridge::KeyboardLayout::enUs());
  bridge.addInput(scanner, scannerConfig);
  bridge.addOutput(text);

  // 3) Build and apply the configuration.
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
