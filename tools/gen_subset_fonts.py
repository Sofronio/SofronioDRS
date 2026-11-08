#!/usr/bin/env python3
"""
Build-time U8g2 font subsetter.

Scans the project's source/header string literals for the characters actually
used by the UI, extracts those glyphs from the full U8g2 wqy GB2312 fonts and
writes a compact subset font header (include/generated_fonts.h).

The glyph RLE payloads are copied verbatim, so rendering is identical; only
unused glyphs and the large GB2312 lookup tables are dropped.

Usage:
  python3 tools/gen_subset_fonts.py [project_dir] [env_name]

Called automatically from platformio.ini as a pre-build script.
"""

import os
import re
import sys

FONT_SPECS = [
    ("u8g2_font_wqy12_t_gb2312", "u8g2_font_wqy12_subset"),
    ("u8g2_font_wqy14_t_gb2312", "u8g2_font_wqy14_subset"),
    ("u8g2_font_wqy16_t_gb2312", "u8g2_font_wqy16_subset"),
]

HEADER_SIZE = 23


# --------------------------------------------------------------------------
# C string / comment scanning
# --------------------------------------------------------------------------
def strip_c_comments(text: str) -> str:
    """Remove // and /* */ comments while preserving string literals."""
    out = []
    i = 0
    n = len(text)
    state = "code"
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if state == "code":
            if c == "/" and nxt == "/":
                state = "line_comment"
                i += 2
                continue
            if c == "/" and nxt == "*":
                state = "block_comment"
                i += 2
                continue
            if c == '"':
                state = "string"
                out.append(c)
                i += 1
                continue
            if c == "'":
                state = "char"
                out.append(c)
                i += 1
                continue
            out.append(c)
            i += 1
        elif state == "line_comment":
            if c == "\n":
                state = "code"
                out.append(c)
            i += 1
        elif state == "block_comment":
            if c == "*" and nxt == "/":
                state = "code"
                i += 2
                continue
            i += 1
        elif state == "string":
            out.append(c)
            if c == "\\" and i + 1 < n:
                out.append(text[i + 1])
                i += 2
                continue
            if c == '"':
                state = "code"
            i += 1
        elif state == "char":
            out.append(c)
            if c == "\\" and i + 1 < n:
                out.append(text[i + 1])
                i += 2
                continue
            if c == "'":
                state = "code"
            i += 1
    return "".join(out)


STRING_RE = re.compile(r'"((?:\\.|[^"\\])*)"', re.DOTALL)


def collect_characters(project_dir: str):
    chars = set(chr(c) for c in range(0x20, 0x7F))  # printable ASCII
    sources = []
    for sub in ("src", "include"):
        root = os.path.join(project_dir, sub)
        for dirpath, _dirnames, filenames in os.walk(root):
            for fn in filenames:
                if fn.endswith((".h", ".hpp", ".c", ".cpp", ".ino")):
                    if fn == "generated_fonts.h":
                        continue
                    sources.append(os.path.join(dirpath, fn))
    for path in sources:
        try:
            text = open(path, encoding="utf-8", errors="ignore").read()
        except OSError:
            continue
        text = strip_c_comments(text)
        for m in STRING_RE.finditer(text):
            literal = m.group(1)
            # Decode the common C escapes; keep UTF-8 source characters.
            literal = literal.replace('\\"', '"').replace("\\n", "\n").replace("\\t", "\t")
            for ch in literal:
                if ord(ch) >= 0x80:
                    chars.add(ch)
    return chars, sources


