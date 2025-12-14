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
#define PACMAN_FPS										60.0f
#define PACMAN_AUDIO_SAMPLING_RATE						96000.0f
#define EMULATED_AUDIO_SAMPLING_RATE_FOR_PACMAN			(PACMAN_AUDIO_SAMPLING_RATE)
#ifdef __EMSCRIPTEN__
#define AUDIO_BUFFER_SIZE_FOR_PACMAN					(CEIL((EMULATED_AUDIO_SAMPLING_RATE_FOR_PACMAN / PACMAN_FPS))) // 32
#else
#define AUDIO_BUFFER_SIZE_FOR_PACMAN					(CEIL((EMULATED_AUDIO_SAMPLING_RATE_FOR_PACMAN / PACMAN_FPS)))
#endif

#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif
#pragma endregion MACROS

#pragma region CORE
class pacMan_t : public abstractEmulation_t
{
#pragma region INFRASTRUCTURE_DECLARATIONS
public:

	std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom;
	const float myFPS = (float)PACMAN_FPS;

public:

	static const uint32_t screen_height = 288;
	static const uint32_t screen_width = 256;
	static const uint32_t pixel_height = 2;
	static const uint32_t pixel_width = 2;
	const char* NAME = "PacMan";

private:

	boost::property_tree::ptree pt;

private:

	uint8_t const MS_PAC_MAN_ROMS = 13;
	uint8_t const PAC_MAN_ROMS = 10;
	uint8_t const TEST_ROMS = 1;
	uint8_t const SST_ROMS = 2;

	FLAG testFinished = false;
	uint64_t testROMCycles = 0;

#pragma endregion INFRASTRUCTURE_DECLARATIONS

#pragma region Z80_DECLARATION
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
		RT_I,				// 8
		RT_R,				// 9
		RT_IXH,				// 10
		RT_IXL,				// 11
		RT_IYH,				// 12
		RT_IYL,				// 13
		RT_PC,				// 14
		RT_SP,				// 15
		RT_IX,				// 16
		RT_IY,				// 17
		RT_AF,				// 18
		RT_BC,				// 19
		RT_DE,				// 20
		RT_HL,				// 21
		RT_IE,				// 22
		RT_SHADOW_A,		// 23
		RT_SHADOW_F,		// 24
		RT_SHADOW_B,		// 25
		RT_SHADOW_C,		// 26
		RT_SHADOW_D,		// 27
		RT_SHADOW_E,		// 28
		RT_SHADOW_H,		// 29
		RT_SHADOW_L,		// 30
		RT_SHADOW_AF,		// 31
		RT_SHADOW_BC,		// 32
		RT_SHADOW_DE,		// 33
		RT_SHADOW_HL,		// 34
		RT_WZ,				// 35
		RT_P,				// 36
		RT_Q,				// 37
		RT_IFF1,			// 38
		RT_IFF2,			// 39
		RT_TOTAL,			// TOTAL = 40
		RT_NONE
	};

	enum class POINTER_TYPE
	{
		RT_M_HL,			// 0
		RT_M_DE,			// 1
		RT_M_BC,			// 2
		RT_M_WZ,			// 3
		RT_M_TOTAL,			// TOTAL = 4
		RT_M_NONE
	};

private:

