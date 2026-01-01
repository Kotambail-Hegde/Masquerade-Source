#pragma region GBA_SPECIFIC_INCLUDES
#include "gba.h"
#pragma endregion GBA_SPECIFIC_INCLUDES

#pragma region INFORMATION
/*
* INFORMATION_001: Regarding Unaligned 16bit and 32bit accesses
* Refer : http://problemkaputt.de/gbatek-arm-cpu-memory-alignments.htm
* Reason:
* During memory access, bus always aligns a 32 bit read/write address to 4 byte and 16 bit read/write address to 2 byte
* As a consequence of this, the data appears to be rotated
* Further explaination:
* The processor fetches the 32-bit word containing the address (aligned to the nearest lower multiple of 4)
* and then rotates the bytes to simulate the requested unaligned access.
* The rotation aligns the requested data to the least significant byte (LSB) of the register.
* For instance, loading a 32-bit word from address 0x01 might involve fetching the word starting at 0x00 and then rotating the result so that the data from 0x01 aligns correctly.
* Example:
* Address:   0x00  0x01  0x02  0x03
* Data:      0xAA  0xBB  0xCC  0xDD
* If you attempt to load a 32-bit word starting at 0x01:
* The ARM7TDMI fetches the aligned word starting at 0x00 (0xAABBCCDD).
* 1) It rotates the word to align the data from 0x01:
* 2) The result is 0xBBCCDDAA.
* Note that ROR performed here doesn't have the "ARM's quirk" as this is not an ROR opcode operation but the rotation peformed by the bus itself
* In case of signed 16 bit/32 bit data
* We still have to perform ROR, but on top of that, further sign extention is needed as signed bit gets shifted as well
* If you try to load a signed 16-bit halfword from address 0x01:
* 1) The processor fetches the 32-bit aligned word starting at 0x00 (0xFF7F0001).
* 2) It rotates the word to align the halfword at 0x01 to the LSB:- The rotated value is 0x7FFF (byte order adjusted).
* 3) The processor performs sign extension:- Since the most significant bit of 0x7FFF is 0 (indicating a positive number), the value is extended as 0x00007FFF.
* Key Points:
* 1) Rotation Happens Regardless of Sign
* 2) Sign Extension Comes After Rotation
*
* INFORMATION_002: Regarding the value of PC to be stored in LR before entering ISR
* As per
* https://discord.com/channels/465585922579103744/465586361731121162/884368553514516480
* https://discord.com/channels/465585922579103744/465586361731121162/950864979375050802
* Before returing from ISR within the BIOS, LR is subtracted by 4 and then loaded to PC (subs PC, LR, #4)
* If this is the case, then we will need to add 4 on top of the value of PC that we would have stored in LR just before entering the ISR
* Currently in our emulator, at the instance of jumping to ISR
* The instruction that we would have executed if not for this interrupt is in "DECODE" and this has not been moved to "EXECUTE" yet
* So, "EXECUTE" still has 'already executed opcode' instead of what we wanted to execute and 'what we wanted to execute' is still in "DECODE"
* And, note that because of the instruction that was just executed, PC is already incremented, i.e. actually points to current "FETCH" + 4 (or 2 in case of Thumb)
* And since we want to fill LR with 'what we wanted to execute' which is in "DECODE", the address of decode now is PC - 8 (or PC - 4 in Thumb)
* But since ISR in BIOS performs "subs PC, LR, #4", we need to add 4 to the address of decode and then store to LR
* Therefore, for ARM: PC - 8 + 4 => PC - 4 in LR and for Thumb: PC - 4 + 4 => PC in LR
*/
#pragma endregion INFORMATION

#pragma region GBA_SPECIFIC_DECLARATIONS
// GBA File Logger (https://github.com/skylersaleh/GBA-Logs)
FLAG skylersalehLogs = DISABLED;
std::vector<uint32_t> skylersalehLogs_BUFFER;

// For Configuration (config.ini) settings
static FLAG _DISABLE_BG = NO;
static FLAG _DISABLE_WIN = NO;
static FLAG _DISABLE_OBJ = NO;
static FLAG _LOAD_GBA_BIOS = NO;
static FLAG _ENABLE_GBA_BIOS = NO;
static FLAG _LOAD_BUT_DONT_EXECUTE_GBA_BIOS = NO;
static std::string _JSON_LOCATION;

// For debug
static COUNTER64 logCounter = ZERO;
static COUNTER64 emulationCounter[100] = { ZERO };

// For DMA
// TODO: This needs to be non-zero as per spec
constexpr uint16_t DMA_START_DELAY = 0;

// For audio
// maximum number of inputs
static uint32_t const MAX_INPUT_LEN = (uint32_t)(EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA / CEIL(GBA_FPS));
// length of filter than can be handled
static uint32_t const MAX_FLT_LEN = 103;
// buffer to hold all of the input samples
static uint32_t const BUFFER_LEN = (MAX_FLT_LEN - 1 + MAX_INPUT_LEN);
// coefficients for the FIR from https://www.arc.id.au/FilterDesign.html
// 
// double type buffers to hold input and output during FIR
static double doubleInput[(uint32_t)(EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA / CEIL(GBA_FPS))];
static double doubleOutput[(uint32_t)(EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA / CEIL(GBA_FPS))];

// For video
static uint32_t gameboyAdvance_texture;
static uint32_t matrix_texture;
static uint32_t matrix[16] = { 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
#pragma endregion GBA_SPECIFIC_DECLARATIONS

#pragma region INFRASTRUCTURE_DEFINITIONS
GBA_t::GBA_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config)
{
	// set log level
#if _DEBUG
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUWARN);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUWARN);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUWARN);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUTODO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUTODO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUTODO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUEVENT);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUEVENT);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUEVENT);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUMOREINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUMOREINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUMOREINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_DISASSEMBLY);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUINFRA);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUINFRA);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUINFRA);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUDEBUG);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUDEBUG);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUDEBUG);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_WARN);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_TODO);
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_EVENT);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_MOREINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFRA);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_DEBUG);
#else
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUWARN);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUWARN);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUWARN);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUTODO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUTODO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUTODO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUEVENT);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUEVENT);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUEVENT);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUMOREINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUMOREINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUMOREINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_DISASSEMBLY);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUINFRA);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUINFRA);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUINFRA);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUDEBUG);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_APUDEBUG);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_PPUDEBUG);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_WARN);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_TODO);
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_EVENT);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_MOREINFO);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFRA);
	//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_DEBUG);
#endif

	isBiosEnabled = NO;

	setEmulationID(EMULATION_ID::GBA_ID);

	std::transform(rom[ZERO].begin(), rom[ZERO].end(), rom[ZERO].begin(), ::tolower);

	if (nFiles == SST_ROMS)
	{
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_WARN);
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_EVENT);

		INFO("Running in sst Cpu Test Mode!");
		_JSON_LOCATION = rom[ONE];

		ROM_TYPE = ROM::TEST_SST;
	}
	else if (nFiles == REPLAY_ROM_FILE)
	{
		if ((toUpper(rom[ZERO]) == "-R") || (toUpper(rom[ZERO]) == "-C"))
		{
			std::transform(rom[ONE].begin(), rom[ONE].end(), rom[ONE].begin(), ::tolower);
			std::transform(rom[TWO].begin(), rom[TWO].end(), rom[TWO].begin(), ::tolower);
			if ((rom[ONE].substr(rom[ONE].find_last_of(".") + ONE) == "gba") && (rom[TWO].substr(rom[TWO].find_last_of(".") + ONE) == "bin"))
			{
				if (toUpper(rom[ZERO]) == "-R")
				{
					ROM_TYPE = ROM::REPLAY;
				}
				else
				{
					ROM_TYPE = ROM::COMPARE;
				}
			}
			else
			{
				FATAL("GBA replay mode supports files of specific format only");
			}
		}
	}
	else if(rom[ZERO].substr(rom[ZERO].find_last_of(".") + ONE) == "gba")
	{
		ROM_TYPE = ROM::GAME_BOY_ADVANCE;
	}
	else
	{
		ROM_TYPE = ROM::NO_ROM;
	}

	this->pt = config;

#ifndef __EMSCRIPTEN__
	_SAVE_LOCATION = pt.get<std::string>("gba._save_location");
#else
	_SAVE_LOCATION = "assets/saves";
#endif

	// check if directory mentioned by "_SAVE_LOCATION" exists, if not we need to explicitly create it
	ifNoDirectoryThenCreate(_SAVE_LOCATION);

	_LOAD_GBA_BIOS = to_bool(config.get<std::string>("gba._load_gba_bios"));
	_ENABLE_GBA_BIOS = to_bool(config.get<std::string>("gba._use_gba_bios"));

	_LOAD_GBA_BIOS = to_bool(config.get<std::string>("gba._load_gba_bios"));
	_ENABLE_GBA_BIOS = to_bool(config.get<std::string>("gba._use_gba_bios"));

	if (_LOAD_GBA_BIOS == YES && _ENABLE_GBA_BIOS == NO)
	{
		_LOAD_BUT_DONT_EXECUTE_GBA_BIOS = YES;
		_ENABLE_GBA_BIOS = YES;
	}

	FLAG searchForBios = NO;

	if (_ENABLE_GBA_BIOS == YES)
	{
#ifndef __EMSCRIPTEN__
		_BIOS_LOCATION = config.get<std::string>("gba._gba_bios_location");
#else
		_BIOS_LOCATION = "assets/gba/bios/gba_bios.bin";
#endif

		LOG("Searching for BIOS in %s", _BIOS_LOCATION.c_str());
		gba_bios.expectedBiosSize = 0x4000;
		searchForBios = YES;
	}
	else
	{
		LOG("By-passing BIOS");
		gba_bios.biosFound = NO;
	}

	if (searchForBios == YES)
	{
		LOG("Expected Bios size %d", gba_bios.expectedBiosSize);

		// Get the list of files in bios directory
		gba_bios.biosFound = NO;
		uint32_t sizeOfBios = ZERO;
		std::string maybeBiosFile;

#if DEACTIVATED
		for (const auto& entry : std::filesystem::directory_iterator(_BIOS_LOCATION))
		{
			maybeBiosFile = entry.path().string();
			std::transform(maybeBiosFile.begin(), maybeBiosFile.end(), maybeBiosFile.begin(), ::tolower);

			if (maybeBiosFile.substr(maybeBiosFile.find_last_of(".") + 1) == "bin")
			{
				maybeBiosFile = entry.path().string();
				gba_bios.biosFound = YES;
				BREAK;
			}
			gba_bios.biosFound = NO;
		}
#else
		gba_bios.biosFound = YES;
		maybeBiosFile = _BIOS_LOCATION;
#endif

		if (gba_bios.biosFound == YES)
		{
			FILE* fp = NULL;
			std::cout << maybeBiosFile << std::endl;
			errno_t err = fopen_portable(&fp, maybeBiosFile.c_str(), "rb");
			if (!err && (fp != NULL))
			{
				fseek(fp, 0, SEEK_END);
				sizeOfBios = ftell(fp);

				if (sizeOfBios == gba_bios.expectedBiosSize)
				{
					rewind(fp);
					fread(gba_bios.biosImage + 0x0000, sizeOfBios, 1, fp);

#if ZERO
					uint32_t scanner = 0;
					uint32_t addressField = 0x10;
					LOG("BIOS DUMP");
					LOG("Address\t\t");
					for (int ii = 0; ii < 0x10; ii++)
					{
						LOG("%02x\t", ii);
					}
					LOG_NEW_LINE;
					LOG("00000000\t");
					for (int ii = 0; ii < (int)sizeOfBios; ii++)
					{
						LOG("0x%02x\t", bios.bios[0x0000 + ii]);
						if (++scanner == 0x10)
						{
							scanner = 0;
							LOG_NEW_LINE;
							LOG("%08x\t", addressField);
							addressField += 0x10;
						}
					}
					LOG_NEW_LINE;
#endif
				}
				else
				{
					gba_bios.biosFound = NO;
				}
			}
		}

		if (gba_bios.biosFound == YES)
		{
			LOG("Using the above mentioned bios");
		}
	}

	if (ROM_TYPE == ROM::GAME_BOY_ADVANCE)
	{
		this->rom[ZERO] = rom[ZERO];
	}
	else if ((ROM_TYPE == ROM::REPLAY) || (ROM_TYPE == ROM::COMPARE))
	{
		this->rom[ZERO] = rom[ONE];
		this->rom[ONE] = rom[TWO];
	}

	// Some Additional Information...

	if (!_ENABLE_REWIND)
	{
		_REWIND_BUFFER_SIZE = RESET;
		LOG("Rewind : Disabled");
	}
	else if (_REWIND_BUFFER_SIZE <= RESET && _ENABLE_REWIND == YES)
	{
		_ENABLE_REWIND = NO;
		LOG("Rewind : Disabled");
	}
	else
	{
		LOG("Rewind : Enabled");
	}

	LOG_NEW_LINE;
}

GBA_t::~GBA_t()
{
	; // Do nothing for now!
}

void GBA_t::setupTheCoreOfEmulation(void* masqueradeInstance, void* audio, void* network)
{
	uint8_t indexToCheck = 0;

	if (!rom[indexToCheck].empty())
	{
		if (!initializeEmulator())
		{
			LOG("memory allocation failure");
			throw std::runtime_error("memory allocation failure");
		}

		loadRom(rom);

		// initialize the graphics

		initializeGraphics();

		// initialize the audio

		initializeAudio();
	}
	else
	{
		LOG("un-supported rom");
		throw std::runtime_error("un-supported rom");
	}
}

void GBA_t::setupTheAlternativeSoundOfEmulation(void* audio)
{
	;
}

uint32_t GBA_t::getScreenWidth()
{
	RETURN this->screen_width;
}

uint32_t GBA_t::getScreenHeight()
{
	RETURN this->screen_height;
}

uint32_t GBA_t::getPixelWidth()
{
	RETURN this->pixel_width;
}

uint32_t GBA_t::getPixelHeight()
{
	RETURN this->pixel_height;
}

void GBA_t::setEmulationWindowOffsets(uint32_t x, uint32_t y, FLAG isEnabled)
{
	if (isEnabled == NO)
	{
		this->x_offset = RESET;
		this->y_offset = RESET;
		RETURN;
	}

	this->x_offset = x;
	this->y_offset = y;
}

uint32_t GBA_t::getTotalScreenWidth()
{
	if (debugConfig._DEBUG_PPU_VIEWER_GUI == ENABLED)
	{
		RETURN this->debugger_screen_width;
	}
	else
	{
		RETURN this->total_screen_width;
	}
}

uint32_t GBA_t::getTotalScreenHeight()
{
	if (debugConfig._DEBUG_PPU_VIEWER_GUI == ENABLED)
	{
		RETURN this->debugger_screen_height;
	}
	else
	{
		RETURN this->total_screen_height;
	}
}

uint32_t GBA_t::getTotalPixelWidth()
{
	if (debugConfig._DEBUG_PPU_VIEWER_GUI == ENABLED)
	{
		RETURN this->debugger_pixel_width;
	}
	else
	{
		RETURN this->pixel_width;
	}
}

uint32_t GBA_t::getTotalPixelHeight()
{
	if (debugConfig._DEBUG_PPU_VIEWER_GUI == ENABLED)
	{
		RETURN this->debugger_pixel_height;
	}
	else
	{
		RETURN this->pixel_height;
	}
}

void GBA_t::setEmulationID(EMULATION_ID ID)
{
	myID = ID;
}

EMULATION_ID GBA_t::getEmulationID()
{
	RETURN myID;
}

const char* GBA_t::getEmulatorName()
{
	RETURN this->NAME;
}

float GBA_t::getEmulationFPS()
{
	RETURN this->myFPS;
}
#pragma endregion INFRASTRUCTURE_DEFINITIONS

#pragma region ARM7TDMI_DEFINITIONS
void GBA_t::cpuSetRegister(REGISTER_BANK_TYPE rb, REGISTER_TYPE rt, STATE_TYPE st, uint32_t u32parameter)
{
	uint8_t registerType = ((uint8_t)rt);
	uint8_t registerBank = ((uint8_t)rb);
	if (registerType < (LO_GP_REGISTERS)) // lower 8 registers are not banked
	{
		pGBA_registers->unbankedLORegisters[registerType] = u32parameter;
		RETURN;
	}
	else if (registerType < (LO_GP_REGISTERS + HI_GP_REGISTERS))
	{
		if ((registerType == SP) || (registerType == LR))
		{
			pGBA_registers->bankedHIRegisters[registerBank][(registerType - LO_GP_REGISTERS)] = u32parameter;
			RETURN;
		}
		else if (registerType == PC) // PC is not banked
		{
			loadPipeline(u32parameter);
			RETURN;
		}
		else // R8 to R12
		{
			if (rb == REGISTER_BANK_TYPE::RB_FIQ) // banked if FIQ mode
			{
				pGBA_registers->bankedHIRegisters[registerBank][(registerType - LO_GP_REGISTERS)] = u32parameter;
			}
			else // not banked if not FIQ mode
			{
				pGBA_registers->bankedHIRegisters[ZERO][(registerType - LO_GP_REGISTERS)] = u32parameter;
				pGBA_registers->bankedHIRegisters[TWO][(registerType - LO_GP_REGISTERS)] = u32parameter;
				pGBA_registers->bankedHIRegisters[THREE][(registerType - LO_GP_REGISTERS)] = u32parameter;
				pGBA_registers->bankedHIRegisters[FOUR][(registerType - LO_GP_REGISTERS)] = u32parameter;
				pGBA_registers->bankedHIRegisters[FIVE][(registerType - LO_GP_REGISTERS)] = u32parameter;
			}
			RETURN;
		}
	}
	else if (registerType < (TOTAL_GP_REGISTERS))
	{
		if (registerType == CPSR) // CPSR is not banked
		{
			pGBA_registers->cpsr.psrMemory = u32parameter;
			setARMMode((OP_MODE_TYPE)pGBA_registers->cpsr.psrFields.psrModeBits);
			RETURN;
		}
		else if (registerType == SPSR)
		{
			pGBA_registers->spsr[registerBank].psrMemory = u32parameter;
			pGBA_registers->spsr[ZERO].psrMemory = ERROR; // SPSR doesn't exist for User and System modes
			RETURN;
		}
	}

	FATAL("Writing to Unknown Register");
}

uint32_t GBA_t::cpuReadRegister(REGISTER_BANK_TYPE rb, REGISTER_TYPE rt)
{
	uint8_t registerType = ((uint8_t)rt);
	uint8_t registerBank = ((uint8_t)rb);
	if (registerType < (LO_GP_REGISTERS)) // lower 8 registers are not banked
	{
		RETURN pGBA_registers->unbankedLORegisters[registerType];
	}
	else if (registerType < (LO_GP_REGISTERS + HI_GP_REGISTERS))
	{
		if ((registerType == SP) || (registerType == LR))
		{
			RETURN pGBA_registers->bankedHIRegisters[registerBank][(registerType - LO_GP_REGISTERS)];
		}
		else if (registerType == PC) // PC is not banked
		{
			RETURN pGBA_registers->pc;
		}
		else // R8 to R12
		{
			if (rb == REGISTER_BANK_TYPE::RB_FIQ) // banked if FIQ mode
			{
				RETURN pGBA_registers->bankedHIRegisters[registerBank][(registerType - LO_GP_REGISTERS)];
			}
			else // not banked if not FIQ mode (so RETURN from any one mode other than FIQ mode)
			{
				RETURN pGBA_registers->bankedHIRegisters[ZERO][(registerType - LO_GP_REGISTERS)];
			}
		}
	}
	else if (registerType < (TOTAL_GP_REGISTERS))
	{
		if (registerType == CPSR) // CPSR is not banked
		{
			RETURN pGBA_registers->cpsr.psrMemory;
		}
		else if (registerType == SPSR)
		{
			if (rb == REGISTER_BANK_TYPE::RB_USR_SYS)
			{
				// NOTE: In system/user mode reading from SPSR RETURNs the current CPSR value.
				// However writes to SPSR appear to do nothing.
				// Refer 3.7.1 of https://www.dwedit.org/files/ARM7TDMI.pdf and Nano 

				RETURN pGBA_registers->cpsr.psrMemory;
			}
			else
			{
				RETURN pGBA_registers->spsr[registerBank].psrMemory;
			}
		}
	}

	FATAL("Reading from Unknown Register");
	RETURN ((uint32_t)NULL);
}

