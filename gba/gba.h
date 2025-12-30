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
#define NOTHING_IN_PROGRESS								NO // If YES, this indicates that the developement of this particular emulator is complete
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

	enum class AUDIO_CHANNELS
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
		int64_t timerCounter[TIMER::TOTAL_TIMER];
		int64_t apuCounter;
		int64_t apuFrameCounter;
		int64_t ppuCounter;
		int64_t lcdCounter;
		int64_t sioCounter;
	} cycle_accurate_t;

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
		int64_t sioCounter;;
	} cycle_count_accurate_t;

	typedef struct
	{
		cycle_accurate_t cycle_accurate;
		cycle_count_accurate_t cycle_count_accurate;
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

	REGISTER_BANK_TYPE getRegisterBankFromOperatingMode(OP_MODE_TYPE opMode);

	REGISTER_BANK_TYPE getCurrentlyValidRegisterBank();

	// TODO: As of now, "getOperatingModeFromRegisterBank" function cannot differentiate between USR mode and SYS modes
	OP_MODE_TYPE getOperatingModeFromRegisterBank(REGISTER_BANK_TYPE rb);

private:

	void setARMState(STATE_TYPE armState);

	void setARMMode(OP_MODE_TYPE opMode);

	STATE_TYPE getARMState();

	OP_MODE_TYPE getARMMode();

private:

	void cpuSetRegister(REGISTER_BANK_TYPE rb, REGISTER_TYPE rt, STATE_TYPE st, uint32_t u32parameter);

	uint32_t cpuReadRegister(REGISTER_BANK_TYPE rb, REGISTER_TYPE rt);

	uint32_t getMemoryAccessCycles(GBA_WORD mCurrentAddress, MEMORY_ACCESS_WIDTH mAccessWidth, MEMORY_ACCESS_SOURCE mSource, MEMORY_ACCESS_TYPE accessType);

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
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.timerCounter[timer] = RESET;
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
			dmaCyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter = RESET;
			// All currenlty enabled DMA transactions should be complete by the time we come here
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.freeBusCyclesCounter = RESET;
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
			dmaCyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter = RESET;
			// All currenlty enabled DMA transactions should be complete by the time we come here
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.freeBusCyclesCounter = RESET;
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

	bool TickMultiply(FLAG isSigned, uint64_t multiplier);

	bool MultiplyCarrySimple(uint32_t multiplier);

	bool MultiplyCarryLo(
		uint32_t multiplicand,
		uint32_t multiplier,
		uint32_t accum = 0
	);

	bool MultiplyCarryHi(
		bool sign_extend,
		uint32_t multiplicand,
		uint32_t multiplier,
		uint32_t accum_hi = 0
	);

	bool didConditionalCheckPass(uint32_t opCodeConditionalBits, uint32_t cpsr);

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

#pragma region CYCLE_ACCURATE
	void cpuTick(TICK_TYPE type = TICK_TYPE::CPU_TICK);

	void syncOtherGBAModuleTicks();

	void timerTick();

	void dmaTick();

	void serialTick();

	void rtcTick();

	void apuTick();

	void ppuTick();
#pragma endregion CYCLE_ACCURATE

#pragma region CYCLE_COUNT_ACCURATE

#pragma endregion CYCLE_COUNT_ACCURATE

private:

	void requestInterrupts(GBA_INTERRUPT interrupt);

	bool shouldUnHaltTheCPU();

	bool isInterruptReadyToBeServed();

	bool handleInterruptsIfApplicable();

private:

	void handleKeypadInterrupts();

	void captureIO();

private:

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

	DIM16 getChannelPeriod(AUDIO_CHANNELS channel);

	FLAG enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS channel);

	void continousDACCheck();

	FLAG isDACEnabled(AUDIO_CHANNELS channel);

	FLAG isAudioChannelEnabled(AUDIO_CHANNELS channel);

	FLAG isChannel3Active();

	void tickChannel(AUDIO_CHANNELS channel, INC64 tCycles);

	void processSoundLength();

	SDIM32 getUpdatedFrequency();

	void processFrequencySweep();

	void processEnvelopeSweep();

	GBA_AUDIO_SAMPLE_TYPE getAmplitude(AUDIO_CHANNELS channel);

	GBA_AUDIO_SAMPLE_TYPE getDACOutput(AUDIO_CHANNELS channel);

	void captureDownsampledAudioSamples(INC64 sampleCount);

	void processAPU(INC64 apuCycles);

	void playTheAudioFrame();

