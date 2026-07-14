#pragma once

#include <ESP32KeyBridge.h>

// BLE input adapters.
//
// MOCK: this header currently contains build-only skeletons that fix the
// sketch-facing API. The underlying BLE library is not decided yet; only
// the API declared here is settled.

namespace esp32keybridge
{

// BLE keyboard input (HID over GATT). Scanning, pairing, and reconnection
// live inside the adapter; the BLE stack starts lazily on the first
// update(). Losing the connection drops all keys, so a battery dying
// mid-press never leaves stuck keys. Whether the stack is owned here or by
// the sketch follows the BLE library decision.
class BleKeyboardInputAdapter : public InputAdapter
{
public:
  void update() override {}
  const KeySet &keys() const override { return keys_; }

  // True while a keyboard is connected.
  bool connected() const override { return false; }

private:
  KeySet keys_;
};

} // namespace esp32keybridge
