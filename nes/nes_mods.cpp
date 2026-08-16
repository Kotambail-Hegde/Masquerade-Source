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

//---------------- Debugger ----------------------//

#ifndef __RPI_PICO__

void NES_t::debugEnsureTexturesCreated()
{
	if (debugTexturesInitialized == YES) RETURN;

	for (int i = 0; i < 2; i++)
	{
		GL_CALL(glGenTextures(ONE, &debugPatternTableTexture[i]));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, debugPatternTableTexture[i]));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	for (int i = 0; i < 4; i++)
	{
		GL_CALL(glGenTextures(ONE, &debugNametableTexture[i]));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, debugNametableTexture[i]));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	GL_CALL(glGenTextures(ONE, &debugOAMSpriteTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugOAMSpriteTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 16 * 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugMiniScreenTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugMiniScreenTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugPatternTileDetailTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugPatternTileDetailTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugCompositeTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugCompositeTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	debugTexturesInitialized = YES;
}

void NES_t::debugSyncScreenIfNeeded()
{
	debugEventViewerCheck();
	if (nesDebugger.ppu.enabled == NO) RETURN;

	// Freeze scroll state at the ONE moment it's guaranteed correct: pre-render, dot 304, right
	// when real hardware finishes reloading v from t for the upcoming frame -- before this
	// frame's CPU VRAM writes (very common during vblank) get a chance to stomp v with an
	// unrelated address. Reading v live at UI-render time was the actual bug.
	if (pNES_instance->NES_state.display.currentScanline == -1
		&& (int)pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY == 304)
	{
		nesDebugger.ppu.debugFrozenScrollX = (pNES_ppuRegisters->ppuInternalRegisters.v.fields.nameTblSelectH * 256)
			+ (pNES_ppuRegisters->ppuInternalRegisters.v.fields.coarseXScroll * 8)
			+ pNES_ppuRegisters->ppuInternalRegisters.x;
		nesDebugger.ppu.debugFrozenScrollY = (pNES_ppuRegisters->ppuInternalRegisters.v.fields.nameTblSelectV * 240)
			+ (pNES_ppuRegisters->ppuInternalRegisters.v.fields.coarseYScroll * 8)
			+ pNES_ppuRegisters->ppuInternalRegisters.v.fields.fineYScroll;
		nesDebugger.ppu.debugFrozenScrollValid = YES;
	}
}

void NES_t::renderNESDebuggerRegistersPanel()
{
	auto onOff = [](uint8_t v) -> const char* { return v ? "ON" : "off"; };
	auto onOffColor = [](uint8_t v) -> ImVec4 { return v ? ImVec4(0.45f, 0.90f, 0.45f, 1.0f) : ImVec4(0.55f, 0.55f, 0.55f, 1.0f); };
	auto beginRegTable = [](const char* id) -> bool
		{
			bool opened = ImGui::BeginTable(id, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);
			if (opened)
			{
				ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 170.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			}
			RETURN opened;
		};
	auto row = [](const char* label, const char* value, ImVec4 color = ImVec4(1, 1, 1, 1))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
			ImGui::TableNextColumn(); ImGui::TextColored(color, "%s", value);
		};

	auto& ppuCtrl = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl;

	if (ImGui::CollapsingHeader("PPUCTRL ($2000)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Raw: 0x%02X", ppuCtrl.PPUCTRL.raw);
		if (beginRegTable("PPUCTRLTable"))
		{
			row("Base Nametable", ppuCtrl.PPUCTRL.ppuctrl.BASE_NAMETABLE_ADDR_H || ppuCtrl.PPUCTRL.ppuctrl.BASE_NAMETABLE_ADDR_V
				? "non-zero" : "0 ($2000)");
			row("VRAM Addr Increment", ppuCtrl.PPUCTRL.ppuctrl.VRAM_ADDRESS_INCREMENT ? "+32 (down)" : "+1 (across)");
			row("Sprite Pattern Table", ppuCtrl.PPUCTRL.ppuctrl.SPRITE_PATTER_TABLE_ADDR_8x8 ? "$1000" : "$0000");
			row("BG Pattern Table", ppuCtrl.PPUCTRL.ppuctrl.BG_PATTERN_TABLE_ADDR ? "$1000" : "$0000");
			row("Sprite Size", ppuCtrl.PPUCTRL.ppuctrl.SPRITE_SIZE ? "8x16" : "8x8");
			row("VBlank NMI Enable", onOff(ppuCtrl.PPUCTRL.ppuctrl.VBLANK_NMI_ENABLE), onOffColor(ppuCtrl.PPUCTRL.ppuctrl.VBLANK_NMI_ENABLE));
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("PPUMASK ($2001)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Raw: 0x%02X", ppuCtrl.PPUMASK.raw);
		if (beginRegTable("PPUMASKTable"))
		{
			row("Greyscale", onOff(ppuCtrl.PPUMASK.ppumask.GREYSCALE), onOffColor(ppuCtrl.PPUMASK.ppumask.GREYSCALE));
			row("Show BG Leftmost 8px", onOff(ppuCtrl.PPUMASK.ppumask.BG_IN_LEFTMOST_8PIXELS), onOffColor(ppuCtrl.PPUMASK.ppumask.BG_IN_LEFTMOST_8PIXELS));
			row("Show Sprites Leftmost 8px", onOff(ppuCtrl.PPUMASK.ppumask.SPRITE_IN_LEFTMOST_8PIXELS), onOffColor(ppuCtrl.PPUMASK.ppumask.SPRITE_IN_LEFTMOST_8PIXELS));
			row("BG Rendering", onOff(ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING), onOffColor(ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING));
			row("Sprite Rendering", onOff(ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING), onOffColor(ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING));
			row("Emphasize Red", onOff(ppuCtrl.PPUMASK.ppumask.EMP_RED), onOffColor(ppuCtrl.PPUMASK.ppumask.EMP_RED));
			row("Emphasize Green", onOff(ppuCtrl.PPUMASK.ppumask.EMP_GREEN), onOffColor(ppuCtrl.PPUMASK.ppumask.EMP_GREEN));
			row("Emphasize Blue", onOff(ppuCtrl.PPUMASK.ppumask.EMP_BLUE), onOffColor(ppuCtrl.PPUMASK.ppumask.EMP_BLUE));
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("PPUSTATUS ($2002)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Raw: 0x%02X", ppuCtrl.PPUSTATUS.raw);
		if (beginRegTable("PPUSTATUSTable"))
		{
			row("Sprite Overflow", onOff(ppuCtrl.PPUSTATUS.ppustatus.SPRITE_OVERFLOW), onOffColor(ppuCtrl.PPUSTATUS.ppustatus.SPRITE_OVERFLOW));
			row("Sprite 0 Hit", onOff(ppuCtrl.PPUSTATUS.ppustatus.SPRITE_0_HIT), onOffColor(ppuCtrl.PPUSTATUS.ppustatus.SPRITE_0_HIT));
			row("VBlank", onOff(ppuCtrl.PPUSTATUS.ppustatus.VBLANK), onOffColor(ppuCtrl.PPUSTATUS.ppustatus.VBLANK));
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("Position / Timing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (beginRegTable("NESPosTable"))
		{
			char buf[24];
			snprintf(buf, sizeof(buf), "%d", (int)pNES_instance->NES_state.display.currentScanline); row("Scanline", buf);
			snprintf(buf, sizeof(buf), "%d", (int)pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY); row("Dot (cycle)", buf);
			snprintf(buf, sizeof(buf), "0x%02X", ppuCtrl.OAMADDR); row("OAMADDR", buf);
			ImGui::EndTable();
		}
	}
}

void NES_t::renderNESDebuggerPalettePanel()
{
	static const char* bgLabels[4] = { "BG0", "BG1", "BG2", "BG3" };
	static const char* objLabels[4] = { "SPR0", "SPR1", "SPR2", "SPR3" };

	ImGui::Columns(2, "NESPaletteColumns", false);

	ImGui::SeparatorText("Background");
	for (int p = 0; p < 4; p++)
	{
		ImGui::Text("%s", bgLabels[p]);
		for (int c = 0; c < 4; c++)
		{
			ImGui::SameLine(60.0f + c * 26.0f);
			ImGui::PushID(p * 10 + c);
			uint16_t addr = PALETTE_RAM_INDEXES_START_ADDRESS + (p << TWO) + c;
			byte colIdx = readPpuRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;
			Pixel col = palScreen[colIdx];
			ImVec4 imc = ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);
			ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(22, 22));
			ImGui::PopID();
		}
	}

	ImGui::NextColumn();

	ImGui::SeparatorText("Sprite");
	for (int p = 0; p < 4; p++)
	{
		ImGui::Text("%s", objLabels[p]);
		for (int c = 0; c < 4; c++)
		{
			ImGui::SameLine(60.0f + c * 26.0f);
			ImGui::PushID(100 + p * 10 + c);
			uint16_t addr = PALETTE_RAM_INDEXES_START_ADDRESS + SIXTEEN + (p << TWO) + c;
			byte colIdx = readPpuRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;
			Pixel col = palScreen[colIdx];
			ImVec4 imc = ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);
			ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(22, 22));
			ImGui::PopID();
		}
	}

	ImGui::Columns(1);
}

