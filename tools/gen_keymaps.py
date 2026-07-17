#!/usr/bin/env python3
"""Generate KeyboardLayoutEntry tables from the shared 4-plane keymap tables.

The related projects (EspUsbHost / EspUsbDevice / EspBle) keep the keymap data
as `uint16_t[N][4]` tables (columns: unshifted / Shift / AltGr / AltGr+Shift,
values are Unicode code points, 0 = none), indexed by HID keyboard usage ID.
This script reads those tables and emits the equivalent `KeyboardLayoutEntry[]`
arrays used by ESP32KeyBridge, deriving `capsAffects` per key with the rule the
projects agreed on: a key is Caps-affected when its Shift column is the Unicode
uppercase of its base column.

Output is written to src/ESP32KeyBridgeLayouts.inc, which is #included inside
the anonymous namespace of ESP32KeyBridge.cpp. en_us and ja_jp stay
hand-authored in the .cpp (they predate the shared tables and are covered by
their own tests); every other locale comes from here.

Usage: python3 tools/gen_keymaps.py [path-to-shared-keymap-dir]
Default shared dir: ../EspUsbHost/src/keymap relative to the repo root.
"""

import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_SHARED = os.path.normpath(os.path.join(REPO, "..", "EspUsbHost", "src", "keymap"))

# Locales generated here. en_us / ja_jp are intentionally excluded (hand-authored).
LOCALES = [
    "de_de", "fr_fr", "es_es", "it_it", "nl_nl", "da_dk", "nb_no", "sv_se",
    "fi_fi", "en_gb", "pt_pt", "pt_br", "fr_ch", "hu_hu",
]

# Usages excluded from layout tables to match KeyBridge conventions:
#  - control keys handled by the typing engine, not the layout table
#  - the keypad block (digits map to the main row; NumLock independence)
EXCLUDED = set([0x28, 0x29, 0x2a, 0x2b])  # Enter, Escape, Backspace, Tab
EXCLUDED |= set(range(0x53, 0x64))  # NumLock + keypad block
EXCLUDED |= {0x85, 0x86}  # keypad comma / equal sign


def parse_usage_names(header_path):
    """usage code -> KeyboardUsage enum name, from ESP32KeyBridge.h."""
    text = open(header_path, encoding="utf-8").read()
    m = re.search(r"enum class KeyboardUsage\s*:\s*uint16_t\s*\{(.*?)\};", text, re.S)
    if not m:
        raise SystemExit("KeyboardUsage enum not found")
    names = {}
    for name, val in re.findall(r"(\w+)\s*=\s*0x([0-9a-fA-F]+)", m.group(1)):
        names[int(val, 16)] = name
    return names


_SIMPLE_ESCAPES = {
    "r": 0x0d, "n": 0x0a, "t": 0x09, "b": 0x08, "f": 0x0c,
    "v": 0x0b, "a": 0x07, "0": 0x00, "\\": 0x5c, "'": 0x27, '"': 0x22,
}


def parse_value(tok):
    tok = tok.strip()
    if tok == "" or tok == "0":
        return 0
    if tok[0] == "'" and tok[-1] == "'":
        inner = tok[1:-1]
        if inner.startswith("\\"):
            esc = inner[1:]
            if esc in _SIMPLE_ESCAPES:
                return _SIMPLE_ESCAPES[esc]
            if esc.startswith("x"):
                return int(esc[1:], 16)
            return int(esc, 8)
        return ord(inner)
    return int(tok, 0)


def extract_rows(body):
    """Split the array body into row strings, honoring char literals.

    A naive brace regex mis-parses rows that contain '{' or '}' as char
    literals (e.g. AltGr = { }), so scan character by character and skip over
    any '...' literal before testing a brace.
    """
    rows = []
    depth = 0
    cur = []
    i = 0
    n = len(body)
    while i < n:
        c = body[i]
        if c == "'":  # consume a full char literal verbatim
            j = i + 1
            lit = ["'"]
            while j < n:
                if body[j] == "\\":
                    lit.append(body[j:j + 2])
                    j += 2
                    continue
                lit.append(body[j])
                j += 1
                if lit[-1] == "'":
                    break
            if depth >= 1:
                cur.append("".join(lit))
            i = j
            continue
        if c == "{":
            depth += 1
            if depth > 1:
                cur.append(c)
        elif c == "}":
            depth -= 1
            if depth == 0:
                rows.append("".join(cur))
                cur = []
            else:
                cur.append(c)
        elif depth >= 1:
            cur.append(c)
        i += 1
    return rows


