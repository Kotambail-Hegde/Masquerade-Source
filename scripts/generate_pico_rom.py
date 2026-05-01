import sys
import os

HEADER = """#pragma once

#pragma region INCLUDES
#include "helpers.h"
#pragma endregion INCLUDES

#pragma region CORE

"""

FOOTER = """
#pragma endregion CORE
"""


# -----------------------------
# helpers
# -----------------------------
def to_hex_array(data, per_line=12):
    lines = []
    line = []

    for i, b in enumerate(data):
        line.append(f"0x{b:02x}")

        if (i + 1) % per_line == 0:
            lines.append("  " + ", ".join(line) + ",")
            line = []

    if line:
        lines.append("  " + ", ".join(line))

    return "\n".join(lines)


# -----------------------------
# generator
# -----------------------------
def generate(rom_path, out_path):
    with open(rom_path, "rb") as f:
        data = f.read()

    rom_name = os.path.basename(rom_path)

    out = []
    out.append(HEADER)

    # ROM name
    out.append(f'inline const std::string PICO_ROM_NAME = "{rom_name}";\n')

    # ROM data
    out.append("inline constexpr uint8_t PICO_ROM_DATA[] = {")
    out.append(to_hex_array(data))
    out.append("};\n")

    # ROM size
    out.append(f"inline constexpr size_t PICO_ROM_SIZE = sizeof(PICO_ROM_DATA);\n")

    out.append(FOOTER)

    with open(out_path, "w", encoding="utf-8") as f:
        f.write("\n".join(out))

    print(f"[OK] Generated ROM header: {out_path}")
    print(f"[INFO] ROM size: {len(data)} bytes")


# -----------------------------
# CLI
# -----------------------------
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python generate_pico_rom.py input.rom pico_rom.h")
        sys.exit(1)

    generate(sys.argv[1], sys.argv[2])
    