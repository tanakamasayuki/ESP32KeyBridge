# Changelog

## Unreleased

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

