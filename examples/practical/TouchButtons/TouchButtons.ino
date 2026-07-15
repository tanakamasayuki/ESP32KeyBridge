// How to write your own input adapter, using the ESP32-S3's built-in
// capacitive touch pads as the example hardware. Touch a pad and a media key
// is sent to the PC. No external library and no extra parts: a short wire
// (or a piece of foil, or just the pin) on each touch GPIO is enough.
//
// The point of this example is the pattern, not the touch sensor. An input
// adapter is any class that:
//   1) subclasses esp32keybridge::InputAdapter,
//   2) reads its hardware in update(), and
//   3) returns the currently-pressed keys from keys().
// That is the whole required contract. connected() (default: always
// present), setLockState() (default: ignore), and takeAxisDelta() (default:
// no movement) are optional overrides. Read your I2C expander, a sensor, a
// network packet, anything, in update(); press/release keys in an internal
// KeySet; hand it back from keys(). The bridge does the rest (merge, remap,
// layers, output).
//
// Hardware: one ESP32-S3 (its single USB OTG port is the keyboard device
// plugged into the PC). Touch GPIOs 4/5/6 (adjust to your board).

#include <ESP32KeyBridge.h>
#include <ESP32KeyBridgeEspUsbDevice.h>
#include <EspUsbDevice.h>

// --- The custom input adapter --------------------------------------------

class TouchButtons : public esp32keybridge::InputAdapter
{
public:
  static constexpr size_t MaxPads = 8;

  // Register a touch pad as a key. Call before the first bridge update().
  // Any Key kind works (keyboard, consumer, mouse button, virtual).
  bool addPad(uint8_t pin, esp32keybridge::Key key)
  {
    if (padCount_ >= MaxPads)
    {
      return false;
    }
    pads_[padCount_].pin = pin;
    pads_[padCount_].key = key;
    ++padCount_;
    return true;
  }

  // The one method that does the work: read the hardware and update the
  // pressed-key set. Called once per bridge.update().
  void update() override
  {
    if (!calibrated_)
    {
      // First run: sample each pad untouched to learn its baseline. Doing
      // this lazily in update() (not the constructor) is the usual adapter
      // pattern - the peripheral is guaranteed ready by the time the bridge
      // starts calling update().
      for (size_t i = 0; i < padCount_; ++i)
      {
        uint32_t sum = 0;
        for (int s = 0; s < 16; ++s)
        {
          sum += touchRead(pads_[i].pin);
        }
        pads_[i].baseline = sum / 16;
      }
      calibrated_ = true;
    }

    for (size_t i = 0; i < padCount_; ++i)
    {
      const uint32_t value = touchRead(pads_[i].pin);
      // On the ESP32-S3 touchRead() rises when a pad is touched (the classic
      // ESP32 falls instead). Hysteresis - press well above the baseline,
      // release closer to it - keeps a value hovering at the edge from
      // chattering. Tune the factors if pads are too eager or too dull;
      // print `value` here to see raw numbers while calibrating.
      if (!pads_[i].pressed && value > pads_[i].baseline * 7 / 5) // > 1.4x
      {
        pads_[i].pressed = true;
        keys_.press(pads_[i].key);
      }
      else if (pads_[i].pressed && value < pads_[i].baseline * 6 / 5) // < 1.2x
      {
        pads_[i].pressed = false;
        keys_.release(pads_[i].key);
      }
    }
  }

  // Hand the current presses to the bridge. That is all the bridge reads.
  const esp32keybridge::KeySet &keys() const override { return keys_; }

private:
  struct Pad
  {
    uint8_t pin = 0;
    esp32keybridge::Key key;
    uint32_t baseline = 0;
    bool pressed = false;
  };

  esp32keybridge::KeySet keys_;
  Pad pads_[MaxPads];
  size_t padCount_ = 0;
  bool calibrated_ = false;
};

// --- Using it, exactly like a built-in adapter ---------------------------

EspUsbDevice usbDevice; // PC side

esp32keybridge::ESP32KeyBridge bridge;
TouchButtons pads;
esp32keybridge::EspUsbDeviceHidOutputAdapter pc(usbDevice);

void setup()
{
  // 1) Start the hardware.
  EspUsbDeviceConfig deviceConfig;
  deviceConfig.product = "TouchButtons";
  usbDevice.begin(deviceConfig);

  // 2) Describe the pads and wire the bridge - a touch media strip. Use
  //    touch-capable GPIOs (1-14 on the ESP32-S3).
  pads.addPad(GPIO_NUM_4, esp32keybridge::ConsumerUsage::VolumeDecrement);
  pads.addPad(GPIO_NUM_5, esp32keybridge::ConsumerUsage::PlayPause);
  pads.addPad(GPIO_NUM_6, esp32keybridge::ConsumerUsage::VolumeIncrement);
  bridge.addInput(pads);
  bridge.addOutput(pc);

  // 3) No transform needed: the pad keys pass straight through. (A config
  //    could still remap or layer them, since this is an ordinary input.)
  esp32keybridge::ESP32KeyBridgeConfig config;
  bridge.applyConfig(config);
}

void loop()
{
  bridge.update();
  delay(1);
}