#pragma pack(push, 1)

	typedef struct
	{
		uint8_t opcode;
		uint8_t prefix1;
		uint8_t prefix2;
		uint8_t port;
		FLAG possFlag;
		uint64_t cpuCounter;
	} cpu_t;

	typedef struct
	{
		uint8_t FCARRY : 1; // bit  0
		uint8_t FNEGATIVE : 1; // bit  1
		uint8_t F_OF_PARITY : 1; // bit  2
		uint8_t THIRD : 1; // bit  3
		uint8_t FHALFCARRY : 1; // bit  4
		uint8_t FIFTH : 1; // bit  5	
		uint8_t FZERO : 1; // bit  6
		uint8_t FSIGN : 1; // bit  7
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
		uint8_t ixl;				// 0 - 7
		uint8_t ixh;				// 8 - 15
	} ixRegisters_t;

	typedef union
	{
		ixRegisters_t ixRegisters;
		uint16_t ix_u16memory;	// <-ixh->|<-ixl->
	} ix_t;

	typedef struct
	{
		uint8_t iyl;				// 0 - 7
		uint8_t iyh;				// 8 - 15
	} iyRegisters_t;

	typedef union
	{
		iyRegisters_t iyRegisters;
		uint16_t iy_u16memory;	// <-iyh->|<-iyl->
	} iy_t;

	typedef struct
	{
		uint8_t i;
		uint8_t r;
		af_t af;
		bc_t bc;
		de_t de;
		hl_t hl;
		ix_t ix;
		iy_t iy;
		uint16_t pc;
		uint16_t sp;
		af_t shadow_af;
		bc_t shadow_bc;
		de_t shadow_de;
		hl_t shadow_hl;
		uint16_t wz;
		uint8_t p;
		uint8_t q;
		uint8_t iff1;
		uint8_t iff2;
	} registers_t;
#pragma endregion Z80_DECLARATION

#pragma region EMULATION_DECLARATIONS
private:

	enum class INTERRUPT_MODE
	{
		INTERRUPT_MODE_0 = 0,
		INTERRUPT_MODE_1,
		INTERRUPT_MODE_2,
	};