# --------------------------------------------------------------------------
# U8g2 font data parsing
# --------------------------------------------------------------------------
def decode_c_escapes(raw: str) -> bytes:
    data = bytearray()
    i = 0
    n = len(raw)
    while i < n:
        c = raw[i]
        if c != "\\":
            data.extend(c.encode("latin-1", errors="ignore"))
            i += 1
            continue
        i += 1
        if i >= n:
            break
        c = raw[i]
        if c in "01234567":
            j = i
            while j < n and j < i + 3 and raw[j] in "01234567":
                j += 1
            data.append(int(raw[i:j], 8) & 0xFF)
            i = j
        elif c == "x":
            j = i + 1
            while j < n and j < i + 3 and raw[j] in "0123456789abcdefABCDEF":
                j += 1
            data.append(int(raw[i + 1:j], 16) & 0xFF)
            i = j
        else:
            mapping = {"n": 10, "r": 13, "t": 9, "\\": 92, '"': 34, "'": 39, "0": 0}
            data.append(mapping.get(c, ord(c) & 0xFF))
            i += 1
    return bytes(data)


def extract_font_array(text: str, name: str) -> bytes:
    marker = 'U8G2_FONT_SECTION("%s")' % name
    pos = text.find(marker)
    if pos < 0:
        raise RuntimeError("font %s not found in u8g2_fonts.c" % name)
    eq = text.find("=", pos)
    i = text.find('"', eq)
    if i < 0:
        raise RuntimeError("font %s: missing string data" % name)
    chunks = []
    n = len(text)
    while i < n:
        # i points at opening quote
        i += 1
        start = i
        while i < n:
            if text[i] == "\\":
                i += 2
                continue
            if text[i] == '"':
                break
            i += 1
        chunks.append(text[start:i])
        i += 1
        while i < n and text[i] in " \t\r\n":
            i += 1
        if i < n and text[i] == '"':
            continue
        break
    return decode_c_escapes("".join(chunks))


def be16(buf: bytes, off: int) -> int:
    return (buf[off] << 8) | buf[off + 1]


def parse_glyph(font: bytes, enc: int):
    """Return (data_start, size) or None, following u8g2_font.c."""
    after = HEADER_SIZE
    if enc <= 255:
        if enc >= 0x61:
            pos = after + be16(font, 19)
        elif enc >= 0x41:
            pos = after + be16(font, 17)
        else:
            pos = after
        while True:
            if pos + 1 >= len(font):
                return None
            total = font[pos + 1]  # entry length including the 2 header bytes
            if total == 0:
                return None
            if font[pos] == enc:
                return pos + 2, total - 2
            pos += total
    else:
        table = after + be16(font, 21)
        t = table
        pos = table  # u8g2_font.c accumulates every index offset
        while True:
            off = be16(font, t)
            max_enc = be16(font, t + 2)
            t += 4
            pos += off
            if max_enc >= enc:
                break
            if t + 4 > len(font):
                return None
        while True:
            if pos + 2 >= len(font):
                return None
            e = be16(font, pos)
            if e == 0:
                return None
            total = font[pos + 2]  # entry length including the 3 header bytes
            if e == enc:
                return pos + 3, total - 3
            pos += total


def build_subset(font: bytes, chars) -> bytes:
    # Header fields 1..16 are copied (bbx mode, RLE encoding, metrics).
    header = bytearray(font[:HEADER_SIZE])

    lists = {0: [], 1: [], 2: []}  # encodings <'A', 'A'..'`', >= 'a'
    unicode_glyphs = []
    missing = []

    for ch in sorted(chars, key=ord):
        enc = ord(ch)
        glyph = parse_glyph(font, enc)
        if glyph is None:
            missing.append(ch)
            continue
        start, size = glyph
        payload = font[start:start + size]
        if enc <= 255:
            if enc >= 0x61:
                lists[2].append((enc, payload))
            elif enc >= 0x41:
                lists[1].append((enc, payload))
            else:
                lists[0].append((enc, payload))
        else:
            unicode_glyphs.append((enc, payload))

    body = bytearray()

    def append_ascii_list(items):
        for enc, payload in items:
            body.append(enc)
            body.append(len(payload) + 2)  # total entry length (encoding + size + data)
            body.extend(payload)
        body.extend(b"\x00\x00")  # size byte 0 terminates the list

    list0_off = len(body)
    append_ascii_list(lists[0])
    listA_off = len(body)
    append_ascii_list(lists[1])
    lista_off = len(body)
    append_ascii_list(lists[2])
    unicode_off = len(body)

    # Single unicode block: index entry (offset=4, max=0xFFFF), then entries.
    body.extend(bytes([0x00, 0x04, 0xFF, 0xFF]))
    for enc, payload in unicode_glyphs:
        body.append((enc >> 8) & 0xFF)
        body.append(enc & 0xFF)
        body.append(len(payload) + 3)  # total entry length (2-byte encoding + size + data)
        body.extend(payload)
    body.extend(b"\x00\x00")

    header[0] = min(255, len(lists[0]) + len(lists[1]) + len(lists[2]))
    header[17:19] = listA_off.to_bytes(2, "big")
    header[19:21] = lista_off.to_bytes(2, "big")
    header[21:23] = unicode_off.to_bytes(2, "big")

    return bytes(header) + bytes(body), missing, len(lists[0]) + len(lists[1]) + len(lists[2]), len(unicode_glyphs)