void NES_t::debugRebuildPatternTablePixels(int table, int paletteIndex)
{
	const uint16_t tableBase = (uint16_t)(table * 0x1000);

	for (int tileIdx = 0; tileIdx < 256; tileIdx++)
	{
		const int tileGridX = tileIdx % 16;
		const int tileGridY = tileIdx / 16;
		const uint16_t tileByteOffset = tableBase + (uint16_t)(tileIdx * 16);

		for (int row = 0; row < 8; row++)
		{
			byte lo = readPpuRawMemory(tileByteOffset + row, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
			byte hi = readPpuRawMemory(tileByteOffset + row + 8, MEMORY_ACCESS_SOURCE::DEBUG_PORT);

			for (int col = 0; col < 8; col++)
			{
				const uint8_t bit = 7 - col;
				const uint8_t colorIdx = (((hi >> bit) & ONE) << ONE) | ((lo >> bit) & ONE);

				Pixel outColor;
				if (paletteIndex < 0)
				{
					// Palette-agnostic grid, same philosophy as GBC's Tiles panel: a tile can be
					// used by multiple different palettes, so the main grid shows raw 2bpp shade
					// rather than committing to one arbitrarily-chosen palette.
					const uint8_t shade = colorIdx * 85;	// 0, 85, 170, 255
					outColor = Pixel(shade, shade, shade, 255);
				}
				else
				{
					uint16_t addr = (colorIdx == 0) ? PALETTE_RAM_INDEXES_START_ADDRESS
						: (PALETTE_RAM_INDEXES_START_ADDRESS + (paletteIndex << TWO) + colorIdx);
					byte palIdx = readPpuRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;
					outColor = palScreen[palIdx];
				}

				const int px = tileGridX * 8 + col;
				const int py = tileGridY * 8 + row;
				debugPatternTablePixels[table][py * 128 + px] = outColor;
			}
		}
	}
}

void NES_t::debugRebuildPatternTileDetailPixels(int table, int tileIdx, int paletteIndex)
{
	uint16_t tileByteOffset = (uint16_t)(table * 0x1000 + tileIdx * 16);

	for (int row = 0; row < 8; row++)
	{
		byte lo = readPpuRawMemory(tileByteOffset + row, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
		byte hi = readPpuRawMemory(tileByteOffset + row + 8, MEMORY_ACCESS_SOURCE::DEBUG_PORT);

		for (int col = 0; col < 8; col++)
		{
			const uint8_t bit = 7 - col;
			const uint8_t colorIdx = (((hi >> bit) & ONE) << ONE) | ((lo >> bit) & ONE);

			uint16_t addr = (colorIdx == 0) ? PALETTE_RAM_INDEXES_START_ADDRESS
				: (PALETTE_RAM_INDEXES_START_ADDRESS + (paletteIndex << TWO) + colorIdx);
			byte palIdx = readPpuRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;
			debugPatternTileDetailPixels[row * 8 + col] = palScreen[palIdx];
		}
	}
}

void NES_t::renderNESDebuggerPatternTablesPanel()
{
	static int previewPalette[2] = { -1, -1 };

	ImVec2 fullAvail = ImGui::GetContentRegionAvail();

	float leftWidth = fullAvail.x;
	float detailWidth = 0.0f;

	if (nesDebugger.ppu.selectedPatternTable >= 0)
	{
		detailWidth = ImMax(220.0f, fullAvail.x * 0.25f);	// 25% of available width, minimum 220px
		leftWidth = fullAvail.x - detailWidth - 8.0f;
	}

	ImGui::BeginChild("NESPatternTablesLeft", ImVec2(leftWidth, 0), false);

	for (int table = 0; table < 2; table++)
	{
		ImGui::PushID(table);
		ImGui::Text("Pattern Table %d ($%04X)", table, table * 0x1000);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(140.0f);
		ImGui::SliderInt("##palPreview", &previewPalette[table], -1, 3, previewPalette[table] < 0 ? "Grayscale" : "BG Palette %d");
		if (table == 0)
		{
			ImGui::Checkbox("Show grid", &nesDebugger.ppu.showPatternTableGrid);
			ImGui::Checkbox("8x16 mode", &nesDebugger.ppu.patternTableUse8x16);
		}

		debugRebuildPatternTablePixels(table, previewPalette[table]);
		GL_CALL(glBindTexture(GL_TEXTURE_2D, debugPatternTableTexture[table]));
		GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugPatternTablePixels[table].data()));

		ImVec2 avail = ImGui::GetContentRegionAvail();
		float side = (avail.x < 260.0f) ? avail.x : 260.0f;
		ImVec2 imgOrigin = ImGui::GetCursorScreenPos();
		ImGui::Image((ImTextureID)(uintptr_t)debugPatternTableTexture[table], ImVec2(side, side));

		if (ImGui::IsItemClicked())
		{
			ImVec2 mouse = ImGui::GetMousePos();
			int col, row;
			if (nesDebugger.ppu.patternTableUse8x16 == YES)
			{
				col = (int)((mouse.x - imgOrigin.x) / side * 16.0f);
				row = (int)((mouse.y - imgOrigin.y) / side * 8.0f);	// 8 rows of 16px-tall pairs
				if (col >= 0 && col < 16 && row >= 0 && row < 8)
				{
					nesDebugger.ppu.selectedPatternTable = table;
					nesDebugger.ppu.selectedPatternTile = (row * 2) * 16 + col;	// top tile of the pair
				}
			}
			else
			{
				col = (int)((mouse.x - imgOrigin.x) / side * 16.0f);
				row = (int)((mouse.y - imgOrigin.y) / side * 16.0f);
				if (col >= 0 && col < 16 && row >= 0 && row < 16)
				{
					nesDebugger.ppu.selectedPatternTable = table;
					nesDebugger.ppu.selectedPatternTile = row * 16 + col;
				}
			}
		}

		if (nesDebugger.ppu.showPatternTableGrid == YES)
		{
			const float scale = side / 128.0f;
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const ImU32 gridColor = nesDebugGridColor(nesDebugger.ppu.gridColorWhite);
			for (int c = 0; c <= 16; c++)
				drawList->AddLine(ImVec2(imgOrigin.x + c * 8 * scale, imgOrigin.y), ImVec2(imgOrigin.x + c * 8 * scale, imgOrigin.y + side), gridColor);
			for (int r = 0; r <= 16; r++)
				drawList->AddLine(ImVec2(imgOrigin.x, imgOrigin.y + r * 8 * scale), ImVec2(imgOrigin.x + side, imgOrigin.y + r * 8 * scale), gridColor);
		}

		if (nesDebugger.ppu.selectedPatternTable == table)
		{
			const float scale = side / 128.0f;
			const int selCol = nesDebugger.ppu.selectedPatternTile % 16;
			const int selRow = nesDebugger.ppu.selectedPatternTile / 16;
			const int selH = (nesDebugger.ppu.patternTableUse8x16 == YES) ? 2 : 1;
			ImVec2 rMin(imgOrigin.x + selCol * 8 * scale, imgOrigin.y + selRow * 8 * scale);
			ImVec2 rMax(rMin.x + 8 * scale, rMin.y + 8 * selH * scale);
			ImGui::GetWindowDrawList()->AddRect(rMin, rMax, IM_COL32(255, 0, 255, 255), 0.0f, 0, 2.0f);
		}

		ImGui::PopID();
	}

	ImGui::EndChild();

	if (nesDebugger.ppu.selectedPatternTable >= 0)
	{
		ImGui::SameLine();
		ImGui::BeginChild("NESPatternDetail", ImVec2(0, 0), true);

		const int table = nesDebugger.ppu.selectedPatternTable;
		const int tileIdx = nesDebugger.ppu.selectedPatternTile;
		const int tallMode = (nesDebugger.ppu.patternTableUse8x16 == YES) ? 1 : 0;

		ImGui::Text("Tile 0x%02X (Table %d)", tileIdx, table);
		ImGui::Text("Addr: $%04X", table * 0x1000 + tileIdx * 16);

		static int detailPalette = 0;
		ImGui::SetNextItemWidth(150.0f);
		ImGui::SliderInt("BG Palette", &detailPalette, 0, 3);

		debugRebuildPatternTileDetailPixels(table, tileIdx, detailPalette);
		GL_CALL(glBindTexture(GL_TEXTURE_2D, debugPatternTileDetailTexture));
		GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugPatternTileDetailPixels.data()));
		ImGui::Image((ImTextureID)(uintptr_t)debugPatternTileDetailTexture, ImVec2(128, 128));

		if (tallMode == 1)
		{
			debugRebuildPatternTileDetailPixels(table, tileIdx + 1, detailPalette);
			GL_CALL(glBindTexture(GL_TEXTURE_2D, debugPatternTileDetailTexture));
			GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugPatternTileDetailPixels.data()));
			ImGui::Image((ImTextureID)(uintptr_t)debugPatternTileDetailTexture, ImVec2(128, 128));
			ImGui::Text("(bottom half, tile 0x%02X)", tileIdx + 1);
		}

		ImGui::Text("Bytes:");
		for (int b = 0; b < 16; b++)
		{
			ImGui::Text("%02X", readPpuRawMemory((uint16_t)(table * 0x1000 + tileIdx * 16 + b), MEMORY_ACCESS_SOURCE::DEBUG_PORT));
			if ((b + 1) % 8 != 0) ImGui::SameLine();
		}

		ImGui::EndChild();
	}
}

