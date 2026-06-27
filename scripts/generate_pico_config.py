import sys
import re

HEADER = """#pragma once
#include <cstdint>
#include <cstddef>

// ======================================================
// AUTO-GENERATED FROM CONFIG.INI (DO NOT EDIT)
// ======================================================

enum PicoType : uint8_t
{
    BOOL = 0,
    INT,
    FLOAT,
    STRING
};

struct PicoConfigEntry
{
    uint32_t hash;
    PicoType type;

    union {
        bool b;
        int i;
        float f;
        const char* s;
    };
};

inline constexpr uint32_t fnv1a(const char* str)
{
    uint32_t hash = 2166136261u;
    while (*str)
    {
        hash ^= (uint8_t)(*str++);
        hash *= 16777619u;
    }
    return hash;
}

inline constexpr PicoConfigEntry PICO_CONFIG_TABLE[] =
{
"""

FOOTER = """
};

// ======================================================
// TABLE SIZE
// ======================================================

inline constexpr size_t PICO_CONFIG_TABLE_SIZE =
    sizeof(PICO_CONFIG_TABLE) / sizeof(PICO_CONFIG_TABLE[0]);

// ======================================================
// END AUTO-GENERATED CONFIG
// ======================================================
"""


# -----------------------------
# helpers
# -----------------------------
DROP_KEYS = {"version", "mute_audio"}

def is_bool(v):
    return v.lower() in ["true", "false"]

def to_bool(v):
    return "true" if v.lower() == "true" else "false"

def is_int(v):
    return re.fullmatch(r"-?\d+", v) is not None

def is_float(v):
    return re.fullmatch(r"-?\d+\.\d+", v) is not None

def escape_str(v):
    return v.replace("\\", "\\\\").replace('"', '\\"')


def fnv1a_py(s: str):
    h = 2166136261
    for c in s:
        h ^= ord(c)
        h = (h * 16777619) & 0xffffffff
    return h


# -----------------------------
# parser
# -----------------------------
def parse_ini(path):
    data = {}
    section = None

    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()

            if not line or line.startswith(";"):
                continue

            if line.startswith("[") and line.endswith("]"):
                section = line[1:-1].lower()
                data.setdefault(section, {})
                continue

            if "=" in line and section:
                k, v = line.split("=", 1)
                data[section][k.strip()] = v.strip()

    return data


# -----------------------------
# emitter
# -----------------------------
def make_entry(hash_val, t, value):
    if t == "BOOL":
        return f"    {{ {hash_val}u, PicoType::BOOL, {{ .b = {value} }} }},"

    if t == "INT":
        return f"    {{ {hash_val}u, PicoType::INT, {{ .i = {value} }} }},"

    if t == "FLOAT":
        return f"    {{ {hash_val}u, PicoType::FLOAT, {{ .f = {value}f }} }},"

    return f"    {{ {hash_val}u, PicoType::STRING, {{ .s = \"{escape_str(value)}\" }} }},"


def convert(section, key, value):
    if key.lower() in DROP_KEYS:
        return None

    full = f"{section}.{key}"
    h = fnv1a_py(full)

    if is_bool(value):
        return make_entry(h, "BOOL", to_bool(value))
    if is_int(value):
        return make_entry(h, "INT", value)
    if is_float(value):
        return make_entry(h, "FLOAT", value)

    return make_entry(h, "STRING", value)


# -----------------------------
# generator
# -----------------------------
def generate(inp, outp):
    cfg = parse_ini(inp)

    out = [HEADER]

    for section, values in cfg.items():
        out.append(f"    // ===== {section.upper()} =====")

        for k, v in values.items():
            line = convert(section, k, v)
            if line:
                out.append(line)

    out.append(FOOTER)

    with open(outp, "w", encoding="utf-8") as f:
        f.write("\n".join(out))

    print(f"[OK] Generated {outp}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python generate_pico_config.py CONFIG.ini pico_config.h")
        sys.exit(1)

    generate(sys.argv[1], sys.argv[2])