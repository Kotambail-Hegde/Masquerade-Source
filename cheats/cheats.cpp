#pragma region CHEAT_SPECIFIC_INCLUDES
#include "cheats.h"
#include <sstream>
#pragma endregion CHEAT_SPECIFIC_INCLUDES

// =============================================================================
// cheats.cpp -- Cheat Engine Implementation
// =============================================================================
//
// Unified cheat engine supporting NES, GB/GBC, and GBA cheat code formats.
// See cheats.md for full documentation, format specifications, and limitations.
//
// -----------------------------------------------------------------------------
// SUPPORTED ENGINES AND MECHANISMS
// -----------------------------------------------------------------------------
//
// Platform    Engine              Mechanism         Notes
// ----------  ------------------  ----------------  ----------------------------
// NES         Game Genie          Read-intercept    ROM only, optional compare
// GB/GBC      Game Genie          Read-intercept    ROM only, always has compare
// GB/GBC      GameShark           VBlank write      RAM only
// GBA         GameShark v1/v2     VBlank write      RAM; ROM via read-intercept
//                                 + Read-intercept  Raw/decrypted format only
// GBA         Action Replay v3    VBlank write      RAM; ROM via read-intercept
//                                 + Read-intercept  TEA-encrypted; basic opcodes
// GBA         CodeBreaker/Xploder VBlank write      RAM only; plaintext
//
// -----------------------------------------------------------------------------
// TWO MECHANISMS EXPLAINED
// -----------------------------------------------------------------------------
//
// Read-intercept (interceptCPURead):
//   Called from the platform's readRawMemory at ROM addresses. When a cheat
//   entry exists for the address being read, the stored value is returned
//   instead of the ROM byte. Optional compare byte (hasCompare) ensures the
//   substitution only fires when the original ROM byte matches -- hardware-
//   accurate for GameGenie, which physically compared before substituting.
//   Takes CHEATING_ENGINE as a parameter to avoid modifying currentEngineMode
//   in the read hot path.
//
// VBlank write (getCheatWrites):
//   Called from the platform's VBlank handler. Returns all enabled entries for
//   a given engine, which the caller then writes via writeRawMemory using
//   MEMORY_ACCESS_SOURCE::CPU. Hardware-accurate -- real cheat devices ran
//   their own firmware at VBlank and stomped game values once per frame.
//   ROM addresses (>= GAMEPAK_ROM_WS0_START_ADDRESS for GBA) are skipped at
//   the VBlank call site -- ROM patches are handled exclusively by read-intercept.
//   Takes CHEATING_ENGINE as a parameter to avoid modifying currentEngineMode.
//
// -----------------------------------------------------------------------------
// KEY DESIGN DECISIONS
// -----------------------------------------------------------------------------
//
// Canonicalization:
//   All cheat strings are key-canonicalized (separators stripped) before map
//   storage. "OZTLLX + AATLGZ" and "OZTLLXAATLGZ" produce the same key.
//   Sub-codes are split at applyNewCheat time and stored in cheatSubCodes so
//   enable/disable/delete never need to re-split a canonical key.
//
// Malformed codes:
//   If decodeAddressAndData returns FAILURE for a sub-code, the cheat is still
//   registered in cheatNames so the user can see and delete it via the UI.
//   enable/disable/delete log a warning and skip the bad sub-code rather than
//   aborting, so well-formed sub-codes in the same entry still apply.
//
// Persistence:
//   saveCheatNames writes tab-delimited sub-codes after the name field so that
//   multi-code entries survive save/load. Old files (no tabs) are still readable.
//   Adding a new CHEATING_ENGINE value increments TOTAL_ENGINES and causes old
//   save files to be rejected (version mismatch on load).
//
// -----------------------------------------------------------------------------
// NOT SUPPORTED
// -----------------------------------------------------------------------------
//
// Master codes (all engines):
//   Master codes install hooks into game code that execute within the game's
//   own context. Supporting them requires full code injection. Out of scope.
//
// AR v3 DEADFACE seed-change codes:
//   DEADFACE 0000XXXX codes change the TEA encryption seeds for subsequent
//   codes. Per-code seed state tracking is not implemented. Codes that require
//   a non-default seed will decrypt to garbage and fail decodeAddressAndData.
//
// AR v3 conditional / fill / indirect / IO-area opcodes:
//   Only basic RAM write opcodes (0x00, 0x02, 0x04) and ROM patch opcodes
//   (0x06, 0x07) are implemented. All others return FAILURE.
//
// CodeBreaker type 3 (conditional) and type B (hook/master):
//   Not implemented. Return FAILURE.
//
// GBA GameShark v1/v2 encrypted codes:
//   The v1/v2 encryption algorithm is not publicly documented cleanly.
//   Only raw/decrypted format is accepted.
//
// =============================================================================

