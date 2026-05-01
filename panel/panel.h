#pragma once

// =========================================================
// panel.h — ONLY file emulators should include
//
// Only include this in Pico builds:
//   #ifdef __RPI_PICO__
//   #include "panel/panel.h"
//   #endif
//
// CMake backend selection:
//   -DPANEL_BACKEND=PICOLCD2   Waveshare 2in LCD
//   -DPANEL_BACKEND=ILI9341    ILI series LCD
// =========================================================

#if defined(PANEL_BACKEND_PICOLCD2)
    #include "picolcd2_backend.h"
#elif defined(PANEL_BACKEND_ILI9341)
    #include "li9341_backend.h"
#else
    #error "No panel backend defined. Add -DPANEL_BACKEND=<backend> to CMake."
#endif

#include "panel_utils.h"