GBA_HALFWORD  GBA_t::readIO(uint32_t address, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType)
{
	// NOTE: Below if condition to handle "MEMORY_ACCESS_SOURCE::HOST" is very important for following reasons
	// * We to handle non 16 bit writes to IO basically do multiple 16 bit reads and combine data and write back
	// * Problem arises when the 16 bit read we do happens to be on a write only register, our standard read will give open-bus data, etc
	// * The combined data which we write in the end will mess the contents of write only registers
	// * Hence, to handle reads used for combine purpose, we should directly read the memory without handling for open-bus, etc
	// * To differentiate this read, we have created "MEMORY_ACCESS_SOURCE::HOST"
	if (source == MEMORY_ACCESS_SOURCE::HOST)
	{
		RETURN pGBA_memory->mGBAMemoryMap.mIO.mIOMemory16bit[(address - IO_START_ADDRESS) / TWO];
	}

	switch (address)
	{
	case IO_DISPCNT:
	{
		RETURN pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTHalfWord;
	}
	case IO_GREENSWAP:
	{
		RETURN pGBA_peripherals->mGREENSWAPHalfWord.mGREENSWAPHalfWord;
	}
	case IO_DISPSTAT:
	{
		RETURN pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord;
	}
	case IO_VCOUNT:
	{
		RETURN pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTHalfWord;
	}
	case IO_BG0CNT:
	{
		pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT = RESET;
		RETURN pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord;
	}
	case IO_BG1CNT:
	{
		pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT = RESET;
		RETURN pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord;
	}
	case IO_BG2CNT:
	{
		RETURN pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord;
	}
	case IO_BG3CNT:
	{
		RETURN pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord;
	}
	case IO_BG0HOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG0VOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG1HOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG1VOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2HOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2VOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3HOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3VOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2PA:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2PB:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2PC:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2PD:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2X_L:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2X_H:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2Y_L:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2Y_H:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3PA:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3PB:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3PC:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3PD:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3X_L:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3X_H:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3Y_L:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3Y_H:
	{
		RETURN 0xDEAD;
	}
	case IO_WIN0H:
	{
		RETURN 0xDEAD;
	}
	case IO_WIN1H:
	{
		RETURN 0xDEAD;
	}
	case IO_WIN0V:
	{
		RETURN 0xDEAD;
	}
	case IO_WIN1V:
	{
		RETURN 0xDEAD;
	}
	case IO_WININ:
	{
		pGBA_peripherals->mWININHalfWord.mWININFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mWININHalfWord.mWININFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mWININHalfWord.mWININHalfWord;
	}
	case IO_WINOUT:
	{
		pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mWINOUTHalfWord.mWINOUTHalfWord;
	}
	case IO_MOSAIC:
	{
		RETURN 0xDEAD;
	}
	case IO_400004E:
	{
		RETURN 0xDEAD;
	}
	case IO_BLDCNT:
	{
		pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTFields.NOT_USED_0 = RESET;
		RETURN pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord;
	}
	case IO_BLDALPHA:
	{
		pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAHalfWord;
	}
	case IO_BLDY:
	{
		RETURN 0xDEAD;
	}
	case IO_4000056:
	case IO_4000058:
	case IO_400005A:
	case IO_400005C:
	case IO_400005E:
	{
		RETURN 0xDEAD;
	}
	case IO_SOUND1CNT_L:
	{
		pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.NOT_USED_0 = RESET;
		RETURN pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord;
	}
	case IO_SOUND1CNT_H:
	{
		mSOUND1CNT_HHalfWord_t data = pGBA_peripherals->mSOUND1CNT_HHalfWord;
		data.mSOUND1CNT_HFields.SOUND_LENGTH = RESET;
		RETURN data.mSOUND1CNT_HHalfWord;
	}
	case IO_SOUND1CNT_X:
	{
		pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.NOT_USED_0 = RESET;
		mSOUND1CNT_XHalfWord_t data = pGBA_peripherals->mSOUND1CNT_XHalfWord;
		data.mSOUND1CNT_XFields.FREQ = RESET;
		data.mSOUND1CNT_XFields.NOT_USED_0 = RESET;
		data.mSOUND1CNT_XFields.INITIAL = RESET;
		RETURN data.mSOUND1CNT_XHalfWord;
	}
	case IO_4000066:
	{
		RETURN 0x0000;
	}
	case IO_SOUND2CNT_L:
	{
		mSOUND2CNT_LHalfWord_t data = pGBA_peripherals->mSOUND2CNT_LHalfWord;
		data.mSOUND2CNT_LFields.SOUND_LENGTH = RESET;
		RETURN data.mSOUND2CNT_LHalfWord;
	}
	case IO_400006A:
	{
		RETURN 0x0000;
	}
	case IO_SOUND2CNT_H:
	{
		pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.NOT_USED_0 = RESET;
		mSOUND2CNT_HHalfWord_t data = pGBA_peripherals->mSOUND2CNT_HHalfWord;
		data.mSOUND2CNT_HFields.FREQ = RESET;
		data.mSOUND2CNT_HFields.NOT_USED_0 = RESET;
		data.mSOUND2CNT_HFields.INITIAL = RESET;
		RETURN data.mSOUND2CNT_HHalfWord;
	}
	case IO_400006E:
	{
		RETURN 0x0000;
	}
	case IO_SOUND3CNT_L:
	{
		pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord;
	}
	case IO_SOUND3CNT_H:
	{
		mSOUND3CNT_HHalfWord_t data = pGBA_peripherals->mSOUND3CNT_HHalfWord;
		data.mSOUND3CNT_HFields.SOUND_LENGTH = RESET;
		data.mSOUND3CNT_HFields.NOT_USED_0 = RESET;
		RETURN data.mSOUND3CNT_HHalfWord;
	}
	case IO_SOUND3CNT_X:
	{
		pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.NOT_USED_0 = RESET;
		mSOUND3CNT_XHalfWord_t data = pGBA_peripherals->mSOUND3CNT_XHalfWord;
		data.mSOUND3CNT_XFields.SAMPLE_RATE = RESET;
		data.mSOUND3CNT_XFields.NOT_USED_0 = RESET;
		data.mSOUND3CNT_XFields.INITIAL = RESET;
		RETURN data.mSOUND3CNT_XHalfWord;
	}
	case IO_4000076:
	{
		RETURN 0x0000;
	}
	case IO_SOUND4CNT_L:
	{
		pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.NOT_USED_0 = RESET;
		mSOUND4CNT_LHalfWord_t data = pGBA_peripherals->mSOUND4CNT_LHalfWord;
		data.mSOUND4CNT_LFields.SOUND_LENGTH = RESET;
		data.mSOUND4CNT_LFields.NOT_USED_0 = RESET;
		RETURN data.mSOUND4CNT_LHalfWord;
	}
	case IO_400007A:
	{
		RETURN 0x0000;
	}
	case IO_SOUND4CNT_H:
	{
		pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.NOT_USED_0 = RESET;
		mSOUND4CNT_HHalfWord_t data = pGBA_peripherals->mSOUND4CNT_HHalfWord;
		data.mSOUND4CNT_HFields.NOT_USED_0 = RESET;
		data.mSOUND4CNT_HFields.INITIAL = RESET;
		RETURN data.mSOUND4CNT_HHalfWord;
	}
	case IO_400007E:
	{
		RETURN 0x0000;
	}
	case IO_SOUNDCNT_L:
	{
		pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.NOT_USED_1 = RESET;
		mSOUNDCNT_LHalfWord_t data = pGBA_peripherals->mSOUNDCNT_LHalfWord;
		data.mSOUNDCNT_LFields.NOT_USED_0 = RESET;
		data.mSOUNDCNT_LFields.NOT_USED_1 = RESET;
		RETURN data.mSOUNDCNT_LHalfWord;
	}
	case IO_SOUNDCNT_H:
	{
		pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.NOT_USED_0 = RESET;
		mSOUNDCNT_HHalfWord_t data = pGBA_peripherals->mSOUNDCNT_HHalfWord;
		data.mSOUNDCNT_HFields.NOT_USED_0 = RESET;
		data.mSOUNDCNT_HFields.DMA_SOUND_A_RESET_FIFO = RESET;
		data.mSOUNDCNT_HFields.DMA_SOUND_B_RESET_FIFO = RESET;
		RETURN data.mSOUNDCNT_HHalfWord;
	}
	case IO_SOUNDCNT_X:
	{
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XHalfWord;
	}
	case IO_4000086:
	{
		RETURN 0x0000;
	}
	case IO_SOUNDBIAS:
	{
		pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASFields.NOT_USED_0 = RESET;
		RETURN pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASHalfWord;
	}
	case IO_400008A:
	{
		RETURN 0x0000;
	}
	case IO_400008C:
	case IO_400008E:
	{
		RETURN 0xDEAD;
	}
	case (IO_WAVERAM_START_ADDRESS + 0):
	case (IO_WAVERAM_START_ADDRESS + 2):
	case (IO_WAVERAM_START_ADDRESS + 4):
	case (IO_WAVERAM_START_ADDRESS + 6):
	case (IO_WAVERAM_START_ADDRESS + 8):
	case (IO_WAVERAM_START_ADDRESS + 10):
	case (IO_WAVERAM_START_ADDRESS + 12):
	case (IO_WAVERAM_START_ADDRESS + 14):
	{
		RETURN pGBA_peripherals->mWAVERAM16[(address - IO_WAVERAM_START_ADDRESS) / TWO].waveRamHalfWord;
	}
	case IO_FIFO_A_L:
	{
		RETURN 0xDEAD;
	}
	case IO_FIFO_A_H:
	{
		RETURN 0xDEAD;
	}
	case IO_FIFO_B_L:
	{
		RETURN 0xDEAD;
	}
	case IO_FIFO_B_H:
	{
		RETURN 0xDEAD;
	}
	case IO_40000A8:
	case IO_40000AA:
	case IO_40000AC:
	case IO_40000AE:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA0SAD_L:
	case IO_DMA0SAD_H:
	case IO_DMA0DAD_L:
	case IO_DMA0DAD_H:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA0CNT_L:
	{
		RETURN 0x0000;
	}
	case IO_DMA0CNT_H:
	{
		mDMAnCNT_HHalfWord_t data = pGBA_peripherals->mDMA0CNT_H;
		data.mDMAnCNT_HFields.NOT_USED_0 = RESET;
		data.mDMAnCNT_HFields.GAME_PAK_DRQ = RESET;
		RETURN data.mDMAnCNT_HHalfWord;
	}
	case IO_DMA1SAD_L:
	case IO_DMA1SAD_H:
	case IO_DMA1DAD_L:
	case IO_DMA1DAD_H:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA1CNT_L:
	{
		RETURN 0x0000;
	}
	case IO_DMA1CNT_H:
	{
		mDMAnCNT_HHalfWord_t data = pGBA_peripherals->mDMA1CNT_H;
		data.mDMAnCNT_HFields.NOT_USED_0 = RESET;
		data.mDMAnCNT_HFields.GAME_PAK_DRQ = RESET;
		RETURN data.mDMAnCNT_HHalfWord;
	}
	case IO_DMA2SAD_L:
	case IO_DMA2SAD_H:
	case IO_DMA2DAD_L:
	case IO_DMA2DAD_H:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA2CNT_L:
	{
		RETURN 0x0000;
	}
	case IO_DMA2CNT_H:
	{
		mDMAnCNT_HHalfWord_t data = pGBA_peripherals->mDMA2CNT_H;
		data.mDMAnCNT_HFields.NOT_USED_0 = RESET;
		data.mDMAnCNT_HFields.GAME_PAK_DRQ = RESET;
		RETURN data.mDMAnCNT_HHalfWord;
	}
	case IO_DMA3SAD_L:
	case IO_DMA3SAD_H:
	case IO_DMA3DAD_L:
	case IO_DMA3DAD_H:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA3CNT_L:
	{
		RETURN 0x0000;
	}
	case IO_DMA3CNT_H:
	{
		mDMAnCNT_HHalfWord_t data = pGBA_peripherals->mDMA3CNT_H;
		data.mDMAnCNT_HFields.NOT_USED_0 = RESET;
		RETURN data.mDMAnCNT_HHalfWord;
	}
	case IO_40000E0:
	case IO_40000E2:
	case IO_40000E4:
	case IO_40000E6:
	case IO_40000E8:
	case IO_40000EA:
	case IO_40000EC:
	case IO_40000EE:
	case IO_40000F0:
	case IO_40000F2:
	case IO_40000F4:
	case IO_40000F6:
	case IO_40000F8:
	case IO_40000FA:
	case IO_40000FC:
	case IO_40000FE:
	{
		RETURN 0xDEAD;
	}
	case IO_TM0CNT_L:
	{
		RETURN pGBA_peripherals->mTIMER0CNT_L;
	}
	case IO_TM0CNT_H:
	{
		RETURN pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HHalfWord;
	}
	case IO_TM1CNT_L:
	{
		RETURN pGBA_peripherals->mTIMER1CNT_L;
	}
	case IO_TM1CNT_H:
	{
		RETURN pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HHalfWord;
	}
	case IO_TM2CNT_L:
	{
		RETURN pGBA_peripherals->mTIMER2CNT_L;
	}
	case IO_TM2CNT_H:
	{
		RETURN pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HHalfWord;
	}
	case IO_TM3CNT_L:
	{
		RETURN pGBA_peripherals->mTIMER3CNT_L;
	}
	case IO_TM3CNT_H:
	{
		RETURN pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HHalfWord;
	}
	case IO_SIOMULTI0:
	{
		RETURN pGBA_peripherals->mSIOMULTI0;
	}
	case IO_SIOMULTI1:
	{
		RETURN pGBA_peripherals->mSIOMULTI1;
	}
	case IO_SIOMULTI2:
	{
		RETURN pGBA_peripherals->mSIOMULTI2;
	}
	case IO_SIOMULTI3:
	{
		RETURN pGBA_peripherals->mSIOMULTI3;
	}
	case IO_SIOCNT:
	{
		RETURN pGBA_peripherals->mSIOCNT.mSIOCNTHalfWord;
	}
	case IO_SIO_DATA8_MLTSEND:
	{
		RETURN pGBA_peripherals->mSIO_DATA8_MLTSEND;
	}
	case IO_KEYINPUT:
	{
		RETURN pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTHalfWord;
	}
	case IO_KEYCNT:
	{
		RETURN pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTHalfWord;
	}
	case IO_RCNT:
	{
		RETURN pGBA_peripherals->mRCNTHalfWord.mRCNTHalfWord;
	}
	case IO_IR:
	{
		RETURN 0x0000;
	}
	case IO_JOYCNT:
	{
		RETURN pGBA_peripherals->mJOYCNTHalfWord.mJOYCNTHalfWord;
	}
	case IO_4000142:
	{
		RETURN 0x0000;
	}
	case IO_JOY_RECV_L:
	{
		RETURN pGBA_peripherals->mJOY_RECV_L;
	}
	case IO_JOY_RECV_H:
	{
		RETURN pGBA_peripherals->mJOY_RECV_H;
	}
	case IO_JOY_TRANS_L:
	{
		RETURN pGBA_peripherals->mJOY_TRANS_L;
	}
	case IO_JOY_TRANS_H:
	{
		RETURN pGBA_peripherals->mJOY_TRANS_H;
	}
	case IO_JOYSTAT:
	{
		RETURN pGBA_peripherals->mJOYSTATHalfWord.mJOYSTATHalfWord;
	}
	case IO_400015A:
	{
		RETURN 0x0000;
	}
	case IO_IE:
	{
		RETURN pGBA_peripherals->mIEHalfWord.mIEHalfWord;
	}
	case IO_IF:
	{
		RETURN pGBA_peripherals->mIFHalfWord.mIFHalfWord;
	}
	case IO_WAITCNT:
	{
		RETURN pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTHalfWord;
	}
	case IO_4000206:
	{
		RETURN 0x0000;
	}
	case IO_IME:
	{
		RETURN pGBA_peripherals->mIMEHalfWord.mIMEHalfWord;
	}
	case IO_400020A:
	{
		RETURN 0x0000;
	}
	case IO_4000302:
	{
		RETURN 0x0000;
	}
	case IO_POSTFLG_HALTCNT:
	{
		RETURN static_cast<GBA_HALFWORD>(pGBA_peripherals->mPOSTFLG_HALTCNT_HalfWord.mPOSTFLG_HALTCNT_Fields.mPOSTFLGByte.mPOSTFLGByte);
	}
	default:
	{
		RETURN readOpenBus<GBA_HALFWORD>(address, accessWidth, source, accessType);
	}
	}
}

