#pragma once

#pragma region INCLUDES
#include "helpers.h"
#pragma endregion INCLUDES

#pragma region MACROS
#define MAX_CHEAT_COUNT_PER_ENGINE			12
#pragma endregion MACROS

#pragma region CORE
class CheatEngine_t
{
public:

	enum class CHEATING_ENGINE : uint8_t
	{
		GAMEGENIE,
		GAMESHARK,
		TOTAL_ENGINES
	};

private:

	CHEATING_ENGINE currentEngineMode;

	EMULATION_ID currentID;

	INC8 cheatCount[TO_UINT8(CHEATING_ENGINE::TOTAL_ENGINES)] = { ZERO };

	std::unordered_map<std::string, std::string> cheatNames[TO_UINT8(CHEATING_ENGINE::TOTAL_ENGINES)][TO_UINT8(EMULATION_ID::TOTAL_ID)];
	std::unordered_map<char, uint8_t> GGDecodeNES =
	{
		{'A', 0x0}, {'P', 0x1}, {'Z', 0x2}, {'L', 0x3},
		{'G', 0x4}, {'I', 0x5}, {'T', 0x6}, {'Y', 0x7},
		{'E', 0x8}, {'O', 0x9}, {'X', 0xA}, {'U', 0xB},
		{'K', 0xC}, {'S', 0xD}, {'V', 0xE}, {'N', 0xF}
	};
	std::unordered_map<int32_t, std::pair<int16_t, int16_t>> fakeData[TO_UINT8(CHEATING_ENGINE::TOTAL_ENGINES)][TO_UINT8(EMULATION_ID::TOTAL_ID)]; // address -> fake data

public:

	CheatEngine_t();

	~CheatEngine_t();

public:

	FLAG setCheatEngineMode(CHEATING_ENGINE engine, EMULATION_ID id);

	CHEATING_ENGINE getCheatEngineMode();

	FLAG applyNewCheat(std::string name, std::string cheat);

	FLAG enableCheat(std::string cheat);

	FLAG disableCheat(std::string cheat);

	FLAG deleteCheat(std::string cheat);

	FLAG interceptCPURead(uint16_t address, int16_t* data, int16_t* other1);

	FLAG listAllTheCheats(CHEATING_ENGINE engine, FLAG* en);

	std::unordered_map<std::string, std::string> getCheatList(CHEATING_ENGINE engine);

	std::unordered_map<std::string, FLAG> getCheatEnDisList(CHEATING_ENGINE engine);

	FLAG saveCheatNames(const std::string& filename);

	FLAG loadCheatNames(const std::string& filename);

private:

	FLAG decodeAddressAndData(std::string cheat, int32_t* address, int16_t* data, int16_t* other1);
};
#pragma endregion CORE