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
#define GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN	YES	// Refer : https://nba-emu.github.io/hw-docs/ppu/background.html
#define GBA_ENABLE_CYCLE_ACCURATE_PPU_TICK				YES	// More accurate, needed to pass the AGS DMA priority tests; note that enabling this will bringdown the FPS!
#define GBA_ENABLE_DELAYED_MMIO_WRITE					YES
#define GBA_ENABLE_AGS_PATCHED_TEST						NO
#pragma endregion WIP

#define GBA_REFERENCE_CLOCK_HZ							16777216.0f
#define GBA_FPS											60.0f
#define EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA			32768.0f
#ifdef __EMSCRIPTEN__
#define AUDIO_BUFFER_SIZE_FOR_GBA						1536 // (((CEIL(EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA / GBA_FPS)) + ONE) & ~ONE)
#else
#define AUDIO_BUFFER_SIZE_FOR_GBA						1536 // (((CEIL(EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA / GBA_FPS)) + ONE) & ~ONE)
#endif

#define GBA_HALFWORD									uint16_t
#define GBA_WORD										uint32_t

// GBA specific definitions
#define SYSTEM_ROM_START_ADDRESS						0x00000000
#define SYSTEM_ROM_END_ADDRESS							0x00003FFF
#define SYSTEM_ROM_SIZE									0x00004000
#define SYSTEM_ROM_UNUSED_START_ADDRESS					0x00004000
#define SYSTEM_ROM_UNUSED_END_ADDRESS					0x01FFFFFF
#define EXT_WORK_RAM_START_ADDRESS						0x02000000
#define EXT_WORK_RAM_END_ADDRESS						0x0203FFFF
#define EXT_WORK_RAM_SIZE								0x00040000
#define EXT_WORK_RAM_MIRROR_START_ADDRESS				0x02040000
#define EXT_WORK_RAM_MIRROR_END_ADDRESS					0x02FFFFFF
#define INT_WORK_RAM_START_ADDRESS						0x03000000
#define INT_WORK_RAM_END_ADDRESS						0x03007FFF
#define INT_WORK_RAM_SIZE								0x00008000
#define INT_WORK_RAM_MIRROR_START_ADDRESS				0x03008000
#define INT_WORK_RAM_MIRROR_END_ADDRESS					0x03FFFFFF
#define IO_START_ADDRESS								0x04000000
#define IO_DISPCNT										0x04000000
#define IO_GREENSWAP									0x04000002
#define IO_DISPSTAT										0x04000004
#define IO_VCOUNT										0x04000006
#define IO_BG0CNT										0x04000008
#define IO_BG1CNT										0x0400000A
#define IO_BG2CNT										0x0400000C
#define IO_BG3CNT										0x0400000E
#define IO_BG0HOFS										0x04000010
#define IO_BG0VOFS										0x04000012
#define IO_BG1HOFS										0x04000014
#define IO_BG1VOFS										0x04000016
#define IO_BG2HOFS										0x04000018
#define IO_BG2VOFS										0x0400001A
#define IO_BG3HOFS										0x0400001C
#define IO_BG3VOFS										0x0400001E
#define IO_BG2PA										0x04000020
#define IO_BG2PB										0x04000022
#define IO_BG2PC										0x04000024
#define IO_BG2PD										0x04000026
#define IO_BG2X_L										0x04000028
#define IO_BG2X_H										0x0400002A
#define IO_BG2Y_L										0x0400002C
#define IO_BG2Y_H										0x0400002E
#define IO_BG3PA										0x04000030
#define IO_BG3PB										0x04000032
#define IO_BG3PC										0x04000034
#define IO_BG3PD										0x04000036
#define IO_BG3X_L										0x04000038
#define IO_BG3X_H										0x0400003A
#define IO_BG3Y_L										0x0400003C
#define IO_BG3Y_H										0x0400003E
#define IO_WIN0H										0x04000040
#define IO_WIN1H										0x04000042
#define IO_WIN0V										0x04000044
#define IO_WIN1V										0x04000046
#define IO_WININ										0x04000048
#define IO_WINOUT										0x0400004A
#define IO_MOSAIC										0x0400004C
#define IO_400004E										0x0400004E
#define IO_BLDCNT										0x04000050
#define IO_BLDALPHA										0x04000052
#define IO_BLDY											0x04000054
#define IO_4000056										0x04000056
#define IO_4000058										0x04000058
#define IO_400005A										0x0400005A
#define IO_400005C										0x0400005C
#define IO_400005E										0x0400005E
#define IO_SOUND1CNT_L									0x04000060 
#define IO_SOUND1CNT_H									0x04000062 
#define IO_SOUND1CNT_X									0x04000064 
#define IO_4000066 										0x04000066 
#define IO_SOUND2CNT_L									0x04000068 
#define IO_400006A										0x0400006A
#define IO_SOUND2CNT_H									0x0400006C 
#define IO_400006E										0x0400006E 
#define IO_SOUND3CNT_L									0x04000070 
#define IO_SOUND3CNT_H									0x04000072 
#define IO_SOUND3CNT_X									0x04000074 
#define IO_4000076										0x04000076 
#define IO_SOUND4CNT_L									0x04000078 
#define IO_400007A										0x0400007A 
#define IO_SOUND4CNT_H									0x0400007C 
#define IO_400007E										0x0400007E 
#define IO_SOUNDCNT_L									0x04000080 
#define IO_SOUNDCNT_H									0x04000082 
#define IO_SOUNDCNT_X									0x04000084 
#define IO_4000086 										0x04000086 
#define IO_SOUNDBIAS									0x04000088 
#define IO_400008A 										0x0400008A 
#define IO_400008C 										0x0400008C 
#define IO_400008E 										0x0400008E
#define IO_WAVERAM_START_ADDRESS						0x04000090
#define IO_WAVERAM_END_ADDRESS							0x0400009E
#define IO_FIFO_A_L										0x040000A0 
#define IO_FIFO_A_H										0x040000A2 
#define IO_FIFO_B_L										0x040000A4
#define IO_FIFO_B_H										0x040000A6
#define IO_40000A8										0x040000A8
#define IO_40000AA										0x040000AA
#define IO_40000AC										0x040000AC
#define IO_40000AE										0x040000AE
#define IO_DMA0SAD_L   									0x040000B0
#define IO_DMA0SAD_H   									0x040000B2
#define IO_DMA0DAD_L									0x040000B4
#define IO_DMA0DAD_H									0x040000B6
#define IO_DMA0CNT_L									0x040000B8
#define IO_DMA0CNT_H									0x040000BA
#define IO_DMA1SAD_L									0x040000BC
#define IO_DMA1SAD_H									0x040000BE
#define IO_DMA1DAD_L									0x040000C0
#define IO_DMA1DAD_H									0x040000C2
#define IO_DMA1CNT_L									0x040000C4
#define IO_DMA1CNT_H									0x040000C6
#define IO_DMA2SAD_L									0x040000C8
#define IO_DMA2SAD_H									0x040000CA
#define IO_DMA2DAD_L									0x040000CC
#define IO_DMA2DAD_H									0x040000CE
#define IO_DMA2CNT_L									0x040000D0
#define IO_DMA2CNT_H									0x040000D2
#define IO_DMA3SAD_L									0x040000D4
#define IO_DMA3SAD_H									0x040000D6
#define IO_DMA3DAD_L									0x040000D8
#define IO_DMA3DAD_H									0x040000DA
#define IO_DMA3CNT_L									0x040000DC
#define IO_DMA3CNT_H									0x040000DE
#define IO_40000E0										0x040000E0
#define IO_40000E2										0x040000E2
#define IO_40000E4										0x040000E4
#define IO_40000E6										0x040000E6
#define IO_40000E8										0x040000E8
#define IO_40000EA										0x040000EA
#define IO_40000EC										0x040000EC
#define IO_40000EE										0x040000EE
#define IO_40000F0										0x040000F0
#define IO_40000F2										0x040000F2
#define IO_40000F4										0x040000F4
#define IO_40000F6										0x040000F6
#define IO_40000F8										0x040000F8
#define IO_40000FA										0x040000FA
#define IO_40000FC										0x040000FC
#define IO_40000FE										0x040000FE
#define IO_TM0CNT_L										0x04000100
#define IO_TM0CNT_H										0x04000102
#define IO_TM1CNT_L										0x04000104
#define IO_TM1CNT_H										0x04000106
#define IO_TM2CNT_L										0x04000108
#define IO_TM2CNT_H										0x0400010A
#define IO_TM3CNT_L										0x0400010C
#define IO_TM3CNT_H										0x0400010E
#define IO_4000110										0x04000110
#define IO_4000112										0x04000112
#define IO_4000114										0x04000114
#define IO_4000116										0x04000116
#define IO_4000118										0x04000118
#define IO_400011A										0x0400011A
#define IO_400011C										0x0400011C
#define IO_400011E										0x0400011E
#define IO_SIOMULTI0									0x04000120
#define IO_SIOMULTI1									0x04000122
#define IO_SIOMULTI2									0x04000124
#define IO_SIOMULTI3									0x04000126
#define IO_SIOCNT										0x04000128
#define IO_SIO_DATA8_MLTSEND							0x0400012A
#define IO_400012C										0x0400012C
#define IO_400012E										0x0400012E
#define IO_KEYINPUT										0x04000130
#define IO_KEYCNT										0x04000132
#define IO_RCNT											0x04000134
#define IO_IR											0x04000136
#define IO_4000138										0x04000138
#define IO_400013A										0x0400013A
#define IO_400013C										0x0400013C
#define IO_400013E										0x0400013E
#define IO_JOYCNT										0x04000140
#define IO_4000142										0x04000142
#define IO_4000144										0x04000144
#define IO_4000146										0x04000146
#define IO_4000148										0x04000148
#define IO_400014A										0x0400014A
#define IO_400014C										0x0400014C
#define IO_400014E										0x0400014E
#define IO_JOY_RECV_L									0x04000150
#define IO_JOY_RECV_H									0x04000152
#define IO_JOY_TRANS_L									0x04000154
#define IO_JOY_TRANS_H									0x04000156
#define IO_JOYSTAT										0x04000158
#define IO_400015A										0x0400015A
#define IO_UNDOC1_START									0x0400015C
#define IO_UNDOC1_END									0x040001FE
#define IO_IE											0x04000200
#define IO_IF											0x04000202
#define IO_WAITCNT										0x04000204
#define IO_4000206										0x04000206
#define IO_IME											0x04000208
#define IO_400020A										0x0400020A
#define IO_UNDOC2_START									0x0400020C
#define IO_UNDOC2_END									0x040002FE
#define IO_POSTFLG_HALTCNT								0x04000300
#define IO_4000302										0x04000302
#define IO_IMEMCTRL										0x04000800
#define IO_END_ADDRESS									0x04FFFFFF
#define PALETTE_RAM_START_ADDRESS						0x05000000
#define PALETTE_RAM_END_ADDRESS							0x050003FF
#define PALETTE_RAM_SIZE								0x00000400
#define PALETTE_RAM_MIRROR_START_ADDRESS				0x05000400
#define PALETTE_RAM_MIRROR_END_ADDRESS					0x05FFFFFF
#define VIDEO_RAM_START_ADDRESS							0x06000000
#define VIDEO_RAM_END_ADDRESS							0x06017FFF
#define VIDEO_RAM_SIZE									0x00018000
#define VIDEO_RAM_MIRROR_START_ADDRESS					0x06018000
#define VIDEO_RAM_MIRROR_END_ADDRESS					0x06FFFFFF
#define OAM_START_ADDRESS								0x07000000
#define OAM_END_ADDRESS									0x070003FF
#define OAM_SIZE										0x00000400
#define OAM_MIRROR_START_ADDRESS						0x07000400
#define OAM_MIRROR_END_ADDRESS							0x07FFFFFF
#define GAMEPAK_ROM_WS0_START_ADDRESS					0x08000000
#define GAMEPAK_ROM_WS0_END_ADDRESS						0x09FFFFFF
#define GAMEPAK_ROM_WS1_START_ADDRESS					0x0A000000
#define GAMEPAK_ROM_WS1_END_ADDRESS						0x0BFFFFFF
#define GAMEPAK_ROM_WS2_START_ADDRESS					0x0C000000
#define GAMEPAK_ROM_WS2_END_ADDRESS						0x0DFFFFFF
#define GAMEPAK_ROM_SIZE								0x02000000
#define GAMEPAK_SRAM_START_ADDRESS						0x0E000000
#define FLASH_ACCESS_MEMORY0							0x0E000000
#define FLASH_ACCESS_MEMORY1							0x0E000001
#define FLASH_ACCESS_MEMORY2							0x0E005555
#define FLASH_ACCESS_MEMORY3							0x0E002AAA
#define GAMEPAK_SRAM_END_ADDRESS						0x0E00FFFF
#define GAMEPAK_SRAM_SIZE								0x00010000
#define GAMEPAK_SRAM_MIRROR_START_ADDRESS				0x0E010000
#define GAMEPAK_SRAM_MIRROR_END_ADDRESS					0x0FFFFFFF

#define LFSR_WIDTH_IS_15_BITS							ZERO
#define LFSR_WIDTH_IS_7_BITS							ONE
#define DIRECT_SOUND_A									ZERO
#define DIRECT_SOUND_B									ONE

#define AUDIO_FIFO_SIZE_FOR_GBA							FOUR

#define APU_FRAME_SEQUENCER_RATE_HZ						(512.0f)
#define DC_BIAS_FOR_AUDIO_SAMPLES						ZERO
#define DISABLE_FIRST_PULSE_CHANNEL						NO
#define DISABLE_SECOND_PULSE_CHANNEL					NO
#define DISABLE_WAVE_CHANNEL							NO
#define DISABLE_NOISE_CHANNEL							NO
#define DISABLE_DMAA_CHANNEL							NO
#define DISABLE_DMAB_CHANNEL							NO

#define NRW_WAVE_BANK									ZERO
#define RW_WAVE_BANK									ONE

// NOTE: 5 bit color to 8 bit color; Refer https://www.pokecommunity.com/threads/how-do-i-convert-rgb-to-gba-colors-and-vice-versa.420695/
#define FIVEBIT_TO_EIGHTBIT_COLOR(x)					((x<<3)|(x&7))
#define DEFAULT_OBJ_PRIORITY							ZERO	// TODO: Should this 0 or 3 ?
#define WIN0											ZERO
#define WIN1											ONE
#define BG0												ZERO
#define BG1												ONE
#define BG2												TWO
#define BG3												THREE
#define OBJ												FOUR
#define BD												FIVE
#define MODE0											ZERO
#define MODE1											ONE
#define MODE2											TWO
#define MODE3											THREE
#define MODE4											FOUR
#define MODE5											FIVE
#define ALLMODES										TEN
#define IS_COLOR_NOT_PALETTE							(0x80000000)

#define CPSR_FLAG(F, C)									(F == 1 ? C:"-")

#define loadPipeline									fetchAndDecode
#define initializeSerialClockSpeed						processSerialClockSpeedBit
#define performOverFlowCheck							getUpdatedFrequency

#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif
#pragma endregion MACROS

#pragma region CORE
class GBA_t : public abstractEmulation_t
{
#pragma region INFRASTRUCTURE_DECLARATIONS
private:

	std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom;
	const float myFPS = (float)GBA_FPS;

private:

	uint32_t y_offset = ZERO;
	uint32_t x_offset = ZERO;
	static const uint32_t screen_y_offset = 0;
	static const uint32_t screen_x_offset = 10;
	static const uint32_t screen_height = 160;
	static const uint32_t screen_width = 240;
	static const uint32_t total_screen_height = screen_height + (screen_y_offset * 2);
	static const uint32_t total_screen_width = screen_width + (screen_x_offset * 2);
	static const uint32_t pixel_height = 1;
	static const uint32_t pixel_width = 1;
	static const uint32_t debugger_screen_height = 560;
	static const uint32_t debugger_screen_width = 880;
	static const uint32_t debugger_pixel_height = 1;
	static const uint32_t debugger_pixel_width = 1;
	const char* NAME = "GBA";

private:

	boost::property_tree::ptree pt;

private:

	uint8_t const SST_ROMS = TWO;

private:

	uint32_t profiler_FrameRate;
	uint64_t functionID;

private:

	void* GBAGameEngine = nullptr;
#pragma endregion INFRASTRUCTURE_DECLARATIONS

#pragma region ARM7TDMI_DECLARATIONS
private:

	enum class STATE_TYPE
	{
		ST_ARM,					// 0
		ST_THUMB,				// 1
		ST_TOTAL,				// TOTAL = 2
		ST_NONE
	};

	enum class OP_MODE_TYPE
	{
		OP_USR = SIXTEEN,		// 16
		OP_FIQ = SEVENTEEN,		// 17
		OP_IRQ = EIGHTEEN,		// 18
		OP_SVC = NINETEEN,		// 19
		OP_ABT = TWENTYTHREE,	// TWO3
		OP_UND = TWENTYSEVEN,	// TWO7
		OP_SYS = THIRTYONE,		// 31
		OP_TOTAL = SEVEN,		// TOTAL = 7
		OP_NONE = THIRTYTWO
	};

	enum class REGISTER_BANK_TYPE
	{
		RB_USR_SYS,
		RB_FIQ,
		RB_IRQ,
		RB_SVC,
		RB_ABT,
		RB_UND,
		RB_TOTAL,
		RB_NONE
	};

	enum class REGISTER_TYPE
	{	//							|		ARM State		|		THUMB State		|
		/*--------------------------|------------------------------------------------*/
		RT_0,				// 0	|		Applicable		|		Applicable		|
		RT_1,				// 1	|		Applicable		|		Applicable		|
		RT_2,				// TWO	|		Applicable		|		Applicable		|
		RT_3,				// 3	|		Applicable		|		Applicable		|
		RT_4,				// 4	|		Applicable		|		Applicable		|
		RT_5,				// 5	|		Applicable		|		Applicable		|
		RT_6,				// 6	|		Applicable		|		Applicable		|
		RT_7,				// 7	|		Applicable		|		Applicable		|
		RT_8,				// 8	|		Applicable		|		Not Applicable	|
		RT_9,				// 9	|		Applicable		|		Not Applicable	|
		RT_10,				// 10	|		Applicable		|		Not Applicable	|
		RT_11,				// 11	|		Applicable		|		Not Applicable	|
		RT_12,				// 12	|		Applicable		|		Not Applicable	|
		RT_13,				// 13	|		SP				|		SP				|
		RT_14,				// 14	|		LR				|		LR				|
		RT_15,				// 15	|		PC				|		PC				|
		/*--------------------------|------------------------------------------------*/
		RT_16,				// 16	|		CPSR			|		CPSR			|
		RT_17,				// 17	|		SPSR			|		SPSR			|
		/*---------------------------------------------------------------------------*/
		RT_TOTAL,			// TOTAL = 18
		RT_NONE
	};

	static constexpr REGISTER_BANK_TYPE OP_MODE_TO_REGISTER_BANK[32] = {
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS,  // 0x10 - OP_USR
		REGISTER_BANK_TYPE::RB_FIQ,      // 0x11 - OP_FIQ
		REGISTER_BANK_TYPE::RB_IRQ,      // 0x12 - OP_IRQ
		REGISTER_BANK_TYPE::RB_SVC,      // 0x13 - OP_SVC
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_ABT,      // 0x17 - OP_ABT
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_UND,      // 0x1B - OP_UND
		REGISTER_BANK_TYPE::RB_USR_SYS, REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS,
		REGISTER_BANK_TYPE::RB_USR_SYS   // 0x1F - OP_SYS
	};

	static constexpr OP_MODE_TYPE REGISTER_BANK_TO_OP_MODE[6] = {
		OP_MODE_TYPE::OP_USR,  // RB_USR_SYS
		OP_MODE_TYPE::OP_FIQ,  // RB_FIQ
		OP_MODE_TYPE::OP_SVC,  // RB_SVC
		OP_MODE_TYPE::OP_ABT,  // RB_ABT
		OP_MODE_TYPE::OP_IRQ,  // RB_IRQ
		OP_MODE_TYPE::OP_UND   // RB_UND
	};

#define SP						(uint8_t)REGISTER_TYPE::RT_13
#define LR						(uint8_t)REGISTER_TYPE::RT_14
#define PC						(uint8_t)REGISTER_TYPE::RT_15
#define CPSR					(uint8_t)REGISTER_TYPE::RT_16
#define SPSR					(uint8_t)REGISTER_TYPE::RT_17
#define REGISTER_BANKS			(uint8_t)REGISTER_BANK_TYPE::RB_TOTAL
#define LO_GP_REGISTERS			EIGHT
#define HI_GP_REGISTERS			EIGHT
#define TOTAL_GP_REGISTERS		(uint8_t)REGISTER_TYPE::RT_TOTAL

	enum class CPSR_CONDITION_CODE
	{
		CPSR_EQ_SetZ = 0b0000,
		CPSR_NE_ClearZ = 0b0001,
		CPSR_CS_SetC = 0b0010,
		CPSR_CS_ClearC = 0b0011,
		CPSR_MI_SetN = 0b0100,
		CPSR_PL_ClearN = 0b0101,
		CPSR_VS_SetV = 0b0110,
		CPSR_VC_ClearV = 0b0111,
		CPSR_HI_SetC_AND_ClearZ = 0b1000,
		CPSR_LS_ClearC_OR_SetZ = 0b1001,
		CPSR_GE_NEqualsV = 0b1010,
		CPSR_LT_NNotEqualsV = 0b1011,
		CPSR_CS_ClearZ_AND_NEqualsV = 0b1100,
		CPSR_CS_SetZ_OR_NNotEqualsV = 0b1101,
		CPSR_AL_IgnoreConditions = 0b1110,
		CPSR_RES = 0b1111
	};

	enum MEMORY_ACCESS_WIDTH : uint8_t
	{
		EIGHT_BIT,
		SIXTEEN_BIT,
		THIRTYTWO_BIT,
		TOTAL_ACCESS_WIDTH_POSSIBLE
	};

	enum MEMORY_ACCESS_TYPE : uint8_t
	{
		NON_SEQUENTIAL_CYCLE,
		SEQUENTIAL_CYCLE,
		TOTAL_MEMORY_ACCESS_TYPES,
		AUTOMATIC
	};

	enum class ADDRESS_TIMING_TYPE
	{
		PIPELINED,
		DE_PIPELINED,
		TOTAL_ADDRESS_TIMING_TYPES
	};

	enum class SHIFT_TYPE
	{
		LSL,
		LSR,
		ASR,
		ROR,
	};

	enum class ALU_SUBCODES
	{
		ALU_AND,
		ALU_XOR,
		ALU_LSL,
		ALU_LSR,
		ALU_ASR,
		ALU_ADC,
		ALU_SBC,
		ALU_ROR,
		ALU_TST,
		ALU_NEG,
		ALU_CMP,
		ALU_CMN,
		ALU_ORR,
		ALU_MUL,
		ALU_BIC,
		ALU_MVN,
		ALU_TOTAL
	};

	enum class DATAPROCESSING_SUBCODES
	{
		DP_AND,
		DP_XOR,
		DP_SUB,
		DP_RSB,
		DP_ADD,
		DP_ADC,
		DP_SBC,
		DP_RSC,
		DP_TST,
		DP_TEQ,
		DP_CMP,
		DP_CMN,
		DP_ORR,
		DP_MOV,
		DP_BIC,
		DP_NOT,
		DP_TOTAL
	};

	enum class HALT_CONTROLLER
	{
		HALT,
		STOP,
		RUN
	};

private:

	// Used only for logging purpose
	const std::string OP_MODE_NAMES[THIRTYTWO] =
	{
		"UNKNOWN",    // 0b00000
		"UNKNOWN",    // 0b00001
		"UNKNOWN",    // 0b00010
		"UNKNOWN",    // 0b00011
		"UNKNOWN",    // 0b00100
		"UNKNOWN",    // 0b00101
		"UNKNOWN",    // 0b00110
		"UNKNOWN",    // 0b00111
		"UNKNOWN",    // 0b01000
		"UNKNOWN",    // 0b01001
		"UNKNOWN",    // 0b01010
		"UNKNOWN",    // 0b01011
		"UNKNOWN",    // 0b01100
		"UNKNOWN",    // 0b01101
		"UNKNOWN",    // 0b01110
		"UNKNOWN",    // 0b01111
		"USER",       // 0b10000
		"FIQ",        // 0b10001
		"IRQ",        // 0b10010
		"SUPERVISOR", // 0b10011
		"UNKNOWN",    // 0b10100
		"UNKNOWN",    // 0b10101
		"UNKNOWN",    // 0b10110
		"ABORT",      // 0b10111
		"UNKNOWN",    // 0b11000
		"UNKNOWN",    // 0b11001
		"UNKNOWN",    // 0b11010
		"UNDEFINED",  // 0b11011
		"UNKNOWN",    // 0b11100
		"UNKNOWN",    // 0b11101
		"UNKNOWN",    // 0b11110
		"SYS",        // 0b11111
	};

	// Used only for logging purpose
	const std::string SWI_NAMES[SEVENTYSIX] = {
		"SoftReset",
		"RegisterRamReset",
		"Halt",
		"Stop/Sleep",
		"IntrWait",
		"VBlankIntrWait",
		"Div",
		"DivArm",
		"Sqrt",
		"ArcTan",
		"ArcTan2",
		"CpuSet",
		"CpuFastSet",
		"GetBiosChecksum",
		"BgAffineSet",
		"ObjAffineSet",
		"BitUnPack",
		"LZ77UnCompReadNormalWrite8bit",
		"LZ77UnCompReadNormalWrite16bit",
		"LZ77UnCompReadByCallbackWrite8bit",
		"LZ77UnCompReadByCallbackWrite16bit",
		"LZ77UnCompReadByCallbackWrite16bit (same as above)",
		"HuffUnCompReadNormal",
		"HuffUnCompReadByCallback",
		"RLUnCompReadNormalWrite8bit",
		"RLUnCompReadNormalWrite16bit",
		"RLUnCompReadByCallbackWrite16bit",
		"Diff8bitUnFilterWrite8bit",
		"Diff8bitUnFilterWrite16bit",
		"Diff16bitUnFilter",
		"Sound (and Multiboot/HardReset/CustomHalt)",
		"SoundBias",
		"SoundDriverInit",
		"SoundDriverMode",
		"SoundDriverMain",
		"SoundDriverVSync",
		"SoundChannelClear",
		"MidiKey2Freq",
		"SoundWhatever0",
		"SoundWhatever1",
		"SoundWhatever2",
		"SoundWhatever3",
		"SoundWhatever4",
		"MultiBoot",
		"HardReset",
		"CustomHalt",
		"SoundDriverVSyncOff",
		"SoundDriverVSyncOn",
		"SoundGetJumpList",
		"New NDS Functions",
		"WaitByLoop",
		"GetCRC16",
		"IsDebugger",
		"GetSineTable",
		"GetPitchTable (DSi7: bugged)",
		"GetVolumeTable",
		"GetBootProcs (DSi7: only 1 proc)",
		"CustomPost",
		"New DSi Functions (RSA/SHA1)",
		"RSA_Init_crypto_heap",
		"RSA_Decrypt",
		"RSA_Decrypt_Unpad",
		"RSA_Decrypt_Unpad_OpenPGP_SHA1",
		"SHA1_Init",
		"SHA1_Update",
		"SHA1_Finish",
		"SHA1_Init_update_fin",
		"SHA1_Compare_20_bytes",
		"SHA1_Random_maybe",
		"Invalid Functions",
		"Crash (SWI xxh..FFh do jump to garbage addresses)",
		"Jump to 0   (on any SWI numbers not listed above)",
		"No function (ignored)",
		"No function (ignored)",
		"Mirror      (SWI 40h..FFh mirror to 00h..3Fh)",
		"Hang        (on any SWI numbers not listed above)",
	};

	// Used only for logging purpose
	std::string disassembled = "Disassembly Unsupported";

private:

#pragma pack(push, 1)

	// Structure of CPSR and SPSRs

	typedef struct
	{
		uint32_t psrModeBits : 5; // bits 0 - 4
		uint32_t psrStateBit : 1; // bit  5
		uint32_t psrFIQDisBit : 1; // bit  6	
		uint32_t psrIRQDisBit : 1; // bit  7
		uint32_t psrReservedBits : 20; // bits 8 - 27
		uint32_t psrOverflowBit : 1; // bit  28
		uint32_t psrCarryBorrowExtBit : 1; // bit  29
		uint32_t psrZeroBit : 1; // bit  30
		uint32_t psrNegativeBit : 1; // bit  31
	} psrFields_t;

	typedef union
	{
		psrFields_t psrFields;
		uint32_t psrMemory;
	} psr_t;

	typedef struct
	{
		uint32_t unbankedLORegisters[LO_GP_REGISTERS];
		uint32_t bankedHIRegisters[REGISTER_BANKS][HI_GP_REGISTERS - ONE];
		uint32_t pc;
		psr_t cpsr;
		psr_t spsr[REGISTER_BANKS];
	} registers_t;

	// THUMB Instructions

	typedef struct moveShiftedRegister
	{
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD rs : 3;
		GBA_HALFWORD offset : 5;
		GBA_HALFWORD opcode : 2;
		GBA_HALFWORD : 3;
	} moveShiftedRegister_t;

	typedef struct addSubtract
	{
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD rs : 3;
		GBA_HALFWORD rn_or_offset : 3;
		GBA_HALFWORD op : 1;
		GBA_HALFWORD i : 1;
		GBA_HALFWORD : 5;
	} addSubtract_t;

	typedef struct moveCompareAddSubtractImmediate
	{
		GBA_HALFWORD offset : 8;
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD opcode : 2;
		GBA_HALFWORD : 3;
	} moveCompareAddSubtractImmediate_t;

	typedef struct aluOperations
	{
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD rs : 3;
		GBA_HALFWORD opcode : 4;
		GBA_HALFWORD : 6;
	} aluOperations_t;

	typedef struct highRegisterOperations
	{
		GBA_HALFWORD rdhd : 3;
		GBA_HALFWORD rshs : 3;
		GBA_HALFWORD h2 : 1;
		GBA_HALFWORD h1 : 1;
		GBA_HALFWORD opcode : 2;
		GBA_HALFWORD : 6;
	} highRegisterOperations_t;

	typedef struct pcRelativeLoad
	{
		GBA_HALFWORD word8 : 8;
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD : 5;
	} pcRelativeLoad_t;

	typedef struct loadStoreRO
	{
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD rb : 3;
		GBA_HALFWORD ro : 3;
		GBA_HALFWORD : 1;
		GBA_HALFWORD b : 1;
		GBA_HALFWORD l : 1;
		GBA_HALFWORD : 4;
	} loadStoreRO_t;

	typedef struct loadStoreSignedByteHalfword
	{
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD rb : 3;
		GBA_HALFWORD ro : 3;
		GBA_HALFWORD : 1;
		GBA_HALFWORD opcode : 2;
		GBA_HALFWORD : 4;
	} loadStoreSignedByteHalfword_t;

	typedef struct loadStoreIO
	{
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD rb : 3;
		GBA_HALFWORD offset : 5;
		GBA_HALFWORD l : 1;
		GBA_HALFWORD b : 1;
		GBA_HALFWORD : 3;
	} loadStoreIO_t;

	typedef struct loadStoreHalfword
	{
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD rb : 3;
		GBA_HALFWORD offset : 5;
		GBA_HALFWORD l : 1;
		GBA_HALFWORD : 4;
	} loadStoreHalfword_t;

	typedef struct spRelativeLoadStore
	{
		GBA_HALFWORD word8 : 8;
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD l : 1;
		GBA_HALFWORD : 4;
	} spRelativeLoadStore_t;

	typedef struct loadAddress
	{
		GBA_HALFWORD word8 : 8;
		GBA_HALFWORD rd : 3;
		GBA_HALFWORD spOrPc : 1;
		GBA_HALFWORD : 4;
	} loadAddress_t;

	typedef struct addOffsetToStackPointer
	{
		GBA_HALFWORD sword7 : 7;
		GBA_HALFWORD s : 1;
		GBA_HALFWORD : 8;
	} addOffsetToStackPointer_t;

	typedef struct pushPopRegisters
	{
		GBA_HALFWORD rlist : 8;
		GBA_HALFWORD r : 1;
		GBA_HALFWORD : 2;
		GBA_HALFWORD l : 1;
		GBA_HALFWORD : 4;
	} pushPopRegisters_t;

	typedef struct multipleLoadStore
	{
		GBA_HALFWORD rlist : 8;
		GBA_HALFWORD rb : 3;
		GBA_HALFWORD l : 1;
		GBA_HALFWORD : 4;
	} multipleLoadStore_t;

	typedef struct conditionalBranch
	{
		GBA_HALFWORD soffset : 8;
		GBA_HALFWORD cond : 4;
		GBA_HALFWORD : 4;
	} conditionalBranch_t;

	typedef struct thumbSoftwareInterrupt
	{
		GBA_HALFWORD value : 8;
		GBA_HALFWORD : 8;
	} thumbSoftwareInterrupt_t;

	typedef struct unconditionalBranch
	{
		GBA_HALFWORD offset : 11;
		GBA_HALFWORD : 5;
	} unconditionalBranch_t;

	typedef struct longBranchWithLink
	{
		GBA_HALFWORD offset : 11;
		GBA_HALFWORD h : 1;
		GBA_HALFWORD : 4;
	} longBranchWithLink_t;

	typedef struct thumbUndefined
	{
		GBA_HALFWORD TODO : 16;
	} thumbUndefined_t;

	// ARM Instructions

	typedef struct dataProcessing {
		GBA_WORD operand2 : 12;
		GBA_WORD rd : 4;
		GBA_WORD rn : 4;
		GBA_WORD s : 1;
		GBA_WORD opcode : 4;
		GBA_WORD immediate : 1;
		GBA_WORD : 2;
		GBA_WORD cond : 4;
	} dataProcessing_t;

	typedef struct multiply {
		GBA_WORD rm : 4;
		GBA_WORD : 4;
		GBA_WORD rs : 4;
		GBA_WORD rn : 4;
		GBA_WORD rd : 4;
		GBA_WORD s : 1;
		GBA_WORD a : 1;
		GBA_WORD : 6;
		GBA_WORD cond : 4;
	} multiply_t;

	typedef struct multiplyLong {
		GBA_WORD rm : 4;
		GBA_WORD : 4;
		GBA_WORD rs : 4;
		GBA_WORD rdlo : 4;
		GBA_WORD rdhi : 4;
		GBA_WORD s : 1;
		GBA_WORD a : 1;
		GBA_WORD u : 1;
		GBA_WORD : 5;
		GBA_WORD cond : 4;
	} multiplyLong_t;

	typedef struct singleDataSwap {
		GBA_WORD rm : 4;
		GBA_WORD : 8;
		GBA_WORD rd : 4;
		GBA_WORD rn : 4;
		GBA_WORD : 2;
		GBA_WORD b : 1;
		GBA_WORD : 5;
		GBA_WORD cond : 4;
	} singleDataSwap_t;

	typedef struct branchExchange {
		GBA_WORD rn : 4;
		GBA_WORD opcode : 4;
		GBA_WORD : 20;
		GBA_WORD cond : 4;
	} branchExchange_t;

	typedef struct halfwordDataTransferRO {
		GBA_WORD rm : 4;
		GBA_WORD : 1;
		GBA_WORD h : 1;
		GBA_WORD s : 1;
		GBA_WORD : 5;
		GBA_WORD rd : 4;
		GBA_WORD rn : 4;
		GBA_WORD l : 1;
		GBA_WORD w : 1;
		GBA_WORD : 1; // this is the immediate flag, always a 0 here, 1 in halfwordDataTransferIO
		GBA_WORD u : 1;
		GBA_WORD p : 1;
		GBA_WORD : 3;
		GBA_WORD cond : 4;
	} halfwordDataTransferRO_t;

	typedef struct halfwordDataTransferIO {
		GBA_WORD offset_low : 4; // low 4 bits of 8 bit offset
		GBA_WORD : 1;
		GBA_WORD h : 1;
		GBA_WORD s : 1;
		GBA_WORD : 1;
		GBA_WORD offset_high : 4; // high 4 bits of 8 bit offset
		GBA_WORD rd : 4;
		GBA_WORD rn : 4;
		GBA_WORD l : 1;
		GBA_WORD w : 1;
		GBA_WORD : 1; // this is the immediate flag, always a 1 here, 0 in halfwordDataTransferRO
		GBA_WORD u : 1;
		GBA_WORD p : 1;
		GBA_WORD : 3;
		GBA_WORD cond : 4;
	} halfwordDataTransferIO_t;

	typedef struct singleDataTransfer {
		GBA_WORD offset : 12;
		GBA_WORD rd : 4;
		GBA_WORD rn : 4;
		GBA_WORD l : 1;
		GBA_WORD w : 1;
		GBA_WORD b : 1;
		GBA_WORD u : 1;
		GBA_WORD p : 1;
		GBA_WORD i : 1;
		GBA_WORD : 2;
		GBA_WORD cond : 4;
	} singleDataTransfer_t;

	typedef struct undefined {
		GBA_WORD TODO : 32;
	} undefined_t;

	typedef struct blockDataTransfer {
		GBA_WORD rlist : 16;
		GBA_WORD rn : 4;
		GBA_WORD l : 1;
		GBA_WORD w : 1;
		GBA_WORD s : 1;
		GBA_WORD u : 1;
		GBA_WORD p : 1;
		GBA_WORD : 3;
		GBA_WORD cond : 4;
	} blockDataTransfer_t;

	typedef struct branch {
		GBA_WORD offset : 24; // This value is actually signed, but needs to be this way because of how C works
		GBA_WORD link : 1;
		GBA_WORD : 3;
		GBA_WORD cond : 4;
	} branch_t;

	typedef struct coprocessorDataTransfer {
		GBA_WORD TODO : 32;
	} coprocessorDataTransfer_t;

	typedef struct coprocessorDataOperation {
		GBA_WORD TODO : 32;
	} coprocessorDataOperation_t;

	typedef struct coprocessorRegisterTransfer {
		GBA_WORD TODO : 32;
	} coprocessorRegisterTransfer_t;

	typedef struct softwareInterrupt {
		GBA_WORD comment : 24;
		GBA_WORD opcode : 4;
		GBA_WORD cond : 4;
	} softwareInterrupt_t;

	// ARM7TDMI Architecture

	typedef union
	{
		struct
		{
			union
			{
				moveShiftedRegister_t MOVE_SHIFTED_REGISTER;
				addSubtract_t ADD_SUBTRACT;
				moveCompareAddSubtractImmediate_t MOVE_COMPARE_ADD_SUBTRACT_IMMEDIATE;
				aluOperations_t ALU_OPERATIONS;
				highRegisterOperations_t HIGH_REGISTER_OPERATIONS;
				pcRelativeLoad_t PC_RELATIVE_LOAD;
				loadStoreRO_t LOAD_STORE_RO;
				loadStoreSignedByteHalfword_t LOAD_STORE_SIGNED_BYTE_HALFWORD;
				loadStoreIO_t LOAD_STORE_IO;
				loadStoreHalfword_t LOAD_STORE_HALFWORD;
				spRelativeLoadStore_t SP_RELATIVE_LOAD_STORE;
				loadAddress_t LOAD_ADDRESS;
				addOffsetToStackPointer_t ADD_OFFSET_TO_STACK_POINTER;
				pushPopRegisters_t PUSH_POP_REGISTERS;
				multipleLoadStore_t MULTIPLE_LOAD_STORE;
				conditionalBranch_t CONDITIONAL_BRANCH;
				thumbSoftwareInterrupt_t THUMB_SOFTWARE_INTERRUPT;
				unconditionalBranch_t UNCONDITIONAL_BRANCH;
				longBranchWithLink_t LONG_BRANCH_WITH_LINK;
				thumbUndefined_t THUMB_UNDEFINED;
			};
			GBA_HALFWORD unused;
		} thumb;
		struct
		{
			union
			{
				struct
				{
					GBA_WORD remaining : 28;
					GBA_WORD cond : 4;
				};
				dataProcessing_t DATA_PROCESSING;
				multiply_t MULTIPLY;
				multiplyLong_t MULTIPLY_LONG;
				singleDataSwap_t SINGLE_DATA_SWAP;
				branchExchange_t BRANCH_EXCHANGE;
				halfwordDataTransferRO_t HALFWORD_DATATRANSFER_RO;
				halfwordDataTransferIO_t HALFWORD_DATATRANSFER_IO;
				singleDataTransfer_t SINGLE_DATA_TRANSFER;
				undefined_t UNDEFINED;
				blockDataTransfer_t BLOCK_DATA_TRANSFER;
				branch_t BRANCH;
				coprocessorDataTransfer_t COPROCESSOR_DATA_TRANSFER;
				coprocessorDataOperation_t COPROCESSOR_DATA_OPERATION;
				coprocessorRegisterTransfer_t COPROCESSOR_REGISTER_TRANSFER;
				softwareInterrupt_t SOFTWARE_INTERRUPT;
			};
		} arm;
		GBA_WORD rawOpCode;
	} opCode_t;

	typedef struct
	{
		opCode_t opCode;
	} opCodeInfo_t;

	typedef struct
	{
		opCodeInfo_t fetchStageOpCode;
		opCodeInfo_t decodeStageOpCode;
		opCodeInfo_t executeStageOpCode;
	} pipeline_t;

	typedef struct
	{
		HALT_CONTROLLER haltCntState;
		pipeline_t pipeline;
		registers_t registers;
		OP_MODE_TYPE armMode;
		STATE_TYPE armState;
	} cpu_t;

private:

	std::unordered_map<ID64, std::string> disassembler;

#pragma endregion ARM7TDMI_DECLARATIONS

#pragma region EMULATION_DECLARATIONS
private:

	bios_t gba_bios;

private:

	enum TICK_TYPE : uint8_t
	{
		CPU_TICK,
		DMA_TICK
	};

	enum MEMORY_REGIONS : uint8_t
	{
		REGION_SYS_ROM = 0x00,
		REGION_DUMMY = 0x01,
		REGION_EWRAM = 0x02,
		REGION_IWRAM = 0x03,
		REGION_IOREG = 0x04,
		REGION_PRAM = 0x05,
		REGION_VRAM = 0x06,
		REGION_OAM = 0x07,
		REGION_FLASH_ROM0_L = 0x08,
		REGION_FLASH_ROM0_H = 0x09,
		REGION_FLASH_ROM1_L = 0x0A,
		REGION_FLASH_ROM1_H = 0x0B,
		REGION_FLASH_ROM2_L = 0x0C,
		REGION_FLASH_ROM2_H = 0x0D,
		REGION_GAMEPAK_SRAM = 0x0E,
		REGION_GAMEPAK_SRAM_MIRR = 0x0F,
		TOTAL_MEMORY_REGIONS = 0x10
	};

	enum class MEMORY_ACCESS_SOURCE
	{
		HOST,
		CPU,
		PPU,
		APU,
		DMA
	};

	enum class GBA_INTERRUPT
	{
		IRQ_VBLANK = ZERO,
		IRQ_HBLANK = ONE,
		IRQ_VCOUNT = TWO,
		IRQ_TIMER0 = THREE,
		IRQ_TIMER1 = FOUR,
		IRQ_TIMER2 = FIVE,
		IRQ_TIMER3 = SIX,
		IRQ_SERIAL = SEVEN,
		IRQ_DMA0 = EIGHT,
		IRQ_DMA1 = NINE,
		IRQ_DMA2 = TEN,
		IRQ_DMA3 = ELEVEN,
		IRQ_KEYPAD = TWELVE,
		IRQ_GAMEPAK = THIRTEEN
	};

	enum TIMER : uint8_t
	{
		TIMER0,
		TIMER1,
		TIMER2,
		TIMER3,
		TOTAL_TIMER
	};

	enum DMA : uint8_t
	{
		DMA0,
		DMA1,
		DMA2,
		DMA3,
		TOTAL_DMA,
		NO_DMA
	};

	enum class DMA_SIZE
	{
		HALFWORD_PER_TRANSFER,
		WORD_PER_TRANSFER,
		TOTAL
	};

	enum class DMA_CONTROL
	{
		INCREMENT,
		DECREMENT,
		FIXED,
		RELOAD,
		TOTAL
	};

	enum DMA_TIMING : uint16_t
	{
		IMMEDIATE,
		VBLANK,
		HBLANK,
		SPECIAL,
		TOTAL_DMA_TIMING,
		NO_DMA_TIMING
	};

	enum class DMA_SPECIAL_TIMING
	{
		FIFO0,
		FIFO1,
		VIDEO,
		TOTAL,
		NONE
	};

	enum class DMA_STATE
	{
		DMA_COMPLETE,
		DMA_RESTART,
		DMA_TRIGGER_MISMATCH,
		DMA_REPEAT,
		DMA_NO_RUN
	};

	enum class JOYPAD_STATES
	{
		PRESSED = ZERO,
		RELEASED = ONE
	};

	enum AUDIO_CHANNELS : uint8_t
	{
		CHANNEL_1 = ZERO,
		CHANNEL_2 = ONE,
		CHANNEL_3 = TWO,
		CHANNEL_4 = THREE,
		CHANNEL_A = FOUR,
		CHANNEL_B = FIVE,
		TOTAL_CHANNELS
	};

	enum class AUDIO_STREAMS
	{
		L = ZERO,
		R = ONE,
		TOTAL_AUDIO_STREAMS
	};

	const uint16_t AUDIO_CHANNEL_4_DIVISOR[EIGHT]
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

	const GBA_AUDIO_SAMPLE_TYPE SQUARE_WAVE_AMPLITUDE[FOUR][EIGHT] =
	{
		{ +EIGHT, -EIGHT, -EIGHT, -EIGHT, -EIGHT, -EIGHT, -EIGHT, -EIGHT },
		{ +EIGHT, +EIGHT, -EIGHT, -EIGHT, -EIGHT, -EIGHT, -EIGHT, -EIGHT },
		{ +EIGHT, +EIGHT, +EIGHT, +EIGHT, -EIGHT, -EIGHT, -EIGHT, -EIGHT },
		{ +EIGHT, +EIGHT, +EIGHT, +EIGHT, +EIGHT, +EIGHT, -EIGHT, -EIGHT }
	};

	enum class LCD_MODES
	{
		MODE_LCD_H_DRAW_V_DRAW = ZERO,
		MODE_LCD_H_BLANK_V_DRAW = ONE,
		MODE_LCD_H_DRAW_V_BLANK = TWO,
		MODE_LCD_H_BLANK_V_BLANK = THREE,
		MODE_TOTAL_LCD_MODES = FOUR
	};

	enum class LCD_MODE_CYCLES
	{
		CYCLES_LCD_H_VISIBLE = 1007 /*960*/, // Refer http://problemkaputt.de/gbatek-lcd-i-o-interrupts-and-status.htm
		CYCLES_LCD_H_BLANK = 225 /*272*/, // Refer http://problemkaputt.de/gbatek-lcd-i-o-interrupts-and-status.htm
		CYCLES_LCD_V_BLANK = 83776
	};

	enum class LCD_DIMENSIONS
	{
		LCD_VISIBLE_PIXEL_PER_LINES = 240,
		LCD_VISIBLE_LINES = 160,
		LCD_V_BLANK_LINES = 68,
		LCD_TOTAL_V_LINES = 228
	};

	enum class OBJECT_TYPE
	{
		OBJECT_IS_INVALID,
		OBJECT_IS_NOT_AFFINE,
		OBJECT_IS_AFFINE
	};

	enum class OBJECT_STAGE
	{
		OBJECT_FETCH_STAGE,
		OBJECT_RENDER_STAGE,
		OBJECT_TOTAL_STAGE
	};

	enum class OBJECT_ACCESS_PATTERN
	{
		OBJECT_A01,
		OBJECT_A2,
		OBJECT_PA,
		OBJECT_PB,
		OBJECT_PC,
		OBJECT_PD,
		OBJECT_BLANK_A01,
		OBJECT_V
	};

	enum class COLOR_SPECIAL_EFFECTS
	{
		NORMAL,
		ALPHA_BLENDING,
		INCREASE_BRIGHTNESS,
		DECREASE_BRIGHTNESS
	};

	enum class OBJECT_MODE
	{
		NORMAL,
		ALPHA_BLENDING,
		OBJ_WINDOW
	};

	enum class SIO_MODE
	{
		NORMAL_8BIT,
		NORMAL_32BIT,
		MULTIPLAY_16BIT,
		UART,
		GP,
		JOYBUS,
		UNKWOWN_SIO_MODE
	};

	enum class SIO_PARTY
	{
		SLAVE,
		MASTER
	};

	enum class SIO_RATE
	{
		SIO_256KHz,
		SIO_2MHz
	};

	enum class BACKUP_TYPE
	{
		UNKNOWN,
		SRAM,
		EEPROM,
		FLASH64K,
		FLASH128K
	};

	enum class BACKUP_FLASH_CMDS
	{
		NO_OPERATION = 0x00,
		CMD_1 = 0xAA,
		CMD_2 = 0x55,
		ENTER_CHIP_INDENTIFICATION_MODE = 0x90,
		EXIT_CHIP_INDENTIFICATION_MODE = 0xF0,
		START_ERASE_CMD = 0x80,
		ERASE_ENTIRE_CHIP = 0x10,
		ERASE_4KB_SECTOR = 0x30,
		START_1BYTE_WRITE_CMD = 0xA0,
		SET_MEMORY_BANK = 0xB0
	};

	enum class BACKUP_FLASH_FSM
	{
		STATE0,
		STATE1,
		STATE2
	};

	enum class BACKUP_FLASH_MEMORY_BANK
	{
		BANK0,
		BANK1,
		TOTAL
	};

	// Refer to http://problemkaputt.de/gbatek-gba-memory-map.htm
	COUNTER32 WAIT_CYCLES
		[(uint8_t)MEMORY_REGIONS::TOTAL_MEMORY_REGIONS]
		[(uint8_t)MEMORY_ACCESS_TYPE::TOTAL_MEMORY_ACCESS_TYPES]
		[(uint8_t)MEMORY_ACCESS_WIDTH::TOTAL_ACCESS_WIDTH_POSSIBLE]
		=
	{
		/* SYSTEM ROM */
		{
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE},			// NON-SEQUENTIAL
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE}				// SEQUENTIAL
		},
		/* DUMMY */
		{
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE},			// NON-SEQUENTIAL
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE}				// SEQUENTIAL
		},
		/* EXT WRAM */
		{
			{/*8-bit*/ THREE, /*16-bit*/ THREE, /*32-bit*/ SIX},		// NON-SEQUENTIAL
			{/*8-bit*/ THREE, /*16-bit*/ THREE, /*32-bit*/ SIX}			// SEQUENTIAL
		},
		/* INT WRAM */
		{
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE},			// NON-SEQUENTIAL
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE}				// SEQUENTIAL
		},
		/* IO REGISTERS */
		{
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE},			// NON-SEQUENTIAL
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE}				// SEQUENTIAL
		},
		/* PALETTE RAM */
		{
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ TWO},			// NON-SEQUENTIAL
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ TWO}				// SEQUENTIAL
		},
		/* VRAM */
		{
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ TWO},			// NON-SEQUENTIAL
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ TWO}				// SEQUENTIAL
		},
		/* OAM */
		{
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE},			// NON-SEQUENTIAL
			{/*8-bit*/ ONE, /*16-bit*/ ONE, /*32-bit*/ ONE}				// SEQUENTIAL
		},
		/* FLASH ROM 0 L */
		{
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT},		// NON-SEQUENTIAL
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT}			// SEQUENTIAL
		},
		/* FLASH ROM 0 H */
		{
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT},		// NON-SEQUENTIAL
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT}			// SEQUENTIAL
		},
		/* FLASH ROM 1 L */
		{
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT},		// NON-SEQUENTIAL
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT}			// SEQUENTIAL
		},
		/* FLASH ROM 1 H */
		{
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT},		// NON-SEQUENTIAL
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT}			// SEQUENTIAL
		},
		/* FLASH ROM 2 L */
		{
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT},		// NON-SEQUENTIAL
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT}			// SEQUENTIAL
		},
		/* FLASH ROM 2 H */
		{
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT},		// NON-SEQUENTIAL
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ EIGHT}			// SEQUENTIAL
		},
		/* GAMEPAK SRAM */
		{
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ FIVE},			// NON-SEQUENTIAL
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ FIVE}			// SEQUENTIAL
		},
		/* GAMEPAK SRAM MIRROR */
		{
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ FIVE},			// NON-SEQUENTIAL
			{/*8-bit*/ FIVE, /*16-bit*/ FIVE, /*32-bit*/ FIVE}			// SEQUENTIAL
		}
	};

private:

	// Define lookup tables for horizontal and vertical flipping
	const BYTE flipLUT[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
	const COUNTER32 WIN_WAIT_CYCLES = ZERO;
	const COUNTER32 OBJECT_WAIT_CYCLES = FORTY;
	const COUNTER32 BG_WAIT_CYCLES = THIRTYTWO;
	const COUNTER32 MERGE_WAIT_CYCLES = FORTYSIX;

	const BYTE ALPHA = 255;

	typedef union
	{
		struct
		{
			GBA_HALFWORD RED : 5; // bit  0 - 4
			GBA_HALFWORD GREEN : 5; // bit  5 - 9
			GBA_HALFWORD BLUE : 5; // bit  10 - 14
			GBA_HALFWORD UNUSED : 1; // bit  15	
		};
		GBA_HALFWORD raw;
	} gbaColor_t;

	gbaColor_t GBA_BLACK = { {.RED = 0x00, .GREEN = 0x00, .BLUE = 0x00 } };
	gbaColor_t GBA_WHITE = { {.RED = 0x1F, .GREEN = 0x1F, .BLUE = 0x1F } };

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

	const char* LICENSE_CODE[0xA5] =
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

private:

	typedef struct
	{
		BYTE romEntryPoint[0x0004];
		BYTE nintendologo[0x009C];
		BYTE gametitle[0x000C];
		BYTE gameCode[0x0004];
		BYTE makerCode[0x0002];
		BYTE fixedValue;
		BYTE mainUnitCode;
		BYTE deviceType;
		BYTE reservedArea00[0x0007];
		BYTE softwareVersion;
		BYTE complementCheck;
		BYTE reservedArea01[0x0002];
	} cartridge_header_SB_fields_t;

	typedef struct
	{
		BYTE romEntryPoint[0x0004];
		BYTE nintendologo[0x009C];
		BYTE gametitle[0x000C];
		BYTE gameCode[0x0004];
		BYTE makerCode[0x0002];
		BYTE fixedValue;
		BYTE mainUnitCode;
		BYTE deviceType;
		BYTE reservedArea00[0x0007];
		BYTE softwareVersion;
		BYTE complementCheck;
		BYTE reservedArea01[0x0002];
		BYTE ramEntryPoint[0x0004];
		BYTE bootMode;
		BYTE slaveIDNumber;
		BYTE notUsed00[0x001A];
		BYTE joybusEntryPoint[0x0004];
	} cartridge_header_MB_fields_t;

	typedef union
	{
		cartridge_header_SB_fields_t cartridge_header_SB_fields;
		uint8_t cartridge_header_SB_buffer[sizeof(cartridge_header_SB_fields_t)];
	} cartridge_header_SB_t;

	typedef union
	{
		cartridge_header_MB_fields_t cartridge_header_MB_fields;
		uint8_t cartridge_header_MB_buffer[sizeof(cartridge_header_MB_fields_t)];
	} cartridge_header_MB_t;

private:

	typedef union
	{
		BYTE mSystemRom8bit[0x4000];
		uint16_t mSystemRom16bit[0x4000 / TWO];
		uint32_t mSystemRom32bit[0x4000 / FOUR];
	} mSystemRom_t;

	typedef union
	{
		BYTE mExtWorkRam8bit[0x40000];
		uint16_t mExtWorkRam16bit[0x40000 / TWO];
		uint32_t mExtWorkRam32bit[0x40000 / FOUR];
	} mExtWorkRam_t;

	typedef union
	{
		BYTE mIntWorkRam8bit[0x8000];
		uint16_t mIntWorkRam16bit[0x8000 / TWO];
		uint32_t mIntWorkRam32bit[0x8000 / FOUR];
	} mIntWorkRam_t;

	// DISPCNT (16-bit)
	typedef struct
	{
		uint16_t BG_MODE : 3; // bit  0 - 2
		uint16_t CGB_MODE : 1; // bit  3
		uint16_t FRAME_SELECT : 1; // bit  4
		uint16_t HBLANK_INTERVAL_FREE : 1; // bit  5	
		uint16_t OBJ_CHAR_VRAM_MAP : 1; // bit  6
		uint16_t FORCED_BLANK : 1; // bit  7
		uint16_t SCREEN_DISP_BG0 : 1; // bit  8
		uint16_t SCREEN_DISP_BG1 : 1; // bit  9
		uint16_t SCREEN_DISP_BG2 : 1; // bit  10
		uint16_t SCREEN_DISP_BG3 : 1; // bit  11
		uint16_t SCREEN_DISP_OBJ : 1; // bit  12
		uint16_t WIN0_DISP_FLAG : 1; // bit  13
		uint16_t WIN1_DISP_FLAG : 1; // bit  14
		uint16_t OBJ_DISP_FLAG : 1; // bit  15
	} mDISPCNTFields_t;

	typedef union
	{
		mDISPCNTFields_t mDISPCNTFields;
		uint16_t mDISPCNTHalfWord;
	} mDISPCNTHalfWord_t;

	// GREENSWAP (16-bit)
	typedef struct
	{
		uint16_t GREEN_SWAP : 1; // bit  0
		uint16_t CGB_MODE : 15; // bit  1 - 15
	} mGREENSWAPFields_t;

	typedef union
	{
		mGREENSWAPFields_t mGREENSWAPFields;
		uint16_t mGREENSWAPHalfWord;
	} mGREENSWAPHalfWord_t;

	// DISPSTAT (16-bit)
	typedef struct
	{
		uint16_t VBLANK_FLAG : 1; // bit  0
		uint16_t HBLANK_FLAG : 1; // bit  1
		uint16_t VCOUNT_FLAG : 1; // bit  2
		uint16_t VBLANK_IRQ_ENABLE : 1; // bit  3
		uint16_t HBLANK_IRQ_ENABLE : 1; // bit  4
		uint16_t VCOUNTER_IRQ_ENABLE : 1; // bit  5	
		uint16_t NOT_USED_0 : 1; // bit  6
		uint16_t NOT_USED_1 : 1; // bit  7
		uint16_t VCOUNT_SETTING_LYC : 8; // bit  8 - 15
	} mDISPSTATFields_t;

	typedef union
	{
		mDISPSTATFields_t mDISPSTATFields;
		uint16_t mDISPSTATHalfWord;
	} mDISPSTATHalfWord_t;

	// VCOUNT (16-bit)
	typedef struct
	{
		uint16_t CURRENT_SCANLINE_LY : 8; // bit  0 - 7
		uint16_t NDS_CURRENT_SCANLINE_LY_MSB : 1; // bit  8
		uint16_t NOT_USED_0 : 7; // bit  9 - 15
	} mVCOUNTFields_t;

	typedef union
	{
		mVCOUNTFields_t mVCOUNTFields;
		uint16_t mVCOUNTHalfWord;
	} mVCOUNTHalfWord_t;

	// BGnCNT (16-bit)
	typedef struct
	{
		uint16_t BG_PRIORITY : 2; // bit  0 - 1
		uint16_t CHARACTER_BASE_BLOCK : 2; // bit  2 - 3
		uint16_t NDS_CHARACTER_BASE_BLOCK_MSBs : 2; // bit  4 - 5
		uint16_t MOSAIC : 1; // bit  6
		uint16_t COLOR_PALETTES : 1; // bit  7
		uint16_t SCREEN_BASE_BLOCK : 5; // bit  8 - 12	
		uint16_t BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT : 1; // bit  13
		uint16_t SCREEN_SIZE : 2; // bit  14 - 15
	} mBGnCNTFields_t;

	typedef union
	{
		mBGnCNTFields_t mBGnCNTFields;
		uint16_t mBGnCNTHalfWord;
	} mBGnCNTHalfWord_t;

	// BGniOFS (16-bit)
	typedef struct
	{
		uint16_t OFFSET : 9; // bit  0 - 8
		uint16_t NOT_USED_0 : 7; // bit  9 - 15
	} mBGniOFSFields_t;

	typedef union
	{
		mBGniOFSFields_t mBGniOFSFields;
		uint16_t mBGniOFSHalfWord;
	} mBGniOFSHalfWord_t;

	// BGni (32-bit)
	typedef struct
	{
		uint32_t FRAC_PORTION : 8; // bit  0 - 7
		uint32_t INT_PORTION : 19; // bit  8 - 26
		uint32_t SIGN : 1; // bit  27
		uint32_t NOT_USED_0 : 4; // bit  28 - 31
	} mBGniFields_t;

	typedef union
	{
		mBGniFields_t mBGniFields;
		struct
		{
			uint32_t mBGniWord_L : 16; // bit  0 - 15
			uint32_t mBGniWord_H : 16; // bit  16 - 31
		} mBGniHalfWords;
		uint32_t mBGniWord;
		int32_t mBGniWord_Signed;
	} mBGniWord_t;

	// BGnPx (16-bit)
	typedef struct
	{
		uint16_t FRAC_PORTION : 8; // bit  0 - 7
		uint16_t INT_PORTION : 7; // bit  8 - 14
		uint16_t SIGN : 1; // bit  15
	} mBGnPxFields_t;

	typedef union
	{
		mBGnPxFields_t mBGnPxFields;
		uint16_t mBGnPxHalfWord;
		int16_t mBGnPxHalfWord_Signed;
	} mBGnPxHalfWord_t;

	// WINni (16-bit)
	typedef struct
	{
		uint16_t i2 : 8; // bit  0 - 7
		uint16_t i1 : 8; // bit  8 - 15
	} mWINniFields_t;

	typedef union
	{
		mWINniFields_t mWINniFields;
		uint16_t mWINniHalfWord;
	} mWINniHalfWord_t;

	// WININ (16-bit)
	typedef struct
	{
		uint16_t WIN0_BG_0_EN : 1; // bit  0
		uint16_t WIN0_BG_1_EN : 1; // bit  1
		uint16_t WIN0_BG_2_EN : 1; // bit  2
		uint16_t WIN0_BG_3_EN : 1; // bit  3 
		uint16_t WIN0_OBJ_EN : 1; // bit  4
		uint16_t WIN0_COLOR_SPL_EFFECT : 1; // bit  5
		uint16_t NOT_USED_0 : 2; // bit  6 - 7
		uint16_t WIN1_BG_0_EN : 1; // bit  8
		uint16_t WIN1_BG_1_EN : 1; // bit  9 
		uint16_t WIN1_BG_2_EN : 1; // bit  10 
		uint16_t WIN1_BG_3_EN : 1; // bit  11 
		uint16_t WIN1_OBJ_EN : 1; // bit  12
		uint16_t WIN1_COLOR_SPL_EFFECT : 1; // bit  13
		uint16_t NOT_USED_1 : 2; // bit  14 - 15
	} mWININFields_t;

	typedef union
	{
		mWININFields_t mWININFields;
		uint16_t mWININHalfWord;
	} mWININHalfWord_t;

	// WINOUT (16-bit)
	typedef struct
	{
		uint16_t OUTSIDE_BG_0_EN : 1; // bit  0
		uint16_t OUTSIDE_BG_1_EN : 1; // bit  1
		uint16_t OUTSIDE_BG_2_EN : 1; // bit  2
		uint16_t OUTSIDE_BG_3_EN : 1; // bit  3 
		uint16_t OUTSIDE_OBJ_EN : 1; // bit  4
		uint16_t OUTSIDE_COLOR_SPL_EFFECT : 1; // bit  5
		uint16_t NOT_USED_0 : 2; // bit  6 - 7
		uint16_t OBJ_WIN_BG_0_EN : 1; // bit  8 
		uint16_t OBJ_WIN_BG_1_EN : 1; // bit  9  
		uint16_t OBJ_WIN_BG_2_EN : 1; // bit  10  
		uint16_t OBJ_WIN_BG_3_EN : 1; // bit  11  
		uint16_t OBJ_WIN_OBJ_EN : 1; // bit  12
		uint16_t OBJ_WIN_COLOR_SPL_EFFECT : 1; // bit  13
		uint16_t NOT_USED_1 : 2; // bit  14 - 15
	} mWINOUTFields_t;

	typedef union
	{
		mWINOUTFields_t mWINOUTFields;
		uint16_t mWINOUTHalfWord;
	} mWINOUTHalfWord_t;

	// MOSAIC (16-bit)
	typedef struct
	{
		uint16_t BG_MOSAIC_H_SIZE : 4; // bit  0 - 3
		uint16_t BG_MOSAIC_V_SIZE : 4; // bit  4 - 7
		uint16_t OBJ_MOSAIC_H_SIZE : 4; // bit  8 - 11
		uint16_t OBJ_MOSAIC_V_SIZE : 4; // bit  12 - 15
	} mMOSAICFields_t;

	typedef union
	{
		mMOSAICFields_t mMOSAICFields;
		uint16_t mMOSAICHalfWord;
	} mMOSAICHalfWord_t;

	// BLDCNT (16-bit)
	typedef struct
	{
		uint16_t BG0_1ST_TARGET_PIXEL : 1; // bit  0
		uint16_t BG1_1ST_TARGET_PIXEL : 1; // bit  1
		uint16_t BG2_1ST_TARGET_PIXEL : 1; // bit  2
		uint16_t BG3_1ST_TARGET_PIXEL : 1; // bit  3
		uint16_t OBJ_1ST_TARGET_PIXEL : 1; // bit  4
		uint16_t BD_1ST_TARGET_PIXEL : 1; // bit  5
		uint16_t COLOR_SPL_EFFECTS : 2; // bit  6 - 7
		uint16_t BG0_2ND_TARGET_PIXEL : 1; // bit  8
		uint16_t BG1_2ND_TARGET_PIXEL : 1; // bit  9
		uint16_t BG2_2ND_TARGET_PIXEL : 1; // bit  10
		uint16_t BG3_2ND_TARGET_PIXEL : 1; // bit  11
		uint16_t OBJ_2ND_TARGET_PIXEL : 1; // bit  12
		uint16_t BD_2ND_TARGET_PIXEL : 1; // bit  13
		uint16_t NOT_USED_0 : 2; // bit  14 - 15
	} mBLDCNTFields_t;

	typedef union
	{
		mBLDCNTFields_t mBLDCNTFields;
		uint16_t mBLDCNTHalfWord;
	} mBLDCNTHalfWord_t;

	// BLDALPHA (16-bit)
	typedef struct
	{
		uint16_t EVA_COEFF : 5; // bit  0 - 4
		uint16_t NOT_USED_0 : 3; // bit  5 - 7
		uint16_t EVB_COEFF : 5; // bit  8 - 12
		uint16_t NOT_USED_1 : 3; // bit  13 - 15
	} mBLDALPHAFields_t;

	typedef union
	{
		mBLDALPHAFields_t mBLDALPHAFields;
		uint16_t mBLDALPHAHalfWord;
	} mBLDALPHAHalfWord_t;

	// BLDY (16-bit)
	typedef struct
	{
		uint16_t EVY_COEFF : 5; // bit  0 - 4
		uint16_t NOT_USED_0 : 11; // bit  5 - 15
	} mBLDYFields_t;

	typedef union
	{
		mBLDYFields_t mBLDYFields;
		uint16_t mBLDYHalfWord;
	} mBLDYHalfWord_t;

	// SOUND1CNT_L (16-bit)
	typedef struct
	{
		uint16_t SWEEP_SHIFT : 3; // bit  0 - 2
		uint16_t SWEEP_FREQ_DIR : 1; // bit  3
		uint16_t SWEEP_TIME : 3; // bit  4 - 6
		uint16_t NOT_USED_0 : 9; // bit  7 - 15
	} mSOUND1CNT_LFields_t;

	typedef union
	{
		mSOUND1CNT_LFields_t mSOUND1CNT_LFields;
		uint16_t mSOUND1CNT_LHalfWord;
	} mSOUND1CNT_LHalfWord_t;

	// SOUND1CNT_H (16-bit)
	typedef struct
	{
		uint16_t SOUND_LENGTH : 6; // bit  0 - 5
		uint16_t WAVE_PATTERN_DUTY : 2; // bit  6 - 7
		uint16_t ENVP_STEP_TIME : 3; // bit  8 - 10
		uint16_t ENVP_DIR : 1; // bit  11
		uint16_t ENVP_INIT_VOL : 4; // bit  12 - 15
	} mSOUND1CNT_HFields_t;

	typedef union
	{
		mSOUND1CNT_HFields_t mSOUND1CNT_HFields;
		uint16_t mSOUND1CNT_HHalfWord;
	} mSOUND1CNT_HHalfWord_t;

	// SOUND1CNT_X (16-bit)
	typedef struct
	{
		uint16_t FREQ : 11; // bit  0 - 10
		uint16_t NOT_USED_0 : 3; // bit  11 - 13
		uint16_t LENGTH_FLAG : 1; // bit  14
		uint16_t INITIAL : 1; // bit  15
	} mSOUND1CNT_XFields_t;

	typedef union
	{
		mSOUND1CNT_XFields_t mSOUND1CNT_XFields;
		uint16_t mSOUND1CNT_XHalfWord;
	} mSOUND1CNT_XHalfWord_t;

	// SOUND2CNT_L (16-bit)
	typedef struct
	{
		uint16_t SOUND_LENGTH : 6; // bit  0 - 5
		uint16_t WAVE_PATTERN_DUTY : 2; // bit  6 - 7
		uint16_t ENVP_STEP_TIME : 3; // bit  8 - 10
		uint16_t ENVP_DIR : 1; // bit  11
		uint16_t ENVP_INIT_VOL : 4; // bit  12 - 15
	} mSOUND2CNT_LFields_t;

	typedef union
	{
		mSOUND2CNT_LFields_t mSOUND2CNT_LFields;
		uint16_t mSOUND2CNT_LHalfWord;
	} mSOUND2CNT_LHalfWord_t;

	// SOUND2CNT_H (16-bit)
	typedef struct
	{
		uint16_t FREQ : 11; // bit  0 - 10
		uint16_t NOT_USED_0 : 3; // bit  11 - 13
		uint16_t LENGTH_FLAG : 1; // bit  14
		uint16_t INITIAL : 1; // bit  15
	} mSOUND2CNT_HFields_t;

	typedef union
	{
		mSOUND2CNT_HFields_t mSOUND2CNT_HFields;
		uint16_t mSOUND2CNT_HHalfWord;
	} mSOUND2CNT_HHalfWord_t;

	// SOUND3CNT_L (16-bit)
	typedef struct
	{
		uint16_t NOT_USED_0 : 5; // bit  0 - 4
		uint16_t WAVE_RAM_DIMENSION : 1; // bit  5
		uint16_t WAVE_RAM_BANK_NUMBER : 1; // bit  6
		uint16_t CHANNEL_3_OFF : 1; // bit  7
		uint16_t NOT_USED_1 : 8; // bit  8 - 15
	} mSOUND3CNT_LFields_t;

	typedef union
	{
		mSOUND3CNT_LFields_t mSOUND3CNT_LFields;
		uint16_t mSOUND3CNT_LHalfWord;
	} mSOUND3CNT_LHalfWord_t;

	// SOUND3CNT_H (16-bit)
	typedef struct
	{
		uint16_t SOUND_LENGTH : 8; // bit  0 - 7
		uint16_t NOT_USED_0 : 5; // bit  8 - 12
		uint16_t SOUND_VOL : 2; // bit  13 - 14
		uint16_t FORCE_VOL : 1; // bit  15
	} mSOUND3CNT_HFields_t;

	typedef union
	{
		mSOUND3CNT_HFields_t mSOUND3CNT_HFields;
		uint16_t mSOUND3CNT_HHalfWord;
	} mSOUND3CNT_HHalfWord_t;

	// SOUNDnCNT_X (16-bit)
	typedef struct
	{
		uint16_t SAMPLE_RATE : 11; // bit  0 - 10
		uint16_t NOT_USED_0 : 3; // bit  11 - 13
		uint16_t LENGTH_FLAG : 1; // bit  14
		uint16_t INITIAL : 1; // bit  15
	} mSOUND3CNT_XFields_t;

	typedef union
	{
		mSOUND3CNT_XFields_t mSOUND3CNT_XFields;
		uint16_t mSOUND3CNT_XHalfWord;
	} mSOUND3CNT_XHalfWord_t;

	// SOUND4CNT_L (16-bit)
	typedef struct
	{
		uint16_t SOUND_LENGTH : 6; // bit  0 - 5
		uint16_t NOT_USED_0 : 2; // bit  6 - 7
		uint16_t ENVP_STEP_TIME : 3; // bit  8 - 10
		uint16_t ENVP_DIR : 1; // bit  11
		uint16_t ENVP_INIT_VOL : 4; // bit  12 - 15
	} mSOUND4CNT_LFields_t;

	typedef union
	{
		mSOUND4CNT_LFields_t mSOUND4CNT_LFields;
		uint16_t mSOUND4CNT_LHalfWord;
	} mSOUND4CNT_LHalfWord_t;

	// SOUND4CNT_H (16-bit)
	typedef struct
	{
		uint16_t DIV_RATIO_OF_FREQ : 3; // bit  0 - 2
		uint16_t COUNTER_STEP : 1; // bit 3
		uint16_t SHIFT_CLK_FREQ : 4; // bit  4 - 7
		uint16_t NOT_USED_0 : 6; // bit  8 - 13
		uint16_t LENGTH_FLAG : 1; // bit  14
		uint16_t INITIAL : 1; // bit  15
	} mSOUND4CNT_HFields_t;

	typedef union
	{
		mSOUND4CNT_HFields_t mSOUND4CNT_HFields;
		uint16_t mSOUND4CNT_HHalfWord;
	} mSOUND4CNT_HHalfWord_t;

	// SOUNDCNT_L (16-bit)
	typedef struct
	{
		uint16_t SOUND_MASTER_VOL_R : 3; // bit  0 - 2
		uint16_t NOT_USED_0 : 1; // bit  3
		uint16_t SOUND_MASTER_VOL_L : 3; // bit  4 - 6
		uint16_t NOT_USED_1 : 1; // bit  7
		uint16_t SOUND_ENABLE_1R : 1; // bit  8
		uint16_t SOUND_ENABLE_2R : 1; // bit  9
		uint16_t SOUND_ENABLE_3R : 1; // bit  10
		uint16_t SOUND_ENABLE_4R : 1; // bit  11
		uint16_t SOUND_ENABLE_1L : 1; // bit  12
		uint16_t SOUND_ENABLE_2L : 1; // bit  13
		uint16_t SOUND_ENABLE_3L : 1; // bit  14
		uint16_t SOUND_ENABLE_4L : 1; // bit  15
	} mSOUNDCNT_LFields_t;

	typedef union
	{
		mSOUNDCNT_LFields_t mSOUNDCNT_LFields;
		uint16_t mSOUNDCNT_LHalfWord;
	} mSOUNDCNT_LHalfWord_t;

	// SOUNDCNT_H (16-bit)
	typedef struct
	{
		uint16_t SOUND_VOL : 2; // bit  0 - 1
		uint16_t DMA_SOUND_A_VOL : 1; // bit  2
		uint16_t DMA_SOUND_B_VOL : 1; // bit  3
		uint16_t NOT_USED_0 : 4; // bit  4 - 7
		uint16_t DMA_SOUND_A_EN_R : 1; // bit  8
		uint16_t DMA_SOUND_A_EN_L : 1; // bit  9
		uint16_t DMA_SOUND_A_TIMER_SEL : 1; // bit  10
		uint16_t DMA_SOUND_A_RESET_FIFO : 1; // bit  11
		uint16_t DMA_SOUND_B_EN_R : 1; // bit  12
		uint16_t DMA_SOUND_B_EN_L : 1; // bit  13
		uint16_t DMA_SOUND_B_TIMER_SEL : 1; // bit  14
		uint16_t DMA_SOUND_B_RESET_FIFO : 1; // bit  15
	} mSOUNDCNT_HFields_t;

	typedef union
	{
		mSOUNDCNT_HFields_t mSOUNDCNT_HFields;
		uint16_t mSOUNDCNT_HHalfWord;
	} mSOUNDCNT_HHalfWord_t;

	// SOUNDCNT_X (16-bit)
	typedef struct
	{
		uint16_t SOUND1_ON_FLAG : 1; // bit  0
		uint16_t SOUND2_ON_FLAG : 1; // bit  1
		uint16_t SOUND3_ON_FLAG : 1; // bit  2
		uint16_t SOUND4_ON_FLAG : 1; // bit  3
		uint16_t NOT_USED_0 : 3; // bit  4 - 6
		uint16_t PSG_FIFO_MASTER_EN : 1; // bit  7
		uint16_t NOT_USED_1 : 8; // bit  8 - 15
	} mSOUNDCNT_XFields_t;

	typedef union
	{
		mSOUNDCNT_XFields_t mSOUNDCNT_XFields;
		uint16_t mSOUNDCNT_XHalfWord;
	} mSOUNDCNT_XHalfWord_t;

	// SOUNDBIAS (16-bit)
	typedef struct
	{
		uint16_t BIAS_LVL : 10; // bit  0 - 9
		uint16_t NOT_USED_0 : 4; // bit  10 - 13
		uint16_t AMP_RES_OR_SAMPLING_CYC : 2; // bit  14 - 15
	} mSOUNDBIASFields_t;

	typedef union
	{
		mSOUNDBIASFields_t mSOUNDBIASFields;
		uint16_t mSOUNDBIASHalfWord;
	} mSOUNDBIASHalfWord_t;

	// DMAnCNT_H (16-bit)
	typedef struct
	{
		uint16_t NOT_USED_0 : 5; // bit  0 - 4
		uint16_t DEST_ADDR_CTRL : 2; // bit  5 - 6
		uint16_t SRC_ADDR_CTRL : 2; // bit  7 - 8
		uint16_t DMA_REPEAT : 1; // bit  9
		uint16_t DMA_TRANSFER_TYPE : 1; // bit  10
		uint16_t GAME_PAK_DRQ : 1; // bit  11
		uint16_t DMA_START_TIMING : 2; // bit  12 - 13
		uint16_t WORD_COUNT_END_IRQ : 1; // bit  14
		uint16_t DMA_EN : 1; // bit  15
	} mDMAnCNT_HFields_t;

	typedef union
	{
		mDMAnCNT_HFields_t mDMAnCNT_HFields;
		uint16_t mDMAnCNT_HHalfWord;
	} mDMAnCNT_HHalfWord_t;

	// TIMERnCNT_H (16-bit)
	typedef struct
	{
		uint16_t PRESCALER_SEL : 2; // bit  0 - 1
		uint16_t COUNT_UP_TIMING : 1; // bit  2
		uint16_t NOT_USED_0 : 3; // bit  3 - 5
		uint16_t TIMER_IRQ_EN : 1; // bit  6
		uint16_t TIMER_START_STOP : 1; // bit  7
		uint16_t NOT_USED_1 : 8; // bit  8 - 15
	} mTIMERnCNT_HFields_t;

	typedef union
	{
		mTIMERnCNT_HFields_t mTIMERnCNT_HFields;
		uint16_t mTIMERnCNT_HHalfWord;
	} mTIMERnCNT_HHalfWord_t;

	// SIOCNT(16-bit)
	typedef struct
	{
		uint16_t SHIFT_CLK : 1; // bit  0
		uint16_t INT_SHIFT_CLK : 1; // bit  1
		uint16_t SI_STATE : 1; // bit  2
		uint16_t SO_DURING_INACTIVITY : 1; // bit  3
		uint16_t NOT_USED_0 : 3; // bit  4 - 6
		uint16_t START_BIT : 1; // bit  7
		uint16_t NOT_USED_1 : 4; // bit  8 - 11
		uint16_t TRANSFER_LENGTH : 1; // bit  12
		uint16_t MODE_SPECIFIC_0 : 1; // bit  13
		uint16_t IRQ_EN : 1; // bit  14
		uint16_t NOT_USED_2 : 1; // bit  15
	} mSIOCNT_SPFields_t;

	typedef struct
	{
		uint16_t BAUD : 2; // bit  0 - 1
		uint16_t SI_TERMINAL : 1; // bit  2
		uint16_t SD_TERMINAL : 1; // bit  3
		uint16_t MP_ID : 2; // bit  4 - 5
		uint16_t MP_ERR : 1; // bit  6
		uint16_t START_BUSY_BIT : 1; // bit  7
		uint16_t NOT_USED_0 : 4; // bit  8 - 11
		uint16_t MODE_SPECIFIC_0 : 1; // bit  12
		uint16_t MODE_SPECIFIC_1 : 1; // bit  13
		uint16_t IRQ_EN : 1; // bit  14
		uint16_t NOT_USED_1 : 1; // bit  15
	} mSIOCNT_MPFields_t;

	typedef struct
	{
		uint16_t BAUD : 2; // bit  0 - 1
		uint16_t CTS_FLAG : 1; // bit  2
		uint16_t PARITY_CTRL : 1; // bit  3
		uint16_t TX_DATA_FLAG : 1; // bit  4
		uint16_t RX_DATA_FLAG : 1; // bit  5
		uint16_t ERR_FLAG : 1; // bit  6
		uint16_t DATA_LENGTH : 1; // bit  7
		uint16_t FIFO_EN_FLAG : 1; // bit  8
		uint16_t PARITY_EN_FLAG : 1; // bit  9
		uint16_t TX_EN_FLAG : 1; // bit  10
		uint16_t RX_EN_FLAG : 1; // bit  11
		uint16_t MODE_SPECIFIC_0 : 1; // bit  12
		uint16_t MODE_SPECIFIC_1 : 1; // bit  13
		uint16_t IRQ_EN : 1; // bit  14
		uint16_t NOT_USED_0 : 1; // bit  15
	} mSCCNTFields_t;

	typedef struct
	{
		uint16_t SC_DATA_BIT : 1; // bit  0
		uint16_t SD_DATA_BIT : 1; // bit  1
		uint16_t SI_DATA_BIT : 1; // bit  2
		uint16_t SO_DATA_BIT : 1; // bit  3
		uint16_t SC_DIR : 1; // bit  4
		uint16_t SD_DIR : 1; // bit  5
		uint16_t SI_DIR : 1; // bit  6
		uint16_t SO_DIR : 1; // bit  7
		uint16_t SI_INTERRUPT_EN : 1; // bit  8
		uint16_t NOT_USED_0 : 5; // bit  9 - 13
		uint16_t MODE_SPECIFIC_0 : 1; // bit  14
		uint16_t MODE_SPECIFIC_1 : 1; // bit  15
	} mGPCNTFields_t;

	typedef struct
	{
		uint16_t BIT_00 : 1; // bit  0
		uint16_t BIT_01 : 1; // bit  1
		uint16_t BIT_02 : 1; // bit  2
		uint16_t BIT_03 : 1; // bit  3
		uint16_t BIT_04 : 1; // bit  4
		uint16_t BIT_05 : 1; // bit  5
		uint16_t BIT_06 : 1; // bit  6
		uint16_t BIT_07 : 1; // bit  7
		uint16_t BIT_08 : 1; // bit  8
		uint16_t BIT_09 : 1; // bit  9
		uint16_t BIT_10 : 1; // bit  10
		uint16_t BIT_11 : 1; // bit  11
		uint16_t BIT_12 : 1; // bit  12
		uint16_t BIT_13 : 1; // bit  13
		uint16_t BIT_14 : 1; // bit  14
		uint16_t BIT_15 : 1; // bit  15
	} mSIO_t;

	typedef union
	{
		mSIOCNT_SPFields_t mSIOCNT_SPFields;
		mSIOCNT_MPFields_t mSIOCNT_MPFields;
		mSCCNTFields_t mSCCNTFields;
		mGPCNTFields_t mGPCNTFields;
		mSIO_t mSIOFields;
		uint16_t mSIOCNTHalfWord;
	} mSIOCNTHalfWord_t;

	// KEYINPUT (16-bit)
	typedef struct
	{
		uint16_t BUTTON_A : 1; // bit  0
		uint16_t BUTTON_B : 1; // bit  1
		uint16_t SELECT : 1; // bit  2
		uint16_t START : 1; // bit  3
		uint16_t RIGHT : 1; // bit  4
		uint16_t LEFT : 1; // bit  5
		uint16_t UP : 1; // bit  6
		uint16_t DOWN : 1; // bit  7
		uint16_t BUTTON_R : 1; // bit  8
		uint16_t BUTTON_L : 1; // bit  9
		uint16_t NOT_USED_0 : 6; // bit  10 - 15
	} mKEYINPUTFields_t;

	typedef union
	{
		mKEYINPUTFields_t mKEYINPUTFields;
		uint16_t mKEYINPUTHalfWord;
	} mKEYINPUTHalfWord_t;

	// KEYCNT (16-bit)
	typedef struct
	{
		uint16_t BUTTON_A : 1; // bit  0
		uint16_t BUTTON_B : 1; // bit  1
		uint16_t SELECT : 1; // bit  2
		uint16_t START : 1; // bit  3
		uint16_t RIGHT : 1; // bit  4
		uint16_t LEFT : 1; // bit  5
		uint16_t UP : 1; // bit  6
		uint16_t DOWN : 1; // bit  7
		uint16_t BUTTON_R : 1; // bit  8
		uint16_t BUTTON_L : 1; // bit  9
		uint16_t NOT_USED_0 : 4; // bit  10 - 13
		uint16_t BUTTON_IRQ_EN : 1; // bit  14
		uint16_t BUTTON_IRQ_CONDITION : 1; // bit  15
	} mKEYCNTFields_t;

	typedef union
	{
		mKEYCNTFields_t mKEYCNTFields;
		uint16_t mKEYCNTHalfWord;
	} mKEYCNTHalfWord_t;

	// RCNT (16-bit)
	typedef struct
	{
		uint16_t UNDOC : 4; // bit  0 - 3
		uint16_t MODE_SPECIFIC_0 : 5; // bit  4 - 8
		uint16_t MODE_SPECIFIC_1 : 5; // bit  9 - 13
		uint16_t MODE_SPECIFIC_2 : 1; // bit  14
		uint16_t MODE_SPECIFIC_3 : 1; // bit  15
	} mRCNTFields_t;

	typedef union
	{
		mRCNTFields_t mRCNTFields;
		uint16_t mRCNTHalfWord;
	} mRCNTHalfWord_t;

	// IR(16-bit)
	typedef struct
	{
		uint16_t TX_DATA : 1; // bit  0
		uint16_t READ_EN : 1; // bit  1
		uint16_t RX_DATA : 1; // bit  2
		uint16_t AMP : 1; // bit  3
		uint16_t IRQ_EN_FLAG : 1; // bit  4
		uint16_t NOT_USED_0 : 11; // bit  5 - 15
	} mIRFields_t;

	typedef union
	{
		mIRFields_t mIRFields;
		uint16_t mIRHalfWord;
	} mIRHalfWord_t;

	// JOYCNT(16-bit)
	typedef struct
	{
		uint16_t DEV_RESET : 1; // bit  0
		uint16_t RX_COMPLETE_FLAG : 1; // bit  1
		uint16_t TX_COMPLETE_FLAG : 1; // bit  2
		uint16_t NOT_USED_0 : 3; // bit  3 - 5
		uint16_t IRQ_ON_DEV_RESET : 1; // bit  6
		uint16_t NOT_USED_1 : 9; // bit  7 - 15
	} mJOYCNTFields_t;

	typedef union
	{
		mJOYCNTFields_t mJOYCNTFields;
		uint16_t mJOYCNTHalfWord;
	} mJOYCNTHalfWord_t;

	// JOYSTAT(16-bit)
	typedef struct
	{
		uint16_t NOT_USED_0 : 1; // bit  0
		uint16_t RX_STATUS_FLAG : 1; // bit  1
		uint16_t NOT_USED_1 : 1; // bit  2
		uint16_t TX_STATUS_FLAG : 1; // bit  3
		uint16_t GP_FLAG : 2; // bit  4 - 5
		uint16_t NOT_USED_2 : 10; // bit  6 - 15
	} mJOYSTATFields_t;

	typedef union
	{
		mJOYSTATFields_t mJOYSTATFields;
		uint16_t mJOYSTATHalfWord;
	} mJOYSTATHalfWord_t;

	// IE (16-bit)
	typedef struct
	{
		uint16_t LCD_VBLANK : 1; // bit  0
		uint16_t LCD_HBLANK : 1; // bit  1
		uint16_t LCD_VCOUNTER_MATCH : 1; // bit  2
		uint16_t TIMER0_OVERFLOW : 1; // bit  3
		uint16_t TIMER1_OVERFLOW : 1; // bit  4
		uint16_t TIMER2_OVERFLOW : 1; // bit  5
		uint16_t TIMER3_OVERFLOW : 1; // bit  6
		uint16_t SERIAL_COMM : 1; // bit  7
		uint16_t DMA0 : 1; // bit  8
		uint16_t DMA1 : 1; // bit  9
		uint16_t DMA2 : 1; // bit  10
		uint16_t DMA3 : 1; // bit  11
		uint16_t KEYPAD : 1; // bit  12
		uint16_t GAMEPAK : 1; // bit  13
		uint16_t NOT_USED_0 : 1; // bit  14 - 15
	} mIEFields_t;

	typedef union
	{
		mIEFields_t mIEFields;
		uint16_t mIEHalfWord;
	} mIEHalfWord_t;

	// IF (16-bit)
	typedef struct
	{
		uint16_t LCD_VBLANK : 1; // bit  0
		uint16_t LCD_HBLANK : 1; // bit  1
		uint16_t LCD_VCOUNTER_MATCH : 1; // bit  2
		uint16_t TIMER0_OVERFLOW : 1; // bit  3
		uint16_t TIMER1_OVERFLOW : 1; // bit  4
		uint16_t TIMER2_OVERFLOW : 1; // bit  5
		uint16_t TIMER3_OVERFLOW : 1; // bit  6
		uint16_t SERIAL_COMM : 1; // bit  7
		uint16_t DMA0 : 1; // bit  8
		uint16_t DMA1 : 1; // bit  9
		uint16_t DMA2 : 1; // bit  10
		uint16_t DMA3 : 1; // bit  11
		uint16_t KEYPAD : 1; // bit  12
		uint16_t GAMEPAK : 1; // bit  13
		uint16_t NOT_USED_0 : 1; // bit  14 - 15
	} mIFFields_t;

	typedef union
	{
		mIFFields_t mIFFields;
		uint16_t mIFHalfWord;
	} mIFHalfWord_t;

	// WAITCNT (16-bit)
	typedef struct
	{
		uint16_t SRAM_WAIT_CTRL : 2; // bit  0 - 1
		uint16_t WAIT_STATE_0_NON_SEQ : 2; // bit  2 - 3
		uint16_t WAIT_STATE_0_SEQ : 1; // bit  4
		uint16_t WAIT_STATE_1_NON_SEQ : 2; // bit  5 - 6
		uint16_t WAIT_STATE_1_SEQ : 1; // bit  7
		uint16_t WAIT_STATE_2_NON_SEQ : 2; // bit  8 - 9
		uint16_t WAIT_STATE_2_SEQ : 1; // bit  10
		uint16_t PHI_TERMINAL_OUTPUT : 1; // bit  11 - 12
		uint16_t NOT_USED_0 : 1; // bit  13
		uint16_t GAMEPAK_PREFETCH_BUFF : 1; // bit  14
		uint16_t GAMEPAK_TYPE_FLAG : 1; // bit  15
	} mWAITCNTFields_t;

	typedef union
	{
		mWAITCNTFields_t mWAITCNTFields;
		uint16_t mWAITCNTHalfWord;
	} mWAITCNTHalfWord_t;

	// IME (16-bit)
	typedef struct
	{
		uint16_t ENABLE_ALL_INTERRUPTS : 1; // bit  0
		uint16_t NOT_USED_0 : 15; // bit  1 - 15
	} mIMEFields_t;

	typedef union
	{
		mIMEFields_t mIMEFields;
		uint16_t mIMEHalfWord;
	} mIMEHalfWord_t;

	// POSTFLG (8-bit)
	typedef struct
	{
		BYTE UNDOC : 1; // bit  0
		BYTE NOT_USED_0 : 7; // bit  1 - 7
	} mPOSTFLGFields_t;

	typedef union
	{
		mPOSTFLGFields_t mPOSTFLGFields;
		BYTE mPOSTFLGByte;
	} mPOSTFLGByte_t;

	// HALTCNT (8-bit)
	typedef struct
	{
		BYTE UNDOC : 7; // bit  0 - 6
		BYTE STOP : 1; // bit  7
	} mHALTCNTFields_t;

	typedef union
	{
		mHALTCNTFields_t mHALTCNTFields;
		BYTE mHALTCNTByte;
	} mHALTCNTByte_t;

	typedef struct
	{
		mPOSTFLGByte_t mPOSTFLGByte;
		mHALTCNTByte_t mHALTCNTByte;
	} mPOSTFLG_HALTCNT_Fields_t;

	typedef union
	{
		mPOSTFLG_HALTCNT_Fields_t mPOSTFLG_HALTCNT_Fields;
		uint16_t mPOSTFLG_HALTCNT_HalfWord;
	} mPOSTFLG_HALTCNT_HalfWord_t;

	// INTMEMCTRL (32-bit)
	typedef struct
	{
		uint32_t DIS_WRAMS : 1; // bit  0
		uint32_t UNKNOWN_0 : 1; // bit  1
		uint32_t UNKNOWN_1 : 1; // bit  2
		uint32_t UNKNOWN_2 : 1; // bit  3
		uint32_t UNUSED_0 : 1; // bit  4
		uint32_t EN_256KWRAM : 1; // bit  5
		uint32_t UNUSED_1 : 18; // bit  6 - 23
		uint32_t WAIT_CTRL_256KWRAM : 4; // bit  24 - 27
		uint32_t UNKNOWN_3 : 4; // bit  28 - 31
	} mINTMEMCTRLFields_t;

	typedef union
	{
		mINTMEMCTRLFields_t mINTMEMCTRLFields;
		uint32_t mINTMEMCTRLWord;
	} mINTMEMCTRLWord_t;

	// audio
	typedef struct
	{
		BYTE lowerNibble : 4; // bits 0 - 3
		BYTE upperNibble : 4; // bits 4 - 7
	} samples_t;

	typedef union
	{
		samples_t samples;
		BYTE waveRamByte;
	} waveRamByte_t;

	typedef union
	{
		samples_t samples[TWO];
		uint16_t waveRamHalfWord;
	} waveRamHalfWord_t;

	struct FIFO_t
	{
		SBYTE fifo[THIRTYTWO];
		COUNTER8 position;
		DIM8 size;
		ID timer;
		GBA_AUDIO_SAMPLE_TYPE latch;

		void fifoByteWrite(uint8_t value) 
		{
			if (size < THIRTYTWO)
			{
				COUNTER8 index = (position + size) & THIRTYONE;
				fifo[index] = (int8_t)value;
				size++;
			}
		}
	};

	// Size = 770 bytes
	typedef struct
	{
		mDISPCNTHalfWord_t mDISPCNTHalfWord;					// 0x04000000
		mGREENSWAPHalfWord_t mGREENSWAPHalfWord;				// 0x04000002
		mDISPSTATHalfWord_t mDISPSTATHalfWord;					// 0x04000004
		mVCOUNTHalfWord_t mVCOUNTHalfWord;						// 0x04000006
		mBGnCNTHalfWord_t mBG0CNTHalfWord;						// 0x04000008
		mBGnCNTHalfWord_t mBG1CNTHalfWord;						// 0x0400000A
		mBGnCNTHalfWord_t mBG2CNTHalfWord;						// 0x0400000C
		mBGnCNTHalfWord_t mBG3CNTHalfWord;						// 0x0400000E
		mBGniOFSHalfWord_t mBG0HOFSHalfWord;					// 0x04000010
		mBGniOFSHalfWord_t mBG0VOFSHalfWord;					// 0x04000012
		mBGniOFSHalfWord_t mBG1HOFSHalfWord;					// 0x04000014
		mBGniOFSHalfWord_t mBG1VOFSHalfWord;					// 0x04000016
		mBGniOFSHalfWord_t mBG2HOFSHalfWord;					// 0x04000018
		mBGniOFSHalfWord_t mBG2VOFSHalfWord;					// 0x0400001A
		mBGniOFSHalfWord_t mBG3HOFSHalfWord;					// 0x0400001C
		mBGniOFSHalfWord_t mBG3VOFSHalfWord;					// 0x0400001E
		mBGnPxHalfWord_t mBG2PAHalfWord;						// 0x04000020
		mBGnPxHalfWord_t mBG2PBHalfWord;						// 0x04000022
		mBGnPxHalfWord_t mBG2PCHalfWord;						// 0x04000024
		mBGnPxHalfWord_t mBG2PDHalfWord;						// 0x04000026
		mBGniWord_t mBG2XWord;									// 0x04000028 - 0x0400002B
		mBGniWord_t mBG2YWord;									// 0x0400002C - 0x0400002F
		mBGnPxHalfWord_t mBG3PAHalfWord;						// 0x04000030
		mBGnPxHalfWord_t mBG3PBHalfWord;						// 0x04000032
		mBGnPxHalfWord_t mBG3PCHalfWord;						// 0x04000034
		mBGnPxHalfWord_t mBG3PDHalfWord;						// 0x04000036
		mBGniWord_t mBG3XWord;									// 0x04000038 - 0x0400003B
		mBGniWord_t mBG3YWord;									// 0x0400003C - 0x0400003F
		mWINniHalfWord_t mWIN0HHalfWord;						// 0x04000040
		mWINniHalfWord_t mWIN1HHalfWord;						// 0x04000042
		mWINniHalfWord_t mWIN0VHalfWord;						// 0x04000044
		mWINniHalfWord_t mWIN1VHalfWord;						// 0x04000046
		mWININHalfWord_t mWININHalfWord;						// 0x04000048
		mWINOUTHalfWord_t mWINOUTHalfWord;						// 0x0400004A
		mMOSAICHalfWord_t mMOSAICHalfWord;						// 0x0400004C
		GBA_HALFWORD m400004E;									// 0x0400004E
		mBLDCNTHalfWord_t mBLDCNTHalfWord;						// 0x04000050
		mBLDALPHAHalfWord_t mBLDALPHAHalfWord;					// 0x04000052
		mBLDYHalfWord_t mBLDYHalfWord;							// 0x04000054
		GBA_HALFWORD m4000056;									// 0x04000056
		GBA_HALFWORD m4000058;									// 0x04000058
		GBA_HALFWORD m400005A;									// 0x0400005A
		GBA_HALFWORD m400005C;									// 0x0400005C
		GBA_HALFWORD m400005E;									// 0x0400005E
		mSOUND1CNT_LHalfWord_t mSOUND1CNT_LHalfWord;			// 0x04000060
		mSOUND1CNT_HHalfWord_t mSOUND1CNT_HHalfWord;			// 0x04000062
		mSOUND1CNT_XHalfWord_t mSOUND1CNT_XHalfWord;			// 0x04000064
		GBA_HALFWORD m4000066;									// 0x04000066
		mSOUND2CNT_LHalfWord_t mSOUND2CNT_LHalfWord;			// 0x04000068
		GBA_HALFWORD m400006A;									// 0x0400006A
		mSOUND2CNT_HHalfWord_t mSOUND2CNT_HHalfWord;			// 0x0400006C
		GBA_HALFWORD m400006E;									// 0x0400006E
		mSOUND3CNT_LHalfWord_t mSOUND3CNT_LHalfWord;			// 0x04000070
		mSOUND3CNT_HHalfWord_t mSOUND3CNT_HHalfWord;			// 0x04000072
		mSOUND3CNT_XHalfWord_t mSOUND3CNT_XHalfWord;			// 0x04000074
		GBA_HALFWORD m4000076;									// 0x04000076
		mSOUND4CNT_LHalfWord_t mSOUND4CNT_LHalfWord;			// 0x04000078
		GBA_HALFWORD m400007A;									// 0x0400007A
		mSOUND4CNT_HHalfWord_t mSOUND4CNT_HHalfWord;			// 0x0400007C
		GBA_HALFWORD m400007E;									// 0x0400007E
		mSOUNDCNT_LHalfWord_t mSOUNDCNT_LHalfWord;				// 0x04000080
		mSOUNDCNT_HHalfWord_t mSOUNDCNT_HHalfWord;				// 0x04000082
		mSOUNDCNT_XHalfWord_t mSOUNDCNT_XHalfWord;				// 0x04000084
		GBA_HALFWORD m4000086;									// 0x04000086
		mSOUNDBIASHalfWord_t mSOUNDBIASHalfWord;				// 0x04000088
		GBA_HALFWORD m400008A;									// 0x0400008A
		GBA_HALFWORD m400008C;									// 0x0400008C
		GBA_HALFWORD m400008E;									// 0x0400008E
		union
		{
			waveRamByte_t mWAVERAM8[EIGHT * TWO];				// 0x04000090 - 0x0400009F
			waveRamHalfWord_t mWAVERAM16[EIGHT];				// 0x04000090 - 0x0400009F
		};
		GBA_HALFWORD mFIFOA_L;									// 0x040000A0
		GBA_HALFWORD mFIFOA_H;									// 0x040000A2
		GBA_HALFWORD mFIFOB_L;									// 0x040000A4
		GBA_HALFWORD mFIFOB_H;									// 0x040000A6
		GBA_HALFWORD m40000A8;									// 0x040000A8
		GBA_HALFWORD m40000AC;									// 0x040000AC
		GBA_HALFWORD m40000AE;									// 0x040000AE
		GBA_HALFWORD mDMA0SAD_L;								// 0x040000B0
		GBA_HALFWORD mDMA0SAD_H;								// 0x040000B2
		GBA_HALFWORD mDMA0DAD_L;								// 0x040000B4
		GBA_HALFWORD mDMA0DAD_H;								// 0x040000B6
		GBA_HALFWORD mDMA0CNT_L;								// 0x040000B8
		mDMAnCNT_HHalfWord_t mDMA0CNT_H;						// 0x040000BA
		GBA_HALFWORD mDMA1SAD_L;								// 0x040000BC
		GBA_HALFWORD mDMA1SAD_H;								// 0x040000BE
		GBA_HALFWORD mDMA1DAD_L;								// 0x040000C0
		GBA_HALFWORD mDMA1DAD_H;								// 0x040000C2
		GBA_HALFWORD mDMA1CNT_L;								// 0x040000C4
		mDMAnCNT_HHalfWord_t mDMA1CNT_H;						// 0x040000C6
		GBA_HALFWORD mDMA2SAD_L;								// 0x040000C8
		GBA_HALFWORD mDMA2SAD_H;								// 0x040000CA
		GBA_HALFWORD mDMA2DAD_L;								// 0x040000CC
		GBA_HALFWORD mDMA2DAD_H;								// 0x040000CE
		GBA_HALFWORD mDMA2CNT_L;								// 0x040000D0
		mDMAnCNT_HHalfWord_t mDMA2CNT_H;						// 0x040000D2
		GBA_HALFWORD mDMA3SAD_L;								// 0x040000D4
		GBA_HALFWORD mDMA3SAD_H;								// 0x040000D6
		GBA_HALFWORD mDMA3DAD_L;								// 0x040000D8
		GBA_HALFWORD mDMA3DAD_H;								// 0x040000DA
		GBA_HALFWORD mDMA3CNT_L;								// 0x040000DC
		mDMAnCNT_HHalfWord_t mDMA3CNT_H;						// 0x040000DE
		GBA_HALFWORD m40000E0;									// 0x040000E0
		GBA_HALFWORD m40000E2;									// 0x040000E2
		GBA_HALFWORD m40000E4;									// 0x040000E4
		GBA_HALFWORD m40000E6;									// 0x040000E6
		GBA_HALFWORD m40000E8;									// 0x040000E8
		GBA_HALFWORD m40000EA;									// 0x040000EA
		GBA_HALFWORD m40000EC;									// 0x040000EC
		GBA_HALFWORD m40000EE;									// 0x040000EE
		GBA_HALFWORD m40000F0;									// 0x040000F0
		GBA_HALFWORD m40000F2;									// 0x040000F2
		GBA_HALFWORD m40000F4;									// 0x040000F4
		GBA_HALFWORD m40000F6;									// 0x040000F6
		GBA_HALFWORD m40000F8;									// 0x040000F8
		GBA_HALFWORD m40000FA;									// 0x040000FA
		GBA_HALFWORD m40000FC;									// 0x040000FC
		GBA_HALFWORD m40000FE;									// 0x040000FE
		GBA_HALFWORD mTIMER0CNT_L;								// 0x04000100
		mTIMERnCNT_HHalfWord_t mTIMER0CNT_H;					// 0x04000102
		GBA_HALFWORD mTIMER1CNT_L;								// 0x04000104
		mTIMERnCNT_HHalfWord_t mTIMER1CNT_H;					// 0x04000106
		GBA_HALFWORD mTIMER2CNT_L;								// 0x04000108
		mTIMERnCNT_HHalfWord_t mTIMER2CNT_H;					// 0x0400010A
		GBA_HALFWORD mTIMER3CNT_L;								// 0x0400010C
		mTIMERnCNT_HHalfWord_t mTIMER3CNT_H;					// 0x0400010E
		GBA_HALFWORD m4000110;									// 0x04000110
		GBA_HALFWORD m4000112;									// 0x04000112
		GBA_HALFWORD m4000114;									// 0x04000114
		GBA_HALFWORD m4000116;									// 0x04000116
		GBA_HALFWORD m4000118;									// 0x04000118
		GBA_HALFWORD m400011A;									// 0x0400011A
		GBA_HALFWORD m400011C;									// 0x0400011C
		GBA_HALFWORD m400011E;									// 0x0400011E
		GBA_HALFWORD mSIOMULTI0;								// 0x04000120
		GBA_HALFWORD mSIOMULTI1;								// 0x04000122
		GBA_HALFWORD mSIOMULTI2;								// 0x04000124
		GBA_HALFWORD mSIOMULTI3;								// 0x04000126
		mSIOCNTHalfWord_t mSIOCNT;								// 0x04000128
		GBA_HALFWORD mSIO_DATA8_MLTSEND;						// 0x0400012A
		GBA_HALFWORD m400012C;									// 0x0400012C
		GBA_HALFWORD m400012E;									// 0x0400012E
		mKEYINPUTHalfWord_t mKEYINPUTHalfWord;					// 0x04000130
		mKEYCNTHalfWord_t mKEYCNTHalfWord;						// 0x04000132
		mRCNTHalfWord_t mRCNTHalfWord;							// 0x04000134
		mIRHalfWord_t mIRHalfWord;								// 0x04000136
		GBA_HALFWORD m4000138;									// 0x04000138
		GBA_HALFWORD m400013A;									// 0x0400013A
		GBA_HALFWORD m400013C;									// 0x0400013C
		GBA_HALFWORD m400013E;									// 0x0400013E
		mJOYCNTHalfWord_t mJOYCNTHalfWord;						// 0x04000140
		GBA_HALFWORD m4000142;									// 0x04000142
		GBA_HALFWORD m4000144;									// 0x04000144
		GBA_HALFWORD m4000146;									// 0x04000146
		GBA_HALFWORD m4000148;									// 0x04000148
		GBA_HALFWORD m400014A;									// 0x0400014A
		GBA_HALFWORD m400014C;									// 0x0400014C
		GBA_HALFWORD m400014E;									// 0x0400014E
		GBA_HALFWORD mJOY_RECV_L;								// 0x04000150
		GBA_HALFWORD mJOY_RECV_H;								// 0x04000152
		GBA_HALFWORD mJOY_TRANS_L;								// 0x04000154
		GBA_HALFWORD mJOY_TRANS_H;								// 0x04000156
		mJOYSTATHalfWord_t mJOYSTATHalfWord;					// 0x04000158
		GBA_HALFWORD m400015A;									// 0x0400015A
		GBA_HALFWORD mUNDOC01[(0x04000200 - 0x0400015C) / TWO];	// 0x0400015C - 0x040001FE
		mIEHalfWord_t mIEHalfWord;								// 0x04000200
		mIFHalfWord_t mIFHalfWord;								// 0x04000202
		mWAITCNTHalfWord_t mWAITCNTHalfWord;					// 0x04000204
		GBA_HALFWORD m4000206;									// 0x04000206
		mIMEHalfWord_t mIMEHalfWord;							// 0x04000208
		GBA_HALFWORD m400020A;									// 0x0400020A
		GBA_HALFWORD mUNDOC02[(0x04000300 - 0x0400020C) / TWO];	// 0x0400020C - 0x040002FE
		mPOSTFLG_HALTCNT_HalfWord_t mPOSTFLG_HALTCNT_HalfWord;	// 0x04000300
		GBA_HALFWORD m4000302;									// 0x04000302
	} mIOFields_t;

	typedef union
	{
		mIOFields_t mIOFields;
		BYTE mIOMemory8bit[sizeof(mIOFields_t)];
		uint16_t mIOMemory16bit[sizeof(mIOFields_t) / TWO];
		uint32_t mIOMemory32bit[sizeof(mIOFields_t) / FOUR];
	} mIO_t;

	// Size = 0x400 
	typedef struct
	{
		BYTE mBGPaletteRam[0x200];
		BYTE mOBJPaletteRam[0x200];
	} mBgObjPaletteRamFields_t;

	typedef union
	{
		mBgObjPaletteRamFields_t mBgObjPaletteRamFields;
		BYTE mBgObjPaletteRamMemory8bit[sizeof(mBgObjPaletteRamFields_t)];
		uint16_t mBgObjPaletteRamMemory16bit[sizeof(mBgObjPaletteRamFields_t) / TWO];
		uint32_t mBgObjPaletteRamMemory32bit[sizeof(mBgObjPaletteRamFields_t) / FOUR];
	} mBgObjPaletteMemory_t;

	// Size = 0x18000
	typedef struct
	{
		BYTE mShared_BGTILES_BGMAP[0x10000];
		BYTE mOBJTILES[0x8000];
	} mVideoRamFields_TILE_MAP_t;

	typedef struct
	{
		BYTE mFrame0Buffer[0x14000];
		BYTE mObJTILES[0x4000];
	} mVideoRamFields_BIT_MAP_STILLIMAGE_t;

	typedef struct
	{
		BYTE mFrame0Buffer[0xA000];
		BYTE mFrame1Buffer[0xA000];
		BYTE mObJTILES[0x4000];
	} mVideoRamFields_BIT_MAP_t;

	typedef union
	{
		mVideoRamFields_TILE_MAP_t TILE_MAP;
		mVideoRamFields_BIT_MAP_STILLIMAGE_t BIT_MAP_STILLIMAGE;
		mVideoRamFields_BIT_MAP_t BIT_MAP;
		BYTE mVideoRam8bit[0x18000];
		uint16_t mVideoRam16bit[0x18000 / TWO];
		uint32_t mVideoRam32bit[0x18000 / FOUR];
	} mVideoRam_t;

	// Size = 0x400 

#if (DEACTIVATED)
	typedef union objextAttribute0
	{
		struct
		{
			GBA_HALFWORD y : 8;
			GBA_HALFWORD affine_object_mode : 2;
			GBA_HALFWORD graphics_mode : 2;
			GBA_HALFWORD mosaic : 1;
			GBA_HALFWORD is_256color : 1;
			GBA_HALFWORD shape : 2;
		};
		GBA_HALFWORD raw;
	} objextAttribute0_t;

	typedef union objextAttribute1
	{
		struct
		{
			GBA_HALFWORD x : 9;
			GBA_HALFWORD affine_index : 5;
			GBA_HALFWORD size : 2;
		};
		struct
		{
			GBA_HALFWORD : 12;
			GBA_HALFWORD hflip : 1;
			GBA_HALFWORD vflip : 1;
			GBA_HALFWORD : 2;
		};
		GBA_HALFWORD raw;
	} objextAttribute1_t;

	typedef union objextAttribute2
	{
		struct
		{
			GBA_HALFWORD tid : 10;
			GBA_HALFWORD priority : 2;
			GBA_HALFWORD pb : 4;
		};
		GBA_HALFWORD raw;
	} objextAttribute2_t;

	typedef union
	{
		struct
		{
			objextAttribute0_t objextAttribute0;
			objextAttribute1_t objextAttribute1;
			objextAttribute2_t objextAttribute2;
			BYTE objectAttribute3;
		};
		GBA_WORD raw;
	} objextAttribute_t;
#endif

	typedef struct
	{
		uint16_t Y_COORDINATE : 8; // bit  0 - 7
		uint16_t ROTATE_SCALE_FLAG : 1; // bit  8
		uint16_t OBJ_DISABLE_OR_DOUBLE_SIZE_FLAG : 1; // bit  9
		uint16_t OBJ_MODE : 2; // bit 10 - 11	
		uint16_t OBJ_MOSAIC : 1; // bit  12
		uint16_t COLOR_PALETTES : 1; // bit  13
		uint16_t OBJ_SHAPE : 2; // bit 14 - 15
	} mOamAttr0Fields_t;

	typedef union
	{
		mOamAttr0Fields_t mOamAttr0Fields;
		uint16_t mOamAttr0HalfWord;
	} mOamAttr0HalfWord_t;

	typedef struct
	{
		uint16_t X_COORDINATE : 9; // bit  0 - 8
		uint16_t ROT_SCALE_PARAM_SEL : 5; // bit  9 - 13
		uint16_t OBJ_SIZE : 2; // bit 14 - 15
	} mOamAttr1Fields_ROT_SCALE_EN_t;

	typedef struct
	{
		uint16_t X_COORDINATE : 9; // bit  0 - 8
		uint16_t NOT_USED_0 : 3; // bit  9 - 11
		uint16_t HOR_FLIP : 1; // bit  12
		uint16_t VER_FLIP : 1; // bit  13
		uint16_t OBJ_SIZE : 2; // bit 14 - 15
	} mOamAttr1Fields_ROT_SCALE_DIS_t;

	typedef union
	{
		mOamAttr1Fields_ROT_SCALE_EN_t ROT_SCALE_EN;
		mOamAttr1Fields_ROT_SCALE_DIS_t ROT_SCALE_DIS;
		uint16_t mOamAttr1HalfWord;
	} mOamAttr1HalfWord_t;

	typedef struct
	{
		uint16_t CHARACTER_NAME : 10; // bit  0 - 9
		uint16_t PRIORITY : 2; // bit  9 - 10 (NOTE: This is not just priority w.r.t BG; Refer https://gbadev.net/gbadoc/sprites.html and https://gbadev.net/tonc/regobj.html)
		uint16_t PALETTE_NUMBER : 4; // bit  12 - 15
	} mOamAttr2Fields_t;

	typedef union
	{
		mOamAttr2Fields_t mOamAttr2Fields;
		uint16_t mOamAttr2HalfWord;
	} mOamAttr2HalfWord_t;

	typedef union
	{
		struct
		{
			mOamAttr0HalfWord_t mOamAttr0HalfWord;
			mOamAttr1HalfWord_t mOamAttr1HalfWord;
		};
		uint32_t raw;
	} mOamAttr01Word_t;

	typedef union
	{
		struct
		{
			mOamAttr2HalfWord_t mOamAttr2HalfWord;
			uint16_t mObjRotation;
		};
		uint32_t raw;
	} mOamAttr23Word_t;

	typedef struct
	{
		mOamAttr01Word_t mOamAttr01Word;
		mOamAttr23Word_t mOamAttr23Word;
	} mOamAttribute_t;

	typedef union
	{
		mOamAttribute_t mOamAttribute[0x80];
		BYTE mOamAttributesMemory8bit[sizeof(mOamAttribute_t) * 0x80];
		uint16_t mOamAttributesMemory16bit[sizeof(mOamAttribute_t) * 0x80 / TWO];
		uint32_t mOamAttributesMemory32bit[sizeof(mOamAttribute_t) * 0x80 / FOUR];
	} mOamAttributes_t;

	typedef struct
	{
		cartridge_header_SB_t cartridge_header_SB;
		BYTE mWaitState0[0x1FFFF40];
	} mWaitState0Fields_t;

	typedef union
	{
		mWaitState0Fields_t mWaitState0Fields;
		BYTE mWaitState0Memory8bit[sizeof(mWaitState0Fields_t)];
		uint16_t mWaitState0Memory16bit[sizeof(mWaitState0Fields_t) / TWO];
		uint32_t mWaitState0Memory32bit[sizeof(mWaitState0Fields_t) / FOUR];
	} mWaitState0_t;

	typedef struct
	{
		mWaitState0_t mWaitState0;
		mWaitState0_t mWaitState1;
		mWaitState0_t mWaitState2;
	} mWaitState_t;

	typedef union
	{
		mWaitState_t mWaitState;
		BYTE mFlashRom8bit[0x6000000];
		GBA_HALFWORD mFlashRom16bit[0x6000000 / TWO];
		GBA_WORD mFlashRom32bit[0x6000000 / FOUR];
	} mGamePakRom_t;

	typedef union
	{
		BYTE mExtSram8bit[0x10000];
	} mGamePakSram_t;

	typedef union
	{
		BYTE mExtFlash8bit[TO_UINT8(BACKUP_FLASH_MEMORY_BANK::TOTAL)][0x10000];
	} mGamePakFlash_t;

	typedef union
	{
		mGamePakSram_t mGamePakSram;
		mGamePakFlash_t mGamePakFlash;
	} mGamePakBackup_t;

	typedef struct
	{
		mSystemRom_t mSystemRom;						// 0x00000000 - 0x00003FFF
		// 0x00004000 - 0x01FFFFFF						Not used
		mExtWorkRam_t mExtWorkRam;						// 0x02000000 - 0x0203FFFF
		// 0x02040000 - 0x02FFFFFF						Not used
		mIntWorkRam_t mIntWorkRam;						// 0x03000000 - 0x03007FFF
		// 0x03008000 - 0x03FFFFFF						Not used
		mIO_t mIO;										// 0x04000000 - 0x04000804
		// 0x04000400 - 0x04FFFFFF						Not used
		mBgObjPaletteMemory_t mBgObjPaletteRam;			// 0x05000000 - 0x050003FF
		// 0x05000400 - 0x05FFFFFF						Not used
		mVideoRam_t mVideoRam;							// 0x06000000 - 0x06017FFF
		// 0x06018000 - 0x06FFFFFF						Not used
		mOamAttributes_t mOamAttributes;				// 0x07000000 - 0x070003FF
		// 0x07000400 - 0x7FFFFFFF						Not used
		mGamePakRom_t mGamePakRom;						// 0x08000000 - 0x0DFFFFFF
		mGamePakBackup_t mGamePakBackup;				// 0x0E000000 - 0x0E00FFFF (x n banks)
	} mGBAMemoryMap_t;

	typedef struct
	{
		union
		{
			mGBAMemoryMap_t mGBAMemoryMap;
			BYTE mGBARawMemory[sizeof(mGBAMemoryMap_t)];
		};
		union
		{
			waveRamByte_t mWAVERAM8[EIGHT * TWO];				// 0x04000090 - 0x0400009F
			waveRamHalfWord_t mWAVERAM16[EIGHT];				// 0x04000090 - 0x0400009F
		}mBankedWAVERAM;
		GBA_WORD previouslyAccessedMemory;
		GBA_WORD previouslyLatchedBiosData;
		MEMORY_ACCESS_TYPE getPreviousMemoryAccessType;
		MEMORY_ACCESS_TYPE setNextMemoryAccessType;
	} gbaMemory_t;

private:

	typedef struct
	{
		int64_t cpuCounter;
		int64_t dmaCounter;
		int64_t freeBusCyclesCounter;
		int64_t globalTimerCounter;
		int64_t timerCounter[TIMER::TOTAL_TIMER];
		int64_t apuCounter;
		int64_t apuFrameCounter;
		int64_t ppuCounter;
		int64_t lcdCounter;
		int64_t sioCounter;
	} cycle_accurate_t;

	typedef struct
	{
		cycle_accurate_t cycle_accurate;
	} ticks_t;

	typedef struct
	{
		BACKUP_TYPE backupType;
		struct flash_t
		{
			BACKUP_FLASH_FSM flashFsmState;
			BACKUP_FLASH_CMDS previousFlashCommand;
			BACKUP_FLASH_CMDS currentFlashCommand;
			BACKUP_FLASH_MEMORY_BANK currentMemoryBank;
			SSTATE32 erase4kbPageNumber;
			GBA_WORD erase4kbStartAddr;
			uint8_t ogByteAtFlashAccessMem0;
			uint8_t ogByteAtFlashAccessMem1;
			FLAG allowFlashWrite;
			FLAG isErased[TWO][0x10000];
			FLAG chooseMemoryBank;
			uint32_t placeholder[0x10];
		} flash;
	} backup_t;

	typedef union
	{
		struct loggerFields_t
		{
			MAP64 INTF_LOG_VERBOSITY : ONE; 				//	bit		ZERO
			MAP64 INTF_LOG_VERBOSITY_CPUWARN : ONE; 		//	bit		ONE	
			MAP64 INTF_LOG_VERBOSITY_APUWARN : ONE; 		//	bit		TWO	
			MAP64 INTF_LOG_VERBOSITY_PPUWARN : ONE; 		//	bit		THREE	
			MAP64 INTF_LOG_VERBOSITY_CPUTODO : ONE; 		//	bit		FOUR	
			MAP64 INTF_LOG_VERBOSITY_APUTODO : ONE; 		//	bit		FIVE	
			MAP64 INTF_LOG_VERBOSITY_PPUTODO : ONE; 		//	bit		SIX
			MAP64 INTF_LOG_VERBOSITY_CPUINFO : ONE; 		//	bit		SEVEN	
			MAP64 INTF_LOG_VERBOSITY_APUINFO : ONE; 		//	bit		EIGHT	
			MAP64 INTF_LOG_VERBOSITY_PPUINFO : ONE; 		//	bit		NINE
			MAP64 INTF_LOG_VERBOSITY_CPUEVENT : ONE; 		//	bit		TEN	
			MAP64 INTF_LOG_VERBOSITY_APUEVENT : ONE; 		//	bit		ELEVEN	
			MAP64 INTF_LOG_VERBOSITY_PPUEVENT : ONE; 		//	bit		TWELVE
			MAP64 INTF_LOG_VERBOSITY_CPUMOREINFO : ONE; 	//	bit		THIRTEEN	
			MAP64 INTF_LOG_VERBOSITY_APUMOREINFO : ONE; 	//	bit		FOURTEEN
			MAP64 INTF_LOG_VERBOSITY_PPUMOREINFO : ONE; 	//	bit		FIFTEEN
			MAP64 INTF_LOG_VERBOSITY_DISASSEMBLY : ONE; 	//	bit		SIXTEEN
			MAP64 INTF_LOG_VERBOSITY_CPUINFRA : ONE; 		//	bit		SEVENTEEN
			MAP64 INTF_LOG_VERBOSITY_APUINFRA : ONE; 		//	bit		EIGHTEEN
			MAP64 INTF_LOG_VERBOSITY_PPUINFRA : ONE; 		//	bit		NINETEEN
			MAP64 INTF_LOG_VERBOSITY_CPUDEBUG : ONE; 		//	bit		TWENTY
			MAP64 INTF_LOG_VERBOSITY_APUDEBUG : ONE; 		//	bit		TWENTYONE
			MAP64 INTF_LOG_VERBOSITY_PPUDEBUG : ONE; 		//	bit		TWENTYTWO
			MAP64 INTF_UNUSED : THIRTYFOUR;					//  bits	TWENTYTHREE to FIFTYSIX	
			MAP64 INTF_LOG_VERBOSITY_WARN : ONE; 			//	bit		FIFTYSEVEN
			MAP64 INTF_LOG_VERBOSITY_TODO : ONE; 			//	bit		FIFTYEIGHT
			MAP64 INTF_LOG_VERBOSITY_INFO : ONE; 			//	bit		FIFTYNINE
			MAP64 INTF_LOG_VERBOSITY_EVENT : ONE; 			//	bit		SIXTY
			MAP64 INTF_LOG_VERBOSITY_MOREINFO : ONE; 		//	bit		SIXTYONE	
			MAP64 INTF_LOG_VERBOSITY_INFRA : ONE; 			//	bit		SIXTYTWO	
			MAP64 INTF_LOG_VERBOSITY_DEBUG : ONE; 			//	bit		SIXTYTHREE		
		} loggerFields;
		MAP64 logger;
	} loggerInterface_t;

	typedef struct
	{
		MAP64 callTrace;
		GBA_HALFWORD agbReturn;
		loggerInterface_t loggerInterface;
	} debugger_t;

	typedef struct
	{
		FLAG isCycleAccurate;
		FLAG isBiosExecutionDone;
		uint32_t checksum;
		uint64_t unusableMemoryReads;
		uint64_t unusableMemoryWrites;
		ticks_t ticks;
		backup_t backup;
		debugger_t debugger;
	} emulatorStatus_t;

private:

	const std::pair<std::string_view, BACKUP_TYPE> signatures[SIX]
	{
	  { "EEPROM_V",   BACKUP_TYPE::EEPROM },
	  { "SRAM_V",     BACKUP_TYPE::SRAM },
	  { "SRAM_F_V",   BACKUP_TYPE::SRAM },
	  { "FLASH_V",    BACKUP_TYPE::FLASH64K },
	  { "FLASH512_V", BACKUP_TYPE::FLASH64K },
	  { "FLASH1M_V",  BACKUP_TYPE::FLASH128K }
	};

private:

	const SBYTE sourceModifierLUT[TO_UINT(DMA_SIZE::TOTAL)][TO_UINT(DMA_CONTROL::TOTAL)] =
	{
	  { TWO, -TWO, ZERO, ZERO },
	  { FOUR, -FOUR, ZERO, ZERO }
	};

	const SBYTE destinationModifierLUT[TO_UINT(DMA_SIZE::TOTAL)][TO_UINT(DMA_CONTROL::TOTAL)] =
	{
	  { TWO, -TWO, ZERO, TWO },
	  { FOUR, -FOUR, ZERO, FOUR }
	};

	typedef struct
	{
		INC8 startupDelay;
		DMA_TIMING scheduleType;
		FLAG currentState;
		FLAG doingChunkTRX;
		GBA_WORD source;
		GBA_WORD destination;
		GBA_WORD length;
		GBA_WORD count;
		GBA_WORD target;
		FLAG isFIFODMA;
		DMA_SIZE chunkSize;
		GBA_WORD latchedData;	// Needed when DMA reads from unused memory
		GBA_WORD wordToBeTransfered;	// Actual valid data that get transfered during valid transfers
		uint16_t io_dmaen;
		FLAG didAccessRom;  // <-- Add this to preserve ROM access state
	} dmaCache_t;

	typedef struct
	{
		MAP8 dmaIdMap;
		MAP8 placeholder;
	} dmaTrigCache_t;

	typedef struct
	{
		DMA currentlyActiveDMA;
		FLAG shouldReenterTransferLoop;
		MAP8 runnableSet;
		dmaCache_t cache[DMA::TOTAL_DMA];
		dmaTrigCache_t trigCache[DMA_TIMING::TOTAL_DMA_TIMING];
	} dma_t;

private:

	const uint16_t timerFrequency[TIMER::TOTAL_TIMER] = { 1, 64, 256, 1024 };

	typedef struct
	{
		uint16_t reload;
		uint16_t counter;
		uint16_t io_tmxcnt_l;
		uint16_t io_tmxcnt_h;
	} timerCache_t;

	typedef struct
	{
		FLAG currentState;
		INC64 startupDelay;
		FLAG overflow;
		INC32 cascadeEvents;
		timerCache_t cache;
	} timer_t;

private:

	GBA_AUDIO_SAMPLE_TYPE capacitor[(BYTE)AUDIO_CHANNELS::TOTAL_CHANNELS] = { ZERO };

	typedef struct
	{
		FLAG isChannelActuallyEnabled;
		int32_t lengthTimer;
		INC64 frequencyTimer;
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
		FIFO_t FIFO[TWO];
		FLAG nextHalfWillNotClockLengthCounter;
		FLAG wasSweepDirectionNegativeAtleastOnceSinceLastTrigger;
		FLAG didChannel3ReadWaveRamPostTrigger;
		BYTE waveRamCurrentIndex;
		BYTE waveRamCurrentBANK;
		BYTE channel3OutputLevelAndShift;
		FLAG wasPowerCycled;
		int32_t div_apu;
		int16_t sampleReadByChannel1;
		int16_t sampleReadByChannel2;
		int16_t sampleReadByChannel3;
		int16_t sampleReadByChannel4;
		INC64 downSamplingRatioCounter;
		uint32_t accumulatedTone;
		MAP8 soundTimerOverFlow;
		audioChannelInstance_t audioChannelInstance[(uint8_t)AUDIO_CHANNELS::TOTAL_CHANNELS];
		GBA_AUDIO_SAMPLE_TYPE audioBuffer[AUDIO_BUFFER_SIZE_FOR_GBA];
		float emulatorVolume;
	} audio_t;

private:

	const INC8 MIN_MAX_BG_LAYERS[EIGHT][TWO]
	{
	  {0,  3}, // Mode 0 (BG0 - BG3 text-mode)
	  {0,  2}, // Mode 1 (BG0 - BG1 text-mode, BG2 affine)
	  {2,  3}, // Mode 2 (BG2 - BG3 affine)
	  {2,  2}, // Mode 3 (BG2 240x160 65526-color bitmap)
	  {2,  2}, // Mode 4 (BG2 240x160 256-color bitmap, double-buffered)
	  {2,  2}, // Mode 5 (BG2 160x128 65536-color bitmap, double-buffered)
	  {0, -1}, // Mode 6 (invalid)
	  {0, -1} // Mode 7 (invalid)
	};

	// [shape][size]
	const DIM16 SPRITE_HEIGHTS[THREE][FOUR] =
	{
		{8,16,32,64},
		{8,8,16,32},
		{16,32,32,64}
	};

	// [shape][size]
	const DIM16 SPRITE_WIDTHS[THREE][FOUR] =
	{
		{8,16,32,64},
		{16,32,32,64},
		{8,8,16,32}
	};

	struct objDisplayCache_t
	{
		mOamAttribute_t ObjAttribute;
		SCOUNTER32 vcount;
		SDIM32 xPixelCoordinate;
		SDIM32 yPixelCoordinate;
		SDIM32 spriteXScreenCoordinate;
		SDIM32 spriteYScreenCoordinate;
		SDIM32 spriteMinXScreenCoordinate;
		SDIM32 spriteMinYScreenCoordinate;
		SDIM32 spriteMaxXScreenCoordinate;
		SDIM32 spriteMaxYScreenCoordinate;
		SDIM32 spriteXPixelCoordinate;
		SDIM32 spriteYPixelCoordinate;
		SDIM32 spriteXTileCoordinate;
		SDIM32 spriteYTileCoordinate;
		SDIM32 spriteHeight;
		SDIM32 spriteWidth;
		SCOUNTER32 spriteXStart;
		SCOUNTER32 spriteXEnd;
		ID baseTileID;
		ID tileIDAfterAccountingForY;
		ID tileIDAfterAccountingForX;
		GBA_WORD addressInTileDataArea;
		FLAG isAffine;
		FLAG isDoubleAffine;
		FLAG isDisabled;
		struct
		{
			int16_t pa;
			int16_t pb;
			int16_t pc;
			int16_t pd;
		} affine;

		void reset()
		{
			ObjAttribute = { RESET };
			xPixelCoordinate = RESET;
			yPixelCoordinate = RESET;
			spriteXScreenCoordinate = RESET;
			spriteYScreenCoordinate = RESET;
			spriteMaxXScreenCoordinate = RESET;
			spriteMaxYScreenCoordinate = RESET;
			spriteMinXScreenCoordinate = RESET;
			spriteMinYScreenCoordinate = RESET;
			spriteXPixelCoordinate = RESET;
			spriteYPixelCoordinate = RESET;
			spriteXTileCoordinate = RESET;
			spriteYTileCoordinate = RESET;
			spriteHeight = RESET;
			spriteWidth = RESET;
			spriteXStart = RESET;
			spriteXEnd = RESET;
			baseTileID = RESET;
			tileIDAfterAccountingForY = RESET;
			tileIDAfterAccountingForX = RESET;
			addressInTileDataArea = RESET;
			isAffine = RESET;
			isDoubleAffine = RESET;
			isDisabled = RESET;
			affine = { RESET };
		};
	};

	typedef struct
	{
		FLAG is8bppMode;
		INC8 subTileScrollingPixelDrops;
		struct
		{
			union
			{
				uint32_t BGX;
				int32_t BGX_Signed;
			};
			FLAG bgxOverwrittenByCPU;
			union
			{
				uint32_t BGY;
				int32_t BGY_Signed;
			};
			FLAG bgyOverwrittenByCPU;
		} internalRefPointRegisters;
		// NOTE: The above "internalRefPointRegisters" only stores partial info about the affine transformation (nothing about BG2/3PA/B/C/D)
		// The below "affine", will include the complete affine info, i.e. the combination of internalRefPointRegisters and BG2/3PA/B/C/D
		struct
		{
			int32_t affineX;
			int32_t affineY;
		} affine;
		ID fetchedTileID;
		uint32_t xPixelCoordinateAffine;
		uint32_t yPixelCoordinateAffine;
		union
		{
			struct
			{
				GBA_HALFWORD fetchedTileID : 10;
				GBA_HALFWORD hflip : 1;
				GBA_HALFWORD vflip : 1;
				GBA_HALFWORD pb : 4;
			};
			GBA_HALFWORD raw;
		} tileDescriptor;
		uint32_t tileMapX;
		uint32_t tileMapY;
		uint32_t hofs;
		uint32_t vofs;
		uint32_t xPixelCoordinate;
		uint32_t yPixelCoordinate;
		uint32_t subTileIndexer;
		GBA_HALFWORD pixelColorNumberFor4PixelsFromTileData;
		GBA_HALFWORD pixelColorNumberFor2PixelsFromTileData;
	} bgDisplayCache_t;

	typedef struct
	{
		uint32_t xCoordinate;
		uint32_t yCoordinate;
	} mergeDisplayCache_t;

	struct bitmapPerScanline_t
	{
		BYTE bitMapPerScanline[TO_UINT(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES) / EIGHT];	// 240 bits per scanline represented as 30 byte bitmap

		void reset()
		{
			memset(bitMapPerScanline, ZERO, sizeof(bitMapPerScanline));
		}
	} bitmapPerScanline_t;

	typedef struct
	{
		FLAG isObject;
		ID colorNumber;
	} colorNumber_t;

	typedef struct
	{
		FLAG wasVblankJustTriggered; // indicates when PPU enterred VBLANK
		FLAG didLCDModeChangeJustNow;	// indicates whenever PPU changes its mode
		FLAG objWaitCyclesDone;
		COUNTER32 cyclesPerSprite;
		COUNTER32 cyclesPerScanline;
		FLAG bgWaitCyclesDone;
		INC8 bgAccessPattern;
		FLAG winWaitCyclesDone;
		INC8 winAccessPattern;
		FLAG mergeWaitCyclesDone;
		INC8 mergeAccessPattern;
		OBJECT_TYPE currentObjectIsAffine;
		OBJECT_TYPE nextObjectIsAffine;
		SSTATE8 objAccessPattern;
		OBJECT_ACCESS_PATTERN objAccessPatternState;
		STATE8 objAccessOAMIDState;
		FLAG firstVRAMCycleForObjFSM;
		FLAG lastVRAMCycleForObjFSM;
		STATE8 vramCyclesStageForObjFSM;
		FLAG oamFoundValidObject;
		FLAG oamFoundAffineObject;
		FLAG allObjectsRenderedForScanline;
		SSTATE8 bgAccessPatternState[SIX];
		SSTATE8 winAccessPatternState[SIX];
		SSTATE8 mergeAccessPatternState[SIX];
		uint16_t currentPPUMode;
		FLAG ppuModeTransition;
		uint16_t currentWinPixel; // gives x coord info
		uint16_t currentBgPixel; // gives x coord info
		uint16_t currentBgPixelInTextMode[FOUR]; // gives x coord info
		uint16_t currentMergePixel; // gives x coord info
		INC64 extraPPUCyclesForProcessPPUModesDuringModeChange;
		LCD_MODES currentLCDMode;
		objDisplayCache_t objCache[TO_UINT(OBJECT_STAGE::OBJECT_TOTAL_STAGE)];
		bgDisplayCache_t bgCache[FOUR];
		mergeDisplayCache_t mergeCache;
		ID bgListAccToPriority[FOUR];
		ID layersForBlending[TWO];
		colorNumber_t colorNumberForBlending[TWO];
		gbaColor_t colorForBlending[TWO];
		uint8_t priorities[TWO];
		BYTE oamAttributes[ONETWENTYEIGHT][FOUR];
		BYTE objPriority[TO_UINT(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)][TO_UINT(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)];
		FLAG gfx_obj_window[TO_UINT(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)][TO_UINT(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)];
		FLAG gfx_window[TWO][TO_UINT(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)][TO_UINT(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)];
		OBJECT_MODE gfx_obj_mode[TO_UINT(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)][TO_UINT(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)];
		ID gfx_obj[TO_UINT(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)][TO_UINT(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)];
		ID gfx_bg[FOUR][TO_UINT(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)][TO_UINT(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)];
		union
		{
			Pixel imGuiBuffer1D[screen_width * screen_height];
			Pixel imGuiBuffer2D[screen_height][screen_width];
		} imGuiBuffer;
		uint64_t filters;
		struct otherMisc_t
		{
			FLAG modeStartEvent[TO_UINT(LCD_MODES::MODE_TOTAL_LCD_MODES)];
			FLAG modeCompleteEvent[TO_UINT(LCD_MODES::MODE_TOTAL_LCD_MODES)];
		} otherMisc;
	} display_t;

private:

	typedef struct
	{
		FLAG  quirk_irqLineEnable_previousState;
	} quirks_t;

private:

	typedef struct
	{
		uint16_t irqQ;
		uint16_t syncDelay;
	} interrupt_t;

	typedef struct
	{
		cpu_t cpuInstance;
		gbaMemory_t gbaMemory;
		FLAG irqPend;
		interrupt_t interrupt;
		uint16_t dmaPendMap;
		dma_t dma;
		uint16_t timerPendMap;
		timer_t timer[FOUR];
		audio_t audio;
		display_t display;
		quirks_t quirks;
		emulatorStatus_t emulatorStatus;
	} GBA_state_t;

	typedef union
	{
		GBA_state_t GBA_state;
		BYTE GBA_memoryState[sizeof(GBA_state_t)];
	} GBA_instance_t;

	typedef struct
	{
		FLAG isRomLoaded;
		uint32_t codeRomSize;
		BYTE pad;
	} aboutRom_t;

	typedef struct
	{
		GBA_instance_t GBA_instance;
		aboutRom_t aboutRom;
		BYTE padding[103200000 - (sizeof(GBA_instance_t) + sizeof(aboutRom_t))];
	} absolute_GBA_state_t;

	union absolute_GBA_instance_t
	{
		absolute_GBA_state_t absolute_GBA_state;
		BYTE GBA_absoluteMemoryState[sizeof(absolute_GBA_state_t)];
		absolute_GBA_instance_t()
		{
			memset(this, ZERO, sizeof(absolute_GBA_instance_t));
		}
	};

	cpu_t* pGBA_cpuInstance = nullptr;									// for readability
	registers_t* pGBA_registers = nullptr;								// for readability
	mIOFields_t* pGBA_peripherals = nullptr;							// for readability
	gbaMemory_t* pGBA_memory = nullptr;									// for readability
	audio_t* pGBA_audio = nullptr;										// for readability
	display_t* pGBA_display = nullptr;									// for readability
	GBA_instance_t* pGBA_instance = nullptr;							// for readability
	std::shared_ptr <absolute_GBA_instance_t> pAbsolute_GBA_instance;

	// Replay Mode -> skylersaleh format
	typedef struct
	{
		GBA_WORD rep_r0;
		GBA_WORD rep_r1;
		GBA_WORD rep_r2;
		GBA_WORD rep_r3;
		GBA_WORD rep_r4;
		GBA_WORD rep_r5;
		GBA_WORD rep_r6;
		GBA_WORD rep_r7;
		GBA_WORD rep_r8;
		GBA_WORD rep_r9;
		GBA_WORD rep_r10;
		GBA_WORD rep_r11;
		GBA_WORD rep_r12;
		GBA_WORD rep_r13;
		GBA_WORD rep_r14;
		GBA_WORD rep_r15;
		GBA_WORD rep_CPSR;
		GBA_WORD rep_SPSR;
	} repSkyFormat_t;
	
	std::vector<repSkyFormat_t>repSkyLog;

	uint64_t repSkyLogIttr = RESET;

#pragma pack(pop)

private:

	SDL_AudioStream* audioStream = nullptr;

private:

	void* GBANetworkEngine;

private:

	std::deque<GBA_state_t> gamePlay;

private:

	static const uint16_t TYPE_BG_WIN = (ZERO << FIFTEEN);
	static const uint16_t TYPE_OBJ = (ONE << FIFTEEN);

#pragma endregion EMULATION_DECLARATIONS

#pragma region INFRASTRUCTURE_DEFINITIONS
public:
	GBA_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config);

	~GBA_t();

	void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* network = nullptr) override;

	void setupTheAlternativeSoundOfEmulation(void* audio);

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

	void setEmulationID(EMULATION_ID ID) override;
	
	EMULATION_ID getEmulationID() override;

	float getEmulationFPS() override;
#pragma endregion INFRASTRUCTURE_DEFINITIONS

#pragma region ARM7TDMI_DEFINITIONS
private:

	MASQ_INLINE REGISTER_BANK_TYPE getRegisterBankFromOperatingMode(OP_MODE_TYPE opMode)
	{
		RETURN OP_MODE_TO_REGISTER_BANK[static_cast<uint8_t>(opMode) & 0x1F];
	}

	MASQ_INLINE REGISTER_BANK_TYPE getCurrentlyValidRegisterBank()
	{
		RETURN OP_MODE_TO_REGISTER_BANK[static_cast<uint8_t>(getARMMode()) & 0x1F];
	}

	// TODO: As of now, "getOperatingModeFromRegisterBank" function cannot differentiate between USR mode and SYS modes
	MASQ_INLINE OP_MODE_TYPE GgetOperatingModeFromRegisterBank(REGISTER_BANK_TYPE rb)
	{
		RETURN REGISTER_BANK_TO_OP_MODE[static_cast<uint8_t>(rb)];
	}

private:

	MASQ_INLINE void setARMState(STATE_TYPE armState)
	{
		pGBA_cpuInstance->registers.cpsr.psrFields.psrStateBit = (uint32_t)armState;
		pGBA_cpuInstance->armState = armState;
	}

	MASQ_INLINE void setARMMode(OP_MODE_TYPE opMode)
	{
		pGBA_cpuInstance->registers.cpsr.psrFields.psrModeBits = (uint32_t)opMode;
		pGBA_cpuInstance->armMode = opMode;
	}

	MASQ_INLINE STATE_TYPE getARMState()
	{
		pGBA_cpuInstance->armState = (STATE_TYPE)pGBA_cpuInstance->registers.cpsr.psrFields.psrStateBit;
		RETURN pGBA_cpuInstance->armState;
	}

	MASQ_INLINE OP_MODE_TYPE getARMMode()
	{
		pGBA_cpuInstance->armMode = (OP_MODE_TYPE)pGBA_cpuInstance->registers.cpsr.psrFields.psrModeBits;
		RETURN pGBA_cpuInstance->armMode;
	}

private:

	void cpuSetRegister(REGISTER_BANK_TYPE rb, REGISTER_TYPE rt, STATE_TYPE st, uint32_t u32parameter);

	uint32_t cpuReadRegister(REGISTER_BANK_TYPE rb, REGISTER_TYPE rt);

	MASQ_INLINE uint32_t getMemoryAccessCycles(GBA_WORD mCurrentAddress,
		MEMORY_ACCESS_WIDTH mAccessWidth,
		MEMORY_ACCESS_SOURCE mSource,
		MEMORY_ACCESS_TYPE accessType)
	{
		static constexpr GBA_WORD ACCESS_WIDTH_OFFSETS[] = { ONE, TWO, FOUR };

		const uint32_t currentRegion = mCurrentAddress >> TWENTYFOUR;
		const uint32_t previousAddr = pGBA_instance->GBA_state.gbaMemory.previouslyAccessedMemory;
		const uint32_t previousRegion = previousAddr >> TWENTYFOUR;

		MEMORY_ACCESS_TYPE mType;

		// Handle override (rare)
		if (pGBA_memory->setNextMemoryAccessType != MEMORY_ACCESS_TYPE::AUTOMATIC) MASQ_UNLIKELY
		{
			mType = pGBA_memory->setNextMemoryAccessType;
			pGBA_memory->setNextMemoryAccessType = MEMORY_ACCESS_TYPE::AUTOMATIC;
		}
			// Handle explicit access type
		else if (accessType != MEMORY_ACCESS_TYPE::AUTOMATIC) MASQ_LIKELY
		{
			mType = accessType;
		}
			// Automatic determination
		else MASQ_LIKELY
		{
			// Different region check (inlined)
			if (currentRegion != previousRegion) MASQ_UNLIKELY
			{
				mType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
			}
			else MASQ_LIKELY
			{
				// Lookup offset (no branch)
				const GBA_WORD offset = ACCESS_WIDTH_OFFSETS[TO_UINT8(mAccessWidth)];

		// GamePak ROM 128KB boundary check
		const FLAG isGamePakROM = (currentRegion >= 0x08) && (currentRegion <= 0x0D);
		const FLAG is128KBoundary = (mCurrentAddress & 0x1FFFF) == ZERO;

		if (isGamePakROM && is128KBoundary) MASQ_UNLIKELY
		{
			mType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
		}
		else MASQ_LIKELY
		{
			// Normal sequential check
			mType = (mCurrentAddress == previousAddr + offset)
				? MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE
				: MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
		}
	}
		}

			// Lookup cycles from table
		uint32_t nCycles;

		if (currentRegion < TO_UINT8(MEMORY_REGIONS::TOTAL_MEMORY_REGIONS) &&
			TO_UINT8(mType) < TO_UINT8(MEMORY_ACCESS_TYPE::TOTAL_MEMORY_ACCESS_TYPES) &&
			TO_UINT8(mAccessWidth) < TO_UINT8(MEMORY_ACCESS_WIDTH::TOTAL_ACCESS_WIDTH_POSSIBLE)) MASQ_LIKELY
		{
			nCycles = WAIT_CYCLES[currentRegion][TO_UINT8(mType)][TO_UINT8(mAccessWidth)];
		}
		else MASQ_UNLIKELY
		{
			nCycles = ONE;  // Invalid memory
		}

			// Update state for next call
		pGBA_instance->GBA_state.gbaMemory.previouslyAccessedMemory = mCurrentAddress;
		pGBA_memory->getPreviousMemoryAccessType = mType;

		RETURN nCycles;
	}

	// Refer "Reading from Unused Memory (00004000-01FFFFFF,10000000-FFFFFFFF)" of https://problemkaputt.de/gbatek-gba-unpredictable-things.htm
	template <typename T>
	T readOpenBus(uint32_t address, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType = MEMORY_ACCESS_TYPE::AUTOMATIC)
	{
		// Refer to "Reading from Unused Memory (00004000-01FFFFFF,10000000-FFFFFFFF)" in https://problemkaputt.de/gbatek-gba-unpredictable-things.htm
		auto shift = (address & THREE) << THREE;

		if (getARMState() == STATE_TYPE::ST_ARM)
		{
			RETURN ((T)(pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode >> shift));
		}
		else
		{
			GBA_WORD data = RESET;
			auto pc = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
			switch (pc >> TWENTYFOUR)
			{
				// EWRAM, PRAM, VRAM, ROM (16-bit)
			case 0x02:
			case 0x05:
			case 0x06:
			case 0x08:
			case 0x09:
			case 0x0A:
			case 0x0B:
			case 0x0C:
			case 0x0D:
			{
				data = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
				data |= (data << SIXTEEN);
				BREAK;
			}
			// BIOS, OAM (32-bit)
			case 0x00:
			case 0x07:
			{
				// ORed with 2 instead of 3 as LSB bit in PC is always cleared for THUMB
				if ((pc & TWO) == ZERO)
				{
					data = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
					// MSW=$+6, but $+6 is not yet fetched at this point, so currently following the same behaviour as that of other emulators
					data |= (pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode << SIXTEEN);
				}
				else
				{
					data = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
					data |= pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode << SIXTEEN;
				}
				BREAK;
			}
			// IWRAM bus (16-bit special-case)
			case 0x03:
			{
				// ORed with 2 instead of 3 as LSB bit in PC is always cleared for THUMB
				if ((pc & TWO) == ZERO)
				{
					data = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
					data |= pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode << SIXTEEN;
				}
				else
				{
					data = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
					data |= pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode << SIXTEEN;
				}
				BREAK;
			}
			}

			RETURN ((T)(data >> shift));
		}
	}

	MASQ_INLINE void ifRegUpdate(uint16_t data)
	{
		pGBA_peripherals->mIFHalfWord.mIFHalfWord |= (ONE << data);
	}

	MASQ_INLINE void cntlRegUpdate(TIMER timer, uint16_t data)
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[timer].cache.reload = data; // Store the new value in "reload"
	}

	MASQ_INLINE void cnthRegUpdate(TIMER timer, uint16_t data)
	{
		auto setTimerCNTLRegister = [&](TIMER timer, uint16_t value)
			{
				// Use a switch statement for better performance in this context.
				switch (timer)
				{
				case TIMER::TIMER0:
					pGBA_peripherals->mTIMER0CNT_L = value;
					BREAK;
				case TIMER::TIMER1:
					pGBA_peripherals->mTIMER1CNT_L = value;
					BREAK;
				case TIMER::TIMER2:
					pGBA_peripherals->mTIMER2CNT_L = value;
					BREAK;
				case TIMER::TIMER3:
					pGBA_peripherals->mTIMER3CNT_L = value;
					BREAK;
				default:
					FATAL("Unknown Timer : %d", TO_UINT8(timer));
					BREAK;
				}
			};

		static mTIMERnCNT_HHalfWord_t* CNTHLUT[] = {
			&pGBA_peripherals->mTIMER0CNT_H,
			&pGBA_peripherals->mTIMER1CNT_H,
			&pGBA_peripherals->mTIMER2CNT_H,
			&pGBA_peripherals->mTIMER3CNT_H
		};

		mTIMERnCNT_HHalfWord_t* CNTH = CNTHLUT[timer];

		BIT timerxEnBeforeUpdate = CNTH->mTIMERnCNT_HFields.TIMER_START_STOP;
		CNTH->mTIMERnCNT_HHalfWord = data;
		if (timerxEnBeforeUpdate == RESET && CNTH->mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[timer].cache.counter = pGBA_instance->GBA_state.timer[timer].cache.reload;
			setTimerCNTLRegister(timer, pGBA_instance->GBA_state.timer[timer].cache.counter);
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.timerCounter[timer] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[timer].currentState = DISABLED;
			pGBA_instance->GBA_state.timer[timer].startupDelay = ONE;
		}
	}

	GBA_HALFWORD readIO(uint32_t address, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType = MEMORY_ACCESS_TYPE::AUTOMATIC);

	// NOTE: For memory mirrors, refer http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
	template <typename T>
	T readRawMemory(uint32_t address, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType = MEMORY_ACCESS_TYPE::AUTOMATIC, FLAG LOCK = NO)
	{
		INC64 dmaCyclesInThisRun = RESET; // Currently there is no use for this, but using this to cache the count before the reset

		if ((IsAnyDMARunning() == YES) && (source == MEMORY_ACCESS_SOURCE::CPU) && (LOCK == NO))
		{
			dmaTick();	
			dmaCyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter = RESET;
			// All currenlty enabled DMA transactions should be complete by the time we come here
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.freeBusCyclesCounter = RESET;
		}

		// https://discord.com/channels/465585922579103744/465586361731121162/1321107870573400137
		// Refer: https://discord.com/channels/465585922579103744/465586361731121162/1321106423446245488
		// Refer: https://discord.com/channels/465585922579103744/465586361731121162/1321105741448347659
		if (IF_ADDRESS_WITHIN(address, SYSTEM_ROM_START_ADDRESS, GAMEPAK_ROM_WS2_END_ADDRESS))
		{
			if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				address &= ~ONE;
			}
			if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				address &= ~THREE;
			}
		}

		if (source == MEMORY_ACCESS_SOURCE::CPU)
		{
			TODO("Support for FORCED_BLANK needs to be added at line %d of file %s", __LINE__, __FILE__);
#if (DISABLED)
			if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == RESET)
			{
				if (pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_DRAW_V_DRAW)
				{
					if (IF_ADDRESS_WITHIN(address, PALETTE_RAM_START_ADDRESS, PALETTE_RAM_END_ADDRESS)
						|| (IF_ADDRESS_WITHIN(address, PALETTE_RAM_MIRROR_START_ADDRESS, PALETTE_RAM_MIRROR_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, VIDEO_RAM_START_ADDRESS, VIDEO_RAM_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, VIDEO_RAM_MIRROR_START_ADDRESS, VIDEO_RAM_MIRROR_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, OAM_MIRROR_START_ADDRESS, OAM_MIRROR_END_ADDRESS)))
					{
						cpuTick();
						RETURN ZERO;
					}
				}
				else if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.HBLANK_INTERVAL_FREE == RESET)
				{
					if (pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_BLANK_V_BLANK
						|| pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_BLANK_V_DRAW)
					{
						if ((IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_END_ADDRESS))
							|| (IF_ADDRESS_WITHIN(address, OAM_MIRROR_START_ADDRESS, OAM_MIRROR_END_ADDRESS)))
						{
							cpuTick();
							RETURN ZERO;
						}
					}
				}
			}
#endif
			uint32_t cpuTicks = getMemoryAccessCycles(address, accessWidth, source, accessType);
			while (cpuTicks)
			{
				cpuTick();
				--cpuTicks;
			}
		}

		if (source == MEMORY_ACCESS_SOURCE::DMA)
		{
			TODO("Support for FORCED_BLANK needs to be added at line %d of file %s", __LINE__, __FILE__);
#if (DISABLED)
			if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == RESET)
			{
				if (pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_DRAW_V_DRAW)
				{
					if (IF_ADDRESS_WITHIN(address, PALETTE_RAM_START_ADDRESS, PALETTE_RAM_END_ADDRESS)
						|| (IF_ADDRESS_WITHIN(address, PALETTE_RAM_MIRROR_START_ADDRESS, PALETTE_RAM_MIRROR_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, VIDEO_RAM_START_ADDRESS, VIDEO_RAM_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, VIDEO_RAM_MIRROR_START_ADDRESS, VIDEO_RAM_MIRROR_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, OAM_MIRROR_START_ADDRESS, OAM_MIRROR_END_ADDRESS)))
					{
						cpuTick();
						RETURN ZERO;
					}
				}
				else if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.HBLANK_INTERVAL_FREE == RESET)
				{
					if (pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_BLANK_V_BLANK
						|| pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_BLANK_V_DRAW)
					{
						if ((IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_END_ADDRESS))
							|| (IF_ADDRESS_WITHIN(address, OAM_MIRROR_START_ADDRESS, OAM_MIRROR_END_ADDRESS)))
						{
							cpuTick();
							RETURN ZERO;
						}
					}
				}
			}
#endif
			uint32_t dmaTicks = getMemoryAccessCycles(address, accessWidth, source, accessType);
			while (dmaTicks)
			{
				cpuTick(TICK_TYPE::DMA_TICK);
				--dmaTicks;
			}
		}

		if (IF_ADDRESS_WITHIN(address, SYSTEM_ROM_START_ADDRESS, SYSTEM_ROM_END_ADDRESS))
		{
			// NOTE: GBATEK mentions this needs to done for BIOS unused memory and not BIOS memory itself, but suite.gba needs this to pass S16 unaligned BIOS load tests 
			auto shift = (address & THREE) << THREE;

			// Refer "Reading from BIOS Memory (00000000-00003FFF)" of https://problemkaputt.de/gbatek-gba-unpredictable-things.htm
			if ((cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC)) > SYSTEM_ROM_END_ADDRESS)
			{
				CPUWARN("Illegal Bios Read");
			}
			else
			{
				// BIOS reads needs to ALWAYS 32 bit for suite.gba to pass
				// https://problemkaputt.de/gbatek-gba-unpredictable-things.htm does hint at this in the "Forward"
				// i.e. all accesses are 32 bit only, and if 8 bit or 16 bit, then data is rotated and then isolated from the 32 bit read word and sent out...

				address &= ~THREE; // Handle 32 bit alignment
				pGBA_instance->GBA_state.gbaMemory.previouslyLatchedBiosData
					= pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mSystemRom.mSystemRom32bit[(address - SYSTEM_ROM_START_ADDRESS) / FOUR];
			}
			RETURN((T)(pGBA_instance->GBA_state.gbaMemory.previouslyLatchedBiosData >> shift));
		}
		else if (IF_ADDRESS_WITHIN(address, SYSTEM_ROM_UNUSED_START_ADDRESS, SYSTEM_ROM_UNUSED_END_ADDRESS))
		{
			// Refer "Reading from Unused Memory (00004000-01FFFFFF,10000000-FFFFFFFF)" of https://problemkaputt.de/gbatek-gba-unpredictable-things.htm
			RETURN readOpenBus<T>(address, accessWidth, source, accessType);
		}
		else if (IF_ADDRESS_WITHIN(address, EXT_WORK_RAM_START_ADDRESS, EXT_WORK_RAM_MIRROR_END_ADDRESS))
		{
			// As part of first level mirroring, EWRAM (0x02000000 - 0x0203FFFF) is mirrored in steps of 256K 
			address &= 0x3FFFF;

			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mExtWorkRam.mExtWorkRam8bit[address]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mExtWorkRam.mExtWorkRam16bit[address / TWO]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mExtWorkRam.mExtWorkRam32bit[address / FOUR]);
			}
		}
		else if (IF_ADDRESS_WITHIN(address, INT_WORK_RAM_START_ADDRESS, INT_WORK_RAM_MIRROR_END_ADDRESS))
		{
			// As part of first level mirroring, IWRAM (0x03000000 - 0x03007FFF) is mirrored in steps of 32K 
			address &= 0x7FFF;

			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mIntWorkRam.mIntWorkRam8bit[address]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mIntWorkRam.mIntWorkRam16bit[address / TWO]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mIntWorkRam.mIntWorkRam32bit[address / FOUR]);
			}
		}
		else if (IF_ADDRESS_WITHIN(address, IO_START_ADDRESS, IO_4000302 + ONE))
		{
			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				if ((address & 0x01) == ZERO)
				{
					RETURN (BYTE)readIO(address, accessWidth, source, accessType);
				}
				else
				{
					RETURN (BYTE)(readIO((address & (~ONE)), accessWidth, source, accessType) >> EIGHT);
				}
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				if ((address & 0x01) == ZERO)
				{
					RETURN static_cast<T>(readIO(address, accessWidth, source, accessType));
				}
				else
				{
					BYTE byte0 = (BYTE)(readIO((address & (~ONE)), accessWidth, source, accessType) >> EIGHT);
					address += TWO;
					BYTE byte1 = (BYTE)readIO(address, accessWidth, source, accessType);
					RETURN static_cast<T>(byte0 | (byte1 << EIGHT));
				}
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				if (((address & 0x03) == ZERO) || ((address & 0x03) == TWO))
				{
					GBA_HALFWORD lsWord = readIO(address, accessWidth, source, accessType);
					GBA_HALFWORD msWord = readIO(address + TWO, accessWidth, source, accessType);
					RETURN (GBA_WORD)(lsWord | (msWord << SIXTEEN));
				}
				else if (((address & 0x03) == ONE) || ((address & 0x03) == THREE))
				{
					BYTE byte0 = (BYTE)(readIO((address & (~ONE)), accessWidth, source, accessType) >> EIGHT);
					address = (address & (~ONE)) + TWO;
					GBA_HALFWORD word1 = readIO(address, accessWidth, source, accessType);
					GBA_HALFWORD byte1 = (BYTE)(word1 & 0x00FF);
					GBA_HALFWORD byte2 = (BYTE)(word1 >> EIGHT);
					address += TWO;
					GBA_HALFWORD byte3 = (BYTE)(readIO(address, accessWidth, source, accessType) & 0x00FF);
					long value =
						((byte0 & 0xFF) << 0) |
						((byte1 & 0xFF) << 8) |
						((byte2 & 0xFF) << 16) |
						((long)(byte3 & 0xFF) << 24);
					RETURN (GBA_WORD)value;
				}
			}

			FATAL("Unknown IO address : %u", address);
		}
		else if (IF_ADDRESS_WITHIN(address, IO_4000302 + TWO, IO_END_ADDRESS))
		{
			if (IF_ADDRESS_WITHIN(address, 0x0400100C, 0x0400100D))
			{
				RETURN (T)0xDEAD;
			}
			RETURN readOpenBus<T>(address, accessWidth, source, accessType);
		}
		else if (IF_ADDRESS_WITHIN(address, PALETTE_RAM_START_ADDRESS, PALETTE_RAM_MIRROR_END_ADDRESS))
		{
			// As part of first level mirroring, Palette RAM (0x05000000 - 0x05FFFFFF) is mirrored in steps of 1K 
			address &= 0x3FF;

			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mBgObjPaletteRam.mBgObjPaletteRamMemory8bit[address]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mBgObjPaletteRam.mBgObjPaletteRamMemory16bit[address / TWO]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mBgObjPaletteRam.mBgObjPaletteRamMemory32bit[address / FOUR]);
			}
		}
		else if (IF_ADDRESS_WITHIN(address, VIDEO_RAM_START_ADDRESS, VIDEO_RAM_MIRROR_END_ADDRESS))
		{
			// NOTE1:
			// As part of first level mirroring, VRAM (0x06000000 - 0x06FFFFFF) is mirrored in steps of 128K 
			address &= 0x1FFFF;
			// 128K is split as 64K + 32K + Mirrored 32K
			// This second level mirroring of the last 32K is needed only after we cross the 96K boundary post the first level mirroring
			// We will cross the 96K boundary only when we are accessing the VRAM OBJ memory and hence no need to handle any mirroring in VRAM BG memory

			// NOTE2:
			// Get the BG-OBJ boundary (COMPLETELY INDEPENDENT OF 64K + 32K + Mirrored 32K memory split)
			// This is needed for the following:
			// 1) To handle the convoluted 8bit writes to BG area
			// 2) To handle the "ignored read/writes" to (0x06010000 - (VIDEO_RAM_START_ADDRESS + bgObjBoundary)) in bit map modes
			// Refer https://github.com/nba-emu/hw-test/tree/master/ppu/vram-mirror
			// "It appears that VRAM accesses above 0x06010000 (text-based modes) or 0x06014000 (bitmap-based modes) are always passed through the OBJ engine, 
			// which in bitmap-based modes cannot access the VRAM bank at 0x06010000 - 0x06013FFF (mirrored at 0x06018000), since it is allocated to the BG engine."
			//
			GBA_WORD bgObjBoundary = ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000);

			// Handling the BG part
			if (address < bgObjBoundary)
			{
				if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
				{
					RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam8bit[address]);
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
				{
					RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam16bit[address / TWO]);
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
				{
					RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam32bit[address / FOUR]);
				}
			}
			// Handling the OBJ part
			// We will come here only is address is above 0x06010000 (or 0x06014000)
			else
			{
				// If address is in range of VIDEO_RAM_MIRROR_START_ADDRESS - 128K boundary
				// we need to handle the 32K mirroring and also the "ignored read/writes" of bit map modes
				if (address >= 0x18000)
				{
					// NOTE3: As mentioned in NOTE1, since we are in VRAM OBJ memory, we need to handle the second level mirroring
					// If address is beyond 64K + 32K and within 128K
					// then we need to map (64K + 32K + <any address>) := (64K + <any address>) as 32K is mirrored
					// Either we subtract the address by 32K which is done using bitwise operation below!
					address &= ~0x8000; // Effectively is a shifting the address down by 32K

					// NOTE4: Also as mentioned in NOTE2, since we are in VRAM OBJ memory, we need to handle the "ignored read/writes" if in bitmap mode
					// In tile mode, BG area ends at 0x06010000 and OBJ starts from here
					// But in bit map mode, BG area ends at 0x06014000 and OBJ starts from here
					// But as per https://github.com/nba-emu/hw-test/tree/master/ppu/vram-mirror
					// OBJ engine is used if we access any address above 0x06010000 (in tile mode) or 0x06014000 (bitmap mode)
					// NOW, ASSUME A CASE WHERE THE ADDRESS IS 0x06018000 IN BIT MAP MODE
					// BECAUSE OF 2nd LEVEL MIRRORING, 0x06018000 GETS MAPPED TO 0x06010000
					// BUT ADDRESS TILL 0x06014000 IS USED BY BG IN BIT MAP MODE, SO IGNORED READ/WRITES OCCURS

					if (address < bgObjBoundary)
					{
						RETURN ZERO;
					}

				}

				if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
				{
					RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam8bit[address]);
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
				{
					RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam16bit[address / TWO]);
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
				{
					RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam32bit[address / FOUR]);
				}
			}
		}
		else if (IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_MIRROR_END_ADDRESS))
		{
			// As part of first level mirroring, OAM (0x07000000 - 0x07FFFFFF) is mirrored in steps of 1K 
			address &= 0x3FF;

			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mOamAttributes.mOamAttributesMemory8bit[address]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mOamAttributes.mOamAttributesMemory16bit[address / TWO]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mOamAttributes.mOamAttributesMemory32bit[address / FOUR]);
			}
		}
		else if (IF_ADDRESS_WITHIN(address, GAMEPAK_ROM_WS0_START_ADDRESS, GAMEPAK_ROM_WS2_END_ADDRESS))
		{
			// As part of first level mirroring, OAM (0x08000000 - 0x0DFFFFFF) is mirrored in steps of GAMEPAK_ROM_SIZE 
			address &= 0x01FFFFFF;

			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Memory8bit[address]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Memory16bit[address / TWO]);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Memory32bit[address / FOUR]);
			}
		}
		else if (IF_ADDRESS_WITHIN(address, GAMEPAK_SRAM_START_ADDRESS, GAMEPAK_SRAM_MIRROR_END_ADDRESS))
		{
			switch (pGBA_instance->GBA_state.emulatorStatus.backup.backupType)
			{
			case BACKUP_TYPE::SRAM:
			{
				// As part of first level of mirroring, every 64K region is mirrored across the complete 32 MB region
				// As part of second level mirroring, every 32K is repeated twice within 64K
				address &= 0x7FFF;

				if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
				{
					RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakBackup.mGamePakSram.mExtSram8bit[address]);
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
				{
					// Refer : http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
					RETURN static_cast<T>((GBA_HALFWORD)pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakBackup.mGamePakSram.mExtSram8bit[address] * 0x0101);
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
				{
					// Refer : http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
					RETURN (GBA_WORD)((GBA_WORD)pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakBackup.mGamePakSram.mExtSram8bit[address] * 0x01010101);
				}
				BREAK;
			}
			case BACKUP_TYPE::FLASH64K:
			case BACKUP_TYPE::FLASH128K:
			{
				while (address >= GAMEPAK_SRAM_MIRROR_START_ADDRESS)
				{
					address = (address % GAMEPAK_SRAM_MIRROR_START_ADDRESS) + GAMEPAK_SRAM_START_ADDRESS;
				}

				auto currentMemoryBank = TO_UINT8(pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentMemoryBank);

				if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
				{
					RETURN static_cast<T>(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][address - GAMEPAK_SRAM_START_ADDRESS]);
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
				{
					// Refer : http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
					RETURN static_cast<T>((GBA_HALFWORD)pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][address - GAMEPAK_SRAM_START_ADDRESS] * 0x0101);
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
				{
					// Refer : http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
					RETURN static_cast<T>((GBA_WORD)pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][address - GAMEPAK_SRAM_START_ADDRESS] * 0x01010101);
				}
				BREAK;
			}
			case BACKUP_TYPE::EEPROM:
			{
				WARN("Backup type EEPROM is not supported yet");
				BREAK;
			}
			default:
			{
				WARN("Unsupported Backup Type : %d", TO_UINT(pGBA_instance->GBA_state.emulatorStatus.backup.backupType));
				RETURN 0xFF;
			}
			}
		}

		WARN("Trying to read from invalid memory : 0x%08X", address);

		// Open Bus needs to aligned address
		if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
		{
			address &= ~ONE;
		}
		if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
		{
			address &= ~THREE;
		}

		RETURN readOpenBus<T>(address, accessWidth, source, accessType);
	}

	void writeIO(uint32_t address, GBA_HALFWORD data, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType = MEMORY_ACCESS_TYPE::AUTOMATIC);

	template <typename T>
	void writeRawMemory(uint32_t address, T data, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType = MEMORY_ACCESS_TYPE::AUTOMATIC, FLAG LOCK = NO)
	{
		INC64 dmaCyclesInThisRun = RESET; // Currently there is no use for this, but using this to cache the count before the reset

		if (source == MEMORY_ACCESS_SOURCE::PPU || source == MEMORY_ACCESS_SOURCE::APU)
		{
			FATAL("PPU or APU writing to memory!");
		}

		if ((IsAnyDMARunning() == YES) && (source == MEMORY_ACCESS_SOURCE::CPU) && (LOCK == NO))
		{
			dmaTick();
			dmaCyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter = RESET;
			// All currenlty enabled DMA transactions should be complete by the time we come here
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.freeBusCyclesCounter = RESET;
		}

		// https://discord.com/channels/465585922579103744/465586361731121162/1321107870573400137
		// Refer: https://discord.com/channels/465585922579103744/465586361731121162/1321105741448347659
		if (IF_ADDRESS_WITHIN(address, SYSTEM_ROM_START_ADDRESS, GAMEPAK_ROM_WS2_END_ADDRESS))
		{
			if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				address &= ~ONE;
			}
			if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				address &= ~THREE;
			}
		}

		if (source == MEMORY_ACCESS_SOURCE::CPU)
		{
			TODO("Support for FORCED_BLANK needs to be added at line %d of file %s", __LINE__, __FILE__);
#if (DISABLED)
			if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == RESET)
			{
				if (pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_DRAW_V_DRAW)
				{
					if (IF_ADDRESS_WITHIN(address, PALETTE_RAM_START_ADDRESS, PALETTE_RAM_END_ADDRESS)
						|| (IF_ADDRESS_WITHIN(address, PALETTE_RAM_MIRROR_START_ADDRESS, PALETTE_RAM_MIRROR_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, VIDEO_RAM_START_ADDRESS, VIDEO_RAM_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, VIDEO_RAM_MIRROR_START_ADDRESS, VIDEO_RAM_MIRROR_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, OAM_MIRROR_START_ADDRESS, OAM_MIRROR_END_ADDRESS)))
					{
						cpuTick();
						RETURN;
					}
				}
				else
				{
					if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.HBLANK_INTERVAL_FREE == RESET)
					{
						if (pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_BLANK_V_BLANK
							|| pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_BLANK_V_DRAW)
						{
							if ((IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_END_ADDRESS))
								|| (IF_ADDRESS_WITHIN(address, OAM_MIRROR_START_ADDRESS, OAM_MIRROR_END_ADDRESS)))
							{
								cpuTick();
								RETURN;
							}
						}
					}
				}
			}
#endif
			uint32_t cpuTicks = getMemoryAccessCycles(address, accessWidth, source, accessType);
			while (cpuTicks)
			{
				cpuTick();
				--cpuTicks;
			}
		}

		if (source == MEMORY_ACCESS_SOURCE::DMA)
		{
			TODO("Support for FORCED_BLANK needs to be added at line %d of file %s", __LINE__, __FILE__);
#if (DISABLED)
			if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == RESET)
			{
				if (pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_DRAW_V_DRAW)
				{
					if (IF_ADDRESS_WITHIN(address, PALETTE_RAM_START_ADDRESS, PALETTE_RAM_END_ADDRESS)
						|| (IF_ADDRESS_WITHIN(address, PALETTE_RAM_MIRROR_START_ADDRESS, PALETTE_RAM_MIRROR_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, VIDEO_RAM_START_ADDRESS, VIDEO_RAM_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, VIDEO_RAM_MIRROR_START_ADDRESS, VIDEO_RAM_MIRROR_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_END_ADDRESS))
						|| (IF_ADDRESS_WITHIN(address, OAM_MIRROR_START_ADDRESS, OAM_MIRROR_END_ADDRESS)))
					{
						cpuTick();
						RETURN;
					}
				}
				else
				{
					if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.HBLANK_INTERVAL_FREE == RESET)
					{
						if (pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_BLANK_V_BLANK
							|| pGBA_display->currentLCDMode == LCD_MODES::MODE_LCD_H_BLANK_V_DRAW)
						{
							if ((IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_END_ADDRESS))
								|| (IF_ADDRESS_WITHIN(address, OAM_MIRROR_START_ADDRESS, OAM_MIRROR_END_ADDRESS)))
							{
								cpuTick();
								RETURN;
							}
						}
					}
				}
			}
#endif
			uint32_t dmaTicks = getMemoryAccessCycles(address, accessWidth, source, accessType);
			while (dmaTicks)
			{
				cpuTick(TICK_TYPE::DMA_TICK);
				--dmaTicks;
			}
		}

		if (IF_ADDRESS_WITHIN(address, SYSTEM_ROM_START_ADDRESS, SYSTEM_ROM_END_ADDRESS))
		{
			RETURN;
		}
		else if (IF_ADDRESS_WITHIN(address, EXT_WORK_RAM_START_ADDRESS, EXT_WORK_RAM_MIRROR_END_ADDRESS))
		{
			// As part of first level mirroring, EWRAM (0x02000000 - 0x0203FFFF) is mirrored in steps of 256K 
			address &= 0x3FFFF;

			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mExtWorkRam.mExtWorkRam8bit[address] = TO_UINT8(data);
				RETURN;
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mExtWorkRam.mExtWorkRam16bit[address / TWO] = TO_UINT16(data);
				RETURN;
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mExtWorkRam.mExtWorkRam32bit[address / FOUR] = TO_UINT32(data);
				RETURN;
			}
		}
		else if (IF_ADDRESS_WITHIN(address, INT_WORK_RAM_START_ADDRESS, INT_WORK_RAM_MIRROR_END_ADDRESS))
		{
			// As part of first level mirroring, IWRAM (0x03000000 - 0x03007FFF) is mirrored in steps of 32K 
			address &= 0x7FFF;

			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mIntWorkRam.mIntWorkRam8bit[address] = TO_UINT8(data);
				RETURN;
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mIntWorkRam.mIntWorkRam16bit[address / TWO] = TO_UINT16(data);
				RETURN;
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mIntWorkRam.mIntWorkRam32bit[address / FOUR] = TO_UINT32(data);
				RETURN;
			}
		}
		else if (IF_ADDRESS_WITHIN(address, IO_START_ADDRESS, IO_4000302 + ONE))
		{
			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				data = (BYTE)data;
				GBA_HALFWORD currentWord = readIO((address & (~ONE)), accessWidth, MEMORY_ACCESS_SOURCE::HOST, accessType);
				GBA_HALFWORD newWord = currentWord;
				if ((address & 0x01) == ZERO)
				{
					newWord = (data | (currentWord & 0xFF00));
				}
				else
				{
					newWord = ((currentWord & 0x00FF) | (data << EIGHT));
				}
				writeIO((address & (~ONE)), newWord, accessWidth, source, accessType);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				data = (GBA_HALFWORD)data;
				if ((address & 0x01) == ZERO)
				{
					writeIO(address, data, accessWidth, source, accessType);
				}
				else
				{
					address &= ~ONE;
					GBA_HALFWORD currentLWord = readIO(address, accessWidth, MEMORY_ACCESS_SOURCE::HOST, accessType);
					GBA_HALFWORD currentHWord = readIO(address + ONE, accessWidth, MEMORY_ACCESS_SOURCE::HOST, accessType);
					GBA_HALFWORD newLWord = ((currentLWord & 0x00FF) | ((data & 0x00FF) << EIGHT));
					GBA_HALFWORD newHWord = (((data & 0xFF00) >> EIGHT) | (currentHWord & 0xFF00));
					writeIO(address, newLWord, accessWidth, source, accessType);
					writeIO(address + TWO, newHWord, accessWidth, source, accessType);
				}
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				data = static_cast<GBA_WORD>(data);
				if (((address & 0x03) == ZERO) || ((address & 0x03) == TWO))
				{
					writeIO(address, (data & 0x0000FFFF), accessWidth, source, accessType);
					writeIO(address + TWO, ((data & 0xFFFF0000) >> SIXTEEN), accessWidth, source, accessType);
				}
				else if (((address & 0x03) == ONE) || ((address & 0x03) == THREE))
				{
					address &= ~ONE;
					GBA_HALFWORD currentLWord = readIO(address, accessWidth, MEMORY_ACCESS_SOURCE::HOST, accessType);
					GBA_HALFWORD currentHWord = readIO(address + ONE, accessWidth, MEMORY_ACCESS_SOURCE::HOST, accessType);
					GBA_WORD newLWord = ((currentLWord & 0x00FF) | ((static_cast<GBA_WORD>(data) & 0x000000FF) << EIGHT));
					GBA_WORD newHWord = (((static_cast<GBA_WORD>(data) & 0x0000FF00) >> EIGHT) | (currentHWord & 0x0000FF00));
					writeIO(address, static_cast<GBA_HALFWORD>(newLWord), accessWidth, source, accessType);
					writeIO(address + ONE, static_cast<GBA_HALFWORD>(newHWord), accessWidth, source, accessType);

					address += TWO;
					address &= ~ONE;

					currentLWord = readIO(address, accessWidth, MEMORY_ACCESS_SOURCE::HOST, accessType);
					currentHWord = readIO(address + ONE, accessWidth, MEMORY_ACCESS_SOURCE::HOST, accessType);
					newLWord = ((currentLWord & 0x000000FF) | (((static_cast<GBA_WORD>(data) >> SIXTEEN) & 0x000000FF) << EIGHT));
					newHWord = ((((static_cast<GBA_WORD>(data) >> SIXTEEN) & 0x0000FF00) >> EIGHT) | (currentHWord & 0x0000FF00));
					writeIO(address, static_cast<GBA_HALFWORD>(newLWord), accessWidth, source, accessType);
					writeIO(address + ONE, static_cast<GBA_HALFWORD>(newHWord), accessWidth, source, accessType);
				}
			}

			RETURN;
		}
		else if (IF_ADDRESS_WITHIN(address, IO_4000302 + TWO, IO_END_ADDRESS))
		{
			RETURN;
		}
		else if (IF_ADDRESS_WITHIN(address, PALETTE_RAM_START_ADDRESS, PALETTE_RAM_MIRROR_END_ADDRESS))
		{
			// As part of first level mirroring, Palette RAM (0x05000000 - 0x05FFFFFF) is mirrored in steps of 1K 
			address &= 0x3FF;

			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				// Formula obtained from http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
				address = (address & ~ONE);
				GBA_HALFWORD modifiedData = (GBA_HALFWORD)data * (GBA_HALFWORD)0x0101;
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mBgObjPaletteRam.mBgObjPaletteRamMemory16bit[address / TWO] = TO_UINT16(modifiedData);
				RETURN;
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mBgObjPaletteRam.mBgObjPaletteRamMemory16bit[address / TWO] = TO_UINT16(data);
				RETURN;
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mBgObjPaletteRam.mBgObjPaletteRamMemory32bit[address / FOUR] = TO_UINT32(data);
				RETURN;
			}
		}
		else if (IF_ADDRESS_WITHIN(address, VIDEO_RAM_START_ADDRESS, VIDEO_RAM_MIRROR_END_ADDRESS))
		{
			// NOTE1:
			// As part of first level mirroring, VRAM (0x06000000 - 0x06FFFFFF) is mirrored in steps of 128K 
			address &= 0x1FFFF;
			// 128K is split as 64K + 32K + Mirrored 32K
			// This second level mirroring of the last 32K is needed only after we cross the 96K boundary post the first level mirroring
			// We will cross the 96K boundary only when we are accessing the VRAM OBJ memory and hence no need to handle any mirroring in VRAM BG memory

			// NOTE2:
			// Get the BG-OBJ boundary (COMPLETELY INDEPENDENT OF 64K + 32K + Mirrored 32K memory split)
			// This is needed for the following:
			// 1) To handle the convoluted 8bit writes to BG area
			// 2) To handle the "ignored read/writes" to (0x06010000 - (VIDEO_RAM_START_ADDRESS + bgObjBoundary)) in bit map modes
			// Refer https://github.com/nba-emu/hw-test/tree/master/ppu/vram-mirror
			// "It appears that VRAM accesses above 0x06010000 (text-based modes) or 0x06014000 (bitmap-based modes) are always passed through the OBJ engine, 
			// which in bitmap-based modes cannot access the VRAM bank at 0x06010000 - 0x06013FFF (mirrored at 0x06018000), since it is allocated to the BG engine."
			//
			GBA_WORD bgObjBoundary = ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000);

			// Handling the BG part
			if (address < bgObjBoundary)
			{
				if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
				{
					// Refer to "Writing 8bit Data to Video Memory" of http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
					// 8-bit write is allowed only in VRAM BG area
					// Formula obtained from http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
					address = (address & ~ONE);
					GBA_HALFWORD modifiedData = data * 0x0101;
					pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam16bit[address / TWO] = TO_UINT16(modifiedData);
					RETURN;
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
				{
					pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam16bit[address / TWO] = TO_UINT16(data);
					RETURN;
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
				{
					pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam32bit[address / FOUR] = TO_UINT32(data);
					RETURN;
				}
			}
			// Handling the OBJ part
			// We will come here only is address is above 0x06010000 (or 0x06014000)
			else
			{
				// If address is in range of VIDEO_RAM_MIRROR_START_ADDRESS - 128K boundary
				if (address >= 0x18000)
				{
					// NOTE3: As mentioned in NOTE1, since we are in VRAM OBJ memory, we need to handle the second level mirroring
					// If address is beyond 64K + 32K and within 128K
					// then we need to map (64K + 32K + <any address>) := (64K + <any address>) as 32K is mirrored
					// Either we subtract the address by 32K which is done using bitwise operation below!
					address &= ~0x8000; // Effectively is a shifting the address down by 32K

					// NOTE4: Also as mentioned in NOTE2, since we are in VRAM OBJ memory, we need to handle the "ignored read/writes" if in bitmap mode
					// In tile mode, BG area ends at 0x06010000 and OBJ starts from here
					// But in bit map mode, BG area ends at 0x06014000 and OBJ starts from here
					// But as per https://github.com/nba-emu/hw-test/tree/master/ppu/vram-mirror
					// OBJ engine is used if we access any address above 0x06010000 (in tile mode) or 0x06014000 (bitmap mode)
					// NOW, ASSUME A CASE WHERE THE ADDRESS IS 0x06018000 IN BIT MAP MODE
					// BECAUSE OF 2nd LEVEL MIRRORING, 0x06018000 GETS MAPPED TO 0x06010000
					// BUT ADDRESS TILL 0x06014000 IS USED BY BG IN BIT MAP MODE, SO IGNORED READ/WRITES OCCURS

					if (address < bgObjBoundary)
					{
						RETURN;
					}

				}

				if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
				{
					// Refer to "Writing 8bit Data to Video Memory" of http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
					// 8-bit write is not allowed in VRAM OBJ area
					RETURN;
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
				{
					pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam16bit[address / TWO] = TO_UINT16(data);
					RETURN;
				}
				else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
				{
					pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mVideoRam.mVideoRam32bit[address / FOUR] = TO_UINT32(data);
					RETURN;
				}
			}
		}
		else if (IF_ADDRESS_WITHIN(address, OAM_START_ADDRESS, OAM_MIRROR_END_ADDRESS))
		{
			// As part of first level mirroring, OAM (0x07000000 - 0x07FFFFFF) is mirrored in steps of 1K 
			address &= 0x3FF;

			if (accessWidth == MEMORY_ACCESS_WIDTH::EIGHT_BIT)
			{
				// Refer to "Writing 8bit Data to Video Memory" of http://problemkaputt.de/gbatek-gba-unpredictable-things.htm
				// 8-bit write is not allowed in OAM area
				RETURN;
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mOamAttributes.mOamAttributesMemory16bit[address / TWO] = TO_UINT16(data);
				RETURN;
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mOamAttributes.mOamAttributesMemory32bit[address / FOUR] = TO_UINT32(data);
				RETURN;
			}
		}
		else if (IF_ADDRESS_WITHIN(address, GAMEPAK_ROM_WS0_START_ADDRESS, GAMEPAK_ROM_WS2_END_ADDRESS))
		{
			RETURN;
		}
		else if (IF_ADDRESS_WITHIN(address, GAMEPAK_SRAM_START_ADDRESS, GAMEPAK_SRAM_MIRROR_END_ADDRESS))
		{
			// Refer "Accessing SRAM Area by 16bit/32bit" of https://problemkaputt.de/gbatek-gba-unpredictable-things.htm
			if (accessWidth == MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)
			{
				data = performShiftOperation(
					NO
					, SHIFT_TYPE::ROR
					, ((address & ONE) << THREE)
					, data
					, DISABLED
				);
			}
			else if (accessWidth == MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)
			{
				data = performShiftOperation(
					NO
					, SHIFT_TYPE::ROR
					, ((address & THREE) << THREE)
					, data
					, DISABLED
				);
			}

			switch (pGBA_instance->GBA_state.emulatorStatus.backup.backupType)
			{
			case BACKUP_TYPE::SRAM:
			{
				// As part of first level of mirroring, every 64K region is mirrored across the complete 32 MB region
				// As part of second level mirroring, every 32K is repeated twice within 64K
				address &= 0x7FFF;
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakBackup.mGamePakSram.mExtSram8bit[address] = TO_UINT8(data);
				BREAK;
			}
			case BACKUP_TYPE::FLASH64K:
			case BACKUP_TYPE::FLASH128K:
			{
				while (address >= GAMEPAK_SRAM_MIRROR_START_ADDRESS)
				{
					address = (address % GAMEPAK_SRAM_MIRROR_START_ADDRESS) + GAMEPAK_SRAM_START_ADDRESS;
				}

				auto currentMemoryBank = TO_UINT8(pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentMemoryBank);

				if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.chooseMemoryBank == YES)
				{
					if (address == FLASH_ACCESS_MEMORY0)
					{
						pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentMemoryBank =
							((data == ZERO) ? BACKUP_FLASH_MEMORY_BANK::BANK0 : BACKUP_FLASH_MEMORY_BANK::BANK1);
						pGBA_instance->GBA_state.emulatorStatus.backup.flash.chooseMemoryBank = NO;
					}
					else
					{
						WARN("Memory Bank Not Chosen!");
					}
				}
				else if
					(
						(address == FLASH_ACCESS_MEMORY2)
						||
						(address == FLASH_ACCESS_MEMORY3)
						)
				{
					pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][address - GAMEPAK_SRAM_START_ADDRESS] = TO_UINT8(data);
				}
				// TO handle for "Erase 4KB sector"
				else if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.previousFlashCommand == BACKUP_FLASH_CMDS::START_ERASE_CMD)
				{
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.erase4kbStartAddr = address;
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.erase4kbPageNumber = ((address & 0xF000) >> TWELVE);

					pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentFlashCommand = (BACKUP_FLASH_CMDS)data;
				}
				else if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.allowFlashWrite == YES)
				{
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.allowFlashWrite = NO;

					if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.isErased[currentMemoryBank][address - GAMEPAK_SRAM_START_ADDRESS] == YES)
					{
						pGBA_instance->GBA_state.emulatorStatus.backup.flash.isErased[currentMemoryBank][address - GAMEPAK_SRAM_START_ADDRESS] = NO;

						pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][address - GAMEPAK_SRAM_START_ADDRESS] = TO_UINT8(data);
					}
				}
				BREAK;
			}
			case BACKUP_TYPE::EEPROM:
			{
				WARN("Backup type EEPROM is not supported yet");
				BREAK;
			}
			default:
			{
				WARN("Unsupported Backup Type : %d", TO_UINT(pGBA_instance->GBA_state.emulatorStatus.backup.backupType));
			}
			}
		}

		WARN("Trying to write to invalid memory : 0x%08X", address);
	}

private:

	bool processSOC();

	void busCycles();

	void cpuIdleCycles();

	void fetchAndDecode(uint32_t newPC);

	MASQ_INLINE bool TickMultiply(FLAG isSigned, uint64_t multiplier)
	{
		uint32_t mask = 0xFFFFFF00;
		bool full = false;

		cpuIdleCycles();

		while (TRUE)
		{
			multiplier &= mask;

			if (multiplier == ZERO)
			{
				BREAK;
			}

			if (isSigned == YES)
			{
				if (multiplier == mask)
				{
					BREAK;
				}
			}

			mask <<= EIGHT;
			full = true;
			cpuIdleCycles();
		}

		RETURN full;
	}

	MASQ_INLINE bool MultiplyCarrySimple(uint32_t multiplier)
	{
		// Carry comes directly from final injected booth carry bit.
		// Final booth addend is negative only if upper 2 bits are 10.
		RETURN (multiplier >> 30) == 2;
	}

	MASQ_INLINE bool MultiplyCarryLo(
		uint32_t multiplicand,
		uint32_t multiplier,
		uint32_t accum = 0
	)
	{
		// Set low bit of multiplicand to cause negation to invert the upper bits.
		// This bit cannot propagate to the resulting carry bit.
		multiplicand |= 1;

		// Optimized first iteration.
		uint32_t booth = (int32_t)(multiplier << 31) >> 31;
		uint32_t carry = multiplicand * booth;
		uint32_t sum = carry + accum;

		int shift = 29;

		do
		{
			// Process 8 multiplier bits using 4 booth iterations.
			for (int i = 0; i < 4; i++, shift -= 2)
			{
				// Get next booth factor (-2 to 2, shifted left by 30-shift).
				uint32_t next_booth = (int32_t)(multiplier << shift) >> shift;
				uint32_t factor = next_booth - booth;
				booth = next_booth;

				// Get scaled value of booth addend.
				uint32_t addend = multiplicand * factor;

				// Accumulate addend with carry-save add.
				accum ^= carry ^ addend;
				sum += addend;
				carry = sum - accum;
			}
		}
		while (booth != multiplier);

		// Carry flag comes from bit 31 of carry-save adder's final carry.
		RETURN (carry >> 31);
	}

	MASQ_INLINE bool MultiplyCarryHi(
		bool sign_extend,
		uint32_t multiplicand,
		uint32_t multiplier,
		uint32_t accum_hi = 0
	)
	{
		// Only last 3 booth iterations are relevant to output carry.
		// Reduce scale of both inputs to get upper bits of 64-bit booth addends
		// in upper bits of 32-bit values, while handling sign extension.
		if (sign_extend)
		{
			multiplicand = (int32_t)multiplicand >> 6;
			multiplier = (int32_t)multiplier >> 26;
		}
		else
		{
			multiplicand >>= 6;
			multiplier >>= 26;
		}

		// Set low bit of multiplicand to cause negation to invert the upper bits.
		// This bit cannot propagate to the resulting carry bit.
		multiplicand |= 1;

		// Pre-populate magic bit 61 for carry.
		uint32_t carry = ~accum_hi & 0x20000000;

		// Pre-populate magic bits 63-60 for accum (with carry magic pre-added).
		uint32_t accum = accum_hi - 0x08000000;

		// Get factors for last 3 booth iterations.
		uint32_t booth0 = (int32_t)(multiplier << 27) >> 27;
		uint32_t booth1 = (int32_t)(multiplier << 29) >> 29;
		uint32_t booth2 = (int32_t)(multiplier << 31) >> 31;

		uint32_t factor0 = multiplier - booth0;
		uint32_t factor1 = booth0 - booth1;
		uint32_t factor2 = booth1 - booth2;

		// Get scaled value of 3rd-last booth addend.
		uint32_t addend = multiplicand * factor2;

		// Finalize bits 61-60 of accum magic using its sign.
		accum -= addend & 0x10000000;

		// Get scaled value of 2nd-last booth addend.
		addend = multiplicand * factor1;

		// Finalize bits 63-62 of accum magic using its sign.
		accum -= addend & 0x40000000;

		// Get carry from carry-save add in bit 61 and propagate it to bit 62.
		uint32_t sum = accum + (addend & 0x20000000);

		// Subtract out carry magic to get actual accum magic.
		accum -= carry;

		// Get scaled value of last booth addend.
		addend = multiplicand * factor0;

		// Add to bit 62 and propagate carry.
		sum += addend & 0x40000000;

		// Cancel out accum magic bit 63 to get carry bit 63.
		RETURN (sum ^ accum) >> 31;
	}

	MASQ_INLINE bool didConditionalCheckPass(uint32_t opCodeConditionalBits, uint32_t cpsr)
	{
		CPUDEBUG("Condition: 0x%X", (uint32_t)opCodeConditionalBits);

		psr_t currentCPSR = { ZERO };
		currentCPSR.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR);

		switch ((CPSR_CONDITION_CODE)opCodeConditionalBits)
		{
		case CPSR_CONDITION_CODE::CPSR_EQ_SetZ:
		{
			RETURN((currentCPSR.psrFields.psrZeroBit == SET) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_NE_ClearZ:
		{
			RETURN((currentCPSR.psrFields.psrZeroBit == RESET) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_CS_SetC:
		{
			RETURN((currentCPSR.psrFields.psrCarryBorrowExtBit == SET) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_CS_ClearC:
		{
			RETURN((currentCPSR.psrFields.psrCarryBorrowExtBit == RESET) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_MI_SetN:
		{
			RETURN((currentCPSR.psrFields.psrNegativeBit == SET) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_PL_ClearN:
		{
			RETURN((currentCPSR.psrFields.psrNegativeBit == RESET) ? YES : NO);;
		}
		case CPSR_CONDITION_CODE::CPSR_VS_SetV:
		{
			RETURN((currentCPSR.psrFields.psrOverflowBit == SET) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_VC_ClearV:
		{
			RETURN((currentCPSR.psrFields.psrOverflowBit == RESET) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_HI_SetC_AND_ClearZ:
		{
			RETURN(((currentCPSR.psrFields.psrCarryBorrowExtBit == SET) && (currentCPSR.psrFields.psrZeroBit == RESET)) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_LS_ClearC_OR_SetZ:
		{
			RETURN(((currentCPSR.psrFields.psrCarryBorrowExtBit == RESET) || (currentCPSR.psrFields.psrZeroBit == SET)) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_GE_NEqualsV:
		{
			RETURN(((currentCPSR.psrFields.psrNegativeBit == currentCPSR.psrFields.psrOverflowBit)) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_LT_NNotEqualsV:
		{
			RETURN(((currentCPSR.psrFields.psrNegativeBit != currentCPSR.psrFields.psrOverflowBit)) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_CS_ClearZ_AND_NEqualsV:
		{
			RETURN(((currentCPSR.psrFields.psrZeroBit == RESET) && ((currentCPSR.psrFields.psrNegativeBit == currentCPSR.psrFields.psrOverflowBit))) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_CS_SetZ_OR_NNotEqualsV:
		{
			RETURN(((currentCPSR.psrFields.psrZeroBit == SET) || ((currentCPSR.psrFields.psrNegativeBit != currentCPSR.psrFields.psrOverflowBit))) ? YES : NO);
		}
		case CPSR_CONDITION_CODE::CPSR_AL_IgnoreConditions:
		{
			RETURN YES;
		}
		default:
		{
			RETURN NO;
		}
		}
	};

	GBA_WORD performShiftOperation(bool updateFlag, SHIFT_TYPE shiftType, uint32_t shiftAmount, uint32_t dataToBeShifted, bool quirkEnabled);

	bool ThumbSoftwareInterrupt();

	bool UnconditionalBranch();

	bool ConditionalBranch();

	bool MultipleLoadStore();

	bool LongBranchWithLink();

	bool AddOffsetToStackPointer();

	bool PushPopRegisters();

	bool LoadStoreHalfword();

	bool SPRelativeLoadStore();

	bool LoadAddress();

	bool LoadStoreWithImmediateOffset();

	bool LoadStoreWithRegisterOffset();

	bool LoadStoreSignExtendedByteHalfword();

	bool PCRelativeLoad();

	bool HiRegisterOperationsBranchExchange();

	bool ALUOperations();

	bool MoveCompareAddSubtractImmediate();

	bool AddSubtract();

	bool MoveShiftedRegister();

	bool BranchAndBranchExchange();

	bool BlockDataTransfer();

	bool BranchAndBranchLink();

	bool SoftwareInterrupt();

	bool Undefined();

	bool SingleDataTransfer();

	bool SingleDataSwap();

	bool MultiplyAndMultiplyAccumulate();

	bool HalfWordDataTransfer();

	bool psrTransfer();

	bool DataProcessing();

	void skylersalehLogProcess();

	void runCPUPipeline();

	void dumpCpuStateToConsole();

	void unimplementedInstruction();
#pragma endregion ARM7TDMI_DEFINITIONS

#pragma region EMULATION_DEFINITIONS

private:

	MASQ_INLINE void cpuTick(TICK_TYPE type = TICK_TYPE::CPU_TICK)
	{
		if (type == TICK_TYPE::DMA_TICK)
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter++;
		}
		else
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter++;
		}

		syncOtherGBAModuleTicks();
	}

	MASQ_INLINE void syncOtherGBAModuleTicks()
	{
		timerTick();
		serialTick();
		apuTick();
		ppuTick();
		processBackup();

#if (GBA_ENABLE_DELAYED_MMIO_WRITE == YES)

		if (pGBA_instance->GBA_state.interrupt.syncDelay > RESET)
		{
			--pGBA_instance->GBA_state.interrupt.syncDelay;
		}

		if (pGBA_instance->GBA_state.irqPend == YES)
		{
			ifRegUpdate(pGBA_instance->GBA_state.interrupt.irqQ);
			pGBA_instance->GBA_state.irqPend = NO;
			pGBA_instance->GBA_state.interrupt.syncDelay = RESET;
		}

		if (pGBA_instance->GBA_state.timerPendMap != RESET) MASQ_UNLIKELY
		{
			for (uint8_t timerID = ZERO; timerID < FOUR; timerID++)
			{
				/* bits (CNT_L | CNT_H) for this timer */
				uint8_t mask = (uint8_t)(THREE << (timerID << ONE)); // 0x03,0x0C,0x30,0xC0
				/* nothing pending for this timer */
				if ((pGBA_instance->GBA_state.timerPendMap & mask) == ZERO)
				{
					CONTINUE;
				}
				uint8_t bitL = (timerID << ONE);       // 0,2,4,6
				uint8_t bitH = bitL + ONE;             // 1,3,5,7
				if (GETBIT(bitL, pGBA_instance->GBA_state.timerPendMap))
				{
					cntlRegUpdate(
						(TIMER)timerID,
						pGBA_instance->GBA_state.timer[timerID]
						.cache.io_tmxcnt_l
					);
				}
				if (GETBIT(bitH, pGBA_instance->GBA_state.timerPendMap))
				{
					cnthRegUpdate(
						(TIMER)timerID,
						pGBA_instance->GBA_state.timer[timerID]
						.cache.io_tmxcnt_h
					);
				}
			}

		/* clear after processing */
		pGBA_instance->GBA_state.timerPendMap = RESET;
		}
#endif
	}

	void timerTick();

	void dmaTick();

	void serialTick();

	void apuTick();

	void ppuTick();

private:

	void requestInterrupts(GBA_INTERRUPT interrupt);

	bool shouldUnHaltTheCPU();

	bool isInterruptReadyToBeServed();

	bool handleInterruptsIfApplicable();

private:

	void handleKeypadInterrupts();

	void captureIO();

private:

	MASQ_INLINE void setTimerCNTLRegister(TIMER timer, uint16_t value)
	{
		// Use array lookup instead of switch for better performance
		static uint16_t* CNTL_LUT[] = {
			&pGBA_peripherals->mTIMER0CNT_L,
			&pGBA_peripherals->mTIMER1CNT_L,
			&pGBA_peripherals->mTIMER2CNT_L,
			&pGBA_peripherals->mTIMER3CNT_L
		};

		const uint8_t timerIdx = TO_UINT8(timer);

		if (timerIdx < 4) MASQ_LIKELY
		{
			*CNTL_LUT[timerIdx] = value;
		}
		else MASQ_UNLIKELY
		{
			FATAL("Unknown Timer : %d", timerIdx);
		}
	}

	MASQ_INLINE void timerCommonProcessing(TIMER timerID, uint16_t reloadValueIfOverflow,
		mTIMERnCNT_HHalfWord_t* CNTH, INC64 timerCycles)
	{
		const uint8_t timerIdx = TO_UINT(timerID);
		auto& timerState = pGBA_instance->GBA_state.timer[timerIdx];
		uint32_t timerValue = timerState.cache.counter;

		while (timerCycles != RESET)
		{
			++timerValue;

			if (timerValue > 0xFFFF) MASQ_UNLIKELY
			{
				timerState.overflow = YES;
				++timerState.cascadeEvents;

				// Reload timer
				timerValue = (timerValue - 0x10000) + reloadValueIfOverflow;

				// Request interrupt if enabled
				if (CNTH->mTIMERnCNT_HFields.TIMER_IRQ_EN == YES) MASQ_UNLIKELY
				{
					requestInterrupts((GBA_INTERRUPT)(timerIdx + TO_UINT(GBA_INTERRUPT::IRQ_TIMER0)));
				}

					// Handle FIFO audio for Timer 0 or Timer 1
					if ((timerIdx == 0 || timerIdx == 1)
						&& pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE) MASQ_UNLIKELY
					{
						// Process both FIFOs (unrolled)
						auto& fifoA = pGBA_audio->FIFO[DIRECT_SOUND_A];
						if (fifoA.timer == timerIdx)
						{
							if (fifoA.size > ZERO) MASQ_LIKELY
							{
								fifoA.latch = ((GBA_AUDIO_SAMPLE_TYPE)fifoA.fifo[fifoA.position] << ONE);
								fifoA.position = (fifoA.position + ONE) & THIRTYONE;
								fifoA.size--;
							}
							else
							{
								fifoA.latch = (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
							}

							if (fifoA.size < SIXTEEN) MASQ_UNLIKELY
							{
								auto& dmacntH = pGBA_peripherals->mDMA1CNT_H;
								if (dmacntH.mDMAnCNT_HFields.DMA_EN == SET
									&& dmacntH.mDMAnCNT_HFields.DMA_START_TIMING == DMA_TIMING::SPECIAL)
								{
									ActivateDMAChannel(DMA::DMA1);
								}
							}
						}

						auto& fifoB = pGBA_audio->FIFO[DIRECT_SOUND_B];
						if (fifoB.timer == timerIdx)
						{
							if (fifoB.size > ZERO) MASQ_LIKELY
							{
								fifoB.latch = ((GBA_AUDIO_SAMPLE_TYPE)fifoB.fifo[fifoB.position] << ONE);
								fifoB.position = (fifoB.position + ONE) & THIRTYONE;
								fifoB.size--;
							}
							else
							{
								fifoB.latch = (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
							}

							if (fifoB.size < SIXTEEN) MASQ_UNLIKELY
							{
								auto& dmacntH = pGBA_peripherals->mDMA2CNT_H;
								if (dmacntH.mDMAnCNT_HFields.DMA_EN == SET
									&& dmacntH.mDMAnCNT_HFields.DMA_START_TIMING == DMA_TIMING::SPECIAL)
								{
									ActivateDMAChannel(DMA::DMA2);
								}
							}
						}
					}
			}
			else MASQ_LIKELY
			{
				timerState.overflow = NO;
			}

			timerState.cache.counter = (uint16_t)timerValue;
			setTimerCNTLRegister(timerID, (uint16_t)timerValue);

			--timerCycles;
		}
	}

	void processTimer(INC64 timerCycles);

private:

	GBA_WORD getDMASADRegister(DMA dma);

	GBA_WORD getDMADADRegister(DMA dma);

	GBA_HALFWORD getDMACNTLRegister(DMA dma);

	mDMAnCNT_HHalfWord_t* getDMACNTHRegister(DMA dma);

	void setDMASADRegister(DMA dma, GBA_WORD data);

	void setDMADADRegister(DMA dma, GBA_WORD data);

	void setDMACNTLRegister(DMA dma, GBA_HALFWORD data);

	void latchDMARegisters(ID dmaID);

	void OnDMAChannelWritten(DMA dmaID, FLAG oldEnable, FLAG newEnable);

	void ActivateDMAChannel(ID dmaID);

	void RequestDMA(DMA_TIMING timing);

	void OnDMAActivated(ID dmaID);

	void SelectNextDMA();

	FLAG IsAnyDMARunning();

	void processDMA();

	void RunDMAChannel();

private:

	MASQ_INLINE void tickChannel(AUDIO_CHANNELS channel, INC64 tCycles)
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN != ONE) MASQ_UNLIKELY
			RETURN;

		const uint8_t ch = static_cast<uint8_t>(channel);
		auto& chInstance = pGBA_instance->GBA_state.audio.audioChannelInstance[ch];

		chInstance.frequencyTimer -= tCycles;

		if (chInstance.frequencyTimer > ZERO) MASQ_LIKELY
			RETURN;

		// Channel-specific processing
		if (ch == AUDIO_CHANNELS::CHANNEL_1) MASQ_LIKELY  // CHANNEL_1
		{
			const uint16_t resetFrequencyTimer = (2048 - pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.FREQ) * SIXTEEN;
			chInstance.frequencyTimer += resetFrequencyTimer;

			chInstance.waveDutyPosition = (chInstance.waveDutyPosition + 1) & 7;  // Wrap at 8

			pGBA_instance->GBA_state.audio.sampleReadByChannel1 =
				(chInstance.isChannelActuallyEnabled == ENABLED)
				? SQUARE_WAVE_AMPLITUDE[pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.WAVE_PATTERN_DUTY][chInstance.waveDutyPosition]
				: (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
		}
		else if (ch == AUDIO_CHANNELS::CHANNEL_2)  // CHANNEL_2
		{
			const uint16_t resetFrequencyTimer = (2048 - pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.FREQ) * SIXTEEN;
			chInstance.frequencyTimer += resetFrequencyTimer;

			chInstance.waveDutyPosition = (chInstance.waveDutyPosition + 1) & 7;

			pGBA_instance->GBA_state.audio.sampleReadByChannel2 =
				(chInstance.isChannelActuallyEnabled == ENABLED)
				? SQUARE_WAVE_AMPLITUDE[pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.WAVE_PATTERN_DUTY][chInstance.waveDutyPosition]
				: (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
		}
		else if (ch == AUDIO_CHANNELS::CHANNEL_3)  // CHANNEL_3
		{
			pGBA_instance->GBA_state.audio.didChannel3ReadWaveRamPostTrigger = YES;

			const uint16_t resetFrequencyTimer = (2048 - pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.SAMPLE_RATE) * EIGHT;
			chInstance.frequencyTimer += resetFrequencyTimer;

			auto& waveIndex = pGBA_instance->GBA_state.audio.waveRamCurrentIndex;
			waveIndex++;

			if (waveIndex >= THIRTYTWO) MASQ_UNLIKELY
			{
				waveIndex = ZERO;

				if (pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_DIMENSION == ONE) MASQ_UNLIKELY
				{
					pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_BANK_NUMBER ^= ONE;
					std::swap_ranges(std::begin(pGBA_peripherals->mWAVERAM8),
								   std::end(pGBA_peripherals->mWAVERAM8),
								   std::begin(pGBA_memory->mBankedWAVERAM.mWAVERAM8));
				}
			}

				if (chInstance.isChannelActuallyEnabled == ENABLED) MASQ_LIKELY
				{
					const uint8_t byteIndex = waveIndex >> 1;  // Divide by 2
					const bool isUpperNibble = (waveIndex & 1) == 0;

					pGBA_instance->GBA_state.audio.sampleReadByChannel3 = isUpperNibble
						? pGBA_memory->mBankedWAVERAM.mWAVERAM8[byteIndex].samples.upperNibble
						: pGBA_memory->mBankedWAVERAM.mWAVERAM8[byteIndex].samples.lowerNibble;

					pGBA_instance->GBA_state.audio.sampleReadByChannel3 =
						(pGBA_instance->GBA_state.audio.sampleReadByChannel3 - EIGHT) * FOUR;
				}
				else
				{
					pGBA_instance->GBA_state.audio.sampleReadByChannel3 = (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
				}
		}
		else  // CHANNEL_4
		{
			const uint16_t divisor = AUDIO_CHANNEL_4_DIVISOR[pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.DIV_RATIO_OF_FREQ];
			const uint16_t shiftAmount = pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.SHIFT_CLK_FREQ;
			const uint16_t resetFrequencyTimer = divisor << (shiftAmount + TWO);

			chInstance.frequencyTimer += resetFrequencyTimer;

			uint16_t LFSR = chInstance.LFSR;
			const BYTE xorResult = (LFSR ^ (LFSR >> 1)) & 1;
			LFSR = (LFSR >> ONE) | (xorResult << FOURTEEN);

			if (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.COUNTER_STEP == LFSR_WIDTH_IS_7_BITS) MASQ_UNLIKELY
			{
				LFSR = (LFSR & ~(ONE << SIX)) | (xorResult << SIX);
			}

			chInstance.LFSR = LFSR;

			pGBA_instance->GBA_state.audio.sampleReadByChannel4 =
				(chInstance.isChannelActuallyEnabled == ENABLED)
				? ((~LFSR & 1) ? EIGHT : -EIGHT)
				: (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
		}
	}

	MASQ_INLINE DIM16 getChannelPeriod(AUDIO_CHANNELS channel)
	{
		const uint8_t ch = static_cast<uint8_t>(channel);

		if (ch == 0)
			RETURN pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.FREQ;
		else if (ch == 1)
			RETURN pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.FREQ;
		else if (ch == 2)
			RETURN pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.SAMPLE_RATE;

		RETURN ZERO;
	}

	MASQ_INLINE FLAG isDACEnabled(AUDIO_CHANNELS channel)
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN != ONE) MASQ_UNLIKELY
			RETURN NO;

		const uint8_t ch = static_cast<uint8_t>(channel);

		if (ch == 2)
			RETURN (pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.CHANNEL_3_OFF == ONE) ? YES : NO;
		else if (ch == 0)
			RETURN ((pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord & 0xF800) != ZERO) ? YES : NO;
		else if (ch == 1)
			RETURN ((pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord & 0xF800) != ZERO) ? YES : NO;
		else if (ch == 3)
			RETURN ((pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord & 0xF800) != ZERO) ? YES : NO;

		RETURN NO;
	}

	MASQ_INLINE FLAG isAudioChannelEnabled(AUDIO_CHANNELS channel)
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN != ONE) MASQ_UNLIKELY
			RETURN NO;

		const uint8_t ch = static_cast<uint8_t>(channel);
		RETURN (pGBA_instance->GBA_state.audio.audioChannelInstance[ch].isChannelActuallyEnabled == ENABLED) ? YES : NO;
	}

	MASQ_INLINE FLAG enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS channel)
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN != ONE) MASQ_UNLIKELY
			RETURN NO;

		const uint8_t ch = static_cast<uint8_t>(channel);
		const FLAG dacEnabled = isDACEnabled(channel);

		pGBA_instance->GBA_state.audio.audioChannelInstance[ch].isChannelActuallyEnabled = dacEnabled ? ENABLED : DISABLED;

		// Update SOUNDCNT_X flags using array indexing
		uint8_t* soundFlags = reinterpret_cast<uint8_t*>(&pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XHalfWord);

		if (dacEnabled) MASQ_LIKELY
		{
			*soundFlags |= (1 << ch);  // Set bit 0, 1, 2, or 3
		}
		else
		{
			*soundFlags &= ~(1 << ch);  // Clear bit 0, 1, 2, or 3

			// Clear INITIAL flag for each channel
			if (ch == 0)
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.INITIAL = ZERO;
			else if (ch == 1)
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.INITIAL = ZERO;
			else if (ch == 2)
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.INITIAL = ZERO;
			else if (ch == 3)
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.INITIAL = ZERO;
		}

		RETURN dacEnabled;
	}

	MASQ_INLINE void continousDACCheck()
	{
		// Unrolled - no loop overhead
		if ((pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord & 0xF800) == ZERO) MASQ_UNLIKELY
		{
			pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = RESET;
			pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
		}

		if ((pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord & 0xF800) == ZERO) MASQ_UNLIKELY
		{
			pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = RESET;
			pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
		}

		if (pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.CHANNEL_3_OFF == ZERO) MASQ_UNLIKELY
		{
			pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = RESET;
			pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
		}

		if ((pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord & 0xF800) == ZERO) MASQ_UNLIKELY
		{
			pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = RESET;
			pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
		}
	}

	MASQ_INLINE FLAG isChannel3Active()
	{
		RETURN (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG == ONE
			&& pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.INITIAL == ONE
			&& pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled == ENABLED) ? YES : NO;
	}

	MASQ_INLINE void processSoundLength()
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN != ONE) MASQ_UNLIKELY
			RETURN;

		// Channel 1 - unrolled for performance
		if (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.LENGTH_FLAG == ONE
			&& pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer > ZERO) MASQ_LIKELY
		{
			if (--pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer == ZERO) MASQ_UNLIKELY
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = RESET;
				pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
			}
		}

		// Channel 2
		if (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.LENGTH_FLAG == ONE
			&& pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer > ZERO) MASQ_LIKELY
		{
			if (--pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer == ZERO) MASQ_UNLIKELY
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = RESET;
				pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
			}
		}

		// Channel 3
		if (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.LENGTH_FLAG == ONE
			&& pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer > ZERO) MASQ_LIKELY
		{
			if (--pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer == ZERO) MASQ_UNLIKELY
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = RESET;
				pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
			}
		}

		// Channel 4
		if (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.LENGTH_FLAG == ONE
			&& pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer > ZERO) MASQ_LIKELY
		{
			if (--pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer == ZERO) MASQ_UNLIKELY
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = RESET;
				pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
			}
		}
	}

	MASQ_INLINE SDIM32 getUpdatedFrequency()
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN != ONE) MASQ_UNLIKELY
			RETURN ZERO;

		SDIM32 newFrequency = pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].shadowFrequency
			>> pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_SHIFT;

		if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_FREQ_DIR == ZERO)
		{
			newFrequency = pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].shadowFrequency + newFrequency;
		}
		else
		{
			pGBA_instance->GBA_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger = YES;
			newFrequency = pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].shadowFrequency - newFrequency;
		}

		if (newFrequency > 2047) MASQ_UNLIKELY
		{
			pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.INITIAL = RESET;
			pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = RESET;
			pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
		}

		RETURN newFrequency;
	}

	MASQ_INLINE void processFrequencySweep()
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN != ONE) MASQ_UNLIKELY
			RETURN;

		auto& ch1 = pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1];

		if (ch1.sweepTimer > ZERO)
		{
			--ch1.sweepTimer;
		}

		if (ch1.sweepTimer == ZERO) MASQ_UNLIKELY
		{
			// Reload the sweep timer
			ch1.sweepTimer = (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME > ZERO)
				? pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME
				: EIGHT;

		// Update frequency sweep
		if (ch1.sweepEnabled == ENABLED
			&& pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME > ZERO)
		{
			int32_t newFrequency = getUpdatedFrequency();

			if (newFrequency <= 2047 && pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_SHIFT > ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.FREQ = newFrequency & 0x7FF;
				ch1.shadowFrequency = newFrequency;
				performOverFlowCheck();
			}
		}
		}
	}

	MASQ_INLINE void processEnvelopeSweep()
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN != ONE) MASQ_UNLIKELY
			RETURN;

		// Channel 1 envelope
		if (pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_STEP_TIME != ZERO) MASQ_LIKELY
		{
			auto& ch1 = pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1];

			if (ch1.envelopePeriodTimer > ZERO && --ch1.envelopePeriodTimer == ZERO)
			{
				ch1.envelopePeriodTimer = pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_STEP_TIME;

				if (ch1.currentVolume > 0x00 && pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_DIR == ZERO)
				{
					--ch1.currentVolume;
				}
				else if (ch1.currentVolume < 0x0F && pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_DIR == ONE)
				{
					++ch1.currentVolume;
				}

				if (ch1.currentVolume == 0x0F || ch1.currentVolume == 0x00)
				{
					ch1.isVolumeEnvelopeStillDoingAutomaticUpdates = NO;
				}
			}
		}

		// Channel 2 envelope
		if (pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_STEP_TIME != ZERO) MASQ_LIKELY
		{
			auto& ch2 = pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2];

			if (ch2.envelopePeriodTimer > ZERO && --ch2.envelopePeriodTimer == ZERO)
			{
				ch2.envelopePeriodTimer = pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_STEP_TIME;

				if (ch2.currentVolume > 0x00 && pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_DIR == ZERO)
				{
					--ch2.currentVolume;
				}
				else if (ch2.currentVolume < 0x0F && pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_DIR == ONE)
				{
					++ch2.currentVolume;
				}

				if (ch2.currentVolume == 0x0F || ch2.currentVolume == 0x00)
				{
					ch2.isVolumeEnvelopeStillDoingAutomaticUpdates = NO;
				}
			}
		}

		// Channel 4 envelope
		if (pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_STEP_TIME != ZERO) MASQ_LIKELY
		{
			auto& ch4 = pGBA_instance->GBA_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4];

			if (ch4.envelopePeriodTimer > ZERO && --ch4.envelopePeriodTimer == ZERO)
			{
				ch4.envelopePeriodTimer = pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_STEP_TIME;

				if (ch4.currentVolume > 0x00 && pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_DIR == ZERO)
				{
					--ch4.currentVolume;
				}
				else if (ch4.currentVolume < 0x0F && pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_DIR == ONE)
				{
					++ch4.currentVolume;
				}

				if (ch4.currentVolume == 0x0F || ch4.currentVolume == 0x00)
				{
					ch4.isVolumeEnvelopeStillDoingAutomaticUpdates = NO;
				}
			}
		}
	}

	MASQ_INLINE GBA_AUDIO_SAMPLE_TYPE getAmplitude(AUDIO_CHANNELS channel)
	{
		const uint8_t ch = static_cast<uint8_t>(channel);

		// Direct array access - no switch
		static GBA_AUDIO_SAMPLE_TYPE const amplitudes[4] = {
			pGBA_instance->GBA_state.audio.sampleReadByChannel1,
			pGBA_instance->GBA_state.audio.sampleReadByChannel2,
			pGBA_instance->GBA_state.audio.sampleReadByChannel3,
			pGBA_instance->GBA_state.audio.sampleReadByChannel4
		};

		RETURN amplitudes[ch];
	}

	MASQ_INLINE GBA_AUDIO_SAMPLE_TYPE getDACOutput(AUDIO_CHANNELS channel)
	{
		if (isDACEnabled(channel) != YES) MASQ_UNLIKELY
			RETURN (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;

		const GBA_AUDIO_SAMPLE_TYPE amplitude = getAmplitude(channel);

		if (channel == AUDIO_CHANNELS::CHANNEL_3) MASQ_UNLIKELY
		{
			RETURN amplitude * (BYTE)pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift;
		}

		RETURN amplitude * (BYTE)pGBA_instance->GBA_state.audio.audioChannelInstance[static_cast<uint8_t>(channel)].currentVolume;
	}

	MASQ_INLINE void captureDownsampledAudioSamples(INC64 sampleCount)
	{
		static constexpr int PSG_VOL[4] = { ONE, TWO, FOUR, ZERO };
		static constexpr int DMA_VOL[2] = { TWO, FOUR };
		static constexpr GBA_AUDIO_SAMPLE_TYPE DOWNSAMPLE_THRESHOLD = CEIL(GBA_REFERENCE_CLOCK_HZ / EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA);

		pGBA_instance->GBA_state.audio.downSamplingRatioCounter += sampleCount;

		if (pGBA_instance->GBA_state.audio.downSamplingRatioCounter < DOWNSAMPLE_THRESHOLD) MASQ_LIKELY
			RETURN;

		pGBA_instance->GBA_state.audio.downSamplingRatioCounter -= DOWNSAMPLE_THRESHOLD;

		// Cache commonly accessed registers
		const auto& soundcntL = pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields;
		const auto& soundcntH = pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields;
		const GBA_AUDIO_SAMPLE_TYPE biasLevel = pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASFields.BIAS_LVL;
		const int psgVolShift = FIVE - PSG_VOL[soundcntH.SOUND_VOL];

		// Get all channel samples once
		const GBA_AUDIO_SAMPLE_TYPE channel1Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_1);
		const GBA_AUDIO_SAMPLE_TYPE channel2Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_2);
		const GBA_AUDIO_SAMPLE_TYPE channel3Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_3);
		const GBA_AUDIO_SAMPLE_TYPE channel4Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_4);

		// Process LEFT sample
		GBA_AUDIO_SAMPLE_TYPE leftSample = static_cast<GBA_AUDIO_SAMPLE_TYPE>(MUTE_AUDIO);

		if (DISABLE_FIRST_PULSE_CHANNEL == NO && soundcntL.SOUND_ENABLE_1L == ONE) MASQ_LIKELY
			leftSample += channel1Sample;
		if (DISABLE_SECOND_PULSE_CHANNEL == NO && soundcntL.SOUND_ENABLE_2L == ONE) MASQ_LIKELY
			leftSample += channel2Sample;
		if (DISABLE_WAVE_CHANNEL == NO && soundcntL.SOUND_ENABLE_3L == ONE) MASQ_LIKELY
			leftSample += channel3Sample;
		if (DISABLE_NOISE_CHANNEL == NO && soundcntL.SOUND_ENABLE_4L == ONE) MASQ_LIKELY
			leftSample += channel4Sample;

		leftSample *= soundcntL.SOUND_MASTER_VOL_L;
		leftSample >>= psgVolShift;

		if (DISABLE_DMAA_CHANNEL == NO && soundcntH.DMA_SOUND_A_EN_L == SET) MASQ_LIKELY
			leftSample += (pGBA_audio->FIFO[DIRECT_SOUND_A].latch << DMA_VOL[soundcntH.DMA_SOUND_A_VOL]);

		if (DISABLE_DMAB_CHANNEL == NO && soundcntH.DMA_SOUND_B_EN_L == SET) MASQ_LIKELY
			leftSample += (pGBA_audio->FIFO[DIRECT_SOUND_B].latch << DMA_VOL[soundcntH.DMA_SOUND_B_VOL]);

		leftSample += biasLevel;
		leftSample = std::clamp(leftSample, GBA_AUDIO_SAMPLE_TYPE(0), GBA_AUDIO_SAMPLE_TYPE(0x3FF));
		leftSample -= biasLevel;
		leftSample *= THIRTYTWO;

		// Process RIGHT sample
		GBA_AUDIO_SAMPLE_TYPE rightSample = static_cast<GBA_AUDIO_SAMPLE_TYPE>(MUTE_AUDIO);

		if (DISABLE_FIRST_PULSE_CHANNEL == NO && soundcntL.SOUND_ENABLE_1R == ONE) MASQ_LIKELY
			rightSample += channel1Sample;
		if (DISABLE_SECOND_PULSE_CHANNEL == NO && soundcntL.SOUND_ENABLE_2R == ONE) MASQ_LIKELY
			rightSample += channel2Sample;
		if (DISABLE_WAVE_CHANNEL == NO && soundcntL.SOUND_ENABLE_3R == ONE) MASQ_LIKELY
			rightSample += channel3Sample;
		if (DISABLE_NOISE_CHANNEL == NO && soundcntL.SOUND_ENABLE_4R == ONE) MASQ_LIKELY
			rightSample += channel4Sample;

		rightSample *= soundcntL.SOUND_MASTER_VOL_R;
		rightSample >>= psgVolShift;

		if (DISABLE_DMAA_CHANNEL == NO && soundcntH.DMA_SOUND_A_EN_R == SET) MASQ_LIKELY
			rightSample += (pGBA_audio->FIFO[DIRECT_SOUND_A].latch << DMA_VOL[soundcntH.DMA_SOUND_A_VOL]);

		if (DISABLE_DMAB_CHANNEL == NO && soundcntH.DMA_SOUND_B_EN_R == SET) MASQ_LIKELY
			rightSample += (pGBA_audio->FIFO[DIRECT_SOUND_B].latch << DMA_VOL[soundcntH.DMA_SOUND_B_VOL]);

		rightSample += biasLevel;
		rightSample = std::clamp(rightSample, GBA_AUDIO_SAMPLE_TYPE(0), GBA_AUDIO_SAMPLE_TYPE(0x3FF));
		rightSample -= biasLevel;
		rightSample *= THIRTYTWO;

		// Buffer management
		auto& accumulatedTone = pGBA_instance->GBA_state.audio.accumulatedTone;

		if (accumulatedTone >= AUDIO_BUFFER_SIZE_FOR_GBA) MASQ_UNLIKELY
		{
			// Volume control (move to separate function if profiler shows this is hot)
			if (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) == YES) MASQ_UNLIKELY
			{
				auto gain = std::clamp(getEmulationVolume() + 0.05f, 0.0001f, 0.9998f);
				setEmulationVolume(gain);
			}
			if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) == YES) MASQ_UNLIKELY
			{
				auto gain = std::clamp(getEmulationVolume() - 0.05f, 0.0001f, 0.9998f);
				setEmulationVolume(gain);
			}

			if (SDL_PutAudioStreamData(audioStream, pGBA_instance->GBA_state.audio.audioBuffer,
										sizeof(pGBA_instance->GBA_state.audio.audioBuffer)) == FAILURE) MASQ_UNLIKELY
			{
				SDL_Log("Could not put data on Audio stream, %s", SDL_GetError());
				FATAL("SDL_PutAudioStreamData Error");
			}

			accumulatedTone = RESET;
		}
		else MASQ_LIKELY
		{
			pGBA_instance->GBA_state.audio.audioBuffer[accumulatedTone++] = leftSample;
			if (accumulatedTone < AUDIO_BUFFER_SIZE_FOR_GBA) MASQ_LIKELY
			{
				pGBA_instance->GBA_state.audio.audioBuffer[accumulatedTone++] = rightSample;
			}
		}
	}

	void processAPU(INC64 apuCycles);

	void playTheAudioFrame();

private:

	// ============================================
	// PIXEL AND WINDOW OPERATIONS
	// ============================================

	MASQ_INLINE void RESET_PIXEL(uint32_t x, uint32_t y)
	{

		pGBA_display->gfx_obj[x][y] = ZERO;
		pGBA_display->objPriority[x][y] = DEFAULT_OBJ_PRIORITY;
		pGBA_display->gfx_obj_window[x][y] = DISABLED;
		pGBA_display->gfx_obj_mode[x][y] = OBJECT_MODE::NORMAL;
		for (ID bgID = BG0; bgID < BG3; bgID++)
		{
			pGBA_display->gfx_bg[bgID][x][y] = ZERO;
		}

	}

	MASQ_INLINE FLAG GET_WINDOW_OUTPUT(uint32_t x, uint32_t y, FLAG win0in, FLAG win1in, FLAG winout, FLAG objin)
	{


#if (DEACTIVATED)
		auto isWin = [&](uint32_t x, uint32_t y, uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2)
			{
				if (x2 > getScreenWidth() || x1 > x2)
				{
					x2 = getScreenWidth();
				}
				if (y2 > getScreenHeight() || y1 > y2)
				{
					y2 = getScreenHeight();
				}

				RETURN(x >= x1 && x <= x2) && (y >= y1 && y <= y2);
			};

		auto isWin0 = [&](uint32_t x, uint32_t y)
			{
				RETURN isWin(
					x
					, y
					, pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i1
					, pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i2
					, pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i1
					, pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i2
				);
			};

		auto isWin1 = [&](uint32_t x, uint32_t y)
			{
				RETURN isWin(
					x
					, y
					, pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i1
					, pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i2
					, pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i1
					, pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i2
				);
			};
#endif

		FLAG is_win0in = pGBA_display->gfx_window[WIN0][x][y];
		FLAG is_win1in = pGBA_display->gfx_window[WIN1][x][y];
		FLAG is_winobj = pGBA_display->gfx_obj_window[x][y];

		FLAG win0_display = pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.WIN0_DISP_FLAG == SET;
		FLAG win1_display = pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.WIN1_DISP_FLAG == SET;
		FLAG winobj_display = (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.OBJ_DISP_FLAG == SET && pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.SCREEN_DISP_OBJ == SET);

		// Atleast one window should be enabled for winout to be enabled
		// Now, if any of the win0, win1 or objwin condition fails (provided one of those windows were enabled), then we enter winout condition
		FLAG winout_display = win0_display || win1_display || winobj_display;

		if (win0_display && is_win0in)
		{
			RETURN win0in;
		}
		else if (win1_display && is_win1in)
		{
			RETURN win1in;
		}
		else if (winobj_display && is_winobj)
		{
			RETURN objin;
		}
		else if (winout_display)
		{
			RETURN winout;
		}

		RETURN ENABLED;

	}

	MASQ_INLINE void HANDLE_WINDOW_FOR_BG(uint32_t x, uint32_t y, ID bgID)
	{

		if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		FLAG isWindowAllowingBG = YES;

		FLAG win0in = CLEAR;
		FLAG win1in = CLEAR;
		FLAG winout = CLEAR;
		FLAG objin = CLEAR;

		switch (bgID)
		{
		case BG0:
		{
			win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_0_EN == SET;
			win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_0_EN == SET;
			winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_0_EN == SET;
			objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_0_EN == SET;
			BREAK;
		}
		case BG1:
		{
			win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_1_EN == SET;
			win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_1_EN == SET;
			winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_1_EN == SET;
			objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_1_EN == SET;
			BREAK;
		}
		case BG2:
		{
			win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_2_EN == SET;
			win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_2_EN == SET;
			winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_2_EN == SET;
			objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_2_EN == SET;
			BREAK;
		}
		case BG3:
		{
			win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_3_EN == SET;
			win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_3_EN == SET;
			winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_3_EN == SET;
			objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_3_EN == SET;
			BREAK;
		}
		default:
		{
			FATAL("Unknown BG Layer");
			RETURN;
		}
		}

		isWindowAllowingBG = GET_WINDOW_OUTPUT(
			x
			, y
			, win0in
			, win1in
			, winout
			, objin
		);

		if (isWindowAllowingBG == NO)
		{
			pGBA_display->gfx_bg[bgID][x][y] = ZERO;
		}

	}

	MASQ_INLINE void HANDLE_WINDOW_FOR_OBJ(uint32_t x, uint32_t y)
	{

		if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		FLAG isWindowAllowingObj = YES;

		FLAG win0in = CLEAR;
		FLAG win1in = CLEAR;
		FLAG winout = CLEAR;
		FLAG objin = CLEAR;

		win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_OBJ_EN == SET;
		win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_OBJ_EN == SET;
		winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_OBJ_EN == SET;
		objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_OBJ_EN == SET;

		isWindowAllowingObj = GET_WINDOW_OUTPUT(
			x
			, y
			, win0in
			, win1in
			, winout
			, objin
		);

		if (isWindowAllowingObj == NO)
		{
			pGBA_display->gfx_obj[x][y] = ZERO;
		}

	}

	MASQ_INLINE FLAG DOES_WINDOW_ALLOW_BLENDING(uint32_t x, uint32_t y)
	{

		FLAG isBlendingAllowed = NO;

		if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN isBlendingAllowed;
		}

		FLAG win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_COLOR_SPL_EFFECT == SET;
		FLAG win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_COLOR_SPL_EFFECT == SET;
		FLAG winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_COLOR_SPL_EFFECT == SET;
		FLAG objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_COLOR_SPL_EFFECT == SET;

		isBlendingAllowed = GET_WINDOW_OUTPUT(
			x
			, y
			, win0in
			, win1in
			, winout
			, objin
		);

		RETURN isBlendingAllowed;

	}

	// ============================================
	// COLOR BLENDING OPERATIONS
	// ============================================

	MASQ_INLINE gbaColor_t BLEND(gbaColor_t layer1Pixel, gbaColor_t layer2Pixel, BYTE eva, BYTE evb)
	{

		gbaColor_t finalPixel = layer1Pixel;

		const int r_a = (layer1Pixel.raw >> 0) & 31;
		const int g_a = ((layer1Pixel.raw >> 4) & 62) | (layer1Pixel.raw >> 15);
		const int b_a = (layer1Pixel.raw >> 10) & 31;

		const int r_b = (layer2Pixel.raw >> 0) & 31;
		const int g_b = ((layer2Pixel.raw >> 4) & 62) | (layer2Pixel.raw >> 15);
		const int b_b = (layer2Pixel.raw >> 10) & 31;

		eva = std::min<int>(16, eva);
		evb = std::min<int>(16, evb);

		const int r = std::min<uint8_t>((r_a * eva + r_b * evb + 8) >> 4, 31);
		const int g = std::min<uint8_t>((g_a * eva + g_b * evb + 8) >> 4, 63) >> 1;
		const int b = std::min<uint8_t>((b_a * eva + b_b * evb + 8) >> 4, 31);

		finalPixel.RED = r;
		finalPixel.GREEN = g;
		finalPixel.BLUE = b;

		RETURN finalPixel;

	}

	MASQ_INLINE gbaColor_t BRIGHTEN(gbaColor_t color, BYTE evy)
	{

		gbaColor_t finalPixel = color;

		evy = std::min<int>(16, evy);

		int r = (color.raw >> 0) & 31;
		int g = ((color.raw >> 4) & 62) | (color.raw >> 15);
		int b = (color.raw >> 10) & 31;

		r += ((31 - r) * evy + 8) >> 4;
		g += ((63 - g) * evy + 8) >> 4;
		b += ((31 - b) * evy + 8) >> 4;

		g >>= 1;

		finalPixel.RED = r;
		finalPixel.GREEN = g;
		finalPixel.BLUE = b;

		RETURN finalPixel;

	}

	MASQ_INLINE gbaColor_t DARKEN(gbaColor_t color, BYTE evy)
	{

		gbaColor_t finalPixel = color;

		evy = std::min<int>(16, evy);

		int r = (color.raw >> 0) & 31;
		int g = ((color.raw >> 4) & 62) | (color.raw >> 15);
		int b = (color.raw >> 10) & 31;

		r -= (r * evy + 7) >> 4;
		g -= (g * evy + 7) >> 4;
		b -= (b * evy + 7) >> 4;

		g >>= 1;

		finalPixel.RED = r;
		finalPixel.GREEN = g;
		finalPixel.BLUE = b;

		RETURN finalPixel;

	}

	// ============================================
	// MERGE AND DISPLAY
	// ============================================

	MASQ_INLINE mBGnCNTHalfWord_t* getBGxCNT(ID bgID)
	{
		switch (bgID)
		{
		case BG0: RETURN& pGBA_peripherals->mBG0CNTHalfWord;
		case BG1: RETURN& pGBA_peripherals->mBG1CNTHalfWord;
		case BG2: RETURN& pGBA_peripherals->mBG2CNTHalfWord;
		case BG3: RETURN& pGBA_peripherals->mBG3CNTHalfWord;
		}
		RETURN nullptr;
	}

	MASQ_INLINE void MERGE_AND_DISPLAY_PHASE1()
	{

		// Refer https://nba-emu.github.io/hw-docs/ppu/composite.html

		uint32_t x = pGBA_display->currentMergePixel;
		uint32_t y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		pGBA_display->mergeCache.xCoordinate = x;
		pGBA_display->mergeCache.yCoordinate = y;

		if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES))) MASQ_UNLIKELY
		{
			; // Do nothing...
		}
		else
		{
			pGBA_display->layersForBlending[ZERO] = BD;
			pGBA_display->layersForBlending[ONE] = BD;
			pGBA_display->colorNumberForBlending[ZERO].colorNumber = ZERO;
			pGBA_display->colorNumberForBlending[ONE].colorNumber = ZERO;
			pGBA_display->colorNumberForBlending[ZERO].isObject = NO;
			pGBA_display->colorNumberForBlending[ONE].isObject = NO;
			pGBA_display->priorities[ZERO] = THREE; // Set to lowest priority by default
			pGBA_display->priorities[ONE] = THREE; // Set to lowest priority by default

			if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == SET) MASQ_UNLIKELY
			{
				pGBA_display->colorForBlending[ZERO] = GBA_WHITE;
			}
			else MASQ_LIKELY
			{
				ID minBG = MIN_MAX_BG_LAYERS[pGBA_display->currentPPUMode][ZERO];
				ID maxBG = MIN_MAX_BG_LAYERS[pGBA_display->currentPPUMode][ONE];

				// Check if window layer is allowing the bg, if not, marks the pixel as transparent
				for (ID bgID = minBG; bgID <= maxBG; bgID++)
				{
					HANDLE_WINDOW_FOR_BG(x, y, bgID);
				}
				// Check if window layer is allowing the obj, if not, marks the pixel as transparent
				HANDLE_WINDOW_FOR_OBJ(x, y);

				INC8 numberOfBGs = ZERO;
				for (INC8 priority = ZERO; priority < FOUR; priority++)
				{
					for (ID bgID = minBG; bgID <= maxBG; bgID++)
					{
						if (getBGxCNT(bgID) != NULL)
						{
							if (getBGxCNT(bgID)->mBGnCNTFields.BG_PRIORITY == priority
								&& (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTHalfWord & (0x100 << bgID)))
							{
								pGBA_display->bgListAccToPriority[numberOfBGs++] = bgID;
							}
						}
					}
				}

				INC8 bgListIndexer = ZERO; // This indexer had to be outside so that we get the second target layer
				for (INC8 targetLayer = ZERO; targetLayer < TWO; targetLayer++)
				{
					// Tranversing from highest priority to lowest priority
					while (bgListIndexer < numberOfBGs)
					{
						ID bgID = pGBA_display->bgListAccToPriority[bgListIndexer];

						++bgListIndexer;

						if (pGBA_display->gfx_bg[bgID][x][y] != ZERO)
						{
							pGBA_display->layersForBlending[targetLayer] = bgID;
							pGBA_display->colorNumberForBlending[targetLayer].colorNumber = pGBA_display->gfx_bg[bgID][x][y];
							pGBA_display->colorNumberForBlending[targetLayer].isObject = NO;
							// Priority needs to be maintained so that we can check against OBJs
							if (getBGxCNT(bgID) != NULL)
							{
								pGBA_display->priorities[targetLayer] = getBGxCNT(bgID)->mBGnCNTFields.BG_PRIORITY;
							}
							// Need to break, else we will never get the second target layer
							BREAK;
						}
					}
				}

				if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.SCREEN_DISP_OBJ == SET
					&& pGBA_display->gfx_obj[x][y] != RESET)
				{
					// If object pixel has higher or equal priority to current target layer 1
					if (pGBA_display->objPriority[x][y] <= pGBA_display->priorities[ZERO])
					{
						// Set OBJ as target layer 1
						// But before this, we need to set target layer 2 as the new target layer 1
						pGBA_display->layersForBlending[ONE] = pGBA_display->layersForBlending[ZERO];
						pGBA_display->colorNumberForBlending[ONE].colorNumber = pGBA_display->colorNumberForBlending[ZERO].colorNumber;
						pGBA_display->colorNumberForBlending[ONE].isObject = pGBA_display->colorNumberForBlending[ZERO].isObject;
						pGBA_display->layersForBlending[ZERO] = OBJ;
						pGBA_display->colorNumberForBlending[ZERO].colorNumber = pGBA_display->gfx_obj[x][y];
						pGBA_display->colorNumberForBlending[ZERO].isObject = YES;
					}
					// If object pixel has higher or equal priority to current target layer 2
					else if (pGBA_display->objPriority[x][y] <= pGBA_display->priorities[ONE])
					{
						pGBA_display->layersForBlending[ONE] = OBJ;
						pGBA_display->colorNumberForBlending[ONE].colorNumber = pGBA_display->gfx_obj[x][y];
						pGBA_display->colorNumberForBlending[ONE].isObject = YES;
					}
				}

				if ((pGBA_display->colorNumberForBlending[ZERO].colorNumber & IS_COLOR_NOT_PALETTE) == 0)
				{
					ID paletteIndex = pGBA_display->colorNumberForBlending[ZERO].colorNumber;
					GBA_WORD addressInPaletteRAM = PALETTE_RAM_START_ADDRESS + paletteIndex;
					if (pGBA_display->colorNumberForBlending[ZERO].isObject == YES)
					{
						addressInPaletteRAM += 0x200;
					}
					GBA_HALFWORD paletteData = readRawMemory<GBA_HALFWORD>(addressInPaletteRAM, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
					pGBA_display->colorForBlending[ZERO].raw = paletteData;
				}
				// Mode 3 or Mode 5
				else
				{
					pGBA_display->colorForBlending[ZERO].raw = pGBA_display->colorNumberForBlending[ZERO].colorNumber;
				}
			}
		}

	}

	MASQ_INLINE void MERGE_AND_DISPLAY_PHASE2()
	{

		// Refer https://nba-emu.github.io/hw-docs/ppu/composite.html

		uint32_t x = pGBA_display->mergeCache.xCoordinate;
		uint32_t y = pGBA_display->mergeCache.yCoordinate;

		if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES))) MASQ_UNLIKELY
		{
			; // Do nothing...
		}
		else
		{
			gbaColor_t finalPixel = { ZERO };

			if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == SET) MASQ_UNLIKELY
			{
				finalPixel = GBA_WHITE;
			}
			else MASQ_LIKELY
			{
				if (pGBA_display->gfx_obj_mode[x][y] == OBJECT_MODE::ALPHA_BLENDING
					// No need to check BLDCNT for OBJ if alpha blending is set in OAM (Refer : http://problemkaputt.de/gbatek-lcd-i-o-color-special-effects.htm)
					&& pGBA_display->layersForBlending[ZERO] == OBJ
					// Check BLDCNT for target layer 2
					&& (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0100 << pGBA_display->layersForBlending[ONE])))
				{
					if ((pGBA_display->colorNumberForBlending[ONE].colorNumber & IS_COLOR_NOT_PALETTE) == 0)
					{
						ID paletteIndex = pGBA_display->colorNumberForBlending[ONE].colorNumber;
						GBA_WORD addressInPaletteRAM = PALETTE_RAM_START_ADDRESS + paletteIndex;
						if (pGBA_display->colorNumberForBlending[ONE].isObject == YES)
						{
							addressInPaletteRAM += 0x200;
						}
						GBA_HALFWORD paletteData = readRawMemory<GBA_HALFWORD>(addressInPaletteRAM, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
						pGBA_display->colorForBlending[ONE].raw = paletteData;
					}
					// Mode 3
					else
					{
						pGBA_display->colorForBlending[ONE].raw = pGBA_display->colorNumberForBlending[ONE].colorNumber;
					}

					finalPixel = BLEND(
						pGBA_display->colorForBlending[ZERO]
						, pGBA_display->colorForBlending[ONE]
						, pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVA_COEFF
						, pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVB_COEFF);
				}
				else if (DOES_WINDOW_ALLOW_BLENDING(x, y) == YES && pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTFields.COLOR_SPL_EFFECTS != TO_UINT(COLOR_SPECIAL_EFFECTS::NORMAL))
				{
					switch ((COLOR_SPECIAL_EFFECTS)pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTFields.COLOR_SPL_EFFECTS)
					{
					case COLOR_SPECIAL_EFFECTS::ALPHA_BLENDING:
					{
						if ((pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0001 << pGBA_display->layersForBlending[ZERO]))
							&& (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0100 << pGBA_display->layersForBlending[ONE])))
						{
							if ((pGBA_display->colorNumberForBlending[ONE].colorNumber & IS_COLOR_NOT_PALETTE) == 0)
							{
								ID paletteIndex = pGBA_display->colorNumberForBlending[ONE].colorNumber;
								GBA_WORD addressInPaletteRAM = PALETTE_RAM_START_ADDRESS + paletteIndex;
								if (pGBA_display->colorNumberForBlending[ONE].isObject == YES)
								{
									addressInPaletteRAM += 0x200;
								}
								GBA_HALFWORD paletteData = readRawMemory<GBA_HALFWORD>(addressInPaletteRAM, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
								pGBA_display->colorForBlending[ONE].raw = paletteData;
							}
							// Mode 3 or Mode 5
							else
							{
								pGBA_display->colorForBlending[ONE].raw = pGBA_display->colorNumberForBlending[ONE].colorNumber;
							}

							finalPixel = BLEND(
								pGBA_display->colorForBlending[ZERO]
								, pGBA_display->colorForBlending[ONE]
								, pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVA_COEFF
								, pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVB_COEFF);
						}
						else
						{
							finalPixel = pGBA_display->colorForBlending[ZERO];
						}

						BREAK;
					}
					case COLOR_SPECIAL_EFFECTS::INCREASE_BRIGHTNESS:
					{
						if (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0001 << pGBA_display->layersForBlending[ZERO]))
						{
							finalPixel = BRIGHTEN(
								pGBA_display->colorForBlending[ZERO]
								, pGBA_peripherals->mBLDYHalfWord.mBLDYFields.EVY_COEFF);
						}
						else
						{
							finalPixel = pGBA_display->colorForBlending[ZERO];
						}

						BREAK;
					}
					case COLOR_SPECIAL_EFFECTS::DECREASE_BRIGHTNESS:
					{
						if (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0001 << pGBA_display->layersForBlending[ZERO]))
						{
							finalPixel = DARKEN(
								pGBA_display->colorForBlending[ZERO]
								, pGBA_peripherals->mBLDYHalfWord.mBLDYFields.EVY_COEFF);
						}
						else
						{
							finalPixel = pGBA_display->colorForBlending[ZERO];
						}

						BREAK;
					}
					default:
					{
						FATAL("Unknown Color Special Effect");
					}
					}
				}
				else
				{
					finalPixel = pGBA_display->colorForBlending[ZERO];
				}
			}

			RESET_PIXEL(x, y);

			if ((y < getScreenHeight()) && (x < getScreenWidth()))
			{
				pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].a = ALPHA;
				pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(finalPixel.RED);
				pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(finalPixel.GREEN);
				pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(finalPixel.BLUE);

				// increment merge cycles
				++pGBA_display->currentMergePixel;
			}
			else
			{
				FATAL("Display Buffer Out Of Bounds");
			}
		}

	}

	// ============================================
	// OBJECT (SPRITE) RENDERING
	// ============================================

	MASQ_INLINE void SET_INITIAL_OBJ_MODE()
	{

		if (pGBA_memory->mGBAMemoryMap.mOamAttributes.mOamAttribute[ZERO].mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.ROTATE_SCALE_FLAG == SET)
		{
			pGBA_display->currentObjectIsAffine = OBJECT_TYPE::OBJECT_IS_AFFINE;
		}
		else
		{
			pGBA_display->currentObjectIsAffine = OBJECT_TYPE::OBJECT_IS_NOT_AFFINE;
		}

	}

	MASQ_INLINE FLAG OBJ_A01_OBJ_CYCLE(ID oamID)
	{

		PPUDEBUG("MODE2_A01_OBJ_CYCLE for OAM%u", oamID);

		FLAG oamIDNeedsToBeRendered = NO;

		// Cache the object fetch stage to avoid repeated memory access
		auto& objCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)];

		objCache.reset();

		if (oamID >= ONETWENTYEIGHT)
		{
			RETURN oamIDNeedsToBeRendered;
		}

		GBA_WORD oamAddress = OAM_START_ADDRESS + (oamID << THREE);
		mOamAttr01Word_t oamAttributes01 = { ZERO };
		oamAttributes01.raw = readRawMemory<GBA_WORD>(oamAddress, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::PPU);

		objCache.ObjAttribute.mOamAttr01Word.raw = oamAttributes01.raw;
		objCache.spriteYScreenCoordinate = oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.Y_COORDINATE;

		if (oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.ROTATE_SCALE_FLAG == SET)
		{
			pGBA_display->nextObjectIsAffine = OBJECT_TYPE::OBJECT_IS_AFFINE;
			objCache.isAffine = YES;
			objCache.spriteHeight = SPRITE_HEIGHTS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_EN.OBJ_SIZE];
			objCache.spriteWidth = SPRITE_WIDTHS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_EN.OBJ_SIZE];

			objCache.isDoubleAffine = (oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_DISABLE_OR_DOUBLE_SIZE_FLAG == SET);
			objCache.isDisabled = NO;
			objCache.spriteXScreenCoordinate = oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_EN.X_COORDINATE;

			// Perform the double affine adjustments if enabled
			// https://gbadev.net/tonc/affobj.html
			if (objCache.isDoubleAffine)
			{
				// We shift the sprite towards the center of the doubled area in the screen
				PPUTODO("Find documentation for shifting sprite to middle when double affine is enabled!");
				objCache.spriteXScreenCoordinate += (objCache.spriteWidth >> ONE);
				objCache.spriteYScreenCoordinate += (objCache.spriteHeight >> ONE);
			}

			// Refer attribute 1 in https://gbadev.net/gbadoc/sprites.html
			// Refer wrapping effect in	https://gbadev.net/tonc/affobj.html
			if (objCache.spriteXScreenCoordinate > static_cast<SDIM32>(getScreenWidth()))
			{
				objCache.spriteXScreenCoordinate -= 512;
			}
		}
		else
		{
			pGBA_display->nextObjectIsAffine = OBJECT_TYPE::OBJECT_IS_NOT_AFFINE;
			objCache.isAffine = NO;
			objCache.spriteHeight = SPRITE_HEIGHTS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_DIS.OBJ_SIZE];
			objCache.spriteWidth = SPRITE_WIDTHS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_DIS.OBJ_SIZE];

			objCache.isDoubleAffine = NO;
			objCache.isDisabled = (oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_DISABLE_OR_DOUBLE_SIZE_FLAG == SET);
			objCache.spriteXScreenCoordinate = oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_DIS.X_COORDINATE;

			// Refer attribute 1 in https://gbadev.net/gbadoc/sprites.html
			// Refer wrapping effect in	https://gbadev.net/tonc/affobj.html
			if (objCache.spriteXScreenCoordinate > static_cast<SDIM32>(getScreenWidth()))
			{
				objCache.spriteXScreenCoordinate -= 512;
			}
		}

		// Refer attribute 0 in https://gbadev.net/gbadoc/sprites.html
		// Refer wrapping effect in	https://gbadev.net/tonc/affobj.html
		if (objCache.spriteYScreenCoordinate > static_cast<SDIM32>(getScreenHeight()))
		{
			objCache.spriteYScreenCoordinate -= 256; // 255 is max value possible
		}

		// Setup the min and max coordinates
		objCache.spriteMaxXScreenCoordinate = objCache.spriteXScreenCoordinate + objCache.spriteWidth;
		objCache.spriteMaxYScreenCoordinate = objCache.spriteYScreenCoordinate + objCache.spriteHeight;
		objCache.spriteMinXScreenCoordinate = objCache.spriteXScreenCoordinate;
		objCache.spriteMinYScreenCoordinate = objCache.spriteYScreenCoordinate;

		if (objCache.isDoubleAffine)
		{
			// We need this separate handling because we went ahead with option 2 for double affine
			objCache.spriteMinXScreenCoordinate -= objCache.spriteWidth / TWO;
			objCache.spriteMaxXScreenCoordinate += objCache.spriteWidth / TWO;
			objCache.spriteMinYScreenCoordinate -= objCache.spriteHeight / TWO;
			objCache.spriteMaxYScreenCoordinate += objCache.spriteHeight / TWO;
		}

		if (
			(objCache.isDisabled == NO)
			&&
			(objCache.vcount >= objCache.spriteMinYScreenCoordinate)
			&&
			(objCache.vcount < objCache.spriteMaxYScreenCoordinate)
			)
		{
			oamIDNeedsToBeRendered = YES;
		}

		RETURN oamIDNeedsToBeRendered;

	}

	MASQ_INLINE void OBJ_A2_OBJ_CYCLE(ID oamID)
	{

		PPUDEBUG("MODE2_A2_OBJ_CYCLE for OAM%u", oamID);

		// Directly copy the FETCH stage to the RENDER stage (No need for intermediate reset)
		auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
		renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)];

		GBA_WORD oamAddress = OAM_START_ADDRESS + (oamID << THREE) + FOUR;
		mOamAttr23Word_t oamAttributes23 = { ZERO };
		oamAttributes23.mOamAttr2HalfWord.mOamAttr2HalfWord = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

		renderCache.ObjAttribute.mOamAttr23Word.raw = oamAttributes23.raw;

		if (ENABLED)
		{
			// If double affine, then we need to start rendering (sprite width / 2) pixels before the actual sprite pixel start and end (sprite width / 2) after actual sprite pixel end
			if (renderCache.isDoubleAffine == YES)
			{
				auto& fetchCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)];
				renderCache.spriteXStart = -(fetchCache.spriteWidth >> ONE);
				renderCache.spriteXEnd = fetchCache.spriteWidth + (fetchCache.spriteWidth >> ONE);
				renderCache.spriteXPixelCoordinate = -(fetchCache.spriteWidth >> ONE); // currently... we havent traversed the sprite in x coordinates...
			}
			else
			{
				renderCache.spriteXStart = ZERO;
				renderCache.spriteXEnd = renderCache.spriteWidth;
				renderCache.spriteXPixelCoordinate = ZERO; // currently... we havent traversed the sprite in x coordinates...
			}

			renderCache.spriteYPixelCoordinate = renderCache.vcount - renderCache.spriteYScreenCoordinate;

			// Store the base tile ID directly in the render stage
			renderCache.baseTileID = renderCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.CHARACTER_NAME;
		}

	}

	MASQ_INLINE void OBJ_PA_OBJ_CYCLE(ID oamID)
	{

		PPUDEBUG("MODE2_PA_OBJ_CYCLE for OAM%u", oamID);

		// How the rotation matrix is stored in memory is as follows:
		// 0x07000000	->	Attr0
		// 0x07000002	->	Attr1
		// 0x07000004	->	Attr2
		// 0x07000006	->	PA
		// 0x07000008	->	Attr0
		// 0x0700000A	->	Attr1
		// 0x0700000C	->	Attr2
		// 0x0700000E	->	PB
		// 0x07000010	->	Attr0
		// 0x07000012	->	Attr1
		// 0x07000014	->	Attr2
		// 0x07000016	->	PC
		// 0x07000018	->	Attr0
		// 0x0700001A	->	Attr1
		// 0x0700001C	->	Attr2
		// 0x0700001E	->	PD
		// 0x07000020	->	Attr0
		// 0x07000022	->	Attr1
		// 0x07000024	->	Attr2
		// 0x07000026	->	PA

		// Gap b/w 1 PA to another is (for eg 0x07000026 - 0x07000006) = 0x20 (32)

		auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
		GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + SIX;
		renderCache.affine.pa = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

	}

	MASQ_INLINE void OBJ_PB_OBJ_CYCLE(ID oamID)
	{

		PPUDEBUG("MODE2_PB_OBJ_CYCLE for OAM%u", oamID);

		// Refer to MODE2_PA_OBJ_CYCLE for info on the below equation
		auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
		GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + FOURTEEN;
		renderCache.affine.pb = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

	}

	MASQ_INLINE void OBJ_PC_OBJ_CYCLE(ID oamID)
	{

		PPUDEBUG("MODE2_PC_OBJ_CYCLE for OAM%u", oamID);

		// Refer to MODE2_PA_OBJ_CYCLE for info on the below equation
		auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
		GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + TWENTYTWO;
		renderCache.affine.pc = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

	}

	MASQ_INLINE void OBJ_PD_OBJ_CYCLE(ID oamID)
	{

		PPUDEBUG("MODE2_PD_OBJ_CYCLE for OAM%u", oamID);

		// Refer to MODE2_PA_OBJ_CYCLE for info on the below equation
		auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
		GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + THIRTY;
		renderCache.affine.pd = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

	}

	MASQ_INLINE void OBJ_V_OBJ_CYCLE(ID oamID, OBJECT_TYPE isAffine, STATE8 state)
	{

		PPUDEBUG("MODE2_V_OBJ_CYCLE[%u] for OAM%u", state, oamID);

		auto& objCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
		INC16 spriteXStart = objCache.spriteXPixelCoordinate;
		INC16 spriteXEnd = RESET;

		if (isAffine == OBJECT_TYPE::OBJECT_IS_NOT_AFFINE)
		{
			// Identity matrix (terms which get multiplied will be set to 1 and terms which gets added will be set to 0)
			objCache.affine.pa = 0xFF;
			objCache.affine.pb = 0x00;
			objCache.affine.pc = 0x00;
			objCache.affine.pd = 0xFF;

			spriteXEnd = spriteXStart + TWO;	// only 2 pixel per VRAM stage
		}
		else
		{
			spriteXEnd = spriteXStart + ONE;	// only 1 pixel per VRAM stage
		}

		INC32 transformedSpriteX = ZERO, transformedSpriteY = ZERO;
		INC16 spriteY = objCache.spriteYPixelCoordinate;
		SDIM32 Y = objCache.vcount;
		// update the spriteX coordinates (needed for next VRAM stages)
		objCache.spriteXPixelCoordinate = spriteXEnd;
		// check if this needs to be the last vram cycle for this object
		pGBA_display->lastVRAMCycleForObjFSM = (spriteXEnd == objCache.spriteXEnd) ? YES : NO;

		auto pa = objCache.affine.pa;
		auto pb = objCache.affine.pb;
		auto pc = objCache.affine.pc;
		auto pd = objCache.affine.pd;
		auto x0 = objCache.spriteWidth >> ONE;
		auto y0 = objCache.spriteHeight >> ONE;

		for (INC16 spriteX = spriteXStart; spriteX < spriteXEnd; spriteX++)
		{
			// NOTE: We need to get the screen coordinates before it got modified because of flip or affine
			// other wise tranformation from screen space to affine space will get cancelled
			// even for rotation, this is basically like flipping the sprite coordinated and then again flipping the screen coordinates, flip gets cancelled!
			int32_t screenX = spriteX + objCache.spriteXScreenCoordinate;

			if (isAffine == OBJECT_TYPE::OBJECT_IS_AFFINE)
			{
				/*
				*	according to formula mentioned in http://problemkaputt.de/gbatek-lcd-i-o-bg-rotation-scaling.htm
				*	affine can be calculated as follows
				*
				*	Using the following expressions,
				*	  x0,y0    Rotation Center
				*	  x1,y1    Old Position of a pixel (before rotation/scaling)
				*	  x2,y2    New position of above pixel (after rotation scaling)
				*	  A,B,C,D  BG2PA-BG2PD Parameters (as calculated above)
				*
				*	the following formula can be used to calculate x2,y2:
				*
				*	  x2 = A(x1-x0) + B(y1-y0) + x0
				*	  y2 = C(x1-x0) + D(y1-y0) + y0
				*
				*/

				// for X
				transformedSpriteX = (pa * (spriteX - x0)) + (pb * (spriteY - y0));
				// get the integer part of transformedSpriteX
				transformedSpriteX >>= EIGHT;
				transformedSpriteX += x0;

				// boundary check post transformation
				if (transformedSpriteX < 0 || transformedSpriteX >= objCache.spriteWidth)
					continue;

				// for Y
				transformedSpriteY = (pc * (spriteX - x0)) + (pd * (spriteY - y0));
				// get the integer part of transformedSpriteX
				transformedSpriteY >>= EIGHT;
				transformedSpriteY += y0;

				// boundary check post transformation
				if (transformedSpriteY < ZERO || transformedSpriteY >= objCache.spriteHeight)
					continue;
			}
			else
			{
				if (objCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_DIS.HOR_FLIP == SET)
					transformedSpriteX = objCache.spriteWidth - spriteX - ONE;
				else
					transformedSpriteX = spriteX;

				if (objCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_DIS.VER_FLIP == SET)
					transformedSpriteY = objCache.spriteHeight - spriteY - ONE;
				else
					transformedSpriteY = spriteY;
			}

			SDIM32 X = screenX;

			if (X >= ZERO && X < static_cast<SDIM32>(getScreenWidth()))
			{
				// Y Transformation:
				if (ENABLED)
				{
					/*
					*	Let's assume the object on the screen is as follows:
					*
					*	 ________________________________________________________
					*	|														|
					*	|														|
					*	|														|
					* 	|														|
					* 	|				_________________						|
					* 	|				|		|		|						|
					*	|				|TY00	|TY01	|						|
					* 	|				|_______| ______|						|
					* 	|				|		|		|						|
					* 	|				|TY10	|TY11	|						|
					* 	|				|		|		|						|
					* 	|				__________________						|
					* 	|														|
					* 	|														|
					* 	|														|
					*	_________________________________________________________
					*
					*/

					// To get the row of the tile of interest
					// we just need to get transformedSpriteY / 8
					int32_t spriteYTile = transformedSpriteY >> THREE;
					// Now, to get the column of the tile of interest
					// we first get the witdth of the tile in terms of number of tiles
					// i.e. totalwidth of object / 8 gives you how many tiles are there in a row for a given object
					int32_t widthOfSpriteInTiles = objCache.spriteWidth >> THREE;

					ID tileIDOffsetBecauseOfY = ZERO;

					// Refer https://gbadev.net/gbadoc/sprites.html
					// Refer http://problemkaputt.de/gbatek-lcd-obj-vram-character-tile-mapping.htm
					// 2D mapping
					if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.OBJ_CHAR_VRAM_MAP == RESET)
					{
						// Assume tiles are arranged as follows for 2D mapping in memory

						/*
						*	TILE0000	| TILE0001	| TILE0002	| TILE0003	| TILE0004  | TILE0005  | TILE0006  .... | TILE0031
						*   TILE0032	| TILE0033	| TILE0034	| TILE0035	| TILE0036	| TILE0037	| TILE0038	.... | TILE0063
						*	:
						*   TILE0992	| TILE0993	| TILE0994	| TILE0995	| TILE0996	| TILE0997	| TILE0998	.... | TILE1023
						*/

						// pixels can be be arranged as a matrix of 32x32 or 16x32 tiles based on 16 or 256 color mode

						// To handle 256 color mode, we need to consider twice as much tiles, we handle this as (<< attribute0.color/palettes)

						// To get the tile offset, lets first process the y tile offset
						// if 8x8, tileIDOffsetBecauseOfY = 0
						// if 16x16, tileIDOffsetBecauseOfY = 32
						// if 32x32, tileIDOffsetBecauseOfY = 64

						// we can get above values of tileIDOffsetBecauseOfY using spriteYTile

						tileIDOffsetBecauseOfY = spriteYTile << FIVE;

						// Note: The reason we don't do anything special for 256 color mode is because "widthOfSpriteInTiles" will be confined to current row
						// So, next row will never get affected, so tile immediatly below (or few more rows below) the 1st row tile needs to be considered...so we just add multiple of 32 
					}
					// 1D mapping
					else
					{
						// Assume tiles are arranged as follows for 2D mapping in memory

						// Refer http://problemkaputt.de/gbatek-lcd-obj-oam-attributes.htm
						// In 256 color mode
						/*
						*	TILE0000	| TILE0002	| TILE0004	| TILE0006	| TILE0008  | TILE0010  | TILE0012  .... | TILE1022
						*/

						// Otherwise

						/*
						*	TILE0000	| TILE0001	| TILE0002	| TILE0003	| TILE0004  | TILE0005  | TILE0006  .... | TILE1023
						*/

						// To handle 256 color mode, we need to consider twice as much tiles, we handle this as (<< attribute0.color/palettes)

						// To get the tile offset, lets first process the y tile offset
						// if 8x8, tileIDOffsetBecauseOfY = 0
						// if 16x16, tileIDOffsetBecauseOfY = 2
						// if 32x32, tileIDOffsetBecauseOfY = 4

						// we can get above values of tileIDOffsetBecauseOfY using spriteYTile and widthOfSpriteInTiles

						tileIDOffsetBecauseOfY = widthOfSpriteInTiles * spriteYTile;
						// handle 256 color mode if needed (Xly by 2 as widthOfSpriteInTiles becomes double)
						tileIDOffsetBecauseOfY <<= objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.COLOR_PALETTES;
					}

					// after shifting the base tile id in y direction, this is the new base for shifing in x direction
					objCache.tileIDAfterAccountingForY = tileIDOffsetBecauseOfY;
				}

				// X Transformation:
				if (ENABLED)
				{
					// now need to get the tile id offset because of X
					// x offset would be the current sprite x coordinate / 8
					// refer to diagrams in MODE2_A2_OBJ_CYCLE for more info

					ID tileIDOffsetBecauseOfX = transformedSpriteX >> THREE;
					// Xly by 2 if 256 color mode because we need to skip every alternative (odd) tiles
					// refer OBJ Tile Number in http://problemkaputt.de/gbatek-lcd-obj-oam-attributes.htm
					// so, if tile number is 0, then we remain at 0
					// if tile number is 1, then we jump to 2
					// if tile number is 3, then we jump to 6
					// if tile number is 4, then we jump to 8
					// and so on...
					tileIDOffsetBecauseOfX <<= objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.COLOR_PALETTES;

					objCache.tileIDAfterAccountingForX = tileIDOffsetBecauseOfX;
				}

				int32_t actualTileID = objCache.baseTileID + objCache.tileIDAfterAccountingForY + objCache.tileIDAfterAccountingForX;

				// using the tileID, we need to fetch the corresponding tile data
				// for this, we need to first figure out addressInTileDataArea 

				// also, even with addressInTileDataArea, we need to figure out the data pertaining to the pixel within the tile of interest
				// NOTE: we still do %8 and not %width even for sprites bigger than 8x8 because
				// 1) for bigger tiles, the tileID would change for every 8x8 which takes care of getting appropriate address... no need to account for it in modulo again
				// 2) tile data is stored in terms of 8x8 

				SDIM32 xTileCoordinate = transformedSpriteX & SEVEN;
				SDIM32 yTileCoordinate = transformedSpriteY & SEVEN;
				GBA_WORD withinTileOffset = xTileCoordinate + (yTileCoordinate << THREE);

				// Refer : http://problemkaputt.de/gbatek-lcd-obj-overview.htm
				// Refer : http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
				// NOTE: in tile data area, granularity within the tile data is decided based on 4bpp mode or 8bpp mode
				// in 4bpp mode, each tile is 32 bytes, first 4 bytes for the topmost row of the tile
				// each byte representing two dots, the lower 4 bits define the color for the left and upper 4 bits the color for the right dot
				// in 8bpp mode, each tile is 64 bytes, first 8 bytes for the topmost row of the tile
				// each byte selects palette entry for each dot
				// 8bpp mode is the 256 color mode that is represented in attribute 0
				// Refer : https://gbadev.net/gbadoc/sprites.html
				// irrespective mode, each tile ID represents 32 bytes of data in sprite tile data area
				// so the memory address of a tile is roughly 0x06010000 + T*32
				const INC16 eachIndexReferences32BytesInShifts = FIVE; // Indicates 32 in power of 2 (to use it for optimized multiplication)
				PPUTODO("Each byte is 64 byte in 8bpp mode, so each index should reference 64 bytes in 8bpp? (Source : NBA)");

				INC32 paletteIndex = RESET;

				// 16 color mode (4bpp mode)
				if (objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.COLOR_PALETTES == RESET)
				{
					GBA_WORD addressInTileDataArea =
						VIDEO_RAM_START_ADDRESS
						+ 0x10000
						+ (actualTileID << eachIndexReferences32BytesInShifts)
						// each byte represents 2 dots
						// divide the withinTileOffset by 2 as single byte can represent 2 pixels
						// so "withinTileOffset" is halved
						+(withinTileOffset >> ONE);

					BYTE pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);

					// In 4bpp mode, only 4 bits out of pixelColorData represents actual color for pixel, other 4 bits are for the adjascent pixels
					// As mentioned in http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
					// lower 4 bits for left dot (lower X) and upper 4 bits for right dot (higher X)
					// take an example of dot 2 and dot 3
					// dot 2 needs to use bit0-3 and dot 3 needs to use bit4-7
					// withinTileOffset gives use dot number within tile
					// basically, for even dot, we simply extract first 4 bits
					// for odd dot, we want to shift bit4-7 right by 4 and then extract only first 4 bits
					// also, using the naming convention of left and right dot given in http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
					// tile data will be as follows:
					// row0 : LEFT(000), RIGHT(001), LEFT(002), RIGHT(003), LEFT(004), RIGHT(005), LEFT(006), RIGHT(007)
					// row1 : LEFT(008), RIGHT(009), LEFT(010), RIGHT(011), LEFT(012), RIGHT(013), LEFT(014), RIGHT(015)
					// so for odd "withinTileOffset" perform the shift and extract, else directly extract						

					// Odd means we need to shift right by 4
					if (withinTileOffset & ONE)
					{
						pixelColorNumberFromTileData >>= FOUR;
					}
					// Extract only the required 4 bits
					pixelColorNumberFromTileData &= 0x0F;

					// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
					paletteIndex = pixelColorNumberFromTileData << ONE;

					PPUTODO("Why paletteIndex != ZERO check is needed before accounting for palette bank number in 4bpp mode? \n\
							Will not having this check cause non-transparent pixel even when transparent is desired? (Source : NBA)");
					if (paletteIndex != ZERO)
					{
						// palette bank number is provided as part of attribute 2 in 4bpp mode
						// this palette number is the index into one of the 16 16-colored palettes (Refer : https://gbadev.net/gbadoc/sprites.html)
						// palette ram for sprites goes from 0x05000200 to 0x050003FF which is 512 bytes
						// Hence, each 16 16-colored palettes should take 32 bytes
						// OR
						// we can think that whole sprite palette area is divided into 16 palettes with each palette having 16 colors (Refer : http://problemkaputt.de/gbatek-lcd-color-palettes.htm)
						// each color is 2 bytes, so each palette takes 16 * 2 bytes = 32 bytes (Refer : http://problemkaputt.de/gbatek-lcd-color-palettes.htm)
						// Hence, each palette is 32 bytes
						// palette number in attribute 2 * THIRTYTWO will give the address of desired palette number in sprite palette area

						const DIM16 sizeOfEachPaletteInBytesInShifts = FIVE; // Indicates 32 in power of 2 (to use it for optimized multiplication) 
						paletteIndex += (objCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.PALETTE_NUMBER << sizeOfEachPaletteInBytesInShifts);
					}
				}
				else
				{
					GBA_WORD addressInTileDataArea =
						VIDEO_RAM_START_ADDRESS
						+ 0x10000
						+ (actualTileID << eachIndexReferences32BytesInShifts)
						+ withinTileOffset;

					BYTE pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);

					// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
					paletteIndex = pixelColorNumberFromTileData << ONE;
				}

				if ((OBJECT_MODE)objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_MODE == OBJECT_MODE::OBJ_WINDOW
					&&
					paletteIndex != ZERO)
				{
					pGBA_display->gfx_obj_window[X][Y] = ENABLED;
				}
				else if (objCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.PRIORITY < pGBA_display->objPriority[X][Y]
					||
					pGBA_display->gfx_obj[X][Y] == ZERO)
				{
					PPUTODO("Do we even need this paletteIndex != ZERO check, even if we populate paletteIndex of zero, wont merge take care of transparent sprites? (Source : NBA)");
					if (paletteIndex != ZERO)
					{
						pGBA_display->gfx_obj[X][Y] = paletteIndex;
						pGBA_display->gfx_obj_mode[X][Y] = (OBJECT_MODE)objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_MODE;
					}
					pGBA_display->objPriority[X][Y] = objCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.PRIORITY;
				}
			}
		}

	}

	MASQ_INLINE int32_t INCREMENT_OAM_ID()
	{

		// Check and increment objAccessOAMIDState only if it's within bounds
		if (pGBA_display->objAccessOAMIDState < ONETWENTYEIGHT - ONE)
		{
			++pGBA_display->objAccessOAMIDState;
			RETURN VALID;
		}
		RETURN INVALID;

	}

	// ============================================
	// WINDOW RENDERING
	// ============================================

	MASQ_INLINE void HANDLE_WINDOW_INTERNAL(uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2, FLAG& winH, FLAG& winV, int winIndex, uint32_t xPixelCoordinate, uint32_t yPixelCoordinate)
	{
		// Check and adjust the boundaries
		if (x2 > getScreenWidth() || x1 > x2) x2 = getScreenWidth();
		if (y2 > getScreenHeight() || y1 > y2) y2 = getScreenHeight();

		// Horizontal and vertical range checks
		winH = (xPixelCoordinate >= x1 && xPixelCoordinate <= x2) ? YES : NO;
		winV = (yPixelCoordinate >= y1 && yPixelCoordinate <= y2) ? YES : NO;

		// Update the window pixel array
		pGBA_display->gfx_window[winIndex][xPixelCoordinate][yPixelCoordinate] = winH && winV;
	};

	MASQ_INLINE void WIN_CYCLE()
	{

		uint32_t xPixelCoordinate = pGBA_display->currentWinPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		if ((xPixelCoordinate >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Handle Win0
		FLAG win01H = NO, win01V = NO;
		HANDLE_WINDOW_INTERNAL(pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i1, pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i2,
			pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i1, pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i2,
			win01H, win01V, WIN0, xPixelCoordinate, yPixelCoordinate);

		// Handle Win1
		FLAG win11H = NO, win11V = NO;
		HANDLE_WINDOW_INTERNAL(pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i1, pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i2,
			pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i1, pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i2,
			win11H, win11V, WIN1, xPixelCoordinate, yPixelCoordinate);

		// increment windows pixel counter
		++pGBA_display->currentWinPixel;

	}

	// ============================================
	// MODE 0 and MODE 1 BACKGROUND RENDERING
	// ============================================

#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
	MASQ_INLINE void MODE0_M_BG_CYCLE(ID bgID)
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Bounds check for bgID
		if (bgID < BG0 || bgID > BG3)
		{
			FATAL("Unknown BG in mode 0");
			RETURN;
		}

		pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
		pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

		GBA_WORD bgTileMapBaseAddr = ZERO;
		mBGnCNTHalfWord_t BGxCNT = { ZERO };
		DIM16 hofs = ZERO;
		DIM16 vofs = ZERO;

		// Cache BG configuration values for fast access
		auto getBGConfig = [&](ID id) -> std::tuple<int, FLAG, uint16_t, uint16_t, uint16_t>
			{
				switch (id)
				{
				case BG0:
					RETURN{
						static_cast<int>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
						pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord),
						static_cast<uint16_t>(pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				case BG1:
					RETURN{
						static_cast<int>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
						pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord),
						static_cast<uint16_t>(pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				case BG2:
					RETURN{
						static_cast<int>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
						pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord),
						static_cast<uint16_t>(pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				case BG3:
					RETURN{
						static_cast<int>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
						pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord),
						static_cast<uint16_t>(pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				default:
					// Safe fallback to avoid undefined behavior
					RETURN{ 0, false, 0, 0, 0 };
				}
			};

		auto [bgTileMapBaseAddr_, is8bppMode, BGxCNT_, hofs_, vofs_] = getBGConfig(bgID);
		bgTileMapBaseAddr = bgTileMapBaseAddr_;
		pGBA_display->bgCache[bgID].is8bppMode = is8bppMode;
		BGxCNT.mBGnCNTHalfWord = BGxCNT_;
		hofs = hofs_;
		vofs = vofs_;

		/*
		* Refer http://problemkaputt.de/gbatek-lcd-i-o-bg-control.htm
		* BGs always wraparound, so
		* offsets = BGxHOFS and BGxHVOFS (BG offsets within static screen)
		* Final offsets from screen (tileMap_x and tileMap_y) = (x + BGxHOFS) % 512 and (y + BGxHVOFS) % 512
		* Offsets in tile coordinates (tile_x and tile_y) = Final offsets from screen / 8
		* Offsets within tile coordinates (WithinTile_x and WithinTile_y) = Final offsets from screen % 8
		*
		* When BGxCNT->Size = 0
		* SC0 -> 256x256
		* When BGxCNT->Size = 1
		* SC0 SC1 -> 512x256
		* When BGxCNT->Size = 2
		* SC0 -> 256x512
		* SC1
		* When BGxCNT->Size = 3
		* SC0 SC1 -> 512x512
		* SC2 SC3
		* SC0 is defined by the normal BG Map base address (Bit 8-12 of BGxCNT),
		* SC1 uses same address +2K, SC2 address +4K, SC3 address +6K
		* So, for given x, y we need to figure out in which SCn it falls...
		* Then each SCn is 256x256, so , number of tiles i.e. tilesPerRow = bitsPerRow / 8 = 32
		* So, withinTileOffset = (tile_x + (tile_y * tilesPerRow))
		* But, we do a 16bit read for tile map as mentioned under Mode 0 in https://nba-emu.github.io/hw-docs/ppu/background.html and http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm unlike non text modes
		* So, 16withinTileOffset = 2 * withinTileOffset
		*
		* Final Address for tile map = [VRAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 2K) + (16withinTileOffset)]
		*/

		// Cached sum of coordinates and offsets
		DIM16 offsetSumX = xPixelCoordinate + hofs;
		DIM16 offsetSumY = yPixelCoordinate + vofs;

		ID SCn = ZERO;
		switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
		{
			// SC0
		case ZERO: SCn = ZERO; BREAK;
			// SC0 SC1
		case ONE: SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO; BREAK;
			// SC0
			// SC1
		case TWO: SCn = ((offsetSumY & 511) > 255) ? ONE : ZERO; BREAK;
			// SC0 SC1
			// SC2 SC3
		case THREE:
			SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO;
			SCn += ((offsetSumY & 511) > 255) ? TWO : ZERO;
			BREAK;
		default:
			FATAL("Unknown screen size in mode 0");
			RETURN;
		}

		pGBA_display->bgCache[bgID].hofs = hofs;
		pGBA_display->bgCache[bgID].vofs = vofs;

		// Within SCn
		DIM16 tileMapX = offsetSumX & 255; // % 256 because because if more than 255, it is SC1 or SC3, but we are already accounting for SCn, so we just get tileX within SCn
		DIM16 tileMapY = offsetSumY & 255; // % 256 because because if more than 255, it is SC2 or SC3, but we are already accounting for SCn, so we just get tileX within SCn

		pGBA_display->bgCache[bgID].tileMapX = tileMapX;
		pGBA_display->bgCache[bgID].tileMapY = tileMapY;

		const DIM16 tilesPerRowInShifts = FIVE; // 1 << 5 is same as * 32; Each SCn is of 256*256, so 256 / 8 = 32
		DIM16 tileMapOffset = (tileMapX >> THREE) + ((tileMapY >> THREE) << tilesPerRowInShifts);

		// Since we have 16 bits per tile instead of 8
		// Refer http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
		tileMapOffset <<= ONE;

		GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileMapArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
			PPUTODO("As per NBA, this is set to some latched value");
		}

	}

	MASQ_INLINE void MODE0_T_BG_CYCLE(ID bgID)
	{

		auto X = pGBA_display->currentBgPixel;
		auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Bounds check for bgID
		if (bgID < BG0 || bgID > BG3)
		{
			FATAL("Unknown BG in mode 0");
			RETURN;
		}

		GBA_WORD bgTileDataBaseAddr =
			(bgID == BG0)
			? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
			: (bgID == BG1)
			? (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
			: (bgID == BG2)
			? (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
			: (pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

		auto xTileCoordinate = pGBA_display->bgCache[bgID].tileMapX & SEVEN; // % 8
		auto yTileCoordinate = pGBA_display->bgCache[bgID].tileMapY & SEVEN; // % 8

		xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
			? flipLUT[xTileCoordinate]
			: xTileCoordinate;

		yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
			? flipLUT[yTileCoordinate]
			: yTileCoordinate;

		GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
		// Refer http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
		DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

		// 4bpp mode
		if (pGBA_display->bgCache[bgID].is8bppMode == NO)
		{
			// 4bpp mode
			GBA_WORD addressInTileDataArea =
				VIDEO_RAM_START_ADDRESS
				+ bgTileDataBaseAddr
				+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
				+ (withinTileDataOffset >> ONE); // Divide by 2 because in 4bpp mode, A single halfword gives data for 4 pixels

			BYTE pixelColorNumberFor2PixelsFromTileData = RESET;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pixelColorNumberFor2PixelsFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				PPUTODO("As per NBA, pixelColorNumberFor2PixelsFromTileData is set to some latched value");
			}

			// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
			// Odd means we need to shift right by 4
			pixelColorNumberFor2PixelsFromTileData >>= ((withinTileDataOffset & ONE) ? FOUR : ZERO);
			// extract only the required 4 bits
			BYTE pixelColorNumberPerPixel = pixelColorNumberFor2PixelsFromTileData & 0x0F;
			INC32 paletteIndex = pixelColorNumberPerPixel << ONE;

			if (paletteIndex != ZERO)
			{
				// Refer mode 2 bg processing to see how we determine sizeOfEachPaletteInBytes
				const DIM16 sizeOfEachPaletteInBytesInShifts = FIVE; // 1 << 5 is same as *32
				paletteIndex += (pGBA_display->bgCache[bgID].tileDescriptor.pb << sizeOfEachPaletteInBytesInShifts);
			}
			pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
		}
		else
		{
			// 8bpp mode
			GBA_WORD addressInTileDataArea =
				VIDEO_RAM_START_ADDRESS
				+ bgTileDataBaseAddr
				+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
				+ withinTileDataOffset;

			BYTE pixelColorNumberFor1PixelFromTileData = RESET;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pixelColorNumberFor1PixelFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				PPUTODO("As per NBA, pixelColorNumberFor1PixelFromTileData is set to some latched value");
			}

			// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
			INC32 paletteIndex = pixelColorNumberFor1PixelFromTileData << ONE;
			pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
		}

	}

	MASQ_INLINE void MODE1_M_TEXT_BG_CYCLE(ID bgID)
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Bounds check for bgID
		if (bgID < BG0 || bgID > BG1)
		{
			FATAL("Unknown BG in mode 1");
			RETURN;
		}

		pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
		pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

		GBA_WORD bgTileMapBaseAddr = ZERO;
		mBGnCNTHalfWord_t BGxCNT = { ZERO };
		DIM16 hofs = ZERO;
		DIM16 vofs = ZERO;

		if (bgID == BG0)
		{
			bgTileMapBaseAddr = (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
			pGBA_display->bgCache[BG0].is8bppMode = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
			BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord;
			hofs = pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET;
			vofs = pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET;
		}
		else if (bgID == BG1)
		{
			bgTileMapBaseAddr = (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
			pGBA_display->bgCache[BG1].is8bppMode = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
			BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord;
			hofs = pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET;
			vofs = pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET;
		}
		else
		{
			FATAL("Unknown BG in mode 1 text mode");
			RETURN;
		}

		/*
		* Refer http://problemkaputt.de/gbatek-lcd-i-o-bg-control.htm
		* BGs always wraparound, so
		* offsets = BGxHOFS and BGxHVOFS (BG offsets within static screen)
		* Final offsets from screen (tileMap_x and tileMap_y) = (x + BGxHOFS) % 512 and (y + BGxHVOFS) % 512
		* Offsets in tile coordinates (tile_x and tile_y) = Final offsets from screen / 8
		* Offsets within tile coordinates (WithinTile_x and WithinTile_y) = Final offsets from screen % 8
		*
		* When BGxCNT->Size = 0
		* SC0 -> 256x256
		* When BGxCNT->Size = 1
		* SC0 SC1 -> 512x256
		* When BGxCNT->Size = 2
		* SC0 -> 256x512
		* SC1
		* When BGxCNT->Size = 3
		* SC0 SC1 -> 512x512
		* SC2 SC3
		* SC0 is defined by the normal BG Map base address (Bit 8-12 of BGxCNT),
		* SC1 uses same address +2K, SC2 address +4K, SC3 address +6K
		* So, for given x, y we need to figure out in which SCn it falls...
		* Then each SCn is 256x256, so , number of tiles i.e. tilesPerRow = bitsPerRow / 8 = 32
		* So, withinTileOffset = (tile_x + (tile_y * tilesPerRow))
		* But, we do a 16bit read for tile map as mentioned under Mode 0 in https://nba-emu.github.io/hw-docs/ppu/background.html and http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm unlike non text modes
		* So, 16withinTileOffset = 2 * withinTileOffset
		*
		* Final Address for tile map = [VRAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 2K) + (16withinTileOffset)]
		*
		*/

		// Cached sum of coordinates and offsets
		DIM16 offsetSumX = xPixelCoordinate + hofs;
		DIM16 offsetSumY = yPixelCoordinate + vofs;

		ID SCn = ZERO;
		switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
		{
			// SC0
		case ZERO: SCn = ZERO; BREAK;
			// SC0 SC1
		case ONE: SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO; BREAK;
			// SC0
			// SC1
		case TWO: SCn = ((offsetSumY & 511) > 255) ? ONE : ZERO; BREAK;
			// SC0 SC1
			// SC2 SC3
		case THREE:
			SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO;
			SCn += ((offsetSumY & 511) > 255) ? TWO : ZERO;
			BREAK;
		default:
			FATAL("Unknown screen size in mode 0");
			RETURN;
		}

		pGBA_display->bgCache[bgID].hofs = hofs;
		pGBA_display->bgCache[bgID].vofs = vofs;

		// Within SCn
		DIM16 tileMapX = offsetSumX & 255; // % 256 because because if more than 255, it is SC1 or SC3, but we are already accounting for SCn, so we just get tileX within SCn
		DIM16 tileMapY = offsetSumY & 255; // % 256 because because if more than 255, it is SC2 or SC3, but we are already accounting for SCn, so we just get tileX within SCn

		pGBA_display->bgCache[bgID].tileMapX = tileMapX;
		pGBA_display->bgCache[bgID].tileMapY = tileMapY;

		const DIM16 tilesPerRowInShifts = FIVE; // 1 << 5 is same as * 32; Each SCn is of 256*256, so 256 / 8 = 32
		DIM16 tileMapOffset = (tileMapX >> THREE) + ((tileMapY >> THREE) << tilesPerRowInShifts);

		// Since we have 16 bits per tile instead of 8
		// Refer http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
		tileMapOffset <<= ONE;

		GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileMapArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
			PPUTODO("As per NBA, this is set to some latched value");
		}

	}

	MASQ_INLINE void MODE1_T_TEXT_BG_CYCLE(ID bgID)
	{

		auto X = pGBA_display->currentBgPixel;
		auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Bounds check for bgID
		if (bgID < BG0 || bgID > BG1)
		{
			FATAL("Unknown BG in mode 1");
			RETURN;
		}

		GBA_WORD bgTileDataBaseAddr = (bgID == BG0)
			? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
			: (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

		auto xTileCoordinate = pGBA_display->bgCache[bgID].tileMapX & SEVEN; // % 8
		auto yTileCoordinate = pGBA_display->bgCache[bgID].tileMapY & SEVEN; // % 8

		xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
			? flipLUT[xTileCoordinate]
			: xTileCoordinate;

		yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
			? flipLUT[yTileCoordinate]
			: yTileCoordinate;

		GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
		// Refer http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
		DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

		// 4bpp mode
		if (pGBA_display->bgCache[bgID].is8bppMode == NO)
		{
			GBA_WORD addressInTileDataArea =
				VIDEO_RAM_START_ADDRESS
				+ bgTileDataBaseAddr
				+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
				+ (withinTileDataOffset >> ONE); // Divide by 2 because in 4bpp mode, A single halfword gives data for 4 pixels

			BYTE pixelColorNumberFor2PixelsFromTileData = RESET;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pixelColorNumberFor2PixelsFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				PPUTODO("As per NBA, pixelColorNumberFor2PixelsFromTileData is set to some latched value");
			}

			// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
			// Odd means we need to shift right by 4
			pixelColorNumberFor2PixelsFromTileData >>= ((withinTileDataOffset & ONE) ? FOUR : ZERO);
			// extract only the required 4 bits
			BYTE pixelColorNumberPerPixel = pixelColorNumberFor2PixelsFromTileData & 0x0F;
			INC32 paletteIndex = pixelColorNumberPerPixel << ONE;

			if (paletteIndex != ZERO)
			{
				// Refer mode 2 bg processing to see how we determine sizeOfEachPaletteInBytes
				const DIM16 sizeOfEachPaletteInBytesInShifts = FIVE; // 1 << 5 is same as *32
				paletteIndex += (pGBA_display->bgCache[bgID].tileDescriptor.pb << sizeOfEachPaletteInBytesInShifts);
			}
			pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
		}
		// 8bpp mode
		else
		{
			// 8bpp mode
			GBA_WORD addressInTileDataArea =
				VIDEO_RAM_START_ADDRESS
				+ bgTileDataBaseAddr
				+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
				+ withinTileDataOffset;

			BYTE pixelColorNumberFor1PixelFromTileData = RESET;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pixelColorNumberFor1PixelFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				PPUTODO("As per NBA, pixelColorNumberFor1PixelFromTileData is set to some latched value");
			}

			// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
			INC32 paletteIndex = pixelColorNumberFor1PixelFromTileData << ONE;
			pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
		}

	}

#else
	MASQ_INLINE void RENDER_MODE0_MODE1_PIXEL_X(ID bgID, GBA_HALFWORD pixelData, STATE8 state)
	{

		auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		if (pGBA_display->currentBgPixelInTextMode[bgID] >= (TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)))
		{
			RETURN;
		}

		if (pGBA_display->bgCache[bgID].is8bppMode == NO)
		{
			// 4bpp mode

			GBA_HALFWORD pixelColorNumberFor4PixelsFromTileData = pixelData;

			if (pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES)
			{
				// First we take care of flipping 8 bits
				pixelColorNumberFor4PixelsFromTileData =
					((pixelColorNumberFor4PixelsFromTileData >> EIGHT) | (pixelColorNumberFor4PixelsFromTileData << EIGHT));

				// Next we flip the 4 bits within the already flipped 8 bits

				pixelColorNumberFor4PixelsFromTileData =
					(((pixelColorNumberFor4PixelsFromTileData & 0xF0F0) >> FOUR)
						| ((pixelColorNumberFor4PixelsFromTileData & 0x0F0F) << FOUR));
			}

			BYTE pixelColorNumberPerPixel = ZERO;
			// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and paletee index is 1, when tile data is 1
			INC32 paletteIndex = ZERO;

			// pixel
			if (pGBA_display->currentBgPixelInTextMode[bgID] < (TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)))
			{
				pixelColorNumberPerPixel = ((pixelColorNumberFor4PixelsFromTileData >> (state * FOUR)) & 0x000F);
				paletteIndex = pixelColorNumberPerPixel << ONE;
				if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops <= ZERO)
				{
					if (paletteIndex != ZERO)
					{
						// Refer mode 2 bg processing to see how we determine sizeOfEachPaletteInBytes
						const DIM16 sizeOfEachPaletteInBytesInShifts = FIVE; // 1 << 5 is same as *32
						paletteIndex += (pGBA_display->bgCache[bgID].tileDescriptor.pb << sizeOfEachPaletteInBytesInShifts);
					}
					pGBA_display->gfx_bg[bgID][pGBA_display->currentBgPixelInTextMode[bgID]++][Y] = paletteIndex;
					++pGBA_display->bgCache[bgID].subTileIndexer;
				}
			}

			if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops > ZERO)
			{
				--pGBA_display->bgCache[bgID].subTileScrollingPixelDrops;
			}
		}
		else
		{
			// 8bpp mode

			GBA_HALFWORD pixelColorNumberFor2PixelsFromTileData = pixelData;;

			if (pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES)
			{
				// We only take care of flipping 8 bits
				pixelColorNumberFor2PixelsFromTileData =
					((pixelColorNumberFor2PixelsFromTileData >> EIGHT) | (pixelColorNumberFor2PixelsFromTileData << EIGHT));
			}

			BYTE pixelColorNumberPerPixel = ZERO;
			// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and paletee index is 1, when tile data is 1
			INC32 paletteIndex = ZERO;

			// pixel
			if (pGBA_display->currentBgPixelInTextMode[bgID] < (TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)))
			{
				pixelColorNumberPerPixel = ((pixelColorNumberFor2PixelsFromTileData >> (state * EIGHT)) & 0x00FF);
				paletteIndex = pixelColorNumberPerPixel << ONE;
				if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops <= ZERO)
				{
					pGBA_display->gfx_bg[bgID][pGBA_display->currentBgPixelInTextMode[bgID]++][Y] = paletteIndex;
					++pGBA_display->bgCache[bgID].subTileIndexer;
				}
			}

			if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops > ZERO)
			{
				--pGBA_display->bgCache[bgID].subTileScrollingPixelDrops;
			}
		}

	}

	MASQ_INLINE void MODE0_M_BG_CYCLE(ID bgID)
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixelInTextMode[bgID];
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
		pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

		GBA_WORD bgTileMapBaseAddr = ZERO;
		mBGnCNTHalfWord_t BGxCNT = { ZERO };

		// Cache BG configuration values for fast access
		auto getBGConfig = [&](ID id) -> std::tuple<int, FLAG, uint16_t, uint16_t, uint16_t>
			{
				switch (id)
				{
				case BG0:
					RETURN{
						static_cast<int>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
						pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord),
						static_cast<uint16_t>(pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				case BG1:
					RETURN{
						static_cast<int>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
						pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord),
						static_cast<uint16_t>(pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				case BG2:
					RETURN{
						static_cast<int>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
						pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord),
						static_cast<uint16_t>(pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				case BG3:
					RETURN{
						static_cast<int>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
						pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord),
						static_cast<uint16_t>(pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				default:
					// Safe fallback to avoid undefined behavior
					RETURN{ 0, false, 0, 0, 0 };
				}
			};

		auto [bgTileMapBaseAddr_, is8bppMode, BGxCNT_, hofs_, vofs_] = getBGConfig(bgID);
		bgTileMapBaseAddr = bgTileMapBaseAddr_;
		pGBA_display->bgCache[bgID].is8bppMode = is8bppMode;
		BGxCNT.mBGnCNTHalfWord = BGxCNT_;
		pGBA_display->bgCache[bgID].hofs = hofs_;
		pGBA_display->bgCache[bgID].vofs = vofs_;

		/*
		* Refer http://problemkaputt.de/gbatek-lcd-i-o-bg-control.htm
		* BGs always wraparound, so
		* offsets = BGxHOFS and BGxHVOFS (BG offsets within static screen)
		* Final offsets from screen (tileMap_x and tileMap_y) = (x + BGxHOFS) % 512 and (y + BGxHVOFS) % 512
		* Offsets in tile coordinates (tile_x and tile_y) = Final offsets from screen / 8
		* Offsets within tile coordinates (WithinTile_x and WithinTile_y) = Final offsets from screen % 8
		*
		* When BGxCNT->Size = 0
		* SC0 -> 256x256
		* When BGxCNT->Size = 1
		* SC0 SC1 -> 512x256
		* When BGxCNT->Size = 2
		* SC0 -> 256x512
		* SC1
		* When BGxCNT->Size = 3
		* SC0 SC1 -> 512x512
		* SC2 SC3
		* SC0 is defined by the normal BG Map base address (Bit 8-12 of BGxCNT),
		* SC1 uses same address +2K, SC2 address +4K, SC3 address +6K
		* So, for given x, y we need to figure out in which SCn it falls...
		* Then each SCn is 256x256, so , number of tiles i.e. tilesPerRow = bitsPerRow / 8 = 32
		* So, withinTileOffset = (tile_x + (tile_y * tilesPerRow))
		* But, we do a 16bit read for tile map as mentioned under Mode 0 in https://nba-emu.github.io/hw-docs/ppu/background.html and http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm unlike non text modes
		* So, 16withinTileOffset = 2 * withinTileOffset
		*
		* Final Address for tile map = [VRAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 2K) + (16withinTileOffset)]
		*/

		// Cached sum of coordinates and offsets
		DIM16 offsetSumX = xPixelCoordinate + pGBA_display->bgCache[bgID].hofs;
		DIM16 offsetSumY = yPixelCoordinate + pGBA_display->bgCache[bgID].vofs;

		ID SCn = ZERO;
		switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
		{
			// SC0
		case ZERO: SCn = ZERO; BREAK;
			// SC0 SC1
		case ONE: SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO; BREAK;
			// SC0
			// SC1
		case TWO: SCn = ((offsetSumY & 511) > 255) ? ONE : ZERO; BREAK;
			// SC0 SC1
			// SC2 SC3
		case THREE:
			SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO;
			SCn += ((offsetSumY & 511) > 255) ? TWO : ZERO;
			BREAK;
		default:
			FATAL("Unknown screen size in mode 0");
			RETURN;
		}

		// Within SCn
		DIM16 tileMapX = offsetSumX & 255; // % 256 because because if more than 255, it is SC1 or SC3, but we are already accounting for SCn, so we just get tileX within SCn
		DIM16 tileMapY = offsetSumY & 255; // % 256 because because if more than 255, it is SC2 or SC3, but we are already accounting for SCn, so we just get tileX within SCn

		pGBA_display->bgCache[bgID].tileMapX = tileMapX;
		pGBA_display->bgCache[bgID].tileMapY = tileMapY;

		const DIM16 tilesPerRowInShifts = FIVE; // 1 << 5 is same as * 32; Each SCn is of 256*256, so 256 / 8 = 32
		DIM16 tileMapOffset = (tileMapX >> THREE) + ((tileMapY >> THREE) << tilesPerRowInShifts);

		// Since we have 16 bits per tile instead of 8
		// Refer http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
		tileMapOffset <<= ONE;

		GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileMapArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
			PPUTODO("As per NBA, this is set to some latched value");
		}

		pGBA_display->bgCache[bgID].subTileIndexer = ZERO;

		if ((tileMapX & SEVEN) != ZERO)
		{
			PPUEVENT("Sub-tile scrolling in mode 0 bg%d!", bgID);

			// This indicates sub-tile scrolling as mentioned in https://nba-emu.github.io/hw-docs/ppu/background.html
			// Assume that tileMapX % 8 is 5, this indicates that only 8-5=3 pixels from the end of previous tile needs to be rendered
			// As mentioned above, only few pixels of the 1st and last tile would be visible in this case...
			// but still, ppu fetches all 8 pixels
			// So, during the previous MODE0_M_BG_CYCLE, we would have fetched the complete tile descriptor for the whole tile (all 8 pixels)
			// Using the example that we were just discussing, 3 pixels from the end needs to be rendered in X=0, X=1 and X=2
			// i.e. first 5 pixels eventhough fetched from VRAM needs to be dropped

			if (xPixelCoordinate == ZERO)
			{
				pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = tileMapX & SEVEN;
			}
			else
			{
				pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
			}
		}
		else
		{
			pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
		}

	}

	MASQ_INLINE void MODE0_T_BG_CYCLE(ID bgID)
	{

		auto X = pGBA_display->currentBgPixelInTextMode[bgID];
		auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Bounds check for bgID
		if (bgID < BG0 || bgID > BG3)
		{
			FATAL("Unknown BG in mode 0");
			RETURN;
		}

		if (ENABLED)
		{
			// Cache BG configuration values for fast access
			auto getBGConfig = [&](ID id) -> std::tuple<FLAG, uint16_t, uint16_t>
				{
					switch (id)
					{
					case BG0:
						RETURN{
							pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					case BG1:
						RETURN{
							pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					case BG2:
						RETURN{
							pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					case BG3:
						RETURN{
							pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					default:
						RETURN{ false, 0, 0 };
					}
				};

			auto [is8bppMode, hofs_, vofs_] = getBGConfig(bgID);
			pGBA_display->bgCache[bgID].is8bppMode = is8bppMode;
			pGBA_display->bgCache[bgID].hofs = hofs_;
			pGBA_display->bgCache[bgID].vofs = vofs_;
		}

		GBA_WORD bgTileDataBaseAddr =
			(bgID == BG0)
			? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
			: (bgID == BG1)
			? (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
			: (bgID == BG2)
			? (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
			: (pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

		auto xTileCoordinate = ((X + pGBA_display->bgCache[bgID].hofs) & 255) & SEVEN; // % 8
		auto yTileCoordinate = ((Y + pGBA_display->bgCache[bgID].vofs) & 255) & SEVEN; // % 8

		xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
			? flipLUT[xTileCoordinate]
			: xTileCoordinate;

		yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
			? flipLUT[yTileCoordinate]
			: yTileCoordinate;

		GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
		// Refer http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
		DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

		if (pGBA_display->bgCache[bgID].is8bppMode == NO)
		{
			// 4bpp mode
			GBA_WORD addressInTileDataArea =
				VIDEO_RAM_START_ADDRESS
				+ bgTileDataBaseAddr
				+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
				+ (withinTileDataOffset >> ONE); // Divide by 2 because in 4bpp mode, A single halfword gives data for 4 pixels

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = RESET;
				PPUTODO("As per NBA, this is set to some latched value");
			}

			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ZERO);
			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ONE);
			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, TWO);
			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, THREE);
		}
		else
		{
			// 8bpp mode
			GBA_WORD addressInTileDataArea =
				VIDEO_RAM_START_ADDRESS
				+ bgTileDataBaseAddr
				+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
				+ withinTileDataOffset;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = RESET;
				PPUTODO("As per NBA, this is set to some latched value");
			}

			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ZERO);
			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ONE);
		}

	}

	MASQ_INLINE void MODE1_M_TEXT_BG_CYCLE(ID bgID)
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixelInTextMode[bgID];
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Bounds check for bgID
		if (bgID < BG0 || bgID > BG1)
		{
			FATAL("Unknown BG in mode 1");
			RETURN;
		}

		pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
		pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

		GBA_WORD bgTileMapBaseAddr = ZERO;
		mBGnCNTHalfWord_t BGxCNT = { ZERO };

		if (bgID == BG0)
		{
			bgTileMapBaseAddr = (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
			pGBA_display->bgCache[BG0].is8bppMode = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
			BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord;
			pGBA_display->bgCache[BG0].hofs = pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET;
			pGBA_display->bgCache[BG0].vofs = pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET;
		}
		else if (bgID == BG1)
		{
			bgTileMapBaseAddr = (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
			pGBA_display->bgCache[BG1].is8bppMode = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
			BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord;
			pGBA_display->bgCache[BG1].hofs = pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET;
			pGBA_display->bgCache[BG1].vofs = pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET;
		}

		/*
		* Refer http://problemkaputt.de/gbatek-lcd-i-o-bg-control.htm
		* BGs always wraparound, so
		* offsets = BGxHOFS and BGxHVOFS (BG offsets within static screen)
		* Final offsets from screen (tileMap_x and tileMap_y) = (x + BGxHOFS) % 512 and (y + BGxHVOFS) % 512
		* Offsets in tile coordinates (tile_x and tile_y) = Final offsets from screen / 8
		* Offsets within tile coordinates (WithinTile_x and WithinTile_y) = Final offsets from screen % 8
		*
		* When BGxCNT->Size = 0
		* SC0 -> 256x256
		* When BGxCNT->Size = 1
		* SC0 SC1 -> 512x256
		* When BGxCNT->Size = 2
		* SC0 -> 256x512
		* SC1
		* When BGxCNT->Size = 3
		* SC0 SC1 -> 512x512
		* SC2 SC3
		* SC0 is defined by the normal BG Map base address (Bit 8-12 of BGxCNT),
		* SC1 uses same address +2K, SC2 address +4K, SC3 address +6K
		* So, for given x, y we need to figure out in which SCn it falls...
		* Then each SCn is 256x256, so , number of tiles i.e. tilesPerRow = bitsPerRow / 8 = 32
		* So, withinTileOffset = (tile_x + (tile_y * tilesPerRow))
		* But, we do a 16bit read for tile map as mentioned under Mode 0 in https://nba-emu.github.io/hw-docs/ppu/background.html and http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm unlike non text modes
		* So, 16withinTileOffset = 2 * withinTileOffset
		*
		* Final Address for tile map = [VRAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 2K) + (16withinTileOffset)]
		*
		*/

		// Cached sum of coordinates and offsets
		DIM16 offsetSumX = xPixelCoordinate + pGBA_display->bgCache[bgID].hofs;
		DIM16 offsetSumY = yPixelCoordinate + pGBA_display->bgCache[bgID].vofs;

		ID SCn = ZERO;
		switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
		{
			// SC0
		case ZERO: SCn = ZERO; BREAK;
			// SC0 SC1
		case ONE: SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO; BREAK;
			// SC0
			// SC1
		case TWO: SCn = ((offsetSumY & 511) > 255) ? ONE : ZERO; BREAK;
			// SC0 SC1
			// SC2 SC3
		case THREE:
			SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO;
			SCn += ((offsetSumY & 511) > 255) ? TWO : ZERO;
			BREAK;
		default:
			FATAL("Unknown screen size in mode 0");
			RETURN;
		}

		// Within SCn
		DIM16 tileMapX = offsetSumX & 255; // % 256 because because if more than 255, it is SC1 or SC3, but we are already accounting for SCn, so we just get tileX within SCn
		DIM16 tileMapY = offsetSumY & 255; // % 256 because because if more than 255, it is SC2 or SC3, but we are already accounting for SCn, so we just get tileX within SCn

		pGBA_display->bgCache[bgID].tileMapX = tileMapX;
		pGBA_display->bgCache[bgID].tileMapY = tileMapY;

		const DIM16 tilesPerRowInShifts = FIVE; // 1 << 5 is same as * 32; Each SCn is of 256*256, so 256 / 8 = 32
		DIM16 tileMapOffset = (tileMapX >> THREE) + ((tileMapY >> THREE) << tilesPerRowInShifts);

		// Since we have 16 bits per tile instead of 8
		// Refer http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
		tileMapOffset <<= ONE;

		GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileMapArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
			PPUTODO("As per NBA, this is set to some latched value");
		}

		pGBA_display->bgCache[bgID].subTileIndexer = ZERO;

		if ((tileMapX & SEVEN) != ZERO)
		{
			PPUEVENT("Sub-tile scrolling in mode 1 bg%d!", bgID);

			// This indicates sub-tile scrolling as mentioned in https://nba-emu.github.io/hw-docs/ppu/background.html
			// Assume that tileMapX % 8 is 5, this indicates that only 8-5=3 pixels from the end of previous tile needs to be rendered
			// As mentioned above, only few pixels of the 1st and last tile would be visible in this case...
			// but still, ppu fetches all 8 pixels
			// So, during the previous MODE0_M_BG_CYCLE, we would have fetched the complete tile descriptor for the whole tile (all 8 pixels)
			// Using the example that we were just discussing, 3 pixels from the end needs to be rendered in X=0, X=1 and X=2
			// i.e. first 5 pixels eventhough fetched from VRAM needs to be dropped

			if (xPixelCoordinate == ZERO)
			{
				pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = tileMapX & SEVEN;
			}
			else
			{
				pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
			}
		}
		else
		{
			pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
		}

	}

	MASQ_INLINE void MODE1_T_TEXT_BG_CYCLE(ID bgID)
	{

		auto X = pGBA_display->currentBgPixelInTextMode[bgID];
		auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Bounds check for bgID
		if (bgID < BG0 || bgID > BG1)
		{
			FATAL("Unknown BG in mode 1");
			RETURN;
		}

		if (ENABLED)
		{
			// Cache BG configuration values for fast access
			if (bgID == BG0)
			{
				pGBA_display->bgCache[BG0].is8bppMode = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
				pGBA_display->bgCache[BG0].hofs = pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET;
				pGBA_display->bgCache[BG0].vofs = pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET;
			}
			else if (bgID == BG1)
			{
				pGBA_display->bgCache[BG1].is8bppMode = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
				pGBA_display->bgCache[BG1].hofs = pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET;
				pGBA_display->bgCache[BG1].vofs = pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET;
			}
		}

		GBA_WORD bgTileDataBaseAddr = (bgID == BG0)
			? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
			: (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

		auto xTileCoordinate = ((X + pGBA_display->bgCache[bgID].hofs) & 255) & SEVEN; // % 8
		auto yTileCoordinate = ((Y + pGBA_display->bgCache[bgID].vofs) & 255) & SEVEN; // % 8

		xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
			? flipLUT[xTileCoordinate]
			: xTileCoordinate;

		yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
			? flipLUT[yTileCoordinate]
			: yTileCoordinate;

		GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
		// Refer http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
		DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

		if (pGBA_display->bgCache[bgID].is8bppMode == NO)
		{
			// 4bpp mode
			GBA_WORD addressInTileDataArea =
				VIDEO_RAM_START_ADDRESS
				+ bgTileDataBaseAddr
				+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
				+ (withinTileDataOffset >> ONE); // Divide by 2 because in 4bpp mode, A single halfword gives data for 4 pixels

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = RESET;
				PPUTODO("As per NBA, this is set to some latched value");
			}

			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ZERO);
			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ONE);
			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, TWO);
			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, THREE);
		}
		else
		{
			// 8bpp mode
			GBA_WORD addressInTileDataArea =
				VIDEO_RAM_START_ADDRESS
				+ bgTileDataBaseAddr
				+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
				+ withinTileDataOffset;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = RESET;
				PPUTODO("As per NBA, this is set to some latched value");
			}

			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ZERO);
			RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ONE);
		}

	}
#endif

	MASQ_INLINE void MODE1_M_AFFINE_BG_CYCLE()
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		GBA_WORD bgTileMapBaseAddr = (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
		DIM16 bgWidth = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);
		DIM16 bgHeight = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);
		FLAG isWrapAroundEnabled = (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT == ONE);

		uint32_t xPixelCoordinateAffine = (GBA_WORD)(pGBA_display->bgCache[BG2].affine.affineX >> EIGHT);	// To get the integer part
		uint32_t yPixelCoordinateAffine = (GBA_WORD)(pGBA_display->bgCache[BG2].affine.affineY >> EIGHT);	// To get the integer part

		// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
		// NOTE: In other modes, adding of PA and PC is done at the end of the pixel processing, but as per NBA, in mode 2, this is done at this point!
		pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
		pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);

		// Used for repetition of particular block of tiles 
		if (isWrapAroundEnabled == YES)
		{
			xPixelCoordinateAffine %= bgWidth;
			yPixelCoordinateAffine %= bgHeight;
		}

		pGBA_display->bgCache[BG2].xPixelCoordinateAffine = xPixelCoordinateAffine;
		pGBA_display->bgCache[BG2].yPixelCoordinateAffine = yPixelCoordinateAffine;

		uint32_t xTileCoordinateAffine = xPixelCoordinateAffine >> THREE;
		uint32_t yTileCoordinateAffine = yPixelCoordinateAffine >> THREE;

		auto bitsPerRow = bgWidth;
		auto tilesPerRow = bitsPerRow >> THREE;

		GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + xTileCoordinateAffine + (yTileCoordinateAffine * tilesPerRow);

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileMapArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[BG2].fetchedTileID = readRawMemory<BYTE>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			pGBA_display->bgCache[BG2].fetchedTileID = readRawMemory<BYTE>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
			PPUTODO("As per NBA, this is set to some latched value");
		}

	}

	MASQ_INLINE void MODE1_T_AFFINE_BG_CYCLE()
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		GBA_WORD bgTileDataBaseAddr = ZERO;
		DIM16 bgWidth = ZERO;
		DIM16 bgHeight = ZERO;
		FLAG isWrapAroundEnabled = NO;

		bgTileDataBaseAddr = (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);
		bgWidth = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);
		bgHeight = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);

		uint32_t xPixelCoordinateAffine = pGBA_display->bgCache[BG2].xPixelCoordinateAffine;
		uint32_t yPixelCoordinateAffine = pGBA_display->bgCache[BG2].yPixelCoordinateAffine;

		// As per http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
		// For Rotation/Scaling BG Screen -> In this mode, only 256 tiles can be used. There are no x/y-flip attributes, the color depth is always 256 colors/1 palette.
		// And as per http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
		// For 8bit depth (256 colors, 1 palette) -> Each tile occupies 64 bytes of memory, the first 8 bytes for the topmost row of the tile, and so on. Each byte selects the palette entry for each dot.

		uint32_t sizeOfEachTileData = 0x40;

		// As mentioned above, within 64 byte tile data, first 8 byte represents the colors for the 8 pixels within the top most row...and so on
		// so to find the offset with the 64 byte data...
		// withinTileOffset = 0 -> color for xTileCoordinate = 0, yTileCoordinate = 0
		// withinTileOffset = 1 -> color for xTileCoordinate = 1, yTileCoordinate = 0 
		//		:						 
		// withinTileOffset = 7 -> color for xTileCoordinate = 7, yTileCoordinate = 0 
		// withinTileOffset = 8 -> color for xTileCoordinate = 0, yTileCoordinate = 1 

		auto xTileCoordinateAffine = xPixelCoordinateAffine & SEVEN;
		auto yTileCoordinateAffine = yPixelCoordinateAffine & SEVEN;

		GBA_WORD withinTileDataOffsetAffine = xTileCoordinateAffine + (yTileCoordinateAffine << THREE);

		GBA_WORD addressInTileDataArea = VIDEO_RAM_START_ADDRESS + bgTileDataBaseAddr + (sizeOfEachTileData * pGBA_display->bgCache[BG2].fetchedTileID) + withinTileDataOffsetAffine;
		BYTE pixelColorNumberFromTileData = RESET;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			PPUTODO("As per NBA, pixelColorNumberFromTileData is set to some latched value");
		}

		ID paletteIndex = pixelColorNumberFromTileData << ONE; // Palette is 16bit data, (ex: tile id 1 should access 2nd and 3rd byte of palette ram) 

		// If out of bounds, needs to be tranparent, so palette index to 0
		if (xPixelCoordinateAffine >= bgWidth || yPixelCoordinateAffine >= bgHeight)
		{
			// Condition will be true only if wrap around was disabled!
			paletteIndex = ZERO;
		}

		// Now use the screen coordinates and fill the gfx buffer with data obtained from texture coordinates (Affine)
		pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = paletteIndex;

	}

	MASQ_INLINE void MODE0_BG_SEQUENCE(SSTATE32 state)
	{

		switch (state)
		{
		case ZERO:
		{
			MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_T_BG_CYCLE(BG0);
#endif
			BREAK;
		}
		case ONE:
		{
			MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_T_BG_CYCLE(BG1);
#endif
			BREAK;
		}
		case TWO:
		{
			MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_T_BG_CYCLE(BG2);
#endif
			BREAK;
		}
		case THREE:
		{
			MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_T_BG_CYCLE(BG3);
#endif
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case FOUR:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG0);
#endif
			MODE0_T_BG_CYCLE(BG0);
			BREAK;
		}
		case FIVE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG1);
#endif
			MODE0_T_BG_CYCLE(BG1);
			BREAK;
		}
		case SIX:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG2);
#endif
			MODE0_T_BG_CYCLE(BG2);
			BREAK;
		}
		case SEVEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG3);
#endif
			MODE0_T_BG_CYCLE(BG3);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case EIGHT:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG0);
			MODE0_T_BG_CYCLE(BG0);
#endif
			BREAK;
		}
		case NINE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG1);
			MODE0_T_BG_CYCLE(BG1);
#endif
			BREAK;
		}
		case TEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG2);
			MODE0_T_BG_CYCLE(BG2);
#endif
			BREAK;
		}
		case ELEVEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG3);
			MODE0_T_BG_CYCLE(BG3);
#endif
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWELVE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG0);
			MODE0_T_BG_CYCLE(BG0);
#else
			if (pGBA_display->bgCache[BG0].is8bppMode == YES)
			{
				MODE0_T_BG_CYCLE(BG0);
			}
#endif
			BREAK;
		}
		case THIRTEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG1);
			MODE0_T_BG_CYCLE(BG1);
#else
			if (pGBA_display->bgCache[BG1].is8bppMode == YES)
			{
				MODE0_T_BG_CYCLE(BG1);
			}
#endif
			BREAK;
		}
		case FOURTEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG2);
			MODE0_T_BG_CYCLE(BG2);
#else
			if (pGBA_display->bgCache[BG2].is8bppMode == YES)
			{
				MODE0_T_BG_CYCLE(BG2);
			}
#endif
			BREAK;
		}
		case FIFTEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG3);
			MODE0_T_BG_CYCLE(BG3);
#else
			if (pGBA_display->bgCache[BG3].is8bppMode == YES)
			{
				MODE0_T_BG_CYCLE(BG3);
			}
#endif
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case SIXTEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG0);
			MODE0_T_BG_CYCLE(BG0);
#endif
			BREAK;
		}
		case SEVENTEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG1);
			MODE0_T_BG_CYCLE(BG1);
#endif
			BREAK;
		}
		case EIGHTEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG2);
			MODE0_T_BG_CYCLE(BG2);
#endif
			BREAK;
		}
		case NINETEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG3);
			MODE0_T_BG_CYCLE(BG3);
#endif
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTY:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG0);
#endif
			MODE0_T_BG_CYCLE(BG0);
			BREAK;
		}
		case TWENTYONE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG1);
#endif
			MODE0_T_BG_CYCLE(BG1);
			BREAK;
		}
		case TWENTYTWO:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG2);
#endif
			MODE0_T_BG_CYCLE(BG2);
			BREAK;
		}
		case TWENTYTHREE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG3);
#endif
			MODE0_T_BG_CYCLE(BG3);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYFOUR:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG0);
			MODE0_T_BG_CYCLE(BG0);
#endif
			BREAK;
		}
		case TWENTYFIVE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG1);
			MODE0_T_BG_CYCLE(BG1);
#endif
			BREAK;
		}
		case TWENTYSIX:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG2);
			MODE0_T_BG_CYCLE(BG2);
#endif
			BREAK;
		}
		case TWENTYSEVEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG3);
			MODE0_T_BG_CYCLE(BG3);
#endif
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYEIGHT:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG0);
			MODE0_T_BG_CYCLE(BG0);
#else
			if (pGBA_display->bgCache[BG0].is8bppMode == YES)
			{
				MODE0_T_BG_CYCLE(BG0);
			}
#endif
			BREAK;
		}
		case TWENTYNINE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG1);
			MODE0_T_BG_CYCLE(BG1);
#else
			if (pGBA_display->bgCache[BG1].is8bppMode == YES)
			{
				MODE0_T_BG_CYCLE(BG1);
			}
#endif
			BREAK;
		}
		case THIRTY:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG2);
			MODE0_T_BG_CYCLE(BG2);
#else
			if (pGBA_display->bgCache[BG2].is8bppMode == YES)
			{
				MODE0_T_BG_CYCLE(BG2);
			}
#endif
			BREAK;
		}
		case THIRTYONE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE0_M_BG_CYCLE(BG3);
			MODE0_T_BG_CYCLE(BG3);
#else
			if (pGBA_display->bgCache[BG3].is8bppMode == YES)
			{
				MODE0_T_BG_CYCLE(BG3);
			}
#endif
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		default:
		{
			FATAL("PPU mode 0 (BG) out of sync");
		}
		}

	}

	MASQ_INLINE void MODE1_BG_SEQUENCE(SSTATE32 state)
	{

		switch (state)
		{
		case ZERO:
		{
			MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
			BREAK;
		}
		case ONE:
		{
			MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
			BREAK;
		}
		case TWO:
		{
			MODE1_M_AFFINE_BG_CYCLE();
			BREAK;
		}
		case THREE:
		{
			MODE1_T_AFFINE_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case FOUR:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG0);
#endif
			MODE1_T_TEXT_BG_CYCLE(BG0);
			BREAK;
		}
		case FIVE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG1);
#endif
			MODE1_T_TEXT_BG_CYCLE(BG1);
			BREAK;
		}
		case SIX:
		{
			MODE1_M_AFFINE_BG_CYCLE();
			BREAK;
		}
		case SEVEN:
		{
			MODE1_T_AFFINE_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case EIGHT:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG0);
			MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
			BREAK;
		}
		case NINE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG1);
			MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
			BREAK;
		}
		case TEN:
		{
			MODE1_M_AFFINE_BG_CYCLE();
			BREAK;
		}
		case ELEVEN:
		{
			MODE1_T_AFFINE_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWELVE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG0);
			MODE1_T_TEXT_BG_CYCLE(BG0);
#else
			if (pGBA_display->bgCache[BG0].is8bppMode == YES)
			{
				MODE1_T_TEXT_BG_CYCLE(BG0);
			}
#endif
			BREAK;
		}
		case THIRTEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG1);
			MODE1_T_TEXT_BG_CYCLE(BG1);
#else
			if (pGBA_display->bgCache[BG1].is8bppMode == YES)
			{
				MODE1_T_TEXT_BG_CYCLE(BG1);
			}
#endif
			BREAK;
		}
		case FOURTEEN:
		{
			MODE1_M_AFFINE_BG_CYCLE();
			BREAK;
		}
		case FIFTEEN:
		{
			MODE1_T_AFFINE_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case SIXTEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG0);
			MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
			BREAK;
		}
		case SEVENTEEN:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG1);
			MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
			BREAK;
		}
		case EIGHTEEN:
		{
			MODE1_M_AFFINE_BG_CYCLE();
			BREAK;
		}
		case NINETEEN:
		{
			MODE1_T_AFFINE_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTY:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG0);
#endif
			MODE1_T_TEXT_BG_CYCLE(BG0);
			BREAK;
		}
		case TWENTYONE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG1);
#endif
			MODE1_T_TEXT_BG_CYCLE(BG1);
			BREAK;
		}
		case TWENTYTWO:
		{
			MODE1_M_AFFINE_BG_CYCLE();
			BREAK;
		}
		case TWENTYTHREE:
		{
			MODE1_T_AFFINE_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYFOUR:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG0);
			MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
			BREAK;
		}
		case TWENTYFIVE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG1);
			MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
			BREAK;
		}
		case TWENTYSIX:
		{
			MODE1_M_AFFINE_BG_CYCLE();
			BREAK;
		}
		case TWENTYSEVEN:
		{
			MODE1_T_AFFINE_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYEIGHT:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG0);
			MODE1_T_TEXT_BG_CYCLE(BG0);
#else
			if (pGBA_display->bgCache[BG0].is8bppMode == YES)
			{
				MODE1_T_TEXT_BG_CYCLE(BG0);

			}
#endif
			BREAK;
		}
		case TWENTYNINE:
		{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
			MODE1_M_TEXT_BG_CYCLE(BG1);
			MODE1_T_TEXT_BG_CYCLE(BG1);
#else
			if (pGBA_display->bgCache[BG1].is8bppMode == YES)
			{
				MODE1_T_TEXT_BG_CYCLE(BG1);

			}
#endif
			BREAK;
		}
		case THIRTY:
		{
			MODE1_M_AFFINE_BG_CYCLE();
			BREAK;
		}
		case THIRTYONE:
		{
			MODE1_T_AFFINE_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		default:
		{
			FATAL("PPU mode 1 out of sync");
		}
		}

	}

	// ============================================
	// MODE 2 BACKGROUND RENDERING
	// ============================================

	MASQ_INLINE void MODE2_M_BG_CYCLE(ID bgID)
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		auto& bgCntHalfWord = (bgID == BG2)
			? pGBA_peripherals->mBG2CNTHalfWord
			: pGBA_peripherals->mBG3CNTHalfWord;

		GBA_WORD bgTileMapBaseAddr = bgCntHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800;
		DIM16 bgWidth = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;
		DIM16 bgHeight = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;
		FLAG isWrapAroundEnabled = bgCntHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT == ONE;

		uint32_t xPixelCoordinateAffine, yPixelCoordinateAffine;

		// Precompute affine coordinates and update them
		auto& bgCache = pGBA_display->bgCache[bgID];
		auto& affineX = bgCache.affine.affineX;
		auto& affineY = bgCache.affine.affineY;

		xPixelCoordinateAffine = affineX >> EIGHT;	// To get the integer part
		yPixelCoordinateAffine = affineY >> EIGHT;	// To get the integer part

		auto affineXIncrement =
			(bgID == BG2) ? pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed
			: pGBA_peripherals->mBG3PAHalfWord.mBGnPxHalfWord_Signed;
		auto affineYIncrement =
			(bgID == BG2) ? pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed
			: pGBA_peripherals->mBG3PCHalfWord.mBGnPxHalfWord_Signed;

		// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
		// NOTE: In other modes, adding of PA and PC is done at the end of the pixel processing, but as per NBA, in mode 2, this is done at this point!
		affineX += affineXIncrement;
		affineY += affineYIncrement;

		// Used for repetition of particular block of tiles 
		if (isWrapAroundEnabled == YES)
		{
			xPixelCoordinateAffine %= bgWidth;
			yPixelCoordinateAffine %= bgHeight;
		}

		bgCache.xPixelCoordinateAffine = xPixelCoordinateAffine;
		bgCache.yPixelCoordinateAffine = yPixelCoordinateAffine;

		uint32_t xTileCoordinateAffine = xPixelCoordinateAffine >> THREE;
		uint32_t yTileCoordinateAffine = yPixelCoordinateAffine >> THREE;

		auto bitsPerRow = bgWidth;
		auto tilesPerRow = bitsPerRow >> THREE;

		GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + xTileCoordinateAffine + (yTileCoordinateAffine * tilesPerRow);

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileMapArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[bgID].fetchedTileID = readRawMemory<BYTE>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);;
		}
		else
		{
			pGBA_display->bgCache[bgID].fetchedTileID = RESET;
			PPUTODO("As per NBA, this is set to some latched value");
		}

	}

	MASQ_INLINE void MODE2_T_BG_CYCLE(ID bgID)
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Use ternary operator for efficient branching
		auto& bgCntHalfWord = (bgID == BG2)
			? pGBA_peripherals->mBG2CNTHalfWord
			: pGBA_peripherals->mBG3CNTHalfWord;

		GBA_WORD bgTileDataBaseAddr = bgCntHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000;
		DIM16 bgWidth = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;
		DIM16 bgHeight = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;

		uint32_t xPixelCoordinateAffine = pGBA_display->bgCache[bgID].xPixelCoordinateAffine;
		uint32_t yPixelCoordinateAffine = pGBA_display->bgCache[bgID].yPixelCoordinateAffine;

		// As per http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
		// For Rotation/Scaling BG Screen -> In this mode, only 256 tiles can be used. There are no x/y-flip attributes, the color depth is always 256 colors/1 palette.
		// And as per http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
		// For 8bit depth (256 colors, 1 palette) -> Each tile occupies 64 bytes of memory, the first 8 bytes for the topmost row of the tile, and so on. Each byte selects the palette entry for each dot.

		const uint32_t sizeOfEachTileData = 0x40;

		// As mentioned above, within 64 byte tile data, first 8 byte represents the colors for the 8 pixels within the top most row...and so on
		// so to find the offset with the 64 byte data...
		// withinTileOffset = 0 -> color for xTileCoordinate = 0, yTileCoordinate = 0
		// withinTileOffset = 1 -> color for xTileCoordinate = 1, yTileCoordinate = 0 
		//		:						 
		// withinTileOffset = 7 -> color for xTileCoordinate = 7, yTileCoordinate = 0 
		// withinTileOffset = 8 -> color for xTileCoordinate = 0, yTileCoordinate = 1 

		auto xTileCoordinateAffine = xPixelCoordinateAffine & SEVEN;
		auto yTileCoordinateAffine = yPixelCoordinateAffine & SEVEN;

		GBA_WORD withinTileDataOffsetAffine = xTileCoordinateAffine + (yTileCoordinateAffine << THREE);

		GBA_WORD addressInTileDataArea = VIDEO_RAM_START_ADDRESS + bgTileDataBaseAddr + (sizeOfEachTileData * pGBA_display->bgCache[bgID].fetchedTileID) + withinTileDataOffsetAffine;

		BYTE pixelColorNumberFromTileData = RESET;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			PPUTODO("As per NBA, pixelColorNumberFromTileData is set to some latched value");
		}

		ID paletteIndex = pixelColorNumberFromTileData << ONE; // Palette is 16bit data, (ex: tile id 1 should access 2nd and 3rd byte of palette ram) 

		// If out of bounds, needs to be tranparent, so palette index to 0
		if (xPixelCoordinateAffine >= bgWidth || yPixelCoordinateAffine >= bgHeight)
		{
			// Condition will be true only if wrap around was disabled!
			paletteIndex = ZERO;
		}

		// Now use the screen coordinates and fill the gfx buffer with data obtained from texture coordinates (Affine)
		pGBA_display->gfx_bg[bgID][xPixelCoordinate][yPixelCoordinate] = paletteIndex;

	}

	MASQ_INLINE void MODE2_BG_SEQUENCE(SSTATE32 state)
	{

		switch (state)
		{
		case ZERO:
		{
			MODE2_M_BG_CYCLE(BG3);
			BREAK;
		}
		case ONE:
		{
			MODE2_T_BG_CYCLE(BG3);
			BREAK;
		}
		case TWO:
		{
			MODE2_M_BG_CYCLE(BG2);
			BREAK;
		}
		case THREE:
		{
			MODE2_T_BG_CYCLE(BG2);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case FOUR:
		{
			MODE2_M_BG_CYCLE(BG3);
			BREAK;
		}
		case FIVE:
		{
			MODE2_T_BG_CYCLE(BG3);
			BREAK;
		}
		case SIX:
		{
			MODE2_M_BG_CYCLE(BG2);
			BREAK;
		}
		case SEVEN:
		{
			MODE2_T_BG_CYCLE(BG2);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case EIGHT:
		{
			MODE2_M_BG_CYCLE(BG3);
			BREAK;
		}
		case NINE:
		{
			MODE2_T_BG_CYCLE(BG3);
			BREAK;
		}
		case TEN:
		{
			MODE2_M_BG_CYCLE(BG2);
			BREAK;
		}
		case ELEVEN:
		{
			MODE2_T_BG_CYCLE(BG2);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWELVE:
		{
			MODE2_M_BG_CYCLE(BG3);
			BREAK;
		}
		case THIRTEEN:
		{
			MODE2_T_BG_CYCLE(BG3);
			BREAK;
		}
		case FOURTEEN:
		{
			MODE2_M_BG_CYCLE(BG2);
			BREAK;
		}
		case FIFTEEN:
		{
			MODE2_T_BG_CYCLE(BG2);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case SIXTEEN:
		{
			MODE2_M_BG_CYCLE(BG3);
			BREAK;
		}
		case SEVENTEEN:
		{
			MODE2_T_BG_CYCLE(BG3);
			BREAK;
		}
		case EIGHTEEN:
		{
			MODE2_M_BG_CYCLE(BG2);
			BREAK;
		}
		case NINETEEN:
		{
			MODE2_T_BG_CYCLE(BG2);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTY:
		{
			MODE2_M_BG_CYCLE(BG3);
			BREAK;
		}
		case TWENTYONE:
		{
			MODE2_T_BG_CYCLE(BG3);
			BREAK;
		}
		case TWENTYTWO:
		{
			MODE2_M_BG_CYCLE(BG2);
			BREAK;
		}
		case TWENTYTHREE:
		{
			MODE2_T_BG_CYCLE(BG2);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYFOUR:
		{
			MODE2_M_BG_CYCLE(BG3);
			BREAK;
		}
		case TWENTYFIVE:
		{
			MODE2_T_BG_CYCLE(BG3);
			BREAK;
		}
		case TWENTYSIX:
		{
			MODE2_M_BG_CYCLE(BG2);
			BREAK;
		}
		case TWENTYSEVEN:
		{
			MODE2_T_BG_CYCLE(BG2);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYEIGHT:
		{
			MODE2_M_BG_CYCLE(BG3);
			BREAK;
		}
		case TWENTYNINE:
		{
			MODE2_T_BG_CYCLE(BG3);
			BREAK;
		}
		case THIRTY:
		{
			MODE2_M_BG_CYCLE(BG2);
			BREAK;
		}
		case THIRTYONE:
		{
			MODE2_T_BG_CYCLE(BG2);
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		default:
		{
			FATAL("PPU mode 2) out of sync");
		}
		}

	}

	// ============================================
	// MODE 3 BACKGROUND RENDERING (BITMAP)
	// ============================================

	MASQ_INLINE void MODE3_B_BG_CYCLE()
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Only BG2 is valid for Mode2

		// Refer to http://problemkaputt.de/gbatek-lcd-vram-bitmap-bg-modes.htm
		uint32_t xPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineX) >> EIGHT); // To get the integer part
		uint32_t yPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineY) >> EIGHT); // To get the integer part

		if (xPixelCoordinateAffine >= getScreenWidth() || yPixelCoordinateAffine >= getScreenHeight())
		{
			pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = ZERO;
		}
		else
		{
			ID vramIndex = xPixelCoordinateAffine + (yPixelCoordinateAffine * getScreenWidth());

			// Since we are accessing 16 bit data instead of 8 bit
			vramIndex <<= ONE;

			GBA_WORD vramAddress = VIDEO_RAM_START_ADDRESS + vramIndex;

			GBA_HALFWORD colorFor1Pixel = RESET;
			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (vramAddress < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				colorFor1Pixel = readRawMemory<GBA_HALFWORD>(vramAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

			}
			else
			{
				PPUTODO("As per NBA, colorFor1Pixel is set to some latched value");
			}

			pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = (colorFor1Pixel | IS_COLOR_NOT_PALETTE);
		}

		// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
		pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
		pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);

	}

	MASQ_INLINE void MODE3_BG_SEQUENCE(SSTATE32 state)
	{

		switch (state)
		{
		case THREE:
		{
			MODE3_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case SEVEN:
		{
			MODE3_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case ELEVEN:
		{
			MODE3_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case FIFTEEN:
		{
			MODE3_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case NINETEEN:
		{
			MODE3_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYTHREE:
		{
			MODE3_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYSEVEN:
		{
			MODE3_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case THIRTYONE:
		{
			MODE3_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		}

	}

	// ============================================
	// MODE 4 BACKGROUND RENDERING (BITMAP)
	// ============================================

	MASQ_INLINE void MODE4_B_BG_CYCLE()
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Only BG2 is valid for Mode2

		// Refer to http://problemkaputt.de/gbatek-lcd-vram-bitmap-bg-modes.htm
		uint32_t xPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineX) >> EIGHT); // To get the integer part
		uint32_t yPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineY) >> EIGHT); // To get the integer part

		if (xPixelCoordinateAffine >= getScreenWidth() || yPixelCoordinateAffine >= getScreenHeight())
		{
			pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = ZERO;
		}
		else
		{
			ID vramIndex = xPixelCoordinateAffine + (yPixelCoordinateAffine * getScreenWidth());

			GBA_WORD vramAddress = VIDEO_RAM_START_ADDRESS + (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FRAME_SELECT * 0xA000) + vramIndex;

			GBA_HALFWORD pixelColorNumberFromTileData = RESET;
			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (vramAddress < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pixelColorNumberFromTileData = readRawMemory<BYTE>(vramAddress, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);

			}
			else
			{
				PPUTODO("As per NBA, pixelColorNumberFromTileData is set to some latched value");
			}

			// NOTE: Palette index multiplied by 2 as each palette index represents 16 bit offset in palette ram instead of 8 bit offset, so basically there is 2 byte difference b/w palette index n to index n+1
			ID paletteIndex = pixelColorNumberFromTileData << ONE;

			pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = paletteIndex;
		}

		// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
		pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
		pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);

	}

	MASQ_INLINE void MODE4_BG_SEQUENCE(SSTATE32 state)
	{

		switch (state)
		{
		case THREE:
		{
			MODE4_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case SEVEN:
		{
			MODE4_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case ELEVEN:
		{
			MODE4_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case FIFTEEN:
		{
			MODE4_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case NINETEEN:
		{
			MODE4_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYTHREE:
		{
			MODE4_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYSEVEN:
		{
			MODE4_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case THIRTYONE:
		{
			MODE4_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		}

	}

	// ============================================
	// MODE 5 BACKGROUND RENDERING (BITMAP)
	// ============================================

	MASQ_INLINE void MODE5_B_BG_CYCLE()
	{

		uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
		uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

		// Early bounds check on X and Y
		if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		{
			RETURN;
		}

		// Only BG2 is valid for Mode2

		// Refer to http://problemkaputt.de/gbatek-lcd-vram-bitmap-bg-modes.htm
		uint32_t xPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineX) >> EIGHT); // To get the integer part
		uint32_t yPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineY) >> EIGHT); // To get the integer part

		if (xPixelCoordinateAffine >= ONEHUNDREDSIXTY || yPixelCoordinateAffine >= ONETWENTYEIGHT)
		{
			pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = ZERO;
		}
		else
		{
			ID vramIndex = xPixelCoordinateAffine + (yPixelCoordinateAffine * ONEHUNDREDSIXTY);

			// Since we are accessing 16 bit data instead of 8 bit
			vramIndex <<= ONE;

			GBA_WORD vramAddress = VIDEO_RAM_START_ADDRESS + (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FRAME_SELECT * 0xA000) + vramIndex;

			GBA_HALFWORD colorFor1Pixel = RESET;
			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (vramAddress < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				colorFor1Pixel = readRawMemory<GBA_HALFWORD>(vramAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

			}
			else
			{
				PPUTODO("As per NBA, colorFor1Pixel is set to some latched value");
			}

			pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = (colorFor1Pixel | IS_COLOR_NOT_PALETTE);
		}

		// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
		pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
		pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);

	}

	MASQ_INLINE void MODE5_BG_SEQUENCE(SSTATE32 state)
	{

		switch (state)
		{
		case THREE:
		{
			MODE5_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case SEVEN:
		{
			MODE5_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case ELEVEN:
		{
			MODE5_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case FIFTEEN:
		{
			MODE5_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case NINETEEN:
		{
			MODE5_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYTHREE:
		{
			MODE5_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case TWENTYSEVEN:
		{
			MODE5_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		case THIRTYONE:
		{
			MODE5_B_BG_CYCLE();
			++pGBA_display->currentBgPixel;
			BREAK;
		}
		}

	}

	// ============================================
	// MODE PROCESSING AND CONTROL
	// ============================================

	MASQ_INLINE void PROCESS_PPU_MODES(INC64 ppuCycles, FLAG renderBG, FLAG renderWindow, FLAG renderObj, FLAG renderMerge)
	{

		pGBA_display->currentPPUMode = pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE;

		// Handle mode specific processing
		if (ENABLED)
		{
			switch (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE)
			{
			case MODE0:
			{
				BREAK;
			}
			case MODE1:
			{
				BREAK;
			}
			case MODE2:
			{
				BREAK;
			}
			case MODE3:
			{
				BREAK;
			}
			case MODE4:
			{
				BREAK;
			}
			case MODE5:
			{
				BREAK;
			}
			default:
			{
				PPUWARN("Unknown PPU Mode");
				RETURN;
			}
			}
		}

		// Handle common processing
		if (ENABLED)
		{
			FLAG performWinRenderring = YES;
			FLAG performObjectRenderring = YES;
			FLAG performBgRenderring = YES;
			FLAG performMerging = YES;

			INC64 winCycles = ppuCycles;
			INC64 objCycles = ppuCycles;
			INC64 bgCycles = ppuCycles;
			INC64 mergeCycles = ppuCycles;

			const auto& currentScanline = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;
			auto& ppuCounter = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.ppuCounter;
			auto& currentMode = pGBA_display->currentPPUMode;

			ppuCounter += ppuCycles;

			if (pGBA_display->winWaitCyclesDone == NO && renderWindow == YES)
			{
				if (ppuCounter < WIN_WAIT_CYCLES)
				{
					performWinRenderring = NO;
				}
				else
				{
					winCycles = ppuCounter - WIN_WAIT_CYCLES;
					pGBA_display->winWaitCyclesDone = YES;
					performWinRenderring = YES;
				}
			}

			if (pGBA_display->objWaitCyclesDone == NO && renderObj == YES)
			{
				if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.ppuCounter < OBJECT_WAIT_CYCLES)
				{
					performObjectRenderring = (pGBA_display->allObjectsRenderedForScanline == NO) ? YES : NO;
				}
				else
				{
					if (
						pGBA_display->cyclesPerScanline > ZERO
						&&
						pGBA_display->allObjectsRenderedForScanline == NO
						&&
						pGBA_display->objAccessPatternState != OBJECT_ACCESS_PATTERN::OBJECT_A01
						)
					{
						if ((0x80 - pGBA_display->objAccessOAMIDState - ONE) > ZERO)
						{
							PPUMOREINFO("Missed %u objects in mode: %u; scanline %u", 0x80 - pGBA_display->objAccessOAMIDState - ONE, currentMode, pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY);
						}
						else
						{
							PPUMOREINFO("VRAM fsm still in mode: %u; scanline %u", currentMode, pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY);
						}
					}

					pGBA_display->allObjectsRenderedForScanline = NO;

					objCycles = ppuCounter - OBJECT_WAIT_CYCLES;
					pGBA_display->objWaitCyclesDone = YES;
					pGBA_display->objAccessOAMIDState = RESET;
					pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)].reset();
					pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)].reset();
					pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)].vcount = (SCOUNTER32)((currentScanline + ONE) % 228);
					pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
					pGBA_display->objAccessPattern = RESET;
					pGBA_display->cyclesPerSprite = RESET;
					pGBA_display->cyclesPerScanline = RESET;
					pGBA_display->vramCyclesStageForObjFSM = RESET;
					SET_INITIAL_OBJ_MODE();
					performObjectRenderring = YES;
				}
			}

			if (pGBA_display->bgWaitCyclesDone == NO && renderBG == YES)
			{
				COUNTER32 offsetForTextMode = ZERO;
				if (currentMode == MODE0 || currentMode == MODE1)
				{
					/*
					* Refer https://nba-emu.github.io/hw-docs/ppu/background.html
					* According to https://nba-emu.github.io/hw-docs/ppu/background.html#2, every BG waits 31 - 4 * (BG[x]HOFS mod 8) cycles in Mode 0 \n\
					* To simply the implementation, we are just going to assume the minimum amount of time, i.e. 31 - 4 * (max(BG[x]HOFS mod 8))
					* so, 32 - 4 * (7%8) = 32 - 4 * (7) = 35 - 28 = 4
					* Therefore offsetForTextMode = 28
					*/
					PPUTODO("Simplified version of bg wait cycles is implemented for text modes");
					offsetForTextMode = TWENTYEIGHT;
				}

				if (ppuCounter < (BG_WAIT_CYCLES - offsetForTextMode))
				{
					performBgRenderring = NO;
				}
				else
				{
					bgCycles = ppuCounter - (BG_WAIT_CYCLES - offsetForTextMode);
					pGBA_display->bgWaitCyclesDone = YES;
					performBgRenderring = YES;
				}
			}

			if (pGBA_display->mergeWaitCyclesDone == NO && renderMerge == YES)
			{
				if (ppuCounter < MERGE_WAIT_CYCLES)
				{
					performMerging = NO;
				}
				else
				{
					mergeCycles = ppuCounter - MERGE_WAIT_CYCLES;
					pGBA_display->mergeWaitCyclesDone = YES;
					performMerging = YES;
				}
			}

			INC64 currentCycles = ZERO;
			INC64 targetCycles = winCycles;

			if (performWinRenderring == YES && renderWindow == YES)
			{
				// Store the current mode and winAccessPatternState in local variables for efficiency
				auto& winState = pGBA_display->winAccessPatternState[currentMode];

				while (currentCycles < targetCycles)
				{
					if (winState == ZERO)
					{
						WIN_CYCLE();
					}

					// Advance the state (modulo 4)
					winState = (winState + ONE) & THREE;

					// next cycle...
					++currentCycles;
				}
			}

			PPUTODO("Optimize below code! We need to use a single while loop at line %d in %s", __LINE__, __FILE__);

			currentCycles = ZERO;
			targetCycles = objCycles;

			if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.SCREEN_DISP_OBJ == SET && performObjectRenderring == YES && renderObj == YES)
			{
				while ((currentCycles < targetCycles) && pGBA_display->allObjectsRenderedForScanline == NO)
				{
					++pGBA_display->cyclesPerSprite;
					++pGBA_display->cyclesPerScanline;

					if (pGBA_display->currentObjectIsAffine == OBJECT_TYPE::OBJECT_IS_AFFINE)
					{
						// FSM runs only on even cycles
						if ((pGBA_display->objAccessPattern & ONE) == RESET)
						{
							switch (pGBA_display->objAccessPatternState)
							{
							case OBJECT_ACCESS_PATTERN::OBJECT_A01:
							{
								pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
								if (pGBA_display->oamFoundValidObject == NO)
								{
									if (INCREMENT_OAM_ID() == INVALID)
									{
										pGBA_display->allObjectsRenderedForScanline = YES;
										pGBA_display->objAccessOAMIDState = RESET;
										pGBA_display->objAccessPattern = RESET;
										pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
									}
									pGBA_display->cyclesPerSprite = RESET;
								}
								else
								{
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
									pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
								}
								BREAK;
							}
							case OBJECT_ACCESS_PATTERN::OBJECT_A2:
							{
								OBJ_A2_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
								pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PA;
								BREAK;
							}
							case OBJECT_ACCESS_PATTERN::OBJECT_PA:
							{
								OBJ_PA_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
								pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PB;
								BREAK;
							}
							case OBJECT_ACCESS_PATTERN::OBJECT_PB:
							{
								OBJ_PB_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
								pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PC;
								BREAK;
							}
							case OBJECT_ACCESS_PATTERN::OBJECT_PC:
							{
								OBJ_PC_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
								pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PD;
								BREAK;
							}
							case OBJECT_ACCESS_PATTERN::OBJECT_PD:
							{
								OBJ_PD_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
								pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_BLANK_A01;
								BREAK;
							}
							case OBJECT_ACCESS_PATTERN::OBJECT_BLANK_A01:
							{
								pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState + ONE);
								pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_V;
								pGBA_display->firstVRAMCycleForObjFSM = YES;
								BREAK;
							}
							case OBJECT_ACCESS_PATTERN::OBJECT_V:
							{
								OBJ_V_OBJ_CYCLE(pGBA_display->objAccessOAMIDState, pGBA_display->currentObjectIsAffine, pGBA_display->vramCyclesStageForObjFSM);
								++pGBA_display->vramCyclesStageForObjFSM;
								if (pGBA_display->lastVRAMCycleForObjFSM == YES)
								{
									pGBA_display->lastVRAMCycleForObjFSM = NO;
									pGBA_display->vramCyclesStageForObjFSM = RESET;
									pGBA_display->cyclesPerSprite = RESET;
									if (INCREMENT_OAM_ID() == INVALID)
									{
										pGBA_display->allObjectsRenderedForScanline = YES;
										pGBA_display->objAccessOAMIDState = RESET;
										pGBA_display->objAccessPattern = RESET;
										pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
									}
									else
									{
										// valid if object is not disabled or we are in correct y coordinate for the object
										if (pGBA_display->oamFoundValidObject == NO)
										{
											if (INCREMENT_OAM_ID() == INVALID)
											{
												pGBA_display->allObjectsRenderedForScanline = YES;
												pGBA_display->objAccessOAMIDState = RESET;
												pGBA_display->objAccessPattern = RESET;
											}
											else
											{
												pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
												if (pGBA_display->oamFoundValidObject == NO)
												{
													if (INCREMENT_OAM_ID() == INVALID)
													{
														pGBA_display->allObjectsRenderedForScanline = YES;
														pGBA_display->objAccessOAMIDState = RESET;
														pGBA_display->objAccessPattern = RESET;
													}
													pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
												}
												else
												{
													pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
													pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
												}
											}
										}
										else
										{
											pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
											pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
										}
									}
								}
								BREAK;
							}
							default:
							{
								FATAL("Unknown Object Access Pattern State (Affine) in PPU Mode %d", currentMode);
							}
							}
						}

						++pGBA_display->objAccessPattern;
					}
					else if (pGBA_display->currentObjectIsAffine == OBJECT_TYPE::OBJECT_IS_NOT_AFFINE)
					{
						// FSM runs only on even cycles
						if ((pGBA_display->objAccessPattern & ONE) == RESET)
						{
							switch (pGBA_display->objAccessPatternState)
							{
							case OBJECT_ACCESS_PATTERN::OBJECT_A01:
							{
								pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
								if (pGBA_display->oamFoundValidObject == NO)
								{
									if (INCREMENT_OAM_ID() == INVALID)
									{
										pGBA_display->allObjectsRenderedForScanline = YES;
										pGBA_display->objAccessOAMIDState = RESET;
										pGBA_display->objAccessPattern = RESET;
										pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
									}
									pGBA_display->cyclesPerSprite = RESET;
								}
								else
								{
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
									pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
								}
								BREAK;
							}
							case OBJECT_ACCESS_PATTERN::OBJECT_A2:
							{
								OBJ_A2_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
								pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_V;
								pGBA_display->firstVRAMCycleForObjFSM = YES;
								BREAK;
							}
							case OBJECT_ACCESS_PATTERN::OBJECT_V:
							{
								OBJ_V_OBJ_CYCLE(pGBA_display->objAccessOAMIDState, pGBA_display->currentObjectIsAffine, pGBA_display->vramCyclesStageForObjFSM);
								++pGBA_display->vramCyclesStageForObjFSM;
								if (pGBA_display->firstVRAMCycleForObjFSM == YES)
								{
									pGBA_display->firstVRAMCycleForObjFSM = NO;
									pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState + ONE);
								}
								else if (pGBA_display->lastVRAMCycleForObjFSM == YES)
								{
									pGBA_display->lastVRAMCycleForObjFSM = NO;
									pGBA_display->vramCyclesStageForObjFSM = RESET;
									pGBA_display->cyclesPerSprite = RESET;
									if (INCREMENT_OAM_ID() == INVALID)
									{
										pGBA_display->allObjectsRenderedForScanline = YES;
										pGBA_display->objAccessOAMIDState = RESET;
										pGBA_display->objAccessPattern = RESET;
										pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
									}
									else
									{
										// valid if object is not disabled or we are in correct y coordinate for the object
										if (pGBA_display->oamFoundValidObject == NO)
										{
											if (INCREMENT_OAM_ID() == INVALID)
											{
												pGBA_display->allObjectsRenderedForScanline = YES;
												pGBA_display->objAccessOAMIDState = RESET;
												pGBA_display->objAccessPattern = RESET;
												pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
											}
											else
											{
												pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
												if (pGBA_display->oamFoundValidObject == NO)
												{
													if (INCREMENT_OAM_ID() == INVALID)
													{
														pGBA_display->allObjectsRenderedForScanline = YES;
														pGBA_display->objAccessOAMIDState = RESET;
														pGBA_display->objAccessPattern = RESET;
													}
													pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
												}
												else
												{
													pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
													pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
												}
											}
										}
										else
										{
											pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
											pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
										}
									}
								}
								BREAK;
							}
							default:
							{
								FATAL("Unknown Object Access Pattern State (Not Affine) in PPU Mode %d", currentMode);
							}
							}
						}

						++pGBA_display->objAccessPattern;
					}

					// next cycle...
					++currentCycles;
				}
			}

			currentCycles = ZERO;
			targetCycles = bgCycles;

			if (performBgRenderring == YES && renderBG == YES)
			{
#if (ENABLED)
				// Cache the state once before the loop to avoid repeated array access
				auto& bgState = pGBA_display->bgAccessPatternState[currentMode];

				switch (currentMode)
				{
				case MODE0:
				{
					while (currentCycles < targetCycles)
					{
						MODE0_BG_SEQUENCE(bgState);

						bgState = (bgState + ONE) & THIRTYONE;

						// next cycle...
						++currentCycles;
					}
					BREAK;
				}
				case MODE1:
				{
					while (currentCycles < targetCycles)
					{
						MODE1_BG_SEQUENCE(bgState);

						bgState = (bgState + ONE) & THIRTYONE;

						// next cycle...
						++currentCycles;
					}
					BREAK;
				}
				case MODE2:
				{
					while (currentCycles < targetCycles)
					{
						MODE2_BG_SEQUENCE(bgState);

						bgState = (bgState + ONE) & THIRTYONE;

						// next cycle...
						++currentCycles;
					}
					BREAK;
				}
				case MODE3:
				{
					while (currentCycles < targetCycles)
					{
						MODE3_BG_SEQUENCE(bgState);

						bgState = (bgState + ONE) & THIRTYONE;

						// next cycle...
						++currentCycles;
					}
					BREAK;
				}
				case MODE4:
				{
					while (currentCycles < targetCycles)
					{
						MODE4_BG_SEQUENCE(bgState);

						bgState = (bgState + ONE) & THIRTYONE;

						// next cycle...
						++currentCycles;
					}
					BREAK;
				}
				case MODE5:
				{
					while (currentCycles < targetCycles)
					{
						MODE5_BG_SEQUENCE(bgState);

						bgState = (bgState + ONE) & THIRTYONE;

						// next cycle...
						++currentCycles;
					}
					BREAK;
				}
				default:
				{
					FATAL("Unknown BG mode");
				}
				}
#else
				while (currentCycles < targetCycles)
				{
					switch (pGBA_display->bgAccessPatternState[currentMode])
					{
					case ZERO:
					{
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
						BREAK;
					}
					case ONE:
					{
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
						BREAK;
					}
					case TWO:
					{
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
						CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
						BREAK;
					}
					case THREE:
					{
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
						CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
						++pGBA_display->currentBgPixel;
						BREAK;
					}
					case FOUR:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
						BREAK;
					}
					case FIVE:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
						BREAK;
					}
					case SIX:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
						BREAK;
					}
					case SEVEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
						CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
						++pGBA_display->currentBgPixel;
						BREAK;
					}
					case EIGHT:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
						BREAK;
					}
					case NINE:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
						BREAK;
					}
					case TEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
						CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
						BREAK;
					}
					case ELEVEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
						CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
						++pGBA_display->currentBgPixel;
						BREAK;
					}
					case TWELVE:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
						BREAK;
					}
					case THIRTEEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
						BREAK;
					}
					case FOURTEEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
						BREAK;
					}
					case FIFTEEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
						CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
						++pGBA_display->currentBgPixel;
						BREAK;
					}
					case SIXTEEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
						BREAK;
					}
					case SEVENTEEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
						BREAK;
					}
					case EIGHTEEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
						CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
						BREAK;
					}
					case NINETEEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
						CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
						++pGBA_display->currentBgPixel;
						BREAK;
					}
					case TWENTY:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
						BREAK;
					}
					case TWENTYONE:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
						BREAK;
					}
					case TWENTYTWO:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
						BREAK;
					}
					case TWENTYTHREE:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
						CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
						++pGBA_display->currentBgPixel;
						BREAK;
					}
					case TWENTYFOUR:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
						BREAK;
					}
					case TWENTYFIVE:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
						BREAK;
					}
					case TWENTYSIX:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
						CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
						BREAK;
					}
					case TWENTYSEVEN:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
						CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
						++pGBA_display->currentBgPixel;
						BREAK;
					}
					case TWENTYEIGHT:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
						BREAK;
					}
					case TWENTYNINE:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
						BREAK;
					}
					case THIRTY:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
						BREAK;
					}
					case THIRTYONE:
					{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
						CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
						CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
						CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
						CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
						CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
						CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
						++pGBA_display->currentBgPixel;
						BREAK;
					}
					default:
					{
						FATAL("PPU mode %d (BG) out of sync", currentMode);
					}
					}

					++pGBA_display->bgAccessPatternState[currentMode];
					if (pGBA_display->bgAccessPatternState[currentMode] >= THIRTYTWO)
					{
						pGBA_display->bgAccessPatternState[currentMode] = ZERO;
					}

					// next cycle...
					++currentCycles;
				}
#endif
			}

			currentCycles = ZERO;
			targetCycles = mergeCycles;

			if (performMerging == YES && renderMerge == YES)
			{
				// Cache the state once before the loop to avoid repeated array access
				auto& mergeState = pGBA_display->mergeAccessPatternState[currentMode];

				while (currentCycles < targetCycles)
				{
					if (mergeState == ZERO)
					{
						MERGE_AND_DISPLAY_PHASE1();
					}
					else if (mergeState == TWO)
					{
						MERGE_AND_DISPLAY_PHASE2();
					}

					// Move to the next state and wrap it at 4
					mergeState = (mergeState + ONE) & THREE;

					// next cycle...
					++currentCycles;
				}
			}

		}

	}

	MASQ_INLINE void HANDLE_VCOUNT()
	{

		if (pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_SETTING_LYC == pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY)
		{
			if (
				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNTER_IRQ_ENABLE
				&&
				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_FLAG == RESET
				)
			{
				requestInterrupts(GBA_INTERRUPT::IRQ_VCOUNT);
			}

			pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_FLAG = SET;
		}
		else
		{
			pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_FLAG = RESET;
		}

	}

	void processPPU(INC64 ppuCycles);

	void displayCompleteScreen();

private:

	MASQ_INLINE void processSIO(INC64 sioCycles)
	{
		// Cache register access
		const auto& rcnt = pGBA_peripherals->mRCNTHalfWord.mRCNTFields;
		const auto& siocnt = pGBA_peripherals->mSIOCNT.mSIOFields;

		// Compute mode select once
		const ID modeSelect = (rcnt.MODE_SPECIFIC_3 << THREE) | (rcnt.MODE_SPECIFIC_2 << TWO)
			| (siocnt.BIT_13 << ONE) | siocnt.BIT_12;

		// Lookup table for SIO mode (faster than switch)
		static constexpr SIO_MODE MODE_LUT[16] = {
			SIO_MODE::NORMAL_8BIT,    // 0
			SIO_MODE::NORMAL_32BIT,   // 1
			SIO_MODE::MULTIPLAY_16BIT,// 2
			SIO_MODE::UART,           // 3
			SIO_MODE::NORMAL_8BIT,    // 4
			SIO_MODE::NORMAL_32BIT,   // 5
			SIO_MODE::MULTIPLAY_16BIT,// 6
			SIO_MODE::UART,           // 7
			SIO_MODE::GP,             // 8
			SIO_MODE::GP,             // 9
			SIO_MODE::GP,             // 10
			SIO_MODE::GP,             // 11
			SIO_MODE::JOYBUS,         // 12
			SIO_MODE::JOYBUS,         // 13
			SIO_MODE::JOYBUS,         // 14
			SIO_MODE::JOYBUS          // 15
		};

		const SIO_MODE sioMode = MODE_LUT[modeSelect];

		// Process based on mode
		if (sioMode == SIO_MODE::NORMAL_8BIT || sioMode == SIO_MODE::NORMAL_32BIT) MASQ_LIKELY
		{
			auto& sioCntSP = pGBA_peripherals->mSIOCNT.mSIOCNT_SPFields;

			if (sioCntSP.START_BIT != SET) MASQ_UNLIKELY
			{
				pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.sioCounter = RESET;
				RETURN;
			}

			if (static_cast<SIO_PARTY>(sioCntSP.SHIFT_CLK) == SIO_PARTY::MASTER) MASQ_LIKELY
			{
				pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.sioCounter += sioCycles;
			}

			const INC32 cyclesToTxRxData = (sioMode == SIO_MODE::NORMAL_8BIT) ? 64 : 256;  // 8*8 or 8*32

			if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.sioCounter >= cyclesToTxRxData) MASQ_UNLIKELY
			{
				pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.sioCounter -= cyclesToTxRxData;

				if (sioCntSP.IRQ_EN == SET) MASQ_LIKELY
				{
					requestInterrupts(GBA_INTERRUPT::IRQ_SERIAL);
				}

				sioCntSP.START_BIT = RESET;
			}
		}
		else if (sioMode == SIO_MODE::MULTIPLAY_16BIT) MASQ_UNLIKELY
		{
			// Empty implementation - only check interrupt flag
			// No actual processing needed
		}
		else if (sioMode == SIO_MODE::UART) MASQ_UNLIKELY
		{
			// Empty implementation - only check interrupt flag
			// No actual processing needed
		}
		// GP and JOYBUS modes have no processing
	}

private:

	// ============================================
	// AGGRESSIVE OPTIMIZATION - processBackup
	// 100% accuracy maintained
	// ============================================

	MASQ_INLINE void processBackup()
	{
		const BACKUP_TYPE backupType = pGBA_instance->GBA_state.emulatorStatus.backup.backupType;

		if (backupType != BACKUP_TYPE::FLASH64K && backupType != BACKUP_TYPE::FLASH128K) MASQ_UNLIKELY
			RETURN;

		auto& flash = pGBA_instance->GBA_state.emulatorStatus.backup.flash;
		const ID currentMemoryBank = (flash.currentMemoryBank == BACKUP_FLASH_MEMORY_BANK::BANK1) ? ONE : ZERO;
		auto* flashMem = pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank];

		const BACKUP_FLASH_FSM state = flash.flashFsmState;

		if (state == BACKUP_FLASH_FSM::STATE0) MASQ_LIKELY
		{
			if (flashMem[FLASH_ACCESS_MEMORY2 - GAMEPAK_SRAM_START_ADDRESS] == TO_UINT8(BACKUP_FLASH_CMDS::CMD_1)) MASQ_UNLIKELY
			{
				flash.flashFsmState = BACKUP_FLASH_FSM::STATE1;
				flashMem[FLASH_ACCESS_MEMORY2 - GAMEPAK_SRAM_START_ADDRESS] = RESET;
			}
		}
		else if (state == BACKUP_FLASH_FSM::STATE1) MASQ_UNLIKELY
		{
			if (flashMem[FLASH_ACCESS_MEMORY3 - GAMEPAK_SRAM_START_ADDRESS] == TO_UINT8(BACKUP_FLASH_CMDS::CMD_2)) MASQ_UNLIKELY
			{
				flash.flashFsmState = BACKUP_FLASH_FSM::STATE2;
				flashMem[FLASH_ACCESS_MEMORY3 - GAMEPAK_SRAM_START_ADDRESS] = RESET;
			}
		}
		else if (state == BACKUP_FLASH_FSM::STATE2) MASQ_UNLIKELY
		{
			// Handle 4KB erase
			if (flash.previousFlashCommand == BACKUP_FLASH_CMDS::START_ERASE_CMD
				&& flash.erase4kbPageNumber != INVALID) MASQ_UNLIKELY
			{
				flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;
				flash.erase4kbPageNumber = INVALID;

				const GBA_WORD eraseStartAddr = flash.erase4kbStartAddr;
				flash.erase4kbStartAddr = RESET;

				if (eraseStartAddr < GAMEPAK_SRAM_START_ADDRESS + 0x10000) MASQ_LIKELY
				{
					memset(&flashMem[eraseStartAddr - GAMEPAK_SRAM_START_ADDRESS], 0xFF, 0x1000);
					memset(&flash.isErased[currentMemoryBank][eraseStartAddr - GAMEPAK_SRAM_START_ADDRESS], YES, 0x1000);
				}
			}
			else
			{
				const BACKUP_FLASH_CMDS currentCmd =
					(BACKUP_FLASH_CMDS)flashMem[FLASH_ACCESS_MEMORY2 - GAMEPAK_SRAM_START_ADDRESS];

				flashMem[FLASH_ACCESS_MEMORY2 - GAMEPAK_SRAM_START_ADDRESS] = RESET;
				flash.currentFlashCommand = currentCmd;

				if (currentCmd == BACKUP_FLASH_CMDS::NO_OPERATION) MASQ_LIKELY
				{
					// Most common case - do nothing
				}
				else if (currentCmd == BACKUP_FLASH_CMDS::ENTER_CHIP_INDENTIFICATION_MODE) MASQ_UNLIKELY
				{
					flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

					if (flash.previousFlashCommand != BACKUP_FLASH_CMDS::START_ERASE_CMD)
					{
						flash.ogByteAtFlashAccessMem0 = flashMem[FLASH_ACCESS_MEMORY0 - GAMEPAK_SRAM_START_ADDRESS];
						flash.ogByteAtFlashAccessMem1 = flashMem[FLASH_ACCESS_MEMORY1 - GAMEPAK_SRAM_START_ADDRESS];

						if (backupType == BACKUP_TYPE::FLASH64K)
						{
							flashMem[ZERO] = 0x32;  // Panasonic
							flashMem[ONE] = 0x1B;
						}
						else
						{
							flashMem[ZERO] = 0x62;  // Sanyo
							flashMem[ONE] = 0x13;
						}
					}
				}
				else if (currentCmd == BACKUP_FLASH_CMDS::EXIT_CHIP_INDENTIFICATION_MODE) MASQ_UNLIKELY
				{
					flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

					if (flash.previousFlashCommand != BACKUP_FLASH_CMDS::START_ERASE_CMD)
					{
						flashMem[FLASH_ACCESS_MEMORY0 - GAMEPAK_SRAM_START_ADDRESS] = flash.ogByteAtFlashAccessMem0;
						flashMem[FLASH_ACCESS_MEMORY1 - GAMEPAK_SRAM_START_ADDRESS] = flash.ogByteAtFlashAccessMem1;
					}
				}
				else if (currentCmd == BACKUP_FLASH_CMDS::START_ERASE_CMD) MASQ_UNLIKELY
				{
					flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;
				}
				else if (currentCmd == BACKUP_FLASH_CMDS::ERASE_ENTIRE_CHIP) MASQ_UNLIKELY
				{
					flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

					if (flash.previousFlashCommand == BACKUP_FLASH_CMDS::START_ERASE_CMD)
					{
						if (backupType == BACKUP_TYPE::FLASH64K)
						{
							memset(flashMem, 0xFF, 0x10000);
							memset(flash.isErased[currentMemoryBank], YES, 0x10000);
						}
						else if (flash.currentMemoryBank == BACKUP_FLASH_MEMORY_BANK::BANK0)
						{
							memset(flashMem, 0xFF, 0x10000);
							memset(flash.isErased[currentMemoryBank], YES, 0x10000);
						}
						else
						{
							memset(&flashMem[0x10000], 0xFF, 0x10000);
							memset(&flash.isErased[currentMemoryBank][0x10000], YES, 0x10000);
						}
					}
				}
				else if (currentCmd == BACKUP_FLASH_CMDS::START_1BYTE_WRITE_CMD) MASQ_UNLIKELY
				{
					flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;
					flash.allowFlashWrite = YES;
				}
				else if (currentCmd == BACKUP_FLASH_CMDS::SET_MEMORY_BANK) MASQ_UNLIKELY
				{
					flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;
					flash.chooseMemoryBank = YES;
				}

				if (currentCmd != BACKUP_FLASH_CMDS::NO_OPERATION)
				{
					flash.previousFlashCommand = currentCmd;
				}
			}
		}
	}

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

public:

	bool runEmulationAtHostRate(uint32_t currentFrame) override;

	bool runEmulationLoopAtHostRate(uint32_t currentFrame) override;

	bool runEmulationAtFixedRate(uint32_t currentFrame) override;

	bool runEmulationLoopAtFixedRate(uint32_t currentFrame) override;

public:

	float getEmulationVolume() override;
	
	void setEmulationVolume(float volume) override;

public:

	void initializeGraphics();

	void initializeAudio();

public:

	bool initializeEmulator() override;

	void destroyEmulator() override;

public:

	bool getRomLoadedStatus() override;

	bool loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) override;

	void dumpRom() override;
#pragma endregion EMULATION_DEFINITIONS
};
#pragma endregion CORE