void GBA_t::writeIO(uint32_t address, GBA_HALFWORD data, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType)
{
	switch (address)
	{
	case IO_DISPCNT:
	{
		pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTHalfWord = data;
		RETURN;
	}
	case IO_GREENSWAP:
	{
		pGBA_peripherals->mGREENSWAPHalfWord.mGREENSWAPHalfWord = data;
		RETURN;
	}
	case IO_DISPSTAT:
	{
		uint16_t dispstatBeforeUpdate = pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord;
		pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord = data;
		GBA_HALFWORD retainMask = dispstatBeforeUpdate & 0x07; // extract last 3 bits
		pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord &= 0xFFF8; // clear last 3 bits
		pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord |= retainMask; // set last 3 bits to extracted value
		RETURN;
	}
	case IO_VCOUNT:
	{
		RETURN;
	}
	case IO_BG0CNT:
	{
		pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord = data;
		RETURN;
	}
	case IO_BG1CNT:
	{
		pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord = data;
		RETURN;
	}
	case IO_BG2CNT:
	{
		pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord = data;
		RETURN;
	}
	case IO_BG3CNT:
	{
		pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord = data;
		RETURN;
	}
	case IO_BG0HOFS:
	{
		pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG0VOFS:
	{
		pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG1HOFS:
	{
		pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG1VOFS:
	{
		pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG2HOFS:
	{
		pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG2VOFS:
	{
		pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG3HOFS:
	{
		pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG3VOFS:
	{
		pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG2PA:
	{
		pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG2PB:
	{
		pGBA_peripherals->mBG2PBHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG2PC:
	{
		pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG2PD:
	{
		pGBA_peripherals->mBG2PDHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG2X_L:
	{
		pGBA_peripherals->mBG2XWord.mBGniHalfWords.mBGniWord_L = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG2].internalRefPointRegisters.bgxOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG2X_H:
	{
		pGBA_peripherals->mBG2XWord.mBGniHalfWords.mBGniWord_H = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG2].internalRefPointRegisters.bgxOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG2Y_L:
	{
		pGBA_peripherals->mBG2YWord.mBGniHalfWords.mBGniWord_L = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG2].internalRefPointRegisters.bgyOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG2Y_H:
	{
		pGBA_peripherals->mBG2YWord.mBGniHalfWords.mBGniWord_H = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG2].internalRefPointRegisters.bgyOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG3PA:
	{
		pGBA_peripherals->mBG3PAHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG3PB:
	{
		pGBA_peripherals->mBG3PBHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG3PC:
	{
		pGBA_peripherals->mBG3PCHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG3PD:
	{
		pGBA_peripherals->mBG3PDHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG3X_L:
	{
		pGBA_peripherals->mBG3XWord.mBGniHalfWords.mBGniWord_L = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG3].internalRefPointRegisters.bgxOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG3X_H:
	{
		pGBA_peripherals->mBG3XWord.mBGniHalfWords.mBGniWord_H = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG3].internalRefPointRegisters.bgxOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG3Y_L:
	{
		pGBA_peripherals->mBG3YWord.mBGniHalfWords.mBGniWord_L = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG3].internalRefPointRegisters.bgyOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG3Y_H:
	{
		pGBA_peripherals->mBG3YWord.mBGniHalfWords.mBGniWord_H = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG3].internalRefPointRegisters.bgyOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_WIN0H:
	{
		pGBA_peripherals->mWIN0HHalfWord.mWINniHalfWord = data;
		RETURN;
	}
	case IO_WIN1H:
	{
		pGBA_peripherals->mWIN1HHalfWord.mWINniHalfWord = data;
		RETURN;
	}
	case IO_WIN0V:
	{
		pGBA_peripherals->mWIN0VHalfWord.mWINniHalfWord = data;
		RETURN;
	}
	case IO_WIN1V:
	{
		pGBA_peripherals->mWIN1VHalfWord.mWINniHalfWord = data;
		RETURN;
	}
	case IO_WININ:
	{
		pGBA_peripherals->mWININHalfWord.mWININHalfWord = data;
		RETURN;
	}
	case IO_WINOUT:
	{
		pGBA_peripherals->mWINOUTHalfWord.mWINOUTHalfWord = data;
		RETURN;
	}
	case IO_MOSAIC:
	{
		pGBA_peripherals->mMOSAICHalfWord.mMOSAICHalfWord = data;
		RETURN;
	}
	case IO_400004E:
	{
		RETURN;
	}
	case IO_BLDCNT:
	{
		pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord = data;
		RETURN;
	}
	case IO_BLDALPHA:
	{
		pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAHalfWord = data;
		RETURN;
	}
	case IO_BLDY:
	{
		pGBA_peripherals->mBLDYHalfWord.mBLDYHalfWord = data;
		RETURN;
	}
	case IO_4000056:
	case IO_4000058:
	case IO_400005A:
	case IO_400005C:
	case IO_400005E:
	{
		RETURN;
	}
	case IO_SOUND1CNT_L:
	{
		BYTE nr10 = pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord & 0xFF;
		pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord = data;

		// writing to Sound channel 1 sweep
		//if (nr10 != (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord |= nr10;
			}
			// NOTE: One of the weird quirks of frequency sweep
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			else if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_FREQ_DIR == ZERO
				&& pGBA_instance->GBA_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger == YES)
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = ZERO;
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.INITIAL = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
			}
		}

		RETURN;
	}
	case IO_SOUND1CNT_H:
	{
		BYTE nr11 = pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord & 0xFF;
		BYTE nr12 = (pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord = data;

		// writing to Sound channel 1 length timer & duty cycle
		//if (nr11 != (pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord |= nr11;
			}
			else
			{

				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer
					= 64 - pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.SOUND_LENGTH;
			}
		}

		// writing to Sound channel 1 volume & envelope
		//if (nr12 != ((pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord |= (nr12 << EIGHT);
			}
			else
			{
				FLAG wasVolumePeriodZero = ((nr12 & 0b111) == ZERO);
				FLAG wasEnvelopeInSubtractMode = ((nr12 & 0b1000) == ZERO);
				// NOTE: One of the weird quirks of APU called the "Zombie Mode"
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (isAudioChannelEnabled(AUDIO_CHANNELS::CHANNEL_1) == YES)
				{
					if (wasVolumePeriodZero == YES
						&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isVolumeEnvelopeStillDoingAutomaticUpdates == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
					}
					else if (wasEnvelopeInSubtractMode == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
					}
					if (
						(wasEnvelopeInSubtractMode == YES
							&& pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_DIR == ONE)
						||
						(wasEnvelopeInSubtractMode == NO
							&& pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_DIR == ZERO))
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume
							= SIXTEEN - pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume
						&= 0x0F;
				}
				continousDACCheck();
			}
		}

		RETURN;
	}
	case IO_SOUND1CNT_X:
	{
		BYTE nr13 = pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord & 0xFF;
		BYTE nr14 = (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord = data;

		// writing to Sound channel 1 period low
		//if (nr13 != (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord |= nr13;
			}
		}

		// writing to Sound channel 1 period high & control
		//if (nr14 != ((pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord |= (nr14 << EIGHT);
			}
			else
			{
				FLAG wasLengthEnableBitZero = ((nr14 & 0b1000000) == ZERO);
				// NOTE: One of the weird quirks of length counter
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (wasLengthEnableBitZero == YES
					&& pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.LENGTH_FLAG == ONE
					&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE
					&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer > ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer;
					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer == ZERO)
					{
						pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = ZERO;
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
					}
				}
				if (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.INITIAL == ONE)
				{
					enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_1);
					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer == ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer = 64;
						// NOTE: One of the weird quirks of length counter
						// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
						if (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.LENGTH_FLAG == ONE
							&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
						{
							--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer;
						}
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer
						= pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_STEP_TIME;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume
						= pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_INIT_VOL;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isVolumeEnvelopeStillDoingAutomaticUpdates = YES;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].shadowFrequency
						= getChannelPeriod(AUDIO_CHANNELS::CHANNEL_1);

					if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME > ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepTimer
							= pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME;
					}
					else
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepTimer = EIGHT;
					}
					if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME > ZERO
						|| pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_SHIFT > ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepEnabled = ENABLED;
					}
					else
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepEnabled = DISABLED;
					}
					pGBA_instance->GBA_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger = NO;
					if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_SHIFT > ZERO)
					{
						performOverFlowCheck();
					}

					// Source : https://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu
					uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_1)) * FOUR;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].frequencyTimer
						= (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].frequencyTimer & THREE)
						+ resetFrequencyTimer;
				}
			}
		}

		RETURN;
	}
	case IO_4000066:
	{
		RETURN;
	}
	case IO_SOUND2CNT_L:
	{
		BYTE nr21 = pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord & 0xFF;
		BYTE nr22 = (pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord = data;

		// writing to Sound channel 2 length timer & duty cycle
		//if (nr21 != (pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord |= nr21;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer
					= 64 - pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.SOUND_LENGTH;
			}
		}

		// writing to Sound channel 2 volume & envelope
		//if (nr22 != ((pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord |= (nr22 << EIGHT);
			}
			else
			{
				FLAG wasVolumePeriodZero = ((nr22 & 0b111) == ZERO);
				FLAG wasEnvelopeInSubtractMode = ((nr22 & 0b1000) == ZERO);
				// NOTE: One of the weird quirks of APU called the "Zombie Mode"
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (isAudioChannelEnabled(AUDIO_CHANNELS::CHANNEL_2) == YES)
				{
					if (wasVolumePeriodZero == YES
						&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isVolumeEnvelopeStillDoingAutomaticUpdates == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
					}
					else if (wasEnvelopeInSubtractMode == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
					}
					if (
						(wasEnvelopeInSubtractMode == YES
							&& pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_DIR == ONE)
						||
						(wasEnvelopeInSubtractMode == NO
							&& pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_DIR == ZERO))
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume
							= SIXTEEN - pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume &= 0x0F;
				}
				continousDACCheck();
			}
		}

		RETURN;
	}
	case IO_400006A:
	{
		RETURN;
	}
	case IO_SOUND2CNT_H:
	{
		BYTE nr23 = pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord & 0xFF;
		BYTE nr24 = (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord = data;

		// writing to Sound channel 2 period low
		//if (nr23 != (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord |= nr23;
			}
		}

		// writing to Sound channel 2 period high & control
		//if (nr24 != ((pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord |= (nr24 << EIGHT);
			}
			else
			{
				FLAG wasLengthEnableBitZero = ((nr24 & 0b1000000) == ZERO);
				// NOTE: One of the weird quirks of length counter
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (wasLengthEnableBitZero == YES
					&& pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.LENGTH_FLAG == ONE
					&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE
					&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer > ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer;

					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer == ZERO)
					{
						pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = ZERO;
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
					}
				}
				if (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.INITIAL == ONE)
				{
					enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_2);
					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer == ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer = 64;
						// NOTE: One of the weird quirks of length counter
						// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
						if (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.LENGTH_FLAG == ONE
							&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
						{
							--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer;
						}
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer
						= pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_STEP_TIME;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume
						= pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_INIT_VOL;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isVolumeEnvelopeStillDoingAutomaticUpdates = YES;
				
					// Source : https://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu
					uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_2)) * FOUR;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].frequencyTimer
						= (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].frequencyTimer & THREE)
						+ resetFrequencyTimer;
				}
			}
		}

		RETURN;
	}
	case IO_400006E:
	{
		RETURN;
	}
	case IO_SOUND3CNT_L:
	{
		BYTE nr30 = pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord & 0xFF;
		BYTE currentWB = pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_BANK_NUMBER;
		BYTE currentWD = pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_DIMENSION;
		pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord = data;

		// writing to Sound channel 3 DAC enable
		//if (nr30 != (pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord |= nr30;
			}
			else
			{
				continousDACCheck();

				if (currentWB != pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_BANK_NUMBER)
				{
					std::swap_ranges(std::begin(pGBA_peripherals->mWAVERAM8), std::end(pGBA_peripherals->mWAVERAM8), std::begin(pGBA_memory->mBankedWAVERAM.mWAVERAM8));
				}

				if (currentWD != pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_DIMENSION)
				{
					;
				}
			}
		}

		RETURN;
	}
	case IO_SOUND3CNT_H:
	{
		BYTE nr31 = pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord & 0xFF;
		BYTE nr32 = (pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord = data;

		// writing to Sound channel 3 length timer
		//if (nr31 != (pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord |= nr31;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer = 256 - nr31;
			}
		}

		// Sound channel 3 output level
		//if (nr32 != ((pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord |= (nr32 << EIGHT);
			}
			else
			{
				if (pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HFields.FORCE_VOL == ONE)
				{
					pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = THREE;
				}
				else
				{
					switch (pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HFields.SOUND_VOL)
					{
					case ZERO:
						pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = ZERO;
						BREAK;
					case ONE:
						pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = FOUR;
						BREAK;
					case TWO:
						pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = TWO;
						BREAK;
					case THREE:
						pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = ONE;
						BREAK;
					}
				}
			}
		}

		RETURN;
	}
	case IO_SOUND3CNT_X:
	{
		BYTE nr33 = pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord & 0xFF;
		BYTE nr34 = (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord = data;

		// Sound channel 3 period low
		//if (nr33 != (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord |= nr33;
			}
		}

		// writing to Sound channel 3 period high & control
		//if (nr34 != ((pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord |= (nr34 << EIGHT);
			}
			else
			{
				FLAG wasLengthEnableBitZero = ((nr34 & 0b1000000) == ZERO);
				// NOTE: One of the weird quirks of length counter
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (wasLengthEnableBitZero == YES
					&& pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.LENGTH_FLAG == ONE
					&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE
					&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer > ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer;

					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer == ZERO)
					{
						pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = ZERO;
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
					}
				}
				if (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.INITIAL == ONE)
				{
					enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_3);

					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer == ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer = 256;

						// NOTE: One of the weird quirks of length counter
						// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
						if (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.LENGTH_FLAG == ONE
							&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
						{
							--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer;
						}
					}

					// Period needs to be reloaded as per https://gbdev.io/pandocs/Audio_Registers.html#ff1e--nr34-channel-3-period-high--control
					uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_3)) * TWO;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].frequencyTimer = resetFrequencyTimer;
					// As per https://forums.nesdev.org/viewtopic.php?p=188035#p188035, another 6 cycle delay is needed
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].frequencyTimer += SIX;

					pGBA_instance->GBA_state.audio.waveRamCurrentIndex = RESET;
					pGBA_instance->GBA_state.audio.didChannel3ReadWaveRamPostTrigger = NO;
				}
			}
		}

		RETURN;
	}
	case IO_4000076:
	{
		RETURN;
	}
	case IO_SOUND4CNT_L:
	{
		BYTE nr41 = pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord & 0xFF;
		BYTE nr42 = (pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord = data;

		// writing to Sound channel 4 length timer
		//if (nr41 != (pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord |= nr41;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer
					= 64 - pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.SOUND_LENGTH;
			}
		}

		// writing to Sound channel 4 volume & envelope
		//if (nr42 != ((pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord |= (nr42 << EIGHT);
			}
			else
			{
				FLAG wasVolumePeriodZero = ((nr42 & 0b111) == ZERO);
				FLAG wasEnvelopeInSubtractMode = ((nr42 & 0b1000) == ZERO);
				// NOTE: One of the weird quirks of APU called the "Zombie Mode"
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (isAudioChannelEnabled(AUDIO_CHANNELS::CHANNEL_4) == YES)
				{
					if (wasVolumePeriodZero == YES
						&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isVolumeEnvelopeStillDoingAutomaticUpdates == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
					}
					else if (wasEnvelopeInSubtractMode == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
					}
					if (
						(wasEnvelopeInSubtractMode == YES
							&& pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_DIR == ONE)
						||
						(wasEnvelopeInSubtractMode == NO
							&& pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_DIR == ZERO))
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume
							= SIXTEEN - pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume &= 0x0F;
				}
				continousDACCheck();
			}
		}

		RETURN;
	}
	case IO_400007A:
	{
		RETURN;
	}
	case IO_SOUND4CNT_H:
	{
		BYTE nr43 = pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord & 0xFF;
		BYTE nr44 = (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord = data;

		// Sound channel 4 frequency & randomness
		//if (nr43 != (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord |= nr43;
			}
		}

		// writing to Sound channel 4 control
		//if (nr44 != ((pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord |= (nr44 << EIGHT);
			}
			else
			{
				FLAG wasLengthEnableBitZero = ((nr44 & 0b1000000) == ZERO);
				// NOTE: One of the weird quirks of length counter
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (wasLengthEnableBitZero == YES
					&& pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.LENGTH_FLAG == ONE
					&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE
					&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer > ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer;

					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer == ZERO)
					{
						pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = ZERO;
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
					}
				}
				if (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.INITIAL == ONE)
				{
					enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_4);
					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer == ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer = 64;
						// NOTE: One of the weird quirks of length counter
						// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
						if (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.LENGTH_FLAG == ONE
							&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
						{
							--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer;
						}

					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer
						= pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_STEP_TIME;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume
						= pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_INIT_VOL;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isVolumeEnvelopeStillDoingAutomaticUpdates = YES;
					// Refer "trigger event" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].LFSR = 0x7FFF;

					// Source : https://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].frequencyTimer += EIGHT;
				}
			}
		}

		RETURN;
	}
	case IO_400007E:
	{
		RETURN;
	}
	case IO_SOUNDCNT_L:
	{
		BYTE nr50 = pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord & 0xFF;
		BYTE nr51 = (pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord = data;

		// writing to Master volume & VIN panning
		//if (nr50 != (pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord |= nr50;
			}
		}

		// writing to Sound panning
		//if (nr51 != ((pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord |= (nr51 << EIGHT);
			}
		}

		RETURN;
	}
	case IO_SOUNDCNT_H:
	{
		pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HHalfWord = data;
		pGBA_audio->FIFO[DIRECT_SOUND_A].timer = pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_A_TIMER_SEL;
		pGBA_audio->FIFO[DIRECT_SOUND_B].timer = pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_B_TIMER_SEL;

		if (pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_A_RESET_FIFO == ONE)
		{
			pGBA_audio->FIFO[DIRECT_SOUND_A].position = RESET;
			pGBA_audio->FIFO[DIRECT_SOUND_A].size = RESET;
		}
		if (pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_B_RESET_FIFO == ONE)
		{
			pGBA_audio->FIFO[DIRECT_SOUND_B].position = RESET;
			pGBA_audio->FIFO[DIRECT_SOUND_B].size = RESET;
		}

		RETURN;
	}
	case IO_SOUNDCNT_X:
	{
		BYTE nr52 = pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XHalfWord & 0xFF;
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XHalfWord = data;

		// writing to Sound ON/OFF
		//if (nr52 != (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XHalfWord & 0xFF))
		{
			BYTE APU_POWER_WAS = pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN;
			pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN = GETBIT(7, data);
			// Channel ON -> Channel OFF
			if (APU_POWER_WAS == ONE && pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				for (uint32_t address = 0x04000060; address <= 0x040000AE /*0x04000075*/; address++)
				{
					pGBA_memory->mGBAMemoryMap.mIO.mIOMemory8bit[address - 0x04000000] = ZERO;
				}
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
			}
			// Channel OFF -> Channel ON
			else if (APU_POWER_WAS == ZERO && pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
			{
				// Reset the length counters in CGB mode during power up
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer = ZERO;

				pGBA_instance->GBA_state.audio.wasPowerCycled = YES;

				// NOTE: As we are resetting the frame sequencer, next half period WILL clock the length counter
				pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter = FALSE;
			}
		}

		RETURN;
	}
	case IO_4000086:
	{
		RETURN;
	}
	case IO_SOUNDBIAS:
	{
		pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASHalfWord = data & ~1;
		RETURN;
	}
	case IO_400008A:
	{
		RETURN;
	}
	case IO_400008C:
	case IO_400008E:
	{
		RETURN;
	}
	case (IO_WAVERAM_START_ADDRESS + 0):
	case (IO_WAVERAM_START_ADDRESS + 2):
	case (IO_WAVERAM_START_ADDRESS + 4):
	case (IO_WAVERAM_START_ADDRESS + 6):
	case (IO_WAVERAM_START_ADDRESS + 8):
	case (IO_WAVERAM_START_ADDRESS + 10):
	case (IO_WAVERAM_START_ADDRESS + 12):
	case (IO_WAVERAM_START_ADDRESS + 14):
	{
		pGBA_peripherals->mWAVERAM16[(address - IO_WAVERAM_START_ADDRESS) / TWO].waveRamHalfWord = data;
		RETURN;
	}
	case IO_FIFO_A_L:
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
		{
			pGBA_peripherals->mFIFOA_L = data;
			pGBA_audio->FIFO[DIRECT_SOUND_A].fifoByteWrite((data >> ZERO) & 0xFF);
			pGBA_audio->FIFO[DIRECT_SOUND_A].fifoByteWrite((data >> EIGHT) & 0xFF);
		}
		RETURN;
	}
	case IO_FIFO_A_H:
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
		{
			pGBA_peripherals->mFIFOA_H = data;
			pGBA_audio->FIFO[DIRECT_SOUND_A].fifoByteWrite((data >> ZERO) & 0xFF);
			pGBA_audio->FIFO[DIRECT_SOUND_A].fifoByteWrite((data >> EIGHT) & 0xFF);
		}
		RETURN;
	}
	case IO_FIFO_B_L:
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
		{
			pGBA_peripherals->mFIFOB_L = data;
			pGBA_audio->FIFO[DIRECT_SOUND_B].fifoByteWrite((data >> ZERO) & 0xFF);
			pGBA_audio->FIFO[DIRECT_SOUND_B].fifoByteWrite((data >> EIGHT) & 0xFF);
		}
		RETURN;
	}
	case IO_FIFO_B_H:
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
		{
			pGBA_peripherals->mFIFOB_H = data;
			pGBA_audio->FIFO[DIRECT_SOUND_B].fifoByteWrite((data >> ZERO) & 0xFF);
			pGBA_audio->FIFO[DIRECT_SOUND_B].fifoByteWrite((data >> EIGHT) & 0xFF);
		}
		RETURN;
	}
	case IO_40000A8:
	case IO_40000AA:
	case IO_40000AC:
	case IO_40000AE:
	{
		RETURN;
	}
	case IO_DMA0SAD_L:
	{
		pGBA_peripherals->mDMA0SAD_L = data;
		RETURN;
	}
	case IO_DMA0SAD_H:
	{
		pGBA_peripherals->mDMA0SAD_H = data;
		RETURN;
	}
	case IO_DMA0DAD_L:
	{
		pGBA_peripherals->mDMA0DAD_L = data;
		RETURN;
	}
	case IO_DMA0DAD_H:
	{
		pGBA_peripherals->mDMA0DAD_H = data;
		RETURN;
	}
	case IO_DMA0CNT_L:
	{
		pGBA_peripherals->mDMA0CNT_L = data;
		RETURN;
	}
	case IO_DMA0CNT_H:
	{
		//if (source == MEMORY_ACCESS_SOURCE::DMA)
		//{
		//	INFO("DMA updating DMA");
		//}

		FLAG oldEnable = pGBA_peripherals->mDMA0CNT_H.mDMAnCNT_HFields.DMA_EN;
		pGBA_peripherals->mDMA0CNT_H.mDMAnCNT_HHalfWord = data;
		FLAG newEnable = pGBA_peripherals->mDMA0CNT_H.mDMAnCNT_HFields.DMA_EN;

		OnDMAChannelWritten(DMA::DMA0, oldEnable, newEnable);
		RETURN;
	}
	case IO_DMA1SAD_L:
	{
		pGBA_peripherals->mDMA1SAD_L = data;
		RETURN;
	}
	case IO_DMA1SAD_H:
	{
		pGBA_peripherals->mDMA1SAD_H = data;
		RETURN;
	}
	case IO_DMA1DAD_L:
	{
		pGBA_peripherals->mDMA1DAD_L = data;
		RETURN;
	}
	case IO_DMA1DAD_H:
	{
		pGBA_peripherals->mDMA1DAD_H = data;
		RETURN;
	}
	case IO_DMA1CNT_L:
	{
		pGBA_peripherals->mDMA1CNT_L = data;
		RETURN;
	}
	case IO_DMA1CNT_H:
	{
		//if (source == MEMORY_ACCESS_SOURCE::DMA)
		//{
		//	INFO("DMA updating DMA");
		//}

		FLAG oldEnable = pGBA_peripherals->mDMA1CNT_H.mDMAnCNT_HFields.DMA_EN;
		pGBA_peripherals->mDMA1CNT_H.mDMAnCNT_HHalfWord = data;
		FLAG newEnable = pGBA_peripherals->mDMA1CNT_H.mDMAnCNT_HFields.DMA_EN;

		OnDMAChannelWritten(DMA::DMA1, oldEnable, newEnable);
		RETURN;
	}
	case IO_DMA2SAD_L:
	{
		pGBA_peripherals->mDMA2SAD_L = data;
		RETURN;
	}
	case IO_DMA2SAD_H:
	{
		pGBA_peripherals->mDMA2SAD_H = data;
		RETURN;
	}
	case IO_DMA2DAD_L:
	{
		pGBA_peripherals->mDMA2DAD_L = data;
		RETURN;
	}
	case IO_DMA2DAD_H:
	{
		pGBA_peripherals->mDMA2DAD_H = data;
		RETURN;
	}
	case IO_DMA2CNT_L:
	{
		pGBA_peripherals->mDMA2CNT_L = data;
		RETURN;
	}
	case IO_DMA2CNT_H:
	{
		//if (source == MEMORY_ACCESS_SOURCE::DMA)
		//{
		//	INFO("DMA updating DMA");
		//}

		FLAG oldEnable = pGBA_peripherals->mDMA2CNT_H.mDMAnCNT_HFields.DMA_EN;
		pGBA_peripherals->mDMA2CNT_H.mDMAnCNT_HHalfWord = data;
		FLAG newEnable = pGBA_peripherals->mDMA2CNT_H.mDMAnCNT_HFields.DMA_EN;

		OnDMAChannelWritten(DMA::DMA2, oldEnable, newEnable);
		RETURN;
	}
	case IO_DMA3SAD_L:
	{
		pGBA_peripherals->mDMA3SAD_L = data;
		RETURN;
	}
	case IO_DMA3SAD_H:
	{
		pGBA_peripherals->mDMA3SAD_H = data;
		RETURN;
	}
	case IO_DMA3DAD_L:
	{
		pGBA_peripherals->mDMA3DAD_L = data;
		RETURN;
	}
	case IO_DMA3DAD_H:
	{
		pGBA_peripherals->mDMA3DAD_H = data;
		RETURN;
	}
	case IO_DMA3CNT_L:
	{
		pGBA_peripherals->mDMA3CNT_L = data;
		RETURN;
	}
	case IO_DMA3CNT_H:
	{
		//if (source == MEMORY_ACCESS_SOURCE::DMA)
		//{
		//	INFO("DMA updating DMA");
		//}

		FLAG oldEnable = pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN;
		pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HHalfWord = data;
		FLAG newEnable = pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN;

		OnDMAChannelWritten(DMA::DMA3, oldEnable, newEnable);
		RETURN;
	}
	case IO_40000E0:
	case IO_40000E2:
	case IO_40000E4:
	case IO_40000E6:
	case IO_40000E8:
	case IO_40000EA:
	case IO_40000EC:
	case IO_40000EE:
	case IO_40000F0:
	case IO_40000F2:
	case IO_40000F4:
	case IO_40000F6:
	case IO_40000F8:
	case IO_40000FA:
	case IO_40000FC:
	case IO_40000FE:
	{
		RETURN;
	}
