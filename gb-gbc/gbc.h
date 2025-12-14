#pragma once

#pragma region REFERENCES
#pragma endregion REFERENCES

#pragma region INCLUDES
//
#include "helpers.h"
//
#include "abstractEmulation.h"
#pragma endregion INCLUDES

#pragma region MACROS
#pragma region WIP
#pragma endregion WIP
#define GB_GBC_FPS										59.73f
#define EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC			48000.0f
#ifdef __EMSCRIPTEN__
#define AUDIO_BUFFER_SIZE_FOR_GB_GBC					(CEIL((EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC / GB_GBC_FPS)))  // 32
#else
#define AUDIO_BUFFER_SIZE_FOR_GB_GBC					(CEIL((EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC / GB_GBC_FPS)))
#endif
#define PIXEL_FIFO_SIZE_FOR_GB_GBC						(SIXTEEN)

#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif
#pragma endregion MACROS

#pragma region CORE
class GBc_t : public abstractEmulation_t
{
#pragma region INFRASTRUCTURE_DECLARATIONS

public:

	std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom;
	const float myFPS = (float)GB_GBC_FPS;

public:

	uint32_t y_offset = 0;
	uint32_t x_offset = 0;
	static const uint32_t screen_height = 144;
	static const uint32_t screen_width = 160;
	static const uint32_t pixel_height = 2;
	static const uint32_t pixel_width = 2;
	static const uint32_t debugger_screen_height = 560;
	static const uint32_t debugger_screen_width = 456; // 880
	static const uint32_t debugger_pixel_height = 1;
	static const uint32_t debugger_pixel_width = 1;
	const char* NAME = "GB-GBC";

private:

	boost::property_tree::ptree pt;

private:

	uint8_t const SST_ROMS = TWO;

private:

	uint32_t profiler_FrameRate;
	uint64_t functionID;

private:

	CheatEngine_t* ceGBGBC;
#pragma endregion INFRASTRUCTURE_DECLARATIONS

#pragma region SM83_DECLARATION
private:

	enum class REGISTER_TYPE	// register_op_type
	{
		RT_A,				// 0
		RT_F,				// 1
		RT_B,				// 2
		RT_C,				// 3
		RT_D,				// 4
		RT_E,				// 5
		RT_H,				// 6
		RT_L,				// 7
		RT_PC,				// 8
		RT_SP,				// 9
		RT_AF,				// 10
		RT_BC,				// 11
		RT_DE,				// 12
		RT_HL,				// 13
		RT_IE,				// 14
		RT_TOTAL,			// TOTAL = 15
		RT_NONE
	};

	enum class POINTER_TYPE
	{
		RT_M_HL,			// 0
		RT_M_DE,			// 1
		RT_M_BC,			// 2
		RT_M_TOTAL,			// TOTAL = 3
		RT_M_NONE
	};

	enum class CPU_TICK_TYPE
	{
		READ_WRITE,
		DUMMY
	};

private:

#pragma pack(push, 1)

	typedef struct
	{
		uint8_t opcode;
		uint8_t previousOpcode;
	} cpu_t;

	typedef struct
	{
		uint8_t ZEROTH : 1; // bit  0
		uint8_t FIRST : 1; // bit  1
		uint8_t SECOND : 1; // bit  2
		uint8_t THIRD : 1; // bit  3
		uint8_t FCARRY : 1; // bit  4
		uint8_t FHALFCARRY : 1; // bit  5	
		uint8_t FSUB : 1; // bit  6
		uint8_t FZERO : 1; // bit  7
	} flagFields_t;

	typedef union
	{
		flagFields_t flagFields;
		uint8_t flagMemory;
	} flag_t;

	typedef struct
	{
		flag_t f;				// 0 - 7
		uint8_t a;				// 8 - 15
	} aAndFRegisters_t;

	typedef union
	{
		aAndFRegisters_t aAndFRegisters;
		uint16_t af_u16memory;	// <-a->|<-f->
	} af_t;

	typedef struct
	{
		uint8_t c;				// 0 - 7
		uint8_t b;				// 8 - 15
	} bAndCRegisters_t;

	typedef union
	{
		bAndCRegisters_t bAndCRegisters;
		uint16_t bc_u16memory;	// <-b->|<-c->
	} bc_t;

	typedef struct
	{
		uint8_t e;				// 0 - 7
		uint8_t d;				// 8 - 15
	} dAndERegisters_t;

	typedef union
	{
		dAndERegisters_t dAndERegisters;
		uint16_t de_u16memory;	// <-d->|<-e->
	} de_t;

	typedef struct
	{
		uint8_t l;				// 0 - 7
		uint8_t h;				// 8 - 15
	} hAndLRegisters_t;

	typedef union
	{
		hAndLRegisters_t hAndLRegisters;
		uint16_t hl_u16memory;	// <-h->|<-l->
	} hl_t;

	typedef struct
	{
		af_t af;
		bc_t bc;
		de_t de;
		hl_t hl;
		uint16_t pc;
		uint16_t sp;
	} registers_t;

#pragma endregion SM83_DECLARATION

#pragma region EMULATION_DECLARATIONS
private:

	bios_t dmg_cgb_bios;

private:

	enum class MEMORY_ACCESS_SOURCE
	{
		DEBUG_PORT,
		CPU,
		PPU,
		APU,
		OAMDMA,
		GPDMA,
		HDMA,
		BESS
	};

	enum HALT_BUG_STATE : bool
	{
		HALT_BUG_DISABLED,
		HALT_BUG_ENABLED
	};

	enum class INTERRUPTS
	{
		INTERRUPT_INVALID = -1,
		NO_INTERRUPT = 0x00,
		VBLANK_INTERRUPT = 0x01,
		LCD_STAT_INTERRUPT = 0x02,
		TIMER_INTERRUPT = 0x04,
		SERIAL_INTERRUPT = 0x08,
		JOYPAD_INTERRUPT = 0x10
	};

	enum EI_ENABLE_STATE : bool
	{
		NOTHING_TO_BE_DONE,
		EI_TO_BE_ENABLED
	};

	enum class CGB_DMA_MODE
	{
		GPDMA = 0,
		HDMA
	};

	enum DIVIDERS : int16_t
	{
		DIVIDER_INVALID = -1,
		DIVIDER_TOTAL = 1,

		// DIVIDER_01 is 16384 Hz -> 16384 increments in 1 second (runs w.r.t to Clock like eveything else in GB)
		// Clock is 4.194304 MHz -> 4194304 increments or ticks in 1 second
		// We have the clock ticks in the form of emulated CPU cycles 
		// Display Rate / VBlank interrupt for GB is @ 60 Hz, so @ 60 Hz, we need to re-process and push the COMPLETE HUGE GFX to display
		// So, our master loop is set to run @ 60 Hz, which inturn will house the while loop for core; after every 60 Hz loop, we process the VBlank
		// We run a continous while loop for simulating the GB core, but the host speed is too much, so to maintain it similar to GB
		// At max, we need to run 70221 core loops per 60 Hz function
		// In one frame @ 60 Hz, we run for 70221 clock ticks (i.e emulated CPU cycles)
		// In one frame @ 60 Hz, we run for (4.194304 MHx / 16384 Hz) divider ticks
		// In one frame @ 60 Hz, we run for 256 divider ticks

		DIVIDER_01 = 256
	};

	enum TIMERS : int16_t
	{
		TIMER_INVALID = -1,
		// Refer DIVIDERS enum for the explaination on how we got the value for TIMER_01
		TIMER_01 = 16,		// 4.194304 MHz / 262144 Hz
		// Refer DIVIDERS enum for the explaination on how we got the value for TIMER_02
		TIMER_10 = 64,		// 4.194304 MHz / 65536 Hz
		// Refer DIVIDERS enum for the explaination on how we got the value for TIMER_03
		TIMER_11 = 256,		// 4.194304 MHz / 16384 Hz
		// Refer DIVIDERS enum for the explaination on how we got the value for TIMER_04
		TIMER_00 = 1024		// 4.194304 MHz / 4096 Hz 
	};

	enum JOYPAD_STATES : uint8_t
	{
		PRESSED = ZERO,
		NOT_PRESSED = ONE
	};

	enum AUDIO_CHANNELS : uint8_t
	{
		CHANNEL_1 = ZERO,
		CHANNEL_2 = ONE,
		CHANNEL_3 = TWO,
		CHANNEL_4 = THREE,
		TOTAL_CHANNELS
	};

	enum class AUDIO_STREAMS
	{
		L = ZERO,
		R = ONE,
		TOTAL_AUDIO_STREAMS
	};

	const uint8_t AUDIO_CHANNEL_4_DIVISOR[EIGHT]
	{
		/* 0 */ 8,
		/* 1 */ 16,
		/* 2 */ 32,
		/* 3 */ 48,
		/* 4 */ 64,
		/* 5 */ 80,
		/* 6 */ 96,
		/* 7 */ 112
	};

	const uint8_t SQUARE_WAVE_AMPLITUDE[FOUR][EIGHT] =
	{
		{LO, LO, LO, LO, LO, LO, LO, HI},
		{LO, LO, LO, LO, LO, LO, HI, HI},
		{LO, LO, LO, LO, HI, HI, HI, HI},
		{HI, HI, HI, HI, HI, HI, LO, LO}
	};

	enum class PIXEL_FETCHER_STATES
	{
		DUMMY = ZERO,
		WAIT_FOR_TILE = ONE,
		GET_TILE = TWO,
		WAIT_FOR_DATA_LOW = THREE,
		GET_TILE_DATA_LOW = FOUR,
		WAIT_FOR_DATA_HIGH = FIVE,
		GET_TILE_DATA_HIGH = SIX,
		SLEEP_OR_PUSH = SEVEN
	};

	enum LCD_MODES : uint8_t
	{
		MODE_LCD_H_BLANK = 0,
		MODE_LCD_V_BLANK = 1,
		MODE_LCD_SEARCHING_OAM = 2,
		MODE_LCD_DISPLAY_PIXELS = 3,
		// This is set to 4 as bits 0-1 is still 0 and we can differentiate b/w MODE_LCD_H_BLANK and MOCE_LCD_BITS_CLEAR
		MODE_LCD_BITS_CLEAR = 4,
	};

	enum LCD_MODE_CYCLES : uint16_t
	{
		LCD_SEARCHING_OAM = 80,
		TX_DATA_LCD_CTRL_MIN = 172,
		TX_DATA_LCD_CTRL_MAX = 289,
		LCD_H_BLANK_MIN = 87,
		LCD_H_BLANK_MAX = 204,
		LCD_TOTAL_CYCLES_PER_SCANLINE = 456,
		LCD_V_BLANK = 4560,
	};

private:

