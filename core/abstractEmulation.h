#pragma once

#pragma region INCLUDES
//
#include "helpers.h"
//
#include "bios.h"
//
#include "cheats.h"
#pragma endregion INCLUDES

#pragma region MACROS
#define PAUSE_OR_RESUME(button)\
if (ImGui::IsKeyPressed(button) == true)\
{\
	bEmulationRun = !bEmulationRun;\
}

#define MUTE_OR_UNMUTE(button)\
if (ImGui::IsKeyPressed(button) == true)\
{\
	_MUTE_AUDIO = !_MUTE_AUDIO;\
}
#pragma endregion MACROS

#pragma region GLOBAL_INFRASTRUCTURE_DECLARATIONS
extern uint32_t nEmulationInstanceID;
extern uint32_t _XFPS;

extern FLAG bWaitingForConnection;

extern debugConfig_t debugConfig;

extern std::string exeName;

extern std::string _BIOS_LOCATION;
extern std::string _CONFIG_LOCATION;
extern std::string _EXE_LOCATION;
extern std::string _SAVE_LOCATION;

extern FLAG _ENABLE_AUDIO;
extern FLAG _MUTE_AUDIO;
extern FLAG _RUN_DISASSEMBLER;
extern FLAG _ENABLE_FRAME_LIMIT;
extern FLAG _ENABLE_NETWORK;
extern int32_t _NETWORK_TIMEOUT_LIMIT;
extern FLAG _ENABLE_QUICK_SAVE;
extern FLAG _ENABLE_BESS_FORMAT;
extern FLAG _ENABLE_REWIND;
extern SBYTE _SET_PPU_VERSION;
extern uint32_t _REWIND_BUFFER_SIZE;
extern int32_t _TEST_NUMBER;

extern FLAG isBiosEnabled;

extern uint32_t frame_buffer;
extern uint32_t masquerade_texture;
extern uint32_t shaderProgramBasic;
extern uint32_t shaderProgramBlend;
extern uint32_t fullscreenVAO;
extern uint32_t fullscreenVBO;
extern uint32_t FRAME_BUFFER_SCALE;

extern float _ACTUAL_FPS;
#pragma endregion GLOBAL_INFRASTRUCTURE_DECLARATIONS

#pragma region CORE
class abstractEmulation_t
{

#pragma region INFRASTRUCTURE_DECLARATIONS
public:

	EMULATION_ID myID = EMULATION_ID::DEFAULT_ID;

	const uint32_t screen_height = 500;
	const uint32_t screen_width = 800;
	const uint32_t pixel_height = 1;
	const uint32_t pixel_width = 1;
	const float myFPS = DEFAULT_FPS;
	const char* NAME = "Masquerade";

#pragma endregion INFRASTRUCTURE_DECLARATIONS

#pragma region EMULATION_DECLARATIONS
#pragma endregion EMULATION_DECLARATIONS

#pragma region INFRASTRUCTURE_DEFINITIONS
public:

	abstractEmulation_t() {};

	virtual ~abstractEmulation_t() {};

	virtual void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* network = nullptr) = 0;

public:

	float getVersion()
	{
		RETURN VERSION;
	}
	virtual const char* getEmulatorName()
	{
		RETURN NAME;
	}
	virtual float getEmulationFPS()
	{
		RETURN myFPS;
	}
	virtual float getEmulationVolume()
	{
		RETURN EMULATION_VOLUME;
	}
	virtual void setEmulationVolume(float volume)
	{
		;
	}
	virtual uint32_t getScreenWidth() 
	{
		RETURN screen_width;
	}
	virtual uint32_t getScreenHeight() 
	{
		RETURN screen_height;
	}
	virtual uint32_t getPixelWidth() 
	{
		RETURN pixel_width;
	}
	virtual uint32_t getPixelHeight() 
	{
		RETURN pixel_height;
	}
	virtual uint32_t getTotalScreenWidth()
	{
		RETURN this->screen_width;
	}
	virtual uint32_t getTotalScreenHeight()
	{
		RETURN this->screen_height;
	}
	virtual uint32_t getTotalPixelWidth()
	{
		RETURN this->pixel_width;
	}
	virtual uint32_t getTotalPixelHeight()
	{
		RETURN this->pixel_height;
	}
	virtual void setScreenWidth(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setScreenHeight(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setPixelWidth(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setPixelHeight(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setTotalScreenWidth(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setTotalScreenHeight(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setTotalPixelWidth(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setTotalPixelHeight(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setEmulationID(EMULATION_ID ID)
	{
		myID = ID;
	}
	virtual EMULATION_ID getEmulationID()
	{
		RETURN myID;
	}

	virtual FLAG getRomLoadedStatus() = 0;
	virtual FLAG loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) = 0;
	virtual void dumpRom() = 0;
#pragma endregion INFRASTRUCTURE_DEFINITIONS

#pragma region EMULATOR_DEFINITIONS
public:

	virtual FLAG saveState(uint8_t id = 0) = 0;
	virtual FLAG loadState(uint8_t id = 0) = 0;
	virtual FLAG fillGamePlayStack() = 0;
	virtual FLAG rewindGamePlay() = 0;

	virtual FLAG runEmulationAtHostRate(uint32_t currentFrame) = 0;
	virtual FLAG runEmulationLoopAtHostRate(uint32_t currentFrame) = 0;
	virtual FLAG runEmulationAtFixedRate(uint32_t currentFrame) = 0;
	virtual FLAG runEmulationLoopAtFixedRate(uint32_t currentFrame) = 0;

	virtual FLAG initializeEmulator() = 0;
	virtual void destroyEmulator() = 0;
#pragma endregion EMULATOR_DEFINITIONS

#pragma region CORE_DEFINITIONS
	virtual void sendBiosToEmulator(bios_t* bios) = 0;
#pragma endregion CORE_DEFINITIONS
};
#pragma endregion CORE