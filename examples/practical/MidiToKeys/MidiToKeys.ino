// Turn a USB MIDI controller's pads/keys into keyboard shortcuts: hitting a
// pad presses a key on the PC. Mapped here to F13-F16 (function keys that
// apps let you bind to macros without clashing with anything), plus two
// media keys. The PC sees an ordinary keyboard, so no driver or MIDI
// software is needed.
//
// A MIDI Note On (velocity > 0) presses the mapped key and Note Off releases
// it, so a held pad holds the key. Only mapped notes do anything; any key
// kind is allowed, and you can remap or layer on top like any input.
//
// Note numbers differ between controllers (drum pads often start at note 36
// = C1). If a pad does the wrong thing, adjust the mapNote() numbers below.
//
// Hardware: one ESP32-P4 (two USB roles are needed, so single-USB chips like
// the ESP32-S3 cannot run this sketch alone).
//   - USB Device = HS port: plug this into the PC.
//   - USB Host = FS port: plug the MIDI controller in here.
// The P4 USB port details are documented in the SwapCtrlCapsLock example.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbHost.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbHost.h>
#include <EspUsbDevice.h>

EspUsbHost usbHost;     // MIDI controller side (FS)
EspUsbDevice usbDevice; // PC side (HS)

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbHostMidiInputAdapter midi(usbHost);
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware. The USB Device is fixed to the HS port, so the
  //    host explicitly takes the FS side.
  EspUsbHostConfig hostConfig;
  hostConfig.port = ESP_USB_HOST_PORT_FULL_SPEED;
  usbHost.begin(hostConfig);

  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "MidiToKeys";
  usbDevice.begin(deviceConfig);

  // 2) Map notes to keys, then wire the bridge. Four pads (notes 36-39) send
  //    F13-F16 for app macros; two more send media keys. Adjust the note
  //    numbers to your controller. setChannel(n) would narrow to one MIDI
  //    channel; the default listens to all.
  midi.mapNote(36, esp32keybridge::KeyboardUsage::F13);
  midi.mapNote(37, esp32keybridge::KeyboardUsage::F14);
  midi.mapNote(38, esp32keybridge::KeyboardUsage::F15);
  midi.mapNote(39, esp32keybridge::KeyboardUsage::F16);
  midi.mapNote(40, esp32keybridge::ConsumerUsage::PlayPause);
  midi.mapNote(41, esp32keybridge::ConsumerUsage::Mute);
  bridge.addInput(midi);
  bridge.addOutput(pc);

  // 3) No transform needed: the mapped keys pass straight through.
  esp32keybridge::ESP32KeyBridgeConfig config;
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
