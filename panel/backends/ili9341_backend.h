#pragma once

// =========================================================
// ILI BACKEND — STUB
// =========================================================

#include "panel_interface.h"

// TODO: #include "ILI9341.h" or similar

#define PANEL_SCREEN_WIDTH            320u
#define PANEL_SCREEN_HEIGHT           240u
#define PANEL_BACKEND_INIT()          /* TODO */
#define PANEL_BACKEND_CLEAR(c)        /* TODO */
#define PANEL_BACKEND_PRESENT(fb)     /* TODO */
#define PANEL_BACKEND_POINT(x, y, c)  /* TODO */

MASQ_INLINE FLAG PANEL_INIT()
{
	RETURN FAILURE;
}