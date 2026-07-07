import subprocess
import textwrap
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def test_core_merge_and_transform(tmp_path):
    source = tmp_path / "core_test.cpp"
    binary = tmp_path / "core_test"

    source.write_text(
        textwrap.dedent(
            r'''
            #include <ESP32KeyBridge.h>
            #include <cassert>

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

            int main()
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
            "-o",
            str(binary),
        ],
        check=True,
    )
    subprocess.run([str(binary)], check=True)


def test_core_merge_options(tmp_path):
    source = tmp_path / "merge_options_test.cpp"
    binary = tmp_path / "merge_options_test"

    source.write_text(
        textwrap.dedent(
            r'''
            #include <ESP32KeyBridge.h>
            #include <cassert>

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
              void write(const esp32keybridge::KeyboardState &state) override { last_ = state; }
              esp32keybridge::KeyboardState last_;
            };

            int main()
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
            "-o",
            str(binary),
        ],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
