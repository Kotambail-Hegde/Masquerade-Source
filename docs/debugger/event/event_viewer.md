# GBC Event Viewer

**Location:** A top-level tab inside the `GB-GBC PPU Debugger` window, alongside `PPU` (and, later, `CPU`/`APU`) — **not** a PPU-specific panel. It's deliberately built as a sibling debugging *domain*, so extending it to CPU/APU/Memory registers later is additive, not a redesign.
**Source:** `gbc.h` (`eventViewer_t` struct, `PPUEvent_t`, `GBC_DEBUG_TRACKED_REGISTER` enum), `gbc_mods.cpp` (`debugEventViewerCheck()`, `renderGBCDebuggerEventViewerTab()`)

---

## 1. What problem this solves

While the PPU Debugger's Registers panel shows you the *current* value of a register, it can't answer "when, exactly, did this register change, and to what, and how many times does it change per frame?" — which matters enormously for Game Boy raster effects (mid-frame palette swaps, split-screen scrolling via per-scanline SCX/SCY writes, status-bar tricks via mid-frame window toggling). The Event Viewer exists to answer exactly that, by recording every write to a small set of tracked registers, precisely timestamped by scanline and dot.

---

## 2. Design approach: diff-based, not read/write hooking

The single biggest design decision here: **this is a cheap value-diff check performed once per tick, not instrumented memory-access hooking.** The alternative — intercepting every `writeRawMemory()` call for the tracked register addresses — would be more precise (it would catch a write of the *same* value as a distinct event, and could trivially support read-tracking) but requires touching the actual memory-write path, which this project deliberately avoids for the same reason the PPU debugger avoids touching the pixel-commit code directly: it's the most accuracy-critical, fragile code in the whole emulator, and a debugging feature should not be adding risk there.