def find_font_file(project_dir: str, env_name: str) -> str:
    candidates = [
        os.path.join(project_dir, ".pio", "libdeps", env_name, "U8g2", "src", "clib", "u8g2_fonts.c"),
    ]
    for cand in candidates:
        if os.path.isfile(cand):
            return cand
    # Fallback: search libdeps for any U8g2 copy.
    libdeps = os.path.join(project_dir, ".pio", "libdeps")
    for dirpath, _dirnames, filenames in os.walk(libdeps):
        if "u8g2_fonts.c" in filenames and os.path.basename(dirpath) == "clib":
            return os.path.join(dirpath, "u8g2_fonts.c")
    raise RuntimeError("u8g2_fonts.c not found; build once so PlatformIO downloads U8g2")


def generate(project_dir: str, env_name: str, verbose: bool = True):
    chars, sources = collect_characters(project_dir)
    font_file = find_font_file(project_dir, env_name)
    text = open(font_file, encoding="latin-1", errors="ignore").read()

    outputs = []
    total_chars = 0
    for original, subset_name in FONT_SPECS:
        font = extract_font_array(text, original)
        subset, missing, ascii_count, unicode_count = build_subset(font, chars)
        outputs.append((subset_name, subset))
        total_chars = max(total_chars, ascii_count + unicode_count)
        if verbose:
            print("[fonts] %s: %d glyphs (%d ascii + %d unicode), %d -> %d bytes%s" % (
                original, ascii_count + unicode_count, ascii_count, unicode_count,
                len(font), len(subset),
                "" if not missing else ", missing: " + "".join(missing[:12])))
        if missing:
            print("[fonts] WARNING: %s missing %d glyphs: %s" % (
                original, len(missing), "".join(missing)))

    out_path = os.path.join(project_dir, "include", "generated_fonts.h")
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    lines = []
    lines.append("// Auto-generated by tools/gen_subset_fonts.py - do not edit.\n")
    lines.append("// Characters are collected from string literals in src/ and include/.\n")
    lines.append("#ifndef GENERATED_FONTS_H\n#define GENERATED_FONTS_H\n\n")
    lines.append("#include <stdint.h>\n\n")
    for name, data in outputs:
        lines.append("// %d bytes\n" % len(data))
        lines.append("static const uint8_t %s[] = {\n" % name)
        for i in range(0, len(data), 16):
            chunk = data[i:i + 16]
            lines.append("  " + ",".join("0x%02x" % b for b in chunk) + ",\n")
        lines.append("};\n\n")
    lines.append("#endif\n")
    new_text = "".join(lines)
    old_text = None
    if os.path.isfile(out_path):
        try:
            old_text = open(out_path, encoding="utf-8").read()
        except OSError:
            old_text = None
    if old_text != new_text:
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(new_text)
        if verbose:
            print("[fonts] wrote %s (%d unique UI characters)" % (out_path, len(chars)))
    elif verbose:
        print("[fonts] %s unchanged (%d unique UI characters)" % (out_path, len(chars)))
    return out_path


if __name__ == "__main__":
    project = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    env = sys.argv[2] if len(sys.argv) > 2 else os.environ.get("PIOENV", "esp32s3_v8_1")
    generate(project, env)
