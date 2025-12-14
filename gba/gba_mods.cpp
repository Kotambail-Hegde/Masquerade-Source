#include "gba.h"

bool GBA_t::saveState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = getSaveFileName(
		pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer
		, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer)
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

	printf("Saved as: %s\n", saveStateNameForThisROM.c_str());

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<GBA_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<GBA_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
	save.write(reinterpret_cast<char*>(&(pGBA_instance->GBA_memoryState)), sizeof(pGBA_instance->GBA_memoryState));
	save.close();

	status = true;

	RETURN status;
}

bool GBA_t::loadState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = getSaveFileName(
		pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer
		, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer)
	);

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ifstream save;

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<GBA_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<GBA_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM, std::ios::binary);
	save.read(reinterpret_cast<char*>(&(pGBA_instance->GBA_memoryState)), sizeof(pGBA_instance->GBA_memoryState));
	save.close();

	status = true;

	RETURN status;
}

bool GBA_t::absoluteSaveState(uint8_t id)
{
	bool status = false;

	std::filesystem::path saveDirectory(_SAVE_LOCATION);
	if (!(std::filesystem::exists(saveDirectory)))
	{
		std::filesystem::create_directory(saveDirectory);
	}

	std::string saveStateNameForThisROM = getSaveFileName(
		pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer
		, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer)
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

	static_assert(std::is_trivially_copyable<absolute_GBA_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<absolute_GBA_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
	save.write(reinterpret_cast<char*>(&(pAbsolute_GBA_instance->GBA_absoluteMemoryState)), sizeof(pAbsolute_GBA_instance->GBA_absoluteMemoryState));
	save.close();

	status = true;

	RETURN status;
}

bool GBA_t::absoluteLoadState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = getSaveFileName(
		pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer
		, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer)
	);

	saveStateNameForThisROM = "_absolute_" + saveStateNameForThisROM + std::to_string(id);

	std::ifstream save;

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<absolute_GBA_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<absolute_GBA_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM, std::ios::binary);
	save.read(reinterpret_cast<char*>(&(pAbsolute_GBA_instance->GBA_absoluteMemoryState)), sizeof(pAbsolute_GBA_instance->GBA_absoluteMemoryState));
	save.close();

	status = true;

	RETURN status;
}

bool GBA_t::fillGamePlayStack()
{
	// assume minimum frame rate is 60 fps
	// so for 5 seconds worth of rewind, 300 elements is required
	// if fps is 1000, for 5 seconds worth of rewind, 5000 elements is required
	// Hence, we will (for now) set the limit to 5000 elements

	if (gamePlay.size() <= _REWIND_BUFFER_SIZE)
	{
		gamePlay.push_front(pGBA_instance->GBA_state);
		RETURN true;
	}
	else
	{
		gamePlay.pop_back();
		gamePlay.push_front(pGBA_instance->GBA_state);
		RETURN false;
	}
}

bool GBA_t::rewindGamePlay()
{
	if (gamePlay.empty())
	{
		RETURN false;
	}
	else
	{
		memcpy(&pGBA_instance->GBA_memoryState, &gamePlay.front(), sizeof(pGBA_instance->GBA_memoryState));
		gamePlay.pop_front();
		RETURN true;
	}
}
