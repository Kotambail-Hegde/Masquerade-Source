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

//---------------- Database ----------------------//

// ============================================================
//  NES_t::loadNESDB()
// ============================================================

FLAG NES_t::loadNESDB(const std::string& jsonPath)
{
	if (pAbsolute_NES_instance->absolute_NES_state.isNes20dbLoaded == YES)
	{
		DEBUG("NES20DB already loaded (%zu entries).", m_nes20db.size());
		RETURN SUCCESS;
	}

	std::ifstream ifs(jsonPath);
	if (!ifs.is_open())
	{
		LOG("NES20DB: cannot open '%s'.", jsonPath.c_str());
		RETURN FAILURE;
	}

	std::string json((std::istreambuf_iterator<char>(ifs)),
		std::istreambuf_iterator<char>());
	ifs.close();

	rapidjson::Document doc;
	doc.Parse(json.c_str());

	if (doc.HasParseError())
	{
		LOG("NES20DB: JSON parse error (code %u, offset %zu) in '%s'.",
			static_cast<unsigned>(doc.GetParseError()),
			doc.GetErrorOffset(),
			jsonPath.c_str());
		RETURN FAILURE;
	}

	if (!doc.HasMember("nes20db") ||
		!doc["nes20db"].HasMember("game") ||
		!doc["nes20db"]["game"].IsArray())
	{
		LOG("NES20DB: expected 'nes20db.game' array in '%s'.", jsonPath.c_str());
		RETURN FAILURE;
	}

	const auto& games = doc["nes20db"]["game"];
	m_nes20db.reserve(games.Size());

	uint32_t count = 0;

	for (rapidjson::SizeType i = 0; i < games.Size(); ++i)
	{
		const auto& g = games[i];
		if (!g.IsObject()) continue;

		NES20DBEntry_t entry = {};

		// ---- combined ROM hash (lookup key) ----
		if (!g.HasMember("rom") || !g["rom"].IsObject()) continue;
		entry.romCRC32 = hexStr32(strval(g["rom"], "_crc32"));

		if (entry.romCRC32 == 0) continue;

		// ---- prgrom ----
		if (g.HasMember("prgrom") && g["prgrom"].IsObject())
		{
			const auto& p = g["prgrom"];
			entry.prgromSize = u32str(p, "_size");
			entry.prgromCRC32 = hexStr32(strval(p, "_crc32"));
		}

		// ---- chrrom (optional) ----
		if (g.HasMember("chrrom") && g["chrrom"].IsObject())
		{
			const auto& c = g["chrrom"];
			entry.chrromSize = u32str(c, "_size");
			entry.chrromCRC32 = hexStr32(strval(c, "_crc32"));
		}

		// ---- RAM / NVRAM (all optional) ----
		if (g.HasMember("prgram") && g["prgram"].IsObject())   entry.prgramSize = u32str(g["prgram"], "_size");
		if (g.HasMember("prgnvram") && g["prgnvram"].IsObject())  entry.prgnvramSize = u32str(g["prgnvram"], "_size");
		if (g.HasMember("chrram") && g["chrram"].IsObject())    entry.chrramSize = u32str(g["chrram"], "_size");
		if (g.HasMember("chrnvram") && g["chrnvram"].IsObject())  entry.chrnvramSize = u32str(g["chrnvram"], "_size");

		// ---- miscrom (optional) ----
		if (g.HasMember("miscrom") && g["miscrom"].IsObject())
			entry.miscRoms = static_cast<uint8_t>(u32str(g["miscrom"], "_number"));

		// ---- pcb ----
		if (g.HasMember("pcb") && g["pcb"].IsObject())
		{
			const auto& pcb = g["pcb"];
			entry.mapper = u32str(pcb, "_mapper");
			entry.subMapper = u32str(pcb, "_submapper");
			const char* mir = strval(pcb, "_mirroring");
			entry.mirroring = mir[0] ? mir[0] : 'H';
			entry.battery = static_cast<uint8_t>(u32str(pcb, "_battery"));
		}

		// ---- console ----
		if (g.HasMember("console") && g["console"].IsObject())
		{
			const auto& con = g["console"];
			entry.consoleType = static_cast<uint8_t>(u32str(con, "_type"));
			entry.consoleRegion = static_cast<uint8_t>(u32str(con, "_region"));
		}

		// ---- expansion ----
		if (g.HasMember("expansion") && g["expansion"].IsObject())
			entry.expansionType = static_cast<uint8_t>(u32str(g["expansion"], "_type"));

		// ---- vs (optional) ----
		if (g.HasMember("vs") && g["vs"].IsObject())
		{
			const auto& vs = g["vs"];
			entry.vsHardware = static_cast<uint8_t>(u32str(vs, "_hardware"));
			entry.vsPpu = static_cast<uint8_t>(u32str(vs, "_ppu"));
		}

		m_nes20db.emplace(entry.romCRC32, entry);
		++count;
	}

	pAbsolute_NES_instance->absolute_NES_state.isNes20dbLoaded = YES;
	LOG("NES20DB: loaded %u entries from '%s'.", count, jsonPath.c_str());
	RETURN SUCCESS;
}

// ============================================================
//  NES_t::lookupNESDB()
//
//  Writes matching entry into absolute_NES_state.dbEntry.
// ============================================================

FLAG NES_t::lookupNESDB(uint32_t romCRC32)
{
	auto it = m_nes20db.find(romCRC32);
	if (it == m_nes20db.end())
	{
		WARN("NES20DB: CRC32 %08X not found.", romCRC32);
		RETURN FAILURE;
	}

	pAbsolute_NES_instance->absolute_NES_state.dbEntry = it->second;

	const auto& e = pAbsolute_NES_instance->absolute_NES_state.dbEntry;
	LOG("NES20DB: CRC32 %08X -> mapper=%u submapper=%u mirror=%c battery=%u",
		romCRC32, e.mapper, e.subMapper, e.mirroring, e.battery);

	RETURN SUCCESS;
}
