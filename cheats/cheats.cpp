#pragma region CHEAT_SPECIFIC_INCLUDES
#include "cheats.h"
#pragma endregion CHEAT_SPECIFIC_INCLUDES

#pragma region CHEAT_DEFINITIONS
CheatEngine_t::CheatEngine_t()
{
	currentEngineMode = CHEATING_ENGINE::GAMEGENIE;
}

CheatEngine_t::~CheatEngine_t()
{
	;
}

FLAG CheatEngine_t::setCheatEngineMode(CHEATING_ENGINE engine, EMULATION_ID id)
{
	currentEngineMode = engine;

	currentID = id;

	RETURN SUCCESS;
}

CheatEngine_t::CHEATING_ENGINE CheatEngine_t::getCheatEngineMode()
{
	RETURN currentEngineMode;
}

FLAG CheatEngine_t::applyNewCheat(std::string name, std::string cheat)
{
	FLAG status = FAILURE;
	int32_t address = INVALID;
	int16_t data = INVALID;
	int16_t other1 = INVALID;

	if (cheatCount[TO_UINT8(currentEngineMode)] > MAX_CHEAT_COUNT_PER_ENGINE)
	{
		RETURN NO;
	}

	++cheatCount[TO_UINT8(currentEngineMode)];

	cheatNames[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)][cheat] = name;

	// TODO: Basically, some cheats encode more than 1 address (for eg, gameshark for gameboy; https://bsfree.org/hack/hacking_gb.html)
	// We are currently handling such cheats here itelf, but in future this needs to be done inside decodeAddressAndData

	std::vector<std::string> inCheat;
	if (currentEngineMode == CHEATING_ENGINE::GAMESHARK)
	{
		if (currentID == EMULATION_ID::GB_GBC_ID && cheat.size() > EIGHT)
		{
			for (std::string::size_type ii = RESET; ii < cheat.size(); ii = ii + EIGHT)
			{
				inCheat.push_back(cheat.substr(ii, EIGHT));
			}
		}
		else
		{
			inCheat.push_back(cheat);
		}
	}
	else
	{
		inCheat.push_back(cheat);
	}

	for (const std::string& ii : inCheat)
	{
		address = INVALID;
		data = INVALID;
		other1 = INVALID;
		if (decodeAddressAndData(ii, &address, &data, &other1))
		{
			fakeData[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)][address].first = data;
			fakeData[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)][address].second = other1;
		}
	}

	RETURN YES;
}

FLAG CheatEngine_t::enableCheat(std::string cheat)
{
	int32_t address = INVALID;
	int16_t data = INVALID;
	int16_t other1 = INVALID;

	std::vector<std::string> inCheat;
	if (currentEngineMode == CHEATING_ENGINE::GAMESHARK)
	{
		if (currentID == EMULATION_ID::GB_GBC_ID && cheat.size() > EIGHT)
		{
			for (std::string::size_type ii = RESET; ii < cheat.size(); ii = ii + EIGHT)
			{
				inCheat.push_back(cheat.substr(ii, EIGHT));
			}
		}
		else
		{
			inCheat.push_back(cheat);
		}
	}
	else
	{
		inCheat.push_back(cheat);
	}

	for (const std::string& ii : inCheat)
	{
		address = INVALID;
		data = INVALID;
		other1 = INVALID;
		if (decodeAddressAndData(ii, &address, &data, &other1))
		{
			fakeData[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)][address].first = data;
			fakeData[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)][address].second = other1;
		}
		else
		{
			RETURN FAILURE;
		}
	}

	RETURN SUCCESS;
}

FLAG CheatEngine_t::disableCheat(std::string cheat)
{
	int32_t address = INVALID;
	int16_t data = INVALID;
	int16_t other1 = INVALID;

	std::vector<std::string> inCheat;
	if (currentEngineMode == CHEATING_ENGINE::GAMESHARK)
	{
		if (currentID == EMULATION_ID::GB_GBC_ID && cheat.size() > EIGHT)
		{
			for (std::string::size_type ii = RESET; ii < cheat.size(); ii = ii + EIGHT)
			{
				inCheat.push_back(cheat.substr(ii, EIGHT));
			}
		}
		else
		{
			inCheat.push_back(cheat);
		}
	}
	else
	{
		inCheat.push_back(cheat);
	}

	for (const std::string& ii : inCheat)
	{
		address = INVALID;
		data = INVALID;
		other1 = INVALID;
		if (decodeAddressAndData(ii, &address, &data, &other1))
		{
			fakeData[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)].erase(address);
		}
		else
		{
			RETURN FAILURE;
		}
	}

	RETURN SUCCESS;
}

