#include "nes.h"

void NES_t::loadQuirks()
{
	if (ImGui::IsKeyReleased(ImGuiKey_Q) == true)
	{
		// re-read CONFIG.ini
		try
		{
			boost::property_tree::ini_parser::read_ini(_CONFIG_LOCATION, pt);
		}
		catch (std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		}

		INFO("\nCONFIG.ini was reloaded!\n");
	}
}

bool NES_t::saveState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = getSaveStateName(
		pINES->iNES_Fields.iNES_header.header
		, sizeof(pINES->iNES_Fields.iNES_header.header)
	);

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ofstream save;

#if ZERO
	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	strftime(buffer, sizeof(buffer), "%d-%m-%Y-%H:%M:%S", &timeinfo);
	printf("Saved on: %s\n", buffer);
	std::string dt(buffer);
	saveFile.append(dt);
#endif

	INFO("Saved as: %s\n", saveStateNameForThisROM.c_str());

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<NES_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<NES_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
	save.write(reinterpret_cast<char*>(&(pNES_instance->NES_memoryState)), sizeof(pNES_instance->NES_memoryState));
	save.close();

	status = true;

	RETURN status;
}

bool NES_t::loadState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = getSaveStateName(
		pINES->iNES_Fields.iNES_header.header
		, sizeof(pINES->iNES_Fields.iNES_header.header)
	);

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ifstream save;

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<NES_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<NES_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM, std::ios::binary);
	save.read(reinterpret_cast<char*>(&(pNES_instance->NES_memoryState)), sizeof(pNES_instance->NES_memoryState));
	save.close();

	status = true;

	RETURN status;
}

bool NES_t::absoluteSaveState(uint8_t id)
{
	bool status = false;

	std::filesystem::path saveDirectory(_SAVE_LOCATION);
	if (!(std::filesystem::exists(saveDirectory)))
	{
		std::filesystem::create_directory(saveDirectory);
	}

	std::string saveStateNameForThisROM = getSaveStateName(
		pINES->iNES_Fields.iNES_header.header
		, sizeof(pINES->iNES_Fields.iNES_header.header)
	);

	saveStateNameForThisROM = "_absolute_" + saveStateNameForThisROM + std::to_string(id);

	std::ofstream save;

#if ZERO
	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	strftime(buffer, sizeof(buffer), "%d-%m-%Y-%H:%M:%S", &timeinfo);
	printf("Saved on: %s\n", buffer);
	std::string dt(buffer);
	saveFile.append(dt);
#endif

	printf("Saved as: %s\n", saveStateNameForThisROM.c_str());

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<absolute_NES_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<absolute_NES_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
	save.write(reinterpret_cast<char*>(&(pAbsolute_NES_instance->NES_absoluteMemoryState)), sizeof(pAbsolute_NES_instance->NES_absoluteMemoryState));
	save.close();

	status = true;

	RETURN status;
}

bool NES_t::absoluteLoadState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = getSaveStateName(
		pINES->iNES_Fields.iNES_header.header
		, sizeof(pINES->iNES_Fields.iNES_header.header)
	);

	saveStateNameForThisROM = "_absolute_" + saveStateNameForThisROM + std::to_string(id);

	std::ifstream save;

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<absolute_NES_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<absolute_NES_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM, std::ios::binary);
	save.read(reinterpret_cast<char*>(&(pAbsolute_NES_instance->NES_absoluteMemoryState)), sizeof(pAbsolute_NES_instance->NES_absoluteMemoryState));
	save.close();

	status = true;

	RETURN status;
}

bool NES_t::fillGamePlayStack()
{
	// assume minimum frame rate is 60 fps
	// so for 5 seconds worth of rewind, 300 elements is required
	// if fps is 1000, for 5 seconds worth of rewind, 5000 elements is required
	// Hence, we will (for now) set the limit to 5000 elements

	if (gamePlay.size() <= _REWIND_BUFFER_SIZE)
	{
		gamePlay.push_front(pNES_instance->NES_state);
		RETURN true;
	}
	else
	{
		gamePlay.pop_back();
		gamePlay.push_front(pNES_instance->NES_state);
		RETURN false;
	}
}

bool NES_t::rewindGamePlay()
{
	if (gamePlay.empty())
	{
		RETURN false;
	}
	else
	{
		memcpy(&pNES_instance->NES_memoryState, &gamePlay.front(), sizeof(pNES_instance->NES_memoryState));
		gamePlay.pop_front();
		RETURN true;
	}
}