	typedef struct
	{
		uint32_t placeholder;
	} quirks_t;

private:

	static const uint8_t ALPHA = 255;

	enum class colorID
	{
		COLOR_099P,
		COLOR_066P,
		COLOR_033P,
		COLOR_000P
	};

	typedef struct
	{
		Pixel COLOR;
		colorID COLOR_ID;
	} COLOR_FORMAT;

	typedef struct
	{
		COLOR_FORMAT COLOR_099P;
		COLOR_FORMAT COLOR_066P;
		COLOR_FORMAT COLOR_033P;
		COLOR_FORMAT COLOR_000P;
	} FORMAT_2BPP;

	std::unordered_map<PALETTE_ID, FORMAT_2BPP> const paletteIDToColor =
	{
		{PALETTE_ID::PALETTE_1,
		{{Pixel(0x14, 0x44, 0x03, ALPHA), colorID::COLOR_099P}, {Pixel(0x2B, 0x55, 0x03, ALPHA), colorID::COLOR_066P}, {Pixel(0x4D, 0x6B, 0x03, ALPHA), colorID::COLOR_033P}, {Pixel(0x87, 0x96, 0x03, ALPHA), colorID::COLOR_000P}}}
		,{PALETTE_ID::PALETTE_2,
		{{Pixel(0x00, 0x00, 0x00, ALPHA), colorID::COLOR_099P}, {Pixel(0x55, 0x55, 0x55, ALPHA), colorID::COLOR_066P}, {Pixel(0xAA, 0xAA, 0xAA, ALPHA), colorID::COLOR_033P}, {Pixel(0xFF, 0xFF, 0xFF, ALPHA), colorID::COLOR_000P}}}
		,{PALETTE_ID::PALETTE_3,
		{{Pixel(0x08, 0x18, 0x10, ALPHA), colorID::COLOR_099P}, {Pixel(0x39, 0x61, 0x39, ALPHA), colorID::COLOR_066P}, {Pixel(0x84, 0xA5, 0x63, ALPHA), colorID::COLOR_033P}, {Pixel(0xC6, 0xDE, 0x8C, ALPHA), colorID::COLOR_000P}}}
		,{PALETTE_ID::PALETTE_4,
		{{Pixel(0x08, 0x18, 0x20, ALPHA), colorID::COLOR_099P}, {Pixel(0x34, 0x68, 0x56, ALPHA), colorID::COLOR_066P}, {Pixel(0x88, 0xC0, 0x70, ALPHA), colorID::COLOR_033P}, {Pixel(0xE0, 0xF8, 0xD0, ALPHA), colorID::COLOR_000P}}}
	};

private:

	const char* ROM_TYPES[0x23] =
	{
		"ROM ONLY",
		"MBC1",
		"MBC1+RAM",
		"MBC1+RAM+BATTERY",
		"0x04 ???",
		"MBC2",
		"MBC2+BATTERY",
		"0x07 ???",
		"ROM+RAM 1",
		"ROM+RAM+BATTERY 1",
		"0x0A ???",
		"MMM01",
		"MMM01+RAM",
		"MMM01+RAM+BATTERY",
		"0x0E ???",
		"MBC3+TIMER+BATTERY",
		"MBC3+TIMER+RAM+BATTERY 2",
		"MBC3",
		"MBC3+RAM 2",
		"MBC3+RAM+BATTERY 2",
		"0x14 ???",
		"0x15 ???",
		"0x16 ???",
		"0x17 ???",
		"0x18 ???",
		"MBC5",
		"MBC5+RAM",
		"MBC5+RAM+BATTERY",
		"MBC5+RUMBLE",
		"MBC5+RUMBLE+RAM",
		"MBC5+RUMBLE+RAM+BATTERY",
		"0x1F ???",
		"MBC6",
		"0x21 ???",
		"MBC7+SENSOR+RUMBLE+RAM+BATTERY",
	};

	const char* LIC_CODE[0xA5] =
	{
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"Ubi Soft",
		"Atlus",
		"unknown",
		"Malibu",
		"unknown",
		"angel",
		"Bullet-Proof",
		"unknown",
		"irem",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"Absolute",
		"Acclaim",
		"Activision",
		"American sammy",
		"Konami",
		"Hi tech entertainment",
		"LJN",
		"Matchbox",
		"Mattel",
		"Milton Bradley",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"Titus",
		"Virgin",
		"unknown",
		"unknown",
		"LucasArts",
		"unknown",
		"unknown",
		"Ocean",
		"unknown",
		"Electronic Arts",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"Infogrames",
		"Interplay",
		"Broderbund",
		"sculptured",
		"unknown",
		"sci",
		"unknown",
		"unknown",
		"THQ",
		"Accolade",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"misawa",
		"unknown",
		"unknown",
		"lozc",
		"Tokuma Shoten Intermedia",
		"unknown",
		"unknown",
		"Tsukuda Original",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"Chunsoft",
		"Video system",
		"Ocean/Acclaim",
		"unknown",
		"Varie",
		"Yonezawa Toys",
		"Kaneko",
		"unknown",
		"Pack in soft",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"Konami (Yu-Gi-Oh!)"
	};

	typedef struct
	{
		uint8_t CLOCK_SELECT : 1; // bit  0
		uint8_t CLOCK_SPEED : 1; // bit  1
		uint8_t RESERVED : 5; // bit  2 - 6
		uint8_t TRANSFER_ENABLE : 1; // bit  7
	} scFields_t;

	typedef union
	{
		uint8_t scMemory;
		scFields_t scFields;
	} sc_t;

	typedef struct
	{
		uint8_t title[0x000F];
		uint8_t cgbType;
	} tile_AND_cgbType_Fields_t;

	typedef union
	{
		tile_AND_cgbType_Fields_t tile_AND_cgbType_Fields;
		uint8_t title[0x0010];
	} title_AND_cgbType_t;

	typedef struct
	{
		uint8_t entryPoint[0x0004];
		uint8_t nintendologo[0x0030];
		title_AND_cgbType_t title;
		uint16_t newLicCode;
		uint8_t sgbFlag;
		uint8_t cartridgeType;
		uint8_t romSize;
		uint8_t ramSize;
		uint8_t destinationCode;
		uint8_t oldLicCode;
		uint8_t maskRomVersion;
		uint8_t headerChecksum;
		uint16_t globalChecksum;
	} cartridge_header_fields_t;

	typedef union
	{
		cartridge_header_fields_t cartridge_header_fields;
		uint8_t cartridge_header_buffer[sizeof(cartridge_header_fields_t)];
	} cartridge_header_t;

	typedef struct
	{
		uint8_t romBank00_Field1[0x0100];		// 0x0000 - 0x00FF
		cartridge_header_t cartridge_header;	// 0x0100 - 0x014F
		uint8_t romBank00_Field2[0x3EB0];		// 0x0150 - 0x3FFF
	} romBank00_Fields_t;

	typedef union
	{
		romBank00_Fields_t romBank00_Fields;
		uint8_t romBank00_Memory[0x4000];
	} romBank00_t;

	typedef struct
	{
		romBank00_t romBank_00;
		BYTE romBank_NN[0x4000];
	} codeRomFields_t;

	typedef union
	{
		codeRomFields_t codeRomFields;
		BYTE codeRomMemory[sizeof(codeRomFields_t)];
	} codeRomMemory_t;

	typedef struct
	{
		uint8_t tileData[0x0800];
	} videoRamTileDataField_t;

	typedef struct
	{
		videoRamTileDataField_t tileDataBlock[3];
	} videoRamTileDataFields_t;

	typedef struct
	{
		uint8_t tileMap[0x0400];
	} videoRamTileMapField_t;

	typedef struct
	{
		videoRamTileMapField_t tileMapBlock[2];
	} videoRamTileMapFields_t;

	typedef struct
	{
		videoRamTileDataFields_t videoRamTileDataFields;
		videoRamTileMapFields_t videoRamTileMapField;
	} videoRamFields_t;

	typedef union
	{
		videoRamFields_t videoRamFields;
		uint8_t videoRamMemory[sizeof(videoRamFields_t)];
	} videoRamMemory_t;

	typedef struct
	{
		uint8_t externalRam[0x2000];
	} externalRamFields_t;

	typedef union
	{
		externalRamFields_t externalRamFields;
		uint8_t externalRamMemory[sizeof(externalRamFields_t)];
	} externalRamMemory_t;

	typedef struct
	{
		uint8_t workRam_00[0x1000];
		uint8_t workRam_NN[0x1000];
	} workRamFields_t;

	typedef union
	{
		workRamFields_t workRamFields;
		uint8_t wRamMemory[sizeof(workRamFields_t)];
	} workRamMemory_t;

	typedef struct
	{
		uint8_t echoRam[0x1E00];
	} echoRamFields_t;

	typedef union
	{
		echoRamFields_t echoRamFields;
		uint8_t echoRamMemory[sizeof(echoRamFields_t)];
	} echoRamMemory_t;

	typedef struct
	{
		uint8_t OAM_PALETTE_NUMBER_CGB : 3; // bit  0 - 2
		uint8_t OAM_TILE_VRAM_BANK : 1; // bit  3
		uint8_t OAM_PALETTE_NUMBER_DMG : 1; // bit  4
		uint8_t OAM_X_FLIP : 1; // bit  5	
		uint8_t OAM_Y_FLIP : 1; // bit  6
		uint8_t OAM_BG_WINDOW_OVER_OBJ : 1; // bit  7
	} oamEntryFields_t;

	typedef union
	{
		oamEntryFields_t oamEntryFields;
		uint8_t oamEntryByte;
	} oamEntryByte_t;

	typedef struct
	{
		BYTE yPosition;
		BYTE xPosition;
		BYTE tileIndex;
		oamEntryByte_t attributes;
	} OAMEntry_t;

	typedef struct
	{
		OAMEntry_t OAM[(0x00A0 / FOUR)];
	} OAMFields_t;

	typedef union
	{
		OAMFields_t OAMFields;
		uint8_t OAMMemory[sizeof(OAMFields_t)];
	} OAMMemory_t;

	typedef struct
	{
		uint8_t CLOCK_SELECT : 2; // bit  0 - 1
		uint8_t TIMER_ENABLE : 1; // bit  2
		uint8_t TAC_3 : 1; // bit  3
		uint8_t TAC_4 : 1; // bit  4
		uint8_t TAC_5 : 1; // bit  5	
		uint8_t TAC_6 : 1; // bit  6
		uint8_t TAC_7 : 1; // bit  7
	} timerControlFields_t;

