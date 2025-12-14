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
#define RAM_START_ADDRESS								(0x0000)
#define ZERO_PAGE_START_ADDRESS							(RAM_START_ADDRESS)
#define ZERO_PAGE_END_ADDRESS							(ZERO_PAGE_START_ADDRESS + 0xFF)
#define STACK_START_ADDRESS								(ZERO_PAGE_END_ADDRESS + 1)
#define STACK_END_ADDRESS								(STACK_START_ADDRESS + 0xFF)
#define RAM_END_ADDRESS									(RAM_START_ADDRESS + 0x07FF)
#define RAM_MIRROR1_START_ADDRESS						(RAM_END_ADDRESS + 1)
#define CPU_RAM_SIZE									(RAM_END_ADDRESS + 1 - RAM_START_ADDRESS)
#define RAM_MIRROR1_END_ADDRESS							(RAM_MIRROR1_START_ADDRESS + 0x07FF)
#define RAM_MIRROR2_START_ADDRESS						(RAM_MIRROR1_END_ADDRESS + 1)
#define RAM_MIRROR2_END_ADDRESS							(RAM_MIRROR2_START_ADDRESS + 0x07FF)
#define RAM_MIRROR3_START_ADDRESS						(RAM_MIRROR2_END_ADDRESS + 1)
#define RAM_MIRROR3_END_ADDRESS							(RAM_MIRROR3_START_ADDRESS + 0x07FF)
#define PPU_START_ADDRESS								(RAM_MIRROR3_END_ADDRESS + 1)
#define PPU_CTRL_ADDRESS								(0x2000)
#define PPU_MASK_ADDRESS								(0x2001)
#define PPU_STATUS_ADDRESS								(0x2002)
#define OAM_ADDR_ADDRESS								(0x2003)
#define OAM_DATA_ADDRESS								(0x2004)
#define PPU_SCROLL_ADDRESS								(0x2005)
#define PPU_ADDR_ADDRESS								(0x2006)
#define PPU_DATA_ADDRESS								(0x2007)
#define PPU_END_ADDRESS									(PPU_START_ADDRESS + 0x0007)
#define PPU_CTRL_REG_SIZE								(PPU_END_ADDRESS + 1 - PPU_START_ADDRESS)
#define PPU_MIRROR_START_ADDRESS						(PPU_END_ADDRESS + 1)
#define PPU_MIRROR_END_ADDRESS							(0x3FFF)
#define APU_AND_IO_START_ADDRESS						(PPU_MIRROR_END_ADDRESS + 1)
#define OAM_DMA_ADDRESS									(0x4014)
#define APU_STATUS_ADDRESS								(0x4015)
#define JOYSTICK1_ADDRESS								(0x4016)
#define JOYSTICK2_OR_FRAMECFG_ADDRESS					(0x4017)
#define APU_AND_IO_END_ADDRESS							(APU_AND_IO_START_ADDRESS + 0x0017)
#define OTHER_APU_AND_IO_START_ADDRESS					(APU_AND_IO_END_ADDRESS + 1)
#define OTHER_APU_AND_IO_END_ADDRESS					(OTHER_APU_AND_IO_START_ADDRESS + 0x0007)
#define UNMAPPED_START_ADDRESS							(OTHER_APU_AND_IO_END_ADDRESS + 1)
#define CATRIDGE_RAM_START_ADDRESS						(0x6000)
#define CATRIDGE_RAM_END_ADDRESS						(CATRIDGE_RAM_START_ADDRESS + 0x1FFF)
#define CATRIDGE_ROM_BANK0_START_ADDRESS				(0x8000)
#define CATRIDGE_ROM_BANK0_END_ADDRESS					(CATRIDGE_ROM_BANK0_START_ADDRESS + 0x3FFF)
#define CATRIDGE_ROM_BANK1_START_ADDRESS				(0xC000)
#define CATRIDGE_ROM_BANK1_END_ADDRESS					(CATRIDGE_ROM_BANK1_START_ADDRESS + 0x3FFF)
#define NMI_VECTOR_START_ADDRESS						(0xFFFA)
#define NMI_VECTOR_END_ADDRESS							(NMI_VECTOR_START_ADDRESS + 1)
#define RESET_VECTOR_START_ADDRESS						(0xFFFC)
#define RESET_VECTOR_END_ADDRESS						(RESET_VECTOR_START_ADDRESS + 1)
#define IRQ_BRK_VECTOR_START_ADDRESS					(0xFFFE)
#define IRQ_BRK_VECTOR_END_ADDRESS						(IRQ_BRK_VECTOR_START_ADDRESS + 1)
#define UNMAPPED_END_ADDRESS							(0xFFFF)

