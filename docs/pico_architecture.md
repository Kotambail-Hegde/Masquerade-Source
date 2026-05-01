# Masquerade — Raspberry Pi Pico Architecture

## Overview

Masquerade is a multi-platform emulator targeting both desktop (Windows/Linux/WebAssembly) and bare-metal embedded hardware (Raspberry Pi Pico). The codebase is shared across all targets via `#ifdef` guards and CMake feature flags.

---

## Directory Structure

```
masquerade/
├── core/               — shared emulator orchestration
├── chip8/              — CHIP-8 / SCHIP / XO-CHIP emulator
├── gb-gbc/             — Game Boy / Game Boy Color emulator
├── nes/                — NES emulator
├── gba/                — GBA emulator (WIP)
├── helpers/            — shared types, macros, logger
├── pico/               — Pico-specific: config, ROM
│   ├── pico_config.h   — auto-generated compile-time config table
│   └── pico_rom.h      — auto-generated ROM data (flash)
├── panel/              — physical display abstraction (Pico only)
│   ├── panel.h         — ONLY file emulators include
│   ├── panel_interface.h — WCTX, PANEL_SET_PARAMS
│   ├── panel_utils.h   — generic blitters, draw primitives, FPS overlay
│   └── backends/
│       ├── picolcd2/   — Waveshare 2in LCD driver files
│       │   picolcd2_backend.h
│       └── ili9341/    — ILI9341 driver files (SD card capable)
│           ili9341_backend.h
└── ui/                 — desktop only: ImGui, OpenGL, SDL
```

---

## CMake Options

### Platform Selection
| Option | Default | Description |
|--------|---------|-------------|
| `-DRPI_PICO=ON` | OFF | Build for Raspberry Pi Pico (bare-metal) |
| `-DEMSCRIPTEN=ON` | OFF | Build for WebAssembly |
| `-DRPI_5=ON` | OFF | Build for Raspberry Pi 5 (GLES) |

These are mutually exclusive — enabling one disables the others.

### Panel Backend (Pico only)
| Option | Description |
|--------|-------------|
| `-DPANEL_BACKEND=PICOLCD2` | Waveshare 2in SPI LCD — no SD card |
| `-DPANEL_BACKEND=ILI9341` | ILI9341 SPI LCD — with SD card slot |

### Module Selection
| Option | Example | Description |
|--------|---------|-------------|
| `-DMASQ_ONLY` | `CHIP8;GBC;NES` | Build only specified emulators |

### Example Build Commands
```bash
# Desktop (Windows/Linux)
cmake ..
cmake --build .

# Pico with Waveshare 2in
cmake -DRPI_PICO=ON -DPANEL_BACKEND=PICOLCD2 -DMASQ_ONLY=CHIP8 ..

# Pico with ILI9341 (SD card)
cmake -DRPI_PICO=ON -DPANEL_BACKEND=ILI9341 -DMASQ_ONLY=CHIP8 ..

# Pico GBC only
cmake -DRPI_PICO=ON -DPANEL_BACKEND=PICOLCD2 -DMASQ_ONLY=GBC ..
```

---

## Panel Abstraction Layer

### Purpose
Emulators should not know or care which physical display is connected. `panel.h` provides a unified interface — swapping backends requires only a CMake flag change, zero code changes in emulator modules.

### Abstract Interface (defined by each backend)
```cpp
PANEL_SCREEN_WIDTH          // uint32_t — physical panel width
PANEL_SCREEN_HEIGHT         // uint32_t — physical panel height
PANEL_BACKEND_INIT()        // initialize panel hardware
PANEL_BACKEND_CLEAR(c)      // fill panel with color (RGB565)
PANEL_BACKEND_PRESENT(fb)   // send framebuffer to panel over SPI
PANEL_BACKEND_POINT(x,y,c)  // draw single pixel
```

### Emulator Usage
```cpp
// in chip8.h, gbc.h, nes.h etc.
#ifdef __RPI_PICO__
#include "panel/panel.h"   // physical panel — Pico only
#endif
// Desktop rendering handled by ImGui/SDL — no panel include needed
```

### Backend Visibility
`-DPANEL_BACKEND=ILI9341` adds `PANEL_BACKEND_ILI9341` as a global compile definition visible in **all** translation units including emulator modules. This allows:
```cpp
#ifdef PANEL_BACKEND_ILI9341
    // SD card available
    // load ROM from SD card
    // XO-CHIP 4-plane mode supported (more RAM available)
#else
    // PICOLCD2 — hardcoded ROM in flash
    // CHIP-8 standard mode only (1 plane)
#endif
```

---

## Platform-Specific Transform Functions

Each emulator has a different pixel and layer format. Transform functions convert emulator-native format to expected format for the panel framebuffer.

| Function | Used by | Input format |
|----------|---------|--------------|
| `PANEL_TRANSFORM_FOR_CHIP8` | CHIP-8 |
| `PANEL_TRANSFORM_FOR_GBC`   | GBC |
| `PANEL_TRANSFORM_FOR_NES`   | NES |

---

## PICOLCD2 (Waveshare 2in) — Capabilities & Limitations

### Specs
- Resolution: 320×240 (landscape via scan direction)
- Interface: SPI
- No SD card
- No touch

