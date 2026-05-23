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
// Features
#define NESTEST_AUTOMATED_MODE							NO

#define NES_ENABLE_AUDIO								YES

#define NES_FPS											60.1f
#define NES_APU_FRAME_COUNTER_FPS						(240)
#define NES_REFERENCE_CLOCK_HZ							21441960.0f							// Refer : https://www.nesdev.org/wiki/CPU
#define NES_CPU_CLOCK_HZ								NES_REFERENCE_CLOCK_HZ / 12.0f		// Refer : https://www.nesdev.org/wiki/CPU
#define NES_PPU_CLOCK_HZ								NES_CPU_CLOCK_HZ * 3.0f				// Refer : https://www.nesdev.org/wiki/CPU

// CPU Memory Mapping
#define RAM_START_ADDRESS								(0x0000) // 0x0000
#define ZERO_PAGE_START_ADDRESS							(RAM_START_ADDRESS) // 0x0000
#define ZERO_PAGE_END_ADDRESS							(ZERO_PAGE_START_ADDRESS + 0xFF) // 0x00FF
#define STACK_START_ADDRESS								(ZERO_PAGE_END_ADDRESS + 1) // 0x0100
#define STACK_END_ADDRESS								(STACK_START_ADDRESS + 0xFF) // 0x01FF
#define RAM_END_ADDRESS									(RAM_START_ADDRESS + 0x07FF) // 0x07FF
#define RAM_MIRROR1_START_ADDRESS						(RAM_END_ADDRESS + 1) // 0x0800
#define CPU_RAM_SIZE									(RAM_END_ADDRESS + 1 - RAM_START_ADDRESS) // 0x0800
#define RAM_MIRROR1_END_ADDRESS							(RAM_MIRROR1_START_ADDRESS + 0x07FF) // 0x0FFF
#define RAM_MIRROR2_START_ADDRESS						(RAM_MIRROR1_END_ADDRESS + 1) // 0x1000
#define RAM_MIRROR2_END_ADDRESS							(RAM_MIRROR2_START_ADDRESS + 0x07FF) // 0x17FF
#define RAM_MIRROR3_START_ADDRESS						(RAM_MIRROR2_END_ADDRESS + 1) // 0x1800
#define RAM_MIRROR3_END_ADDRESS							(RAM_MIRROR3_START_ADDRESS + 0x07FF) // 0x1FFF
#define PPU_START_ADDRESS								(RAM_MIRROR3_END_ADDRESS + 1) // 0x2000
#define PPU_CTRL_ADDRESS								(0x2000) // 0x2000
#define PPU_MASK_ADDRESS								(0x2001) // 0x2001
#define PPU_STATUS_ADDRESS								(0x2002) // 0x2002
#define OAM_ADDR_ADDRESS								(0x2003) // 0x2003
#define OAM_DATA_ADDRESS								(0x2004) // 0x2004
#define PPU_SCROLL_ADDRESS								(0x2005) // 0x2005
#define PPU_ADDR_ADDRESS								(0x2006) // 0x2006
#define PPU_DATA_ADDRESS								(0x2007) // 0x2007
#define PPU_END_ADDRESS									(PPU_START_ADDRESS + 0x0007) // 0x2007
#define PPU_CTRL_REG_SIZE								(PPU_END_ADDRESS + 1 - PPU_START_ADDRESS) // 0x0008
#define PPU_MIRROR_START_ADDRESS						(PPU_END_ADDRESS + 1) // 0x2008
#define PPU_MIRROR_END_ADDRESS							(0x3FFF) // 0x3FFF
#define APU_AND_IO_START_ADDRESS						(PPU_MIRROR_END_ADDRESS + 1) // 0x4000
#define OAM_DMA_ADDRESS									(0x4014) // 0x4014
#define APU_STATUS_ADDRESS								(0x4015) // 0x4015
#define JOYSTICK1_ADDRESS								(0x4016) // 0x4016
#define JOYSTICK2_OR_FRAMECFG_ADDRESS					(0x4017) // 0x4017
#define APU_AND_IO_END_ADDRESS							(APU_AND_IO_START_ADDRESS + 0x0017) // 0x4017
#define OTHER_APU_AND_IO_START_ADDRESS					(APU_AND_IO_END_ADDRESS + 1) // 0x4018
#define OTHER_APU_AND_IO_END_ADDRESS					(OTHER_APU_AND_IO_START_ADDRESS + 0x0007) // 0x401F
#define UNMAPPED_START_ADDRESS							(OTHER_APU_AND_IO_END_ADDRESS + 1) // 0x4020
#define CATRIDGE_RAM_START_ADDRESS						(0x6000) // 0x6000
#define CATRIDGE_RAM_END_ADDRESS						(CATRIDGE_RAM_START_ADDRESS + 0x1FFF) // 0x7FFF
#define CATRIDGE_ROM_BANK0_START_ADDRESS				(0x8000) // 0x8000
#define CATRIDGE_ROM_BANK0_END_ADDRESS					(CATRIDGE_ROM_BANK0_START_ADDRESS + 0x3FFF) // 0xBFFF
#define CATRIDGE_ROM_BANK1_START_ADDRESS				(0xC000) // 0xC000
#define CATRIDGE_ROM_BANK1_END_ADDRESS					(CATRIDGE_ROM_BANK1_START_ADDRESS + 0x3FFF) // 0xFFFF
#define NMI_VECTOR_START_ADDRESS						(0xFFFA) // 0xFFFA
#define NMI_VECTOR_END_ADDRESS							(NMI_VECTOR_START_ADDRESS + 1) // 0xFFFB
#define RESET_VECTOR_START_ADDRESS						(0xFFFC) // 0xFFFC
#define RESET_VECTOR_END_ADDRESS						(RESET_VECTOR_START_ADDRESS + 1) // 0xFFFD
#define IRQ_BRK_VECTOR_START_ADDRESS					(0xFFFE) // 0xFFFE
#define IRQ_BRK_VECTOR_END_ADDRESS						(IRQ_BRK_VECTOR_START_ADDRESS + 1) // 0xFFFF
#define UNMAPPED_END_ADDRESS							(0xFFFF) // 0xFFFF

// PPU Memory Mapping
#define PATTERN_TABLE0_START_ADDRESS					(0x0000) // 0x0000
#define PATTERN_TABLE0_END_ADDRESS						(PATTERN_TABLE0_START_ADDRESS + 0x0FFF) // 0x0FFF
#define PATTERN_TABLE1_START_ADDRESS					(PATTERN_TABLE0_END_ADDRESS + 1) // 0x1000
#define PATTERN_TABLE1_END_ADDRESS						(PATTERN_TABLE1_START_ADDRESS + 0x0FFF) // 0x1FFF
#define NAME_TABLE0_START_ADDRESS						(PATTERN_TABLE1_END_ADDRESS + 1) // 0x2000
#define NAME_TABLE0_END_ADDRESS							(NAME_TABLE0_START_ADDRESS + 0x03FF) // 0x23FF
#define NAME_TABLE1_START_ADDRESS						(NAME_TABLE0_END_ADDRESS + 1) // 0x2400
#define NAME_TABLE1_END_ADDRESS							(NAME_TABLE1_START_ADDRESS + 0x03FF) // 0x27FF
#define NAME_TABLE2_START_ADDRESS						(NAME_TABLE1_END_ADDRESS + 1) // 0x2800
#define NAME_TABLE2_END_ADDRESS							(NAME_TABLE2_START_ADDRESS + 0x03FF) // 0x2BFF
#define NAME_TABLE3_START_ADDRESS						(NAME_TABLE2_END_ADDRESS + 1) // 0x2C00
#define NAME_TABLE3_END_ADDRESS							(NAME_TABLE3_START_ADDRESS + 0x03FF) // 0x2FFF
#define NAME_TABLE_SIZE									(NAME_TABLE1_END_ADDRESS + 1 - NAME_TABLE0_START_ADDRESS) // 0x0800
#define PPU_UNUSED_START_ADDRESS						(NAME_TABLE3_END_ADDRESS + 1) // 0x3000
#define PPU_UNUSED_END_ADDRESS							(PPU_UNUSED_START_ADDRESS + 0x0EFF) // 0x3EFF
#define PALETTE_RAM_INDEXES_START_ADDRESS				(PPU_UNUSED_END_ADDRESS + 1) // 0x3F00
#define PALETTE_RAM_INDEXES_END_ADDRESS					(PALETTE_RAM_INDEXES_START_ADDRESS + 0x001F) // 0x3F1F
#define PALETTE_RAM_INDEXES_MIRROR_START_ADDRESS		(PALETTE_RAM_INDEXES_END_ADDRESS + 1) // 0x3F20
#define PALETTE_RAM_INDEXES_MIRROR_END_ADDRESS			(PALETTE_RAM_INDEXES_MIRROR_START_ADDRESS + 0x00DF) // 0x3FFF

// Others
#define EMULATED_AUDIO_SAMPLING_RATE_FOR_NES			(48000.0)
#ifdef __EMSCRIPTEN__
#define AUDIO_BUFFER_SIZE_FOR_NES						(CEIL((EMULATED_AUDIO_SAMPLING_RATE_FOR_NES / NES_FPS))) // 32
#else
#define AUDIO_BUFFER_SIZE_FOR_NES						(CEIL((EMULATED_AUDIO_SAMPLING_RATE_FOR_NES / NES_FPS)))
#endif
#define NES_PPU_WAIT_CPU_CYCLES_POST_RESET				(29658)
#define NES_LAST_PPU_CYCLE_PER_SCANLINE					(340)
#define NES_TOTAL_PPU_CYCLES_PER_SCANLINE				(341)
#define NES_LAST_VISIBLE_PPU_SCANLINE					(239)
#define NES_LAST_PPU_SCANLINE							(260)
#define NES_TOTAL_PPU_SCANLINE							(261)
#define NES_PRE_RENDER_SCANLINE							(-1)
#define NES_FIRST_VISIBLE_SCANLINE						(0)
#define NES_POST_RENDER_SCANLINE						(241)
#define NES_LAST_SCANLINE_PER_FRAME						(239)

#define NES_IRQ_SRC_NONE								(RESET)

#define NES_PPU_NTSC_FRAME_DOTS							(89342)

#define NES_MAX_ROM_SIZE								(0x202000)
#pragma endregion MACROS

#pragma region CORE
class NES_t : public abstractEmulation_t
{
#pragma region INFRASTRUCTURE_DECLARATIONS
public:

	std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom;
	const float myFPS = (float)NES_FPS;

public:

	uint32_t y_offset = ZERO;
	uint32_t x_offset = ZERO;
	static const uint32_t screen_height = 240;
	static const uint32_t screen_width = 256;
	static const uint32_t pixel_height = 2;
	static const uint32_t pixel_width = 2;
	static const uint32_t debugger_screen_height = 560;
	static const uint32_t debugger_screen_width = 880;
	static const uint32_t debugger_pixel_height = 1;
	static const uint32_t debugger_pixel_width = 1;
	const char* NAME = "NES";

private:

	MasqConfig_t pt;

private:

	uint8_t const TEST_ROMS = ONE;
	uint8_t const SST_ROMS = TWO;

private:

	uint32_t profiler_FrameRate;
	uint64_t functionID;

private:

	CheatEngine_t *ceNES;
#pragma endregion INFRASTRUCTURE_DECLARATIONS

#pragma region RP2C02_DECLARATIONS

PACK_BEGIN
private:

	enum class NAMETABLE_MIRROR
	{
		HORIZONTAL_MIRROR,
		VERTICAL_MIRROR,
		ONESCREEN_LO_MIRROR,
		ONESCREEN_HI_MIRROR,
	};

	enum class PPU_BG_FSM
	{
		RELOAD_SHIFTERS = ZERO,
		FETCH_NAMETABLE_BYTE = ONE,
		FETCH_ATTRTABLE_BYTE = THREE,
		FETCH_PATTTABLE_LBYTE = FIVE,
		FETCH_PATTTABLE_HBYTE = SEVEN,
		MAX_PPU_BG_FSM = EIGHT
	};

private:

	typedef struct
	{
		uint8_t BASE_NAMETABLE_ADDR_H : 1; // bit   0
		uint8_t BASE_NAMETABLE_ADDR_V : 1; // bit   1
		uint8_t VRAM_ADDRESS_INCREMENT : 1; // bit  2
		uint8_t SPRITE_PATTER_TABLE_ADDR_8x8 : 1; // bit  3
		uint8_t BG_PATTERN_TABLE_ADDR : 1; // bit  4
		uint8_t SPRITE_SIZE : 1; // bit  5	
		uint8_t PPU_MASTER_SLAVE_SEL : 1; // bit  6
		uint8_t VBLANK_NMI_ENABLE : 1; // bit  7
	} ppuctrl_t;

	typedef union
	{
		ppuctrl_t ppuctrl;
		BYTE raw;
	} PPUCTRL_t;

	typedef struct
	{
		uint8_t GREYSCALE : 1; // bits  0
		uint8_t BG_IN_LEFTMOST_8PIXELS : 1; // bits  1
		uint8_t SPRITE_IN_LEFTMOST_8PIXELS : 1; // bit  2
		uint8_t ENABLE_BG_RENDERING : 1; // bit  3
		uint8_t ENABLE_SPRITE_RENDERING : 1; // bit  4
		uint8_t EMP_RED : 1; // bit  5	
		uint8_t EMP_GREEN : 1; // bit  6
		uint8_t EMP_BLUE : 1; // bit  7
	} ppumask_t;

	typedef union
	{
		ppumask_t ppumask;
		BYTE raw;
	} PPUMASK_t;

	typedef struct
	{
		uint8_t ID_2C05 : 5; // bits  0 - 4
		uint8_t SPRITE_OVERFLOW : 1; // bit  5	
		uint8_t SPRITE_0_HIT : 1; // bit  6
		uint8_t VBLANK : 1; // bit  7
	} ppustatus_t;

	typedef union
	{
		ppustatus_t ppustatus;
		BYTE raw;
	} PPUSTATUS_t;

	typedef union
	{
		struct
		{
			uint16_t coarseXScroll : 5; // bits  0 - 4
			uint16_t coarseYScroll : 5; // bits  5 - 9
			uint16_t nameTblSelectH : 1; // bit  10 
			uint16_t nameTblSelectV : 1; // bit  11
			uint16_t fineYScroll : 3; // bits 12 - 14
			uint16_t unused : 1; // bit 15
		} fields;
		struct
		{
			uint16_t lo : 8; // bits 0 - 7
			uint16_t hi : 8; // bits 8 - 15
		} addr;
		uint16_t raw;
	} tv_t;

	typedef struct
	{
		BYTE cpu2ppu;
		tv_t v;
		tv_t t;
		BYTE x;
		FLAG w;
		BYTE ignoreVramRead;
		FLAG needVideoRamIncrement;
		uint8_t  vramAddrUpdateDelay;   // countdown: 3->0 then fires
		uint16_t vramAddrPendingValue;  // the t.raw value to apply
		struct openBus_t
		{
			uint8_t pad[7];
			uint8_t openBusValue;
			// Store per-bit stamps
			uint64_t openBusDecayStamp[8]; // one per bit
		} openBus;
	} ppuInternalRegisters_t;

