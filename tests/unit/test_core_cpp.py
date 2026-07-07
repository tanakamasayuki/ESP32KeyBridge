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
              const esp32keybridge::KeyboardState &state() const override { return state_; }
              esp32keybridge::KeyboardState state_;
            };

            class RecordingOutput : public esp32keybridge::OutputAdapter
            {
            public:
              void write(const esp32keybridge::KeyboardState &state) override
              {
                last_ = state;
                ++writeCount_;
              }

              esp32keybridge::KeyboardState last_;
              int writeCount_ = 0;
            };

            static void test_core_merge_and_transform()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput left;
              VirtualInput right;
              RecordingOutput output;

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

              assert(output.writeCount_ == 1);
              assert(output.last_.isPressed(esp32keybridge::Key::LeftShift));
              assert(output.last_.isPressed(esp32keybridge::Key::LeftCtrl));
              assert(output.last_.isPressed(esp32keybridge::Key::A));
              assert(!output.last_.isPressed(esp32keybridge::Key::CapsLock));
              assert(!output.last_.isPressed(esp32keybridge::Key::Insert));
              assert(output.last_.keyCount() == 3);
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
              const esp32keybridge::InputCode consumer{esp32keybridge::InputDomain::Consumer, 0x00e9};

              assert(a == anotherA);
              assert(a != consumer);
              assert(a.domain == esp32keybridge::InputDomain::Keyboard);
              assert(a.code == static_cast<uint16_t>(esp32keybridge::Key::A));
              assert(esp32keybridge::keyFromCode(a) == esp32keybridge::Key::A);
              assert(esp32keybridge::keyFromCode(consumer) == esp32keybridge::Key::None);
              assert(std::strcmp(esp32keybridge::inputDomainName(esp32keybridge::InputDomain::Keyboard), "Keyboard") == 0);
              assert(std::strcmp(esp32keybridge::inputDomainName(esp32keybridge::InputDomain::Consumer), "Consumer") == 0);
              assert(std::strcmp(esp32keybridge::inputDomainName(static_cast<esp32keybridge::InputDomain>(77)), "Unknown") == 0);
            }

            static void test_keyboard_state_accepts_keyboard_input_code()
            {
              esp32keybridge::KeyboardState state;
              const esp32keybridge::InputCode a = esp32keybridge::keyboardCode(esp32keybridge::Key::A);
              const esp32keybridge::InputCode consumer{esp32keybridge::InputDomain::Consumer, 0x00e9};

              assert(state.press(a));
              assert(state.isPressed(a));
              assert(state.isPressed(esp32keybridge::Key::A));
              assert(state.codeAt(0) == a);
              assert(state.keyAt(0) == esp32keybridge::Key::A);
              assert(!state.press(consumer));
              assert(state.keyCount() == 1);
              assert(state.release(a));
              assert(!state.isPressed(a));
              assert(state.keyCount() == 0);
            }

            static void test_keyboard_state_applies_input_events()
            {
              esp32keybridge::KeyboardState state;
              const esp32keybridge::InputEvent press = esp32keybridge::keyEvent(esp32keybridge::Key::A, true, 123);
              const esp32keybridge::InputEvent release = esp32keybridge::keyEvent(esp32keybridge::Key::A, false, 456);
              const esp32keybridge::InputEvent consumer{{esp32keybridge::InputDomain::Consumer, 0x00e9}, true, 789};

              assert(press.pressed);
              assert(press.timestampMs == 123);
              assert(esp32keybridge::keyFromCode(press.code) == esp32keybridge::Key::A);
              assert(state.apply(press));
              assert(state.isPressed(esp32keybridge::Key::A));
              assert(!state.apply(consumer));
              assert(state.keyCount() == 1);
              assert(state.apply(release));
              assert(!state.isPressed(esp32keybridge::Key::A));
              assert(state.keyCount() == 0);
              assert(!state.apply(release));
            }

            static void test_event_input_adapter_applies_events()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              esp32keybridge::EventInputAdapter input;
              RecordingOutput output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              assert(input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, true, 100)));
              bridge.update();
              assert(output.last_.isPressed(esp32keybridge::Key::A));

              assert(input.apply(esp32keybridge::keyEvent(esp32keybridge::Key::A, false, 200)));
              bridge.update();
              assert(!output.last_.isPressed(esp32keybridge::Key::A));
              assert(output.last_.keyCount() == 0);
            }

            static void test_core_merge_options()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              RecordingOutput output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              config.merge.shareModifiers = true;
              config.merge.shareKeys = false;
              bridge.applyConfig(config);

              input.state_.press(esp32keybridge::Key::LeftShift);
              input.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.last_.isPressed(esp32keybridge::Key::LeftShift));
              assert(!output.last_.isPressed(esp32keybridge::Key::A));
              assert(output.last_.keyCount() == 1);
            }

            static void test_per_input_transform_runs_before_merge_and_global_transform()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput keyboard;
              VirtualInput scanner;
              RecordingOutput output;

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

              assert(output.last_.isPressed(esp32keybridge::Key::Enter));
              assert(output.last_.isPressed(esp32keybridge::Key::Tab));
              assert(output.last_.isPressed(esp32keybridge::Key::B));
              assert(!output.last_.isPressed(esp32keybridge::Key::A));
              assert(!output.last_.isPressed(esp32keybridge::Key::Insert));
              assert(output.last_.keyCount() == 3);
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

            static void test_momentary_layer_remaps_while_trigger_is_pressed()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              RecordingOutput output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              config.layer.setMomentary(esp32keybridge::Key::Fn1);
              assert(config.layer.remap(esp32keybridge::Key::A, esp32keybridge::Key::B));
              bridge.applyConfig(config);

              input.state_.press(esp32keybridge::Key::Fn1);
              input.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.last_.isPressed(esp32keybridge::Key::B));
              assert(!output.last_.isPressed(esp32keybridge::Key::A));
              assert(!output.last_.isPressed(esp32keybridge::Key::Fn1));
              assert(output.last_.keyCount() == 1);

              input.state_.clear();
              input.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.last_.isPressed(esp32keybridge::Key::A));
              assert(!output.last_.isPressed(esp32keybridge::Key::B));
              assert(output.last_.keyCount() == 1);
            }

            static void test_transform_macro_expands_trigger_to_key_sequence()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              RecordingOutput output;

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

              assert(output.last_.isPressed(esp32keybridge::Key::LeftCtrl));
              assert(output.last_.isPressed(esp32keybridge::Key::A));
              assert(output.last_.isPressed(esp32keybridge::Key::B));
              assert(!output.last_.isPressed(esp32keybridge::Key::Fn1));
              assert(!output.last_.isPressed(esp32keybridge::Key::C));
              assert(output.last_.keyCount() == 3);
            }

            static void test_layout_conversion_runs_before_global_transform()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput input;
              RecordingOutput output;

              assert(bridge.addInput(input));
              assert(bridge.addOutput(output));

              esp32keybridge::ESP32KeyBridgeConfig config;
              assert(config.layout.map(esp32keybridge::Key::A, esp32keybridge::Key::B));
              assert(config.global.remap(esp32keybridge::Key::B, esp32keybridge::Key::C));
              bridge.applyConfig(config);

              input.state_.press(esp32keybridge::Key::A);

              bridge.update();

              assert(output.last_.isPressed(esp32keybridge::Key::C));
              assert(!output.last_.isPressed(esp32keybridge::Key::A));
              assert(!output.last_.isPressed(esp32keybridge::Key::B));
              assert(output.last_.keyCount() == 1);
            }

            static void test_capacity_limits_report_failure()
            {
              esp32keybridge::ESP32KeyBridge bridge;
              VirtualInput inputs[esp32keybridge::ESP32KeyBridge::MaxInputs + 1];
              RecordingOutput outputs[esp32keybridge::ESP32KeyBridge::MaxOutputs + 1];

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
              run("keyboard_state_accepts_keyboard_input_code", test_keyboard_state_accepts_keyboard_input_code);
              run("keyboard_state_applies_input_events", test_keyboard_state_applies_input_events);
              run("event_input_adapter_applies_events", test_event_input_adapter_applies_events);
              run("core_merge_options", test_core_merge_options);
              run("per_input_transform_runs_before_merge_and_global_transform", test_per_input_transform_runs_before_merge_and_global_transform);
              run("config_try_input_reports_out_of_range", test_config_try_input_reports_out_of_range);
              run("validate_config_rejects_out_of_range_input_config", test_validate_config_rejects_out_of_range_input_config);
              run("momentary_layer_remaps_while_trigger_is_pressed", test_momentary_layer_remaps_while_trigger_is_pressed);
              run("transform_macro_expands_trigger_to_key_sequence", test_transform_macro_expands_trigger_to_key_sequence);
              run("layout_conversion_runs_before_global_transform", test_layout_conversion_runs_before_global_transform);
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