	typedef union
	{
		timerControlFields_t timerControlFields;
		uint8_t timerControlMemory;
	} timerControl_t;

	typedef struct
	{
		uint8_t VBLANK : 1; // bit  0
		uint8_t LCD_STAT : 1; // bit  1
		uint8_t TIMER : 1; // bit  2
		uint8_t SERIAL : 1; // bit  3
		uint8_t JOYPAD : 1; // bit  4
		uint8_t NO_INT05 : 1; // bit  5	
		uint8_t NO_INT06 : 1; // bit  6
		uint8_t NO_INT07 : 1; // bit  7
	} interruptRequestFields_t;

	typedef union
	{
		interruptRequestFields_t interruptRequestFields;
		uint8_t interruptRequestMemory;
	} interruptRequest_t;

	typedef struct
	{
		uint8_t DIV_ZERO : 1; // bit  0
		uint8_t DIV_ONE : 1; // bit  1
		uint8_t DIV_TWO : 1; // bit  2
		uint8_t DIV_THREE : 1; // bit  3
		uint8_t DIV_FOUR : 1; // bit  4
		uint8_t DIV_FIVE : 1; // bit  5	
		uint8_t DIV_SIX : 1; // bit  6
		uint8_t DIV_SEVEN : 1; // bit  7
	} divFields_t;

	typedef union
	{
		divFields_t divFields;
		uint8_t divByte;
	} divByte_t;

	typedef struct
	{
		divByte_t DIV_LSB;			// FF03
		divByte_t DIV_MSB;			// FF04
	} divBytes_t;

	typedef union
	{
		divBytes_t divBytes;
		uint16_t divMemory;
	} div_t;

	typedef struct
	{
		uint8_t BG_WINDOW_LAYER_ENABLE : 1; // bit  0
		uint8_t OBJ_ENABLE : 1; // bit  1
		uint8_t OBJ_SIZE : 1; // bit  2
		uint8_t BG_TILE_MAP_AREA : 1; // bit  3
		uint8_t BG_WINDOW_TILE_DATA_AREA : 1; // bit  4
		uint8_t WINDOW_LAYER_ENABLE : 1; // bit  5	
		uint8_t WINDOW_TILE_MAP_AREA : 1; // bit  6
		uint8_t LCD_PPU_ENABLE : 1; // bit  7
	} lcdControlFields_t;

	typedef union
	{
		lcdControlFields_t lcdControlFields;
		uint8_t lcdControlMemory;
	} lcdControl_t;

	typedef struct
	{
		uint8_t MODE : 2; // bit  0 - 1
		uint8_t LYC_EQL_LY_FLAG : 1; // bit  2
		uint8_t MODE0_HBLANK_STAT_INT_SRC : 1; // bit  3
		uint8_t MODE1_VBLANK_STAT_INT_SRC : 1; // bit  4
		uint8_t MODE2_OAM_STAT_INT_SRC : 1; // bit  5	
		uint8_t LYC_EQL_LY_STAT_INT_SRC : 1; // bit  6
		uint8_t UNUSED_07 : 1; // bit  7
	} lcdStatusFields_t;

	typedef union
	{
		lcdStatusFields_t lcdStatusFields;
		uint8_t lcdStatusMemory;
	} lcdStatus_t;

	typedef struct
	{
		uint8_t P10_RIGHT_A : 1; // bit  0
		uint8_t P11_LEFT_B : 1; // bit  1
		uint8_t P12_UP_SELECT : 1; // bit  2	
		uint8_t P13_DOWN_START : 1; // bit  3
		uint8_t P14_SEL_DIRECTION_KEYS : 1; // bit  4
		uint8_t P15_SEL_ACTION_KEYS : 1; // bit  5	
		uint8_t JP_SPARE_06 : 1; // bit  6
		uint8_t JP_SPARE_07 : 1; // bit  7
	} joyPadFields_t;

	typedef union
	{
		joyPadFields_t joyPadFields;
		BYTE joyPadMemory;
	} joyPadMemory_t;

	typedef struct
	{
		uint8_t Address : 6; // bit  0 - 5
		uint8_t SPARE_06 : 1; // bit  6
		uint8_t AutoIncrement : 1; // bit  7
	} BCPSFields_t;

	typedef union
	{
		BCPSFields_t BCPSFields;
		BYTE BCPSMemory;
	} BCPSMemory_t;

	typedef struct
	{
		uint8_t Address : 6; // bit  0 - 5
		uint8_t SPARE_06 : 1; // bit  6
		uint8_t AutoIncrement : 1; // bit  7
	} OCPSFields_t;

	typedef union
	{
		BCPSFields_t OCPSFields;
		BYTE OCPSMemory;
	} OCPSMemory_t;

	typedef struct
	{
		uint8_t Reserved0 : 2; // bits  0 - 1
		uint8_t DMGCompatibility : 1; // bit  2
		uint8_t Reserved1 : 5; // bit  3 - 7
	} KEY0Fields_t;

	typedef union
	{
		KEY0Fields_t KEY0Fields;
		BYTE KEY0Memory;
	} KEY0Memory_t;

	typedef struct
	{
		uint8_t PrepareSpeedSwitch : 1; // bit  0
		uint8_t Reserved : 6; // bit  1 - 6
		uint8_t CurrentSpeed : 1; // bit  7
	} KEY1Fields_t;

	typedef union
	{
		KEY1Fields_t KEY1Fields;
		BYTE KEY1Memory;
	} KEY1Memory_t;

	typedef struct
	{
		uint8_t sweepSlopeControl : 3; // bit  0 - 2
		uint8_t sweepDirection : 1; // bit  3
		uint8_t sweepPace : 3; // bit  4 - 6
		uint8_t reserved : 1; // bit  7 
	} channelSweepFields_t;

	typedef union
	{
		channelSweepFields_t channelSweepFields;
		BYTE channelSweepMemory;
	} channelSweepMemory_t;

	typedef struct
	{
		uint8_t initialLengthTimer : 6; // bit  0 - 5
		uint8_t waveDuty : 2; // bit  6 - 7
	} channelLengthAndDutyFields_t;

	typedef union
	{
		channelLengthAndDutyFields_t channelLengthAndDutyFields;
		BYTE channelLengthAndDutyMemory;
	} channelLengthAndDutyMemory_t;

	typedef struct
	{
		uint8_t envelopeSweepPace : 3; // bit  0 - 2
		uint8_t envelopeDirection : 1; // bit  3
		uint8_t initialVolumeOfEnvelope : 4; // bit  4 - 7
	} channelVolumeAndEnvelopeFields_t;

	typedef union
	{
		channelVolumeAndEnvelopeFields_t channelVolumeAndEnvelopeFields;
		BYTE channelVolumeAndEnvelopeMemory;
	} channelVolumeAndEnvelopeMemory_t;

	typedef struct
	{
		BYTE lowerPeriodValue;
	} channelLowerPeriodMemory_t;

	typedef struct
	{
		uint8_t higherPeriodValue : 3; // bit  0 - 2
		uint8_t reserved : 3; // bit  3 - 5
		uint8_t soundLengthEnable : 1; // bit  6
		uint8_t trigger : 1; // bit  7
	} channelHigherPeriodAndControlFields_t;

	typedef union
	{
		channelHigherPeriodAndControlFields_t channelHigherPeriodAndControlFields;
		BYTE channelHigherPeriodAndControlMemory;
	} channelHigherPeriodAndControlMemory_t;

	typedef struct
	{
		uint8_t reserved : 7; // bit  0 - 6
		uint8_t dacEnable : 1; // bit  7
	} channelDACEnableFields_t;

	typedef union
	{
		channelDACEnableFields_t channelDACEnableFields;
		BYTE channelDACEnableMemory;
	} channelDACEnableMemory_t;

	typedef struct
	{
		uint8_t reserved00 : 5; // bit  0 - 4
		uint8_t outputLevelSelection : 2; // bit  5 - 6
		uint8_t reserved01 : 1; // bit  7
	} channelOutputLevelFields_t;

	typedef union
	{
		channelOutputLevelFields_t channelOutputLevelFields;
		BYTE channelOutputLevelMemory;
	} channelOutputLevelMemory_t;

	typedef struct
	{
		uint8_t lengthTimer : 6; // bit  0 - 5
		uint8_t reserved : 1; // bit  6 - 7
	} channelLengthTimerFields_t;

	typedef union
	{
		channelLengthTimerFields_t channelLengthTimerFields;
		BYTE lengthTimerMemory;
	} channelLengthTimerMemory_t;

	typedef struct
	{
		uint8_t clockDivider : 3; // bit  0 - 2
		uint8_t LFSRwidth : 1; // bit  3
		uint8_t clockShift : 4; // bit  4 - 7
	} channelFrequencyAndRandomnessFields_t;

	typedef union
	{
		channelFrequencyAndRandomnessFields_t channelFrequencyAndRandomnessFields;
		BYTE channelFrequencyAndRandomnessMemory;
	} channelFrequencyAndRandomnessMemory_t;

	typedef struct
	{
		uint8_t rightOutputVolume : 3; // bit  0 - 2
		uint8_t mixVINToRightOutput : 1; // bit  3
		uint8_t leftOutputVolume : 3; // bit  4 - 6
		uint8_t mixVINToLeftOutput : 1; // bit  7
	} channelMasterVolumeAndVINPanningFields_t;

	typedef union
	{
		channelMasterVolumeAndVINPanningFields_t channelMasterVolumeAndVINPanningFields;
		BYTE channelMasterVolumeAndVINPanningMemory;
	} channelMasterVolumeAndVINPanningMemory_t;

	typedef struct
	{
		uint8_t mixChannel1ToRightOutput : 1; // bit  0
		uint8_t mixChannel2ToRightOutput : 1; // bit  1
		uint8_t mixChannel3ToRightOutput : 1; // bit  2
		uint8_t mixChannel4ToRightOutput : 1; // bit  3
		uint8_t mixChannel1ToLeftOutput : 1; // bit  4
		uint8_t mixChannel2ToLeftOutput : 1; // bit  5
		uint8_t mixChannel3ToLeftOutput : 1; // bit  6
		uint8_t mixChannel4ToLeftOutput : 1; // bit  7
	} channelSoundPanningFields_t;

	typedef union
	{
		channelSoundPanningFields_t channelSoundPanningFields;
		BYTE channelSoundPanningMemory;
	} channelSoundPanningMemory_t;

	typedef struct
	{
		uint8_t channel1ONFlag : 1; // bit  0
		uint8_t channel2ONFlag : 1; // bit  1
		uint8_t channel3ONFlag : 1; // bit  2
		uint8_t channel4ONFlag : 1; // bit  3
		uint8_t reserved : 3; // bit  4 - 6
		uint8_t allChannelONOFFToggle : 1; // bit  7
	} channelSoundONOFFFields_t;