void NES_t::debugRebuildNametablePixels(int nametableIndex)
{
	const uint16_t ntBase = (uint16_t)(0x2000 + nametableIndex * 0x0400);
	auto& ppuCtrl = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl;
	const int chrTable = ppuCtrl.PPUCTRL.ppuctrl.BG_PATTERN_TABLE_ADDR;

	for (int tileY = 0; tileY < 30; tileY++)
	{
		for (int tileX = 0; tileX < 32; tileX++)
		{
			uint16_t ntEntryAddr = ntBase + (uint16_t)(tileY * 32 + tileX);
			byte tileIndex = readPpuRawMemory(ntEntryAddr, MEMORY_ACCESS_SOURCE::DEBUG_PORT);

			// Attribute table: one byte per 4x4-tile (32x32px) block, 2 bits per 2x2-tile quadrant
			uint16_t attrAddr = ntBase + 0x03C0 + (uint16_t)((tileY / 4) * 8 + (tileX / 4));
			byte attrByte = readPpuRawMemory(attrAddr, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
			int quadrantShift = (((tileY / 2) & ONE) * 4) + (((tileX / 2) & ONE) * 2);
			byte paletteIndex = (attrByte >> quadrantShift) & 0x03;

			uint16_t tileByteOffset = (uint16_t)(chrTable * 0x1000 + tileIndex * 16);

			for (int row = 0; row < 8; row++)
			{
				byte lo = readPpuRawMemory(tileByteOffset + row, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
				byte hi = readPpuRawMemory(tileByteOffset + row + 8, MEMORY_ACCESS_SOURCE::DEBUG_PORT);

				for (int col = 0; col < 8; col++)
				{
					const uint8_t bit = 7 - col;
					const uint8_t colorIdx = (((hi >> bit) & ONE) << ONE) | ((lo >> bit) & ONE);

					// Real hardware quirk: background color index 0 ALWAYS uses the single
					// universal backdrop at $3F00, never the per-palette $3F04/$3F08/$3F0C
					// slots, regardless of this tile's attribute palette. Ignoring this was
					// exactly the black-box bug.
					uint16_t addr = (colorIdx == 0) ? PALETTE_RAM_INDEXES_START_ADDRESS
						: (PALETTE_RAM_INDEXES_START_ADDRESS + (paletteIndex << TWO) + colorIdx);
					byte palIdx = readPpuRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;
					Pixel outColor = palScreen[palIdx];

					const int px = tileX * 8 + col;
					const int py = tileY * 8 + row;
					debugNametablePixels[nametableIndex][py * 256 + px] = outColor;
				}
			}
		}
	}
}

void NES_t::renderNESDebuggerNametablesPanel()
{
	auto& ppuCtrl = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl;
	int activeNametable = ppuCtrl.PPUCTRL.ppuctrl.BASE_NAMETABLE_ADDR_H | (ppuCtrl.PPUCTRL.ppuctrl.BASE_NAMETABLE_ADDR_V << ONE);

	static const char* mirrorNames[] = { "Horizontal", "Vertical", "Single-screen (lo)", "Single-screen (hi)" };
	ImGui::Text("Mirroring: %s", mirrorNames[(int)pNES_instance->NES_state.catridgeInfo.nameTblMir]);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &nesDebugger.ppu.showNametableGrid);

	ImVec2 avail = ImGui::GetContentRegionAvail();
	float cellW = (avail.x - 8.0f) * 0.5f;
	float cellH = cellW * (240.0f / 256.0f);

	for (int nt = 0; nt < 4; nt++)
	{
		debugRebuildNametablePixels(nt);
		GL_CALL(glBindTexture(GL_TEXTURE_2D, debugNametableTexture[nt]));
		GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 240, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugNametablePixels[nt].data()));

		ImGui::BeginGroup();
		ImGui::TextColored(nt == activeNametable ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
			"NT%d ($%04X)%s", nt, 0x2000 + nt * 0x400, nt == activeNametable ? " (ACTIVE)" : "");
		ImVec2 ntOrigin = ImGui::GetCursorScreenPos();
		ImGui::Image((ImTextureID)(uintptr_t)debugNametableTexture[nt], ImVec2(cellW, cellH));

		if (ImGui::IsItemClicked())
		{
			ImVec2 mouse = ImGui::GetMousePos();
			int tileX = (int)((mouse.x - ntOrigin.x) / cellW * 32.0f);
			int tileY = (int)((mouse.y - ntOrigin.y) / cellH * 30.0f);
			if (tileX >= 0 && tileX < 32 && tileY >= 0 && tileY < 30)
			{
				nesDebugger.ppu.selectedNametableIndex = nt;
				nesDebugger.ppu.selectedNametableTileX = tileX;
				nesDebugger.ppu.selectedNametableTileY = tileY;
			}
		}

		if (nesDebugger.ppu.showNametableGrid == YES)
		{
			const float scaleX = cellW / 256.0f;
			const float scaleY = cellH / 240.0f;
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const ImU32 gridColor = nesDebugGridColor(nesDebugger.ppu.gridColorWhite);
			for (int col = 0; col <= 32; col++)
				drawList->AddLine(ImVec2(ntOrigin.x + col * 8 * scaleX, ntOrigin.y), ImVec2(ntOrigin.x + col * 8 * scaleX, ntOrigin.y + cellH), gridColor);
			for (int row = 0; row <= 30; row++)
				drawList->AddLine(ImVec2(ntOrigin.x, ntOrigin.y + row * 8 * scaleY), ImVec2(ntOrigin.x + cellW, ntOrigin.y + row * 8 * scaleY), gridColor);
		}

		if (nesDebugger.ppu.selectedNametableIndex == nt)
		{
			const float scaleX = cellW / 256.0f;
			const float scaleY = cellH / 240.0f;
			ImVec2 rMin(ntOrigin.x + nesDebugger.ppu.selectedNametableTileX * 8 * scaleX, ntOrigin.y + nesDebugger.ppu.selectedNametableTileY * 8 * scaleY);
			ImVec2 rMax(rMin.x + 8 * scaleX, rMin.y + 8 * scaleY);
			ImGui::GetWindowDrawList()->AddRect(rMin, rMax, IM_COL32(255, 0, 255, 255), 0.0f, 0, 2.0f);
		}

		ImGui::EndGroup();
		if (nt % 2 == 0) ImGui::SameLine();
	}

	ImGui::Separator();

	// ---- Pixel/tile info, inline in this same panel, not a separate dock window ----
	if (nesDebugger.ppu.selectedNametableIndex < 0)
	{
		ImGui::TextDisabled("Click a tile above to see its nametable/attribute info here.");
	}
	else
	{
		const int nt = nesDebugger.ppu.selectedNametableIndex;
		const int tileX = nesDebugger.ppu.selectedNametableTileX;
		const int tileY = nesDebugger.ppu.selectedNametableTileY;
		const uint16_t ntBase = (uint16_t)(0x2000 + nt * 0x0400);

		uint16_t ntEntryAddr = ntBase + (uint16_t)(tileY * 32 + tileX);
		byte tileIndex = readPpuRawMemory(ntEntryAddr, MEMORY_ACCESS_SOURCE::DEBUG_PORT);

		uint16_t attrAddr = ntBase + 0x03C0 + (uint16_t)((tileY / 4) * 8 + (tileX / 4));
		byte attrByte = readPpuRawMemory(attrAddr, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
		int quadrantShift = (((tileY / 2) & ONE) * 4) + (((tileX / 2) & ONE) * 2);
		byte paletteIndex = (attrByte >> quadrantShift) & 0x03;

		int chrTable = ppuCtrl.PPUCTRL.ppuctrl.BG_PATTERN_TABLE_ADDR;

		ImGui::Text("NT%d, tile col %d, row %d", nt, tileX, tileY);
		ImGui::Text("Nametable byte addr: $%04X   Tile index: 0x%02X", ntEntryAddr, tileIndex);
		ImGui::Text("Attribute byte addr: $%04X (0x%02X)   BG Palette: %d", attrAddr, attrByte, paletteIndex);
		ImGui::Text("Pattern table: %d ($%04X)   Tile addr: $%04X", chrTable, chrTable * 0x1000, chrTable * 0x1000 + tileIndex * 16);
	}
}

