#pragma once

// =========================================================
// ILI BACKEND — STUB
// =========================================================

#include "panel_interface.h"

// TODO: #include "ILI9341.h" or similar

#define PANEL_SCREEN_WIDTH            320u
#define PANEL_SCREEN_HEIGHT           240u
#define PANEL_BACKEND_INIT()          /* TODO */
#define PANEL_BACKEND_DEINIT()        /* TODO */
#define PANEL_BACKEND_CLEAR(c)        /* TODO */
#define PANEL_BACKEND_PRESENT(fb)     /* TODO */
#define PANEL_BACKEND_POINT(x, y, c)  /* TODO */

// STUB — naive row-major write, NOT verified against real ILI9341 GRAM/MADCTL
// orientation. Compiles only; will render wrong until replaced.
#define PANEL_FB_WRITE(fb, x, y, outW, outH, c) ((fb)[(y) * (outW) + (x)] = (c))

// STUB font — zeroed table, compiles but draws nothing until replaced
// with a real ILI9341 font source.
struct PANEL_FONT_T
{
    uint16_t       Width;
    uint16_t       Height;
    const uint8_t* table;
};
static const uint8_t  __panel_font_stub_table[1] = { 0 };
static PANEL_FONT_T   PANEL_FONT_SMALL  = { 8,  8,  __panel_font_stub_table };
static PANEL_FONT_T   PANEL_FONT_MEDIUM = { 12, 12, __panel_font_stub_table };

MASQ_INLINE FLAG PANEL_INIT()
{
	RETURN FAILURE;
}