	typedef union
	{
		channelSoundONOFFFields_t channelSoundONOFFFields;
		BYTE channelSoundONOFFMemory;
	} channelSoundONOFFMemory_t;

	typedef struct
	{
		uint8_t lowerNibble : 4; // bits 0 - 3
		uint8_t upperNibble : 4; // bits 4 - 7
	} samples_t;

	typedef union
	{
		samples_t samples;
		uint8_t waveRamByte;
	} waveRamByte_t;

	typedef struct
	{
		joyPadMemory_t P1_JOYP;							// FF00
		uint8_t SB;										// FF01
		sc_t SC;										// FF02
		div_t DIV;										// FF03 - FF04
		uint8_t TIMA;									// FF05
		uint8_t TMA;									// FF06
		timerControl_t TAC;								// FF07
		uint8_t SPARE_01;								// FF08
		uint8_t SPARE_02;								// FF09
		uint8_t SPARE_03;								// FF0A
		uint8_t SPARE_04;								// FF0B
		uint8_t SPARE_05;								// FF0C
		uint8_t SPARE_06;								// FF0D
		uint8_t SPARE_07;								// FF0E
		interruptRequest_t IF;							// FF0F
		channelSweepMemory_t NR10;						// FF10
		channelLengthAndDutyMemory_t NR11;				// FF11
		channelVolumeAndEnvelopeMemory_t NR12;			// FF12
		channelLowerPeriodMemory_t NR13;				// FF13
		channelHigherPeriodAndControlMemory_t NR14;		// FF14
		uint8_t SPARE_08;								// FF15
		channelLengthAndDutyMemory_t NR21;				// FF16
		channelVolumeAndEnvelopeMemory_t NR22;			// FF17
		channelLowerPeriodMemory_t NR23;				// FF18
		channelHigherPeriodAndControlMemory_t NR24;		// FF19
		channelDACEnableMemory_t NR30;					// FF1A
		uint8_t NR31;									// FF1B
		channelOutputLevelMemory_t NR32;				// FF1C
		channelLowerPeriodMemory_t NR33;				// FF1D
		channelHigherPeriodAndControlMemory_t NR34;		// FF1E
		uint8_t SPARE_09;								// FF1F
		channelLengthTimerMemory_t NR41;				// FF20
		channelVolumeAndEnvelopeMemory_t NR42;			// FF21
		channelFrequencyAndRandomnessMemory_t NR43;		// FF22
		channelHigherPeriodAndControlMemory_t NR44;		// FF23
		channelMasterVolumeAndVINPanningMemory_t NR50;	// FF24
		channelSoundPanningMemory_t NR51;				// FF25
		channelSoundONOFFMemory_t NR52;					// FF26
		uint8_t SPARE_10;								// FF27
		uint8_t SPARE_11;								// FF28
		uint8_t SPARE_12;								// FF29
		uint8_t SPARE_13;								// FF2A
		uint8_t SPARE_14;								// FF2B
		uint8_t SPARE_15;								// FF2C
		uint8_t SPARE_16;								// FF2D
		uint8_t SPARE_17;								// FF2E
		uint8_t SPARE_18;								// FF2F
		waveRamByte_t waveRam[0x0010];					// FF30 - FF3F
		lcdControl_t LCDC;								// FF40
		lcdStatus_t STAT;								// FF41
		uint8_t SCY;									// FF42
		uint8_t SCX;									// FF43
		uint8_t LY;										// FF44
		uint8_t LYC;									// FF45	
		uint8_t DMA;									// FF46	
		uint8_t BGP;									// FF47	
		uint8_t OBP0;									// FF48	
		uint8_t OBP1;									// FF49	
		uint8_t WY;										// FF4A	
		uint8_t WX;										// FF4B	
		KEY0Memory_t KEY0;								// FF4C
		KEY1Memory_t KEY1;								// FF4D
		uint8_t SPARE_20;								// FF4E	
		uint8_t VBK;									// FF4F
		uint8_t BANK;									// FF50	
		uint8_t HDMA1;									// FF51	
		uint8_t HDMA2;									// FF52	
		uint8_t HDMA3;									// FF53	
		uint8_t HDMA4;									// FF54	
		uint8_t HDMA5;									// FF55	
		uint8_t RP;										// FF56
		uint8_t SPARE_22;								// FF57	
		uint8_t SPARE_23;								// FF58	
		uint8_t SPARE_24;								// FF59	
		uint8_t SPARE_25;								// FF5A	
		uint8_t SPARE_26;								// FF5B	
		uint8_t SPARE_27;								// FF5C
		uint8_t SPARE_28;								// FF5D
		uint8_t SPARE_29;								// FF5E	
		uint8_t SPARE_30;								// FF5F
		uint8_t SPARE_31;								// FF60	
		uint8_t SPARE_32;								// FF61	
		uint8_t SPARE_33;								// FF62	
		uint8_t SPARE_34;								// FF63	
		uint8_t SPARE_35;								// FF64	
		uint8_t SPARE_36;								// FF65	
		uint8_t SPARE_37;								// FF66
		uint8_t SPARE_38;								// FF67
		BCPSMemory_t BCPS_BGPI;							// FF68	
		uint8_t BCPD_BGPD;								// FF69	
		OCPSMemory_t OCPS_OBPI;							// FF6A	
		uint8_t OCPD_OBPD;								// FF6B	
		uint8_t OPRI;									// FF6C	
		uint8_t SPARE_39;								// FF6D
		uint8_t SPARE_40;								// FF6E	
		uint8_t SPARE_41;								// FF6F	
		uint8_t SVBK;									// FF70	
		uint8_t SPARE_42;								// FF71
		uint8_t SPARE_43;   							// FF72
		uint8_t SPARE_44;   							// FF73
		uint8_t SPARE_45;   							// FF74
		uint8_t SPARE_46;								// FF75	
		uint8_t PCM12;									// FF76		
		uint8_t PCM34;									// FF77	
		uint8_t SPARE_47;								// FF78
		uint8_t SPARE_48;   							// FF79
		uint8_t SPARE_49;   							// FF7A
		uint8_t SPARE_50;   							// FF7B
		uint8_t SPARE_51;								// FF7C	
		uint8_t SPARE_52;   							// FF7D
		uint8_t SPARE_53;								// FF7E	
		uint8_t SPARE_54;   							// FF7F
	} IOFields_t;

	typedef union
	{
		IOFields_t IOFields;
		uint8_t IOMemory[sizeof(IOFields_t)];
	} IOMemory_t;

	typedef struct
	{
		uint8_t highRam[0x007F];
	} highRamFields_t;

	typedef union
	{
		highRamFields_t highRamFields;
		uint8_t highRamMemory[sizeof(highRamFields_t)];
	} highRamMemory_t;

	typedef struct
	{
		uint8_t VBLANK : 1; // bit  0
		uint8_t LCD_STAT : 1; // bit  1
		uint8_t TIMER : 1; // bit  2
		uint8_t SERIAL : 1; // bit  3
		uint8_t JOYPAD : 1; // bit  4
		uint8_t NO_INT05 : 1; // bit  5	
		uint8_t NO_INT06 : 1; // bit  6
		uint8_t NO_INT07 : 1; // bit  7
	} interruptEnableFields_t;

	typedef union
	{
		interruptEnableFields_t interruptEnableFields;
		uint8_t interruptEnableMemory;
	} interruptEnable_t;

	typedef struct
	{
		codeRomMemory_t mCodeRom;
		videoRamMemory_t mVideoRam;
		externalRamMemory_t mExternalRam;
		workRamMemory_t mWorkRam;
		echoRamMemory_t mEchoRam;
		OAMMemory_t mOAM;
		BYTE mForbidden[0x0060];
		IOMemory_t mIO;
		highRamMemory_t mHighRam;
		interruptEnable_t mInterruptEnable;
	} memoryMap_t;

	typedef union
	{
		memoryMap_t GBcMemoryMap;
		BYTE GBcRawMemory[sizeof(memoryMap_t)];
	} GBcMemory_t;

	typedef struct
	{
		uint8_t DAYCOUNTER_MSB : 1; // bit  0
		uint8_t SPARE_1 : 1; // bit  1
		uint8_t SPARE_2 : 1; // bit  2	
		uint8_t SPARE_3 : 1; // bit  3
		uint8_t SPARE_4 : 1; // bit  4
		uint8_t SPARE_5 : 1; // bit  5	
		uint8_t DAYCOUNTER_HALT : 1; // bit  6
		uint8_t DAYCOUNTER_CARRY : 1; // bit  7
	} rtcDHFields_t;

	typedef union
	{
		rtcDHFields_t rtcDHFields;
		BYTE rtcDHMemory;
	} rtcDHMemory_t;

	typedef struct
	{
		BYTE rtc_S;
		BYTE rtc_M;
		BYTE rtc_H;
		BYTE rtc_DL;
		rtcDHMemory_t rtc_DH;
	} rtcFields_t;

	typedef union
	{
		rtcFields_t rtcFields;
		uint8_t rtcBuffer[sizeof(rtcFields_t)];
	} rtc_t;

	typedef struct
	{
		FLAG isChannelActuallyEnabled;
		int32_t lengthTimer;
		int32_t frequencyTimer;
		int32_t waveDutyPosition;
		int32_t envelopePeriodTimer;
		uint8_t currentVolume;
		FLAG sweepEnabled;
		int32_t shadowFrequency;
		int32_t sweepTimer;
		uint16_t LFSR;
		FLAG isVolumeEnvelopeStillDoingAutomaticUpdates;
	} audioChannelInstance_t;

	typedef struct
	{
		FLAG nextHalfWillNotClockLengthCounter;
		FLAG wasSweepDirectionNegativeAtleastOnceSinceLastTrigger;
		FLAG didChannel3ReadWaveRamPostTrigger;
		BYTE waveRamCurrentIndex;
		BYTE channel3OutputLevelAndShift;
		FLAG wasDivAPUUpdated;
		FLAG wasPowerCycled;
		int32_t div_apu;
		MAP8 dacEnMap;
		BYTE sampleReadByChannel1;
		BYTE sampleReadByChannel2;
		BYTE sampleReadByChannel3;
		BYTE sampleReadByChannel4;
		int32_t downSamplingRatioCounter;
		uint32_t accumulatedTone;
		audioChannelInstance_t audioChannelInstance[(uint8_t)AUDIO_CHANNELS::TOTAL_CHANNELS];
		GBC_AUDIO_SAMPLE_TYPE audioBuffer[AUDIO_BUFFER_SIZE_FOR_GB_GBC];
		float emulatorVolume;
	} audio_t;