// PPU Memory Mapping
#define PATTERN_TABLE0_START_ADDRESS					(0x0000)
#define PATTERN_TABLE0_END_ADDRESS						(PATTERN_TABLE0_START_ADDRESS + 0x0FFF)
#define PATTERN_TABLE1_START_ADDRESS					(PATTERN_TABLE0_END_ADDRESS + 1)
#define PATTERN_TABLE1_END_ADDRESS						(PATTERN_TABLE1_START_ADDRESS + 0x0FFF)
#define NAME_TABLE0_START_ADDRESS						(PATTERN_TABLE1_END_ADDRESS + 1)
#define NAME_TABLE0_END_ADDRESS							(NAME_TABLE0_START_ADDRESS + 0x03FF)
#define NAME_TABLE1_START_ADDRESS						(NAME_TABLE0_END_ADDRESS + 1)
#define NAME_TABLE1_END_ADDRESS							(NAME_TABLE1_START_ADDRESS + 0x03FF)
#define NAME_TABLE2_START_ADDRESS						(NAME_TABLE1_END_ADDRESS + 1)
#define NAME_TABLE2_END_ADDRESS							(NAME_TABLE2_START_ADDRESS + 0x03FF)
#define NAME_TABLE3_START_ADDRESS						(NAME_TABLE2_END_ADDRESS + 1)
#define NAME_TABLE3_END_ADDRESS							(NAME_TABLE3_START_ADDRESS + 0x03FF)
#define NAME_TABLE_SIZE									(NAME_TABLE1_END_ADDRESS + 1 - NAME_TABLE0_START_ADDRESS)
#define PPU_UNUSED_START_ADDRESS						(NAME_TABLE3_END_ADDRESS + 1)
#define PPU_UNUSED_END_ADDRESS							(PPU_UNUSED_START_ADDRESS + 0x0EFF)
#define PALETTE_RAM_INDEXES_START_ADDRESS				(PPU_UNUSED_END_ADDRESS + 1)
#define PALETTE_RAM_INDEXES_END_ADDRESS					(PALETTE_RAM_INDEXES_START_ADDRESS + 0x001F)
#define PALETTE_RAM_INDEXES_MIRROR_START_ADDRESS		(PALETTE_RAM_INDEXES_END_ADDRESS + 1)
#define PALETTE_RAM_INDEXES_MIRROR_END_ADDRESS			(PALETTE_RAM_INDEXES_MIRROR_START_ADDRESS + 0x00DF)

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

#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif
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

	boost::property_tree::ptree pt;

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

