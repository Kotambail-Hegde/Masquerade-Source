# Game Boy Printer Emulation — Software Design & Architecture

**Component:** `GBcPrinterEngine_t` (printer-side protocol/state engine) + `GBc_t::serialTick()` (link-level bit exchange) + ImGui roll viewer  
**Project:** Masquerade Emulator

---

## 1. Overview

The Game Boy Printer subsystem emulates a real GB Printer accessory connected over the Game Boy's serial link port. It is split into three layers, each with a distinct responsibility:

| Layer | Responsibility |
|---|---|
| **Link layer** (`GBc_t::serialTick`, `detectSerialDevice`) | Bit-level serial clocking between the Game Boy and whatever's plugged into the link port (link cable or printer). Detects that a printer is present. |
| **Protocol/state layer** (`GBcPrinterEngine_t`) | Byte framing, packet parsing, checksum validation, command dispatch, printer status simulation (`INIT`/`DATA`/`PRINT`/`STATUS`, paper jam, print timing). |
| **Presentation layer** (`drawImGuiWindows`, roll compositing, PNG export) | Turns decoded tile data into viewable/saveable images, using an ImGui window per continuous roll of "paper." |

This separation mirrors the real hardware boundary: a real GB Printer's microcontroller doesn't know or care that its output ends up on thermal paper versus a debug window — that's purely our presentation layer's problem.

---

## 2. Link Layer: Serial Bit Exchange

### 2.1 Why this layer exists separately

The Game Boy's serial port (`SB`/`SC` registers) is a synchronous, full-duplex shift register: each clock edge simultaneously shifts one bit out (MSB of `SB`) and one bit in (into `SB`'s LSB). `serialTick()` is called once per serial clock and is responsible for this bit exchange regardless of what's on the other end (link cable, another emulator instance, or the printer engine).

### 2.2 Device detection

`detectSerialDevice()` watches the Game Boy's outgoing bits (`bitToBeSent`) and, before any device has been identified, shifts them into a 16-bit detection register. If the first 16 bits sent ever equal `$8833` (the printer's magic handshake), it commits to `GB_SERIAL_DEVICE::GB_PRINTER` and calls `gbPrinterEngine.startPacket()` — which resets the printer's internal state and puts it directly into the `COMMAND` parsing state, since the magic bytes were already consumed by detection and won't be re-delivered to the printer engine.

Once a device is detected, `detectSerialDevice()` is a no-op for the rest of the session — subsequent `$88 $33` sequences (start of every later packet) are routed normally to `gbPrinterEngine.receiveBitFromGB()` like any other bytes.

### 2.3 Bit-order fix (critical timing bug)

**Original bug:** in master mode, `serialTick()` called `gbPrinterEngine.receiveBitFromGB()` *before* `gbPrinterEngine.sendBitToGB()` on every tick. On the tick that completed reception of a packet's checksum-high byte, `processReceivedByte()` synchronously prepared a brand-new response byte (`txByte = 0x81`, `txBitCount = 0`). Because `sendBitToGB()` ran immediately afterward *in the same tick*, the first bit of that fresh response byte got shifted into the **current**, still-in-progress Game Boy byte instead of starting cleanly on the **next** one — permanently bit-rotating every response byte the Game Boy read back for that transaction (e.g. reading `0x02` instead of a clean `0x81`).

**Fix:** for the printer path specifically, `sendBitToGB()` is now called *before* `receiveBitFromGB()` each tick. This means the bit output on a given tick always reflects state as of the *start* of that tick (matching real synchronous shift-register semantics: the outgoing bit was already latched before the clock edge), while any state change triggered by the incoming bit only takes effect starting the *next* tick. The GB_LINK_CABLE path is unaffected (its send-then-receive ordering was already correct, representing a real network round trip rather than a same-clock exchange).

```
Master-mode ordering (per tick), printer path:
  1. sendBitToGB(&bitReceived)     // uses PRE-existing txByte/txBitCount
  2. receiveBitFromGB(bitToBeSent) // may prepare a NEW txByte, effective next tick
  3. SB = (SB << 1) | bitReceived
```

This fix was verified by reconstructing the Game Boy's own `SB` register bit-by-bit against captured logs: response bytes read back as clean `0x81` immediately after the fix, versus scrambled values before it.

---

## 3. Protocol/State Layer: `GBcPrinterEngine_t`

### 3.1 Byte framing state machine

`processReceivedByte()` implements the documented GB Printer packet format as a state machine (`GB_PRINTER_STATE`):

```
NONE → MAGIC_33 → COMMAND → COMPRESSION → LENGTH_LOW → LENGTH_HIGH
     → [DATA]* → CHECKSUM_LOW → CHECKSUM_HIGH → (back to NONE)
```