	// Since we are going to sort the OAM objects w.r.t "x", using linked list instead of array
	struct visibleObjects_t
	{
		FLAG alreadyProcessed;
		OAMEntry_t oamEntry;
		int32_t indexWithinOAMMemory;
		struct visibleObjects_t* next;
	};

	typedef struct
	{
		uint8_t BG_PALETTE_NUMBER : 3; // bit  0 - 2	
		uint8_t BG_TILE_VRAM_BANK_NUMBER : 1; // bit  3
		uint8_t BG_SPARE_04 : 1; // bit  4
		uint8_t BG_XFLIP : 1; // bit  5	
		uint8_t BG_YFLIP : 1; // bit  6
		uint8_t BG_to_OAM_Priority : 1; // bit  7
	} bgMapAttributesFields_t;

	struct pixelFIFOEntity_t
	{
		int8_t color;
		int8_t palette;
		int8_t spritePriority;
		int8_t backgroundPriority;
		int8_t validity;

		pixelFIFOEntity_t() : color(ZERO), palette(ZERO), spritePriority(ZERO), backgroundPriority(ZERO), validity(INVALID)
		{
			this->color = ZERO;
			this->palette = ZERO;
			this->spritePriority = ZERO;
			this->backgroundPriority = ZERO;
			this->validity = INVALID;
		}
	};

	struct pixelFIFO_t
	{
		struct pixelFIFOEntity_t pEntities[PIXEL_FIFO_SIZE_FOR_GB_GBC];
		BYTE numberOfEntities;

		pixelFIFO_t() : numberOfEntities(ZERO)
		{
			pixelFIFOEntity_t dummy;
			dummy.validity = INVALID;

			for (BYTE ii = ZERO; ii < PIXEL_FIFO_SIZE_FOR_GB_GBC; ii++)
			{
				pEntities[ii] = dummy;
			}
		}

		FLAG isEmpty()
		{
			RETURN (numberOfEntities <= ZERO);
		}

		FLAG isFull()
		{
			RETURN (numberOfEntities >= PIXEL_FIFO_SIZE_FOR_GB_GBC);
		}

		FLAG needsFilling()
		{
			RETURN (numberOfEntities <= (PIXEL_FIFO_SIZE_FOR_GB_GBC - EIGHT));
		}

		FLAG pop(struct pixelFIFOEntity_t* pEntity)
		{
			if (isEmpty() == YES)
			{
				RETURN FAILURE;
			}

			// Is not empty, but still first entry is invalid !
			if (pEntities[ZERO].validity == INVALID)
			{
				FATAL("Trying to pop an invalid pixel entry");
				RETURN FAILURE;
			}

			*pEntity = pEntities[ZERO];

			numberOfEntities -= ONE; // 1 old entry was popped out of fifo

			for (BYTE ii = ZERO; ii < (PIXEL_FIFO_SIZE_FOR_GB_GBC - ONE); ii++)
			{
				pEntities[ii] = pEntities[ii + ONE];
			}

			struct pixelFIFOEntity_t dummy;
			dummy.validity = INVALID;

			pEntities[PIXEL_FIFO_SIZE_FOR_GB_GBC - ONE] = dummy;

			RETURN SUCCESS;
		}

		FLAG pop(void)
		{
			struct pixelFIFOEntity_t dummy;
			RETURN pop(&dummy);
		}

		FLAG push(struct pixelFIFOEntity_t pEntity[EIGHT], uint8_t validEntryCount, DIM8 actualFIFOSize)
		{
			if (actualFIFOSize == EIGHT)
			{
				if (isEmpty() == NO)
				{
					RETURN FAILURE;
				}
			}
			else if (actualFIFOSize == SIXTEEN)
			{
				if (needsFilling() == NO)
				{
					RETURN FAILURE;
				}
			}

			BYTE jj = ZERO;
			auto startIdx = numberOfEntities;
			auto endIdx = numberOfEntities + validEntryCount;
			for (BYTE ii = startIdx; ii < endIdx; ii++)
			{
				if (pEntity[jj].validity != INVALID)
				{
					pEntities[ii] = pEntity[jj];
					++jj;
					++numberOfEntities;
				}
			}

			RETURN SUCCESS;
		}

		struct pixelFIFOEntity_t* referenceElement(BYTE index)
		{
			if (index > (numberOfEntities - ONE))
			{
				RETURN NULL;
			}

			RETURN &(pEntities[index]);
		}

		FLAG insertValidElementAt(BYTE index, struct pixelFIFOEntity_t pEntity)
		{
			if (isFull() == YES)
			{
				RETURN FAILURE;
			}

			if (pEntity.validity == INVALID)
			{
				RETURN FAILURE;
			}

			if (index > (numberOfEntities - ONE))
			{
				numberOfEntities++;
			}

			pEntities[index] = pEntity;

			RETURN SUCCESS;
		}

		void clearFIFO()
		{
			numberOfEntities = ZERO;

			struct pixelFIFOEntity_t dummy;
			dummy.backgroundPriority = 0;
			dummy.color = 0;
			dummy.palette = 0;
			dummy.spritePriority = 0;
			dummy.validity = INVALID;

			for (BYTE ii = ZERO; ii < PIXEL_FIFO_SIZE_FOR_GB_GBC; ii++)
			{
				pEntities[ii] = dummy;
			}
		}
	};

	typedef union
	{
		bgMapAttributesFields_t bgMapAttributesFields;
		BYTE bgMapAttributesMemory;
	} bgMapAttributes_t;