private:

	// pacman.6e 0000 - 0FFF
	// pacman.6f 1000 - 1FFF
	// pacman.6h 2000 - 2FFF
	// pacman.6j 3000 - 3FFF

	typedef struct
	{
		BYTE code_rom_6e[0x1000];
		BYTE code_rom_6f[0x1000];
		BYTE code_rom_6h[0x1000];
		BYTE code_rom_6j[0x1000];
	} codeRomFields_t;

	typedef union
	{
		codeRomFields_t codeRomFields;
		BYTE codeRomMemory[sizeof(codeRomFields_t)];
	} codeRomMemory_t;

	typedef struct
	{
		uint8_t videoRam[0x0400];
		uint8_t colorRam[0x0400];
	} vramFields_t;

	typedef union
	{
		vramFields_t vramFields;
		uint8_t vramMemory[sizeof(vramFields_t)];
	} vramMemory_t;

	typedef struct
	{
		uint8_t YFlip : 1; // bit  0
		uint8_t XFlip : 1; // bit  1
		uint8_t spriteNumber : 6; // bit  2 - bit 7	
	} spriteMemByte1Fields_t;

	typedef union
	{
		spriteMemByte1Fields_t spriteMemByte1Fields;
		BYTE spriteMemByte1_t[sizeof(spriteMemByte1Fields_t)];
	} spriteMemByte1_t;

	typedef struct
	{
		spriteMemByte1_t spriteMemByte1;
		BYTE spritePalette;
	} spriteMemory_t;

	typedef struct
	{
		uint8_t UP1 : 1; // bit  0
		uint8_t LEFT1 : 1; // bit  1
		uint8_t RIGHT1 : 1; // bit  2	
		uint8_t DOWN1 : 1; // bit  3
		uint8_t RACK_ADV : 1; // bit  4
		uint8_t COIN1 : 1; // bit  5	
		uint8_t COIN2 : 1; // bit  6
		uint8_t CREDIT : 1; // bit  7
	} IN0Fields_t;

	typedef union
	{
		IN0Fields_t IN0Fields[0x40];
		BYTE IN0Memory[0x40];
	} IN0Memory_t;

	typedef struct
	{
		BYTE interruptEnable;	// bit 0: 0 = disabled, 1 = enabled
		BYTE soundEnable;		// bit 0: 0 = disabled, 1 = enabled
		BYTE auxBoardEnable;
		BYTE flipScreen;		// bit 0: 0 = normal, 1 = flipped
		BYTE P1StartLamp;		// bit 0: 0 = on, 1 = off
		BYTE P2StartLamp;		// bit 0: 0 = on, 1 = off
		BYTE coinLockOut;		// bit 0: 0 = unlocked, 1 = locked
		BYTE coinCounter;		// bit 0: trigerred by 0 -> 1  
		BYTE SPARE[0x38];
	} miscellaneousFields_t;

	typedef union
	{
		miscellaneousFields_t miscellaneousFields;
	} miscellaneousMemoryMap_t;

	typedef struct
	{
		uint8_t UP2 : 1; // bit  0
		uint8_t LEFT2 : 1; // bit  1
		uint8_t RIGHT2 : 1; // bit  2	
		uint8_t DOWN2 : 1; // bit  3
		uint8_t BOARDTEST : 1; // bit  4
		uint8_t START1 : 1; // bit  5	
		uint8_t START2 : 1; // bit  6
		uint8_t CABINET : 1; // bit  7
	} IN1Fields_t;

	typedef union
	{
		IN1Fields_t IN1Fields[0x40];
		BYTE IN1Memory[0x40];
	} IN1Memory_t;

	typedef struct
	{
		BYTE x;
		BYTE y;
	} spriteCoordinate_t;

	typedef struct
	{
		BYTE SV1_accumulator[5];
		BYTE SV1_waveform;
		BYTE SV2_accumulator[4];
		BYTE SV2_waveform;
		BYTE SV3_accumulator[4];
		BYTE SV3_waveform;
		BYTE V1_frequency[5];
		BYTE V1_volume;
		BYTE V2_frequency[4];
		BYTE V2_volume;
		BYTE V3_frequency[4];
		BYTE V3_volume;
		spriteCoordinate_t spriteCoordinate[8];
		BYTE SPARE[0x10];
	} audioANDSpriteCoordinates_t;

	typedef union
	{
		audioANDSpriteCoordinates_t audioANDSpriteCoordinates;
	} audioANDSpriteCoordinatesMemoryMap_t;

	typedef struct
	{
		uint8_t DIP0 : 1; // bit  0
		uint8_t DIP1 : 1; // bit  1
		uint8_t DIP2 : 1; // bit  2	
		uint8_t DIP3 : 1; // bit  3
		uint8_t DIP4 : 1; // bit  4
		uint8_t DIP5 : 1; // bit  5	
		uint8_t SOLDER2 : 1; // bit  6
		uint8_t SOLDER1 : 1; // bit  7
	} DIP_t;

	typedef union
	{
		DIP_t DIP[0x40];
		uint8_t DIPMemory[0x40];
	} DIPMemory_t;

	typedef struct
	{
		BYTE auxCode_rom_u5[0x0800];
		BYTE SPARE0[0x0800];
		BYTE auxCode_rom_u6[0x1000];
		BYTE SPARE1[0x1000];
		BYTE auxCode_rom_u7[0x1000];
	} auxillaryCodeRomFields_t;

	typedef union
	{
		auxillaryCodeRomFields_t auxillaryCodeRomFields;
		BYTE auxillaryCodeRomMemory[sizeof(auxillaryCodeRomFields_t)];
	} auxillaryCodeRomMemory_t;

	typedef union
	{
		codeRomMemory_t mCodeRomPatched;
	} patchedMemory_t;

	typedef struct
	{
		codeRomMemory_t mCodeRom;													// 0x0000 - 0x3FFF	|	^
		vramMemory_t mVram;															// 0x4000 - 0x47FF  v	|		
		BYTE mRam[0x07F0];															// 0x4800 - 0x4FEF  |	|
		spriteMemory_t spriteMemory[8];												// 0x4FF0 - 0x4FFF  v	|
		miscellaneousMemoryMap_t miscellaneousMemoryMap;							// 0x5000 - 0x503F	|	|
		audioANDSpriteCoordinatesMemoryMap_t audioANDSpriteCoordinatesMemoryMap;	// 0x5040 - 0x507F	v	|	Zex Roms
		DIPMemory_t DIPMemory;														// 0x5080 - 0x50BF	|	|
		BYTE watchDogReset[64];														// 0x50C0 - 0x50FF	v	|
		BYTE SPARE[0x2F00];													// PacMan MsPacMan Demarcation	|
		auxillaryCodeRomMemory_t mAuxCodeRom;										// 0x8000 - 0xBFFF  ^	|
		patchedMemory_t mPatched;											// Virtual 0x0000 - 0x3FFF  |	v
	} memoryMap_t;

	typedef union
	{
		memoryMap_t pacManMemoryMap;
		BYTE pacManRawMemory[sizeof(memoryMap_t)];
	} pacManMemory_t;

	typedef struct
	{
		BYTE sound_rom1[0x0100];
		BYTE sound_rom2[0x0100];
	} soundROM_t;

	typedef union
	{
		soundROM_t soundROM;
		BYTE soundROMMemory[0x0200];
	} sound_rom_t;

	typedef union
	{
		// 16x16 pixels with each pixel being 2 bits
		// so, 16x16x2 = 512 bites per tile
		// which comes to 512/8 => 64 bytes per tile
		BYTE completeSprite[64][64];	// 64 sprites with each sprite being 64 bytes
		BYTE spriteROMMemory[0x1000];
	} sprite_rom_t;

	typedef union
	{
		// 8x8 pixels with each pixel being 2 bits
		// so, 8x8x2 = 128 bites per tile
		// which comes to 128/8 => 16 bytes per tile
		BYTE completeTile[256][16];	// 256 tiles with each tile being 16 bytes
		BYTE tileROMMemory[0x1000];
	} tile_rom_t;

	typedef union
	{
		BYTE completePalette[64][4];
		BYTE paletteROMMemory[0x0100];
	} palette_rom_t;

	typedef union
	{
		uint8_t completeColor[2][16];
		uint8_t colorROMMemory[0x0020];
	} color_rom_t;

	typedef struct
	{
		uint32_t apuCounter;
		uint32_t voice1Accumulator;
		uint32_t voice2Accumulator;
		uint32_t voice3Accumulator;
		PACMAN_AUDIO_SAMPLE_TYPE voice1Sample;
		PACMAN_AUDIO_SAMPLE_TYPE voice2Sample;
		PACMAN_AUDIO_SAMPLE_TYPE voice3Sample;
		uint32_t accumulatedAudioSamples;
		float emulatorVolume;
	} audio_t;

	typedef struct
	{
		FLAG waitingForRefresh;
		FLAG isVblank;
		union
		{
			Pixel imGuiBuffer1D[screen_width * screen_height];
			Pixel imGuiBuffer2D[screen_height][screen_width];
		} imGuiBuffer;
		uint64_t filters;
	} display_t;

	struct others_t
	{
		uint64_t hostCPUCycle;
		struct
		{
			uint32_t testCount[TWOFIFTYSIX];
			uint32_t cbtestCount[TWOFIFTYSIX];
			uint32_t edtestCount[TWOFIFTYSIX];
			uint32_t ddtestCount[TWOFIFTYSIX];
			uint32_t fdtestCount[TWOFIFTYSIX];
			uint32_t ddcbtestCount[TWOFIFTYSIX];
			uint32_t fdcbtestCount[TWOFIFTYSIX];
			struct
			{
				uint32_t indexer;
				uint32_t cyclePerInst;
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
				} cycles[FORTY];
			} cycles;
		} tomHarte;
	};

	typedef struct
	{
		// core
		registers_t registers;
		cpu_t cpuInstance;
		pacManMemory_t pacManMemory;
		IN0Memory_t IN0Memory;
		IN1Memory_t IN1Memory;
		color_rom_t color_rom;
		palette_rom_t palette_rom;
		tile_rom_t tile_rom;
		sprite_rom_t sprite_rom;
		sound_rom_t sound_rom;
		//
		uint16_t port0Data;
		enum INTERRUPT_MODE interruptMode;
		FLAG HaltEnabled;
		// semi - core
		audio_t audio;
		display_t display;
		// non - core
		others_t others;
	} pacMan_state_t;

	typedef union
	{
		pacMan_state_t pacMan_state;
		uint8_t pacMan_memoryState[sizeof(pacMan_state_t)];
	} pacMan_instance_t;

	typedef struct
	{
		FLAG areRomsLoaded;
		uint32_t codeRomSize;
		uint32_t colorRomSize;
		uint32_t paletteRomSize;
		uint32_t tileRomSize;
		uint32_t spriteRomSize;
		uint32_t soundRom1Size;
		uint32_t soundRom2Size;
		uint32_t auxillaryCodeRomSize;
	} aboutRom_t;

	typedef struct
	{
		Pixel color_0;
		Pixel color_1;
		Pixel color_2;
		Pixel color_3;
	} paletteColors_t;

	typedef struct
	{							// origin is same as the origin of the display
		Pixel cPixel[8][8];		// x, y where x indicates horizontal progression and y indicates vertical progression 					
	}tileWithColors_t;

	typedef struct
	{							// origin is same as the origin of the display
		Pixel sCPixel[16][16];	// x, y where x indicates horizontal progression and y indicates vertical progression 					
	}spriteWithColors_t;

	/*
	*
	* For Graphics
	*
	* Structure of prerenderedTiles table
	*
	*				palette 0	palette 1	palette 2	...		palette 62	palette 63
	*	tile 0			x			x			x					x			x
	*   tile 1			x			x			x					x			x
	*   tile 2			x			x			x					x			x
	*   tile 3			x			x			x					x			x
	*   :
	*   :
	*   :
	*   tile 253		x			x			x					x			x
	*   tile 254		x			x			x					x			x
	*   tile 255		x			x			x					x			x
	*
	*
	* For Audio
	*
	* TBDs
	*
	*/

	typedef struct
	{
		pacMan_instance_t pacMan_instance;
		aboutRom_t aboutRom;
		Pixel decodedColorROM[sizeof(color_rom_t)];
		paletteColors_t decodedPaletteROM[(sizeof(palette_rom_t) / 4)];
		tileWithColors_t prerenderedTiles[(sizeof(tile_rom_t) / 16)][(sizeof(palette_rom_t) / 4)];
		spriteWithColors_t prerenderedSprites[(sizeof(sprite_rom_t) / 64)][(sizeof(palette_rom_t) / 4)];
		spriteWithColors_t prerenderedSpritesFlipX[(sizeof(sprite_rom_t) / 64)][(sizeof(palette_rom_t) / 4)];
		spriteWithColors_t prerenderedSpritesFlipY[(sizeof(sprite_rom_t) / 64)][(sizeof(palette_rom_t) / 4)];
		spriteWithColors_t prerenderedSpritesFlipBoth[(sizeof(sprite_rom_t) / 64)][(sizeof(palette_rom_t) / 4)];
		int16_t prerenderedSoundWaveform[16][32];
		PACMAN_AUDIO_SAMPLE_TYPE audioBuffer[AUDIO_BUFFER_SIZE_FOR_PACMAN];
	} absolute_pacMan_state_t;

	union absolute_pacMan_instance_t
	{
		absolute_pacMan_state_t absolute_pacMan_state;
		uint8_t pacMan_absoluteMemoryState[sizeof(absolute_pacMan_state_t)];

		absolute_pacMan_instance_t() { memset(this, ZERO, sizeof(absolute_pacMan_instance_t)); }
	};

	std::shared_ptr <absolute_pacMan_instance_t> pAbsolute_pacMan_instance;
	pacMan_instance_t* pPacMan_instance = nullptr;			// for readability
	registers_t* pPacMan_registers = nullptr;				// for readability
	cpu_t* pPacMan_cpuInstance = nullptr;					// for readability
	pacManMemory_t* pPacMan_memory = nullptr;				// for readability
	flagFields_t* pPacMan_flags = nullptr;					// for readability
	audioANDSpriteCoordinates_t* pAudioRegisters = nullptr;	// for readability
	IN0Memory_t* pIN0Memory = nullptr;						// for readability
	IN1Memory_t* pIN1Memory = nullptr;						// for readability