// ---------------------------------------------------------------------------
// Internal helpers (file-local)
// ---------------------------------------------------------------------------
#pragma region CHEAT_HELPERS

// TEA decrypt for GBA Action Replay v3/v4.
// NOTE: DEADFACE master codes that change these seeds are not yet handled;
// default seeds are always used. Codes requiring a master code will likely
// produce garbage addresses and silently fail decode (RETURN FAILURE).
static const uint32_t ARV3_SEEDS[4] =
{
	0x7AA9648F, 0x7FAE6994, 0xC0EFAAD5, 0x42712C57
};

static void arv3Decrypt(uint32_t& op1, uint32_t& op2)
{
	uint32_t sum = 0xC6EF3720; // 0x9E3779B9 * 32
	for (int i = 0; i < 32; ++i)
	{
		op2 -= ((op1 << 4) + ARV3_SEEDS[2]) ^ (op1 + sum) ^ ((op1 >> 5) + ARV3_SEEDS[3]);
		op1 -= ((op2 << 4) + ARV3_SEEDS[0]) ^ (op2 + sum) ^ ((op2 >> 5) + ARV3_SEEDS[1]);
		sum -= 0x9E3779B9;
	}
}

// Canonical key: strip separators, keep hex chars.
// "OZTLLX + AATLGZ", "OZTLLX AATLGZ", and "OZTLLXAATLGZ" all produce the same key.
static std::string canonicalize(const std::string& cheat)
{
	std::string result;
	result.reserve(cheat.size());
	for (char c : cheat)
	{
		if (c != ' ' && c != '+' && c != '-' && c != '\n' && c != '\r' && c != '\t')
			result += c;
	}
	return result;
}

// Split a raw cheat string into individual sub-codes based on engine + ID rules.
//
// NES GameGenie
//   Priority: " + " > "+" alone > whitespace > single code.
//   Do NOT strip spaces before whitespace-split — needed to disambiguate 6/8 char codes.
//
// GB/GBC GameGenie
//   XXX-XXX-XXX per code (9 hex chars after dash removal).
//   Multiple codes space-separated.
//
// GB/GBC GameShark
//   8 hex chars per sub-code. Remove all separators, chunk by 8.
//
// GBA GameShark v1/v2
//   16 hex chars per code. Remove all separators, chunk by 16.
//
// GBA Action Replay v3
//   16 hex chars per code. Remove all separators, chunk by 16.
//
// GBA CodeBreaker/Xploder
//   Variable length: type nibble 8 uses 12 chars (8 addr + 4 val),
//   all others use 16 chars (8 addr + 8 val).
//   Strategy: if total divisible by 16, chunk by 16;
//             if total divisible by 12 (not 16), chunk by 12;
//             otherwise sequential parse by first nibble.