	struct pixelFetcherContext_t
	{
		BYTE bgWinTileID;
		BYTE bgWinTileDataLo;
		BYTE bgWinTileDataHi;
		BYTE objTileID;
		BYTE objTileDataLo;
		BYTE objTileDataHi;
		bgMapAttributes_t bgAttribute;
		struct
		{
			pixelFIFOEntity_t cachedFifo[EIGHT];
			BYTE validEntries;
		} cachedFifo_bg_win;
		struct
		{
			pixelFIFOEntity_t cachedFifo[EIGHT];
			BYTE validEntries;
		} cachedFifo_obj;
		void clear_cachedFifo_bg_win()
		{
			for (BYTE ii = 0; ii < EIGHT; ii++)
			{
				cachedFifo_bg_win.cachedFifo[ii].backgroundPriority = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].color = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].palette = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].spritePriority = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].validity = INVALID;
				cachedFifo_bg_win.validEntries = RESET;
			}
		}
		void clear_cachedFifo_obj()
		{
			for (BYTE ii = 0; ii < EIGHT; ii++)
			{
				cachedFifo_obj.cachedFifo[ii].backgroundPriority = ZERO;
				cachedFifo_obj.cachedFifo[ii].color = ZERO;
				cachedFifo_obj.cachedFifo[ii].palette = ZERO;
				cachedFifo_obj.cachedFifo[ii].spritePriority = ZERO;
				cachedFifo_obj.cachedFifo[ii].validity = INVALID;
				cachedFifo_obj.validEntries = RESET;
			}
		}
		void clearAllCachedFifos()
		{
			for (BYTE ii = 0; ii < EIGHT; ii++)
			{
				cachedFifo_bg_win.cachedFifo[ii].backgroundPriority = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].color = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].palette = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].spritePriority = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].validity = INVALID;
				cachedFifo_bg_win.validEntries = RESET;
				cachedFifo_obj.cachedFifo[ii].backgroundPriority = ZERO;
				cachedFifo_obj.cachedFifo[ii].color = ZERO;
				cachedFifo_obj.cachedFifo[ii].palette = ZERO;
				cachedFifo_obj.cachedFifo[ii].spritePriority = ZERO;
				cachedFifo_obj.cachedFifo[ii].validity = INVALID;
				cachedFifo_obj.validEntries = RESET;
			}
		}
		void reset()
		{
			bgWinTileID = RESET;
			bgWinTileDataLo = RESET;
			bgWinTileDataHi = RESET;
			objTileID = RESET;
			objTileDataLo = RESET;
			objTileDataHi = RESET;
			bgAttribute.bgMapAttributesMemory = RESET;
			clearAllCachedFifos();
		}
	};

	typedef struct
	{
		uint16_t fakeBgFetcherRuns;
		LCD_MODES currentLCDMode;
		LCD_MODES currentSpecialLCDMode;
		uint8_t currentScanline;
		uint16_t tickAtMode3ToMode0;
		FLAG lcdJustEn;
		FLAG skipMode2;
		uint16_t latchedSCYForGBC;
		uint16_t windowLineCounter;
		FLAG wasVblankJustTriggerred;
		FLAG yConditionForWindowIsMetForCurrentFrame;
		FLAG shouldFetchAndRenderWindowInsteadOfBG;
		FLAG shouldIncrementWindowLineCounter;
		FLAG shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile;
		FLAG waitForNextLineForWindSyncGlitch;
		FLAG performWindSyncGlitch;
		FLAG gfxOfCurrentScanLineUpdated;
		FLAG isNewM3Scanline;
		int16_t oamSearchCount;
		int16_t spriteCountPerScanLine;
		FLAG shouldSimulateBGScrollingPenaltyNow;
		PIXEL_FETCHER_STATES pixelFetcherState;
		struct pixelFetcherContext_t pixelFetcherContext;
		struct pixelFIFO_t bgWinPixelFIFO;
		struct pixelFIFO_t objPixelFIFO;
		struct pixelFIFO_t tempBgWinPixelFIFO;
		int16_t discardedPixelCount;
		BYTE xBGPerPixel;
		COUNTER8 cgbSCYDelayTCycles;
		BYTE cgbLatchedSCY;
		int16_t pixelFetcherDots;
		int16_t pixelRendererDots;
		int16_t pixelPipelineDots;
		int16_t pixelFetcherCounterPerScanLine;
		int16_t pixelRenderCounterPerScanLine;
		FLAG x159SpritesPresent;
		BYTE nX159SpritesPresent;
		FLAG x159SpritesDone;
		FLAG shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone;
		FLAG shouldFetchObjInsteadOfWinAndBgNow;
		FLAG isThereAnyObjectCurrentlyGettingRendered;
		int16_t indexOfOBJToFetchFromVisibleSpritesArray;
		visibleObjects_t* visibleObjectsPerScanLine;			// Linked List for the visible sprites per scanline
		visibleObjects_t arrayOfVisibleObjectsPerScanLine[TEN];	// Memory for the visibleObjectsPerScanLine
		FLAG wasFetchingOBJ;
		int16_t prevSpriteX;
		FLAG wasNotFirstSpriteInX;
		FLAG wasX0Object;
		uint16_t addressInTileMapArea;
		uint16_t addressInTileDataArea;
		COUNTER8 tileSelGlitchTCycles;
		BYTE tileSelGlitchedData;
		BYTE cached_BG_WINDOW_TILE_DATA_AREA;
		BYTE cached_BG_TILE_VRAM_BANK_NUMBER;
		FLAG fetchDone;
		FLAG pushDone;
		FLAG bgToObjectPenalty;
		FLAG isTheLastVblankLine;
		FLAG blockVramR;
		FLAG blockOAMR;
		FLAG blockVramW;
		FLAG blockOAMW;
		FLAG blockCGBPalette;
		int16_t emulatedPPUCyclePerPPUMode;
		uint16_t gfxVisibleColorMap_BG_WINDOW_OBJ[screen_height][screen_width];
		COLOR_FORMAT gfxVisible_BG_WINDOW_OBJ[screen_height][screen_width];
		union
		{
			Pixel imGuiBuffer1D[screen_width * screen_height];
			Pixel imGuiBuffer2D[screen_height][screen_width];
		} imGuiBuffer;
		COLOR_FORMAT gfx_BG_WINDOW[256][256];
		COLOR_FORMAT imGuiFullBuffer2D[256][256];
		uint64_t filters;
		uint64_t debugVariable;
	} display_t;

	typedef struct
	{
		uint8_t bg_colorID : 2; // bit  0 - 1
		uint8_t bg_has_priority_for_cgb : 1; // bit  2
		uint8_t this_x_coordinate_is_already_populated_for_dmg : 1; // bit  3	
		uint8_t this_x_coordinate_is_already_populated_for_cgb : 1; // bit  4
		uint8_t spare05 : 1; // bit  5
		uint8_t spare06 : 1; // bit  6	
		uint8_t spare07 : 1; // bit  7
	} colorCacheFields_t;

	typedef union
	{
		colorCacheFields_t colorCacheFields;
		BYTE colorCacheMemory;
	} colorCacheMemory_t;

	typedef struct
	{
		uint64_t apuCounter;
		uint64_t cpuCounter;
		FLAG isValidTickForDoubleSpeed;
		uint16_t dividerCounter;
		uint16_t serialCounter;
		uint16_t ppuCounterPerLY;
		uint16_t ppuCounterPerMode;
		uint32_t ppuCounterPerFrame;
		uint64_t rtcCounter;
		uint16_t rtcDayCounter;
		uint16_t timerCounter;
	} ticks_t;

	typedef struct
	{
		uint8_t HBLANK_SIGNAL : ONE; // bits 0
		uint8_t VBLANK_SIGNAL : ONE; // bits 1
		uint8_t OAM_SIGNAL : ONE; // bits 3
		uint8_t LY_LYC_SIGNAL : ONE; // bits 
		uint8_t UNUSED0 : ONE; // bits 5
		uint8_t UNUSED1 : ONE; // bits 6
		uint8_t UNUSED2 : ONE; // bits 7
		uint8_t UNUSED3 : ONE; // bits 8
	} STATInterruptSources_t;

	typedef union
	{
		STATInterruptSources_t STATInterruptSources;
		BYTE aggregateSignal;
	} STATInterruptSignal_t;

	struct debugger_t
	{
		FLAG wasDebuggerJustTriggerred;
		int64_t debuggerTriggerOnWhichLY;
		int64_t lyChangePersistance;
		struct
		{
			uint32_t testCount[TWOFIFTYSIX];
			uint32_t cbtestCount[TWOFIFTYSIX];
			struct
			{
				uint32_t indexer;
				struct
				{
					FLAG isRead;
					FLAG isWrite;
					uint8_t data;
					uint16_t address;

					void reset()
					{
						isRead = CLEAR;
						isWrite = CLEAR;
						data = RESET;
						address = RESET;
					};
				} cycles[TWENTY];
			} cycles;
		} tomHarte;
	};

	typedef enum
	{
		HBLANK,
		VBLANK,
		OAM,
		LY_LYC,
		NONE
	} STAT_INTR_SRC;

	typedef struct
	{
		STAT_INTR_SRC STAT_src;
		STATInterruptSignal_t STATInterruptSignal;
		FLAG checkSTATTPlusOneCycle;
		byte prevSTAT;
		byte newSTAT;
		byte dataWrittenToMBCReg0;
		byte dataWrittenToMBCReg1;
		byte dataWrittenToMBCReg2;
		byte dataWrittenToMBCReg3;
		byte dataWrittenToMBCReg4;
		FLAG is_mbc2_rom_mode;
		FLAG no_mbc;
		FLAG mbc1;
		FLAG mbc2;
		FLAG mbc3;
		FLAG mbc5;
		FLAG isMBC1_Mode1;
		FLAG romBank32K;
		FLAG romBank64K;
		FLAG romBank128K;
		FLAG romBank256K;
		FLAG romBank512K;
		FLAG romBank1M;
		FLAG romBank2M;
		FLAG romBank4M;
		FLAG romBank8M;
		FLAG romBank1_1M;
		FLAG romBank1_2M;
		FLAG romBank1_5M;
		union
		{
			struct
			{
				uint16_t mbcBank1Reg : FIVE; // bits 0 - 4
				uint16_t mbcBank2Reg : TWO;	// bits 5 - 6
				uint16_t pad : NINE; // bits 7 - 16
			} mbc1Fields;
			uint16_t raw;
		} currentROMBankNumber;
		FLAG no_ramBank;
		FLAG ramBank2K;
		FLAG ramBank8K;
		FLAG ramBank32K;
		FLAG ramBank128K;
		FLAG ramBank64K;
		FLAG enableRAMBanking;
		uint8_t currentRAMBankNumber;
		uint8_t currentVRAMBankNumber;
		uint8_t currentWRAMBankNumber;
		uint16_t serialMaxClockPerTransfer;
		uint16_t serialMasterByteShiftCount;
		uint16_t serialSlaveByteShiftCount;
		FLAG isRTCAvailableInMBC3;
		FLAG enableRTCAccessTimer;
		FLAG mapRTCRegisters;
		uint8_t currentRTCRegister;
		BYTE rtcFsm;
		FLAG keyUP;
		FLAG keyDOWN;
		FLAG keyLEFT;
		FLAG keyRIGHT;
		FLAG keySTART;
		FLAG keySELECT;
		FLAG keyA;
		FLAG keyB;
		FLAG isNewTimerCycle;
		SIGNAL timaIncSignal;
		SIGNAL fallingEdgeDetectorDelay;
		FLAG instantTimerIF;
		FLAG waitingToRequestTimerInterrupt;
		int16_t clocksAfterTIMAOverflow;
		FLAG isCPUHalted;
		FLAG isCPUJustHalted;
		FLAG isCPUStopped;
		FLAG freezeLCD;
		FLAG freezeLCDOneFrame;
		int32_t exitHaltInTCycles;
		enum HALT_BUG_STATE isHaltBugActivated;
		enum EI_ENABLE_STATE eiEnState;
		FLAG interruptMasterEn;	// IME
		FLAG gfxEn;
		FLAG isCPUExecutionBlocked;
		FLAG isDMAActive;
		uint16_t DMAStartDelay;
		uint16_t DMASource;
		uint16_t DMABytesTransferred;
		FLAG DMARestarted;
		uint16_t DMAEndDelayUponRestart;
		FLAG DMASTATGlitchEn;
		enum CGB_DMA_MODE cgbDMAMode;
		FLAG isHDMAActive;
		FLAG isHDMAAllowedToBlockCPUPipeline;
		uint16_t hDMASource;
		uint16_t hDMADestination;
		int16_t hDMATXLength;
		uint16_t hDMABytesTransferred;
		FLAG isGPDMAActive;
		FLAG isCGBDoubleSpeedMode;
		uint32_t checksum;
		uint64_t unusableMemoryReads;
		uint64_t unusableMemoryWrites;
		ticks_t ticks;
		debugger_t debugger;
	} emulatorStatus_t;

	// Data stored in all the ROM memory banks of the cartridge
	typedef struct
	{
		uint8_t mROMBanks[0x200][0x4000];
	} romMemoryBanks_t;

	typedef union
	{
		romMemoryBanks_t romMemoryBanks;
		uint8_t entireRomMemory[sizeof(romMemoryBanks_t)];
	} entireRom_t;

	// Data stored in all the RAM memory banks of the cartridge
	typedef struct
	{
		uint8_t mRAMBanks[0x80][0x2000];
	} ramMemoryBanks_t;

	typedef union
	{
		ramMemoryBanks_t ramMemoryBanks;
		uint8_t entireRamMemory[sizeof(ramMemoryBanks_t)];
	} entireRam_t;

	// Data stored in all the VRAM memory banks in CGB mode
	typedef struct
	{
		uint8_t mVRAMBanks[0x02][0x2000];
	} vramMemoryBanks_t;

	typedef union
	{
		vramMemoryBanks_t vramMemoryBanks;
#if DISABLED
		bgMapAttributes_t entireVramMemoryFields[sizeof(vramMemoryBanks_t)];
#endif
		uint8_t entireVramMemory[sizeof(vramMemoryBanks_t)];
	} entireVram_t;

	// Data stored in all the WRAM memory banks in CGB mode
	typedef struct
	{
		uint8_t mWRAM01Banks[0x07][0x1000];
	} wram01MemoryBanks_t;

	typedef union
	{
		wram01MemoryBanks_t wram01MemoryBanks;
		uint8_t entireWram01Memory[sizeof(wram01MemoryBanks_t)];
	} entireWram01_t;

	// Palette ram of size 64 bytes for background and object 
	// Palette ram's data granularity is 16 bit; https://gbdev.io/pandocs/Palettes.html#ff69--bcpdbgpd-cgb-mode-only-background-color-palette-data--background-palette-data

	typedef struct
	{
		uint8_t LOWER_BYTE : 8; // bit 0 - 7	
		uint8_t HIGHER_BYTE : 8; // bit 7 - 15
	} gbcColorByteFields_t;

	typedef union
	{
		gbcColorByteFields_t gbcColorByteFields;
		uint16_t gbcColor;
	} gbcColor_t;

	typedef union
	{
		gbcColor_t paletteRAM[EIGHT][FOUR];
		uint8_t paletteRAMMemory[sizeof(gbcColor_t) * EIGHT * FOUR];
	} entireBackgroundPaletteRAM_t;

	typedef union
	{
		gbcColor_t paletteRAM[EIGHT][FOUR];
		uint8_t paletteRAMMemory[sizeof(gbcColor_t) * EIGHT * FOUR];
	} entireObjectPaletteRAM_t;

	typedef struct
	{
		// core
		registers_t registers;
		cpu_t cpuInstance;
		//
		GBcMemory_t GBcMemory;
		//
		rtc_t rtcLatched;
		rtc_t rtc;
		//
		entireRom_t entireRom;
		entireRam_t entireRam;
		entireVram_t entireVram;
		entireWram01_t entireWram01;
		entireBackgroundPaletteRAM_t entireBackgroundPaletteRAM;
		entireObjectPaletteRAM_t entireObjectPaletteRAM;
		// semi - core
		display_t display;
		audio_t audio;
		// non - core
		quirks_t quirks;
		PALETTE_ID gb_palette;
		PALETTE_ID gbc_palette; // Used to handle GBC color correction
		emulatorStatus_t emulatorStatus;
	} GBc_state_t;

	typedef union
	{
		GBc_state_t GBc_state;
		uint8_t GBc_memoryState[sizeof(GBc_state_t)];
	} GBc_instance_t;

	typedef struct
	{
		FLAG isRomLoaded;
		uint32_t codeRomSize;
	} aboutRom_t;

	typedef struct
	{
		GBc_instance_t GBc_instance;
		aboutRom_t aboutRom;
	} absolute_GBc_state_t;

	union absolute_GBc_instance_t
	{
		absolute_GBc_state_t absolute_GBc_state;
		uint8_t GBc_absoluteMemoryState[sizeof(absolute_GBc_state_t)];
		absolute_GBc_instance_t()
		{
			memset(this, ZERO, sizeof(absolute_GBc_instance_t));
		}
	};

	std::shared_ptr <absolute_GBc_instance_t> pAbsolute_GBc_instance;
	GBc_instance_t* pGBc_instance = nullptr;			// for readability
	registers_t* pGBc_registers = nullptr;				// for readability
	cpu_t* pGBc_cpuInstance = nullptr;					// for readability
	GBcMemory_t* pGBc_memory = nullptr;					// for readability
	flagFields_t* pGBc_flags = nullptr;					// for readability
	IOFields_t* pGBc_peripherals = nullptr;				// for readability
	emulatorStatus_t* pGBc_emuStatus = nullptr;			// for readability
	display_t* pGBc_display = nullptr;			// for readability