	typedef struct
	{
		BYTE yPosition;
		BYTE tileID;
		union
		{
			struct
			{
				BYTE palette : 2; // bits 0 - 1
				BYTE unused : 3; // bits 2 - 4
				BYTE priority : 1; // bit 5
				BYTE flipHorizontally : 1; // bit 6
				BYTE flipVertically : 1; // bit 7
			} fields;
			BYTE raw;
		} attributes;
		BYTE xPosition;
	} oamFields_t;

	typedef union
	{
		BYTE oamB[TWOFIFTYSIX];
		BYTE oam2B[SIXTYFOUR][FOUR];
		oamFields_t oamW[sizeof(oamB) / sizeof(oamFields_t)];
	} primaryOam_t;

	typedef union
	{
		BYTE oamB[THIRTYTWO];
		BYTE oam2B[EIGHT][FOUR];
		oamFields_t oamW[sizeof(oamB) / sizeof(oamFields_t)];
	} secondaryOam_t;

	typedef union
	{
		BYTE oamB[TWOFIFTYSIX];
		BYTE oam2B[SIXTYFOUR][FOUR];
		oamFields_t oamW[sizeof(oamB) / sizeof(oamFields_t)];
	} overflowOam_t;

	typedef struct
	{
		FLAG spriteOverflow;
		FLAG sprite0hit;
		FLAG vblank;
		FLAG ppuStatusReadQuirkEnable;
		ID pn;
		ID pm;
		ID sn;
		ID sm;
		FLAG startSpriteOverflowEvaluation;
		FLAG stopSpriteEvaluation;
		BYTE oamByte;
		uint8_t oamCorruptionSeed;   // row to corrupt (0xFF = no corruption pending)
		FLAG oamCorruptionPending;
		uint8_t oamCorruptionDelay;
		FLAG prevRendering;
		ppuInternalRegisters_t ppuInternalRegisters;
		// Debug
		uint64_t vblSetPPUCycle;
		uint64_t vblClearPPUCycle;
		uint64_t startOfFrameToNMITriggerPPUCycles;
		uint64_t startOfFrameToNMIHandlerPPUCycles;
	} ppuRegisters_t;

	typedef union
	{
		struct
		{
			BYTE patternTable0[PATTERN_TABLE0_END_ADDRESS + ONE - PATTERN_TABLE0_START_ADDRESS];
			BYTE patternTable1[PATTERN_TABLE1_END_ADDRESS + ONE - PATTERN_TABLE1_START_ADDRESS];
		};
		BYTE raw[PATTERN_TABLE1_END_ADDRESS + ONE - PATTERN_TABLE0_START_ADDRESS];
	} patternTable_t;

	typedef struct
	{
		patternTable_t patternTable;
		BYTE nameTable0[NAME_TABLE0_END_ADDRESS + ONE - NAME_TABLE0_START_ADDRESS];
		BYTE nameTable1[NAME_TABLE1_END_ADDRESS + ONE - NAME_TABLE1_START_ADDRESS];
		BYTE nameTable2[NAME_TABLE2_END_ADDRESS + ONE - NAME_TABLE2_START_ADDRESS];
		BYTE nameTable3[NAME_TABLE3_END_ADDRESS + ONE - NAME_TABLE3_START_ADDRESS];
		BYTE unused[PPU_UNUSED_END_ADDRESS + ONE - PPU_UNUSED_START_ADDRESS];
		BYTE paletteRamIndex[PALETTE_RAM_INDEXES_END_ADDRESS + ONE - PALETTE_RAM_INDEXES_START_ADDRESS];
		BYTE paletteRamIndexMir[PALETTE_RAM_INDEXES_MIRROR_END_ADDRESS + ONE - PALETTE_RAM_INDEXES_MIRROR_START_ADDRESS];
		//
		primaryOam_t primaryOam;
		secondaryOam_t secondaryOam;
		overflowOam_t overflowOam;
	} ppuMemoryMap_t;

	typedef union
	{
		ppuMemoryMap_t NESMemoryMap;
		BYTE NESRawMemory[sizeof(ppuMemoryMap_t)];
	} NES_ppuMemory_t;

#pragma endregion RP2C02_DECLARATIONS

#pragma region RP2A03_DECLARATIONS
private:

	enum class CYCLES_TYPE
	{
		READ_CYCLE,
		WRITE_CYCLE
	};

	enum class REGISTER_TYPE	// register_op_type
	{
		RT_A,
		RT_X,
		RT_Y,
		RT_P,
		RT_PC,
		RT_SP,
		RT_TOTAL,
		RT_NONE
	};

	enum class EXCEPTION_EVENT_TYPE
	{
		EVENT_NONE,
		EVENT_NMI,
		EVENT_IRQ,
		EVENT_RESET,
		EVENT_BRK
	};

	enum class AUDIO_CHANNELS
	{
		PULSE_1,
		PULSE_2,
		TRIANGLE,
		NOISE,
		DMC,
		TOTAL_AUDIO_CHANNELS
	};

	enum class FRAME_SEQUENCER_MODE
	{
		FOUR_STEP_MODE,
		FIVE_STEP_MODE,
		TOTAL_FRAME_SEQUENCER_MODES
	};

	// Refer : https://forums.nesdev.org/viewtopic.php?p=64359#p64359 for the apuSequencer magic numbers
	enum class FRAME_SEQUENCER_CYCLES_MODE_0
	{
		STEP_M00 = 7459,
		STEP_M01 = 14915,
		STEP_M02 = 22373,
		STEP_M03 = 29831,
		STEP_M04 = 37289
	};
	enum class FRAME_SEQUENCER_CYCLES_MODE_1
	{
		STEP_M10 = 1,
		STEP_M11 = 7459,
		STEP_M12 = 14915,
		STEP_M13 = 22373,
		STEP_M14 = 37283
	};

	const BYTE LENGTH_COUNTER_LUT[THIRTYTWO] =
	{
		10,254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
		12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
	};

	const BYTE PULSE_AMPLITUDE_LUT[FOUR][EIGHT] =
	{
		{LO, HI, LO, LO, LO, LO, LO, LO},
		{LO, HI, HI, LO, LO, LO, LO, LO},
		{LO, HI, HI, HI, HI, LO, LO, LO},
		{HI, LO, LO, HI, HI, HI, HI, HI}
	};

	const uint16_t NOISE_PERIOD_LUT[SIXTEEN] =
	{
		4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
	};

	const uint16_t DMC_PERIOD_LUT[SIXTEEN] =
	{
		428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106,  84,  72,  54
	};

private:

	typedef struct
	{
		uint8_t opcode;
	} cpu_t;

	typedef struct
	{
		uint8_t FCARRY : 1; // bit  0
		uint8_t FZERO : 1; // bit  1
		uint8_t INTERRUPT_DISABLE : 1; // bit  2
		uint8_t DECIMAL : 1; // bit  3
		uint8_t FCAUSE : 1; // bit  4
		uint8_t FORCED_TO_ONE : 1; // bit  5	
		uint8_t FOVERFLOW : 1; // bit  6
		uint8_t FNEGATIVE : 1; // bit  7
	} flagFields_t;

	typedef union
	{
		flagFields_t flagFields;
		uint8_t p;
	} p_t;

	typedef struct
	{
		uint8_t a;
		uint8_t x;
		uint8_t y;
		p_t p;
		uint8_t sp;
		uint16_t pc;
		byte openbus;
		uint8_t pad;
	} cpuRegisters_t;

	typedef struct
	{
		FLAG DMAInProgress;
		BYTE dataToTx;
		uint16_t sourceAddress;
	} DMA_t;

	typedef union
	{
		struct
		{
			PPUCTRL_t PPUCTRL;
			PPUMASK_t PPUMASK;
			PPUSTATUS_t PPUSTATUS;
			BYTE OAMADDR;
			BYTE OAMDATA;
			BYTE PPUSCROLL;
			BYTE PPUADDR;
			BYTE PPUDATA;
		} ppuCtrl;
		BYTE raw[PPU_END_ADDRESS + ONE - PPU_START_ADDRESS];
	} ppuCtrl_t;

	typedef union
	{
		struct
		{
			BYTE ENV_PERIOD_OR_VOL : 4; // bits  0 - 3
			BYTE CONSTANT_VOL : 1; // bit  4
			BYTE LOOP_ENV_OR_DIS_LENGTH_COUNTER : 1; // bit  5
			BYTE DUTY : 2; // bits  6 - 7
		};
		BYTE raw;
	} SQx_VOL_t;

	typedef union
	{
		struct
		{
			BYTE SHIFT_COUNT : 3; // bits  0 - 2
			BYTE NEGATIVE : 1; // bit  3
			BYTE PERIOD : 3; // bits  4 - 6
			BYTE ENABLE : 1; // bit 7
		};
		BYTE raw;
	} SQx_SWEEP_t;

	typedef union
	{
		struct
		{
			BYTE HIGH : 3; // bits  0 - 2
			BYTE LENGTH_COUNTER : 5; // bits  3 - 7
		};
		BYTE raw;
	} SQx_HIGH_t;

	typedef union
	{
		struct
		{
			BYTE LINEAR_COUNTER : 7; // bits  0 - 6
			BYTE DIS_LENGTH_COUNTER : 1; // bit	7
		};
		BYTE raw;
	} TRI_LINEAR_t;

	typedef union
	{
		struct
		{
			BYTE LINEAR_COUNTER : 4; // bits  0 - 3
			BYTE UNUSED : 3; // bits 4 - 6
			BYTE LOOP_NOISE : 1; // bit	7
		};
		BYTE raw;
	} NOISE_PERIOD_t;

	typedef union
	{
		struct
		{
			BYTE FREQUENCY_INDEX : 4; // bits  0 - 3
			BYTE UNUSED : 2; // bits 4 - 5
			BYTE LOOP_SAMPLE : 1; // bit	6
			BYTE IRQ_ENABLE : 1; // bit	7
		};
		BYTE raw;
	} DMC_FREQ_INDEX_t;

	typedef union
	{
		struct
		{
			BYTE DIRECT_LOAD : 7; // bits  0 - 6
			BYTE UNUSED : 1; // bit	7
		};
		BYTE raw;
	} DMC_DIRECT_LOAD_t;

	typedef union
	{
		struct
		{
			BYTE PULSE1 : 1; // bit 0
			BYTE PULSE2 : 1; // bit 1
			BYTE TRIANGLE : 1; // bit 2
			BYTE NOISE : 1; // bit 3
			BYTE DMC_ENABLE : 1; // bit	4
			BYTE UNUSED : 1; // bit 5
			BYTE FRAME_INTR : 1; // bit 6
			BYTE DMC_INTR : 1; // bit 7
		};
		struct
		{
			BYTE rw : 5; // bits 0 - 4
			BYTE ro : 3; // bits 5 - 7
		};
		BYTE raw;
	} SND_CHN_t;

	typedef union
	{
		struct
		{
			BYTE UNUSED : 6; // bits 0 - 5
			BYTE DIS_FRAME_INTR : 1; // bit 6
			BYTE FRAME_SEQ_MODE : 1; // bit 7
		} FRAME_CONFIG;
		struct
		{
			BYTE joy2;
		} JOY2;
		BYTE raw;
	} JOY2_OR_APU_FRAME_CONFIG_t;

	typedef union
	{
		struct
		{
			SQx_VOL_t SQ1_VOL;
			SQx_SWEEP_t SQ1_SWEEP;
			BYTE SQ1_LO;
			SQx_HIGH_t SQ1_HI;
			SQx_VOL_t SQ2_VOL;
			SQx_SWEEP_t SQ2_SWEEP;
			BYTE SQ2_LO;
			SQx_HIGH_t SQ2_HI;
			TRI_LINEAR_t TRI_LINEAR;
			BYTE UNUSED0;
			BYTE TRI_LO;
			SQx_HIGH_t TRI_HI;
			SQx_VOL_t NOISE_VOL;
			BYTE UNUSED1;
			NOISE_PERIOD_t NOISE_PERIOD;
			SQx_HIGH_t NOISE_LENGTH_COUNTER;
			DMC_FREQ_INDEX_t DMC_FREQ;
			DMC_DIRECT_LOAD_t DMC_RAW;
			BYTE DMC_START;
			BYTE DMC_LEN;
			BYTE OAMDMA;
			SND_CHN_t SND_CHN;
			BYTE JOY1;
			JOY2_OR_APU_FRAME_CONFIG_t JOY2_OR_FRAME_CONFIG;
		};
		BYTE raw[APU_AND_IO_END_ADDRESS + ONE - APU_AND_IO_START_ADDRESS];
	} io_t;

	typedef struct
	{
		BYTE wram[RAM_END_ADDRESS + ONE - RAM_START_ADDRESS];
		BYTE wramMir1[RAM_MIRROR1_END_ADDRESS + ONE - RAM_MIRROR1_START_ADDRESS];
		BYTE wramMir2[RAM_MIRROR2_END_ADDRESS + ONE - RAM_MIRROR2_START_ADDRESS];
		BYTE wramMir3[RAM_MIRROR3_END_ADDRESS + ONE - RAM_MIRROR3_START_ADDRESS];
		ppuCtrl_t ppuCtrl;
		BYTE ppuMir[PPU_MIRROR_END_ADDRESS + ONE - PPU_MIRROR_START_ADDRESS];
		io_t apuAndIO;
		BYTE otherApuAndIO[OTHER_APU_AND_IO_END_ADDRESS + ONE - OTHER_APU_AND_IO_START_ADDRESS];
		BYTE catridgeMappedMemory[UNMAPPED_END_ADDRESS + ONE - UNMAPPED_START_ADDRESS];
	} cpuMemoryMap_t;

	typedef union
	{
		cpuMemoryMap_t NESMemoryMap;
		BYTE NESRawMemory[sizeof(cpuMemoryMap_t)];
	} NES_cpuMemory_t;

#pragma endregion RP2A03_DECLARATIONS

#pragma region EMULATION_DECLARATIONS
private:

	enum MAPPER : int16_t
	{
		MAPPER_NOT_APPLICABLE = INVALID,
		NROM = ZERO,
		MMC1 = ONE,
		UxROM_002 = TWO,
		CNROM = THREE,
		MMC3 = FOUR,
		MMC5 = FIVE,
		INES_MAPPER_006 = SIX,
		AxROM = SEVEN,
		MMC2 = NINE,
		MMC4 = TEN,
		COLOR_DREAMS = ELEVEN,
		CPROM = THIRTEEN,
		INES_MAPPER_016 = SIXTEEN,
		INES_MAPPER_018 = EIGHTEEN,
		INES_MAPPER_019 = NINETEEN,
		VRC2_022 = TWENTYTWO,
		VRC4_021 = TWENTYONE,
		VRC2_VRC4_023 = TWENTYTHREE,
		VRC6_024 = TWENTYFOUR,
		VRC2_VRC4_025 = TWENTYFIVE,
		VRC6_026 = TWENTYSIX,
		INES_MAPPER_034 = THIRTYFOUR,
		INES_MAPPER_037 = THIRTYSEVEN,
		INES_MAPPER_047 = FORTYSEVEN,
		RAMBO1 = SIXTYFOUR,
		GxROM = SIXTYSIX,
		INES_MAPPER_067 = SIXTYSEVEN,
		INES_MAPPER_068 = SIXTYEIGHT,
		INES_MAPPER_069 = SIXTYNINE,
		INES_MAPPER_070 = SEVENTY,
		J87 = EIGHTYSEVEN,
		INES_MAPPER_105 = ONEHUNDREDFIVE,
		INES_MAPPER_118 = ONEHUNDREDEIGHTEEN,
		INES_MAPPER_119 = ONEHUNDREDNINETEEN,
		INES_MAPPER_152 = ONEHUNDREDFIFTYTWO,
		INES_MAPPER_155 = ONEHUNDREDFIFTYFIVE,
		INES_MAPPER_158 = ONEHUNDREDFIFTYEIGHT,
		NANJING_FC001 = ONEHUNDREDSIXTYTHREE,
		INES_MAPPER_180 = ONEEIGHTY,
		INES_MAPPER_210 = TWOHUNDREDTEN
	};