FLAG CheatEngine_t::deleteCheat(std::string cheat)
{
	int32_t address = INVALID;
	int16_t data = INVALID;
	int16_t other1 = INVALID;

	if (cheatCount[TO_UINT8(currentEngineMode)] == ZERO)
	{
		RETURN FAILURE;
	}

	std::vector<std::string> inCheat;
	if (currentEngineMode == CHEATING_ENGINE::GAMESHARK)
	{
		if (currentID == EMULATION_ID::GB_GBC_ID && cheat.size() > EIGHT)
		{
			for (std::string::size_type ii = RESET; ii < cheat.size(); ii = ii + EIGHT)
			{
				inCheat.push_back(cheat.substr(ii, EIGHT));
			}
		}
		else
		{
			inCheat.push_back(cheat);
		}
	}
	else
	{
		inCheat.push_back(cheat);
	}

	for (const std::string& ii : inCheat)
	{
		address = INVALID;
		data = INVALID;
		other1 = INVALID;
		if (decodeAddressAndData(ii, &address, &data, &other1))
		{
			fakeData[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)].erase(address);
		}
		else
		{
			RETURN FAILURE;
		}
	}

	cheatNames[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)].erase(cheat);
	--cheatCount[TO_UINT8(currentEngineMode)];

	RETURN SUCCESS;
}

FLAG CheatEngine_t::interceptCPURead(uint16_t address, int16_t* data, int16_t* other1)
{
	if (fakeData[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)].count(address))
	{
		*data = fakeData[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)][address].first;
		*other1 = fakeData[TO_UINT8(currentEngineMode)][TO_UINT8(currentID)][address].second;
		RETURN SUCCESS;
	}

	RETURN FAILURE;
}

FLAG CheatEngine_t::listAllTheCheats(CHEATING_ENGINE engine, FLAG* en)
{
	INC8 ii = RESET;
	for (auto& [key, value] : cheatNames[TO_UINT8(engine)][TO_UINT8(currentID)])
	{
		if (ii < MAX_CHEAT_COUNT_PER_ENGINE)
		{
			en[ii] = YES;
			ImGui::Checkbox(value.c_str(), &(en[ii]));
			ii++;
		}
	}

	RETURN SUCCESS;
}

std::unordered_map<std::string, std::string> CheatEngine_t::getCheatList(CHEATING_ENGINE engine)
{
	RETURN cheatNames[TO_UINT8(engine)][TO_UINT8(currentID)];
}

FLAG CheatEngine_t::saveCheatNames(const std::string& filename)
{
	std::ofstream ofs(filename);
	if (!ofs) RETURN FAILURE;

	uint8_t m = TO_UINT8(CHEATING_ENGINE::TOTAL_ENGINES);
	uint8_t n = TO_UINT8(EMULATION_ID::TOTAL_ID);

	ofs << "m=" << static_cast<int>(m) << "\n";
	ofs << "n=" << static_cast<int>(n) << "\n";

	for (int i = 0; i < m; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			auto& map = cheatNames[i][j];
			ofs << "i=" << i << " j=" << j << " size=" << map.size() << "\n";
			for (const auto& [key, val] : map)
			{
				ofs << key << "=" << val << "\n";
			}
		}
	}

	RETURN SUCCESS;
}

FLAG CheatEngine_t::loadCheatNames(const std::string& filename)
{
	uint8_t m = TO_UINT8(CHEATING_ENGINE::TOTAL_ENGINES);
	uint8_t n = TO_UINT8(EMULATION_ID::TOTAL_ID);

	std::ifstream ifs(filename);
	if (!ifs) RETURN FAILURE;

	std::string line;
	int file_m = 0, file_n = 0;
	std::getline(ifs, line); sscanf(line.c_str(), "m=%d", &file_m);
	std::getline(ifs, line); sscanf(line.c_str(), "n=%d", &file_n);
	if (file_m != m || file_n != n) RETURN FAILURE;

	while (std::getline(ifs, line))
	{
		int i, j, size;
		if (sscanf(line.c_str(), "i=%d j=%d size=%d", &i, &j, &size) != 3)
			continue;

		auto& map = cheatNames[i][j];
		map.clear();

		for (int k = 0; k < size; ++k)
		{
			if (!std::getline(ifs, line)) BREAK;
			auto pos = line.find('=');
			if (pos != std::string::npos)
			{
				std::string key = line.substr(0, pos);
				std::string val = line.substr(pos + 1);
				map[key] = val;
				++cheatCount[i];
			}
		}
	}

	RETURN SUCCESS;
}