void NES_t::debugRebuildOAMSpritePixels()
{
	auto& ppuCtrl = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl;
	const FLAG tallSprites = (ppuCtrl.PPUCTRL.ppuctrl.SPRITE_SIZE == ONE) ? YES : NO;
	const int spriteHeight = tallSprites ? 16 : 8;

	debugOAMSpritePixels.fill(Pixel(ZERO, ZERO, ZERO, ZERO));

	for (int s = 0; s < 64; s++)
	{
		const auto& oam = pNES_ppuMemory->NESMemoryMap.primaryOam.oamW[s];

		uint16_t patternTable;
		uint8_t baseTileIndex;
		if (tallSprites == YES)
		{
			patternTable = (oam.tileID & ONE) ? 0x1000 : 0x0000;
			baseTileIndex = oam.tileID & 0xFE;
		}
		else
		{
			patternTable = ppuCtrl.PPUCTRL.ppuctrl.SPRITE_PATTER_TABLE_ADDR_8x8 ? 0x1000 : 0x0000;
			baseTileIndex = oam.tileID;
		}

		const uint8_t paletteIndex = oam.attributes.fields.palette;
		const FLAG xFlip = oam.attributes.fields.flipHorizontally;
		const FLAG yFlip = oam.attributes.fields.flipVertically;

		for (int row = 0; row < spriteHeight; row++)
		{
			const int logicalRow = yFlip ? (spriteHeight - 1 - row) : row;
			const int tileOfRow = logicalRow / 8;
			const int rowInTile = logicalRow % 8;
			uint16_t tileByteOffset = (uint16_t)(patternTable + (baseTileIndex + tileOfRow) * 16);

			byte lo = readPpuRawMemory(tileByteOffset + rowInTile, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
			byte hi = readPpuRawMemory(tileByteOffset + rowInTile + 8, MEMORY_ACCESS_SOURCE::DEBUG_PORT);

			for (int col = 0; col < 8; col++)
			{
				const int srcBit = xFlip ? col : (7 - col);
				const uint8_t colorIdx = (((hi >> srcBit) & ONE) << ONE) | ((lo >> srcBit) & ONE);

				if (colorIdx == 0) continue;	// transparent

				uint16_t addr = PALETTE_RAM_INDEXES_START_ADDRESS + SIXTEEN + (paletteIndex << TWO) + colorIdx;
				byte palIdx = readPpuRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;
				Pixel outColor = palScreen[palIdx];

				const int py = s * 16 + row;
				debugOAMSpritePixels[py * 8 + col] = outColor;
			}
		}
	}
}

void NES_t::renderNESDebuggerOAMPanel()
{
	debugRebuildOAMSpritePixels();
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugOAMSpriteTexture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 16 * 64, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugOAMSpritePixels.data()));

	auto& ppuCtrl = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl;
	const FLAG tallSprites = (ppuCtrl.PPUCTRL.ppuctrl.SPRITE_SIZE == ONE) ? YES : NO;
	const int spriteHeight = tallSprites ? 16 : 8;

	int viewSel = (nesDebugger.ppu.oamUseGalleryView == YES) ? 1 : 0;
	ImGui::RadioButton("List", &viewSel, 0); ImGui::SameLine();
	ImGui::RadioButton("Gallery", &viewSel, 1);
	nesDebugger.ppu.oamUseGalleryView = (viewSel == 1) ? YES : NO;

	ImGui::BeginChild("NESOAMTable", ImVec2(ImGui::GetContentRegionAvail().x * 0.45f, 0), true);

	if (nesDebugger.ppu.oamUseGalleryView == NO)
	{
		if (ImGui::BeginTable("NESOAMEntries", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
		{
		ImGui::TableSetupColumn("#");
		ImGui::TableSetupColumn("X");
		ImGui::TableSetupColumn("Y");
		ImGui::TableSetupColumn("Tile");
		ImGui::TableSetupColumn("Flags");
		ImGui::TableHeadersRow();

		for (int s = 0; s < 64; s++)
		{
			const auto& oam = pNES_ppuMemory->NESMemoryMap.primaryOam.oamW[s];
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			bool isSelected = (nesDebugger.ppu.selectedOAMEntry == s);
			if (ImGui::Selectable(std::to_string(s).c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
				nesDebugger.ppu.selectedOAMEntry = s;
			ImGui::TableNextColumn(); ImGui::Text("%d", oam.xPosition);
			ImGui::TableNextColumn(); ImGui::Text("%d", oam.yPosition);
			ImGui::TableNextColumn(); ImGui::Text("0x%02X", oam.tileID);
			ImGui::TableNextColumn();
			ImGui::Text("%s%s%sP%d",
				oam.attributes.fields.flipHorizontally ? "X " : "",
				oam.attributes.fields.flipVertically ? "Y " : "",
				oam.attributes.fields.priority ? "bg " : "",
				oam.attributes.fields.palette);
		}
		ImGui::EndTable();
		}
	}
	else
	{
		const float thumbSize = 40.0f;
		const float spacing = 4.0f;
		const int columns = (int)(ImGui::GetContentRegionAvail().x / (thumbSize + spacing));
		const int colCount = columns < 1 ? 1 : columns;

		for (int s = 0; s < 64; s++)
		{
			ImVec2 uv0(0.0f, (s * 16) / (16.0f * 64.0f));
			ImVec2 uv1(1.0f, (s * 16 + spriteHeight) / (16.0f * 64.0f));

			ImGui::PushID(s);
			ImVec2 imgOrigin = ImGui::GetCursorScreenPos();
			ImGui::Image((ImTextureID)(uintptr_t)debugOAMSpriteTexture, ImVec2(thumbSize, thumbSize), uv0, uv1);
			if (ImGui::IsItemClicked())
				nesDebugger.ppu.selectedOAMEntry = s;
			if (ImGui::IsItemHovered())
			{
				const auto& oam = pNES_ppuMemory->NESMemoryMap.primaryOam.oamW[s];
				ImGui::SetTooltip("Sprite #%d\nX:%d Y:%d Tile:0x%02X", s, oam.xPosition, oam.yPosition, oam.tileID);
			}

			ImU32 borderColor = (nesDebugger.ppu.selectedOAMEntry == s) ? IM_COL32(255, 0, 255, 255) : IM_COL32(60, 60, 60, 255);
			ImGui::GetWindowDrawList()->AddRect(imgOrigin, ImVec2(imgOrigin.x + thumbSize, imgOrigin.y + thumbSize), borderColor, 0.0f, 0, 2.0f);
			ImGui::PopID();

			if ((s + 1) % colCount != 0 && s != 63) ImGui::SameLine(0.0f, spacing);
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("NESOAMPreview", ImVec2(0, 0), true);
	if (nesDebugger.ppu.selectedOAMEntry >= 0 && nesDebugger.ppu.selectedOAMEntry < 64)
	{
		const int s = nesDebugger.ppu.selectedOAMEntry;
		const auto& oam = pNES_ppuMemory->NESMemoryMap.primaryOam.oamW[s];

		ImVec2 uv0(0.0f, (s * 16) / (16.0f * 64.0f));
		ImVec2 uv1(1.0f, (s * 16 + spriteHeight) / (16.0f * 64.0f));
		ImGui::Text("Sprite #%d", s);
		ImGui::Image((ImTextureID)(uintptr_t)debugOAMSpriteTexture, ImVec2(8 * 8.0f, spriteHeight * 8.0f), uv0, uv1);

		ImGui::Separator();
		ImGui::Text("Palette used:");
		for (int c = 0; c < 4; c++)
		{
			ImGui::SameLine(0.0f, (c == 0) ? 0.0f : 4.0f);
			ImGui::PushID(c);
			uint16_t addr = PALETTE_RAM_INDEXES_START_ADDRESS + SIXTEEN + (oam.attributes.fields.palette << TWO) + c;
			byte palIdx = readPpuRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;
			Pixel col = palScreen[palIdx];
			ImVec4 imc = ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);
			ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(24, 24));
			ImGui::PopID();
		}

		ImGui::Separator();
		ImGui::Text("Position on screen");

		GL_CALL(glBindTexture(GL_TEXTURE_2D, debugMiniScreenTexture));
		GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screen_width, screen_height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pNES_instance->NES_state.display.imGuiBuffer.imGuiBuffer1D));

		ImVec2 miniAvail = ImGui::GetContentRegionAvail();
		const float miniAspect = (float)screen_width / (float)screen_height;
		ImVec2 miniSize = (miniAvail.x / miniAspect <= miniAvail.y) ? ImVec2(miniAvail.x, miniAvail.x / miniAspect) : ImVec2(miniAvail.y * miniAspect, miniAvail.y);
		ImVec2 miniOrigin = ImGui::GetCursorScreenPos();
		ImGui::Image((ImTextureID)(uintptr_t)debugMiniScreenTexture, miniSize);

		const float miniScale = miniSize.x / (float)screen_width;
		ImVec2 rectMin(miniOrigin.x + oam.xPosition * miniScale, miniOrigin.y + oam.yPosition * miniScale);
		ImVec2 rectMax(rectMin.x + 8 * miniScale, rectMin.y + spriteHeight * miniScale);
		ImGui::GetWindowDrawList()->AddRect(rectMin, rectMax, IM_COL32(255, 0, 255, 255), 0.0f, 0, 2.5f);
	}
	else
	{
		ImGui::TextDisabled("Select a sprite to preview it here.");
	}
	ImGui::EndChild();
}

void NES_t::renderNESDebuggerPPUTab()
{
	ImGui::Checkbox("Enable PPU debug instrumentation", &nesDebugger.ppu.enabled);

	ImGui::BeginDisabled(nesDebugger.ppu.enabled == NO);

	ImGui::TextDisabled("Screen refresh:"); ImGui::SameLine();
	int sampleMode = (int)nesDebugger.ppu.pixelOutputSampleMode;
	ImGui::RadioButton("Per Frame", &sampleMode, (int)NES_DEBUG_PIXEL_SAMPLE_MODE::PER_FRAME); ImGui::SameLine();
	ImGui::RadioButton("Per Scanline", &sampleMode, (int)NES_DEBUG_PIXEL_SAMPLE_MODE::PER_LY); ImGui::SameLine();
	ImGui::RadioButton("Per Dot", &sampleMode, (int)NES_DEBUG_PIXEL_SAMPLE_MODE::PER_DOT);
	nesDebugger.ppu.pixelOutputSampleMode = (NES_DEBUG_PIXEL_SAMPLE_MODE)sampleMode;

	ImGui::SameLine(); ImGui::Text("  Grid:"); ImGui::SameLine();
	int gridColSel = (nesDebugger.ppu.gridColorWhite == YES) ? 0 : 1;
	ImGui::RadioButton("White", &gridColSel, 0); ImGui::SameLine();
	ImGui::RadioButton("Black", &gridColSel, 1);
	nesDebugger.ppu.gridColorWhite = (gridColSel == 0) ? YES : NO;

	ImGui::Separator();

	int breakScanlineI = nesDebugger.ppu.breakpointScanline;
	int breakDotI = nesDebugger.ppu.breakpointDot;
	ImGui::Text("Run to  Scanline:"); ImGui::SameLine(); ImGui::SetNextItemWidth(90.0f);
	ImGui::InputInt("##breakScanline", &breakScanlineI, 1, 10);
	ImGui::SameLine(); ImGui::Text("Dot:"); ImGui::SameLine(); ImGui::SetNextItemWidth(110.0f);
	ImGui::InputInt("##breakDot", &breakDotI, 1, 10);
	breakScanlineI = (breakScanlineI < -1) ? -1 : (breakScanlineI > 260 ? 260 : breakScanlineI);
	breakDotI = (breakDotI < 0) ? 0 : (breakDotI > 340 ? 340 : breakDotI);
	nesDebugger.ppu.breakpointScanline = (int16_t)breakScanlineI;
	nesDebugger.ppu.breakpointDot = (uint16_t)breakDotI;

	ImGui::SameLine();
	if (ImGui::Button("Run to breakpoint"))
	{
		nesDebugger.ppu.runToBreakpointArmed = YES;
		nesDebugger.ppu.paused = NO;
	}

	ImGui::SameLine();
	if (nesDebugger.ppu.paused == YES)
	{
		if (ImGui::Button("Step")) nesDebugger.ppu.stepRequested = YES;
		ImGui::SameLine();
		if (ImGui::Button("Resume"))
		{
			nesDebugger.ppu.paused = NO; nesDebugger.ppu.runToBreakpointArmed = NO;
		}
	}
	else
	{
		ImGui::TextDisabled("Step / Resume (available once paused)");
	}

	ImGui::Text("Current -- Scanline: %3d   Dot: %3d / 340   %s",
		(int)pNES_instance->NES_state.display.currentScanline,
		(int)pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY,
		nesDebugger.ppu.paused == YES ? "[PAUSED]" : (nesDebugger.ppu.runToBreakpointArmed == YES ? "[RUNNING TO BREAKPOINT]" : "[RUNNING]"));

	ImGui::EndDisabled();

	ImGui::Separator();

	ImGuiID dockspaceID = ImGui::GetID("NESPPUDockSpace");

	if (nesDebugger.ppu.dockLayoutBuilt == NO && ImGui::DockBuilderGetNode(dockspaceID) == nullptr)
	{
		ImGui::DockBuilderRemoveNode(dockspaceID);
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, ImVec2(1000.0f, 600.0f));

		ImGuiID dockCenter = dockspaceID;
		ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Left, 0.25f, nullptr, &dockCenter);
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Right, 0.33f, nullptr, &dockCenter);
		ImGuiID dockRightBottom = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.4f, nullptr, &dockRight);
		ImGuiID dockCenterBottom = ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Down, 0.35f, nullptr, &dockCenter);

		ImGui::DockBuilderDockWindow("Registers##NESDebug", dockLeft);
		ImGui::DockBuilderDockWindow("Composite Viewport##NESDebug", dockCenter);
		ImGui::DockBuilderDockWindow("Nametables##NESDebug", dockCenter);
		ImGui::DockBuilderDockWindow("Pattern Tables##NESDebug", dockRight);
		ImGui::DockBuilderDockWindow("OAM / Sprites##NESDebug", dockRightBottom);
		ImGui::DockBuilderDockWindow("Palettes##NESDebug", dockRightBottom);

		ImGui::DockBuilderFinish(dockspaceID);
		nesDebugger.ppu.dockLayoutBuilt = YES;
	}

	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	if (nesDebugger.ppu.showRegisters)
	{
		ImGui::SetNextWindowDockID(dockspaceID, ImGuiCond_FirstUseEver);
		ImGui::Begin("Registers##NESDebug", &nesDebugger.ppu.showRegisters);
		renderNESDebuggerRegistersPanel();
		ImGui::End();
	}
	if (nesDebugger.ppu.showNametables)
	{
		ImGui::SetNextWindowDockID(dockspaceID, ImGuiCond_FirstUseEver);
		ImGui::Begin("Nametables##NESDebug", &nesDebugger.ppu.showNametables);
		renderNESDebuggerNametablesPanel();
		ImGui::End();
	}
	if (nesDebugger.ppu.showCompositeViewport)
	{
		ImGui::SetNextWindowDockID(dockspaceID, ImGuiCond_FirstUseEver);
		ImGui::Begin("Composite Viewport##NESDebug", &nesDebugger.ppu.showCompositeViewport);
		renderNESDebuggerCompositePanel();
		ImGui::End();
	}
	if (nesDebugger.ppu.showPatternTables)
	{
		ImGui::SetNextWindowDockID(dockspaceID, ImGuiCond_FirstUseEver);
		ImGui::Begin("Pattern Tables##NESDebug", &nesDebugger.ppu.showPatternTables);
		renderNESDebuggerPatternTablesPanel();
		ImGui::End();
	}
	if (nesDebugger.ppu.showOAMViewer)
	{
		ImGui::SetNextWindowDockID(dockspaceID, ImGuiCond_FirstUseEver);
		ImGui::Begin("OAM / Sprites##NESDebug", &nesDebugger.ppu.showOAMViewer);
		renderNESDebuggerOAMPanel();
		ImGui::End();
	}
	if (nesDebugger.ppu.showPaletteViewer)
	{
		ImGui::SetNextWindowDockID(dockspaceID, ImGuiCond_FirstUseEver);
		ImGui::Begin("Palettes##NESDebug", &nesDebugger.ppu.showPaletteViewer);
		renderNESDebuggerPalettePanel();
		ImGui::End();
	}
}