	enum SUB_MAPPER : int16_t
	{
		SUB_MAPPER_NOT_APPLICABLE = INVALID,
		SEROM_SHROM_SH1ROM = ZERO,
		SUROM = ONE,
		MMC6 = ONE,
		NINA = ONE,
		SOROM = TWO,
		BNROM = TWO,
		SNROM = THREE,
		SXROM = FOUR,
		VRC4AC = TWENTYONE,
		VRC2A = ZERO,
		VRC2B = THREE,
		VRC2C = THREE,
		VRC4A = ONE,
		VRC4B = ONE,
		VRC4C = TWO,
		VRC4D = TWO,
		VRC4E = TWO,
		VRC4F = ONE,
		BANDAI_LZ93D50_24C01 = ONE, // 128-byte EEPROM
		BANDAI_FCG_1_2 = FOUR, // Standard FCG-1 / FCG-2 (No EEPROM or standard size)
		BANDAI_LZ93D50_24C02 = FIVE,  // 256-byte EEPROM
		NAMCO175 = ONE,              // Mapper 210 sub 1 = Namco 175 (no audio)
		NAMCO340 = TWO,              // Mapper 210 sub 2 = Namco 340 (no audio, mirroring)
	};

	enum PCBs : int32_t
	{
		PCB_NOT_APPLICABLE = INVALID,
		PCB_352398 = 352398,
		PCB_351406 = 351406,
		PCB_352889 = 352889,
		PCB_352400 = 352400,
		PCB_352396 = 352396,
		PCB_351948 = 351948,
		PCB_350603 = 350603,
		PCB_350636 = 350636,
		PCB_350926 = 350926,
		PCB_351179 = 351179,
		PCB_351618 = 351618,
	};

	enum class MEMORY_ACCESS_SOURCE
	{
		DEBUG_PORT,
		DMA,
		CPU,
		PPU,
		APU
	};

	enum class TYPE_OF_MEMORY_ACCESS
	{
		PPU_READ,
		PPU_WRITE,
		CPU_READ,
		CPU_WRITE
	};

private:

	typedef struct
	{
		uint32_t placeholder;
	} quirks_t;

private:

	typedef struct
	{
		union
		{
			ID mapperID;
			MAPPER mapper;
		};
		SUB_MAPPER subMapper;
		NAMETABLE_MIRROR nameTblMir;
		struct
		{
			BYTE placeholder;
		} nrom;
		struct
		{
			union
			{
				struct
				{
					BYTE data03 : 4; // bits  0 - 3
					BYTE data4 : 1; // bit 4
					BYTE data57 : 3; // bits  5 - 7
				} fields1;
				struct
				{
					BYTE shiftValue : 5; // bits 0 - 4
					BYTE unused : 3; // bits  5 - 7
				} fields2;
				BYTE raw;
			} intfShiftReg;
			union
			{
				struct
				{
					BYTE mm : 2; // bits 0 - 1
					BYTE pp : 2; // bits 2 - 3
					BYTE c : 1; // bit 4
					BYTE unused : 3; // bits  5 - 7
				} fields1;
				BYTE raw;
			} intfControlReg;
			BYTE chrBank4Lo;
			BYTE chrBank4Hi;
			BYTE chrBank8;
			BYTE prgBank16Lo;
			BYTE prgBank16Hi;
			BYTE prgBank32;
			FLAG prgRamEnable;
			FLAG isMmc1A; // set when mapperID == 155
			COUNTER8 clrWriteCount;
			SUB_MAPPER subMapper;
			struct
			{
				BYTE prgBank256;
				BYTE prgRamBank8_sxrom;
			} surom_sxrom;
			struct
			{
				BYTE   initState;        // 0 = locked, 1 = I-bit went 0, 2 = unlocked
				uint32_t irqCounter;     // 30-bit up-counter clocked every CPU cycle
				FLAG   irqEnabled;       // true when I-bit (chrBank4Lo bit 4) == 0
				BYTE   dipSwitches;      // 4 bits from config (DIP switches 3..0 → bits 28..25)
			} nes_event;
		} mmc1;
		struct
		{
			BYTE prgBank16;
		} uxrom_002;
		struct
		{
			BYTE chrBank8;
		} cnrom;
		struct
		{
			struct
			{
				union
				{
					struct
					{
						BYTE bankRegSel : 3; // bits  0 - 2
						BYTE unused : 2; // bits 3 - 4
						BYTE mm6PrgRamEnable : 1; // bit 5
						BYTE prgRomMode : 1; // bit 6
						BYTE chrA12Inversion : 1; // bit 7
					} fields;
					BYTE raw;
				} bankRegisterSelect_even8k;
				BYTE bankData_odd8k;
				union
				{
					struct
					{
						BYTE isHorizontal : 1; // bit  0
						BYTE unused : 7; // bits 1 - 7
					} fields;
					BYTE raw;
				} mirroring_evenAk;
				union
				{
					struct
					{
						BYTE unused : 6; // bits  0 - 5
						BYTE denyWrite : 1; // bit 6
						BYTE prgRamEnable : 1; // bit 7
					} fields;
					struct
					{
						BYTE unused : 4; // bits  0 - 3
						BYTE enW01FF : 1; // bit 4
						BYTE enR01FF : 1; // bit 5
						BYTE enW23FF : 1; // bit 6
						BYTE enR23FF : 1; // bit 7
					} mmc6Fields;
					BYTE raw;
				} prgRamProtect_oddAk;
				BYTE irqReload_evenCk;
				BYTE irqReload_oddCk;
				BYTE irqDisable_evenEk;
				BYTE irqEnable_oddEk;
			} exRegisters;
			struct
			{
				BYTE chrBank2a;
				BYTE chrBank2b;
				BYTE chrBank1a;
				BYTE chrBank1b;
				BYTE chrBank1c;
				BYTE chrBank1d;
				BYTE prgBank8a;
				BYTE prgBank8b;
				FLAG unfilteredA12RiseEvent;
				FLAG filteredA12RiseEvent;
				BYTE currentMMC3IrqCounter;
				FLAG mmc3IrqCounterReloadEnabled;
				FLAG mmc3IrqEnable;
			} inRegisters;
			struct
			{
				BYTE outerBank;
			} ines037;
			struct
			{
				BYTE multicart;
			} ines047;
			struct
			{
				BYTE chrBank2aCS; // chip select for chrBank2a (0=ROM, 1=RAM)
				BYTE chrBank2bCS;
				BYTE chrBank1aCS;
				BYTE chrBank1bCS;
				BYTE chrBank1cCS;
				BYTE chrBank1dCS;
			} ines119;
			struct rambo1_t
			{
				// RAMBO-1 ($8000 bank select register, raw)
				// Bit layout mirrors MMC3 bankRegisterSelect_even8k but adds bit 5 (K)
				// Bit 7 = CHR A12 inversion
				// Bit 6 = PRG mode  (0: R6@$8000 R7@$A000 RF@$C000
				//                     1: RF@$8000 R7@$A000 R6@$C000)
				// Bit 5 = K — full 1KB CHR mode for lower pattern table
				//          0: R0/R1 are 2KB banks at $0000/$0800 (or $1000/$1800)
				//          1: R0/R1 are 1KB banks; R8/R9 supply the odd 1KB slots
				// Bits 3:0 = register select (0-9, 0xF)
				BYTE currentRegister;  // raw $8000 value

				// 16 bank registers (indices 0-9 used, 0xF also used)
				// R0-R5  : CHR banks
				// R6, R7 : PRG 8KB banks
				// R8, R9 : extra 1KB CHR banks (K=1 mode)
				// RF (index 15) : third switchable PRG 8KB bank ($C000 or $8000)
				BYTE reg[16];

				// IRQ state
				FLAG irqEnabled;
				FLAG irqCycleMode;    // 0=scanline (A12), 1=CPU cycle (every 4 clocks)
				FLAG needReload;      // set on $C001 write; causes reload on next clock
				BYTE irqCounter;
				BYTE irqReloadValue;
				BYTE cpuClockCounter; // 0-3 prescaler for cycle mode
				FLAG forceClock;      // Skull & Crossbones: extra clock when switching modes

				// IRQ fire delay (in CPU cycles) counting down to assert
				BYTE irqDelay;        // 0=idle, non-zero counting down
			} rambo1;

			struct txsrom_t
			{
				// Per-nametable override: four entries, each 0 or 1 (CIRAM page)
				// Updated every time $8001 is written with a CHR register that maps
				// into the *active* pattern table half ($0000-$0FFF).
				BYTE ntPage[4];  // CIRAM page assigned to NT0-NT3
			} txsrom;
			FLAG isRevA;
		} mmc3;
		struct
		{
			// $5100: PRG banking mode
			// 0 = 32KB ROM  ($8000-$FFFF via $5117)
			// 1 = 16KB+16KB ($8000-$BFFF via $5115, $C000-$FFFF via $5117)
			// 2 = 16KB+8KB+8KB ($8000-$BFFF via $5115, $C000-$DFFF via $5116, $E000-$FFFF via $5117)
			// 3 = 8KB+8KB+8KB+8KB ($8000-$9FFF via $5114, $A000-$BFFF via $5115, $C000-$DFFF via $5116, $E000-$FFFF via $5117)
			BYTE prgMode;

			// $5102/$5103: PRG-RAM write protect
			// Writes to RAM allowed only when protect1 == 0x02 && protect2 == 0x01
			BYTE prgRamProtect1;
			BYTE prgRamProtect2;

			// $5113-$5117: PRG bank registers (index 0=$5113, 1=$5114, ... 4=$5117)
			// Bit 7 = 0: select 8KB PRG-RAM bank (except $5117 which is always ROM)
			// Bit 7 = 1: select 8KB PRG-ROM bank
			// $6000-$7FFF always via prgBanks[0] ($5113, always RAM)
			// $5117 (index 4) always ROM — bit 7 forced to 1 on write
			BYTE prgBanks[5];

			// $5101: CHR banking mode
			// 0 = 8KB, 1 = two 4KB, 2 = four 2KB, 3 = eight 1KB
			BYTE chrMode;

			// $5130: CHR upper address bits (A18/A19) applied to all CHR bank writes
			BYTE chrUpperBits;

			// $5120-$512B: CHR bank registers (indices 0-11)
			// Indices 0-7  = set A ($5120-$5127): used for sprites (8x16 mode) / everything (8x8 mode)
			// Indices 8-11 = set B ($5128-$512B): used for BG tiles when in 8x16 sprite mode
			// chrUpperBits already ORed into the high byte on write
			uint16_t chrBanks[12];

			// Last CHR register written ($5120-$512B address) — drives chrA switching logic
			uint16_t lastChrReg;
			FLAG chrA;

			// $5104: Extended RAM mode
			// 0/1 = nametable extension (CPU write-only during rendering, else writes 0)
			// 2   = general-purpose read/write RAM
			// 3   = read-only RAM
			BYTE extendedRamMode;

			// $5105: Per-nametable mapping (2 bits each, packed: NT3[7:6] NT2[5:4] NT1[3:2] NT0[1:0])
			// 0 = CIRAM bank 0 (nameTable0)
			// 1 = CIRAM bank 1 (nameTable1)
			// 2 = ExRAM (exRam[])
			// 3 = Fill mode (fillTile / fillColor)
			BYTE nametableMapping;

			// $5106/$5107: Fill mode
			// fillTile: tile byte returned for all non-attribute nametable fetches in fill nametables
			// fillColor: 2-bit palette index; attribute byte = c | c<<2 | c<<4 | c<<6
			BYTE fillTile;
			BYTE fillColor;

			// $5203: IRQ scanline counter target
			BYTE irqCounterTarget;
			// $5204 bit 7: IRQ enable
			FLAG irqEnabled;
			// Internal IRQ state
			BYTE scanlineCounter;
			FLAG irqPending;

			// $5205/$5206: 8x8 hardware multiplier
			BYTE multiplier1;
			BYTE multiplier2;

			// PPU in-frame detection state
			// Used for: IRQ scanline counting, ExRAM mode 0/1 write gating, chrA switching
			FLAG ppuInFrame;
			FLAG needInFrame;
			// Decremented each PPU tick; reset to 9 (~3 CPU cycles) on each PPU read
			// When it hits 0: ppuInFrame is cleared
			BYTE ppuIdleCounter;
			uint16_t lastPpuReadAddr;
			// Counts consecutive identical NT reads; >= 2 means scanline boundary detected
			BYTE ntReadCounter;
			// Tile counter within current scanline (increments on each NT fetch, resets on scanline detect)
			int32_t splitTileNumber;

			// Extended attribute mode ($5104 == 1) intercept state
			// Refer https://www.nesdev.org/wiki/MMC5#Extended_attribute_mode
			uint16_t exAttrLastNTFetch;   // nametable offset of last NT fetch
			int8_t exAttrFetchCounter;    // 3=NT fetched, 2=attr fetch, 1=CHR low, 0=CHR high
			BYTE exAttrSelectedChrBank;   // 8KB CHR bank index from ExRAM for current tile

			// ExRAM: 1KB at CPU $5C00-$5FFF
			// Also mapped to nametable slots (when nametableMapping selects slot 2)
			BYTE exRam[0x400];

			// PRG-RAM: up to 64KB, banked in 8KB slots via prgBanks (when bit 7 == 0)
			// Uses lower 64KB of what would otherwise be wasted space
			BYTE prgRam[0x10000];

			// MMC5 Audio
			struct mmc5Audio_t
			{
				// Pulse 1 ($5000-$5003)
				// Pulse 2 ($5004-$5007)
				struct mmc5Pulse_t
				{
					uint16_t frequencyCounter;
					union
					{
						struct
						{
							uint16_t lo : 8;
							uint16_t hi : 3;
							uint16_t unused : 5;
						} fields;
						uint16_t raw;
					} frequencyPeriod;
					uint8_t  dutyCounter;
					uint16_t lengthCounter;
					struct
					{
						FLAG   startFlag;
						uint8_t decayLevelCounter;
						uint8_t divider;
					} envelope;
					uint8_t  duty;           // from $5000/$5004 bits 6-7
					FLAG     envelopeLoop;   // from $5000/$5004 bit 5 (also length counter halt)
					FLAG     constantVolume; // from $5000/$5004 bit 4
					uint8_t  envelopePeriod; // from $5000/$5004 bits 0-3
					FLAG     enabled;        // from $5015 bits 0-1
					NES_AUDIO_SAMPLE_TYPE dacInput;
				} pulse[2];

				// PCM ($5010-$5011)
				FLAG    pcmIrqEnable;     // $5010 bit 7
				FLAG    pcmReadMode;      // $5010 bit 0 (0=write, 1=read)
				uint8_t pcmRawSample;     // $5011
				FLAG    pcmIrqPending;

				// $5015 status
				uint8_t status;
			} mmc5Audio;
		} mmc5;
		struct
		{
			BYTE prgBank;
			FLAG vramPage;
		} axrom;
		struct
		{
			BYTE prgBank;
			BYTE chrBank;
		} gxrom;
		struct
		{
			BYTE prgBank;
			BYTE chrBankFD[TWO];
			BYTE chrBankFE[TWO];
			BYTE chrBankLatch[TWO];
		} mmc2;
		struct
		{
			BYTE prgBank16;
			BYTE chrBankFD[TWO];
			BYTE chrBankFE[TWO];
			BYTE chrBankLatch[TWO];
		} mmc4;
		struct
		{
			BYTE prgBank32;
			BYTE chrBank8;
		} colorDreams;
		struct
		{
			BYTE chrBank;
		} cprom;
		struct
		{
			// PRG: three switchable 8KB banks + fixed last bank
			uint8_t  prgBank[3];        // $8000/$A000/$C000 — each 8KB

			// CHR: eight 1KB banks, written as two nibbles
			uint8_t  chrBank[8];        // $A000-$D003

			// IRQ subsystem
			// Four 4-bit reload registers, assembled into a 16-bit counter
			uint8_t  irqReload[4];      // $E000-$E003 — nibbles 0..3 (lo→hi)
			uint16_t irqCounter;        // assembled from irqReload on $F000 write
			uint8_t  irqCounterSize;    // 0=16-bit, 1=12-bit, 2=8-bit, 3=4-bit
			FLAG     irqEnabled;        // bit 0 of $F001
		} jaleco18;
		struct
		{
			uint16_t prgBank0;
			uint16_t prgBank1;
			uint16_t chrBank[EIGHT];
			BYTE swapMode;
			BYTE wramEnable;
			int16_t prescaler;
			BYTE irqCounter;
			BYTE irqLatch;
			BYTE irqControl;
			BYTE irqEnable;
			BYTE latch;
		} vrc24;
		struct
		{
			uint8_t prgBank0;       // $8000
			uint8_t prgBank1;       // $C000
			uint8_t chrBank[8];     // $D000-$E003
			uint8_t b003_reg;       // Control register (mirroring / banking mode)

			// IRQ Subsystem
			uint8_t irqLatch;       // $F000
			uint8_t irqControl;     // $F001
			uint8_t irqCounter;     // $F002 / internal counter
			int32_t prescaler;      // Coarse cycle divider tracking

			// ---------------------------------------------------------------
			// VRC6 Audio
			// Refer: https://www.nesdev.org/wiki/VRC6_audio
			// ---------------------------------------------------------------
			struct vrc6Audio_t
			{
				// --- Pulse 1 ($9000-$9002) / Pulse 2 ($A000-$A002) ---
				struct vrc6Pulse_t
				{
					// $x000: duty[6:4], volume[3:0]
					uint8_t  volume;        // bits 3:0  — constant volume (no envelope on VRC6)
					uint8_t  duty;          // bits 6:4  — 0..6 give 1/16..7/16; 7 = always high
					// $x001: freq low byte
					// $x002: halt[7], freq high bits[3:0]
					uint16_t freqPeriod;    // 12-bit period ($x002[3:0] << 8 | $x001)
					FLAG     halt;          // bit 7 of $x002 — stops sequencer when set
					FLAG     enabled;       // gate: cleared by global halt ($9003 bit 0)

					// Internal
					uint16_t freqCounter;   // counts down from freqPeriod to 0
					uint8_t  dutyCounter;   // 0..15, increments on timer reload
					NES_AUDIO_SAMPLE_TYPE dacInput;
				} pulse[2];

				// --- Sawtooth ($B000-$B002) ---
				struct vrc6Sawtooth_t
				{
					// $B000: accum rate[5:0]
					uint8_t  accumRate;     // bits 5:0
					// $B001: freq low byte
					// $B002: halt[7], freq high bits[3:0]
					uint16_t freqPeriod;    // 12-bit period
					FLAG     halt;          // bit 7 of $B002
					FLAG     enabled;       // gate: cleared by global halt ($9003 bit 0)

					// Internal
					uint16_t freqCounter;   // counts down from freqPeriod to 0
					uint8_t  accumulator;   // 8-bit accumulator; added to every other clock
					uint8_t  accumClock;    // counts 0..6; resets+clears accumulator at 7
					NES_AUDIO_SAMPLE_TYPE dacInput;
				} sawtooth;

				// --- Global halt/frequency-shift ($9003) ---
				FLAG     globalHalt;    // bit 0 — halts all three channels when set
				FLAG     freqShift4;    // bit 2 — right-shift freqPeriod by 4 when set
				FLAG     freqShift8;    // bit 1 — right-shift freqPeriod by 8 when set
			} vrc6Audio;
		} vrc6;
		struct
		{
			BYTE prgBank32;
			BYTE chrBank4Lo;
			BYTE chrBank4Hi;
		} ines034;
		struct
		{
			BYTE chrReg;
			BYTE prgReg;
			BYTE m;
		} ines_070_152;
		struct
		{
			FLAG irqCounterEnable;
			FLAG writeState;
			uint16_t irqCounter;
			BYTE chrBank0;       // $8000: 2KB CHR bank at PPU $0000
			BYTE chrBank1;       // $9000: 2KB CHR bank at PPU $0800
			BYTE chrBank2;       // $A000: 2KB CHR bank at PPU $1000
			BYTE chrBank3;       // $B000: 2KB CHR bank at PPU $1800
			BYTE ntBank0;        // $C000: 1KB CHR Nametable bank 0
			BYTE ntBank1;        // $D000: 1KB CHR Nametable bank 1
			BYTE ntControl;      // $E000: Nametable configuration & mirroring type
			BYTE prgBank;        // $F000: 16KB switchable PRG bank at CPU $8000
			FLAG prgRamEnable;
		} ines_067_068;
		struct
		{
			BYTE prgBank[FOUR];
			BYTE chrBank[EIGHT];
			BYTE command;
			BYTE nm;
			FLAG irqEnable;
			FLAG irqCounterEnable;
			BYTE irqControl;
			FLAG ramMode;
			FLAG prgRamEnable;
			uint16_t irqCounter;
		} ines069;
		struct
		{
			// PRG/CHR registers
			BYTE prgBank;
			BYTE chrBank[8];
			BYTE mirroringMode;

			// FIX FOR MESEN ACCURACY: Internal registers for tracking accumulated upper PRG banks
			BYTE prgPage;              // Tracks raw lower bits written to 0x08
			BYTE prgBankSelectUpper;   // Tracks accumulated bit 0 from chrBank[0..7]

			// IRQ Counter
			bool irqCountingEnable;
			uint16_t irqCounter;
			uint16_t irqLatch;

			// Serial EEPROM State Machine
			BYTE eepromState;     // 0: Idle/Waiting for Start, 1: Receiving Address/Command, 2: R/W Payload
			BYTE eepromBitCounter;
			BYTE eepromCommandBuffer;
			BYTE eepromAddress;
			bool eepromAckBit;
			bool lastScl;         // Tracks rising/falling edges of Serial Clock
			bool lastSda;         // Tracks Data line states
			BYTE outBuffer;       // Data currently being shifted out to the CPU

			// Physical EEPROM storage arrays
			BYTE dataArray[256];  // Fits both 24C01 (128B) and 24C02 (256B)
		} ines016;
		struct
		{
			union
			{
				struct
				{
					BYTE lo : 4; // bits 0 - 3
					BYTE hi : 2; // bits 4 - 5
					BYTE unused : 2; // bits 6 - 7
				} fields;
				BYTE raw;
			} prgRomBank;
			FLAG chrRamAutoSwitch;
			FLAG A;
			FLAG B;
			FLAG E;
			FLAG F;
		} nanjing_fc001;
		struct
		{
			// -----------------------------------------------------------------------
			// Variant — stored as BYTE, not enum (matches your macro style)
			// 0 = Namco163 (full chip: audio + IRQ + WRAM write-protect)
			// 1 = Namco175 (no audio, WRAM enable via $C000 bit 0)
			// 2 = Namco340 (no audio, mirroring control via $E000 bits[7:6])
			// 3 = Unknown  (auto-detect in progress)
			// Ref: Mesen NamcoVariant enum + InitMapper()
			// -----------------------------------------------------------------------
			BYTE    variant;            // 0=163, 1=175, 2=340, 3=Unknown
			FLAG    autoDetect;         // YES = variant not confirmed by DB board string
			FLAG    notNamco340;        // YES = $6000-$7FFF write seen; Namco340 locked out

			// -----------------------------------------------------------------------
			// Register state
			// -----------------------------------------------------------------------
			BYTE    writeProtect;       // $F800 write-protect bits (163) / $C000 enable bit (175)
			FLAG    lowChrNtMode;       // $E800 bit 6: YES = $8000-$9FFF ignores $E0-$FF NT redirect
			FLAG    highChrNtMode;      // $E800 bit 7: YES = $A000-$BFFF ignores $E0-$FF NT redirect

			// -----------------------------------------------------------------------
			// IRQ — 15-bit up-counter, bit 15 = enable
			// Increments every CPU cycle when enabled; fires at $7FFF, then stops
			// Acknowledge by writing to $5000 or $5800 (clears IRQ_SRC_NAMECO163)
			// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#IRQ_Operation
			// -----------------------------------------------------------------------
			uint16_t irqCounter;        // [15]=enable, [14:0]=counter

			// -----------------------------------------------------------------------
			// CHR / NT banking — 12 x 1KB banks
			// Slots  0–3  => PPU $0000-$0FFF  (written via $8000-$9FFF, one per $0800 step)
			// Slots  4–7  => PPU $1000-$1FFF  (written via $A000-$BFFF)
			// Slots  8–11 => PPU $2000-$2FFF  (written via $C000-$DFFF = nametables)
			// Value >= 0xE0 with NT-mode active => redirect to CIRAM (bit0=bank select)
			// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#CHR_and_NT_Select
			// -----------------------------------------------------------------------
			BYTE    chrBanks[12];       // raw 8-bit values as written by game

			// -----------------------------------------------------------------------
			// PRG banking — three 8KB switchable banks, one fixed
			// prgBanks[0] = $8000-$9FFF  (written via $E000, bits [5:0])
			// prgBanks[1] = $A000-$BFFF  (written via $E800, bits [5:0])
			// prgBanks[2] = $C000-$DFFF  (written via $F000, bits [5:0])
			// $E000-$FFFF always fixed to last 8KB bank
			// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#Banks
			// -----------------------------------------------------------------------
			BYTE    prgBanks[3];        // 6-bit bank indices

			// -----------------------------------------------------------------------
			// WRAM — 8KB at $6000-$7FFF
			// Namco163: four independently write-protected 2KB windows
			// Namco175: single write-enable bit
			// Namco340: no WRAM (open bus)
			// -----------------------------------------------------------------------
			BYTE    prgRam[0x2000];

			// -----------------------------------------------------------------------
			// N163 Audio — 128-byte internal RAM
			// Lower $40 bytes ($00-$3F): wavetable sample data (4-bit samples, 2/byte)
			// Upper $40 bytes ($40-$7F): 8 channel register sets, 8 bytes each
			//   Channel N base = $40 + N*8
			//   +0 = FreqLow, +1 = PhaseLow, +2 = FreqMid, +3 = PhaseMid
			//   +4 = FreqHigh | WaveLength, +5 = PhaseHigh
			//   +6 = WaveAddress, +7 = Volume (low 4 bits)
			// $7F bits[6:4]: number of extra channels (0=1ch total, 7=8ch total)
			// Ref: https://www.nesdev.org/wiki/Namco_163_audio
			// -----------------------------------------------------------------------
			BYTE    audioRam[0x80];

			// Audio address port state ($F800/$4800)
			BYTE    audioRamAddr;       // current 7-bit pointer into audioRam
			FLAG    audioAutoInc;       // YES = pointer increments on every read/write
			FLAG    audioDisable;       // YES = $E000 bit 6 set; all channels silenced

			// Channel cycling state (one channel updated every 15 CPU cycles)
			// Cycles from channel 7 down to (7 - numExtraChannels)
			// Ref: Mesen Namco163Audio::ClockAudio()
			BYTE    audioUpdateCounter; // 0..14; update fires when it reaches 15
			int8_t  audioCurrentCh;     // channel being updated next (7 down to minCh)

			// Per-channel output, summed in generateNamco163Output()
			// Range per channel: (sample-8)*volume = [-8*15 .. +7*15] = [-120..105]
			int16_t audioChannelOutput[8];

			// Last summed+divided output; read by captureDownsampledAudioSamples()
			// This is the dacInput equivalent, same role as vrc6 pulse/saw dacInput
			NES_AUDIO_SAMPLE_TYPE audioDacInput;

			// -----------------------------------------------------------------------
			// Battery / WRAM tracking
			// -----------------------------------------------------------------------
			FLAG    hasBattery;         // from iNES flag 6 bit 1
		} namco163;
		FLAG isBusConflictPresent;
	} catridgeInfo_t;

	typedef struct
	{
		BYTE maxCatridgePRGROM[0x202000];	// TODO: Do we need this much? if this much is needed, maybe should use a vector
		BYTE maxCatridgeCHRROM[0x202000];	// TODO: Do we need this much? if this much is needed, maybe should use a vector
	} NES_catridgeMemory_t;

	typedef struct
	{
		FLAG startPolling;
		FLAG endPolling;
		SCOUNTER8 keyID;
		MAP8 keyStatus;
	} controller_t;

	typedef struct
	{
		uint16_t frequencyCounter;
		union frequencyPeriod_t
		{
			struct fields_t
			{
				uint16_t lo : 8; // bits 0 - 7
				uint16_t hi : 3; // bits 8 - 10
				uint16_t unused : 5; // bits 11 - 15
			} fields;
			uint16_t raw;
		} frequencyPeriod;
		uint16_t dutyCounter;
		uint16_t lengthCounter;
		struct triangle_t
		{
			FLAG isUltrasonic;
			BYTE triangleStep;
			struct linearCounter_t
			{
				uint16_t linearCounterReload;
				uint16_t linearCounter;
				FLAG linearReload;
			} linearCounter;
		} triangle;
		struct noise_t
		{
			uint16_t noiseFrequencyPeriod;
			union noiseShiftRegister_t
			{
				struct fields_t
				{
					uint16_t zero : 1; // bit 0
					uint16_t one : 1; // bit 1
					uint16_t twoToFive : 4; // bits 2 - 5
					uint16_t six : 1; // bit 6
					uint16_t sevenToFourteen : 8; // bits 7 - 14
					uint16_t fifteen : 1; // bit 15
				} fields;
				uint16_t raw;
			} noiseShiftRegister;
		} noise;
		struct envelope_t
		{
			FLAG startFlag;
			BYTE decayLevelCounter;
			uint16_t divider;
		} envelope;
		struct sweep_t
		{
			FLAG mute;
			FLAG reload;
			uint16_t sweepCounter;
		} sweep;
		struct dmc_t
		{
			FLAG isOutputUnitSilent;
			FLAG isSampleBufferEmpty;
			BYTE bitsInOutputUnit;
			BYTE outputShift;
			BYTE sampleBuffer;
			uint16_t dmcFrequencyPeriod;
			uint16_t dmcSampleAddress;
			uint16_t dmcSampleLength;
			uint16_t dmcAddress;
		} dmc;
		NES_AUDIO_SAMPLE_TYPE dacInput;
	} apuInternalRegisters_t;