Instead, `debugEventViewerCheck()` — called once per tick, from the same single hook point as everything else in this debugger (`ppuTick()`'s tail, via `debugSyncScreenIfNeeded()`) — takes a fresh snapshot of every tracked register's current value and diffs it against the value observed on the *previous* tick:

```cpp
void GBc_t::debugEventViewerCheck()
{
    if (gbcDebugger.eventViewer.enabled == NO) RETURN;   // <-- entire cost when disabled

    uint8_t currentValues[COUNT] = { LCDC, STAT, SCX, SCY, LY, LYC, DMA, BGP, OBP0, OBP1, WX, WY };

    // ... frame-wrap housekeeping ...

    for (int r = 0; r < regCount; r++)
    {
        if (currentValues[r] == lastValues[r]) continue;   // no change, nothing to record

        // record event: frame, scanline, dot, register, old value, new value, PC
        // ...
        lastValues[r] = currentValues[r];
    }
}
```

**Direct consequence of this design, stated plainly:** a write of a value *identical* to what's already there produces no event — because there's nothing to diff. If a game writes `$00` to `SCX` when `SCX` is already `$00`, that write is invisible to this tool. This is an accepted tradeoff for keeping the tracking mechanism this cheap and this uninvasive; it is not a bug, and it will never be "fixed" without switching to genuine write-hooking (a much bigger change).

### 2.1 Master switch (own, separate from the PPU debugger's)

```cpp
gbcDebugger.eventViewer.enabled     // independent of gbcDebugger.ppu.enabled
```

This is deliberately its own toggle, off by default, because it is — by a meaningful margin — **the single most expensive opt-in feature in this whole debugger**: it diffs 12 byte values on every single tick while active (roughly 70,000+ comparisons per frame), which is trivial in absolute terms on modern hardware but is genuinely the priciest checkbox here. Keeping it as its own switch means turning on the PPU debugger's other features (Registers panel, Complete Viewport, etc.) never implicitly pays this cost.

---

## 3. Tracked registers

Currently (extensible later — see §7):

| Register | Notes |
|---|---|
| `LCDC` ($FF40) | |
| `STAT` ($FF41) | Includes mode bits — this is *also* separately captured every tick into a dedicated mode-timeline, see §4, independent of whether it counts as a "changed" event |
| `SCX`, `SCY` | Scroll registers — the most common target of mid-frame raster effects |
| `LY` | Included deliberately, unlike an earlier version that excluded it as "too noisy" — filterable via its own checkbox instead of hard-excluded |
| `LYC` | |
| `DMA` ($FF46) | The OAM DMA source-page register; a write here signals an OAM DMA transfer request |
| `BGP`, `OBP0`, `OBP1` | DMG palette registers — mid-frame writes here are the classic "palette swap partway down the screen" trick |
| `WX`, `WY` | Window position registers |

`STAT`'s **mode** (bits 0-1) is additionally captured every single tick into a separate `modeTimeline[154][456]` array *regardless* of whether the diff loop considers `STAT` "changed" — this is what powers the mode-band background in the scatter view (§4), and needs continuous per-dot coverage rather than only-on-change events, since a mode band is a *span*, not a point event.

---

## 4. The scatter view: (dot, scanline) plot with a live pixel-accurate backdrop

This is the most involved piece of UI in the Event Viewer, and went through several iterations before landing on something that's actually informative rather than misleading. Documenting the failed intermediate versions here deliberately, since the reasons they failed are exactly the kind of thing worth knowing before touching this code again.

### 4.1 What it shows

- **X axis: dot (0-455)** within a scanline.
- **Y axis: scanline (0-153)**, including the 10 VBlank rows (144-153), shown as a flat purple band.
- **Background, per visible row (0-143)**: colored bands showing which STAT mode was active at each dot — blue for Mode 2 (OAM Search), maroon for Mode 0 (HBlank), and, for Mode 3 (Drawing), **the actual rendered pixel colors** for that row, reusing Complete Viewport's per-pixel capture (`debugViewportPixels`/`debugViewportPixelInfo`) rather than a flat color — so you can visually correlate "this register changed right as this specific part of the picture was being drawn."
- **Event markers**: a small colored square per recorded register-change event, color-coded per register, drawn on top of the mode-band backdrop.
- **Hover tooltip**: hovering any point shows the exact scanline, dot, decoded mode name, and any event(s) that landed on that *exact* cell.

### 4.2 Why "Prev / Curr" frame toggle was removed

An earlier version double-buffered everything (a `curr` ring/timeline that filled live, copied into a frozen `prev` at each frame boundary) specifically to let you look at a fully-completed, stable frame while the next one was still accumulating. This was removed after direct feedback that it added real complexity (double the storage, double the branching in every render path) for a distinction that wasn't earning its keep in practice — the tool now always shows the current/most-recent frame's data only.

### 4.3 Why the pixel backdrop's dot-position bug happened (and the general lesson from it)

The first working version of the pixel-accurate Mode-3 backdrop read `pixelRenderCounterPerScanLine` to decide where in the 0-455 dot range each captured pixel belonged. This is wrong: `pixelRenderCounterPerScanLine` is **"which of the 160 visible output pixels is this"** (a small 0-159 counter that resets near the start of every line's drawing phase), not **"what is this pixel's absolute dot position within the full 456-dot scanline."** Using it caused every row's real image content to appear starting at dot 0 — completely overlapping and hiding the OAM-search (blue) band, since the image data was drawn *before* where Mode 2 should have ended.

The correct field is `ppuCounterPerLY` — the absolute per-scanline dot counter — which is what's actually captured into `PixelDebugInfo_t` (§4 of `PPU_DEBUGGER.md`) and what the scatter view's pixel-lookup now uses. **General lesson for future work in this file:** when correlating a captured pixel to an absolute timing position, always reach for the counter with "PPU" and "LY" in its name (`ppuCounterPerLY`), never the various "pixel render/fetcher counter" fields, which track *output pixel index*, a different and smaller quantity.

A second, related bug came from **trusting the mode-timeline's boundary** to decide where to start drawing real pixel data, instead of trusting the pixel-capture data directly. `STAT.MODE` and the per-pixel capture are two independently-read values, taken at slightly different points in the same tick, and can disagree by a few dots. The fix: **captured pixel data always wins** over the mode-timeline when deciding what to draw for a given dot; the mode-timeline is only consulted as a fallback for dots where no pixel data exists at all. Any place this view *still* shows a flat mode-color where you intuitively expected real image content is therefore not a rendering bug — it's the actual STAT-vs-pixel-commit timing gap, which is directly relevant information if you're chasing dot-accuracy questions (see `PPU_DEBUGGER.md` §9, the "off by ~5 dots vs. Emulicious" note).

### 4.4 Distinguishing HBlank from "no data yet" (Mode 3 fallback)

Both cases originally shared the same flat gray color, making it impossible to tell "this is genuinely HBlank" from "this is Mode 3, but the pixel-capture pipeline just hasn't recorded anything here" at a glance. HBlank now gets its own distinct maroon; the Mode-3-without-data fallback keeps the original gray.

---

## 5. Event log table

A standard scrollable table (Scanline, Dot, Event/register name, Old Value, New Value, PC), row-tinted by the same per-register color used in the scatter view and the register-filter tree, filterable by the same checkboxes.

### 5.1 True ring buffer, and why an earlier version wasn't one

The very first version of the event log simply stopped recording once its fixed-size array filled up — which is not a ring buffer, just a cap. Combined with a per-UI-frame full-table redraw (see §5.2), a long enough debugging session would both silently stop capturing anything new *and* visibly stall the UI. The corrected version is a genuine ring: once full, each new event **overwrites the oldest slot**, via a wrapping `head` index, so the log always reflects the most recent N events rather than "the first N events ever, then nothing more."

### 5.2 `ImGuiListClipper`, and why it's required here

Rendering a full ImGui table row per event, for a table that can legitimately hold thousands of entries, every single UI frame — even while the emulator is still running and the count keeps growing — is genuinely enough CPU work to look and feel like the application has hung. `ImGuiListClipper` is used specifically to only construct/render the rows that are actually scrolled into view, which is what makes a several-thousand-row log tractable at all. Any future table added to this debugger with an unbounded or large row count should use the same clipper pattern from the start.

### 5.3 PC column — an honest limitation

The `PC` column shows `pGBc_registers->pc` read **at the moment the diff is detected** (the tick immediately after a change is observed), not at the exact instruction that performed the write. For the vast majority of single-M-cycle register writes, this lands at-or-one-instruction-past the real write site — close enough to navigate to in practice — but it is **not** the same guarantee a genuine write-breakpoint would give you. If you need exact write-site precision, that requires real memory-access hooking (see §2), not this diff-based approach.

### 5.4 "Show Reads" — deliberately not implemented

A natural companion feature ("also show when a register is *read*, not just written") is not present, and isn't a partial/half-built stub — it's fully out of scope for this diffing approach, since a read produces no value change to diff against. Implementing it for real would mean hooking every memory-read site for the tracked addresses, a fundamentally different (and larger) mechanism than everything else in this file. Flagging this explicitly rather than silently omitting it, since a reasonable person could otherwise assume it was simply forgotten.

---

## 6. Data structures, for reference

```cpp
enum class GBC_DEBUG_TRACKED_REGISTER : uint8_t
{
    LCDC = 0, STAT, SCX, SCY, LY, LYC, DMA, BGP, OBP0, OBP1, WX, WY, COUNT
    // Extend here for CPU/APU registers later -- same pattern, same diff mechanism.
};

struct PPUEvent_t
{
    uint32_t frameNumber;
    BYTE scanline;
    uint16_t dot;           // absolute dot within the scanline (ppuCounterPerLY at capture time)
    uint8_t registerIndex;  // indexes GBC_DEBUG_TRACKED_REGISTER
    uint8_t oldValue;
    uint8_t newValue;
    uint16_t pc;            // best-effort, see §5.3
};

struct eventViewer_t
{
    static const int CAPACITY = 4096;

    FLAG enabled = NO;
    uint8_t lastValues[COUNT];          // previous-tick snapshot, for diffing
    FLAG showRegister[COUNT];           // per-register filter, shared by scatter/log/tree

    PPUEvent_t ring[CAPACITY];
    int head;   // next write slot (wraps)
    int count;  // valid entries, caps at CAPACITY

    uint8_t modeTimeline[154][456];     // STAT mode captured every tick, independent of diffing
    uint32_t frameCounter;
    int lastLY;
};
```

All of this lives inside `gbcDebugger_t` (`gbc.h`), outside the packed/save-state region — same rule as everything else in this debugger (see `PPU_DEBUGGER.md` §1, goal 2).

---

## 7. Extending this to CPU / APU / Memory

The register-tree UI already has grayed-out `APU (Coming Soon)` / `Memory (Coming Soon)` sections, structurally parallel to `PPU`. To add real tracking for a new domain:

1. Add new entries to (or a new sibling enum alongside) `GBC_DEBUG_TRACKED_REGISTER`.
2. Add their current-value reads to the `currentValues[]` build-up at the top of `debugEventViewerCheck()`.
3. Give them names/colors in the `regNames[]`/`regColors[]` arrays used by the scatter view, tree, and log table.

The diff/ring-buffer/scatter/log machinery itself needs no changes — it was written to be register-index-generic from the start, specifically so this extension is additive.

---

## 8. Known limitations (stated explicitly)

- **Write-only.** No read tracking (§5.4), and no plans to add it without a fundamentally different (hooking-based) mechanism.
- **A write of an unchanged value is invisible.** Diff-based by design (§2).
- **PC is approximate**, not exact-write-site (§5.3).
- **No historical frame navigation** beyond the current/most-recent frame (§4.2) — this was a deliberate simplification, not an oversight.
- **Mode-3 pixel backdrop reuses Complete Viewport's per-pixel capture**, which means it inherits that capture's own limitations (see `PPU_DEBUGGER.md` §5.4) — in particular, a pixel tagged as coming from a raster-scrolled row is placed using the dot/scanline it was captured at, which is correct for *this* view (it doesn't need SCX/SCY translation the way Complete Viewport's map overlay does), but if Complete Viewport's capture pipeline itself has a gap for a given frame, so does this backdrop.

---

## 9. Quick usage recipes

- **"Does this game do a mid-frame palette swap, and exactly where?"** → Enable tracking, watch the `BGP`/`OBP0`/`OBP1` rows in the scatter view for a marker partway down the frame; hover it for the exact scanline/dot and old→new value.
- **"Is this a raster-split effect (SCX/SCY changing every scanline), or a one-time scroll?"** → A dense, near-vertical streak of `SCX`/`SCY` markers spanning most/all of the visible scanlines means "every line" (raster split); a single isolated marker means a one-time change.
- **"What was the CPU doing when this register changed?"** → Read the `PC` column in the event log (with the accuracy caveat in §5.3 in mind).
- **"I'm chasing a dot-accuracy discrepancy against a reference emulator"** → Use the scatter view's pixel-accurate Mode-3 backdrop alongside the event markers; any place the real image content doesn't line up with where you'd expect based on the mode bands is itself useful signal, not just a display quirk (see §4.3).