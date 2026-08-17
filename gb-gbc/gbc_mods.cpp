#include "gbc.h"

#ifndef __RPI_PICO__
#include "imgui_internal.h"		// needed for ImGui::DockBuilder* (programmatic default dock layout)
#endif

#pragma region STB_INCLUDES
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#pragma endregion STB_INCLUDES

#pragma region GB_GBC_SPECIFIC_MACROS
// GB palette byte format (same encoding as BGP/OBP): 2 bits per shade,
// bits [1:0] = shade for raw color index 0, [3:2] = index 1, [5:4] = index 2,
// [7:6] = index 3. Maps a raw 2bpp tile pixel value through the palette to
// get the actual displayed shade (0 = lightest, 3 = darkest).
#define GET_GB_COLOR_NUMBER(palette, colorIndex) (static_cast<BYTE>(((palette) >> ((colorIndex) * 2)) & 0x03))
#pragma endregion GB_GBC_SPECIFIC_MACROS

bool GBc_t::saveState(uint8_t id)
{
	bool status = false;

	if (dmg_cgb_bios.biosFound == YES && dmg_cgb_bios.unMapBios == NO)
	{
		WARN("Save states are allowed only after BIOS is unmapped");
		status = true;
		RETURN status;
	}

	if (_ENABLE_BESS_FORMAT == YES)
	{
		status = bessSaveState(id);
	}
	else
	{

		std::string saveStateNameForThisROM = getSaveStateName(
			pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer
			, sizeof(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer)
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
		LOG("Saved on: %s", buffer);
		std::string dt(buffer);
		saveFile.append(dt);
#endif

		LOG("Saved as: %s", saveStateNameForThisROM.c_str());

		saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

		static_assert(std::is_trivially_copyable<GBc_instance_t>::value, "not trivially copyable");
		static_assert(std::is_standard_layout<GBc_instance_t>::value, "not standard layout");

		save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
		save.write(reinterpret_cast<char*>(&(pGBc_instance->GBc_memoryState)), sizeof(pGBc_instance->GBc_memoryState));
		save.close();

		status = true;
	}

	RETURN status;
}

bool GBc_t::loadState(uint8_t id)
{
	bool status = false;

	if (dmg_cgb_bios.biosFound == YES && dmg_cgb_bios.unMapBios == NO)
	{
		WARN("Load states are allowed only after BIOS is unmapped");
		status = true;
		RETURN status;
	}

	if (_ENABLE_BESS_FORMAT == YES)
	{
		status = bessLoadState(id);
	}
	else
	{
		std::string saveStateNameForThisROM = getSaveStateName(
			pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer
			, sizeof(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer)
		);

		saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

		std::ifstream save;

		saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

		static_assert(std::is_trivially_copyable<GBc_instance_t>::value, "not trivially copyable");
		static_assert(std::is_standard_layout<GBc_instance_t>::value, "not standard layout");

		save.open(saveStateNameForThisROM, std::ios::binary);
		save.read(reinterpret_cast<char*>(&(pGBc_instance->GBc_memoryState)), sizeof(pGBc_instance->GBc_memoryState));
		save.close();

		displayCompleteScreen();

		status = true;
	}

	RETURN status;
}

bool GBc_t::absoluteSaveState(uint8_t id)
{
	bool status = false;

	std::filesystem::path saveDirectory(_SAVE_LOCATION);
	if (!(std::filesystem::exists(saveDirectory)))
	{
		std::filesystem::create_directory(saveDirectory);
	}

	std::string saveStateNameForThisROM = getSaveStateName(
		pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer
		, sizeof(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer)
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
	LOG("Saved on: %s", buffer);
	std::string dt(buffer);
	saveFile.append(dt);
#endif

	LOG("Saved as: %s", saveStateNameForThisROM.c_str());

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<absolute_GBc_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<absolute_GBc_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
	save.write(reinterpret_cast<char*>(&(pAbsolute_GBc_instance->GBc_absoluteMemoryState)), sizeof(pAbsolute_GBc_instance->GBc_absoluteMemoryState));
	save.close();

	status = true;

	RETURN status;
}

bool GBc_t::absoluteLoadState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = getSaveStateName(
		pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer
		, sizeof(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer)
	);

	saveStateNameForThisROM = "_absolute_" + saveStateNameForThisROM + std::to_string(id);

	std::ifstream save;

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	static_assert(std::is_trivially_copyable<absolute_GBc_instance_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<absolute_GBc_instance_t>::value, "not standard layout");

	save.open(saveStateNameForThisROM, std::ios::binary);
	save.read(reinterpret_cast<char*>(&(pAbsolute_GBc_instance->GBc_absoluteMemoryState)), sizeof(pAbsolute_GBc_instance->GBc_absoluteMemoryState));
	save.close();

	status = true;

	RETURN status;
}

bool GBc_t::fillGamePlayStack()
{
	// assume minimum frame rate is 60 fps
	// so for 5 seconds worth of rewind, 300 elements is required
	// if fps is 1000, for 5 seconds worth of rewind, 5000 elements is required
	// Hence, we will (for now) set the limit to 5000 elements

	if (gamePlay.size() <= _REWIND_BUFFER_SIZE)
	{
		gamePlay.push_front(pGBc_instance->GBc_state);
		RETURN true;
	}
	else
	{
		gamePlay.pop_back();
		gamePlay.push_front(pGBc_instance->GBc_state);
		RETURN false;
	}
}

bool GBc_t::rewindGamePlay()
{
	if (gamePlay.empty())
	{
		RETURN false;
	}
	else
	{
		memcpy(&pGBc_instance->GBc_memoryState, &gamePlay.front(), sizeof(pGBc_instance->GBc_memoryState));
		gamePlay.pop_front();
		RETURN true;
	}
}

bool GBc_t::bessSaveState(uint8_t id)
{
	// Refer to https://github.com/LIJI32/SameBoy/blob/master/BESS.md

	bool status = false;

	std::string saveStateNameForThisROM = getSaveStateName(
		pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer
		, sizeof(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer)
	);

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	LOG("Saved as: %s", saveStateNameForThisROM.c_str());

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	// Open file in binary mode
	std::ofstream save(saveStateNameForThisROM.c_str(), std::ios::binary);
	if (!save)
	{
		FATAL("Not able to create save state");
		RETURN FAILURE;
	}

	// Verify POD
	static_assert(std::is_trivially_copyable<BESS_BLOCK_NAME_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_NAME_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_INFO_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_INFO_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_CORE_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_CORE_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_XOAM_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_XOAM_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_MBC_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_MBC_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_RTC_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_RTC_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_END_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_END_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_FOOTER_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_FOOTER_t>::value, "not standard layout");

	INC32 curr_off = RESET;
	INC32 ram_off = RESET;
	INC32 vram_off = RESET;
	INC32 mbc_off = RESET;
	INC32 oam_off = RESET;
	INC32 hram_off = RESET;
	INC32 bg_pram_off = RESET;
	INC32 obj_pram_off = RESET;
	INC32 first_blk_off = RESET;

#if _DEBUG
	// For debug... keep this disabled in normal operation
	save.rdbuf()->pubsetbuf(nullptr, 0);  // unbuffered mode
#endif

	// 0.1) RAM

	// Save to file
	ram_off = curr_off;
	if (ROM_TYPE == ROM::GAME_BOY_COLOR)
	{
		for (int jj = ZERO; jj < 0x1000; jj++)
		{
			BYTE ramByte = pGBc_memory->GBcMemoryMap.mWorkRam.wRamMemory[jj];
			save.write(reinterpret_cast<const char*>(&ramByte), ONE);
			++curr_off;
		}
		for (int ii = ZERO; ii < 7; ii++)
		{
			for (int jj = ZERO; jj < 0x1000; jj++)
			{
				BYTE ramByte = pGBc_instance->GBc_state.entireWram01.wram01MemoryBanks.mWRAM01Banks[ii][jj];
				save.write(reinterpret_cast<const char*>(&ramByte), ONE);
				++curr_off;
			}
		}
	}
	else if (ROM_TYPE == ROM::GAME_BOY)
	{
		for (int jj = ZERO; jj < 0x2000; jj++)
		{
			BYTE ramByte = pGBc_memory->GBcMemoryMap.mWorkRam.wRamMemory[jj];
			save.write(reinterpret_cast<const char*>(&ramByte), ONE);
			++curr_off;
		}
	}

	// 0.2) VRAM

	// Save to file
	vram_off = curr_off;
	if (ROM_TYPE == ROM::GAME_BOY_COLOR)
	{
		for (int ii = ZERO; ii < 2; ii++)
		{
			for (int jj = ZERO; jj < 0x2000; jj++)
			{
				BYTE ramByte = debugReadVRAM(ii, jj);
				save.write(reinterpret_cast<const char*>(&ramByte), ONE);
				++curr_off;
			}
		}
	}
	else if (ROM_TYPE == ROM::GAME_BOY)
	{
		for (int jj = ZERO; jj < 0x2000; jj++)
		{
			BYTE ramByte = pGBc_memory->GBcMemoryMap.mVideoRam.videoRamMemory[jj];
			save.write(reinterpret_cast<const char*>(&ramByte), ONE);
			++curr_off;
		}
	}

	// 0.3) MBCRAM

	// Save to file
	mbc_off = curr_off;
	for (int ii = ZERO; ii < getNumberOfRAMBanksUsed(); ii++)
	{
		for (int jj = ZERO; jj < 0x2000; jj++)
		{
			BYTE ramByte = pGBc_instance->GBc_state.entireRam.ramMemoryBanks.mRAMBanks[ii][jj];
			save.write(reinterpret_cast<const char*>(&ramByte), ONE);
			++curr_off;
		}
	}

	// 0.4) OAM

	// Save to file
	oam_off = curr_off;
	for (int jj = ZERO; jj < 0xA0; jj++)
	{
		BYTE ramByte = pGBc_memory->GBcMemoryMap.mOAM.OAMMemory[jj];
		save.write(reinterpret_cast<const char*>(&ramByte), ONE);
		++curr_off;
	}

	// 0.4) HRAM

	// Save to file
	hram_off = curr_off;
	for (int jj = ZERO; jj < 0x7F; jj++)
	{
		BYTE ramByte = pGBc_memory->GBcMemoryMap.mHighRam.highRamMemory[jj];
		save.write(reinterpret_cast<const char*>(&ramByte), ONE);
		++curr_off;
	}

	if (ROM_TYPE == ROM::GAME_BOY_COLOR)
	{
		// 0.5 BG PRAM
		bg_pram_off = curr_off;
		for (int jj = ZERO; jj < 0x40; jj++)
		{
			BYTE ramByte = pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAMMemory[jj];
			save.write(reinterpret_cast<const char*>(&ramByte), ONE);
			++curr_off;
		}

		// 0.6 OBJ PRAM
		obj_pram_off = curr_off;
		for (int jj = ZERO; jj < 0x40; jj++)
		{
			BYTE ramByte = pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAMMemory[jj];
			save.write(reinterpret_cast<const char*>(&ramByte), ONE);
			++curr_off;
		}
	}

	// Pad zeros to ensure 4 byte alignment
	auto pad = (4 - (curr_off & 3)) & 3;  // zero if already aligned
	for (INC8 ii = 0; ii < pad; ii++)
	{
		BYTE ramByte = ZERO;
		save.write(reinterpret_cast<const char*>(&ramByte), ONE);
		++curr_off;
	}

	// 1) BESS_BLOCK_NAME_t

	first_blk_off = curr_off;

	BESS_BLOCK_NAME_t BESS_BLOCK_NAME;

	// Copy the name of block
	std::memcpy(BESS_BLOCK_NAME.BESS_BLOCK_HEADER.ascii_ident, "NAME", 4);

	// Build version string
	char version_str[16];
	std::snprintf(version_str, sizeof(version_str), "v%.4f", VERSION);

	// Build full name and zero-pad
	const char* base_name = "MASQUERADE ";
	char full_name[0x20] = {}; // zero-initialize
	std::snprintf(full_name, sizeof(full_name), "%s%s", base_name, version_str);

	// Copy into struct
	std::memcpy(BESS_BLOCK_NAME.name_ver, full_name, sizeof(BESS_BLOCK_NAME.name_ver));

	// Update the size of the block
	BESS_BLOCK_NAME.BESS_BLOCK_HEADER.blk_len = sizeof(BESS_BLOCK_NAME.name_ver);

	// Save to file
	save.write(reinterpret_cast<char*>(&BESS_BLOCK_NAME), sizeof(BESS_BLOCK_NAME_t));

	// 2) BESS_BLOCK_INFO_t

	BESS_BLOCK_INFO_t BESS_BLOCK_INFO;

	// Copy the name of block
	std::memcpy(BESS_BLOCK_INFO.BESS_BLOCK_HEADER.ascii_ident, "INFO", 4);

	// Assuming `title_bytes` is your source array:
	const uint8_t* title_bytes = pGBc_memory->GBcMemoryMap.mCodeRom
		.codeRomFields.romBank_00
		.romBank00_Fields
		.cartridge_header
		.cartridge_header_fields
		.title.title;

	// Copy 16 bytes into the struct
	std::memcpy(BESS_BLOCK_INFO.title, title_bytes, sizeof(BESS_BLOCK_INFO.title));

	// Copy global checksum
	BESS_BLOCK_INFO.chksum = pGBc_memory->GBcMemoryMap.mCodeRom
		.codeRomFields.romBank_00
		.romBank00_Fields
		.cartridge_header
		.cartridge_header_fields.globalChecksum;

	// Update the size of the block
	BESS_BLOCK_INFO.BESS_BLOCK_HEADER.blk_len = 0x12;

	// Save to file
	save.write(reinterpret_cast<char*>(&BESS_BLOCK_INFO), sizeof(BESS_BLOCK_INFO_t));

	// 3) BESS_BLOCK_CORE_t

	BESS_BLOCK_CORE_t BESS_BLOCK_CORE;

	// Copy the name of block
	std::memcpy(BESS_BLOCK_CORE.BESS_BLOCK_HEADER.ascii_ident, "CORE", 4);

	// Update the contents
	// Update version
	BESS_BLOCK_CORE.maj_bess_ver = 0x01;
	BESS_BLOCK_CORE.min_bess_ver = 0x01;
	// Update GB mode
	if (ROM_TYPE == ROM::GAME_BOY_COLOR)
	{
		std::memcpy(BESS_BLOCK_CORE.mdl_indent, "CC  ", 4);
	}
	else if (ROM_TYPE == ROM::GAME_BOY)
	{
		std::memcpy(BESS_BLOCK_CORE.mdl_indent, "GD  ", 4);
	}
	// Copy cpu registers
	BESS_BLOCK_CORE.pc = pGBc_registers->pc;
	BESS_BLOCK_CORE.af = pGBc_registers->af.af_u16memory;
	BESS_BLOCK_CORE.bc = pGBc_registers->bc.bc_u16memory;
	BESS_BLOCK_CORE.de = pGBc_registers->de.de_u16memory;
	BESS_BLOCK_CORE.hl = pGBc_registers->hl.hl_u16memory;
	BESS_BLOCK_CORE.sp = pGBc_registers->sp;
	BESS_BLOCK_CORE.ime = (pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn == ENABLED ? 0x01 : 0x00);
	BESS_BLOCK_CORE.ie = pGBc_memory->GBcMemoryMap.mInterruptEnable.interruptEnableMemory;
	TODO("BESS doesn't support STOP state yet!");
	BESS_BLOCK_CORE.exec_state = (pGBc_instance->GBc_state.emulatorStatus.isCPUHalted == YES ? 0x01 : 0x00);
	BESS_BLOCK_CORE.rsv = 0x00;
	// Copy 128 bytes of mmr
	std::memcpy(BESS_BLOCK_CORE.mmr, pGBc_memory->GBcMemoryMap.mIO.IOMemory, 0x80);
	// Copy the RAM info
	BESS_BLOCK_CORE.size_ram = (ROM_TYPE == ROM::GAME_BOY_COLOR ? (8 * 0x1000) : 0x2000);
	BESS_BLOCK_CORE.off_ram = ram_off;
	BESS_BLOCK_CORE.size_vram = (ROM_TYPE == ROM::GAME_BOY_COLOR ? (2 * 0x2000) : 0x2000);
	BESS_BLOCK_CORE.off_vram = vram_off;
	BESS_BLOCK_CORE.size_mbcram = getNumberOfRAMBanksUsed() * 0x2000;
	BESS_BLOCK_CORE.off_mbcram = mbc_off;
	BESS_BLOCK_CORE.size_oam = 0xA0;
	BESS_BLOCK_CORE.off_oam = oam_off;
	BESS_BLOCK_CORE.size_hram = 0x7F;
	BESS_BLOCK_CORE.off_hram = hram_off;
	BESS_BLOCK_CORE.size_bg_pram = (ROM_TYPE == ROM::GAME_BOY_COLOR ? 0x40 : 0x00);
	BESS_BLOCK_CORE.off_bg_pram = bg_pram_off;
	BESS_BLOCK_CORE.size_obj_pram = (ROM_TYPE == ROM::GAME_BOY_COLOR ? 0x40 : 0x00);
	BESS_BLOCK_CORE.off_obj_pram = obj_pram_off;

	// Update the size of the block
	BESS_BLOCK_CORE.BESS_BLOCK_HEADER.blk_len = 0xD0;

	// Save to file
	save.write(reinterpret_cast<char*>(&BESS_BLOCK_CORE), sizeof(BESS_BLOCK_CORE_t));

	// 4) BESS_BLOCK_XOAM_t

	BESS_BLOCK_XOAM_t BESS_BLOCK_XOAM;

	// Copy the name of block
	std::memcpy(BESS_BLOCK_XOAM.BESS_BLOCK_HEADER.ascii_ident, "XOAM", 4);

	for (INC8 jj = ZERO; jj < 0x60; jj++)
	{
		BESS_BLOCK_XOAM.xoam[jj] = pGBc_memory->GBcMemoryMap.mForbidden[jj];
	}

	// Update the size of the block
	BESS_BLOCK_XOAM.BESS_BLOCK_HEADER.blk_len = 0x60;

	// Save to file
	save.write(reinterpret_cast<char*>(&BESS_BLOCK_XOAM), sizeof(BESS_BLOCK_XOAM_t));

	// 5) BESS_BLOCK_MBC_t

	if (!isNoMBC())
	{
		uint32_t mbc_size = RESET;

		if (isMBC1())
		{
			mbc_size = 12;
		}
		else if (isMBC2())
		{
			mbc_size = 6;
		}
		else if (isMBC3())
		{
			mbc_size = 15;
		}
		else if (isMBC5())
		{
			mbc_size = 12;
		}

		// allocate struct + extra space for mbc[]
		BESS_BLOCK_MBC_t* BESS_BLOCK_MBC = (BESS_BLOCK_MBC_t*)std::malloc(sizeof(BESS_BLOCK_MBC_t) + mbc_size);

		// Copy the name of block
		std::memcpy(BESS_BLOCK_MBC->BESS_BLOCK_HEADER.ascii_ident, "MBC ", 4);

		if (isMBC1())
		{
			auto mbc_reg = 0x0000;
			BESS_BLOCK_MBC->mbc[0x00] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x01] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x02] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg0;
			//BESS_BLOCK_MBC->mbc[0x02] = ((isRAMBankEnabled() == YES) ? 0x0A : 0x00);
			mbc_reg = 0x2000;
			BESS_BLOCK_MBC->mbc[0x06] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x07] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x08] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg1;
			//BESS_BLOCK_MBC->mbc[0x08] = pGBc_instance->GBc_state.emulatorStatus.currentROMBankNumber.mbc1Fields.romBankLo;
			mbc_reg = 0x6000;
			BESS_BLOCK_MBC->mbc[0x03] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x04] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x05] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg3;
			//BESS_BLOCK_MBC->mbc[0x05] = ((pGBc_instance->GBc_state.emulatorStatus.isMBC1_Mode1 == true) ? 0x01 : 0x00);
			mbc_reg = 0x4000;
			BESS_BLOCK_MBC->mbc[0x09] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0A] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0B] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg2;
			//BESS_BLOCK_MBC->mbc[0x0B] = pGBc_instance->GBc_state.emulatorStatus.currentROMBankNumber.mbc1Fields.romBankHi;
		}
		else if (isMBC2())
		{
			if (pGBc_instance->GBc_state.emulatorStatus.isMBC2ROMMode == YES)
			{
				auto mbc_reg = 0x0000;
				BESS_BLOCK_MBC->mbc[0x00] = mbc_reg & 0xFF;
				BESS_BLOCK_MBC->mbc[0x01] = (mbc_reg >> EIGHT) & 0xFF;
				BESS_BLOCK_MBC->mbc[0x02] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg0;
				//BESS_BLOCK_MBC->mbc[0x02] = ((isRAMBankEnabled() == YES) ? 0x0A : 0x00);
				mbc_reg = 0x0100;
				BESS_BLOCK_MBC->mbc[0x03] = mbc_reg & 0xFF;
				BESS_BLOCK_MBC->mbc[0x04] = (mbc_reg >> EIGHT) & 0xFF;
				BESS_BLOCK_MBC->mbc[0x05] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg1;
				//BESS_BLOCK_MBC->mbc[0x05] = getROMBankNumber();
			}
			else
			{
				auto mbc_reg = 0x0100;
				BESS_BLOCK_MBC->mbc[0x00] = mbc_reg & 0xFF;
				BESS_BLOCK_MBC->mbc[0x01] = (mbc_reg >> EIGHT) & 0xFF;
				BESS_BLOCK_MBC->mbc[0x02] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg1;
				//BESS_BLOCK_MBC->mbc[0x02] = getROMBankNumber();
				mbc_reg = 0x0000;
				BESS_BLOCK_MBC->mbc[0x03] = mbc_reg & 0xFF;
				BESS_BLOCK_MBC->mbc[0x04] = (mbc_reg >> EIGHT) & 0xFF;
				BESS_BLOCK_MBC->mbc[0x05] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg0;
				//BESS_BLOCK_MBC->mbc[0x05] = ((isRAMBankEnabled() == YES) ? 0x0A : 0x00);
			}
		}
		else if (isMBC3())
		{
			auto mbc_reg = 0x0000;
			BESS_BLOCK_MBC->mbc[0x00] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x01] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x02] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg0;
			//BESS_BLOCK_MBC->mbc[0x02] = ((isRAMBankEnabled() == YES) ? 0x0A : 0x00);
			mbc_reg = 0x2000;
			BESS_BLOCK_MBC->mbc[0x03] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x04] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x05] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg1;
			//BESS_BLOCK_MBC->mbc[0x05] = getROMBankNumber() & 0xFF;
			mbc_reg = 0x4000;
			BESS_BLOCK_MBC->mbc[0x06] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x07] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x08] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg2;
			mbc_reg = 0x6000;
			BESS_BLOCK_MBC->mbc[0x09] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0A] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0B] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg3;
			mbc_reg = 0xA000;
			BESS_BLOCK_MBC->mbc[0x0C] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0D] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0E] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg4;
		}
		else if (isMBC5())
		{
			auto mbc_reg = 0x0000;
			BESS_BLOCK_MBC->mbc[0x00] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x01] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x02] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg0;
			//BESS_BLOCK_MBC->mbc[0x02] = ((isRAMBankEnabled() == YES) ? 0x0A : 0x00);
			mbc_reg = 0x2000;
			BESS_BLOCK_MBC->mbc[0x03] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x04] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x05] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg1;
			//BESS_BLOCK_MBC->mbc[0x05] = getROMBankNumber() & 0xFF;
			mbc_reg = 0x3000;
			BESS_BLOCK_MBC->mbc[0x06] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x07] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x08] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg2;
			//BESS_BLOCK_MBC->mbc[0x08] = getRAMBankNumber();
			mbc_reg = 0x4000;
			BESS_BLOCK_MBC->mbc[0x09] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0A] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0B] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg3;
			//BESS_BLOCK_MBC->mbc[0x0B] = getRAMBankNumber();
		}

		// Update the size of the block
		BESS_BLOCK_MBC->BESS_BLOCK_HEADER.blk_len = mbc_size;

		// Save to file
		save.write(reinterpret_cast<char*>(BESS_BLOCK_MBC), sizeof(BESS_BLOCK_MBC_t) + mbc_size);
	}

	// 6) BESS_BLOCK_RTC_t

	if (isMBC3())
	{
		BESS_BLOCK_RTC_t BESS_BLOCK_RTC;

		// Copy the name of block
		std::memcpy(BESS_BLOCK_RTC.BESS_BLOCK_HEADER.ascii_ident, "RTC ", 4);

		BESS_BLOCK_RTC.curr_sec = pGBc_instance->GBc_state.rtc.rtcFields.rtc_S;
		BESS_BLOCK_RTC.curr_min = pGBc_instance->GBc_state.rtc.rtcFields.rtc_M;
		BESS_BLOCK_RTC.curr_hr = pGBc_instance->GBc_state.rtc.rtcFields.rtc_H;
		BESS_BLOCK_RTC.curr_day = pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL;
		BESS_BLOCK_RTC.curr_ovf = pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHMemory;
		BESS_BLOCK_RTC.latched_sec = pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_S;
		BESS_BLOCK_RTC.latched_min = pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_M;
		BESS_BLOCK_RTC.latched_hr = pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_H;
		BESS_BLOCK_RTC.latched_day = pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DL;
		BESS_BLOCK_RTC.latched_ovf = pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DH.rtcDHMemory;
		BESS_BLOCK_RTC.unix_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		// Update the size of the block
		BESS_BLOCK_RTC.BESS_BLOCK_HEADER.blk_len = 0x60;

		// Save to file
		save.write(reinterpret_cast<char*>(&BESS_BLOCK_RTC), sizeof(BESS_BLOCK_RTC_t));
	}

	// 7) BESS_BLOCK_END_t

	BESS_BLOCK_END_t BESS_BLOCK_END;

	// Copy the name of block
	std::memcpy(BESS_BLOCK_END.BESS_BLOCK_HEADER.ascii_ident, "END ", 4);

	// Update the size of the block
	BESS_BLOCK_END.BESS_BLOCK_HEADER.blk_len = 0x00;

	// Save to file
	save.write(reinterpret_cast<char*>(&BESS_BLOCK_END), sizeof(BESS_BLOCK_END_t));

	// 8) BESS_FOOTER_t

	BESS_FOOTER_t BESS_FOOTER;

	// Update the BESS tag
	std::memcpy(BESS_FOOTER.ascii_tag, "BESS", 4);

	// Update the offset to first block
	BESS_FOOTER.off_blk_0 = first_blk_off;

	// Save to file
	save.write(reinterpret_cast<char*>(&BESS_FOOTER), sizeof(BESS_FOOTER_t));

	save.close();

	status = true;

	RETURN status;
}