	typedef struct
	{
		FLAG isReset;
		INC8 cyclesToSequencerModeChange;
		FLAG skipClockingLengthCounter;
		INC8 lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::TOTAL_AUDIO_CHANNELS)];
		FLAG effectivelengthCounterHaltFlag[TO_UINT8(AUDIO_CHANNELS::TOTAL_AUDIO_CHANNELS)];
		FRAME_SEQUENCER_MODE frameSequencerMode;
		apuInternalRegisters_t apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TOTAL_AUDIO_CHANNELS)];
		//
		BYTE sampleReadByChannel1;
		BYTE sampleReadByChannel2;
		BYTE sampleReadByChannel3;
		BYTE sampleReadByChannel4;
		int32_t downSamplingRatioCounter;
		uint32_t accumulatedTone;
		NES_AUDIO_SAMPLE_TYPE audioBuffer[AUDIO_BUFFER_SIZE_FOR_NES];
		//
		NES_AUDIO_SAMPLE_TYPE LP_In;
		NES_AUDIO_SAMPLE_TYPE LP_Out;
		NES_AUDIO_SAMPLE_TYPE HPA_Prev;
		NES_AUDIO_SAMPLE_TYPE HPA_Out;
		NES_AUDIO_SAMPLE_TYPE HPB_Prev;
		NES_AUDIO_SAMPLE_TYPE HPB_Out;
		//
		float emulatorVolume;
	} audio_t;

	typedef union
	{
		struct
		{
			byte palette : 2; // bits 0 - 1
			byte unused : 3; // bits 2 - 4
			byte priority : 1; // bit 5
			byte flipHorizontally : 1; // bit 6
			byte flipVertically : 1; // bit 7
		} fields;
		byte raw;
	} spriteAttribute_t;

	typedef struct
	{
		FLAG wasVblankJustTriggerred;
		SCOUNTER32 currentScanline;
		struct
		{
			uint16_t nameTblAddr;
			byte nameTblByte;
			uint16_t attrTblAddr;
			byte attrTblByte;
			uint16_t patternTableLAddr;
			uint16_t patternTableMAddr;
			byte patternTblLByte;
			byte patternTblMByte;
			ID paletteID;
			union
			{
				uint8_t loAttrShiftSplit[TWO];
				uint16_t loAttrShift;
			} loAttrShifter;
			union
			{
				uint8_t hiAttrShiftSplit[TWO];
				uint16_t hiAttrShift;
			} hiAttrShifter;
			union
			{
				uint8_t loPatternShiftSplit[TWO];
				uint16_t loPatternShift;
			} loPatternShifter;
			union
			{
				uint8_t hiPatternShiftSplit[TWO];
				uint16_t hiPatternShift;
			} hiPatternShifter;
		} bg;
		struct
		{
			FLAG isSprite0PresentInSecondaryOam;
			byte spriteXCoordinate;
			byte spriteYCoordinate;
			spriteAttribute_t spriteAttribute;
			byte tileNumber;
			uint16_t patternTableLAddr;
			uint16_t patternTableMAddr;
			byte patternTblLByte;
			byte patternTblMByte;
			ID paletteID;
			SCOUNTER32 spriteCountPerScanline;
			struct
			{
				FLAG isSpriteZero;
				uint8_t loPatternShifter;
				uint8_t hiPatternShifter;
				spriteAttribute_t spriteAttribute;
				uint16_t xSubtractor;	// Refer : https://www.reddit.com/r/EmuDev/comments/1diasb2/comment/l92t6gr/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
			} shifter[EIGHT];
		} obj;
		FLAG isOddFrame;
		ID gfxColorID[screen_width][screen_height];
		union
		{
			Pixel imGuiBuffer1D[screen_width * screen_height];
			Pixel imGuiBuffer2D[screen_height][screen_width];
		} imGuiBuffer;
		uint64_t filters;
	} display_t;

	struct debugger_t
	{
		FLAG wasDebuggerJustTriggerred;
		int16_t debuggerTriggerOnWhichLY;
		int64_t lyChangePersistance;
		struct
		{
			uint32_t testCount[TWOFIFTYSIX];
			struct
			{
				uint32_t indexer;
				struct
				{
					FLAG isRead;
					uint8_t data;
					uint16_t address;

					void reset()
					{
						isRead = CLEAR;
						data = RESET;
						address = RESET;
					};
				}cycles[TWENTY];
			} cycles;
		} tomHarte;
	};

	typedef struct
	{
		uint64_t masterCounter;
		uint64_t cpuCounter;
		uint64_t dmaGetPutCounter;
		uint64_t oamDmaCounter;
		uint64_t dmcDmaCounter;
		uint64_t apuSequencer;
		uint64_t apuCounter;
		uint64_t apuFrameCounter;
		uint64_t ppuCounter;
		uint64_t ppuCounterPerLY;
		uint64_t ppuCounterPerFrame;
		uint64_t ppuCounterMMC3A12;
	} ticks_t;

	typedef struct
	{
		ticks_t ticks;
		struct
		{
			TYPE_OF_MEMORY_ACCESS previousAccessType;
			TYPE_OF_MEMORY_ACCESS currentAccessType;
			TYPE_OF_MEMORY_ACCESS previousPPUAccessType;
			TYPE_OF_MEMORY_ACCESS currentPPUAccessType;
			TYPE_OF_MEMORY_ACCESS previousCPUAccessType;
			TYPE_OF_MEMORY_ACCESS currentCPUAccessType;
		} memoryAccessType;
		debugger_t debugger;
	} emulatorStatus_t;

	typedef struct
	{
		FLAG isNMI;
		union isIRQ_t
		{
			struct
			{
				uint16_t IRQ_SRC_DMC : 1;       // bit 0
				uint16_t IRQ_SRC_FRAMECTR : 1;  // bit 1
				uint16_t IRQ_SRC_MMC3 : 1;      // bit 2
				uint16_t IRQ_SRC_MMC5 : 1;      // bit 3
				uint16_t IRQ_SRC_VRC467 : 1;    // bit 4
				uint16_t IRQ_SRC_FME7 : 1;      // bit 5
				uint16_t IRQ_SRC_NAMECO163 : 1; // bit 6
				uint16_t IRQ_SRC_FDS : 1;       // bit 7
				uint16_t IRQ_SRC_INES016 : 1;   // bit 8
				uint16_t IRQ_SRC_SUNSOFT : 1;   // bit 9
				uint16_t IRQ_SRC_MMC1 : 1;      // bit 10
				uint16_t reserved : 5;          // bits 11-15
			} fields;
			uint16_t signal;
		} isIRQ;
		SCOUNTER32 nmiDelayInInstructions;
		SCOUNTER32 irqDelayInInstructions;
		SCOUNTER32 irqDelayInCpuCycles;
		SCOUNTER32 cliDelayInCpuCycles;
		SCOUNTER32 seiDelayInCpuCycles;
		SCOUNTER32 plpDelayInCpuCycles;
		FLAG wasNMI;
		FLAG wasIRQ;
	} interrupts_t;

	typedef struct
	{
		cpu_t cpuInstance;
		cpuRegisters_t cpuRegisters;
		interrupts_t interrupts;
		DMA_t oamDMA;
		DMA_t dmcDMA;
		NES_cpuMemory_t cpuMemory;
		ppuRegisters_t ppuRegisters;
		NES_ppuMemory_t ppuMemory;
		catridgeInfo_t catridgeInfo;
		NES_catridgeMemory_t catridgeMemory;
		controller_t controller;
		audio_t audio;
		display_t display;
		emulatorStatus_t emulatorStatus;
	} NES_state_t;

	typedef union
	{
		NES_state_t NES_state;
		uint8_t NES_memoryState[sizeof(NES_state_t)];
	} NES_instance_t;

	typedef union
	{
		struct
		{
			BYTE constant[FOUR];
			BYTE sizeOfPrgRomIn16KB;
			BYTE sizeOfChrRomIn8KB;
			union
			{
				struct
				{
					BYTE nametableArrangement : 1; // bit  0
					BYTE hasPersistantMemory : 1; // bit  1
					BYTE trainerPresent : 1; // bit  2
					BYTE alternativeNametable : 1; // bit  3
					BYTE mapperLo : 4; // bits  4 - 7
				} fields;
				BYTE raw;
			} flag6;
			union
			{
				struct
				{
					BYTE vsUnisystem : 1; // bit  0
					BYTE playChoice : 1; // bit  1
					BYTE nes2p0 : 2; // bits  2 - 3
					BYTE mapperHi : 4; // bits  4 - 7
				} fields;
				BYTE raw;
			} flag7;
			union
			{
				struct
				{
					BYTE prgRamSize;
					union
					{
						struct
						{
							BYTE tvSystem : 2; // bit  0
							BYTE reserved : 6; // bits  1 - 7
						} fields;
						BYTE raw;
					} flag9;
					union
					{
						struct
						{
							BYTE tvSystem : 2; // bit  0 - 1
							BYTE reserved0 : 2; // bits 2 - 3
							BYTE prgRamNotPresent : 1; // bit  4
							BYTE busConflict : 1; // bit 5
							BYTE reserved1 : 2; // bits 6 - 7
						} fields;
						BYTE raw;
					} flag10;
					BYTE pad[FIVE];
				} ines;
				struct
				{
					union
					{
						struct
						{
							BYTE mapperNBHi : 4; // bits  0 - 3
							BYTE subMapper : 4; // bits 4 - 7
						} fields;
						BYTE raw;
					} flag8;
					union
					{
						struct
						{
							BYTE prgRomMSB : 4; // bits  0 - 3
							BYTE chrRomMSB : 4; // bits 4 - 7
						} fields;
						BYTE raw;
					} flag9;
					union
					{
						struct
						{
							BYTE prgVolRam : 4; // bits  0 - 3 
							BYTE prgNonVolRam : 4; // bits 4 - 7
						} fields;
						BYTE raw;
					} flag10;
					union
					{
						struct
						{
							BYTE chrVolRam : 4; // bits  0 - 3 
							BYTE chrNonVolRam : 4; // bits 4 - 7
						} fields;
						BYTE raw;
					} flag11;
					union
					{
						struct
						{
							BYTE variant : 2; // bits  0 - 1 
							BYTE reserved : 6; // bits 2 - 7
						} fields;
						BYTE raw;
					} flag12;
					union
					{
						struct
						{
							BYTE ppuType : 4; // bits  0 - 3 
							BYTE hwType : 4; // bits 4 - 7
						} fields;
						BYTE raw;
					} flag13;
					union
					{
						struct
						{
							BYTE miscRoms : 2; // bits  0 - 1 
							BYTE reserved : 6; // bits 2 - 7
						} fields;
						BYTE raw;
					} flag14;
					union
					{
						struct
						{
							BYTE expDev : 7; // bits  0 - 6 
							BYTE reserved : 1; // bit 7
						} fields;
						BYTE raw;
					} flag15;
				} nes2p0;
			} flags_8to15;
		} fields;
		BYTE header[SIXTEEN];
	} iNES_header_t;

	typedef union
	{
		struct
		{
			iNES_header_t iNES_header;
			union
			{
				struct
				{
					BYTE trainer[512];
					BYTE romData[NES_MAX_ROM_SIZE - sizeof(iNES_header_t) - sizeof(trainer)];
				} withTrainer;
				struct
				{
					BYTE romData[NES_MAX_ROM_SIZE - sizeof(iNES_header_t)];
				} withoutTrainer;
			} remaining;
		} iNES_Fields;
		BYTE completeROM[NES_MAX_ROM_SIZE];	// Maximum rom size seen so far is 2049 KB
	} iNES_t;

	typedef struct
	{
		FLAG isRomLoaded;
		uint32_t codeRomSize;
		iNES_t iNES;
	} aboutRom_t;

	typedef struct
	{
		// PCB
		uint32_t  mapper;
		uint32_t  subMapper;
		char      mirroring;
		uint8_t   battery;
		// ROM regions
		uint32_t  prgromSize;
		uint32_t  chrromSize;
		uint32_t  prgromCRC32;
		uint32_t  chrromCRC32;
		// RAM / NVRAM
		uint32_t  prgramSize;
		uint32_t  prgnvramSize;
		uint32_t  chrramSize;
		uint32_t  chrnvramSize;
		// Console / expansion
		uint8_t   consoleType;
		uint8_t   consoleRegion;
		uint8_t   expansionType;
		// Misc
		uint8_t   miscRoms;
		uint8_t   vsHardware;
		uint8_t   vsPpu;
		// Lookup key
		uint32_t  romCRC32;
	} NES20DBEntry_t;

	typedef struct
	{
		NES_instance_t NES_instance;
		aboutRom_t aboutRom;
		FLAG enable_nes_db;
		FLAG isNes20dbLoaded;
		NES20DBEntry_t dbEntry;
	} absolute_NES_state_t;

	union absolute_NES_instance_t
	{
		absolute_NES_state_t absolute_NES_state;
		uint8_t NES_absoluteMemoryState[sizeof(absolute_NES_state_t)];
		absolute_NES_instance_t()
		{
			memset(this, 0, sizeof(absolute_NES_instance_t));
		}
	};

	std::shared_ptr <absolute_NES_instance_t> pAbsolute_NES_instance;
	cpuRegisters_t* pNES_cpuRegisters = nullptr;		// for readability
	ppuRegisters_t* pNES_ppuRegisters = nullptr;		// for readability
	cpu_t* pNES_cpuInstance = nullptr;					// for readability
	flagFields_t* pNES_flags = nullptr;					// for readability
	NES_cpuMemory_t* pNES_cpuMemory = nullptr;			// for readability
	NES_ppuMemory_t* pNES_ppuMemory = nullptr;			// for readability
	NES_catridgeMemory_t* pNES_catridgeMemory = nullptr;			// for readability
	NES_instance_t* pNES_instance = nullptr;			// for readability
	iNES_t* pINES = nullptr;							// for readability

PACK_END

private:

	std::unordered_map<uint32_t, NES20DBEntry_t> m_nes20db;

private:

	SDL_AudioStream* audioStream = nullptr;

private:

	Pixel palScreen[0x40];

private:

	void* NESNetworkEngine;

private:

	std::deque<NES_state_t> gamePlay;

#pragma endregion EMULATION_DECLARATIONS

#pragma region INFRASTRUCTURE_DEFINITIONS
public:
	NES_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, MasqConfig_t& config, CheatEngine_t* ce = nullptr);

	~NES_t();

	void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* input = nullptr, void* network = nullptr) override;

	void sendBiosToEmulator(bios_t* bios = nullptr) override {};

	float getVersion();

	uint32_t getScreenWidth() override;

	uint32_t getScreenHeight() override;

	uint32_t getPixelWidth() override;

	uint32_t getPixelHeight() override;

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

#pragma endregion INFRASTRUCTURE_DEFINITIONS

