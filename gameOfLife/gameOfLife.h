#pragma once

#pragma region REFERENCES
//https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
#pragma endregion REFERENCES

#pragma region INCLUDES
//
#include "helpers.h"
//
#include "abstractEmulation.h"
#pragma endregion INCLUDES

#pragma region MACROS
#define GAME_OF_LIFE_FPS						60

#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif

#pragma endregion MACROS

#pragma region CORE
class gameOfLife_t : public abstractEmulation_t
{
#pragma region INFRASTRUCTURE_DECLARATIONS
public:

	static const uint32_t screen_height = 1024; // Needs to be power of 2
	static const uint32_t screen_width = 1024; // Needs to be power of 2
	static const uint32_t pixel_height = 1;
	static const uint32_t pixel_width = 1;
	int32_t delta_screen_height;
	int32_t delta_screen_width;
	int32_t delta_pixel_height;
	int32_t delta_pixel_width;
	const char* NAME = "Game Of Life";

private:

	boost::property_tree::ptree pt;

private:

	void* gameoflifeGameEngine = nullptr;
#pragma endregion INFRASTRUCTURE_DECLARATIONS

#pragma region GOL_DECLARATIONS
private:

#pragma pack(push, 1)

	typedef struct
	{
		FLAG lifeDatabase[screen_width * screen_height];
	} memory_t;
#pragma endregion GOL_DECLARATIONS

#pragma region EMULATION_DECLARATIONS
private:

	typedef struct
	{
		uint32_t placeHolder;
	} audio_t;

	typedef struct
	{
		float fOffsetX;
		float fOffsetY;
		float fScaleX;
		float fScaleY;
		float fStartPanX;
		float fStartPanY;
		float fSelectedCellX;
		float fSelectedCellY;
	} transformation_t;

	typedef struct
	{
		transformation_t transformation;
		union
		{
			Pixel imGuiBuffer1D[screen_width * screen_height];
			Pixel imGuiBuffer2D[screen_height][screen_width];
		} imGuiBuffer;
		uint64_t filters;
		FLAG freeRunning;
	} display_t;

	typedef struct
	{
		uint32_t placeHolder;
	} io_t;

	typedef struct
	{
		FLAG _isTorroidal;
		FLAG anyActivity;
		uint32_t hostCPUCycle;
	}other_t;

	typedef struct
	{
		int64_t emulatedCPUCycle;
		// semi-core
		memory_t memory;	// cpu instance embedded within memory
		display_t display;
		audio_t audio;
		io_t io;
		// non - core
		other_t others;
	}gol_state_t;

	typedef union
	{
		gol_state_t gol_state;
		uint8_t gol_memoryState[sizeof(gol_state_t)];
	}gol_instance_t;

	typedef struct
	{
		FLAG isRomLoaded;
		uint32_t romSize;
	}aboutRom_t;

	typedef struct
	{
		gol_instance_t gol_instance;
		aboutRom_t aboutRom;
	}absolute_gol_state_t;

	union absolute_gol_instance_t
	{
		absolute_gol_state_t absolute_gol_state;
		uint8_t gol_absoluteMemoryState[sizeof(absolute_gol_state_t)];

		absolute_gol_instance_t() { memset(this, 0, sizeof(absolute_gol_instance_t)); }
	};

	std::shared_ptr < absolute_gol_instance_t> pAbsolute_gol_instance;
	gol_instance_t* pGol_instance;		// for readability

#pragma pack(pop)

private:

	std::unordered_set<uint32_t> activeNow;
	std::unordered_set<uint32_t> activeNext;
	std::unordered_set<uint32_t> potentialNow;
	std::unordered_set<uint32_t> potentialNext;

private:

	FLAG wasGamePlayActive;
	std::deque<gol_state_t> gamePlay;
#pragma endregion EMULATION_DECLARATIONS

#pragma region INFRASTRUCTURE_METHOD_DECLARATION
public:

	gameOfLife_t(boost::property_tree::ptree& config);
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

	FLAG getRomLoadedStatus() override { RETURN false; }
	FLAG loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) override { RETURN false; }
	void dumpRom() override {}
#pragma endregion INFRASTRUCTURE_METHOD_DECLARATION

#pragma region EMULATION_METHOD_DECLARATION
public:

	uint32_t getHostCPUCycle();
	void nextCycle() {};

public:

	void captureIO();

public:

	void processAudio() {};
	void playTheAudioFrame() {}
	
private:

	void insertNeighbors(uint32_t pixel, bool erase = NO);

public:

	// Convert coordinates from World Space --> Screen Space
	void worldToScreen(float fWorldX, float fWorldY, int& nScreenX, int& nScreenY);

	// Convert coordinates from Screen Space --> World Space
	void screenToWorld(int nScreenX, int nScreenY, float& fWorldX, float& fWorldY);

	FLAG processDisplay(uint32_t arg0 = NOT_USED) { RETURN false; };
	void displayCompleteScreen();
	void performBackgroundActivity(uint32_t pixel);
	void performUserActivity();

public:

	FLAG saveState(uint8_t id = 0) override;
	FLAG loadState(uint8_t id = 0) override;

	FLAG fillGamePlayStack() override;
	FLAG rewindGamePlay() override;

public:

	void birth1DLife(int x, int y, std::wstring s);

public:

	FLAG runEmulationAtHostRate(uint32_t) override;
	FLAG runEmulationLoopAtHostRate(uint32_t) override;
	FLAG runEmulationAtFixedRate(uint32_t) override;
	FLAG runEmulationLoopAtFixedRate(uint32_t) override;

public:

	FLAG initializeEmulator() override;
	void destroyEmulator() override;
#pragma endregion EMULATION_METHOD_DECLARATION

#pragma region GOL_METHOD_DECLARATION
public:

	int64_t getEmulatedCPUCycle();
	void setEmulatedCPUCycle(int64_t cycles);

	int32_t getEmulatedCPUCyclePerEmulationLoop() { RETURN ZERO; };
	void setEmulatedCPUCyclePerEmulationLoop(int32_t cycles) {};
	void incrementEmulatedCPUCycle(uint32_t nCycles, FLAG flag1 = false) {};
	void decrementEmulatedCPUCycle(uint32_t nCycles, FLAG flag1 = false) {};

public:

	void processTimers() {};
	FLAG processInterrupts() { RETURN false; };
#pragma endregion GOL_METHOD_DECLARATION
};
#pragma endregion CORE