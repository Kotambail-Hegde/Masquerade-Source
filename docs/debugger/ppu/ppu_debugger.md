# GBC PPU Debugger

**Location in menu:** `Emulation → Debug → GBC` (opens the `GB-GBC PPU Debugger` window)
**Source:** `gbc.h` (declarations, `gbcDebugger_t` struct), `gbc.cpp` (single hook in `ppuTick()` / loop-yield in `runEmulationLoopAtFixedRate()`), `gbc_mods.cpp` (all rendering/rebuild logic)

---

## 1. Design goals

This debugger was built under four hard constraints, in priority order:

1. **Zero cost when disabled.** A person who never opens the debugger, or opens it but leaves it disabled, must get byte-for-byte identical emulation behavior and performance to a build with no debugger at all.
2. **Never touch save-state layout.** All debugger state lives in a plain `GBc_t` member (`gbcDebugger_t gbcDebugger;`), declared *outside* the `PACK_BEGIN`/`PACK_END` region that defines `GBc_state_t` (the struct that gets memcpy'd for save states / BESS). Nothing added for debugging purposes changes the size or layout of anything that gets serialized.
3. **Reuse real emulator data wherever possible**, instead of re-deriving it. Sprite visibility, VRAM contents, register values, and even "was this sprite hardware-selected this scanline" all come from fields the emulator core already maintains (`visibleOamIndexPerLY`, `visibleOamIndexPerFrame`, `entireVram`, `pGBc_peripherals->*`), not from a parallel debug-only simulation that could drift from what the real PPU does.
4. **Minimal footprint in the hot path.** Every piece of per-tick instrumentation funnels through a *single* function call, `debugSyncScreenIfNeeded()`, invoked once at the very tail of `GBc_t::ppuTick()` — the one and only line added to the actual PPU tick function. Everything else lives in `gbc_mods.cpp`, a separate translation unit, so the core rendering pipeline in `gbc.cpp` is essentially untouched.

---

## 2. Architecture

### 2.1 Where debugger state lives

```cpp
// gbc.h, immediately after PACK_END (i.e. outside GBc_state_t entirely)
struct gbcDebugger_t
{
    FLAG windowOpen = NO;   // Emulation -> Debug -> GBC

    struct ppu_t
    {
        FLAG enabled = NO;  // master switch for the "instrumented" behaviors below
        GBC_DEBUG_PIXEL_SAMPLE_MODE pixelOutputSampleMode = PER_FRAME;
        // ... panel visibility flags, view-mode toggles, breakpoint state, etc.
    } ppu;

    struct eventViewer_t { ... } eventViewer; // see EVENT_VIEWER.md
} gbcDebugger;
```

Because this whole struct sits outside the packed region, adding new debugger fields is always safe — it can never silently change what a save state file looks like, and there's no need to bump any save-state version number for debugger-only additions.

### 2.2 The single hook point

`ppuTick()` runs once per T-cycle (once per "dot"). At the very end of that function, after all real PPU logic has executed for this tick, there's exactly one added line:

```cpp
#ifndef __RPI_PICO__
    debugSyncScreenIfNeeded();
#endif
```

`debugSyncScreenIfNeeded()` (defined in `gbc_mods.cpp`) is where *all* per-tick debug work happens: breakpoint checking, live pixel/layer capture, Event Viewer register diffing. Its very first lines are cheap short-circuits:

```cpp
void GBc_t::debugSyncScreenIfNeeded()
{
    // Run-to-breakpoint check -- always active, cheap int comparison, independent of "enabled"
    if (gbcDebugger.ppu.runToBreakpointArmed == YES && ...) { ... }

    debugEventViewerCheck(); // has its own independent master switch, see EVENT_VIEWER.md

    if (gbcDebugger.ppu.enabled == NO) RETURN; // <-- when off, this is the ENTIRE cost
    // ... live capture, screen-refresh cadence logic ...
}
```

When the debugger is untouched (default state), this amounts to a small handful of integer comparisons per T-cycle — no allocations, no texture uploads, no branching into any panel logic.

### 2.3 Screen-refresh cadence vs. the emulation loop (important architectural note)

Early in this debugger's development, "Per LY" and "Per Dot" screen-refresh modes were implemented by calling `displayCompleteScreen()` (the function that uploads the emulator's framebuffer to its OpenGL texture) extra times from inside `ppuTick()`. **This turned out to be almost entirely pointless** and was later removed. The reason: ImGui only actually renders/presents once per real host frame, driven from `masquerade.cpp`'s outer loop, *after* the emulation loop for that frame has already fully finished running. Calling `displayCompleteScreen()` many times while still deep inside that loop just re-uploads the same texture repeatedly before anyone ever looks at it — only the very last upload before the real present call matters.

