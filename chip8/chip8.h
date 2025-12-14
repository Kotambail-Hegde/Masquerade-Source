#pragma once

#pragma region REFERENCES
// https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
// https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
// https://stackoverflow.com/questions/51004250/saving-an-array-to-file-in-c
// https://github.com/Timendus/chip8-test-suite
#pragma endregion REFERENCES

#pragma region INCLUDES
//
#include "helpers.h"
//
#include "abstractEmulation.h"
#pragma endregion INCLUDES

#pragma region MACROS
#define CHIP8_FPS										(60.0)
#define EMULATED_AUDIO_SAMPLING_RATE_FOR_CHIP8			(48000.0)
#define AUDIO_BUFFER_SIZE_FOR_XO_CHIP					(CEIL((EMULATED_AUDIO_SAMPLING_RATE_FOR_CHIP8 / CHIP8_FPS)))

#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif
#pragma endregion MACROS

#pragma region CORE
class chip8_t : public abstractEmulation_t
{
#pragma region INFRASTRUCTURE_DECLARATIONS
public:

	std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom;
	float myFPS = CHIP8_FPS;

public:

	static const uint32_t screen_height = 64;
	static const uint32_t screen_width = 128;
	static const uint32_t pixel_height = 4;
	static const uint32_t pixel_width = 4;
	static const uint32_t memory_size = 4096;
	const char* NAME = "Chip8";

private:

	boost::property_tree::ptree pt;
	SHA1_CUSTOM sha1;
	std::string rom_sha1;
	boost::property_tree::ptree prg;

private:

	void* chip8GameEngine = nullptr;

private:

	enum class io_status
	{
		FREE = 0,
		PRESSED,
		HELD,
		RELEASED,
		TOTAL
	};

	enum class mouse
	{
		LEFT,
		RIGHT,
		MIDDLE,
		TOTAL
	};

#pragma endregion INFRASTRUCTURE_DECLARATIONS

#pragma region CHIP8_DECLARATIONS
private:

#pragma pack(push, 1)

	typedef struct
	{
		uint16_t opcode;
		int64_t emulatedCPUCycle;
	}cpu_t;

	typedef struct
	{
		uint8_t V[SIXTEEN];
		uint16_t I;
		uint16_t pc;
		uint16_t sp;
		uint16_t stack[SIXTEEN];
		uint8_t delay_timer;
	} registers_t;
#pragma endregion CHIP8_DECLARATIONS

#pragma region EMULATION_DECLARATIONS

private:
	
	enum class VARIANT
	{
		CHIP8,
		SCHIP_LEGACY,
		SCHIP_MODERN,
		XO_CHIP,
		MODERN_CHIP8,
		TOTAL,
		UNKNOWN
	};