void NES_t::debugRebuildCompositePixels()
{
	auto& ppuCtrl = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl;
	const int chrTable = ppuCtrl.PPUCTRL.ppuctrl.BG_PATTERN_TABLE_ADDR;

	// Reconstruct true scroll position from the loopy v register + fine-X, exactly the way
	// real hardware derives it -- this correctly spans a 2x2 nametable virtual space (512x480),
	// wrapping/crossing nametable boundaries the same way a scrolling screen actually does.
	const int scrollX = (nesDebugger.ppu.debugFrozenScrollValid == YES) ? nesDebugger.ppu.debugFrozenScrollX : 0;
	const int scrollY = (nesDebugger.ppu.debugFrozenScrollValid == YES) ? nesDebugger.ppu.debugFrozenScrollY : 0;

	if (nesDebugger.ppu.compositeShowBG == YES)
	{
		for (int py = 0; py < 240; py++)
		{
			const int virtualY = (scrollY + py) % 480;
			const int ntVertical = virtualY / 240;
			const int localY = virtualY % 240;
			const int tileY = localY / 8;
			const int rowInTile = localY % 8;

			for (int px = 0; px < 256; px++)
			{
				const int virtualX = (scrollX + px) % 512;
				const int ntHorizontal = virtualX / 256;
				const int localX = virtualX % 256;
				const int tileX = localX / 8;
				const int colInTile = localX % 8;

				const int ntIndex = ntHorizontal + ntVertical * 2;	// logical slot; mirroring handled by readPpuRawMemory
				const uint16_t ntBase = (uint16_t)(0x2000 + ntIndex * 0x400);

				uint16_t ntEntryAddr = ntBase + (uint16_t)(tileY * 32 + tileX);
				byte tileIndex = readPpuRawMemory(ntEntryAddr, MEMORY_ACCESS_SOURCE::DEBUG_PORT);

				uint16_t attrAddr = ntBase + 0x03C0 + (uint16_t)((tileY / 4) * 8 + (tileX / 4));
				byte attrByte = readPpuRawMemory(attrAddr, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
				int quadrantShift = (((tileY / 2) & ONE) * 4) + (((tileX / 2) & ONE) * 2);
				byte paletteIndex = (attrByte >> quadrantShift) & 0x03;

				uint16_t tileByteOffset = (uint16_t)(chrTable * 0x1000 + tileIndex * 16);
				byte lo = readPpuRawMemory(tileByteOffset + rowInTile, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
				byte hi = readPpuRawMemory(tileByteOffset + rowInTile + 8, MEMORY_ACCESS_SOURCE::DEBUG_PORT);

				const uint8_t bit = 7 - colInTile;
				const uint8_t colorIdx = (((hi >> bit) & ONE) << ONE) | ((lo >> bit) & ONE);

				uint16_t addr = (colorIdx == 0) ? PALETTE_RAM_INDEXES_START_ADDRESS
					: (PALETTE_RAM_INDEXES_START_ADDRESS + (paletteIndex << TWO) + colorIdx);
				byte palIdx = readPpuRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;

				debugCompositePixels[py * 256 + px] = palScreen[palIdx];
			}
		}
	}
	else
	{
		byte backdropIdx = readPpuRawMemory(PALETTE_RAM_INDEXES_START_ADDRESS, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;
		debugCompositePixels.fill(palScreen[backdropIdx]);
	}

	if (nesDebugger.ppu.compositeShowSprites == YES)
	{
		// NES sprites are already in screen-space coordinates -- no scroll translation needed,
		// unlike GBC's window/OBJ overlay which had to translate through SCX/SCY.
		const FLAG tallSprites = (ppuCtrl.PPUCTRL.ppuctrl.SPRITE_SIZE == ONE) ? YES : NO;
		const int spriteHeight = tallSprites ? 16 : 8;

		for (int s = 63; s >= 0; s--)	// low index = highest priority; draw high-to-low so low ends up on top
		{
			const auto& oam = pNES_ppuMemory->NESMemoryMap.primaryOam.oamW[s];
			if (oam.yPosition >= 240) continue;

			uint16_t patternTable;
			uint8_t baseTileIndex;
			if (tallSprites == YES)
			{
				patternTable = (oam.tileID & ONE) ? 0x1000 : 0x0000;
				baseTileIndex = oam.tileID & 0xFE;
			}
			else
			{
				patternTable = ppuCtrl.PPUCTRL.ppuctrl.SPRITE_PATTER_TABLE_ADDR_8x8 ? 0x1000 : 0x0000;
				baseTileIndex = oam.tileID;
			}

			const uint8_t paletteIndex = oam.attributes.fields.palette;
			const FLAG xFlip = oam.attributes.fields.flipHorizontally;
			const FLAG yFlip = oam.attributes.fields.flipVertically;
			const int spriteScreenY = oam.yPosition + 1;	// real hardware quirk: OAM Y is one scanline before the true display row

			for (int row = 0; row < spriteHeight; row++)
			{
				const int py = spriteScreenY + row;
				if (py < 0 || py >= 240) continue;

				const int logicalRow = yFlip ? (spriteHeight - 1 - row) : row;
				const int tileOfRow = logicalRow / 8;
				const int rowInTile = logicalRow % 8;
				uint16_t tileByteOffset = (uint16_t)(patternTable + (baseTileIndex + tileOfRow) * 16);

				byte lo = readPpuRawMemory(tileByteOffset + rowInTile, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
				byte hi = readPpuRawMemory(tileByteOffset + rowInTile + 8, MEMORY_ACCESS_SOURCE::DEBUG_PORT);

				for (int col = 0; col < 8; col++)
				{
					const int px = oam.xPosition + col;
					if (px < 0 || px >= 256) continue;

					const int srcBit = xFlip ? col : (7 - col);
					const uint8_t colorIdx = (((hi >> srcBit) & ONE) << ONE) | ((lo >> srcBit) & ONE);
					if (colorIdx == 0) continue;

					uint16_t addr = PALETTE_RAM_INDEXES_START_ADDRESS + SIXTEEN + (paletteIndex << TWO) + colorIdx;
					byte palIdx = readPpuRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT) & 0x3F;
					debugCompositePixels[py * 256 + px] = palScreen[palIdx];
				}
			}
		}
	}
}

void NES_t::renderNESDebuggerCompositePanel()
{
	auto& ppuCtrl = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl;

	ImGui::Checkbox("BG", &nesDebugger.ppu.compositeShowBG); ImGui::SameLine();
	ImGui::Checkbox("Sprites", &nesDebugger.ppu.compositeShowSprites); ImGui::SameLine();
	ImGui::Checkbox("Show grid", &nesDebugger.ppu.compositeShowGrid);

	if (ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == ZERO)
		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "BG rendering disabled via PPUMASK -- real screen wouldn't show this right now.");
	if (ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == ZERO)
		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Sprite rendering disabled via PPUMASK -- real screen wouldn't show sprites right now.");

	debugRebuildCompositePixels();
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugCompositeTexture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 240, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugCompositePixels.data()));

	ImVec2 avail = ImGui::GetContentRegionAvail();
	const float aspect = 256.0f / 240.0f;
	ImVec2 imgSize = (avail.x / aspect <= avail.y) ? ImVec2(avail.x, avail.x / aspect) : ImVec2(avail.y * aspect, avail.y);
	ImVec2 imgOrigin = ImGui::GetCursorScreenPos();
	ImGui::Image((ImTextureID)(uintptr_t)debugCompositeTexture, imgSize);

	if (nesDebugger.ppu.compositeShowGrid == YES)
	{
		const float scaleX = imgSize.x / 256.0f;
		const float scaleY = imgSize.y / 240.0f;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImU32 gridColor = nesDebugGridColor(nesDebugger.ppu.gridColorWhite);
		for (int col = 0; col <= 32; col++)
			drawList->AddLine(ImVec2(imgOrigin.x + col * 8 * scaleX, imgOrigin.y), ImVec2(imgOrigin.x + col * 8 * scaleX, imgOrigin.y + imgSize.y), gridColor);
		for (int row = 0; row <= 30; row++)
			drawList->AddLine(ImVec2(imgOrigin.x, imgOrigin.y + row * 8 * scaleY), ImVec2(imgOrigin.x + imgSize.x, imgOrigin.y + row * 8 * scaleY), gridColor);
	}
}