#pragma region RP2C02_DEFINITIONS
private:

	MASQ_INLINE FLAG isVRC2()
	{
		const MAPPER mapper = pNES_instance->NES_state.catridgeInfo.mapper;
		const SUB_MAPPER sub = pNES_instance->NES_state.catridgeInfo.subMapper;

		switch (mapper)
		{
		case VRC2_022:
			RETURN YES; // VRC2a (sub 0)

		case VRC2_VRC4_023:
			RETURN (sub == VRC2B);

		case VRC2_VRC4_025:
			RETURN (sub == VRC2C);

		default:
			RETURN NO;
		}
	}

	MASQ_INLINE FLAG isVRC4()
	{
		const MAPPER mapper = pNES_instance->NES_state.catridgeInfo.mapper;
		const SUB_MAPPER sub = pNES_instance->NES_state.catridgeInfo.subMapper;

		switch (mapper)
		{
		case VRC4_021:
			RETURN (sub == VRC4A || sub == VRC4C);

		case VRC2_VRC4_023:
			RETURN (sub == VRC4E || sub == VRC4F);

		case VRC2_VRC4_025:
			RETURN (sub == VRC4B || sub == VRC4D);

		default:
			RETURN NO;
		}
	}

	MASQ_INLINE FLAG isVRC6()
	{
		const MAPPER mapper = pNES_instance->NES_state.catridgeInfo.mapper;
		RETURN (mapper == VRC6_024 || mapper == VRC6_026);
	}

	MASQ_INLINE FLAG isVRC7()
	{
		// TODO : Yet to be implemented
		RETURN FALSE;
	}

	MASQ_INLINE FLAG isJaleco18()
	{
		RETURN (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_018) ? YES : NO;
	}

	MASQ_INLINE bool isNamco163() const
	{
		const MAPPER m = pNES_instance->NES_state.catridgeInfo.mapper;
		RETURN m == MAPPER::INES_MAPPER_019 || m == MAPPER::INES_MAPPER_210;
	}

	MASQ_INLINE bool isNamco163WithAudio() const
	{
		if (!isNamco163()) RETURN NO;
		// variant 0 = Namco163 only; variant 1/2 have no audio chip
		RETURN pNES_instance->NES_state.catridgeInfo.namco163.variant == 0
			&& pNES_instance->NES_state.catridgeInfo.namco163.audioDisable == NO;
	}

	void clockMMC3IRQ(uint16_t address, MEMORY_ACCESS_SOURCE source, FLAG isWriteOperation);
	
	void updateMMC5ChrA();

	byte readPpuRawMemory(uint16_t address, MEMORY_ACCESS_SOURCE source);

	void writePpuRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source);
#pragma endregion RP2C02_DEFINITIONS

#pragma region RP2A03_DEFINITIONS
private:

	void cpuSetRegister(REGISTER_TYPE rt, uint16_t u16parameter);

	uint16_t cpuReadRegister(REGISTER_TYPE rt);

	MASQ_INLINE void setOpenBus(uint8_t mask, uint8_t value)
	{
		auto& openBus = pNES_ppuRegisters->ppuInternalRegisters.openBus;
		uint64_t now = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounter;

		for (int i = 0; i < 8; i++)
		{
			if (mask & 0x01)
			{
				if (value & 0x01)
				{
					openBus.openBusValue |= (1 << i);
				}
				else
				{
					openBus.openBusValue &= ~(1 << i);
				}
				openBus.openBusDecayStamp[i] = now;
			}
			else if ((now - openBus.openBusDecayStamp[i]) > (3 * NES_PPU_NTSC_FRAME_DOTS)) // 3 frames
			{
				openBus.openBusValue &= ~(1 << i);
			}
			mask >>= 1;
			value >>= 1;
		}
	}

	MASQ_INLINE uint8_t applyOpenBus(uint8_t mask, uint8_t value)
	{
		setOpenBus(~mask, value);
		RETURN value | (pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue & mask);
	}

	// replaces refreshOpenBus
	MASQ_INLINE void refreshOpenBus(uint8_t value, uint8_t mask = 0xFF)
	{
		setOpenBus(mask, value);
	}

	// replaces applyOpenBusDecay (write-only register reads)
	MASQ_INLINE uint8_t applyOpenBusDecay()
	{
		RETURN applyOpenBus(0xFF, 0);
	}

	byte readCpuRawMemory(uint16_t address, MEMORY_ACCESS_SOURCE source);

	inline byte readCpuRawMemoryInternal(uint16_t address, MEMORY_ACCESS_SOURCE source);

	void writeCpuRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source);

	inline void writeCpuRawMemoryInternal(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source);

	void stackPush(BYTE data);

	BYTE stackPop();

	void processUnusedFlags(BYTE result);

	FLAG processSOC();

	MASQ_INLINE FLAG isNMI_SuppressedAtVBLEdge(SCOUNTER32 ly, SCOUNTER64 ppuCycle)
	{
		// Must be right at the VBL set moment � post-render scanline within PPU cycle window
		// Refer 08-nmi_off_timing; This is the method used to achieve the PPU cycle accuracy expected by the test in our emulator where CPU cycle is min resolution
		if (ly != NES_POST_RENDER_SCANLINE || ppuCycle > SIX)
		{
			RETURN NO;
		}
		// VBLANK_NMI_ENABLE was cleared within the timing window � suppress NMI
		RETURN (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.VBLANK_NMI_ENABLE == RESET);
	}

	MASQ_INLINE FLAG isNMI_ReadyToDispatch(SCOUNTER32 ly, SCOUNTER64 ppuCycle)
	{
		// NOTE 1: NMI latency guard � VBL was already set sometime before, delay must have elapsed before we can dispatch
		if (pNES_instance->NES_state.interrupts.nmiDelayInInstructions > RESET)
		{
			RETURN NO;
		}
		// On the post-render scanline, VBL is being set right now � enforce PPU cycle window
		// NOTE 2: This handles for NMI latency assuming VBL is set just few PPU clocks before; This is the method used to achieve the PPU cycle accuracy expected by the test in our emulator where CPU cycle is min resolution
		if (ly == NES_POST_RENDER_SCANLINE)
		{
			RETURN (ppuCycle > SIX);
		}
		// Past the post-render scanline � VBL was already set, NMI dispatch window is open
		RETURN YES;
	}

	EXCEPTION_EVENT_TYPE processNMI();

	EXCEPTION_EVENT_TYPE processIRQ();

	void runCPUPipeline();

	void unimplementedInstruction();
#pragma endregion RP2A03_DEFINITIONS

#pragma region EMULATION_DEFINITIONS
private:

	void clockVRC467IRQ(void);

	void clockJalecoIRQ(void);

	void clockINES016IRQ(void);

	void clockINES067IRQ(void);

	void clockINES069IRQ(void);

	void clockINES105IRQ(void);

	void clockRambo1CpuIRQ();

	void clockNamco163IRQ();

	void clockMMC5(void);

	void cpuTickT(CYCLES_TYPE cycleType);

	void syncOtherGBModuleTicks();

	MASQ_INLINE void resetPPUState()
	{
		// Must be right at the VBL set moment � post-render scanline within PPU cycle window
		// Refer 08-nmi_off_timing; This is the method used to achieve the PPU cycle accuracy expected by the test in our emulator where CPU cycle is min resolution

		pNES_instance->NES_state.display.bg.nameTblAddr = RESET;
		pNES_instance->NES_state.display.bg.nameTblByte = RESET;
		pNES_instance->NES_state.display.bg.attrTblAddr = RESET;
		pNES_instance->NES_state.display.bg.attrTblByte = RESET;
		pNES_instance->NES_state.display.bg.patternTableLAddr = RESET;
		pNES_instance->NES_state.display.bg.patternTblLByte = RESET;
		pNES_instance->NES_state.display.bg.patternTblMByte = RESET;
		pNES_ppuRegisters->pn = RESET;
		pNES_ppuRegisters->pm = RESET;
		pNES_ppuRegisters->sn = RESET;
		pNES_ppuRegisters->sm = RESET;
		pNES_ppuRegisters->oamByte = RESET;
		pNES_ppuRegisters->stopSpriteEvaluation = CLEAR;
		pNES_ppuRegisters->ppuInternalRegisters.ignoreVramRead = RESET;
		pNES_ppuRegisters->ppuInternalRegisters.needVideoRamIncrement = NO;
		pNES_instance->NES_state.display.obj.isSprite0PresentInSecondaryOam = CLEAR;
		pNES_instance->NES_state.display.obj.spriteYCoordinate = RESET;
		pNES_instance->NES_state.display.obj.patternTableLAddr = RESET;
		pNES_instance->NES_state.display.obj.patternTableMAddr = RESET;
		pNES_instance->NES_state.display.obj.patternTblLByte = RESET;
		pNES_instance->NES_state.display.obj.patternTblMByte = RESET;
		pNES_instance->NES_state.display.obj.spriteAttribute.raw = RESET;
		pNES_instance->NES_state.display.obj.spriteXCoordinate = RESET;
		pNES_instance->NES_state.display.obj.tileNumber = RESET;
		pNES_instance->NES_state.display.obj.paletteID = RESET;
	}

	MASQ_INLINE FLAG checkIfRenderring()
	{
		if ((pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET)
			|| (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET))
		{
			RETURN YES;
		}
		else
		{
			RETURN NO;
		}
	}

	MASQ_INLINE void xInc()
	{
		// if coarse X == 31
		if ((pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x001F) == THIRTYONE)
		{
			pNES_ppuRegisters->ppuInternalRegisters.v.raw &= (~0x001F);						// coarse X = 0
			pNES_ppuRegisters->ppuInternalRegisters.v.raw ^= 0x0400;						// switch horizontal nametable
		}
		else
		{
			pNES_ppuRegisters->ppuInternalRegisters.v.raw += ONE;							// increment coarse X
		}
	}

	MASQ_INLINE void yInc()
	{
		// if fine Y < 7
		if ((pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x7000) != 0x7000)
		{
			pNES_ppuRegisters->ppuInternalRegisters.v.raw += 0x1000;						// increment fine Y
		}
		else
		{
			pNES_ppuRegisters->ppuInternalRegisters.v.raw &= (~0x7000);						// fine Y = 0
			uint16_t y = (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x03E0) >> FIVE;	// let y = coarse Y
			if (y == TWENTYNINE)
			{
				y = ZERO;																	// coarse Y = 0
				pNES_ppuRegisters->ppuInternalRegisters.v.raw ^= 0x0800;					// switch vertical nametable
			}
			else if (y == THIRTYONE)
			{
				y = ZERO;																	// coarse Y = 0, nametable not switched
			}
			else
			{
				y += ONE;																	// increment coarse Y
			}
			// put coarse Y back into v
			pNES_ppuRegisters->ppuInternalRegisters.v.raw = (pNES_ppuRegisters->ppuInternalRegisters.v.raw & (~0x03E0)) | (y << FIVE);
		}
	}

	MASQ_INLINE void populatePixelShiftRegisters()
	{
		// Loading the shift registers

		pNES_instance->NES_state.display.bg.loPatternShifter.loPatternShiftSplit[LO]
			= pNES_instance->NES_state.display.bg.patternTblLByte;
		pNES_instance->NES_state.display.bg.hiPatternShifter.hiPatternShiftSplit[LO]
			= pNES_instance->NES_state.display.bg.patternTblMByte;

		// Even though ideally this needs to be 1 bit latch, we will still use 8 bit register and set all bits to same value...
		// NOTE: bit[n] Xly 256 sets all 8 bits to bit[n] 

		pNES_instance->NES_state.display.bg.loAttrShifter.loAttrShiftSplit[LO]
			= GETBIT(LO, pNES_instance->NES_state.display.bg.paletteID) * 0xFF;

		pNES_instance->NES_state.display.bg.hiAttrShifter.hiAttrShiftSplit[LO]
			= GETBIT(HI, pNES_instance->NES_state.display.bg.paletteID) * 0xFF;
	}

	MASQ_INLINE void shiftThePixelShiftRegisters(SCOUNTER64 cycle)
	{
		// Note: Background shift registers should be initialized and clocked when only rendering sprites.
		pNES_instance->NES_state.display.bg.loPatternShifter.loPatternShift <<= ONE;
		pNES_instance->NES_state.display.bg.hiPatternShifter.hiPatternShift <<= ONE;
		pNES_instance->NES_state.display.bg.loAttrShifter.loAttrShift <<= ONE;
		pNES_instance->NES_state.display.bg.hiAttrShifter.hiAttrShift <<= ONE;

		// We check for cycles because we don't want sprite shifter to run during ((cycle >= THREETWENTYONE) && (cycle <= THREETHIRTYSIX))
		if ((cycle >= ONE) && (cycle <= TWOFIFTYSIX))
		{
			for (COUNTER8 spriteI = ZERO; spriteI < pNES_instance->NES_state.display.obj.spriteCountPerScanline; spriteI++)
			{
				if (pNES_instance->NES_state.display.obj.shifter[spriteI].xSubtractor > ZERO)
				{
					pNES_instance->NES_state.display.obj.shifter[spriteI].xSubtractor--;
				}
				else
				{
					pNES_instance->NES_state.display.obj.shifter[spriteI].loPatternShifter <<= ONE;
					pNES_instance->NES_state.display.obj.shifter[spriteI].hiPatternShifter <<= ONE;
				}
			}
		}
	}

	void ppuTick();

	void apuTick();

	void joypadTick();

private:

	void captureIO();

private:

private:

	// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102 for pseudocode
	MASQ_INLINE void tickPulse(AUDIO_CHANNELS channel)
	{
		if ((TO_UINT8(channel) > TO_UINT8(AUDIO_CHANNELS::PULSE_2))
			||
			(TO_UINT8(channel) < TO_UINT8(AUDIO_CHANNELS::PULSE_1)))
		{
			FATAL("Unknown Audio Channel");
		}
		else
		{
			auto& pulseReg = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)];

			if (pulseReg.frequencyCounter > RESET)
			{
				--pulseReg.frequencyCounter;
			}
			else
			{
				pulseReg.frequencyPeriod.fields.unused = RESET;
				pulseReg.frequencyCounter = pulseReg.frequencyPeriod.raw;
				pulseReg.dutyCounter = ((pulseReg.dutyCounter + ONE) & SEVEN);
			}
		}
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163155#p163155 for pseudocode
	MASQ_INLINE void tickTriangle()
	{
		auto& triReg = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)];
		auto& tri = triReg.triangle;

		tri.isUltrasonic = NO;

		triReg.frequencyPeriod.fields.unused = RESET;
		if ((triReg.frequencyPeriod.raw < TWO) && (triReg.frequencyCounter == RESET))
		{
			tri.isUltrasonic = YES;
		}

		const FLAG clockTriangle = ((triReg.lengthCounter != RESET)
			&& (tri.linearCounter.linearCounter != RESET)
			&& (tri.isUltrasonic != YES)) ? YES : NO;

		if (clockTriangle == YES)
		{
			if (triReg.frequencyCounter > RESET)
			{
				--triReg.frequencyCounter;
			}
			else
			{
				triReg.frequencyPeriod.fields.unused = ZERO;
				triReg.frequencyCounter = triReg.frequencyPeriod.raw;
				tri.triangleStep = ((tri.triangleStep + ONE) & 0x1F);
			}
		}
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163157#p163157 for pseudocode
	MASQ_INLINE void tickNoise()
	{
		auto& noiseReg = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)];
		auto& noise = noiseReg.noise;
		auto& shiftReg = noise.noiseShiftRegister;

		if (noiseReg.frequencyCounter > RESET)
		{
			--noiseReg.frequencyCounter;
		}
		else
		{
			noiseReg.frequencyCounter = noise.noiseFrequencyPeriod;

			shiftReg.fields.fifteen = (pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_PERIOD.LOOP_NOISE == SET)
				? (shiftReg.fields.six ^ shiftReg.fields.zero)
				: (shiftReg.fields.one ^ shiftReg.fields.zero);

			shiftReg.raw >>= ONE;
		}
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163187&sid=4e4aade9cf7e18093eb1fd3afad14c49#p163187 for pseudocode
	MASQ_INLINE void tickDMC()
	{
		// Cache references to avoid repeated deep dereferences
		auto& dmcReg = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)];
		auto& dmc = dmcReg.dmc;
		auto& apuIO = pNES_cpuMemory->NESMemoryMap.apuAndIO;

		if (dmcReg.frequencyCounter > RESET)
		{
			--dmcReg.frequencyCounter;
		}
		else
		{
			dmcReg.frequencyCounter = dmc.dmcFrequencyPeriod;

			if (dmc.isOutputUnitSilent == NO)
			{
				const auto outputShift = dmc.outputShift;

				if ((outputShift & 0x01) && (apuIO.DMC_RAW.DIRECT_LOAD < 0x7E))
				{
					apuIO.DMC_RAW.DIRECT_LOAD += TWO;
				}
				else if (!(outputShift & 0x01) && (apuIO.DMC_RAW.DIRECT_LOAD > 0x01))
				{
					apuIO.DMC_RAW.DIRECT_LOAD -= TWO;
				}
			}

			--dmc.bitsInOutputUnit;
			dmc.outputShift >>= ONE;

			if (dmc.bitsInOutputUnit == RESET)
			{
				dmc.bitsInOutputUnit = EIGHT;
				dmc.outputShift = dmc.sampleBuffer;
				dmc.isOutputUnitSilent = dmc.isSampleBufferEmpty;
				dmc.isSampleBufferEmpty = YES;
			}
		}

		if ((dmcReg.lengthCounter > RESET) && (dmc.isSampleBufferEmpty == YES))
		{
			pNES_instance->NES_state.emulatorStatus.ticks.dmcDmaCounter = RESET;

			APUTODO("Emulate the CPU halt cycles during the below DMC DMA read operation");

			dmc.sampleBuffer = readCpuRawMemory(dmc.dmcAddress, MEMORY_ACCESS_SOURCE::DMA);
			dmc.isSampleBufferEmpty = NO;
			dmc.dmcAddress = ((dmc.dmcAddress + ONE) & 0x7FFF) | 0x8000;

			if (--dmcReg.lengthCounter == RESET)
			{
				if (apuIO.DMC_FREQ.LOOP_SAMPLE == SET)
				{
					dmcReg.lengthCounter = dmc.dmcSampleLength;
					dmc.dmcAddress = dmc.dmcSampleAddress;
					pNES_instance->NES_state.dmcDMA.sourceAddress = dmc.dmcSampleAddress;
				}
				else if (apuIO.DMC_FREQ.IRQ_ENABLE == SET)
				{
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_DMC = SET;
				}
			}
		}
	}

	// For MMC5
	MASQ_INLINE void tickMMC5Pulse(uint8_t idx)
	{
		auto& p = pNES_instance->NES_state.catridgeInfo.mmc5.mmc5Audio.pulse[idx];

		if (p.frequencyCounter > 0)
		{
			p.frequencyCounter--;
		}
		else
		{
			p.frequencyPeriod.fields.unused = 0;
			p.frequencyCounter = p.frequencyPeriod.raw;
			p.dutyCounter = (p.dutyCounter + 1) & 0x07;
		}
	}

	// -----------------------------------------------------------------------
	// clockMMC5FrameSequencer
	// Must be called BEFORE ++apuSequencer so it reads the same step value
	// the NES channels just processed. Mirrors 4-step and 5-step modes.
	// -----------------------------------------------------------------------
	MASQ_INLINE void clockMMC5FrameSequencer()
	{
		const auto seq = pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer;
		auto& audio = pNES_instance->NES_state.catridgeInfo.mmc5.mmc5Audio;

		bool isHalfFrame = false;
		bool isQuarterFrame = false;

		if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FOUR_STEP_MODE)
		{
			// Quarter frame: steps 0, 1, 2, 3
			// Half frame:    steps 1, 3
			isQuarterFrame =
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M00)) ||
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01)) ||
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M02)) ||
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03));

			isHalfFrame =
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01)) ||
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03));
		}
		else // FIVE_STEP_MODE
		{
			// Quarter frame: steps 0, 1, 2, 3  (step 4 is silent — no clocking)
			// Half frame:    steps 0, 2
			isQuarterFrame =
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10)) ||
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M11)) ||
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12)) ||
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M13));

			isHalfFrame =
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10)) ||
				(seq == TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12));
		}

		for (uint8_t i = 0; i < 2; i++)
		{
			auto& p = audio.pulse[i];

			// Quarter frame: clock envelope
			if (isQuarterFrame)
			{
				if (p.envelope.startFlag == YES)
				{
					p.envelope.startFlag = NO;
					p.envelope.decayLevelCounter = 15;
					p.envelope.divider = p.envelopePeriod;
				}
				else
				{
					if (p.envelope.divider == 0)
					{
						p.envelope.divider = p.envelopePeriod;
						if (p.envelope.decayLevelCounter == 0)
						{
							if (p.envelopeLoop == YES)
							{
								p.envelope.decayLevelCounter = 15;
							}
						}
						else
						{
							p.envelope.decayLevelCounter--;
						}
					}
					else
					{
						p.envelope.divider--;
					}
				}
			}

			// Half frame: clock length counter
			if (isHalfFrame)
			{
				if (p.lengthCounter > 0 && p.envelopeLoop == NO)
				{
					p.lengthCounter--;
				}
			}
		}
	}

	MASQ_INLINE void generateMMC5PulseOutput(uint8_t idx)
	{
		static const uint8_t DUTY_TABLE[4][8] = {
			{ 0, 1, 0, 0, 0, 0, 0, 0 }, // 12.5%
			{ 0, 1, 1, 0, 0, 0, 0, 0 }, // 25%
			{ 0, 1, 1, 1, 1, 0, 0, 0 }, // 50%
			{ 1, 0, 0, 1, 1, 1, 1, 1 }, // 75% (negated 25%)
		};

		auto& p = pNES_instance->NES_state.catridgeInfo.mmc5.mmc5Audio.pulse[idx];

		// MMC5 pulse: no sweep muting, no frequency < 8 muting (unlike NES)
		// Only muted when length counter == 0 or duty output is 0
		if (p.lengthCounter == 0 || DUTY_TABLE[p.duty][p.dutyCounter] == 0)
		{
			p.dacInput = MUTE_AUDIO;
			return;
		}

		p.dacInput = static_cast<NES_AUDIO_SAMPLE_TYPE>(
			p.constantVolume == YES ? p.envelopePeriod : p.envelope.decayLevelCounter
			);
	}

	// -----------------------------------------------------------------------
	// VRC6 Audio — Register Write
	// Refer: https://www.nesdev.org/wiki/VRC6_audio
	// Called from CPU write handler for $9000-$9002, $A000-$A002, $B000-$B002, $9003
	// baseRange = address & 0xF000, regLine = decoded 0..2 (after mapper-26 swap)
	// -----------------------------------------------------------------------
	MASQ_INLINE void writeVRC6Audio(uint16_t baseRange, uint8_t regLine, uint8_t data, uint16_t rawAddress)
	{
		auto& aud = pNES_instance->NES_state.catridgeInfo.vrc6.vrc6Audio;

		// $9003: Global halt/freq-shift — check raw address directly (pre-swap)
		// On mapper 26 the decoded regLine is wrong for this register
		if ((rawAddress & 0xF003) == 0x9003)
		{
			aud.globalHalt = (data & 0x01) ? YES : NO;
			aud.freqShift8 = (data & 0x02) ? YES : NO;
			aud.freqShift4 = (data & 0x04) ? YES : NO;
			const FLAG en = (aud.globalHalt == NO) ? YES : NO;
			aud.pulse[0].enabled = en;
			aud.pulse[1].enabled = en;
			aud.sawtooth.enabled = en;
			RETURN;
		}

		// Pulse 1 ($9000-$9002) and Pulse 2 ($A000-$A002)
		if (baseRange == 0x9000 || baseRange == 0xA000)
		{
			const uint8_t idx = (baseRange == 0x9000) ? 0 : 1;
			auto& p = aud.pulse[idx];

			switch (regLine)
			{
			case 0: // $x000: duty[6:4], volume[3:0]
				p.volume = data & 0x0F;
				p.duty = (data >> 4) & 0x07;
				BREAK;
			case 1: // $x001: freq low byte
				p.freqPeriod = (p.freqPeriod & 0x0F00) | data;
				BREAK;
			case 2: // $x002: halt[7], freq high[3:0]
				p.halt = (data & 0x80) ? YES : NO;
				p.freqPeriod = (p.freqPeriod & 0x00FF) | (static_cast<uint16_t>(data & 0x0F) << 8);
				BREAK;
			default:
				BREAK;
			}
			RETURN;
		}

		// Sawtooth ($B000-$B002)
		if (baseRange == 0xB000)
		{
			auto& s = aud.sawtooth;

			switch (regLine)
			{
			case 0: // $B000: accum rate[5:0]
				s.accumRate = data & 0x3F;
				BREAK;
			case 1: // $B001: freq low byte
				s.freqPeriod = (s.freqPeriod & 0x0F00) | data;
				BREAK;
			case 2: // $B002: halt[7], freq high[3:0]
				s.halt = (data & 0x80) ? YES : NO;
				s.freqPeriod = (s.freqPeriod & 0x00FF) | (static_cast<uint16_t>(data & 0x0F) << 8);
				BREAK;
			default:
				BREAK;
			}
			RETURN;
		}
	}

	// -----------------------------------------------------------------------
	// VRC6 Audio — Timer Tick (call once per CPU cycle from clockVRC6Audio)
	// Refer: https://www.nesdev.org/wiki/VRC6_audio
	// -----------------------------------------------------------------------
	MASQ_INLINE void tickVRC6Audio()
	{
		if (!isVRC6())
		{
			RETURN;
		}

		auto& aud = pNES_instance->NES_state.catridgeInfo.vrc6.vrc6Audio;

		if (aud.globalHalt == YES)
		{
			RETURN;
		}

		// --- Pulse channels ---
		for (uint8_t i = 0; i < 2; i++)
		{
			auto& p = aud.pulse[i];

			if (p.halt == YES)
			{
				CONTINUE;
			}

			// Apply frequency shift (divides effective period)
			uint16_t effectivePeriod = p.freqPeriod;
			if (aud.freqShift8 == YES)       effectivePeriod >>= 8;
			else if (aud.freqShift4 == YES)  effectivePeriod >>= 4;

			if (p.freqCounter > ZERO)
			{
				--p.freqCounter;
			}
			else
			{
				p.freqCounter = effectivePeriod;
				p.dutyCounter = (p.dutyCounter + ONE) & 0x0F; // 0..15
			}
		}

		// --- Sawtooth channel ---
		{
			auto& s = aud.sawtooth;

			if (s.halt == NO)
			{
				uint16_t effectivePeriod = s.freqPeriod;
				if (aud.freqShift8 == YES)       effectivePeriod >>= 8;
				else if (aud.freqShift4 == YES)  effectivePeriod >>= 4;

				if (s.freqCounter > ZERO)
				{
					--s.freqCounter;
				}
				else
				{
					s.freqCounter = effectivePeriod;
					++s.accumClock;

					// Accumulator is added on even clocks (clocks 2, 4, 6)
					if ((s.accumClock & 0x01) == 0)
					{
						s.accumulator += s.accumRate;
					}

					// Reset on clock 7
					if (s.accumClock >= 7)
					{
						s.accumClock = ZERO;
						s.accumulator = ZERO;
					}
				}
			}
		}
	}

	// -----------------------------------------------------------------------
	// VRC6 Audio — DAC output snapshot (call after tickVRC6Audio each CPU cycle)
	// -----------------------------------------------------------------------
	MASQ_INLINE void generateVRC6AudioOutput()
	{
		auto& aud = pNES_instance->NES_state.catridgeInfo.vrc6.vrc6Audio;

		// --- Pulse 1 and Pulse 2 ---
		for (uint8_t i = 0; i < 2; i++)
		{
			auto& p = aud.pulse[i];

			if (p.halt == YES || aud.globalHalt == YES)
			{
				p.dacInput = MUTE_AUDIO;
				CONTINUE;
			}

			// duty == 7: always output full volume (ignore duty sequencer)
			// Otherwise: output if dutyCounter <= duty threshold
			// dutyCounter cycles 0..15; a duty value of N means high for N+1 out of 16 steps
			const FLAG high = (p.duty == 7) ? YES : (p.dutyCounter <= p.duty) ? YES : NO;

			p.dacInput = (high == YES)
				? static_cast<NES_AUDIO_SAMPLE_TYPE>(p.volume)
				: MUTE_AUDIO;
		}

		// --- Sawtooth ---
		{
			auto& s = aud.sawtooth;

			if (s.halt == YES || aud.globalHalt == YES)
			{
				s.dacInput = MUTE_AUDIO;
			}
			else
			{
				// Upper 5 bits of the 8-bit accumulator are the output (0..31)
				s.dacInput = static_cast<NES_AUDIO_SAMPLE_TYPE>(s.accumulator >> 3);
			}
		}
	}

	// -----------------------------------------------------------------------
	// NAMCO Audio
	// -----------------------------------------------------------------------
	MASQ_INLINE void tickNamco163Audio()
	{
		auto& n163 = pNES_instance->NES_state.catridgeInfo.namco163;

		n163.audioUpdateCounter++;
		if (n163.audioUpdateCounter < 15)
			RETURN;

		n163.audioUpdateCounter = 0;

		const int ch = (int)n163.audioCurrentCh;

		// Channel register base in audioRam: ch7 = $78, ch6 = $70, ch0 = $40
		const uint8_t base = (uint8_t)(0x40 + ch * 0x08);

		// Read channel parameters
		// +0=FreqLow +2=FreqMid +4=FreqHigh|WaveLen +1=PhaseLow +3=PhaseMid +5=PhaseHigh
		// +6=WaveAddr +7=Volume
		const uint32_t freq = ((uint32_t)(n163.audioRam[base + 4] & 0x03) << 16)
			| ((uint32_t)n163.audioRam[base + 2] << 8)
			| n163.audioRam[base + 0];

		uint32_t phase = ((uint32_t)n163.audioRam[base + 5] << 16)
			| ((uint32_t)n163.audioRam[base + 3] << 8)
			| n163.audioRam[base + 1];

		// waveLength = 256 - (reg[+4] & 0xFC)
		// Ref: https://www.nesdev.org/wiki/Namco_163_audio#Sound_RAM
		const uint16_t waveLen = 256u - (uint16_t)(n163.audioRam[base + 4] & 0xFC);
		const uint8_t  waveAddr = n163.audioRam[base + 6];
		const uint8_t  volume = n163.audioRam[base + 7] & 0x0F;

		// Advance phase accumulator
		// Ref: https://www.nesdev.org/wiki/Namco_163_audio#Channel_Update pseudocode
		phase = (phase + freq) % ((uint32_t)waveLen << 16);

		// Sample lookup: 4-bit samples packed 2-per-byte, little-endian nibbles
		// samplePosition = ((phase >> 16) + waveAddr) & 0xFF
		const uint8_t samplePos = (uint8_t)(((phase >> 16) + waveAddr) & 0xFF);
		int8_t sample;
		if (samplePos & 0x01)
			sample = (int8_t)(n163.audioRam[samplePos / 2] >> 4);
		else
			sample = (int8_t)(n163.audioRam[samplePos / 2] & 0x0F);

		// Output = (sample - 8) * volume; range [-120, 105]
		n163.audioChannelOutput[ch] = (int16_t)(sample - 8) * (int16_t)volume;

		// Write phase back into audioRam
		n163.audioRam[base + 5] = (uint8_t)((phase >> 16) & 0xFF);
		n163.audioRam[base + 3] = (uint8_t)((phase >> 8) & 0xFF);
		n163.audioRam[base + 1] = (uint8_t)(phase & 0xFF);

		// Advance to next channel
		// numCh = (audioRam[$7F] >> 4) & 7: 0=1 channel, 7=8 channels
		const uint8_t numCh = (n163.audioRam[0x7F] >> 4) & 0x07;
		const int8_t  minCh = (int8_t)(7 - (int8_t)numCh);

		n163.audioCurrentCh--;
		if (n163.audioCurrentCh < minCh)
			n163.audioCurrentCh = 7;
	}

	MASQ_INLINE void generateNamco163Output()
	{
		auto& n163 = pNES_instance->NES_state.catridgeInfo.namco163;

		// numCh = (audioRam[$7F] >> 4) & 7; 0=1ch, 7=8ch
		const uint8_t numCh = (n163.audioRam[0x7F] >> 4) & 0x07;
		const int8_t  minCh = (int8_t)(7 - (int8_t)numCh);

		// Sum all active channels then divide by (numCh+1)
		// Ref: Mesen Namco163Audio::UpdateOutputLevel()
		int16_t sumOutput = 0;
		for (int i = 7; i >= (int)minCh; i--)
			sumOutput += n163.audioChannelOutput[i];

		sumOutput /= (int16_t)(numCh + 1);

		n163.audioDacInput = static_cast<NES_AUDIO_SAMPLE_TYPE>(sumOutput);
	}