static std::vector<std::string> splitCheatCodes(
	const std::string& rawCheat,
	CheatEngine_t::CHEATING_ENGINE engine,
	EMULATION_ID id)
{
	std::vector<std::string> result;

	if (engine == CheatEngine_t::CHEATING_ENGINE::GAMEGENIE)
	{
		if (id == EMULATION_ID::NES_ID)
		{
			if (rawCheat.find(" + ") != std::string::npos)
			{
				// " + " is the delimiter; tokens are already clean
				std::string::size_type start = 0;
				std::string::size_type pos;
				while ((pos = rawCheat.find(" + ", start)) != std::string::npos)
				{
					std::string sub = rawCheat.substr(start, pos - start);
					if (!sub.empty()) result.push_back(sub);
					start = pos + 3;
				}
				std::string last = rawCheat.substr(start);
				if (!last.empty()) result.push_back(last);
			}
			else if (rawCheat.find('+') != std::string::npos)
			{
				std::istringstream ss(rawCheat);
				std::string token;
				while (std::getline(ss, token, '+'))
					if (!token.empty()) result.push_back(token);
			}
			else if (rawCheat.find(' ') != std::string::npos
				|| rawCheat.find('\t') != std::string::npos)
			{
				// Whitespace is the delimiter; do NOT strip — needed to distinguish 6/8 char codes
				std::istringstream ss(rawCheat);
				std::string token;
				while (ss >> token)
					if (!token.empty()) result.push_back(token);
			}
			else
			{
				std::string clean = rawCheat;
				clean.erase(std::remove_if(clean.begin(), clean.end(),
					[](char c) { return c == '\n' || c == '\r' || c == '\t'; }),
					clean.end());
				if (!clean.empty()) result.push_back(clean);
			}
		}
		else if (id == EMULATION_ID::GB_GBC_ID)
		{
			// Remove dashes, +, non-space whitespace; keep space as delimiter
			std::string clean;
			clean.reserve(rawCheat.size());
			for (char c : rawCheat)
			{
				if (c == '-' || c == '+' || c == '\n' || c == '\r' || c == '\t')
					continue;
				clean += c;
			}

			std::istringstream ss(clean);
			std::string token;
			while (ss >> token)
				for (std::string::size_type ii = 0; ii < token.size(); ii += 9)
				{
					std::string sub = token.substr(ii, 9);
					if (sub.size() == 9) result.push_back(sub);
				}

			if (result.empty())
				for (std::string::size_type ii = 0; ii < clean.size(); ii += 9)
				{
					std::string sub = clean.substr(ii, 9);
					if (sub.size() == 9) result.push_back(sub);
				}
		}
	}
	else if (engine == CheatEngine_t::CHEATING_ENGINE::GAMESHARK)
	{
		if (id == EMULATION_ID::GB_GBC_ID)
		{
			std::string clean;
			clean.reserve(rawCheat.size());
			for (char c : rawCheat)
				if (c != ' ' && c != '+' && c != '-' && c != '\n' && c != '\r' && c != '\t')
					clean += c;
			for (std::string::size_type ii = 0; ii < clean.size(); ii += 8)
			{
				std::string sub = clean.substr(ii, 8);
				if (sub.size() == 8) result.push_back(sub);
			}
		}
		else if (id == EMULATION_ID::GBA_ID)
		{
			// GBA GameShark v1/v2 — 16 hex chars per code
			std::string clean;
			clean.reserve(rawCheat.size());
			for (char c : rawCheat)
				if (c != ' ' && c != '+' && c != '-' && c != '\n' && c != '\r' && c != '\t')
					clean += c;
			for (std::string::size_type ii = 0; ii < clean.size(); ii += 16)
			{
				std::string sub = clean.substr(ii, 16);
				if (sub.size() == 16) result.push_back(sub);
			}
		}
	}
	else if (engine == CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3)
	{
		if (id == EMULATION_ID::GBA_ID)
		{
			// AR v3: 16 hex chars per code
			// NOTE: DEADFACE seed-change codes are chunked here but will fail decode gracefully
			std::string clean;
			clean.reserve(rawCheat.size());
			for (char c : rawCheat)
				if (c != ' ' && c != '+' && c != '-' && c != '\n' && c != '\r' && c != '\t')
					clean += c;
			for (std::string::size_type ii = 0; ii < clean.size(); ii += 16)
			{
				std::string sub = clean.substr(ii, 16);
				if (sub.size() == 16) result.push_back(sub);
			}
		}
	}
	else if (engine == CheatEngine_t::CHEATING_ENGINE::CODEBREAKER)
	{
		if (id == EMULATION_ID::GBA_ID)
		{
			// Refer https://problemkaputt.de/gbatek-gba-cheat-codes-codebreaker-xploder.htm
			// Type 8 short codes: 12 chars (8 addr + 4 val)
			// All other types:    16 chars (8 addr + 8 val)
			// Strategy: prefer 16-char chunks; fall back to 12-char if total size fits

			std::string clean;
			clean.reserve(rawCheat.size());
			for (char c : rawCheat)
				if (c != ' ' && c != '+' && c != '-' && c != '\n' && c != '\r' && c != '\t')
					clean += c;

			if (clean.size() % 16 == 0)
			{
				// All standard-length codes
				for (std::string::size_type ii = 0; ii < clean.size(); ii += 16)
				{
					std::string sub = clean.substr(ii, 16);
					if (sub.size() == 16) result.push_back(sub);
				}
			}
			else if (clean.size() % 12 == 0)
			{
				// All short-length codes (type 8)
				for (std::string::size_type ii = 0; ii < clean.size(); ii += 12)
				{
					std::string sub = clean.substr(ii, 12);
					if (sub.size() == 12) result.push_back(sub);
				}
			}
			else
			{
				// Mixed: parse sequentially by first nibble
				std::string::size_type pos = 0;
				while (pos + 8 <= clean.size())
				{
					char nibble = (char)toupper((unsigned char)clean[pos]);
					// Type 8 with short value: 12 chars total
					std::string::size_type codeLen = (nibble == '8') ? 12 : 16;
					if (pos + codeLen > clean.size()) BREAK;
					result.push_back(clean.substr(pos, codeLen));
					pos += codeLen;
				}
			}
		}
	}

	// Fallback: nothing produced
	if (result.empty())
	{
		std::string clean = rawCheat;
		clean.erase(std::remove_if(clean.begin(), clean.end(),
			[](char c) { return c == '\n' || c == '\r' || c == '\t'; }),
			clean.end());
		if (!clean.empty()) result.push_back(clean);
	}

	return result;
}