private:

	// ============================================
	// PIXEL AND WINDOW OPERATIONS
	// ============================================

	MASQ_INLINE void RESET_PIXEL(uint32_t x, uint32_t y);

	MASQ_INLINE FLAG GET_WINDOW_OUTPUT(uint32_t x, uint32_t y,
		FLAG win0in, FLAG win1in,
		FLAG winout, FLAG objin);

	MASQ_INLINE void HANDLE_WINDOW_FOR_BG(uint32_t x, uint32_t y, ID bgID);

	MASQ_INLINE void HANDLE_WINDOW_FOR_OBJ(uint32_t x, uint32_t y);

	MASQ_INLINE FLAG DOES_WINDOW_ALLOW_BLENDING(uint32_t x, uint32_t y);

	// ============================================
	// COLOR BLENDING OPERATIONS
	// ============================================

	MASQ_INLINE gbaColor_t BLEND(gbaColor_t layer1Pixel, gbaColor_t layer2Pixel,
		BYTE eva, BYTE evb);

	MASQ_INLINE gbaColor_t BRIGHTEN(gbaColor_t color, BYTE evy);

	MASQ_INLINE gbaColor_t DARKEN(gbaColor_t color, BYTE evy);

	// ============================================
	// MERGE AND DISPLAY
	// ============================================

	MASQ_INLINE void MERGE_AND_DISPLAY_PHASE1();

	MASQ_INLINE void MERGE_AND_DISPLAY_PHASE2();

	// ============================================
	// OBJECT (SPRITE) RENDERING
	// ============================================

	MASQ_INLINE void SET_INITIAL_OBJ_MODE();

	MASQ_INLINE FLAG OBJ_A01_OBJ_CYCLE(ID oamID);

	MASQ_INLINE void OBJ_A2_OBJ_CYCLE(ID oamID);

	MASQ_INLINE void OBJ_PA_OBJ_CYCLE(ID oamID);

	MASQ_INLINE void OBJ_PB_OBJ_CYCLE(ID oamID);

	MASQ_INLINE void OBJ_PC_OBJ_CYCLE(ID oamID);

	MASQ_INLINE void OBJ_PD_OBJ_CYCLE(ID oamID);

	MASQ_INLINE void OBJ_V_OBJ_CYCLE(ID oamID, OBJECT_TYPE isAffine, STATE8 state);

	MASQ_INLINE int32_t INCREMENT_OAM_ID();

	// ============================================
	// WINDOW RENDERING
	// ============================================

	MASQ_INLINE void WIN_CYCLE();

	// ============================================
	// MODE 0 BACKGROUND RENDERING
	// ============================================

	MASQ_INLINE void MODE0_M_BG_CYCLE(ID bgID);
	MASQ_INLINE void MODE0_T_BG_CYCLE(ID bgID);
	MASQ_INLINE void RENDER_MODE0_MODE1_PIXEL_X(ID bgID, GBA_HALFWORD pixelData, STATE8 state);
	MASQ_INLINE void MODE0_BG_SEQUENCE(SSTATE32 state);

	// ============================================
	// MODE 1 BACKGROUND RENDERING
	// ============================================

	MASQ_INLINE void MODE1_M_TEXT_BG_CYCLE(ID bgID);
	MASQ_INLINE void MODE1_T_TEXT_BG_CYCLE(ID bgID);
	MASQ_INLINE void MODE1_M_AFFINE_BG_CYCLE();
	MASQ_INLINE void MODE1_T_AFFINE_BG_CYCLE();
	MASQ_INLINE void MODE1_BG_SEQUENCE(SSTATE32 state);

	// ============================================
	// MODE 2 BACKGROUND RENDERING
	// ============================================

	MASQ_INLINE void MODE2_M_BG_CYCLE(ID bgID);
	MASQ_INLINE void MODE2_T_BG_CYCLE(ID bgID);
	MASQ_INLINE void MODE2_BG_SEQUENCE(SSTATE32 state);

	// ============================================
	// MODE 3 BACKGROUND RENDERING (BITMAP)
	// ============================================

	MASQ_INLINE void MODE3_B_BG_CYCLE();
	MASQ_INLINE void MODE3_BG_SEQUENCE(SSTATE32 state);

	// ============================================
	// MODE 4 BACKGROUND RENDERING (BITMAP)
	// ============================================

	MASQ_INLINE void MODE4_B_BG_CYCLE();
	MASQ_INLINE void MODE4_BG_SEQUENCE(SSTATE32 state);

	// ============================================
	// MODE 5 BACKGROUND RENDERING (BITMAP)
	// ============================================

	MASQ_INLINE void MODE5_B_BG_CYCLE();
	MASQ_INLINE void MODE5_BG_SEQUENCE(SSTATE32 state);

	// ============================================
	// MODE PROCESSING AND CONTROL
	// ============================================

	MASQ_INLINE void PROCESS_PPU_MODES(INC64 ppuCycles, FLAG renderBG,
		FLAG renderWindow, FLAG renderObj,
		FLAG renderMerge);

	MASQ_INLINE void HANDLE_VCOUNT();

	void processPPU(INC64 ppuCycles);

	void displayCompleteScreen();

private:

	void processSIO(INC64 sioCycles);

private:

	void processBackup();

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