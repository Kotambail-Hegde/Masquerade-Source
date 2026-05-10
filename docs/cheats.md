# Cheat Engine Documentation

## Overview

The cheat engine (`CheatEngine_t`) is a unified, platform-agnostic system that supports multiple cheat
code formats across NES, GB/GBC, and GBA. It is designed around two mechanisms:

- **Read-intercept** (`interceptCPURead`): substitutes a value when the CPU reads a specific address.
  Used for ROM patches — hardware-accurate for GameGenie, which physically sat on the cartridge bus.
- **VBlank write** (`getCheatWrites`): writes values directly to RAM at every VBlank.
  Used for GameShark, Action Replay, and CodeBreaker — hardware-accurate, as real cheat devices
  ran their own firmware at VBlank and stomped game-written values once per frame.

All memory writes go through the platform's native `writeRawMemory` with `MEMORY_ACCESS_SOURCE::CPU`,
meaning all hardware side effects (APU register triggers, MBC banking, DMA conflict checks, etc.)
behave identically to a real CPU write. This is intentional and hardware-accurate.

---

## Supported Platforms and Engines

### NES

| Engine      | Mechanism      | Status    |
|-------------|----------------|-----------|
| Game Genie  | Read-intercept | Supported |

#### NES Game Genie

- **Format**: 6-char (no compare) or 8-char (with compare), encoded using the Game Genie alphabet.
- **Address range**: ROM only (`0x8000` - `0xFFFF`).
- **Compare byte**: 8-char codes include a compare byte — the substitution only fires when the
  current ROM byte matches the original value. This is hardware-accurate; the real Game Genie
  cartridge used the compare byte to handle bank-switched ROMs where the same address in different
  banks holds different data.
- **Multiple codes**: separated by ` + ` (space-plus-space), `+` alone, or whitespace.
  Whitespace delimiter is preserved without stripping — necessary to correctly disambiguate
  mixed 6-char and 8-char codes in the same input string.
- **Decode reference**: https://tuxnes.sourceforge.net/gamegenie.html

---

### GB / GBC

| Engine      | Mechanism      | Status    |
|-------------|----------------|-----------|
| Game Genie  | Read-intercept | Supported |
| GameShark   | VBlank write   | Supported |

#### GB/GBC Game Genie

- **Format**: `XXX-XXX-XXX` (11 chars with dashes; 9 hex chars after dash removal).
- **Address range**: ROM only (`0x0000` - `0x7FFF`).
- **Compare byte**: always present. The compare byte is encoded via rotate-left-6 then XOR `0xBA`.
  Hardware-accurate — the real Game Genie cartridge sat on the bus and compared before substituting.
- **Multiple codes**: space-separated. Dashes and `+` are stripped before splitting.
- **Decode reference**: https://gbdev.io/pandocs/Shark_Cheats.html
- **Compare decode reference**: https://www.youtube.com/watch?v=C86OsYRACTM

#### GB/GBC GameShark

- **Format**: 8 hex chars per code (`TTDDAAAA` where TT = type/bank, DD = data, AAAA = address).
- **Address range**: RAM (WRAM, SRAM, ERAM). ROM writes are not a GameShark feature.
- **Mechanism**: VBlank write — hardware-accurate. The real GameShark ran its own firmware at VBlank
  and wrote values directly to RAM, overwriting whatever the game had written during the frame.
  This means the game can see its own value briefly during the frame before VBlank stomps it —
  same window that existed on real hardware, including any flicker this causes.
- **Bank byte**: the type/bank byte (`TT`) is stored in `compare` in `CheatPatch_t`. It is NOT
  a value compare — it was used by the original read-intercept implementation to identify which
  RAM bank to target. With the move to VBlank writes, the bank byte is no longer needed at the
  write site (the address is written directly via `writeRawMemory` which handles banking internally).
  It is retained in the struct for potential future use.
- **Multiple codes**: remove all separators, chunk by 8 chars.
- **Decode reference**: https://gbdev.io/pandocs/Shark_Cheats.html

---

### GBA

| Engine            | Mechanism                      | Status             |
|-------------------|--------------------------------|--------------------|
| GameShark v1/v2   | VBlank write (RAM) / Read-intercept (ROM) | Supported (RAW format only) |
| Action Replay v3  | VBlank write (RAM) / Read-intercept (ROM) | Supported (basic opcodes) |
| CodeBreaker       | VBlank write (RAM)             | Supported (basic types) |

#### GBA GameShark v1/v2

- **Format**: 16 hex chars per code (`TWWWWWWW VVVVVVVV`).
  Top nibble `T` of the first word encodes width; lower 28 bits are the address; second word is value.
- **Width encoding**:
  - `0x0` -> 8-bit write, value masked to `0xFF`
  - `0x1` -> 16-bit write, value masked to `0xFFFF`
  - `0x2` -> 32-bit write
- **RAM writes**: applied at VBlank via `getCheatWrites`.
- **ROM patches**: addresses `>= 0x08000000` are intercepted via `interceptCPURead`.
- **Encryption**: NOT supported. Only raw/decrypted codes are accepted.
  Most GameShark v1/v2 codes found online are already in raw format for emulator use.
- **Multiple codes**: remove all separators, chunk by 16 chars.
- **Reference**: https://problemkaputt.de/gbatek-gba-cheat-codes-gameshark-action-replay-v1-v2.htm

#### GBA Action Replay v3