private:

	MASQ_INLINE void processLengthCounter(AUDIO_CHANNELS channel)
	{
		if ((TO_UINT8(channel) >= TO_UINT8(AUDIO_CHANNELS::TOTAL_AUDIO_CHANNELS))
			||
			(TO_UINT8(channel) < TO_UINT8(AUDIO_CHANNELS::PULSE_1)))
		{
			FATAL("Unknown Audio Channel");
		}
		else
		{
			auto& chanReg = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)];

			// If length counter is non zero and halt flag is not set, decrement the length counter
			if (chanReg.lengthCounter > ZERO)
			{
				const auto isHaltEn = (pNES_instance->NES_state.audio.effectivelengthCounterHaltFlag[TO_UINT8(channel)] == SET);

				if (isHaltEn == NO && pNES_instance->NES_state.audio.skipClockingLengthCounter == NO)
				{
					--chanReg.lengthCounter;
				}

				pNES_instance->NES_state.audio.skipClockingLengthCounter = CLEAR;
			}
		}
	}

	MASQ_INLINE void processEnvelope(AUDIO_CHANNELS channel)
	{
		auto& chanReg = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)];
		auto& envelope = chanReg.envelope;
		auto& apuIO = pNES_cpuMemory->NESMemoryMap.apuAndIO;

		auto reloadDivider = [&](AUDIO_CHANNELS channel)
			{
				switch (channel)
				{
				case AUDIO_CHANNELS::PULSE_1:
				{
					envelope.divider = apuIO.SQ1_VOL.ENV_PERIOD_OR_VOL;
					BREAK;
				}
				case AUDIO_CHANNELS::PULSE_2:
				{
					envelope.divider = apuIO.SQ2_VOL.ENV_PERIOD_OR_VOL;
					BREAK;
				}
				case AUDIO_CHANNELS::NOISE:
				{
					envelope.divider = apuIO.NOISE_VOL.ENV_PERIOD_OR_VOL;
					BREAK;
				}
				default:
				{
					FATAL("Envelope is not supported in Channel : %d", TO_UINT8(channel));
				}
				}
			};

		if (envelope.startFlag == YES)
		{
			envelope.startFlag = NO;
			envelope.decayLevelCounter = FIFTEEN;
			reloadDivider(channel);
		}
		else
		{
			if (envelope.divider == RESET)
			{
				reloadDivider(channel);
				if (envelope.decayLevelCounter == RESET)
				{
					switch (channel)
					{
					case AUDIO_CHANNELS::PULSE_1:
					{
						if (apuIO.SQ1_VOL.LOOP_ENV_OR_DIS_LENGTH_COUNTER == SET)
						{
							envelope.decayLevelCounter = FIFTEEN;
						}
						BREAK;
					}
					case AUDIO_CHANNELS::PULSE_2:
					{
						if (apuIO.SQ2_VOL.LOOP_ENV_OR_DIS_LENGTH_COUNTER == SET)
						{
							envelope.decayLevelCounter = FIFTEEN;
						}
						BREAK;
					}
					case AUDIO_CHANNELS::NOISE:
					{
						if (apuIO.NOISE_VOL.LOOP_ENV_OR_DIS_LENGTH_COUNTER == SET)
						{
							envelope.decayLevelCounter = FIFTEEN;
						}
						BREAK;
					}
					default:
					{
						FATAL("Envelope is not supported in Channel : %d", TO_UINT8(channel));
					}
					}
				}
				else
				{
					--envelope.decayLevelCounter;
				}
			}
			else
			{
				--envelope.divider;
			}
		}
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102 for pseudocode
	MASQ_INLINE FLAG isSweepMutingChannel(AUDIO_CHANNELS channel, BYTE shift, FLAG negate)
	{
		FLAG isMute = NO;

		if ((TO_UINT8(channel) >= TO_UINT8(AUDIO_CHANNELS::TOTAL_AUDIO_CHANNELS))
			||
			(TO_UINT8(channel) < TO_UINT8(AUDIO_CHANNELS::PULSE_1)))
		{
			FATAL("Unknown Audio Channel");
			RETURN isMute;
		}
		else
		{
			auto& freqPeriod = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod;

			freqPeriod.fields.unused = RESET;
			const auto upperBound = freqPeriod.raw + (freqPeriod.raw >> shift);
			freqPeriod.fields.unused = RESET;

			if (freqPeriod.raw < EIGHT)
			{
				isMute = YES;
			}
			else if ((negate == NO) && (upperBound >= 0x0800))
			{
				isMute = YES;
			}
			else
			{
				isMute = NO;
			}
		}

		RETURN isMute;
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102 for pseudocode
	MASQ_INLINE void processSweep(AUDIO_CHANNELS channel)
	{
		auto sweep = [&](AUDIO_CHANNELS channel, FLAG enable, BYTE shift, BYTE period, FLAG negate)
			{
				if ((TO_UINT8(channel) >= TO_UINT8(AUDIO_CHANNELS::TOTAL_AUDIO_CHANNELS))
					||
					(TO_UINT8(channel) < TO_UINT8(AUDIO_CHANNELS::PULSE_1)))
				{
					FATAL("Unknown Audio Channel");
					RETURN;
				}

				auto& chanReg = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)];
				auto& sweep = chanReg.sweep;
				auto& freqPeriod = chanReg.frequencyPeriod;

				if (sweep.reload == YES)
				{
					sweep.sweepCounter = period;
					APUTODO("Handle Sweep Reload Edge Case at %d in %s", __LINE__, __FILE__);
					sweep.reload = NO;
				}
				else if (sweep.sweepCounter > RESET)
				{
					--sweep.sweepCounter;
				}
				else
				{
					sweep.sweepCounter = period;
					if ((enable == YES) && (isSweepMutingChannel(channel, shift, negate) == NO))
					{
						freqPeriod.fields.unused = RESET;
						if (negate == YES)
						{
							if (channel == AUDIO_CHANNELS::PULSE_1)
							{
								freqPeriod.raw -= ((freqPeriod.raw >> shift) + ONE);
							}
							else if (channel == AUDIO_CHANNELS::PULSE_2)
							{
								freqPeriod.raw -= (freqPeriod.raw >> shift);
							}
							else
							{
								FATAL("Sweep not supported for channel %d", TO_UINT8(channel));
							}
						}
						else
						{
							freqPeriod.raw += (freqPeriod.raw >> shift);
						}
						freqPeriod.fields.unused = RESET;
					}
				}
			};

		switch (channel)
		{
		case AUDIO_CHANNELS::PULSE_1:
		{
			auto& sq1 = pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_SWEEP;
			sweep(
				channel
				, (FLAG)((sq1.ENABLE == SET) && (sq1.SHIFT_COUNT != RESET))
				, sq1.SHIFT_COUNT
				, sq1.PERIOD
				, sq1.NEGATIVE
			);
			BREAK;
		}
		case AUDIO_CHANNELS::PULSE_2:
		{
			auto& sq2 = pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_SWEEP;
			sweep(
				channel
				, (FLAG)((sq2.ENABLE == SET) && (sq2.SHIFT_COUNT != RESET))
				, sq2.SHIFT_COUNT
				, sq2.PERIOD
				, sq2.NEGATIVE
			);
			BREAK;
		}
		default:
		{
			FATAL("Sweep is not supported for %u", TO_UINT(channel));
		}
		}
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163155#p163155 for pseudocode
	MASQ_INLINE void processLinearCounter()
	{
		auto& triReg = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)];
		auto& linCtr = triReg.triangle.linearCounter;

		if (linCtr.linearReload == YES)
		{
			linCtr.linearCounter = linCtr.linearCounterReload;
		}
		else if (linCtr.linearCounter > RESET)
		{
			--linCtr.linearCounter;
		}

		if (pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_LINEAR.DIS_LENGTH_COUNTER == RESET)
		{
			linCtr.linearReload = NO;
		}
	}

private:

	// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102 for pulse part of the pseudocode
	// Refer https://forums.nesdev.org/viewtopic.php?p=163155#p163155 for triangle part of the pseudocode
	// Refer https://forums.nesdev.org/viewtopic.php?p=163157#p163157 for noise part of the pseudocode
	MASQ_INLINE void generateLogicalOutput(AUDIO_CHANNELS channel)
	{
		NES_AUDIO_SAMPLE_TYPE sample = MUTE_AUDIO;
		auto& chanReg = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)];
		auto& apuIO = pNES_cpuMemory->NESMemoryMap.apuAndIO;

		switch (channel)
		{
		case AUDIO_CHANNELS::PULSE_1:
		{
			if ((PULSE_AMPLITUDE_LUT[apuIO.SQ1_VOL.DUTY][chanReg.dutyCounter] == HI)
				&& (chanReg.lengthCounter != RESET)
				&& (isSweepMutingChannel(channel, apuIO.SQ1_SWEEP.SHIFT_COUNT, apuIO.SQ1_SWEEP.NEGATIVE) == NO))
			{
				sample = static_cast<float>(apuIO.SQ1_VOL.CONSTANT_VOL == SET
					? apuIO.SQ1_VOL.ENV_PERIOD_OR_VOL
					: chanReg.envelope.decayLevelCounter);
			}
			chanReg.dacInput = sample;
			BREAK;
		}
		case AUDIO_CHANNELS::PULSE_2:
		{
			if ((PULSE_AMPLITUDE_LUT[apuIO.SQ2_VOL.DUTY][chanReg.dutyCounter] == HI)
				&& (chanReg.lengthCounter != RESET)
				&& (isSweepMutingChannel(channel, apuIO.SQ2_SWEEP.SHIFT_COUNT, apuIO.SQ2_SWEEP.NEGATIVE) == NO))
			{
				sample = static_cast<float>(apuIO.SQ2_VOL.CONSTANT_VOL == SET
					? apuIO.SQ2_VOL.ENV_PERIOD_OR_VOL
					: chanReg.envelope.decayLevelCounter);
			}
			chanReg.dacInput = sample;
			BREAK;
		}
		case AUDIO_CHANNELS::TRIANGLE:
		{
			auto& tri = chanReg.triangle;
			if (tri.isUltrasonic == YES)
			{
				sample = 7.5f;
			}
			else if (tri.triangleStep & 0x10)
			{
				sample = static_cast<float>(tri.triangleStep ^ 0x1F);
			}
			else
			{
				sample = static_cast<float>(tri.triangleStep);
			}
			chanReg.dacInput = sample;
			BREAK;
		}
		case AUDIO_CHANNELS::NOISE:
		{
			if ((chanReg.noise.noiseShiftRegister.fields.zero == RESET)
				&& (chanReg.lengthCounter != RESET))
			{
				sample = static_cast<float>(apuIO.NOISE_VOL.CONSTANT_VOL == SET
					? apuIO.NOISE_VOL.ENV_PERIOD_OR_VOL
					: chanReg.envelope.decayLevelCounter);
			}
			chanReg.dacInput = sample;
			BREAK;
		}
		case AUDIO_CHANNELS::DMC:
		{
			chanReg.dacInput = static_cast<float>(apuIO.DMC_RAW.DIRECT_LOAD);
			BREAK;
		}
		default:
		{
			FATAL("Unknown Audio Channel");
		}
		}
	}

private:

	void captureDownsampledAudioSamples();

	void playTheAudioFrame();

private:

	// NOTE: This function is used only when we a have a need to restore the graphics from scratch, for example load/save states
	void displayCompleteScreen();

private:

	void loadConfig();

	void loadQuirks();

public:

	FLAG saveState(uint8_t = 0) override;

	FLAG loadState(uint8_t = 0) override;

	FLAG absoluteSaveState(uint8_t id);

	FLAG absoluteLoadState(uint8_t id);

	FLAG fillGamePlayStack() override;

	FLAG rewindGamePlay() override;

private:

	FLAG loadNESDB(const std::string& jsonPath);

	FLAG lookupNESDB(uint32_t romCRC32);

private:

	FLAG runEmulationAtHostRate(uint32_t currentFrame) override;

	FLAG runEmulationLoopAtHostRate(uint32_t currentFrame) override;

	FLAG runEmulationAtFixedRate(uint32_t currentFrame) override;

	FLAG runEmulationLoopAtFixedRate(uint32_t currentFrame) override;

	FLAG onKeyEvent(EmuKey key, EmuKeyAction action) override
	{
		RETURN YES;
	}

public:

	float getEmulationVolume() override;

	void setEmulationVolume(float volume) override;

private:

	void initializeGraphics();

	void initializeAudio();

	void reInitializeAudio();

private:

	FLAG initializeEmulator() override;

	void destroyEmulator() override;

public:

	FLAG getRomLoadedStatus() override;

	FLAG loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) override;

	void dumpRom() override;
#pragma endregion EMULATION_DEFINITIONS
};
#pragma endregion CORE