#if (GBA_ENABLE_DELAYED_MMIO_WRITE == YES)
	case IO_TM0CNT_L:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, ZERO);
		pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.io_tmxcnt_l = data;
		RETURN;
	}
	case IO_TM0CNT_H:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, ONE);
		pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.io_tmxcnt_h = data;
		RETURN;
	}
	case IO_TM1CNT_L:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, TWO);
		pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.io_tmxcnt_l = data;
		RETURN;
	}
	case IO_TM1CNT_H:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, THREE);
		pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.io_tmxcnt_h = data;
		RETURN;
	}
	case IO_TM2CNT_L:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, FOUR);
		pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.io_tmxcnt_l = data;
		RETURN;
	}
	case IO_TM2CNT_H:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, FIVE);
		pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.io_tmxcnt_h = data;
		RETURN;
	}
	case IO_TM3CNT_L:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, SIX);
		pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.io_tmxcnt_l = data;
		RETURN;
	}
	case IO_TM3CNT_H:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, SEVEN);
		pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.io_tmxcnt_h = data;
		RETURN;
	}
#else
	case IO_TM0CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM0CNT_H:
	{
		BIT timer0EnBeforeUpdate = pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HHalfWord = data;

		// Handles loading of "reload" to "counter" when timer is enabled (0 -> 1)
		if (timer0EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.counter = pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.reload;
			pGBA_peripherals->mTIMER0CNT_L = pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.timerCounter[TIMER::TIMER0] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TIMER::TIMER0].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TIMER::TIMER0].startupDelay = TWO;
		}
		RETURN;
	}
	case IO_TM1CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM1CNT_H:
	{
		BIT timer1EnBeforeUpdate = pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HHalfWord = data;
		if (timer1EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.counter = pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.reload;
			pGBA_peripherals->mTIMER1CNT_L = pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.timerCounter[TIMER::TIMER1] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TIMER::TIMER1].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TIMER::TIMER1].startupDelay = TWO;
		}
		RETURN;
	}
	case IO_TM2CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM2CNT_H:
	{
		BIT timer2EnBeforeUpdate = pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HHalfWord = data;
		if (timer2EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.counter = pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.reload;
			pGBA_peripherals->mTIMER2CNT_L = pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.timerCounter[TIMER::TIMER2] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TIMER::TIMER2].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TIMER::TIMER2].startupDelay = TWO;
		}
		RETURN;
	}
	case IO_TM3CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM3CNT_H:
	{
		BIT timer3EnBeforeUpdate = pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HHalfWord = data;
		if (timer3EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.counter = pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.reload;
			pGBA_peripherals->mTIMER3CNT_L = pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.timerCounter[TIMER::TIMER3] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TIMER::TIMER3].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TIMER::TIMER3].startupDelay = TWO;
		}
		RETURN;
	}
#endif
	case IO_SIOMULTI0:
	{
		pGBA_peripherals->mSIOMULTI0 = data;
		RETURN;
	}
	case IO_SIOMULTI1:
	{
		pGBA_peripherals->mSIOMULTI1 = data;
		RETURN;
	}
	case IO_SIOMULTI2:
	{
		pGBA_peripherals->mSIOMULTI2 = data;
		RETURN;
	}
	case IO_SIOMULTI3:
	{
		pGBA_peripherals->mSIOMULTI3 = data;
		RETURN;
	}
	case IO_SIOCNT:
	{
		pGBA_peripherals->mSIOCNT.mSIOCNTHalfWord = data;
		RETURN;
	}
	case IO_SIO_DATA8_MLTSEND:
	{
		pGBA_peripherals->mSIO_DATA8_MLTSEND = data;
		RETURN;
	}
	case IO_KEYINPUT:
	{
		RETURN;
	}
	case IO_KEYCNT:
	{
		pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTHalfWord = data;
		handleKeypadInterrupts();
		RETURN;
	}
	case IO_RCNT:
	{
		pGBA_peripherals->mRCNTHalfWord.mRCNTHalfWord = data;
		RETURN;
	}
	case IO_IR:
	{
		pGBA_peripherals->mIRHalfWord.mIRHalfWord = data;
		RETURN;
	}
	case IO_JOYCNT:
	{
		pGBA_peripherals->mJOYCNTHalfWord.mJOYCNTHalfWord = data;
		RETURN;
	}
	case IO_4000142:
	{
		RETURN;
	}
	case IO_JOY_RECV_L:
	{
		pGBA_peripherals->mJOY_RECV_L = data;
		RETURN;
	}
	case IO_JOY_RECV_H:
	{
		pGBA_peripherals->mJOY_RECV_H = data;
		RETURN;
	}
	case IO_JOY_TRANS_L:
	{
		pGBA_peripherals->mJOY_TRANS_L = data;
		RETURN;
	}
	case IO_JOY_TRANS_H:
	{
		pGBA_peripherals->mJOY_TRANS_H = data;
		RETURN;
	}
	case IO_JOYSTAT:
	{
		pGBA_peripherals->mJOYSTATHalfWord.mJOYSTATHalfWord = data;
		RETURN;
	}
	case IO_400015A:
	{
		RETURN;
	}
	case IO_IE:
	{
		pGBA_peripherals->mIEHalfWord.mIEHalfWord = data;
		RETURN;
	}
	case IO_IF:
	{
		// For IF: Refer http://problemkaputt.de/gbatek-gba-interrupt-control.htm
		// Writing from CPU is always an ACK, so this is basically W1C register for CPU
		// Masters other than CPU should be able to write this without triggering W1C
		pGBA_peripherals->mIFHalfWord.mIFHalfWord &= ~data;
		RETURN;
	}
	case IO_WAITCNT:
	{
		// Refer to https://problemkaputt.de/gbatek-gba-system-control.htm

		pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTHalfWord = data;

		auto NonSequentialWaitStates = [&](uint32_t x) {
			switch (x)
			{
			case ZERO:  RETURN FIVE;  // 4 + 1
			case ONE:   RETURN FOUR;  // 3 + 1
			case TWO:   RETURN THREE; // 2 + 1
			case THREE: RETURN NINE;  // 8 + 1
			default: FATAL("Invalid nonsequential WAITCNT setting: %d!", x); RETURN ZERO;
			}
			};

		// WS0
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_NON_SEQ);
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= (pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_SEQ == 1) ? TWO : THREE;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			+ WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] * TWO;

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];

		// WS1
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_NON_SEQ);
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= (pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_SEQ == 1) ? TWO : FIVE;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			+ WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] * TWO;

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];

		// WS2
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_NON_SEQ);
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= (pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_SEQ == 1) ? TWO : NINE;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			+ WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] * TWO;

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];

		// SRAM (same timing for N and S)
		uint32_t sramCycles = NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.SRAM_WAIT_CTRL);

		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT] = sramCycles;

		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT] = sramCycles;

		RETURN;
	}
	case IO_4000206:
	{
		RETURN;
	}
	case IO_IME:
	{
		pGBA_peripherals->mIMEHalfWord.mIMEHalfWord = data;
		pGBA_peripherals->mIMEHalfWord.mIMEFields.NOT_USED_0 = ZERO;
		RETURN;
	}
	case IO_400020A:
	{
		RETURN;
	}
	case IO_POSTFLG_HALTCNT:
	{
		BIT stopBit = pGBA_peripherals->mPOSTFLG_HALTCNT_HalfWord.mPOSTFLG_HALTCNT_Fields.mHALTCNTByte.mHALTCNTFields.STOP;
		pGBA_peripherals->mPOSTFLG_HALTCNT_HalfWord.mPOSTFLG_HALTCNT_HalfWord = data;

		if (stopBit != pGBA_peripherals->mPOSTFLG_HALTCNT_HalfWord.mPOSTFLG_HALTCNT_Fields.mHALTCNTByte.mHALTCNTFields.STOP)
		{
			if (pGBA_peripherals->mPOSTFLG_HALTCNT_HalfWord.mPOSTFLG_HALTCNT_Fields.mHALTCNTByte.mHALTCNTFields.STOP == SET)
			{
				pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::STOP;
				FATAL("Stop Mode is not supported")
			}
			else
			{
				if (shouldUnHaltTheCPU() == YES)
				{
					CPUEVENT("[RUN] -> [RUN]");
					pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::RUN;
				}
				else
				{
					// Refer "4000301h - HALTCNT - BYTE - Undocumented - Low Power Mode Control (W)" of  https://problemkaputt.de/gbatek-gba-system-control.htm
					CPUEVENT("[RUN] -> [HALT]");
					pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::HALT;
				}
			}
		}

		RETURN;
	}
	case IO_4000302:
	{
		RETURN;
	}
	default:
	{
		pGBA_memory->mGBAMemoryMap.mIO.mIOMemory16bit[(address - IO_START_ADDRESS) / TWO] = data;
		RETURN;
	}
	}
}

FLAG GBA_t::processSOC()
{
	FLAG status = true;

#if (DEACTIVATED) // We cannot have this anymore as ENABLE_LOGS can get modified via GUI, but below code snippet will override this
	ENABLE_LOGS = pGBA_instance->GBA_state.emulatorStatus.debugger.loggerInterface.logger;
#endif

#if (DEACTIVATED) 
#if _DEBUG
	static COUNTER64 threshold = UINT64_MAX;

	if (emulationCounter[ZERO] == threshold)
	{
		volatile FLAG breakpoint0 = 1;
	}

#if (DISABLED)
	if (emulationCounter[ZERO] < threshold - 500)
	{
		;
	}
	else
	{
		;
	}
#endif

	if (pGBA_instance->GBA_state.emulatorStatus.debugger.agbReturn != ZERO)
	{
		volatile int breakpoint = 0;
	}
#endif
#endif

	INFRA("[Loop 0]: %" PRId64, emulationCounter[ZERO]);
	++emulationCounter[ZERO];

	INC64 cyclesInThisRun = RESET;
	INC64 dmaCyclesInThisRun = RESET;

	if (pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::RUN)
	{
		runCPUPipeline();
		cyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter;
	}
	else if (pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::HALT)
	{
		if (IsAnyDMARunning() == YES)
		{
			// Cycles can never be zero; In actual device, clocks are always running
			// Since the emulation's clocks are based on CPU, even when other masters (eg: DMA) running, cycles can appear to be zero
			// Ideally, all master's should be running in parallel
			// However, our emulation works based on ticking the CPU, seeing how many cycles it ticked and ticking other masters by same cycles
			// But, when CPU is HALTED, we need an alternative source of cycles to tick other masters
			// We can use DMA for this if its is running, so basically when CPU is HALTED, "cpuCounter == dmaCounter (if dmaCounter != 0)"
			// If DMA is also not running, then we just tick by one until we reach vblank

			dmaTick();
		}

		dmaCyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter;
		pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter = RESET;

		if (dmaCyclesInThisRun == RESET)
		{
			busCycles();
			cyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter;
		}

		// Refer to http://problemkaputt.de/gbatek-gba-system-control.htm
		if (shouldUnHaltTheCPU() == YES)
		{
			pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::RUN;
			CPUEVENT("[HALT] -> [RUN]");
		}
	}
	else
	{
		FATAL("CPU is in STOP mode");
	}

#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_TICK == NO)
	processPPU(cyclesInThisRun);
#endif

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter = RESET;
	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter = RESET;

	RETURN status;
}

void GBA_t::busCycles()
{
	cpuTick();
}

void GBA_t::cpuIdleCycles()
{
	// Run DMA if any channel is active
	if (IsAnyDMARunning() == YES)
	{
		dmaTick();
	}

	auto& ticks = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate;

	// Note that below method of using "free bus cycles" is inspired from NBA

	// Refill free internal cycles using DMA counter
	ticks.freeBusCyclesCounter += ticks.dmaCounter;
	ticks.dmaCounter = RESET;

	if (ticks.freeBusCyclesCounter <= RESET)
	{
		ticks.freeBusCyclesCounter = RESET;
		busCycles();
	}
	else
	{
		--ticks.freeBusCyclesCounter;
	}

	// Next access after internal cycle is always non-sequential
	// Ref: https://discord.com/channels/465585922579103744/465586361731121162/1269384605136322571
	pGBA_memory->setNextMemoryAccessType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
}

void GBA_t::fetchAndDecode(uint32_t newPC)
{
	if (getARMState() == STATE_TYPE::ST_THUMB)
	{
		// New PC is being loaded, so we have the flush the contents of pipeline and reload all the stages again
		pGBA_cpuInstance->registers.pc = (newPC & 0xFFFFFFFE);

		// Stage 1:
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_HALFWORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU, MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE);
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		// Stage 2:
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_HALFWORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU, MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE);
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		// Note: We still don't have the valid instruction in "executeStageOpCode"... for this, we need one more cycle i.e. Stage 3
		// We don't do Stage 3 here because by default, one cycle is always executed as part of of "runCPUPipeline"... so this can be considered Stage 3

		CPUINFO("[THUMB] Filling the instruction pipeline: {DECODE} 0x%04X: [0x%04X] | {FETCH} 0x%04X: [0x%04X]",
			pGBA_cpuInstance->registers.pc - TWO,
			pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode,
			pGBA_cpuInstance->registers.pc,
			pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode)
	}
	else if (getARMState() == STATE_TYPE::ST_ARM)
	{
		// New PC is being loaded, so we have the flush the contents of pipeline and reload all the stages again
		pGBA_cpuInstance->registers.pc = (newPC & 0xFFFFFFFC);

		// Stage 1:
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_WORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU, MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE);
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		// Stage 2:
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_WORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU, MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE);
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		// Note: We still don't have the valid instruction in "executeStageOpCode"... for this, we need one more cycle i.e. Stage 3
		// We don't do Stage 3 here because by default, one cycle is always executed as part of of "runCPUPipeline"... so this can be considered Stage 3

		CPUINFO("[ARM] Filling the instruction pipeline: {DECODE} 0x%08X: [0x%08X] | {FETCH} 0x%08X: [0x%08X]",
			pGBA_cpuInstance->registers.pc - FOUR,
			pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode,
			pGBA_cpuInstance->registers.pc,
			pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode)
	}
	else
	{
		FATAL("Unknown Operating Mode");
	}
}

void GBA_t::runCPUPipeline()
{
#if _DEBUG
	// NOTE: 15006 in masquerade-gba is 17049 in nba before internal cycles were added for arm.gba (BIOS BYPASSED)
	// NOTE: 14412 in masquerade-gba is 27309 in nba for IE.gba (BIOS BYPASSED)
	// NOTE: 707850 in masquerade-gba is 675687 in nba for BIOS (BEFORE HALT IMPLEMENTATION)
	// NOTE: 829250 in masquerade-gba is 797084 in nba for BIOS (BEFORE HALT IMPLEMENTATION)
	// NOTE: 832893 in masquerade-gba is 800727 in nba for BIOS (BEFORE HALT IMPLEMENTATION)
	// NOTE: 832975 in masquerade-gba is 800809 in nba for BIOS (BEFORE HALT IMPLEMENTATION)
	// NOTE: 977672 in masquerade-gba is APPROXIMATELY 800813 in nba for BIOS (BEFORE HALT IMPLEMENTATION)
	// NOTE: 977688 in masquerade-gba is 800829 in nba for BIOS (BEFORE HALT IMPLEMENTATION)
	// NOTE: 978688 in masquerade-gba is 801828 in nba for BIOS (BEFORE HALT IMPLEMENTATION)
	// NOTE: VBLANK interrupts in nba @ (BEFORE HALT IMPLEMENTATION)	:807860	 -> 811373	-> 814886  -> 818399  -> 821912	 -> 825436
	// NOTE: VBLANK interrupts in masquerade-gba @ 						:1273235 -> 1421016 -> 1568797 -> 1716580 -> 1864361 -> 2012143

	// NOTE: 7324044 in masquerade-gba is 7445797 in nba for BIOS with HALT implemented (BIOS -> ROM)

	static COUNTER64 threshold = UINT64_MAX;

	if (emulationCounter[ONE] == threshold)
	{
		volatile FLAG breakpoint0 = 1;
	}

	if (emulationCounter[ONE] > threshold && pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode == 0xe8bd4010)
	{
		volatile FLAG breakpoint1 = 1;
	}

#if (DISABLED)
	if (emulationCounter[ONE] < threshold - 500)
	{
		;
	}
	else
	{
		;
	}
#endif
#endif
	if (pGBA_cpuInstance->registers.pc >= GAMEPAK_ROM_WS0_START_ADDRESS)
	{
		pGBA_instance->GBA_state.emulatorStatus.isBiosExecutionDone = YES;
	}

	CPUINFRA("[Loop 1]: %" PRId64, emulationCounter[ONE]);
	++emulationCounter[ONE];

	handleInterruptsIfApplicable();

	CPUDEBUG("r0:  0x%08X   r1: 0x%08X   r2: 0x%08X   r3: 0x%08X", cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_0), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_1), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_2), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_3));
	CPUDEBUG("r4:  0x%08X   r5: 0x%08X   r6: 0x%08X   r7: 0x%08X", cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_4), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_5), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_6), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_7));
	CPUDEBUG("r8:  0x%08X   r9: 0x%08X  r10: 0x%08X  r11: 0x%08X", cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_8), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_9), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_10), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_11));
	CPUDEBUG("r12: 0x%08X  r13: 0x%08X  r14: 0x%08X  r15: 0x%08X", cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_12), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_13), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_14), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_15));
#if (DISABLED)
	CPUDEBUG("spsr [USR/SYS]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_USR_SYS, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [FIQ]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_FIQ, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [IRQ]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_IRQ, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [SVC]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_SVC, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [ABT]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_ABT, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [UND]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_UND, (REGISTER_TYPE)SPSR));
#endif
	psr_t currentCPSR = { ZERO };
	currentCPSR.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR);
	CPUDEBUG("cpsr: 0x%08X [%s%s%s%s%s%s%s]", currentCPSR.psrMemory, CPSR_FLAG(currentCPSR.psrFields.psrNegativeBit, "N"), CPSR_FLAG(currentCPSR.psrFields.psrZeroBit, "Z"),
		CPSR_FLAG(currentCPSR.psrFields.psrCarryBorrowExtBit, "C"), CPSR_FLAG(currentCPSR.psrFields.psrOverflowBit, "V"), CPSR_FLAG(currentCPSR.psrFields.psrIRQDisBit, "I"),
		CPSR_FLAG(currentCPSR.psrFields.psrFIQDisBit, "F"), CPSR_FLAG(currentCPSR.psrFields.psrStateBit, "T"));

#if (GBA_ENABLE_AGS_PATCHED_TEST == YES)
	static const std::string chk1 = "TCHK01–";
	static const std::string chk2 = "AGB CHECKER";
	static const std::string test1 = reinterpret_cast<char*>(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.gameCode);
	static const std::string test2 = reinterpret_cast<char*>(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.gametitle);
	if (
		(!test1.compare(chk1))
		&&
		(!test2.compare(chk2))
		&&
		(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.softwareVersion == SIXTEEN)
		)
	{
#if (DISABLED)
		static FLAG openFileDone = false;
		static std::ofstream out;
		if (openFileDone == false)
		{
			out.open("AGS_LOGs.txt");
			openFileDone = true;
		}
#endif
		if (pGBA_instance->GBA_state.emulatorStatus.isBiosExecutionDone == YES && ((emulationCounter[ONE] % 10000) == ZERO))
		{
			pGBA_instance->GBA_state.emulatorStatus.debugger.agbReturn = readRawMemory<GBA_HALFWORD>(0x0004, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::HOST);
			INFO("ERR : %d", pGBA_instance->GBA_state.emulatorStatus.debugger.agbReturn);
			//out << "ERR : " << (GBA_HALFWORD)pGBA_instance->GBA_state.emulatorStatus.debugger.agbReturn << std::endl;
		}
	}