#pragma pack(pop)

private:

	SDL_AudioStream* audioStream = nullptr;

private:

	// TODO : Placeholder to handle network

private:

	std::deque<GBc_state_t> gamePlay;

private:

	static const uint16_t TYPE_BG_WIN = (ZERO << FIFTEEN);
	static const uint16_t TYPE_OBJ = (ONE << FIFTEEN);
	uint16_t mapPalette[8192 /* TODO: Calculate this based on VRAM instead of using magic numbers */] = {ZERO}; // MSB == 1 (OBJ) ; MSB == 0 (BG)
	std::set<BYTE> visibleOamIndexPerLY;
	std::set<BYTE> visibleOamIndexPerFrame;
	//std::map<uint16_t, std::string> mapAsm;

#pragma endregion EMULATION_DECLARATIONS

#pragma region BESS
private:
#pragma pack(push, 1)
	// BESS specifications
	// Refer to https://github.com/LIJI32/SameBoy/blob/master/BESS.md

	enum class BESS_BLOCKS
	{
		BESS_NAME,
		BESS_INFO,
		BESS_CORE,
		BESS_XOAM,
		BESS_MBC,
		BESS_RTC,
		BESS_END,
		BESS_HEADER,
		BESS_FOOTER,
		BESS_TOTAL
	};

	struct BESS_FOOTER_t
	{
		uint32_t off_blk_0;								// 4 bytes
		char ascii_tag[0x04];							// 4 bytes
	};

	struct BESS_BLOCK_HEADER_t
	{
		char ascii_ident[0x04];							// 4 bytes
		uint32_t blk_len;								// 4 bytes
	};

	struct BESS_BLOCK_NAME_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		char name_ver[0x20];							// 32 bytes
	};

	struct BESS_BLOCK_INFO_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		char title[0x10];								// 16 bytes
		uint16_t chksum;								// 2 bytes
	};

	struct BESS_BLOCK_CORE_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		uint16_t maj_bess_ver;							// 2 bytes		-> offset 0x00
		uint16_t min_bess_ver;							// 2 bytes		-> offset 0x02
		char mdl_indent[0x04];							// 4 bytes		-> offset 0x04
		uint16_t pc;									// 2 bytes		-> offset 0x08
		uint16_t af;									// 2 bytes		-> offset 0x0A
		uint16_t bc;									// 2 bytes		-> offset 0x0C
		uint16_t de;									// 2 bytes		-> offset 0x0E
		uint16_t hl;									// 2 bytes		-> offset 0x10
		uint16_t sp;									// 2 bytes		-> offset 0x12
		uint8_t ime;									// 1 byte		-> offset 0x14
		uint8_t ie;										// 1 byte		-> offset 0x15
		uint8_t exec_state;								// 1 byte		-> offset 0x16
		uint8_t rsv;									// 1 byte		-> offset 0x17
		uint8_t mmr[0x80];								// 128 bytes	-> offset 0x18
		uint32_t size_ram;								// 4 bytes		-> offset 0x98
		uint32_t off_ram;								// 4 bytes		-> offset 0x9C
		uint32_t size_vram;								// 4 bytes		-> offset 0xA0
		uint32_t off_vram;								// 4 bytes		-> offset 0xA4
		uint32_t size_mbcram;							// 4 bytes		-> offset 0xA8
		uint32_t off_mbcram;							// 4 bytes		-> offset 0xAC
		uint32_t size_oam;								// 4 bytes		-> offset 0xB0
		uint32_t off_oam;								// 4 bytes		-> offset 0xB4
		uint32_t size_hram;								// 4 bytes		-> offset 0xB8
		uint32_t off_hram;								// 4 bytes		-> offset 0xBC
		uint32_t size_obj_pram;							// 4 bytes		-> offset 0xC0
		uint32_t off_bg_pram;							// 4 bytes		-> offset 0xC4
		uint32_t size_bg_pram;							// 4 bytes		-> offset 0xC8
		uint32_t off_obj_pram;							// 4 bytes		-> offset 0xCC
	};

	struct BESS_BLOCK_XOAM_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		uint8_t xoam[0x60];								// 96 bytes
	};

#pragma warning(disable: 4200)
	struct BESS_BLOCK_MBC_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		uint8_t mbc[];									// variable bytes
	};
#pragma warning(default: 4200)

	struct BESS_BLOCK_RTC_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		uint8_t curr_sec;								// 1 byte
		uint8_t pad0[0x03];								// 3 bytes
		uint8_t curr_min;								// 1 byte
		uint8_t pad1[0x03];								// 3 bytes
		uint8_t curr_hr;								// 1 byte
		uint8_t pad2[0x03];								// 3 bytes
		uint8_t curr_day;								// 1 byte
		uint8_t pad3[0x03];								// 3 bytes
		uint8_t curr_ovf;								// 1 byte
		uint8_t pad4[0x03];								// 3 bytes
		uint8_t latched_sec;							// 1 byte
		uint8_t pad5[0x03];								// 3 bytes
		uint8_t latched_min;							// 1 byte
		uint8_t pad6[0x03];								// 3 bytes
		uint8_t latched_hr;								// 1 byte
		uint8_t pad7[0x03];								// 3 bytes
		uint8_t latched_day;							// 1 byte
		uint8_t pad8[0x03];								// 3 bytes
		uint8_t latched_ovf;							// 1 byte
		uint8_t pad9[0x03];								// 3 bytes
		uint64_t unix_time;								// 8 bytes
	};

	// TODO: BESS_BLOCK_HUC3_t
	// TODO: BESS_BLOCK_TPP1_t
	// TODO: BESS_BLOCK_MBC7_t
	// TODO: BESS_BLOCK_SGB_t

	struct BESS_BLOCK_END_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
	};

	// Internal BESS blocks

	struct BESS_BLOCKS_PRESENT_t
	{

	};

#pragma pack(pop)
#pragma endregion BESS

#pragma region INFRASTRUCTURE_METHOD_DECLARATION
public:

	GBc_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config, CheatEngine_t* ce);
	void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* network = nullptr) override;
	void sendBiosToEmulator(bios_t* bios = nullptr) override {};