#pragma endregion CHEAT_HELPERS

// ---------------------------------------------------------------------------
// CheatEngine_t definitions
// ---------------------------------------------------------------------------
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
	if (cheatCount[currentEngineMode] >= MAX_CHEAT_COUNT_PER_ENGINE)
		RETURN NO;

	std::string key = canonicalize(cheat);
	std::vector<std::string> subCodes = splitCheatCodes(cheat, currentEngineMode, currentID);

	cheatNames[currentEngineMode][currentID][key] = name;
	cheatEnabled[currentEngineMode][currentID][key] = YES;
	cheatSubCodes[currentEngineMode][currentID][key] = subCodes;
	++cheatCount[currentEngineMode];

	for (const std::string& code : subCodes)
	{
		uint32_t   address = 0;
		uint32_t   data = 0;
		uint32_t   compare = 0;
		FLAG       hasCompare = NO;
		CheatWidth width = CheatWidth::U8;

		if (decodeAddressAndData(code, &address, &data, &compare, &hasCompare, &width))
		{
			auto& table = fakeData[currentEngineMode][currentID];
			auto  it = table.find(address);
			if (it != table.end() && (it->second.data != data || it->second.compare != compare))
			{
				WARN("[CHEAT WARNING] Address collision at 0x%X | Existing(%u,%u) New(%u,%u)",
					address, it->second.data, it->second.compare, data, compare);
			}
			auto& entry = table[address];
			entry.data = data;
			entry.compare = compare;
			entry.hasCompare = hasCompare;
			entry.width = width;
			entry.enabled = YES;
		}
		// If decode fails (malformed code), the cheat is still registered in
		// cheatNames so the user can see and delete it, but it won't patch anything.
	}

	RETURN YES;
}

FLAG CheatEngine_t::enableCheat(std::string cheat)
{
	std::string key = canonicalize(cheat);

	auto& storedMap = cheatSubCodes[currentEngineMode][currentID];
	auto  it = storedMap.find(key);
	std::vector<std::string> subCodes;
	if (it != storedMap.end())
		subCodes = it->second;
	else
		subCodes = splitCheatCodes(cheat, currentEngineMode, currentID);

	for (const std::string& code : subCodes)
	{
		uint32_t   address = 0;
		uint32_t   data = 0;
		uint32_t   compare = 0;
		FLAG       hasCompare = NO;
		CheatWidth width = CheatWidth::U8;

		if (decodeAddressAndData(code, &address, &data, &compare, &hasCompare, &width))
		{
			auto& table = fakeData[currentEngineMode][currentID];
			auto  fakeIt = table.find(address);
			if (fakeIt != table.end() && (fakeIt->second.data != data || fakeIt->second.compare != compare))
				LOG("[CHEAT WARNING] Enable collision at 0x%X", address);

			auto& entry = table[address];
			entry.data = data;
			entry.compare = compare;
			entry.hasCompare = hasCompare;
			entry.width = width;
			entry.enabled = YES;
		}
		else
		{
			// FIX: malformed sub-code -- log and continue rather than aborting.
			// The cheat is still marked enabled; well-formed sub-codes still apply.
			WARN("[CHEAT] Failed to decode sub-code during enable, skipping: %s", code.c_str());
		}
	}

	cheatEnabled[currentEngineMode][currentID][key] = YES;
	RETURN SUCCESS;
}

FLAG CheatEngine_t::disableCheat(std::string cheat)
{
	std::string key = canonicalize(cheat);

	auto& storedMap = cheatSubCodes[currentEngineMode][currentID];
	auto  it = storedMap.find(key);
	std::vector<std::string> subCodes;
	if (it != storedMap.end())
		subCodes = it->second;
	else
		subCodes = splitCheatCodes(cheat, currentEngineMode, currentID);

	for (const std::string& code : subCodes)
	{
		uint32_t   address = 0;
		uint32_t   data = 0;
		uint32_t   compare = 0;
		FLAG       hasCompare = NO;
		CheatWidth width = CheatWidth::U8;

		if (decodeAddressAndData(code, &address, &data, &compare, &hasCompare, &width))
		{
			auto& table = fakeData[currentEngineMode][currentID];
			auto  fakeIt = table.find(address);
			if (fakeIt != table.end())
				fakeIt->second.enabled = NO;
		}
		else
		{
			// FIX: malformed sub-code -- log and continue
			WARN("[CHEAT] Failed to decode sub-code during disable, skipping: %s", code.c_str());
		}
	}

	cheatEnabled[currentEngineMode][currentID][key] = NO;
	RETURN SUCCESS;
}