#endif

	if (getARMState() == STATE_TYPE::ST_THUMB)
	{
		// PC increment OR Stage 3 (Note that we have 3 stage pipeline of which 2 stages are taken care in "fetchAndDecode" and last remaining stage is taken care below):
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		// NOTE: PC always point to fetchStageOpCode
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_HALFWORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU);

		uint32_t extension1 = TO_UINT32(11 - OP_MODE_NAMES[currentCPSR.psrFields.psrModeBits].length());
		DISASSEMBLY("[THUMB] [%s] %*c 0x%08X : [0x%04X]     [%-23s]", OP_MODE_NAMES[currentCPSR.psrFields.psrModeBits].c_str(), extension1, ' ', (GBA_WORD)(pGBA_cpuInstance->registers.pc - FOUR), pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode, disassembled.c_str());

		// Now, since all the pipelining is handled for this run, proceed to execute the opcode...

		if (ThumbSoftwareInterrupt() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (UnconditionalBranch() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (ConditionalBranch() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (MultipleLoadStore() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LongBranchWithLink() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (AddOffsetToStackPointer() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (PushPopRegisters() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreHalfword() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (SPRelativeLoadStore() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadAddress() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreWithImmediateOffset() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreWithRegisterOffset() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreSignExtendedByteHalfword() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (PCRelativeLoad() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (HiRegisterOperationsBranchExchange() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (ALUOperations() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (MoveCompareAddSubtractImmediate() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (AddSubtract() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (MoveShiftedRegister() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}

		FATAL("Unknown THUMB Instruction");
	}
	else if (getARMState() == STATE_TYPE::ST_ARM)
	{
		// PC increment OR Stage 3 (Note that we have 3 stage pipeline of which 2 stages are taken care in "fetchAndDecode" and last remaining stage is taken care below):
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		// NOTE: PC always point to fetchStageOpCode
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_WORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);

		uint32_t extension1 = TO_UINT32(11 - OP_MODE_NAMES[currentCPSR.psrFields.psrModeBits].length());
		DISASSEMBLY("[ARM]   [%s] %*c 0x%08X : [0x%08X] [%-23s]", OP_MODE_NAMES[currentCPSR.psrFields.psrModeBits].c_str(), extension1, ' ', (GBA_WORD)(pGBA_cpuInstance->registers.pc - EIGHT), pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode, disassembled.c_str());

		// Now, since all the pipelining is handled for this run, proceed to execute the opcode...

		if (didConditionalCheckPass(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.cond, pGBA_cpuInstance->registers.cpsr.psrMemory) == YES)
		{
			if (BranchAndBranchExchange() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (BlockDataTransfer() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (BranchAndBranchLink() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (SoftwareInterrupt() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (Undefined() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (SingleDataTransfer() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (SingleDataSwap() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (MultiplyAndMultiplyAccumulate() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (HalfWordDataTransfer() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (psrTransfer() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (DataProcessing() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}

			unimplementedInstruction();
			FATAL("Unknown ARM Instruction");
		}
		else
		{
			CPUINFO("Skipping instruction because condition %d was not met.", pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.cond);
			pGBA_cpuInstance->registers.pc += FOUR;
			pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;
		}
	}
	else
	{
		FATAL("Unknown Operating Mode");
	}

	ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
}

void GBA_t::skylersalehLogProcess()
{
#if (DEACTIVATED)
	if (skylersalehLogs == ENABLED)
	{
		static uint32_t registerID = ZERO;
		ENABLE_LOGS = RESET; // don't want debug prints, only the logger prints

		for (const auto e : { REGISTER_TYPE::RT_0, REGISTER_TYPE::RT_1, REGISTER_TYPE::RT_2, REGISTER_TYPE::RT_3, REGISTER_TYPE::RT_4, REGISTER_TYPE::RT_5, REGISTER_TYPE::RT_6, REGISTER_TYPE::RT_7, REGISTER_TYPE::RT_8, REGISTER_TYPE::RT_9, REGISTER_TYPE::RT_10, REGISTER_TYPE::RT_11, REGISTER_TYPE::RT_12, REGISTER_TYPE::RT_13, REGISTER_TYPE::RT_14, REGISTER_TYPE::RT_15, REGISTER_TYPE::RT_16, REGISTER_TYPE::RT_17 })
		{
			auto registerContent = cpuReadRegister(getCurrentlyValidRegisterBank(), e);
			LOG("R%d: 0x%X", registerID, registerContent);
			++registerID;
			if (registerID >= 18)
			{
				registerID = 0;
				LOG_NEW_LINE;
			}
			skylersalehLogs_BUFFER.push_back(registerContent);
		}

		if (emulationCounter[ZERO] == 100)
		{
			std::ofstream output_file("./extracted-underTest.txt");
			std::ostream_iterator<uint32_t> output_iterator(output_file, "");
			std::copy(skylersalehLogs_BUFFER.begin(), skylersalehLogs_BUFFER.end(), output_iterator);

			volatile FLAG pause = 1;
		}
	}
#endif
}

void GBA_t::dumpCpuStateToConsole()
{
	LOG_NEW_LINE;
	LOG("------------------------------------------------------------");

	/* R0–R7 */
	for (uint8_t r = ZERO; r < LO_GP_REGISTERS; r++)
	{
		LOG("BANK NA R%-2u %-10s : 0x%08X",
			r,
			"",
			pGBA_instance->GBA_state.cpuInstance.registers.unbankedLORegisters[r]);
	}

	/* Banked registers */
	for (uint8_t bank = ZERO; bank < REGISTER_BANKS; bank++)
	{
		for (uint8_t jj = ZERO; jj < HI_GP_REGISTERS; jj++)
		{
			uint8_t reg = LO_GP_REGISTERS + jj;
			const char* tag = "";

			if (reg == SP)      tag = "(SP)";
			else if (reg == LR) tag = "(LR)";
			else if (reg == PC) tag = "(PC)";

			LOG("BANK %-2u R%-2u %-10s : 0x%08X",
				bank,
				reg,
				tag,
				pGBA_instance->GBA_state.cpuInstance.registers.bankedHIRegisters[bank][jj]);
		}

		LOG("BANK %-2u %-14s : 0x%08X",
			bank,
			"(SPSR)",
			pGBA_instance->GBA_state.cpuInstance.registers.spsr[bank].psrMemory);
	}

	/* CPSR */
	LOG("BANK NA %-14s : 0x%08X",
		"(CPSR)",
		pGBA_instance->GBA_state.cpuInstance.registers.cpsr.psrMemory);

	LOG("------------------------------------------------------------");
}

void GBA_t::unimplementedInstruction()
{
	LOG_NEW_LINE;
	LOG("CPU Panic; unknown opcode! %02X", pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode);
	dumpCpuStateToConsole();
	FATAL("Unknown Opcode");
}
#pragma endregion ARM7TDMI_DEFINITIONS

#pragma region EMULATION_DEFINITIONS

#pragma region CYCLE_ACCURATE

void GBA_t::dmaTick()
{
	processDMA();
}

void GBA_t::timerTick()
{
	processTimer(ONE);
}

void GBA_t::serialTick()
{
	processSIO(ONE);
}

void GBA_t::apuTick()
{
	processAPU(ONE);
}

void GBA_t::ppuTick()
{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_TICK == YES)
	processPPU(ONE);
#endif
}
#pragma endregion CYCLE_ACCURATE

#pragma region CYCLE_COUNT_ACCURATE
#pragma endregion CYCLE_COUNT_ACCURATE

void GBA_t::requestInterrupts(GBA_INTERRUPT interrupt)
{
#if (GBA_ENABLE_DELAYED_MMIO_WRITE == YES)
	pGBA_instance->GBA_state.irqPend = YES;
	pGBA_instance->GBA_state.interrupt.irqQ = (uint16_t)interrupt;
#else
	ifRegUpdate((uint16_t)interrupt);
#endif
}

FLAG GBA_t::shouldUnHaltTheCPU()
{
	if (((pGBA_peripherals->mIFHalfWord.mIFHalfWord & pGBA_peripherals->mIEHalfWord.mIEHalfWord & 0x3FFF) != ZERO)
		&& (pGBA_instance->GBA_state.interrupt.syncDelay == RESET))
	{
		RETURN YES;
	}

	RETURN NO;
}

FLAG GBA_t::isInterruptReadyToBeServed()
{
	if (((pGBA_peripherals->mIFHalfWord.mIFHalfWord & pGBA_peripherals->mIEHalfWord.mIEHalfWord & 0x3FFF) != ZERO)
		&& (pGBA_instance->GBA_state.interrupt.syncDelay == RESET))
	{
		RETURN YES;
	}

	RETURN NO;
}

FLAG GBA_t::handleInterruptsIfApplicable()
{
	FLAG interruptWasServiced = NO;

	if (pGBA_peripherals->mIMEHalfWord.mIMEFields.ENABLE_ALL_INTERRUPTS == ENABLED
		&& pGBA_registers->cpsr.psrFields.psrIRQDisBit == RESET
		&& isInterruptReadyToBeServed() == YES)
	{
		// Refer : http://problemkaputt.de/gbatek-arm-cpu-exceptions.htm for details on steps to be taken when interrupt occurs

		// Make sure Halt if cleared (It will be cleared...but just making sure)

		pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::RUN;

		// Save CPSR to SPSR.IRQ

		psr_t cpsr = { ZERO };
		cpsr.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR);
		cpuSetRegister(REGISTER_BANK_TYPE::RB_IRQ, (REGISTER_TYPE)SPSR, getARMState(), cpsr.psrMemory);

		// Switch to IRQ mode as CPSR is saved now

		setARMMode(OP_MODE_TYPE::OP_IRQ);

		// Disable IRQs

		pGBA_registers->cpsr.psrFields.psrIRQDisBit = SET; // https://gbadev.net/tonc/interrupts.html

		// Idle Cycles...

		cpuIdleCycles();

		// Switch to ARM state (if in thumb)

		setARMState(STATE_TYPE::ST_ARM);

		// Save PC in LR
		auto currentPC = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
		if ((STATE_TYPE)cpsr.psrFields.psrStateBit == STATE_TYPE::ST_THUMB)
		{
			// Refer to "INFORMATION_002" to understand why we store PC instead of PC - 4 in Thumb state
			currentPC = currentPC;
		}
		else
		{
			// Refer to "INFORMATION_002" to understand why we store PC - 4 instead of PC - 8 in ARM state
			currentPC = currentPC - FOUR;
		}
		cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)LR, getARMState(), currentPC);

		// Set PC to IRQ exception vector
		cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), 0x18);

		interruptWasServiced = YES;
	}

	RETURN interruptWasServiced;
}

void GBA_t::handleKeypadInterrupts()
{
	if (pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTFields.BUTTON_IRQ_EN == SET
		// Keypad interrupts are used to come out of STOP mode...
		|| pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::STOP)
	{
		if (pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTFields.BUTTON_IRQ_CONDITION == SET)
		{
			// Logical AND mode
			if
				(
					(
						((~(pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTHalfWord & 0x03FF)) & 0x03FF)
						==
						(pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTHalfWord & 0x03FF)
						)
					)
			{
				requestInterrupts(GBA_INTERRUPT::IRQ_KEYPAD);
			}
		}
		else
		{
			// Logical OR mode
			if
				(
					(
						((~(pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTHalfWord & 0x03FF)) & 0x03FF)
						&
						(pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTHalfWord & 0x03FF)
						) != ZERO
					)
			{
				requestInterrupts(GBA_INTERRUPT::IRQ_KEYPAD);
			}
		}

		if (pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::STOP)
		{
			pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::RUN;
		}
	}
}

void GBA_t::captureIO()
{
	// Refer http://problemkaputt.de/gbatek-gba-keypad-input.htm

	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.START = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_Enter) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.SELECT = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_Space) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);

	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.BUTTON_A = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_Z) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.BUTTON_B = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_X) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);

	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.LEFT = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_LeftArrow) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.RIGHT = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_RightArrow) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.UP = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_UpArrow) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.DOWN = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_DownArrow) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);

	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.BUTTON_R = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_S) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.BUTTON_L = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_A) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);

	handleKeypadInterrupts();
}

void GBA_t::processTimer(INC64 timerCycles)
{
	// Cache pointers
	static mTIMERnCNT_HHalfWord_t* const CNTHLUT[4] = {
		&pGBA_peripherals->mTIMER0CNT_H,
		&pGBA_peripherals->mTIMER1CNT_H,
		&pGBA_peripherals->mTIMER2CNT_H,
		&pGBA_peripherals->mTIMER3CNT_H
	};

	auto* timers = pGBA_instance->GBA_state.timer;

	// Reset cascade events
	timers[0].cascadeEvents = RESET;
	timers[1].cascadeEvents = RESET;
	timers[2].cascadeEvents = RESET;
	timers[3].cascadeEvents = RESET;

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.globalTimerCounter += timerCycles;
	const auto systemTimer = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.globalTimerCounter;

	// Unroll timer loop for performance
	for (INC8 timerID = ZERO; timerID < FOUR; timerID++)
	{
		mTIMERnCNT_HHalfWord_t* const CNTH = CNTHLUT[timerID];

		if (CNTH->mTIMERnCNT_HFields.TIMER_START_STOP != SET) MASQ_UNLIKELY
			CONTINUE;

		auto& timer = timers[timerID];
		const uint16_t CNTL = timer.cache.reload;

		if (timer.startupDelay > RESET) MASQ_UNLIKELY
		{
			timer.startupDelay -= timerCycles;
			CONTINUE;
		}

		timer.startupDelay = RESET;
		timer.currentState = ENABLED;

		const FLAG isCountUpMode = (CNTH->mTIMERnCNT_HFields.COUNT_UP_TIMING == SET && timerID != TIMER::TIMER0) ? YES : NO;

		if (isCountUpMode == NO) MASQ_LIKELY
		{
			const auto prescalar = timerFrequency[CNTH->mTIMERnCNT_HFields.PRESCALER_SEL];
			const auto moduloPow2Prescalar = prescalar - ONE;

			if ((systemTimer & moduloPow2Prescalar) == ZERO) MASQ_LIKELY
			{
				timerCommonProcessing((TIMER)timerID, CNTL, CNTH, timerCycles);
			}
		}
		else
		{
			auto& prevCascade = timers[timerID - ONE].cascadeEvents;
			while (prevCascade > ZERO)
			{
				timerCommonProcessing((TIMER)timerID, CNTL, CNTH, timerCycles);
				--prevCascade;
			}
		}
	}
}

GBA_WORD GBA_t::getDMASADRegister(DMA dma)
{
	GBA_WORD DMAxSAD = RESET;
	if (dma == DMA::DMA0)
	{
		DMAxSAD = (pGBA_peripherals->mDMA0SAD_L | (pGBA_peripherals->mDMA0SAD_H << SIXTEEN));
		RETURN (DMAxSAD & 0x07FFFFFF);
	}
	if (dma == DMA::DMA1)
	{
		DMAxSAD = (pGBA_peripherals->mDMA1SAD_L | (pGBA_peripherals->mDMA1SAD_H << SIXTEEN));
		RETURN (DMAxSAD & 0x0FFFFFFF);
	}
	if (dma == DMA::DMA2)
	{
		DMAxSAD = (pGBA_peripherals->mDMA2SAD_L | (pGBA_peripherals->mDMA2SAD_H << SIXTEEN));
		RETURN (DMAxSAD & 0x0FFFFFFF);
	}
	if (dma == DMA::DMA3)
	{
		DMAxSAD = (pGBA_peripherals->mDMA3SAD_L | (pGBA_peripherals->mDMA3SAD_H << SIXTEEN));
		RETURN (DMAxSAD & 0x0FFFFFFF);
	}

	FATAL("Unknown DMA");
	RETURN (GBA_WORD)NULL;
}

GBA_WORD GBA_t::getDMADADRegister(DMA dma)
{
	GBA_WORD DMAxDAD = RESET;
	if (dma == DMA::DMA0)
	{
		DMAxDAD = (pGBA_peripherals->mDMA0DAD_L | (pGBA_peripherals->mDMA0DAD_H << SIXTEEN));
		RETURN (DMAxDAD & 0x07FFFFFF);
	}
	if (dma == DMA::DMA1)
	{
		DMAxDAD = (pGBA_peripherals->mDMA1DAD_L | (pGBA_peripherals->mDMA1DAD_H << SIXTEEN));
		RETURN (DMAxDAD & 0x07FFFFFF);
	}
	if (dma == DMA::DMA2)
	{
		DMAxDAD = (pGBA_peripherals->mDMA2DAD_L | (pGBA_peripherals->mDMA2DAD_H << SIXTEEN));
		RETURN (DMAxDAD & 0x07FFFFFF);
	}
	if (dma == DMA::DMA3)
	{
		DMAxDAD = (pGBA_peripherals->mDMA3DAD_L | (pGBA_peripherals->mDMA3DAD_H << SIXTEEN));
		RETURN (DMAxDAD & 0x0FFFFFFF);
	}

	FATAL("Unknown DMA");
	RETURN (GBA_WORD)NULL;
}

GBA_HALFWORD GBA_t::getDMACNTLRegister(DMA dma)
{
	if (dma == DMA::DMA0)
	{
		RETURN (pGBA_peripherals->mDMA0CNT_L & 0x3FFF);
	}
	if (dma == DMA::DMA1)
	{
		RETURN (pGBA_peripherals->mDMA1CNT_L & 0x3FFF);
	}
	if (dma == DMA::DMA2)
	{
		RETURN (pGBA_peripherals->mDMA2CNT_L & 0x3FFF);
	}
	if (dma == DMA::DMA3)
	{
		RETURN (pGBA_peripherals->mDMA3CNT_L & 0xFFFF);
	}

	FATAL("Unknown DMA");
	RETURN (GBA_HALFWORD)NULL;
}

GBA_t::mDMAnCNT_HHalfWord_t* GBA_t::getDMACNTHRegister(DMA dma)
{
	if (dma == DMA::DMA0)
	{
		RETURN &pGBA_peripherals->mDMA0CNT_H;
	}
	if (dma == DMA::DMA1)
	{
		RETURN &pGBA_peripherals->mDMA1CNT_H;
	}
	if (dma == DMA::DMA2)
	{
		RETURN &pGBA_peripherals->mDMA2CNT_H;
	}
	if (dma == DMA::DMA3)
	{
		RETURN &pGBA_peripherals->mDMA3CNT_H;
	}

	FATAL("Unknown DMA");
	RETURN ((mDMAnCNT_HHalfWord_t*)nullptr);
}

void GBA_t::setDMASADRegister(DMA dma, GBA_WORD data)
{
	if (dma == DMA::DMA0)
	{
		pGBA_peripherals->mDMA0SAD_L = data & 0xFFFF;
		pGBA_peripherals->mDMA0SAD_H = (data >> SIXTEEN) & 0xFFFF;
	}
	if (dma == DMA::DMA1)
	{
		pGBA_peripherals->mDMA1SAD_L = data & 0xFFFF;
		pGBA_peripherals->mDMA1SAD_H = (data >> SIXTEEN) & 0xFFFF;
	}
	if (dma == DMA::DMA2)
	{
		pGBA_peripherals->mDMA2SAD_L = data & 0xFFFF;
		pGBA_peripherals->mDMA2SAD_H = (data >> SIXTEEN) & 0xFFFF;
	}
	if (dma == DMA::DMA3)
	{
		pGBA_peripherals->mDMA3SAD_L = data & 0xFFFF;
		pGBA_peripherals->mDMA3SAD_H = (data >> SIXTEEN) & 0xFFFF;
	}
}

void GBA_t::setDMADADRegister(DMA dma, GBA_WORD data)
{
	if (dma == DMA::DMA0)
	{
		pGBA_peripherals->mDMA0DAD_L = data & 0xFFFF;
		pGBA_peripherals->mDMA0DAD_H = (data >> SIXTEEN) & 0xFFFF;
	}
	if (dma == DMA::DMA1)
	{
		pGBA_peripherals->mDMA1DAD_L = data & 0xFFFF;
		pGBA_peripherals->mDMA1DAD_H = (data >> SIXTEEN) & 0xFFFF;
	}
	if (dma == DMA::DMA2)
	{
		pGBA_peripherals->mDMA2DAD_L = data & 0xFFFF;
		pGBA_peripherals->mDMA2DAD_H = (data >> SIXTEEN) & 0xFFFF;
	}
	if (dma == DMA::DMA3)
	{
		pGBA_peripherals->mDMA3DAD_L = data & 0xFFFF;
		pGBA_peripherals->mDMA3DAD_H = (data >> SIXTEEN) & 0xFFFF;
	}
}

void GBA_t::setDMACNTLRegister(DMA dma, GBA_HALFWORD data)
{
	if (dma == DMA::DMA0)
	{
		pGBA_peripherals->mDMA0CNT_L = data;
	}
	if (dma == DMA::DMA1)
	{
		pGBA_peripherals->mDMA1CNT_L = data;
	}
	if (dma == DMA::DMA2)
	{
		pGBA_peripherals->mDMA2CNT_L = data;
	}
	if (dma == DMA::DMA3)
	{
		pGBA_peripherals->mDMA3CNT_L = data;
	}
}

void GBA_t::latchDMARegisters(ID dmaID)
{
	pGBA_instance->GBA_state.dma.cache[dmaID].source = (getDMASADRegister((DMA)dmaID) & 0x0FFFFFFF);
	pGBA_instance->GBA_state.dma.cache[dmaID].destination = (getDMADADRegister((DMA)dmaID) & 0x0FFFFFFF);
	pGBA_instance->GBA_state.dma.cache[dmaID].length = (getDMACNTLRegister((DMA)dmaID) & 0xFFFF);

	// Check if FIFO DMA

	mDMAnCNT_HHalfWord_t* CNTH = getDMACNTHRegister((DMA)dmaID);

	if ((((DMA_TIMING)CNTH->mDMAnCNT_HFields.DMA_START_TIMING) == DMA_TIMING::SPECIAL)
		&& ((((DMA)dmaID) == DMA::DMA1) || (((DMA)dmaID) == DMA::DMA2)))
	{
		pGBA_instance->GBA_state.dma.cache[dmaID].isFIFODMA = YES;

		pGBA_instance->GBA_state.dma.cache[dmaID].chunkSize = DMA_SIZE::WORD_PER_TRANSFER;	// Refer Sound DMA in http://problemkaputt.de/gbatek-gba-dma-transfers.htm

		// Refer : https://discord.com/channels/465585922579103744/465586361731121162/1217591657109782628
		// DMA force aligns the address before it goes on bus

		int32_t mask = ~THREE;
		pGBA_instance->GBA_state.dma.cache[dmaID].source &= mask;
		pGBA_instance->GBA_state.dma.cache[dmaID].destination &= mask;

		pGBA_instance->GBA_state.dma.cache[dmaID].length = FOUR;	// Refer Sound DMA in http://problemkaputt.de/gbatek-gba-dma-transfers.htm
	}
	else
	{
		pGBA_instance->GBA_state.dma.cache[dmaID].isFIFODMA = NO;

		pGBA_instance->GBA_state.dma.cache[dmaID].chunkSize = (DMA_SIZE)(CNTH->mDMAnCNT_HFields.DMA_TRANSFER_TYPE);

		// Align the address based on chunk size

		if (pGBA_instance->GBA_state.dma.cache[dmaID].chunkSize == DMA_SIZE::HALFWORD_PER_TRANSFER)
		{
			// Refer https://discord.com/channels/465585922579103744/465586361731121162/1217591657109782628
			// Needed to pass the suite.gba's memory tests
			// DMA force aligns the address before it goes on bus

			int32_t mask = ~ONE;
			pGBA_instance->GBA_state.dma.cache[dmaID].source &= mask;
			pGBA_instance->GBA_state.dma.cache[dmaID].destination &= mask;
		}
		else if (pGBA_instance->GBA_state.dma.cache[dmaID].chunkSize == DMA_SIZE::WORD_PER_TRANSFER)
		{
			// Refer : https://discord.com/channels/465585922579103744/465586361731121162/1217591657109782628
			// Needed to pass the suite.gba's memory tests
			// DMA force aligns the address before it goes on bus

			int32_t mask = ~THREE;
			pGBA_instance->GBA_state.dma.cache[dmaID].source &= mask;
			pGBA_instance->GBA_state.dma.cache[dmaID].destination &= mask;
		}

		// Length of zero is treated as max values; Refer : http://problemkaputt.de/gbatek-gba-dma-transfers.htm

		if (pGBA_instance->GBA_state.dma.cache[dmaID].length == ZERO)
		{
			if (((DMA)dmaID) == DMA::DMA3)
			{
				pGBA_instance->GBA_state.dma.cache[dmaID].length = 0x10000;
			}
			else
			{
				pGBA_instance->GBA_state.dma.cache[dmaID].length = 0x4000;
			}
		}
	}
}

