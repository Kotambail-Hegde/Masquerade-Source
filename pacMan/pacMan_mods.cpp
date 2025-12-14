#include "pacMan.h"

#pragma region PACMAN_SPECIFIC_MACROS
#define VRAM_START_ADDRESS 0x4000
#define VRAM_END_ADDRESS 0x47FF
#define CHARACTER_VRAM_START_ADDRESS VRAM_START_ADDRESS
#define CHARACTER_VRAM_END_ADDRESS 0x43FF
#define CHARACTER_CRAM_START_ADDRESS 0x4400
#define CHARACTER_CRAM_END_ADDRESS VRAM_END_ADDRESS
#define FLIPX true
#define FLIPY true
#define FLIPBOTH true
#define MAX_COLUMN_TILES 28
#define MAX_ROW_TILES 36
#pragma endregion PACMAN_SPECIFIC_MACROS

void pacMan_t::loadQuirks()
{
	// Loadable only once per execution
	//if (pacManGameEngine->GetKey(olc::Q).bReleased)
	static bool quirksLoaded = false;

	if (!quirksLoaded)
	{
		quirksLoaded = true;
		LOG("No Quirks to load");
	}
}

bool pacMan_t::saveState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = "_save_pm_";

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ofstream save;

#if ZERO
	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	strftime(buffer, sizeof(buffer), "%d-%m-%Y-%H:%M:%S", &timeinfo);
	LOG("Saved on: %s\n", buffer);
	std::string dt(buffer);
	saveFile.append(dt);
#endif

	LOG("Saved as: %s\n", saveStateNameForThisROM.c_str());

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<pacMan_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<pacMan_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
	save.write(reinterpret_cast<char*>(&(pPacMan_instance->pacMan_memoryState)), sizeof(pPacMan_instance->pacMan_memoryState));
	save.close();

	status = true;

	RETURN status;
}

bool pacMan_t::loadState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = "_save_pm_";

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ifstream save;

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<pacMan_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<pacMan_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM, std::ios::binary);
	save.read(reinterpret_cast<char*>(&(pPacMan_instance->pacMan_memoryState)), sizeof(pPacMan_instance->pacMan_memoryState));
	save.close();

	displayCompleteScreen();

	status = true;

	RETURN status;
}

bool pacMan_t::fillGamePlayStack()
{
	// assume minimum frame rate is 60 fps
	// so for 5 seconds worth of rewind, 300 elements is required
	// if fps is 1000, for 5 seconds worth of rewind, 5000 elements is required
	// Hence, we will (for now) set the limit to 5000 elements

	if (gamePlay.size() <= _REWIND_BUFFER_SIZE)
	{
		gamePlay.push_front(pPacMan_instance->pacMan_state);
		RETURN true;
	}
	else
	{
		gamePlay.pop_back();
		gamePlay.push_front(pPacMan_instance->pacMan_state);
		RETURN false;
	}
}

bool pacMan_t::rewindGamePlay()
{
	if (gamePlay.empty())
	{
		RETURN false;
	}
	else
	{
		memcpy(&pPacMan_instance->pacMan_memoryState, &gamePlay.front(), sizeof(pPacMan_instance->pacMan_memoryState));
		gamePlay.pop_front();
		pPacMan_instance->pacMan_state.display.waitingForRefresh = true;
		RETURN true;
	}
}