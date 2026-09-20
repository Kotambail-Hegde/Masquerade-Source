#pragma once

// =========================================================
// PANEL INTERFACE — common types only
// Emulators include panel.h, NOT this file directly
//
// Each backend must define:
//   PANEL_SCREEN_WIDTH          uint32_t
//   PANEL_SCREEN_HEIGHT         uint32_t
//   PANEL_BACKEND_INIT()        void
//   PANEL_BACKEND_DEINIT()      void  — release backend hardware/resources
//   PANEL_BACKEND_CLEAR(c)      void  — c: uint16_t RGB565
//   PANEL_BACKEND_PRESENT(fb)   void  — fb: uint8_t*
//   PANEL_BACKEND_POINT(x,y,c)  void  — draw single pixel
//   PANEL_FB_WRITE(fb,x,y,outW,outH,c) void — write pixel directly into
//                                       raw fb, bypassing PANEL_BACKEND_POINT.
//                                       Backend owns its own layout (row-major,
//                                       column-major, flipped, etc.)
//   PANEL_FONT_T                 typedef — font struct type for DrawChar/DrawString
//   PANEL_FONT_SMALL             PANEL_FONT_T instance — used by PrintStr
//   PANEL_FONT_MEDIUM            PANEL_FONT_T instance — used by ShowFPS
// =========================================================

#include "helpers.h"

struct PCTX
{
    uint16_t* fb;
    uint32_t  outW;
    uint32_t  outH;
    uint32_t  inW;
    uint32_t  inH;
    uint32_t  scale;
    uint32_t  offsetX;
    uint32_t  offsetY;
};

MASQ_INLINE void PANEL_SET_PARAMS(
    PCTX&     ctx,
    uint16_t* framebuffer,
    uint32_t  outW,
    uint32_t  outH
){
    ctx.fb   = framebuffer;
    ctx.outW = outW;
    ctx.outH = outH;
}