bool GBc_t::bessLoadState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = getSaveStateName(
		pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer
		, sizeof(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer)
	);

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);
	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	// Open file in binary mode
	std::ifstream save(saveStateNameForThisROM.c_str(), std::ios::binary);
	if (!save)
	{
		WARN("Not able to open save state");
		RETURN FAILURE;
	}

	// Maintain dummy internal state for tracking BESS blocks
	FLAG BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_TOTAL)] = { NO };

	// Verify POD
	static_assert(std::is_trivially_copyable<BESS_BLOCK_NAME_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_NAME_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_INFO_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_INFO_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_CORE_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_CORE_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_XOAM_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_XOAM_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_MBC_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_MBC_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_RTC_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_RTC_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_BLOCK_END_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_BLOCK_END_t>::value, "not standard layout");
	static_assert(std::is_trivially_copyable<BESS_FOOTER_t>::value, "not trivially copyable");
	static_assert(std::is_standard_layout<BESS_FOOTER_t>::value, "not standard layout");

	INC32 curr_off = RESET;
	INC32 ram_off = RESET;
	INC32 vram_off = RESET;
	INC32 mbc_off = RESET;
	INC32 oam_off = RESET;
	INC32 hram_off = RESET;
	INC32 bg_pram_off = RESET;
	INC32 obj_pram_off = RESET;
	INC32 first_blk_off = RESET;

	auto read_NAME_block = [&](std::ifstream& save, const BESS_BLOCK_HEADER_t& BESS_BLOCK_HEADER)
		{
			EVENT("This is the NAME block!");
			BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_NAME)] = YES;

			BESS_BLOCK_NAME_t BESS_BLOCK_NAME;
			save.read(reinterpret_cast<char*>(&BESS_BLOCK_NAME), sizeof(BESS_BLOCK_NAME_t));

			// Extract null-terminated string
			auto len = 0;
			while (len < sizeof(BESS_BLOCK_NAME.name_ver) && BESS_BLOCK_NAME.name_ver[len] != '\0') ++len;
			std::string emuNameAndVer(BESS_BLOCK_NAME.name_ver, len);
			DEBUG("Emulator : %s", emuNameAndVer.c_str());
		};

	auto read_INFO_block = [&](std::ifstream& save, const BESS_BLOCK_HEADER_t& BESS_BLOCK_HEADER)
		{
			EVENT("This is the INFO block!");
			BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_INFO)] = YES;

			BESS_BLOCK_INFO_t BESS_BLOCK_INFO;
			save.read(reinterpret_cast<char*>(&BESS_BLOCK_INFO), sizeof(BESS_BLOCK_INFO_t));

			std::string romTitle(BESS_BLOCK_INFO.title, sizeof(BESS_BLOCK_INFO.title));
			DEBUG("ROM Title: %s", romTitle.c_str());
			DEBUG("ROM Checksum: 0x%X", BESS_BLOCK_INFO.chksum);
		};

	auto read_CORE_block = [&](std::ifstream& save, const BESS_BLOCK_HEADER_t& BESS_BLOCK_HEADER)
		{
			EVENT("This is the CORE block!");
			BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_CORE)] = YES;

			BESS_BLOCK_CORE_t BESS_BLOCK_CORE;
			save.read(reinterpret_cast<char*>(&BESS_BLOCK_CORE), sizeof(BESS_BLOCK_CORE_t));

			if (BESS_BLOCK_CORE.maj_bess_ver != 1)
			{
				FATAL("BESS version mismatch");
				RETURN FAILURE;
			}

			std::string model(BESS_BLOCK_CORE.mdl_indent, sizeof(BESS_BLOCK_CORE.mdl_indent));
			DEBUG("GB Model: %s", model.c_str());

			if (((ROM_TYPE == ROM::GAME_BOY_COLOR) && (BESS_BLOCK_CORE.mdl_indent[0] != 'C')) 
				|| ((ROM_TYPE == ROM::GAME_BOY) && (BESS_BLOCK_CORE.mdl_indent[0] != 'G')))
			{
				FATAL("BESS gb model mismatch");
				RETURN FAILURE;
			}

			// Restore CPU registers
			pGBc_registers->pc = BESS_BLOCK_CORE.pc;
			pGBc_registers->af.af_u16memory = BESS_BLOCK_CORE.af;
			pGBc_registers->bc.bc_u16memory = BESS_BLOCK_CORE.bc;
			pGBc_registers->de.de_u16memory = BESS_BLOCK_CORE.de;
			pGBc_registers->hl.hl_u16memory = BESS_BLOCK_CORE.hl;
			pGBc_registers->sp = BESS_BLOCK_CORE.sp;
			pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn = (BESS_BLOCK_CORE.ime == 0x01 ? ENABLED : DISABLED);
			pGBc_memory->GBcMemoryMap.mInterruptEnable.interruptEnableMemory = BESS_BLOCK_CORE.ie;
			pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = (BESS_BLOCK_CORE.exec_state == 0x01 ? YES : NO);

			if (BESS_BLOCK_CORE.exec_state == 2)
			{
				FATAL("BESS doesn't support STOP state yet!");
				RETURN FAILURE;
			}

			// Restore WRAM
			size_t total_size = BESS_BLOCK_CORE.size_ram;
			size_t bank_size = (ROM_TYPE == ROM::GAME_BOY_COLOR) ? 0x1000 : 0x2000;
			save.seekg(static_cast<std::streamoff>(BESS_BLOCK_CORE.off_ram), std::ios::beg);

			// First bank
			size_t to_read = std::min(bank_size, total_size);
			save.read(reinterpret_cast<char*>(pGBc_memory->GBcMemoryMap.mWorkRam.wRamMemory), to_read);
			total_size -= to_read;

			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				for (int bank = 0; bank < 7 && total_size > 0; bank++)
				{
					to_read = std::min(bank_size, total_size);
					save.read(reinterpret_cast<char*>(pGBc_instance->GBc_state.entireWram01.wram01MemoryBanks.mWRAM01Banks[bank]), to_read);
					total_size -= to_read;
				}
			}

			// VRAM
			size_t total_vram_size = BESS_BLOCK_CORE.size_vram;
			bank_size = 0x2000;
			save.seekg(static_cast<std::streamoff>(BESS_BLOCK_CORE.off_vram), std::ios::beg);
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				for (int bank = 0; bank < 2 && total_vram_size > 0; bank++)
				{
					to_read = std::min(bank_size, total_vram_size);
					save.read(reinterpret_cast<char*>(pGBc_instance->GBc_state.entireVram.vramMemoryBanks.mVRAMBanks[bank]), to_read);
					total_vram_size -= to_read;
				}
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				to_read = std::min(bank_size, total_vram_size);
				save.read(reinterpret_cast<char*>(pGBc_memory->GBcMemoryMap.mVideoRam.videoRamMemory), to_read);
				total_vram_size -= to_read;
			}

			// MBCRAM
			size_t mbc_total_size = BESS_BLOCK_CORE.size_mbcram;
			save.seekg(static_cast<std::streamoff>(BESS_BLOCK_CORE.off_mbcram), std::ios::beg);
			size_t num_ram_banks = getNumberOfRAMBanksUsed();
			for (size_t bank = 0; bank < num_ram_banks && mbc_total_size > 0; bank++)
			{
				to_read = std::min(static_cast<size_t>(0x2000), mbc_total_size);
				save.read(reinterpret_cast<char*>(pGBc_instance->GBc_state.entireRam.ramMemoryBanks.mRAMBanks[bank]), to_read);
				mbc_total_size -= to_read;
			}

			// OAM
			save.seekg(static_cast<std::streamoff>(BESS_BLOCK_CORE.off_oam), std::ios::beg);
			to_read = std::min(static_cast<size_t>(0xA0), (size_t)BESS_BLOCK_CORE.size_oam);
			save.read(reinterpret_cast<char*>(pGBc_memory->GBcMemoryMap.mOAM.OAMMemory), to_read);

			// HRAM
			save.seekg(static_cast<std::streamoff>(BESS_BLOCK_CORE.off_hram), std::ios::beg);
			to_read = std::min(static_cast<size_t>(0x7F), (size_t)BESS_BLOCK_CORE.size_hram);
			save.read(reinterpret_cast<char*>(pGBc_memory->GBcMemoryMap.mHighRam.highRamMemory), to_read);

			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				// BG PRAM
				save.seekg(static_cast<std::streamoff>(BESS_BLOCK_CORE.off_bg_pram), std::ios::beg);
				to_read = std::min(static_cast<size_t>(0x40), (size_t)BESS_BLOCK_CORE.size_bg_pram);
				save.read(reinterpret_cast<char*>(pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAMMemory), to_read);

				// OBJ PRAM
				save.seekg(static_cast<std::streamoff>(BESS_BLOCK_CORE.off_obj_pram), std::ios::beg);
				to_read = std::min(static_cast<size_t>(0x40), (size_t)BESS_BLOCK_CORE.size_obj_pram);
				save.read(reinterpret_cast<char*>(pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAMMemory), to_read);
			}

			// Restore MMR (I/O memory)
			uint8_t size = static_cast<uint8_t>(sizeof(pGBc_memory->GBcMemoryMap.mIO.IOMemory));
			bessIoSeq(BESS_BLOCK_CORE.mmr, size);

			RETURN SUCCESS;
		};

	auto read_XOAM_block = [&](std::ifstream& save, const BESS_BLOCK_HEADER_t& BESS_BLOCK_HEADER)
		{
			EVENT("This is the XOAM block!");
			BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_XOAM)] = YES;

			// Calculate how much we can safely read into mForbidden
			size_t to_read = std::min(sizeof(pGBc_memory->GBcMemoryMap.mForbidden), static_cast<size_t>(BESS_BLOCK_HEADER.blk_len));

			// Read into buffer
			save.read(reinterpret_cast<char*>(pGBc_memory->GBcMemoryMap.mForbidden), to_read);

			if (!save)
			{
				FATAL("Failed to read XOAM block data");
				RETURN FAILURE;
			}

			// If block is larger than our buffer, skip the remaining bytes in file
			if (BESS_BLOCK_HEADER.blk_len > to_read)
			{
				save.seekg(BESS_BLOCK_HEADER.blk_len - to_read, std::ios::cur);
			}

			RETURN SUCCESS;
		};

	auto read_MBC_block = [&](std::ifstream& save, const BESS_BLOCK_HEADER_t& BESS_BLOCK_HEADER)
		{
			EVENT("This is the MBC block!");
			BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_MBC)] = YES;

			if (!isNoMBC())
			{
				size_t blk_len = static_cast<size_t>(BESS_BLOCK_HEADER.blk_len);

				if (blk_len % 3 != 0)
				{
					FATAL("MBC block length is not a multiple of 3");
					RETURN FAILURE;
				}

				// Allocate a vector to hold the MBC data
				std::vector<uint8_t> mbc_data(blk_len);

				// Read the block from file
				save.read(reinterpret_cast<char*>(mbc_data.data()), blk_len);
				if (static_cast<size_t>(save.gcount()) != blk_len)
				{
					FATAL("Failed to read complete MBC block");
					RETURN FAILURE;
				}

				// Restore MBC as sequence of writes
				for (size_t ii = 0; ii < blk_len; ii += 3)
				{
					uint16_t mbc_reg = (mbc_data[ii + 1] << 8) | mbc_data[ii];
					writeRawMemory(mbc_reg, mbc_data[ii + 2], MEMORY_ACCESS_SOURCE::BESS);
				}
			}
			else
			{
				FATAL("ROM doesn't support MBC, but MBC block was found");
				RETURN FAILURE;
			}

			RETURN SUCCESS;
		};

	auto read_RTC_block = [&](std::ifstream& save, const BESS_BLOCK_HEADER_t& BESS_BLOCK_HEADER)
		{
			EVENT("This is the RTC block!");
			BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_RTC)] = YES;

			if (isMBC3())
			{
				BESS_BLOCK_RTC_t BESS_BLOCK_RTC;
				save.read(reinterpret_cast<char*>(&BESS_BLOCK_RTC), sizeof(BESS_BLOCK_RTC_t));

				pGBc_instance->GBc_state.rtc.rtcFields.rtc_S = BESS_BLOCK_RTC.curr_sec;
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_M = BESS_BLOCK_RTC.curr_min;
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_H = BESS_BLOCK_RTC.curr_hr;
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL = BESS_BLOCK_RTC.curr_day;
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHMemory = BESS_BLOCK_RTC.curr_ovf;

				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_S = BESS_BLOCK_RTC.latched_sec;
				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_M = BESS_BLOCK_RTC.latched_min;
				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_H = BESS_BLOCK_RTC.latched_hr;
				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DL = BESS_BLOCK_RTC.latched_day;
				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DH.rtcDHMemory = BESS_BLOCK_RTC.latched_ovf;

				TODO("What to do with UNIX time stamp present in RTC block");
				RETURN SUCCESS;
			}
			else
			{
				FATAL("RTC block is present in a non mbc3 rom");
				RETURN FAILURE;
			}
		};

	auto read_END_block = [&](std::ifstream& save, const BESS_BLOCK_HEADER_t& BESS_BLOCK_HEADER)
		{
			EVENT("This is the END block!");
			BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_END)] = YES;
		};

	// Decode footer and see if this is really a BESS file

	// Seek to end minus footer size
	save.seekg(-static_cast<std::streamoff>(sizeof(BESS_FOOTER_t)), std::ios::end);

	// Read footer into struct
	BESS_FOOTER_t BESS_FOOTER;
	save.read(reinterpret_cast<char*>(&BESS_FOOTER), sizeof(BESS_FOOTER_t));

	if (std::memcmp(BESS_FOOTER.ascii_tag, "BESS", 4) != 0)
	{
		FATAL("Not BESS compliant (BESS FOOTER missing)");
		RETURN FAILURE;
	}

	first_blk_off = BESS_FOOTER.off_blk_0;

	// Move to the first block
	save.seekg(static_cast<std::streamoff>(first_blk_off), std::ios::beg);

	// Now read the first block header
	BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;
	save.read(reinterpret_cast<char*>(&BESS_BLOCK_HEADER), sizeof(BESS_BLOCK_HEADER_t));

	// You can verify the block's ascii_ident and blk_len
	char ident0[5] = {}; // null-terminate for printing
	std::memcpy(ident0, BESS_BLOCK_HEADER.ascii_ident, 4);
	DEBUG("---------------------------------------------------------");
	DEBUG("BESS Block Name : %s", ident0);
	DEBUG("BESS Block Size : %u", BESS_BLOCK_HEADER.blk_len);
	// Seek back to start of first block
	save.seekg(static_cast<std::streamoff>(first_blk_off), std::ios::beg);
	if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_NAME)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "NAME", 4) == 0))
	{
		read_NAME_block(save, BESS_BLOCK_HEADER);
	}
	else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_INFO)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "INFO", 4) == 0))
	{
		read_INFO_block(save, BESS_BLOCK_HEADER);
	}
	else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_CORE)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "CORE", 4) == 0))
	{
		read_CORE_block(save, BESS_BLOCK_HEADER);
	}
	else
	{
		FATAL("Invalid first BESS block: expected NAME, INFO, or CORE");
		RETURN FAILURE;
	}
	DEBUG("---------------------------------------------------------");

	// Get the size of current block from header
	auto first_blk_size = BESS_BLOCK_HEADER.blk_len + sizeof(BESS_BLOCK_HEADER_t);
	// Seek to next block
	save.seekg(static_cast<std::streamoff>(first_blk_off + first_blk_size), std::ios::beg);
	// Now read the block header
	save.read(reinterpret_cast<char*>(&BESS_BLOCK_HEADER), sizeof(BESS_BLOCK_HEADER_t));
	// You can verify the block's ascii_ident and blk_len
	char ident1[5] = {}; // null-terminate for printing
	std::memcpy(ident1, BESS_BLOCK_HEADER.ascii_ident, 4);
	DEBUG("---------------------------------------------------------");
	DEBUG("BESS Block Name : %s", ident1);
	DEBUG("BESS Block Size : %u", BESS_BLOCK_HEADER.blk_len);
	// Seek back to start of block
	save.seekg(static_cast<std::streamoff>(first_blk_off + first_blk_size), std::ios::beg);
	if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_INFO)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "INFO", 4) == 0))
	{
		read_INFO_block(save, BESS_BLOCK_HEADER);
	}
	else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_CORE)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "CORE", 4) == 0))
	{
		read_CORE_block(save, BESS_BLOCK_HEADER);
	}
	else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_XOAM)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "XOAM", 4) == 0))
	{
		read_XOAM_block(save, BESS_BLOCK_HEADER);
	}
	else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_MBC)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "MBC ", 4) == 0))
	{
		read_MBC_block(save, BESS_BLOCK_HEADER);
	}
	else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_RTC)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "RTC ", 4) == 0))
	{
		read_RTC_block(save, BESS_BLOCK_HEADER);
	}
	else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_END)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "END ", 4) == 0))
	{
		read_END_block(save, BESS_BLOCK_HEADER);
	}
	else
	{
		WARN("Unexpected BESS block encountered");
	}
	DEBUG("---------------------------------------------------------");

	// Return back to start of second block
	save.seekg(static_cast<std::streamoff>(first_blk_off + first_blk_size), std::ios::beg);
	auto current_pos = save.tellg();

	while (BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_END)] == NO)
	{
		auto curr_blk_size = BESS_BLOCK_HEADER.blk_len + sizeof(BESS_BLOCK_HEADER_t);

		// Seek to next block
		save.seekg(current_pos + static_cast<std::streamoff>(curr_blk_size), std::ios::beg);
		current_pos = save.tellg();

		// Try to read new block header
		save.read(reinterpret_cast<char*>(&BESS_BLOCK_HEADER), sizeof(BESS_BLOCK_HEADER_t));

		if (!save)
		{
			// Reached EOF or failed to read header
			FATAL("Unexpected end of file: no END block found");
			RETURN FAILURE;
		}

		// Null-terminate ascii_ident for printing
		char ident[5] = {};
		std::memcpy(ident, BESS_BLOCK_HEADER.ascii_ident, 4);

		DEBUG("---------------------------------------------------------");
		DEBUG("BESS Block Name : %s", ident);
		DEBUG("BESS Block Size : %u", BESS_BLOCK_HEADER.blk_len);

		// Go back to saved current_pos
		save.seekg(current_pos, std::ios::beg);

		// Dispatch based on block type
		if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_CORE)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "CORE", 4) == 0))
		{
			read_CORE_block(save, BESS_BLOCK_HEADER);
		}
		else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_XOAM)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "XOAM", 4) == 0))
		{
			read_XOAM_block(save, BESS_BLOCK_HEADER);
		}
		else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_MBC)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "MBC ", 4) == 0))
		{
			read_MBC_block(save, BESS_BLOCK_HEADER);
		}
		else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_RTC)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "RTC ", 4) == 0))
		{
			read_RTC_block(save, BESS_BLOCK_HEADER);
		}
		else if ((BESS_OCCURENCE[TO_UINT(BESS_BLOCKS::BESS_END)] == NO) && (std::memcmp(BESS_BLOCK_HEADER.ascii_ident, "END ", 4) == 0))
		{
			read_END_block(save, BESS_BLOCK_HEADER);
		}
		else
		{
			WARN("Unexpected BESS block encountered");
		}

		DEBUG("---------------------------------------------------------");
	}

	save.close();

	displayCompleteScreen();

	status = true;

	RETURN status;
}

