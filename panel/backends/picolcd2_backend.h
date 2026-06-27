#pragma once

// =========================================================
// WAVESHARE 2IN BACKEND
// =========================================================

#include "panel_interface.h"

extern "C" {
    #include "LCD_2in.h"
    #include "fonts.h"
}

#pragma region CLEANUP_POLLUTING_MACROS
#undef WHITE
#undef BLACK
#undef BLUE
#undef RED
#undef GREEN
#undef CYAN
#undef MAGENTA
#undef YELLOW
#undef BROWN
#undef GRAY
#undef GRED
#undef GBLUE
#undef BRED
#undef BRRED
#pragma endregion CLEANUP_POLLUTING_MACROS

#define PANEL_SCREEN_WIDTH            ((uint32_t)LCD_2IN_WIDTH)
#define PANEL_SCREEN_HEIGHT           ((uint32_t)LCD_2IN_HEIGHT)
#define PANEL_BACKEND_INIT()          LCD_2IN_Init(HORIZONTAL)
#define PANEL_BACKEND_CLEAR(c)        LCD_2IN_Clear(c)
#define PANEL_BACKEND_PRESENT(fb)     LCD_2IN_Display((UBYTE*)(fb))
#define PANEL_BACKEND_POINT(x, y, c)  LCD_2IN_DisplayPoint((x), (y), (c))

MASQ_INLINE FLAG PANEL_INIT()
{
	DEV_Delay_ms(100);
	if (DEV_Module_Init() != RESET)
	{
		RETURN FAILURE;
	}
	DEV_SET_PWM(50);
    
	RETURN SUCCESS;
}