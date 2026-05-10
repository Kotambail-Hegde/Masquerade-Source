#pragma once
#pragma region INCLUDES
#include "helpers.h"
#pragma endregion INCLUDES
#pragma region MACROS
#define MAX_CHEAT_COUNT_PER_ENGINE 12
#pragma endregion MACROS
#pragma region CORE
class CheatEngine_t
{
public:
	enum CHEATING_ENGINE : uint8_t
	{
		GAMEGENIE,
		GAMESHARK,
		ACTION_REPLAY_V3,
		CODEBREAKER,
		TOTAL_ENGINES
	};
	// GBA prep – not used by NES/GB yet
	enum class CheatWidth : uint8_t
	{
		U8 = 1,
		U16 = 2,
		U32 = 4
	};
private:
	CHEATING_ENGINE currentEngineMode;
	EMULATION_ID currentID;
	INC8 cheatCount[CHEATING_ENGINE::TOTAL_ENGINES] = { ZERO };
	std::unordered_map<std::string, std::string> cheatNames[CHEATING_ENGINE::TOTAL_ENGINES][EMULATION_ID::TOTAL_ID];
	std::unordered_map<char, uint8_t> GGDecodeNES =
	{
		{'A', 0x0}, {'P', 0x1}, {'Z', 0x2}, {'L', 0x3},
		{'G', 0x4}, {'I', 0x5}, {'T', 0x6}, {'Y', 0x7},
		{'E', 0x8}, {'O', 0x9}, {'X', 0xA}, {'U', 0xB},
		{'K', 0xC}, {'S', 0xD}, {'V', 0xE}, {'N', 0xF}
	};
	struct CheatPatch_t
	{
		uint32_t   data = 0;
		uint32_t   compare = 0;    // replaces other/other1
		FLAG       hasCompare = NO;   // explicit – no INVALID sentinel
		CheatWidth width = CheatWidth::U8;  // GBA prep
		FLAG       enabled = NO;
	};

	struct CheatWrite_t
	{
		uint32_t   address;
		uint32_t   data;
		CheatWidth width;
	};

	// key widened to uint32_t for GBA prep
	std::unordered_map<uint32_t, CheatPatch_t> fakeData[CHEATING_ENGINE::TOTAL_ENGINES][EMULATION_ID::TOTAL_ID];
	std::unordered_map<std::string, FLAG> cheatEnabled[CHEATING_ENGINE::TOTAL_ENGINES][EMULATION_ID::TOTAL_ID];
	std::unordered_map<std::string, std::vector<std::string>> cheatSubCodes[CHEATING_ENGINE::TOTAL_ENGINES][EMULATION_ID::TOTAL_ID];
	std::unordered_map<std::string, uint32_t> cheatSubCodeCount[CHEATING_ENGINE::TOTAL_ENGINES][EMULATION_ID::TOTAL_ID];

public:
	CheatEngine_t();
	~CheatEngine_t();
public:
	FLAG setCheatEngineMode(CHEATING_ENGINE engine, EMULATION_ID id);
	CHEATING_ENGINE getCheatEngineMode();
	FLAG applyNewCheat(std::string name, std::string cheat);
	MASQ_INLINE uint32_t getSubCodeCount(CHEATING_ENGINE engine, const std::string& key)
	{
		auto& map = cheatSubCodeCount[TO_UINT8(engine)][TO_UINT8(currentID)];
		auto it = map.find(key);
		RETURN (it != map.end()) ? it->second : 1;
	}
	FLAG enableCheat(std::string cheat);
	FLAG disableCheat(std::string cheat);
	FLAG deleteCheat(std::string cheat);
	// address widened; compare + hasCompare exposed so call sites can use them
	FLAG interceptCPURead(CHEATING_ENGINE engine, uint32_t address, uint32_t* data, uint32_t* compare, FLAG* hasCompare);
	// Returns all enabled cheat entries for the current engine + ID.
	// Caller is responsible for applying writes via their own writeRawMemory.
	std::vector<CheatWrite_t> getCheatWrites(CHEATING_ENGINE engine);
	FLAG listAllTheCheats(CHEATING_ENGINE engine, FLAG* en);
	std::unordered_map<std::string, std::string> getCheatList(CHEATING_ENGINE engine);
	MASQ_INLINE std::unordered_map<std::string, FLAG> getCheatEnDisList(CHEATING_ENGINE engine)
	{
		RETURN cheatEnabled[engine][currentID];
	}
	FLAG saveCheatNames(const std::string& filename);
	FLAG loadCheatNames(const std::string& filename);
private:
	FLAG decodeAddressAndData(const std::string& cheat,
		uint32_t* address,
		uint32_t* data,
		uint32_t* compare,
		FLAG* hasCompare,
		CheatWidth* width);
};
#pragma endregion CORE