#pragma pack(push, 1)
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
		struct openBus_t
		{
			BYTE openBusValue;
			long long lastRefreshTimeInMs;
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

	enum class MAPPER
	{
		NOT_APPLICABLE = INVALID,
		NROM,
		MMC1,
		UxROM_002,
		CNROM,
		MMC3,
		MMC5,
		INES_MAPPER_006,
		AxROM,
		GxROM = 66,
		NANJING_FC001 = 163
	};

	enum class SUB_MAPPER
	{
		NOT_APPLICABLE = INVALID,
		SEROM_SHROM_SH1ROM,
		SNROM,
		SOROM,
		SUROM,
		SXROM,
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
			COUNTER8 clrWriteCount;
			SUB_MAPPER subMapper;
			struct
			{
				BYTE prgBank256;
				BYTE prgRamBank8_sxrom;
			} surom_sxrom;
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
						BYTE unused : 3; // bits 3 - 5
						BYTE prgRomMode : 1; // bit 6
						BYTE chrRomMode : 1; // bit 7
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
		} mmc3;
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
		bool wasDebuggerJustTriggerred;
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
				uint16_t IRQ_SRC_DMC : 1; // bit 0
				uint16_t IRQ_SRC_FRAMECTR : 1; // bit 1
				uint16_t IRQ_SRC_MMC3 : 1; // bit 2
				uint16_t IRQ_SRC_MMC5 : 1; // bit 3
				uint16_t IRQ_SRC_VRC467 : 1; // bit 4
				uint16_t IRQ_SRC_FME7 : 1; // bit 5
				uint16_t IRQ_SRC_NAMECO163 : 1; // bit 6
				uint16_t IRQ_SRC_FDS : 1; // bit 7
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
					BYTE flags8_15Type : 2; // bits  2 - 3
					BYTE mapperHi : 4; // bits  4 - 7
				} fields;
				BYTE raw;
			} flag7;
			BYTE prgRamSize;
			union
			{
				struct
				{
					BYTE tvSystem : 1; // bit  0
					BYTE reserved : 7; // bits  1 - 7
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
					BYTE romData[0x10000 - sizeof(iNES_header_t) - sizeof(trainer)];
				} withTrainer;
				struct
				{
					BYTE romData[0x10000 - sizeof(iNES_header_t)];
				} withoutTrainer;
			} remaining;
		} iNES_Fields;
		BYTE completeROM[0x202000];	// Maximum rom size seen so far is 2049 KB
	} iNES_t;

	typedef struct
	{
		FLAG isRomLoaded;
		uint32_t codeRomSize;
		iNES_t iNES;
	} aboutRom_t;

	typedef struct
	{
		NES_instance_t NES_instance;
		aboutRom_t aboutRom;
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

#pragma pack(pop)

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
	NES_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config, CheatEngine_t* ce);

	~NES_t();

	void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* network = nullptr) override;

	void sendBiosToEmulator(bios_t* bios = nullptr) override {};

	float getVersion();

	uint32_t getScreenWidth() override;

	uint32_t getScreenHeight() override;

	uint32_t getPixelWidth() override;

	uint32_t getPixelHeight() override;

	void setEmulationWindowOffsets(uint32_t x, uint32_t y, bool isEnabled);

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

	void clockMMC3IRQ(uint16_t address, MEMORY_ACCESS_SOURCE source, FLAG isWriteOperation);

	byte readPpuRawMemory(uint16_t address, MEMORY_ACCESS_SOURCE source);

	void writePpuRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source);
#pragma endregion RP2C02_DEFINITIONS

#pragma region RP2A03_DEFINITIONS
private:

	void cpuSetRegister(REGISTER_TYPE rt, uint16_t u16parameter);

	uint16_t cpuReadRegister(REGISTER_TYPE rt);

	byte readCpuRawMemory(uint16_t address, MEMORY_ACCESS_SOURCE source);

	void writeCpuRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source);

	void stackPush(BYTE data);

	BYTE stackPop();

	void processUnusedFlags(BYTE result);

	bool processSOC();

	EXCEPTION_EVENT_TYPE processNMI();

	EXCEPTION_EVENT_TYPE processIRQ();

	void runCPUPipeline();

	void unimplementedInstruction();
#pragma endregion RP2A03_DEFINITIONS

#pragma region EMULATION_DEFINITIONS
private:

	void cpuTickT(CYCLES_TYPE cycleType);

	void syncOtherGBModuleTicks();

	void ppuTick();

	void apuTick();

private:

	void captureIO();

private:

private:

	// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102 for pseudocode
	inline void tickPulse(AUDIO_CHANNELS channel)
	{
		if ((TO_UINT8(channel) > TO_UINT8(AUDIO_CHANNELS::PULSE_2))
			||
			(TO_UINT8(channel) < TO_UINT8(AUDIO_CHANNELS::PULSE_1)))
		{
			FATAL("Unknown Audio Channel");
		}
		else
		{
			if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyCounter > RESET)
			{
				--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyCounter;
			}
			else
			{
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.fields.unused = RESET;
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyCounter
					= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw;

				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].dutyCounter
					= ((pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].dutyCounter + ONE) & SEVEN);
			}
		}
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163155#p163155 for pseudocode
	inline void tickTriangle()
	{
		pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.isUltrasonic = NO;

		pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyPeriod.fields.unused = RESET;
		if ((pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyPeriod.raw < TWO)
			&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyCounter == RESET)
		{
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.isUltrasonic = YES;
		}

		FLAG clockTriangle = YES;
		if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
		{
			clockTriangle = NO;
		}
		if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.linearCounter.linearCounter == RESET)
		{
			clockTriangle = NO;
		}
		if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.isUltrasonic == YES)
		{
			clockTriangle = NO;
		}

		if (clockTriangle == YES)
		{
			if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyCounter > RESET)
			{
				--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyCounter;
			}
			else
			{
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyPeriod.fields.unused = ZERO;
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyCounter
					= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyPeriod.raw;

				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.triangleStep
					= ((pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.triangleStep + ONE) & 0x1F);
			}
		}
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163157#p163157 for pseudocode
	inline void tickNoise()
	{
		if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].frequencyCounter > RESET)
		{
			--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].frequencyCounter;
		}
		else
		{
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].frequencyCounter
				= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseFrequencyPeriod;

			if (pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_PERIOD.LOOP_NOISE == SET)
			{
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseShiftRegister.fields.fifteen
					= (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseShiftRegister.fields.six
						^ pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseShiftRegister.fields.zero);
			}
			else
			{
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseShiftRegister.fields.fifteen
					= (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseShiftRegister.fields.one
						^ pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseShiftRegister.fields.zero);
			}

			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseShiftRegister.raw >>= ONE;
		}
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163187&sid=4e4aade9cf7e18093eb1fd3afad14c49#p163187 for pseudocode
	inline void tickDMC()
	{
		if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].frequencyCounter > RESET)
		{
			--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].frequencyCounter;
		}
		else
		{
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].frequencyCounter
				= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcFrequencyPeriod;

			if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.isOutputUnitSilent == NO)
			{
				auto outputShift = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.outputShift;

				if ((outputShift & 0x01) 
					&& (pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_RAW.DIRECT_LOAD < 0x7E))
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_RAW.DIRECT_LOAD += TWO;
				}
				if ((!(outputShift & 0x01))
					&& (pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_RAW.DIRECT_LOAD > 0x01))
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_RAW.DIRECT_LOAD -= TWO;
				}
			}

			--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.bitsInOutputUnit;
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.outputShift >>= ONE;

			if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.bitsInOutputUnit == RESET)
			{
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.bitsInOutputUnit = EIGHT;
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.outputShift
					= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.sampleBuffer;
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.isOutputUnitSilent
					= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.isSampleBufferEmpty;
				pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.isSampleBufferEmpty = YES;
			}
		}

		if ((pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].lengthCounter > RESET)
			&& (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.isSampleBufferEmpty == YES))
		{
			pNES_instance->NES_state.emulatorStatus.ticks.dmcDmaCounter = RESET;

			APUTODO("Emulate the CPU halt cycles during the below DMC DMA read operation");

			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.sampleBuffer
				= readCpuRawMemory(pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcAddress, MEMORY_ACCESS_SOURCE::DMA);

			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.isSampleBufferEmpty = NO;
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcAddress
				= ((pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcAddress + ONE) | 0x8000);

			// Handle length counter processing
			--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].lengthCounter;
			if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].lengthCounter == RESET)
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_FREQ.LOOP_SAMPLE == SET)
				{
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].lengthCounter
						= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleLength;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcAddress
						= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleAddress;
					pNES_instance->NES_state.dmcDMA.sourceAddress = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleAddress;
				}
				else if (pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_FREQ.IRQ_ENABLE == SET)
				{
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_DMC = SET;
				}
			}

			pNES_instance->NES_state.dmcDMA.sourceAddress = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleAddress;
		}
	}