def parse_table(header_path):
    """Return list indexed by usage: each entry is a 4-tuple of code points."""
    text = open(header_path, encoding="utf-8").read()
    body = text[text.index("= {") + 3: text.rindex("};")]
    # strip line comments (no '//' ever occurs inside a single-char literal)
    body = "\n".join(line.split("//", 1)[0] for line in body.splitlines())
    # A value is a char literal (which may itself contain a comma, e.g. ','),
    # a hex constant, or a decimal -- never split on commas blindly.
    tok_re = re.compile(r"'(?:\\.|[^'])*'|0[xX][0-9a-fA-F]+|\d+")
    table = []
    for row in extract_rows(body):
        cols = [parse_value(t) for t in tok_re.findall(row)]
        cols += [0] * (4 - len(cols))
        table.append(tuple(cols[:4]))
    return table


def caps_affects(base, shift):
    if base == 0 or shift == 0:
        return False
    bc = chr(base)
    if not bc.isalpha():
        return False
    up = bc.upper()
    return len(up) == 1 and ord(up) == shift and up != bc


def fmt_cp(cp):
    """Format a code point as a readable C token."""
    if cp == 0:
        return "0"
    if cp == ord("'"):
        return "'\\''"
    if cp == ord("\\"):
        return "'\\\\'"
    if 0x20 <= cp <= 0x7e:
        return "'%s'" % chr(cp)
    return "0x%04x" % cp


def gen_entries(names, table):
    lines = []
    for usage in range(len(table)):
        if usage in EXCLUDED or usage < 0x04:
            continue
        base, shift, altgr, altgrshift = table[usage]
        if base == 0 and shift == 0 and altgr == 0 and altgrshift == 0:
            continue
        if usage not in names:
            raise SystemExit("no KeyboardUsage name for 0x%02x" % usage)
        caps = "true" if caps_affects(base, shift) else "false"
        parts = ["U(KeyboardUsage::%s)" % names[usage], fmt_cp(base), fmt_cp(shift), caps]
        # Only spell out the AltGr planes when present (compact rows otherwise).
        if altgr != 0 or altgrshift != 0:
            parts.append(fmt_cp(altgr))
            if altgrshift != 0:
                parts.append(fmt_cp(altgrshift))
        lines.append("    {%s}," % ", ".join(parts))
    return lines


def main():
    shared = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_SHARED
    names = parse_usage_names(os.path.join(REPO, "src", "ESP32KeyBridge.h"))

    out = []
    out.append("// AUTO-GENERATED by tools/gen_keymaps.py -- do not edit by hand.")
    out.append("// Source: EspUsbHost/src/keymap/keymap_*.h (shared 4-plane tables).")
    out.append("// Regenerate with: python3 tools/gen_keymaps.py")
    out.append("// clang-format off")

    registry = []
    for loc in LOCALES:
        path = os.path.join(shared, "keymap_%s.h" % loc)
        if not os.path.exists(path):
            raise SystemExit("missing %s" % path)
        table = parse_table(path)
        entries = gen_entries(names, table)
        arr = "k%sEntries" % loc.title().replace("_", "")
        out.append("")
        out.append("const KeyboardLayoutEntry %s[] = {" % arr)
        out.extend(entries)
        out.append("};")
        registry.append((loc, arr))

    out.append("")
    out.append("struct GeneratedLayout")
    out.append("{")
    out.append("  const char *name;")
    out.append("  const KeyboardLayoutEntry *entries;")
    out.append("  size_t count;")
    out.append("};")
    out.append("")
    out.append("const GeneratedLayout kGeneratedLayouts[] = {")
    for loc, arr in registry:
        out.append('    {"%s", %s, sizeof(%s) / sizeof(%s[0])},' % (loc, arr, arr, arr))
    out.append("};")
    out.append("// clang-format on")
    out.append("")

    dest = os.path.join(REPO, "src", "ESP32KeyBridgeLayouts.inc")
    open(dest, "w", encoding="utf-8").write("\n".join(out) + "\n")
    print("wrote %s (%d locales)" % (dest, len(registry)))


if __name__ == "__main__":
    main()