	uint8_t chip8_fontset[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	uint8_t schip_fontset[100] =
	{
		0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C, // big 0
		0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, // big 1
		0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF, // big 2
		0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C, // big 3
		0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06, // big 4
		0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C, // big 5
		0x3E, 0x7C, 0xE0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C, // big 6
		0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60, // big 7
		0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C, // big 8
		0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C  // big 9
	};

	uint32_t INST_PER_FRAME[TO_UINT(VARIANT::TOTAL)] =
	{
		15,
		30,
		30,
		100000,
		12
	};

	uint8_t FRAME_RATE[TO_UINT(VARIANT::TOTAL)] =
	{
		60,
		64,
		60,
		60,
		60
	};

	enum class CHIP8_KEYS
	{
		CHIP8_ZERO = 0,
		CHIP8_ONE,
		CHIP8_TWO,
		CHIP8_THREE,
		CHIP8_FOUR,
		CHIP8_FIVE,
		CHIP8_SIX,
		CHIP8_SEVEN,
		CHIP8_EIGHT,
		CHIP8_NINE,
		CHIP8_A,
		CHIP8_B,
		CHIP8_C,
		CHIP8_D,
		CHIP8_E,
		CHIP8_F,
		CHIP8_TOTAL
	};

	// Map CHIP8 keys to ImGui keys
	std::pair<CHIP8_KEYS, ImGuiKey> keyMap[16] = {
		{CHIP8_KEYS::CHIP8_ZERO,  ImGuiKey_Keypad0},
		{CHIP8_KEYS::CHIP8_ONE,   ImGuiKey_Keypad1},
		{CHIP8_KEYS::CHIP8_TWO,   ImGuiKey_Keypad2},
		{CHIP8_KEYS::CHIP8_THREE, ImGuiKey_Keypad3},
		{CHIP8_KEYS::CHIP8_FOUR,  ImGuiKey_Keypad4},
		{CHIP8_KEYS::CHIP8_FIVE,  ImGuiKey_Keypad5},
		{CHIP8_KEYS::CHIP8_SIX,   ImGuiKey_Keypad6},
		{CHIP8_KEYS::CHIP8_SEVEN, ImGuiKey_Keypad7},
		{CHIP8_KEYS::CHIP8_EIGHT, ImGuiKey_Keypad8},
		{CHIP8_KEYS::CHIP8_NINE,  ImGuiKey_Keypad9},
		{CHIP8_KEYS::CHIP8_A,     ImGuiKey_A},
		{CHIP8_KEYS::CHIP8_B,     ImGuiKey_B},
		{CHIP8_KEYS::CHIP8_C,     ImGuiKey_C},
		{CHIP8_KEYS::CHIP8_D,     ImGuiKey_D},
		{CHIP8_KEYS::CHIP8_E,     ImGuiKey_E},
		{CHIP8_KEYS::CHIP8_F,     ImGuiKey_F}
	};

	//0x000 - 0x1FF - Chip 8 interpreter (contains font set in emu)
	//0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
	//0x200 - 0xFFF - Program ROM and work RAM

	enum class C8RES
	{
		LORES,
		HIRES,
		NON_EXTENDED,
		EXTENDED
	};

private:

	typedef struct
	{
		uint8_t memory[65536];
		uint8_t rpl[SIXTEEN];
	} ram_t;

	typedef struct
	{
		FLAG needAudio;
		uint8_t sound_timer;
		float emulatorVolume;
		float pitch;
		BYTE audioPattern[SIXTEEN];
		double phase;
		CHIP8_AUDIO_SAMPLE_TYPE audioBuffer[AUDIO_BUFFER_SIZE_FOR_XO_CHIP];
	} audio_t;

	typedef struct
	{
		FLAG vblank;
		union
		{
			FLAG gfx1D[4][screen_width * screen_height];
			FLAG gfx2D[4][screen_height][screen_width];
		} gfx;
		union
		{
			Pixel imGuiBuffer1D[screen_width * screen_height];
			Pixel imGuiBuffer2D[screen_height][screen_width];
		} imGuiBuffer;
		C8RES res;
		INC32 scrollH;
		INC32 scrollV;
		MAP8 planes;
		MAP64 filters;
	} display_t;

	typedef struct
	{
		io_status inputKeys[(uint8_t)CHIP8_KEYS::CHIP8_TOTAL];
		FLAG keyRelEvtC;
		FLAG keyRelEvtA;
	} io_t;

	typedef struct
	{
		FLAG _quirk_chip8;
		FLAG _quirk_schip_modern;
		FLAG _quirk_schip_legacy;
		FLAG _quirk_xo_chip;
		FLAG _quirk_modern_chip8;
		FLAG _QUIRK_VF_RESET;
		FLAG _QUIRK_MEMORY;
		FLAG _QUIRK_DISPLAY_WAIT;	// if enabled, at max we should allow only 60 sprites per second
		FLAG _QUIRK_CLIP;
		FLAG _QUIRK_SHIFT;
		FLAG _QUIRK_JUMP;
		FLAG _enable_c8_db;
		VARIANT variant;
	}quirks_t;

	typedef struct
	{
		uint32_t placeholder;
	}others_t;

	typedef struct
	{
		// core
		registers_t registers;
		cpu_t cpuInstance;
		ram_t ram;
		// semi - core
		audio_t audio;
		display_t display;
		io_t io;
		// non - core
		quirks_t quirks;
		others_t others;
	}chip8_state_t;

	union chip8_instance_t
	{
		chip8_state_t chip8_state;
		uint8_t chip8_memoryState[sizeof(chip8_state_t)];

		chip8_instance_t() {
			memset(this, 0, sizeof(chip8_instance_t));
		}
	};

	typedef struct
	{
		FLAG isRomLoaded;
		uint32_t romSize;
		uint8_t originalROM[4096];
	} aboutRom_t;

	typedef struct
	{
		chip8_instance_t chip8_instance;
		aboutRom_t aboutRom;
		// others if any
		int32_t emulationSpecific_TestVariable;
	}absolute_chip8_state_t;

	union absolute_chip8_instance_t
	{
		absolute_chip8_state_t absolute_chip8_state;
		uint8_t chip8_absoluteMemoryState[sizeof(absolute_chip8_state_t)];

		absolute_chip8_instance_t() 
		{
			memset(this, 0, sizeof(absolute_chip8_instance_t));
		}
	};

	std::shared_ptr <absolute_chip8_instance_t> pAbsolute_chip8_instance;
	chip8_instance_t* pChip8_instance;		// for readability
	registers_t* pChip8_registers;		// for readability
	ram_t* pChip8_memory;		// for readability
	audio_t* pChip8_audio;		// for readability
	display_t* pChip8_display;		// for readability
	quirks_t* pChip8_quirks;		// for readability
	io_t* pChip8_io;		// for readability

#pragma pack(pop)

private:

	SDL_AudioStream* audioStream = nullptr;
	CHIP8_AUDIO_SAMPLE_TYPE tone[TO_UINT(EMULATED_AUDIO_SAMPLING_RATE_FOR_CHIP8)];

private:

	Pixel SUPERNOVA = Pixel(255, 204, 0); 
	Pixel CHELSEAGEM = Pixel(153, 102, 0);
	Pixel BLAZEORANGE = Pixel(255, 102, 0);
	Pixel CEDARWOODFINISH = Pixel(102, 34, 0);

	Pixel color0 = SUPERNOVA;
	Pixel color1 = CHELSEAGEM;
	Pixel color2 = BLAZEORANGE;
	Pixel color3 = CEDARWOODFINISH;

private:
	
	std::deque<chip8_state_t> gamePlay;
#pragma endregion EMULATION_DECLARATIONS

#pragma region INFRASTRUCTURE_METHOD_DECLARATION
public:

	chip8_t(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config);
	void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* network = nullptr) override;
	void sendBiosToEmulator(bios_t* bios = nullptr) override {};

public:

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