#pragma pack(pop)

private:

	// controller mapping

private:

	SDL_AudioStream* audioStream = nullptr;

private:

	std::deque<pacMan_state_t> gamePlay;
#pragma endregion EMULATION_DECLARATIONS

#pragma region INFRASTRUCTURE_METHOD_DECLARATION
public:

	pacMan_t(int nfiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config);
	void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* network = nullptr) override;
	void sendBiosToEmulator(bios_t* bios = nullptr) override {};

public:

	float getVersion();
	uint32_t getScreenWidth() override;
	uint32_t getScreenHeight() override;
	uint32_t getPixelWidth() override;
	uint32_t getPixelHeight() override;
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
	FLAG loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) override;
	void dumpRom() override;
#pragma endregion INFRASTRUCTURE_METHOD_DECLARATION

#pragma region EMULATION_METHOD_DECLARATION
public:

	void processTimers();

public:

	void captureIO();

public:

	void processAudio();
	void playTheAudioFrame();

public:

	FLAG processDisplay(uint32_t vramAddress);
	void clearCompleteScreen();
	void displayCompleteScreen();

public:

private:

	void loadQuirks();

public:

	FLAG saveState(uint8_t id = 0) override;
	FLAG loadState(uint8_t id = 0) override;

	FLAG fillGamePlayStack() override;
	FLAG rewindGamePlay() override;