#ifndef __RPI_PICO__

// =====================================================================================
// GB/GBC Printer
// =====================================================================================

void GBcPrinterEngine_t::reset()
{
	state = GB_PRINTER_STATE::GB_PRINTER_NONE;

	txByte = ZERO;
	rxByte = ZERO;

	txBitCount = ZERO;
	rxBitCount = ZERO;

	command = GB_PRINTER_COMMAND::INIT;
	packetLength = ZERO;
	packetIndex = ZERO;
	checksum = ZERO;

	txState = GB_PRINTER_TX_STATE::GB_PRINTER_TX_RESPONSE;
}

void GBcPrinterEngine_t::startPacket()
{
	reset();
	state = GB_PRINTER_STATE::GB_PRINTER_COMMAND;
}

FLAG GBcPrinterEngine_t::sendBitToGB(BIT* bitToSend)
{
	*bitToSend = GETBIT(SEVEN - txBitCount, txByte);

	txBitCount++;

	if (txBitCount == EIGHT)
	{
		txBitCount = ZERO;

		if (txState == GB_PRINTER_TX_STATE::GB_PRINTER_TX_RESPONSE)
		{
			// 0x81 has been sent.
			// The next byte is the printer status.

			txByte = status;

			txState = GB_PRINTER_TX_STATE::GB_PRINTER_TX_STATUS;
		}
		else if (txState == GB_PRINTER_TX_STATE::GB_PRINTER_TX_STATUS)
		{
			// Status byte has been sent.

			txByte = ZERO;

			txState = GB_PRINTER_TX_STATE::GB_PRINTER_TX_NONE;
		}
	}

	RETURN SUCCESS;
}

FLAG GBcPrinterEngine_t::receiveBitFromGB(BIT bitReceived)
{
	rxByte <<= ONE;

	if (bitReceived == ONE)
	{
		SETBIT(rxByte, ZERO);
	}

	rxBitCount++;

	if (rxBitCount == EIGHT)
	{
		processReceivedByte(rxByte);

		rxByte = ZERO;
		rxBitCount = ZERO;
	}

	RETURN SUCCESS;
}

void GBcPrinterEngine_t::dispatchCommand()
{
	switch (command)
	{
	case GB_PRINTER_COMMAND::INIT:
	{
		imageBuffer.clear();
		printTicksRemaining = ZERO;

		// INIT clears everything, including any pending error/printing bits
		// -- EXCEPT a paper jam. Real hardware can't be talked out of an
		// empty roll by the Game Boy re-initializing the link; the jam bit
		// stays set until the person physically changes the paper (here:
		// closes the jammed roll's window, see drawImGuiWindows).
		status = ZERO;

		if (isPaperJammed)
		{
			status |= STATUS_PAPER_JAM;
		}

		BREAK;
	}

	case GB_PRINTER_COMMAND::DATA:
	{
		// An empty DATA packet (length 0) is the "end of image" marker.
		// Real hardware reports STATUS_UNPROCESSED once it has a full,
		// unprinted image sitting in its buffer.
		if (packetLength == ZERO && !imageBuffer.empty())
		{
			status |= STATUS_UNPROCESSED;
		}

		BREAK;
	}

	case GB_PRINTER_COMMAND::PRINT:
	{
		if (isPaperJammed)
		{
			// Out of paper. Refuse the print -- do NOT consume imageBuffer,
			// so the game's retry (most driver code does retry on an error
			// status) will actually have data to work with once the jam
			// clears.
			status |= STATUS_PAPER_JAM;
			BREAK;
		}

		// Only start a print job if we actually have a completed image.
		if (status & STATUS_UNPROCESSED)
		{
			status &= ~STATUS_UNPROCESSED;
			status |= STATUS_PRINTING;

			// numSheets == 0 means "line feed only": the head advances the
			// paper by the margins but no image is burned. numSheets >= 1
			// means N physical copies -- each one is a real sheet fed
			// through the printer, so each counts against the roll's
			// MAX_PRINTS_PER_ROLL cap (and can independently trip a jam
			// mid-run if the roll runs out partway through).
			if (printArgs.numSheets == ZERO)
			{
				appendBlankFeedToRoll();
			}
			else
			{
				for (uint32_t sheet = 0; sheet < printArgs.numSheets && !isPaperJammed; sheet++)
				{
					appendPrintToRoll(imageBuffer);
				}
			}

			imageBuffer.clear();

			// Real print time scales with number of sheets -- one sheet's
			// worth of burn+feed time per copy, not a flat duration
			// regardless of numSheets.
			printTicksRemaining = PRINT_DURATION_TICKS * std::max<uint32_t>(1, printArgs.numSheets);
		}
		else
		{
			status |= STATUS_OTHER_ERROR;
		}

		BREAK;
	}

	case GB_PRINTER_COMMAND::STATUS:
	default:
		// No state change; the response already reports current `status`.
		BREAK;
	}
}

void GBcPrinterEngine_t::tick()
{
	// Status/timing simulation only. The image itself was already committed
	// (rendered/saved) synchronously in dispatchCommand()'s PRINT case --
	// don't touch imageBuffer here.
	if (status & STATUS_PRINTING)
	{
		if (printTicksRemaining > ZERO)
		{
			printTicksRemaining--;
		}

		if (printTicksRemaining == ZERO)
		{
			// Printing finished: swap PRINTING for IMAGE_FULL ("done, buffer
			// still holds the last image") until the game clears it via a
			// fresh INIT (or the 16 zero-byte buffer-clear some games send).
			status &= ~STATUS_PRINTING;
			status |= STATUS_IMAGE_FULL;
		}
	}
}

void GBcPrinterEngine_t::processReceivedByte(BYTE dataReceived)
{
	//INFO("GB Printer received byte: 0x%02X", dataReceived);

	switch (state)
	{
	case GB_PRINTER_STATE::GB_PRINTER_NONE:
	{
		if (dataReceived == 0x88)
		{
			state = GB_PRINTER_STATE::GB_PRINTER_MAGIC_33;
		}

		BREAK;
	}

	case GB_PRINTER_STATE::GB_PRINTER_MAGIC_33:
	{
		if (dataReceived == 0x33)
		{
			state = GB_PRINTER_STATE::GB_PRINTER_COMMAND;
		}
		else
		{
			state = GB_PRINTER_STATE::GB_PRINTER_NONE;
		}

		BREAK;
	}

	case GB_PRINTER_STATE::GB_PRINTER_COMMAND:
	{
		command = static_cast<GB_PRINTER_COMMAND>(dataReceived);

		checksum = dataReceived;

		state = GB_PRINTER_STATE::GB_PRINTER_COMPRESSION;

		BREAK;
	}

	case GB_PRINTER_STATE::GB_PRINTER_COMPRESSION:
	{
		compression = dataReceived;

		checksum += dataReceived;

		state = GB_PRINTER_STATE::GB_PRINTER_LENGTH_LOW;

		BREAK;
	}

	case GB_PRINTER_STATE::GB_PRINTER_LENGTH_LOW:
	{
		packetLength = dataReceived;

		checksum += dataReceived;

		state = GB_PRINTER_STATE::GB_PRINTER_LENGTH_HIGH;

		BREAK;
	}

	case GB_PRINTER_STATE::GB_PRINTER_LENGTH_HIGH:
	{
		packetLength |= static_cast<uint16_t>(dataReceived) << 8;

		checksum += dataReceived;

		packetIndex = ZERO;

		if (packetLength != ZERO)
		{
			state = GB_PRINTER_STATE::GB_PRINTER_DATA;
		}
		else
		{
			state = GB_PRINTER_STATE::GB_PRINTER_CHECKSUM_LOW;
		}

		BREAK;
	}

	case GB_PRINTER_STATE::GB_PRINTER_DATA:
	{
		// GB_PRINTER_DATA is reused by both the DATA command (tile pixel
		// data, goes to imageBuffer) and the PRINT command (4 fixed
		// argument bytes: sheets/margins/palette/exposure). Route by
		// `command` so PRINT's args don't get appended to the image.
		if (command == GB_PRINTER_COMMAND::PRINT)
		{
			switch (packetIndex)
			{
			case 0: printArgs.numSheets = dataReceived; BREAK;
			case 1: printArgs.margins = dataReceived; BREAK;
			case 2: printArgs.palette = dataReceived; BREAK;
			case 3: printArgs.exposure = dataReceived; BREAK;
			default: BREAK; // spec caps this at 4 bytes; ignore anything past it
			}
		}
		else if (command == GB_PRINTER_COMMAND::DATA)
		{
			if (compression == SET)
			{
				// State 1: We are expecting a new Run Control Header byte
				if (runLength == 0)
				{
					// Bit 7 determines run type: 1 = Compressed, 0 = Uncompressed
					isCompressedRun = (dataReceived & 0x80) != 0;

					// Extract Bits 0-6 for length:
					// Compressed runs offset by +2 (min length 2)
					// Uncompressed runs offset by +1 (min length 1)
					uint8_t count = dataReceived & 0x7F;
					runLength = count + (isCompressedRun ? 2 : 1);
				}
				// State 2: Processing data bytes belonging to the current run
				else
				{
					if (isCompressedRun)
					{
						// Repeat the received byte 'runLength' times into the buffer in one call
						imageBuffer.insert(imageBuffer.end(), runLength, dataReceived);
						runLength = 0; // Compressed run fully consumed in a single payload byte
					}
					else
					{
						// Pass-through raw byte verbatim and decrement remaining uncompressed count
						imageBuffer.push_back(dataReceived);
						runLength--;
					}
				}
			}
			else
			{
				// Compression flag unset: Direct verbatim transfer
				imageBuffer.push_back(dataReceived);
			}
		}
		else
		{
			// A command we don't expect to carry a data payload (INIT,
			// STATUS) showed up with packetLength > 0. Not supposed to
			// happen per spec -- flag it rather than silently dropping the
			// bytes, since this is more likely a bug upstream (header
			// parsing, a new/unhandled command) than something to ignore.
			status |= STATUS_PACKET_ERROR;
		}

		checksum += dataReceived;

		packetIndex++;

		if (packetIndex >= packetLength)
		{
			state = GB_PRINTER_STATE::GB_PRINTER_CHECKSUM_LOW;
		}

		BREAK;
	}

	case GB_PRINTER_STATE::GB_PRINTER_CHECKSUM_LOW:
	{
		receivedChecksum = dataReceived;

		state = GB_PRINTER_STATE::GB_PRINTER_CHECKSUM_HIGH;

		BREAK;
	}

	case GB_PRINTER_STATE::GB_PRINTER_CHECKSUM_HIGH:
	{
		receivedChecksum |=
			static_cast<uint16_t>(dataReceived) << 8;

		if (checksum != receivedChecksum)
		{
			status |= STATUS_CHECKSUM_ERROR;
		}
		else
		{
			status &= ~STATUS_CHECKSUM_ERROR;

			// Only act on the command if the packet actually checked out.
			dispatchCommand();
		}

		// Prepare the FIRST response byte immediately.
		txByte = STATUS_RESPONSE;
		txBitCount = ZERO;
		txState = GB_PRINTER_TX_STATE::GB_PRINTER_TX_RESPONSE;

		state = GB_PRINTER_STATE::GB_PRINTER_NONE;

		BREAK;
	}

	default:
		state = GB_PRINTER_STATE::GB_PRINTER_NONE;
		BREAK;
	}
}