### Performance
| SPI Clock | Theoretical FPS | Measured FPS |
|-----------|-----------------|--------------|
| 10 MHz | ~5 | 5.6 |
| 40 MHz | ~27 | 18 |
| 62.5 MHz | ~33 | 34 |

62.5 MHz is the stable ceiling at 125 MHz system clock (125/2 = clean divider).

### Limitations
- **No SD card** → ROM must be compiled into flash via `pico_rom.h`
- **Single ROM at a time** → ROM baked at compile time via `generate_pico_rom.py`
- **Limited SRAM (264KB)** → XO-CHIP 4-plane mode not supported (too much gfx memory)
- **FPS ceiling** → SPI bandwidth limited; NES/GBC may need frameskip
- **Framebuffer layout** → column-major (`fb[x * HEIGHT + y]`) to match `LCD_2IN_Display`

### SRAM Budget (CHIP-8)
```
absolute_chip8_instance_t  ~12KB
chip8_t object             ~4KB
waveshare framebuffer      153KB  (malloc'd)
stack + SDK                ~20KB
─────────────────────────────────
total                      ~189KB / 264KB

```

### SRAM Budget (GBC)
```
TBD

```

### SRAM Budget (NES)
```
TBD

```

---

## ILI9341 — Capabilities & Future Possibilities

### Specs
- Resolution: 320×240 (typically)
- Interface: SPI
- **SD card slot** → runtime ROM loading
- Optional touch

### Advantages over PICOLCD2
| Feature | PICOLCD2 | ILI9341 |
|---------|----------|---------|
| ROM loading | Compile-time (flash) | Runtime (SD card) |
| ROM size limit | Flash size (~2MB) | SD card (GB+) |
| Multiple ROMs | Recompile required | File picker at runtime |
| CONFIG.ini | Recompile required | Read at runtime |
| XO-CHIP support | No (1 plane only) | Possible (select via `-DPANEL_BACKEND=ILI9341`) |
| CHIP-8 DB lookup | Limited | Full database on SD |

### Future Possibilities with ILI9341
- **ROM picker menu** — list ROMs from SD card, select at runtime
- **CONFIG load support** — Config can be dynamically read and updated
- **Save states to SD** — persistent save/load without reflashing
- **XO-CHIP 4-plane support** — enabled when `PANEL_BACKEND_ILI9341` defined
- **CHIP-8 database on SD** — full `programs.json` / `quirks.json` lookup
- **GBC SRAM saves** — battery save equivalent via SD

---

## Dual Core Strategy

Core1 handles SPI transfer, Core0 handles emulation. For CHIP-8 (~1ms emulation vs ~30ms SPI) the gain is minimal. For GBC/NES (~15-20ms emulation) the gain is significant.

```
Core0: [emulate][transform][signal Core1][emulate][transform][signal Core1]...
Core1:          [SPI 30ms ]              [SPI 30ms]
```

### API
```cpp
PANEL_INIT_CORE1(pctx);       // call once at init
PANEL_PRESENT_FRAME(pctx);    // call each frame — signals Core1, returns immediately
```

---

## ROM Management

### PICOLCD2 — Compile-time ROM
```bash
# Generate ROM header from .ch8 file
python scripts/generate_pico_rom.py path/to/game.ch8

# Rebuild
cmake -DRPI_PICO=ON -DPANEL_BACKEND=PICOLCD2 ..
cmake --build .
```

### ILI9341 — Runtime ROM (planned)
```
SD card structure:
  /roms/
    chip8/   ← .ch8 files
    gbc/     ← .gbc files
    nes/     ← .nes files
  /saves/
  /config/
    CONFIG.ini
  /db/
    programs.json
    quirks.json
```

---

## Per-Platform Emulator Status

| Emulator | Desktop | PICOLCD2 | ILI9341 |
|----------|---------|----------|---------|
| CHIP-8 | ✅ | ✅ | ✅ planned |
| SCHIP | ✅ | ✅ | ✅ planned |
| XO-CHIP | ✅ | ⚠️ 1-plane only | ✅ planned |
| GBC | ✅ | 🚧 WIP | 🚧 WIP |
| NES | ✅ | 🔜 | 🔜 |
| GBA | ✅ | ❌ too heavy | ❌ |

---

## Adding a New Panel Backend

1. Create `panel/backends/myboard/myboard_backend.h`
2. Define all `PANEL_*` macros
3. Add `.c` source files
4. Add `elseif(PANEL_BACKEND STREQUAL "MYBOARD")` block in `CMakeLists.txt`
5. Build with `-DPANEL_BACKEND=MYBOARD`

Zero changes needed in emulator code.

---

## Known Issues & Constraints

- **`std::string` on Pico** — heap allocated, avoid in hot paths; use `const char*` or fixed `char[]`
- **`new` / `make_shared` on Pico** — heap is ~100KB usable; large allocations must be sized carefully
- **`tone[48000]` guard** — full 48kHz audio buffer must be `#ifdef`'d out on Pico; buzzer only
- **Pico framebuffer** — 153KB for 320×240 RGB565; must be `malloc`'d not stack/global
- **Column-major layout** — `LCD_2IN_Display` reads `fb[x * HEIGHT + y]`; all writes must match
- **SPI after overclock** — always call `spi_init` after `set_sys_clock_khz`; SPI divider is recalculated at init time
