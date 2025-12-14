#pragma once

#pragma region INCLUDES
//
#include "helpers.h"
//
#include "abstractEmulation.h"
#pragma endregion INCLUDES

#pragma region MACROS
#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif
#pragma endregion MACROS

#pragma region CORE
class defaults_t : public abstractEmulation_t
{
#pragma region INFRASTRUCTURE_DECLARATION
public:

	defaults_t() 
	{
		this->myID = EMULATION_ID::DEFAULT_ID;
		this->screen_height = 800 - WINDOW_PADDING;
		this->screen_width = 1280 - WINDOW_PADDING - WINDOW_PADDING;
		this->pixel_height = 1;
		this->pixel_width = 1;
	}
	~defaults_t() {};
	void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* network = nullptr) override {};
	void sendBiosToEmulator(bios_t* bios = nullptr) override {};

	uint32_t screen_height;
	uint32_t screen_width;
	uint32_t pixel_height;
	uint32_t pixel_width;
#pragma endregion INFRASTRUCTURE_DECLARATION

#pragma region INFRASTRUCTURE_METHOD_DECLARATION
private:

	const char* getEmulatorName() override { RETURN NAME; }
	float getEmulationFPS() override { RETURN myFPS; }
	float getEmulationVolume() override { RETURN EMULATION_VOLUME; }
	void setEmulationVolume(float volume)override { ; }
	uint32_t getScreenWidth() override { RETURN this->screen_width; }
	uint32_t getScreenHeight() override { RETURN this->screen_height; }
	uint32_t getPixelWidth() override { RETURN this->pixel_width; }
	uint32_t getPixelHeight() override { RETURN this->pixel_height; }
	uint32_t getTotalScreenWidth() override { RETURN this->screen_width; }
	uint32_t getTotalScreenHeight() override { RETURN this->screen_height; }
	uint32_t getTotalPixelWidth() override { RETURN this->pixel_width; }
	uint32_t getTotalPixelHeight() override { RETURN this->pixel_height; }
	void setScreenWidth(uint32_t size) override {this->screen_width = size;}
	void setScreenHeight(uint32_t size) override {this->screen_height = size;}
	void setPixelWidth(uint32_t size) override {this->pixel_width = size;}
	void setPixelHeight(uint32_t size) override {this->pixel_height = size;}
	void setTotalScreenWidth(uint32_t size) override {this->screen_width = size;}
	void setTotalScreenHeight(uint32_t size) override {this->screen_height = size;}
	void setTotalPixelWidth(uint32_t size) override {this->pixel_width = size;}
	void setTotalPixelHeight(uint32_t size) override {this->pixel_height = size;}
	void setEmulationID(EMULATION_ID ID) override
	{
		myID = ID;
	}
	EMULATION_ID getEmulationID() override
	{
		RETURN myID;
	}

	bool getRomLoadedStatus() override { RETURN true; }
	bool loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) override { RETURN true; }
	void dumpRom() override {}
#pragma endregion INFRASTRUCTURE_METHOD_DECLARATION

#pragma region EMULATION_METHOD_DECLARATION
public:

	bool saveState(uint8_t id = 0) override { RETURN false; }
	bool loadState(uint8_t id = 0) override { RETURN false; }

	bool fillGamePlayStack() override { RETURN false; }
	bool rewindGamePlay() override { RETURN false; }

	bool runEmulationAtHostRate(uint32_t) override { RETURN true; }
	bool runEmulationLoopAtHostRate(uint32_t) override { RETURN true; }
	bool runEmulationAtFixedRate(uint32_t) override { RETURN true; }
	bool runEmulationLoopAtFixedRate(uint32_t) override { RETURN true; }

	bool initializeEmulator() override { RETURN true; }
	void destroyEmulator() override {};
#pragma endregion EMULATION_METHOD_DECLARATION
};
#pragma endregion CORE