- `MAGIC_88`/`MAGIC_33`: only reachable on the very first packet in a session before `detectSerialDevice()` has claimed the link; all later packets enter directly at `COMMAND` via `startPacket()`.
- `DATA` is skipped entirely (`packetLength == 0` jumps straight to `CHECKSUM_LOW`) for commands that carry no payload.
- `CHECKSUM_HIGH` validates the running checksum against the two received checksum bytes, sets/clears `STATUS_CHECKSUM_ERROR`, and — only on a valid checksum — calls `dispatchCommand()` before preparing the response.

### 3.2 `GB_PRINTER_DATA` routing

The same `DATA` state is reused by two different commands with different payload shapes, and must route bytes accordingly (this was a real bug found during implementation — routing by index alone would silently corrupt image data):

| Command | Payload | Routed to |
|---|---|---|
| `DATA` (`0x04`) | Tile pixel data (2bpp, up to 640 bytes/packet) | `imageBuffer` (via RLE decompressor if `compression == 1`) |
| `PRINT` (`0x02`) | Exactly 4 fixed bytes: sheets, margins, palette, exposure | `printArgs` struct, by `packetIndex` (0–3) |
| anything else with `packetLength > 0` | Not expected per spec | Bytes are still counted toward the checksum, but not stored; `STATUS_PACKET_ERROR` is set (malformed-packet visibility, not silent drop) |

### 3.3 Command dispatch (`dispatchCommand`)

Called once, after checksum validation, from `CHECKSUM_HIGH`:

- **`INIT`** — clears `imageBuffer`, resets the print timer, and resets `status` to zero **except** it re-asserts `STATUS_PAPER_JAM` if the printer is currently jammed. Real hardware can't be talked out of an empty paper roll by the Game Boy re-initializing the link — the jam is a physical condition, not a protocol condition, so it must survive `INIT`.
- **`DATA`** — sets `STATUS_UNPROCESSED` once an *empty* `DATA` packet arrives with a non-empty `imageBuffer` (the documented "end of image" marker — an empty `DATA` command terminates the image and signals "ready to print").
- **`PRINT`** — refuses immediately (`STATUS_PAPER_JAM`, `imageBuffer` left untouched so a retry has data to work with) if the roll is jammed. Otherwise, if `STATUS_UNPROCESSED` is set: clears it, sets `STATUS_PRINTING`, commits the image to the roll **synchronously** (see §4), starts the print timer, and — `numSheets == 0` means a line-feed-only operation (no image, just paper advance); `numSheets >= 1` means that many physical copies, each one independently appended to the roll (and independently capable of triggering a jam mid-run). If `STATUS_UNPROCESSED` was not set, sets `STATUS_OTHER_ERROR` instead.
- **`STATUS`** — no state change; the response already reflects current `status`.

**Design decision — commit-on-dispatch, not commit-on-timer-expiry:** the image is rendered and appended to the roll *the moment* `PRINT` is dispatched, not when the print timer later reaches zero. `tick()` only advances a status/timing simulation — it must never be the thing deciding whether image data survives. An earlier version of this code cleared `imageBuffer` inside `tick()`'s completion branch, which meant any print job that hadn't finished "ticking down" by the time something else cleared state would silently lose the image. This is a correctness-critical ordering: **render before you start counting down**, not count-down-then-render.

### 3.4 Print timing simulation (`tick()`)

Printing on real hardware is a physical, real-time process (thermal head + paper feed motor) with no official documented duration. `tick()` is called once per real frame (gated on vblank, **not** per-instruction/per-M-cycle — calling it too often would finish print jobs almost instantly and would also risk duplicate `ImGui::Begin()` calls for the same window within one ImGui frame) and simply counts down `printTicksRemaining`. On reaching zero: `STATUS_PRINTING` clears, `STATUS_IMAGE_FULL` sets (mirrors real hardware: "done printing, last image still sitting there until acknowledged"). `PRINT_DURATION_TICKS` is a tunable constant (~5–10s at 60fps as a rough real-hardware approximation) since no authoritative timing spec exists; multi-sheet jobs scale duration by `numSheets`.

### 3.5 Paper jam (`STATUS_PAPER_JAM`)

Grounded in Nintendo's published spec: official paper rolls are rated for **up to 180 prints**. `MAX_PRINTS_PER_ROLL = 180` enforces this — once a roll's `printCountInRoll` hits the cap, `isPaperJammed` is set, `STATUS_PAPER_JAM` is reported to the Game Boy, and the active roll stops accepting new prints (`activeRollId = -1`). The jam is sticky across `INIT` (see §3.3) and only clears when the person closes the jammed roll's window — the emulated equivalent of physically loading fresh paper.

---

## 4. Image Decoding & RLE Decompression