private:

	inline void processLengthCounter(AUDIO_CHANNELS channel)
	{
		if ((TO_UINT8(channel) >= TO_UINT8(AUDIO_CHANNELS::TOTAL_AUDIO_CHANNELS))
			||
			(TO_UINT8(channel) < TO_UINT8(AUDIO_CHANNELS::PULSE_1)))
		{
			FATAL("Unknown Audio Channel");
		}
		else
		{
			// If length counter is non zero and halt flag is not set, decrement the length counter
			if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].lengthCounter > ZERO)
			{
				auto isHaltEn = (pNES_instance->NES_state.audio.effectivelengthCounterHaltFlag[TO_UINT8(channel)] == SET);

				if (isHaltEn == NO && pNES_instance->NES_state.audio.skipClockingLengthCounter == NO)
				{
					--(pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].lengthCounter);
				}

				pNES_instance->NES_state.audio.skipClockingLengthCounter = CLEAR;
			}
		}
	}

	inline void processEnvelope(AUDIO_CHANNELS channel)
	{
		auto reloadDivider = [&](AUDIO_CHANNELS channel)
			{
				switch (channel)
				{
				case AUDIO_CHANNELS::PULSE_1:
				{
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.divider
						= pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_VOL.ENV_PERIOD_OR_VOL;
					BREAK;
				}
				case AUDIO_CHANNELS::PULSE_2:
				{
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.divider
						= pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_VOL.ENV_PERIOD_OR_VOL;
					BREAK;
				}
				case AUDIO_CHANNELS::NOISE:
				{
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.divider
						= pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_VOL.ENV_PERIOD_OR_VOL;
					BREAK;
				}
				default:
				{
					FATAL("Envelope is not supported in Channel : %d", TO_UINT8(channel));
				}
				}
			};

		if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.startFlag == YES)
		{
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.startFlag = NO;
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.decayLevelCounter = FIFTEEN;
			reloadDivider(channel);
		}
		else
		{
			if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.divider == RESET)
			{
				reloadDivider(channel);
				if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.decayLevelCounter == RESET)
				{
					switch (channel)
					{
					case AUDIO_CHANNELS::PULSE_1:
					{
						if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_VOL.LOOP_ENV_OR_DIS_LENGTH_COUNTER == SET)
						{
							pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.decayLevelCounter = FIFTEEN;
						}
						BREAK;
					}
					case AUDIO_CHANNELS::PULSE_2:
					{
						if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_VOL.LOOP_ENV_OR_DIS_LENGTH_COUNTER == SET)
						{
							pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.decayLevelCounter = FIFTEEN;
						}
						BREAK;
					}
					case AUDIO_CHANNELS::NOISE:
					{
						if (pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_VOL.LOOP_ENV_OR_DIS_LENGTH_COUNTER == SET)
						{
							pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.decayLevelCounter = FIFTEEN;
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
					--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.decayLevelCounter;
				}
			}
			else
			{
				--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.divider;
			}
		}
	}

	// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102 for pseudocode
	inline FLAG isSweepMutingChannel(AUDIO_CHANNELS channel, BYTE shift, FLAG negate)
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
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.fields.unused = RESET;
			auto upperBound
				= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw
				+ (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw >> shift);
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.fields.unused = RESET;

			if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw < EIGHT)
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
	inline void processSweep(AUDIO_CHANNELS channel)
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

				if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].sweep.reload == YES)
				{
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].sweep.sweepCounter = period;
					APUTODO("Handle Sweep Reload Edge Case at %d in %s", __LINE__, __FILE__);
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].sweep.reload = NO;
				}
				else if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].sweep.sweepCounter > RESET)
				{
					--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].sweep.sweepCounter;
				}
				else
				{
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].sweep.sweepCounter = period;
					if ((enable == YES) && (isSweepMutingChannel(channel, shift, negate) == NO))
					{
						pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.fields.unused = RESET;
						if (negate == YES)
						{
							if (channel == AUDIO_CHANNELS::PULSE_1)
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw
									-= ((pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw >> shift) + ONE);
							}
							else if (channel == AUDIO_CHANNELS::PULSE_2)
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw
									-= (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw >> shift);
							}
							else
							{
								FATAL("Sweep not supported for channel %d", TO_UINT8(channel));
							}
						}
						else
						{
							pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw
								+= (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.raw >> shift);
						}
						pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].frequencyPeriod.fields.unused = RESET;
					}
				}

			};

		switch (channel)
		{
		case AUDIO_CHANNELS::PULSE_1:
		{
			sweep(
				channel
				, (FLAG)((pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_SWEEP.ENABLE == SET)
					&& (pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_SWEEP.SHIFT_COUNT != RESET))
				, pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_SWEEP.SHIFT_COUNT
				, pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_SWEEP.PERIOD
				, pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_SWEEP.NEGATIVE
			);

			BREAK;
		}
		case AUDIO_CHANNELS::PULSE_2:
		{
			sweep(
				channel
				, (FLAG)((pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_SWEEP.ENABLE == SET)
					&& (pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_SWEEP.SHIFT_COUNT != RESET))
				, pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_SWEEP.SHIFT_COUNT
				, pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_SWEEP.PERIOD
				, pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_SWEEP.NEGATIVE
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
	inline void processLinearCounter()
	{
		if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.linearCounter.linearReload == YES)
		{
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.linearCounter.linearCounter
				= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.linearCounter.linearCounterReload;
		}
		else if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.linearCounter.linearCounter > RESET)
		{
			--pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.linearCounter.linearCounter;
		}

		if (pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_LINEAR.DIS_LENGTH_COUNTER == RESET)
		{
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.linearCounter.linearReload = NO;
		}
	}

private:

	// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102 for pulse part of the pseudocode
	// Refer https://forums.nesdev.org/viewtopic.php?p=163155#p163155 for triangle part of the pseudocode
	// Refer https://forums.nesdev.org/viewtopic.php?p=163157#p163157 for noise part of the pseudocode
	inline void generateLogicalOutput(AUDIO_CHANNELS channel)
	{
		NES_AUDIO_SAMPLE_TYPE sample = MUTE_AUDIO;

		switch (channel)
		{
		case AUDIO_CHANNELS::PULSE_1:
		{
			auto duty = pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_VOL.DUTY;
			auto dutyCounter = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].dutyCounter;
			auto lengthCounter = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].lengthCounter;
			auto shift = pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_SWEEP.SHIFT_COUNT;
			auto negate = pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_SWEEP.NEGATIVE;
			if (((PULSE_AMPLITUDE_LUT[duty][dutyCounter]) == HI)
				&& lengthCounter != RESET
				&& isSweepMutingChannel(channel, shift, negate) == NO)
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_VOL.CONSTANT_VOL == SET)
				{
					sample = static_cast<float>(pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_VOL.ENV_PERIOD_OR_VOL);
				}
				else
				{
					sample = static_cast<float>(pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.decayLevelCounter);
				}
			}
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].dacInput = sample;
			BREAK;
		}
		case AUDIO_CHANNELS::PULSE_2:
		{
			auto duty = pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_VOL.DUTY;
			auto dutyCounter = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].dutyCounter;
			auto lengthCounter = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].lengthCounter;
			auto shift = pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_SWEEP.SHIFT_COUNT;
			auto negate = pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_SWEEP.NEGATIVE;
			if (((PULSE_AMPLITUDE_LUT[duty][dutyCounter]) == HI)
				&& lengthCounter != RESET
				&& isSweepMutingChannel(channel, shift, negate) == NO)
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_VOL.CONSTANT_VOL == SET)
				{
					sample = static_cast<float>(pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_VOL.ENV_PERIOD_OR_VOL);
				}
				else
				{
					sample = static_cast<float>(pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.decayLevelCounter);
				}
			}
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].dacInput = sample;
			BREAK;
		}
		case AUDIO_CHANNELS::TRIANGLE:
		{
			if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].triangle.isUltrasonic == YES)
			{
				sample = 7.5f;
			}
			else if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].triangle.triangleStep & 0x10)
			{
				sample = static_cast<float>(pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].triangle.triangleStep ^ 0x1F);
			}
			else
			{
				sample = static_cast<float>(pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].triangle.triangleStep);
			}
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].dacInput = sample;
			BREAK;
		}
		case AUDIO_CHANNELS::NOISE:
		{
			auto lengthCounter = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].lengthCounter;
			if ((pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].noise.noiseShiftRegister.fields.zero == RESET)
				&& lengthCounter != RESET)
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_VOL.CONSTANT_VOL == SET)
				{
					sample = static_cast<float>(pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_VOL.ENV_PERIOD_OR_VOL);
				}
				else
				{
					sample = static_cast<float>(pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].envelope.decayLevelCounter);
				}
			}
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].dacInput = sample;
			BREAK;
		}
		case AUDIO_CHANNELS::DMC:
		{
			sample = static_cast<float>(pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_RAW.DIRECT_LOAD);
			pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(channel)].dacInput = sample;
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

	bool saveState(uint8_t = 0) override;

	bool loadState(uint8_t = 0) override;

	bool absoluteSaveState(uint8_t id);

	bool absoluteLoadState(uint8_t id);

	bool fillGamePlayStack() override;

	bool rewindGamePlay() override;

private:

	bool runEmulationAtHostRate(uint32_t currentFrame) override;

	bool runEmulationLoopAtHostRate(uint32_t currentFrame) override;

	bool runEmulationAtFixedRate(uint32_t currentFrame) override;

	bool runEmulationLoopAtFixedRate(uint32_t currentFrame) override;

public:

	float getEmulationVolume() override;

	void setEmulationVolume(float volume) override;

private:

	void initializeGraphics();

	void initializeAudio();

	void reInitializeAudio();

private:

	bool initializeEmulator() override;

	void destroyEmulator() override;

public:

	bool getRomLoadedStatus() override;

	bool loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) override;

	void dumpRom() override;
#pragma endregion EMULATION_DEFINITIONS
};
#pragma endregion CORE