void NES_t::debugEventViewerCheck()
{
	if (nesDebugger.eventViewer.enabled == NO) RETURN;

	auto& ppuCtrl = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl;
	const int regCount = (int)NES_DEBUG_TRACKED_REGISTER::COUNT;
	uint8_t currentValues[(int)NES_DEBUG_TRACKED_REGISTER::COUNT];
	currentValues[(int)NES_DEBUG_TRACKED_REGISTER::PPUCTRL] = ppuCtrl.PPUCTRL.raw;
	currentValues[(int)NES_DEBUG_TRACKED_REGISTER::PPUMASK] = ppuCtrl.PPUMASK.raw;
	currentValues[(int)NES_DEBUG_TRACKED_REGISTER::PPUSTATUS] = ppuCtrl.PPUSTATUS.raw;
	currentValues[(int)NES_DEBUG_TRACKED_REGISTER::OAMADDR] = ppuCtrl.OAMADDR;
	currentValues[(int)NES_DEBUG_TRACKED_REGISTER::OAMDATA] = ppuCtrl.OAMDATA;
	currentValues[(int)NES_DEBUG_TRACKED_REGISTER::PPUSCROLL] = ppuCtrl.PPUSCROLL;
	currentValues[(int)NES_DEBUG_TRACKED_REGISTER::PPUADDR] = ppuCtrl.PPUADDR;
	currentValues[(int)NES_DEBUG_TRACKED_REGISTER::PPUDATA] = ppuCtrl.PPUDATA;
	currentValues[(int)NES_DEBUG_TRACKED_REGISTER::OAMDMA] = pNES_cpuMemory->NESMemoryMap.apuAndIO.OAMDMA;

	const int curScanline = (int)pNES_instance->NES_state.display.currentScanline;
	const int curDot = (int)pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;
	auto& ev = nesDebugger.eventViewer;

	if (curScanline == 0 && ev.lastScanline != 0)
	{
		ev.count = 0;
		ev.head = 0;
		ev.frameCounter++;
	}
	ev.lastScanline = curScanline;

	if (ev.snapshotValid == NO)
	{
		for (int r = 0; r < regCount; r++) ev.lastValues[r] = currentValues[r];
		ev.snapshotValid = YES;
		RETURN;
	}

	for (int r = 0; r < regCount; r++)
	{
		if (currentValues[r] == ev.lastValues[r]) continue;

		NESPPUEvent_t& e = ev.ring[ev.head];
		e.frameNumber = ev.frameCounter;
		e.scanline = (int16_t)curScanline;
		e.dot = (uint16_t)curDot;
		e.registerIndex = (uint8_t)r;
		e.oldValue = ev.lastValues[r];
		e.newValue = currentValues[r];
		e.pc = pNES_cpuRegisters->pc;

		ev.head = (ev.head + 1) % nesDebugger_t::eventViewer_t::CAPACITY;
		if (ev.count < nesDebugger_t::eventViewer_t::CAPACITY) ev.count++;

		ev.lastValues[r] = currentValues[r];
	}
}

