// Types text received over the serial port into the PC as keystrokes.
// Useful for test automation, kiosk provisioning, or sending clipboard
// text to a machine you can only reach as a keyboard.
//
// Hardware: one ESP32-S3. Its USB OTG port is the keyboard device plugged
// into the PC, so the text arrives on the UART serial (pins or the
// board's USB-serial converter), line by line.
//
// There is no input adapter here: the text stream is a first-class input
// of the bridge (typeText / typeChar).
//
// NOTE: the USB adapter is a build-only mock until implementation step 7.

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbDevice.h>

EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

// Characters are typed as soon as they arrive; only an incomplete UTF-8
// sequence waits for its remaining bytes.
char pending[8];
size_t pendingLength = 0;

size_t utf8SequenceLength(uint8_t lead)
{
  if ((lead & 0xe0) == 0xc0)
  {
    return 2;
  }
  if ((lead & 0xf0) == 0xe0)
  {
    return 3;
  }
  if ((lead & 0xf8) == 0xf0)
  {
    return 4;
  }
  return 1; // ASCII (invalid leads pass through and are dropped + counted)
}

void setup()
{
  Serial.begin(115200);

  // 1) Start the hardware.
  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "SerialTextTyper";
  usbDevice.begin(deviceConfig);

  // 2) Wire the bridge. Characters are expanded into keystrokes with
  //    config.hostLayout (en_us by default; see README.ja.md to match a
  //    differently configured PC).
  bridge.addOutput(pc);
}

void loop()
{
  // Feed each completed UTF-8 sequence straight into the text stream.
  // Control characters use the standard table ('\n' becomes Enter);
  // characters the host layout cannot type are skipped and counted
  // (textEncodeFailCount()).
  //
  // Backpressure instead of a timeout: typing is much slower than 115200
  // baud, so when the text queue has no room left we stop reading and let
  // the bytes wait in the UART RX buffer. Nothing blocks, nothing is lost.
  while (Serial.available() > 0 && bridge.typeAvailable() > 0)
  {
    pending[pendingLength++] = (char)Serial.read();
    if (pendingLength >= utf8SequenceLength(static_cast<uint8_t>(pending[0])))
    {
      pending[pendingLength] = '\0';
      bridge.typeText(pending);
      pendingLength = 0;
    }
  }

  // The typing engine emits one phase per update, so the loop period sets
  // the typing pace (~1ms per phase here).
  bridge.update();
  delay(1);
}