FLAG CheatEngine_t::deleteCheat(std::string cheat)
{
	if (cheatCount[currentEngineMode] == ZERO)
		RETURN FAILURE;

	std::string key = canonicalize(cheat);

	// FIX: check existence first; if not found at all, genuinely fail
	if (cheatNames[currentEngineMode][currentID].find(key) ==
		cheatNames[currentEngineMode][currentID].end())
		RETURN FAILURE;

	auto& storedMap = cheatSubCodes[currentEngineMode][currentID];
	auto  it = storedMap.find(key);
	std::vector<std::string> subCodes;
	if (it != storedMap.end())
		subCodes = it->second;
	else
		subCodes = splitCheatCodes(cheat, currentEngineMode, currentID);

	for (const std::string& code : subCodes)
	{
		uint32_t   address = 0;
		uint32_t   data = 0;
		uint32_t   compare = 0;
		FLAG       hasCompare = NO;
		CheatWidth width = CheatWidth::U8;

		if (decodeAddressAndData(code, &address, &data, &compare, &hasCompare, &width))
		{
			fakeData[currentEngineMode][currentID].erase(address);
		}
		// FIX: if decode fails (malformed code), skip fakeData erase but still
		// clean up the name/enabled/subcode maps below. This was the root cause
		// of cheats that couldn't be deleted after entering a wrong format.
	}

	// Always clean up maps regardless of decode outcome
	cheatNames[currentEngineMode][currentID].erase(key);
	cheatEnabled[currentEngineMode][currentID].erase(key);
	cheatSubCodes[currentEngineMode][currentID].erase(key);
	--cheatCount[currentEngineMode];

	RETURN SUCCESS;
}

FLAG CheatEngine_t::interceptCPURead(CHEATING_ENGINE engine, uint32_t address, uint32_t* data, uint32_t* compare, FLAG* hasCompare)
{
	auto& table = fakeData[TO_UINT8(engine)][currentID];
	auto  it = table.find(address);
	if (it != table.end() && it->second.enabled == YES)
	{
		*data = it->second.data;
		*compare = it->second.compare;
		*hasCompare = it->second.hasCompare;
		RETURN SUCCESS;
	}
	RETURN FAILURE;
}

std::vector<CheatEngine_t::CheatWrite_t> CheatEngine_t::getCheatWrites(CHEATING_ENGINE engine)
{
	std::vector<CheatWrite_t> result;
	auto& table = fakeData[TO_UINT8(engine)][TO_UINT8(currentID)];
	for (auto& [address, patch] : table)
	{
		if (patch.enabled == YES)
		{
			CheatWrite_t w;
			w.address = address;
			w.data = patch.data;
			w.width = patch.width;
			result.push_back(w);
		}
	}
	RETURN result;
}

FLAG CheatEngine_t::listAllTheCheats(CHEATING_ENGINE engine, FLAG* en)
{
	INC8 ii = RESET;
	for (auto& [key, value] : cheatNames[engine][currentID])
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
	RETURN cheatNames[engine][currentID];
}

FLAG CheatEngine_t::saveCheatNames(const std::string& filename)
{
	std::ofstream ofs(filename);
	if (!ofs) RETURN FAILURE;

	uint8_t m = CHEATING_ENGINE::TOTAL_ENGINES;
	uint8_t n = EMULATION_ID::TOTAL_ID;

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
				// Format: key=name\tsubcode1\tsubcode2\t...
				// Tab-delimited sub-codes after the name allow full restore on load.
				// On load, old files (no tabs) work unchanged; subCodes just falls
				// back to the single-code path.
				ofs << key << "=" << val;
				auto& scMap = cheatSubCodes[i][j];
				auto  scIt = scMap.find(key);
				if (scIt != scMap.end())
					for (const auto& sc : scIt->second)
						ofs << "\t" << sc;
				ofs << "\n";
			}
		}
	}

	RETURN SUCCESS;
}