public:

	float getVersion();
	MASQ_INLINE uint32_t getScreenWidth() override
	{
		RETURN this->screen_width;
	}
	MASQ_INLINE uint32_t getScreenHeight() override
	{
		RETURN this->screen_height;
	}
	MASQ_INLINE uint32_t getPixelWidth() override
	{
		RETURN this->pixel_width;
	}
	MASQ_INLINE uint32_t getPixelHeight() override
	{
		RETURN this->pixel_height;
	}
	void setEmulationWindowOffsets(uint32_t x, uint32_t y, FLAG isEnabled);
	uint32_t getTotalScreenWidth() override;
	uint32_t getTotalScreenHeight() override;
	uint32_t getTotalPixelWidth() override;
	uint32_t getTotalPixelHeight() override;
	void setScreenWidth(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setScreenHeight(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setPixelWidth(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setPixelHeight(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setTotalScreenWidth(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setTotalScreenHeight(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setTotalPixelWidth(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setTotalPixelHeight(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	const char* getEmulatorName() override;
	float getEmulationFPS() override;
	void setEmulationID(EMULATION_ID ID) override;
	EMULATION_ID getEmulationID() override;

public:

	FLAG getRomLoadedStatus() override;
	void randomizeRAM();
	FLAG loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) override;
	void dumpRom() override;

public:

	void blarggConsoleOutput();

public:

	//std::map<uint16_t, std::string> disassemble(uint16_t nStart, uint16_t nStop);

	void dumpCPURegisters(int x, int y, FLAG dumpCPU);
	void dumpCode(int x, int y, int nLines, FLAG dumpCode);
	void dumpGFXData(int x1, int y1, FLAG dumpVRAM, int x2, int y2, FLAG dumpPalette, int x3, int y3, FLAG dumpOAM, int x4, int y4, FLAG dumpBG, int x5, int y5, FLAG dumpInfo, FLAG hoverCheck);
	void dumpCartInfo(int x5, int y5, FLAG dumpCartInfo);
	void optimizedClearScreen(FLAG shouldPerform);
	void runDebugger();
#pragma endregion INFRASTRUCTURE_METHOD_DECLARATION

#pragma region EMULATION_METHOD_DECLARATION
private:

	const char* cartridgeLicName();
	const char* cartridgeTypeName();

private:

	FLAG isCGBDoubleSpeedEnabled();
	void toggleCGBSpeedMode();
	FLAG isCGBCompatibilityModeEnabled();

private:

	FLAG isRTCAvailableInMBC3();
	void enableRTCAccess();
	void disableRTCAccess();
	FLAG isRTCAccessEnabled();

	void setRTCFSM(uint8_t fsmState);
	uint8_t getRTCFSM();

	uint8_t getRTCRegisterNumber();
	void shouldMapRTCToExternalRAM(FLAG shouldMapRTC);
	FLAG isRTCMappedToExternalRAM();
	void setRTCRegisterNumber(uint8_t rtcRegisterNumber);
	int readFromRTCRegisterIfApplicable();
	uint16_t getRTCDayCounter();
	void setRTCDayCounter(uint16_t dayCounterValue);
	void writeToRTCRegisterIfApplicable(uint8_t data);

	void latchRTCRegisters();

private:

	void setMBCType(uint16_t mbcType);
	void setROMBankType(uint16_t romBankType);
	void setROMBankNumber(uint16_t romBankNumber);
	uint16_t getROMBankNumber();
	uint16_t getNumberOfROMBanksUsed();
	void setROMModeIfMBC1();
	void setRAMModeInMBC1();
	FLAG getROMOrRAMModeInMBC1();

	void enableRAMBank();
	void disableRAMBank();
	FLAG isRAMBankEnabled();
	void setRAMBankType(uint16_t ramBankType);
	uint8_t getRAMBankNumber();
	void setRAMBankNumber(uint8_t ramBankNumber);

	uint8_t getNumberOfRAMBanksUsed();
	uint8_t getVRAMBankNumber();
	void setVRAMBankNumber(uint8_t vramBankNumber);
	uint8_t getWRAMBankNumber();
	void setWRAMBankNumber(uint8_t wramBankNumber);

public:

	BYTE getGBDividerMSB();
	BYTE getGBDividerLSB();
	void setGBDividerMSB(BYTE value);
	void setGBDividerLSB(BYTE value);
	BIT getDIVSpecialBitStatus(TIMERS timer);

	SIGNAL getTIMASignalForGB();
	BYTE getGBTimer();
	void setGBTimer(BYTE value);
	void resetGBTimerToZero();
	void resetGBTimer(uint8_t resetVal);
	TIMERS getWhichGBTimerToUse();

public:

	void processDMA();
	void processGPDMA();
	void processHDMA();

public:

	void captureIO();

public:

	void processSerialClockSpeedBit();
	FLAG sendOverSerialLink(BIT bitToSend);
	FLAG receiveOverSerialLink(BIT* bitReceived, FLAG* rxStatus, FLAG isBlocking = NO, INC32 timeoutInUs = ONE);

public:

	void resetDivAPU(uint32_t value);
	void incrementDivAPU(uint32_t nCycles);
	DIM16 getChannelPeriod(AUDIO_CHANNELS channel);
	FLAG enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS channel);
	void continousDACCheck();
	FLAG isDACEnabled(AUDIO_CHANNELS channel);
	FLAG isChannel3Active();
	void tickChannel(AUDIO_CHANNELS channel, uint32_t tCycles);
	void processSoundLength();
	SDIM32 getUpdatedFrequency();
	void processFrequencySweep();
	void processEnvelopeSweep();
	BYTE getLogicalAmplitude(AUDIO_CHANNELS channel);
	float getDACOutput(AUDIO_CHANNELS channel);
	float finHPF(float sampleIn);
	void captureDownsampledAudioSamples();
	void playTheAudioFrame();

public:

	void setPPULCDMode(LCD_MODES lcdMode);
	LCD_MODES getPPULCDMode();
	FLAG isPPULCDEnabled();
	void compareLYToLYC(ID LY);
	void requestVblankStatInterrupt();
	void requestOamStatInterrupt();
	void requestHblankStatInterrupt();
	void processLCDEnable();
	void processLCDDisable();
	BYTE getColorNumberFromColorIDForGB(BYTE palette, BYTE colorID);
	COLOR_FORMAT getColorFromColorIDForGB(BYTE palette, BYTE colorID);
	COLOR_FORMAT getColorFromColorIDForGBC(uint16_t colorID, FLAG isColorCorrectionEnabled);
	void setPaletteIndexForCGB(FLAG isThisForBackground, uint8_t value);
	void setPaletteColorForCGB(FLAG isThisForBackground, uint8_t value);
	void processPixelPipelineAndRender(int32_t dots);
	void translateGFX(PALETTE_ID from, PALETTE_ID to, PALETTE_ID colorCorrectionBefore, PALETTE_ID colorCorrectionAfter);
	void displayCompleteScreen();
	void OAMDMASTATModeGlitch();

private:

	void loadConfig();
	void loadQuirks();

public:

	FLAG saveState(uint8_t id = 0) override;
	FLAG loadState(uint8_t id = 0) override;

	FLAG absoluteSaveState(uint8_t id);
	FLAG absoluteLoadState(uint8_t id);

	FLAG fillGamePlayStack() override;
	FLAG rewindGamePlay() override;

	FLAG bessSaveState(uint8_t id = 0);
	void bessIoSeq(uint8_t* mmr, uint8_t size);
	FLAG bessLoadState(uint8_t id = 0);

public:

	FLAG runEmulationAtHostRate(uint32_t currentFrame) override;
	FLAG runEmulationLoopAtHostRate(uint32_t currentFrame) override;
	FLAG runEmulationAtFixedRate(uint32_t currentFrame) override;
	FLAG runEmulationLoopAtFixedRate(uint32_t currentFrame) override;

public:

	float getEmulationVolume() override;
	void setEmulationVolume(float volume) override;

public:

	void initializeGraphics();
	void initializeAudio();
	void reInitializeAudio();
	FLAG initializeEmulator() override;
	void destroyEmulator() override;
#pragma endregion EMULATION_METHOD_DECLARATION

#pragma region SM83_METHOD_DECLARATION
private:
	
	void cpuSetRegister(REGISTER_TYPE rt, uint16_t u16parameter);
	uint16_t cpuReadRegister(REGISTER_TYPE rt);
	void cpuWritePointer(POINTER_TYPE mrt, uint16_t u16parameter);
	BYTE cpuReadPointer(POINTER_TYPE mrt);

	byte readRawMemory(uint16_t address
		, MEMORY_ACCESS_SOURCE source
		, FLAG FirstPriority_readFromVRAMBank01ForCGB = false
		, FLAG SecondPriority_readFromVRAMBank00ForCGB = false);
	void writeRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source);

	void stackPush(BYTE data);
	BYTE stackPop();

	void processZeroFlag
	(
		byte value
	);
	void processFlagsForLogicalOperation
	(
		byte value,
		FLAG isOperationAND
	);
	void processFlagsFor8BitAdditionOperation
	(
		byte value1,
		byte value2,
		FLAG includeCarryInOperation,
		FLAG affectsCarryFlag = true
	);
	void processFlagsFor16BitAdditionOperation
	(
		uint16_t value1,
		uint16_t value2,
		FLAG includeCarryInOperation,
		FLAG setSZPoF = true
	);
	void processFlagsFor8BitSubtractionOperation
	(
		byte value1,
		byte value2,
		FLAG includeCarryInOperation,
		FLAG affectsCarryFlag = true
	);
	void processFlagsFor16BitSubtractionOperation
	(
		uint16_t value1,
		uint16_t value2,
		FLAG includeCarryInOperation,
		FLAG affectsCarryFlag = true
	);
	void processFlagFor0xE8And0xF8
	(
		byte value1,
		byte value2
	);

	void processUnusedFlags(BYTE result);
	void processUnusedJoyPadBits(BYTE value);
	void processUnusedIFBits(BYTE value);

private:

	inline uint16_t GET_PC()
	{
		RETURN pGBc_registers->pc;
	}

	inline void SET_PC(uint16_t pc)
	{
		pGBc_registers->pc = pc;
	}

	inline void INCREMENT_BC_BY_ONE()
	{
		pGBc_registers->bc.bc_u16memory++;
	}

	inline void INCREMENT_DE_BY_ONE()
	{
		pGBc_registers->de.de_u16memory++;
	}

	inline void INCREMENT_HL_BY_ONE()
	{
		pGBc_registers->hl.hl_u16memory++;
	}

	inline FLAG INCREMENT_PC_BY_ONE()
	{
		if (pGBc_instance->GBc_state.emulatorStatus.isHaltBugActivated == HALT_BUG_STATE::HALT_BUG_ENABLED)
		{
			pGBc_instance->GBc_state.emulatorStatus.isHaltBugActivated = HALT_BUG_STATE::HALT_BUG_DISABLED;
			RETURN YES;
		}
		else
		{
			pGBc_registers->pc++;
			RETURN NO;
		}
	}

	inline void INCREMENT_SP_BY_ONE()
	{
		pGBc_registers->sp++;
	}

	inline void DECREMENT_BC_BY_ONE()
	{
		pGBc_registers->bc.bc_u16memory--;
	}

	inline void DECREMENT_DE_BY_ONE()
	{
		pGBc_registers->de.de_u16memory--;
	}

	inline void DECREMENT_HL_BY_ONE()
	{
		pGBc_registers->hl.hl_u16memory--;
	}

	inline void DECREMENT_PC_BY_ONE()
	{
		pGBc_registers->pc--;
	}

	inline void DECREMENT_SP_BY_ONE()
	{
		pGBc_registers->sp--;
	}

private:

	FLAG processSOC();
	void runCPUPipeline();
	void dumpCpuStateToConsole();
	void unimplementedInstruction();

private:

	MASQ_INLINE void handleStopBasedHalt()
	{
		if (pGBc_instance->GBc_state.emulatorStatus.exitHaltInTCycles > RESET) MASQ_UNLIKELY
		{
			if (--pGBc_instance->GBc_state.emulatorStatus.exitHaltInTCycles == RESET)
			{
				pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = NO;
				pGBc_instance->GBc_state.emulatorStatus.isCPUJustHalted = NO; // precaution
			}
		}
	}
	void cpuTickM(CPU_TICK_TYPE type = CPU_TICK_TYPE::READ_WRITE);
	void gbCpuTick2T(FLAG isT2orT3);
	void syncOtherGBModuleTicks();
	void timerTick();
	void dmaTick();
	void serialTick();
	void rtcTick();
	void ppuTick();
	void apuTick();

public:

	void checkAllSTATInterrupts(FLAG isFF);
	FLAG isInterruptReadyToBeServed();
	void requestInterrupts(INTERRUPTS interrupt);
	FLAG handleInterruptsIfApplicable(FLAG effectiveIME, FLAG effectiveInterruptQ);
#pragma endregion SM83_METHOD_DECLARATION
};
#pragma endregion CORE