public:

	FLAG runEmulationAtHostRate(uint32_t currentFrame) override;
	FLAG runEmulationLoopAtHostRate(uint32_t currentFrame) override;
	FLAG runEmulationAtFixedRate(uint32_t currentFrame) override;
	FLAG runEmulationLoopAtFixedRate(uint32_t currentFrame) override;

public:

	uint32_t decryptAddressMethod1(uint32_t in);
	uint32_t decryptAddressMethod2(uint32_t in);
	BYTE decryptData(uint32_t in);
	FLAG setupEmulatorForMsPacMan(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, uint32_t* totalAuxilaryRomSize);

public:

	float getEmulationVolume() override;
	void setEmulationVolume(float volume) override;

public:

	void initializeGraphics();
	void initializeAudio();
	FLAG initializeEmulator() override;
	void destroyEmulator() override;

#pragma endregion EMULATION_METHOD_DECLARATION

#pragma region Z80_METHOD_DECLARATION
public:

	void cpuTickT();
	void syncOtherGBModuleTicks();
	void apuTick();

public:

	uint32_t getEmulatedAPUCycle();
	void setEmulatedAPUCycle(uint32_t cycles);

private:
	
	void stackPush(uint8_t data);
	uint8_t stackPop();

	void processSZPFlags
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

	void cpuSetRegister(REGISTER_TYPE rt, uint16_t u16parameter);
	uint16_t cpuReadRegister(REGISTER_TYPE rt);

	void cpuWritePointer(POINTER_TYPE mrt, uint16_t u16parameter);
	BYTE cpuReadPointer(POINTER_TYPE mrt);

	// Should be called only from the following functions:
	// 1) processOpcode
	// 2) performOperation
	// 3) stackPop
	// 4) stackPush
	// 5) processinterrupts?
	BYTE readRawMemoryFromCPU(uint16_t address, FLAG opcodeFetch = NO);

	// Should be called only from the following functions:
	// 1) processDisplay
	// 2) displayCompleteScreen
	BYTE readRawMemoryFromGFX(uint16_t address);

	// Should be called only from the following functions:
	// 1) processOpcode
	// 2) performOperation
	// 3) stackPop
	// 4) stackPush
	// 5) processinterrupts?
	void writeRawMemoryFromCPU(uint16_t address, BYTE data);

private:

	FLAG processCPU();

	FLAG processOpcode();

	void incrementR();

	void updateXY(uint8_t opcode, uint8_t src);

	// Should be called only from "processOpcode" or from "processInterrupts" (Except for test ROM)
	FLAG performOperation(int32_t anySpecificOpcode = INVALID);

	// Should be called only from "performOperation"
	void process0xCB();

	// Should be called only from "performOperation"
	void process0xDD0xFD(REGISTER_TYPE r, REGISTER_TYPE rh, REGISTER_TYPE rl);

	// Should be called only from "process0xDD0xFD"
	void process0xDDCB0xFDCB(REGISTER_TYPE r, REGISTER_TYPE rh, REGISTER_TYPE rl);

	// Should be called only from "performOperation"
	void process0xED();

	void unimplementedInstruction();

public:

	FLAG processInterrupts();
#pragma endregion Z80_METHOD_DECLARATION
};
#pragma endregion CORE