The correct fix (and the one actually implemented) is architecturally different: **make the emulation loop itself yield control back to the host at LY/Dot boundaries**, the same way it already yields at vblank. Once the loop yields, the host's *existing*, already-correct once-per-real-frame pipeline (which already calls `displayCompleteScreen()`, already lets ImGui actually render) does the rest for free — no manual display calls needed anywhere.

This is implemented in `GBc_t::runEmulationLoopAtFixedRate()`:

```cpp
FLAG GBc_t::runEmulationLoopAtFixedRate(uint32_t currentFrame)
{
    ...
    FLAG shouldYieldForSampleMode = NO;
    ...
    // near the tail, after normal vblank handling:
    if (gbcDebugger.ppu.enabled == YES && gbcDebugger.ppu.runToBreakpointArmed == NO && gbcDebugger.ppu.paused == NO)
    {
        if (pixelOutputSampleMode == PER_DOT) shouldYieldForSampleMode = YES;
        else if (pixelOutputSampleMode == PER_LY && pGBc_peripherals->LY != debugLastLYSeenByLoop)
        {
            shouldYieldForSampleMode = YES;
            debugLastLYSeenByLoop = pGBc_peripherals->LY;
        }
    }

    RETURN (wasVblankJustTriggerred || paused == YES || shouldYieldForSampleMode == YES);
}
```

**Important caveats, stated plainly rather than glossed over:**

- **This is genuinely slower, by design.** In `PER_LY` mode, one call to the outer `runEmulationCore()` loop now only advances *one scanline* instead of a full frame — meaning a single emulated GB frame can take on the order of 100+ real host-application draw calls to fully complete. If the game content is static during that stretch (e.g. a title screen), watching LY count 0→143→144→0 will *correctly* show the same unchanging picture over and over; it isn't stuck, it's just slow-motion.
- **LY-boundary detection uses "did LY change from what the loop last saw," not "does the tick counter equal exactly 1."** An earlier version tried the latter and it essentially never fired, because a single call to `runEmulationLoopAtFixedRate()` (via `processSOC()`) can advance the T-cycle counter by more than one tick in a single call — jumping clean over the value `1` without ever equaling it. Comparing "has LY changed since the loop last observed it" catches every transition regardless of how many T-cycles a given call advances by.
- **Per-Dot granularity is bounded by `processSOC()`'s natural step size** (roughly one M-cycle), not a true single-T-cycle guarantee. True per-T-cycle interruption would require restructuring the CPU pipeline into an explicitly resumable state machine at the T-cycle level — a much larger architectural change, out of scope for this debugger.

---

## 3. The `Emulation → Debug` menu structure

```
Emulation
 └─ Debug
     ├─ GBC              (enabled only when a GB/GBC ROM is loaded; toggles gbcDebugger.windowOpen)
     ├─ NES              (disabled, "Coming soon")
     ├─ GBA              (disabled, "Coming soon")
     ├─ Pac-Man          (disabled, "Coming soon")
     ├─ Space Invaders   (disabled, "Coming soon")
     └─ CHIP-8           (disabled, "Coming soon")
```

The GBC entry is wired via:
```cpp
if (current_instance->getEmulationID() == EMULATION_ID::GB_GBC_ID)
{
    GBc_t* gbc = static_cast<GBc_t*>(current_instance);
    if (ImGui::MenuItem("GBC", NULL, gbc->gbcDebugger.windowOpen))
        gbc->gbcDebugger.windowOpen = (gbc->gbcDebugger.windowOpen == NO) ? YES : NO;
}
```
and the actual render call in `masquerade.cpp`'s per-frame ImGui pass:
```cpp
if (current_instance->getEmulationID() == EMULATION_ID::GB_GBC_ID)
    static_cast<GBc_t*>(current_instance)->renderGBCDebuggerUI();
```
`renderGBCDebuggerUI()` itself checks `gbcDebugger.windowOpen` and returns immediately if closed — so this call costs nothing when the window isn't open, and there is no need to gate the call site itself.

