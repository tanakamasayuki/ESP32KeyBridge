# Changelog

## Unreleased

- Add AltGr (Right Alt) support to the keyboard layout model, reaching
  feature parity with the shared 4-plane keymap tables of the related
  projects (see `docs/KEYMAP_FIX_REQUEST.ja.md`). `KeyboardLayoutEntry`
  gains `altGr` / `altGrShift` Unicode codepoint planes and `KeyStroke`
  gains an `altGr` flag. `encode()` now searches base -> Shift -> AltGr ->
  AltGr+Shift and sets the matched plane's flags; `decode(key, shift,
  altGr)` selects the AltGr planes; `hasAltGr()` reports whether a layout
  uses them. The typing engine emits Right Alt for AltGr characters, and
  layout conversion synthesizes/consumes Right Alt symmetrically: on an
  AltGr layout Right Alt drives the AltGr plane and is consumed like Shift,
  while layouts without AltGr (`en_us` / `ja_jp`) keep Right Alt as an
  ordinary shortcut modifier (unchanged behavior).
- Bundle 14 additional keyboard layouts generated from the shared 4-plane
  keymap tables of the related projects (`de_de`, `fr_fr`, `es_es`, `it_it`,
  `nl_nl`, `da_dk`, `nb_no`, `sv_se`, `fi_fi`, `en_gb`, `pt_pt`, `pt_br`,
  `fr_ch`, `hu_hu`), each with full AltGr support. `en_us` / `ja_jp` stay
  hand-authored; the rest are produced by `tools/gen_keymaps.py` into
  `src/ESP32KeyBridgeLayouts.inc`, so all projects stay in sync from one
  source. The generator derives each key's `capsAffects` with the agreed
  rule (Shift column == Unicode uppercase of base), which also handles
  non-Latin-1 case pairs (e.g. Hungarian `ő`/`Ő`, `ű`/`Ű`). Get any locale
  with `KeyboardLayout::byName("fr_fr")` (or `deDe()`); `de_DE` exercises the
  AltGr plane (`@ € { [ ] } \ ~ | ² ³ µ`).
- Add two input-only USB Host adapters (EspUsbHost 2.3.0).
  `EspUsbHostGamepadInputAdapter`: `mapButton(number, key)` maps HID buttons
  (1-based) and `mapHat(up, down, left, right)` maps the D-pad to any keys;
  only mapped controls emit, all gamepads on the stack merge, analog sticks
  are out of scope for now. `EspUsbHostMidiInputAdapter`: `mapNote(note,
  key)` maps MIDI notes (Note On presses, Note Off releases), `setChannel()`
  narrows to one channel. Both feed the normal key set, so remap/layers/
  merge apply; there is no gamepad/MIDI *output* (that would need the core
  data model to carry those semantics). The shared EspUsbHost hub gained
  gamepad and MIDI sinks. Examples: `GamepadToKeys` and `MidiToKeys` (P4).
  Peer tests `usb_host_gamepad` and `usb_host_midi` drive a gamepad/MIDI
  device board and check the mapped keys on the host board.
- Add `TouchButtons` (S3): a tutorial example for writing your own input
  adapter. It defines a `TouchButtons : InputAdapter` in the sketch that
  reads the ESP32-S3's built-in capacitive touch pads (no external library
  or parts) and presses a media key per pad, showing the whole contract -
  `update()` reads the hardware, `keys()` returns the pressed set, the rest
  is optional. The examples README points custom-input authors here.
- Add four examples that fill the remaining feature gaps. `NavLayer` (P4):
  hold CapsLock to turn H/J/K/L into arrow keys on any USB keyboard — the
  first momentary-layer example (`addLayer`/`setTrigger` with a virtual
  trigger, press-time resolution). `LayerKeypad` (S3, GPIO): a keypad whose
  four keys are navigation normally and media keys while an Fn key is held
  (a layer remapping across key kinds). `MergeKeyboards` (P4): a USB
  keyboard and GPIO macro buttons merged into one HID device (union of
  multiple inputs, plus the USB adapter's own multi-keyboard merge).
  `ScrollDial` (S3, GPIO): a rotary encoder as a mouse scroll/jog dial with
  a middle-click push switch — the `mapToAxis()` counterpart to VolumeKnob.
- Document the rotary encoder's x4 quadrature decoding and detent handling
  in ADAPTERS (both phases, both edges → 4 counts per cycle, collapsed to
  one notch by `setPulsesPerDetent`, remainder carried so no drift).
- Add two Print-based output adapters (new `ESP32KeyBridgeSerial.h`, no
  external dependency): `TextOutputAdapter` writes the text stream to an
  Arduino `Print` (USB CDC Serial, a UART, ...) as UTF-8, and
  `LogOutputAdapter` writes a human-readable line on key-set changes,
  typed characters, and axis moves (a bring-up / monitoring tool).
  Examples: `BarcodeToText` (a USB HID barcode reader becomes a stream of
  text) and `KeyMonitor` (watch the bridge output on the console). Both
  ESP32-S3.
- Route decoded characters from `convertLayout()` inputs into the text
  stream: when a converting input's key is pressed, the bridge decodes it
  with that input's engraving layout and delivers the character to
  text-native outputs via writeText() (Enter/Tab become newline/tab),
  in addition to the existing re-encode to hostLayout for HID outputs.
  The layout stays a single input-side setting; text outputs carry none.
  Additive and scoped to converting inputs — HID outputs ignore
  writeText(), so existing behavior (e.g. UsKeyboardOnJapanesePc) is
  unchanged.

- Expand the host unit coverage: KeySet iteration/merge/clear, the full
  MouseUsage button range mapped to report bits, all four relative axes
  scaled independently, text-macro lookup and capacity, fan-out to
  multiple outputs, and the two Serial output adapters checked
  byte-for-byte through a capturing `Print`.
- The core_smoke unit sketch (~60 assertions over the whole core) can now
  also run on a real ESP32-S3: the same sketch gained an `esp32s3`
  profile (`pytest unit/core_smoke --profile esp32s3` with the optional
  single S3 connected) — no duplicated test sketch and no host/device
  branch. The tests allocate a bridge (~7.3 KB) and a config (~5.6 KB) as
  locals, so the sketch defines `getArduinoLoopTaskStackSize()` (24 KB) —
  the weak hook arduino-esp32 calls to size the loop task; on the host
  core it is just an uncalled function. The `esp32s3` profile uses
  `USBMode=default` (like the examples), so Serial goes to the board's
  external USB-serial (UART0). The result summary is re-emitted from
  loop() because the test runner attaches only after the board has run
  setup(), so a one-shot boot print is lost in that gap. Verified on real
  ESP32-S3 hardware; tests/single/ stays a placeholder pointing there.
- Add hardware peer smoke tests (two directly-wired ESP32-S3 boards)
  covering both USB adapter boundaries. `usb_host_keyboard`: the bridge
  input pipeline against a real boot keyboard — remap, modifier-only,
  multi-key rollover (remap applied inside the chord), key-with-modifier,
  terminal-host CapsLock toggle forwarded to the LEDs, and a held key
  released when the device drops off the bus (the device resets, since
  EspUsbDevice has no bus-detach API). `usb_host_consumer`: consumer
  events merging into the key set. `usb_host_mouse`: the mouse input
  adapter — relative X/Y, wheel, and left/right buttons.
  `usb_device_keyboard_output`: the composite output adapter — single key,
  modifier, six-key rollover, consumer, mouse button/wheel/XY, a
  composite frame emitting keyboard+consumer+mouse together (observed in
  KEY_STATE→CONSUMER→MOUSE order), and the host LED output report becoming
  the bridge lock authority. The LED-to-lock test needs EspUsbHost 2.3.0,
  which sends a report-ID Set_Report to composite/NKRO keyboards that
  declare no boot interface (2.2.0 reached only boot-declared keyboard
  interfaces); the peer and example profiles pin 2.3.0.
- Implement `GpioMatrixInputAdapter` and `RotaryEncoderInputAdapter` with
  the NumPad (GPIO key matrix, 4x3 numeric keypad) and VolumeKnob (rotary
  encoder volume control with a scroll-dial variation) examples. The
  matrix separates wiring from keymap: setRowPins/setColPins describe the
  lines, setKeys takes the keys in row-major order so the code layout
  mirrors the physical layout (no coordinates or indices; the example
  annotates the pins in comments). Scanning, per-key debouncing, and —
  without diodes — the ghost blocking that holds back the ambiguous
  fourth corner of a 2x2 rectangle (setHasDiodes(true) lifts the limit)
  run in a dedicated 1 kHz task started on the first update(), so short
  taps survive a blocked loop; update() only snapshots the debounced
  state, and idle rows stay high-Z. The encoder counts on the PCNT
  peripheral (x4 quadrature decoding with a glitch filter, no
  interrupts); each detent either taps a key pair (mapToKeys) or feeds a
  relative axis (mapToAxis), with setPulsesPerDetent defaulting to the
  EC11's 4.
- Move BLE out of this library: BLE support will come from a dedicated
  sibling library (a single library hosting both the central and
  peripheral HID roles, NimBLE-based, no Bluetooth Classic — the S3-era
  chips have no BR/EDR), mirroring the sketch-owned-stack model of the USB
  adapters. Until it exists this library ships no BLE surface: the
  build-only `ESP32KeyBridgeBle.h` mock and the `BleToUsb` example are
  removed rather than left guessing an API the new library will define.
- Implement `EspUsbHostKeyboardInputAdapter` on EspUsbHost 2.2.0's new
  `onKeyboardState()`: one format-independent 256-bit snapshot per changed
  report (boot / report-ID boot / NKRO alike, modifier-only changes
  included — this replaces the earlier change request about missing
  modifier events) maps directly onto the key set. All keyboards on the
  stack are merged with per-device tracking (up to 4, so a disconnect
  releases only that device's keys), boot rollover error codes (usages
  0x01-0x03) are filtered, consumer press/release events (media keys,
  remotes; up to 8 held usages) join the same key set, and the bridge lock
  state is forwarded to every keyboard's LEDs (num/caps/scroll; EspUsbHost
  has no kana LED path). The sketch-facing rule tightens: onKeyboardState /
  onConsumerControl / onMouse / onDeviceDisconnected belong to the
  adapters' shared hub, while the per-event onKeyboard stays free for
  sketches. Every adapter used by the examples is now real; the example
  profiles pin EspUsbHost 2.3.0.
- Implement `EspUsbHostMouseInputAdapter`: all mice on the stack merged
  (button union, deltas summed) with per-address tracking so a disconnect
  releases only that mouse's buttons; movement and wheel accumulate as
  relative axis deltas behind a critical section (EspUsbHost callbacks fire
  from the library task). Because EspUsbHost callbacks are single-slot, the
  USB Host adapters now share subscriptions through an internal per-stack
  hub — sketches must not set onMouse / onDeviceDisconnected themselves.
  A mouse counts as present from its first report (the library has no
  enumeration query). The keyboard input adapter stays a mock, waiting on
  an EspUsbHost change (modifier-only presses produce no onKeyboard
  events). Added the `NaturalScroll` example (ESP32-P4): invert the mouse
  wheel direction in hardware via a negative axis scale, with the
  left-handed button swap as a README variation. Added the `MouseUsage`
  enum (Left/Right/Middle/Back/Forward, Button6-8) so mouse buttons follow
  the no-magic-number rule like the other key kinds; `mouseButtonKey(n)`
  still accepts any button number.
- Unify the USB Device output into a single always-composite HID adapter:
  `EspUsbDeviceHidOutputAdapter` (keyboard + consumer + mouse) replaces the
  keyboard-only `EspUsbDeviceKeyboardOutputAdapter`. Unused interfaces are
  harmless on the USB side (they simply never send reports), the keyboard
  interface still speaks the boot protocol for BIOS/UEFI, and adding mouse
  or media keys to a configuration later never requires re-enumeration.
  The consumer report (16-bit usage) and mouse report (buttons plus
  X/Y/Wheel from the relative axes, int8 saturation with carry; Pan is not
  in the boot mouse report and is dropped) follow the same
  send-on-change-with-busy-retry pattern as the keyboard report. Interface
  disable options are deferred until a real need appears (they would be
  non-breaking constructor arguments).
- Remove the pre-spec prototype implementation, examples, and their tests
  (reference: commit `4d2d48151c62`). New examples and hardware tests will
  be recreated as the implementation steps land.
- Implement core step 1 of the finalized data model: key identity as kind +
  value (`Key`, `KeyKind`, `KeyboardUsage`, `ConsumerUsage`), the pressed
  key set (`KeySet`), union merge across inputs with release-when-all-release
  semantics, disconnect handling that drops a lost input's keys, and keyboard
  modifier normalization helpers. Fixed by host unit tests.
- Implement core step 2, key transforms: single-step kind-crossing remap and
  disable (`TransformConfig`), per-input transforms bound by config slot,
  momentary layers triggered by virtual keys (`LayerConfig`), and press-time
  resolution (a held key keeps the mapping it was pressed with, so layer and
  config changes never produce stuck keys). Add the `SwapCtrlCapsLock`
  example (UC1).
- Implement core step 3, lock state: an internal `LockState` shadow with a
  single authority chain (the first connected lock-reporting output wins;
  without one the bridge acts as the terminal host and toggles
  Caps/Num/Scroll on resolved key presses), push notification to input
  adapters on change and on (re)connect, and the `OutputAdapter` interface
  with `ManualOutputAdapter` for tests and sketches.
- Implement core step 4, one-shot events: the text stream (`typeChar` /
  `typeText`, UTF-8) with `HostLayout` tables (`en_us`, `ja_jp`) that expand
  characters into keystrokes at the output edge, a typing engine with atomic
  per-character frames (modifier-first phases, user modifier parking with an
  optional defer mode, Caps Lock compensation, standard control character
  mapping with CRLF collapsing, overflow / unencodable counters), text
  macros triggered by consumed keys, `writeText` for text-native outputs,
  and relative axes with per-axis invert/scale and one-shot drain semantics.
- Implement core step 5, keyboard layout conversion: per-input opt-in
  (`convertLayout`) that decodes printable keys with the engraving layout
  (using that input's own Shift, which is consumed) and re-encodes them with
  the host layout, synthesizing or suppressing Shift while the key is held.
  Non-printable keys and Ctrl/Alt/GUI shortcuts pass through unconverted,
  the real Shift is reflected while non-printable keys are held
  (Shift+Arrow), untypable characters are dropped and counted, and a
  configurable toggle key (`layoutConversionToggle`) switches conversion
  off for BIOS-like environments.
- Rename `HostLayout` to `KeyboardLayout` (and `HostLayoutEntry` to
  `KeyboardLayoutEntry`): the type is a role-neutral layout description
  used both as an input device's engraving and as the declaration of a
  host's layout setting; the role is expressed at the usage site
  (`config.hostLayout`, `convertLayout(engraving)`), not in the type.
- API refinements from the example review: `Key` converts implicitly from
  `KeyboardUsage` / `ConsumerUsage` / `VirtualUsage` (the enum determines
  the kind, so `remap(KeyboardUsage::CapsLock, KeyboardUsage::LeftCtrl)`
  works without `keyboardKey()` wrappers); added the `VirtualUsage`
  predefined virtual key slots (V1-V16; `virtualKey(n)` still accepts any
  code); added `config.addLayer(trigger)` so layers are created by their
  trigger key instead of a slot index; per-input settings became the
  handle-based `InputConfig` (`config.addInputConfig()` auto-numbers a
  slot, `bridge.addInput(input, inputConfig)` binds it, the same handle on
  several inputs shares the settings), replacing the index-based
  `config.input(i)` / `config.convertLayout(i, ...)` — project-wide rule:
  no public API takes a caller-managed number with a range check
  (auto-numbered handles or enums instead); text input got Serial-style
  backpressure — `typeAvailable()` reports the queue space and
  `typeText()` now returns the number of consumed bytes and stops without
  dropping when the queue fills (only unresumable paths — `typeChar` and
  macro playback — still drop and count on overflow); removed the empty
  `ESP32KeyBridge::begin()` —
  there is no ordering between addInput/addOutput, applyConfig, and adapter
  startup, everything starts working once `update()` runs.
- Adapter ownership model: communication stacks (EspUsbHost / EspUsbDevice)
  are owned by the sketch and started with their own configs (port
  selection, VID/PID); adapters take a reference and have no `begin()`.
  Added build-only mock adapter headers fixing the sketch-facing API
  (`ESP32KeyBridgeEspUsbHost.h`, `ESP32KeyBridgeEspUsbDevice.h`,
  `ESP32KeyBridgeGpio.h`, `ESP32KeyBridgeBle.h`); real implementations land
  with implementation step 7.
- Rebuild the examples as practical, flash-as-is sketches only (a shared
  three-step structure: start the hardware, wire the bridge, build and
  apply the configuration): SwapCtrlCapsLock, UsKeyboardOnJapanesePc,
  MediaKeys (ESP32-P4, USB Host + USB Device), FootSwitch, BleToUsb,
  SerialTextTyper (ESP32-S3). Hardware-free API demos were dropped —
  custom adapters start from ManualInputAdapter / ManualOutputAdapter and
  the adapter header comments instead. Example READMEs lead with the
  intended situation instead of use-case numbers; the SwapCtrlCapsLock README
  documents the ESP32-P4 USB port layout (Device fixed to HS on
  arduino-esp32 3.3.10, Host on FS, CDC/OTG PHY swap, label-vs-wiring
  caveats, M5Stack Tab5 case, HS-host hub restriction).
- Implement `GpioKeyInputAdapter`: per-pin configuration (pull-up,
  active-low) on the first update() after registration, deferred debouncing
  (default 5 ms, `setDebounceMillis()`), and logical presses into the key
  set. FootSwitch now runs on real adapters end to end.
- Implement `EspUsbDeviceKeyboardOutputAdapter` (the first real hardware
  adapter, EspUsbDevice-backed): boot keyboard reports sent on change with
  a retry when the endpoint is busy, host LED output reports (including
  kana) published thread-safely as the lock state authority, and
  connected() reflecting the USB mount state. SerialTextTyper is the first
  example whose adapters are all real.
- Add `docs/ADAPTERS.ja.md`: the adapter roster with implementation status,
  what each output adapter makes the PC see (USB device classes /
  interfaces), and the extra library dependencies per adapter header.
- Document the time/concurrency boundary: sampling accuracy lives inside
  adapters (tasks / ISRs / peripherals such as PCNT for encoders) with a
  thread-safe handoff into the single update() context; the bridge itself
  is never driven by an internal task.
- Implement core step 6, HID report builders: pure functions that pack the
  output key set into a boot keyboard report (6KRO), a 32-key rollover
  report, a minimal consumer report, and a relative mouse report (button
  bits plus X/Y/Wheel/Pan with int8 saturation and carry). Each builder
  emits only what its report can represent.

- Define the intermediate data model through use-case-driven review:
  key identity as kind + value (keyboard / consumer / mouse button /
  virtual, HID-identical values), two traffic classes (held state sets
  and one-shot event streams for characters, key actions, and relative
  axes), lock state with a single authority chain and LED forwarding,
  keyboard layout conversion with shift synthesis, string-to-keystroke
  typing, and relative-only mouse support. See `docs/DATA_MODEL.ja.md`,
  `docs/USE_CASES.ja.md`, and `docs/DECISIONS.ja.md`.
- Start repository structure, documentation, and test plan.