FLAG CheatEngine_t::decodeAddressAndData(std::string cheat, int32_t* address, int16_t* data, int16_t* other1)
{
	// Refer to https://tuxnes.sourceforge.net/gamegenie.html

	*address = INVALID;
	*data = INVALID;
	*other1 = INVALID;

	std::vector<BYTE> decodeStage2(cheat.size());

	if (!cheat.empty())
	{
		if (currentEngineMode == CHEATING_ENGINE::GAMEGENIE)
		{
			if (currentID == EMULATION_ID::NES_ID)
			{
				for (std::string::size_type ii = 0; ii < cheat.size(); ++ii)
				{
					decodeStage2[ii] = GGDecodeNES[cheat[ii]];
				}

				*address = 0x8000 +
					((decodeStage2[3] & 7) << 12)
					| ((decodeStage2[5] & 7) << 8) | ((decodeStage2[4] & 8) << 8)
					| ((decodeStage2[2] & 7) << 4) | ((decodeStage2[1] & 8) << 4)
					| (decodeStage2[4] & 7) | (decodeStage2[3] & 8);

				if (cheat.size() == SIX)
				{
					*data =
						((decodeStage2[1] & 7) << 4) | ((decodeStage2[0] & 8) << 4)
						| (decodeStage2[0] & 7) | (decodeStage2[5] & 8);

					*other1 = INVALID;

					RETURN SUCCESS;
				}
				else if (cheat.size() == EIGHT)
				{
					*data =
						((decodeStage2[1] & 7) << 4) | ((decodeStage2[0] & 8) << 4)
						| (decodeStage2[0] & 7) | (decodeStage2[7] & 8);

					*other1 =
						((decodeStage2[7] & 7) << 4) | ((decodeStage2[6] & 8) << 4)
						| (decodeStage2[6] & 7) | (decodeStage2[5] & 8);

					RETURN SUCCESS;
				}
			}
			else if (currentID == EMULATION_ID::GB_GBC_ID)
			{
				// Refer to https://gbdev.io/pandocs/Shark_Cheats.html
				// For oDat, refer to https://www.youtube.com/watch?v=C86OsYRACTM&ab_channel=RetroGameMechanicsExplained
				// The youtube link tells us to first rotate left by 6 and then XOR with 0xBA (And this seems to be working!)

				std::string nDat = { cheat[0], cheat[1] };
				std::string addr = { cheat[6], cheat[2], cheat[4], cheat[5] };
				std::string oDat = { cheat[8], cheat[10] };

				*address = std::stoi(addr, nullptr, 16) ^ 0xF000;
				*data = std::stoi(nDat, nullptr, 16);
				auto temp = std::stoi(oDat, nullptr, 16);
				*other1 = (((temp << 6) | (temp >> 2)) & 0xFF) ^ 0xBA;

				RETURN SUCCESS;
			}
		}
		else if (currentEngineMode == CHEATING_ENGINE::GAMESHARK)
		{
			if (currentID == EMULATION_ID::GB_GBC_ID)
			{
				// Also Refer to https://gbdev.io/pandocs/Shark_Cheats.html
				// Also refer to https://www.reddit.com/r/pokemoncrystal/comments/1dfmwca/gameshark_codes_and_you_a_basic_guide_on_how_they/

				std::string nDat = { cheat[2], cheat[3] };
				std::string addr = { cheat[6], cheat[7], cheat[4], cheat[5] };
				std::string type = { cheat[0], cheat[1] };

				*address = std::stoi(addr, nullptr, 16);
				*data = std::stoi(nDat, nullptr, 16);
				*other1 = std::stoi(type, nullptr, 16);

				RETURN SUCCESS;
			}
		}

		RETURN FAILURE;
	}
	else
	{
		RETURN FAILURE;
	}
}
#pragma endregion CHEAT_DEFINITIONS