FLAG CheatEngine_t::loadCheatNames(const std::string& filename)
{
	uint8_t m = CHEATING_ENGINE::TOTAL_ENGINES;
	uint8_t n = EMULATION_ID::TOTAL_ID;

	std::ifstream ifs(filename);
	if (!ifs) RETURN FAILURE;

	std::string line;
	int file_m = 0, file_n = 0;

	if (std::getline(ifs, line))
	{
		if (sscanf(line.c_str(), "m=%d", &file_m) != 1) RETURN FAILURE;
	}
	else RETURN FAILURE;

	if (std::getline(ifs, line))
	{
		if (sscanf(line.c_str(), "n=%d", &file_n) != 1) RETURN FAILURE;
	}
	else RETURN FAILURE;

	// NOTE: if TOTAL_ENGINES or TOTAL_ID changed since last save (e.g. adding
	// CODEBREAKER), the version check fails and old save files are rejected.
	// To handle migration, remove or relax this check and guard array accesses.
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

			auto eqPos = line.find('=');
			if (eqPos == std::string::npos)
			{
				--k; continue;
			}

			std::string key = line.substr(0, eqPos);
			std::string rest = line.substr(eqPos + 1);

			// Split rest into name + optional tab-delimited sub-codes
			auto tabPos = rest.find('\t');
			std::string nameStr = rest.substr(0, tabPos);

			map[key] = nameStr;
			++cheatCount[i];

			// Restore sub-codes if present (new format)
			if (tabPos != std::string::npos)
			{
				std::istringstream scs(rest.substr(tabPos + 1));
				std::string sc;
				while (std::getline(scs, sc, '\t'))
					if (!sc.empty())
						cheatSubCodes[i][j][key].push_back(sc);
			}
			// If no tabs (old format), subCodes is empty; enable/disable/delete
			// fall back to splitCheatCodes using the canonical key as the raw input,
			// which works correctly for single-code cheats.
		}
	}

	RETURN SUCCESS;
}