	bool getRomLoadedStatus() override;
	bool loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) override;
	void dumpRom() override;

#pragma endregion INFRASTRUCTURE_METHOD_DECLARATION

#pragma region EMULATION_METHOD_DECLARATION
public:

	uint32_t getHostCPUCycle();
	void nextCycle();

public:

	// Utility to remap a key at runtime
	void reMapKeys(CHIP8_KEYS chipKey, ImGuiKey newKey);
	io_status getKeyStatus(ImGuiKey key);
	void captureIO();

public:

	void processAudio();
	void playTheAudioFrame() {};

public:

	bool processDisplay(uint32_t arg0 = NOT_USED) { RETURN NOT_USED; };
	void scrollDisplay(INC32 H, INC32 V);
	void displayCompleteScreen();
	void clearCompleteScreen();

private:

	void loadQuirks();

public:

	bool saveState(uint8_t id = 0) override;
	bool loadState(uint8_t id = 0) override;

	bool fillGamePlayStack() override;
	bool rewindGamePlay() override;

	int32_t getIdFromSHA1();
	FLAG getProgramFromId(int32_t id, boost::property_tree::ptree* prg);
	FLAG getRomInfo();

public:

	bool runEmulationAtHostRate(uint32_t currentFrame) override;
	bool runEmulationLoopAtHostRate(uint32_t currentFrame) override;
	bool runEmulationAtFixedRate(uint32_t currentFrame) override;
	bool runEmulationLoopAtFixedRate(uint32_t currentFrame) override;

public:

	float getEmulationVolume() override;
	void setEmulationVolume(float volume) override;

public:

	void setupVariant(VARIANT ovrd = VARIANT::UNKNOWN);
	bool initializeEmulator() override;
	void destroyEmulator() override;
#pragma endregion EMULATION_METHOD_DECLARATION

#pragma region CHIP8_METHOD_DECLARATION
public:

	int64_t getEmulatedCPUCycle();
	void setEmulatedCPUCycle(int64_t cycles);

	int32_t getEmulatedCPUCyclePerEmulationLoop() { RETURN ZERO; };
	void setEmulatedCPUCyclePerEmulationLoop(int32_t cycles) {};
	void incrementEmulatedCPUCycle(uint32_t nCycles, bool flag1 = false) {};
	void decrementEmulatedCPUCycle(uint32_t nCycles, bool flag1 = false) {};

private:

	bool processCPU();
	bool processOpcode();

public:

	void processTimers();
	bool processInterrupts();
#pragma endregion CHIP8_METHOD_DECLARATION
};
#pragma endregion CORE