void GBA_t::OnDMAChannelWritten(DMA dmaID, FLAG oldEnable, FLAG newEnable)
{
	mDMAnCNT_HHalfWord_t* CNTH = getDMACNTHRegister(dmaID);
	auto& cache = pGBA_instance->GBA_state.dma.cache[dmaID];

	// Remove from all trigger maps
	for (int timing = DMA_TIMING::IMMEDIATE; timing < DMA_TIMING::TOTAL_DMA_TIMING; timing++)
	{
		UNSETBIT(pGBA_instance->GBA_state.dma.trigCache[timing].dmaIdMap, dmaID);
	}

	if (newEnable == SET)
	{
		cache.scheduleType = (DMA_TIMING)CNTH->mDMAnCNT_HFields.DMA_START_TIMING;

		if (oldEnable == RESET)
		{
			// 0->1: Latch registers
			latchDMARegisters(dmaID);
			cache.count = ZERO;
			cache.target = cache.length;
			cache.currentState = YES;
			cache.didAccessRom = NO;
		}
		else
		{
			// 1->1: If modifying currently active DMA, trigger re-enter
			if (pGBA_instance->GBA_state.dma.currentlyActiveDMA == dmaID)
			{
				pGBA_instance->GBA_state.dma.shouldReenterTransferLoop = YES;
			}
		}

		// Add to trigger map
		SETBIT(pGBA_instance->GBA_state.dma.trigCache[cache.scheduleType].dmaIdMap, dmaID);

		// Activate if IMMEDIATE
		if (cache.scheduleType == DMA_TIMING::IMMEDIATE)
		{
			ActivateDMAChannel(dmaID);
		}
	}
	else
	{
		// Disable
		cache.currentState = NO;
		UNSETBIT(pGBA_instance->GBA_state.dma.runnableSet, dmaID);

		// Remove from all trigger maps (already done above, but keeping for clarity)
		for (int timing = DMA_TIMING::IMMEDIATE; timing < DMA_TIMING::TOTAL_DMA_TIMING; timing++)
		{
			UNSETBIT(pGBA_instance->GBA_state.dma.trigCache[timing].dmaIdMap, dmaID);
		}

		if (pGBA_instance->GBA_state.dma.currentlyActiveDMA == dmaID)
		{
			pGBA_instance->GBA_state.dma.shouldReenterTransferLoop = YES;
			SelectNextDMA();
		}
	}
}

void GBA_t::ActivateDMAChannel(ID dmaID)
{
	auto& dmaState = pGBA_instance->GBA_state.dma;

	//INFO("ActivateDMAChannel(%d): runnableSet=0x%02X, currentActive=%d",
	//	dmaID, dmaState.runnableSet, dmaState.currentlyActiveDMA);

	if (dmaState.runnableSet == RESET)
	{
		dmaState.currentlyActiveDMA = (DMA)dmaID;
		//INFO("  First DMA, set currentActive=%d", dmaID);
	}
	else if (dmaID < dmaState.currentlyActiveDMA)
	{
		//INFO("  Preempting DMA%d with DMA%d", dmaState.currentlyActiveDMA, dmaID);
		dmaState.currentlyActiveDMA = (DMA)dmaID;
		dmaState.shouldReenterTransferLoop = YES;
	}

	SETBIT(dmaState.runnableSet, dmaID);
}

void GBA_t::RequestDMA(DMA_TIMING timing)
{
	MAP8 dmaMap = pGBA_instance->GBA_state.dma.trigCache[timing].dmaIdMap;

	for (ID dmaID = DMA::DMA0; dmaID < DMA::TOTAL_DMA; dmaID++)
	{
		if (GETBIT(dmaID, dmaMap))
		{
			ActivateDMAChannel(dmaID);
		}
	}
}

void GBA_t::OnDMAActivated(ID dmaID)
{
	auto& dmaState = pGBA_instance->GBA_state.dma;

	if (dmaState.runnableSet == RESET)
	{
		// First DMA to activate
		dmaState.currentlyActiveDMA = (DMA)dmaID;
	}
	else if (dmaID < dmaState.currentlyActiveDMA)
	{
		// Higher priority DMA - preempt
		dmaState.currentlyActiveDMA = (DMA)dmaID;
		dmaState.shouldReenterTransferLoop = YES;
	}

	SETBIT(dmaState.runnableSet, dmaID);
}

void GBA_t::SelectNextDMA()
{
	auto& dmaState = pGBA_instance->GBA_state.dma;

	dmaState.currentlyActiveDMA = DMA::NO_DMA;

	for (ID dmaID = DMA::DMA0; dmaID < DMA::TOTAL_DMA; dmaID++)
	{
		if (GETBIT(dmaID, dmaState.runnableSet) == SET)
		{
			dmaState.currentlyActiveDMA = (DMA)dmaID;
			BREAK;
		}
	}
}

FLAG GBA_t::IsAnyDMARunning()
{
	RETURN (pGBA_instance->GBA_state.dma.runnableSet != RESET);
}

void GBA_t::processDMA()
{
	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter = RESET;

	if (IsAnyDMARunning() == NO)
	{
		RETURN;
	}

	cpuTick(TICK_TYPE::DMA_TICK);

	do
	{
		RunDMAChannel();
	}
	while (IsAnyDMARunning());

	cpuTick(TICK_TYPE::DMA_TICK);
}

void GBA_t::RunDMAChannel()
{
#define DMA_REENTER_EARLY_RETURN(dmaState)								\
	do																	\
	{																	\
		if ((dmaState).shouldReenterTransferLoop == YES) MASQ_UNLIKELY  \
		{																\
			(dmaState).shouldReenterTransferLoop = NO;					\
			RETURN;														\
		}																\
	} while (ZERO)

	auto& dmaState = pGBA_instance->GBA_state.dma;
	ID activeDMA = dmaState.currentlyActiveDMA;

	if (activeDMA >= DMA::TOTAL_DMA)
	{
		RETURN;
	}

	auto& cache = dmaState.cache[activeDMA];
	mDMAnCNT_HHalfWord_t* CNTH = getDMACNTHRegister((DMA)activeDMA);

	SBYTE srcModifier = sourceModifierLUT[CNTH->mDMAnCNT_HFields.DMA_TRANSFER_TYPE][CNTH->mDMAnCNT_HFields.SRC_ADDR_CTRL];
	SBYTE dstModifier = destinationModifierLUT[CNTH->mDMAnCNT_HFields.DMA_TRANSFER_TYPE][CNTH->mDMAnCNT_HFields.DEST_ADDR_CTRL];

	if (cache.isFIFODMA == YES)
	{
		dstModifier = ZERO;
	}

	while (cache.count < cache.target)
	{
		// Reason for calling this here : https://discord.com/channels/465585922579103744/465586361731121162/959447055967879218
		DMA_REENTER_EARLY_RETURN(dmaState);

		MEMORY_ACCESS_TYPE srcType = MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE;
		MEMORY_ACCESS_TYPE dstType = MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE;

		if (cache.didAccessRom == NO)
		{
			if (cache.source >= GAMEPAK_ROM_WS0_START_ADDRESS)
			{
				srcType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
				cache.didAccessRom = YES;
			}
			else if (cache.destination >= GAMEPAK_ROM_WS0_START_ADDRESS)
			{
				dstType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
				cache.didAccessRom = YES;
			}
		}

		if (cache.chunkSize == DMA_SIZE::HALFWORD_PER_TRANSFER)
		{
			if (cache.source >= EXT_WORK_RAM_START_ADDRESS)
			{
				cache.wordToBeTransfered = readRawMemory<GBA_HALFWORD>(cache.source, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::DMA, srcType);

				cache.latchedData = (cache.wordToBeTransfered << SIXTEEN) | cache.wordToBeTransfered;
			}
			else
			{
				cache.wordToBeTransfered = (cache.destination & TWO) ? (GBA_HALFWORD)(cache.latchedData >> SIXTEEN) : (GBA_HALFWORD)(cache.latchedData);
				cpuTick(TICK_TYPE::DMA_TICK);
			}

			writeRawMemory<GBA_HALFWORD>(cache.destination, cache.wordToBeTransfered, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::DMA, dstType);
		}
		else
		{
			if (cache.source >= EXT_WORK_RAM_START_ADDRESS)
			{
				cache.latchedData = readRawMemory<GBA_WORD>(cache.source, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::DMA, srcType);
			}
			else
			{
				cpuTick(TICK_TYPE::DMA_TICK);
			}

			writeRawMemory<GBA_WORD>(cache.destination, cache.latchedData, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::DMA, dstType);
		}

		cache.source += srcModifier;
		cache.destination += dstModifier;
		cache.length--;
		cache.count++;
	}

	// Completed
	UNSETBIT(dmaState.runnableSet, activeDMA);
	cache.count = ZERO;

	if (CNTH->mDMAnCNT_HFields.WORD_COUNT_END_IRQ == SET)
	{
		requestInterrupts((GBA_INTERRUPT)(activeDMA + TO_UINT(GBA_INTERRUPT::IRQ_DMA0)));
	}

	if (CNTH->mDMAnCNT_HFields.DMA_REPEAT == SET && CNTH->mDMAnCNT_HFields.DMA_START_TIMING != DMA_TIMING::IMMEDIATE)
	{
		if (CNTH->mDMAnCNT_HFields.DEST_ADDR_CTRL == THREE)
		{
			cache.destination = getDMADADRegister((DMA)activeDMA);
		}

		if (cache.isFIFODMA == YES)
		{
			cache.length = FOUR;
		}
		else
		{
			cache.length = getDMACNTLRegister((DMA)activeDMA) & 0xFFFF;
			if (cache.length == ZERO)
			{
				cache.length = (activeDMA == DMA::DMA3) ? 0x10000 : 0x4000;
			}
		}

		cache.target = cache.length;
		SETBIT(pGBA_instance->GBA_state.dma.trigCache[CNTH->mDMAnCNT_HFields.DMA_START_TIMING].dmaIdMap, activeDMA);
	}
	else
	{
		CNTH->mDMAnCNT_HFields.DMA_EN = RESET;
		cache.currentState = NO;

		// Remove from all trigger maps
		for (int timing = DMA_TIMING::IMMEDIATE; timing < DMA_TIMING::TOTAL_DMA_TIMING; timing++)
		{
			UNSETBIT(pGBA_instance->GBA_state.dma.trigCache[timing].dmaIdMap, activeDMA);
		}
	}

	SelectNextDMA();

#undef DMA_REENTER_EARLY_RETURN
}

void GBA_t::processAPU(INC64 apuCycles)
{
	auto& apuState = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate;

	apuState.apuCounter += apuCycles;

	// DAC check - if DAC disabled, channel disabled immediately
	continousDACCheck();

	// Tick all 4 channels
	tickChannel(AUDIO_CHANNELS::CHANNEL_1, apuCycles);
	tickChannel(AUDIO_CHANNELS::CHANNEL_2, apuCycles);
	tickChannel(AUDIO_CHANNELS::CHANNEL_3, apuCycles);
	tickChannel(AUDIO_CHANNELS::CHANNEL_4, apuCycles);

	// Frame sequencer @ 512 Hz
	static constexpr int64_t FRAME_SEQ_THRESHOLD = static_cast<int64_t>(GBA_REFERENCE_CLOCK_HZ / APU_FRAME_SEQUENCER_RATE_HZ);

	if (apuState.apuCounter >= FRAME_SEQ_THRESHOLD) MASQ_UNLIKELY
	{
		if (pGBA_instance->GBA_state.audio.wasPowerCycled == YES) MASQ_UNLIKELY
		{
			APUTODO("Not handling power cycling!");
			pGBA_instance->GBA_state.audio.wasPowerCycled = NO;
		}

		pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter = FALSE;

		// 256 Hz - every other frame
		if ((apuState.apuFrameCounter & ONE) == ZERO) MASQ_UNLIKELY
		{
			pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter = TRUE;
			processSoundLength();
		}

		// 128 Hz - every 4th frame at offset 2
		if ((apuState.apuFrameCounter & THREE) == TWO) MASQ_UNLIKELY
		{
			processFrequencySweep();
		}

		// 64 Hz - frame 7 only
		if (apuState.apuFrameCounter == SEVEN) MASQ_UNLIKELY
		{
			processEnvelopeSweep();
		}

		apuState.apuFrameCounter = (apuState.apuFrameCounter + ONE) & SEVEN;
		apuState.apuCounter -= FRAME_SEQ_THRESHOLD;
	}

	captureDownsampledAudioSamples(apuCycles);
}

void GBA_t::playTheAudioFrame()
{
	RETURN;
}

void GBA_t::processPPU(INC64 ppuCycles)
{
	if (ppuCycles == RESET) MASQ_UNLIKELY
	{
		RETURN;
	}

	// NOTE: (PPU:01) Refer to "Internal Reference Point Registers" section in http://problemkaputt.de/gbatek-lcd-i-o-bg-rotation-scaling.htm 
	// Reference points getting copied to internal registers can happen in 2 cases
	// 1) We are jumping to scanline zero
	// TWO) We wrote to BGX or BGY register from cpu (then we need to handle the copying irrespective of whether scanline is 0 or not)
	// Both the points are handled below
	//
	// NOTE: (PPU:02) Refer to "Internal Reference Point Registers" section in http://problemkaputt.de/gbatek-lcd-i-o-bg-rotation-scaling.htm 
	// Latched internal register are incremented by BG2/3PB and BG2/3PD
	// Note that point (2) of PPU:01 can override this increment (Source: NBA) 
	//
	// NOTE: (PPU:03) Refer to https://gbadev.net/tonc/affine.html
	// Affine transformation has following items as per TONC
	// 1) BGX and BGY Initial reference point per frame (or mid scanline because of point 2 of PPU:01)
	// TWO) pa : texture x - increment / pixel (Not directly added to internal reference point registers, refer below)
	// 3) pb : texture x - increment / scanline
	// 4) pc : texture y - increment / pixel (Not directly added to internal reference point registers, refer below)
	// 5) pd : texture y - increment / scanline
	//
	// Point (1) is done as part of PPU:01
	// Point (3) and (5) is done as part of PPU:02
	//
	// 6) IMPORTANT DETAILS:
	// If you read carefully in (PPU:01) and (PPU:02), the document mentions that the "Internal Reference Point" registers are added by ONLY BGX/BGY (per frame) and PB/PD (per scanline)
	// There is no mention of adding of PA and PC directly to "Internal Reference Point" registers
	// Basically during the start of scanline we latch the "Internal Reference Point"
	// Then as we proceed over the pixels of the particular scanline, we use the latched "Internal Reference Point" values and PA/PC, without modifying "Internal Reference Point" registers...
	// The "Affine Buffer" that this emulator maintains is to handle this pixel processing per scanline which involves using/incrementing the latched "Internal Reference Point" value by PA/PC without directly modifying "Internal Reference Point" registers
	// On every new scanline, we just add the PD/PB to "Internal Reference Point" registers and load it to the "Affine Buffer"
	// On every new frame, we just add BGX/BGY to "Internal Reference Point" registers and load it to the "Affine Buffer"

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter += ppuCycles;

	switch (pGBA_display->currentLCDMode)
	{
	case LCD_MODES::MODE_LCD_H_DRAW_V_DRAW: // H Draw V Draw
	{
		PROCESS_PPU_MODES(ppuCycles + pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange, YES, YES, YES, YES);
		pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = RESET;

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter >= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_VISIBLE)
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter -= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_VISIBLE;

			// Note: In process PPU, we should account for cycles which occur only in MODE_LCD_H_DRAW_V_DRAW
			ppuCycles -= pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter;

			pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_FLAG = SET;

			if (pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_IRQ_ENABLE == SET)
			{
				requestInterrupts(GBA_INTERRUPT::IRQ_HBLANK);
			}

			RequestDMA(DMA_TIMING::HBLANK);

			// Proceed to next lcd mode
			pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter;
			pGBA_display->didLCDModeChangeJustNow = YES;
			pGBA_display->currentLCDMode = LCD_MODES::MODE_LCD_H_BLANK_V_DRAW;
			PPUINFO("Entering MODE_LCD_H_BLANK_V_DRAW");
		}

		BREAK;
	}
	case LCD_MODES::MODE_LCD_H_BLANK_V_DRAW: // H Blank V Draw
	{
		// needed alteast for objects
		PROCESS_PPU_MODES(ppuCycles + pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange, NO, NO, YES, NO);
		pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = RESET;

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter >= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_BLANK)
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter -= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_BLANK;

			pGBA_display->currentMergePixel = RESET;
			pGBA_display->currentBgPixel = RESET;
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == YES)
			pGBA_display->currentBgPixelInTextMode[BG0] = RESET;
			pGBA_display->currentBgPixelInTextMode[BG1] = RESET;
			pGBA_display->currentBgPixelInTextMode[BG2] = RESET;
			pGBA_display->currentBgPixelInTextMode[BG3] = RESET;
