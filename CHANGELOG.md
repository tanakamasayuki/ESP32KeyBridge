# Changelog

## Unreleased

- Define the intermediate data model through use-case-driven review:
  key identity as kind + value (keyboard / consumer / mouse button /
  virtual, HID-identical values), two traffic classes (held state sets
  and one-shot event streams for characters, key actions, and relative
  axes), lock state with a single authority chain and LED forwarding,
  keyboard layout conversion with shift synthesis, string-to-keystroke
  typing, and relative-only mouse support. See `docs/DATA_MODEL.ja.md`,
  `docs/USE_CASES.ja.md`, and `docs/DECISIONS.ja.md`.
- Start repository structure, documentation, and test plan.

