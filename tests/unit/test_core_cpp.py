import subprocess
import textwrap
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def test_core_cpp_behaviors(tmp_path):
    source = tmp_path / "core_behaviors.cpp"
    binary = tmp_path / "core_behaviors"
    source.write_text(
        textwrap.dedent(
            r'''
            #include <ESP32KeyBridge.h>
            #include <cassert>
            #include <cstring>
            #include <iostream>

            class VirtualInput : public esp32keybridge::InputAdapter
            {
            public:
              void update() override {}
              const esp32keybridge::InputState &state() const override { return state_; }
              esp32keybridge::InputState state_;
            };

            static void test_core_merge_and_transform()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput left;
              VirtualInput right;
              esp32keybridge::RecordingOutputAdapter output;

              assert(bridge.addInput(left));
              assert(bridge.addInput(right));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              assert(config.global.remap(esp32keybridge::Key::CapsLock, esp32keybridge::Key::LeftCtrl));
              assert(config.global.disable(esp32keybridge::Key::Insert));

              esp32keybridge::ESP32KeyBridgeConfigError error;
              assert(bridge.validateConfig(config, error));
              assert(error.message == nullptr);
              bridge.applyConfig(config);

              left.state_.press(esp32keybridge::Key::LeftShift);
              left.state_.press(esp32keybridge::Key::CapsLock);
              right.state_.press(esp32keybridge::Key::A);
              right.state_.press(esp32keybridge::Key::Insert);

              bridge.update();

              assert(output.writeCount() == 1);
              assert(output.state().isPressed(esp32keybridge::Key::LeftShift));
              assert(output.state().isPressed(esp32keybridge::Key::LeftCtrl));
              assert(output.state().isPressed(esp32keybridge::Key::A));
              assert(!output.state().isPressed(esp32keybridge::Key::CapsLock));
              assert(!output.state().isPressed(esp32keybridge::Key::Insert));
              assert(output.state().codeCount() == 3);
              assert(bridge.mergedState().isPressed(esp32keybridge::Key::CapsLock));
              assert(bridge.mergedState().isPressed(esp32keybridge::Key::Insert));
              assert(!bridge.outputState().isPressed(esp32keybridge::Key::CapsLock));
              assert(!bridge.outputState().isPressed(esp32keybridge::Key::Insert));
              assert(bridge.outputState().isPressed(esp32keybridge::Key::LeftCtrl));
            }

            static void test_key_name_helper()
            {
              assert(std::strcmp(esp32keybridge::keyName(esp32keybridge::Key::A), "A") == 0);
              assert(std::strcmp(esp32keybridge::keyName(esp32keybridge::Key::CapsLock), "CapsLock") == 0);
              assert(std::strcmp(esp32keybridge::keyName(esp32keybridge::Key::Fn1), "Fn1") == 0);
              assert(std::strcmp(esp32keybridge::keyName(static_cast<esp32keybridge::Key>(999)), "Unknown") == 0);
            }

            static void test_input_code_helpers()
            {
              const esp32keybridge::InputCode a = esp32keybridge::keyboardCode(esp32keybridge::Key::A);
              const esp32keybridge::InputCode anotherA = esp32keybridge::keyboardCode(esp32keybridge::Key::A);
              const esp32keybridge::InputCode consumer = esp32keybridge::consumerCode(0x00e9);
              const esp32keybridge::InputCode pointerButton = esp32keybridge::pointerButtonCode(1);
              const esp32keybridge::InputCode pointerAxis = esp32keybridge::pointerAxisCode(2);
              const esp32keybridge::InputCode vendor = esp32keybridge::vendorCode(0x1001);

              assert(a == anotherA);
              assert(a != consumer);
              assert(a.domain == esp32keybridge::InputDomain::Keyboard);
              assert(a.code == static_cast<uint16_t>(esp32keybridge::Key::A));
              assert(consumer.domain == esp32keybridge::InputDomain::Consumer);
              assert(pointerButton.domain == esp32keybridge::InputDomain::PointerButton);
              assert(pointerAxis.domain == esp32keybridge::InputDomain::PointerAxis);
              assert(vendor.domain == esp32keybridge::InputDomain::Vendor);
              assert(esp32keybridge::isValid(a));
              assert(esp32keybridge::isValid(consumer));
              assert(!esp32keybridge::isValid(esp32keybridge::keyboardCode(esp32keybridge::Key::None)));
              assert(!esp32keybridge::isValid(esp32keybridge::consumerCode(0)));
              assert(esp32keybridge::keyFromCode(a) == esp32keybridge::Key::A);
              assert(esp32keybridge::keyFromCode(consumer) == esp32keybridge::Key::None);
              assert(std::strcmp(esp32keybridge::inputDomainName(esp32keybridge::InputDomain::Keyboard), "Keyboard") == 0);
              assert(std::strcmp(esp32keybridge::inputDomainName(esp32keybridge::InputDomain::Consumer), "Consumer") == 0);
              assert(std::strcmp(esp32keybridge::inputDomainName(static_cast<esp32keybridge::InputDomain>(77)), "Unknown") == 0);
            }

            static void test_input_state_accepts_input_codes()
            {
              esp32keybridge::InputState state;
              const esp32keybridge::InputCode a = esp32keybridge::keyboardCode(esp32keybridge::Key::A);
              const esp32keybridge::InputCode consumer = esp32keybridge::consumerCode(0x00e9);

              assert(state.press(a));
              assert(state.contains(a));
              assert(state.contains(esp32keybridge::Key::A));
              assert(state.isPressed(a));
              assert(state.isPressed(esp32keybridge::Key::A));
              assert(state.codeAt(0) == a);
              assert(state.keyAt(0) == esp32keybridge::Key::A);
              assert(state.press(consumer));
              assert(state.contains(consumer));
              assert(state.isPressed(consumer));
              assert(state.codeAt(1) == consumer);
              assert(state.keyAt(1) == esp32keybridge::Key::None);
              assert(state.codeCount() == 2);
              assert(state.release(a));
              assert(!state.contains(a));
              assert(!state.isPressed(a));
              assert(state.codeCount() == 1);
              assert(!state.press(esp32keybridge::consumerCode(0)));
            }

            static void test_input_state_accepts_non_keyboard_input_code()
            {
              esp32keybridge::InputState state;
              const esp32keybridge::InputCode consumer = esp32keybridge::consumerCode(0x00e9);
              const esp32keybridge::InputCode pointerButton = esp32keybridge::pointerButtonCode(1);

              assert(state.press(consumer));
              assert(state.press(pointerButton));
              assert(state.contains(consumer));
              assert(state.contains(pointerButton));
              assert(state.isPressed(consumer));
              assert(state.isPressed(pointerButton));
              assert(state.keyAt(0) == esp32keybridge::Key::None);
              assert(state.codeAt(0) == consumer);
              assert(state.codeAt(1) == pointerButton);
              assert(state.codeCount() == 2);
              assert(state.release(consumer));
              assert(!state.contains(consumer));
              assert(!state.isPressed(consumer));
              assert(state.codeCount() == 1);
            }

            static void test_input_state_can_merge_other_state()
            {
              esp32keybridge::InputState left;
              esp32keybridge::InputState right;
              const esp32keybridge::InputCode consumer = esp32keybridge::consumerCode(0x00e9);

              assert(left.press(esp32keybridge::Key::A));
              assert(left.press(consumer));
              assert(right.press(consumer));
              assert(right.press(esp32keybridge::Key::B));

              assert(left.mergeFrom(right));
              assert(left.contains(esp32keybridge::Key::A));
              assert(left.contains(esp32keybridge::Key::B));
              assert(left.contains(consumer));
              assert(left.codeCount() == 3);
            }

            static void test_input_state_applies_input_events()
            {
              esp32keybridge::InputState state;
              const esp32keybridge::InputEvent press = esp32keybridge::keyEvent(esp32keybridge::Key::A, true, 123);
              const esp32keybridge::InputEvent release = esp32keybridge::keyEvent(esp32keybridge::Key::A, false, 456);
              const esp32keybridge::InputEvent consumer = esp32keybridge::inputEvent(esp32keybridge::consumerCode(0x00e9), true, 789);

              assert(press.pressed);
              assert(press.timestampMs == 123);
              assert(esp32keybridge::keyFromCode(press.code) == esp32keybridge::Key::A);
              assert(state.apply(press));
              assert(state.isPressed(esp32keybridge::Key::A));
              assert(state.apply(consumer));
              assert(state.isPressed(consumer.code));
              assert(state.codeCount() == 2);
              assert(state.apply(release));
              assert(!state.isPressed(esp32keybridge::Key::A));
              assert(state.codeCount() == 1);
              assert(!state.apply(release));
            }

            static void test_event_input_adapter_applies_events()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              esp32keybridge::EventInputAdapter input;
              esp32keybridge::RecordingOutputAdapter output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              assert(input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, true, 100)));
              bridge.update();
              assert(output.state().isPressed(esp32keybridge::Key::A));

              assert(input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, false, 200)));
              bridge.update();
              assert(!output.state().isPressed(esp32keybridge::Key::A));
              assert(output.state().codeCount() == 0);
            }

            static void test_bridge_can_clear_inputs_and_outputs()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              esp32keybridge::EventInputAdapter input;
              esp32keybridge::RecordingOutputAdapter output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, true));
              bridge.update();
              assert(output.writeCount() == 1);
              assert(output.state().isPressed(esp32keybridge::Key::A));

              bridge.clearInputs();
              bridge.update();
              assert(output.writeCount() == 2);
              assert(output.state().codeCount() == 0);
              assert(bridge.mergedState().codeCount() == 0);
              assert(bridge.outputState().codeCount() == 0);

              assert(bridge.addInput(input));
              input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::B, true));
              bridge.clearOutputs();
              bridge.update();
              assert(output.writeCount() == 2);
              assert(bridge.outputState().isPressed(esp32keybridge::Key::A));
              assert(bridge.outputState().isPressed(esp32keybridge::Key::B));
            }

            static void test_core_merge_options()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              esp32keybridge::RecordingOutputAdapter output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              config.merge.shareModifiers = true;
              config.merge.shareKeyboardKeys = false;
              bridge.applyConfig(config);

              input.state_.press(esp32keybridge::Key::LeftShift);
              input.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.state().isPressed(esp32keybridge::Key::LeftShift));
              assert(!output.state().isPressed(esp32keybridge::Key::A));
              assert(output.state().codeCount() == 1);
            }

            static void test_core_merge_options_are_domain_specific()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              esp32keybridge::RecordingOutputAdapter output;
              const esp32keybridge::InputCode consumer = esp32keybridge::consumerCode(0x00e9);
              const esp32keybridge::InputCode pointerButton = esp32keybridge::pointerButtonCode(1);
              const esp32keybridge::InputCode vendor = esp32keybridge::vendorCode(0x1001);

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              config.merge.shareModifiers = true;
              config.merge.shareKeyboardKeys = false;
              config.merge.shareConsumer = true;
              config.merge.sharePointerButtons = false;
              config.merge.shareVendor = false;
              bridge.applyConfig(config);

              input.state_.press(esp32keybridge::Key::LeftShift);
              input.state_.press(esp32keybridge::Key::A);
              input.state_.press(consumer);
              input.state_.press(pointerButton);
              input.state_.press(vendor);

              bridge.update();

              assert(output.state().isPressed(esp32keybridge::Key::LeftShift));
              assert(!output.state().isPressed(esp32keybridge::Key::A));
              assert(output.state().isPressed(consumer));
              assert(!output.state().isPressed(pointerButton));
              assert(!output.state().isPressed(vendor));
              assert(output.state().codeCount() == 2);
            }

            static void test_per_input_transform_runs_before_merge_and_global_transform()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput keyboard;
              VirtualInput scanner;
              esp32keybridge::RecordingOutputAdapter output;

              assert(bridge.addInput(keyboard));
              assert(bridge.addInput(scanner));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              assert(config.input(1).remap(esp32keybridge::Key::Enter, esp32keybridge::Key::Tab));
              assert(config.input(0).disable(esp32keybridge::Key::Insert));
              assert(config.global.remap(esp32keybridge::Key::A, esp32keybridge::Key::B));
              bridge.applyConfig(config);

              keyboard.state_.press(esp32keybridge::Key::Enter);
              keyboard.state_.press(esp32keybridge::Key::Insert);
              scanner.state_.press(esp32keybridge::Key::Enter);
              scanner.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.state().isPressed(esp32keybridge::Key::Enter));
              assert(output.state().isPressed(esp32keybridge::Key::Tab));
              assert(output.state().isPressed(esp32keybridge::Key::B));
              assert(!output.state().isPressed(esp32keybridge::Key::A));
              assert(!output.state().isPressed(esp32keybridge::Key::Insert));
              assert(output.state().codeCount() == 3);
            }

            static void test_input_can_bind_to_explicit_config_index()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              esp32keybridge::RecordingOutputAdapter output;

              assert(bridge.addInput(input, 2));
              assert(bridge.addOutput(output));
              assert(!bridge.addInput(input, esp32keybridge::ESP32KeyBridgeConfig::MaxInputConfigs));

              esp32keybridge::ESP32KeyBridgeConfig config;
              assert(config.input(0).remap(esp32keybridge::Key::A, esp32keybridge::Key::C));
              assert(config.input(2).remap(esp32keybridge::Key::A, esp32keybridge::Key::B));
              bridge.applyConfig(config);

              input.state_.press(esp32keybridge::Key::A);
              bridge.update();

              assert(output.state().isPressed(esp32keybridge::Key::B));
              assert(!output.state().isPressed(esp32keybridge::Key::A));
              assert(!output.state().isPressed(esp32keybridge::Key::C));
            }

            static void test_config_try_input_reports_out_of_range()
            {
              esp32keybridge::ESP32KeyBridgeConfig config;

              assert(config.tryInput(0) != nullptr);
              assert(config.tryInput(esp32keybridge::ESP32KeyBridgeConfig::MaxInputConfigs - 1) != nullptr);
              assert(config.tryInput(esp32keybridge::ESP32KeyBridgeConfig::MaxInputConfigs) == nullptr);

              config.input(esp32keybridge::ESP32KeyBridgeConfig::MaxInputConfigs).remap(
                esp32keybridge::Key::A,
                esp32keybridge::Key::B
              );

              assert(config.input(0).map(esp32keybridge::Key::A) == esp32keybridge::Key::A);
            }

            static void test_validate_config_rejects_out_of_range_input_config()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              esp32keybridge::ESP32KeyBridgeConfig config;
              esp32keybridge::ESP32KeyBridgeConfigError error;

              assert(bridge.validateConfig(config, error));
              assert(error.message == nullptr);

              config.input(esp32keybridge::ESP32KeyBridgeConfig::MaxInputConfigs).remap(
                esp32keybridge::Key::A,
                esp32keybridge::Key::B
              );

              assert(!bridge.validateConfig(config, error));
              assert(error.message != nullptr);
              assert(std::strcmp(error.message, "input config index out of range") == 0);
            }

            static void test_config_clear_resets_all_sections()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              esp32keybridge::ESP32KeyBridgeConfig config;
              esp32keybridge::ESP32KeyBridgeConfigError error;

              assert(config.input(0).remap(esp32keybridge::Key::A, esp32keybridge::Key::B));
              assert(config.input(esp32keybridge::ESP32KeyBridgeConfig::MaxInputConfigs).remap(
                esp32keybridge::Key::B,
                esp32keybridge::Key::C
              ));
              assert(config.global.disable(esp32keybridge::Key::Insert));
              config.layer.setMomentary(esp32keybridge::Key::Fn1);
              assert(config.layer.remap(esp32keybridge::Key::A, esp32keybridge::Key::C));
              assert(config.layout.map(esp32keybridge::Key::A, esp32keybridge::Key::B));
              config.merge.shareKeyboardKeys = false;

              assert(!bridge.validateConfig(config, error));
              config.clear();

              assert(bridge.validateConfig(config, error));
              assert(error.message == nullptr);
              assert(config.input(0).map(esp32keybridge::Key::A) == esp32keybridge::Key::A);
              assert(!config.global.isDisabled(esp32keybridge::Key::Insert));
              assert(!config.layer.enabled());
              assert(config.layout.convert(esp32keybridge::Key::A) == esp32keybridge::Key::A);
              assert(config.merge.shareModifiers);
              assert(config.merge.shareKeyboardKeys);
              assert(config.merge.shareConsumer);
              assert(config.merge.sharePointerButtons);
              assert(config.merge.sharePointerAxes);
              assert(config.merge.shareVendor);
            }

            static void test_momentary_layer_remaps_while_trigger_is_pressed()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              esp32keybridge::RecordingOutputAdapter output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              config.layer.setMomentary(esp32keybridge::Key::Fn1);
              assert(config.layer.remap(esp32keybridge::Key::A, esp32keybridge::Key::B));
              bridge.applyConfig(config);

              input.state_.press(esp32keybridge::Key::Fn1);
              input.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.state().isPressed(esp32keybridge::Key::B));
              assert(!output.state().isPressed(esp32keybridge::Key::A));
              assert(!output.state().isPressed(esp32keybridge::Key::Fn1));
              assert(output.state().codeCount() == 1);

              input.state_.clear();
              input.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.state().isPressed(esp32keybridge::Key::A));
              assert(!output.state().isPressed(esp32keybridge::Key::B));
              assert(output.state().codeCount() == 1);
            }

            static void test_transform_macro_expands_trigger_to_key_sequence()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              esp32keybridge::RecordingOutputAdapter output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              const esp32keybridge::Key macroKeys[] = {
                esp32keybridge::Key::LeftCtrl,
                esp32keybridge::Key::A,
                esp32keybridge::Key::B,
              };
              assert(config.global.macro(esp32keybridge::Key::Fn1, macroKeys, 3));
              assert(config.global.remap(esp32keybridge::Key::Fn1, esp32keybridge::Key::C));
              bridge.applyConfig(config);

              input.state_.press(esp32keybridge::Key::Fn1);

              bridge.update();

              assert(output.state().isPressed(esp32keybridge::Key::LeftCtrl));
              assert(output.state().isPressed(esp32keybridge::Key::A));
              assert(output.state().isPressed(esp32keybridge::Key::B));
              assert(!output.state().isPressed(esp32keybridge::Key::Fn1));
              assert(!output.state().isPressed(esp32keybridge::Key::C));
              assert(output.state().codeCount() == 3);
            }

            static void test_layout_conversion_runs_before_global_transform()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              esp32keybridge::RecordingOutputAdapter output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              assert(config.layout.map(esp32keybridge::Key::A, esp32keybridge::Key::B));
              assert(config.global.remap(esp32keybridge::Key::B, esp32keybridge::Key::C));
              bridge.applyConfig(config);

              input.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.state().isPressed(esp32keybridge::Key::C));
              assert(!output.state().isPressed(esp32keybridge::Key::A));
              assert(!output.state().isPressed(esp32keybridge::Key::B));
              assert(output.state().codeCount() == 1);
            }

            static void test_non_keyboard_codes_pass_through_pipeline()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              esp32keybridge::RecordingOutputAdapter output;
              const esp32keybridge::InputCode consumer = esp32keybridge::consumerCode(0x00e9);
              const esp32keybridge::InputCode pointerButton = esp32keybridge::pointerButtonCode(1);

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              assert(config.global.remap(esp32keybridge::Key::A, esp32keybridge::Key::B));
              assert(config.layout.map(esp32keybridge::Key::B, esp32keybridge::Key::C));
              config.layer.setMomentary(esp32keybridge::Key::Fn1);
              assert(config.layer.remap(esp32keybridge::Key::A, esp32keybridge::Key::C));
              bridge.applyConfig(config);

              input.state_.press(consumer);
              input.state_.press(pointerButton);
              input.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.state().isPressed(consumer));
              assert(output.state().isPressed(pointerButton));
              assert(output.state().isPressed(esp32keybridge::Key::B));
              assert(!output.state().isPressed(esp32keybridge::Key::A));
              assert(output.state().codeCount() == 3);
            }

            static void test_transform_can_remap_and_disable_input_codes()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              esp32keybridge::RecordingOutputAdapter output;
              const esp32keybridge::InputCode volumeUp = esp32keybridge::consumerCode(0x00e9);
              const esp32keybridge::InputCode volumeDown = esp32keybridge::consumerCode(0x00ea);
              const esp32keybridge::InputCode pointerButton = esp32keybridge::pointerButtonCode(1);

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              assert(config.global.remap(volumeUp, volumeDown));
              assert(config.global.disable(pointerButton));
              bridge.applyConfig(config);

              input.state_.press(volumeUp);
              input.state_.press(pointerButton);

              bridge.update();

              assert(output.state().isPressed(volumeDown));
              assert(!output.state().isPressed(volumeUp));
              assert(!output.state().isPressed(pointerButton));
              assert(output.state().codeCount() == 1);
            }

            static void test_capacity_limits_report_failure()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput inputs[esp32keybridge::ESP32KeyBridge::MaxInputs + 1];
              esp32keybridge::RecordingOutputAdapter outputs[esp32keybridge::ESP32KeyBridge::MaxOutputs + 1];

              for (size_t i = 0; i < esp32keybridge::ESP32KeyBridge::MaxInputs; ++i)
              {
                assert(bridge.addInput(inputs[i]));
              }
              assert(!bridge.addInput(inputs[esp32keybridge::ESP32KeyBridge::MaxInputs]));

              for (size_t i = 0; i < esp32keybridge::ESP32KeyBridge::MaxOutputs; ++i)
              {
                assert(bridge.addOutput(outputs[i]));
              }
              assert(!bridge.addOutput(outputs[esp32keybridge::ESP32KeyBridge::MaxOutputs]));

              esp32keybridge::TransformConfig transform;
              for (size_t i = 0; i < esp32keybridge::TransformConfig::MaxRemaps; ++i)
              {
                assert(transform.remap(static_cast<esp32keybridge::Key>(100 + i), esp32keybridge::Key::A));
              }
              assert(!transform.remap(esp32keybridge::Key::CapsLock, esp32keybridge::Key::LeftCtrl));

              esp32keybridge::TransformConfig disabled;
              for (size_t i = 0; i < esp32keybridge::TransformConfig::MaxDisabledKeys; ++i)
              {
                assert(disabled.disable(static_cast<esp32keybridge::Key>(200 + i)));
              }
              assert(!disabled.disable(esp32keybridge::Key::Insert));

              esp32keybridge::TransformConfig macros;
              const esp32keybridge::Key macroKeys[] = {esp32keybridge::Key::A};
              for (size_t i = 0; i < esp32keybridge::TransformConfig::MaxMacros; ++i)
              {
                assert(macros.macro(static_cast<esp32keybridge::Key>(300 + i), macroKeys, 1));
              }
              assert(!macros.macro(esp32keybridge::Key::Fn1, macroKeys, 1));

              esp32keybridge::LayoutConfig layout;
              for (size_t i = 0; i < esp32keybridge::LayoutConfig::MaxMappings; ++i)
              {
                assert(layout.map(static_cast<esp32keybridge::Key>(400 + i), esp32keybridge::Key::B));
              }
              assert(!layout.map(esp32keybridge::Key::A, esp32keybridge::Key::C));

              esp32keybridge::InputState full;
              esp32keybridge::InputState extra;
              for (size_t i = 0; i < esp32keybridge::InputState::MaxCodes; ++i)
              {
                assert(full.press(esp32keybridge::vendorCode(static_cast<uint16_t>(i + 1))));
              }
              assert(extra.press(esp32keybridge::vendorCode(1000)));
              assert(!full.mergeFrom(extra));
            }

            static void run(const char *name, void (*test)())
            {
              std::cout << "RUN " << name << std::endl;
              test();
            }

            int main()
            {
              run("core_merge_and_transform", test_core_merge_and_transform);
              run("key_name_helper", test_key_name_helper);
              run("input_code_helpers", test_input_code_helpers);
              run("input_state_accepts_input_codes", test_input_state_accepts_input_codes);
              run("input_state_accepts_non_keyboard_input_code", test_input_state_accepts_non_keyboard_input_code);
              run("input_state_can_merge_other_state", test_input_state_can_merge_other_state);
              run("input_state_applies_input_events", test_input_state_applies_input_events);
              run("event_input_adapter_applies_events", test_event_input_adapter_applies_events);
              run("bridge_can_clear_inputs_and_outputs", test_bridge_can_clear_inputs_and_outputs);
              run("core_merge_options", test_core_merge_options);
              run("core_merge_options_are_domain_specific", test_core_merge_options_are_domain_specific);
              run("per_input_transform_runs_before_merge_and_global_transform", test_per_input_transform_runs_before_merge_and_global_transform);
              run("input_can_bind_to_explicit_config_index", test_input_can_bind_to_explicit_config_index);
              run("config_try_input_reports_out_of_range", test_config_try_input_reports_out_of_range);
              run("validate_config_rejects_out_of_range_input_config", test_validate_config_rejects_out_of_range_input_config);
              run("config_clear_resets_all_sections", test_config_clear_resets_all_sections);
              run("momentary_layer_remaps_while_trigger_is_pressed", test_momentary_layer_remaps_while_trigger_is_pressed);
              run("transform_macro_expands_trigger_to_key_sequence", test_transform_macro_expands_trigger_to_key_sequence);
              run("layout_conversion_runs_before_global_transform", test_layout_conversion_runs_before_global_transform);
              run("non_keyboard_codes_pass_through_pipeline", test_non_keyboard_codes_pass_through_pipeline);
              run("transform_can_remap_and_disable_input_codes", test_transform_can_remap_and_disable_input_codes);
              run("capacity_limits_report_failure", test_capacity_limits_report_failure);
              return 0;
            }
            '''
        ),
        encoding="utf-8",
    )

    subprocess.run(
        [
            "g++",
            "-std=c++17",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-I",
            str(ROOT / "src"),
            str(source),
            str(ROOT / "src" / "ESP32KeyBridge.cpp"),
            "-o",
            str(binary),
        ],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