std::vector<BYTE> GBcPrinterEngine_t::decodeTilesToRgba(const std::vector<BYTE>& pixelData, uint32_t& outHeightPx)
{
	constexpr uint32_t TILE_WIDTH_PX = 8;
	constexpr uint32_t TILE_HEIGHT_PX = 8;
	constexpr uint32_t TILES_PER_ROW = 20; // ROLL_WIDTH_PX / TILE_WIDTH_PX
	constexpr uint32_t BYTES_PER_TILE = 16;

	outHeightPx = ZERO;

	if (pixelData.empty())
	{
		RETURN{};
	}

	uint32_t tileCount = static_cast<uint32_t>(pixelData.size() / BYTES_PER_TILE);
	if (tileCount == ZERO)
	{
		INFO("GB Printer: pixelData smaller than one tile (%zu bytes), skipping", pixelData.size());
		RETURN{};
	}

	uint32_t rowCount = (tileCount + TILES_PER_ROW - 1) / TILES_PER_ROW;
	uint32_t imgHeight = rowCount * TILE_HEIGHT_PX;
	outHeightPx = imgHeight;

	std::vector<BYTE> rgba(static_cast<size_t>(ROLL_WIDTH_PX) * imgHeight * 4, 255);

	// Exposure (7-bit, default $40 per GB Camera) sets thermal head burn
	// time -> print darkness. Manual: -25% darkness at $00, +25% at $7F,
	// $40 = nominal (0%). No documented formula for values in between, so
	// this linearly scales how much "ink" (i.e. how far from white) each
	// shade gets, which matches the two documented endpoints and is roughly
	// physically sensible for a thermal head. Treat as approximate.
	float exposureAdjustment = (static_cast<float>(printArgs.exposure) - 64.0f) / 64.0f * 0.25f;
	float darknessScale = 1.0f + exposureAdjustment; // ~0.75 .. ~1.246875

	for (uint32_t tileIndex = 0; tileIndex < tileCount; tileIndex++)
	{
		uint32_t tileCol = tileIndex % TILES_PER_ROW;
		uint32_t tileRow = tileIndex / TILES_PER_ROW;

		const BYTE* tile = &pixelData[static_cast<size_t>(tileIndex) * BYTES_PER_TILE];

		for (uint32_t py = 0; py < TILE_HEIGHT_PX; py++)
		{
			BYTE lo = tile[py * 2];
			BYTE hi = tile[py * 2 + 1];

			for (uint32_t px = 0; px < TILE_WIDTH_PX; px++)
			{
				BIT loBit = GETBIT(SEVEN - px, lo);
				BIT hiBit = GETBIT(SEVEN - px, hi);
				BYTE rawColorIndex = static_cast<BYTE>((hiBit << 1) | loBit);

				BYTE colorNumber = GET_GB_COLOR_NUMBER(printArgs.palette, rawColorIndex);
				BYTE baseGray = static_cast<BYTE>(255 - (colorNumber * 85));

				// Scale "ink" (distance from white) by exposure, not the
				// raw gray value directly -- keeps white (255) white
				// regardless of exposure, since exposure is a burn-time
				// setting, not a global brightness knob.
				float ink = (255.0f - baseGray) * darknessScale;
				BYTE gray = static_cast<BYTE>(255.0f - std::clamp(ink, 0.0f, 255.0f));

				uint32_t x = tileCol * TILE_WIDTH_PX + px;
				uint32_t y = tileRow * TILE_HEIGHT_PX + py;
				size_t rgbaOffset = (static_cast<size_t>(y) * ROLL_WIDTH_PX + x) * 4;

				rgba[rgbaOffset + 0] = gray;
				rgba[rgbaOffset + 1] = gray;
				rgba[rgbaOffset + 2] = gray;
				rgba[rgbaOffset + 3] = 255;
			}
		}
	}

	RETURN rgba;
}

void GBcPrinterEngine_t::appendPrintToRoll(const std::vector<BYTE>& pixelData)
{
	PrintedImageWindow* roll = nullptr;

	if (activeRollId >= 0)
	{
		roll = findWindowById(static_cast<uint64_t>(activeRollId));
	}

	if (roll == nullptr)
	{
		// No active roll (first print ever, or previous one was torn off /
		// hit the paper-count cap) -- start a fresh one.
		PrintedImageWindow newRoll;
		newRoll.id = nextRollId++;
		newRoll.title = "GB Print Roll #" + std::to_string(newRoll.id);
		newRoll.width = ROLL_WIDTH_PX;
		newRoll.height = 0;

		printedImageWindows.push_back(std::move(newRoll));
		roll = &printedImageWindows.back();

		activeRollId = static_cast<int64_t>(roll->id);
	}

	// Presence, not magnitude: real ROMs use these nibbles inconsistently
	// enough that only "was a margin sent at all" is meaningfully portable
	// across games -- the exact nibble value isn't a reliable pixel count.
	bool hasMarginBefore = ((printArgs.margins >> 4) & 0x0F) != 0;
	bool hasMarginAfter = (printArgs.margins & 0x0F) != 0;

	if (hasMarginBefore)
	{
		roll->blocks.push_back(RollBlock{ true, {}, 0 });
	}

	uint32_t thisPrintHeightPx = ZERO;
	std::vector<BYTE> thisPrintRgba = decodeTilesToRgba(pixelData, thisPrintHeightPx);
	roll->blocks.push_back(RollBlock{ false, std::move(thisPrintRgba), static_cast<int>(thisPrintHeightPx) });

	if (hasMarginAfter)
	{
		roll->blocks.push_back(RollBlock{ true, {}, 0 });
	}

	roll->printCountInRoll++;
	roll->savedToDisk = false; // roll's content changed since last save
	roll->savedPath.clear();

	recompositeRoll(*roll);

	INFO("GB Printer: appended print #%u to roll '%s' (now %dpx tall)",
		roll->printCountInRoll, roll->title.c_str(), roll->height);

	if (roll->printCountInRoll >= MAX_PRINTS_PER_ROLL)
	{
		// Real paper roll is physically out (Nintendo's spec: up to 180
		// prints per roll). This is a genuine hardware condition, not just
		// bookkeeping -- report it to the game as a real jam, and stop
		// accepting further prints until the person "changes the paper" by
		// closing this window (see drawImGuiWindows).
		INFO("GB Printer: roll '%s' hit %u prints (paper spec limit) -- reporting paper jam",
			roll->title.c_str(), MAX_PRINTS_PER_ROLL);

		roll->isJamSource = true;
		isPaperJammed = true;
		status |= STATUS_PAPER_JAM;
		activeRollId = -1;
	}
}

void GBcPrinterEngine_t::appendBlankFeedToRoll()
{
	PrintedImageWindow* roll = nullptr;

	if (activeRollId >= 0)
	{
		roll = findWindowById(static_cast<uint64_t>(activeRollId));
	}

	if (roll == nullptr)
	{
		PrintedImageWindow newRoll;
		newRoll.id = nextRollId++;
		newRoll.title = "GB Print Roll #" + std::to_string(newRoll.id);
		newRoll.width = ROLL_WIDTH_PX;
		newRoll.height = 0;

		printedImageWindows.push_back(std::move(newRoll));
		roll = &printedImageWindows.back();

		activeRollId = static_cast<int64_t>(roll->id);
	}

	// numSheets == 0 is a feed-only operation, not a print -- no image, and
	// (unlike appendPrintToRoll) it deliberately does NOT touch
	// printCountInRoll, since it isn't one of the roll's 180 rated prints.
	bool hasAnyMargin = printArgs.margins != 0;

	if (hasAnyMargin)
	{
		roll->blocks.push_back(RollBlock{ true, {}, 0 });
	}

	roll->savedToDisk = false;
	roll->savedPath.clear();

	recompositeRoll(*roll);
}

void GBcPrinterEngine_t::regenerateRollTexture(PrintedImageWindow& window)
{
	if (window.textureId != 0)
	{
		glDeleteTextures(1, &window.textureId);
	}

	glGenTextures(1, &window.textureId);
	glBindTexture(GL_TEXTURE_2D, window.textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window.width, window.height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, window.rgbaPixels.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GBcPrinterEngine_t::recompositeRoll(PrintedImageWindow& window)
{
	window.rgbaPixels.clear();
	window.height = 0;

	for (const auto& block : window.blocks)
	{
		if (block.isGap)
		{
			uint32_t gapRows = static_cast<uint32_t>(std::max(0, cosmeticGapPx));
			if (gapRows > 0)
			{
				window.rgbaPixels.insert(window.rgbaPixels.end(),
					static_cast<size_t>(ROLL_WIDTH_PX) * gapRows * 4, static_cast<BYTE>(255));
				window.height += static_cast<int>(gapRows);
			}
		}
		else
		{
			window.rgbaPixels.insert(window.rgbaPixels.end(), block.imageRgba.begin(), block.imageRgba.end());
			window.height += block.imageHeightPx;
		}
	}

	window.width = ROLL_WIDTH_PX;

	if (window.height > 0)
	{
		regenerateRollTexture(window);
	}
}

GBcPrinterEngine_t::PrintedImageWindow* GBcPrinterEngine_t::findWindowById(uint64_t id)
{
	auto it = std::find_if(printedImageWindows.begin(), printedImageWindows.end(),
		[id](const PrintedImageWindow& w) { RETURN w.id == id; });

	RETURN(it != printedImageWindows.end()) ? &(*it) : nullptr;
}

void GBcPrinterEngine_t::drawImGuiWindows()
{
	// Force these to stay floating/undocked rather than snapping into
	// whatever dockspace the rest of your UI uses -- these are printouts,
	// not tool panels.
	ImGuiWindowClass windowClass;
	windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoDocking;

	constexpr float MIN_DISPLAY_SCALE = 1.0f;
	constexpr float MAX_DISPLAY_SCALE = 8.0f;

	for (auto& window : printedImageWindows)
	{
		if (!window.open)
		{
			continue;
		}

		ImGui::SetNextWindowClass(&windowClass);

		// AlwaysAutoResize removes the manual resize grip entirely, so the
		// window can never be dragged into a non-uniform aspect ratio, and
		// it always sizes exactly to its content (nothing gets clipped, so
		// "Save PNG" is always visible without scrolling). Zoom is done
		// deliberately via the buttons below instead, scaling width/height
		// together every time.
		if (ImGui::Begin(window.title.c_str(), &window.open,
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_MenuBar))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Settings"))
				{
					if (ImGui::SliderInt("Print gap (px)", &cosmeticGapPx, 0, 32))
					{
						for (auto& w : printedImageWindows)
						{
							recompositeRoll(w);
						}
					}
					ImGui::TextDisabled("Cosmetic spacing only -- not hardware-accurate.");
					ImGui::TextDisabled("Applies to every gap, on every roll, immediately.");
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::Image(
				(ImTextureID)(intptr_t)window.textureId,
				ImVec2(window.width * window.displayScale, window.height * window.displayScale));

			ImGui::Spacing();

			if (ImGui::SmallButton("-"))
			{
				window.displayScale = std::max(MIN_DISPLAY_SCALE, window.displayScale - 1.0f);
			}
			ImGui::SameLine();
			ImGui::Text("%.0fx", window.displayScale);
			ImGui::SameLine();
			if (ImGui::SmallButton("+"))
			{
				window.displayScale = std::min(MAX_DISPLAY_SCALE, window.displayScale + 1.0f);
			}

			ImGui::SameLine();
			ImGui::Dummy(ImVec2(20.0f, 0.0f)); // gap before the save controls
			ImGui::SameLine();

			if (window.savedToDisk)
			{
				// Disabled instead of hidden, so the button doesn't jump
				// around / the window doesn't resize when this flips.
				ImGui::BeginDisabled();
				ImGui::Button("Saved");
				ImGui::EndDisabled();
			}
			else if (ImGui::Button("Save PNG"))
			{
				window.savedToDisk = savePrintedImageAsPng(window);
			}

			if (window.savedToDisk && !window.savedPath.empty())
			{
				ImGui::TextDisabled("%s", window.savedPath.c_str());
			}

			if (window.isJamSource && isPaperJammed)
			{
				ImGui::Separator();
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
					"Paper jam: roll is full. Close this window to load fresh paper.");
			}

			// Only the roll currently receiving prints gets the tuning
			// control -- adjusting it won't affect rolls that already
			// finished/were torn off, so showing it there would be
			// misleading clutter.
		}
		ImGui::End();
	}

	// If the window the printer is currently feeding got torn off (closed),
	// stop feeding it -- next print starts a fresh roll. Checked here,
	// before the erase below, since erase can shift/invalidate references.
	if (activeRollId >= 0)
	{
		PrintedImageWindow* active = findWindowById(static_cast<uint64_t>(activeRollId));
		if (active == nullptr || !active->open)
		{
			activeRollId = -1;
		}
	}

	// Closing the roll that caused a jam is the emulated equivalent of the
	// person opening the printer and loading a fresh roll -- that's the
	// only thing that clears STATUS_PAPER_JAM (see the INIT/PRINT cases in
	// dispatchCommand, which otherwise keep it sticky on purpose).
	if (isPaperJammed)
	{
		for (const auto& window : printedImageWindows)
		{
			if (window.isJamSource && !window.open)
			{
				isPaperJammed = false;
				status &= ~STATUS_PAPER_JAM;
				INFO("GB Printer: jam cleared -- '%s' torn off, fresh paper loaded", window.title.c_str());
				BREAK;
			}
		}
	}

	// Prune closed windows and free their GL textures. Done as a second pass
	// so we're not mutating printedImageWindows while iterating it above.
	printedImageWindows.erase(
		std::remove_if(printedImageWindows.begin(), printedImageWindows.end(),
			[](const PrintedImageWindow& w)
			{
				if (!w.open)
				{
					GLuint id = w.textureId;
					glDeleteTextures(1, &id);
				}
				RETURN !w.open;
			}),
		printedImageWindows.end());
}

bool GBcPrinterEngine_t::savePrintedImageAsPng(PrintedImageWindow& window)
{
	if (window.rgbaPixels.empty())
	{
		INFO("GB Printer: no pixel data to save for '%s'", window.title.c_str());
		RETURN FALSE;
	}

	// For now: a "gb_prints" folder next
	// to wherever the process's current working directory is, created if
	// it doesn't exist, so it's at least deterministic and discoverable
	// rather than silently landing in whatever CWD happened to be.
	std::filesystem::path outputDir = std::filesystem::current_path() / "gb_prints";

	std::error_code ec;
	std::filesystem::create_directories(outputDir, ec);
	if (ec)
	{
		INFO("GB Printer: failed to create output directory %s (%s)",
			outputDir.string().c_str(), ec.message().c_str());
		RETURN FALSE;
	}

	static uint32_t saveSequenceNumber = ZERO;
	char filename[64];
	snprintf(filename, sizeof(filename), "gb_print_%03u.png", saveSequenceNumber++);

	std::filesystem::path outputPath = outputDir / filename;

	int result = stbi_write_png(
		outputPath.string().c_str(),
		window.width,
		window.height,
		4, // RGBA
		window.rgbaPixels.data(),
		window.width * 4); // stride in bytes

	if (result == 0)
	{
		INFO("GB Printer: failed to write %s", outputPath.string().c_str());
		RETURN FALSE;
	}

	window.savedPath = std::filesystem::absolute(outputPath).string();

	INFO("GB Printer: saved '%s' to %s", window.title.c_str(), window.savedPath.c_str());
	RETURN TRUE;
}

// =====================================================================================
// GBC Debugger
// =====================================================================================

void GBc_t::debugSyncScreenIfNeeded()
{
	debugEventViewerCheck();	// own master switch, fully independent of ppu.enabled below

	// Master switch off -> behaves exactly like a non-debug build beyond the check above.
	if (gbcDebugger.ppu.enabled == NO) RETURN;

	// Run-to-breakpoint check runs every dot regardless of the "enabled" master switch --
	// it's a plain int comparison (cheap) independent of the texture-refresh feature.
	if (gbcDebugger.ppu.runToBreakpointArmed == YES
		&& pGBc_peripherals->LY == gbcDebugger.ppu.breakpointLY
		&& pGBc_display->pixelRenderCounterPerScanLine == gbcDebugger.ppu.breakpointDot)
	{
		gbcDebugger.ppu.runToBreakpointArmed = NO;
		gbcDebugger.ppu.paused = YES;
	}

	// ---- live per-pixel layer capture (BG / Window / OBJ) ---------------------------
	// Detects the most recently committed pixel by watching pixelRenderCounterPerScanLine
	// advance, tags it with whichever layer actually produced it -- read straight from
	// prevCGBPixelIsBG/prevCGBPixelIsOBJ, which your own commit code already sets on both
	// the DMG and CGB paths -- and files it into the BG/Window buffers plus the tagged
	// Complete Viewport buffer. Cleared once per frame.
	if (ENABLED)
	{
		const int curLY = pGBc_peripherals->LY;
		const int curCounter = (int)pGBc_display->pixelRenderCounterPerScanLine;

		if (curLY == 0 && debugLastCapturedLY != 0)
		{
			debugLiveBGPixels.fill(Pixel(ZERO, ZERO, ZERO, 255));
			debugLiveWindowPixels.fill(Pixel(ZERO, ZERO, ZERO, 255));
			debugViewportPixels.fill(Pixel(ZERO, ZERO, ZERO, 255));
			debugViewportSource.fill((uint8_t)PIXEL_SOURCE_TAG::NONE);
			debugViewportPixelInfo.fill(PixelDebugInfo_t());
			debugWindowPixelsCapturedThisFrame = ZERO;
		}

		if (curLY != debugLastCapturedLY)
		{
			debugLastPixelCounterCaptured = -1;
		}

		if (curCounter == debugLastPixelCounterCaptured + 1
			&& curCounter >= 1 && curCounter <= (int)getScreenWidth()
			&& curLY < (int)getScreenHeight())
		{
			const int x = curCounter - 1;
			const int y = curLY;
			Pixel committed = pGBc_display->imGuiBuffer.imGuiBuffer2D[y][x];

			// Two separate flag pairs exist in the real commit code (CGB path vs DMG path) --
			// only one of them will ever be non-NO for a given pixel, so checking both is safe
			// and makes this correct regardless of which path actually ran.
			const FLAG isObjPixel = (pGBc_display->prevCGBPixelIsOBJ == YES || pGBc_display->prevDMGPixelIsOBJ == YES) ? YES : NO;
			const FLAG isBgPixel = (pGBc_display->prevCGBPixelIsBG == YES || pGBc_display->prevDMGPixelIsBG == YES) ? YES : NO;

			PIXEL_SOURCE_TAG source = PIXEL_SOURCE_TAG::NONE;
			if (isObjPixel == YES) source = PIXEL_SOURCE_TAG::OBJ;
			else if (isBgPixel == YES)
				source = (pGBc_display->shouldFetchAndRenderWindowInsteadOfBG == YES) ? PIXEL_SOURCE_TAG::WINDOW : PIXEL_SOURCE_TAG::BG;

			if (source == PIXEL_SOURCE_TAG::WINDOW)
			{
				debugLiveWindowPixels[y * getScreenWidth() + x] = committed;
				debugWindowPixelsCapturedThisFrame++;
			}
			else if (source == PIXEL_SOURCE_TAG::BG)
				debugLiveBGPixels[y * getScreenWidth() + x] = committed;

			debugViewportPixels[y * getScreenWidth() + x] = committed;
			debugViewportSource[y * getScreenWidth() + x] = (uint8_t)source;

			PixelDebugInfo_t& info = debugViewportPixelInfo[y * getScreenWidth() + x];
			info.LY = (BYTE)y;
			info.pixelRenderCounterPerScanLine = (uint16_t)x;
			info.ppuCounterPerLY = pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY;
			info.ppuCounterPerMode = pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode;
			info.ppuCounterPerFrame = pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerFrame;
			info.pixelFetcherState = (int)pGBc_display->pixelFetcherState;
			info.capturedSCX = pGBc_peripherals->SCX;
			info.capturedSCY = pGBc_peripherals->SCY;
			info.captured = YES;
		}

		debugLastCapturedLY = curLY;
		debugLastPixelCounterCaptured = curCounter;
	}
}

void GBc_t::debugEventViewerCheck()
{
	if (gbcDebugger.eventViewer.enabled == NO) RETURN;

	const int regCount = (int)GBC_DEBUG_TRACKED_REGISTER::COUNT;
	uint8_t currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::COUNT];
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::LCDC] = pGBc_peripherals->LCDC.lcdControlMemory;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::STAT] = pGBc_peripherals->STAT.lcdStatusMemory;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::SCX] = pGBc_peripherals->SCX;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::SCY] = pGBc_peripherals->SCY;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::LY] = pGBc_peripherals->LY;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::LYC] = pGBc_peripherals->LYC;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::DMA] = pGBc_peripherals->DMA;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::BGP] = pGBc_peripherals->BGP;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::OBP0] = pGBc_peripherals->OBP0;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::OBP1] = pGBc_peripherals->OBP1;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::WX] = pGBc_peripherals->WX;
	currentValues[(int)GBC_DEBUG_TRACKED_REGISTER::WY] = pGBc_peripherals->WY;

	const int curLY = pGBc_peripherals->LY;
	const int curDot = (int)pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY;
	auto& ev = gbcDebugger.eventViewer;

	if (curLY == 0 && ev.lastLY != 0)
	{
		ev.count = 0;
		ev.head = 0;
		ev.frameCounter++;
		for (int r = 0; r < 154; r++)
			for (int d = 0; d < 456; d++)
				ev.modeTimeline[r][d] = ZERO;
	}
	ev.lastLY = curLY;

	if (curLY >= 0 && curLY < 154 && curDot >= 0 && curDot < 456)
	{
		ev.modeTimeline[curLY][curDot] = (uint8_t)(pGBc_peripherals->STAT.lcdStatusFields.MODE & 0x03);
	}

	if (ev.snapshotValid == NO)
	{
		for (int r = 0; r < regCount; r++) ev.lastValues[r] = currentValues[r];
		ev.snapshotValid = YES;
		RETURN;
	}

	for (int r = 0; r < regCount; r++)
	{
		if (currentValues[r] == ev.lastValues[r]) continue;

		PPUEvent_t& e = ev.ring[ev.head];
		e.frameNumber = ev.frameCounter;
		e.scanline = (BYTE)curLY;
		e.dot = (uint16_t)curDot;
		e.registerIndex = (uint8_t)r;
		e.oldValue = ev.lastValues[r];
		e.newValue = currentValues[r];
		e.pc = pGBc_registers->pc;

		ev.head = (ev.head + 1) % gbcDebugger_t::eventViewer_t::CAPACITY;
		if (ev.count < gbcDebugger_t::eventViewer_t::CAPACITY) ev.count++;

		ev.lastValues[r] = currentValues[r];
	}
}

