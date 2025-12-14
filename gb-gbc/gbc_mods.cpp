#include "gbc.h"

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
				BYTE ramByte = pGBc_instance->GBc_state.entireVram.vramMemoryBanks.mVRAMBanks[ii][jj];
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

	if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.no_mbc == NO)
	{
		uint32_t mbc_size = RESET;

		if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc1 == YES)
		{
			mbc_size = 12;
		}
		else if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc2 == YES)
		{
			mbc_size = 6;
		}
		else if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc3 == YES)
		{
			mbc_size = 15;
		}
		else if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc5 == YES)
		{
			mbc_size = 12;
		}

		// allocate struct + extra space for mbc[]
		BESS_BLOCK_MBC_t* BESS_BLOCK_MBC = (BESS_BLOCK_MBC_t*)std::malloc(sizeof(BESS_BLOCK_MBC_t) + mbc_size);

		// Copy the name of block
		std::memcpy(BESS_BLOCK_MBC->BESS_BLOCK_HEADER.ascii_ident, "MBC ", 4);

		if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc1 == YES)
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
			//BESS_BLOCK_MBC->mbc[0x08] = pGBc_instance->GBc_state.emulatorStatus.currentROMBankNumber.mbc1Fields.mbcBank1Reg;
			mbc_reg = 0x6000;
			BESS_BLOCK_MBC->mbc[0x03] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x04] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x05] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg3;
			//BESS_BLOCK_MBC->mbc[0x05] = ((pGBc_instance->GBc_state.emulatorStatus.isMBC1_Mode1 == true) ? 0x01 : 0x00);
			mbc_reg = 0x4000;
			BESS_BLOCK_MBC->mbc[0x09] = mbc_reg & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0A] = (mbc_reg >> EIGHT) & 0xFF;
			BESS_BLOCK_MBC->mbc[0x0B] = pGBc_instance->GBc_state.emulatorStatus.dataWrittenToMBCReg2;
			//BESS_BLOCK_MBC->mbc[0x0B] = pGBc_instance->GBc_state.emulatorStatus.currentROMBankNumber.mbc1Fields.mbcBank2Reg;
		}
		else if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc2 == YES)
		{
			if (pGBc_instance->GBc_state.emulatorStatus.is_mbc2_rom_mode == YES)
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
		else if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc3 == YES)
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
		else if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc5 == YES)
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

	if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc3 == YES)
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

			if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.no_mbc == NO)
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

			if (pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus.mbc3 == YES)
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