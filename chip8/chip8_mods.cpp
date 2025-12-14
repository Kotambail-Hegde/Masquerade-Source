#include "chip8.h"

std::string sha1_path = "/assets/c8/db/chip-8-database/database/sha1-hashes.json";
std::string prg_path = "/assets/c8/db/chip-8-database/database/programs.json";

void chip8_t::loadQuirks()
{
	if (ImGui::IsKeyPressed(ImGuiKey_Q) == YES)
	{
		auto& quirks = pChip8_instance->chip8_state.quirks;

		// Load mode flags
		quirks._quirk_chip8 = to_bool(pt.get<std::string>("chip8._chip8"));
		quirks._quirk_schip_modern = to_bool(pt.get<std::string>("chip8._schip_modern"));
		quirks._quirk_schip_legacy = to_bool(pt.get<std::string>("chip8._schip_legacy"));
		quirks._quirk_xo_chip = to_bool(pt.get<std::string>("chip8._xo_chip"));

		// Preset quirks table: {modeName, VF_RESET, MEMORY, DISPLAY_WAIT, CLIP, SHIFT, JUMP}
		struct Preset {
			const char* name;
			bool vf_reset;
			bool memory;
			bool display_wait;
			bool clip;
			bool shift;
			bool jump;
		};

		const Preset presets[] = {
			{ "Chip8 Mode",        true,  true,  false, true,  false, false },
			{ "SChip (Modern) Mode", false, false, false, true,  true,  true  },
			{ "SChip (Legacy) Mode", false, false, true,  true,  true,  true  },
			{ "XO-Chip Mode",      false, true,  false, false, false, false },
		};

		// Select preset if one of the modes is active
		const Preset* activePreset = nullptr;
		if (quirks._quirk_chip8)        activePreset = &presets[0];
		else if (quirks._quirk_schip_modern) activePreset = &presets[1];
		else if (quirks._quirk_schip_legacy) activePreset = &presets[2];
		else if (quirks._quirk_xo_chip)      activePreset = &presets[3];

		if (activePreset)
		{
			LOG("%s", activePreset->name);
			quirks._QUIRK_VF_RESET = activePreset->vf_reset;
			quirks._QUIRK_MEMORY = activePreset->memory;
			quirks._QUIRK_DISPLAY_WAIT = activePreset->display_wait;
			quirks._QUIRK_CLIP = activePreset->clip;
			quirks._QUIRK_SHIFT = activePreset->shift;
			quirks._QUIRK_JUMP = activePreset->jump;
		}
		else
		{
			LOG("Custom Mode");
			quirks._QUIRK_VF_RESET = to_bool(pt.get<std::string>("chip8._QUIRK_VF_RESET"));
			quirks._QUIRK_MEMORY = to_bool(pt.get<std::string>("chip8._QUIRK_MEMORY"));
			quirks._QUIRK_DISPLAY_WAIT = to_bool(pt.get<std::string>("chip8._QUIRK_DISPLAY_WAIT"));
			quirks._QUIRK_CLIP = to_bool(pt.get<std::string>("chip8._QUIRK_CLIP"));
			quirks._QUIRK_SHIFT = to_bool(pt.get<std::string>("chip8._QUIRK_SHIFT"));
			quirks._QUIRK_JUMP = to_bool(pt.get<std::string>("chip8._QUIRK_JUMP"));
		}

		LOG("Quirks Reloaded");
	}
}

FLAG chip8_t::saveState(uint8_t id)
{
	FLAG status = FALSE;

	std::string saveStateNameForThisROM = rom_sha1 + ".state";

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ofstream save;

#if ZERO
	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	strftime(buffer, sizeof(buffer), "%d-%m-%Y-%H:%M:%S", &timeinfo);
	LOG("Saved on: %s", buffer);
	std::string dt(buffer);
	saveFile.append(dt);
#endif

	LOG("Saved as: %s", saveStateNameForThisROM.c_str());

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<chip8_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<chip8_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
	save.write(reinterpret_cast<char*>(&(pChip8_instance->chip8_memoryState)), sizeof(pChip8_instance->chip8_memoryState));
	save.close();

	status = YES;

	RETURN status;
}

