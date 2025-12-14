//******************************************//
// additional functionalities
//******************************************//

#include "spaceInvaders.h"

void spaceInvaders_t::loadQuirks()
{
	// Loadable only once per execution
	//if (ImGui::IsKeyReleased(ImGuiKey_Q) == true)
	static bool quirksLoaded = false;

	if (!quirksLoaded)
	{
		quirksLoaded = true;
		pSi_instance->si_state.quirks._DIP3 = to_bool(pt.get<std::string>("spaceinvaders._DIP3"));
		pSi_instance->si_state.quirks._DIP5 = to_bool(pt.get<std::string>("spaceinvaders._DIP5"));
		pSi_instance->si_state.quirks._DIP6 = to_bool(pt.get<std::string>("spaceinvaders._DIP6"));
		pSi_instance->si_state.quirks._DIP7 = to_bool(pt.get<std::string>("spaceinvaders._DIP7"));

		LOG("Quirks Loaded");
	}
}

FLAG spaceInvaders_t::saveState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = "_save_si_";

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

	static_assert(std::is_trivially_copyable<si_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<si_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
	save.write(reinterpret_cast<char*>(&(pSi_instance->si_memoryState)), sizeof(pSi_instance->si_memoryState));
	save.close();

	status = true;

	RETURN status;
}

FLAG spaceInvaders_t::loadState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = "_save_si_";

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ifstream save;

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<si_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<si_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM, std::ios::binary);
	save.read(reinterpret_cast<char*>(&(pSi_instance->si_memoryState)), sizeof(pSi_instance->si_memoryState));
	save.close();

	displayCompleteScreen();

	status = true;

	RETURN status;
}

FLAG spaceInvaders_t::fillGamePlayStack()
{
	// assume minimum frame rate is 60 fps
	// so for 5 seconds worth of rewind, 300 elements is required
	// if fps is 1000, for 5 seconds worth of rewind, 5000 elements is required
	// Hence, we will (for now) set the limit to 5000 elements

	if (gamePlay.size() <= _REWIND_BUFFER_SIZE)
	{
		gamePlay.push_front(pSi_instance->si_state);
		RETURN true;
	}
	else
	{
		gamePlay.pop_back();
		gamePlay.push_front(pSi_instance->si_state);
		RETURN false;
	}
}

FLAG spaceInvaders_t::rewindGamePlay()
{
	if (gamePlay.empty())
	{
		RETURN false;
	}
	else
	{
		memcpy(&pSi_instance->si_memoryState, &gamePlay.front(), sizeof(pSi_instance->si_memoryState));
		gamePlay.pop_front();
		RETURN true;
	}
}