static const char* nesScanlinePhaseName(int scanline)
{
	if (scanline == -1) RETURN "Pre-render";
	if (scanline >= 0 && scanline <= 239) RETURN "Visible";
	if (scanline == 240) RETURN "Post-render";
	if (scanline >= 241 && scanline <= 260) RETURN "VBlank";
	RETURN "?";
}

static ImU32 nesScanlinePhaseColor(int scanline)
{
	if (scanline == -1) RETURN IM_COL32(150, 100, 255, 255);		// pre-render -- purple
	if (scanline >= 0 && scanline <= 239) RETURN IM_COL32(40, 90, 40, 255);	// visible -- green
	if (scanline == 240) RETURN IM_COL32(120, 70, 30, 255);		// post-render -- brown (was gray -- see next fix for why that was confusing)
	RETURN IM_COL32(60, 20, 70, 255);								// vblank -- dark purple
}

void NES_t::renderNESDebuggerEventViewerTab()
{
	ImGui::Checkbox("Enable event tracking", &nesDebugger.eventViewer.enabled);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Master switch. Off = zero cost, same as no debugger.\nOnly detects WRITES (value changes). PPU only for now -- CPU/APU coming later, same as GBC.");

	ImGui::SameLine();
	if (ImGui::Button("Clear"))
	{
		nesDebugger.eventViewer.count = 0;
		nesDebugger.eventViewer.head = 0;
		nesDebugger.eventViewer.snapshotValid = NO;
	}

	if (nesDebugger.eventViewer.enabled == NO)
	{
		ImGui::TextDisabled("Enable tracking to begin recording register changes.");
		RETURN;
	}

	static const char* regNames[] = { "PPUCTRL", "PPUMASK", "PPUSTATUS", "OAMADDR", "OAMDATA", "PPUSCROLL", "PPUADDR", "PPUDATA", "OAMDMA" };
	static const ImU32 regColors[] = {
		IM_COL32(255, 215, 0, 255), IM_COL32(180, 255, 60, 255), IM_COL32(255, 100, 180, 255),
		IM_COL32(100, 200, 255, 255), IM_COL32(120, 255, 120, 255), IM_COL32(255, 255, 255, 255),
		IM_COL32(0, 255, 200, 255), IM_COL32(255, 140, 0, 255), IM_COL32(255, 0, 0, 255)
	};
	const int regCount = (int)NES_DEBUG_TRACKED_REGISTER::COUNT;
	const int CAP = nesDebugger_t::eventViewer_t::CAPACITY;

	NESPPUEvent_t* events = nesDebugger.eventViewer.ring;
	int eventCount = nesDebugger.eventViewer.count;
	int eventHead = nesDebugger.eventViewer.head;

	auto chronoIndex = [&](int i) -> int
		{
			int start = ((eventHead - eventCount) % CAP + CAP) % CAP;
			RETURN(start + i) % CAP;
		};

	ImVec2 fullAvail = ImGui::GetContentRegionAvail();
	float rightPanelWidth = fullAvail.x * 0.22f;
	if (rightPanelWidth < 160.0f) rightPanelWidth = 160.0f;
	if (rightPanelWidth > 260.0f) rightPanelWidth = 260.0f;

	// Aspect-locked to the true 341:262 dot/scanline ratio -- previously scaleX/scaleY were
	// computed independently against a fixed-height box, which stretched or squeezed the
	// picture depending on the window's actual proportions.
	const float scatterAspect = 341.0f / 262.0f;
	float scatterWidth = fullAvail.x - rightPanelWidth - 8.0f;
	float scatterHeight = scatterWidth / scatterAspect;
	const float maxScatterHeight = fullAvail.y * 0.6f;	// leave room for the log table below
	if (scatterHeight > maxScatterHeight)
	{
		scatterHeight = maxScatterHeight;
		scatterWidth = scatterHeight * scatterAspect;
	}

	ImGui::BeginChild("NESEventScatter", ImVec2(scatterWidth, scatterHeight), true);
	{
		ImVec2 scatterAvail = ImGui::GetContentRegionAvail();
		ImVec2 scatterOrigin = ImGui::GetCursorScreenPos();
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		const float scale = scatterAvail.x / 341.0f;	// single locked scale -- aspect-correct
		const float scaleX = scale;
		const float scaleY = scale;

		for (int scanline = -1; scanline <= 260; scanline++)
		{
			const int displayRow = scanline + 1;	// shift only for pixel math, never for logic/labels
			float rowY0 = scatterOrigin.y + displayRow * scaleY;
			float rowY1 = rowY0 + scaleY;

			if (scanline < 0 || scanline > 239)
			{
				drawList->AddRectFilled(ImVec2(scatterOrigin.x, rowY0), ImVec2(scatterOrigin.x + scatterAvail.x, rowY1), nesScanlinePhaseColor(scanline));
				continue;
			}

			int d = 0;
			while (d < 341)
			{
				ImU32 col;
				int runStart = d;
				if (d >= 1 && d <= 256)
				{
					const Pixel& p = pNES_instance->NES_state.display.imGuiBuffer.imGuiBuffer2D[scanline][d - 1];
					col = IM_COL32(p.r, p.g, p.b, 255);
					d++;
				}
				else
				{
					// dots 257-340: sprite eval / next-line prefetch -- still part of the
					// "Visible" scanline's real activity, just not producing a screen pixel.
					// Distinct gray from post-render's brown so the two are never confused.
					col = IM_COL32(50, 50, 50, 255);
					while (d < 341 && (d < 1 || d > 256)) d++;
				}
				float x0 = scatterOrigin.x + runStart * scaleX;
				float x1 = scatterOrigin.x + d * scaleX;
				drawList->AddRectFilled(ImVec2(x0, rowY0), ImVec2(x1, rowY1), col);
			}
		}

		drawList->AddRect(scatterOrigin, ImVec2(scatterOrigin.x + scatterAvail.x, scatterOrigin.y + scatterAvail.y), IM_COL32(120, 120, 120, 255));

		for (int i = 0; i < eventCount; i++)
		{
			const NESPPUEvent_t& e = events[chronoIndex(i)];
			if (nesDebugger.eventViewer.showRegister[e.registerIndex] == NO) continue;

			float px = scatterOrigin.x + e.dot * scaleX;
			float py = scatterOrigin.y + (e.scanline + 1) * scaleY;
			drawList->AddRectFilled(ImVec2(px - 1.5f, py - 1.5f), ImVec2(px + 1.5f, py + 1.5f), e.registerIndex < 9 ? regColors[e.registerIndex] : IM_COL32(255, 255, 255, 255));
		}

		ImGui::Dummy(scatterAvail);

		if (ImGui::IsItemHovered())
		{
			ImVec2 mouse = ImGui::GetMousePos();
			int hoverDot = (int)((mouse.x - scatterOrigin.x) / scaleX);
			int hoverScanline = (int)((mouse.y - scatterOrigin.y) / scaleY) - 1;

			if (hoverDot >= 0 && hoverDot < 341 && hoverScanline >= -1 && hoverScanline <= 260)
			{
				ImGui::BeginTooltip();
				ImGui::Text("Scanline: %d   Dot: %d", hoverScanline, hoverDot);
				ImGui::Text("Phase: %s", nesScanlinePhaseName(hoverScanline));

				FLAG anyEventHere = NO;
				for (int i = 0; i < eventCount; i++)
				{
					const NESPPUEvent_t& e = events[chronoIndex(i)];
					if ((int)e.scanline != hoverScanline || (int)e.dot != hoverDot) continue;
					if (nesDebugger.eventViewer.showRegister[e.registerIndex] == NO) continue;

					if (anyEventHere == NO)
					{
						ImGui::Separator(); anyEventHere = YES;
					}
					ImGui::Text("%s: 0x%02X -> 0x%02X  (PC:%04X)", regNames[e.registerIndex], e.oldValue, e.newValue, e.pc);
				}
				ImGui::EndTooltip();
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("NESEventRegTree", ImVec2(rightPanelWidth, scatterHeight), true);
	ImGui::Separator();
	if (ImGui::TreeNodeEx("PPU", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (int r = 0; r < regCount; r++)
		{
			ImGui::PushID(r);
			ImGui::ColorButton("##col", ImGui::ColorConvertU32ToFloat4(regColors[r]), ImGuiColorEditFlags_NoTooltip, ImVec2(10, 10));
			ImGui::SameLine();
			ImGui::Checkbox(regNames[r], &nesDebugger.eventViewer.showRegister[r]);
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	ImGui::BeginDisabled();
	if (ImGui::TreeNodeEx("CPU (Coming Soon)")) ImGui::TreePop();
	if (ImGui::TreeNodeEx("APU (Coming Soon)")) ImGui::TreePop();
	ImGui::EndDisabled();
	ImGui::EndChild();

	static std::vector<int> filteredIndices;
	filteredIndices.clear();
	for (int i = 0; i < eventCount; i++)
	{
		const NESPPUEvent_t& e = events[chronoIndex(i)];
		if (nesDebugger.eventViewer.showRegister[e.registerIndex] == YES)
			filteredIndices.push_back(chronoIndex(i));
	}

	ImGui::Text("Event log (%d of %d events shown):", (int)filteredIndices.size(), eventCount);

	ImGui::BeginChild("NESEventLogTable", ImVec2(0, 0), true);
	if (ImGui::BeginTable("NESEventLog", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
	{
		ImGui::TableSetupColumn("Scanline");
		ImGui::TableSetupColumn("Dot");
		ImGui::TableSetupColumn("Event");
		ImGui::TableSetupColumn("Old Value");
		ImGui::TableSetupColumn("New Value");
		ImGui::TableSetupColumn("PC");
		ImGui::TableHeadersRow();

		ImGuiListClipper clipper;
		clipper.Begin((int)filteredIndices.size());
		while (clipper.Step())
		{
			for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
			{
				const NESPPUEvent_t& e = events[filteredIndices[row]];
				ImGui::TableNextRow();
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, (regColors[e.registerIndex] & 0x00FFFFFF) | 0x30000000);
				ImGui::TableNextColumn(); ImGui::Text("%d", (int)e.scanline);
				ImGui::TableNextColumn(); ImGui::Text("%d", (int)e.dot);
				ImGui::TableNextColumn(); ImGui::TextUnformatted(regNames[e.registerIndex]);
				ImGui::TableNextColumn(); ImGui::Text("0x%02X", e.oldValue);
				ImGui::TableNextColumn(); ImGui::Text("0x%02X", e.newValue);
				ImGui::TableNextColumn(); ImGui::Text("ROM:%04X", e.pc);
			}
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void NES_t::renderNESDebuggerUI()
{
	if (nesDebugger.windowOpen == NO) RETURN;

	debugEnsureTexturesCreated();

	enum class DEBUGGER_TAB
	{
		PPU,
		CPU,
		APU,
		EVENT_VIEWER
	};

	static DEBUGGER_TAB activeTab = DEBUGGER_TAB::PPU;

	ImGuiWindowFlags winFlags = ImGuiWindowFlags_MenuBar;
	if (nesDebugger.ppu.fullscreen == YES)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		winFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	}
	else
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float defaultWidth = 1700.0f;
		const float defaultHeight = 950.0f;
		ImGui::SetNextWindowPos(
			ImVec2(
				viewport->WorkPos.x + (viewport->WorkSize.x - defaultWidth) * 0.5f,
				viewport->WorkPos.y + 40.0f),
			ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(defaultWidth, defaultHeight), ImGuiCond_FirstUseEver);
	}

	ImGui::Begin("NES PPU Debugger", &nesDebugger.windowOpen, winFlags);

	if (ImGui::BeginMenuBar())
	{
		bool isFullscreen = (nesDebugger.ppu.fullscreen == YES);
		if (ImGui::MenuItem(isFullscreen ? "Exit Fullscreen" : "Fullscreen"))
			nesDebugger.ppu.fullscreen = isFullscreen ? NO : YES;

		ImGui::Separator();

		switch (activeTab)
		{
		case DEBUGGER_TAB::PPU:
		{
			if (ImGui::BeginMenu("Panels"))
			{
				ImGui::MenuItem("Registers", NULL, (bool*)&nesDebugger.ppu.showRegisters);
				ImGui::MenuItem("Pattern Tables", NULL, (bool*)&nesDebugger.ppu.showPatternTables);
				ImGui::MenuItem("Composite Viewport", NULL, (bool*)&nesDebugger.ppu.showCompositeViewport);
				ImGui::MenuItem("Nametables", NULL, (bool*)&nesDebugger.ppu.showNametables);
				ImGui::MenuItem("OAM / Sprites", NULL, (bool*)&nesDebugger.ppu.showOAMViewer);
				ImGui::MenuItem("Palettes", NULL, (bool*)&nesDebugger.ppu.showPaletteViewer);
				ImGui::EndMenu();
			}
			break;
		}

		case DEBUGGER_TAB::CPU:
		case DEBUGGER_TAB::APU:
		case DEBUGGER_TAB::EVENT_VIEWER:
		{
			ImGui::BeginDisabled();
			if (ImGui::BeginMenu("Panels"))
				ImGui::EndMenu();
			ImGui::EndDisabled();
			break;
		}
		}

		ImGui::EndMenuBar();
	}

	if (ImGui::BeginTabBar("NESDebuggerTabs"))
	{
		if (ImGui::BeginTabItem("PPU"))
		{
			activeTab = DEBUGGER_TAB::PPU;
			renderNESDebuggerPPUTab();
			ImGui::EndTabItem();
		}

		ImGui::BeginDisabled();
		if (ImGui::BeginTabItem("CPU (Coming Soon)"))
		{
			activeTab = DEBUGGER_TAB::CPU;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("APU (Coming Soon)"))
		{
			activeTab = DEBUGGER_TAB::APU;
			ImGui::EndTabItem();
		}
		ImGui::EndDisabled();

		if (ImGui::BeginTabItem("Event Viewer"))
		{
			activeTab = DEBUGGER_TAB::EVENT_VIEWER;
			renderNESDebuggerEventViewerTab();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

#endif // !__RPI_PICO__
