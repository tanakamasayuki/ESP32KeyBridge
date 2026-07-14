// Lets a US-engraved keyboard (or barcode reader) type exactly what its
// keycaps say on a PC that stays configured for the Japanese (ja_jp)
// layout. No settings are changed on the PC.
//
// Hardware: one ESP32-P4.
//   - USB Host port: plug the US keyboard / barcode reader in here.
//   - USB Device port: plug this into the PC.
//
// NOTE: the USB adapters are build-only mocks until implementation step 7.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbHost.h>
#include <EspUsbDevice.h>

EspUsbHost usbHost;     // US keyboard side
EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostKeyboardInputAdapter keyboard(usbHost);
esp32keybridge::EspUsbDeviceKeyboardOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware.
  usbHost.begin();

  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "UsKeyboardOnJapanesePc";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge. The PC is set to ja_jp; the keyboard is engraved as
  //    en_us. Printable keys are re-encoded so the typed character matches
  //    the keycap, with Shift consumed / synthesized as needed. Ctrl/Alt/GUI
  //    shortcuts pass through unconverted.
  bridge.addInput(keyboard);
  bridge.addOutput(pc);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.hostLayout = esp32keybridge::HostLayout::jaJp();
  config.convertLayout(0, esp32keybridge::HostLayout::enUs());
  // BIOS-like environments interpret keys themselves: this key toggles the
  // conversion off/on at runtime. Pause exists on most keyboards and is
  // rarely used.
  config.layoutConversionToggle = esp32keybridge::KeyboardUsage::Pause;
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
