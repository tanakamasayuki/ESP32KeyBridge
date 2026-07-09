#include <Arduino.h>
#include <ESP32KeyBridge.h>

esp32keybridge::EventInputAdapter input;
esp32keybridge::RecordingOutputAdapter output;
esp32keybridge::ESP32KeyBridge bridge;

void setup()
{
  Serial.begin(115200);
  delay(1500);

  Serial.println("TEST_BEGIN core_smoke");

  bridge.addInput(input);
  bridge.addOutput(output);

  esp32keybridge::ESP32KeyBridgeConfig config;
  config.global.remap(esp32keybridge::Key::CapsLock, esp32keybridge::Key::LeftCtrl);
  config.global.disable(esp32keybridge::Key::Insert);
  bridge.applyConfig(config);
  bridge.begin();

  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::CapsLock, true, millis()));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, true, millis()));
  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::Insert, true, millis()));
  bridge.update();

  const bool firstOk =
      output.writeCount() == 1 &&
      bridge.mergedState().isPressed(esp32keybridge::Key::CapsLock) &&
      bridge.mergedState().isPressed(esp32keybridge::Key::Insert) &&
      bridge.outputState().isPressed(esp32keybridge::Key::LeftCtrl) &&
      bridge.outputState().isPressed(esp32keybridge::Key::A) &&
      !bridge.outputState().isPressed(esp32keybridge::Key::CapsLock) &&
      !bridge.outputState().isPressed(esp32keybridge::Key::Insert) &&
      output.state().isPressed(esp32keybridge::Key::LeftCtrl) &&
      output.state().isPressed(esp32keybridge::Key::A);

  input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, false, millis()));
  bridge.update();

  const bool secondOk =
      output.writeCount() == 2 &&
      bridge.outputState().isPressed(esp32keybridge::Key::LeftCtrl) &&
      !bridge.outputState().isPressed(esp32keybridge::Key::A);

  esp32keybridge::InputState rolloverState;
  for (uint16_t usage = 4; usage < 4 + 8; ++usage)
  {
    rolloverState.press(esp32keybridge::keyboardCode(static_cast<esp32keybridge::Key>(usage)));
  }
  const esp32keybridge::HidKeyboardReport bootReport = esp32keybridge::buildHidKeyboardReport(rolloverState);
  const esp32keybridge::HidKeyboardRolloverReport rolloverReport =
      esp32keybridge::buildHidKeyboardRolloverReport(rolloverState);
  const bool rolloverOk =
      rolloverState.codeCount() == 8 &&
      bootReport.keyCount == esp32keybridge::HidKeyboardReport::MaxKeys &&
      bootReport.overflow &&
      rolloverReport.keyCount == 8 &&
      rolloverReport.keys[0] == 0x04 &&
      rolloverReport.keys[7] == 0x0b &&
      !rolloverReport.overflow;

  Serial.println("TEST_END");
  Serial.println(firstOk && secondOk && rolloverOk ? "OK" : "NG");
}

void loop()
{
}