`decodeTilesToRgba()` converts raw GB 2bpp tile data (16 bytes/tile, 20 tiles/row per the printer's 20×18-tile buffer) into an RGBA byte buffer.

### 4.1 On-the-Fly RLE Decompression

When `compression == 1`, payload bytes received during the `DATA` command pass through an on-the-fly RLE decompressor before hitting `imageBuffer`. The state machine tracks run parameters across single-byte transfers via two member variables:

- **`runLength`**: Counter tracking remaining output bytes to generate for the active run.
- **`isCompressedRun`**: Flag indicating whether the run repeats a single payload byte (`true`) or copies incoming bytes verbatim (`false`).

```
                    ┌──────────────────────────┐
                    │      runLength == 0      │
                    │  (Control Header Byte)   │
                    └────────────┬─────────────┘
                                 │
                   Bit 7 of incoming data byte
                                 │
           ┌─────────────────────┴─────────────────────┐
           ▼ (Bit 7 == 1)                              ▼ (Bit 7 == 0)
  [Compressed Run]                            [Uncompressed Run]
  isCompressedRun = true                      isCompressedRun = false
  runLength = (data & 0x7F) + 2               runLength = (data & 0x7F) + 1
           │                                           │
           ▼                                           ▼
  Receive Payload Byte                        Receive Payload Byte
  imageBuffer.insert(..., runLength, byte)    imageBuffer.push_back(byte)
  runLength = 0                               runLength--
```

1. **Header Byte Evaluation (`runLength == 0`)**:
   - **Bit 7 = 1 (Compressed Run):** Bits 0–6 represent run length minus 2. Length is calculated as `(data & 0x7F) + 2` (min length 2, max 129).
   - **Bit 7 = 0 (Uncompressed Run):** Bits 0–6 represent run length minus 1. Length is calculated as `(data & 0x7F) + 1` (min length 1, max 128).
2. **Data Payload Processing (`runLength > 0`)**:
   - **Compressed (`isCompressedRun == true`):** The next single incoming byte is repeated `runLength` times in a single operation via `imageBuffer.insert(imageBuffer.end(), runLength, dataReceived)`, completely consuming the run in O(1) allocation overhead and resetting `runLength` to `0`.
   - **Uncompressed (`isCompressedRun == false`):** Subsequent raw bytes are appended directly to `imageBuffer` via `push_back()`, decrementing `runLength` on each byte until zero.

### 4.2 Pixel & Shade Decoding

1. For each tile, each 8×8 pixel is reconstructed from its two bitplanes (`GETBIT(7-x, loByte)` / `GETBIT(7-x, hiByte)` → 2-bit color index).
2. The raw color index is mapped through the print job's **palette byte** (`printArgs.palette`, same 2-bits-per-shade encoding as the `BGP`/`OBP` registers) via `GET_GB_COLOR_NUMBER(palette, colorIndex)`.
3. The resulting shade is adjusted by **exposure** (`printArgs.exposure`, 7-bit burn-time value; official manual: −25% darkness at `$00`, +25% at `$7F`, `$40` = nominal). Exposure scales *ink* (distance from white), not the raw gray value directly, so white pixels stay white regardless of exposure setting — matching the real-world semantics of a burn-time knob rather than a brightness knob. No documented formula exists for intermediate values; a linear interpolation between the two documented endpoints is used as a reasonable approximation.

---

## 5. Presentation Layer: Rolls, Compositing, and the Viewer

### 5.1 Why "rolls," not "one PNG per print"

Real GB Printer paper is a **continuous strip** — a `PRINT` command feeds paper through the print head but does not cut it. The only thing that ever "cuts" the paper is a person physically tearing it off. Consecutive `PRINT` commands (e.g. Pokémon Yellow's Pokédex, which prints a stats page and a description page as two separate `PRINT` commands that are conceptually one continuous printout) should therefore land on the *same* image, not two separate files.

This is modeled with:
- **`PrintedImageWindow`** — one continuous roll's full state: accumulated pixels, GL texture, title, open/closed flag, save state, print count.
- **`activeRollId`** — which roll (if any) is currently "loaded in the printer" and receiving new prints. `-1` means the next `PRINT` starts a fresh roll. Tracked by a stable `id`, not a vector index, since closing/pruning older windows shifts indices.
- **Closing a window = tearing off the paper.** This both (a) removes that roll from view and frees its texture, and (b) if it was the active roll, clears `activeRollId` so the next print starts fresh — and if it was the roll that caused a jam, clears the jam.

### 5.2 Block-based compositing (not baked-in pixels)

Each roll stores its content as an ordered list of `RollBlock`s — either an image block (pre-decoded RGBA + height) or a *gap marker* with no baked-in size. `recompositeRoll()` rebuilds a roll's full pixel buffer and GL texture from its blocks on demand, resolving each gap marker's height from the *current* `cosmeticGapPx` value at composite time.

This design exists specifically so that the visual gap between prints is **uniform across an entire roll, always** — if it were baked into pixels at append-time (an earlier version did this), changing the gap setting mid-session would leave old gaps at their old size while new ones used the new size, producing a visibly inconsistent roll. Recompositing on every change (both on append and on slider adjustment) guarantees one global value applies identically everywhere, live.

**Gap semantics, not spacing accuracy:** GB Printer's margin byte (high/low nibble = feed-before/feed-after) has no documented, confirmed pixel-per-unit conversion, and independent research against real hardware captures (GBE+ author's writeup; a decoder project cross-validated against ~110 real games) found that ROMs use these values inconsistently enough that some emulators ignore the byte's magnitude entirely and instead treat *presence* of a margin as a boundary signal between logical images. This implementation follows that approach: `hasMarginBefore`/`hasMarginAfter` (nibble != 0) each insert a gap block; the gap's pixel size is a **cosmetic display preference** (`cosmeticGapPx`, user-tunable, default 8px), not a hardware-accurate value.

### 5.3 ImGui viewer

`drawImGuiWindows()` renders one undocked (`ImGuiDockNodeFlags_NoDocking`), fixed-aspect (`ImGuiWindowFlags_AlwaysAutoResize`, no manual resize grip, no scrollbar) window per open roll:

- **Image** — drawn via `ImGui::Image()` from the roll's GL texture at `width * displayScale` / `height * displayScale`.
- **Zoom controls** — `-`/`+` buttons adjust `displayScale` uniformly (both axes together, never independently), avoiding any possibility of a non-uniform stretch.
- **Save PNG** — writes the roll's current composited RGBA buffer to `<cwd>/gb_prints/gb_print_NNN.png` via `stb_image_write`, and displays the resolved absolute path once saved.
- **Menu bar → Settings** — houses the cosmetic gap slider (moved out of the window body to avoid permanently consuming vertical space / triggering the scrollbar-vs-autoresize interaction that made early versions look cramped).
- **Jam notice** — if this roll caused the active jam, a red status line explains that closing the window loads fresh paper.

### 5.4 PNG export

`savePrintedImageAsPng()` uses `stb_image_write` (single-header, public domain) to encode the roll's current RGBA buffer. Output directory is created on demand (`std::filesystem::create_directories`); the resolved absolute path is stored on the window and surfaced in the UI so there's no ambiguity about where the file landed.

---

## 6. Data Flow Summary

```
Game Boy CPU writes SC=0x81
        │
        ▼
GBc_t::serialTick()   (per serial clock, master mode)
  - sendBitToGB()      → printer's current outgoing bit (state as of tick start)
  - receiveBitFromGB() → feeds GB's outgoing bit into printer; may prep NEXT tick's txByte
  - SB shift register updates both directions
        │   (every 8 bits)
        ▼
GBcPrinterEngine_t::processReceivedByte()   [byte framing state machine]
  NONE → COMMAND → COMPRESSION → LENGTH → [DATA →imageBuffer/printArgs] → CHECKSUM
        │ (checksum valid)
        ▼
GBcPrinterEngine_t::dispatchCommand()
  INIT: reset status/buffer (jam persists)
  DATA: mark STATUS_UNPROCESSED on empty terminating packet
  PRINT: appendPrintToRoll() × numSheets ──────┐
  STATUS: no-op                                 │
        │                                       ▼
        │                          decodeTilesToRgba() → RLE Decompress → palette → exposure
        │                                       │
        │                                       ▼
        │                          RollBlock{gap?, image} pushed to roll
        │                                       │
        │                                       ▼
        │                          recompositeRoll() → GL texture (uniform gaps)
        ▼
tick() [per vblank]
  counts down printTicksRemaining → STATUS_PRINTING → STATUS_IMAGE_FULL
        │
        ▼
drawImGuiWindows() [per frame]
  one auto-sized, undocked window per roll: image, zoom, Save PNG, gap setting, jam notice
  window closed → roll torn off: freed, active-roll/jam state cleared as applicable
```

---

## 7. Known Limitations / Explicit TODOs

- **Exposure curve** is a linear approximation between the two documented endpoints (`$00`/`$7F`); no confirmed intermediate-value formula exists.
- **Margin-to-pixel mapping** is deliberately *not* modeled as a hardware-accurate conversion — see §5.2. If a future, confirmed source specifies exact real-hardware feed distances, this would need to move from "cosmetic gap" back to "true proportional feed simulation."
- **`numSheets == 0` (line-feed-only) feed distance** uses the same cosmetic-gap logic as a margin boundary; the exact real-hardware feed amount for this specific case is unconfirmed.
- **Texture cleanup on app shutdown**: open roll windows' GL textures are freed when the user closes them, but not swept on process exit if left open.