#endif
			pGBA_display->currentWinPixel = RESET;
			pGBA_display->mergeWaitCyclesDone = CLEAR;
			pGBA_display->winWaitCyclesDone = CLEAR;
			pGBA_display->bgWaitCyclesDone = CLEAR;
			pGBA_display->objWaitCyclesDone = CLEAR;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.ppuCounter = RESET;
			for (INC8 mode = MODE0; mode <= MODE5; mode++)
			{
				pGBA_display->winAccessPatternState[mode] = RESET;
				pGBA_display->bgAccessPatternState[mode] = RESET;
				pGBA_display->mergeAccessPatternState[mode] = RESET;
			}
			pGBA_display->winAccessPattern = RESET;
			pGBA_display->bgAccessPattern = RESET;
			pGBA_display->mergeAccessPattern = RESET;

			// LINE == 159
			if (pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY == (TO_UINT(LCD_DIMENSIONS::LCD_VISIBLE_LINES) - ONE))
			{
				++pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_FLAG = RESET;

				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VBLANK_FLAG = SET;

				HANDLE_VCOUNT();

				// Refer http://problemkaputt.de/gbatek-gba-dma-transfers.htm
				if (pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY >= TWO)
				{
					/*
					* https://gbadev.net/tonc/dma.html
					* For DMA3 it will start the copy at the start of each rendering line, but with a 2 scanline delay.
					*/
					if (pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN == SET 
						&&
						pGBA_instance->GBA_state.dma.cache[DMA::DMA3].scheduleType == DMA_TIMING::SPECIAL)
					{
						ActivateDMAChannel(DMA::DMA3);
					}
				}

				RequestDMA(DMA_TIMING::VBLANK);

				if (pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VBLANK_IRQ_ENABLE == SET)
				{
					requestInterrupts(GBA_INTERRUPT::IRQ_VBLANK);
				}

				// Proceed to next lcd mode
				pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter;
				pGBA_display->wasVblankJustTriggered = YES;
				pGBA_display->didLCDModeChangeJustNow = YES;
				pGBA_display->currentLCDMode = LCD_MODES::MODE_LCD_H_DRAW_V_BLANK;
				PPUINFO("Entering MODE_LCD_H_DRAW_V_BLANK");
			}
			// LINE 0 - 158
			else
			{
				// Handling PPU:02

				pGBA_display->bgCache[BG2].internalRefPointRegisters.BGX_Signed += (int16_t)pGBA_peripherals->mBG2PBHalfWord.mBGnPxHalfWord_Signed;
				pGBA_display->bgCache[BG2].internalRefPointRegisters.BGY_Signed += (int16_t)pGBA_peripherals->mBG2PDHalfWord.mBGnPxHalfWord_Signed;

				if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE == MODE2)
				{
					pGBA_display->bgCache[BG3].internalRefPointRegisters.BGX_Signed += (int16_t)pGBA_peripherals->mBG3PBHalfWord.mBGnPxHalfWord_Signed;
					pGBA_display->bgCache[BG3].internalRefPointRegisters.BGY_Signed += (int16_t)pGBA_peripherals->mBG3PDHalfWord.mBGnPxHalfWord_Signed;
				}

				// Handling point (2) of PPU:01

				PPUTODO("Should the latching of Internal Reference Point when CPU writes to them be done only during end of Scanline or mid-scanline is also possible!");

				if (pGBA_display->bgCache[BG2].internalRefPointRegisters.bgxOverwrittenByCPU == YES)
				{
					pGBA_display->bgCache[BG2].internalRefPointRegisters.BGX = (int32_t)pGBA_peripherals->mBG2XWord.mBGniWord_Signed;
					pGBA_display->bgCache[BG2].internalRefPointRegisters.bgxOverwrittenByCPU = NO;
				}

				if (pGBA_display->bgCache[BG2].internalRefPointRegisters.bgyOverwrittenByCPU == YES)
				{
					pGBA_display->bgCache[BG2].internalRefPointRegisters.BGY = (int32_t)pGBA_peripherals->mBG2YWord.mBGniWord_Signed;
					pGBA_display->bgCache[BG2].internalRefPointRegisters.bgyOverwrittenByCPU = NO;
				}

				if (pGBA_display->bgCache[BG3].internalRefPointRegisters.bgxOverwrittenByCPU == YES)
				{
					pGBA_display->bgCache[BG3].internalRefPointRegisters.BGX = (int32_t)pGBA_peripherals->mBG3XWord.mBGniWord_Signed;
					pGBA_display->bgCache[BG3].internalRefPointRegisters.bgxOverwrittenByCPU = NO;
				}

				if (pGBA_display->bgCache[BG3].internalRefPointRegisters.bgyOverwrittenByCPU == YES)
				{
					pGBA_display->bgCache[BG3].internalRefPointRegisters.BGY = (int32_t)pGBA_peripherals->mBG3YWord.mBGniWord_Signed;
					pGBA_display->bgCache[BG3].internalRefPointRegisters.bgyOverwrittenByCPU = NO;
				}

				// Handling point (6) of PPU:03
				// Setting up the initial values in Affine buffer by loading it with the Internal Reference Point registers for per pixel processing of next scanline

				pGBA_display->bgCache[BG2].affine.affineX = pGBA_display->bgCache[BG2].internalRefPointRegisters.BGX_Signed;
				pGBA_display->bgCache[BG2].affine.affineY = pGBA_display->bgCache[BG2].internalRefPointRegisters.BGY_Signed;
				pGBA_display->bgCache[BG3].affine.affineX = pGBA_display->bgCache[BG3].internalRefPointRegisters.BGX_Signed;
				pGBA_display->bgCache[BG3].affine.affineY = pGBA_display->bgCache[BG3].internalRefPointRegisters.BGY_Signed;

				++pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_FLAG = RESET;

				HANDLE_VCOUNT();

				// Refer http://problemkaputt.de/gbatek-gba-dma-transfers.htm
				if (pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY >= TWO)
				{
					/*
					* https://gbadev.net/tonc/dma.html
					* For DMA3 it will start the copy at the start of each rendering line, but with a 2 scanline delay.
					*/
					if (pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN == SET
						&&
						pGBA_instance->GBA_state.dma.cache[DMA::DMA3].scheduleType == DMA_TIMING::SPECIAL)
					{
						ActivateDMAChannel(DMA::DMA3);
					}
				}

				// Proceed to next lcd mode
				pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter;
				pGBA_display->didLCDModeChangeJustNow = YES;
				pGBA_display->currentLCDMode = LCD_MODES::MODE_LCD_H_DRAW_V_DRAW;
				PPUINFO("Entering MODE_LCD_H_DRAW_V_DRAW");
			}
		}

		BREAK;
	}
	case LCD_MODES::MODE_LCD_H_DRAW_V_BLANK: // H Draw V Blank
	{
		// needed alteast for objects and perhaps windows as well
		PROCESS_PPU_MODES(ppuCycles + pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange, NO, YES, YES, NO);
		pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = RESET;

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter >= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_VISIBLE)
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter -= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_VISIBLE;

			// Note: In process PPU, we should account for cycles which occur only in MODE_LCD_H_DRAW_V_BLANK
			ppuCycles -= pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter;

			pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_FLAG = SET;

			// According to http://problemkaputt.de/gbatek-lcd-dimensions-and-timings.htm, we should not...but this is needed for tonc's irq_demo.gba
			// GBATEK is confirmed to be wrong w.r.t this as per https://discord.com/channels/465585922579103744/465586361731121162/1313454859131031574
			if (pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_IRQ_ENABLE == SET)
			{
				requestInterrupts(GBA_INTERRUPT::IRQ_HBLANK);
			}

			// Proceed to the next lcd mode
			pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter;
			pGBA_display->didLCDModeChangeJustNow = YES;
			pGBA_display->currentLCDMode = LCD_MODES::MODE_LCD_H_BLANK_V_BLANK;
			PPUINFO("Entering MODE_LCD_H_BLANK_V_BLANK");
		}

		BREAK;
	}
	case LCD_MODES::MODE_LCD_H_BLANK_V_BLANK: // H Blank V Blank
	{
		// needed alteast for objects
		PROCESS_PPU_MODES(ppuCycles + pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange, NO, NO, YES, NO);
		pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = RESET;

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter >= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_BLANK)
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter -= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_BLANK;

			pGBA_display->currentMergePixel = RESET;
			pGBA_display->currentBgPixel = RESET;
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == YES)
			pGBA_display->currentBgPixelInTextMode[BG0] = RESET;
			pGBA_display->currentBgPixelInTextMode[BG1] = RESET;
			pGBA_display->currentBgPixelInTextMode[BG2] = RESET;
			pGBA_display->currentBgPixelInTextMode[BG3] = RESET;
#endif
			pGBA_display->currentWinPixel = RESET;
			pGBA_display->mergeWaitCyclesDone = CLEAR;
			pGBA_display->winWaitCyclesDone = CLEAR;
			pGBA_display->bgWaitCyclesDone = CLEAR;
			pGBA_display->objWaitCyclesDone = CLEAR;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.ppuCounter = RESET;
			for (INC8 mode = MODE0; mode <= MODE5; mode++)
			{
				pGBA_display->winAccessPatternState[mode] = RESET;
				pGBA_display->bgAccessPatternState[mode] = RESET;
				pGBA_display->mergeAccessPatternState[mode] = RESET;
			}
			pGBA_display->winAccessPattern = RESET;
			pGBA_display->bgAccessPattern = RESET;
			pGBA_display->mergeAccessPattern = RESET;

			// LINE == 227
			if (pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY == ((uint32_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES - ONE))
			{
				// Handling point (1) of PPU:01

				pGBA_display->bgCache[BG2].internalRefPointRegisters.BGX_Signed = pGBA_peripherals->mBG2XWord.mBGniWord_Signed;
				pGBA_display->bgCache[BG2].internalRefPointRegisters.BGY_Signed = pGBA_peripherals->mBG2YWord.mBGniWord_Signed;
				pGBA_display->bgCache[BG3].internalRefPointRegisters.BGX_Signed = pGBA_peripherals->mBG3XWord.mBGniWord_Signed;
				pGBA_display->bgCache[BG3].internalRefPointRegisters.BGY_Signed = pGBA_peripherals->mBG3YWord.mBGniWord_Signed;

				// Reset the flags for point (2) of PPU:01 as we have already latched the registers
				pGBA_display->bgCache[BG2].internalRefPointRegisters.bgxOverwrittenByCPU = NO;
				pGBA_display->bgCache[BG2].internalRefPointRegisters.bgyOverwrittenByCPU = NO;
				pGBA_display->bgCache[BG3].internalRefPointRegisters.bgxOverwrittenByCPU = NO;
				pGBA_display->bgCache[BG3].internalRefPointRegisters.bgyOverwrittenByCPU = NO;

				// Handling point (6) of PPU:03
				// Setting up the initial values in Affine buffer by loading it with the Internal Reference Point registers for per pixel processing of next frame

				pGBA_display->bgCache[BG2].affine.affineX = pGBA_display->bgCache[BG2].internalRefPointRegisters.BGX_Signed;
				pGBA_display->bgCache[BG2].affine.affineY = pGBA_display->bgCache[BG2].internalRefPointRegisters.BGY_Signed;
				pGBA_display->bgCache[BG3].affine.affineX = pGBA_display->bgCache[BG3].internalRefPointRegisters.BGX_Signed;
				pGBA_display->bgCache[BG3].affine.affineY = pGBA_display->bgCache[BG3].internalRefPointRegisters.BGY_Signed;

				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VBLANK_FLAG = RESET;
				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_FLAG = RESET;

				pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY = RESET;

				HANDLE_VCOUNT();

				// Proceed to the next lcd mode
				pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter;
				pGBA_display->didLCDModeChangeJustNow = YES;
				pGBA_display->currentLCDMode = LCD_MODES::MODE_LCD_H_DRAW_V_DRAW;
				PPUINFO("Entering MODE_LCD_H_DRAW_V_DRAW");
			}
			// LINE 160 to 226
			else
			{
				++pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_FLAG = RESET;

				// If the LINE just became 227....
				// Refer http://problemkaputt.de/gbatek-lcd-i-o-interrupts-and-status.htm
				if (pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY == ((uint32_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES - ONE))
				{
					pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VBLANK_FLAG = RESET;
				}

				HANDLE_VCOUNT();

				// Refer http://problemkaputt.de/gbatek-gba-dma-transfers.htm
				if (pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY < 162)
				{
					/*
					* https://gbadev.net/tonc/dma.html
					* For DMA3 it will start the copy at the start of each rendering line, but with a 2 scanline delay.
					*/
					if (pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN == SET
						&&
						pGBA_instance->GBA_state.dma.cache[DMA::DMA3].scheduleType == DMA_TIMING::SPECIAL)
					{
						ActivateDMAChannel(DMA::DMA3);
					}
				}
				else if (pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY >= 162)
				{
					// Need to stop scheduling the Video Capture DMA (DMA 3) 
					if (pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN == SET
						&&
						pGBA_instance->GBA_state.dma.cache[DMA::DMA3].scheduleType == DMA_TIMING::SPECIAL)
					{
						pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN = RESET;
						OnDMAChannelWritten(DMA::DMA3, ENABLED, DISABLED);
					}
				}

				// Proceed to the next lcd mode
				pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter;
				pGBA_display->didLCDModeChangeJustNow = YES;
				pGBA_display->currentLCDMode = LCD_MODES::MODE_LCD_H_DRAW_V_BLANK;
				PPUINFO("Entering MODE_LCD_H_DRAW_V_BLANK");
			}
		}

		BREAK;
	}
	default:
	{
		FATAL("Unknown LCD Mode");
	}
	}
}

void GBA_t::displayCompleteScreen()
{
	if (debugConfig._DEBUG_PPU_VIEWER_GUI == ENABLED)
	{
		FATAL("_DEBUG_PPU_VIEWER_GUI is not supported yet");
	}
	else
	{
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

		glDisable(GL_BLEND);

		// Handle for gameboy system's texture

		glBindTexture(GL_TEXTURE_2D, gameboyAdvance_texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGBA_instance->GBA_state.display.imGuiBuffer.imGuiBuffer1D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, getScreenWidth() * FRAME_BUFFER_SCALE, 0, getScreenHeight() * FRAME_BUFFER_SCALE, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f(0.0, 0.0);
		glTexCoord2f(1.0, 0.0);
		glVertex2f(getScreenWidth() * FRAME_BUFFER_SCALE, 0.0);
		glTexCoord2f(1.0, 1.0);
		glVertex2f(getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE);
		glTexCoord2f(0.0, 1.0);
		glVertex2f(0.0, getScreenHeight() * FRAME_BUFFER_SCALE);
		glEnd();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Handle for dot matrix texture

		if (currEnVFilter == VIDEO_FILTERS::LCD_FILTER)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
			glEnable(GL_BLEND);

			glColor4f(1.0f, 1.0f, 1.0f, 0.3f / 4.0f);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBindTexture(GL_TEXTURE_2D, matrix_texture);

			int viewportWidth = getScreenWidth() * FRAME_BUFFER_SCALE;
			int viewportHeight = getScreenHeight() * FRAME_BUFFER_SCALE;

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			glOrtho(0, viewportWidth, 0, viewportHeight, -1, 1);

			glMatrixMode(GL_MODELVIEW);
			glViewport(0, 0, viewportWidth, viewportHeight);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0);
			glVertex2f(0.0, 0.0);
			glTexCoord2f(getScreenWidth(), 0.0);
			glVertex2f(viewportWidth, 0.0);
			glTexCoord2f(getScreenWidth(), getScreenHeight());
			glVertex2f(viewportWidth, viewportHeight);
			glTexCoord2f(0.0, getScreenHeight());
			glVertex2f(0.0, viewportHeight);
			glEnd();

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			glDisable(GL_BLEND);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Handle for renderer's texture

		glBindTexture(GL_TEXTURE_2D, masquerade_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (currEnVFilter == VIDEO_FILTERS::LCD_FILTER)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
#else
		// 1. Upload emulator framebuffer to gameboyAdvance_texture
		GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboyAdvance_texture));
		GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
			(GLvoid*)pGBA_instance->GBA_state.display.imGuiBuffer.imGuiBuffer1D));

		// Choose filtering mode (NEAREST or LINEAR)
		GLint filter = (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER) ? GL_LINEAR : GL_NEAREST;
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));

		// 2. Render gameboyAdvance_texture into framebuffer (masquerade_texture target)
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
		GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

		// Pass 1: Render base texture (Game Boy framebuffer)
		GL_CALL(glUseProgram(shaderProgramBasic));
		GL_CALL(glActiveTexture(GL_TEXTURE0));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboyAdvance_texture));
		GL_CALL(glUniform1i(glGetUniformLocation(shaderProgramBasic, "u_Texture"), 0));

		GL_CALL(glBindVertexArray(fullscreenVAO));
		GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
		GL_CALL(glBindVertexArray(0));
		GL_CALL(glUseProgram(0));

		// 3. Optional: LCD matrix overlay (dot matrix)
		if (currEnVFilter == VIDEO_FILTERS::LCD_FILTER)
		{
			GL_CALL(glEnable(GL_BLEND));
			GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

			GL_CALL(glUseProgram(shaderProgramBlend));

			// Set alpha (0.3 / 4.0)
			GL_CALL(glUniform1f(glGetUniformLocation(shaderProgramBlend, "u_Alpha"), 0.075f));

			// Set texture
			GL_CALL(glActiveTexture(GL_TEXTURE0));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, matrix_texture));
			GL_CALL(glUniform1i(glGetUniformLocation(shaderProgramBlend, "u_Texture"), 0));

			// Set texel size (1 / 4) to repeat the matrix texture per pixel
			float texelSize[2] = { 1.0f / 4.0f, 1.0f / 4.0f };
			GL_CALL(glUniform2fv(glGetUniformLocation(shaderProgramBlend, "u_TexelSize"), 1, texelSize));

			GL_CALL(glBindVertexArray(fullscreenVAO));
			GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
			GL_CALL(glBindVertexArray(0));

			GL_CALL(glUseProgram(0));
			GL_CALL(glDisable(GL_BLEND));
		}

		// 4. Done rendering to framebuffer (masquerade_texture)
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		// 5. Setup filtering for final display (e.g., ImGui::Image or screen blit)
		GL_CALL(glBindTexture(GL_TEXTURE_2D, masquerade_texture));

		filter = (currEnVFilter == VIDEO_FILTERS::LCD_FILTER) ? GL_LINEAR : GL_NEAREST;
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
#endif
	}
}

void GBA_t::loadQuirks()
{
	if (ImGui::IsKeyReleased(ImGuiKey_Q) == YES)
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

		LOG("CONFIG.ini was reloaded!");
	}
}

void GBA_t::initializeGraphics()
{
	// Clear the screen
	for (int y = ZERO; y < this->total_screen_height; y++)
	{
		for (int x = ZERO; x < this->total_screen_width; x++)
		{
			if ((y >= this->screen_y_offset && y < this->screen_height + this->screen_y_offset)
				&&
				(x >= this->screen_x_offset && y < this->screen_width + this->screen_x_offset))
			{
				pGBA_display->gfx_bg[BG0][x - this->screen_x_offset][y - this->screen_y_offset] = RESET;
				pGBA_display->gfx_bg[BG1][x - this->screen_x_offset][y - this->screen_y_offset] = RESET;
				pGBA_display->gfx_bg[BG2][x - this->screen_x_offset][y - this->screen_y_offset] = RESET;
				pGBA_display->gfx_bg[BG3][x - this->screen_x_offset][y - this->screen_y_offset] = RESET;
				pGBA_display->gfx_obj[x - this->screen_x_offset][y - this->screen_y_offset] = RESET;
				pGBA_display->objPriority[x - this->screen_x_offset][y - this->screen_y_offset] = DEFAULT_OBJ_PRIORITY;
			}
		}
	}

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.ppuCounter = RESET;
	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.lcdCounter = RESET;

	// From Nano Boy Advance
	pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord = 0x100;
	pGBA_peripherals->mBG2PDHalfWord.mBGnPxHalfWord = 0x100;
	pGBA_peripherals->mBG3PAHalfWord.mBGnPxHalfWord = 0x100;
	pGBA_peripherals->mBG3PDHalfWord.mBGnPxHalfWord = 0x100;
	pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY = 225;
	pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_FLAG = SET;
	pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VBLANK_FLAG = SET;
	pGBA_display->currentLCDMode = LCD_MODES::MODE_LCD_H_BLANK_V_BLANK;
	pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)].vcount = ((pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY + ONE) % 228);
	pGBA_display->allObjectsRenderedForScanline = YES;
}

float GBA_t::getEmulationVolume()
{
	pGBA_audio->emulatorVolume = SDL_GetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream));
	RETURN pGBA_audio->emulatorVolume;
}

void GBA_t::setEmulationVolume(float volume)
{
	pGBA_audio->emulatorVolume = volume;
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), volume);
	pt.put("gba._volume", volume);
	boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, pt);
}

void GBA_t::initializeAudio()
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	SDL_AudioFormat format = SDL_AUDIO_S16;
	const SDL_AudioSpec AudioSettings{ format, TO_UINT8(AUDIO_STREAMS::TOTAL_AUDIO_STREAMS), TO_UINT(EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA) };
	audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &AudioSettings, NULL, NULL);
	
#if DEACTIVATED
	SDL_AudioSpec actualSpec;
	SDL_GetAudioDeviceFormat(SDL_GetAudioStreamDevice(audioStream), &actualSpec, NULL);
#endif

	SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.apuCounter = RESET;

	// Refer : http://problemkaputt.de/gbatek-gba-sound-control-registers.htm
	pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASHalfWord = 0x200;

	// Setup the volume for audio
	pGBA_audio->emulatorVolume = pt.get<std::float_t>("gba._volume");
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), pGBA_audio->emulatorVolume);
}

FLAG GBA_t::runEmulationAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

FLAG GBA_t::runEmulationLoopAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

FLAG GBA_t::runEmulationAtFixedRate(uint32_t currentFrame)
{
	FLAG status = true;

#if (DISABLED)
	pGBA_instance->GBA_state.emulatorStatus.debugger.wasDebuggerJustTriggerred = CLEAR;
#endif

	loadQuirks();

	captureIO();

	playTheAudioFrame();

	displayCompleteScreen();

	RETURN status;
}

FLAG GBA_t::runEmulationLoopAtFixedRate(uint32_t currentFrame)
{
	pGBA_display->wasVblankJustTriggered = NO;
	pGBA_display->didLCDModeChangeJustNow = NO;

	if (ROM_TYPE == ROM::COMPARE)
	{
#if _DEBUG
		if (repSkyLogIttr == 1362 - 1)
		{
			volatile int breakpoint = RESET;
		}
#endif

		FLAG matches = YES;
		SCOUNTER8 failedRegister = -ONE;
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r0 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_0)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_0);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r1 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_1)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_1);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r2 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_2)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_2);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r3 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_3)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_3);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r4 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_4)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_4);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r5 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_5)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_5);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r6 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_6)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_6);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r7 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_7)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_7);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r8 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_8)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_8);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r9 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_9)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_9);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r10 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_10)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_10);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r11 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_11)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_11);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r12 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_12)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_12);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r13 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_13)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_13);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r14 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_14)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_14);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_r15 == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_15)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_15);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_CPSR == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_16)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_16);
		if (matches) matches &= (repSkyLog[repSkyLogIttr].rep_SPSR == cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_17)); else if (failedRegister == -ONE) failedRegister = TO_UINT(REGISTER_TYPE::RT_17);

		if (matches == NO || failedRegister != -ONE)
		{
			FATAL("Compare Mismatch Detected for register R%d on iteration %u", failedRegister, repSkyLogIttr);
		}

		++repSkyLogIttr;
	}

	processSOC();

#if (DISABLED)
	runDebugger();
#endif

	RETURN pGBA_display->wasVblankJustTriggered;
}