---

## 4. Window layout: docking, not tabs

The `GB-GBC PPU Debugger` window uses a **local ImGui DockSpace** (via `ImGui::DockBuilder*`), not a tab bar, for its panels. This was a deliberate design decision, changed mid-development after initial feedback that a tabbed layout (only one panel visible at a time) defeated the purpose of a debugger where you want to correlate Registers, Tiles, BG Map, and OAM simultaneously.

### 4.1 Default layout

Built once via `ImGui::DockBuilder*` calls, only if the dockspace node doesn't already exist (so the layout persists across app restarts via `imgui.ini` rather than being force-rebuilt every launch):

```
+------------+------------------+----------+
| Registers  |                  |  Tiles   |
+------------+ Complete Viewport+----------+
| OAM/Sprites|                  | Palettes |
+------------+------------------+----------+
              | BG Map           |
              +------------------+
              | Window Map       |
              +------------------+
```

A separate top-level tab bar (`PPU | Event Viewer | CPU (Coming Soon) | APU (Coming Soon)`) exists *above* the dockspace, for switching between entirely different debugging *domains* (PPU vs. the register-change Event Viewer vs. future CPU/APU debuggers) — this is intentionally a different UI mechanism from the docked panels, because domains are mutually exclusive (you're either looking at PPU state or CPU state) whereas panels within a domain are meant to be viewed together.

### 4.2 Reopening an accidentally-closed panel

Each panel is a normal closable ImGui window. If you close one by mistake, use **Panels** in the window's own menu bar — every panel has a checkbox there tied to the same `show*` flag its own close button uses.

### 4.3 Fullscreen

**Fullscreen** in the menu bar expands the debugger window to fill the main viewport's work area (`ImGui::GetMainViewport()->WorkPos/WorkSize`), with resize/move disabled while active. Toggling it off restores normal windowed behavior. This is independent of your OS/monitor fullscreen — it only affects this one ImGui window.

---

## 5. Panels, in detail

### 5.1 Registers

Read-only, live view of:
- **LCDC ($FF40)** — every bit individually decoded (LCD/PPU enable, window tile map area, window enable, BG/window tile data area, BG tile map area, OBJ size, OBJ enable, BG/window enable), plus the raw hex byte.
- **STAT ($FF41)** — current mode (0-3, named), LYC==LY flag, all four STAT interrupt-source bits.
- **Position/Timing** — LY, LYC, SCX, SCY, WX, WY, dot-in-scanline (`ppuCounterPerLY`), fetcher state.
- **Palettes (DMG)** — raw BGP/OBP0/OBP1 bytes.

All values are read directly from `pGBc_peripherals->*` and `pGBc_display->*` — there is no separate "debug copy" of register state; what you see here is exactly what the live PPU sees on the same tick.

Boolean fields are shown as colored ON/off text (green/gray) rather than raw `1`/`0`, and every section uses a fixed-width label column (`ImGuiTableColumnFlags_WidthFixed, 150.0f`) so the value column aligns consistently across every collapsible section — an earlier version let each table auto-size independently, which caused the value column to visibly jump left/right depending on which section had the longest label.

### 5.2 Tiles

Renders all 384 tiles of the selected VRAM bank (bank 0/1 selectable on CGB; DMG is always bank 0, since DMG has no VRAM banking) as a 16×24 grid of 8×8 tiles, using **nearest-neighbor texture filtering** (`GL_NEAREST` on both min/max) — so it's pixelated/blocky by design, not smoothed, matching how the actual hardware tile grid looks.

The grid itself is intentionally rendered **palette-agnostic** (raw 2bpp shade, not remapped through any specific BG/OBJ palette) — a tile can be used by multiple different palettes simultaneously (different sprites, or BG vs. window with different CGB palette indices), so showing it through one arbitrarily-chosen palette in the main grid would be misleading. Click a tile to open a **detail panel** on the right, which:
- Shows the tile enlarged, decoded through an *actual, correctly-applied* palette (this is the one place palette-accurate decoding happens for a single tile) — CGB has an 0-7 BG-palette preview slider; DMG shows shades via `BGP`.
- Lists the tile's 16 raw bytes, with the correct source address shown (`$8000 + bank*$2000 + tileIndex*16`).
- Shows the swatches for whichever palette is currently selected for preview, always in sync with the enlarged image above (an earlier version showed a grayscale crop next to unrelated colored swatches — since fixed by giving the detail view its own small dedicated 8×8 texture, rebuilt through the selected palette every frame it's open).

### 5.3 BG Map / Window Map

Two independent panels, **not** a shared "map browser with a layer toggle" — this was also a design change after early feedback that a single toggle made it look like both panels were showing the same underlying data (they aren't; see 5.3.1).

- **BG Map**: shows **both** `$9800` and `$9C00` tilemaps side-by-side, always, with the currently LCDC-active one labeled `(ACTIVE)` in green. This differs from an earlier "browse one map with a radio button" design — showing both simultaneously removes the need to toggle back and forth to compare, and makes it immediately obvious which one the hardware is actually using right now.
- **Window Map**: shows the **live-captured** window layer only (see 5.4) — i.e., exactly the pixels the window fetcher actually produced this frame, not a raw tilemap browse. If the window hasn't drawn anything this frame (not yet triggered, disabled, or WX/WY places it off-screen), this is stated explicitly rather than showing a misleadingly blank map.

#### 5.3.1 Why Window Map doesn't just browse a static tilemap

An earlier version had Window Map browse `$9800`/`$9C00` exactly like BG Map, with its own independent map-select toggle. The problem: because BG and window often legitimately point at the *same* tilemap address (very common — many games reuse one tilemap for both, differentiating purely via WX/WY positioning), the two panels would frequently show pixel-identical content, which looked like a bug (mirrored panels) even though it wasn't. Switching Window Map to a **live per-pixel capture** (see below) makes it show only what the window layer *actually contributed to the screen this frame* — genuinely different from BG Map's content whenever the window only covers part of the screen, which is the common case.

A live-captured panel can legitimately go blank mid-session if the game disables the window mid-frame after already drawing something with it (a very common technique for status-bar effects — draw a banner with the window for the first few scanlines, then disable it for the rest of the frame). The panel explicitly distinguishes "never drew anything" from "drew something earlier, then got disabled," so this doesn't look like a bug when it's expected behavior.

### 5.4 Live per-pixel capture (the mechanism behind Window Map, and Complete Viewport's toggles)

This is the single most load-bearing piece of new instrumentation in the whole debugger, and it's entirely contained inside `debugSyncScreenIfNeeded()`.

**The problem it solves:** distinguishing, per screen pixel, whether that pixel's final color came from the BG layer, the Window layer, or an OBJ (sprite) — without touching the actual pixel-commit code deep inside `ppuTick()` (which has ~7 separate, historically-duplicated write sites across DMG/CGB code paths, and is exactly the kind of fragile, accuracy-critical code you don't want a debugger feature adding risk to).

**How it works:** `pGBc_display->pixelRenderCounterPerScanLine` increments by exactly one each time a real pixel is pushed to the display. By comparing its value on this tick to its value on the previous tick, the capture code can tell "a new pixel was just committed, and it's at position (LY, counter-1)" without needing to know anything about *how* that pixel was produced.

To classify *which layer* produced it, the code reads flags your emulator core already sets at the exact moment of commit: `prevCGBPixelIsBG` / `prevCGBPixelIsOBJ` (set on the CGB code path) and `prevDMGPixelIsBG` / `prevDMGPixelIsOBJ` (set on a **separate** DMG code path — see §7 for why this distinction matters and bit the debugger once already). Both pairs are checked and OR'd together, since only one pair is ever non-zero for a given pixel depending on which code path actually executed:

```cpp
const FLAG isObjPixel = (prevCGBPixelIsOBJ == YES || prevDMGPixelIsOBJ == YES) ? YES : NO;
const FLAG isBgPixel  = (prevCGBPixelIsBG  == YES || prevDMGPixelIsBG  == YES) ? YES : NO;

PIXEL_SOURCE_TAG source = NONE;
if (isObjPixel) source = OBJ;
else if (isBgPixel) source = shouldFetchAndRenderWindowInsteadOfBG ? WINDOW : BG;
```

The result — a color plus a source tag, per screen pixel, plus (for Complete Viewport's click-to-inspect) a small metadata bundle (LY, `pixelRenderCounterPerScanLine`, `ppuCounterPerLY`, `ppuCounterPerMode`, `ppuCounterPerFrame`, `pixelFetcherState`) — is stored in flat `160×144` arrays, cleared once per frame at the LY-wrap boundary.

**Known limitation, stated explicitly:** this only stores the *final, already-composited* pixel. If a sprite happens to sit on top of a BG/window pixel, that pixel is tagged `OBJ` and its BG/window color underneath is simply gone — not retained anywhere. So disabling Sprites in Complete Viewport shows black where a sprite was, not "what the BG would have looked like underneath it." Getting true layer-stacking (seeing what's *underneath* a sprite) would require capturing BG/window results *before* OBJ compositing runs, which does mean touching the fetcher code directly — a larger, riskier change deliberately not made here.

### 5.5 Complete Viewport

The one panel that shows the **full 256×256 active BG tilemap** (not just the current 160×144 visible slice), with Window and Sprite content correctly translated from screen-space into their true scrolled map-space position via `(x+SCX)&255, (y+SCY)&255`, and the visible-viewport rectangle drawn on top (wrap-aware — correctly split into up to 4 rectangle segments when the viewport wraps past either the right or bottom edge of the 256×256 torus).

- **BG / Window / Sprites checkboxes**: independently show/hide each layer's contribution. Unchecking a layer paints its tagged pixels black (see the layer-stacking limitation above — this is "hide," not "reveal what's underneath").
- **Show viewport rect**: the SCX/SCY 160×144 box, drawn with correct 256×256 torus wraparound.
- **Show grid**: 8px tile-boundary grid, shared color setting (white/black, see §5.7) with every other grid overlay in this debugger.
- **Click-to-inspect**: click any pixel *inside* the currently-visible viewport rectangle to see its exact `LY`, `pixelRenderCounterPerScanLine`, `ppuCounterPerLY`, `ppuCounterPerMode`, `ppuCounterPerFrame`, and `pixelFetcherState`. Clicking outside the rectangle, or on a map position that genuinely wasn't rendered this frame, says so explicitly rather than showing stale/misleading numbers. This is a **click**, not a hover — deliberately, so the selection persists while you read the numbers rather than disappearing the instant you move the mouse.

This panel is intentionally the *only* place per-pixel timing metadata is exposed, rather than being a separate "per-dot debug mode" — the reasoning being that the metadata is inherently about a pixel that's already sitting on a canvas this panel already draws, so a separate tab showing the same picture with a different interaction bolted on would be redundant.

### 5.6 OAM / Sprites

Two view modes (**List** / **Gallery** radio), both driven by the same underlying data:

- **List**: a table of all 40 OAM entries (X, Y, Tile, Palette, Flags), with row background color indicating visibility status — cyan if the hardware's real OAM search selected this sprite for the *current* scanline (`visibleOamIndexPerLY`), green if selected at any point *this frame* (`visibleOamIndexPerFrame`), orange if merely positioned on-screen but not currently hardware-selected (e.g. it's the 11th+ sprite on an already-full scanline — a real hardware limitation, not a bug), gray/none otherwise.
- **Gallery**: the same 40 entries as small clickable thumbnails, using the exact same color-coding as border colors, arranged in a grid sized to the available panel width.

Selecting a sprite (either view) shows, on the right:
- The sprite's own tile pixels, enlarged.
- A full detail table: X, Y, Tile, raw attribute byte, X/Y flip, priority, and (CGB) palette number + VRAM bank, or (DMG) which OBP register.
- The actual palette swatches used by *this specific sprite* (not a generic palette list).
- A live mini-screen (a direct copy of the real, already-rendered `imGuiBuffer`, not a re-derived approximation) with **every** currently-visible sprite boxed in orange/green/cyan (frame/LY/on-screen tiers, same color scheme as the list), and the selected sprite boxed in magenta on top, drawn last so it's never obscured.

`visibleOamIndexPerLY` / `visibleOamIndexPerFrame` are pre-existing `GBc_t` members (not something this debugger added) — they're maintained by the real OAM search logic itself, which is why this view is hardware-accurate rather than an approximation based on simple on-screen bounding-box checks (an earlier version used exactly such a bounding-box check as a stand-in, before these fields were discovered to already exist and do the job precisely).

### 5.7 Palettes

- **DMG**: three rows (BG0 from `BGP`, OBJ0 from `OBP0`, OBJ1 from `OBP1`), each decoded via the same `getColorFromColorIDForGB()` helper the real renderer uses — not a hand-rolled reimplementation of BGP shade-extraction, specifically to avoid any risk of the debug view disagreeing with what's actually on screen.
- **CGB**: all 8 BG palettes and all 8 OBJ palettes, shown side-by-side in two columns (not stacked vertically) to keep the panel compact, each decoded via `getColorFromColorIDForGBC()`.

### 5.8 Shared conventions across panels

- **Grid color** (white/black) and **screen-refresh mode** (Per Frame/LY/Dot) are set once, in the PPU tab's toolbar, and apply uniformly to every panel with a grid overlay (Tiles, BG Map, Window Map, Complete Viewport) via a single small helper:
  ```cpp
  static ImU32 gbcDebugGridColor(FLAG white) {
      return white ? IM_COL32(255,255,255,70) : IM_COL32(0,0,0,90);
  }
  ```
- **Every rendered image uses `GL_NEAREST` filtering** — pixelation is a deliberate, permanent property of every debug texture in this system, not a toggle, since smoothing would misrepresent what the actual hardware tile/pixel grid looks like.
- **Numeric input avoids the keyboard entirely.** The Run-to-breakpoint LY/Dot fields use `ImGui::InputInt` with its built-in mouse-clickable `-`/`+` step buttons (typed numpad entry still works too, but isn't required) — this was a deliberate fix for keyboard focus fighting between the debugger window and the emulation window (pressing Enter to confirm a typed value was also being interpreted by the game's own input handling simultaneously). If you want the debugger to be strictly mouse-only, don't type into these fields — just click the step buttons.

---

## 6. Run-to-breakpoint, Pause, and Step

Distinct from screen-refresh cadence (§2.3) — this is the mechanism for **actually freezing emulation**, not just changing how often the picture updates.

- **Run to breakpoint**: arms a target (LY, Dot) pair and runs the emulator at **full, undecorated speed** (no forced extra texture uploads, regardless of the Screen Refresh setting) until that exact (LY, Dot) is reached, at which point emulation freezes completely — every subsequent call to the outer emulation loop returns immediately without executing anything further, until you explicitly Step or Resume.
- **Step**: executes exactly one more `processSOC()` call's worth of ticks (≈1 M-cycle, not a guaranteed single T-cycle — see the granularity caveat in §2.3), then re-freezes.
- **Resume**: clears the frozen/armed state and lets emulation run freely again, honoring whatever Screen Refresh mode is currently selected.

This combination is the intended way to inspect "what does the hardware actually do right at this specific dot," and is a completely separate mechanism from Screen Refresh mode — Run-to-breakpoint ignores the Screen Refresh setting entirely and ignores it deliberately, so that reaching a distant breakpoint doesn't cost you the (potentially enormous) slowdown that Per-LY/Per-Dot screen refresh would otherwise impose.

---

## 7. DMG vs. CGB: the dual-storage-path problem (and why it matters for anyone extending this debugger)

This bit its own author (this debugger's author) once already, and is worth documenting clearly so it doesn't happen again when this debugger is extended (e.g. for NES/GBA, or for deeper GBC features).

The emulator core has **two entirely separate storage locations for the same conceptual data**, selected at runtime by `ROM_TYPE`:

- **CGB path**: VRAM lives in `GBc_state.entireVram.vramMemoryBanks.mVRAMBanks[bank][offset]`, a dedicated top-level member of `GBc_state_t`.
- **DMG path**: VRAM lives in `GBc_state.GBcMemory.GBcRawMemory[0x8000 + offset]` — a flat byte array that's a **union member** aliasing the same bytes as the structured `GBcMemoryMap` view, entirely separate from `entireVram`.

For a long stretch of this debugger's development, every VRAM-reading function (Tiles, BG Map, Window Map, OAM sprite-pixel decoding, Complete Viewport) read *only* from `entireVram` — which is simply never written to for a DMG ROM. This didn't cause a crash; it silently returned all-zero bytes, which decoded as flat single-color fills everywhere — looking like "BG and Window aren't working" for DMG games specifically, while CGB games (which do use `entireVram`) worked correctly the whole time. Sprite *gallery thumbnails* were affected the same way (blank tile pixels), while OAM *metadata* (X/Y/tile index/attributes) was unaffected, since OAM lives in the shared `GBcMemory` union regardless of mode — which is why, when this bug was live, sprite positions/attributes displayed correctly but sprite thumbnails were blank.

**The fix**, and the pattern any future VRAM-reading debug code should use, is a single helper:

```cpp
BYTE GBc_t::debugReadVRAM(uint8_t bank, uint16_t offsetWithinBank)
{
    if (ROM_TYPE == ROM::GAME_BOY)
    {
        // DMG has no VRAM banking; real storage is the flat memory-map union at $8000+offset.
        RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0x8000 + offsetWithinBank];
    }
    RETURN pGBc_instance->GBc_state.entireVram.vramMemoryBanks.mVRAMBanks[bank][offsetWithinBank];
}
```

A structurally identical issue exists for the per-pixel BG/OBJ classification flags (§5.4): `prevCGBPixelIsBG/OBJ` vs. `prevDMGPixelIsBG/OBJ` — two separate pairs, only one of which is live depending on `ROM_TYPE`, both must be checked (OR'd) rather than assuming one is authoritative.

**Rule of thumb for anyone extending this debugger:** before trusting that a `GBc_state_t` field is populated for *both* DMG and CGB ROMs, check whether the real emulator core has an `if (ROM_TYPE == ROM::GAME_BOY_COLOR) { ... } else { ... }` branch touching that data anywhere in `gbc.cpp`. If it does, the debugger needs to know about both branches, not just the one that happens to get exercised by whichever ROM was used while writing that feature.

---

## 8. Known limitations (stated explicitly, not hidden)

- **No true single-T-cycle stepping.** Step/Run-to-breakpoint granularity is bounded by `processSOC()`'s natural step size. True T-cycle-exact interruption needs a resumable CPU pipeline, out of scope here.
- **Complete Viewport with a layer disabled shows black, not "what's underneath."** Per-pixel capture only stores the final composited result, tagged by source — it doesn't retain a full per-layer stack.
- **Event log PC is best-effort**, not an exact write-time breakpoint (see `EVENT_VIEWER.md` §4).
- **Tile viewer's main grid is deliberately palette-agnostic.** Only the click-to-detail view is palette-accurate.

---

## 9. Quick usage recipes

- **"What does the screen actually look like right now, decomposed by layer?"** → Complete Viewport, toggle BG/Window/Sprites individually.
- **"Is this specific sprite actually being drawn by the hardware, or just sitting in OAM unused?"** → OAM/Sprites panel; cyan = this exact scanline, green = somewhere this frame, orange = positioned but not currently selected (likely past the 10-sprite hardware cap).
- **"What was on screen at exactly LY=90, dot=200?"** → Run to breakpoint (LY 90, Dot 200), then use Complete Viewport's click-to-inspect once frozen.
- **"Watch the picture build scanline by scanline / dot by dot"** → Screen Refresh: Per LY or Per Dot (expect this to be much slower in wall-clock time — see §2.3).
- **"Why does my BG map browser and my composited scene disagree?"** → That's very likely correct layer isolation (see the worked example in this repo's troubleshooting notes) — verify by toggling Window/Sprites back on one at a time in Complete Viewport before assuming a bug.