FLAG CheatEngine_t::decodeAddressAndData(const std::string& cheat,
	uint32_t* address,
	uint32_t* data,
	uint32_t* compare,
	FLAG* hasCompare,
	CheatWidth* width)
{
	*address = 0;
	*data = 0;
	*compare = 0;
	*hasCompare = NO;
	*width = CheatWidth::U8;

	if (cheat.empty()) RETURN FAILURE;

	std::vector<BYTE> decodeStage2(cheat.size());

	// -----------------------------------------------------------------------
	// NES / GB GameGenie
	// -----------------------------------------------------------------------
	if (currentEngineMode == CHEATING_ENGINE::GAMEGENIE)
	{
		if (currentID == EMULATION_ID::NES_ID)
		{
			// Refer https://tuxnes.sourceforge.net/gamegenie.html
			// Arrives as clean 6 or 8 chars from splitCheatCodes

			for (std::string::size_type ii = 0; ii < cheat.size(); ++ii)
				decodeStage2[ii] = GGDecodeNES[cheat[ii]];

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
				*hasCompare = NO;
				*width = CheatWidth::U8;
				RETURN SUCCESS;
			}
			else if (cheat.size() == EIGHT)
			{
				*data =
					((decodeStage2[1] & 7) << 4) | ((decodeStage2[0] & 8) << 4)
					| (decodeStage2[0] & 7) | (decodeStage2[7] & 8);
				*compare =
					((decodeStage2[7] & 7) << 4) | ((decodeStage2[6] & 8) << 4)
					| (decodeStage2[6] & 7) | (decodeStage2[5] & 8);
				*hasCompare = YES;
				*width = CheatWidth::U8;
				RETURN SUCCESS;
			}
		}
		else if (currentID == EMULATION_ID::GB_GBC_ID)
		{
			// Refer https://gbdev.io/pandocs/Shark_Cheats.html
			// Arrives as 9 hex chars (dashes stripped by splitCheatCodes).
			//
			// Index mapping (dashes removed, 9 chars):
			//   A(0) B(1) C(2) D(3) F(4) G(5) H(6) J(7) K(8)
			//   nDat = [0],[1]
			//   addr = [5],[2],[3],[4]   (G, C, D, F)
			//   oDat = [6],[8]           (H, K)
			// Compare rotate: rotate-left-6 then XOR 0xBA
			// Refer https://www.youtube.com/watch?v=C86OsYRACTM

			if (cheat.size() != 9) RETURN FAILURE;

			std::string nDat = { cheat[0], cheat[1] };
			std::string addr = { cheat[5], cheat[2], cheat[3], cheat[4] };
			std::string oDat = { cheat[6], cheat[8] };

			*address = std::stoi(addr, nullptr, 16) ^ 0xF000;
			*data = std::stoi(nDat, nullptr, 16);
			auto temp = std::stoi(oDat, nullptr, 16);
			*compare = (((temp << 6) | (temp >> 2)) & 0xFF) ^ 0xBA;
			*hasCompare = YES;
			*width = CheatWidth::U8;
			RETURN SUCCESS;
		}
	}

	// -----------------------------------------------------------------------
	// GB/GBC GameShark
	// -----------------------------------------------------------------------
	else if (currentEngineMode == CHEATING_ENGINE::GAMESHARK)
	{
		if (currentID == EMULATION_ID::GB_GBC_ID)
		{
			// Refer https://gbdev.io/pandocs/Shark_Cheats.html
			// Arrives as 8 hex chars.
			// type/bank byte in compare; hasCompare = NO (not a value compare —
			// it is a bank selector used at intercept sites via getCheatWrites).

			if (cheat.size() != 8) RETURN FAILURE;

			std::string nDat = { cheat[2], cheat[3] };
			std::string addr = { cheat[6], cheat[7], cheat[4], cheat[5] };
			std::string type = { cheat[0], cheat[1] };

			*address = std::stoi(addr, nullptr, 16);
			*data = std::stoi(nDat, nullptr, 16);
			*compare = std::stoi(type, nullptr, 16);
			*hasCompare = NO;
			*width = CheatWidth::U8;
			RETURN SUCCESS;
		}
		else if (currentID == EMULATION_ID::GBA_ID)
		{
			// Refer https://problemkaputt.de/gbatek-gba-cheat-codes-gameshark-action-replay-v1-v2.htm
			// Arrives as 16 hex chars.
			// Top nibble of first word encodes write width:
			//   0x0 -> U8  (address + 0xFF value mask)
			//   0x1 -> U16 (address + 0xFFFF value mask)
			//   0x2 -> U32

			if (cheat.size() != 16) RETURN FAILURE;

			uint32_t word1 = (uint32_t)std::stoul(cheat.substr(0, 8), nullptr, 16);
			uint32_t word2 = (uint32_t)std::stoul(cheat.substr(8, 8), nullptr, 16);

			uint8_t  typeNibble = (word1 >> 28) & 0x0F;
			uint32_t rawAddress = word1 & 0x0FFFFFFF;

			switch (typeNibble)
			{
			case 0x0:
				*address = rawAddress;
				*data = word2 & 0xFF;
				*width = CheatWidth::U8;
				*hasCompare = NO;
				RETURN SUCCESS;
			case 0x1:
				*address = rawAddress;
				*data = word2 & 0xFFFF;
				*width = CheatWidth::U16;
				*hasCompare = NO;
				RETURN SUCCESS;
			case 0x2:
				*address = rawAddress;
				*data = word2;
				*width = CheatWidth::U32;
				*hasCompare = NO;
				RETURN SUCCESS;
			default:
				RETURN FAILURE;
			}
		}
	}

	// -----------------------------------------------------------------------
	// GBA Action Replay v3
	// -----------------------------------------------------------------------
	else if (currentEngineMode == CHEATING_ENGINE::ACTION_REPLAY_V3)
	{
		if (currentID == EMULATION_ID::GBA_ID)
		{
			// Refer https://problemkaputt.de/gbatek-gba-cheat-codes-pro-action-replay-v3.htm
			// Arrives as 16 hex chars. TEA-decrypt first, then decode opcode.
			//
			// Address expansion after decrypt:
			//   rawAddr = decrypted_word1 & 0x00FFFFFF (24 bits)
			//   fullAddr = ((rawAddr & 0xF00000) << 4) | (rawAddr & 0x0FFFFF)
			//   Example: 0x225E90 -> 0x02025E90 (EWRAM)
			//
			// RAM write opcodes (go via getCheatWrites / VBlank write):
			//   0x00 -> U8  write
			//   0x02 -> U16 write
			//   0x04 -> U32 write
			//
			// ROM patch opcodes (go via interceptCPURead):
			//   0x06 -> U16 ROM patch, no compare
			//   0x07 -> U8  ROM patch with compare (original byte in bits 15:8 of word2)
			//
			// Unsupported (conditionals, fill, indirect, IO-area, DEADFACE):
			//   All other opcodes -> RETURN FAILURE (silently skipped)

			if (cheat.size() != 16) RETURN FAILURE;

			uint32_t word1 = (uint32_t)std::stoul(cheat.substr(0, 8), nullptr, 16);
			uint32_t word2 = (uint32_t)std::stoul(cheat.substr(8, 8), nullptr, 16);

			arv3Decrypt(word1, word2);

			uint8_t  opcode = (uint8_t)((word1 >> 24) & 0xFF);
			uint32_t rawAddr = word1 & 0x00FFFFFF;
			uint32_t fullAddr = ((rawAddr & 0xF00000) << 4) | (rawAddr & 0x0FFFFF);

			switch (opcode)
			{
				// RAM writes -- applied via getCheatWrites at VBlank
			case 0x00:
				*address = fullAddr;
				*data = word2 & 0xFF;
				*width = CheatWidth::U8;
				*hasCompare = NO;
				RETURN SUCCESS;
			case 0x02:
				*address = fullAddr;
				*data = word2 & 0xFFFF;
				*width = CheatWidth::U16;
				*hasCompare = NO;
				RETURN SUCCESS;
			case 0x04:
				*address = fullAddr;
				*data = word2;
				*width = CheatWidth::U32;
				*hasCompare = NO;
				RETURN SUCCESS;

				// ROM patches -- applied via interceptCPURead (GBA read function)
				// These patch ROM addresses (0x08000000+); interceptCPURead handles them
				// exactly like NES GameGenie: compare optional, substitute on read.
			case 0x06:
				// 16-bit ROM patch, no compare
				*address = fullAddr;
				*data = word2 & 0xFFFF;
				*width = CheatWidth::U16;
				*hasCompare = NO;
				RETURN SUCCESS;
			case 0x07:
				// 8-bit ROM patch with compare
				// TODO: verify exact compare byte position in word2 for AR v3 opcode 0x07
				*address = fullAddr;
				*data = word2 & 0xFF;
				*compare = (word2 >> 8) & 0xFF; // original ROM byte
				*width = CheatWidth::U8;
				*hasCompare = YES;
				RETURN SUCCESS;

			default:
				// Unsupported: conditionals, fill/range, indirect, IO-area, DEADFACE
				WARN("[CHEAT] AR v3 opcode 0x%02X not yet supported", opcode);
				RETURN FAILURE;
			}
		}
	}

	// -----------------------------------------------------------------------
	// GBA CodeBreaker/Xploder
	// -----------------------------------------------------------------------
	else if (currentEngineMode == CHEATING_ENGINE::CODEBREAKER)
	{
		if (currentID == EMULATION_ID::GBA_ID)
		{
			// Refer https://problemkaputt.de/gbatek-gba-cheat-codes-codebreaker-xploder.htm
			// Not encrypted. Arrives as 12 or 16 hex chars.
			//
			// word1 = TTAAAAAA where TT = type nibble (upper 4 bits), AAAAAA = address
			// address = word1 & 0x0FFFFFFF (lower 28 bits, directly usable as GBA addr)
			//   Example: 0x82005274 -> addr = 0x02005274 (EWRAM)
			//
			// Supported type nibbles:
			//   0x0 -> U8  write: 0AAAAAAA 000000DD
			//   0x1 -> U16 write: 1AAAAAAA 0000DDDD
			//   0x2 -> U32 write: 2AAAAAAA DDDDDDDD
			//   0x8 -> U16 write: 8AAAAAAA DDDD (short 12-char format)
			//
			// Unsupported (conditionals, master/hook):
			//   0x3 -> conditional -- RETURN FAILURE
			//   0xB -> master/hook -- RETURN FAILURE

			if (cheat.size() != 12 && cheat.size() != 16) RETURN FAILURE;

			uint32_t word1 = (uint32_t)std::stoul(cheat.substr(0, 8), nullptr, 16);

			// Pad 4-char value (12-char code) to 8 chars for uniform parsing
			std::string valStr = (cheat.size() == 12)
				? std::string("0000") + cheat.substr(8)
				: cheat.substr(8);
			uint32_t word2 = (uint32_t)std::stoul(valStr, nullptr, 16);

			uint8_t  typeNibble = (word1 >> 28) & 0x0F;
			uint32_t addr = word1 & 0x0FFFFFFF; // direct GBA address, no expansion

			switch (typeNibble)
			{
			case 0x0:
				*address = addr;
				*data = word2 & 0xFF;
				*width = CheatWidth::U8;
				*hasCompare = NO;
				RETURN SUCCESS;
			case 0x1:
			case 0x8: // short 16-bit write
				*address = addr;
				*data = word2 & 0xFFFF;
				*width = CheatWidth::U16;
				*hasCompare = NO;
				RETURN SUCCESS;
			case 0x2:
				*address = addr;
				*data = word2;
				*width = CheatWidth::U32;
				*hasCompare = NO;
				RETURN SUCCESS;
			case 0x3:
				WARN("[CHEAT] CodeBreaker conditional codes (type 3) not yet supported");
				RETURN FAILURE;
			case 0xB:
				WARN("[CHEAT] CodeBreaker master/hook codes (type B) not yet supported");
				RETURN FAILURE;
			default:
				RETURN FAILURE;
			}
		}
	}

	RETURN FAILURE;
}

#pragma endregion CHEAT_DEFINITIONS
