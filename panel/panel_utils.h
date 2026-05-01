#pragma once

// =========================================================
// PANEL UTILITIES — backend agnostic
// Requires backend to be included first (via panel.h)
// Uses only: PANEL_SCREEN_WIDTH, PANEL_SCREEN_HEIGHT,
//            PANEL_BACKEND_POINT, PANEL_BACKEND_CLEAR
// =========================================================

#pragma region PIXEL_CONVERSION
MASQ_INLINE uint16_t PixelToRGB565_fast(const Pixel& p)
{
    RETURN (uint16_t)(
        ((p.r >> 3) << 11) |
        ((p.g >> 2) << 5 ) |
        ((p.b >> 3))
    );
}

static MASQ_INLINE uint16_t swap16(uint16_t v)
{
    RETURN (v >> 8) | (v << 8);
}
#pragma endregion PIXEL_CONVERSION

#pragma region BLITTER_PIXEL
#if MASQ_ENABLE_GOL
#endif
#if MASQ_ENABLE_CHIP8
// Chip-8 graphics are stored in separate bitplanes, so we need to combine them into RGB565 format for our framebuffer.
MASQ_INLINE void PANEL_TRANSFORM_FOR_CHIP8(
    PCTX&           ctx,
    const uint8_t*  gfx,
    uint8_t         numPlanes,
    const uint32_t* colorLUT,
    uint32_t        inW,
    uint32_t        inH,
    FLAG            maintainAspect = true
){
    if (inW == 0 || inH == 0) RETURN;

    const uint32_t outW = ctx.outW;
    const uint32_t outH = ctx.outH;
    uint32_t scaleX, scaleY;

    if (maintainAspect)
    {
        const uint32_t scale = ((outW / inW) < (outH / inH)) ? (outW / inW) : (outH / inH);
        if (scale == 0) RETURN;
        scaleX = scale;
        scaleY = scale;
    }
    else
    {
        scaleX = outW / inW;
        scaleY = outH / inH;
        if (scaleX == 0 || scaleY == 0) RETURN;
    }

    const uint32_t offsetX   = (outW - inW * scaleX) / 2;
    const uint32_t offsetY   = (outH - inH * scaleY) / 2;
    const uint32_t planeSize = inW * inH;
    uint16_t*      dst       = ctx.fb;

    // precompute RGB565 LUT — avoids per-pixel conversion
    const uint8_t numColors = 1 << numPlanes;
    uint16_t rgb565LUT[4];
    for (uint8_t i = 0; i < numColors; i++)
    {
        Pixel p; p.n = colorLUT[i];
        rgb565LUT[i] = swap16(PixelToRGB565_fast(p));
    }

    memset(dst, 0, outW * outH * sizeof(uint16_t));

    for (uint32_t y = 0; y < inH; y++)
    {
        for (uint32_t x = 0; x < inW; x++)
        {
            uint8_t idx = 0;
            for (uint8_t plane = 0; plane < numPlanes; plane++)
            {
                if (gfx[plane * planeSize + y * inW + x] == SET)
                    idx |= (1 << plane);
            }

            const uint16_t color  = rgb565LUT[idx];
            const uint32_t basePX = (outW - 1) - (offsetX + x * scaleX);
            const uint32_t basePY = offsetY + y * scaleY;

            for (uint32_t dy = 0; dy < scaleY; dy++)
            {
                for (uint32_t dx = 0; dx < scaleX; dx++)
                {
                    // column-major to match PANEL_BACKEND_PRESENT
                    dst[(basePX - dx) * outH + (basePY + dy)] = color;
                }
            }
        }
    }
}
#endif
#if MASQ_ENABLE_SI
#endif
#if MASQ_ENABLE_PACMAN
#endif
#if MASQ_ENABLE_NES
#endif
#if MASQ_ENABLE_GBC
#endif
#if MASQ_ENABLE_GBA
#endif
#pragma endregion BLITTER_PIXEL