void GBc_t::renderGBCDebuggerEventViewerTab()
{
	ImGui::Checkbox("Enable event tracking", &gbcDebugger.eventViewer.enabled);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Master switch. Off = zero cost, same as no debugger.\nOnly detects WRITES (value changes).");

	ImGui::SameLine();
	if (ImGui::Button("Clear"))
	{
		gbcDebugger.eventViewer.count = 0;
		gbcDebugger.eventViewer.head = 0;
		gbcDebugger.eventViewer.snapshotValid = NO;
	}

	if (gbcDebugger.eventViewer.enabled == NO)
	{
		ImGui::TextDisabled("Enable tracking to begin recording register changes.");
		RETURN;
	}

	static const char* regNames[] = { "LCDC", "STAT", "SCX", "SCY", "LY", "LYC", "DMA", "BGP", "OBP0", "OBP1", "WX", "WY" };
	static const ImU32 regColors[] = {
		IM_COL32(255, 215, 0, 255), IM_COL32(180, 255, 60, 255), IM_COL32(255, 100, 180, 255),
		IM_COL32(100, 200, 255, 255), IM_COL32(255, 255, 255, 255), IM_COL32(0, 255, 200, 255),
		IM_COL32(150, 100, 255, 255), IM_COL32(255, 140, 0, 255), IM_COL32(0, 255, 0, 255),
		IM_COL32(255, 0, 0, 255), IM_COL32(0, 200, 255, 255), IM_COL32(255, 255, 0, 255)
	};
	const int regCount = (int)GBC_DEBUG_TRACKED_REGISTER::COUNT;
	const int CAP = gbcDebugger_t::eventViewer_t::CAPACITY;

	PPUEvent_t* events = gbcDebugger.eventViewer.ring;
	int eventCount = gbcDebugger.eventViewer.count;
	int eventHead = gbcDebugger.eventViewer.head;

	auto chronoIndex = [&](int i) -> int
		{
			int start = ((eventHead - eventCount) % CAP + CAP) % CAP;
			return (start + i) % CAP;
		};

	ImVec2 fullAvail = ImGui::GetContentRegionAvail();
	float rightPanelWidth = fullAvail.x * 0.22f;
	if (rightPanelWidth < 160.0f) rightPanelWidth = 160.0f;
	if (rightPanelWidth > 260.0f) rightPanelWidth = 260.0f;
	const float scatterHeight = fullAvail.y * 0.5f;

	ImGui::BeginChild("EventScatter", ImVec2(fullAvail.x - rightPanelWidth - 8.0f, scatterHeight), true);
	{
		ImVec2 scatterAvail = ImGui::GetContentRegionAvail();
		ImVec2 scatterOrigin = ImGui::GetCursorScreenPos();
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		const float scaleX = scatterAvail.x / 456.0f;
		const float scaleY = scatterAvail.y / 154.0f;

		static int dotToScreenX[456];
		for (int row = 0; row < 154; row++)
		{
			float rowY0 = scatterOrigin.y + row * scaleY;
			float rowY1 = rowY0 + scaleY;

			if (row >= 144)
			{
				drawList->AddRectFilled(ImVec2(scatterOrigin.x, rowY0), ImVec2(scatterOrigin.x + scatterAvail.x, rowY1), IM_COL32(60, 20, 70, 255));
				continue;
			}

			for (int d = 0; d < 456; d++) dotToScreenX[d] = -1;
			for (int sx = 0; sx < 160; sx++)
			{
				const auto& info = debugViewportPixelInfo[row * 160 + sx];
				// KEY FIX: ppuCounterPerLY is the true absolute dot position this pixel
				// committed at. pixelRenderCounterPerScanLine (used here before) is just
				// "which of the 160 visible pixels this is" (0-159) -- a different, smaller
				// counter that starts near 0 regardless of when Mode 3 actually began, which
				// is exactly why the image was appearing at dot 0 and swallowing the OAM band.
				if (info.captured == YES && (int)info.LY == row)
				{
					int d = (int)info.ppuCounterPerLY;
					if (d >= 0 && d < 456) dotToScreenX[d] = sx;
				}
			}

			int d = 0;
			while (d < 456)
			{
				if (dotToScreenX[d] >= 0)
				{
					int runStart = d;
					while (d < 456 && dotToScreenX[d] >= 0) d++;
					for (int dd = runStart; dd < d; dd++)
					{
						Pixel p = debugViewportPixels[row * 160 + dotToScreenX[dd]];
						ImU32 col = IM_COL32(p.r, p.g, p.b, 255);
						float x0 = scatterOrigin.x + dd * scaleX;
						float x1 = scatterOrigin.x + (dd + 1) * scaleX;
						drawList->AddRectFilled(ImVec2(x0, rowY0), ImVec2(x1, rowY1), col);
					}
				}
				else
				{
					int runStart = d;
					uint8_t runMode = gbcDebugger.eventViewer.modeTimeline[row][d];
					while (d < 456 && dotToScreenX[d] < 0 && gbcDebugger.eventViewer.modeTimeline[row][d] == runMode) d++;
					ImU32 col;
					if (runMode == 2) col = IM_COL32(40, 60, 110, 255);		// OAM Search -- blue
					else if (runMode == 0) col = IM_COL32(110, 40, 40, 255);	// HBlank -- maroon
					else col = IM_COL32(50, 50, 50, 255);						// Mode 3 dots with no pixel captured yet -- gray
					float x0 = scatterOrigin.x + runStart * scaleX;
					float x1 = scatterOrigin.x + d * scaleX;
					drawList->AddRectFilled(ImVec2(x0, rowY0), ImVec2(x1, rowY1), col);
				}
			}
		}

		drawList->AddRect(scatterOrigin, ImVec2(scatterOrigin.x + scatterAvail.x, scatterOrigin.y + scatterAvail.y), IM_COL32(120, 120, 120, 255));

		for (int i = 0; i < eventCount; i++)
		{
			const PPUEvent_t& e = events[chronoIndex(i)];
			if (gbcDebugger.eventViewer.showRegister[e.registerIndex] == NO) continue;

			float px = scatterOrigin.x + e.dot * scaleX;
			float py = scatterOrigin.y + e.scanline * scaleY;
			drawList->AddRectFilled(ImVec2(px - 1.5f, py - 1.5f), ImVec2(px + 1.5f, py + 1.5f), e.registerIndex < 12 ? regColors[e.registerIndex] : IM_COL32(255, 255, 255, 255));
		}

		ImGui::Dummy(scatterAvail);

		if (ImGui::IsItemHovered())
		{
			ImVec2 mouse = ImGui::GetMousePos();
			int hoverDot = (int)((mouse.x - scatterOrigin.x) / scaleX);
			int hoverScanline = (int)((mouse.y - scatterOrigin.y) / scaleY);

			if (hoverDot >= 0 && hoverDot < 456 && hoverScanline >= 0 && hoverScanline < 154)
			{
				ImGui::BeginTooltip();
				ImGui::Text("Scanline: %d   Dot: %d", hoverScanline, hoverDot);

				if (hoverScanline >= 144)
				{
					ImGui::Text("Mode: VBlank");
				}
				else
				{
					uint8_t mode = gbcDebugger.eventViewer.modeTimeline[hoverScanline][hoverDot];
					static const char* modeNames[] = { "0 - HBlank", "1 - VBlank", "2 - OAM Search", "3 - Drawing" };
					ImGui::Text("Mode: %s", modeNames[mode & 0x03]);
				}

				FLAG anyEventHere = NO;
				for (int i = 0; i < eventCount; i++)
				{
					const PPUEvent_t& e = events[chronoIndex(i)];
					if ((int)e.scanline != hoverScanline || (int)e.dot != hoverDot) continue;
					if (gbcDebugger.eventViewer.showRegister[e.registerIndex] == NO) continue;

					if (anyEventHere == NO)
					{
						ImGui::Separator(); anyEventHere = YES;
					}
					ImGui::Text("%s: 0x%02X -> 0x%02X  (ROM:%04X)", regNames[e.registerIndex], e.oldValue, e.newValue, e.pc);
				}

				ImGui::EndTooltip();
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("EventRegTree", ImVec2(rightPanelWidth, scatterHeight), true);
	ImGui::Separator();
	if (ImGui::TreeNodeEx("PPU", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (int r = 0; r < regCount; r++)
		{
			ImGui::PushID(r);
			ImGui::ColorButton("##col", ImGui::ColorConvertU32ToFloat4(regColors[r]), ImGuiColorEditFlags_NoTooltip, ImVec2(10, 10));
			ImGui::SameLine();
			ImGui::Checkbox(regNames[r], &gbcDebugger.eventViewer.showRegister[r]);
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	ImGui::BeginDisabled();
	if (ImGui::TreeNodeEx("APU (Coming Soon)")) ImGui::TreePop();
	if (ImGui::TreeNodeEx("Memory (Coming Soon)")) ImGui::TreePop();
	ImGui::EndDisabled();
	ImGui::EndChild();

	static std::vector<int> filteredIndices;
	filteredIndices.clear();
	for (int i = 0; i < eventCount; i++)
	{
		const PPUEvent_t& e = events[chronoIndex(i)];
		if (gbcDebugger.eventViewer.showRegister[e.registerIndex] == YES)
			filteredIndices.push_back(chronoIndex(i));
	}

	ImGui::Text("Event log (%d of %d events shown):", (int)filteredIndices.size(), eventCount);

	ImGui::BeginChild("EventLogTable", ImVec2(0, 0), true);
	if (ImGui::BeginTable("EventLog", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
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
				const PPUEvent_t& e = events[filteredIndices[row]];
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

void GBc_t::debugEnsureTexturesCreated()
{
	if (debugTexturesInitialized == YES) RETURN;

	GL_CALL(glGenTextures(ONE, &debugTileViewerTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugTileViewerTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugBGMapTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugBGMapTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugWindowMapTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugWindowMapTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugOAMSpriteTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugOAMSpriteTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 16 * 40, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugMiniScreenTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugMiniScreenTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugTileDetailTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugTileDetailTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugLiveBGTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugLiveBGTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugLiveWindowTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugLiveWindowTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugViewportTexture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugViewportTexture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugBGMap9800Texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugBGMap9800Texture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GL_CALL(glGenTextures(ONE, &debugBGMap9C00Texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugBGMap9C00Texture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	debugTexturesInitialized = YES;
}

// ---- pixel buffer builders (CPU-side; called once per host frame at most, only while the relevant tab is visible) ----

void GBc_t::debugRebuildTileViewerPixels()
{
	const uint8_t bank = (gbcDebugger.ppu.tileViewerUseBank1 == YES) ? ONE : ZERO;
	const FLAG isCGB = (ROM_TYPE == ROM::GAME_BOY_COLOR);
	const PALETTE_ID activePalette = pGBc_instance->GBc_state.gb_palette;

	for (int tileIdx = 0; tileIdx < 384; tileIdx++)
	{
		const int tileGridX = tileIdx % 16;
		const int tileGridY = tileIdx / 16;
		const int tileByteOffset = tileIdx * 16;

		for (int row = 0; row < 8; row++)
		{
			const uint8_t lo = debugReadVRAM(bank, tileByteOffset + row * 2);
			const uint8_t hi = debugReadVRAM(bank, tileByteOffset + row * 2 + 1);

			for (int col = 0; col < 8; col++)
			{
				const uint8_t bit = 7 - col;
				const uint8_t colorIdx = (((hi >> bit) & ONE) << ONE) | ((lo >> bit) & ONE);

				Pixel outColor;
				if (isCGB)
				{
					// Tile viewer is palette-agnostic (a tile can be used by many BG/OBJ palettes),
					// so we preview it through BG palette 0 as a neutral reference.
					outColor = getColorFromColorIDForGBC(
						pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM[ZERO][colorIdx].gbcColor,
						pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
				}
				else
				{
					switch (colorIdx)
					{
					case 0: outColor = paletteIDToColor.at(activePalette).COLOR_000P.COLOR; BREAK;
					case 1: outColor = paletteIDToColor.at(activePalette).COLOR_033P.COLOR; BREAK;
					case 2: outColor = paletteIDToColor.at(activePalette).COLOR_066P.COLOR; BREAK;
					default: outColor = paletteIDToColor.at(activePalette).COLOR_099P.COLOR; BREAK;
					}
				}

				const int px = tileGridX * 8 + col;
				const int py = tileGridY * 8 + row;
				debugTileViewerPixels[py * 128 + px] = outColor;
			}
		}
	}
}

void GBc_t::debugRebuildTileDetailPixels(int tileIdx, uint8_t bank, int paletteIdx)
{
	const FLAG isCGB = (ROM_TYPE == ROM::GAME_BOY_COLOR);
	const int tileByteOffset = tileIdx * 16;

	for (int row = 0; row < 8; row++)
	{
		const uint8_t lo = debugReadVRAM(bank, tileByteOffset + row * 2);
		const uint8_t hi = debugReadVRAM(bank, tileByteOffset + row * 2 + 1);

		for (int col = 0; col < 8; col++)
		{
			const uint8_t bit = 7 - col;
			const uint8_t colorIdx = (((hi >> bit) & ONE) << ONE) | ((lo >> bit) & ONE);

			Pixel outColor;
			if (isCGB)
				outColor = getColorFromColorIDForGBC(pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM[paletteIdx][colorIdx].gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
			else
				outColor = getColorFromColorIDForGB(pGBc_peripherals->BGP, colorIdx).COLOR;

			debugTileDetailPixels[row * 8 + col] = outColor;
		}
	}
}

void GBc_t::debugRebuildSpecificBGMap(uint16_t mapBaseOffset, std::array<Pixel, 256 * 256>& outBuffer)
{
	const FLAG isCGB = (ROM_TYPE == ROM::GAME_BOY_COLOR);
	const FLAG unsignedAddressing = (pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_TILE_DATA_AREA == ONE);
	const PALETTE_ID activePalette = pGBc_instance->GBc_state.gb_palette;

	for (int mapY = 0; mapY < 32; mapY++)
	{
		for (int mapX = 0; mapX < 32; mapX++)
		{
			const uint16_t mapEntryOffset = mapBaseOffset + (mapY * 32) + mapX;
			const uint8_t tileNumber = debugReadVRAM(ZERO, mapEntryOffset);

			BYTE bgAttr = ZERO;
			if (isCGB) bgAttr = debugReadVRAM(ONE, mapEntryOffset);

			const uint8_t cgbPalette = bgAttr & 0x07;
			const uint8_t cgbBank = (bgAttr >> 3) & ONE;
			const FLAG xFlip = (bgAttr >> 5) & ONE;
			const FLAG yFlip = (bgAttr >> 6) & ONE;

			int tileIndex = unsignedAddressing ? tileNumber : (256 + (int8_t)tileNumber);
			const int tileByteOffset = tileIndex * 16;

			for (int row = 0; row < 8; row++)
			{
				const int srcRow = yFlip ? (7 - row) : row;
				const uint8_t lo = debugReadVRAM(cgbBank, tileByteOffset + srcRow * 2);
				const uint8_t hi = debugReadVRAM(cgbBank, tileByteOffset + srcRow * 2 + 1);

				for (int col = 0; col < 8; col++)
				{
					const int srcBit = xFlip ? col : (7 - col);
					const uint8_t colorIdx = (((hi >> srcBit) & ONE) << ONE) | ((lo >> srcBit) & ONE);

					Pixel outColor;
					if (isCGB)
						outColor = getColorFromColorIDForGBC(pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM[cgbPalette][colorIdx].gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
					else
						outColor = getColorFromColorIDForGB(pGBc_peripherals->BGP, colorIdx).COLOR;

					outBuffer[(mapY * 8 + row) * 256 + (mapX * 8 + col)] = outColor;
				}
			}
		}
	}
}

void GBc_t::debugRebuildOAMSpritePixels()
{
	const FLAG isCGB = (ROM_TYPE == ROM::GAME_BOY_COLOR);
	const FLAG tallSprites = (pGBc_peripherals->LCDC.lcdControlFields.OBJ_SIZE == ONE);
	const int spriteHeight = tallSprites ? 16 : 8;

	debugOAMSpritePixels.fill(Pixel(ZERO, ZERO, ZERO, ZERO)); // transparent -- color 0 is transparent for sprites

	for (int s = 0; s < 40; s++)
	{
		const OAMEntry_t& oam = pGBc_memory->GBcMemoryMap.mOAM.OAMFields.OAM[s];

		const uint8_t tileIndex = tallSprites ? (oam.tileIndex & 0xFE) : oam.tileIndex;
		const uint8_t cgbBank = isCGB ? oam.attributes.oamEntryFields.OAM_TILE_VRAM_BANK : ZERO;
		const uint8_t dmgPalNum = oam.attributes.oamEntryFields.OAM_PALETTE_NUMBER_DMG;
		const uint8_t cgbPalNum = oam.attributes.oamEntryFields.OAM_PALETTE_NUMBER_CGB;
		const FLAG xFlip = oam.attributes.oamEntryFields.OAM_X_FLIP;
		const FLAG yFlip = oam.attributes.oamEntryFields.OAM_Y_FLIP;

		for (int row = 0; row < spriteHeight; row++)
		{
			const int logicalRow = yFlip ? (spriteHeight - 1 - row) : row;
			const int tileOfRow = logicalRow / 8;
			const int rowInTile = logicalRow % 8;
			const int tileByteOffset = (tileIndex + tileOfRow) * 16;

			const uint8_t lo = debugReadVRAM(cgbBank, tileByteOffset + rowInTile * 2);
			const uint8_t hi = debugReadVRAM(cgbBank, tileByteOffset + rowInTile * 2 + 1);

			for (int col = 0; col < 8; col++)
			{
				const int srcBit = xFlip ? col : (7 - col);
				const uint8_t colorIdx = (((hi >> srcBit) & ONE) << ONE) | ((lo >> srcBit) & ONE);

				if (colorIdx == 0) continue; // transparent

				Pixel outColor;
				if (isCGB)
				{
					outColor = getColorFromColorIDForGBC(
						pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM[cgbPalNum][colorIdx].gbcColor,
						pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
				}
				else
				{
					const uint8_t obp = (dmgPalNum == ZERO) ? pGBc_peripherals->OBP0 : pGBc_peripherals->OBP1;
					outColor = getColorFromColorIDForGB(obp, colorIdx).COLOR;
				}

				const int py = s * 16 + row;
				debugOAMSpritePixels[py * 8 + col] = outColor;
			}
		}
	}
}

void GBc_t::debugRebuildViewportBGMapPixels()
{
	const FLAG isCGB = (ROM_TYPE == ROM::GAME_BOY_COLOR);
	const FLAG use9C00 = (pGBc_peripherals->LCDC.lcdControlFields.BG_TILE_MAP_AREA == ONE) ? YES : NO;
	const uint16_t mapBaseOffset = (use9C00 == YES) ? 0x1C00 : 0x1800;
	const FLAG unsignedAddressing = (pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_TILE_DATA_AREA == ONE);
	const PALETTE_ID activePalette = pGBc_instance->GBc_state.gb_palette;

	for (int mapY = 0; mapY < 32; mapY++)
	{
		for (int mapX = 0; mapX < 32; mapX++)
		{
			const uint16_t mapEntryOffset = mapBaseOffset + (mapY * 32) + mapX;
			const uint8_t tileNumber = debugReadVRAM(ZERO, mapEntryOffset);

			BYTE bgAttr = ZERO;
			if (isCGB) bgAttr = debugReadVRAM(ONE, mapEntryOffset);

			const uint8_t cgbPalette = bgAttr & 0x07;
			const uint8_t cgbBank = (bgAttr >> 3) & ONE;
			const FLAG xFlip = (bgAttr >> 5) & ONE;
			const FLAG yFlip = (bgAttr >> 6) & ONE;

			int tileIndex = unsignedAddressing ? tileNumber : (256 + (int8_t)tileNumber);
			const int tileByteOffset = tileIndex * 16;

			for (int row = 0; row < 8; row++)
			{
				const int srcRow = yFlip ? (7 - row) : row;
				const uint8_t lo = debugReadVRAM(cgbBank, tileByteOffset + srcRow * 2);
				const uint8_t hi = debugReadVRAM(cgbBank, tileByteOffset + srcRow * 2 + 1);

				for (int col = 0; col < 8; col++)
				{
					const int srcBit = xFlip ? col : (7 - col);
					const uint8_t colorIdx = (((hi >> srcBit) & ONE) << ONE) | ((lo >> srcBit) & ONE);

					Pixel outColor;
					if (isCGB)
					{
						outColor = getColorFromColorIDForGBC(
							pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM[cgbPalette][colorIdx].gbcColor,
							pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
					}
					else
					{
						outColor = getColorFromColorIDForGB(pGBc_peripherals->BGP, colorIdx).COLOR;
					}

					const int px = mapX * 8 + col;
					const int py = mapY * 8 + row;
					debugViewportMapPixels[py * 256 + px] = outColor;
				}
			}
		}
	}
}

// ---- ImGui panels ----

static ImU32 gbcDebugGridColor(FLAG white)
{
	return white ? IM_COL32(255, 255, 255, 70) : IM_COL32(0, 0, 0, 90);
}

void GBc_t::renderGBCDebuggerRegistersPanel()
{
	const BYTE lcdcRaw = pGBc_peripherals->LCDC.lcdControlMemory;
	const BYTE statRaw = pGBc_peripherals->STAT.lcdStatusMemory;
	const BYTE curLY = pGBc_peripherals->LY;
	const BYTE curLYC = pGBc_peripherals->LYC;
	const BYTE curSCX = pGBc_peripherals->SCX;
	const BYTE curSCY = pGBc_peripherals->SCY;
	const BYTE curWX = pGBc_peripherals->WX;
	const BYTE curWY = pGBc_peripherals->WY;
	const BYTE curBGP = pGBc_peripherals->BGP;
	const BYTE curOBP0 = pGBc_peripherals->OBP0;
	const BYTE curOBP1 = pGBc_peripherals->OBP1;
	const int curDot = (int)pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY;
	const int curFetcher = (int)pGBc_display->pixelFetcherState;

	auto onOff = [](uint8_t v) -> const char* { return v ? "ON" : "off"; };
	auto onOffColor = [](uint8_t v) -> ImVec4 { return v ? ImVec4(0.45f, 0.90f, 0.45f, 1.0f) : ImVec4(0.55f, 0.55f, 0.55f, 1.0f); };

	auto beginRegTable = [](const char* id) -> bool
		{
			bool opened = ImGui::BeginTable(id, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);
			if (opened)
			{
				ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			}
			return opened;
		};
	auto row = [](const char* label, const char* value, ImVec4 color = ImVec4(1, 1, 1, 1))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
			ImGui::TableNextColumn(); ImGui::TextColored(color, "%s", value);
		};

if (ImGui::CollapsingHeader("LCDC ($FF40)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Raw: 0x%02X", (uint8_t)lcdcRaw);
		if (beginRegTable("LCDCTable"))
		{
			row("LCD/PPU Enable", onOff(lcdcRaw & 0x80), onOffColor(lcdcRaw & 0x80));
			row("Window Tile Map", (lcdcRaw & 0x40) ? "$9C00" : "$9800");
			row("Window Enable", onOff(lcdcRaw & 0x20), onOffColor(lcdcRaw & 0x20));
			row("BG/Win Tile Data", (lcdcRaw & 0x10) ? "$8000" : "$8800");
			row("BG Tile Map", (lcdcRaw & 0x08) ? "$9C00" : "$9800");
			row("OBJ Size", (lcdcRaw & 0x04) ? "8x16" : "8x8");
			row("OBJ Enable", onOff(lcdcRaw & 0x02), onOffColor(lcdcRaw & 0x02));
			row("BG/Win Enable", onOff(lcdcRaw & 0x01), onOffColor(lcdcRaw & 0x01));
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("STAT ($FF41)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Raw: 0x%02X", (uint8_t)statRaw);
		static const char* modeNames[] = { "0 - HBlank", "1 - VBlank", "2 - OAM Search", "3 - Drawing" };
		if (beginRegTable("STATTable"))
		{
			row("Mode", modeNames[statRaw & 0x03], ImVec4(0.55f, 0.80f, 1.0f, 1.0f));
			row("LYC == LY", onOff(statRaw & 0x04), onOffColor(statRaw & 0x04));
			row("HBlank IRQ src", onOff(statRaw & 0x08), onOffColor(statRaw & 0x08));
			row("VBlank IRQ src", onOff(statRaw & 0x10), onOffColor(statRaw & 0x10));
			row("OAM IRQ src", onOff(statRaw & 0x20), onOffColor(statRaw & 0x20));
			row("LYC==LY IRQ src", onOff(statRaw & 0x40), onOffColor(statRaw & 0x40));
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("Position / Timing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (beginRegTable("PosTable"))
		{
			char buf[16];
			snprintf(buf, sizeof(buf), "%d", curLY); row("LY", buf);
			snprintf(buf, sizeof(buf), "%d", curLYC); row("LYC", buf);
			snprintf(buf, sizeof(buf), "%d", curSCX); row("SCX", buf);
			snprintf(buf, sizeof(buf), "%d", curSCY); row("SCY", buf);
			snprintf(buf, sizeof(buf), "%d", curWX); row("WX", buf);
			snprintf(buf, sizeof(buf), "%d", curWY); row("WY", buf);
			snprintf(buf, sizeof(buf), "%d / 456", curDot); row("Dot in scanline", buf);
			snprintf(buf, sizeof(buf), "%d", curFetcher); row("Fetcher state", buf);
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("Palettes (DMG)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (beginRegTable("DMGPalTable"))
		{
			char buf[8];
			snprintf(buf, sizeof(buf), "0x%02X", curBGP); row("BGP", buf);
			snprintf(buf, sizeof(buf), "0x%02X", curOBP0); row("OBP0", buf);
			snprintf(buf, sizeof(buf), "0x%02X", curOBP1); row("OBP1", buf);
			ImGui::EndTable();
		}
	}
}

void GBc_t::renderGBCDebuggerTileViewerPanel()
{
	if (ROM_TYPE == ROM::GAME_BOY_COLOR)
	{
		int bankSel = (gbcDebugger.ppu.tileViewerUseBank1 == YES) ? 1 : 0;
		ImGui::RadioButton("Bank 0", &bankSel, 0); ImGui::SameLine();
		ImGui::RadioButton("Bank 1", &bankSel, 1);
		gbcDebugger.ppu.tileViewerUseBank1 = (bankSel == 1) ? YES : NO;
		ImGui::SameLine();
	}
	ImGui::Checkbox("Show grid", &gbcDebugger.ppu.tileViewerShowGrid);

	debugRebuildTileViewerPixels();

	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugTileViewerTexture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 192, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugTileViewerPixels.data()));

	ImVec2 fullAvail = ImGui::GetContentRegionAvail();
	const float detailPanelWidth = 220.0f;
	const float imgAvailWidth = (gbcDebugger.ppu.selectedTileIndex >= 0) ? (fullAvail.x - detailPanelWidth - 8.0f) : fullAvail.x;

	const float aspect = 128.0f / 192.0f;
	ImVec2 imgSize = (imgAvailWidth / aspect <= fullAvail.y) ? ImVec2(imgAvailWidth, imgAvailWidth / aspect) : ImVec2(fullAvail.y * aspect, fullAvail.y);
	ImVec2 imgOrigin = ImGui::GetCursorScreenPos();
	ImGui::Image((ImTextureID)(uintptr_t)debugTileViewerTexture, imgSize);

	if (ImGui::IsItemClicked())
	{
		ImVec2 mouse = ImGui::GetMousePos();
		int col = (int)((mouse.x - imgOrigin.x) / imgSize.x * 16.0f);
		int row = (int)((mouse.y - imgOrigin.y) / imgSize.y * 24.0f);
		if (col >= 0 && col < 16 && row >= 0 && row < 24)
			gbcDebugger.ppu.selectedTileIndex = row * 16 + col;
	}

	if (gbcDebugger.ppu.tileViewerShowGrid == YES)
	{
		const float scaleX = imgSize.x / 128.0f;
		const float scaleY = imgSize.y / 192.0f;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImU32 gridColor = gbcDebugGridColor(gbcDebugger.ppu.gridColorWhite);

		for (int col = 0; col <= 16; col++)
			drawList->AddLine(ImVec2(imgOrigin.x + col * 8 * scaleX, imgOrigin.y), ImVec2(imgOrigin.x + col * 8 * scaleX, imgOrigin.y + imgSize.y), gridColor);
		for (int row = 0; row <= 24; row++)
			drawList->AddLine(ImVec2(imgOrigin.x, imgOrigin.y + row * 8 * scaleY), ImVec2(imgOrigin.x + imgSize.x, imgOrigin.y + row * 8 * scaleY), gridColor);
	}

	if (gbcDebugger.ppu.selectedTileIndex >= 0)
	{
		ImGui::SameLine();
		ImGui::BeginChild("TileDetail", ImVec2(detailPanelWidth, 0), true);

		const int tileIdx = gbcDebugger.ppu.selectedTileIndex;
		const int col = tileIdx % 16;
		const int row = tileIdx / 16;

		ImGui::Text("Tile #%d (0x%02X)", tileIdx, tileIdx);
		ImGui::Text("Grid position: col %d, row %d", col, row);

		const uint8_t detailBank = (gbcDebugger.ppu.tileViewerUseBank1 == YES) ? ONE : ZERO;
		const int detailPalette = (ROM_TYPE == ROM::GAME_BOY_COLOR) ? gbcDebugger.ppu.tileViewerPreviewPalette : ZERO;
		debugRebuildTileDetailPixels(tileIdx, detailBank, detailPalette);
		GL_CALL(glBindTexture(GL_TEXTURE_2D, debugTileDetailTexture));
		GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugTileDetailPixels.data()));
		ImVec2 detailImgAvail = ImGui::GetContentRegionAvail();
		const float detailSide = (detailImgAvail.x < 150.0f) ? detailImgAvail.x : 150.0f;
		ImGui::Image((ImTextureID)(uintptr_t)debugTileDetailTexture, ImVec2(detailSide, detailSide));

		const uint8_t bank = (gbcDebugger.ppu.tileViewerUseBank1 == YES) ? ONE : ZERO;
		const int tileByteOffset = tileIdx * 16;
		ImGui::Text("Bytes ($%04X):", 0x8000 + bank * 0x2000 + tileByteOffset);
		for (int b = 0; b < 16; b++)
		{
			ImGui::Text("%02X", debugReadVRAM(bank, tileByteOffset + b));
			if ((b + 1) % 8 != 0) ImGui::SameLine();
		}

		ImGui::Separator();

		if (ROM_TYPE == ROM::GAME_BOY_COLOR)
		{
			ImGui::Text("Preview with BG palette:");
			ImGui::SetNextItemWidth(150.0f);
			ImGui::SliderInt("##tilePalPreview", &gbcDebugger.ppu.tileViewerPreviewPalette, 0, 7);
			for (int c = 0; c < 4; c++)
			{
				Pixel colr = getColorFromColorIDForGBC(pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM[gbcDebugger.ppu.tileViewerPreviewPalette][c].gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
				ImVec4 imc = ImVec4(colr.r / 255.0f, colr.g / 255.0f, colr.b / 255.0f, 1.0f);
				ImGui::PushID(c);
				ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(24, 24));
				ImGui::PopID();
				ImGui::SameLine(0.0f, (c == 0) ? 0.0f : 4.0f);
			}
		}
		else
		{
			ImGui::Text("Shades shown (via BGP):");
			for (int c = 0; c < 4; c++)
			{
				Pixel colr = getColorFromColorIDForGB(pGBc_peripherals->BGP, c).COLOR;
				ImVec4 imc = ImVec4(colr.r / 255.0f, colr.g / 255.0f, colr.b / 255.0f, 1.0f);
				ImGui::PushID(c);
				ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(24, 24));
				ImGui::PopID();
				ImGui::SameLine(0.0f, (c == 0) ? 0.0f : 4.0f);
			}
		}

		ImGui::EndChild();
	}
}

void GBc_t::renderGBCDebuggerBGMapPanel()
{
	ImGui::Checkbox("Show grid", &gbcDebugger.ppu.bgMapViewerShowGrid);

	if (pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE == ZERO)
		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "BG/Window disabled via LCDC bit 0 -- expected, not a bug.");
	ImGui::Text("SCX: %d  SCY: %d", pGBc_peripherals->SCX, pGBc_peripherals->SCY);

	const FLAG activeIs9C00 = (pGBc_peripherals->LCDC.lcdControlFields.BG_TILE_MAP_AREA == ONE) ? YES : NO;

	debugRebuildSpecificBGMap(0x1800, debugBGMap9800Pixels);
	debugRebuildSpecificBGMap(0x1C00, debugBGMap9C00Pixels);

	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugBGMap9800Texture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugBGMap9800Pixels.data()));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugBGMap9C00Texture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugBGMap9C00Pixels.data()));

	ImVec2 avail = ImGui::GetContentRegionAvail();
	const float side = ((avail.x * 0.5f) < avail.y ? (avail.x * 0.5f) : avail.y) - 8.0f;

	auto drawOneMap = [&](const char* label, FLAG isActive, GLuint tex)
		{
			ImGui::BeginGroup();
			ImGui::TextColored(isActive ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s%s", label, isActive ? " (ACTIVE)" : "");
			ImVec2 imgOrigin = ImGui::GetCursorScreenPos();
			ImGui::Image((ImTextureID)(uintptr_t)tex, ImVec2(side, side));
			if (gbcDebugger.ppu.bgMapViewerShowGrid == YES)
			{
				const float scale = side / 256.0f;
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				const ImU32 gridColor = gbcDebugGridColor(gbcDebugger.ppu.gridColorWhite);
				for (int col = 0; col <= 32; col++)
					drawList->AddLine(ImVec2(imgOrigin.x + col * 8 * scale, imgOrigin.y), ImVec2(imgOrigin.x + col * 8 * scale, imgOrigin.y + side), gridColor);
				for (int row = 0; row <= 32; row++)
					drawList->AddLine(ImVec2(imgOrigin.x, imgOrigin.y + row * 8 * scale), ImVec2(imgOrigin.x + side, imgOrigin.y + row * 8 * scale), gridColor);
			}
			ImGui::EndGroup();
		};

	drawOneMap("$9800", activeIs9C00 == NO, debugBGMap9800Texture);
	ImGui::SameLine();
	drawOneMap("$9C00", activeIs9C00 == YES, debugBGMap9C00Texture);
}

void GBc_t::renderGBCDebuggerWindowMapPanel()
{
	ImGui::Checkbox("Show grid", &gbcDebugger.ppu.winMapViewerShowGrid);
	ImGui::Text("WX: %d  WY: %d", pGBc_peripherals->WX, pGBc_peripherals->WY);

	if (debugWindowPixelsCapturedThisFrame == ZERO)
	{
		if (pGBc_peripherals->LCDC.lcdControlFields.WINDOW_LAYER_ENABLE == ZERO)
			ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Window layer disabled via LCDC bit 5 (as of the most recent read).");
		else
			ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Window hasn't drawn anything this frame (not triggered yet, or WX/WY places it off-screen).");
		RETURN;
	}

	if (pGBc_peripherals->LCDC.lcdControlFields.WINDOW_LAYER_ENABLE == ZERO)
		ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Note: window was drawn earlier this frame, then disabled (common for status-bar effects).");

	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugLiveWindowTexture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugLiveWindowPixels.data()));

	ImVec2 avail = ImGui::GetContentRegionAvail();
	const float aspect = (float)getScreenWidth() / (float)getScreenHeight();
	ImVec2 imgSize = (avail.x / aspect <= avail.y) ? ImVec2(avail.x, avail.x / aspect) : ImVec2(avail.y * aspect, avail.y);
	ImVec2 imgOrigin = ImGui::GetCursorScreenPos();
	ImGui::Image((ImTextureID)(uintptr_t)debugLiveWindowTexture, imgSize);

	if (gbcDebugger.ppu.winMapViewerShowGrid == YES)
	{
		const float scaleX = imgSize.x / (float)getScreenWidth();
		const float scaleY = imgSize.y / (float)getScreenHeight();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImU32 gridColor = gbcDebugGridColor(gbcDebugger.ppu.gridColorWhite);
		for (int col = 0; col <= 20; col++)
			drawList->AddLine(ImVec2(imgOrigin.x + col * 8 * scaleX, imgOrigin.y), ImVec2(imgOrigin.x + col * 8 * scaleX, imgOrigin.y + imgSize.y), gridColor);
		for (int row = 0; row <= 18; row++)
			drawList->AddLine(ImVec2(imgOrigin.x, imgOrigin.y + row * 8 * scaleY), ImVec2(imgOrigin.x + imgSize.x, imgOrigin.y + row * 8 * scaleY), gridColor);
	}
}

void GBc_t::renderGBCDebuggerViewportPanel()
{
	ImGui::Checkbox("BG", &gbcDebugger.ppu.viewportShowBG); ImGui::SameLine();
	ImGui::Checkbox("Window", &gbcDebugger.ppu.viewportShowWindow); ImGui::SameLine();
	ImGui::Checkbox("Sprites", &gbcDebugger.ppu.viewportShowSprites); ImGui::SameLine();
	ImGui::Checkbox("Show viewport rect", &gbcDebugger.ppu.viewportShowViewportRect); ImGui::SameLine();
	ImGui::Checkbox("Show grid", &gbcDebugger.ppu.viewportShowGrid);

	if (gbcDebugger.ppu.viewportShowBG == YES)
		debugRebuildViewportBGMapPixels();
	else
		debugViewportMapPixels.fill(Pixel(ZERO, ZERO, ZERO, 255));

	// Rectangle uses "live/current" SCX/SCY -- for a ROM that changes scroll mid-frame
	// (a raster split), there is no single correct rectangle; this represents the scroll
	// position as of the end of the last completed frame. See PPU_DEBUGGER.md.
	const int scx = pGBc_peripherals->SCX;
	const int scy = pGBc_peripherals->SCY;

	// Overlay placement uses the SCX/SCY captured AT THE TIME each specific pixel committed,
	// not the live register -- critical for raster-split ROMs where scroll changes every
	// scanline (e.g. dmg-acid2). Using the live value here placed every row except the very
	// last one at the wrong map position, since their true scroll had already changed by the
	// time this panel re-reads the register.
	for (int sy = 0; sy < 144; sy++)
	{
		for (int sx = 0; sx < 160; sx++)
		{
			const PixelDebugInfo_t& pxInfo = debugViewportPixelInfo[sy * 160 + sx];
			if (pxInfo.captured == NO) continue;

			PIXEL_SOURCE_TAG src = (PIXEL_SOURCE_TAG)debugViewportSource[sy * 160 + sx];
			FLAG shouldOverlay =
				(src == PIXEL_SOURCE_TAG::WINDOW && gbcDebugger.ppu.viewportShowWindow == YES) ||
				(src == PIXEL_SOURCE_TAG::OBJ && gbcDebugger.ppu.viewportShowSprites == YES);

			if (shouldOverlay == YES)
			{
				const int mapX = (sx + pxInfo.capturedSCX) & 255;
				const int mapY = (sy + pxInfo.capturedSCY) & 255;
				debugViewportMapPixels[mapY * 256 + mapX] = debugViewportPixels[sy * 160 + sx];
			}
		}
	}

	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugViewportTexture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugViewportMapPixels.data()));

	ImVec2 avail = ImGui::GetContentRegionAvail();
	const float side = (avail.x < avail.y) ? avail.x : avail.y;
	ImVec2 imgOrigin = ImGui::GetCursorScreenPos();
	ImGui::Image((ImTextureID)(uintptr_t)debugViewportTexture, ImVec2(side, side));

	if (ImGui::IsItemClicked())
	{
		const float scale = side / 256.0f;
		ImVec2 mouse = ImGui::GetMousePos();
		int mapX = (int)((mouse.x - imgOrigin.x) / scale);
		int mapY = (int)((mouse.y - imgOrigin.y) / scale);

		// True wrapped offset over the full 256 range first -- THEN check range.
		// (Forcing a mod-160/144 before checking range was the bug: it wraps
		// out-of-range positions right back into range, silently.)
		const int offsetX = ((mapX - scx) % 256 + 256) % 256;
		const int offsetY = ((mapY - scy) % 256 + 256) % 256;

		if (offsetX < 160 && offsetY < 144)
		{
			viewportPixelSelected = YES;
			viewportSelectedMapX = mapX;
			viewportSelectedMapY = mapY;
			viewportSelectedPixelInfo = debugViewportPixelInfo[offsetY * 160 + offsetX];
		}
		else
		{
			viewportPixelSelected = NO;	// clicked outside the currently-rendered 160x144 area
		}
	}

	if (viewportPixelSelected == YES)
	{
		const float scale = side / 256.0f;
		ImVec2 rectMin(imgOrigin.x + viewportSelectedMapX * scale, imgOrigin.y + viewportSelectedMapY * scale);
		ImVec2 rectMax(rectMin.x + scale, rectMin.y + scale);
		ImGui::GetWindowDrawList()->AddRect(rectMin, rectMax, IM_COL32(255, 0, 255, 255), 0.0f, 0, 2.0f);
	}

	if (gbcDebugger.ppu.viewportShowViewportRect == YES)
	{
		const float scale = side / 256.0f;
		int xStarts[2] = { scx, 0 };
		int xLens[2] = { (scx + 160 <= 256) ? 160 : (256 - scx), (scx + 160 <= 256) ? 0 : ((scx + 160) - 256) };
		int yStarts[2] = { scy, 0 };
		int yLens[2] = { (scy + 144 <= 256) ? 144 : (256 - scy), (scy + 144 <= 256) ? 0 : ((scy + 144) - 256) };

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		for (int xi = 0; xi < 2; xi++)
		{
			if (xLens[xi] == 0) continue;
			for (int yi = 0; yi < 2; yi++)
			{
				if (yLens[yi] == 0) continue;
				ImVec2 rectMin(imgOrigin.x + xStarts[xi] * scale, imgOrigin.y + yStarts[yi] * scale);
				ImVec2 rectMax(rectMin.x + xLens[xi] * scale, rectMin.y + yLens[yi] * scale);
				drawList->AddRect(rectMin, rectMax, IM_COL32(218, 165, 32, 255), 0.0f, 0, 2.0f);
			}
		}
	}

	if (gbcDebugger.ppu.viewportShowGrid == YES)
	{
		const float scale = side / 256.0f;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImU32 gridColor = gbcDebugGridColor(gbcDebugger.ppu.gridColorWhite);
		for (int col = 0; col <= 32; col++)
			drawList->AddLine(ImVec2(imgOrigin.x + col * 8 * scale, imgOrigin.y), ImVec2(imgOrigin.x + col * 8 * scale, imgOrigin.y + side), gridColor);
		for (int row = 0; row <= 32; row++)
			drawList->AddLine(ImVec2(imgOrigin.x, imgOrigin.y + row * 8 * scale), ImVec2(imgOrigin.x + side, imgOrigin.y + row * 8 * scale), gridColor);
	}

	ImGui::SameLine();
	ImGui::BeginChild("PixelInfoPanel", ImVec2(0, 0), true);

	if (viewportPixelSelected == NO)
	{
		ImGui::TextDisabled("Click a viewport pixel to get its render info.");
	}
	else if (viewportSelectedPixelInfo.captured == NO)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "That pixel wasn't rendered this frame.");
		ImGui::Text("Map pos: %d, %d", viewportSelectedMapX, viewportSelectedMapY);
	}
	else
	{
		ImGui::Text("Map pos: %d, %d", viewportSelectedMapX, viewportSelectedMapY);
		if (ImGui::BeginTable("PixelInfoTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 30.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			auto row = [](const char* label, const char* fmt, auto value)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
					ImGui::TableNextColumn(); ImGui::Text(fmt, value);
				};
			row("LY", "%d", (int)viewportSelectedPixelInfo.LY);
			row("PX", "%d", (int)viewportSelectedPixelInfo.pixelRenderCounterPerScanLine);
			row("TL", "%d", (int)viewportSelectedPixelInfo.ppuCounterPerLY);
			row("TM", "%d", (int)viewportSelectedPixelInfo.ppuCounterPerMode);
			row("TF", "%u", viewportSelectedPixelInfo.ppuCounterPerFrame);
			row("FS", "%d", viewportSelectedPixelInfo.pixelFetcherState);
			ImGui::EndTable();
		}
	}

	ImGui::EndChild();
}

void GBc_t::renderGBCDebuggerOAMPanel()
{
	debugRebuildOAMSpritePixels();
	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugOAMSpriteTexture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 16 * 40, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)debugOAMSpritePixels.data()));

	const FLAG tallSprites = (pGBc_peripherals->LCDC.lcdControlFields.OBJ_SIZE == ONE);
	const int spriteScreenH = tallSprites ? 16 : 8;
	const FLAG isCGB = (ROM_TYPE == ROM::GAME_BOY_COLOR);

	bool onScreen[40];
	for (int s = 0; s < 40; s++)
	{
		const OAMEntry_t& oam = pGBc_memory->GBcMemoryMap.mOAM.OAMFields.OAM[s];
		const int x = (int)oam.xPosition - 8;
		const int y = (int)oam.yPosition - 16;
		onScreen[s] = (x + 8 > 0 && x < (int)getScreenWidth() && y + spriteScreenH > 0 && y < (int)getScreenHeight());
	}

	int viewSel = (gbcDebugger.ppu.oamUseGalleryView == YES) ? 1 : 0;
	ImGui::RadioButton("List", &viewSel, 0); ImGui::SameLine();
	ImGui::RadioButton("Gallery", &viewSel, 1);
	gbcDebugger.ppu.oamUseGalleryView = (viewSel == 1) ? YES : NO;

	ImGui::BeginChild("OAMLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.45f, 0), true);

	if (gbcDebugger.ppu.oamUseGalleryView == NO)
	{
		if (ImGui::BeginTable("OAMEntries", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
		{
			ImGui::TableSetupColumn("#");
			ImGui::TableSetupColumn("X");
			ImGui::TableSetupColumn("Y");
			ImGui::TableSetupColumn("Tile");
			ImGui::TableSetupColumn("Pal");
			ImGui::TableSetupColumn("Flags");
			ImGui::TableHeadersRow();

			for (int s = 0; s < 40; s++)
			{
				const OAMEntry_t& oam = pGBc_memory->GBcMemoryMap.mOAM.OAMFields.OAM[s];
				ImGui::TableNextRow();

				const FLAG isVisibleThisLY = (visibleOamIndexPerLY.count((BYTE)s) > 0) ? YES : NO;
				const FLAG isVisibleThisFrame = (visibleOamIndexPerFrame.count((BYTE)s) > 0) ? YES : NO;

				if (isVisibleThisLY == YES)
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(0, 220, 220, 200));
				else if (isVisibleThisFrame == YES)
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(40, 200, 40, 180));
				else if (onScreen[s])
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(255, 140, 0, 160));

				ImGui::TableNextColumn();
				bool isSelected = (gbcDebugger.ppu.selectedOAMEntry == s);
				if (ImGui::Selectable(std::to_string(s).c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
					gbcDebugger.ppu.selectedOAMEntry = s;
				ImGui::TableNextColumn(); ImGui::Text("%d", oam.xPosition - 8);
				ImGui::TableNextColumn(); ImGui::Text("%d", oam.yPosition - 16);
				ImGui::TableNextColumn(); ImGui::Text("0x%02X", oam.tileIndex);
				ImGui::TableNextColumn();
				if (isCGB) ImGui::Text("%d", oam.attributes.oamEntryFields.OAM_PALETTE_NUMBER_CGB);
				else ImGui::Text("%d", oam.attributes.oamEntryFields.OAM_PALETTE_NUMBER_DMG);
				ImGui::TableNextColumn();
				ImGui::Text("%s%s%s",
					oam.attributes.oamEntryFields.OAM_X_FLIP ? "X " : "",
					oam.attributes.oamEntryFields.OAM_Y_FLIP ? "Y " : "",
					oam.attributes.oamEntryFields.OAM_BG_WINDOW_OVER_OBJ ? "P" : "");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("X = flipped horizontally\nY = flipped vertically\nP = priority: BG/Window color 1-3 drawn OVER this sprite");
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

		for (int s = 0; s < 40; s++)
		{
			const OAMEntry_t& oam = pGBc_memory->GBcMemoryMap.mOAM.OAMFields.OAM[s];
			ImVec2 uv0(0.0f, (s * 16) / 640.0f);
			ImVec2 uv1(1.0f, (s * 16 + spriteScreenH) / 640.0f);

			ImGui::PushID(s);
			ImVec2 imgOrigin = ImGui::GetCursorScreenPos();
			ImGui::Image((ImTextureID)(uintptr_t)debugOAMSpriteTexture, ImVec2(thumbSize, thumbSize), uv0, uv1);
			if (ImGui::IsItemClicked())
				gbcDebugger.ppu.selectedOAMEntry = s;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Sprite #%d\nX:%d Y:%d Tile:0x%02X", s, oam.xPosition - 8, oam.yPosition - 16, oam.tileIndex);

			ImU32 borderColor = IM_COL32(60, 60, 60, 255);
			if (gbcDebugger.ppu.selectedOAMEntry == s) borderColor = IM_COL32(255, 0, 255, 255);
			else if (visibleOamIndexPerLY.count((BYTE)s) > 0) borderColor = IM_COL32(0, 240, 240, 255);
			else if (visibleOamIndexPerFrame.count((BYTE)s) > 0) borderColor = IM_COL32(40, 220, 40, 220);
			else if (onScreen[s]) borderColor = IM_COL32(255, 140, 0, 200);

			ImGui::GetWindowDrawList()->AddRect(imgOrigin, ImVec2(imgOrigin.x + thumbSize, imgOrigin.y + thumbSize), borderColor, 0.0f, 0, 2.0f);
			ImGui::PopID();

			if ((s + 1) % colCount != 0 && s != 39) ImGui::SameLine(0.0f, spacing);
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("OAMRight", ImVec2(0, 0), true);

	if (gbcDebugger.ppu.selectedOAMEntry >= 0)
	{
		const int s = gbcDebugger.ppu.selectedOAMEntry;
		const OAMEntry_t& oam = pGBc_memory->GBcMemoryMap.mOAM.OAMFields.OAM[s];

		ImVec2 uv0(0.0f, (s * 16) / 640.0f);
		ImVec2 uv1(1.0f, (s * 16 + spriteScreenH) / 640.0f);
		ImGui::Text("Sprite #%d", s);
		const float spriteImgWidth = 96.0f;	// fixed, sensible size -- scales with aspect, not with panel width (see note below)
		ImGui::Image((ImTextureID)(uintptr_t)debugOAMSpriteTexture, ImVec2(spriteImgWidth, spriteImgWidth * ((float)spriteScreenH / 8.0f)), uv0, uv1);

		ImGui::SameLine();
		ImGui::BeginGroup();
		if (ImGui::BeginTable("OAMDetailTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 90.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			auto row = [](const char* label, const char* fmt, auto value)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
					ImGui::TableNextColumn(); ImGui::Text(fmt, value);
				};
			row("X", "%d", oam.xPosition - 8);
			row("Y", "%d", oam.yPosition - 16);
			row("Tile", "0x%02X", oam.tileIndex);
			row("Attributes", "0x%02X", oam.attributes.oamEntryByte);
			row("X Flip", "%d", (int)oam.attributes.oamEntryFields.OAM_X_FLIP);
			row("Y Flip", "%d", (int)oam.attributes.oamEntryFields.OAM_Y_FLIP);
			row("Priority", "%s", oam.attributes.oamEntryFields.OAM_BG_WINDOW_OVER_OBJ ? "BG/Win over OBJ" : "OBJ on top");
			if (isCGB)
			{
				row("CGB Palette", "%d", (int)oam.attributes.oamEntryFields.OAM_PALETTE_NUMBER_CGB);
				row("VRAM Bank", "%d", (int)oam.attributes.oamEntryFields.OAM_TILE_VRAM_BANK);
			}
			else
			{
				row("DMG Palette", "%s", oam.attributes.oamEntryFields.OAM_PALETTE_NUMBER_DMG ? "OBP1" : "OBP0");
			}
			ImGui::EndTable();
		}

		ImGui::Text("Palette used by this sprite:");
		if (isCGB)
		{
			const int pal = oam.attributes.oamEntryFields.OAM_PALETTE_NUMBER_CGB;
			for (int c = 0; c < 4; c++)
			{
				Pixel col = getColorFromColorIDForGBC(pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM[pal][c].gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
				ImVec4 imc = ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);
				ImGui::PushID(c);
				ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(24, 24));
				ImGui::PopID();
				ImGui::SameLine(0.0f, (c == 0) ? 0.0f : 4.0f);
			}
		}
		else
		{
			const BYTE obp = oam.attributes.oamEntryFields.OAM_PALETTE_NUMBER_DMG ? pGBc_peripherals->OBP1 : pGBc_peripherals->OBP0;
			for (int c = 0; c < 4; c++)
			{
				Pixel col = getColorFromColorIDForGB(obp, c).COLOR;
				ImVec4 imc = ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);
				ImGui::PushID(c);
				ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(24, 24));
				ImGui::PopID();
				ImGui::SameLine(0.0f, (c == 0) ? 0.0f : 4.0f);
			}
		}
		ImGui::EndGroup();
		ImGui::Separator();
	}

	//ImGui::Text("orange = positioned on-screen   green = frame   cyan = this LY   magenta = selected");

	GL_CALL(glBindTexture(GL_TEXTURE_2D, debugMiniScreenTexture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGBc_display->imGuiBuffer.imGuiBuffer1D));

	ImVec2 miniAvail = ImGui::GetContentRegionAvail();
	const float miniAspect = (float)getScreenWidth() / (float)getScreenHeight();
	ImVec2 miniSize = (miniAvail.x / miniAspect <= miniAvail.y) ? ImVec2(miniAvail.x, miniAvail.x / miniAspect) : ImVec2(miniAvail.y * miniAspect, miniAvail.y);
	const float miniScale = miniSize.x / (float)getScreenWidth();
	ImVec2 miniOrigin = ImGui::GetCursorScreenPos();
	ImGui::Image((ImTextureID)(uintptr_t)debugMiniScreenTexture, miniSize);

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	auto drawSpriteBox = [&](int s, ImU32 color, float thickness)
		{
			const OAMEntry_t& oam = pGBc_memory->GBcMemoryMap.mOAM.OAMFields.OAM[s];
			const int x = (int)oam.xPosition - 8;
			const int y = (int)oam.yPosition - 16;
			ImVec2 rectMin(miniOrigin.x + x * miniScale, miniOrigin.y + y * miniScale);
			ImVec2 rectMax(rectMin.x + 8 * miniScale, rectMin.y + spriteScreenH * miniScale);
			drawList->AddRect(rectMin, rectMax, color, 0.0f, 0, thickness);
		};

	for (int s = 0; s < 40; s++)
		if (onScreen[s] && s != gbcDebugger.ppu.selectedOAMEntry) drawSpriteBox(s, IM_COL32(255, 140, 0, 200), 1.5f);

	for (BYTE s : visibleOamIndexPerFrame)
		if (s != gbcDebugger.ppu.selectedOAMEntry) drawSpriteBox(s, IM_COL32(40, 220, 40, 220), 2.0f);

	for (BYTE s : visibleOamIndexPerLY)
		if (s != gbcDebugger.ppu.selectedOAMEntry) drawSpriteBox(s, IM_COL32(0, 240, 240, 255), 2.0f);

	if (gbcDebugger.ppu.selectedOAMEntry >= 0)
	{
		const int s = gbcDebugger.ppu.selectedOAMEntry;
		if (onScreen[s])
			drawSpriteBox(s, IM_COL32(255, 0, 255, 255), 3.0f);
		else
			ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Selected sprite is positioned off-screen right now.");
	}

	ImGui::EndChild();
}

void GBc_t::renderGBCDebuggerPalettePanel()
{
	if (ROM_TYPE != ROM::GAME_BOY_COLOR)
	{
		ImGui::SeparatorText("DMG Palettes");

		auto drawDmgRow = [&](const char* label, BYTE paletteRegisterValue, int idBase)
			{
				ImGui::TextUnformatted(label);
				for (int c = 0; c < 4; c++)
				{
					ImGui::SameLine(60.0f + c * 36.0f);	// absolute offset -- immune to label width
					ImGui::PushID(idBase + c);
					Pixel col = getColorFromColorIDForGB(paletteRegisterValue, c).COLOR;
					ImVec4 imc = ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);
					ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(32, 32));
					ImGui::PopID();
				}
			};

		drawDmgRow("BG0 ", pGBc_peripherals->BGP, 0);
		drawDmgRow("OBJ0", pGBc_peripherals->OBP0, 10);
		drawDmgRow("OBJ1", pGBc_peripherals->OBP1, 20);

		RETURN;
	}

	ImGui::Columns(2, "GBCPaletteColumns", false);

	ImGui::SeparatorText("Background");
	for (int p = 0; p < 8; p++)
	{
		ImGui::Text("BG%d", p);
		for (int c = 0; c < 4; c++)
		{
			ImGui::SameLine(45.0f + c * 24.0f);
			ImGui::PushID(p * 10 + c);
			Pixel col = getColorFromColorIDForGBC(pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM[p][c].gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
			ImVec4 imc = ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);
			ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
			ImGui::PopID();
		}
	}

	ImGui::NextColumn();

	ImGui::SeparatorText("Object");
	for (int p = 0; p < 8; p++)
	{
		ImGui::Text("OBJ%d", p);
		for (int c = 0; c < 4; c++)
		{
			ImGui::SameLine();
			ImGui::PushID(100 + p * 10 + c);
			Pixel col = getColorFromColorIDForGBC(pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM[p][c].gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
			ImVec4 imc = ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, 1.0f);
			ImGui::ColorButton("##swatch", imc, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
			ImGui::PopID();
		}
	}

	ImGui::Columns(1);
}

void GBc_t::renderGBCDebuggerPPUTab()
{
	// ---- master switch + screen-refresh sample mode --------------------------------
	ImGui::Checkbox("Enable PPU debug instrumentation", &gbcDebugger.ppu.enabled);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Master switch. When off, this behaves exactly like a normal (non-debug) run.");

	ImGui::BeginDisabled(gbcDebugger.ppu.enabled == NO);

	ImGui::TextDisabled("Screen refresh:"); ImGui::SameLine();
	int sampleMode = (int)gbcDebugger.ppu.pixelOutputSampleMode;
	ImGui::RadioButton("Per Frame", &sampleMode, (int)GBC_DEBUG_PIXEL_SAMPLE_MODE::PER_FRAME); ImGui::SameLine();
	ImGui::RadioButton("Per LY", &sampleMode, (int)GBC_DEBUG_PIXEL_SAMPLE_MODE::PER_LY); ImGui::SameLine();
	ImGui::RadioButton("Per Dot", &sampleMode, (int)GBC_DEBUG_PIXEL_SAMPLE_MODE::PER_DOT);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Refreshes the screen texture every T-cycle. Very slow -- best used only once you're already close to the dot you care about (see Run-to-breakpoint below).");
	gbcDebugger.ppu.pixelOutputSampleMode = (GBC_DEBUG_PIXEL_SAMPLE_MODE)sampleMode;

	ImGui::SameLine(); ImGui::Text("  Grid:"); ImGui::SameLine();
	int gridColSel = (gbcDebugger.ppu.gridColorWhite == YES) ? 0 : 1;
	ImGui::RadioButton("White", &gridColSel, 0); ImGui::SameLine();
	ImGui::RadioButton("Black", &gridColSel, 1);
	gbcDebugger.ppu.gridColorWhite = (gridColSel == 0) ? YES : NO;

	ImGui::Separator();

	// ---- run-to-breakpoint / step controls ------------------------------------------
	int breakLYi = gbcDebugger.ppu.breakpointLY;
	int breakDoti = gbcDebugger.ppu.breakpointDot;
	ImGui::Text("Run to  LY:"); ImGui::SameLine(); ImGui::SetNextItemWidth(90.0f);
	ImGui::InputInt("##breakLY", &breakLYi, 1, 10);
	ImGui::SameLine(); ImGui::Text("Dot:"); ImGui::SameLine(); ImGui::SetNextItemWidth(110.0f);
	ImGui::InputInt("##breakDot", &breakDoti, 1, 10);
	breakLYi = (breakLYi < 0) ? 0 : (breakLYi > 153 ? 153 : breakLYi);
	breakDoti = (breakDoti < 0) ? 0 : (breakDoti > 456 ? 456 : breakDoti);
	gbcDebugger.ppu.breakpointLY = (uint8_t)breakLYi;
	gbcDebugger.ppu.breakpointDot = (uint16_t)breakDoti;

	ImGui::SameLine();
	if (ImGui::Button("Run to breakpoint"))
	{
		gbcDebugger.ppu.runToBreakpointArmed = YES;
		gbcDebugger.ppu.paused = NO;
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Runs at full, undecorated speed (no per-dot texture cost) until LY/dot match, then freezes.");

	ImGui::SameLine();
	if (gbcDebugger.ppu.paused == YES)
	{
		if (ImGui::Button("Step")) gbcDebugger.ppu.stepRequested = YES;
		ImGui::SameLine();
		if (ImGui::Button("Resume"))
		{
			gbcDebugger.ppu.paused = NO; gbcDebugger.ppu.runToBreakpointArmed = NO;
		}
	}
	else
	{
		ImGui::TextDisabled("Step / Resume (available once paused)");
	}

	ImGui::Text("Current -- LY: %3d   Dot: %3d / 456   %s",
		pGBc_peripherals->LY,
		(int)pGBc_display->pixelRenderCounterPerScanLine,
		gbcDebugger.ppu.paused == YES ? "[PAUSED]" : (gbcDebugger.ppu.runToBreakpointArmed == YES ? "[RUNNING TO BREAKPOINT]" : "[RUNNING]"));

	ImGui::EndDisabled();

	ImGui::Separator();

	// ---- docked panels: all visible simultaneously, like the Gearboy reference ------
	ImGuiID dockspaceID = ImGui::GetID("GBCPPUDockSpace");

	if (gbcDebugger.ppu.dockLayoutBuilt == NO && ImGui::DockBuilderGetNode(dockspaceID) == nullptr)
	{
		// Use a fixed nominal size instead of GetContentRegionAvail() -- the latter can be
		// (0,0) for more than one frame while the window settles, and if even a single frame
		// slips through before DockBuilderDockWindow runs, that panel's Begin() call already
		// floats it and it never re-docks on its own. A fixed size avoids that race entirely;
		// the dockspace itself auto-resizes to the real window on the next line regardless.
		ImGui::DockBuilderRemoveNode(dockspaceID);
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, ImVec2(1000.0f, 600.0f));

		ImGuiID dockCenter = dockspaceID;
		ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Left, 0.25f, nullptr, &dockCenter);
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Right, 0.33f, nullptr, &dockCenter);
		ImGuiID dockLeftBottom = ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Down, 0.5f, nullptr, &dockLeft);
		ImGuiID dockRightBottom = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.4f, nullptr, &dockRight);
		ImGuiID dockCenterMid = ImGui::DockBuilderSplitNode(dockCenter, ImGuiDir_Down, 0.66f, nullptr, &dockCenter);
		ImGuiID dockCenterBottom = ImGui::DockBuilderSplitNode(dockCenterMid, ImGuiDir_Down, 0.5f, nullptr, &dockCenterMid);

		// seven distinct panes, nothing tab-grouped -- everything visible at once
		ImGui::DockBuilderDockWindow("Registers##GBCDebug", dockLeft);
		ImGui::DockBuilderDockWindow("OAM / Sprites##GBCDebug", dockLeftBottom);
		ImGui::DockBuilderDockWindow("Complete Viewport##GBCDebug", dockCenter);
		ImGui::DockBuilderDockWindow("BG Map##GBCDebug", dockCenterMid);
		ImGui::DockBuilderDockWindow("Window Map##GBCDebug", dockCenterBottom);
		ImGui::DockBuilderDockWindow("Tiles##GBCDebug", dockRight);
		ImGui::DockBuilderDockWindow("Palettes##GBCDebug", dockRightBottom);

		ImGui::DockBuilderFinish(dockspaceID);
		gbcDebugger.ppu.dockLayoutBuilt = YES;
	}

	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	if (gbcDebugger.ppu.showRegisters)
	{
		ImGui::Begin("Registers##GBCDebug", &gbcDebugger.ppu.showRegisters);
		renderGBCDebuggerRegistersPanel();
		ImGui::End();
	}
	if (gbcDebugger.ppu.showTileViewer)
	{
		ImGui::Begin("Tiles##GBCDebug", &gbcDebugger.ppu.showTileViewer);
		renderGBCDebuggerTileViewerPanel();
		ImGui::End();
	}
	ImGui::Begin("Complete Viewport##GBCDebug");
	renderGBCDebuggerViewportPanel();
	ImGui::End();

	if (gbcDebugger.ppu.showBGMapViewer)
	{
		ImGui::Begin("BG Map##GBCDebug", &gbcDebugger.ppu.showBGMapViewer);
		renderGBCDebuggerBGMapPanel();
		ImGui::End();
	}
	if (gbcDebugger.ppu.showWindowMapViewer)
	{
		ImGui::Begin("Window Map##GBCDebug", &gbcDebugger.ppu.showWindowMapViewer);
		renderGBCDebuggerWindowMapPanel();
		ImGui::End();
	}
	if (gbcDebugger.ppu.showOAMViewer)
	{
		ImGui::Begin("OAM / Sprites##GBCDebug", &gbcDebugger.ppu.showOAMViewer);
		renderGBCDebuggerOAMPanel();
		ImGui::End();
	}
	if (gbcDebugger.ppu.showPaletteViewer)
	{
		ImGui::Begin("Palettes##GBCDebug", &gbcDebugger.ppu.showPaletteViewer);
		renderGBCDebuggerPalettePanel();
		ImGui::End();
	}
}

void GBc_t::renderGBCDebuggerUI()
{
	if (gbcDebugger.windowOpen == NO) RETURN;

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
	if (gbcDebugger.ppu.fullscreen == YES)
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

	ImGui::Begin("GB-GBC PPU Debugger", &gbcDebugger.windowOpen, winFlags);

	if (ImGui::BeginMenuBar())
	{
		bool isFullscreen = (gbcDebugger.ppu.fullscreen == YES);
		if (ImGui::MenuItem(isFullscreen ? "Exit Fullscreen" : "Fullscreen"))
			gbcDebugger.ppu.fullscreen = isFullscreen ? NO : YES;

		ImGui::Separator();

		switch (activeTab)
		{
		case DEBUGGER_TAB::PPU:
		{
			if (ImGui::BeginMenu("Panels"))
			{
				ImGui::MenuItem("Registers", NULL, (bool*)&gbcDebugger.ppu.showRegisters);
				ImGui::MenuItem("Tiles", NULL, (bool*)&gbcDebugger.ppu.showTileViewer);
				ImGui::MenuItem("BG Map", NULL, (bool*)&gbcDebugger.ppu.showBGMapViewer);
				ImGui::MenuItem("Window Map", NULL, (bool*)&gbcDebugger.ppu.showWindowMapViewer);
				ImGui::MenuItem("OAM / Sprites", NULL, (bool*)&gbcDebugger.ppu.showOAMViewer);
				ImGui::MenuItem("Palettes", NULL, (bool*)&gbcDebugger.ppu.showPaletteViewer);
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

	if (ImGui::BeginTabBar("GBCDebuggerTabs"))
	{
		if (ImGui::BeginTabItem("PPU"))
		{
			activeTab = DEBUGGER_TAB::PPU;
			renderGBCDebuggerPPUTab();
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
			renderGBCDebuggerEventViewerTab();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

#endif // !__RPI_PICO__