- **Format**: 16 hex chars per code, TEA-encrypted.
- **Encryption**: TEA (Tiny Encryption Algorithm) with fixed seeds
  `{0x7AA9648F, 0x7FAE6994, 0xC0EFAAD5, 0x42712C57}`, 32 rounds, initial sum `0xC6EF3720`.
- **Address expansion after decrypt**:
  raw 24-bit address `aaaaaa` -> full GBA address via `((rawAddr & 0xF00000) << 4) | (rawAddr & 0x0FFFFF)`.
  Example: `0x225E90` -> `0x02025E90` (EWRAM).
- **Supported opcodes**:
  - `0x00` -> 8-bit RAM write (VBlank)
  - `0x02` -> 16-bit RAM write (VBlank)
  - `0x04` -> 32-bit RAM write (VBlank)
  - `0x06` -> 16-bit ROM patch, no compare (read-intercept)
  - `0x07` -> 8-bit ROM patch with compare (read-intercept)
- **Unsupported opcodes**: conditionals, fill/range writes, indirect writes, IO-area writes.
  These return `FAILURE` from `decodeAddressAndData` and are silently skipped. The cheat entry
  is still registered in the UI so the user can see and delete it.
- **DEADFACE master codes**: NOT supported. DEADFACE codes (`DEADFACE 0000XXXX`) change the TEA
  encryption seeds for subsequent codes. Codes that depend on a non-default seed will decrypt
  to garbage addresses and fail decode gracefully.
- **Master codes in general**: NOT supported for any engine. Master codes install hooks into game
  code that run within the game's execution context at VBlank. Supporting them properly requires
  a full code injection and execution system, which is out of scope.
- **Multiple codes**: remove all separators, chunk by 16 chars.
- **Reference**: https://problemkaputt.de/gbatek-gba-cheat-codes-pro-action-replay-v3.htm

#### GBA CodeBreaker / Xploder

- **Format**: NOT encrypted. Plaintext hex codes.
  - Standard codes: 16 chars (`TAAAAAAA VVVVVVVV`)
  - Short codes (type 8): 12 chars (`8AAAAAAA VVVV`)
- **Address**: lower 28 bits of first word, directly usable as a GBA address (no expansion needed).
  Example: `0x82005274` -> address `0x02005274` (EWRAM).
- **Supported type nibbles**:
  - `0x0` -> 8-bit RAM write (VBlank)
  - `0x1` -> 16-bit RAM write (VBlank)
  - `0x2` -> 32-bit RAM write (VBlank)
  - `0x8` -> 16-bit RAM write, short format (VBlank)
- **Unsupported type nibbles**:
  - `0x3` -> conditional — NOT supported, returns FAILURE
  - `0xB` -> master/hook code — NOT supported, returns FAILURE
- **ROM patches**: CodeBreaker has no ROM patch opcode. All writes target RAM.
- **Multiple codes**: mixed-length aware. If total length is divisible by 16, chunk by 16.
  If divisible by 12 (not 16), chunk by 12. Otherwise parse sequentially by first nibble.
- **Reference**: https://problemkaputt.de/gbatek-gba-cheat-codes-codebreaker-xploder.htm

---

## Architecture

### `CheatPatch_t` struct

```
data       : uint32_t  — value to write or substitute
compare    : uint32_t  — compare byte (GameGenie) or bank byte (GB GameShark)
hasCompare : FLAG      — whether compare is a value compare (NO for GameShark bank byte)
width      : CheatWidth — U8 / U16 / U32
enabled    : FLAG
```

### `fakeData` table

Two-dimensional unordered_map keyed by `[CHEATING_ENGINE][EMULATION_ID]`, then by `uint32_t address`.
All engines share the same table structure. The read and VBlank write paths both query this table.

### Key canonicalization

All cheat strings are canonicalized (separators stripped) before being used as map keys.
This means `"OZTLLX + AATLGZ"`, `"OZTLLX AATLGZ"`, and `"OZTLLXAATLGZ"` all map to the same entry.
The original raw string is split into sub-codes at `applyNewCheat` time and stored in `cheatSubCodes`
so that `enableCheat`, `disableCheat`, and `deleteCheat` never need to re-split a canonical key
(which would fail for multi-code inputs like `"OZTLLX + AATLGZ"`).

### Persistence (`saveCheatNames` / `loadCheatNames`)

Cheat names are saved as:
```
key=name\tsubcode1\tsubcode2\t...
```
Tab-delimited sub-codes after the name allow full restore on load, including multi-code entries.
Old save files (no tabs) are still readable — single-code cheats fall back to using the key directly.

**Breaking change**: adding a new `CHEATING_ENGINE` enum value increments `TOTAL_ENGINES`, causing
`loadCheatNames` to reject old save files (version mismatch). Future mitigation: version field in
the save file header, or relaxing the strict `file_m == m` check with forward-compatible loading.

---

## Known Limitations

| Limitation | Reason |
|---|---|
| Master codes (all engines) | Requires code injection and in-game execution — out of scope |
| AR v3 DEADFACE seed changes | Per-code seed state would need to be tracked during decode |
| AR v3 conditional / fill / indirect opcodes | Complex execution model, not yet implemented |
| CodeBreaker type 3 (conditional) | Same as above |
| CodeBreaker type B (hook) | Equivalent to master code |
| GBA GameShark v1/v2 encrypted codes | Encryption algorithm not publicly documented cleanly; raw format accepted |
| Pokemon Emerald money cheat without master code | Emerald XOR-encrypts money with a per-save security key; raw write produces wrong value |
| Save file migration on engine enum change | Old saves rejected; no migration path yet |