FLAG GBA_t::initializeEmulator()
{
	FLAG status = true;

	pAbsolute_GBA_instance = std::make_shared<absolute_GBA_instance_t>();

	// Initialize the memory

	memset(pAbsolute_GBA_instance->GBA_absoluteMemoryState, RESET, sizeof(absolute_GBA_state_t));

	// for readability

	pGBA_instance = (GBA_instance_t*)&(pAbsolute_GBA_instance->absolute_GBA_state.GBA_instance);
	pGBA_registers = &(pGBA_instance->GBA_state.cpuInstance.registers);
	pGBA_cpuInstance = &(pGBA_instance->GBA_state.cpuInstance);
	pGBA_memory = &(pGBA_instance->GBA_state.gbaMemory);
	pGBA_peripherals = &(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mIO.mIOFields);
	pGBA_audio = &(pGBA_instance->GBA_state.audio);
	pGBA_display = &(pGBA_instance->GBA_state.display);

	pGBA_instance->GBA_state.dma.currentlyActiveDMA = DMA::NO_DMA;

	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTHalfWord = 0x03FF; // Start with "Released"

	pGBA_peripherals->mSIO_DATA8_MLTSEND = ZERO;
	pGBA_peripherals->mSIOMULTI0 = ZERO;
	pGBA_peripherals->mSIOMULTI1 = ZERO;
	pGBA_peripherals->mSIOMULTI2 = ZERO;
	pGBA_peripherals->mSIOMULTI3 = ZERO;

	pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::RUN;

	WARN("Running in Cycle Count Accuracy Mode instead of Cycle Accuracy Mode");
	pGBA_instance->GBA_state.emulatorStatus.isCycleAccurate = NO;

	// Initialize the backup interface

	pGBA_instance->GBA_state.emulatorStatus.backup.flash.erase4kbPageNumber = (SSTATE32)INVALID;

	// Setup the logger interface

	pGBA_instance->GBA_state.emulatorStatus.debugger.loggerInterface.logger = ENABLE_LOGS;

	// Initialize few memory variables

	pGBA_memory->previouslyAccessedMemory = RESET;
	pGBA_memory->setNextMemoryAccessType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;

	// Initialize memory

	memset(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ZERO], 0xFF, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ZERO]));
	memset(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ONE], 0xFF, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ONE]));

	// initialization specific to OpenGL
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glEnable(GL_TEXTURE_2D);
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glGenTextures(1, &masquerade_texture);
	glBindTexture(GL_TEXTURE_2D, masquerade_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, masquerade_texture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(1, &gameboyAdvance_texture);
	glBindTexture(GL_TEXTURE_2D, gameboyAdvance_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGBA_instance->GBA_state.display.imGuiBuffer.imGuiBuffer1D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// for "Dot Matrix"
	glGenTextures(1, &matrix_texture);

	glBindTexture(GL_TEXTURE_2D, matrix_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)matrix);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#else
	// 1. Setup framebuffer
	GL_CALL(glGenFramebuffers(1, &frame_buffer));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));

	// 2. Create texture to attach to framebuffer (masquerade_texture)
	GL_CALL(glGenTextures(1, &masquerade_texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, masquerade_texture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, masquerade_texture, 0));

	// Optional: Check framebuffer status
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG("Error: Framebuffer is not complete!");
	}
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0)); // Unbind

	// 3. Game Boy Advance texture (used to upload emulated framebuffer)
	GL_CALL(glGenTextures(1, &gameboyAdvance_texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboyAdvance_texture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGBA_instance->GBA_state.display.imGuiBuffer.imGuiBuffer1D));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	// 4. Dot Matrix overlay texture
	GL_CALL(glGenTextures(1, &matrix_texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, matrix_texture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)matrix));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	// 5. Fullscreen Quad VAO/VBO (for textured quad rendering)
	float fullscreenVertices[] = {
		//  X     Y      U     V
		-1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
		-1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
		 1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right

		-1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
		 1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right
		 1.0f,  1.0f,  1.0f, 1.0f   // Top-right
	};

	GL_CALL(glGenVertexArrays(1, &fullscreenVAO));
	GL_CALL(glBindVertexArray(fullscreenVAO));

	GL_CALL(glGenBuffers(1, &fullscreenVBO));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, fullscreenVBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenVertices), fullscreenVertices, GL_STATIC_DRAW));

	// Attribute 0: position (vec2)
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));

	// Attribute 1: UV (vec2)
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));

	GL_CALL(glBindVertexArray(0));

	std::string shaderPath;
#ifndef __EMSCRIPTEN__
	shaderPath = pt.get<std::string>("internal._working_directory");
#else
	shaderPath = "assets/internal";
#endif

	// 6. Compile passthrough shader
	shaderProgramSource_t passthroughShader = parseShader(shaderPath + "/shaders/passthrough.shaders");
	shaderProgramBasic = createShader(passthroughShader.vertexSource, passthroughShader.fragmentSource);
	// 7. Compile blend shader (for LCD effect)
	shaderProgramSource_t blendShader = parseShader(shaderPath + "/shaders/blend.shaders");
	shaderProgramBlend = createShader(blendShader.vertexSource, blendShader.fragmentSource);

	DEBUG("PASSTHROUGH VERTEX");
	DEBUG("%s", passthroughShader.vertexSource.c_str());
	DEBUG("PASSTHROUGH FRAGMENT");
	DEBUG("%s", passthroughShader.fragmentSource.c_str());
	DEBUG("BLEND VERTEX");
	DEBUG("%s", blendShader.vertexSource.c_str());
	DEBUG("BLEND FRAGMENT");
	DEBUG("%s", blendShader.fragmentSource.c_str());
#endif

	RETURN status;
}

void GBA_t::destroyEmulator()
{
	FLAG status = true;

	// save SRAM + RTC (if applicable)

	std::filesystem::path saveDirectory(_SAVE_LOCATION);
	if (!(std::filesystem::exists(saveDirectory)))
	{
		std::filesystem::create_directory(saveDirectory);
	}

	// 1) saving "SRAM"

	std::string saveFileNameForThisROM = getSaveFileName(
		pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer
		, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer)
	);

	saveFileNameForThisROM = _SAVE_LOCATION + "\\" + saveFileNameForThisROM;

	std::cout << "\nSaving to " << saveFileNameForThisROM << std::endl;

	std::ofstream outSRAM(saveFileNameForThisROM.c_str(), std::ios_base::binary);

	uint32_t sizeOfSRAMSlot = 0x10000;
	uint8_t numberOfBackupBanks = ONE;

	switch (pGBA_instance->GBA_state.emulatorStatus.backup.backupType)
	{
	case BACKUP_TYPE::FLASH64K:
	case BACKUP_TYPE::FLASH128K:
	{
		if (pGBA_instance->GBA_state.emulatorStatus.backup.backupType == BACKUP_TYPE::FLASH128K)
		{
			sizeOfSRAMSlot = 0x10000;
			numberOfBackupBanks = TWO;
		}

		if (outSRAM.fail() == false)
		{
			for (ID banks = ZERO; banks < numberOfBackupBanks; banks++)
			{
				for (uint32_t ii = ZERO; ii < sizeOfSRAMSlot; ii++)
				{
					BYTE ramByte = pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[banks][ii];
					outSRAM.write(reinterpret_cast<const char*> (&ramByte), ONE);
				}
				outSRAM.flush();
			}
		}

		outSRAM.close();

		BREAK;
	}
	default:
	{
		WARN("Unknown Backup Type!");
		BREAK;
	}
	}

#if (DEACTIVATED)
	// TWO) loading RTC

	std::string rtcSaveForThisROM = getRTCSaveName(
		pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer
		, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer)
	);

	rtcSaveForThisROM = _SAVE_LOCATION + "\\" + rtcSaveForThisROM;

	std::cout << "\nSaving to " << rtcSaveForThisROM << std::endl;

	std::ofstream outRTC(rtcSaveForThisROM.c_str(), std::ios_base::binary);

	if (outRTC.fail() == false)
	{
		for (int ii = ZERO; ii < sizeof(pGBA_instance->GBA_state.rtc.rtcBuffer); ii++)
		{
			BYTE rtcByte = pGBA_instance->GBA_state.rtc.rtcBuffer[ii];
			outRTC.write(reinterpret_cast<const char*> (&rtcByte), ONE);
		}

		outRTC.flush();
	}

	outRTC.close();
#endif

	skylersalehLogs = DISABLED;

	_DISABLE_BG = NO;
	_DISABLE_WIN = NO;
	_DISABLE_OBJ = NO;
	_LOAD_GBA_BIOS = NO;
	_ENABLE_GBA_BIOS = NO;
	_LOAD_BUT_DONT_EXECUTE_GBA_BIOS = NO;

	logCounter = ZERO;
	memset(emulationCounter, ZERO, ((sizeof(emulationCounter[100])) / sizeof(emulationCounter[0])));

	pGBA_instance = nullptr;
	pGBA_registers = nullptr;
	pGBA_cpuInstance = nullptr;
	pGBA_memory = nullptr;
	pGBA_audio = nullptr;
	pGBA_display = nullptr;
	pGBA_peripherals = nullptr;
	pAbsolute_GBA_instance.reset();

#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glDeleteTextures(1, &gameboyAdvance_texture);
	glDeleteTextures(1, &matrix_texture);
#else
	glDeleteTextures(1, &gameboyAdvance_texture);
	glDeleteTextures(1, &matrix_texture);
#endif

	auto audioDevId = SDL_GetAudioStreamDevice(audioStream);
	SDL_PauseAudioDevice(audioDevId);
	SDL_ClearAudioStream(audioStream);
	SDL_UnbindAudioStream(audioStream);
	SDL_DestroyAudioStream(audioStream);
	SDL_CloseAudioDevice(audioDevId);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

FLAG GBA_t::loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom)
{
	// open the rom file

	FILE* fp = NULL;
	uint32_t totalRomSize = 0;
	uint32_t totalAuxilaryRomSize = 0;

	if ((ROM_TYPE == ROM::GAME_BOY_ADVANCE) || (ROM_TYPE == ROM::REPLAY) || (ROM_TYPE == ROM::COMPARE))
	{
		errno_t err = fopen_portable(&fp, rom[ZERO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// lets first do some basic initialization before loading the rom

			// fill rom out of bounds values
			// refer: https://discord.com/channels/465585922579103744/465586361731121162/1200182383031373905
			for (auto ii = pAbsolute_GBA_instance->absolute_GBA_state.aboutRom.codeRomSize; ii < GAMEPAK_ROM_SIZE; ii += TWO)
			{
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakRom.mFlashRom8bit[ii + ZERO] = ii >> ONE; // lower nibble (addr >> 1)
				pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakRom.mFlashRom8bit[ii + ONE] = ii >> NINE; // upper nibble (addr >> (8 + 1))
			}

			// if bios was loaded, replace the first 256 bytes with actual bios
			if (gba_bios.biosFound == YES)
			{
				// Load bios to system rom memory space
				memcpy_portable(pGBA_memory->mGBAMemoryMap.mSystemRom.mSystemRom8bit, 0x4000, gba_bios.biosImage, 0x4000);

				// Refer : http://problemkaputt.de/gbatek-arm-cpu-exceptions.htm

				// CPU Boot up in supervisor mode
				setARMMode(OP_MODE_TYPE::OP_SVC);

				// CPU boots up in ARM state
				setARMState(STATE_TYPE::ST_ARM);

				// By default, IRQ and FIQ is disabled
				pGBA_registers->cpsr.psrFields.psrIRQDisBit = SET;
				pGBA_registers->cpsr.psrFields.psrFIQDisBit = SET;

				// Initialize the pipeline

				pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = 0xF0000000; // NOP with condition set to always
				pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = 0xF0000000; // NOP with condition set to always
				pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = 0xF0000000; // NOP with condition set to always
			}

			if (gba_bios.biosFound == NO || _LOAD_BUT_DONT_EXECUTE_GBA_BIOS == YES)
			{
				if (_LOAD_BUT_DONT_EXECUTE_GBA_BIOS == YES)
				{
					WARN("BIOS is loaded but bypassed!");
					_ENABLE_GBA_BIOS = NO; // make sure bios is not executed
				}

				WARN("Initializing few registers and memory regions of GBA before starting the emulation as we are skipping BIOS");

				// put the cpu in system mode
				setARMMode(OP_MODE_TYPE::OP_SYS);

				// run the cpu in arm state
				setARMState(STATE_TYPE::ST_ARM);

				pGBA_registers->unbankedLORegisters[ZERO] = 0x00000000;
				pGBA_registers->unbankedLORegisters[ONE] = 0x00000000;
				pGBA_registers->unbankedLORegisters[TWO] = 0x00000000;
				pGBA_registers->unbankedLORegisters[THREE] = 0x00000000;
				pGBA_registers->unbankedLORegisters[FOUR] = 0x00000000;
				pGBA_registers->unbankedLORegisters[FIVE] = 0x00000000;
				pGBA_registers->unbankedLORegisters[SIX] = 0x00000000;
				pGBA_registers->unbankedLORegisters[SEVEN] = 0x00000000;
				pGBA_registers->bankedHIRegisters[(BYTE)getCurrentlyValidRegisterBank()][ZERO] = 0x00000000;
				pGBA_registers->bankedHIRegisters[(BYTE)getCurrentlyValidRegisterBank()][ONE] = 0x00000000;
				pGBA_registers->bankedHIRegisters[(BYTE)getCurrentlyValidRegisterBank()][TWO] = 0x00000000;
				pGBA_registers->bankedHIRegisters[(BYTE)getCurrentlyValidRegisterBank()][THREE] = 0x00000000;
				pGBA_registers->bankedHIRegisters[(BYTE)getCurrentlyValidRegisterBank()][FOUR] = 0x00000000;
				pGBA_registers->bankedHIRegisters[(BYTE)getCurrentlyValidRegisterBank()][FIVE] = 0x03007F00;
				pGBA_registers->bankedHIRegisters[(BYTE)REGISTER_BANK_TYPE::RB_IRQ][FIVE] = 0x3007FA0;
				pGBA_registers->bankedHIRegisters[(BYTE)REGISTER_BANK_TYPE::RB_SVC][FIVE] = 0x3007FA0;
				pGBA_registers->bankedHIRegisters[(BYTE)getCurrentlyValidRegisterBank()][SIX] = 0x00000000;
				pGBA_registers->pc = 0x08000000;
				pGBA_registers->cpsr.psrMemory = 0x000000DF;
				pGBA_registers->spsr[(BYTE)getCurrentlyValidRegisterBank()].psrMemory = 0x000000DF;

				// initialize the pipeline

				pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = 0xF0000000; // NOP with condition set to always
				pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = 0xF0000000; // NOP with condition set to always
				pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = 0xF0000000; // NOP with condition set to always
			}

			// now, time to load the rom

			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_GBA_instance->absolute_GBA_state.aboutRom.codeRomSize = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pGBA_instance->GBA_state.gbaMemory.mGBAMemoryMap.mGamePakRom.mFlashRom8bit + 0x0000, pAbsolute_GBA_instance->absolute_GBA_state.aboutRom.codeRomSize, 1, fp);

			// display some of the Cartridge information
			LOG_NEW_LINE;
			LOG("Cartridge Loaded:");
			LOG(" Title : %s", pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.gametitle);
			LOG(" Game Code : AGB-%s", pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.gameCode);
			LOG(" Maker Code : %s", pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.makerCode);
			LOG(" Fixed Value (0x96) : %X", pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.fixedValue);
			LOG(" Main Unit Code : %X", pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.mainUnitCode);
			LOG(" Device Type : %X", pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.deviceType);
			LOG(" SW Version : %X", pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.softwareVersion);

			uint8_t checksum = 0;
			for (uint16_t address = 0x00A0; address <= 0x00BC; address++)
			{
				checksum = checksum - pGBA_memory->mGBAMemoryMap.mGamePakRom.mFlashRom8bit[address];
			}
			checksum -= 0x19;
			checksum &= 0xFF;

			pGBA_instance->GBA_state.emulatorStatus.checksum = checksum;

			LOG(" Checksum : %2.2X (%s)", checksum, (checksum == pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.complementCheck) ? "PASSED" : "FAILED");
			LOG_NEW_LINE;

			// Get the backup type information
			// Loop through every single word aligned address in the ROM and try to find what backup type it is 
			// Refer https://dillonbeliveau.com/2020/06/05/GBA-FLASH.html and http://problemkaputt.de/gbatek-gba-cart-backup-ids.htm

			FLAG foundBackupType = NO;
			auto size = pAbsolute_GBA_instance->absolute_GBA_state.aboutRom.codeRomSize;
			for (uint32_t i = 0; i < size; i += sizeof(GBA_WORD))
			{
				for (auto const& [signature, type] : signatures)
				{
					if ((i + signature.size()) <= size &&
						std::memcmp(&pGBA_memory->mGBAMemoryMap.mGamePakRom.mFlashRom8bit[i], signature.data(), signature.size()) == 0)
					{
						LOG(" Backup Type ID : %X", TO_UINT(type));
						pGBA_instance->GBA_state.emulatorStatus.backup.backupType = type;
						foundBackupType = YES;
					}
				}
			}

			if (foundBackupType == NO)
			{
				LOG(" Backup Type ID : UNKNOWN");
				WARN("Backup Type Not Found!");
			}

			if (pGBA_instance->GBA_state.emulatorStatus.backup.backupType == BACKUP_TYPE::FLASH64K
				|| pGBA_instance->GBA_state.emulatorStatus.backup.backupType == BACKUP_TYPE::FLASH128K)
			{
				memset(
					pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ZERO]
					, 0xFF
					, 0x10000
				);

				memset(
					pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ONE]
					, 0xFF
					, 0x10000
				);

				memset(
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.isErased[ZERO]
					, YES
					, 0x10000
				);

				memset(
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.isErased[ONE]
					, YES
					, 0x10000
				);
			}

			// load SRAM + RTC (if applicable)

			// 1) loading SRAM

			std::string saveFileNameForThisROM = getSaveFileName(
				pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer
				, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_buffer)
			);

			saveFileNameForThisROM = _SAVE_LOCATION + "\\" + saveFileNameForThisROM;

			std::cout << "\nAttempting to load " << saveFileNameForThisROM << std::endl;

			std::ifstream inSRAM(saveFileNameForThisROM.c_str(), std::ios::in | std::ios_base::binary);

			uint32_t sizeOfSRAMSlot = 0x10000;
			uint8_t numberOfBackupBanks = ONE;

			switch (pGBA_instance->GBA_state.emulatorStatus.backup.backupType)
			{
			case BACKUP_TYPE::FLASH64K:
			case BACKUP_TYPE::FLASH128K:
			{
				if (pGBA_instance->GBA_state.emulatorStatus.backup.backupType == BACKUP_TYPE::FLASH128K)
				{
					sizeOfSRAMSlot = 0x10000;
					numberOfBackupBanks = TWO;
				}

				if (inSRAM.fail() == false)
				{
					for (ID banks = ZERO; banks < numberOfBackupBanks; banks++)
					{
						for (uint32_t ii = ZERO; ii < sizeOfSRAMSlot; ii++)
						{
							BYTE sramByte = ZERO;
							inSRAM.read(reinterpret_cast<char*> (&sramByte), ONE);
							pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[banks][ii] = sramByte;
						}
					}
				}

				inSRAM.close();

				BREAK;
			}
			default:
			{
				WARN("Unknown Backup Type!");
				BREAK;
			}
			}

			inSRAM.close();

#if (DISABLED)
			// 2) loading RTC

			std::string rtcSaveForThisROM = getRTCSaveName(
				pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer
				, sizeof(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer)
			);

			rtcSaveForThisROM = _SAVE_LOCATION + "\\" + rtcSaveForThisROM;

			std::cout << "\nAttempting to load " << rtcSaveForThisROM << std::endl;

			std::ifstream inRTC(rtcSaveForThisROM.c_str(), std::ios::in | std::ios_base::binary);

			if (inRTC.fail() == false)
			{
				for (int ii = ZERO; ii < sizeof(pGBA_instance->GBA_state.rtc.rtcBuffer); ii++)
				{
					BYTE rtcByte = ZERO;
					inRTC.read(reinterpret_cast<char*> (&rtcByte), ONE);
					pGBA_instance->GBA_state.rtc.rtcBuffer[ii] = rtcByte;
				}
			}

			inRTC.close();
#endif

			LOG_NEW_LINE;

			rewind(fp);

			// close the rom for now
			fclose(fp);

			if ((ROM_TYPE == ROM::REPLAY) || (ROM_TYPE == ROM::COMPARE))
			{
				fp = nullptr;
				errno_t err = fopen_portable(&fp, rom[ONE].c_str(), "rb");

				if (!err && (fp != NULL))
				{
					// get the size of the complete rom
					fseek(fp, 0, SEEK_END);
					auto size = ftell(fp);
					rewind(fp);

					// Sanity check: file size must be multiple of struct size
					size_t frameSize = sizeof(repSkyFormat_t);
					if (size % frameSize != 0)
					{
						FATAL("Invalid file size (%ld): not a multiple of frame size (%zu)", size, frameSize);
						fclose(fp);
					}

					size_t numFrames = size / frameSize;

					// Allocate vector
					repSkyLog.resize(numFrames);

					// Read all frames at once
					size_t readCount = fread(repSkyLog.data(), frameSize, numFrames, fp);
					fclose(fp);

					if (readCount != numFrames)
					{
						FATAL("Read error: got %zu of %zu frames", readCount, numFrames);
						repSkyLog.clear();
					}

					LOG("Loaded %zu replay/compare frames\n", repSkyLog.size());
				}
				else
				{
					FATAL("Issue while reading the replay/compare logs");
				}
			}
		}
	}
	else
	{
		FATAL("Unsupported ROM type for GBA : %d", TO_UINT(ROM_TYPE));
		RETURN false;
	}

	RETURN true;
}

void GBA_t::dumpRom()
{
	uint32_t scanner = 0;
	uint32_t addressField = 0x10;

	LOG("ROM DUMP");
	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (int ii = 0; ii < (int)pAbsolute_GBA_instance->absolute_GBA_state.aboutRom.codeRomSize; ii++)
	{
		LOG("0x%02x\t", pGBA_instance->GBA_state.gbaMemory.mGBARawMemory[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	LOG_NEW_LINE;
}

FLAG GBA_t::getRomLoadedStatus()
{
	RETURN pAbsolute_GBA_instance->absolute_GBA_state.aboutRom.isRomLoaded;
}
#pragma endregion EMULATION_DEFINITIONS