FLAG chip8_t::loadState(uint8_t id)
{
	FLAG status = FALSE;

	std::string saveStateNameForThisROM = rom_sha1 + ".state";

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ifstream save;

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<chip8_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<chip8_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM, std::ios::binary);
	save.read(reinterpret_cast<char*>(&(pChip8_instance->chip8_memoryState)), sizeof(pChip8_instance->chip8_memoryState));
	save.close();

	displayCompleteScreen();

	status = YES;

	RETURN status;
}

bool chip8_t::fillGamePlayStack()
{
	// assume minimum frame rate is 60 fps
	// so for 5 seconds worth of rewind, 300 elements is required
	// if fps is 1000, for 5 seconds worth of rewind, 5000 elements is required
	// Hence, we will (for now) set the limit to 5000 elements

	if (gamePlay.size() <= _REWIND_BUFFER_SIZE)
	{
		gamePlay.push_front(pChip8_instance->chip8_state);
		RETURN SUCCESS;
	}
	else
	{
		gamePlay.pop_back();
		gamePlay.push_front(pChip8_instance->chip8_state);
		RETURN FAILURE;
	}
}

bool chip8_t::rewindGamePlay()
{
	if (gamePlay.empty())
	{
		RETURN FAILURE;
	}
	else
	{
		memcpy(&pChip8_instance->chip8_memoryState, &gamePlay.front(), sizeof(pChip8_instance->chip8_memoryState));
		gamePlay.pop_front();
		RETURN SUCCESS;
	}
}

//---------------- Database ----------------------//

int32_t chip8_t::getIdFromSHA1()
{
	boost::property_tree::ptree sha1db;

	const std::string jsonPath = _EXE_LOCATION + sha1_path;

	// Check if file exists
	if (!std::filesystem::exists(jsonPath))
	{
		LOG("SHA1 database file not found: %s", jsonPath.c_str());
		RETURN INVALID;
	}

	// Load JSON only once
	if (sha1db.empty())
	{
		try
		{
			boost::property_tree::read_json(jsonPath, sha1db);
			DEBUG("SHA1 database loaded successfully.");
		}
		catch (const boost::property_tree::json_parser_error& ex)
		{
			DEBUG("Failed to read SHA1 database: %s", ex.what());
			RETURN INVALID;
		}
	}

	try
	{
		uint32_t id = sha1db.get<uint32_t>(rom_sha1);
		DEBUG("Found SHA1 match: %s => ID %u", rom_sha1.c_str(), id);
		RETURN static_cast<int32_t>(id);
	}
	catch (const boost::property_tree::ptree_bad_path&)
	{
		DEBUG("SHA1 not found in database: %s", rom_sha1.c_str());
		RETURN INVALID;
	}
	catch (const boost::property_tree::ptree_bad_data& ex)
	{
		DEBUG("Invalid value for SHA1 entry: %s", ex.what());
		RETURN INVALID;
	}
}

FLAG chip8_t::getProgramFromId(int32_t id, boost::property_tree::ptree* prg)
{
	boost::property_tree::ptree prg1db;

	const std::string jsonPath = _EXE_LOCATION + prg_path;

	// Check if file exists
	if (!std::filesystem::exists(jsonPath))
	{
		LOG("Programs database file not found: %s", jsonPath.c_str());
		RETURN FAILURE;
	}

	try
	{
		boost::property_tree::ptree programsTree;
		boost::property_tree::read_json(jsonPath, programsTree);

		uint32_t idx = 0;
		for (auto& item : programsTree)
		{
			if (idx == id)
			{
				if (prg)
				{
					*prg = item.second; // copy node
				}
				DEBUG("Program found at ID %u", id);
				RETURN SUCCESS;
			}
			++idx;
		}

		DEBUG("Program ID %u out of bounds", id);
		RETURN FAILURE;
	}
	catch (const std::exception& ex)
	{
		LOG("Failed to read programs.json: %s", ex.what());
		RETURN FAILURE;
	}

	RETURN FAILURE; // ID out of bounds
}

FLAG chip8_t::getRomInfo()
{
	int32_t id = getIdFromSHA1();
	if (id != INVALID)
	{
		if (getProgramFromId(id, &prg))
		{
			// Print the JSON node stored in the class member
			std::ostringstream oss;
			boost::property_tree::write_json(oss, prg);
			DEBUG("%s", oss.str().c_str());
			RETURN YES;
		}
		else
		{
			RETURN NO;
		}
	}
	else
	{
		RETURN NO;
	}
}