#pragma region DRAW_PRIMITIVES
MASQ_INLINE void PANEL_DrawChar(uint16_t x, uint16_t y, char c,
                                 sFONT* font, uint16_t fg, uint16_t bg)
{
    uint32_t       offset = (c - ' ') * font->Height *
                            (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const uint8_t* ptr   = &font->table[offset];

    for (uint16_t row = 0; row < font->Height; row++)
    {
        for (uint16_t col = 0; col < font->Width; col++)
        {
            PANEL_BACKEND_POINT(x + col, y + row,
                (*ptr & (0x80 >> (col % 8))) ? fg : bg);
            if (col % 8 == 7) ptr++;
        }
        if (font->Width % 8 != 0) ptr++;
    }
}

MASQ_INLINE void PANEL_DrawString(uint16_t x, uint16_t y, const char* str,
                                   sFONT* font, uint16_t fg, uint16_t bg)
{
    uint16_t cx = x;
    uint16_t cy = y;
    while (*str != '\0')
    {
        if ((cx + font->Width)  > PANEL_SCREEN_WIDTH)  { cx = x; cy += font->Height; }
        if ((cy + font->Height) > PANEL_SCREEN_HEIGHT) { cx = x; cy = y; }
        PANEL_DrawChar(cx, cy, *str, font, fg, bg);
        str++;
        cx += font->Width;
    }
}

static MASQ_INLINE void PANEL_DrawLine(uint16_t x0, uint16_t y0,
                                        uint16_t x1, uint16_t y1,
                                        uint16_t color, uint8_t thickness)
{
    int32_t dx  = abs((int32_t)x1 - x0);
    int32_t dy  = abs((int32_t)y1 - y0);
    int32_t sx  = x0 < x1 ? 1 : -1;
    int32_t sy  = y0 < y1 ? 1 : -1;
    int32_t err = dx - dy;

    while (true)
    {
        for (int8_t tx = -thickness/2; tx <= thickness/2; tx++)
            for (int8_t ty = -thickness/2; ty <= thickness/2; ty++)
                PANEL_BACKEND_POINT(x0 + tx, y0 + ty, color);
        if (x0 == x1 && y0 == y1) break;
        int32_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

MASQ_INLINE void PANEL_DrawGFX(const bool* gfx, uint32_t inW, uint32_t inH,
                                 uint16_t fgColor, uint16_t bgColor)
{
    for (uint32_t y = 0; y < inH; y++)
        for (uint32_t x = 0; x < inW; x++)
            PANEL_BACKEND_POINT(x, y, gfx[y * inW + x] ? fgColor : bgColor);
}
#pragma endregion DRAW_PRIMITIVES

#pragma region PANEL_STATUS
MASQ_INLINE void PANEL_ShowStatus(bool flag)
{
    const uint16_t cx = PANEL_SCREEN_WIDTH  / 2;
    const uint16_t cy = PANEL_SCREEN_HEIGHT / 2;
    const uint16_t s  = 30;

    if (flag)
    {
        PANEL_DrawLine(cx - s,   cy,       cx - s/3, cy + s/2, swap16(PixelToRGB565_fast(GREEN)), 4);
        PANEL_DrawLine(cx - s/3, cy + s/2, cx + s,   cy - s/2, swap16(PixelToRGB565_fast(GREEN)), 4);
    }
    else
    {
        PANEL_DrawLine(cx - s, cy - s, cx + s, cy + s, swap16(PixelToRGB565_fast(RED)), 4);
        PANEL_DrawLine(cx + s, cy - s, cx - s, cy + s, swap16(PixelToRGB565_fast(RED)), 4);
    }
}
#pragma endregion PANEL_STATUS

#pragma region PANEL_PRINTF
// Forward declared in helpers.h to avoid circular include
MASQ_INLINE void PANEL_PrintStr(const char* buf)
{
    static uint16_t cursorY    = 12;
    const uint16_t  lineHeight = Font8.Height + 2;
    const uint16_t  startY     = 10;
    const uint16_t  maxY       = PANEL_SCREEN_HEIGHT - lineHeight;

    if (cursorY + lineHeight > maxY)
    {
        PANEL_BACKEND_CLEAR(swap16(PixelToRGB565_fast(WHITE)));
        cursorY = startY;
    }
    PANEL_DrawString(10, cursorY, buf, &Font8,
                     swap16(PixelToRGB565_fast(BLACK)),
                     swap16(PixelToRGB565_fast(WHITE)));
    cursorY += lineHeight;
}

inline void PANEL_printf(const char* fmt, ...)
{
    char    buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    PANEL_PrintStr(buf);
}
#pragma endregion PANEL_PRINTF

#pragma region PANEL_FPS_OVERLAY
// Draws FPS counter directly into ctx.fb — no extra SPI call
MASQ_INLINE void PANEL_ShowFPS(PCTX& ctx)
{
    static uint32_t        frameCount = 0;
    static absolute_time_t lastTime   = get_absolute_time();
    static char            fpsBuf[32] = "FPS: --";

    frameCount++;
    absolute_time_t now     = get_absolute_time();
    int64_t         elapsed = absolute_time_diff_us(lastTime, now);

    if (elapsed >= 1000000)
    {
        snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %.1f",
                 (float)frameCount * 1000000.0f / (float)elapsed);
        frameCount = 0;
        lastTime   = now;
    }

    sFONT*      font = &Font12;
    const char* str  = fpsBuf;
    uint16_t    cx   = 10;
    uint16_t    cy   = 10;

    while (*str != '\0')
    {
        uint32_t       offset = (*str - ' ') * font->Height *
                                (font->Width / 8 + (font->Width % 8 ? 1 : 0));
        const uint8_t* ptr   = &font->table[offset];

        for (uint16_t row = 0; row < font->Height; row++)
        {
            for (uint16_t col = 0; col < font->Width; col++)
            {
                uint16_t color = (*ptr & (0x80 >> (col % 8)))
                    ? swap16(PixelToRGB565_fast(WHITE))
                    : swap16(PixelToRGB565_fast(BLACK));
                // column-major, x-flipped to match PANEL_TRANSFORM_FOR_CHIP8
                ctx.fb[(ctx.outW - 1 - (cx + col)) * ctx.outH + (cy + row)] = color;
                if (col % 8 == 7) ptr++;
            }
            if (font->Width % 8 != 0) ptr++;
        }
        str++;
        cx += font->Width;
    }
}
#pragma endregion PANEL_FPS_OVERLAY

#pragma region DUAL_CORE
static volatile FLAG  frameReady = CLEAR;
static volatile FLAG  core1Busy  = CLEAR;
static uint8_t*       g_panelFb  = nullptr;

inline void panel_core1_entry()
{
    while (true)
    {
        if (frameReady)
        {
            core1Busy  = YES;
            frameReady = NO;
            PANEL_BACKEND_PRESENT(g_panelFb);
            core1Busy  = NO;
        }
    }
}

// Call once after pctx.fb is set up
MASQ_INLINE void PANEL_INIT_CORE1(PCTX& ctx)
{
    g_panelFb = (uint8_t*)ctx.fb;
    multicore_launch_core1(panel_core1_entry);
}

// Call at end of each frame — signals core1 to send fb
MASQ_INLINE void PANEL_PRESENT_FRAME(PCTX& ctx)
{
    PANEL_ShowFPS(ctx);
    while (core1Busy) {}  // wait only if core1 still sending
    __dmb();
    frameReady = true;    // signal core1 — core0 RETURNs immediately
}
#pragma endregion DUAL_CORE
