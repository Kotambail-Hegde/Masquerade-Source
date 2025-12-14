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

#pragma region GBA_SPECIFIC_MACROS
#define AUDIO_FIFO_SIZE_FOR_GBA							FOUR

#define LFSR_WIDTH_IS_15_BITS							ZERO
#define LFSR_WIDTH_IS_7_BITS							ONE
#define APU_FRAME_SEQUENCER_RATE_HZ						(512.0f)
#define DC_BIAS_FOR_AUDIO_SAMPLES						ZERO
#define DISABLE_FIRST_PULSE_CHANNEL						NO
#define DISABLE_SECOND_PULSE_CHANNEL					NO
#define DISABLE_WAVE_CHANNEL							NO
#define DISABLE_NOISE_CHANNEL							NO
#define DISABLE_DMAA_CHANNEL							NO
#define DISABLE_DMAB_CHANNEL							NO

#define NRW_WAVE_BANK									ZERO
#define RW_WAVE_BANK									ONE

#define DIRECT_SOUND_A									ZERO
#define DIRECT_SOUND_B									ONE

#define CPSR_FLAG(F, C)									(F == 1 ? C:"-")

#define loadPipeline									fetchAndDecode
#define initializeSerialClockSpeed						processSerialClockSpeedBit
#define performOverFlowCheck							getUpdatedFrequency

#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif
#pragma endregion GBA_SPECIFIC_MACROS

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

// for audio
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

// for video
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

#if (GBA_ENABLE_AUDIO == NO)
	_ENABLE_AUDIO = NO;
#endif

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
GBA_t::REGISTER_BANK_TYPE GBA_t::getRegisterBankFromOperatingMode(OP_MODE_TYPE opMode)
{
	REGISTER_BANK_TYPE rb;

	switch (opMode)
	{
	case OP_MODE_TYPE::OP_USR:
	{
		rb = REGISTER_BANK_TYPE::RB_USR_SYS;
		BREAK;
	}
	case OP_MODE_TYPE::OP_FIQ:
	{
		rb = REGISTER_BANK_TYPE::RB_FIQ;
		BREAK;
	}
	case OP_MODE_TYPE::OP_IRQ:
	{
		rb = REGISTER_BANK_TYPE::RB_IRQ;
		BREAK;
	}
	case OP_MODE_TYPE::OP_SVC:
	{
		rb = REGISTER_BANK_TYPE::RB_SVC;
		BREAK;
	}
	case OP_MODE_TYPE::OP_ABT:
	{
		rb = REGISTER_BANK_TYPE::RB_ABT;
		BREAK;
	}
	case OP_MODE_TYPE::OP_UND:
	{
		rb = REGISTER_BANK_TYPE::RB_UND;
		BREAK;
	}
	case OP_MODE_TYPE::OP_SYS:
	{
		rb = REGISTER_BANK_TYPE::RB_USR_SYS;
		BREAK;
	}
	default:
	{
		rb = REGISTER_BANK_TYPE::RB_USR_SYS;
		BREAK;
	}
	}

	RETURN rb;
}

GBA_t::REGISTER_BANK_TYPE GBA_t::getCurrentlyValidRegisterBank()
{
	RETURN getRegisterBankFromOperatingMode(getARMMode());
}

// TODO: As of now, "getOperatingModeFromRegisterBank" function cannot differentiate between USR mode and SYS modes
GBA_t::OP_MODE_TYPE GBA_t::getOperatingModeFromRegisterBank(REGISTER_BANK_TYPE rb)
{
	OP_MODE_TYPE opMode;

	switch (rb)
	{
	case REGISTER_BANK_TYPE::RB_USR_SYS:
	{
		opMode = OP_MODE_TYPE::OP_USR; // TODO: We are determining the opMode to be USR, but it can be SYS as well...
		BREAK;
	}
	case REGISTER_BANK_TYPE::RB_FIQ:
	{
		opMode = OP_MODE_TYPE::OP_FIQ;
		BREAK;
	}
	case REGISTER_BANK_TYPE::RB_IRQ:
	{
		opMode = OP_MODE_TYPE::OP_IRQ;
		BREAK;
	}
	case REGISTER_BANK_TYPE::RB_SVC:
	{
		opMode = OP_MODE_TYPE::OP_SVC;
		BREAK;
	}
	case REGISTER_BANK_TYPE::RB_ABT:
	{
		opMode = OP_MODE_TYPE::OP_ABT;
		BREAK;
	}
	case REGISTER_BANK_TYPE::RB_UND:
	{
		opMode = OP_MODE_TYPE::OP_UND;
		BREAK;
	}
	default:
	{
		opMode = OP_MODE_TYPE::OP_USR;
		BREAK;
	}
	}

	RETURN opMode;
}

void GBA_t::setARMState(STATE_TYPE armState)
{
	pGBA_cpuInstance->registers.cpsr.psrFields.psrStateBit = (uint32_t)armState;
	pGBA_cpuInstance->armState = armState;

}

void GBA_t::setARMMode(OP_MODE_TYPE opMode)
{
	pGBA_cpuInstance->registers.cpsr.psrFields.psrModeBits = (uint32_t)opMode;
	pGBA_cpuInstance->armMode = opMode;
}

GBA_t::STATE_TYPE GBA_t::getARMState()
{
	pGBA_cpuInstance->armState = (STATE_TYPE)pGBA_cpuInstance->registers.cpsr.psrFields.psrStateBit;
	RETURN pGBA_cpuInstance->armState;
}

GBA_t::OP_MODE_TYPE GBA_t::getARMMode()
{
	pGBA_cpuInstance->armMode = (OP_MODE_TYPE)pGBA_cpuInstance->registers.cpsr.psrFields.psrModeBits;
	RETURN pGBA_cpuInstance->armMode;
}

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

uint32_t GBA_t::getMemoryAccessCycles(GBA_WORD mCurrentAddress, MEMORY_ACCESS_WIDTH mAccessWidth, MEMORY_ACCESS_SOURCE mSource, MEMORY_ACCESS_TYPE accessType)
{
	auto isAddressInDifferentRegion = [&](uint32_t currentAddr, uint32_t previousAddr)
		{
			if ((currentAddr >> TWENTYFOUR) != (previousAddr >> TWENTYFOUR))
			{
				RETURN YES;
			}
			else
			{
				RETURN NO;
			}
		};

	uint32_t nCycles = ZERO;
	MEMORY_REGIONS mRegion = static_cast<MEMORY_REGIONS>(mCurrentAddress >> TWENTYFOUR);
	MEMORY_ACCESS_TYPE mType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;

	if (accessType == MEMORY_ACCESS_TYPE::AUTOMATIC)
	{
		if (isAddressInDifferentRegion(mCurrentAddress, pGBA_instance->GBA_state.gbaMemory.previouslyAccessedMemory) == YES)
		{
			mType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
		}
		else
		{
			GBA_WORD offset = ZERO;
			switch (mAccessWidth)
			{
			case MEMORY_ACCESS_WIDTH::EIGHT_BIT:     offset = ONE; BREAK;
			case MEMORY_ACCESS_WIDTH::SIXTEEN_BIT:   offset = TWO; BREAK;
			case MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT: offset = FOUR; BREAK;
			default:
				FATAL("Unknown memory access width");
			}

			// Check for 128KB boundary special case in GamePak ROM
			FLAG isGamePakROM =
				(mRegion >= MEMORY_REGIONS::REGION_FLASH_ROM0_L) &&
				(mRegion <= MEMORY_REGIONS::REGION_FLASH_ROM2_H);

			FLAG is128KBoundary = (mCurrentAddress & 0x1FFFF) == ZERO;

			if (isGamePakROM && is128KBoundary)
			{
				mType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
			}
			else
			{
				// Normal sequential check
				mType = (mCurrentAddress == pGBA_instance->GBA_state.gbaMemory.previouslyAccessedMemory + offset)
					? MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE
					: MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
			}
		}
	}
	else
	{
		// Override the access type if asked by caller (Needed for special conditions where normal algorithm is not suitable)
		mType = accessType;
	}

	if (pGBA_memory->setNextMemoryAccessType != MEMORY_ACCESS_TYPE::AUTOMATIC)
	{
		mType = pGBA_memory->setNextMemoryAccessType;
		pGBA_memory->setNextMemoryAccessType = MEMORY_ACCESS_TYPE::AUTOMATIC;
	}

	if (
		((TO_UINT8(mRegion) < TO_UINT8(MEMORY_REGIONS::TOTAL_MEMORY_REGIONS)) && (TO_UINT8(mRegion) >= ZERO))
		&&
		((TO_UINT8(mType) < TO_UINT8(MEMORY_ACCESS_TYPE::TOTAL_MEMORY_ACCESS_TYPES)) && (TO_UINT8(mType) >= ZERO))
		&&
		((TO_UINT8(mAccessWidth) < TO_UINT8(MEMORY_ACCESS_WIDTH::TOTAL_ACCESS_WIDTH_POSSIBLE)) && (TO_UINT8(mAccessWidth) >= ZERO))
		)
	{
		nCycles = WAIT_CYCLES[TO_UINT8(mRegion)][TO_UINT8(mType)][TO_UINT8(mAccessWidth)];
	}
	else
	{
		nCycles = ONE; // For invalid memory
	}

	// previous address <= current address 
	pGBA_instance->GBA_state.gbaMemory.previouslyAccessedMemory = mCurrentAddress;
	// previous access type <= current access type 
	pGBA_memory->getPreviousMemoryAccessType = mType;

#if (DEACTIVATED)
	if (mSource == MEMORY_ACCESS_SOURCE::CPU)
	{
		if (mType == MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)
		{
			LOG("NS : 0x%X; cycles = %d", mCurrentAddress, nCycles);
		}
		if (mType == MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)
		{
			LOG("S : 0x%X; cycles = %d", mCurrentAddress, nCycles);
		}
	}
#endif

	RETURN nCycles;
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
		pGBA_peripherals->mDMA0CNT_H.mDMAnCNT_HHalfWord = data;
		pGBA_instance->GBA_state.dma.cache[TO_UINT8(DMA::DMA0)].enSetTime = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.globalTimerCounter;
		pGBA_instance->GBA_state.dma.cache[TO_UINT8(DMA::DMA0)].needStartupDelay = YES;
		scheduleDMATrigger(DMA_TIMING::IMMEDIATE);
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
		pGBA_peripherals->mDMA1CNT_H.mDMAnCNT_HHalfWord = data;
		pGBA_instance->GBA_state.dma.cache[TO_UINT8(DMA::DMA1)].enSetTime = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.globalTimerCounter;
		pGBA_instance->GBA_state.dma.cache[TO_UINT8(DMA::DMA1)].needStartupDelay = YES;
		scheduleDMATrigger(DMA_TIMING::IMMEDIATE);
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
		pGBA_peripherals->mDMA2CNT_H.mDMAnCNT_HHalfWord = data;
		pGBA_instance->GBA_state.dma.cache[TO_UINT8(DMA::DMA2)].enSetTime = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.globalTimerCounter;
		pGBA_instance->GBA_state.dma.cache[TO_UINT8(DMA::DMA2)].needStartupDelay = YES;
		scheduleDMATrigger(DMA_TIMING::IMMEDIATE);
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
		pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HHalfWord = data;
		pGBA_instance->GBA_state.dma.cache[TO_UINT8(DMA::DMA3)].enSetTime = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.globalTimerCounter;
		pGBA_instance->GBA_state.dma.cache[TO_UINT8(DMA::DMA3)].needStartupDelay = YES;
		scheduleDMATrigger(DMA_TIMING::IMMEDIATE);
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
	case IO_TM0CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER0)].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM0CNT_H:
	{
		BIT timer0EnBeforeUpdate = pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HHalfWord = data;
		// Handles loading of "reload" to "counter" when timer is enabled (0 -> 1)
		if (timer0EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER0)].cache.counter = pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER0)].cache.reload;
			pGBA_peripherals->mTIMER0CNT_L = pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER0)].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.timerCounter[TO_UINT(TIMER::TIMER0)] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER0)].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER0)].startupDelay = TWO;
		}
		RETURN;
	}
	case IO_TM1CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER1)].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM1CNT_H:
	{
		BIT timer1EnBeforeUpdate = pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HHalfWord = data;
		if (timer1EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER1)].cache.counter = pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER1)].cache.reload;
			pGBA_peripherals->mTIMER1CNT_L = pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER1)].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.timerCounter[TO_UINT(TIMER::TIMER1)] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER1)].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER1)].startupDelay = TWO;
		}
		RETURN;
	}
	case IO_TM2CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER2)].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM2CNT_H:
	{
		BIT timer2EnBeforeUpdate = pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HHalfWord = data;
		if (timer2EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER2)].cache.counter = pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER2)].cache.reload;
			pGBA_peripherals->mTIMER2CNT_L = pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER2)].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.timerCounter[TO_UINT(TIMER::TIMER2)] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER2)].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER2)].startupDelay = TWO;
		}
		RETURN;
	}
	case IO_TM3CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER3)].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM3CNT_H:
	{
		BIT timer3EnBeforeUpdate = pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HHalfWord = data;
		if (timer3EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER3)].cache.counter = pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER3)].cache.reload;
			pGBA_peripherals->mTIMER3CNT_L = pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER3)].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.timerCounter[TO_UINT(TIMER::TIMER3)] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER3)].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TO_UINT(TIMER::TIMER3)].startupDelay = TWO;
		}
		RETURN;
	}
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
		pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTHalfWord = data;

		auto NonSequentialWaitStates = [&](uint32_t x)
			{
				switch (x)
				{
				case ZERO: RETURN FOUR;
				case ONE: RETURN THREE;
				case TWO: RETURN TWO;
				case THREE: RETURN EIGHT;
				default: FATAL("Invalid nonsequential WAITCNT setting: %d!", x); RETURN ZERO;
				}
			};

		if (ENABLED)
		{
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_NON_SEQ);
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_NON_SEQ);
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_H)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)];
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_H)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)];


			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_SEQ ? ONE : TWO;
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_SEQ ? ONE : TWO;
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_H)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_SEQ ? ONE : TWO;
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_H)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_SEQ ? ONE : TWO;

			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= ((WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)])
					+ (WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]));
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_H)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= ((WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM0_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]) * TWO);
		}

		if (ENABLED)
		{
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_NON_SEQ);
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_NON_SEQ);
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_H)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)];
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_H)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)];


			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_SEQ ? ONE : FOUR;
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_SEQ ? ONE : FOUR;
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_H)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_SEQ ? ONE : FOUR;
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_H)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_SEQ ? ONE : FOUR;

			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= ((WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)])
					+ (WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]));
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_H)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= ((WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM1_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]) * TWO);
		}

		if (ENABLED)
		{
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_NON_SEQ);
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_NON_SEQ);
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_H)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)];
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_H)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)];


			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_SEQ ? ONE : EIGHT;
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_SEQ ? ONE : EIGHT;
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_H)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_SEQ ? ONE : EIGHT;
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_H)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_SEQ ? ONE : EIGHT;

			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= ((WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)])
					+ (WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]));
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_H)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= ((WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_FLASH_ROM2_L)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]) * TWO);
		}

		if (ENABLED)
		{
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.SRAM_WAIT_CTRL);
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.SRAM_WAIT_CTRL);

			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)];
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)];

			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= ((WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)])
					+ (WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]));
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= ((WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)])
					+ (WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]));

			//

			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.SRAM_WAIT_CTRL);
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.SRAM_WAIT_CTRL);

			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::EIGHT_BIT)];
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::SIXTEEN_BIT)];

			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)];
			WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)]
				= WAIT_CYCLES[TO_UINT8(MEMORY_REGIONS::REGION_GAMEPAK_SRAM)][TO_UINT8(MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)][TO_UINT8(MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT)];
		}

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
				TODO("Do we need to check for (IE AND IF)=0 for enabling HALT?");
				// Refer "4000301h - HALTCNT - BYTE - Undocumented - Low Power Mode Control (W)" of  https://problemkaputt.de/gbatek-gba-system-control.htm
				CPUEVENT("[RUN] -> [HALT]");
				pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::HALT;
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

	if (pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::RUN)
	{
		if ((isAnyDMAInProgress() == YES) || (isAnyDMAScheduled() == YES))
		{
			cyclesInThisRun = processDMA();
		}

		if (cyclesInThisRun == RESET)
		{
			runCPUPipeline();
			cyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter = RESET;
		}
	}
	else if (pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::HALT)
	{
		if ((isAnyDMAInProgress() == YES) || (isAnyDMAScheduled() == YES))
		{
			// Cycles can never be zero; In actual device, clocks are always running
			// Since the emulation's clocks are based on CPU, even when other masters (eg: DMA) running, cycles can appear to be zero
			// Ideally, all master's should be running in parallel
			// However, our emulation works based on ticking the CPU, seeing how many cycles it ticked and ticking other masters by same cycles
			// But, when CPU is HALTED, we need an alternative source of cycles to tick other masters
			// We can use DMA for this if its is running, so basically when CPU is HALTED, "cpuCounter == dmaCounter (if dmaCounter != 0)"
			// If DMA is also not running, then we just tick by one until we reach vblank

			cyclesInThisRun = processDMA();
		}

		if (cyclesInThisRun == RESET)
		{
			InternalCycles(ONE);
			cyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter = RESET;
		}

		// Refer to http://problemkaputt.de/gbatek-gba-system-control.htm
		if (shouldUnHaltTheCPU() == YES)
		{
			pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::RUN;
			CPUEVENT("[HALT] -> [RUN]");
		}
	}
	else if (pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::STOP)
	{
		FATAL("CPU is in STOP mode");
	}
	else
	{
		FATAL("Unknown Halt Controller State");
	}

	processBackup();

	if (pGBA_instance->GBA_state.emulatorStatus.isCycleAccurate == NO)
	{
		CPUINFRA("CPU Cycles: %" PRId64, cyclesInThisRun);

		if (cyclesInThisRun == ZERO)
		{
			RETURN status;
		}

#if (GBA_LIMIT_ALL_SLAVE_SUBSYSTEM_TICKS == YES)
		for (INC32 ticks = ZERO; ticks < cyclesInThisRun; ticks++)
		{
			processTimer(ONE);
			processSIO(ONE);
			processAPU(ONE);
			processPPU(ONE);
		}
#else
		for (INC32 ticks = ZERO; ticks < cyclesInThisRun; ticks++)
		{
			processTimer(ONE);
			processSIO(ONE);
			processAPU(ONE);
		}
		processPPU(cyclesInThisRun);
#endif

	}

	RETURN status;
}

void GBA_t::InternalCycles(uint32_t cycles)
{
	if (cycles != ONE)
	{
		FATAL("I cycles always takes one clock");
	}

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter += cycles;

#if (DISABLED)
	// Refer https://discord.com/channels/465585922579103744/465586361731121162/1269384605136322571
	if (pGBA_memory->getPreviousMemoryAccessType == MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE)
	{
		pGBA_memory->setNextMemoryAccessType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
	}
#endif
}

void GBA_t::cpuIdleCycles(uint32_t cycles)
{
	InternalCycles(cycles);
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

#if (GBA_AGS_PATCHED_TEST_ENABLED == YES)
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
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (UnconditionalBranch() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (ConditionalBranch() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (MultipleLoadStore() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LongBranchWithLink() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (AddOffsetToStackPointer() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (PushPopRegisters() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreHalfword() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (SPRelativeLoadStore() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadAddress() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreWithImmediateOffset() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreWithRegisterOffset() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreSignExtendedByteHalfword() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (PCRelativeLoad() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (HiRegisterOperationsBranchExchange() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (ALUOperations() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (MoveCompareAddSubtractImmediate() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (AddSubtract() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (MoveShiftedRegister() == YES)
		{
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
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
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (BlockDataTransfer() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (BranchAndBranchLink() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (SoftwareInterrupt() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (Undefined() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (SingleDataTransfer() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (SingleDataSwap() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (MultiplyAndMultiplyAccumulate() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (HalfWordDataTransfer() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (psrTransfer() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (DataProcessing() == YES)
			{
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
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

			cpuIdleCycles(ONE);
		}
	}
	else
	{
		FATAL("Unknown Operating Mode");
	}

	ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.cpuCounter != RESET);
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
	LOG("--------------------------------------------");

	for (uint8_t jj = ZERO; jj < LO_GP_REGISTERS; jj++)
	{
		LOG("register %d			: %02x", jj, pGBA_instance->GBA_state.cpuInstance.registers.unbankedLORegisters[jj]);
	}
	for (uint8_t ii = ZERO; ii < REGISTER_BANKS; ii++)
	{
		for (uint8_t jj = ZERO; jj < HI_GP_REGISTERS; jj++)
		{
			if (jj + LO_GP_REGISTERS == SP)
			{
				LOG("bank %d register %d (SP)	: %02x", ii, (LO_GP_REGISTERS + jj), pGBA_instance->GBA_state.cpuInstance.registers.bankedHIRegisters[ii][jj]);
			}
			else if (jj + LO_GP_REGISTERS == LR)
			{
				LOG("bank %d register %d	(LR)		: %02x", ii, (LO_GP_REGISTERS + jj), pGBA_instance->GBA_state.cpuInstance.registers.bankedHIRegisters[ii][jj]);
			}
			else if (jj + LO_GP_REGISTERS == PC)
			{
				LOG("bank %d register %d	(PC)		: %02x", ii, (LO_GP_REGISTERS + jj), pGBA_instance->GBA_state.cpuInstance.registers.bankedHIRegisters[ii][jj]);
			}
			else
			{
				LOG("bank %d register %d			: %02x", ii, (LO_GP_REGISTERS + jj), pGBA_instance->GBA_state.cpuInstance.registers.bankedHIRegisters[ii][jj]);
			}
		}
		LOG("bank %d register (SPSR)		: %02x", ii, pGBA_instance->GBA_state.cpuInstance.registers.spsr[ii].psrMemory);
	}
	LOG("register (CPSR)		: %02x", pGBA_instance->GBA_state.cpuInstance.registers.cpsr.psrMemory);

	LOG("--------------------------------------------");
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
void GBA_t::cpuTick()
{
	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter++;

	syncOtherGBModuleTicks();
}

void GBA_t::syncOtherGBModuleTicks()
{
	timerTick();
	dmaTick();
	serialTick();
	rtcTick();
	apuTick();
	ppuTick();
}

void GBA_t::timerTick()
{
	;
}

void GBA_t::dmaTick()
{
	;
}

void GBA_t::serialTick()
{
	;
}

void GBA_t::rtcTick()
{
	;
}

void GBA_t::apuTick()
{
	;
}

void GBA_t::ppuTick()
{
	;
}
#pragma endregion CYCLE_ACCURATE

#pragma region CYCLE_COUNT_ACCURATE
#pragma endregion CYCLE_COUNT_ACCURATE

void GBA_t::requestInterrupts(GBA_INTERRUPT interrupt)
{
	pGBA_peripherals->mIFHalfWord.mIFHalfWord |= (ONE << ((uint16_t)interrupt));
}

FLAG GBA_t::shouldUnHaltTheCPU()
{
	if ((pGBA_peripherals->mIFHalfWord.mIFHalfWord
		& pGBA_peripherals->mIEHalfWord.mIEHalfWord
		& 0x3FFF) != ZERO)
	{
		RETURN YES;
	}

	RETURN NO;
}

FLAG GBA_t::isInterruptReadyToBeServed()
{
	if ((pGBA_peripherals->mIFHalfWord.mIFHalfWord
		& pGBA_peripherals->mIEHalfWord.mIEHalfWord
		& 0x3FFF) != ZERO)
	{
		RETURN YES;
	}
	else
	{
		RETURN NO;
	}
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

		cpuIdleCycles(ONE);

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
	// Inline setTimerCNTLRegister for direct access, avoiding the function call overhead.
	auto setTimerCNTLRegister = [&](TIMER timer, uint16_t value)
		{
			// Use a switch statement for better performance in this context.
			switch (timer)
			{
			case TIMER::TIMER0:
				pGBA_peripherals->mTIMER0CNT_L = value;
				BREAK;
			case TIMER::TIMER1:
				pGBA_peripherals->mTIMER1CNT_L = value;
				BREAK;
			case TIMER::TIMER2:
				pGBA_peripherals->mTIMER2CNT_L = value;
				BREAK;
			case TIMER::TIMER3:
				pGBA_peripherals->mTIMER3CNT_L = value;
				BREAK;
			default:
				FATAL("Unknown Timer : %d", TO_UINT8(timer));
				BREAK;
			}
		};

	// Use a lookup table for faster register retrieval.
	static mTIMERnCNT_HHalfWord_t* CNTHLUT[] = {
		&pGBA_peripherals->mTIMER0CNT_H,
		&pGBA_peripherals->mTIMER1CNT_H,
		&pGBA_peripherals->mTIMER2CNT_H,
		&pGBA_peripherals->mTIMER3CNT_H
	};

	auto timerCommonProcessing = [&](TIMER timerID, uint16_t reloadValueIfOverflow, mTIMERnCNT_HHalfWord_t* CNTH, INC64 timerCycles)
		{
			uint32_t oldTimerValue = pGBA_instance->GBA_state.timer[TO_UINT(timerID)].cache.counter;
			uint16_t newTimerValue = RESET;

			while (timerCycles != RESET)
			{
				// Timer increment
				++oldTimerValue;

				// Overflow
				if (oldTimerValue > 0xFFFF)
				{
					pGBA_instance->GBA_state.timer[TO_UINT(timerID)].overflow = YES;

					// Increase cascade events for next timers.
					++pGBA_instance->GBA_state.timer[TO_UINT(timerID)].cascadeEvents;

					// Account for few additional ticks that would have happened post reload (if overflow was not clean...i.e overflow resulted in value > 0xFFFF + 1)
					newTimerValue = (uint16_t)((uint32_t)oldTimerValue - ((uint32_t)0x10000));
					// Reload the internal counter
					newTimerValue += reloadValueIfOverflow;

					// Request interrupt (if needed)
					if (CNTH->mTIMERnCNT_HFields.TIMER_IRQ_EN == YES)
					{
						requestInterrupts((GBA_INTERRUPT)(TO_UINT(timerID) + TO_UINT(GBA_INTERRUPT::IRQ_TIMER0)));
					}

					// Indicate there is an overflow in Timer 0 or Timer 1 to APU
					if (timerID == TIMER::TIMER0 || timerID == TIMER::TIMER1)
					{
						if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
						{
							for (INC8 fifoID = DIRECT_SOUND_A; fifoID <= DIRECT_SOUND_B; ++fifoID)
							{
								if (pGBA_audio->FIFO[fifoID].timer == TO_UINT8(timerID))
								{
									if (pGBA_audio->FIFO[fifoID].size > ZERO)
									{
										pGBA_audio->FIFO[fifoID].latch = ((GBA_AUDIO_SAMPLE_TYPE)pGBA_audio->FIFO[fifoID].fifo[pGBA_audio->FIFO[fifoID].position] << ONE);
										pGBA_audio->FIFO[fifoID].position = (pGBA_audio->FIFO[fifoID].position + ONE) & THIRTYONE;
										pGBA_audio->FIFO[fifoID].size--;
									}
									else
									{
										pGBA_audio->FIFO[fifoID].latch = (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
									}
								}
								if (pGBA_audio->FIFO[fifoID].size < SIXTEEN)
								{
									// As per http://problemkaputt.de/gbatek-gba-dma-transfers.htm
									// only DMA 1 and DMA 2 can be requested to fill sound FIFO, that too in Special Mode

									mDMAnCNT_HHalfWord_t* dmacntH = ((fifoID == DIRECT_SOUND_A) ? &pGBA_peripherals->mDMA1CNT_H : &pGBA_peripherals->mDMA2CNT_H);

									if (dmacntH->mDMAnCNT_HFields.DMA_EN == SET
										&&
										dmacntH->mDMAnCNT_HFields.DMA_START_TIMING == TO_UINT8(DMA_TIMING::SPECIAL))
									{
										scheduleDMATrigger(DMA_TIMING::SPECIAL);
										DMA_SPECIAL_TIMING id = ((fifoID == DIRECT_SOUND_A) ? DMA_SPECIAL_TIMING::FIFO0 : DMA_SPECIAL_TIMING::FIFO1);
										scheduleSpecialDMATrigger(id);
									}
								}
							}
						}
					}
				}
				else
				{
					newTimerValue = (uint16_t)oldTimerValue;
					pGBA_instance->GBA_state.timer[TO_UINT(timerID)].overflow = NO;
				}

				pGBA_instance->GBA_state.timer[TO_UINT(timerID)].cache.counter = newTimerValue;
				// Set the TIMER CNTL register with current timer count (needed for read operation)
				setTimerCNTLRegister((TIMER)timerID, newTimerValue);

				// Decrement 'timerCycles'
				--timerCycles;
			}
		};

	pGBA_instance->GBA_state.timer[ZERO].cascadeEvents = RESET;
	pGBA_instance->GBA_state.timer[ONE].cascadeEvents = RESET;
	pGBA_instance->GBA_state.timer[TWO].cascadeEvents = RESET;
	pGBA_instance->GBA_state.timer[THREE].cascadeEvents = RESET;

	// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/929789580889178142
	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.globalTimerCounter += timerCycles;

	// Process timers in a loop
	for (INC8 timerID = ZERO; timerID < FOUR; timerID++)
	{
		mTIMERnCNT_HHalfWord_t* CNTH = CNTHLUT[timerID]; // Use cached pointer for CNTH

		// NOTE: Don't read from the actual register, read from the cached register as actual register will be overriden with "internal counter value"
		uint16_t CNTL = pGBA_instance->GBA_state.timer[timerID].cache.reload;

		FLAG isCountUpMode = (CNTH->mTIMERnCNT_HFields.COUNT_UP_TIMING == SET ? YES : NO);

		if ((isCountUpMode == YES) && (timerID == TO_UINT(TIMER::TIMER0)))
		{
			WARN("Timer 0 cannot be run in Cascade Mode");
			isCountUpMode = NO;
		}

		if (CNTH->mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			if (pGBA_instance->GBA_state.timer[timerID].startupDelay <= RESET)
			{
				pGBA_instance->GBA_state.timer[timerID].startupDelay = RESET;
				pGBA_instance->GBA_state.timer[timerID].currentState = ENABLED;

				if (isCountUpMode == NO)
				{
					// Refer : https://discord.com/channels/465585922579103744/465586361731121162/929789580889178142
					auto systemTimer = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.globalTimerCounter;
					auto prescalar = timerFrequency[CNTH->mTIMERnCNT_HFields.PRESCALER_SEL];
					auto moduloPow2Prescalar = prescalar - ONE;

					if ((systemTimer & moduloPow2Prescalar) == ZERO)
					{
						timerCommonProcessing((TIMER)timerID, CNTL, CNTH, timerCycles);
					}
				}
				else
				{
					while (pGBA_instance->GBA_state.timer[timerID - ONE].cascadeEvents > ZERO)
					{
						timerCommonProcessing((TIMER)timerID, CNTL, CNTH, timerCycles);
						--pGBA_instance->GBA_state.timer[timerID - ONE].cascadeEvents;
					}
				}
			}
			else
			{
				pGBA_instance->GBA_state.timer[timerID].startupDelay -= timerCycles;
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

void GBA_t::scheduleSpecialDMATrigger(DMA_SPECIAL_TIMING trigger)
{
	pGBA_instance->GBA_state.dma.specialTriggersForThisRun |= ((ONE << (TO_UINT8(trigger))) & 0x0F);
}

void GBA_t::cancelScheduledSpecialDMATrigger(DMA_SPECIAL_TIMING trigger)
{
	pGBA_instance->GBA_state.dma.specialTriggersForThisRun &= ((~(ONE << (TO_UINT8(trigger)))) & 0x0F);
}

MAP8 GBA_t::getSpecialDMASchedule()
{
	RETURN pGBA_instance->GBA_state.dma.specialTriggersForThisRun;
}

void GBA_t::scheduleDMATrigger(DMA_TIMING trigger)
{
	pGBA_instance->GBA_state.dma.triggersForThisRun |= ((ONE << (TO_UINT8(trigger))) & 0x0F);
}

void GBA_t::cancelScheduledDMATrigger(DMA_TIMING trigger)
{
	pGBA_instance->GBA_state.dma.triggersForThisRun &= ((~(ONE << (TO_UINT8(trigger)))) & 0x0F);
}

MAP8 GBA_t::getDMASchedule()
{
	RETURN pGBA_instance->GBA_state.dma.triggersForThisRun;
}

FLAG GBA_t::isThisDMATriggerScheduled(DMA_TIMING trigger)
{
	RETURN((pGBA_instance->GBA_state.dma.triggersForThisRun & ((ONE << (TO_UINT8(trigger))) & 0x0F)) > ZERO ? YES : NO);
}

FLAG GBA_t::isAnyDMAScheduled()
{
	RETURN((pGBA_instance->GBA_state.dma.triggersForThisRun == ZERO) ? NO : YES);
}

void GBA_t::findNextActiveDMA()
{
	if (pGBA_peripherals->mDMA0CNT_H.mDMAnCNT_HFields.DMA_EN == SET)
	{
		pGBA_instance->GBA_state.dma.currentlyActiveDMA = DMA::DMA0;
	}
	else if (pGBA_peripherals->mDMA1CNT_H.mDMAnCNT_HFields.DMA_EN == SET)
	{
		pGBA_instance->GBA_state.dma.currentlyActiveDMA = DMA::DMA1;
	}
	else if (pGBA_peripherals->mDMA2CNT_H.mDMAnCNT_HFields.DMA_EN == SET)
	{
		pGBA_instance->GBA_state.dma.currentlyActiveDMA = DMA::DMA2;
	}
	else if (pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN == SET)
	{
		pGBA_instance->GBA_state.dma.currentlyActiveDMA = DMA::DMA3;
	}
	else
	{
		pGBA_instance->GBA_state.dma.currentlyActiveDMA = DMA::NONE;
	}
}

FLAG GBA_t::isAnyDMAEnabled()
{
	findNextActiveDMA();
	RETURN((pGBA_instance->GBA_state.dma.currentlyActiveDMA == DMA::NONE) ? NO : YES);
}

void GBA_t::updateDMAInProgressFlag(FLAG flag)
{
	pGBA_instance->GBA_state.dma.anyDMAInMiddleOfTransaction = flag;
}

FLAG GBA_t::isAnyDMAInProgress()
{
	RETURN pGBA_instance->GBA_state.dma.anyDMAInMiddleOfTransaction;
}

INC64 GBA_t::processDMA()
{
	MAP8 dmaSchedule = getDMASchedule();
	MAP8 specialDmaSchedule = getSpecialDMASchedule();

	cancelScheduledDMATrigger(DMA_TIMING::IMMEDIATE);
	cancelScheduledDMATrigger(DMA_TIMING::VBLANK);
	cancelScheduledDMATrigger(DMA_TIMING::HBLANK);
	cancelScheduledDMATrigger(DMA_TIMING::SPECIAL);
	cancelScheduledSpecialDMATrigger(DMA_SPECIAL_TIMING::FIFO0);
	cancelScheduledSpecialDMATrigger(DMA_SPECIAL_TIMING::FIFO1);
	cancelScheduledSpecialDMATrigger(DMA_SPECIAL_TIMING::VIDEO);
	updateDMAInProgressFlag(CLEAR);

	auto getSourceModifier = [&](DMA dma)
		{
			mDMAnCNT_HHalfWord_t* CNTH = getDMACNTHRegister(dma);
			RETURN sourceModifierLUT[CNTH->mDMAnCNT_HFields.DMA_TRANSFER_TYPE][CNTH->mDMAnCNT_HFields.SRC_ADDR_CTRL];
		};

	auto getDestinationModifier = [&](DMA dma)
		{
			mDMAnCNT_HHalfWord_t* CNTH = getDMACNTHRegister(dma);
			RETURN destinationModifierLUT[CNTH->mDMAnCNT_HFields.DMA_TRANSFER_TYPE][CNTH->mDMAnCNT_HFields.DEST_ADDR_CTRL];
		};

	auto oneChunkTransfer = [&](ID dmaID, SBYTE sourceModifier, SBYTE destinationModifier)
		{
			TODO("Get more information on the \"Invalid DMA Source Address\" as mentioned below at %d in %s (Source other than NBA and SkyEmu)", __LINE__, __FILE__);
			// The need for "wordToBeTransfered" and "latchedData":
			// When DMA accesses invalid memory, it should just store the last latched data
			// Tricky part is lets assume the last valid access was GBA_HALFWORD and current invalid access in GBA_WORD
			// In this case, we need to store the GBA_HALFWORD to a 32 bit address (for the GBA_WORD DMA)
			// Basically the data is just "repeated at higher bits" and hence we do (latchedData = x << 16 | x)
			// Now Assume a case where last valid access was GBA_WORD and current invalid is GBA_HALFWORD
			// We can only store the first 16 or the last 16 bits
			// We decide which GBA_HALFWORD to store based on the destination address
			// If the 32 bit address is the "lower" 16 bit address (bit 1 is 0; we check bit 1 instead 0 as we are dealing with 16bit address), we store the lower 16 bits
			// If the 32 bit address is the "higher" 16 bit address (bit 1 is 1; we check bit 1 instead 0 as we are dealing with 16bit address), we store the higher 16 bits

			if (pGBA_instance->GBA_state.dma.cache[dmaID].chunkSize == DMA_SIZE::HALFWORD_PER_TRANSFER)
			{
				// If DMA SAD is valid address
				if (pGBA_instance->GBA_state.dma.cache[dmaID].source >= EXT_WORK_RAM_START_ADDRESS)
				{
					pGBA_instance->GBA_state.dma.cache[dmaID].wordToBeTransfered = (GBA_HALFWORD)readRawMemory<GBA_HALFWORD>(
						pGBA_instance->GBA_state.dma.cache[dmaID].source
						, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT
						, MEMORY_ACCESS_SOURCE::DMA
					);

					pGBA_instance->GBA_state.dma.cache[dmaID].latchedData =
						(pGBA_instance->GBA_state.dma.cache[dmaID].wordToBeTransfered << SIXTEEN)
						| pGBA_instance->GBA_state.dma.cache[dmaID].wordToBeTransfered;
				}
				else
				{
					if (pGBA_instance->GBA_state.dma.cache[dmaID].destination & TWO)
					{
						pGBA_instance->GBA_state.dma.cache[dmaID].wordToBeTransfered
							= (GBA_HALFWORD)(pGBA_instance->GBA_state.dma.cache[dmaID].latchedData >> SIXTEEN);
					}
					else
					{
						pGBA_instance->GBA_state.dma.cache[dmaID].wordToBeTransfered
							= (GBA_HALFWORD)(pGBA_instance->GBA_state.dma.cache[dmaID].latchedData);
					}

					++pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter;
				}

				writeRawMemory<GBA_HALFWORD>(
					pGBA_instance->GBA_state.dma.cache[dmaID].destination
					, (static_cast<GBA_HALFWORD>(pGBA_instance->GBA_state.dma.cache[dmaID].wordToBeTransfered))
					, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT
					, MEMORY_ACCESS_SOURCE::DMA
				);
			}
			else if (pGBA_instance->GBA_state.dma.cache[dmaID].chunkSize == DMA_SIZE::WORD_PER_TRANSFER)
			{
				// If DMA SAD is valid address
				if (pGBA_instance->GBA_state.dma.cache[dmaID].source >= EXT_WORK_RAM_START_ADDRESS)
				{
					pGBA_instance->GBA_state.dma.cache[dmaID].latchedData = readRawMemory<GBA_WORD>(
						pGBA_instance->GBA_state.dma.cache[dmaID].source
						, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT
						, MEMORY_ACCESS_SOURCE::DMA
					);
				}
				else
				{
					++pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter;
				}

				writeRawMemory<GBA_WORD>(
					pGBA_instance->GBA_state.dma.cache[dmaID].destination
					, pGBA_instance->GBA_state.dma.cache[dmaID].latchedData
					, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT
					, MEMORY_ACCESS_SOURCE::DMA
				);
			}
			else
			{
				FATAL("Unknown DMA Transfer Type");
			}

			pGBA_instance->GBA_state.dma.cache[dmaID].source += sourceModifier;
			pGBA_instance->GBA_state.dma.cache[dmaID].destination += destinationModifier;
			pGBA_instance->GBA_state.dma.cache[dmaID].length -= ONE;
		};

	// Reset the dma cycles counter

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter = RESET;

	// Service the DMA

	for (ID dmaID = TO_UINT8(DMA::DMA0); dmaID < TO_UINT8(DMA::TOTAL); dmaID++)
	{
		mDMAnCNT_HHalfWord_t* CNTH = getDMACNTHRegister((DMA)dmaID);

		// First check whether the DMA is enabled or not

		FLAG isDMAEnabled = ((CNTH->mDMAnCNT_HFields.DMA_EN == SET) ? YES : NO);

		// Handle the DMA start-up delay if just enabled via setting DMA_EN
		// Refer : https://discord.com/channels/465585922579103744/465586361731121162/1327022839655825428

		if (pGBA_instance->GBA_state.dma.cache[dmaID].needStartupDelay == YES
			&& ((pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.globalTimerCounter - pGBA_instance->GBA_state.dma.cache[dmaID].enSetTime) < TWO))
		{
			// It will definetly take minimum of 1 cpu cycle after DMA is enabled via runCpuPipeline to come till here where we actually check for DMA scheduling
			// If we took more than 1 cycle, then the start-up delay is already satisifed, so this 'if block' is useless
			// But, if we took only 1 cycle, we will enter this 'if block', we just clear the 'startDelay' flag, because by the time we come again, we surely would have satified the 2 cycle startup delay 

			pGBA_instance->GBA_state.dma.cache[dmaID].needStartupDelay = NO;

			// Since we cleared the schedule earlier while entering this function, we re-schedule it since we are still handling the start-up delay

			scheduleDMATrigger(DMA_TIMING::IMMEDIATE);

			CONTINUE;
		}

		// Clear 'startDelay' flag ... if we come till here, startup delay was definetely satisfied

		pGBA_instance->GBA_state.dma.cache[dmaID].needStartupDelay = NO;

		if (isDMAEnabled == YES)
		{
			// If this was a 0 -> 1 transition, latch the DMA registers

			if (pGBA_instance->GBA_state.dma.cache[dmaID].currentState == NO)
			{
				pGBA_instance->GBA_state.dma.cache[dmaID].currentState = isDMAEnabled;

				latchDMARegisters(dmaID);

				// Extract the target transfer count

				pGBA_instance->GBA_state.dma.cache[dmaID].target = pGBA_instance->GBA_state.dma.cache[dmaID].length;

				// Reset the count

				pGBA_instance->GBA_state.dma.cache[dmaID].count = RESET;
			}

			// Run the DMA loop

			SBYTE sourceModifier = getSourceModifier((DMA)dmaID);
			SBYTE destinationModifier = getDestinationModifier((DMA)dmaID);

			// Handle for FIFO DMA

			if (pGBA_instance->GBA_state.dma.cache[dmaID].isFIFODMA == YES)
			{
				destinationModifier = ZERO;	// Refer Sound DMA in http://problemkaputt.de/gbatek-gba-dma-transfers.htm
			}

			// If we are just about to start a new DMA transfer

			if (pGBA_instance->GBA_state.dma.cache[dmaID].count == RESET)
			{
				// Handle Prohibited case for DMA0

				if ((DMA_TIMING)pGBA_peripherals->mDMA0CNT_H.mDMAnCNT_HFields.DMA_START_TIMING == DMA_TIMING::SPECIAL)
				{
					WARN("Prohibited to run DMA0 in Special Mode");
					CONTINUE;
				}

				// Check whether trigger type is scheduled

				if (GETBIT(CNTH->mDMAnCNT_HFields.DMA_START_TIMING, dmaSchedule) == RESET)
				{
					CONTINUE;
				}

				// NOTE: Below if condition added to handle intro of "Pokemon_Ruby_Working_and_Clock_Fixed_Read_NFO_USA_GBA-MUGS.gba"
				if (ENABLED)
				{
					if ((DMA)dmaID == DMA::DMA1
						&& (DMA_TIMING)CNTH->mDMAnCNT_HFields.DMA_START_TIMING == DMA_TIMING::SPECIAL
						&& GETBIT(TO_UINT8(DMA_SPECIAL_TIMING::FIFO0), specialDmaSchedule) == RESET)
					{
						CONTINUE;
					}

					if ((DMA)dmaID == DMA::DMA2
						&& (DMA_TIMING)CNTH->mDMAnCNT_HFields.DMA_START_TIMING == DMA_TIMING::SPECIAL
						&& GETBIT(TO_UINT8(DMA_SPECIAL_TIMING::FIFO1), specialDmaSchedule) == RESET)
					{
						CONTINUE;
					}
				}

				TODO("DMA takes 2I(or 4I) cycles on top of the cycles for memory access. We need to account for one of them in the start of DMA");
#if (DISABLED)
				++pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter;
#endif
			}

			// If we are in the middle of the DMA transfer

			if (pGBA_instance->GBA_state.dma.cache[dmaID].count < pGBA_instance->GBA_state.dma.cache[dmaID].target)
			{
				// Perform the transfer

				oneChunkTransfer(dmaID, sourceModifier, destinationModifier);

				// Update the 'count'

				++pGBA_instance->GBA_state.dma.cache[dmaID].count;
			}

			// If we are done with the DMA transfer

			if (pGBA_instance->GBA_state.dma.cache[dmaID].count >= pGBA_instance->GBA_state.dma.cache[dmaID].target)
			{
				// Reset the count

				pGBA_instance->GBA_state.dma.cache[dmaID].count = RESET;

				// Enable interrupt if requested

				if (CNTH->mDMAnCNT_HFields.WORD_COUNT_END_IRQ == SET)
				{
					requestInterrupts((GBA_INTERRUPT)(dmaID + TO_UINT(GBA_INTERRUPT::IRQ_DMA0)));
				}

				// NOTE: Need to check for "DMA_TRANSFER_TYPE != DMA_TIMING::IMMEDIATE", else when repeat is set with immediate mode, we will remain stuck here for ever

				if ((CNTH->mDMAnCNT_HFields.DMA_REPEAT == SET) && ((DMA_TIMING)CNTH->mDMAnCNT_HFields.DMA_START_TIMING != DMA_TIMING::IMMEDIATE))
				{
					// DAD can be set during the Repeat Mode provided DAD is of increment/Reload type

					if (CNTH->mDMAnCNT_HFields.DEST_ADDR_CTRL == THREE)
					{
						pGBA_instance->GBA_state.dma.cache[dmaID].destination = getDMADADRegister((DMA)dmaID);
					}

					// CNTL can be set during the Repeat Mode as well...

					if (pGBA_instance->GBA_state.dma.cache[dmaID].isFIFODMA == YES)
					{
						pGBA_instance->GBA_state.dma.cache[dmaID].length = FOUR;	// Refer Sound DMA in http://problemkaputt.de/gbatek-gba-dma-transfers.htm
					}
					else
					{
						pGBA_instance->GBA_state.dma.cache[dmaID].length = (getDMACNTLRegister((DMA)dmaID) & 0xFFFF);

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

					// Extract the target transfer count

					pGBA_instance->GBA_state.dma.cache[dmaID].target = pGBA_instance->GBA_state.dma.cache[dmaID].length;
				}
				else
				{
					CNTH->mDMAnCNT_HFields.DMA_EN = RESET;
					isDMAEnabled = NO;
				}

				TODO("DMA takes 2I(or 4I) cycles on top of the cycles for memory access.We account for one of them in the end of DMA");
#if (DISABLED)
				++pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter;
#endif
			}
		}

		// Update the current state

		pGBA_instance->GBA_state.dma.cache[dmaID].currentState = isDMAEnabled;

		// If we have run atleast one DMA, then we should exit the loop

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter > ZERO)
		{
			BREAK;
		}
	}

	if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter > ZERO)
	{
		updateDMAInProgressFlag(SET);
	}
	else
	{
		updateDMAInProgressFlag(CLEAR);
	}

	RETURN pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.dmaCounter;
}

DIM16 GBA_t::getChannelPeriod(AUDIO_CHANNELS channel)
{
	DIM16 period = ZERO;
	switch (channel)
	{
	case AUDIO_CHANNELS::CHANNEL_1:
		period = pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.FREQ;
		RETURN period;
	case AUDIO_CHANNELS::CHANNEL_2:
		period = pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.FREQ;
		RETURN period;
	case AUDIO_CHANNELS::CHANNEL_3:
		period = pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.SAMPLE_RATE;
		RETURN period;
	default:
		RETURN period;
	}
}

FLAG GBA_t::enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS channel)
{
	FLAG wasEnableSuccess = NO;

	if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
	{
		switch (channel)
		{
		case AUDIO_CHANNELS::CHANNEL_1:
			if (isDACEnabled(channel) == YES) // DAC check
			{
				// Since we triggered the channel, "length timer expiring" or "frequency sweep overflow" checks are reset
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = ENABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = ONE;
				wasEnableSuccess = YES;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = ZERO;
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.INITIAL = ZERO;
				wasEnableSuccess = NO;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_2:
			if (isDACEnabled(channel) == YES) // DAC check
			{
				// Since we triggered the channel, "length timer expiring" or "frequency sweep overflow" checks are reset
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = ENABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = ONE;
				wasEnableSuccess = YES;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = ZERO;
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.INITIAL = ZERO;
				wasEnableSuccess = NO;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_3:
			if (isDACEnabled(channel) == YES) // DAC check
			{
				// Since we triggered the channel, "length timer expiring" or "frequency sweep overflow" checks are reset
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = ENABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = ONE;
				wasEnableSuccess = YES;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = ZERO;
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.INITIAL = ZERO;
				wasEnableSuccess = NO;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_4:
			if (isDACEnabled(channel) == YES) // DAC check
			{
				// Since we triggered the channel, "length timer expiring" or "frequency sweep overflow" checks are reset
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = ENABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = ONE;
				wasEnableSuccess = YES;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = ZERO;
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.INITIAL = ZERO;
				wasEnableSuccess = NO;
			}
			BREAK;
		default:
			FATAL("Unknown Audio Channel : %d", TO_UINT8(channel));
			BREAK;
		}
	}

	RETURN wasEnableSuccess;
}

void GBA_t::continousDACCheck()
{
	if ((pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord & 0xF800) == ZERO)
	{
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = RESET;
		pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
	}

	if ((pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord & 0xF800) == ZERO)
	{
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = RESET;
		pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
	}

	if (pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.CHANNEL_3_OFF == ZERO)
	{
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = RESET;
		pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
	}

	if ((pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord & 0xF800) == ZERO)
	{
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = RESET;
		pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
	}
}

FLAG GBA_t::isDACEnabled(AUDIO_CHANNELS channel)
{
	FLAG status = NO;

	if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
	{
		switch (channel)
		{
		case AUDIO_CHANNELS::CHANNEL_1:
			if ((pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord & 0xF800) != ZERO)
			{
				status = YES;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_2:
			if ((pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord & 0xF800) != ZERO)
			{
				status = YES;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_3:
			if (pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.CHANNEL_3_OFF == ONE)
			{
				status = YES;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_4:
			if ((pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord & 0xF800) != ZERO)
			{
				status = YES;
			}
			BREAK;
		default:
			RETURN status;
		}
	}

	RETURN status;
}

FLAG GBA_t::isAudioChannelEnabled(AUDIO_CHANNELS channel)
{
	FLAG status = NO;

	if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
	{
		switch (channel)
		{
		case AUDIO_CHANNELS::CHANNEL_1:
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled == ENABLED)
			{
				status = YES;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_2:
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled == ENABLED)
			{
				status = YES;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_3:
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled == ENABLED)
			{
				status = YES;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_4:
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled == ENABLED)
			{
				status = YES;
			}
			BREAK;
		default:
			RETURN status;
		}
	}

	RETURN status;
}

FLAG GBA_t::isChannel3Active()
{
	RETURN(pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG == ONE
		&& pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.INITIAL == ONE
		&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled == ENABLED);
}

void GBA_t::tickChannel(AUDIO_CHANNELS channel, INC64 tCycles)
{
	if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
	{
		switch (channel)
		{
		case AUDIO_CHANNELS::CHANNEL_1:
			pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].frequencyTimer -= tCycles;
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].frequencyTimer <= ZERO)
			{
				uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_1)) * SIXTEEN;

				// Refer https://gbdev.io/pandocs/Audio_Registers.html#ff13--nr13-channel-1-period-low-write-only
				// Xly by 4 in GB/GBC was because of 4194304 / 1048576
				// so in GBA, it should be Xly by 16777216 / 1048576

				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].frequencyTimer += resetFrequencyTimer;

				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].waveDutyPosition++;
				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].waveDutyPosition >= EIGHT)
				{
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].waveDutyPosition = ZERO;
				}

				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled == ENABLED)
				{
					pGBA_instance->GBA_state.audio.sampleReadByChannel1 = SQUARE_WAVE_AMPLITUDE[pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.WAVE_PATTERN_DUTY]
						[pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].waveDutyPosition];
				}
				else
				{
					pGBA_instance->GBA_state.audio.sampleReadByChannel1 = TO_UINT16(MUTE_AUDIO);
				}
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_2:
			pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].frequencyTimer -= tCycles;
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].frequencyTimer <= ZERO)
			{
				uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_2)) * SIXTEEN;

				// Refer https://gbdev.io/pandocs/Audio_Registers.html#ff13--nr13-channel-1-period-low-write-only
				// Xly by 4 in GB/GBC was because of 4194304 / 1048576
				// so in GBA, it should be Xly by 16777216 / 1048576

				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].frequencyTimer += resetFrequencyTimer;

				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].waveDutyPosition++;
				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].waveDutyPosition >= EIGHT)
				{
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].waveDutyPosition = ZERO;
				}

				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled == ENABLED)
				{
					pGBA_instance->GBA_state.audio.sampleReadByChannel2 = SQUARE_WAVE_AMPLITUDE[pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.WAVE_PATTERN_DUTY]
						[pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].waveDutyPosition];
				}
				else
				{
					pGBA_instance->GBA_state.audio.sampleReadByChannel2 = (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
				}
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_3:
			pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].frequencyTimer -= tCycles;
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].frequencyTimer <= ZERO)
			{
				pGBA_instance->GBA_state.audio.didChannel3ReadWaveRamPostTrigger = YES;

				uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_3)) * EIGHT;

				// Refer https://gbdev.io/pandocs/Audio_Registers.html#ff13--nr13-channel-1-period-low-write-only
				// Xly by 4 in GB/GBC was because of 4194304 / 2097152
				// so in GBA, it should be Xly by 16777216 / 2097152

				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].frequencyTimer += resetFrequencyTimer;

				pGBA_instance->GBA_state.audio.waveRamCurrentIndex++;
				if (pGBA_instance->GBA_state.audio.waveRamCurrentIndex >= THIRTYTWO)
				{
					pGBA_instance->GBA_state.audio.waveRamCurrentIndex = ZERO;

					if ((pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_DIMENSION == ONE) && (pGBA_instance->GBA_state.audio.waveRamCurrentIndex == ZERO))
					{
						pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_BANK_NUMBER ^= ONE;
						std::swap_ranges(std::begin(pGBA_peripherals->mWAVERAM8), std::end(pGBA_peripherals->mWAVERAM8), std::begin(pGBA_memory->mBankedWAVERAM.mWAVERAM8));
					}
				}

				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled == ENABLED)
				{
					if (GETBIT(ZERO, pGBA_instance->GBA_state.audio.waveRamCurrentIndex) == ZERO)
					{
						pGBA_instance->GBA_state.audio.sampleReadByChannel3
							= pGBA_memory->mBankedWAVERAM.mWAVERAM8[(uint8_t)(pGBA_instance->GBA_state.audio.waveRamCurrentIndex / TWO)].samples.upperNibble;
					}
					else
					{
						pGBA_instance->GBA_state.audio.sampleReadByChannel3
							= pGBA_memory->mBankedWAVERAM.mWAVERAM8[(uint8_t)(pGBA_instance->GBA_state.audio.waveRamCurrentIndex / TWO)].samples.lowerNibble;
					}

					APUTODO("Source for the below operations");
					pGBA_instance->GBA_state.audio.sampleReadByChannel3 -= EIGHT;
					pGBA_instance->GBA_state.audio.sampleReadByChannel3 *= FOUR;
				}
				else
				{
					pGBA_instance->GBA_state.audio.sampleReadByChannel3 = (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
				}
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_4:
			pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].frequencyTimer -= tCycles;
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].frequencyTimer <= ZERO)
			{
				uint16_t divisor = AUDIO_CHANNEL_4_DIVISOR[pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.DIV_RATIO_OF_FREQ];
				uint16_t shiftAmount = pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.SHIFT_CLK_FREQ;
				uint16_t resetFrequencyTimer = divisor << (shiftAmount + TWO);

				// When compared to GB, in GBA shift amount is increased by 2 ... Xly by 4

				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].frequencyTimer += resetFrequencyTimer;

				uint16_t LFSR = pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].LFSR;
				BYTE LFSRWidth = pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.COUNTER_STEP;

				BYTE xorResult = (BYTE)((GETBIT(ZERO, LFSR)) ^ (GETBIT(ONE, LFSR)));
				LFSR = (uint16_t)((LFSR >> ONE) | (xorResult << FOURTEEN));

				if (LFSRWidth == LFSR_WIDTH_IS_7_BITS)
				{
					LFSR &= ~(ONE << SIX);
					LFSR |= (xorResult << SIX);
				}

				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].LFSR = LFSR;

				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled == ENABLED)
				{
					uint16_t LFSR = pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].LFSR;
					if (GETBIT(ZERO, (~LFSR)))
					{
						pGBA_instance->GBA_state.audio.sampleReadByChannel4 = EIGHT;
					}
					else
					{
						pGBA_instance->GBA_state.audio.sampleReadByChannel4 = -EIGHT;
					}
				}
				else
				{
					pGBA_instance->GBA_state.audio.sampleReadByChannel4 = (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;
				}
			}
			BREAK;
		default:
			RETURN;
		}
	}
}

void GBA_t::processSoundLength()
{
	if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
	{
		if (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.LENGTH_FLAG == ONE
			&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer > ZERO)
		{
			--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer;

			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer == ZERO)
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = RESET;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
			}
		}

		if (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.LENGTH_FLAG == ONE
			&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer > ZERO)
		{
			--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer;

			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer == ZERO)
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = RESET;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
			}
		}

		if (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.LENGTH_FLAG == ONE
			&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer > ZERO)
		{
			--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer;

			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer == ZERO)
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = RESET;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
			}
		}

		if (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.LENGTH_FLAG == ONE
			&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer > ZERO)
		{
			--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer;

			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer == ZERO)
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = RESET;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
			}
		}
	}
}

SDIM32 GBA_t::getUpdatedFrequency()
{
	SDIM32 newFrequency = ZERO;

	if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
	{
		newFrequency = pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].shadowFrequency
			>> pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_SHIFT;

		if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_FREQ_DIR == ZERO)
		{
			newFrequency = pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].shadowFrequency + newFrequency;
		}
		else
		{
			pGBA_instance->GBA_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger = YES;
			newFrequency = pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].shadowFrequency - newFrequency;
		}

		if (newFrequency > 2047)
		{
			pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.INITIAL = RESET;
			pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = RESET;
			pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
		}
	}

	RETURN newFrequency;
}

void GBA_t::processFrequencySweep()
{
	if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
	{
		if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepTimer > ZERO)
		{
			--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepTimer;
		}

		if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepTimer == ZERO)
		{
			// reload the sweep timer
			if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME > ZERO)
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepTimer
					= pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepTimer
					= EIGHT;
			}

			// since the sweep timer has expired (and reloaded), update the frequency sweep
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepEnabled == ENABLED
				&& pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME > ZERO)
			{
				int32_t newFrequency = getUpdatedFrequency();

				if (newFrequency <= 2047 && pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_SHIFT > ZERO)
				{
					pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.FREQ = newFrequency & 0x7FF;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].shadowFrequency = newFrequency;

					performOverFlowCheck();
				}
			}
		}
	}
}

void GBA_t::processEnvelopeSweep()
{
	if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
	{
		if (pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_STEP_TIME != ZERO)
		{
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer > ZERO)
			{
				--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer;
			}

			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer == ZERO)
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer
					= pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_STEP_TIME;

				// volume is greater than 0 and we are in decrementing mode
				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume > 0x00
					&& pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_DIR == ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
				}
				// volume is less than 15 and we are in incrementing mode
				else if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume < 0x0F
					&& pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_DIR == ONE)
				{
					++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
				}

				// volume is either 0 or 15
				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume == 0x0F
					|| pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume == 0x00)
				{
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isVolumeEnvelopeStillDoingAutomaticUpdates = NO;
				}
			}
		}

		if (pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_STEP_TIME != ZERO)
		{
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer > ZERO)
			{
				--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer;
			}

			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer == ZERO)
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer
					= pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_STEP_TIME;

				// volume is greater than 0 and we are in decrementing mode
				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume > 0x00
					&& pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_DIR == ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
				}
				// volume is less than 15 and we are in incrementing mode
				else if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume < 0x0F
					&& pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_DIR == ONE)
				{
					++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
				}

				// volume is either 0 or 15
				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume == 0x0F
					|| pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume == 0x00)
				{
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isVolumeEnvelopeStillDoingAutomaticUpdates = NO;
				}
			}
		}

		if (pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_STEP_TIME != ZERO)
		{
			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer > ZERO)
			{
				--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer;
			}

			if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer == ZERO)
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer
					= pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_STEP_TIME;

				// volume is greater than 0 and we are in decrementing mode
				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume > 0x00
					&& pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_DIR == ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
				}
				// volume is less than 15 and we are in incrementing mode
				else if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume < 0x0F
					&& pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_DIR == ONE)
				{
					++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
				}

				// volume is either 0 or 15
				if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume == 0x0F
					|| pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume == 0x00)
				{
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isVolumeEnvelopeStillDoingAutomaticUpdates = NO;
				}
			}
		}
	}
}

GBA_AUDIO_SAMPLE_TYPE GBA_t::getAmplitude(AUDIO_CHANNELS channel)
{
	GBA_AUDIO_SAMPLE_TYPE amplitude = LO;

	switch (channel)
	{
	case AUDIO_CHANNELS::CHANNEL_1:
		amplitude = pGBA_instance->GBA_state.audio.sampleReadByChannel1;
		BREAK;
	case AUDIO_CHANNELS::CHANNEL_2:
		amplitude = pGBA_instance->GBA_state.audio.sampleReadByChannel2;
		BREAK;
	case AUDIO_CHANNELS::CHANNEL_3:
		amplitude = pGBA_instance->GBA_state.audio.sampleReadByChannel3;
		BREAK;
	case AUDIO_CHANNELS::CHANNEL_4:
		amplitude = pGBA_instance->GBA_state.audio.sampleReadByChannel4;
		BREAK;
	default:
		FATAL("Unknown Audio Channel : %d", TO_UINT8(channel));
		BREAK;
	}

	RETURN amplitude;
}

GBA_AUDIO_SAMPLE_TYPE GBA_t::getDACOutput(AUDIO_CHANNELS channel)
{
	GBA_AUDIO_SAMPLE_TYPE dacOutput = (GBA_AUDIO_SAMPLE_TYPE)MUTE_AUDIO;

	// Refer https://discord.com/channels/465585922579103744/465586361731121162/908490205814738974
	if (isDACEnabled(channel) == YES)
	{
		if (channel == AUDIO_CHANNELS::CHANNEL_3)
		{
			dacOutput = (getAmplitude(channel) * (BYTE)pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift);
		}
		else
		{
			dacOutput = (getAmplitude(channel) * (BYTE)pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)(channel)].currentVolume);
		}
	}

	RETURN dacOutput;
}

void GBA_t::captureDownsampledAudioSamples(INC64 sampleCount)
{
	constexpr int PSG_VOL[FOUR] = { ONE, TWO, FOUR, ZERO };
	constexpr int DMA_VOL[TWO] = { TWO, FOUR};

	pGBA_instance->GBA_state.audio.downSamplingRatioCounter += sampleCount;

	if (pGBA_instance->GBA_state.audio.downSamplingRatioCounter >= (CEIL(GBA_REFERENCE_CLOCK_HZ / EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA)))
	{
		pGBA_instance->GBA_state.audio.downSamplingRatioCounter -= (CEIL(GBA_REFERENCE_CLOCK_HZ / EMULATED_AUDIO_SAMPLING_RATE_FOR_GBA));

		GBA_AUDIO_SAMPLE_TYPE leftSample = TO_UINT16(MUTE_AUDIO);
		GBA_AUDIO_SAMPLE_TYPE rightSample = TO_UINT16(MUTE_AUDIO);

		GBA_AUDIO_SAMPLE_TYPE channel1Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_1);
		GBA_AUDIO_SAMPLE_TYPE channel2Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_2);
		GBA_AUDIO_SAMPLE_TYPE channel3Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_3);
		GBA_AUDIO_SAMPLE_TYPE channel4Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_4);

		// process left samples
		if (DISABLE_FIRST_PULSE_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_ENABLE_1L == ONE)
		{
			leftSample += channel1Sample;
		}
		if (DISABLE_SECOND_PULSE_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_ENABLE_2L == ONE)
		{
			leftSample += channel2Sample;
		}
		if (DISABLE_WAVE_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_ENABLE_3L == ONE)
		{
			leftSample += channel3Sample;
		}
		if (DISABLE_NOISE_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_ENABLE_4L == ONE)
		{
			leftSample += channel4Sample;
		}

		leftSample *= pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_MASTER_VOL_L;
		leftSample >>= (FIVE - PSG_VOL[pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.SOUND_VOL]);

		if (DISABLE_DMAA_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_A_EN_L == SET)
		{
			leftSample += (pGBA_audio->FIFO[DIRECT_SOUND_A].latch << DMA_VOL[pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_A_VOL]);
		}

		if (DISABLE_DMAB_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_B_EN_L == SET)
		{
			leftSample += (pGBA_audio->FIFO[DIRECT_SOUND_B].latch << DMA_VOL[pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_B_VOL]);
		}

		leftSample += pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASFields.BIAS_LVL;
		leftSample = std::clamp(GBA_AUDIO_SAMPLE_TYPE(leftSample), GBA_AUDIO_SAMPLE_TYPE(0), GBA_AUDIO_SAMPLE_TYPE(0x3FF));
		leftSample -= pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASFields.BIAS_LVL; // Getting back signed value

		leftSample *= THIRTYTWO;

		// process right samples
		if (DISABLE_FIRST_PULSE_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_ENABLE_1R == ONE)
		{
			rightSample += channel1Sample;
		}
		if (DISABLE_SECOND_PULSE_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_ENABLE_2R == ONE)
		{
			rightSample += channel2Sample;
		}
		if (DISABLE_WAVE_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_ENABLE_3R == ONE)
		{
			rightSample += channel3Sample;
		}
		if (DISABLE_NOISE_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_ENABLE_4R == ONE)
		{
			rightSample += channel4Sample;
		}

		rightSample *= pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.SOUND_MASTER_VOL_R;
		rightSample >>= (FIVE - PSG_VOL[pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.SOUND_VOL]);

		if (DISABLE_DMAA_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_A_EN_R == SET)
		{
			rightSample += (pGBA_audio->FIFO[DIRECT_SOUND_A].latch << DMA_VOL[pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_A_VOL]);
		}

		if (DISABLE_DMAB_CHANNEL == NO && pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_B_EN_R == SET)
		{
			rightSample += (pGBA_audio->FIFO[DIRECT_SOUND_B].latch << DMA_VOL[pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_B_VOL]);
		}

		rightSample += pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASFields.BIAS_LVL;
		rightSample = std::clamp(GBA_AUDIO_SAMPLE_TYPE(rightSample), GBA_AUDIO_SAMPLE_TYPE(0), GBA_AUDIO_SAMPLE_TYPE(0x3FF));
		rightSample -= pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASFields.BIAS_LVL; // Getting back signed value

		rightSample *= THIRTYTWO;

		if (pGBA_instance->GBA_state.audio.accumulatedTone >= AUDIO_BUFFER_SIZE_FOR_GBA)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) == YES)
			{
				auto gain = getEmulationVolume();

				gain += 0.05f;

				if (gain >= 1.000f)
				{
					gain = 0.9998f;
				}
				else if (gain <= 0.000f)
				{
					gain = 0.0001f;
				}

				setEmulationVolume(gain);
			}
			if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) == YES)
			{
				auto gain = getEmulationVolume();

				gain -= 0.05f;

				if (gain >= 1.000f)
				{
					gain = 0.9998f;
				}
				else if (gain <= 0.000f)
				{
					gain = 0.0001f;
				}

				setEmulationVolume(gain);
			}

			if (SDL_PutAudioStreamData(audioStream, pGBA_instance->GBA_state.audio.audioBuffer, sizeof(pGBA_instance->GBA_state.audio.audioBuffer)) == FAILURE)
			{
				SDL_Log("Could not put data on Audio stream, %s", SDL_GetError());
				FATAL("SDL_PutAudioStreamData Error");
			}

			pGBA_instance->GBA_state.audio.accumulatedTone = RESET;
		}
		else
		{
			pGBA_instance->GBA_state.audio.audioBuffer[pGBA_instance->GBA_state.audio.accumulatedTone] = leftSample;
			++pGBA_instance->GBA_state.audio.accumulatedTone;
			if (pGBA_instance->GBA_state.audio.accumulatedTone < AUDIO_BUFFER_SIZE_FOR_GBA)
			{
				pGBA_instance->GBA_state.audio.audioBuffer[pGBA_instance->GBA_state.audio.accumulatedTone] = rightSample;
				++pGBA_instance->GBA_state.audio.accumulatedTone;
			}

		}
	}

	RETURN;
}

void GBA_t::processAPU(INC64 apuCycles)
{
	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.apuCounter += apuCycles;

	// GBA Audio
	continousDACCheck();	// If DAC is disabled, then Channel needs to be disabled immediately

	tickChannel(AUDIO_CHANNELS::CHANNEL_1, apuCycles);
	tickChannel(AUDIO_CHANNELS::CHANNEL_2, apuCycles);
	tickChannel(AUDIO_CHANNELS::CHANNEL_3, apuCycles);
	tickChannel(AUDIO_CHANNELS::CHANNEL_4, apuCycles);

	if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.apuCounter >= static_cast<int64_t>(GBA_REFERENCE_CLOCK_HZ / APU_FRAME_SEQUENCER_RATE_HZ))	// 512 Hz
	{
		APUTODO("Make sure we are running @ 512Hz");

		if (pGBA_instance->GBA_state.audio.wasPowerCycled == YES)
		{
			APUTODO("Not handling power cycling!");
			pGBA_instance->GBA_state.audio.wasPowerCycled = NO;
		}

		pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter = FALSE;

		if ((pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.apuFrameCounter & ONE) == ZERO) // 256 Hz
		{
			pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter = TRUE;
			processSoundLength();
		}

		if ((pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.apuFrameCounter & THREE) == TWO) // 128 Hz
		{
			processFrequencySweep();
		}

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.apuFrameCounter == SEVEN) // 64 Hz
		{
			processEnvelopeSweep();
		}

		++pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.apuFrameCounter;
		pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.apuFrameCounter &= SEVEN;

		pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.apuCounter -= static_cast<int64_t>(GBA_REFERENCE_CLOCK_HZ / APU_FRAME_SEQUENCER_RATE_HZ);
	}

	captureDownsampledAudioSamples(apuCycles);
}

void GBA_t::playTheAudioFrame()
{
	RETURN;
}

#if (GBA_PPU_ENABLE_LAMBDA_FUNCTIONS == NO)
void GBA_t::RESET_PIXEL(uint32_t x, uint32_t y) 
{
	pGBA_display->gfx_obj[x][y] = ZERO;
	pGBA_display->objPriority[x][y] = DEFAULT_OBJ_PRIORITY;
	pGBA_display->gfx_obj_window[x][y] = DISABLED;
	pGBA_display->gfx_obj_mode[x][y] = OBJECT_MODE::NORMAL;
	for (ID bgID = BG0; bgID < BG3; bgID++)
	{
		pGBA_display->gfx_bg[bgID][x][y] = ZERO;
	}
}

FLAG GBA_t::GET_WINDOW_OUTPUT(uint32_t x, uint32_t y, FLAG win0in, FLAG win1in, FLAG winout, FLAG objin) 
{
	FLAG is_win0in = pGBA_display->gfx_window[WIN0][x][y];
	FLAG is_win1in = pGBA_display->gfx_window[WIN1][x][y];
	FLAG is_winobj = pGBA_display->gfx_obj_window[x][y];

	FLAG win0_display = pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.WIN0_DISP_FLAG == SET;
	FLAG win1_display = pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.WIN1_DISP_FLAG == SET;
	FLAG winobj_display = (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.OBJ_DISP_FLAG == SET && pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.SCREEN_DISP_OBJ == SET);
	FLAG winout_display = win0_display || win1_display || winobj_display;

	if (win0_display && is_win0in) RETURN win0in;
	if (win1_display && is_win1in) RETURN win1in;
	if (winobj_display && is_winobj) RETURN objin;
	if (winout_display) RETURN winout;

	RETURN ENABLED;
}

void GBA_t::HANDLE_WINDOW_FOR_BG(uint32_t x, uint32_t y, ID bgID) 
{
	if (x >= (uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES || y >= (uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES) RETURN;

	FLAG win0in = CLEAR, win1in = CLEAR, winout = CLEAR, objin = CLEAR;

	switch (bgID)
	{
	case BG0:
		win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_0_EN == SET;
		win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_0_EN == SET;
		winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_0_EN == SET;
		objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_0_EN == SET;
		break;
	case BG1:
		win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_1_EN == SET;
		win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_1_EN == SET;
		winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_1_EN == SET;
		objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_1_EN == SET;
		break;
	case BG2:
		win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_2_EN == SET;
		win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_2_EN == SET;
		winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_2_EN == SET;
		objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_2_EN == SET;
		break;
	case BG3:
		win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_3_EN == SET;
		win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_3_EN == SET;
		winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_3_EN == SET;
		objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_3_EN == SET;
		break;
	default:
		FATAL("Unknown BG Layer");
		RETURN;
	}

	if (GET_WINDOW_OUTPUT(x, y, win0in, win1in, winout, objin) == NO)
	{
		pGBA_display->gfx_bg[bgID][x][y] = ZERO;
	}
}

void GBA_t::HANDLE_WINDOW_FOR_OBJ(uint32_t x, uint32_t y) 
{
	if (x >= (uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES || y >= (uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES) RETURN;

	FLAG win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_OBJ_EN == SET;
	FLAG win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_OBJ_EN == SET;
	FLAG winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_OBJ_EN == SET;
	FLAG objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_OBJ_EN == SET;

	if (GET_WINDOW_OUTPUT(x, y, win0in, win1in, winout, objin) == NO)
	{
		pGBA_display->gfx_obj[x][y] = ZERO;
	}
}

FLAG GBA_t::DOES_WINDOW_ALLOW_BLENDING(uint32_t x, uint32_t y) 
{
	if (x >= (uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES || y >= (uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)
		RETURN NO;

	FLAG win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_COLOR_SPL_EFFECT == SET;
	FLAG win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_COLOR_SPL_EFFECT == SET;
	FLAG winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_COLOR_SPL_EFFECT == SET;
	FLAG objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_COLOR_SPL_EFFECT == SET;

	RETURN GET_WINDOW_OUTPUT(x, y, win0in, win1in, winout, objin);
}

GBA_t::gbaColor_t GBA_t::BLEND(GBA_t::gbaColor_t layer1Pixel, GBA_t::gbaColor_t layer2Pixel, BYTE eva, BYTE evb) 
{
	GBA_t::gbaColor_t finalPixel = layer1Pixel;

	const int r_a = (layer1Pixel.raw >> 0) & 31;
	const int g_a = ((layer1Pixel.raw >> 4) & 62) | (layer1Pixel.raw >> 15);
	const int b_a = (layer1Pixel.raw >> 10) & 31;

	const int r_b = (layer2Pixel.raw >> 0) & 31;
	const int g_b = ((layer2Pixel.raw >> 4) & 62) | (layer2Pixel.raw >> 15);
	const int b_b = (layer2Pixel.raw >> 10) & 31;

	eva = std::min<int>(16, eva);
	evb = std::min<int>(16, evb);

	const int r = std::min<uint8_t>((r_a * eva + r_b * evb + 8) >> 4, 31);
	const int g = std::min<uint8_t>((g_a * eva + g_b * evb + 8) >> 4, 63) >> 1;
	const int b = std::min<uint8_t>((b_a * eva + b_b * evb + 8) >> 4, 31);

	finalPixel.RED = r;
	finalPixel.GREEN = g;
	finalPixel.BLUE = b;

	RETURN finalPixel;
}

GBA_t::gbaColor_t GBA_t::BRIGHTEN(GBA_t::gbaColor_t color, BYTE evy) 
{
	GBA_t::gbaColor_t finalPixel = color;

	evy = std::min<int>(16, evy);

	int r = (color.raw >> 0) & 31;
	int g = ((color.raw >> 4) & 62) | (color.raw >> 15);
	int b = (color.raw >> 10) & 31;

	r += ((31 - r) * evy + 8) >> 4;
	g += ((63 - g) * evy + 8) >> 4;
	b += ((31 - b) * evy + 8) >> 4;

	g >>= 1;

	finalPixel.RED = r;
	finalPixel.GREEN = g;
	finalPixel.BLUE = b;

	RETURN finalPixel;
}

GBA_t::gbaColor_t GBA_t::DARKEN(GBA_t::gbaColor_t color, BYTE evy) 
{
	GBA_t::gbaColor_t finalPixel = color;

	evy = std::min<int>(16, evy);

	int r = (color.raw >> 0) & 31;
	int g = ((color.raw >> 4) & 62) | (color.raw >> 15);
	int b = (color.raw >> 10) & 31;

	r -= (r * evy + 7) >> 4;
	g -= (g * evy + 7) >> 4;
	b -= (b * evy + 7) >> 4;

	g >>= 1;

	finalPixel.RED = r;
	finalPixel.GREEN = g;
	finalPixel.BLUE = b;

	RETURN finalPixel;
}

void GBA_t::MERGE_AND_DISPLAY_PHASE1()
{
	// Refer https://nba-emu.github.io/hw-docs/ppu/composite.html

	uint32_t x = pGBA_display->currentMergePixel;
	uint32_t y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	pGBA_display->mergeCache.xCoordinate = x;
	pGBA_display->mergeCache.yCoordinate = y;

	if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	pGBA_display->layersForBlending[ZERO] = BD;
	pGBA_display->layersForBlending[ONE] = BD;
	pGBA_display->colorNumberForBlending[ZERO].colorNumber = ZERO;
	pGBA_display->colorNumberForBlending[ONE].colorNumber = ZERO;
	pGBA_display->colorNumberForBlending[ZERO].isObject = NO;
	pGBA_display->colorNumberForBlending[ONE].isObject = NO;
	pGBA_display->priorities[ZERO] = THREE;
	pGBA_display->priorities[ONE] = THREE;

	if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == SET)
	{
		pGBA_display->colorForBlending[ZERO] = GBA_WHITE;
		RETURN;
	}

	auto getBGxCNT = [&](ID bgID) -> mBGnCNTHalfWord_t*
		{
			switch (bgID)
			{
			case BG0: RETURN &pGBA_peripherals->mBG0CNTHalfWord;
			case BG1: RETURN &pGBA_peripherals->mBG1CNTHalfWord;
			case BG2: RETURN &pGBA_peripherals->mBG2CNTHalfWord;
			case BG3: RETURN &pGBA_peripherals->mBG3CNTHalfWord;
			default: RETURN nullptr;
			}
		};

	ID minBG = MIN_MAX_BG_LAYERS[pGBA_display->currentPPUMode][ZERO];
	ID maxBG = MIN_MAX_BG_LAYERS[pGBA_display->currentPPUMode][ONE];

	for (ID bgID = minBG; bgID <= maxBG; ++bgID)
	{
		HANDLE_WINDOW_FOR_BG(x, y, bgID);
	}
	HANDLE_WINDOW_FOR_OBJ(x, y);

	INC8 numberOfBGs = ZERO;
	for (INC8 priority = ZERO; priority < FOUR; ++priority)
	{
		for (ID bgID = minBG; bgID <= maxBG; ++bgID)
		{
			auto* bgCNT = getBGxCNT(bgID);
			if (bgCNT && bgCNT->mBGnCNTFields.BG_PRIORITY == priority &&
				(pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTHalfWord & (0x100 << bgID)))
			{
				pGBA_display->bgListAccToPriority[numberOfBGs++] = bgID;
			}
		}
	}

	INC8 bgListIndexer = ZERO;
	for (INC8 targetLayer = ZERO; targetLayer < TWO; ++targetLayer)
	{
		while (bgListIndexer < numberOfBGs)
		{
			ID bgID = pGBA_display->bgListAccToPriority[bgListIndexer++];
			if (pGBA_display->gfx_bg[bgID][x][y] != ZERO)
			{
				pGBA_display->layersForBlending[targetLayer] = bgID;
				pGBA_display->colorNumberForBlending[targetLayer].colorNumber = pGBA_display->gfx_bg[bgID][x][y];
				pGBA_display->colorNumberForBlending[targetLayer].isObject = NO;
				auto* bgCNT = getBGxCNT(bgID);
				if (bgCNT)
				{
					pGBA_display->priorities[targetLayer] = bgCNT->mBGnCNTFields.BG_PRIORITY;
				}
				break;
			}
		}
	}

	if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.SCREEN_DISP_OBJ == SET && pGBA_display->gfx_obj[x][y] != RESET)
	{
		if (pGBA_display->objPriority[x][y] <= pGBA_display->priorities[ZERO])
		{
			pGBA_display->layersForBlending[ONE] = pGBA_display->layersForBlending[ZERO];
			pGBA_display->colorNumberForBlending[ONE] = pGBA_display->colorNumberForBlending[ZERO];
			pGBA_display->layersForBlending[ZERO] = OBJ;
			pGBA_display->colorNumberForBlending[ZERO].colorNumber = pGBA_display->gfx_obj[x][y];
			pGBA_display->colorNumberForBlending[ZERO].isObject = YES;
		}
		else if (pGBA_display->objPriority[x][y] <= pGBA_display->priorities[ONE])
		{
			pGBA_display->layersForBlending[ONE] = OBJ;
			pGBA_display->colorNumberForBlending[ONE].colorNumber = pGBA_display->gfx_obj[x][y];
			pGBA_display->colorNumberForBlending[ONE].isObject = YES;
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		if ((pGBA_display->colorNumberForBlending[i].colorNumber & IS_COLOR_NOT_PALETTE) == 0)
		{
			ID paletteIndex = pGBA_display->colorNumberForBlending[i].colorNumber;
			GBA_WORD address = PALETTE_RAM_START_ADDRESS + paletteIndex;
			if (pGBA_display->colorNumberForBlending[i].isObject == YES) address += 0x200;
			GBA_HALFWORD data = readRawMemory<GBA_HALFWORD>(address, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
			pGBA_display->colorForBlending[i].raw = data;
		}
		else
		{
			pGBA_display->colorForBlending[i].raw = pGBA_display->colorNumberForBlending[i].colorNumber;
		}
	}
}

void GBA_t::MERGE_AND_DISPLAY_PHASE2()
{
	uint32_t x = pGBA_display->mergeCache.xCoordinate;
	uint32_t y = pGBA_display->mergeCache.yCoordinate;

	if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	GBA_t::gbaColor_t finalPixel = { ZERO };

	if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == SET)
	{
		finalPixel = GBA_WHITE;
	}
	else
	{
		FLAG isAlphaBlendObj = pGBA_display->gfx_obj_mode[x][y] == OBJECT_MODE::ALPHA_BLENDING &&
			pGBA_display->layersForBlending[ZERO] == OBJ &&
			(pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0100 << pGBA_display->layersForBlending[ONE]));

		if (isAlphaBlendObj)
		{
			finalPixel = BLEND(
				pGBA_display->colorForBlending[ZERO],
				pGBA_display->colorForBlending[ONE],
				pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVA_COEFF,
				pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVB_COEFF);
		}
		else if (DOES_WINDOW_ALLOW_BLENDING(x, y) == YES && pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTFields.COLOR_SPL_EFFECTS != TO_UINT(COLOR_SPECIAL_EFFECTS::NORMAL))
		{
			switch ((COLOR_SPECIAL_EFFECTS)pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTFields.COLOR_SPL_EFFECTS)
			{
			case COLOR_SPECIAL_EFFECTS::ALPHA_BLENDING:
				if ((pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0001 << pGBA_display->layersForBlending[ZERO])) &&
					(pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0100 << pGBA_display->layersForBlending[ONE])))
				{
					finalPixel = BLEND(
						pGBA_display->colorForBlending[ZERO],
						pGBA_display->colorForBlending[ONE],
						pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVA_COEFF,
						pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVB_COEFF);
				}
				else
				{
					finalPixel = pGBA_display->colorForBlending[ZERO];
				}
				break;
			case COLOR_SPECIAL_EFFECTS::INCREASE_BRIGHTNESS:
				if (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0001 << pGBA_display->layersForBlending[ZERO]))
				{
					finalPixel = BRIGHTEN(pGBA_display->colorForBlending[ZERO], pGBA_peripherals->mBLDYHalfWord.mBLDYFields.EVY_COEFF);
				}
				else finalPixel = pGBA_display->colorForBlending[ZERO];
				break;
			case COLOR_SPECIAL_EFFECTS::DECREASE_BRIGHTNESS:
				if (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0001 << pGBA_display->layersForBlending[ZERO]))
				{
					finalPixel = DARKEN(pGBA_display->colorForBlending[ZERO], pGBA_peripherals->mBLDYHalfWord.mBLDYFields.EVY_COEFF);
				}
				else finalPixel = pGBA_display->colorForBlending[ZERO];
				break;
			default:
				FATAL("Unknown Color Special Effect");
			}
		}
		else
		{
			finalPixel = pGBA_display->colorForBlending[ZERO];
		}
	}

	RESET_PIXEL(x, y);

	pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].a = ALPHA;
	pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(finalPixel.RED);
	pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(finalPixel.GREEN);
	pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(finalPixel.BLUE);

	++pGBA_display->currentMergePixel;
}

void GBA_t::SET_INITIAL_OBJ_MODE()
{
	if (pGBA_memory->mGBAMemoryMap.mOamAttributes.mOamAttribute[ZERO].mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.ROTATE_SCALE_FLAG == SET)
	{
		pGBA_display->currentObjectIsAffine = OBJECT_TYPE::OBJECT_IS_AFFINE;
	}
	else
	{
		pGBA_display->currentObjectIsAffine = OBJECT_TYPE::OBJECT_IS_NOT_AFFINE;
	}
}

FLAG GBA_t::OBJ_A01_OBJ_CYCLE(ID oamID)
{
	PPUDEBUG("MODE2_A01_OBJ_CYCLE for OAM%u", oamID);

	FLAG oamIDNeedsToBeRendered = NO;

	auto& objCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)];
	objCache.reset();

	if (oamID >= ONETWENTYEIGHT)
		RETURN oamIDNeedsToBeRendered;

	GBA_WORD oamAddress = OAM_START_ADDRESS + (oamID << THREE);
	mOamAttr01Word_t oamAttributes01 = { ZERO };
	oamAttributes01.raw = readRawMemory<GBA_WORD>(oamAddress, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::PPU);

	objCache.ObjAttribute.mOamAttr01Word.raw = oamAttributes01.raw;
	objCache.spriteYScreenCoordinate = oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.Y_COORDINATE;

	if (oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.ROTATE_SCALE_FLAG == SET)
	{
		pGBA_display->nextObjectIsAffine = OBJECT_TYPE::OBJECT_IS_AFFINE;
		objCache.isAffine = YES;
		objCache.spriteHeight = SPRITE_HEIGHTS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_EN.OBJ_SIZE];
		objCache.spriteWidth = SPRITE_WIDTHS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_EN.OBJ_SIZE];

		objCache.isDoubleAffine = (oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_DISABLE_OR_DOUBLE_SIZE_FLAG == SET);
		objCache.isDisabled = NO;
		objCache.spriteXScreenCoordinate = oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_EN.X_COORDINATE;

		if (objCache.isDoubleAffine)
		{
			PPUTODO("Find documentation for shifting sprite to middle when double affine is enabled!");
			objCache.spriteXScreenCoordinate += (objCache.spriteWidth >> ONE);
			objCache.spriteYScreenCoordinate += (objCache.spriteHeight >> ONE);
		}

		if (objCache.spriteXScreenCoordinate > static_cast<SDIM32>(getScreenWidth()))
			objCache.spriteXScreenCoordinate -= 512;
	}
	else
	{
		pGBA_display->nextObjectIsAffine = OBJECT_TYPE::OBJECT_IS_NOT_AFFINE;
		objCache.isAffine = NO;
		objCache.spriteHeight = SPRITE_HEIGHTS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_DIS.OBJ_SIZE];
		objCache.spriteWidth = SPRITE_WIDTHS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_DIS.OBJ_SIZE];

		objCache.isDoubleAffine = NO;
		objCache.isDisabled = (oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_DISABLE_OR_DOUBLE_SIZE_FLAG == SET);
		objCache.spriteXScreenCoordinate = oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_DIS.X_COORDINATE;

		if (objCache.spriteXScreenCoordinate > static_cast<SDIM32>(getScreenWidth()))
			objCache.spriteXScreenCoordinate -= 512;
	}

	if (objCache.spriteYScreenCoordinate > getScreenHeight())
		objCache.spriteYScreenCoordinate -= 256;

	objCache.spriteMaxXScreenCoordinate = objCache.spriteXScreenCoordinate + objCache.spriteWidth;
	objCache.spriteMaxYScreenCoordinate = objCache.spriteYScreenCoordinate + objCache.spriteHeight;
	objCache.spriteMinXScreenCoordinate = objCache.spriteXScreenCoordinate;
	objCache.spriteMinYScreenCoordinate = objCache.spriteYScreenCoordinate;

	if (objCache.isDoubleAffine)
	{
		objCache.spriteMinXScreenCoordinate -= objCache.spriteWidth / TWO;
		objCache.spriteMaxXScreenCoordinate += objCache.spriteWidth / TWO;
		objCache.spriteMinYScreenCoordinate -= objCache.spriteHeight / TWO;
		objCache.spriteMaxYScreenCoordinate += objCache.spriteHeight / TWO;
	}

	if (!objCache.isDisabled &&
		objCache.vcount >= objCache.spriteMinYScreenCoordinate &&
		objCache.vcount < objCache.spriteMaxYScreenCoordinate)
	{
		oamIDNeedsToBeRendered = YES;
	}

	RETURN oamIDNeedsToBeRendered;
}

void GBA_t::OBJ_A2_OBJ_CYCLE(ID oamID)
{
	PPUDEBUG("MODE2_A2_OBJ_CYCLE for OAM%u", oamID);

	auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
	renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)];

	GBA_WORD oamAddress = OAM_START_ADDRESS + (oamID << THREE) + FOUR;
	mOamAttr23Word_t oamAttributes23 = { ZERO };
	oamAttributes23.mOamAttr2HalfWord.mOamAttr2HalfWord = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

	renderCache.ObjAttribute.mOamAttr23Word.raw = oamAttributes23.raw;

	if (ENABLED)
	{
		if (renderCache.isDoubleAffine)
		{
			auto& fetchCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)];
			renderCache.spriteXStart = -(fetchCache.spriteWidth >> ONE);
			renderCache.spriteXEnd = fetchCache.spriteWidth + (fetchCache.spriteWidth >> ONE);
			renderCache.spriteXPixelCoordinate = -(fetchCache.spriteWidth >> ONE);
		}
		else
		{
			renderCache.spriteXStart = ZERO;
			renderCache.spriteXEnd = renderCache.spriteWidth;
			renderCache.spriteXPixelCoordinate = ZERO;
		}

		renderCache.spriteYPixelCoordinate = renderCache.vcount - renderCache.spriteYScreenCoordinate;
		renderCache.baseTileID = renderCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.CHARACTER_NAME;
	}
}

void GBA_t::OBJ_PA_OBJ_CYCLE(ID oamID)
{
	PPUDEBUG("MODE2_PA_OBJ_CYCLE for OAM%u", oamID);

	auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
	GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + SIX;
	renderCache.affine.pa = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
}

void GBA_t::OBJ_PB_OBJ_CYCLE(ID oamID)
{
	PPUDEBUG("MODE2_PB_OBJ_CYCLE for OAM%u", oamID);

	auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
	GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + FOURTEEN;
	renderCache.affine.pb = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
}

void GBA_t::OBJ_PC_OBJ_CYCLE(ID oamID)
{
	PPUDEBUG("MODE2_PC_OBJ_CYCLE for OAM%u", oamID);

	// Refer to MODE2_PA_OBJ_CYCLE for info on the below equation
	auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
	GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + TWENTYTWO;
	renderCache.affine.pc = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
}

void GBA_t::OBJ_PD_OBJ_CYCLE(ID oamID)
{
	PPUDEBUG("MODE2_PD_OBJ_CYCLE for OAM%u", oamID);

	// Refer to MODE2_PA_OBJ_CYCLE for info on the below equation
	auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
	GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + THIRTY;
	renderCache.affine.pd = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
}

void GBA_t::OBJ_V_OBJ_CYCLE(ID oamID, OBJECT_TYPE isAffine, STATE8 state)
{
	PPUDEBUG("MODE2_V_OBJ_CYCLE[%u] for OAM%u", state, oamID);

	auto& objCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
	INC16 spriteXStart = objCache.spriteXPixelCoordinate;
	INC16 spriteXEnd = RESET;

	if (isAffine == OBJECT_TYPE::OBJECT_IS_NOT_AFFINE)
	{
		// Identity matrix (terms which get multiplied will be set to 1 and terms which gets added will be set to 0)
		objCache.affine.pa = 0xFF;
		objCache.affine.pb = 0x00;
		objCache.affine.pc = 0x00;
		objCache.affine.pd = 0xFF;

		spriteXEnd = spriteXStart + TWO;	// only 2 pixel per VRAM stage
	}
	else
	{
		spriteXEnd = spriteXStart + ONE;	// only 1 pixel per VRAM stage
	}

	INC32 transformedSpriteX = ZERO, transformedSpriteY = ZERO;
	INC16 spriteY = objCache.spriteYPixelCoordinate;
	SDIM32 Y = objCache.vcount;
	// update the spriteX coordinates (needed for next VRAM stages)
	objCache.spriteXPixelCoordinate = spriteXEnd;
	// check if this needs to be the last vram cycle for this object
	pGBA_display->lastVRAMCycleForObjFSM = (spriteXEnd == objCache.spriteXEnd) ? YES : NO;

	auto pa = objCache.affine.pa;
	auto pb = objCache.affine.pb;
	auto pc = objCache.affine.pc;
	auto pd = objCache.affine.pd;
	auto x0 = objCache.spriteWidth >> ONE;
	auto y0 = objCache.spriteHeight >> ONE;

	for (INC16 spriteX = spriteXStart; spriteX < spriteXEnd; spriteX++)
	{
		// NOTE: We need to get the screen coordinates before it got modified because of flip or affine
		// other wise tranformation from screen space to affine space will get cancelled
		// even for rotation, this is basically like flipping the sprite coordinated and then again flipping the screen coordinates, flip gets cancelled!
		int32_t screenX = spriteX + objCache.spriteXScreenCoordinate;

		if (isAffine == OBJECT_TYPE::OBJECT_IS_AFFINE)
		{
			/*
			*	according to formula mentioned in http://problemkaputt.de/gbatek-lcd-i-o-bg-rotation-scaling.htm
			*	affine can be calculated as follows
			*
			*	Using the following expressions,
			*	  x0,y0    Rotation Center
			*	  x1,y1    Old Position of a pixel (before rotation/scaling)
			*	  x2,y2    New position of above pixel (after rotation scaling)
			*	  A,B,C,D  BG2PA-BG2PD Parameters (as calculated above)
			*
			*	the following formula can be used to calculate x2,y2:
			*
			*	  x2 = A(x1-x0) + B(y1-y0) + x0
			*	  y2 = C(x1-x0) + D(y1-y0) + y0
			*
			*/

			// for X
			transformedSpriteX = (pa * (spriteX - x0)) + (pb * (spriteY - y0));
			// get the integer part of transformedSpriteX
			transformedSpriteX >>= EIGHT;
			transformedSpriteX += x0;

			// boundary check post transformation
			if (transformedSpriteX < 0 || transformedSpriteX >= objCache.spriteWidth)
				continue;

			// for Y
			transformedSpriteY = (pc * (spriteX - x0)) + (pd * (spriteY - y0));
			// get the integer part of transformedSpriteX
			transformedSpriteY >>= EIGHT;
			transformedSpriteY += y0;

			// boundary check post transformation
			if (transformedSpriteY < ZERO || transformedSpriteY >= objCache.spriteHeight)
				continue;
		}
		else
		{
			if (objCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_DIS.HOR_FLIP == SET)
				transformedSpriteX = objCache.spriteWidth - spriteX - ONE;
			else
				transformedSpriteX = spriteX;

			if (objCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_DIS.VER_FLIP == SET)
				transformedSpriteY = objCache.spriteHeight - spriteY - ONE;
			else
				transformedSpriteY = spriteY;
		}

		SDIM32 X = screenX;

		if (X >= ZERO && X < getScreenWidth())
		{
			// Y Transformation:
			if (ENABLED)
			{
				/*
				*	Let's assume the object on the screen is as follows:
				*
				*	 ________________________________________________________
				*	|														|
				*	|														|
				*	|														|
				* 	|														|
				* 	|				_________________						|
				* 	|				|		|		|						|
				*	|				|TY00	|TY01	|						|
				* 	|				|_______| ______|						|
				* 	|				|		|		|						|
				* 	|				|TY10	|TY11	|						|
				* 	|				|		|		|						|
				* 	|				__________________						|
				* 	|														|
				* 	|														|
				* 	|														|
				*	_________________________________________________________
				*
				*/

				// To get the row of the tile of interest
				// we just need to get transformedSpriteY / 8
				int32_t spriteYTile = transformedSpriteY >> THREE;
				// Now, to get the column of the tile of interest
				// we first get the witdth of the tile in terms of number of tiles
				// i.e. totalwidth of object / 8 gives you how many tiles are there in a row for a given object
				int32_t widthOfSpriteInTiles = objCache.spriteWidth >> THREE;

				ID tileIDOffsetBecauseOfY = ZERO;

				// Refer https://gbadev.net/gbadoc/sprites.html
				// Refer http://problemkaputt.de/gbatek-lcd-obj-vram-character-tile-mapping.htm
				// 2D mapping
				if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.OBJ_CHAR_VRAM_MAP == RESET)
				{
					// Assume tiles are arranged as follows for 2D mapping in memory

					/*
					*	TILE0000	| TILE0001	| TILE0002	| TILE0003	| TILE0004  | TILE0005  | TILE0006  .... | TILE0031
					*   TILE0032	| TILE0033	| TILE0034	| TILE0035	| TILE0036	| TILE0037	| TILE0038	.... | TILE0063
					*	:
					*   TILE0992	| TILE0993	| TILE0994	| TILE0995	| TILE0996	| TILE0997	| TILE0998	.... | TILE1023
					*/

					// pixels can be be arranged as a matrix of 32x32 or 16x32 tiles based on 16 or 256 color mode

					// To handle 256 color mode, we need to consider twice as much tiles, we handle this as (<< attribute0.color/palettes)

					// To get the tile offset, lets first process the y tile offset
					// if 8x8, tileIDOffsetBecauseOfY = 0
					// if 16x16, tileIDOffsetBecauseOfY = 32
					// if 32x32, tileIDOffsetBecauseOfY = 64

					// we can get above values of tileIDOffsetBecauseOfY using spriteYTile

					tileIDOffsetBecauseOfY = spriteYTile << FIVE;

					// Note: The reason we don't do anything special for 256 color mode is because "widthOfSpriteInTiles" will be confined to current row
					// So, next row will never get affected, so tile immediatly below (or few more rows below) the 1st row tile needs to be considered...so we just add multiple of 32 
				}
				// 1D mapping
				else
				{
					// Assume tiles are arranged as follows for 2D mapping in memory

					// Refer http://problemkaputt.de/gbatek-lcd-obj-oam-attributes.htm
					// In 256 color mode
					/*
					*	TILE0000	| TILE0002	| TILE0004	| TILE0006	| TILE0008  | TILE0010  | TILE0012  .... | TILE1022
					*/

					// Otherwise

					/*
					*	TILE0000	| TILE0001	| TILE0002	| TILE0003	| TILE0004  | TILE0005  | TILE0006  .... | TILE1023
					*/

					// To handle 256 color mode, we need to consider twice as much tiles, we handle this as (<< attribute0.color/palettes)

					// To get the tile offset, lets first process the y tile offset
					// if 8x8, tileIDOffsetBecauseOfY = 0
					// if 16x16, tileIDOffsetBecauseOfY = 2
					// if 32x32, tileIDOffsetBecauseOfY = 4

					// we can get above values of tileIDOffsetBecauseOfY using spriteYTile and widthOfSpriteInTiles

					tileIDOffsetBecauseOfY = widthOfSpriteInTiles * spriteYTile;
					// handle 256 color mode if needed (Xly by 2 as widthOfSpriteInTiles becomes double)
					tileIDOffsetBecauseOfY <<= objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.COLOR_PALETTES;
				}

				// after shifting the base tile id in y direction, this is the new base for shifing in x direction
				objCache.tileIDAfterAccountingForY = tileIDOffsetBecauseOfY;
			}

			// X Transformation:
			if (ENABLED)
			{
				// now need to get the tile id offset because of X
				// x offset would be the current sprite x coordinate / 8
				// refer to diagrams in MODE2_A2_OBJ_CYCLE for more info

				ID tileIDOffsetBecauseOfX = transformedSpriteX >> THREE;
				// Xly by 2 if 256 color mode because we need to skip every alternative (odd) tiles
				// refer OBJ Tile Number in http://problemkaputt.de/gbatek-lcd-obj-oam-attributes.htm
				// so, if tile number is 0, then we remain at 0
				// if tile number is 1, then we jump to 2
				// if tile number is 3, then we jump to 6
				// if tile number is 4, then we jump to 8
				// and so on...
				tileIDOffsetBecauseOfX <<= objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.COLOR_PALETTES;

				objCache.tileIDAfterAccountingForX = tileIDOffsetBecauseOfX;
			}

			int32_t actualTileID = objCache.baseTileID + objCache.tileIDAfterAccountingForY + objCache.tileIDAfterAccountingForX;

			// using the tileID, we need to fetch the corresponding tile data
			// for this, we need to first figure out addressInTileDataArea 

			// also, even with addressInTileDataArea, we need to figure out the data pertaining to the pixel within the tile of interest
			// NOTE: we still do %8 and not %width even for sprites bigger than 8x8 because
			// 1) for bigger tiles, the tileID would change for every 8x8 which takes care of getting appropriate address... no need to account for it in modulo again
			// 2) tile data is stored in terms of 8x8 

			SDIM32 xTileCoordinate = transformedSpriteX & SEVEN;
			SDIM32 yTileCoordinate = transformedSpriteY & SEVEN;
			GBA_WORD withinTileOffset = xTileCoordinate + (yTileCoordinate << THREE);

			// Refer : http://problemkaputt.de/gbatek-lcd-obj-overview.htm
			// Refer : http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
			// NOTE: in tile data area, granularity within the tile data is decided based on 4bpp mode or 8bpp mode
			// in 4bpp mode, each tile is 32 bytes, first 4 bytes for the topmost row of the tile
			// each byte representing two dots, the lower 4 bits define the color for the left and upper 4 bits the color for the right dot
			// in 8bpp mode, each tile is 64 bytes, first 8 bytes for the topmost row of the tile
			// each byte selects palette entry for each dot
			// 8bpp mode is the 256 color mode that is represented in attribute 0
			// Refer : https://gbadev.net/gbadoc/sprites.html
			// irrespective mode, each tile ID represents 32 bytes of data in sprite tile data area
			// so the memory address of a tile is roughly 0x06010000 + T*32
			const INC16 eachIndexReferences32BytesInShifts = FIVE; // Indicates 32 in power of 2 (to use it for optimized multiplication)
			PPUTODO("Each byte is 64 byte in 8bpp mode, so each index should reference 64 bytes in 8bpp? (Source : NBA)");

			INC32 paletteIndex = RESET;

			// 16 color mode (4bpp mode)
			if (objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.COLOR_PALETTES == RESET)
			{
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ 0x10000
					+ (actualTileID << eachIndexReferences32BytesInShifts)
					// each byte represents 2 dots
					// divide the withinTileOffset by 2 as single byte can represent 2 pixels
					// so "withinTileOffset" is halved
					+(withinTileOffset >> ONE);

				BYTE pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);

				// In 4bpp mode, only 4 bits out of pixelColorData represents actual color for pixel, other 4 bits are for the adjascent pixels
				// As mentioned in http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
				// lower 4 bits for left dot (lower X) and upper 4 bits for right dot (higher X)
				// take an example of dot 2 and dot 3
				// dot 2 needs to use bit0-3 and dot 3 needs to use bit4-7
				// withinTileOffset gives use dot number within tile
				// basically, for even dot, we simply extract first 4 bits
				// for odd dot, we want to shift bit4-7 right by 4 and then extract only first 4 bits
				// also, using the naming convention of left and right dot given in http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
				// tile data will be as follows:
				// row0 : LEFT(000), RIGHT(001), LEFT(002), RIGHT(003), LEFT(004), RIGHT(005), LEFT(006), RIGHT(007)
				// row1 : LEFT(008), RIGHT(009), LEFT(010), RIGHT(011), LEFT(012), RIGHT(013), LEFT(014), RIGHT(015)
				// so for odd "withinTileOffset" perform the shift and extract, else directly extract						

				// Odd means we need to shift right by 4
				if (withinTileOffset & ONE)
				{
					pixelColorNumberFromTileData >>= FOUR;
				}
				// Extract only the required 4 bits
				pixelColorNumberFromTileData &= 0x0F;

				// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
				paletteIndex = pixelColorNumberFromTileData << ONE;

				PPUTODO("Why paletteIndex != ZERO check is needed before accounting for palette bank number in 4bpp mode? \n\
						Will not having this check cause non-transparent pixel even when transparent is desired? (Source : NBA)");
				if (paletteIndex != ZERO)
				{
					// palette bank number is provided as part of attribute 2 in 4bpp mode
					// this palette number is the index into one of the 16 16-colored palettes (Refer : https://gbadev.net/gbadoc/sprites.html)
					// palette ram for sprites goes from 0x05000200 to 0x050003FF which is 512 bytes
					// Hence, each 16 16-colored palettes should take 32 bytes
					// OR
					// we can think that whole sprite palette area is divided into 16 palettes with each palette having 16 colors (Refer : http://problemkaputt.de/gbatek-lcd-color-palettes.htm)
					// each color is 2 bytes, so each palette takes 16 * 2 bytes = 32 bytes (Refer : http://problemkaputt.de/gbatek-lcd-color-palettes.htm)
					// Hence, each palette is 32 bytes
					// palette number in attribute 2 * THIRTYTWO will give the address of desired palette number in sprite palette area

					const DIM16 sizeOfEachPaletteInBytesInShifts = FIVE; // Indicates 32 in power of 2 (to use it for optimized multiplication) 
					paletteIndex += (objCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.PALETTE_NUMBER << sizeOfEachPaletteInBytesInShifts);
				}
			}
			else
			{
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ 0x10000
					+ (actualTileID << eachIndexReferences32BytesInShifts)
					+ withinTileOffset;

				BYTE pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);

				// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
				paletteIndex = pixelColorNumberFromTileData << ONE;
			}

			if ((OBJECT_MODE)objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_MODE == OBJECT_MODE::OBJ_WINDOW
				&&
				paletteIndex != ZERO)
			{
				pGBA_display->gfx_obj_window[X][Y] = ENABLED;
			}
			else if (objCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.PRIORITY < pGBA_display->objPriority[X][Y]
				||
				pGBA_display->gfx_obj[X][Y] == ZERO)
			{
				PPUTODO("Do we even need this paletteIndex != ZERO check, even if we populate paletteIndex of zero, wont merge take care of transparent sprites? (Source : NBA)");
				if (paletteIndex != ZERO)
				{
					pGBA_display->gfx_obj[X][Y] = paletteIndex;
					pGBA_display->gfx_obj_mode[X][Y] = (OBJECT_MODE)objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_MODE;
				}
				pGBA_display->objPriority[X][Y] = objCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.PRIORITY;
			}
		}
	}
}

INC8 GBA_t::INCREMENT_OAM_ID()
{
	// Check and increment objAccessOAMIDState only if it's within bounds
	if (pGBA_display->objAccessOAMIDState < ONETWENTYEIGHT - ONE)
	{
		++pGBA_display->objAccessOAMIDState;
		RETURN VALID;
	}
	RETURN INVALID;
}

void GBA_t::WIN_CYCLE()
{
	uint32_t xPixelCoordinate = pGBA_display->currentWinPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= static_cast<uint16_t>(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) ||
		(yPixelCoordinate >= static_cast<uint16_t>(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	// Helper lambda to handle the window logic for both Win0 and Win1
	auto handle_window = [&](uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2, FLAG& winH, FLAG& winV, int winIndex)
		{
			// Check and adjust the boundaries
			if (x2 > getScreenWidth() || x1 > x2) x2 = getScreenWidth();
			if (y2 > getScreenHeight() || y1 > y2) y2 = getScreenHeight();

			// Horizontal and vertical range checks
			winH = (xPixelCoordinate >= x1 && xPixelCoordinate <= x2) ? YES : NO;
			winV = (yPixelCoordinate >= y1 && yPixelCoordinate <= y2) ? YES : NO;

			// Update the window pixel array
			pGBA_display->gfx_window[winIndex][xPixelCoordinate][yPixelCoordinate] = winH && winV;
		};

	// Handle Win0
	FLAG win01H = NO, win01V = NO;
	handle_window(
		pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i1,
		pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i2,
		pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i1,
		pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i2,
		win01H, win01V, WIN0
	);

	// Handle Win1
	FLAG win11H = NO, win11V = NO;
	handle_window(
		pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i1,
		pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i2,
		pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i1,
		pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i2,
		win11H, win11V, WIN1
	);

	// Increment windows pixel counter
	++pGBA_display->currentWinPixel;
}

#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
void GBA_t::MODE0_M_BG_CYCLE(ID bgID)
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		RETURN;

	if (bgID < BG0 || bgID > BG3)
	{
		FATAL("Unknown BG in mode 0");
		RETURN;
	}

	pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
	pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

	GBA_WORD bgTileMapBaseAddr = 0;
	mBGnCNTHalfWord_t BGxCNT = { 0 };
	DIM16 hofs = 0;
	DIM16 vofs = 0;

	auto getBGConfig = [&](ID id) -> std::tuple<int, FLAG, uint16_t, uint16_t, uint16_t>
		{
			switch (id)
			{
			case BG0:
				RETURN {
					static_cast<int>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
					pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
					static_cast<uint16_t>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord),
					static_cast<uint16_t>(pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET),
					static_cast<uint16_t>(pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET)
				};
			case BG1:
				RETURN {
					static_cast<int>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
					pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
					static_cast<uint16_t>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord),
					static_cast<uint16_t>(pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET),
					static_cast<uint16_t>(pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET)
				};
			case BG2:
				RETURN {
					static_cast<int>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
					pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
					static_cast<uint16_t>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord),
					static_cast<uint16_t>(pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSFields.OFFSET),
					static_cast<uint16_t>(pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSFields.OFFSET)
				};
			case BG3:
				RETURN {
					static_cast<int>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
					pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
					static_cast<uint16_t>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord),
					static_cast<uint16_t>(pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSFields.OFFSET),
					static_cast<uint16_t>(pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSFields.OFFSET)
				};
			default:
				RETURN { 0, false, 0, 0, 0 };
			}
		};

	auto [bgTileMapBaseAddr_, is8bppMode, BGxCNT_, hofs_, vofs_] = getBGConfig(bgID);
	bgTileMapBaseAddr = bgTileMapBaseAddr_;
	pGBA_display->bgCache[bgID].is8bppMode = is8bppMode;
	BGxCNT.mBGnCNTHalfWord = BGxCNT_;
	hofs = hofs_;
	vofs = vofs_;

	DIM16 offsetSumX = xPixelCoordinate + hofs;
	DIM16 offsetSumY = yPixelCoordinate + vofs;

	ID SCn = 0;
	switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
	{
	case 0: SCn = 0; break;
	case 1: SCn = ((offsetSumX & 511) > 255) ? 1 : 0; break;
	case 2: SCn = ((offsetSumY & 511) > 255) ? 1 : 0; break;
	case 3:
		SCn = ((offsetSumX & 511) > 255) ? 1 : 0;
		SCn += ((offsetSumY & 511) > 255) ? 2 : 0;
		break;
	default:
		FATAL("Unknown screen size in mode 0");
		RETURN;
	}

	pGBA_display->bgCache[bgID].hofs = hofs;
	pGBA_display->bgCache[bgID].vofs = vofs;

	DIM16 tileMapX = offsetSumX & 255;
	DIM16 tileMapY = offsetSumY & 255;

	pGBA_display->bgCache[bgID].tileMapX = tileMapX;
	pGBA_display->bgCache[bgID].tileMapY = tileMapY;

	const DIM16 tilesPerRowInShifts = 5;
	DIM16 tileMapOffset = (tileMapX >> 3) + ((tileMapY >> 3) << tilesPerRowInShifts);
	tileMapOffset <<= 1;

	GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

	// NOTE: Needed for tonc's cbb_demo
	// Refer: https://www.coranac.com/tonc/text/regbg.htm
	if (addressInTileMapArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
	{
		pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
	}
	else
	{
		PPUTODO("As per NBA, this is set to some latched value");
		pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
	}
}

void GBA_t::MODE0_T_BG_CYCLE(ID bgID)
{
	auto X = pGBA_display->currentBgPixel;
	auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
		RETURN;

	if (bgID < BG0 || bgID > BG3)
	{
		FATAL("Unknown BG in mode 0");
		RETURN;
	}

	GBA_WORD bgTileDataBaseAddr =
		(bgID == BG0) ? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000) :
		(bgID == BG1) ? (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000) :
		(bgID == BG2) ? (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000) :
		(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

	auto xTileCoordinate = pGBA_display->bgCache[bgID].tileMapX & 7;
	auto yTileCoordinate = pGBA_display->bgCache[bgID].tileMapY & 7;

	xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES ? flipLUT[xTileCoordinate] : xTileCoordinate;
	yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES ? flipLUT[yTileCoordinate] : yTileCoordinate;

	GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << 3);
	DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

	if (pGBA_display->bgCache[bgID].is8bppMode == NO)
	{
		GBA_WORD addressInTileDataArea =
			VIDEO_RAM_START_ADDRESS +
			bgTileDataBaseAddr +
			(sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID) +
			(withinTileDataOffset >> 1);

		BYTE pixelColorNumberFor2Pixels = RESET;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pixelColorNumberFor2Pixels = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			PPUTODO("As per NBA, pixelColorNumberFor2Pixels is set to some latched value");
		}

		pixelColorNumberFor2Pixels >>= ((withinTileDataOffset & 1) ? 4 : 0);
		BYTE pixelColorNumber = pixelColorNumberFor2Pixels & 0x0F;
		INC32 paletteIndex = pixelColorNumber << 1;

		if (paletteIndex != 0)
		{
			const DIM16 sizeOfEachPaletteInBytesInShifts = 5;
			paletteIndex += (pGBA_display->bgCache[bgID].tileDescriptor.pb << sizeOfEachPaletteInBytesInShifts);
		}

		pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
	}
	else
	{
		GBA_WORD addressInTileDataArea =
			VIDEO_RAM_START_ADDRESS +
			bgTileDataBaseAddr +
			(sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID) +
			withinTileDataOffset;

		BYTE pixelColor = RESET;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pixelColor = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			PPUTODO("As per NBA, pixelColor is set to some latched value");
		}

		INC32 paletteIndex = pixelColor << 1;
		pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
	}
}

void GBA_t::MODE1_M_TEXT_BG_CYCLE(ID bgID)
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if (xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES) ||
		yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES))
	{
		RETURN;
	}

	if (bgID < BG0 || bgID > BG1)
	{
		FATAL("Unknown BG in mode 1");
		RETURN;
	}

	pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
	pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

	GBA_WORD bgTileMapBaseAddr = 0;
	mBGnCNTHalfWord_t BGxCNT = { 0 };
	DIM16 hofs = 0, vofs = 0;

	if (bgID == BG0)
	{
		bgTileMapBaseAddr = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800;
		pGBA_display->bgCache[BG0].is8bppMode = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
		BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord;
		hofs = pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET;
		vofs = pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET;
	}
	else
	{
		bgTileMapBaseAddr = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800;
		pGBA_display->bgCache[BG1].is8bppMode = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
		BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord;
		hofs = pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET;
		vofs = pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET;
	}

	DIM16 offsetSumX = xPixelCoordinate + hofs;
	DIM16 offsetSumY = yPixelCoordinate + vofs;

	ID SCn = 0;
	switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
	{
	case 0: SCn = 0; break;
	case 1: SCn = ((offsetSumX & 511) > 255) ? 1 : 0; break;
	case 2: SCn = ((offsetSumY & 511) > 255) ? 1 : 0; break;
	case 3:
		SCn = ((offsetSumX & 511) > 255) ? 1 : 0;
		SCn += ((offsetSumY & 511) > 255) ? 2 : 0;
		break;
	default:
		FATAL("Unknown screen size in mode 0");
		RETURN;
	}

	pGBA_display->bgCache[bgID].hofs = hofs;
	pGBA_display->bgCache[bgID].vofs = vofs;

	DIM16 tileMapX = offsetSumX & 255;
	DIM16 tileMapY = offsetSumY & 255;
	pGBA_display->bgCache[bgID].tileMapX = tileMapX;
	pGBA_display->bgCache[bgID].tileMapY = tileMapY;

	const DIM16 tilesPerRowInShifts = 5;
	DIM16 tileMapOffset = (tileMapX >> 3) + ((tileMapY >> 3) << tilesPerRowInShifts);
	tileMapOffset <<= 1;

	GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;
	GBA_HALFWORD tileDescriptor = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
	pGBA_display->bgCache[bgID].tileDescriptor.raw = tileDescriptor;
}

void GBA_t::MODE1_T_TEXT_BG_CYCLE(ID bgID)
{
	auto X = pGBA_display->currentBgPixel;
	auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) ||
		(Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	if (bgID < BG0 || bgID > BG1)
	{
		FATAL("Unknown BG in mode 1");
		RETURN;
	}

	GBA_WORD bgTileDataBaseAddr = (bgID == BG0)
		? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
		: (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

	auto xTileCoordinate = pGBA_display->bgCache[bgID].tileMapX & 7;
	auto yTileCoordinate = pGBA_display->bgCache[bgID].tileMapY & 7;

	if (pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES)
		xTileCoordinate = flipLUT[xTileCoordinate];
	if (pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES)
		yTileCoordinate = flipLUT[yTileCoordinate];

	GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << 3);
	DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

	if (pGBA_display->bgCache[bgID].is8bppMode == NO)
	{
		GBA_WORD address = VIDEO_RAM_START_ADDRESS + bgTileDataBaseAddr +
			sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID +
			(withinTileDataOffset >> 1);

		BYTE data = RESET;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (address < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			data = readRawMemory<BYTE>(address, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			PPUTODO("As per NBA, data is set to some latched value");
		}

		data >>= ((withinTileDataOffset & 1) ? 4 : 0);
		BYTE pixelColor = data & 0x0F;
		INC32 paletteIndex = pixelColor << 1;
		if (paletteIndex != 0)
		{
			const DIM16 shift = 5;
			paletteIndex += (pGBA_display->bgCache[bgID].tileDescriptor.pb << shift);
		}
		pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
	}
	else
	{
		GBA_WORD address = VIDEO_RAM_START_ADDRESS + bgTileDataBaseAddr +
			sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID +
			withinTileDataOffset;
		BYTE pixelColor = RESET;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (address < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pixelColor = readRawMemory<BYTE>(address, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			PPUTODO("As per NBA, pixelColor is set to some latched value");
		}

		INC32 paletteIndex = pixelColor << 1;
		pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
	}
}


#else
void GBA_t::RENDER_MODE0_MODE1_PIXEL_X(ID bgID, GBA_HALFWORD pixelData, STATE8 state)
{
	auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if (pGBA_display->currentBgPixelInTextMode[bgID] >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES))
	{
		RETURN;
	}

	if (pGBA_display->bgCache[bgID].is8bppMode == NO)
	{
		// 4bpp mode
		GBA_HALFWORD pixelColorNumberFor4PixelsFromTileData = pixelData;

		if (pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES)
		{
			// Flip the two 8-bit halves
			pixelColorNumberFor4PixelsFromTileData =
				((pixelColorNumberFor4PixelsFromTileData >> 8) | (pixelColorNumberFor4PixelsFromTileData << 8));

			// Flip 4-bit nibbles within each byte
			pixelColorNumberFor4PixelsFromTileData =
				(((pixelColorNumberFor4PixelsFromTileData & 0xF0F0) >> 4)
					| ((pixelColorNumberFor4PixelsFromTileData & 0x0F0F) << 4));
		}

		BYTE pixelColorNumberPerPixel = 0;
		INC32 paletteIndex = 0;

		if (pGBA_display->currentBgPixelInTextMode[bgID] < TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES))
		{
			pixelColorNumberPerPixel = ((pixelColorNumberFor4PixelsFromTileData >> (state * 4)) & 0x000F);
			paletteIndex = pixelColorNumberPerPixel << 1;

			if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops <= 0)
			{
				if (paletteIndex != 0)
				{
					const DIM16 sizeOfEachPaletteInBytesInShifts = 5; // 1 << 5 == *32
					paletteIndex += (pGBA_display->bgCache[bgID].tileDescriptor.pb << sizeOfEachPaletteInBytesInShifts);
				}
				pGBA_display->gfx_bg[bgID][pGBA_display->currentBgPixelInTextMode[bgID]++][Y] = paletteIndex;
				++pGBA_display->bgCache[bgID].subTileIndexer;
			}
		}

		if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops > 0)
		{
			--pGBA_display->bgCache[bgID].subTileScrollingPixelDrops;
		}
	}
	else
	{
		// 8bpp mode
		GBA_HALFWORD pixelColorNumberFor2PixelsFromTileData = pixelData;

		if (pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES)
		{
			pixelColorNumberFor2PixelsFromTileData =
				((pixelColorNumberFor2PixelsFromTileData >> 8) | (pixelColorNumberFor2PixelsFromTileData << 8));
		}

		BYTE pixelColorNumberPerPixel = 0;
		INC32 paletteIndex = 0;

		if (pGBA_display->currentBgPixelInTextMode[bgID] < TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES))
		{
			pixelColorNumberPerPixel = ((pixelColorNumberFor2PixelsFromTileData >> (state * 8)) & 0x00FF);
			paletteIndex = pixelColorNumberPerPixel << 1;

			if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops <= 0)
			{
				pGBA_display->gfx_bg[bgID][pGBA_display->currentBgPixelInTextMode[bgID]++][Y] = paletteIndex;
				++pGBA_display->bgCache[bgID].subTileIndexer;
			}
		}

		if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops > 0)
		{
			--pGBA_display->bgCache[bgID].subTileScrollingPixelDrops;
		}
	}
}

void GBA_t::MODE0_M_BG_CYCLE(ID bgID)
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixelInTextMode[bgID];
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
	pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

	GBA_WORD bgTileMapBaseAddr = 0;
	mBGnCNTHalfWord_t BGxCNT = { 0 };

	auto getBGConfig = [&](ID id) -> std::tuple<int, FLAG, uint16_t, uint16_t, uint16_t>
		{
			switch (id)
			{
			case BG0:
				RETURN {
					static_cast<int>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
					pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
					static_cast<uint16_t>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord),
					static_cast<uint16_t>(pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET),
					static_cast<uint16_t>(pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET)
				};
			case BG1:
				RETURN {
					static_cast<int>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
					pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
					static_cast<uint16_t>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord),
					static_cast<uint16_t>(pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET),
					static_cast<uint16_t>(pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET)
				};
			case BG2:
				RETURN {
					static_cast<int>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
					pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
					static_cast<uint16_t>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord),
					static_cast<uint16_t>(pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSFields.OFFSET),
					static_cast<uint16_t>(pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSFields.OFFSET)
				};
			case BG3:
				RETURN {
					static_cast<int>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
					pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
					static_cast<uint16_t>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord),
					static_cast<uint16_t>(pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSFields.OFFSET),
					static_cast<uint16_t>(pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSFields.OFFSET)
				};
			default:
				RETURN { 0, false, 0, 0, 0 };
			}
		};

	auto [bgTileMapBaseAddr_, is8bppMode, BGxCNT_, hofs_, vofs_] = getBGConfig(bgID);
	bgTileMapBaseAddr = bgTileMapBaseAddr_;
	pGBA_display->bgCache[bgID].is8bppMode = is8bppMode;
	BGxCNT.mBGnCNTHalfWord = BGxCNT_;
	pGBA_display->bgCache[bgID].hofs = hofs_;
	pGBA_display->bgCache[bgID].vofs = vofs_;

	DIM16 offsetSumX = xPixelCoordinate + pGBA_display->bgCache[bgID].hofs;
	DIM16 offsetSumY = yPixelCoordinate + pGBA_display->bgCache[bgID].vofs;

	ID SCn = 0;
	switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
	{
	case 0: SCn = 0; break;
	case 1: SCn = ((offsetSumX & 511) > 255) ? 1 : 0; break;
	case 2: SCn = ((offsetSumY & 511) > 255) ? 1 : 0; break;
	case 3:
		SCn = ((offsetSumX & 511) > 255) ? 1 : 0;
		SCn += ((offsetSumY & 511) > 255) ? 2 : 0;
		break;
	default:
		FATAL("Unknown screen size in mode 0");
		RETURN;
	}

	DIM16 tileMapX = offsetSumX & 255;
	DIM16 tileMapY = offsetSumY & 255;

	pGBA_display->bgCache[bgID].tileMapX = tileMapX;
	pGBA_display->bgCache[bgID].tileMapY = tileMapY;

	const DIM16 tilesPerRowInShifts = 5;
	DIM16 tileMapOffset = (tileMapX >> 3) + ((tileMapY >> 3) << tilesPerRowInShifts);
	tileMapOffset <<= 1;

	GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

	// NOTE: Needed for tonc's cbb_demo
	// Refer: https://www.coranac.com/tonc/text/regbg.htm
	if (addressInTileMapArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
	{
		pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
	}
	else
	{
		pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
		PPUTODO("As per NBA, this is set to some latched value");
	}

	pGBA_display->bgCache[bgID].subTileIndexer = 0;

	if ((tileMapX & 7) != 0)
	{
		PPUEVENT("Sub-tile scrolling in mode 0 bg%d!", bgID);
		if (xPixelCoordinate == 0)
		{
			pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = tileMapX & 7;
		}
		else
		{
			pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = 0;
		}
	}
	else
	{
		pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = 0;
	}
}

void GBA_t::MODE0_T_BG_CYCLE(ID bgID)
{
	auto X = pGBA_display->currentBgPixelInTextMode[bgID];
	auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	if (bgID < BG0 || bgID > BG3)
	{
		FATAL("Unknown BG in mode 0");
		RETURN;
	}

	if (ENABLED)
	{
		auto getBGConfig = [&](ID id) -> std::tuple<FLAG, uint16_t, uint16_t>
			{
				switch (id)
				{
				case BG0:
					RETURN {
						pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				case BG1:
					RETURN {
						pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				case BG2:
					RETURN {
						pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				case BG3:
					RETURN {
						pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
						static_cast<uint16_t>(pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSFields.OFFSET),
						static_cast<uint16_t>(pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSFields.OFFSET)
					};
				default:
					RETURN { false, 0, 0 };
				}
			};

		auto [is8bppMode, hofs_, vofs_] = getBGConfig(bgID);
		pGBA_display->bgCache[bgID].is8bppMode = is8bppMode;
		pGBA_display->bgCache[bgID].hofs = hofs_;
		pGBA_display->bgCache[bgID].vofs = vofs_;
	}

	GBA_WORD bgTileDataBaseAddr =
		(bgID == BG0)
		? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
		: (bgID == BG1)
		? (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
		: (bgID == BG2)
		? (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
		: (pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

	auto xTileCoordinate = ((X + pGBA_display->bgCache[bgID].hofs) & 255) & SEVEN;
	auto yTileCoordinate = ((Y + pGBA_display->bgCache[bgID].vofs) & 255) & SEVEN;

	xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
		? flipLUT[xTileCoordinate]
		: xTileCoordinate;

	yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
		? flipLUT[yTileCoordinate]
		: yTileCoordinate;

	GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
	DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

	if (pGBA_display->bgCache[bgID].is8bppMode == NO)
	{
		GBA_WORD addressInTileDataArea =
			VIDEO_RAM_START_ADDRESS
			+ bgTileDataBaseAddr
			+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
			+ (withinTileDataOffset >> ONE);

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = RESET;
			PPUTODO("As per NBA, this is set to some latched value");
		}

		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ZERO);
		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ONE);
		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, TWO);
		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, THREE);
	}
	else
	{
		GBA_WORD addressInTileDataArea =
			VIDEO_RAM_START_ADDRESS
			+ bgTileDataBaseAddr
			+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
			+ withinTileDataOffset;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = RESET;
			PPUTODO("As per NBA, this is set to some latched value");
		}

		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ZERO);
		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ONE);
	}
}

void GBA_t::MODE1_M_TEXT_BG_CYCLE(ID bgID)
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixelInTextMode[bgID];
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	if (bgID < BG0 || bgID > BG1)
	{
		FATAL("Unknown BG in mode 1");
		RETURN;
	}

	pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
	pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

	GBA_WORD bgTileMapBaseAddr = ZERO;
	mBGnCNTHalfWord_t BGxCNT = { ZERO };

	if (bgID == BG0)
	{
		bgTileMapBaseAddr = (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
		pGBA_display->bgCache[BG0].is8bppMode = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
		BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord;
		pGBA_display->bgCache[BG0].hofs = pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET;
		pGBA_display->bgCache[BG0].vofs = pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET;
	}
	else if (bgID == BG1)
	{
		bgTileMapBaseAddr = (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
		pGBA_display->bgCache[BG1].is8bppMode = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
		BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord;
		pGBA_display->bgCache[BG1].hofs = pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET;
		pGBA_display->bgCache[BG1].vofs = pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET;
	}

	DIM16 offsetSumX = xPixelCoordinate + pGBA_display->bgCache[bgID].hofs;
	DIM16 offsetSumY = yPixelCoordinate + pGBA_display->bgCache[bgID].vofs;

	ID SCn = ZERO;
	switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
	{
	case ZERO: SCn = ZERO; BREAK;
	case ONE: SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO; BREAK;
	case TWO: SCn = ((offsetSumY & 511) > 255) ? ONE : ZERO; BREAK;
	case THREE:
		SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO;
		SCn += ((offsetSumY & 511) > 255) ? TWO : ZERO;
		BREAK;
	default:
		FATAL("Unknown screen size in mode 0");
		RETURN;
	}

	DIM16 tileMapX = offsetSumX & 255;
	DIM16 tileMapY = offsetSumY & 255;

	pGBA_display->bgCache[bgID].tileMapX = tileMapX;
	pGBA_display->bgCache[bgID].tileMapY = tileMapY;

	const DIM16 tilesPerRowInShifts = FIVE;
	DIM16 tileMapOffset = (tileMapX >> THREE) + ((tileMapY >> THREE) << tilesPerRowInShifts);

	tileMapOffset <<= ONE;

	GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

	// NOTE: Needed for tonc's cbb_demo
	// Refer: https://www.coranac.com/tonc/text/regbg.htm
	if (addressInTileMapArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
	{
		pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
	}
	else
	{
		pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
		PPUTODO("As per NBA, this is set to some latched value");
	}

	pGBA_display->bgCache[bgID].subTileIndexer = ZERO;

	if ((tileMapX & SEVEN) != ZERO)
	{
		PPUEVENT("Sub-tile scrolling in mode 1 bg%d!", bgID);

		if (xPixelCoordinate == ZERO)
		{
			pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = tileMapX & SEVEN;
		}
		else
		{
			pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
		}
	}
	else
	{
		pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
	}
}

void GBA_t::MODE1_T_TEXT_BG_CYCLE(ID bgID)
{
	auto X = pGBA_display->currentBgPixelInTextMode[bgID];
	auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	// Early bounds check on X and Y
	if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	// Bounds check for bgID
	if (bgID < BG0 || bgID > BG1)
	{
		FATAL("Unknown BG in mode 1");
		RETURN;
	}

	if (ENABLED)
	{
		// Cache BG configuration values for fast access
		if (bgID == BG0)
		{
			pGBA_display->bgCache[BG0].is8bppMode = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
			pGBA_display->bgCache[BG0].hofs = pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET;
			pGBA_display->bgCache[BG0].vofs = pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET;
		}
		else if (bgID == BG1)
		{
			pGBA_display->bgCache[BG1].is8bppMode = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
			pGBA_display->bgCache[BG1].hofs = pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET;
			pGBA_display->bgCache[BG1].vofs = pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET;
		}
	}

	GBA_WORD bgTileDataBaseAddr = (bgID == BG0)
		? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
		: (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

	auto xTileCoordinate = ((X + pGBA_display->bgCache[bgID].hofs) & 255) & SEVEN; // % 8
	auto yTileCoordinate = ((Y + pGBA_display->bgCache[bgID].vofs) & 255) & SEVEN; // % 8

	xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
		? flipLUT[xTileCoordinate]
		: xTileCoordinate;

	yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
		? flipLUT[yTileCoordinate]
		: yTileCoordinate;

	GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
	DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

	if (pGBA_display->bgCache[bgID].is8bppMode == NO)
	{
		// 4bpp mode
		GBA_WORD addressInTileDataArea =
			VIDEO_RAM_START_ADDRESS
			+ bgTileDataBaseAddr
			+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
			+ (withinTileDataOffset >> ONE); // Divide by 2 because in 4bpp mode, A single halfword gives data for 4 pixels

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = readRawMemory<GBA_HALFWORD>(
				addressInTileDataArea,
				MEMORY_ACCESS_WIDTH::SIXTEEN_BIT,
				MEMORY_ACCESS_SOURCE::PPU
			);
		}
		else
		{
			pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = RESET;
			PPUTODO("As per NBA, this is set to some latched value");
		}	

		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ZERO);
		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ONE);
		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, TWO);
		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, THREE);
	}
	else
	{
		// 8bpp mode
		GBA_WORD addressInTileDataArea =
			VIDEO_RAM_START_ADDRESS
			+ bgTileDataBaseAddr
			+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
			+ withinTileDataOffset;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = readRawMemory<GBA_HALFWORD>(
				addressInTileDataArea,
				MEMORY_ACCESS_WIDTH::SIXTEEN_BIT,
				MEMORY_ACCESS_SOURCE::PPU
			);
		}
		else
		{
			pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = RESET;
			PPUTODO("As per NBA, this is set to some latched value");
		}

		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ZERO);
		RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ONE);
	}
}
#endif

void GBA_t::MODE1_M_AFFINE_BG_CYCLE()
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) ||
		(yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	GBA_WORD bgTileMapBaseAddr = (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
	DIM16 bgWidth = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);
	DIM16 bgHeight = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);
	FLAG isWrapAroundEnabled = (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT == ONE);

	uint32_t xPixelCoordinateAffine = (GBA_WORD)(pGBA_display->bgCache[BG2].affine.affineX >> EIGHT);
	uint32_t yPixelCoordinateAffine = (GBA_WORD)(pGBA_display->bgCache[BG2].affine.affineY >> EIGHT);

	pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
	pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);

	if (isWrapAroundEnabled == YES)
	{
		xPixelCoordinateAffine %= bgWidth;
		yPixelCoordinateAffine %= bgHeight;
	}

	pGBA_display->bgCache[BG2].xPixelCoordinateAffine = xPixelCoordinateAffine;
	pGBA_display->bgCache[BG2].yPixelCoordinateAffine = yPixelCoordinateAffine;

	uint32_t xTileCoordinateAffine = xPixelCoordinateAffine >> THREE;
	uint32_t yTileCoordinateAffine = yPixelCoordinateAffine >> THREE;

	auto bitsPerRow = bgWidth;
	auto tilesPerRow = bitsPerRow >> THREE;

	GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + xTileCoordinateAffine + (yTileCoordinateAffine * tilesPerRow);

	// NOTE: Needed for tonc's cbb_demo
	// Refer: https://www.coranac.com/tonc/text/regbg.htm
	if (addressInTileMapArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
	{
		pGBA_display->bgCache[BG2].fetchedTileID = readRawMemory<BYTE>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
	}
	else
	{
		pGBA_display->bgCache[BG2].fetchedTileID = RESET;
		PPUTODO("As per NBA, this is set to some latched value");
	}
}

void GBA_t::MODE1_T_AFFINE_BG_CYCLE()
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) ||
		(yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	GBA_WORD bgTileDataBaseAddr = ZERO;
	DIM16 bgWidth = ZERO;
	DIM16 bgHeight = ZERO;
	FLAG isWrapAroundEnabled = NO;

	bgTileDataBaseAddr = (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);
	bgWidth = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);
	bgHeight = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);

	uint32_t xPixelCoordinateAffine = pGBA_display->bgCache[BG2].xPixelCoordinateAffine;
	uint32_t yPixelCoordinateAffine = pGBA_display->bgCache[BG2].yPixelCoordinateAffine;

	uint32_t sizeOfEachTileData = 0x40;

	auto xTileCoordinateAffine = xPixelCoordinateAffine & SEVEN;
	auto yTileCoordinateAffine = yPixelCoordinateAffine & SEVEN;

	GBA_WORD withinTileDataOffsetAffine = xTileCoordinateAffine + (yTileCoordinateAffine << THREE);

	GBA_WORD addressInTileDataArea = VIDEO_RAM_START_ADDRESS + bgTileDataBaseAddr +
		(sizeOfEachTileData * pGBA_display->bgCache[BG2].fetchedTileID) + withinTileDataOffsetAffine;

	BYTE pixelColorNumberFromTileData = RESET;

	// NOTE: Needed for tonc's cbb_demo
	// Refer: https://www.coranac.com/tonc/text/regbg.htm
	if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
	{
		pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
	}
	else
	{
		PPUTODO("As per NBA, pixelColorNumberFromTileData is set to some latched value");
	}

	ID paletteIndex = pixelColorNumberFromTileData << ONE;

	if (xPixelCoordinateAffine >= bgWidth || yPixelCoordinateAffine >= bgHeight)
	{
		paletteIndex = ZERO;
	}

	pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = paletteIndex;
}

void GBA_t::MODE2_M_BG_CYCLE(ID bgID)
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) ||
		(yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	auto& bgCntHalfWord = (bgID == BG2)
		? pGBA_peripherals->mBG2CNTHalfWord
		: pGBA_peripherals->mBG3CNTHalfWord;

	GBA_WORD bgTileMapBaseAddr = bgCntHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800;
	DIM16 bgWidth = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;
	DIM16 bgHeight = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;
	FLAG isWrapAroundEnabled = bgCntHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT == ONE;

	auto& bgCache = pGBA_display->bgCache[bgID];
	auto& affineX = bgCache.affine.affineX;
	auto& affineY = bgCache.affine.affineY;

	uint32_t xPixelCoordinateAffine = affineX >> EIGHT;
	uint32_t yPixelCoordinateAffine = affineY >> EIGHT;

	auto affineXIncrement = (bgID == BG2)
		? pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed
		: pGBA_peripherals->mBG3PAHalfWord.mBGnPxHalfWord_Signed;
	auto affineYIncrement = (bgID == BG2)
		? pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed
		: pGBA_peripherals->mBG3PCHalfWord.mBGnPxHalfWord_Signed;

	affineX += affineXIncrement;
	affineY += affineYIncrement;

	if (isWrapAroundEnabled == YES)
	{
		xPixelCoordinateAffine %= bgWidth;
		yPixelCoordinateAffine %= bgHeight;
	}

	bgCache.xPixelCoordinateAffine = xPixelCoordinateAffine;
	bgCache.yPixelCoordinateAffine = yPixelCoordinateAffine;

	uint32_t xTileCoordinateAffine = xPixelCoordinateAffine >> THREE;
	uint32_t yTileCoordinateAffine = yPixelCoordinateAffine >> THREE;

	auto bitsPerRow = bgWidth;
	auto tilesPerRow = bitsPerRow >> THREE;

	GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + xTileCoordinateAffine + (yTileCoordinateAffine * tilesPerRow);

	// NOTE: Needed for tonc's cbb_demo
	// Refer: https://www.coranac.com/tonc/text/regbg.htm
	if (addressInTileMapArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
	{
		pGBA_display->bgCache[bgID].fetchedTileID = readRawMemory<BYTE>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
	}
	else
	{
		pGBA_display->bgCache[bgID].fetchedTileID = RESET;
		PPUTODO("As per NBA, this is set to some latched value");
	}
}

void GBA_t::MODE2_T_BG_CYCLE(ID bgID)
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) ||
		(yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	auto& bgCntHalfWord = (bgID == BG2)
		? pGBA_peripherals->mBG2CNTHalfWord
		: pGBA_peripherals->mBG3CNTHalfWord;

	GBA_WORD bgTileDataBaseAddr = bgCntHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000;
	DIM16 bgWidth = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;
	DIM16 bgHeight = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;

	uint32_t xPixelCoordinateAffine = pGBA_display->bgCache[bgID].xPixelCoordinateAffine;
	uint32_t yPixelCoordinateAffine = pGBA_display->bgCache[bgID].yPixelCoordinateAffine;

	const uint32_t sizeOfEachTileData = 0x40;

	auto xTileCoordinateAffine = xPixelCoordinateAffine & SEVEN;
	auto yTileCoordinateAffine = yPixelCoordinateAffine & SEVEN;

	GBA_WORD withinTileDataOffsetAffine = xTileCoordinateAffine + (yTileCoordinateAffine << THREE);

	GBA_WORD addressInTileDataArea = VIDEO_RAM_START_ADDRESS + bgTileDataBaseAddr +
		(sizeOfEachTileData * pGBA_display->bgCache[bgID].fetchedTileID) + withinTileDataOffsetAffine;

	BYTE pixelColorNumberFromTileData = RESET;

	// NOTE: Needed for tonc's cbb_demo
	// Refer: https://www.coranac.com/tonc/text/regbg.htm
	if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
	{
		pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
	}
	else
	{
		PPUTODO("As per NBA, pixelColorNumberFromTileData is set to some latched value");
	}

	ID paletteIndex = pixelColorNumberFromTileData << ONE;

	if (xPixelCoordinateAffine >= bgWidth || yPixelCoordinateAffine >= bgHeight)
	{
		paletteIndex = ZERO;
	}

	pGBA_display->gfx_bg[bgID][xPixelCoordinate][yPixelCoordinate] = paletteIndex;
}

void GBA_t::MODE3_B_BG_CYCLE()
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) ||
		(yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	uint32_t xPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineX) >> EIGHT);
	uint32_t yPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineY) >> EIGHT);

	if (xPixelCoordinateAffine >= getScreenWidth() || yPixelCoordinateAffine >= getScreenHeight())
	{
		pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = ZERO;
	}
	else
	{
		ID vramIndex = xPixelCoordinateAffine + (yPixelCoordinateAffine * getScreenWidth());

		vramIndex <<= ONE;

		GBA_WORD vramAddress = VIDEO_RAM_START_ADDRESS + vramIndex;

		GBA_HALFWORD colorFor1Pixel = RESET;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (vramAddress < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			colorFor1Pixel = readRawMemory<GBA_HALFWORD>(vramAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			PPUTODO("As per NBA, colorFor1Pixel is set to some latched value");
		}

		pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = (colorFor1Pixel | IS_COLOR_NOT_PALETTE);
	}

	pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
	pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);
}

void GBA_t::MODE4_B_BG_CYCLE()
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) ||
		(yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	uint32_t xPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineX) >> EIGHT);
	uint32_t yPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineY) >> EIGHT);

	if (xPixelCoordinateAffine >= getScreenWidth() || yPixelCoordinateAffine >= getScreenHeight())
	{
		pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = ZERO;
	}
	else
	{
		ID vramIndex = xPixelCoordinateAffine + (yPixelCoordinateAffine * getScreenWidth());

		GBA_WORD vramAddress = VIDEO_RAM_START_ADDRESS +
			(pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FRAME_SELECT * 0xA000) + vramIndex;

		ID pixelColorNumberFromTileData = RESET;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (vramAddress < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			pixelColorNumberFromTileData = readRawMemory<BYTE>(vramAddress, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			PPUTODO("As per NBA, pixelColorNumberFromTileData is set to some latched value");
		}

		ID paletteIndex = pixelColorNumberFromTileData << ONE;

		pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = paletteIndex;
	}

	pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
	pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);
}

void GBA_t::MODE5_B_BG_CYCLE()
{
	uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
	uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

	if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) ||
		(yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
	{
		RETURN;
	}

	uint32_t xPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineX) >> EIGHT);
	uint32_t yPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineY) >> EIGHT);

	if (xPixelCoordinateAffine >= ONEHUNDREDSIXTY || yPixelCoordinateAffine >= ONETWENTYEIGHT)
	{
		pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = ZERO;
	}
	else
	{
		ID vramIndex = xPixelCoordinateAffine + (yPixelCoordinateAffine * ONEHUNDREDSIXTY);

		vramIndex <<= ONE;

		GBA_WORD vramAddress = VIDEO_RAM_START_ADDRESS +
			(pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FRAME_SELECT * 0xA000) + vramIndex;

		GBA_HALFWORD colorFor1Pixel = RESET;

		// NOTE: Needed for tonc's cbb_demo
		// Refer: https://www.coranac.com/tonc/text/regbg.htm
		if (vramAddress < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
		{
			colorFor1Pixel = readRawMemory<GBA_HALFWORD>(vramAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		}
		else
		{
			PPUTODO("As per NBA, colorFor1Pixel is set to some latched value");
		}

		pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = (colorFor1Pixel | IS_COLOR_NOT_PALETTE);
	}

	pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
	pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);
}

void GBA_t::MODE0_BG_SEQUENCE(SSTATE32 state)
{
	switch (state)
	{
	case ZERO:
		MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG0);
#endif
		BREAK;
	case ONE:
		MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG1);
#endif
		BREAK;
	case TWO:
		MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG2);
#endif
		BREAK;
	case THREE:
		MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG3);
#endif
		++pGBA_display->currentBgPixel;
		BREAK;
	case FOUR:
		MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG0);
#endif
		BREAK;
	case FIVE:
		MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG1);
#endif
		BREAK;
	case SIX:
		MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG2);
#endif
		BREAK;
	case SEVEN:
		MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG3);
#endif
		++pGBA_display->currentBgPixel;
		BREAK;
	case EIGHT:
		MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG0);
#endif
		BREAK;
	case NINE:
		MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG1);
#endif
		BREAK;
	case TEN:
		MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG2);
#endif
		BREAK;
	case ELEVEN:
		MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG3);
#endif
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWELVE:
		MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG0);
#endif
		BREAK;
	case THIRTEEN:
		MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG1);
#endif
		BREAK;
	case FOURTEEN:
		MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG2);
#endif
		BREAK;
	case FIFTEEN:
		MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG3);
#endif
		++pGBA_display->currentBgPixel;
		BREAK;
	case SIXTEEN:
		MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG0);
#endif
		BREAK;
	case SEVENTEEN:
		MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG1);
#endif
		BREAK;
	case EIGHTEEN:
		MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG2);
#endif
		BREAK;
	case NINETEEN:
		MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG3);
#endif
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTY:
		MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG0);
#endif
		BREAK;
	case TWENTYONE:
		MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG1);
#endif
		BREAK;
	case TWENTYTWO:
		MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG2);
#endif
		BREAK;
	case TWENTYTHREE:
		MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG3);
#endif
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYFOUR:
		MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG0);
#endif
		BREAK;
	case TWENTYFIVE:
		MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG1);
#endif
		BREAK;
	case TWENTYSIX:
		MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG2);
#endif
		BREAK;
	case TWENTYSEVEN:
		MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG3);
#endif
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYEIGHT:
		MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG0);
#endif
		BREAK;
	case TWENTYNINE:
		MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG1);
#endif
		BREAK;
	case THIRTY:
		MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG2);
#endif
		BREAK;
	case THIRTYONE:
		MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE0_T_BG_CYCLE(BG3);
#endif
		++pGBA_display->currentBgPixel;
		BREAK;
	default:
		FATAL("PPU mode 0 (BG) out of sync");
	}
}

void GBA_t::MODE1_BG_SEQUENCE(SSTATE32 state)
{
	switch (state)
	{
	case ZERO:
		MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
		BREAK;
	case ONE:
		MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
		BREAK;
	case TWO:
		MODE1_M_AFFINE_BG_CYCLE();
		BREAK;
	case THREE:
		MODE1_T_AFFINE_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case FOUR:
		MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
		BREAK;
	case FIVE:
		MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
		BREAK;
	case SIX:
		MODE1_M_AFFINE_BG_CYCLE();
		BREAK;
	case SEVEN:
		MODE1_T_AFFINE_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case EIGHT:
		MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
		BREAK;
	case NINE:
		MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
		BREAK;
	case TEN:
		MODE1_M_AFFINE_BG_CYCLE();
		BREAK;
	case ELEVEN:
		MODE1_T_AFFINE_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWELVE:
		MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
		BREAK;
	case THIRTEEN:
		MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
		BREAK;
	case FOURTEEN:
		MODE1_M_AFFINE_BG_CYCLE();
		BREAK;
	case FIFTEEN:
		MODE1_T_AFFINE_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case SIXTEEN:
		MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
		BREAK;
	case SEVENTEEN:
		MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
		BREAK;
	case EIGHTEEN:
		MODE1_M_AFFINE_BG_CYCLE();
		BREAK;
	case NINETEEN:
		MODE1_T_AFFINE_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTY:
		MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
		BREAK;
	case TWENTYONE:
		MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
		BREAK;
	case TWENTYTWO:
		MODE1_M_AFFINE_BG_CYCLE();
		BREAK;
	case TWENTYTHREE:
		MODE1_T_AFFINE_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYFOUR:
		MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
		BREAK;
	case TWENTYFIVE:
		MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
		BREAK;
	case TWENTYSIX:
		MODE1_M_AFFINE_BG_CYCLE();
		BREAK;
	case TWENTYSEVEN:
		MODE1_T_AFFINE_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYEIGHT:
		MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
		BREAK;
	case TWENTYNINE:
		MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
		MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
		BREAK;
	case THIRTY:
		MODE1_M_AFFINE_BG_CYCLE();
		BREAK;
	case THIRTYONE:
		MODE1_T_AFFINE_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	default:
		FATAL("PPU mode 1 (BG) out of sync");
	}
}

void GBA_t::MODE2_BG_SEQUENCE(SSTATE32 state)
{
	switch (state)
	{
	case ZERO:
		MODE2_M_BG_CYCLE(BG3);
		BREAK;
	case ONE:
		MODE2_T_BG_CYCLE(BG3);
		BREAK;
	case TWO:
		MODE2_M_BG_CYCLE(BG2);
		BREAK;
	case THREE:
		MODE2_T_BG_CYCLE(BG2);
		++pGBA_display->currentBgPixel;
		BREAK;
	case FOUR:
		MODE2_M_BG_CYCLE(BG3);
		BREAK;
	case FIVE:
		MODE2_T_BG_CYCLE(BG3);
		BREAK;
	case SIX:
		MODE2_M_BG_CYCLE(BG2);
		BREAK;
	case SEVEN:
		MODE2_T_BG_CYCLE(BG2);
		++pGBA_display->currentBgPixel;
		BREAK;
	case EIGHT:
		MODE2_M_BG_CYCLE(BG3);
		BREAK;
	case NINE:
		MODE2_T_BG_CYCLE(BG3);
		BREAK;
	case TEN:
		MODE2_M_BG_CYCLE(BG2);
		BREAK;
	case ELEVEN:
		MODE2_T_BG_CYCLE(BG2);
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWELVE:
		MODE2_M_BG_CYCLE(BG3);
		BREAK;
	case THIRTEEN:
		MODE2_T_BG_CYCLE(BG3);
		BREAK;
	case FOURTEEN:
		MODE2_M_BG_CYCLE(BG2);
		BREAK;
	case FIFTEEN:
		MODE2_T_BG_CYCLE(BG2);
		++pGBA_display->currentBgPixel;
		BREAK;
	case SIXTEEN:
		MODE2_M_BG_CYCLE(BG3);
		BREAK;
	case SEVENTEEN:
		MODE2_T_BG_CYCLE(BG3);
		BREAK;
	case EIGHTEEN:
		MODE2_M_BG_CYCLE(BG2);
		BREAK;
	case NINETEEN:
		MODE2_T_BG_CYCLE(BG2);
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTY:
		MODE2_M_BG_CYCLE(BG3);
		BREAK;
	case TWENTYONE:
		MODE2_T_BG_CYCLE(BG3);
		BREAK;
	case TWENTYTWO:
		MODE2_M_BG_CYCLE(BG2);
		BREAK;
	case TWENTYTHREE:
		MODE2_T_BG_CYCLE(BG2);
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYFOUR:
		MODE2_M_BG_CYCLE(BG3);
		BREAK;
	case TWENTYFIVE:
		MODE2_T_BG_CYCLE(BG3);
		BREAK;
	case TWENTYSIX:
		MODE2_M_BG_CYCLE(BG2);
		BREAK;
	case TWENTYSEVEN:
		MODE2_T_BG_CYCLE(BG2);
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYEIGHT:
		MODE2_M_BG_CYCLE(BG3);
		BREAK;
	case TWENTYNINE:
		MODE2_T_BG_CYCLE(BG3);
		BREAK;
	case THIRTY:
		MODE2_M_BG_CYCLE(BG2);
		BREAK;
	case THIRTYONE:
		MODE2_T_BG_CYCLE(BG2);
		++pGBA_display->currentBgPixel;
		BREAK;
	default:
		FATAL("PPU mode 2 (BG) out of sync");
	}
}

void GBA_t::MODE3_BG_SEQUENCE(SSTATE32 state)
{
	switch (state)
	{
	case THREE:
		MODE3_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case SEVEN:
		MODE3_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case ELEVEN:
		MODE3_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case FIFTEEN:
		MODE3_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case NINETEEN:
		MODE3_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYTHREE:
		MODE3_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYSEVEN:
		MODE3_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case THIRTYONE:
		MODE3_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	}
}

void GBA_t::MODE4_BG_SEQUENCE(SSTATE32 state)
{
	switch (state)
	{
	case THREE:
		MODE4_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case SEVEN:
		MODE4_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case ELEVEN:
		MODE4_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case FIFTEEN:
		MODE4_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case NINETEEN:
		MODE4_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYTHREE:
		MODE4_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYSEVEN:
		MODE4_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case THIRTYONE:
		MODE4_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	}
}

void GBA_t::MODE5_BG_SEQUENCE(SSTATE32 state)
{
	switch (state)
	{
	case THREE:
		MODE5_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case SEVEN:
		MODE5_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case ELEVEN:
		MODE5_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case FIFTEEN:
		MODE5_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case NINETEEN:
		MODE5_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYTHREE:
		MODE5_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case TWENTYSEVEN:
		MODE5_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	case THIRTYONE:
		MODE5_B_BG_CYCLE();
		++pGBA_display->currentBgPixel;
		BREAK;
	}
}

void GBA_t::HANDLE_VCOUNT()
{
	if (pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_SETTING_LYC ==
		pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY)
	{
		if (
			pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNTER_IRQ_ENABLE &&
			pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_FLAG == RESET
			)
		{
			requestInterrupts(GBA_INTERRUPT::IRQ_VCOUNT);
		}

		pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_FLAG = SET;
	}
	else
	{
		pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_FLAG = RESET;
	}
}

void GBA_t::PROCESS_PPU_MODES(uint32_t ppuCycles, FLAG renderBG, FLAG renderWindow, FLAG renderObj, FLAG renderMerge)
{
	if (pGBA_display->currentPPUMode != pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE)
	{
		pGBA_display->ppuModeTransition = YES;
		pGBA_display->currentPPUMode = pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE;
	}
	else
	{
		pGBA_display->ppuModeTransition = NO;
	}

	// Handle mode specific processing
	if (ENABLED)
	{
		switch (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE)
		{
		case MODE0:
		{
			BREAK;
		}
		case MODE1:
		{
			BREAK;
		}
		case MODE2:
		{
			BREAK;
		}
		case MODE3:
		{
			BREAK;
		}
		case MODE4:
		{
			BREAK;
		}
		case MODE5:
		{
			BREAK;
		}
		default:
		{
			PPUWARN("Unknown PPU Mode");
			RETURN;
		}
		}
	}

	// Handle common processing
	if (ENABLED)
	{
		FLAG performWinRenderring = YES;
		FLAG performObjectRenderring = YES;
		FLAG performBgRenderring = YES;
		FLAG performMerging = YES;

		uint32_t winCycles = ppuCycles;
		uint32_t objCycles = ppuCycles;
		uint32_t bgCycles = ppuCycles;
		uint32_t mergeCycles = ppuCycles;

		const auto& currentScanline = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;
		auto& ppuCounter = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.ppuCounter;
		auto& currentMode = pGBA_display->currentPPUMode;

		ppuCounter += ppuCycles;

		if (pGBA_display->winWaitCyclesDone == NO && renderWindow == YES)
		{
			if (ppuCounter < WIN_WAIT_CYCLES)
			{
				performWinRenderring = NO;
			}
			else
			{
				winCycles = ppuCounter - WIN_WAIT_CYCLES;
				pGBA_display->winWaitCyclesDone = YES;
				performWinRenderring = YES;
			}
		}

		if (pGBA_display->objWaitCyclesDone == NO && renderObj == YES)
		{
			if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.ppuCounter < OBJECT_WAIT_CYCLES)
			{
				performObjectRenderring = (pGBA_display->allObjectsRenderedForScanline == NO) ? YES : NO;
			}
			else
			{
				if (
					pGBA_display->cyclesPerScanline > ZERO
					&&
					pGBA_display->allObjectsRenderedForScanline == NO
					&&
					pGBA_display->objAccessPatternState != OBJECT_ACCESS_PATTERN::OBJECT_A01
					)
				{
					if ((0x80 - pGBA_display->objAccessOAMIDState - ONE) > ZERO)
					{
						PPUMOREINFO("Missed %u objects in mode: %u; scanline %u", 0x80 - pGBA_display->objAccessOAMIDState - ONE, currentMode, pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY);
					}
					else
					{
						PPUMOREINFO("VRAM fsm still in mode: %u; scanline %u", currentMode, pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY);
					}
				}

				pGBA_display->allObjectsRenderedForScanline = NO;

				objCycles = ppuCounter - OBJECT_WAIT_CYCLES;
				pGBA_display->objWaitCyclesDone = YES;
				pGBA_display->objAccessOAMIDState = RESET;
				pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)].reset();
				pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)].reset();
				pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)].vcount = (SCOUNTER32)((currentScanline + ONE) % 228);
				pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
				pGBA_display->objAccessPattern = RESET;
				pGBA_display->cyclesPerSprite = RESET;
				pGBA_display->cyclesPerScanline = RESET;
				pGBA_display->vramCyclesStageForObjFSM = RESET;
				SET_INITIAL_OBJ_MODE();
				performObjectRenderring = YES;
			}
		}

		if (pGBA_display->bgWaitCyclesDone == NO && renderBG == YES)
		{
			COUNTER32 offsetForTextMode = ZERO;
			if (currentMode == MODE0 || currentMode == MODE1)
			{
				/*
				* Refer https://nba-emu.github.io/hw-docs/ppu/background.html
				* According to https://nba-emu.github.io/hw-docs/ppu/background.html#2, every BG waits 31 - 4 * (BG[x]HOFS mod 8) cycles in Mode 0 \n\
				* To simply the implementation, we are just going to assume the minimum amount of time, i.e. 31 - 4 * (max(BG[x]HOFS mod 8))
				* so, 32 - 4 * (7%8) = 32 - 4 * (7) = 35 - 28 = 4
				* Therefore offsetForTextMode = 28
				*/
				PPUTODO("Simplified version of bg wait cycles is implemented for text modes");
				offsetForTextMode = TWENTYEIGHT;
			}

			if (ppuCounter < (BG_WAIT_CYCLES - offsetForTextMode))
			{
				performBgRenderring = NO;
			}
			else
			{
				bgCycles = ppuCounter - (BG_WAIT_CYCLES - offsetForTextMode);
				pGBA_display->bgWaitCyclesDone = YES;
				performBgRenderring = YES;
			}
		}

		if (pGBA_display->mergeWaitCyclesDone == NO && renderMerge == YES)
		{
			if (ppuCounter < MERGE_WAIT_CYCLES)
			{
				performMerging = NO;
			}
			else
			{
				mergeCycles = ppuCounter - MERGE_WAIT_CYCLES;
				pGBA_display->mergeWaitCyclesDone = YES;
				performMerging = YES;
			}
		}

		uint32_t currentCycles = ZERO;
		uint32_t targetCycles = winCycles;

		if (performWinRenderring == YES && renderWindow == YES)
		{
			// Store the current mode and winAccessPatternState in local variables for efficiency
			auto& winState = pGBA_display->winAccessPatternState[currentMode];

			while (currentCycles < targetCycles)
			{
				if (winState == ZERO)
				{
					WIN_CYCLE();
				}

				// Advance the state (modulo 4)
				winState = (winState + ONE) & THREE;

				// next cycle...
				++currentCycles;
			}
		}

		PPUTODO("Optimize below code! We need to use a single while loop at line %d in %s", __LINE__, __FILE__);

		currentCycles = ZERO;
		targetCycles = objCycles;

		if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.SCREEN_DISP_OBJ == SET && performObjectRenderring == YES && renderObj == YES)
		{
			while ((currentCycles < targetCycles) && pGBA_display->allObjectsRenderedForScanline == NO)
			{
				++pGBA_display->cyclesPerSprite;
				++pGBA_display->cyclesPerScanline;

				if (pGBA_display->currentObjectIsAffine == OBJECT_TYPE::OBJECT_IS_AFFINE)
				{
					// FSM runs only on even cycles
					if ((pGBA_display->objAccessPattern & ONE) == RESET)
					{
						switch (pGBA_display->objAccessPatternState)
						{
						case OBJECT_ACCESS_PATTERN::OBJECT_A01:
						{
							pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
							if (pGBA_display->oamFoundValidObject == NO)
							{
								if (INCREMENT_OAM_ID() == INVALID)
								{
									pGBA_display->allObjectsRenderedForScanline = YES;
									pGBA_display->objAccessOAMIDState = RESET;
									pGBA_display->objAccessPattern = RESET;
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
								}
								pGBA_display->cyclesPerSprite = RESET;
							}
							else
							{
								pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
								pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
							}
							BREAK;
						}
						case OBJECT_ACCESS_PATTERN::OBJECT_A2:
						{
							OBJ_A2_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
							pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PA;
							BREAK;
						}
						case OBJECT_ACCESS_PATTERN::OBJECT_PA:
						{
							OBJ_PA_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
							pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PB;
							BREAK;
						}
						case OBJECT_ACCESS_PATTERN::OBJECT_PB:
						{
							OBJ_PB_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
							pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PC;
							BREAK;
						}
						case OBJECT_ACCESS_PATTERN::OBJECT_PC:
						{
							OBJ_PC_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
							pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PD;
							BREAK;
						}
						case OBJECT_ACCESS_PATTERN::OBJECT_PD:
						{
							OBJ_PD_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
							pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_BLANK_A01;
							BREAK;
						}
						case OBJECT_ACCESS_PATTERN::OBJECT_BLANK_A01:
						{
							pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState + ONE);
							pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_V;
							pGBA_display->firstVRAMCycleForObjFSM = YES;
							BREAK;
						}
						case OBJECT_ACCESS_PATTERN::OBJECT_V:
						{
							OBJ_V_OBJ_CYCLE(pGBA_display->objAccessOAMIDState, pGBA_display->currentObjectIsAffine, pGBA_display->vramCyclesStageForObjFSM);
							++pGBA_display->vramCyclesStageForObjFSM;
							if (pGBA_display->lastVRAMCycleForObjFSM == YES)
							{
								pGBA_display->lastVRAMCycleForObjFSM = NO;
								pGBA_display->vramCyclesStageForObjFSM = RESET;
								pGBA_display->cyclesPerSprite = RESET;
								if (INCREMENT_OAM_ID() == INVALID)
								{
									pGBA_display->allObjectsRenderedForScanline = YES;
									pGBA_display->objAccessOAMIDState = RESET;
									pGBA_display->objAccessPattern = RESET;
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
								}
								else
								{
									// valid if object is not disabled or we are in correct y coordinate for the object
									if (pGBA_display->oamFoundValidObject == NO)
									{
										if (INCREMENT_OAM_ID() == INVALID)
										{
											pGBA_display->allObjectsRenderedForScanline = YES;
											pGBA_display->objAccessOAMIDState = RESET;
											pGBA_display->objAccessPattern = RESET;
										}
										else
										{
											pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
											if (pGBA_display->oamFoundValidObject == NO)
											{
												if (INCREMENT_OAM_ID() == INVALID)
												{
													pGBA_display->allObjectsRenderedForScanline = YES;
													pGBA_display->objAccessOAMIDState = RESET;
													pGBA_display->objAccessPattern = RESET;
												}
												pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
											}
											else
											{
												pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
												pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
											}
										}
									}
									else
									{
										pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
										pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
									}
								}
							}
							BREAK;
						}
						default:
						{
							FATAL("Unknown Object Access Pattern State (Affine) in PPU Mode %d", currentMode);
						}
						}
					}

					++pGBA_display->objAccessPattern;
				}
				else if (pGBA_display->currentObjectIsAffine == OBJECT_TYPE::OBJECT_IS_NOT_AFFINE)
				{
					// FSM runs only on even cycles
					if ((pGBA_display->objAccessPattern & ONE) == RESET)
					{
						switch (pGBA_display->objAccessPatternState)
						{
						case OBJECT_ACCESS_PATTERN::OBJECT_A01:
						{
							pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
							if (pGBA_display->oamFoundValidObject == NO)
							{
								if (INCREMENT_OAM_ID() == INVALID)
								{
									pGBA_display->allObjectsRenderedForScanline = YES;
									pGBA_display->objAccessOAMIDState = RESET;
									pGBA_display->objAccessPattern = RESET;
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
								}
								pGBA_display->cyclesPerSprite = RESET;
							}
							else
							{
								pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
								pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
							}
							BREAK;
						}
						case OBJECT_ACCESS_PATTERN::OBJECT_A2:
						{
							OBJ_A2_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
							pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_V;
							pGBA_display->firstVRAMCycleForObjFSM = YES;
							BREAK;
						}
						case OBJECT_ACCESS_PATTERN::OBJECT_V:
						{
							OBJ_V_OBJ_CYCLE(pGBA_display->objAccessOAMIDState, pGBA_display->currentObjectIsAffine, pGBA_display->vramCyclesStageForObjFSM);
							++pGBA_display->vramCyclesStageForObjFSM;
							if (pGBA_display->firstVRAMCycleForObjFSM == YES)
							{
								pGBA_display->firstVRAMCycleForObjFSM = NO;
								pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState + ONE);
							}
							else if (pGBA_display->lastVRAMCycleForObjFSM == YES)
							{
								pGBA_display->lastVRAMCycleForObjFSM = NO;
								pGBA_display->vramCyclesStageForObjFSM = RESET;
								pGBA_display->cyclesPerSprite = RESET;
								if (INCREMENT_OAM_ID() == INVALID)
								{
									pGBA_display->allObjectsRenderedForScanline = YES;
									pGBA_display->objAccessOAMIDState = RESET;
									pGBA_display->objAccessPattern = RESET;
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
								}
								else
								{
									// valid if object is not disabled or we are in correct y coordinate for the object
									if (pGBA_display->oamFoundValidObject == NO)
									{
										if (INCREMENT_OAM_ID() == INVALID)
										{
											pGBA_display->allObjectsRenderedForScanline = YES;
											pGBA_display->objAccessOAMIDState = RESET;
											pGBA_display->objAccessPattern = RESET;
											pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
										}
										else
										{
											pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
											if (pGBA_display->oamFoundValidObject == NO)
											{
												if (INCREMENT_OAM_ID() == INVALID)
												{
													pGBA_display->allObjectsRenderedForScanline = YES;
													pGBA_display->objAccessOAMIDState = RESET;
													pGBA_display->objAccessPattern = RESET;
												}
												pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
											}
											else
											{
												pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
												pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
											}
										}
									}
									else
									{
										pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
										pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
									}
								}
							}
							BREAK;
						}
						default:
						{
							FATAL("Unknown Object Access Pattern State (Not Affine) in PPU Mode %d", currentMode);
						}
						}
					}

					++pGBA_display->objAccessPattern;
				}

				// next cycle...
				++currentCycles;
			}
		}

		currentCycles = ZERO;
		targetCycles = bgCycles;

		if (performBgRenderring == YES && renderBG == YES)
		{
#if (ENABLED)
			// Cache the state once before the loop to avoid repeated array access
			auto& bgState = pGBA_display->bgAccessPatternState[currentMode];

			switch (currentMode)
			{
			case MODE0:
			{
				while (currentCycles < targetCycles)
				{
					MODE0_BG_SEQUENCE(bgState);

					bgState = (bgState + ONE) & THIRTYONE;

					// next cycle...
					++currentCycles;
				}
				BREAK;
			}
			case MODE1:
			{
				while (currentCycles < targetCycles)
				{
					MODE1_BG_SEQUENCE(bgState);

					bgState = (bgState + ONE) & THIRTYONE;

					// next cycle...
					++currentCycles;
				}
				BREAK;
			}
			case MODE2:
			{
				while (currentCycles < targetCycles)
				{
					MODE2_BG_SEQUENCE(bgState);

					bgState = (bgState + ONE) & THIRTYONE;

					// next cycle...
					++currentCycles;
				}
				BREAK;
			}
			case MODE3:
			{
				while (currentCycles < targetCycles)
				{
					MODE3_BG_SEQUENCE(bgState);

					bgState = (bgState + ONE) & THIRTYONE;

					// next cycle...
					++currentCycles;
				}
				BREAK;
			}
			case MODE4:
			{
				while (currentCycles < targetCycles)
				{
					MODE4_BG_SEQUENCE(bgState);

					bgState = (bgState + ONE) & THIRTYONE;

					// next cycle...
					++currentCycles;
				}
				BREAK;
			}
			case MODE5:
			{
				while (currentCycles < targetCycles)
				{
					MODE5_BG_SEQUENCE(bgState);

					bgState = (bgState + ONE) & THIRTYONE;

					// next cycle...
					++currentCycles;
				}
				BREAK;
			}
			default:
			{
				FATAL("Unknown BG mode");
			}
			}
#else
			while (currentCycles < targetCycles)
			{
				switch (pGBA_display->bgAccessPatternState[currentMode])
				{
				case ZERO:
				{
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
					BREAK;
				}
				case ONE:
				{
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
					BREAK;
				}
				case TWO:
				{
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
					CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
					BREAK;
				}
				case THREE:
				{
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
					CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
					++pGBA_display->currentBgPixel;
					BREAK;
				}
				case FOUR:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
					BREAK;
				}
				case FIVE:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
					BREAK;
				}
				case SIX:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
					BREAK;
				}
				case SEVEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
					CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
					++pGBA_display->currentBgPixel;
					BREAK;
				}
				case EIGHT:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
					BREAK;
				}
				case NINE:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
					BREAK;
				}
				case TEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
					CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
					BREAK;
				}
				case ELEVEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
					CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
					++pGBA_display->currentBgPixel;
					BREAK;
				}
				case TWELVE:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
					BREAK;
				}
				case THIRTEEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
					BREAK;
				}
				case FOURTEEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
					BREAK;
				}
				case FIFTEEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
					CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
					++pGBA_display->currentBgPixel;
					BREAK;
				}
				case SIXTEEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
					BREAK;
				}
				case SEVENTEEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
					BREAK;
				}
				case EIGHTEEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
					CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
					BREAK;
				}
				case NINETEEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
					CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
					++pGBA_display->currentBgPixel;
					BREAK;
				}
				case TWENTY:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
					BREAK;
				}
				case TWENTYONE:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
					BREAK;
				}
				case TWENTYTWO:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
					BREAK;
				}
				case TWENTYTHREE:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
					CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
					++pGBA_display->currentBgPixel;
					BREAK;
				}
				case TWENTYFOUR:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
					BREAK;
				}
				case TWENTYFIVE:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
					BREAK;
				}
				case TWENTYSIX:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
					CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
					BREAK;
				}
				case TWENTYSEVEN:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
					CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
					++pGBA_display->currentBgPixel;
					BREAK;
				}
				case TWENTYEIGHT:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
					BREAK;
				}
				case TWENTYNINE:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
					BREAK;
				}
				case THIRTY:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
					BREAK;
				}
				case THIRTYONE:
				{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
					CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
					CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
					CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
					CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
					CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
					CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
					++pGBA_display->currentBgPixel;
					BREAK;
				}
				default:
				{
					FATAL("PPU mode %d (BG) out of sync", currentMode);
				}
				}

				++pGBA_display->bgAccessPatternState[currentMode];
				if (pGBA_display->bgAccessPatternState[currentMode] >= THIRTYTWO)
				{
					pGBA_display->bgAccessPatternState[currentMode] = ZERO;
				}

				// next cycle...
				++currentCycles;
			}
#endif
		}

		currentCycles = ZERO;
		targetCycles = mergeCycles;

		if (performMerging == YES && renderMerge == YES)
		{
			// Cache the state once before the loop to avoid repeated array access
			auto& mergeState = pGBA_display->mergeAccessPatternState[currentMode];

			while (currentCycles < targetCycles)
			{
				if (mergeState == ZERO)
				{
					MERGE_AND_DISPLAY_PHASE1();
				}
				else if (mergeState == TWO)
				{
					MERGE_AND_DISPLAY_PHASE2();
				}

				// Move to the next state and wrap it at 4
				mergeState = (mergeState + ONE) & THREE;

				// next cycle...
				++currentCycles;
			}
		}

	}
}
#endif

void GBA_t::processPPU(INC64 ppuCycles)
{
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

#if (GBA_PPU_ENABLE_LAMBDA_FUNCTIONS == YES)
	auto RESET_PIXEL = [&](uint32_t x, uint32_t y)
		{
			pGBA_display->gfx_obj[x][y] = ZERO;
			pGBA_display->objPriority[x][y] = DEFAULT_OBJ_PRIORITY;
			pGBA_display->gfx_obj_window[x][y] = DISABLED;
			pGBA_display->gfx_obj_mode[x][y] = OBJECT_MODE::NORMAL;
			for (ID bgID = BG0; bgID < BG3; bgID++)
			{
				pGBA_display->gfx_bg[bgID][x][y] = ZERO;
			}
		};

	auto GET_WINDOW_OUTPUT = [&](uint32_t x, uint32_t y, FLAG win0in, FLAG win1in, FLAG winout, FLAG objin)
		{

#if (DEACTIVATED)
			auto isWin = [&](uint32_t x, uint32_t y, uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2)
				{
					if (x2 > getScreenWidth() || x1 > x2)
					{
						x2 = getScreenWidth();
					}
					if (y2 > getScreenHeight() || y1 > y2)
					{
						y2 = getScreenHeight();
					}

					RETURN(x >= x1 && x <= x2) && (y >= y1 && y <= y2);
				};

			auto isWin0 = [&](uint32_t x, uint32_t y)
				{
					RETURN isWin(
						x
						, y
						, pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i1
						, pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i2
						, pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i1
						, pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i2
					);
				};

			auto isWin1 = [&](uint32_t x, uint32_t y)
				{
					RETURN isWin(
						x
						, y
						, pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i1
						, pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i2
						, pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i1
						, pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i2
					);
				};
#endif

			FLAG is_win0in = pGBA_display->gfx_window[WIN0][x][y];
			FLAG is_win1in = pGBA_display->gfx_window[WIN1][x][y];
			FLAG is_winobj = pGBA_display->gfx_obj_window[x][y];

			FLAG win0_display = pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.WIN0_DISP_FLAG == SET;
			FLAG win1_display = pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.WIN1_DISP_FLAG == SET;
			FLAG winobj_display = (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.OBJ_DISP_FLAG == SET && pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.SCREEN_DISP_OBJ == SET);

			// Atleast one window should be enabled for winout to be enabled
			// Now, if any of the win0, win1 or objwin condition fails (provided one of those windows were enabled), then we enter winout condition
			FLAG winout_display = win0_display || win1_display || winobj_display;

			if (win0_display && is_win0in)
			{
				RETURN win0in;
			}
			else if (win1_display && is_win1in)
			{
				RETURN win1in;
			}
			else if (winobj_display && is_winobj)
			{
				RETURN objin;
			}
			else if (winout_display)
			{
				RETURN winout;
			}

			RETURN ENABLED;
		};

	auto HANDLE_WINDOW_FOR_BG = [&](uint32_t x, uint32_t y, ID bgID)
		{
			if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			FLAG isWindowAllowingBG = YES;

			FLAG win0in = CLEAR;
			FLAG win1in = CLEAR;
			FLAG winout = CLEAR;
			FLAG objin = CLEAR;

			switch (bgID)
			{
			case BG0:
			{
				win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_0_EN == SET;
				win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_0_EN == SET;
				winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_0_EN == SET;
				objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_0_EN == SET;
				BREAK;
			}
			case BG1:
			{
				win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_1_EN == SET;
				win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_1_EN == SET;
				winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_1_EN == SET;
				objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_1_EN == SET;
				BREAK;
			}
			case BG2:
			{
				win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_2_EN == SET;
				win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_2_EN == SET;
				winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_2_EN == SET;
				objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_2_EN == SET;
				BREAK;
			}
			case BG3:
			{
				win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_BG_3_EN == SET;
				win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_BG_3_EN == SET;
				winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_BG_3_EN == SET;
				objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_BG_3_EN == SET;
				BREAK;
			}
			default:
			{
				FATAL("Unknown BG Layer");
				RETURN;
			}
			}

			isWindowAllowingBG = GET_WINDOW_OUTPUT(
				x
				, y
				, win0in
				, win1in
				, winout
				, objin
			);

			if (isWindowAllowingBG == NO)
			{
				pGBA_display->gfx_bg[bgID][x][y] = ZERO;
			}
		};

	auto HANDLE_WINDOW_FOR_OBJ = [&](uint32_t x, uint32_t y)
		{
			if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			FLAG isWindowAllowingObj = YES;

			FLAG win0in = CLEAR;
			FLAG win1in = CLEAR;
			FLAG winout = CLEAR;
			FLAG objin = CLEAR;

			win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_OBJ_EN == SET;
			win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_OBJ_EN == SET;
			winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_OBJ_EN == SET;
			objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_OBJ_EN == SET;

			isWindowAllowingObj = GET_WINDOW_OUTPUT(
				x
				, y
				, win0in
				, win1in
				, winout
				, objin
			);

			if (isWindowAllowingObj == NO)
			{
				pGBA_display->gfx_obj[x][y] = ZERO;
			}
		};

	auto DOES_WINDOW_ALLOW_BLENDING = [&](uint32_t x, uint32_t y)
		{
			FLAG isBlendingAllowed = NO;

			if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN isBlendingAllowed;
			}

			FLAG win0in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN0_COLOR_SPL_EFFECT == SET;
			FLAG win1in = pGBA_peripherals->mWININHalfWord.mWININFields.WIN1_COLOR_SPL_EFFECT == SET;
			FLAG winout = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OUTSIDE_COLOR_SPL_EFFECT == SET;
			FLAG objin = pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.OBJ_WIN_COLOR_SPL_EFFECT == SET;

			isBlendingAllowed = GET_WINDOW_OUTPUT(
				x
				, y
				, win0in
				, win1in
				, winout
				, objin
			);

			RETURN isBlendingAllowed;
		};

	auto BLEND = [&](gbaColor_t layer1Pixel, gbaColor_t layer2Pixel, BYTE eva, BYTE evb)
		{
			gbaColor_t finalPixel = layer1Pixel;

			const int r_a = (layer1Pixel.raw >> 0) & 31;
			const int g_a = ((layer1Pixel.raw >> 4) & 62) | (layer1Pixel.raw >> 15);
			const int b_a = (layer1Pixel.raw >> 10) & 31;

			const int r_b = (layer2Pixel.raw >> 0) & 31;
			const int g_b = ((layer2Pixel.raw >> 4) & 62) | (layer2Pixel.raw >> 15);
			const int b_b = (layer2Pixel.raw >> 10) & 31;

			eva = std::min<int>(16, eva);
			evb = std::min<int>(16, evb);

			const int r = std::min<uint8_t>((r_a * eva + r_b * evb + 8) >> 4, 31);
			const int g = std::min<uint8_t>((g_a * eva + g_b * evb + 8) >> 4, 63) >> 1;
			const int b = std::min<uint8_t>((b_a * eva + b_b * evb + 8) >> 4, 31);

			finalPixel.RED = r;
			finalPixel.GREEN = g;
			finalPixel.BLUE = b;

			RETURN finalPixel;
		};

	auto BRIGHTEN = [&](gbaColor_t color, BYTE evy)
		{
			gbaColor_t finalPixel = color;

			evy = std::min<int>(16, evy);

			int r = (color.raw >> 0) & 31;
			int g = ((color.raw >> 4) & 62) | (color.raw >> 15);
			int b = (color.raw >> 10) & 31;

			r += ((31 - r) * evy + 8) >> 4;
			g += ((63 - g) * evy + 8) >> 4;
			b += ((31 - b) * evy + 8) >> 4;

			g >>= 1;

			finalPixel.RED = r;
			finalPixel.GREEN = g;
			finalPixel.BLUE = b;

			RETURN finalPixel;
		};

	auto DARKEN = [&](gbaColor_t color, BYTE evy)
		{
			gbaColor_t finalPixel = color;

			evy = std::min<int>(16, evy);

			int r = (color.raw >> 0) & 31;
			int g = ((color.raw >> 4) & 62) | (color.raw >> 15);
			int b = (color.raw >> 10) & 31;

			r -= (r * evy + 7) >> 4;
			g -= (g * evy + 7) >> 4;
			b -= (b * evy + 7) >> 4;

			g >>= 1;

			finalPixel.RED = r;
			finalPixel.GREEN = g;
			finalPixel.BLUE = b;

			RETURN finalPixel;
		};

	auto MERGE_AND_DISPLAY_PHASE1 = [&]()
		{
			// Refer https://nba-emu.github.io/hw-docs/ppu/composite.html

			uint32_t x = pGBA_display->currentMergePixel;
			uint32_t y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			pGBA_display->mergeCache.xCoordinate = x;
			pGBA_display->mergeCache.yCoordinate = y;

			if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				; // Do nothing...
			}
			else
			{
				pGBA_display->layersForBlending[ZERO] = BD;
				pGBA_display->layersForBlending[ONE] = BD;
				pGBA_display->colorNumberForBlending[ZERO].colorNumber = ZERO;
				pGBA_display->colorNumberForBlending[ONE].colorNumber = ZERO;
				pGBA_display->colorNumberForBlending[ZERO].isObject = NO;
				pGBA_display->colorNumberForBlending[ONE].isObject = NO;
				pGBA_display->priorities[ZERO] = THREE; // Set to lowest priority by default
				pGBA_display->priorities[ONE] = THREE; // Set to lowest priority by default

				if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == SET)
				{
					pGBA_display->colorForBlending[ZERO] = GBA_WHITE;
				}
				else
				{
					auto getBGxCNT = [&](ID bgID)
						{
							if (bgID == BG0)
							{
								RETURN (&pGBA_peripherals->mBG0CNTHalfWord);
							}
							if (bgID == BG1)
							{
								RETURN (&pGBA_peripherals->mBG1CNTHalfWord);
							}
							if (bgID == BG2)
							{
								RETURN (&pGBA_peripherals->mBG2CNTHalfWord);
							}
							if (bgID == BG3)
							{
								RETURN (&pGBA_peripherals->mBG3CNTHalfWord);
							}

							RETURN (mBGnCNTHalfWord_t*)NULL;
						};

					ID minBG = MIN_MAX_BG_LAYERS[pGBA_display->currentPPUMode][ZERO];
					ID maxBG = MIN_MAX_BG_LAYERS[pGBA_display->currentPPUMode][ONE];

					// Check if window layer is allowing the bg, if not, marks the pixel as transparent
					for (ID bgID = minBG; bgID <= maxBG; bgID++)
					{
						HANDLE_WINDOW_FOR_BG(x, y, bgID);
					}
					// Check if window layer is allowing the obj, if not, marks the pixel as transparent
					HANDLE_WINDOW_FOR_OBJ(x, y);

					INC8 numberOfBGs = ZERO;
					for (INC8 priority = ZERO; priority < FOUR; priority++)
					{
						for (ID bgID = minBG; bgID <= maxBG; bgID++)
						{
							if (getBGxCNT(bgID) != NULL)
							{
								if (getBGxCNT(bgID)->mBGnCNTFields.BG_PRIORITY == priority
									&& (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTHalfWord & (0x100 << bgID)))
								{
									pGBA_display->bgListAccToPriority[numberOfBGs++] = bgID;
								}
							}
						}
					}

					INC8 bgListIndexer = ZERO; // This indexer had to be outside so that we get the second target layer
					for (INC8 targetLayer = ZERO; targetLayer < TWO; targetLayer++)
					{
						// Tranversing from highest priority to lowest priority
						while (bgListIndexer < numberOfBGs)
						{
							ID bgID = pGBA_display->bgListAccToPriority[bgListIndexer];

							++bgListIndexer;

							if (pGBA_display->gfx_bg[bgID][x][y] != ZERO)
							{
								pGBA_display->layersForBlending[targetLayer] = bgID;
								pGBA_display->colorNumberForBlending[targetLayer].colorNumber = pGBA_display->gfx_bg[bgID][x][y];
								pGBA_display->colorNumberForBlending[targetLayer].isObject = NO;
								// Priority needs to be maintained so that we can check against OBJs
								if (getBGxCNT(bgID) != NULL)
								{
									pGBA_display->priorities[targetLayer] = getBGxCNT(bgID)->mBGnCNTFields.BG_PRIORITY;
								}
								// Need to break, else we will never get the second target layer
								BREAK;
							}
						}
					}

					if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.SCREEN_DISP_OBJ == SET
						&& pGBA_display->gfx_obj[x][y] != RESET)
					{
						// If object pixel has higher or equal priority to current target layer 1
						if (pGBA_display->objPriority[x][y] <= pGBA_display->priorities[ZERO])
						{
							// Set OBJ as target layer 1
							// But before this, we need to set target layer 2 as the new target layer 1
							pGBA_display->layersForBlending[ONE] = pGBA_display->layersForBlending[ZERO];
							pGBA_display->colorNumberForBlending[ONE].colorNumber = pGBA_display->colorNumberForBlending[ZERO].colorNumber;
							pGBA_display->colorNumberForBlending[ONE].isObject = pGBA_display->colorNumberForBlending[ZERO].isObject;
							pGBA_display->layersForBlending[ZERO] = OBJ;
							pGBA_display->colorNumberForBlending[ZERO].colorNumber = pGBA_display->gfx_obj[x][y];
							pGBA_display->colorNumberForBlending[ZERO].isObject = YES;
						}
						// If object pixel has higher or equal priority to current target layer 2
						else if (pGBA_display->objPriority[x][y] <= pGBA_display->priorities[ONE])
						{
							pGBA_display->layersForBlending[ONE] = OBJ;
							pGBA_display->colorNumberForBlending[ONE].colorNumber = pGBA_display->gfx_obj[x][y];
							pGBA_display->colorNumberForBlending[ONE].isObject = YES;
						}
					}

					if ((pGBA_display->colorNumberForBlending[ZERO].colorNumber & IS_COLOR_NOT_PALETTE) == 0)
					{
						ID paletteIndex = pGBA_display->colorNumberForBlending[ZERO].colorNumber;
						GBA_WORD addressInPaletteRAM = PALETTE_RAM_START_ADDRESS + paletteIndex;
						if (pGBA_display->colorNumberForBlending[ZERO].isObject == YES)
						{
							addressInPaletteRAM += 0x200;
						}
						GBA_HALFWORD paletteData = readRawMemory<GBA_HALFWORD>(addressInPaletteRAM, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
						pGBA_display->colorForBlending[ZERO].raw = paletteData;
					}
					// Mode 3 or Mode 5
					else
					{
						pGBA_display->colorForBlending[ZERO].raw = pGBA_display->colorNumberForBlending[ZERO].colorNumber;
					}
				}
			}
		};

	auto MERGE_AND_DISPLAY_PHASE2 = [&]()
		{
			// Refer https://nba-emu.github.io/hw-docs/ppu/composite.html

			uint32_t x = pGBA_display->mergeCache.xCoordinate;
			uint32_t y = pGBA_display->mergeCache.yCoordinate;

			if ((x >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (y >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				; // Do nothing...
			}
			else
			{
				gbaColor_t finalPixel = { ZERO };

				if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FORCED_BLANK == SET)
				{
					finalPixel = GBA_WHITE;
				}
				else
				{
					if (pGBA_display->gfx_obj_mode[x][y] == OBJECT_MODE::ALPHA_BLENDING
						// No need to check BLDCNT for OBJ if alpha blending is set in OAM (Refer : http://problemkaputt.de/gbatek-lcd-i-o-color-special-effects.htm)
						&& pGBA_display->layersForBlending[ZERO] == OBJ
						// Check BLDCNT for target layer 2
						&& (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0100 << pGBA_display->layersForBlending[ONE])))
					{
						if ((pGBA_display->colorNumberForBlending[ONE].colorNumber & IS_COLOR_NOT_PALETTE) == 0)
						{
							ID paletteIndex = pGBA_display->colorNumberForBlending[ONE].colorNumber;
							GBA_WORD addressInPaletteRAM = PALETTE_RAM_START_ADDRESS + paletteIndex;
							if (pGBA_display->colorNumberForBlending[ONE].isObject == YES)
							{
								addressInPaletteRAM += 0x200;
							}
							GBA_HALFWORD paletteData = readRawMemory<GBA_HALFWORD>(addressInPaletteRAM, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
							pGBA_display->colorForBlending[ONE].raw = paletteData;
						}
						// Mode 3
						else
						{
							pGBA_display->colorForBlending[ONE].raw = pGBA_display->colorNumberForBlending[ONE].colorNumber;
						}

						finalPixel = BLEND(
							pGBA_display->colorForBlending[ZERO]
							, pGBA_display->colorForBlending[ONE]
							, pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVA_COEFF
							, pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVB_COEFF);
					}
					else if (DOES_WINDOW_ALLOW_BLENDING(x, y) == YES && pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTFields.COLOR_SPL_EFFECTS != TO_UINT(COLOR_SPECIAL_EFFECTS::NORMAL))
					{
						switch ((COLOR_SPECIAL_EFFECTS)pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTFields.COLOR_SPL_EFFECTS)
						{
						case COLOR_SPECIAL_EFFECTS::ALPHA_BLENDING:
						{
							if ((pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0001 << pGBA_display->layersForBlending[ZERO]))
								&& (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0100 << pGBA_display->layersForBlending[ONE])))
							{
								if ((pGBA_display->colorNumberForBlending[ONE].colorNumber & IS_COLOR_NOT_PALETTE) == 0)
								{
									ID paletteIndex = pGBA_display->colorNumberForBlending[ONE].colorNumber;
									GBA_WORD addressInPaletteRAM = PALETTE_RAM_START_ADDRESS + paletteIndex;
									if (pGBA_display->colorNumberForBlending[ONE].isObject == YES)
									{
										addressInPaletteRAM += 0x200;
									}
									GBA_HALFWORD paletteData = readRawMemory<GBA_HALFWORD>(addressInPaletteRAM, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
									pGBA_display->colorForBlending[ONE].raw = paletteData;
								}
								// Mode 3 or Mode 5
								else
								{
									pGBA_display->colorForBlending[ONE].raw = pGBA_display->colorNumberForBlending[ONE].colorNumber;
								}

								finalPixel = BLEND(
									pGBA_display->colorForBlending[ZERO]
									, pGBA_display->colorForBlending[ONE]
									, pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVA_COEFF
									, pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.EVB_COEFF);
							}
							else
							{
								finalPixel = pGBA_display->colorForBlending[ZERO];
							}

							BREAK;
						}
						case COLOR_SPECIAL_EFFECTS::INCREASE_BRIGHTNESS:
						{
							if (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0001 << pGBA_display->layersForBlending[ZERO]))
							{
								finalPixel = BRIGHTEN(
									pGBA_display->colorForBlending[ZERO]
									, pGBA_peripherals->mBLDYHalfWord.mBLDYFields.EVY_COEFF);
							}
							else
							{
								finalPixel = pGBA_display->colorForBlending[ZERO];
							}

							BREAK;
						}
						case COLOR_SPECIAL_EFFECTS::DECREASE_BRIGHTNESS:
						{
							if (pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord & (0x0001 << pGBA_display->layersForBlending[ZERO]))
							{
								finalPixel = DARKEN(
									pGBA_display->colorForBlending[ZERO]
									, pGBA_peripherals->mBLDYHalfWord.mBLDYFields.EVY_COEFF);
							}
							else
							{
								finalPixel = pGBA_display->colorForBlending[ZERO];
							}

							BREAK;
						}
						default:
						{
							FATAL("Unknown Color Special Effect");
						}
						}
					}
					else
					{
						finalPixel = pGBA_display->colorForBlending[ZERO];
					}
				}

				RESET_PIXEL(x, y);

				if ((y < getScreenHeight()) && (x < getScreenWidth()))
				{
					pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].a = ALPHA;
					pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(finalPixel.RED);
					pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(finalPixel.GREEN);
					pGBA_display->imGuiBuffer.imGuiBuffer2D[y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(finalPixel.BLUE);

					// increment merge cycles
					++pGBA_display->currentMergePixel;
				}
				else
				{
					FATAL("Display Buffer Out Of Bounds");
				}
			}
		};

	auto SET_INITIAL_OBJ_MODE = [&]()
		{
			if (pGBA_memory->mGBAMemoryMap.mOamAttributes.mOamAttribute[ZERO].mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.ROTATE_SCALE_FLAG == SET)
			{
				pGBA_display->currentObjectIsAffine = OBJECT_TYPE::OBJECT_IS_AFFINE;
			}
			else
			{
				pGBA_display->currentObjectIsAffine = OBJECT_TYPE::OBJECT_IS_NOT_AFFINE;
			}
		};

	auto OBJ_A01_OBJ_CYCLE = [&](ID oamID)
		{
			PPUDEBUG("MODE2_A01_OBJ_CYCLE for OAM%u", oamID);

			FLAG oamIDNeedsToBeRendered = NO;

			// Cache the object fetch stage to avoid repeated memory access
			auto& objCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)];

			objCache.reset();

			if (oamID >= ONETWENTYEIGHT)
			{
				RETURN oamIDNeedsToBeRendered;
			}

			GBA_WORD oamAddress = OAM_START_ADDRESS + (oamID << THREE);
			mOamAttr01Word_t oamAttributes01 = { ZERO };
			oamAttributes01.raw = readRawMemory<GBA_WORD>(oamAddress, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::PPU);

			objCache.ObjAttribute.mOamAttr01Word.raw = oamAttributes01.raw;
			objCache.spriteYScreenCoordinate = oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.Y_COORDINATE;

			if (oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.ROTATE_SCALE_FLAG == SET)
			{
				pGBA_display->nextObjectIsAffine = OBJECT_TYPE::OBJECT_IS_AFFINE;
				objCache.isAffine = YES;
				objCache.spriteHeight = SPRITE_HEIGHTS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_EN.OBJ_SIZE];
				objCache.spriteWidth = SPRITE_WIDTHS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_EN.OBJ_SIZE];

				objCache.isDoubleAffine = (oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_DISABLE_OR_DOUBLE_SIZE_FLAG == SET);
				objCache.isDisabled = NO;
				objCache.spriteXScreenCoordinate = oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_EN.X_COORDINATE;

				// Perform the double affine adjustments if enabled
				// https://gbadev.net/tonc/affobj.html
				if (objCache.isDoubleAffine)
				{
					// We shift the sprite towards the center of the doubled area in the screen
					PPUTODO("Find documentation for shifting sprite to middle when double affine is enabled!");
					objCache.spriteXScreenCoordinate += (objCache.spriteWidth >> ONE);
					objCache.spriteYScreenCoordinate += (objCache.spriteHeight >> ONE);
				}

				// Refer attribute 1 in https://gbadev.net/gbadoc/sprites.html
				// Refer wrapping effect in	https://gbadev.net/tonc/affobj.html
				if (objCache.spriteXScreenCoordinate > static_cast<SDIM32>(getScreenWidth()))
				{
					objCache.spriteXScreenCoordinate -= 512;
				}
			}
			else
			{
				pGBA_display->nextObjectIsAffine = OBJECT_TYPE::OBJECT_IS_NOT_AFFINE;
				objCache.isAffine = NO;
				objCache.spriteHeight = SPRITE_HEIGHTS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_DIS.OBJ_SIZE];
				objCache.spriteWidth = SPRITE_WIDTHS[oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_SHAPE][oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_DIS.OBJ_SIZE];

				objCache.isDoubleAffine = NO;
				objCache.isDisabled = (oamAttributes01.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_DISABLE_OR_DOUBLE_SIZE_FLAG == SET);
				objCache.spriteXScreenCoordinate = oamAttributes01.mOamAttr1HalfWord.ROT_SCALE_DIS.X_COORDINATE;

				// Refer attribute 1 in https://gbadev.net/gbadoc/sprites.html
				// Refer wrapping effect in	https://gbadev.net/tonc/affobj.html
				if (objCache.spriteXScreenCoordinate > static_cast<SDIM32>(getScreenWidth()))
				{
					objCache.spriteXScreenCoordinate -= 512;
				}
			}

			// Refer attribute 0 in https://gbadev.net/gbadoc/sprites.html
			// Refer wrapping effect in	https://gbadev.net/tonc/affobj.html
			if (objCache.spriteYScreenCoordinate > static_cast<SDIM32>(getScreenHeight()))
			{
				objCache.spriteYScreenCoordinate -= 256; // 255 is max value possible
			}

			// Setup the min and max coordinates
			objCache.spriteMaxXScreenCoordinate = objCache.spriteXScreenCoordinate + objCache.spriteWidth;
			objCache.spriteMaxYScreenCoordinate = objCache.spriteYScreenCoordinate + objCache.spriteHeight;
			objCache.spriteMinXScreenCoordinate = objCache.spriteXScreenCoordinate;
			objCache.spriteMinYScreenCoordinate = objCache.spriteYScreenCoordinate;

			if (objCache.isDoubleAffine)
			{
				// We need this separate handling because we went ahead with option 2 for double affine
				objCache.spriteMinXScreenCoordinate -= objCache.spriteWidth / TWO;
				objCache.spriteMaxXScreenCoordinate += objCache.spriteWidth / TWO;
				objCache.spriteMinYScreenCoordinate -= objCache.spriteHeight / TWO;
				objCache.spriteMaxYScreenCoordinate += objCache.spriteHeight / TWO;
			}

			if (
				(objCache.isDisabled == NO)
				&&
				(objCache.vcount >= objCache.spriteMinYScreenCoordinate)
				&&
				(objCache.vcount < objCache.spriteMaxYScreenCoordinate)
				)
			{
				oamIDNeedsToBeRendered = YES;
			}

			RETURN oamIDNeedsToBeRendered;
		};

	auto OBJ_A2_OBJ_CYCLE = [&](ID oamID)
		{
			PPUDEBUG("MODE2_A2_OBJ_CYCLE for OAM%u", oamID);

			// Directly copy the FETCH stage to the RENDER stage (No need for intermediate reset)
			auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
			renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)];

			GBA_WORD oamAddress = OAM_START_ADDRESS + (oamID << THREE) + FOUR;
			mOamAttr23Word_t oamAttributes23 = { ZERO };
			oamAttributes23.mOamAttr2HalfWord.mOamAttr2HalfWord = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

			renderCache.ObjAttribute.mOamAttr23Word.raw = oamAttributes23.raw;

			if (ENABLED)
			{
				// If double affine, then we need to start rendering (sprite width / 2) pixels before the actual sprite pixel start and end (sprite width / 2) after actual sprite pixel end
				if (renderCache.isDoubleAffine == YES)
				{
					auto& fetchCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)];
					renderCache.spriteXStart = -(fetchCache.spriteWidth >> ONE);
					renderCache.spriteXEnd = fetchCache.spriteWidth + (fetchCache.spriteWidth >> ONE);
					renderCache.spriteXPixelCoordinate = -(fetchCache.spriteWidth >> ONE); // currently... we havent traversed the sprite in x coordinates...
				}
				else
				{
					renderCache.spriteXStart = ZERO;
					renderCache.spriteXEnd = renderCache.spriteWidth;
					renderCache.spriteXPixelCoordinate = ZERO; // currently... we havent traversed the sprite in x coordinates...
				}

				renderCache.spriteYPixelCoordinate = renderCache.vcount - renderCache.spriteYScreenCoordinate;

				// Store the base tile ID directly in the render stage
				renderCache.baseTileID = renderCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.CHARACTER_NAME;
			}
		};

	auto OBJ_PA_OBJ_CYCLE = [&](ID oamID)
		{
			PPUDEBUG("MODE2_PA_OBJ_CYCLE for OAM%u", oamID);

			// How the rotation matrix is stored in memory is as follows:
			// 0x07000000	->	Attr0
			// 0x07000002	->	Attr1
			// 0x07000004	->	Attr2
			// 0x07000006	->	PA
			// 0x07000008	->	Attr0
			// 0x0700000A	->	Attr1
			// 0x0700000C	->	Attr2
			// 0x0700000E	->	PB
			// 0x07000010	->	Attr0
			// 0x07000012	->	Attr1
			// 0x07000014	->	Attr2
			// 0x07000016	->	PC
			// 0x07000018	->	Attr0
			// 0x0700001A	->	Attr1
			// 0x0700001C	->	Attr2
			// 0x0700001E	->	PD
			// 0x07000020	->	Attr0
			// 0x07000022	->	Attr1
			// 0x07000024	->	Attr2
			// 0x07000026	->	PA

			// Gap b/w 1 PA to another is (for eg 0x07000026 - 0x07000006) = 0x20 (32)

			auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
			GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + SIX;
			renderCache.affine.pa = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		};

	auto OBJ_PB_OBJ_CYCLE = [&](ID oamID)
		{
			PPUDEBUG("MODE2_PB_OBJ_CYCLE for OAM%u", oamID);

			// Refer to MODE2_PA_OBJ_CYCLE for info on the below equation
			auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
			GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + FOURTEEN;
			renderCache.affine.pb = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		};

	auto OBJ_PC_OBJ_CYCLE = [&](ID oamID)
		{
			PPUDEBUG("MODE2_PC_OBJ_CYCLE for OAM%u", oamID);

			// Refer to MODE2_PA_OBJ_CYCLE for info on the below equation
			auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
			GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + TWENTYTWO;
			renderCache.affine.pc = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		};

	auto OBJ_PD_OBJ_CYCLE = [&](ID oamID)
		{
			PPUDEBUG("MODE2_PD_OBJ_CYCLE for OAM%u", oamID);

			// Refer to MODE2_PA_OBJ_CYCLE for info on the below equation
			auto& renderCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
			GBA_WORD oamAddress = OAM_START_ADDRESS + (renderCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_EN.ROT_SCALE_PARAM_SEL << FIVE) + THIRTY;
			renderCache.affine.pd = readRawMemory<GBA_HALFWORD>(oamAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
		};

	auto OBJ_V_OBJ_CYCLE = [&](ID oamID, OBJECT_TYPE isAffine, STATE8 state)
		{
			PPUDEBUG("MODE2_V_OBJ_CYCLE[%u] for OAM%u", state, oamID);

			auto& objCache = pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)];
			INC16 spriteXStart = objCache.spriteXPixelCoordinate;
			INC16 spriteXEnd = RESET;

			if (isAffine == OBJECT_TYPE::OBJECT_IS_NOT_AFFINE)
			{
				// Identity matrix (terms which get multiplied will be set to 1 and terms which gets added will be set to 0)
				objCache.affine.pa = 0xFF;
				objCache.affine.pb = 0x00;
				objCache.affine.pc = 0x00;
				objCache.affine.pd = 0xFF;

				spriteXEnd = spriteXStart + TWO;	// only 2 pixel per VRAM stage
			}
			else
			{
				spriteXEnd = spriteXStart + ONE;	// only 1 pixel per VRAM stage
			}

			INC32 transformedSpriteX = ZERO, transformedSpriteY = ZERO;
			INC16 spriteY = objCache.spriteYPixelCoordinate;
			SDIM32 Y = objCache.vcount;
			// update the spriteX coordinates (needed for next VRAM stages)
			objCache.spriteXPixelCoordinate = spriteXEnd;
			// check if this needs to be the last vram cycle for this object
			pGBA_display->lastVRAMCycleForObjFSM = (spriteXEnd == objCache.spriteXEnd) ? YES : NO;

			auto pa = objCache.affine.pa;
			auto pb = objCache.affine.pb;
			auto pc = objCache.affine.pc;
			auto pd = objCache.affine.pd;
			auto x0 = objCache.spriteWidth >> ONE;
			auto y0 = objCache.spriteHeight >> ONE;

			for (INC16 spriteX = spriteXStart; spriteX < spriteXEnd; spriteX++)
			{
				// NOTE: We need to get the screen coordinates before it got modified because of flip or affine
				// other wise tranformation from screen space to affine space will get cancelled
				// even for rotation, this is basically like flipping the sprite coordinated and then again flipping the screen coordinates, flip gets cancelled!
				int32_t screenX = spriteX + objCache.spriteXScreenCoordinate;

				if (isAffine == OBJECT_TYPE::OBJECT_IS_AFFINE)
				{
					/*
					*	according to formula mentioned in http://problemkaputt.de/gbatek-lcd-i-o-bg-rotation-scaling.htm
					*	affine can be calculated as follows
					*
					*	Using the following expressions,
					*	  x0,y0    Rotation Center
					*	  x1,y1    Old Position of a pixel (before rotation/scaling)
					*	  x2,y2    New position of above pixel (after rotation scaling)
					*	  A,B,C,D  BG2PA-BG2PD Parameters (as calculated above)
					*
					*	the following formula can be used to calculate x2,y2:
					*
					*	  x2 = A(x1-x0) + B(y1-y0) + x0
					*	  y2 = C(x1-x0) + D(y1-y0) + y0
					*
					*/

					// for X
					transformedSpriteX = (pa * (spriteX - x0)) + (pb * (spriteY - y0));
					// get the integer part of transformedSpriteX
					transformedSpriteX >>= EIGHT;
					transformedSpriteX += x0;

					// boundary check post transformation
					if (transformedSpriteX < 0 || transformedSpriteX >= objCache.spriteWidth)
						continue;

					// for Y
					transformedSpriteY = (pc * (spriteX - x0)) + (pd * (spriteY - y0));
					// get the integer part of transformedSpriteX
					transformedSpriteY >>= EIGHT;
					transformedSpriteY += y0;

					// boundary check post transformation
					if (transformedSpriteY < ZERO || transformedSpriteY >= objCache.spriteHeight)
						continue;
				}
				else
				{
					if (objCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_DIS.HOR_FLIP == SET)
						transformedSpriteX = objCache.spriteWidth - spriteX - ONE;
					else
						transformedSpriteX = spriteX;

					if (objCache.ObjAttribute.mOamAttr01Word.mOamAttr1HalfWord.ROT_SCALE_DIS.VER_FLIP == SET)
						transformedSpriteY = objCache.spriteHeight - spriteY - ONE;
					else
						transformedSpriteY = spriteY;
				}

				SDIM32 X = screenX;

				if (X >= ZERO && X < static_cast<SDIM32>(getScreenWidth()))
				{
					// Y Transformation:
					if (ENABLED)
					{
						/*
						*	Let's assume the object on the screen is as follows:
						*
						*	 ________________________________________________________
						*	|														|
						*	|														|
						*	|														|
						* 	|														|
						* 	|				_________________						|
						* 	|				|		|		|						|
						*	|				|TY00	|TY01	|						|
						* 	|				|_______| ______|						|
						* 	|				|		|		|						|
						* 	|				|TY10	|TY11	|						|
						* 	|				|		|		|						|
						* 	|				__________________						|
						* 	|														|
						* 	|														|
						* 	|														|
						*	_________________________________________________________
						*
						*/

						// To get the row of the tile of interest
						// we just need to get transformedSpriteY / 8
						int32_t spriteYTile = transformedSpriteY >> THREE;
						// Now, to get the column of the tile of interest
						// we first get the witdth of the tile in terms of number of tiles
						// i.e. totalwidth of object / 8 gives you how many tiles are there in a row for a given object
						int32_t widthOfSpriteInTiles = objCache.spriteWidth >> THREE;

						ID tileIDOffsetBecauseOfY = ZERO;

						// Refer https://gbadev.net/gbadoc/sprites.html
						// Refer http://problemkaputt.de/gbatek-lcd-obj-vram-character-tile-mapping.htm
						// 2D mapping
						if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.OBJ_CHAR_VRAM_MAP == RESET)
						{
							// Assume tiles are arranged as follows for 2D mapping in memory

							/*
							*	TILE0000	| TILE0001	| TILE0002	| TILE0003	| TILE0004  | TILE0005  | TILE0006  .... | TILE0031
							*   TILE0032	| TILE0033	| TILE0034	| TILE0035	| TILE0036	| TILE0037	| TILE0038	.... | TILE0063
							*	:
							*   TILE0992	| TILE0993	| TILE0994	| TILE0995	| TILE0996	| TILE0997	| TILE0998	.... | TILE1023
							*/

							// pixels can be be arranged as a matrix of 32x32 or 16x32 tiles based on 16 or 256 color mode

							// To handle 256 color mode, we need to consider twice as much tiles, we handle this as (<< attribute0.color/palettes)

							// To get the tile offset, lets first process the y tile offset
							// if 8x8, tileIDOffsetBecauseOfY = 0
							// if 16x16, tileIDOffsetBecauseOfY = 32
							// if 32x32, tileIDOffsetBecauseOfY = 64

							// we can get above values of tileIDOffsetBecauseOfY using spriteYTile

							tileIDOffsetBecauseOfY = spriteYTile << FIVE;

							// Note: The reason we don't do anything special for 256 color mode is because "widthOfSpriteInTiles" will be confined to current row
							// So, next row will never get affected, so tile immediatly below (or few more rows below) the 1st row tile needs to be considered...so we just add multiple of 32 
						}
						// 1D mapping
						else
						{
							// Assume tiles are arranged as follows for 2D mapping in memory

							// Refer http://problemkaputt.de/gbatek-lcd-obj-oam-attributes.htm
							// In 256 color mode
							/*
							*	TILE0000	| TILE0002	| TILE0004	| TILE0006	| TILE0008  | TILE0010  | TILE0012  .... | TILE1022
							*/

							// Otherwise

							/*
							*	TILE0000	| TILE0001	| TILE0002	| TILE0003	| TILE0004  | TILE0005  | TILE0006  .... | TILE1023
							*/

							// To handle 256 color mode, we need to consider twice as much tiles, we handle this as (<< attribute0.color/palettes)

							// To get the tile offset, lets first process the y tile offset
							// if 8x8, tileIDOffsetBecauseOfY = 0
							// if 16x16, tileIDOffsetBecauseOfY = 2
							// if 32x32, tileIDOffsetBecauseOfY = 4

							// we can get above values of tileIDOffsetBecauseOfY using spriteYTile and widthOfSpriteInTiles

							tileIDOffsetBecauseOfY = widthOfSpriteInTiles * spriteYTile;
							// handle 256 color mode if needed (Xly by 2 as widthOfSpriteInTiles becomes double)
							tileIDOffsetBecauseOfY <<= objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.COLOR_PALETTES;
						}

						// after shifting the base tile id in y direction, this is the new base for shifing in x direction
						objCache.tileIDAfterAccountingForY = tileIDOffsetBecauseOfY;
					}

					// X Transformation:
					if (ENABLED)
					{
						// now need to get the tile id offset because of X
						// x offset would be the current sprite x coordinate / 8
						// refer to diagrams in MODE2_A2_OBJ_CYCLE for more info

						ID tileIDOffsetBecauseOfX = transformedSpriteX >> THREE;
						// Xly by 2 if 256 color mode because we need to skip every alternative (odd) tiles
						// refer OBJ Tile Number in http://problemkaputt.de/gbatek-lcd-obj-oam-attributes.htm
						// so, if tile number is 0, then we remain at 0
						// if tile number is 1, then we jump to 2
						// if tile number is 3, then we jump to 6
						// if tile number is 4, then we jump to 8
						// and so on...
						tileIDOffsetBecauseOfX <<= objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.COLOR_PALETTES;

						objCache.tileIDAfterAccountingForX = tileIDOffsetBecauseOfX;
					}

					int32_t actualTileID = objCache.baseTileID + objCache.tileIDAfterAccountingForY + objCache.tileIDAfterAccountingForX;

					// using the tileID, we need to fetch the corresponding tile data
					// for this, we need to first figure out addressInTileDataArea 

					// also, even with addressInTileDataArea, we need to figure out the data pertaining to the pixel within the tile of interest
					// NOTE: we still do %8 and not %width even for sprites bigger than 8x8 because
					// 1) for bigger tiles, the tileID would change for every 8x8 which takes care of getting appropriate address... no need to account for it in modulo again
					// 2) tile data is stored in terms of 8x8 

					SDIM32 xTileCoordinate = transformedSpriteX & SEVEN;
					SDIM32 yTileCoordinate = transformedSpriteY & SEVEN;
					GBA_WORD withinTileOffset = xTileCoordinate + (yTileCoordinate << THREE);

					// Refer : http://problemkaputt.de/gbatek-lcd-obj-overview.htm
					// Refer : http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
					// NOTE: in tile data area, granularity within the tile data is decided based on 4bpp mode or 8bpp mode
					// in 4bpp mode, each tile is 32 bytes, first 4 bytes for the topmost row of the tile
					// each byte representing two dots, the lower 4 bits define the color for the left and upper 4 bits the color for the right dot
					// in 8bpp mode, each tile is 64 bytes, first 8 bytes for the topmost row of the tile
					// each byte selects palette entry for each dot
					// 8bpp mode is the 256 color mode that is represented in attribute 0
					// Refer : https://gbadev.net/gbadoc/sprites.html
					// irrespective mode, each tile ID represents 32 bytes of data in sprite tile data area
					// so the memory address of a tile is roughly 0x06010000 + T*32
					const INC16 eachIndexReferences32BytesInShifts = FIVE; // Indicates 32 in power of 2 (to use it for optimized multiplication)
					PPUTODO("Each byte is 64 byte in 8bpp mode, so each index should reference 64 bytes in 8bpp? (Source : NBA)");

					INC32 paletteIndex = RESET;

					// 16 color mode (4bpp mode)
					if (objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.COLOR_PALETTES == RESET)
					{
						GBA_WORD addressInTileDataArea =
							VIDEO_RAM_START_ADDRESS
							+ 0x10000
							+ (actualTileID << eachIndexReferences32BytesInShifts)
							// each byte represents 2 dots
							// divide the withinTileOffset by 2 as single byte can represent 2 pixels
							// so "withinTileOffset" is halved
							+(withinTileOffset >> ONE);

						BYTE pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);

						// In 4bpp mode, only 4 bits out of pixelColorData represents actual color for pixel, other 4 bits are for the adjascent pixels
						// As mentioned in http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
						// lower 4 bits for left dot (lower X) and upper 4 bits for right dot (higher X)
						// take an example of dot 2 and dot 3
						// dot 2 needs to use bit0-3 and dot 3 needs to use bit4-7
						// withinTileOffset gives use dot number within tile
						// basically, for even dot, we simply extract first 4 bits
						// for odd dot, we want to shift bit4-7 right by 4 and then extract only first 4 bits
						// also, using the naming convention of left and right dot given in http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
						// tile data will be as follows:
						// row0 : LEFT(000), RIGHT(001), LEFT(002), RIGHT(003), LEFT(004), RIGHT(005), LEFT(006), RIGHT(007)
						// row1 : LEFT(008), RIGHT(009), LEFT(010), RIGHT(011), LEFT(012), RIGHT(013), LEFT(014), RIGHT(015)
						// so for odd "withinTileOffset" perform the shift and extract, else directly extract						

						// Odd means we need to shift right by 4
						if (withinTileOffset & ONE)
						{
							pixelColorNumberFromTileData >>= FOUR;
						}
						// Extract only the required 4 bits
						pixelColorNumberFromTileData &= 0x0F;

						// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
						paletteIndex = pixelColorNumberFromTileData << ONE;

						PPUTODO("Why paletteIndex != ZERO check is needed before accounting for palette bank number in 4bpp mode? \n\
							Will not having this check cause non-transparent pixel even when transparent is desired? (Source : NBA)");
						if (paletteIndex != ZERO)
						{
							// palette bank number is provided as part of attribute 2 in 4bpp mode
							// this palette number is the index into one of the 16 16-colored palettes (Refer : https://gbadev.net/gbadoc/sprites.html)
							// palette ram for sprites goes from 0x05000200 to 0x050003FF which is 512 bytes
							// Hence, each 16 16-colored palettes should take 32 bytes
							// OR
							// we can think that whole sprite palette area is divided into 16 palettes with each palette having 16 colors (Refer : http://problemkaputt.de/gbatek-lcd-color-palettes.htm)
							// each color is 2 bytes, so each palette takes 16 * 2 bytes = 32 bytes (Refer : http://problemkaputt.de/gbatek-lcd-color-palettes.htm)
							// Hence, each palette is 32 bytes
							// palette number in attribute 2 * THIRTYTWO will give the address of desired palette number in sprite palette area

							const DIM16 sizeOfEachPaletteInBytesInShifts = FIVE; // Indicates 32 in power of 2 (to use it for optimized multiplication) 
							paletteIndex += (objCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.PALETTE_NUMBER << sizeOfEachPaletteInBytesInShifts);
						}
					}
					else
					{
						GBA_WORD addressInTileDataArea =
							VIDEO_RAM_START_ADDRESS
							+ 0x10000
							+ (actualTileID << eachIndexReferences32BytesInShifts)
							+ withinTileOffset;

						BYTE pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);

						// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
						paletteIndex = pixelColorNumberFromTileData << ONE;
					}

					if ((OBJECT_MODE)objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_MODE == OBJECT_MODE::OBJ_WINDOW
						&&
						paletteIndex != ZERO)
					{
						pGBA_display->gfx_obj_window[X][Y] = ENABLED;
					}
					else if (objCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.PRIORITY < pGBA_display->objPriority[X][Y]
						||
						pGBA_display->gfx_obj[X][Y] == ZERO)
					{
						PPUTODO("Do we even need this paletteIndex != ZERO check, even if we populate paletteIndex of zero, wont merge take care of transparent sprites? (Source : NBA)");
						if (paletteIndex != ZERO)
						{
							pGBA_display->gfx_obj[X][Y] = paletteIndex;
							pGBA_display->gfx_obj_mode[X][Y] = (OBJECT_MODE)objCache.ObjAttribute.mOamAttr01Word.mOamAttr0HalfWord.mOamAttr0Fields.OBJ_MODE;
						}
						pGBA_display->objPriority[X][Y] = objCache.ObjAttribute.mOamAttr23Word.mOamAttr2HalfWord.mOamAttr2Fields.PRIORITY;
					}
				}
			}
		};

	auto INCREMENT_OAM_ID = [&]()
		{
			// Check and increment objAccessOAMIDState only if it's within bounds
			if (pGBA_display->objAccessOAMIDState < ONETWENTYEIGHT - ONE)
			{
				++pGBA_display->objAccessOAMIDState;
				RETURN VALID;
			}
			RETURN INVALID;
		};

	auto WIN_CYCLE = [&]()
		{
			uint32_t xPixelCoordinate = pGBA_display->currentWinPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			if ((xPixelCoordinate >= ((uint16_t)LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= ((uint16_t)LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Helper lambda to handle the window logic for both Win0 and Win1
			auto handle_window = [&](uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2, FLAG& winH, FLAG& winV, int winIndex)
				{
					// Check and adjust the boundaries
					if (x2 > getScreenWidth() || x1 > x2) x2 = getScreenWidth();
					if (y2 > getScreenHeight() || y1 > y2) y2 = getScreenHeight();

					// Horizontal and vertical range checks
					winH = (xPixelCoordinate >= x1 && xPixelCoordinate <= x2) ? YES : NO;
					winV = (yPixelCoordinate >= y1 && yPixelCoordinate <= y2) ? YES : NO;

					// Update the window pixel array
					pGBA_display->gfx_window[winIndex][xPixelCoordinate][yPixelCoordinate] = winH && winV;
				};

			// Handle Win0
			FLAG win01H = NO, win01V = NO;
			handle_window(pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i1, pGBA_peripherals->mWIN0HHalfWord.mWINniFields.i2,
				pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i1, pGBA_peripherals->mWIN0VHalfWord.mWINniFields.i2,
				win01H, win01V, WIN0);

			// Handle Win1
			FLAG win11H = NO, win11V = NO;
			handle_window(pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i1, pGBA_peripherals->mWIN1HHalfWord.mWINniFields.i2,
				pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i1, pGBA_peripherals->mWIN1VHalfWord.mWINniFields.i2,
				win11H, win11V, WIN1);

			// increment windows pixel counter
			++pGBA_display->currentWinPixel;
		};

#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
	auto MODE0_M_BG_CYCLE = [&](ID bgID)
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Bounds check for bgID
			if (bgID < BG0 || bgID > BG3)
			{
				FATAL("Unknown BG in mode 0");
				RETURN;
			}

			pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
			pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

			GBA_WORD bgTileMapBaseAddr = ZERO;
			mBGnCNTHalfWord_t BGxCNT = { ZERO };
			DIM16 hofs = ZERO;
			DIM16 vofs = ZERO;

			// Cache BG configuration values for fast access
			auto getBGConfig = [&](ID id) -> std::tuple<int, FLAG, uint16_t, uint16_t, uint16_t>
				{
					switch (id)
					{
					case BG0:
						RETURN {
							static_cast<int>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
							pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord),
							static_cast<uint16_t>(pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					case BG1:
						RETURN {
							static_cast<int>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
							pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord),
							static_cast<uint16_t>(pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					case BG2:
						RETURN {
							static_cast<int>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
							pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord),
							static_cast<uint16_t>(pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					case BG3:
						RETURN {
							static_cast<int>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
							pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord),
							static_cast<uint16_t>(pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					default:
						// Safe fallback to avoid undefined behavior
						RETURN { 0, false, 0, 0, 0 };
					}
				};

			auto [bgTileMapBaseAddr_, is8bppMode, BGxCNT_, hofs_, vofs_] = getBGConfig(bgID);
			bgTileMapBaseAddr = bgTileMapBaseAddr_;
			pGBA_display->bgCache[bgID].is8bppMode = is8bppMode;
			BGxCNT.mBGnCNTHalfWord = BGxCNT_;
			hofs = hofs_;
			vofs = vofs_;

			/*
			* Refer http://problemkaputt.de/gbatek-lcd-i-o-bg-control.htm
			* BGs always wraparound, so
			* offsets = BGxHOFS and BGxHVOFS (BG offsets within static screen)
			* Final offsets from screen (tileMap_x and tileMap_y) = (x + BGxHOFS) % 512 and (y + BGxHVOFS) % 512
			* Offsets in tile coordinates (tile_x and tile_y) = Final offsets from screen / 8
			* Offsets within tile coordinates (WithinTile_x and WithinTile_y) = Final offsets from screen % 8
			*
			* When BGxCNT->Size = 0
			* SC0 -> 256x256
			* When BGxCNT->Size = 1
			* SC0 SC1 -> 512x256
			* When BGxCNT->Size = 2
			* SC0 -> 256x512
			* SC1
			* When BGxCNT->Size = 3
			* SC0 SC1 -> 512x512
			* SC2 SC3
			* SC0 is defined by the normal BG Map base address (Bit 8-12 of BGxCNT),
			* SC1 uses same address +2K, SC2 address +4K, SC3 address +6K
			* So, for given x, y we need to figure out in which SCn it falls...
			* Then each SCn is 256x256, so , number of tiles i.e. tilesPerRow = bitsPerRow / 8 = 32
			* So, withinTileOffset = (tile_x + (tile_y * tilesPerRow))
			* But, we do a 16bit read for tile map as mentioned under Mode 0 in https://nba-emu.github.io/hw-docs/ppu/background.html and http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm unlike non text modes
			* So, 16withinTileOffset = 2 * withinTileOffset
			*
			* Final Address for tile map = [VRAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 2K) + (16withinTileOffset)]
			*/

			// Cached sum of coordinates and offsets
			DIM16 offsetSumX = xPixelCoordinate + hofs;
			DIM16 offsetSumY = yPixelCoordinate + vofs;

			ID SCn = ZERO;
			switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
			{
				// SC0
			case ZERO: SCn = ZERO; BREAK;
				// SC0 SC1
			case ONE: SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO; BREAK;
				// SC0
				// SC1
			case TWO: SCn = ((offsetSumY & 511) > 255) ? ONE : ZERO; BREAK;
				// SC0 SC1
				// SC2 SC3
			case THREE:
				SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO;
				SCn += ((offsetSumY & 511) > 255) ? TWO : ZERO;
				BREAK;
			default:
				FATAL("Unknown screen size in mode 0");
				RETURN;
			}

			pGBA_display->bgCache[bgID].hofs = hofs;
			pGBA_display->bgCache[bgID].vofs = vofs;

			// Within SCn
			DIM16 tileMapX = offsetSumX & 255; // % 256 because because if more than 255, it is SC1 or SC3, but we are already accounting for SCn, so we just get tileX within SCn
			DIM16 tileMapY = offsetSumY & 255; // % 256 because because if more than 255, it is SC2 or SC3, but we are already accounting for SCn, so we just get tileX within SCn

			pGBA_display->bgCache[bgID].tileMapX = tileMapX;
			pGBA_display->bgCache[bgID].tileMapY = tileMapY;

			const DIM16 tilesPerRowInShifts = FIVE; // 1 << 5 is same as * 32; Each SCn is of 256*256, so 256 / 8 = 32
			DIM16 tileMapOffset = (tileMapX >> THREE) + ((tileMapY >> THREE) << tilesPerRowInShifts);

			// Since we have 16 bits per tile instead of 8
			// Refer http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
			tileMapOffset <<= ONE;

			GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileMapArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
				PPUTODO("As per NBA, this is set to some latched value");
			}
		};

	auto MODE0_T_BG_CYCLE = [&](ID bgID)
		{
			auto X = pGBA_display->currentBgPixel;
			auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Bounds check for bgID
			if (bgID < BG0 || bgID > BG3)
			{
				FATAL("Unknown BG in mode 0");
				RETURN;
			}

			GBA_WORD bgTileDataBaseAddr =
				(bgID == BG0)
				? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
				: (bgID == BG1)
				? (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
				: (bgID == BG2)
				? (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
				: (pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

			auto xTileCoordinate = pGBA_display->bgCache[bgID].tileMapX & SEVEN; // % 8
			auto yTileCoordinate = pGBA_display->bgCache[bgID].tileMapY & SEVEN; // % 8

			xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
				? flipLUT[xTileCoordinate]
				: xTileCoordinate;

			yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
				? flipLUT[yTileCoordinate]
				: yTileCoordinate;

			GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
			// Refer http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
			DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

			// 4bpp mode
			if (pGBA_display->bgCache[bgID].is8bppMode == NO)
			{
				// 4bpp mode
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ bgTileDataBaseAddr
					+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
					+ (withinTileDataOffset >> ONE); // Divide by 2 because in 4bpp mode, A single halfword gives data for 4 pixels

				BYTE pixelColorNumberFor2PixelsFromTileData = RESET;

				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					pixelColorNumberFor2PixelsFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
				}
				else
				{
					PPUTODO("As per NBA, pixelColorNumberFor2PixelsFromTileData is set to some latched value");
				}

				// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
				// Odd means we need to shift right by 4
				pixelColorNumberFor2PixelsFromTileData >>= ((withinTileDataOffset & ONE) ? FOUR : ZERO);
				// extract only the required 4 bits
				BYTE pixelColorNumberPerPixel = pixelColorNumberFor2PixelsFromTileData & 0x0F;
				INC32 paletteIndex = pixelColorNumberPerPixel << ONE;

				if (paletteIndex != ZERO)
				{
					// Refer mode 2 bg processing to see how we determine sizeOfEachPaletteInBytes
					const DIM16 sizeOfEachPaletteInBytesInShifts = FIVE; // 1 << 5 is same as *32
					paletteIndex += (pGBA_display->bgCache[bgID].tileDescriptor.pb << sizeOfEachPaletteInBytesInShifts);
				}
				pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
			}
			else
			{
				// 8bpp mode
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ bgTileDataBaseAddr
					+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
					+ withinTileDataOffset;

				BYTE pixelColorNumberFor1PixelFromTileData = RESET;

				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					pixelColorNumberFor1PixelFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
				}
				else
				{
					PPUTODO("As per NBA, pixelColorNumberFor1PixelFromTileData is set to some latched value");
				}
				
				// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
				INC32 paletteIndex = pixelColorNumberFor1PixelFromTileData << ONE;
				pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
			}
		};

	auto MODE1_M_TEXT_BG_CYCLE = [&](ID bgID)
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Bounds check for bgID
			if (bgID < BG0 || bgID > BG1)
			{
				FATAL("Unknown BG in mode 1");
				RETURN;
			}

			pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
			pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

			GBA_WORD bgTileMapBaseAddr = ZERO;
			mBGnCNTHalfWord_t BGxCNT = { ZERO };
			DIM16 hofs = ZERO;
			DIM16 vofs = ZERO;

			if (bgID == BG0)
			{
				bgTileMapBaseAddr = (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
				pGBA_display->bgCache[BG0].is8bppMode = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
				BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord;
				hofs = pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET;
				vofs = pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET;
			}
			else if (bgID == BG1)
			{
				bgTileMapBaseAddr = (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
				pGBA_display->bgCache[BG1].is8bppMode = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
				BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord;
				hofs = pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET;
				vofs = pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET;
			}
			else
			{
				FATAL("Unknown BG in mode 1 text mode");
				RETURN;
			}

			/*
			* Refer http://problemkaputt.de/gbatek-lcd-i-o-bg-control.htm
			* BGs always wraparound, so
			* offsets = BGxHOFS and BGxHVOFS (BG offsets within static screen)
			* Final offsets from screen (tileMap_x and tileMap_y) = (x + BGxHOFS) % 512 and (y + BGxHVOFS) % 512
			* Offsets in tile coordinates (tile_x and tile_y) = Final offsets from screen / 8
			* Offsets within tile coordinates (WithinTile_x and WithinTile_y) = Final offsets from screen % 8
			*
			* When BGxCNT->Size = 0
			* SC0 -> 256x256
			* When BGxCNT->Size = 1
			* SC0 SC1 -> 512x256
			* When BGxCNT->Size = 2
			* SC0 -> 256x512
			* SC1
			* When BGxCNT->Size = 3
			* SC0 SC1 -> 512x512
			* SC2 SC3
			* SC0 is defined by the normal BG Map base address (Bit 8-12 of BGxCNT),
			* SC1 uses same address +2K, SC2 address +4K, SC3 address +6K
			* So, for given x, y we need to figure out in which SCn it falls...
			* Then each SCn is 256x256, so , number of tiles i.e. tilesPerRow = bitsPerRow / 8 = 32
			* So, withinTileOffset = (tile_x + (tile_y * tilesPerRow))
			* But, we do a 16bit read for tile map as mentioned under Mode 0 in https://nba-emu.github.io/hw-docs/ppu/background.html and http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm unlike non text modes
			* So, 16withinTileOffset = 2 * withinTileOffset
			*
			* Final Address for tile map = [VRAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 2K) + (16withinTileOffset)]
			*
			*/

			// Cached sum of coordinates and offsets
			DIM16 offsetSumX = xPixelCoordinate + hofs;
			DIM16 offsetSumY = yPixelCoordinate + vofs;

			ID SCn = ZERO;
			switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
			{
				// SC0
			case ZERO: SCn = ZERO; BREAK;
				// SC0 SC1
			case ONE: SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO; BREAK;
				// SC0
				// SC1
			case TWO: SCn = ((offsetSumY & 511) > 255) ? ONE : ZERO; BREAK;
				// SC0 SC1
				// SC2 SC3
			case THREE:
				SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO;
				SCn += ((offsetSumY & 511) > 255) ? TWO : ZERO;
				BREAK;
			default:
				FATAL("Unknown screen size in mode 0");
				RETURN;
			}

			pGBA_display->bgCache[bgID].hofs = hofs;
			pGBA_display->bgCache[bgID].vofs = vofs;

			// Within SCn
			DIM16 tileMapX = offsetSumX & 255; // % 256 because because if more than 255, it is SC1 or SC3, but we are already accounting for SCn, so we just get tileX within SCn
			DIM16 tileMapY = offsetSumY & 255; // % 256 because because if more than 255, it is SC2 or SC3, but we are already accounting for SCn, so we just get tileX within SCn

			pGBA_display->bgCache[bgID].tileMapX = tileMapX;
			pGBA_display->bgCache[bgID].tileMapY = tileMapY;

			const DIM16 tilesPerRowInShifts = FIVE; // 1 << 5 is same as * 32; Each SCn is of 256*256, so 256 / 8 = 32
			DIM16 tileMapOffset = (tileMapX >> THREE) + ((tileMapY >> THREE) << tilesPerRowInShifts);

			// Since we have 16 bits per tile instead of 8
			// Refer http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
			tileMapOffset <<= ONE;

			GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileMapArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET
				PPUTODO("As per NBA, this is set to some latched value");
			}		
		};

	auto MODE1_T_TEXT_BG_CYCLE = [&](ID bgID)
		{
			auto X = pGBA_display->currentBgPixel;
			auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Bounds check for bgID
			if (bgID < BG0 || bgID > BG1)
			{
				FATAL("Unknown BG in mode 1");
				RETURN;
			}

			GBA_WORD bgTileDataBaseAddr = (bgID == BG0)
				? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
				: (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

			auto xTileCoordinate = pGBA_display->bgCache[bgID].tileMapX & SEVEN; // % 8
			auto yTileCoordinate = pGBA_display->bgCache[bgID].tileMapY & SEVEN; // % 8

			xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
				? flipLUT[xTileCoordinate]
				: xTileCoordinate;

			yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
				? flipLUT[yTileCoordinate]
				: yTileCoordinate;

			GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
			// Refer http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
			DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

			// 4bpp mode
			if (pGBA_display->bgCache[bgID].is8bppMode == NO)
			{
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ bgTileDataBaseAddr
					+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
					+ (withinTileDataOffset >> ONE); // Divide by 2 because in 4bpp mode, A single halfword gives data for 4 pixels

				BYTE pixelColorNumberFor2PixelsFromTileData = RESET;

				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					pixelColorNumberFor2PixelsFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
				}
				else
				{
					PPUTODO("As per NBA, pixelColorNumberFor2PixelsFromTileData is set to some latched value");
				}

				// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
				// Odd means we need to shift right by 4
				pixelColorNumberFor2PixelsFromTileData >>= ((withinTileDataOffset & ONE) ? FOUR : ZERO);
				// extract only the required 4 bits
				BYTE pixelColorNumberPerPixel = pixelColorNumberFor2PixelsFromTileData & 0x0F;
				INC32 paletteIndex = pixelColorNumberPerPixel << ONE;

				if (paletteIndex != ZERO)
				{
					// Refer mode 2 bg processing to see how we determine sizeOfEachPaletteInBytes
					const DIM16 sizeOfEachPaletteInBytesInShifts = FIVE; // 1 << 5 is same as *32
					paletteIndex += (pGBA_display->bgCache[bgID].tileDescriptor.pb << sizeOfEachPaletteInBytesInShifts);
				}
				pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
			}
			// 8bpp mode
			else
			{
				// 8bpp mode
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ bgTileDataBaseAddr
					+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
					+ withinTileDataOffset;

				BYTE pixelColorNumberFor1PixelFromTileData = RESET;

				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (addressInTileDataArea < (VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					pixelColorNumberFor1PixelFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
				}
				else
				{
					PPUTODO("As per NBA, pixelColorNumberFor1PixelFromTileData is set to some latched value");
				}

				// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and palette index is 1, when tile data is 1
				INC32 paletteIndex = pixelColorNumberFor1PixelFromTileData << ONE;
				pGBA_display->gfx_bg[bgID][X][Y] = paletteIndex;
			}
		};
#else
	auto RENDER_MODE0_MODE1_PIXEL_X = [&](ID bgID, GBA_HALFWORD pixelData, STATE8 state)
		{
			auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			if (pGBA_display->currentBgPixelInTextMode[bgID] >= (TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)))
			{
				RETURN;
			}

			if (pGBA_display->bgCache[bgID].is8bppMode == NO)
			{
				// 4bpp mode

				GBA_HALFWORD pixelColorNumberFor4PixelsFromTileData = pixelData;

				if (pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES)
				{
					// First we take care of flipping 8 bits
					pixelColorNumberFor4PixelsFromTileData =
						((pixelColorNumberFor4PixelsFromTileData >> EIGHT) | (pixelColorNumberFor4PixelsFromTileData << EIGHT));

					// Next we flip the 4 bits within the already flipped 8 bits

					pixelColorNumberFor4PixelsFromTileData =
						(((pixelColorNumberFor4PixelsFromTileData & 0xF0F0) >> FOUR)
							| ((pixelColorNumberFor4PixelsFromTileData & 0x0F0F) << FOUR));
				}

				BYTE pixelColorNumberPerPixel = ZERO;
				// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and paletee index is 1, when tile data is 1
				INC32 paletteIndex = ZERO;

				// pixel
				if (pGBA_display->currentBgPixelInTextMode[bgID] < (TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)))
				{
					pixelColorNumberPerPixel = ((pixelColorNumberFor4PixelsFromTileData >> (state * FOUR)) & 0x000F);
					paletteIndex = pixelColorNumberPerPixel << ONE;
					if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops <= ZERO)
					{
						if (paletteIndex != ZERO)
						{
							// Refer mode 2 bg processing to see how we determine sizeOfEachPaletteInBytes
							const DIM16 sizeOfEachPaletteInBytesInShifts = FIVE; // 1 << 5 is same as *32
							paletteIndex += (pGBA_display->bgCache[bgID].tileDescriptor.pb << sizeOfEachPaletteInBytesInShifts);
						}
						pGBA_display->gfx_bg[bgID][pGBA_display->currentBgPixelInTextMode[bgID]++][Y] = paletteIndex;
						++pGBA_display->bgCache[bgID].subTileIndexer;
					}
				}

				if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops > ZERO)
				{
					--pGBA_display->bgCache[bgID].subTileScrollingPixelDrops;
				}
			}
			else
			{
				// 8bpp mode

				GBA_HALFWORD pixelColorNumberFor2PixelsFromTileData = pixelData;;

				if (pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES)
				{
					// We only take care of flipping 8 bits
					pixelColorNumberFor2PixelsFromTileData =
						((pixelColorNumberFor2PixelsFromTileData >> EIGHT) | (pixelColorNumberFor2PixelsFromTileData << EIGHT));
				}

				BYTE pixelColorNumberPerPixel = ZERO;
				// Note: Palette is of 16 bytes, so palette index is 0, when tile data is 0 and paletee index is 1, when tile data is 1
				INC32 paletteIndex = ZERO;

				// pixel
				if (pGBA_display->currentBgPixelInTextMode[bgID] < (TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)))
				{
					pixelColorNumberPerPixel = ((pixelColorNumberFor2PixelsFromTileData >> (state * EIGHT)) & 0x00FF);
					paletteIndex = pixelColorNumberPerPixel << ONE;
					if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops <= ZERO)
					{
						pGBA_display->gfx_bg[bgID][pGBA_display->currentBgPixelInTextMode[bgID]++][Y] = paletteIndex;
						++pGBA_display->bgCache[bgID].subTileIndexer;
					}
				}

				if (pGBA_display->bgCache[bgID].subTileScrollingPixelDrops > ZERO)
				{
					--pGBA_display->bgCache[bgID].subTileScrollingPixelDrops;
				}
			}
		};

	auto MODE0_M_BG_CYCLE = [&](ID bgID)
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixelInTextMode[bgID];
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
			pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

			GBA_WORD bgTileMapBaseAddr = ZERO;
			mBGnCNTHalfWord_t BGxCNT = { ZERO };

			// Cache BG configuration values for fast access
			auto getBGConfig = [&](ID id) -> std::tuple<int, FLAG, uint16_t, uint16_t, uint16_t>
				{
					switch (id)
					{
					case BG0:
						RETURN {
							static_cast<int>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
							pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord),
							static_cast<uint16_t>(pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					case BG1:
						RETURN {
							static_cast<int>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
							pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord),
							static_cast<uint16_t>(pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					case BG2:
						RETURN {
							static_cast<int>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
							pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord),
							static_cast<uint16_t>(pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					case BG3:
						RETURN {
							static_cast<int>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800),
							pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
							static_cast<uint16_t>(pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord),
							static_cast<uint16_t>(pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSFields.OFFSET),
							static_cast<uint16_t>(pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSFields.OFFSET)
						};
					default:
						// Safe fallback to avoid undefined behavior
						RETURN { 0, false, 0, 0, 0 };
					}
				};

			auto [bgTileMapBaseAddr_, is8bppMode, BGxCNT_, hofs_, vofs_] = getBGConfig(bgID);
			bgTileMapBaseAddr = bgTileMapBaseAddr_;
			pGBA_display->bgCache[bgID].is8bppMode = is8bppMode;
			BGxCNT.mBGnCNTHalfWord = BGxCNT_;
			pGBA_display->bgCache[bgID].hofs = hofs_;
			pGBA_display->bgCache[bgID].vofs = vofs_;

			/*
			* Refer http://problemkaputt.de/gbatek-lcd-i-o-bg-control.htm
			* BGs always wraparound, so
			* offsets = BGxHOFS and BGxHVOFS (BG offsets within static screen)
			* Final offsets from screen (tileMap_x and tileMap_y) = (x + BGxHOFS) % 512 and (y + BGxHVOFS) % 512
			* Offsets in tile coordinates (tile_x and tile_y) = Final offsets from screen / 8
			* Offsets within tile coordinates (WithinTile_x and WithinTile_y) = Final offsets from screen % 8
			*
			* When BGxCNT->Size = 0
			* SC0 -> 256x256
			* When BGxCNT->Size = 1
			* SC0 SC1 -> 512x256
			* When BGxCNT->Size = 2
			* SC0 -> 256x512
			* SC1
			* When BGxCNT->Size = 3
			* SC0 SC1 -> 512x512
			* SC2 SC3
			* SC0 is defined by the normal BG Map base address (Bit 8-12 of BGxCNT),
			* SC1 uses same address +2K, SC2 address +4K, SC3 address +6K
			* So, for given x, y we need to figure out in which SCn it falls...
			* Then each SCn is 256x256, so , number of tiles i.e. tilesPerRow = bitsPerRow / 8 = 32
			* So, withinTileOffset = (tile_x + (tile_y * tilesPerRow))
			* But, we do a 16bit read for tile map as mentioned under Mode 0 in https://nba-emu.github.io/hw-docs/ppu/background.html and http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm unlike non text modes
			* So, 16withinTileOffset = 2 * withinTileOffset
			*
			* Final Address for tile map = [VRAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 2K) + (16withinTileOffset)]
			*/

			// Cached sum of coordinates and offsets
			DIM16 offsetSumX = xPixelCoordinate + pGBA_display->bgCache[bgID].hofs;
			DIM16 offsetSumY = yPixelCoordinate + pGBA_display->bgCache[bgID].vofs;

			ID SCn = ZERO;
			switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
			{
				// SC0
			case ZERO: SCn = ZERO; BREAK;
				// SC0 SC1
			case ONE: SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO; BREAK;
				// SC0
				// SC1
			case TWO: SCn = ((offsetSumY & 511) > 255) ? ONE : ZERO; BREAK;
				// SC0 SC1
				// SC2 SC3
			case THREE:
				SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO;
				SCn += ((offsetSumY & 511) > 255) ? TWO : ZERO;
				BREAK;
			default:
				FATAL("Unknown screen size in mode 0");
				RETURN;
			}

			// Within SCn
			DIM16 tileMapX = offsetSumX & 255; // % 256 because because if more than 255, it is SC1 or SC3, but we are already accounting for SCn, so we just get tileX within SCn
			DIM16 tileMapY = offsetSumY & 255; // % 256 because because if more than 255, it is SC2 or SC3, but we are already accounting for SCn, so we just get tileX within SCn

			pGBA_display->bgCache[bgID].tileMapX = tileMapX;
			pGBA_display->bgCache[bgID].tileMapY = tileMapY;

			const DIM16 tilesPerRowInShifts = FIVE; // 1 << 5 is same as * 32; Each SCn is of 256*256, so 256 / 8 = 32
			DIM16 tileMapOffset = (tileMapX >> THREE) + ((tileMapY >> THREE) << tilesPerRowInShifts);

			// Since we have 16 bits per tile instead of 8
			// Refer http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
			tileMapOffset <<= ONE;

			GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileMapArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
				PPUTODO("As per NBA, this is set to some latched value");
			}

			pGBA_display->bgCache[bgID].subTileIndexer = ZERO;

			if ((tileMapX & SEVEN) != ZERO)
			{
				PPUEVENT("Sub-tile scrolling in mode 0 bg%d!", bgID);

				// This indicates sub-tile scrolling as mentioned in https://nba-emu.github.io/hw-docs/ppu/background.html
				// Assume that tileMapX % 8 is 5, this indicates that only 8-5=3 pixels from the end of previous tile needs to be rendered
				// As mentioned above, only few pixels of the 1st and last tile would be visible in this case...
				// but still, ppu fetches all 8 pixels
				// So, during the previous MODE0_M_BG_CYCLE, we would have fetched the complete tile descriptor for the whole tile (all 8 pixels)
				// Using the example that we were just discussing, 3 pixels from the end needs to be rendered in X=0, X=1 and X=2
				// i.e. first 5 pixels eventhough fetched from VRAM needs to be dropped

				if (xPixelCoordinate == ZERO)
				{
					pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = tileMapX & SEVEN;
				}
				else
				{
					pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
				}
			}
			else
			{
				pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
			}
		};

	auto MODE0_T_BG_CYCLE = [&](ID bgID)
		{
			auto X = pGBA_display->currentBgPixelInTextMode[bgID];
			auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Bounds check for bgID
			if (bgID < BG0 || bgID > BG3)
			{
				FATAL("Unknown BG in mode 0");
				RETURN;
			}

			if (ENABLED)
			{
				// Cache BG configuration values for fast access
				auto getBGConfig = [&](ID id) -> std::tuple<FLAG, uint16_t, uint16_t>
					{
						switch (id)
						{
						case BG0:
							RETURN {
								pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
								static_cast<uint16_t>(pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET),
								static_cast<uint16_t>(pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET)
							};
						case BG1:
							RETURN {
								pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
								static_cast<uint16_t>(pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET),
								static_cast<uint16_t>(pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET)
							};
						case BG2:
							RETURN {
								pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
								static_cast<uint16_t>(pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSFields.OFFSET),
								static_cast<uint16_t>(pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSFields.OFFSET)
							};
						case BG3:
							RETURN {
								pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET,
								static_cast<uint16_t>(pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSFields.OFFSET),
								static_cast<uint16_t>(pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSFields.OFFSET)
							};
						default:
							RETURN { false, 0, 0 };
						}
					};

				auto [is8bppMode, hofs_, vofs_] = getBGConfig(bgID);
				pGBA_display->bgCache[bgID].is8bppMode = is8bppMode;
				pGBA_display->bgCache[bgID].hofs = hofs_;
				pGBA_display->bgCache[bgID].vofs = vofs_;
			}

			GBA_WORD bgTileDataBaseAddr =
				(bgID == BG0)
				? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
				: (bgID == BG1)
				? (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
				: (bgID == BG2)
				? (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
				: (pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

			auto xTileCoordinate = ((X + pGBA_display->bgCache[bgID].hofs) & 255) & SEVEN; // % 8
			auto yTileCoordinate = ((Y + pGBA_display->bgCache[bgID].vofs) & 255) & SEVEN; // % 8

			xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
				? flipLUT[xTileCoordinate]
				: xTileCoordinate;

			yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
				? flipLUT[yTileCoordinate]
				: yTileCoordinate;

			GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
			// Refer http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
			DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

			if (pGBA_display->bgCache[bgID].is8bppMode == NO)
			{
				// 4bpp mode
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ bgTileDataBaseAddr
					+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
					+ (withinTileDataOffset >> ONE); // Divide by 2 because in 4bpp mode, A single halfword gives data for 4 pixels

				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
				}
				else
				{
					pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = RESET;
					PPUTODO("As per NBA, this is set to some latched value");
				}

				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ZERO);
				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ONE);
				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, TWO);
				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, THREE);
			}
			else
			{
				// 8bpp mode
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ bgTileDataBaseAddr
					+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
					+ withinTileDataOffset;

				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
				}
				else
				{
					pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = RESET;
					PPUTODO("As per NBA, this is set to some latched value");
				}

				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ZERO);
				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ONE);
			}
		};

	auto MODE1_M_TEXT_BG_CYCLE = [&](ID bgID)
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixelInTextMode[bgID];
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Bounds check for bgID
			if (bgID < BG0 || bgID > BG1)
			{
				FATAL("Unknown BG in mode 1");
				RETURN;
			}

			pGBA_display->bgCache[bgID].xPixelCoordinate = xPixelCoordinate;
			pGBA_display->bgCache[bgID].yPixelCoordinate = yPixelCoordinate;

			GBA_WORD bgTileMapBaseAddr = ZERO;
			mBGnCNTHalfWord_t BGxCNT = { ZERO };

			if (bgID == BG0)
			{
				bgTileMapBaseAddr = (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
				pGBA_display->bgCache[BG0].is8bppMode = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
				BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord;
				pGBA_display->bgCache[BG0].hofs = pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET;
				pGBA_display->bgCache[BG0].vofs = pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET;
			}
			else if (bgID == BG1)
			{
				bgTileMapBaseAddr = (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
				pGBA_display->bgCache[BG1].is8bppMode = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
				BGxCNT.mBGnCNTHalfWord = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord;
				pGBA_display->bgCache[BG1].hofs = pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET;
				pGBA_display->bgCache[BG1].vofs = pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET;
			}

			/*
			* Refer http://problemkaputt.de/gbatek-lcd-i-o-bg-control.htm
			* BGs always wraparound, so
			* offsets = BGxHOFS and BGxHVOFS (BG offsets within static screen)
			* Final offsets from screen (tileMap_x and tileMap_y) = (x + BGxHOFS) % 512 and (y + BGxHVOFS) % 512
			* Offsets in tile coordinates (tile_x and tile_y) = Final offsets from screen / 8
			* Offsets within tile coordinates (WithinTile_x and WithinTile_y) = Final offsets from screen % 8
			*
			* When BGxCNT->Size = 0
			* SC0 -> 256x256
			* When BGxCNT->Size = 1
			* SC0 SC1 -> 512x256
			* When BGxCNT->Size = 2
			* SC0 -> 256x512
			* SC1
			* When BGxCNT->Size = 3
			* SC0 SC1 -> 512x512
			* SC2 SC3
			* SC0 is defined by the normal BG Map base address (Bit 8-12 of BGxCNT),
			* SC1 uses same address +2K, SC2 address +4K, SC3 address +6K
			* So, for given x, y we need to figure out in which SCn it falls...
			* Then each SCn is 256x256, so , number of tiles i.e. tilesPerRow = bitsPerRow / 8 = 32
			* So, withinTileOffset = (tile_x + (tile_y * tilesPerRow))
			* But, we do a 16bit read for tile map as mentioned under Mode 0 in https://nba-emu.github.io/hw-docs/ppu/background.html and http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm unlike non text modes
			* So, 16withinTileOffset = 2 * withinTileOffset
			*
			* Final Address for tile map = [VRAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 2K) + (16withinTileOffset)]
			*
			*/

			// Cached sum of coordinates and offsets
			DIM16 offsetSumX = xPixelCoordinate + pGBA_display->bgCache[bgID].hofs;
			DIM16 offsetSumY = yPixelCoordinate + pGBA_display->bgCache[bgID].vofs;

			ID SCn = ZERO;
			switch (BGxCNT.mBGnCNTFields.SCREEN_SIZE)
			{
				// SC0
			case ZERO: SCn = ZERO; BREAK;
				// SC0 SC1
			case ONE: SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO; BREAK;
				// SC0
				// SC1
			case TWO: SCn = ((offsetSumY & 511) > 255) ? ONE : ZERO; BREAK;
				// SC0 SC1
				// SC2 SC3
			case THREE:
				SCn = ((offsetSumX & 511) > 255) ? ONE : ZERO;
				SCn += ((offsetSumY & 511) > 255) ? TWO : ZERO;
				BREAK;
			default:
				FATAL("Unknown screen size in mode 0");
				RETURN;
			}

			// Within SCn
			DIM16 tileMapX = offsetSumX & 255; // % 256 because because if more than 255, it is SC1 or SC3, but we are already accounting for SCn, so we just get tileX within SCn
			DIM16 tileMapY = offsetSumY & 255; // % 256 because because if more than 255, it is SC2 or SC3, but we are already accounting for SCn, so we just get tileX within SCn

			pGBA_display->bgCache[bgID].tileMapX = tileMapX;
			pGBA_display->bgCache[bgID].tileMapY = tileMapY;

			const DIM16 tilesPerRowInShifts = FIVE; // 1 << 5 is same as * 32; Each SCn is of 256*256, so 256 / 8 = 32
			DIM16 tileMapOffset = (tileMapX >> THREE) + ((tileMapY >> THREE) << tilesPerRowInShifts);

			// Since we have 16 bits per tile instead of 8
			// Refer http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
			tileMapOffset <<= ONE;

			GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + (SCn * 0x800) + tileMapOffset;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileMapArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[bgID].tileDescriptor.raw = readRawMemory<GBA_HALFWORD>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				pGBA_display->bgCache[bgID].tileDescriptor.raw = RESET;
				PPUTODO("As per NBA, this is set to some latched value");
			}

			pGBA_display->bgCache[bgID].subTileIndexer = ZERO;

			if ((tileMapX & SEVEN) != ZERO)
			{
				PPUEVENT("Sub-tile scrolling in mode 1 bg%d!", bgID);

				// This indicates sub-tile scrolling as mentioned in https://nba-emu.github.io/hw-docs/ppu/background.html
				// Assume that tileMapX % 8 is 5, this indicates that only 8-5=3 pixels from the end of previous tile needs to be rendered
				// As mentioned above, only few pixels of the 1st and last tile would be visible in this case...
				// but still, ppu fetches all 8 pixels
				// So, during the previous MODE0_M_BG_CYCLE, we would have fetched the complete tile descriptor for the whole tile (all 8 pixels)
				// Using the example that we were just discussing, 3 pixels from the end needs to be rendered in X=0, X=1 and X=2
				// i.e. first 5 pixels eventhough fetched from VRAM needs to be dropped

				if (xPixelCoordinate == ZERO)
				{
					pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = tileMapX & SEVEN;
				}
				else
				{
					pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
				}
			}
			else
			{
				pGBA_display->bgCache[bgID].subTileScrollingPixelDrops = RESET;
			}
		};

	auto MODE1_T_TEXT_BG_CYCLE = [&](ID bgID)
		{
			auto X = pGBA_display->currentBgPixelInTextMode[bgID];
			auto Y = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((X >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (Y >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Bounds check for bgID
			if (bgID < BG0 || bgID > BG1)
			{
				FATAL("Unknown BG in mode 1");
				RETURN;
			}

			if (ENABLED)
			{
				// Cache BG configuration values for fast access
				if (bgID == BG0)
				{
					pGBA_display->bgCache[BG0].is8bppMode = pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
					pGBA_display->bgCache[BG0].hofs = pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSFields.OFFSET;
					pGBA_display->bgCache[BG0].vofs = pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSFields.OFFSET;
				}
				else if (bgID == BG1)
				{
					pGBA_display->bgCache[BG1].is8bppMode = pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.COLOR_PALETTES == SET;
					pGBA_display->bgCache[BG1].hofs = pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSFields.OFFSET;
					pGBA_display->bgCache[BG1].vofs = pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSFields.OFFSET;
				}
			}

			GBA_WORD bgTileDataBaseAddr = (bgID == BG0)
				? (pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000)
				: (pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);

			auto xTileCoordinate = ((X + pGBA_display->bgCache[bgID].hofs) & 255) & SEVEN; // % 8
			auto yTileCoordinate = ((Y + pGBA_display->bgCache[bgID].vofs) & 255) & SEVEN; // % 8

			xTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.hflip == YES
				? flipLUT[xTileCoordinate]
				: xTileCoordinate;

			yTileCoordinate = pGBA_display->bgCache[bgID].tileDescriptor.vflip == YES
				? flipLUT[yTileCoordinate]
				: yTileCoordinate;

			GBA_WORD withinTileDataOffset = xTileCoordinate + (yTileCoordinate << THREE);
			// Refer http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
			DIM16 sizeOfEachTileData = (pGBA_display->bgCache[bgID].is8bppMode == NO) ? 0x20 : 0x40;

			if (pGBA_display->bgCache[bgID].is8bppMode == NO)
			{
				// 4bpp mode
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ bgTileDataBaseAddr
					+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
					+ (withinTileDataOffset >> ONE); // Divide by 2 because in 4bpp mode, A single halfword gives data for 4 pixels

				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
				}
				else
				{
					pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData = RESET;
					PPUTODO("As per NBA, this is set to some latched value");
				}

				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ZERO);
				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, ONE);
				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, TWO);
				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor4PixelsFromTileData, THREE);
			}
			else
			{
				// 8bpp mode
				GBA_WORD addressInTileDataArea =
					VIDEO_RAM_START_ADDRESS
					+ bgTileDataBaseAddr
					+ (sizeOfEachTileData * pGBA_display->bgCache[bgID].tileDescriptor.fetchedTileID)
					+ withinTileDataOffset;

				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = readRawMemory<GBA_HALFWORD>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);
				}
				else
				{
					pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData = RESET;
					PPUTODO("As per NBA, this is set to some latched value");
				}

				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ZERO);
				RENDER_MODE0_MODE1_PIXEL_X(bgID, pGBA_display->bgCache[bgID].pixelColorNumberFor2PixelsFromTileData, ONE);
			}
		};
#endif

	auto MODE1_M_AFFINE_BG_CYCLE = [&]()
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			GBA_WORD bgTileMapBaseAddr = (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800);
			DIM16 bgWidth = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);
			DIM16 bgHeight = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);
			FLAG isWrapAroundEnabled = (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT == ONE);

			uint32_t xPixelCoordinateAffine = (GBA_WORD)(pGBA_display->bgCache[BG2].affine.affineX >> EIGHT);	// To get the integer part
			uint32_t yPixelCoordinateAffine = (GBA_WORD)(pGBA_display->bgCache[BG2].affine.affineY >> EIGHT);	// To get the integer part

			// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
			// NOTE: In other modes, adding of PA and PC is done at the end of the pixel processing, but as per NBA, in mode 2, this is done at this point!
			pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
			pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);

			// Used for repetition of particular block of tiles 
			if (isWrapAroundEnabled == YES)
			{
				xPixelCoordinateAffine %= bgWidth;
				yPixelCoordinateAffine %= bgHeight;
			}

			pGBA_display->bgCache[BG2].xPixelCoordinateAffine = xPixelCoordinateAffine;
			pGBA_display->bgCache[BG2].yPixelCoordinateAffine = yPixelCoordinateAffine;

			uint32_t xTileCoordinateAffine = xPixelCoordinateAffine >> THREE;
			uint32_t yTileCoordinateAffine = yPixelCoordinateAffine >> THREE;

			auto bitsPerRow = bgWidth;
			auto tilesPerRow = bitsPerRow >> THREE;

			GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + xTileCoordinateAffine + (yTileCoordinateAffine * tilesPerRow);

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileMapArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[BG2].fetchedTileID = readRawMemory<BYTE>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				pGBA_display->bgCache[BG2].fetchedTileID = readRawMemory<BYTE>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
				PPUTODO("As per NBA, this is set to some latched value");
			}
		};

	auto MODE1_T_AFFINE_BG_CYCLE = [&]()
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			GBA_WORD bgTileDataBaseAddr = ZERO;
			DIM16 bgWidth = ZERO;
			DIM16 bgHeight = ZERO;
			FLAG isWrapAroundEnabled = NO;

			bgTileDataBaseAddr = (pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000);
			bgWidth = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);
			bgHeight = (0x80 << pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTFields.SCREEN_SIZE);

			uint32_t xPixelCoordinateAffine = pGBA_display->bgCache[BG2].xPixelCoordinateAffine;
			uint32_t yPixelCoordinateAffine = pGBA_display->bgCache[BG2].yPixelCoordinateAffine;

			// As per http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
			// For Rotation/Scaling BG Screen -> In this mode, only 256 tiles can be used. There are no x/y-flip attributes, the color depth is always 256 colors/1 palette.
			// And as per http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
			// For 8bit depth (256 colors, 1 palette) -> Each tile occupies 64 bytes of memory, the first 8 bytes for the topmost row of the tile, and so on. Each byte selects the palette entry for each dot.

			uint32_t sizeOfEachTileData = 0x40;

			// As mentioned above, within 64 byte tile data, first 8 byte represents the colors for the 8 pixels within the top most row...and so on
			// so to find the offset with the 64 byte data...
			// withinTileOffset = 0 -> color for xTileCoordinate = 0, yTileCoordinate = 0
			// withinTileOffset = 1 -> color for xTileCoordinate = 1, yTileCoordinate = 0 
			//		:						 
			// withinTileOffset = 7 -> color for xTileCoordinate = 7, yTileCoordinate = 0 
			// withinTileOffset = 8 -> color for xTileCoordinate = 0, yTileCoordinate = 1 

			auto xTileCoordinateAffine = xPixelCoordinateAffine & SEVEN;
			auto yTileCoordinateAffine = yPixelCoordinateAffine & SEVEN;

			GBA_WORD withinTileDataOffsetAffine = xTileCoordinateAffine + (yTileCoordinateAffine << THREE);

			GBA_WORD addressInTileDataArea = VIDEO_RAM_START_ADDRESS + bgTileDataBaseAddr + (sizeOfEachTileData * pGBA_display->bgCache[BG2].fetchedTileID) + withinTileDataOffsetAffine;
			BYTE pixelColorNumberFromTileData = RESET;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				PPUTODO("As per NBA, pixelColorNumberFromTileData is set to some latched value");
			}

			ID paletteIndex = pixelColorNumberFromTileData << ONE; // Palette is 16bit data, (ex: tile id 1 should access 2nd and 3rd byte of palette ram) 

			// If out of bounds, needs to be tranparent, so palette index to 0
			if (xPixelCoordinateAffine >= bgWidth || yPixelCoordinateAffine >= bgHeight)
			{
				// Condition will be true only if wrap around was disabled!
				paletteIndex = ZERO;
			}

			// Now use the screen coordinates and fill the gfx buffer with data obtained from texture coordinates (Affine)
			pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = paletteIndex;
		};

	auto MODE2_M_BG_CYCLE = [&](ID bgID)
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			auto& bgCntHalfWord = (bgID == BG2)
				? pGBA_peripherals->mBG2CNTHalfWord
				: pGBA_peripherals->mBG3CNTHalfWord;

			GBA_WORD bgTileMapBaseAddr = bgCntHalfWord.mBGnCNTFields.SCREEN_BASE_BLOCK * 0x0800;
			DIM16 bgWidth = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;
			DIM16 bgHeight = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;
			FLAG isWrapAroundEnabled = bgCntHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT == ONE;

			uint32_t xPixelCoordinateAffine, yPixelCoordinateAffine;

			// Precompute affine coordinates and update them
			auto& bgCache = pGBA_display->bgCache[bgID];
			auto& affineX = bgCache.affine.affineX;
			auto& affineY = bgCache.affine.affineY;

			xPixelCoordinateAffine = affineX >> EIGHT;	// To get the integer part
			yPixelCoordinateAffine = affineY >> EIGHT;	// To get the integer part

			auto affineXIncrement =
				(bgID == BG2) ? pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed
				: pGBA_peripherals->mBG3PAHalfWord.mBGnPxHalfWord_Signed;
			auto affineYIncrement =
				(bgID == BG2) ? pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed
				: pGBA_peripherals->mBG3PCHalfWord.mBGnPxHalfWord_Signed;

			// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
			// NOTE: In other modes, adding of PA and PC is done at the end of the pixel processing, but as per NBA, in mode 2, this is done at this point!
			affineX += affineXIncrement;
			affineY += affineYIncrement;

			// Used for repetition of particular block of tiles 
			if (isWrapAroundEnabled == YES)
			{
				xPixelCoordinateAffine %= bgWidth;
				yPixelCoordinateAffine %= bgHeight;
			}

			bgCache.xPixelCoordinateAffine = xPixelCoordinateAffine;
			bgCache.yPixelCoordinateAffine = yPixelCoordinateAffine;

			uint32_t xTileCoordinateAffine = xPixelCoordinateAffine >> THREE;
			uint32_t yTileCoordinateAffine = yPixelCoordinateAffine >> THREE;

			auto bitsPerRow = bgWidth;
			auto tilesPerRow = bitsPerRow >> THREE;

			GBA_WORD addressInTileMapArea = VIDEO_RAM_START_ADDRESS + bgTileMapBaseAddr + xTileCoordinateAffine + (yTileCoordinateAffine * tilesPerRow);

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileMapArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pGBA_display->bgCache[bgID].fetchedTileID = readRawMemory<BYTE>(addressInTileMapArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);;
			}
			else
			{
				pGBA_display->bgCache[bgID].fetchedTileID = RESET;
				PPUTODO("As per NBA, this is set to some latched value");
			}
		};

	auto MODE2_T_BG_CYCLE = [&](ID bgID)
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Use ternary operator for efficient branching
			auto& bgCntHalfWord = (bgID == BG2)
				? pGBA_peripherals->mBG2CNTHalfWord
				: pGBA_peripherals->mBG3CNTHalfWord;

			GBA_WORD bgTileDataBaseAddr = bgCntHalfWord.mBGnCNTFields.CHARACTER_BASE_BLOCK * 0x4000;
			DIM16 bgWidth = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;
			DIM16 bgHeight = 0x80 << bgCntHalfWord.mBGnCNTFields.SCREEN_SIZE;

			uint32_t xPixelCoordinateAffine = pGBA_display->bgCache[bgID].xPixelCoordinateAffine;
			uint32_t yPixelCoordinateAffine = pGBA_display->bgCache[bgID].yPixelCoordinateAffine;

			// As per http://problemkaputt.de/gbatek-lcd-vram-bg-screen-data-format-bg-map.htm
			// For Rotation/Scaling BG Screen -> In this mode, only 256 tiles can be used. There are no x/y-flip attributes, the color depth is always 256 colors/1 palette.
			// And as per http://problemkaputt.de/gbatek-lcd-vram-character-data.htm
			// For 8bit depth (256 colors, 1 palette) -> Each tile occupies 64 bytes of memory, the first 8 bytes for the topmost row of the tile, and so on. Each byte selects the palette entry for each dot.

			const uint32_t sizeOfEachTileData = 0x40;

			// As mentioned above, within 64 byte tile data, first 8 byte represents the colors for the 8 pixels within the top most row...and so on
			// so to find the offset with the 64 byte data...
			// withinTileOffset = 0 -> color for xTileCoordinate = 0, yTileCoordinate = 0
			// withinTileOffset = 1 -> color for xTileCoordinate = 1, yTileCoordinate = 0 
			//		:						 
			// withinTileOffset = 7 -> color for xTileCoordinate = 7, yTileCoordinate = 0 
			// withinTileOffset = 8 -> color for xTileCoordinate = 0, yTileCoordinate = 1 

			auto xTileCoordinateAffine = xPixelCoordinateAffine & SEVEN;
			auto yTileCoordinateAffine = yPixelCoordinateAffine & SEVEN;

			GBA_WORD withinTileDataOffsetAffine = xTileCoordinateAffine + (yTileCoordinateAffine << THREE);

			GBA_WORD addressInTileDataArea = VIDEO_RAM_START_ADDRESS + bgTileDataBaseAddr + (sizeOfEachTileData * pGBA_display->bgCache[bgID].fetchedTileID) + withinTileDataOffsetAffine;

			BYTE pixelColorNumberFromTileData = RESET;

			// NOTE: Needed for tonc's cbb_demo
			// Refer: https://www.coranac.com/tonc/text/regbg.htm
			if (addressInTileDataArea < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
			{
				pixelColorNumberFromTileData = readRawMemory<BYTE>(addressInTileDataArea, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);
			}
			else
			{
				PPUTODO("As per NBA, pixelColorNumberFromTileData is set to some latched value");
			}

			ID paletteIndex = pixelColorNumberFromTileData << ONE; // Palette is 16bit data, (ex: tile id 1 should access 2nd and 3rd byte of palette ram) 

			// If out of bounds, needs to be tranparent, so palette index to 0
			if (xPixelCoordinateAffine >= bgWidth || yPixelCoordinateAffine >= bgHeight)
			{
				// Condition will be true only if wrap around was disabled!
				paletteIndex = ZERO;
			}

			// Now use the screen coordinates and fill the gfx buffer with data obtained from texture coordinates (Affine)
			pGBA_display->gfx_bg[bgID][xPixelCoordinate][yPixelCoordinate] = paletteIndex;
		};

	auto MODE3_B_BG_CYCLE = [&]()
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Only BG2 is valid for Mode2

			// Refer to http://problemkaputt.de/gbatek-lcd-vram-bitmap-bg-modes.htm
			uint32_t xPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineX) >> EIGHT); // To get the integer part
			uint32_t yPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineY) >> EIGHT); // To get the integer part

			if (xPixelCoordinateAffine >= getScreenWidth() || yPixelCoordinateAffine >= getScreenHeight())
			{
				pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = ZERO;
			}
			else
			{
				ID vramIndex = xPixelCoordinateAffine + (yPixelCoordinateAffine * getScreenWidth());

				// Since we are accessing 16 bit data instead of 8 bit
				vramIndex <<= ONE;

				GBA_WORD vramAddress = VIDEO_RAM_START_ADDRESS + vramIndex;

				GBA_HALFWORD colorFor1Pixel = RESET;
				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (vramAddress < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					colorFor1Pixel = readRawMemory<GBA_HALFWORD>(vramAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

				}
				else
				{
					PPUTODO("As per NBA, colorFor1Pixel is set to some latched value");
				}

				pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = (colorFor1Pixel | IS_COLOR_NOT_PALETTE);
			}

			// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
			pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
			pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);
		};

	auto MODE4_B_BG_CYCLE = [&]()
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Only BG2 is valid for Mode2

			// Refer to http://problemkaputt.de/gbatek-lcd-vram-bitmap-bg-modes.htm
			uint32_t xPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineX) >> EIGHT); // To get the integer part
			uint32_t yPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineY) >> EIGHT); // To get the integer part

			if (xPixelCoordinateAffine >= getScreenWidth() || yPixelCoordinateAffine >= getScreenHeight())
			{
				pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = ZERO;
			}
			else
			{
				ID vramIndex = xPixelCoordinateAffine + (yPixelCoordinateAffine * getScreenWidth());

				GBA_WORD vramAddress = VIDEO_RAM_START_ADDRESS + (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FRAME_SELECT * 0xA000) + vramIndex;

				GBA_HALFWORD pixelColorNumberFromTileData = RESET;
				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (vramAddress < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					pixelColorNumberFromTileData = readRawMemory<BYTE>(vramAddress, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::PPU);

				}
				else
				{
					PPUTODO("As per NBA, pixelColorNumberFromTileData is set to some latched value");
				}

				// NOTE: Palette index multiplied by 2 as each palette index represents 16 bit offset in palette ram instead of 8 bit offset, so basically there is 2 byte difference b/w palette index n to index n+1
				ID paletteIndex = pixelColorNumberFromTileData << ONE;

				pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = paletteIndex;
			}

			// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
			pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
			pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);
		};

	auto MODE5_B_BG_CYCLE = [&]()
		{
			uint32_t xPixelCoordinate = pGBA_display->currentBgPixel;
			uint32_t yPixelCoordinate = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;

			// Early bounds check on X and Y
			if ((xPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_VISIBLE_PIXEL_PER_LINES)) || (yPixelCoordinate >= TO_UINT16(LCD_DIMENSIONS::LCD_TOTAL_V_LINES)))
			{
				RETURN;
			}

			// Only BG2 is valid for Mode2

			// Refer to http://problemkaputt.de/gbatek-lcd-vram-bitmap-bg-modes.htm
			uint32_t xPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineX) >> EIGHT); // To get the integer part
			uint32_t yPixelCoordinateAffine = (GBA_WORD)((pGBA_display->bgCache[BG2].affine.affineY) >> EIGHT); // To get the integer part

			if (xPixelCoordinateAffine >= ONEHUNDREDSIXTY || yPixelCoordinateAffine >= ONETWENTYEIGHT)
			{
				pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = ZERO;
			}
			else
			{
				ID vramIndex = xPixelCoordinateAffine + (yPixelCoordinateAffine * ONEHUNDREDSIXTY);

				// Since we are accessing 16 bit data instead of 8 bit
				vramIndex <<= ONE;

				GBA_WORD vramAddress = VIDEO_RAM_START_ADDRESS + (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.FRAME_SELECT * 0xA000) + vramIndex;

				GBA_HALFWORD colorFor1Pixel = RESET;
				// NOTE: Needed for tonc's cbb_demo
				// Refer: https://www.coranac.com/tonc/text/regbg.htm
				if (vramAddress < static_cast<GBA_WORD>(VIDEO_RAM_START_ADDRESS + ((pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE >= MODE3) ? 0x14000 : 0x10000)))
				{
					colorFor1Pixel = readRawMemory<GBA_HALFWORD>(vramAddress, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::PPU);

				}
				else
				{
					PPUTODO("As per NBA, colorFor1Pixel is set to some latched value");
				}

				pGBA_display->gfx_bg[BG2][xPixelCoordinate][yPixelCoordinate] = (colorFor1Pixel | IS_COLOR_NOT_PALETTE);
			}

			// For points (2) and (4) of PPU:03 AND point (6) of PPU:03
			pGBA_display->bgCache[BG2].affine.affineX += (int16_t)(pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord_Signed);
			pGBA_display->bgCache[BG2].affine.affineY += (int16_t)(pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord_Signed);
		};

	auto MODE0_BG_SEQUENCE = [&](SSTATE32 state)
		{
			switch (state)
			{
			case ZERO:
			{
				MODE0_M_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_T_BG_CYCLE(BG0);
#endif
				BREAK;
			}
			case ONE:
			{
				MODE0_M_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_T_BG_CYCLE(BG1);
#endif
				BREAK;
			}
			case TWO:
			{
				MODE0_M_BG_CYCLE(BG2);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_T_BG_CYCLE(BG2);
#endif
				BREAK;
			}
			case THREE:
			{
				MODE0_M_BG_CYCLE(BG3);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_T_BG_CYCLE(BG3);
#endif
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case FOUR:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG0);
#endif
				MODE0_T_BG_CYCLE(BG0);
				BREAK;
			}
			case FIVE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG1);
#endif
				MODE0_T_BG_CYCLE(BG1);
				BREAK;
			}
			case SIX:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG2);
#endif
				MODE0_T_BG_CYCLE(BG2);
				BREAK;
			}
			case SEVEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG3);
#endif
				MODE0_T_BG_CYCLE(BG3);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case EIGHT:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG0);
				MODE0_T_BG_CYCLE(BG0);
#endif
				BREAK;
			}
			case NINE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG1);
				MODE0_T_BG_CYCLE(BG1);
#endif
				BREAK;
			}
			case TEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG2);
				MODE0_T_BG_CYCLE(BG2);
#endif
				BREAK;
			}
			case ELEVEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG3);
				MODE0_T_BG_CYCLE(BG3);
#endif
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWELVE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG0);
				MODE0_T_BG_CYCLE(BG0);
#else
				if (pGBA_display->bgCache[BG0].is8bppMode == YES)
				{
					MODE0_T_BG_CYCLE(BG0);
				}
#endif
				BREAK;
			}
			case THIRTEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG1);
				MODE0_T_BG_CYCLE(BG1);
#else
				if (pGBA_display->bgCache[BG1].is8bppMode == YES)
				{
					MODE0_T_BG_CYCLE(BG1);
				}
#endif
				BREAK;
			}
			case FOURTEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG2);
				MODE0_T_BG_CYCLE(BG2);
#else
				if (pGBA_display->bgCache[BG2].is8bppMode == YES)
				{
					MODE0_T_BG_CYCLE(BG2);
				}
#endif
				BREAK;
			}
			case FIFTEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG3);
				MODE0_T_BG_CYCLE(BG3);
#else
				if (pGBA_display->bgCache[BG3].is8bppMode == YES)
				{
					MODE0_T_BG_CYCLE(BG3);
				}
#endif
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case SIXTEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG0);
				MODE0_T_BG_CYCLE(BG0);
#endif
				BREAK;
			}
			case SEVENTEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG1);
				MODE0_T_BG_CYCLE(BG1);
#endif
				BREAK;
			}
			case EIGHTEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG2);
				MODE0_T_BG_CYCLE(BG2);
#endif
				BREAK;
			}
			case NINETEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG3);
				MODE0_T_BG_CYCLE(BG3);
#endif
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTY:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG0);
#endif
				MODE0_T_BG_CYCLE(BG0);
				BREAK;
			}
			case TWENTYONE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG1);
#endif
				MODE0_T_BG_CYCLE(BG1);
				BREAK;
			}
			case TWENTYTWO:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG2);
#endif
				MODE0_T_BG_CYCLE(BG2);
				BREAK;
			}
			case TWENTYTHREE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG3);
#endif
				MODE0_T_BG_CYCLE(BG3);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYFOUR:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG0);
				MODE0_T_BG_CYCLE(BG0);
#endif
				BREAK;
			}
			case TWENTYFIVE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG1);
				MODE0_T_BG_CYCLE(BG1);
#endif
				BREAK;
			}
			case TWENTYSIX:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG2);
				MODE0_T_BG_CYCLE(BG2);
#endif
				BREAK;
			}
			case TWENTYSEVEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG3);
				MODE0_T_BG_CYCLE(BG3);
#endif
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYEIGHT:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG0);
				MODE0_T_BG_CYCLE(BG0);
#else
				if (pGBA_display->bgCache[BG0].is8bppMode == YES)
				{
					MODE0_T_BG_CYCLE(BG0);
				}
#endif
				BREAK;
			}
			case TWENTYNINE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG1);
				MODE0_T_BG_CYCLE(BG1);
#else
				if (pGBA_display->bgCache[BG1].is8bppMode == YES)
				{
					MODE0_T_BG_CYCLE(BG1);
				}
#endif
				BREAK;
			}
			case THIRTY:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG2);
				MODE0_T_BG_CYCLE(BG2);
#else
				if (pGBA_display->bgCache[BG2].is8bppMode == YES)
				{
					MODE0_T_BG_CYCLE(BG2);
				}
#endif
				BREAK;
			}
			case THIRTYONE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE0_M_BG_CYCLE(BG3);
				MODE0_T_BG_CYCLE(BG3);
#else
				if (pGBA_display->bgCache[BG3].is8bppMode == YES)
				{
					MODE0_T_BG_CYCLE(BG3);
				}
#endif
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			default:
			{
				FATAL("PPU mode 0 (BG) out of sync");
			}
			}
		};

	auto MODE1_BG_SEQUENCE = [&](SSTATE32 state)
		{
			switch (state)
			{
			case ZERO:
			{
				MODE1_M_TEXT_BG_CYCLE(BG0);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
				BREAK;
			}
			case ONE:
			{
				MODE1_M_TEXT_BG_CYCLE(BG1);
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
				BREAK;
			}
			case TWO:
			{
				MODE1_M_AFFINE_BG_CYCLE();
				BREAK;
			}
			case THREE:
			{
				MODE1_T_AFFINE_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case FOUR:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG0);
#endif
				MODE1_T_TEXT_BG_CYCLE(BG0);
				BREAK;
			}
			case FIVE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG1);
#endif
				MODE1_T_TEXT_BG_CYCLE(BG1);
				BREAK;
			}
			case SIX:
			{
				MODE1_M_AFFINE_BG_CYCLE();
				BREAK;
			}
			case SEVEN:
			{
				MODE1_T_AFFINE_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case EIGHT:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG0);
				MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
				BREAK;
			}
			case NINE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG1);
				MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
				BREAK;
			}
			case TEN:
			{
				MODE1_M_AFFINE_BG_CYCLE();
				BREAK;
			}
			case ELEVEN:
			{
				MODE1_T_AFFINE_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWELVE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG0);
				MODE1_T_TEXT_BG_CYCLE(BG0);
#else
				if (pGBA_display->bgCache[BG0].is8bppMode == YES)
				{
					MODE1_T_TEXT_BG_CYCLE(BG0);
				}
#endif
				BREAK;
			}
			case THIRTEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG1);
				MODE1_T_TEXT_BG_CYCLE(BG1);
#else
				if (pGBA_display->bgCache[BG1].is8bppMode == YES)
				{
					MODE1_T_TEXT_BG_CYCLE(BG1);
				}
#endif
				BREAK;
			}
			case FOURTEEN:
			{
				MODE1_M_AFFINE_BG_CYCLE();
				BREAK;
			}
			case FIFTEEN:
			{
				MODE1_T_AFFINE_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case SIXTEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG0);
				MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
				BREAK;
			}
			case SEVENTEEN:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG1);
				MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
				BREAK;
			}
			case EIGHTEEN:
			{
				MODE1_M_AFFINE_BG_CYCLE();
				BREAK;
			}
			case NINETEEN:
			{
				MODE1_T_AFFINE_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTY:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG0);
#endif
				MODE1_T_TEXT_BG_CYCLE(BG0);
				BREAK;
			}
			case TWENTYONE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG1);
#endif
				MODE1_T_TEXT_BG_CYCLE(BG1);
				BREAK;
			}
			case TWENTYTWO:
			{
				MODE1_M_AFFINE_BG_CYCLE();
				BREAK;
			}
			case TWENTYTHREE:
			{
				MODE1_T_AFFINE_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYFOUR:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG0);
				MODE1_T_TEXT_BG_CYCLE(BG0);
#endif
				BREAK;
			}
			case TWENTYFIVE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG1);
				MODE1_T_TEXT_BG_CYCLE(BG1);
#endif
				BREAK;
			}
			case TWENTYSIX:
			{
				MODE1_M_AFFINE_BG_CYCLE();
				BREAK;
			}
			case TWENTYSEVEN:
			{
				MODE1_T_AFFINE_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYEIGHT:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG0);
				MODE1_T_TEXT_BG_CYCLE(BG0);
#else
				if (pGBA_display->bgCache[BG0].is8bppMode == YES)
				{
					MODE1_T_TEXT_BG_CYCLE(BG0);

				}
#endif
				BREAK;
			}
			case TWENTYNINE:
			{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
				MODE1_M_TEXT_BG_CYCLE(BG1);
				MODE1_T_TEXT_BG_CYCLE(BG1);
#else
				if (pGBA_display->bgCache[BG1].is8bppMode == YES)
				{
					MODE1_T_TEXT_BG_CYCLE(BG1);

				}
#endif
				BREAK;
			}
			case THIRTY:
			{
				MODE1_M_AFFINE_BG_CYCLE();
				BREAK;
			}
			case THIRTYONE:
			{
				MODE1_T_AFFINE_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			default:
			{
				FATAL("PPU mode 1 out of sync");
			}
			}
		};

	auto MODE2_BG_SEQUENCE = [&](SSTATE32 state)
		{
			switch (state)
			{
			case ZERO:
			{
				MODE2_M_BG_CYCLE(BG3);
				BREAK;
			}
			case ONE:
			{
				MODE2_T_BG_CYCLE(BG3);
				BREAK;
			}
			case TWO:
			{
				MODE2_M_BG_CYCLE(BG2);
				BREAK;
			}
			case THREE:
			{
				MODE2_T_BG_CYCLE(BG2);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case FOUR:
			{
				MODE2_M_BG_CYCLE(BG3);
				BREAK;
			}
			case FIVE:
			{
				MODE2_T_BG_CYCLE(BG3);
				BREAK;
			}
			case SIX:
			{
				MODE2_M_BG_CYCLE(BG2);
				BREAK;
			}
			case SEVEN:
			{
				MODE2_T_BG_CYCLE(BG2);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case EIGHT:
			{
				MODE2_M_BG_CYCLE(BG3);
				BREAK;
			}
			case NINE:
			{
				MODE2_T_BG_CYCLE(BG3);
				BREAK;
			}
			case TEN:
			{
				MODE2_M_BG_CYCLE(BG2);
				BREAK;
			}
			case ELEVEN:
			{
				MODE2_T_BG_CYCLE(BG2);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWELVE:
			{
				MODE2_M_BG_CYCLE(BG3);
				BREAK;
			}
			case THIRTEEN:
			{
				MODE2_T_BG_CYCLE(BG3);
				BREAK;
			}
			case FOURTEEN:
			{
				MODE2_M_BG_CYCLE(BG2);
				BREAK;
			}
			case FIFTEEN:
			{
				MODE2_T_BG_CYCLE(BG2);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case SIXTEEN:
			{
				MODE2_M_BG_CYCLE(BG3);
				BREAK;
			}
			case SEVENTEEN:
			{
				MODE2_T_BG_CYCLE(BG3);
				BREAK;
			}
			case EIGHTEEN:
			{
				MODE2_M_BG_CYCLE(BG2);
				BREAK;
			}
			case NINETEEN:
			{
				MODE2_T_BG_CYCLE(BG2);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTY:
			{
				MODE2_M_BG_CYCLE(BG3);
				BREAK;
			}
			case TWENTYONE:
			{
				MODE2_T_BG_CYCLE(BG3);
				BREAK;
			}
			case TWENTYTWO:
			{
				MODE2_M_BG_CYCLE(BG2);
				BREAK;
			}
			case TWENTYTHREE:
			{
				MODE2_T_BG_CYCLE(BG2);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYFOUR:
			{
				MODE2_M_BG_CYCLE(BG3);
				BREAK;
			}
			case TWENTYFIVE:
			{
				MODE2_T_BG_CYCLE(BG3);
				BREAK;
			}
			case TWENTYSIX:
			{
				MODE2_M_BG_CYCLE(BG2);
				BREAK;
			}
			case TWENTYSEVEN:
			{
				MODE2_T_BG_CYCLE(BG2);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYEIGHT:
			{
				MODE2_M_BG_CYCLE(BG3);
				BREAK;
			}
			case TWENTYNINE:
			{
				MODE2_T_BG_CYCLE(BG3);
				BREAK;
			}
			case THIRTY:
			{
				MODE2_M_BG_CYCLE(BG2);
				BREAK;
			}
			case THIRTYONE:
			{
				MODE2_T_BG_CYCLE(BG2);
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			default:
			{
				FATAL("PPU mode 2) out of sync");
			}
			}
		};

	auto MODE3_BG_SEQUENCE = [&](SSTATE32 state)
		{
			switch (state)
			{
			case THREE:
			{
				MODE3_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case SEVEN:
			{
				MODE3_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case ELEVEN:
			{
				MODE3_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case FIFTEEN:
			{
				MODE3_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case NINETEEN:
			{
				MODE3_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYTHREE:
			{
				MODE3_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYSEVEN:
			{
				MODE3_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case THIRTYONE:
			{
				MODE3_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			}
		};

	auto MODE4_BG_SEQUENCE = [&](SSTATE32 state)
		{
			switch (state)
			{
			case THREE:
			{
				MODE4_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case SEVEN:
			{
				MODE4_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case ELEVEN:
			{
				MODE4_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case FIFTEEN:
			{
				MODE4_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case NINETEEN:
			{
				MODE4_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYTHREE:
			{
				MODE4_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYSEVEN:
			{
				MODE4_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case THIRTYONE:
			{
				MODE4_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			}
		};

	auto MODE5_BG_SEQUENCE = [&](SSTATE32 state)
		{
			switch (state)
			{
			case THREE:
			{
				MODE5_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case SEVEN:
			{
				MODE5_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case ELEVEN:
			{
				MODE5_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case FIFTEEN:
			{
				MODE5_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case NINETEEN:
			{
				MODE5_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYTHREE:
			{
				MODE5_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case TWENTYSEVEN:
			{
				MODE5_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			case THIRTYONE:
			{
				MODE5_B_BG_CYCLE();
				++pGBA_display->currentBgPixel;
				BREAK;
			}
			}
		};

	auto PROCESS_PPU_MODES = [&](INC64 ppuCycles, FLAG renderBG, FLAG renderWindow, FLAG renderObj, FLAG renderMerge)
		{
			if (pGBA_display->currentPPUMode != pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE)
			{
				pGBA_display->ppuModeTransition = YES;
				pGBA_display->currentPPUMode = pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE;
			}
			else
			{
				pGBA_display->ppuModeTransition = NO;
			}

			// Handle mode specific processing
			if (ENABLED)
			{
				switch (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.BG_MODE)
				{
				case MODE0:
				{
					BREAK;
				}
				case MODE1:
				{
					BREAK;
				}
				case MODE2:
				{
					BREAK;
				}
				case MODE3:
				{
					BREAK;
				}
				case MODE4:
				{
					BREAK;
				}
				case MODE5:
				{
					BREAK;
				}
				default:
				{
					PPUWARN("Unknown PPU Mode");
					RETURN;
				}
				}
			}

			// Handle common processing
			if (ENABLED)
			{
				FLAG performWinRenderring = YES;
				FLAG performObjectRenderring = YES;
				FLAG performBgRenderring = YES;
				FLAG performMerging = YES;

				INC64 winCycles = ppuCycles;
				INC64 objCycles = ppuCycles;
				INC64 bgCycles = ppuCycles;
				INC64 mergeCycles = ppuCycles;

				const auto& currentScanline = pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY;
				auto& ppuCounter = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.ppuCounter;
				auto& currentMode = pGBA_display->currentPPUMode;

				ppuCounter += ppuCycles;

				if (pGBA_display->winWaitCyclesDone == NO && renderWindow == YES)
				{
					if (ppuCounter < WIN_WAIT_CYCLES)
					{
						performWinRenderring = NO;
					}
					else
					{
						winCycles = ppuCounter - WIN_WAIT_CYCLES;
						pGBA_display->winWaitCyclesDone = YES;
						performWinRenderring = YES;
					}
				}

				if (pGBA_display->objWaitCyclesDone == NO && renderObj == YES)
				{
					if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.ppuCounter < OBJECT_WAIT_CYCLES)
					{
						performObjectRenderring = (pGBA_display->allObjectsRenderedForScanline == NO) ? YES : NO;
					}
					else
					{
						if (
							pGBA_display->cyclesPerScanline > ZERO
							&&
							pGBA_display->allObjectsRenderedForScanline == NO
							&&
							pGBA_display->objAccessPatternState != OBJECT_ACCESS_PATTERN::OBJECT_A01
							)
						{
							if ((0x80 - pGBA_display->objAccessOAMIDState - ONE) > ZERO)
							{
								PPUMOREINFO("Missed %u objects in mode: %u; scanline %u", 0x80 - pGBA_display->objAccessOAMIDState - ONE, currentMode, pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY);
							}
							else
							{
								PPUMOREINFO("VRAM fsm still in mode: %u; scanline %u", currentMode, pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY);
							}
						}

						pGBA_display->allObjectsRenderedForScanline = NO;

						objCycles = ppuCounter - OBJECT_WAIT_CYCLES;
						pGBA_display->objWaitCyclesDone = YES;
						pGBA_display->objAccessOAMIDState = RESET;
						pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)].reset();
						pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_RENDER_STAGE)].reset();
						pGBA_display->objCache[TO_UINT(OBJECT_STAGE::OBJECT_FETCH_STAGE)].vcount = (SCOUNTER32)((currentScanline + ONE) % 228);
						pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
						pGBA_display->objAccessPattern = RESET;
						pGBA_display->cyclesPerSprite = RESET;
						pGBA_display->cyclesPerScanline = RESET;
						pGBA_display->vramCyclesStageForObjFSM = RESET;
						SET_INITIAL_OBJ_MODE();
						performObjectRenderring = YES;
					}
				}

				if (pGBA_display->bgWaitCyclesDone == NO && renderBG == YES)
				{
					COUNTER32 offsetForTextMode = ZERO;
					if (currentMode == MODE0 || currentMode == MODE1)
					{
						/*
						* Refer https://nba-emu.github.io/hw-docs/ppu/background.html
						* According to https://nba-emu.github.io/hw-docs/ppu/background.html#2, every BG waits 31 - 4 * (BG[x]HOFS mod 8) cycles in Mode 0 \n\
						* To simply the implementation, we are just going to assume the minimum amount of time, i.e. 31 - 4 * (max(BG[x]HOFS mod 8))
						* so, 32 - 4 * (7%8) = 32 - 4 * (7) = 35 - 28 = 4
						* Therefore offsetForTextMode = 28
						*/
						PPUTODO("Simplified version of bg wait cycles is implemented for text modes");
						offsetForTextMode = TWENTYEIGHT;
					}

					if (ppuCounter < (BG_WAIT_CYCLES - offsetForTextMode))
					{
						performBgRenderring = NO;
					}
					else
					{
						bgCycles = ppuCounter - (BG_WAIT_CYCLES - offsetForTextMode);
						pGBA_display->bgWaitCyclesDone = YES;
						performBgRenderring = YES;
					}
				}

				if (pGBA_display->mergeWaitCyclesDone == NO && renderMerge == YES)
				{
					if (ppuCounter < MERGE_WAIT_CYCLES)
					{
						performMerging = NO;
					}
					else
					{
						mergeCycles = ppuCounter - MERGE_WAIT_CYCLES;
						pGBA_display->mergeWaitCyclesDone = YES;
						performMerging = YES;
					}
				}

				INC64 currentCycles = ZERO;
				INC64 targetCycles = winCycles;

				if (performWinRenderring == YES && renderWindow == YES)
				{
					// Store the current mode and winAccessPatternState in local variables for efficiency
					auto& winState = pGBA_display->winAccessPatternState[currentMode];

					while (currentCycles < targetCycles)
					{
						if (winState == ZERO)
						{
							WIN_CYCLE();
						}

						// Advance the state (modulo 4)
						winState = (winState + ONE) & THREE;

						// next cycle...
						++currentCycles;
					}
				}

				PPUTODO("Optimize below code! We need to use a single while loop at line %d in %s", __LINE__, __FILE__);

				currentCycles = ZERO;
				targetCycles = objCycles;

				if (pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTFields.SCREEN_DISP_OBJ == SET && performObjectRenderring == YES && renderObj == YES)
				{
					while ((currentCycles < targetCycles) && pGBA_display->allObjectsRenderedForScanline == NO)
					{
						++pGBA_display->cyclesPerSprite;
						++pGBA_display->cyclesPerScanline;

						if (pGBA_display->currentObjectIsAffine == OBJECT_TYPE::OBJECT_IS_AFFINE)
						{
							// FSM runs only on even cycles
							if ((pGBA_display->objAccessPattern & ONE) == RESET)
							{
								switch (pGBA_display->objAccessPatternState)
								{
								case OBJECT_ACCESS_PATTERN::OBJECT_A01:
								{
									pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
									if (pGBA_display->oamFoundValidObject == NO)
									{
										if (INCREMENT_OAM_ID() == INVALID)
										{
											pGBA_display->allObjectsRenderedForScanline = YES;
											pGBA_display->objAccessOAMIDState = RESET;
											pGBA_display->objAccessPattern = RESET;
											pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
										}
										pGBA_display->cyclesPerSprite = RESET;
									}
									else
									{
										pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
										pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
									}
									BREAK;
								}
								case OBJECT_ACCESS_PATTERN::OBJECT_A2:
								{
									OBJ_A2_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PA;
									BREAK;
								}
								case OBJECT_ACCESS_PATTERN::OBJECT_PA:
								{
									OBJ_PA_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PB;
									BREAK;
								}
								case OBJECT_ACCESS_PATTERN::OBJECT_PB:
								{
									OBJ_PB_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PC;
									BREAK;
								}
								case OBJECT_ACCESS_PATTERN::OBJECT_PC:
								{
									OBJ_PC_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_PD;
									BREAK;
								}
								case OBJECT_ACCESS_PATTERN::OBJECT_PD:
								{
									OBJ_PD_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_BLANK_A01;
									BREAK;
								}
								case OBJECT_ACCESS_PATTERN::OBJECT_BLANK_A01:
								{
									pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState + ONE);
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_V;
									pGBA_display->firstVRAMCycleForObjFSM = YES;
									BREAK;
								}
								case OBJECT_ACCESS_PATTERN::OBJECT_V:
								{
									OBJ_V_OBJ_CYCLE(pGBA_display->objAccessOAMIDState, pGBA_display->currentObjectIsAffine, pGBA_display->vramCyclesStageForObjFSM);
									++pGBA_display->vramCyclesStageForObjFSM;
									if (pGBA_display->lastVRAMCycleForObjFSM == YES)
									{
										pGBA_display->lastVRAMCycleForObjFSM = NO;
										pGBA_display->vramCyclesStageForObjFSM = RESET;
										pGBA_display->cyclesPerSprite = RESET;
										if (INCREMENT_OAM_ID() == INVALID)
										{
											pGBA_display->allObjectsRenderedForScanline = YES;
											pGBA_display->objAccessOAMIDState = RESET;
											pGBA_display->objAccessPattern = RESET;
											pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
										}
										else
										{
											// valid if object is not disabled or we are in correct y coordinate for the object
											if (pGBA_display->oamFoundValidObject == NO)
											{
												if (INCREMENT_OAM_ID() == INVALID)
												{
													pGBA_display->allObjectsRenderedForScanline = YES;
													pGBA_display->objAccessOAMIDState = RESET;
													pGBA_display->objAccessPattern = RESET;
												}
												else
												{
													pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
													if (pGBA_display->oamFoundValidObject == NO)
													{
														if (INCREMENT_OAM_ID() == INVALID)
														{
															pGBA_display->allObjectsRenderedForScanline = YES;
															pGBA_display->objAccessOAMIDState = RESET;
															pGBA_display->objAccessPattern = RESET;
														}
														pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
													}
													else
													{
														pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
														pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
													}
												}
											}
											else
											{
												pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
												pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
											}
										}
									}
									BREAK;
								}
								default:
								{
									FATAL("Unknown Object Access Pattern State (Affine) in PPU Mode %d", currentMode);
								}
								}
							}

							++pGBA_display->objAccessPattern;
						}
						else if (pGBA_display->currentObjectIsAffine == OBJECT_TYPE::OBJECT_IS_NOT_AFFINE)
						{
							// FSM runs only on even cycles
							if ((pGBA_display->objAccessPattern & ONE) == RESET)
							{
								switch (pGBA_display->objAccessPatternState)
								{
								case OBJECT_ACCESS_PATTERN::OBJECT_A01:
								{
									pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
									if (pGBA_display->oamFoundValidObject == NO)
									{
										if (INCREMENT_OAM_ID() == INVALID)
										{
											pGBA_display->allObjectsRenderedForScanline = YES;
											pGBA_display->objAccessOAMIDState = RESET;
											pGBA_display->objAccessPattern = RESET;
											pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
										}
										pGBA_display->cyclesPerSprite = RESET;
									}
									else
									{
										pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
										pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
									}
									BREAK;
								}
								case OBJECT_ACCESS_PATTERN::OBJECT_A2:
								{
									OBJ_A2_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
									pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_V;
									pGBA_display->firstVRAMCycleForObjFSM = YES;
									BREAK;
								}
								case OBJECT_ACCESS_PATTERN::OBJECT_V:
								{
									OBJ_V_OBJ_CYCLE(pGBA_display->objAccessOAMIDState, pGBA_display->currentObjectIsAffine, pGBA_display->vramCyclesStageForObjFSM);
									++pGBA_display->vramCyclesStageForObjFSM;
									if (pGBA_display->firstVRAMCycleForObjFSM == YES)
									{
										pGBA_display->firstVRAMCycleForObjFSM = NO;
										pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState + ONE);
									}
									else if (pGBA_display->lastVRAMCycleForObjFSM == YES)
									{
										pGBA_display->lastVRAMCycleForObjFSM = NO;
										pGBA_display->vramCyclesStageForObjFSM = RESET;
										pGBA_display->cyclesPerSprite = RESET;
										if (INCREMENT_OAM_ID() == INVALID)
										{
											pGBA_display->allObjectsRenderedForScanline = YES;
											pGBA_display->objAccessOAMIDState = RESET;
											pGBA_display->objAccessPattern = RESET;
											pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
										}
										else
										{
											// valid if object is not disabled or we are in correct y coordinate for the object
											if (pGBA_display->oamFoundValidObject == NO)
											{
												if (INCREMENT_OAM_ID() == INVALID)
												{
													pGBA_display->allObjectsRenderedForScanline = YES;
													pGBA_display->objAccessOAMIDState = RESET;
													pGBA_display->objAccessPattern = RESET;
													pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
												}
												else
												{
													pGBA_display->oamFoundValidObject = OBJ_A01_OBJ_CYCLE(pGBA_display->objAccessOAMIDState);
													if (pGBA_display->oamFoundValidObject == NO)
													{
														if (INCREMENT_OAM_ID() == INVALID)
														{
															pGBA_display->allObjectsRenderedForScanline = YES;
															pGBA_display->objAccessOAMIDState = RESET;
															pGBA_display->objAccessPattern = RESET;
														}
														pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A01;
													}
													else
													{
														pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
														pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
													}
												}
											}
											else
											{
												pGBA_display->objAccessPatternState = OBJECT_ACCESS_PATTERN::OBJECT_A2;
												pGBA_display->currentObjectIsAffine = pGBA_display->nextObjectIsAffine;
											}
										}
									}
									BREAK;
								}
								default:
								{
									FATAL("Unknown Object Access Pattern State (Not Affine) in PPU Mode %d", currentMode);
								}
								}
							}

							++pGBA_display->objAccessPattern;
						}

						// next cycle...
						++currentCycles;
					}
				}

				currentCycles = ZERO;
				targetCycles = bgCycles;

				if (performBgRenderring == YES && renderBG == YES)
				{
#if (ENABLED)
					// Cache the state once before the loop to avoid repeated array access
					auto& bgState = pGBA_display->bgAccessPatternState[currentMode];

					switch (currentMode)
					{
					case MODE0:
					{
						while (currentCycles < targetCycles)
						{
							MODE0_BG_SEQUENCE(bgState);

							bgState = (bgState + ONE) & THIRTYONE;

							// next cycle...
							++currentCycles;
						}
						BREAK;
					}
					case MODE1:
					{
						while (currentCycles < targetCycles)
						{
							MODE1_BG_SEQUENCE(bgState);

							bgState = (bgState + ONE) & THIRTYONE;

							// next cycle...
							++currentCycles;
						}
						BREAK;
					}
					case MODE2:
					{
						while (currentCycles < targetCycles)
						{
							MODE2_BG_SEQUENCE(bgState);

							bgState = (bgState + ONE) & THIRTYONE;

							// next cycle...
							++currentCycles;
						}
						BREAK;
					}
					case MODE3:
					{
						while (currentCycles < targetCycles)
						{
							MODE3_BG_SEQUENCE(bgState);

							bgState = (bgState + ONE) & THIRTYONE;

							// next cycle...
							++currentCycles;
						}
						BREAK;
					}
					case MODE4:
					{
						while (currentCycles < targetCycles)
						{
							MODE4_BG_SEQUENCE(bgState);

							bgState = (bgState + ONE) & THIRTYONE;

							// next cycle...
							++currentCycles;
						}
						BREAK;
					}
					case MODE5:
					{
						while (currentCycles < targetCycles)
						{
							MODE5_BG_SEQUENCE(bgState);

							bgState = (bgState + ONE) & THIRTYONE;

							// next cycle...
							++currentCycles;
						}
						BREAK;
					}
					default:
					{
						FATAL("Unknown BG mode");
					}
					}
#else
					while (currentCycles < targetCycles)
					{
						switch (pGBA_display->bgAccessPatternState[currentMode])
						{
						case ZERO:
						{
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
							BREAK;
						}
						case ONE:
						{
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
							BREAK;
						}
						case TWO:
						{
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
							CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
							BREAK;
						}
						case THREE:
						{
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
							CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
							++pGBA_display->currentBgPixel;
							BREAK;
						}
						case FOUR:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
							BREAK;
						}
						case FIVE:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
							BREAK;
						}
						case SIX:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
							BREAK;
						}
						case SEVEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
							CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
							++pGBA_display->currentBgPixel;
							BREAK;
						}
						case EIGHT:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
							BREAK;
						}
						case NINE:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
							BREAK;
						}
						case TEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
							CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
							BREAK;
						}
						case ELEVEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
							CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
							++pGBA_display->currentBgPixel;
							BREAK;
						}
						case TWELVE:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
							BREAK;
						}
						case THIRTEEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
							BREAK;
						}
						case FOURTEEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
							BREAK;
						}
						case FIFTEEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
							CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
							++pGBA_display->currentBgPixel;
							BREAK;
						}
						case SIXTEEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
							BREAK;
						}
						case SEVENTEEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
							BREAK;
						}
						case EIGHTEEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
							CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
							BREAK;
						}
						case NINETEEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
							CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
							++pGBA_display->currentBgPixel;
							BREAK;
						}
						case TWENTY:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
							BREAK;
						}
						case TWENTYONE:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
							BREAK;
						}
						case TWENTYTWO:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
							BREAK;
						}
						case TWENTYTHREE:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
							CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
							++pGBA_display->currentBgPixel;
							BREAK;
						}
						case TWENTYFOUR:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
#endif
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
							BREAK;
						}
						case TWENTYFIVE:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
#endif
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
							BREAK;
						}
						case TWENTYSIX:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
#endif
							CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
							BREAK;
						}
						case TWENTYSEVEN:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
#endif
							CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
							++pGBA_display->currentBgPixel;
							BREAK;
						}
						case TWENTYEIGHT:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG0));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG0));
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG3));
							BREAK;
						}
						case TWENTYNINE:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_M_TEXT_BG_CYCLE(BG1));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE1, MODE1_T_TEXT_BG_CYCLE(BG1));
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG3));
							BREAK;
						}
						case THIRTY:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG2));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE1, MODE1_M_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_M_BG_CYCLE(BG2));
							BREAK;
						}
						case THIRTYONE:
						{
#if (GBA_ENABLE_CYCLE_ACCURATE_PPU_ACCESS_PATTERN == NO)
							CONDITIONAL(currentMode == MODE0, MODE0_M_BG_CYCLE(BG3));
#endif
							CONDITIONAL(currentMode == MODE0, MODE0_T_BG_CYCLE(BG3));
							CONDITIONAL(currentMode == MODE1, MODE1_T_AFFINE_BG_CYCLE());
							CONDITIONAL(currentMode == MODE2, MODE2_T_BG_CYCLE(BG2));
							CONDITIONAL(currentMode == MODE3, MODE3_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE4, MODE4_B_BG_CYCLE());
							CONDITIONAL(currentMode == MODE5, MODE5_B_BG_CYCLE());
							++pGBA_display->currentBgPixel;
							BREAK;
						}
						default:
						{
							FATAL("PPU mode %d (BG) out of sync", currentMode);
						}
						}

						++pGBA_display->bgAccessPatternState[currentMode];
						if (pGBA_display->bgAccessPatternState[currentMode] >= THIRTYTWO)
						{
							pGBA_display->bgAccessPatternState[currentMode] = ZERO;
						}

						// next cycle...
						++currentCycles;
					}
#endif
				}

				currentCycles = ZERO;
				targetCycles = mergeCycles;

				if (performMerging == YES && renderMerge == YES)
				{
					// Cache the state once before the loop to avoid repeated array access
					auto& mergeState = pGBA_display->mergeAccessPatternState[currentMode];

					while (currentCycles < targetCycles)
					{
						if (mergeState == ZERO)
						{
							MERGE_AND_DISPLAY_PHASE1();
						}
						else if (mergeState == TWO)
						{
							MERGE_AND_DISPLAY_PHASE2();
						}

						// Move to the next state and wrap it at 4
						mergeState = (mergeState + ONE) & THREE;

						// next cycle...
						++currentCycles;
					}
				}

			}
		};

	auto HANDLE_VCOUNT = [&]()
		{
			if (pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_SETTING_LYC == pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY)
			{
				if (
					pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNTER_IRQ_ENABLE
					&&
					pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_FLAG == RESET
					)
				{
					requestInterrupts(GBA_INTERRUPT::IRQ_VCOUNT);
				}

				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_FLAG = SET;
			}
			else
			{
				pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VCOUNT_FLAG = RESET;
			}
		};
#endif

	pGBA_display->wasVblankJustTriggered = NO;

	pGBA_display->didLCDModeChangeJustNow = NO;

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter += ppuCycles;

	switch (pGBA_display->currentLCDMode)
	{
	case LCD_MODES::MODE_LCD_H_DRAW_V_DRAW: // H Draw V Draw
	{
		PROCESS_PPU_MODES(ppuCycles + pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange, YES, YES, YES, YES);
		pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = RESET;

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter >= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_VISIBLE)
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter -= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_VISIBLE;

			// Note: In process PPU, we should account for cycles which occur only in MODE_LCD_H_DRAW_V_DRAW
			ppuCycles -= pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter;

			pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_FLAG = SET;

			if (pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_IRQ_ENABLE == SET)
			{
				requestInterrupts(GBA_INTERRUPT::IRQ_HBLANK);
			}

			scheduleDMATrigger(DMA_TIMING::HBLANK);

			// Proceed to next lcd mode
			pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter;
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

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter >= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_BLANK)
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter -= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_BLANK;

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
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.ppuCounter = RESET;
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

				scheduleDMATrigger(DMA_TIMING::VBLANK);

				// Refer http://problemkaputt.de/gbatek-gba-dma-transfers.htm
				if (pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY >= TWO)
				{
					/*
					* https://gbadev.net/tonc/dma.html
					* For DMA1 and DMA2 it'll refill the FIFO when it has been emptied.
					* Count and size are forced to 1 and 32bit, respectively.
					* For DMA3 it will start the copy at the start of each rendering line, but with a 2 scanline delay.
					*/
					scheduleDMATrigger(DMA_TIMING::SPECIAL);
					scheduleSpecialDMATrigger(DMA_SPECIAL_TIMING::VIDEO);
				}

				if (pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.VBLANK_IRQ_ENABLE == SET)
				{
					requestInterrupts(GBA_INTERRUPT::IRQ_VBLANK);
				}

				// Proceed to next lcd mode
				pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter;
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
					* For DMA1 and DMA2 it'll refill the FIFO when it has been emptied.
					* Count and size are forced to 1 and 32bit, respectively.
					* For DMA3 it will start the copy at the start of each rendering line, but with a 2 scanline delay.
					*/
					scheduleDMATrigger(DMA_TIMING::SPECIAL);
					scheduleSpecialDMATrigger(DMA_SPECIAL_TIMING::VIDEO);
				}

				// Proceed to next lcd mode
				pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter;
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

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter >= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_VISIBLE)
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter -= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_VISIBLE;

			// Note: In process PPU, we should account for cycles which occur only in MODE_LCD_H_DRAW_V_BLANK
			ppuCycles -= pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter;

			pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_FLAG = SET;

			// According to http://problemkaputt.de/gbatek-lcd-dimensions-and-timings.htm, we should not...but this is needed for tonc's irq_demo.gba
			PPUTODO("Should we raise H-BLANK interrupt when we are in V-BLANK ?");
			if (pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATFields.HBLANK_IRQ_ENABLE == SET)
			{
				requestInterrupts(GBA_INTERRUPT::IRQ_HBLANK);
			}

			// Proceed to the next lcd mode
			pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter;
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

		if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter >= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_BLANK)
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter -= (uint32_t)LCD_MODE_CYCLES::CYCLES_LCD_H_BLANK;

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
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.ppuCounter = RESET;
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
				pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter;
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
					* For DMA1 and DMA2 it'll refill the FIFO when it has been emptied.
					* Count and size are forced to 1 and 32bit, respectively.
					* For DMA3 it will start the copy at the start of each rendering line, but with a 2 scanline delay.
					*/
					scheduleDMATrigger(DMA_TIMING::SPECIAL);
					scheduleSpecialDMATrigger(DMA_SPECIAL_TIMING::VIDEO);
				}
				else if (pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTFields.CURRENT_SCANLINE_LY >= 162)
				{
					// Need to stop scheduling the Video Capture DMA (DMA 3) 
					cancelScheduledDMATrigger(DMA_TIMING::SPECIAL);
					cancelScheduledSpecialDMATrigger(DMA_SPECIAL_TIMING::VIDEO);

					// Disable DMA 3 if it was enabled and in special mode
					TODO("Does GBA stop DMA3 in special mode by directly disabling the ENABLE bit in mDMA3CNT_H ?");
					if ((((DMA_TIMING)pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_START_TIMING) == DMA_TIMING::SPECIAL)
						&&
						(pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN == SET)
						)
					{
						pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN = RESET;
					}
				}

				// Proceed to the next lcd mode
				pGBA_display->extraPPUCyclesForProcessPPUModesDuringModeChange = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter;
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

void GBA_t::processSIO(INC64 sioCycles)
{
	// Retrieve the mode select value once, instead of repeatedly accessing memory
	ID modeSelect = (
		(pGBA_peripherals->mRCNTHalfWord.mRCNTFields.MODE_SPECIFIC_3 << THREE)
		| (pGBA_peripherals->mRCNTHalfWord.mRCNTFields.MODE_SPECIFIC_2 << TWO)
		| (pGBA_peripherals->mSIOCNT.mSIOFields.BIT_13 << ONE)
		| pGBA_peripherals->mSIOCNT.mSIOFields.BIT_12
		);

	// Determine SIO mode
	SIO_MODE sioMode = SIO_MODE::NORMAL_8BIT;
	switch (modeSelect)
	{
	case ZERO: case FOUR: sioMode = SIO_MODE::NORMAL_8BIT; BREAK;
	case ONE: case FIVE: sioMode = SIO_MODE::NORMAL_32BIT; BREAK;
	case TWO: case SIX: sioMode = SIO_MODE::MULTIPLAY_16BIT; BREAK;
	case THREE: case SEVEN: sioMode = SIO_MODE::UART; BREAK;
	case EIGHT: case NINE: case TEN: case ELEVEN: sioMode = SIO_MODE::GP; BREAK;
	case TWELVE: case THIRTEEN: case FOURTEEN: case FIFTEEN: sioMode = SIO_MODE::JOYBUS; BREAK;
	default: FATAL("Unsupported SIO Mode");
	}

	// Initialize variables
	FLAG interruptRequested = NO;
	INC32 cyclesToTxRxData = RESET;

	// Handle different SIO modes
	switch (sioMode)
	{
	case SIO_MODE::NORMAL_8BIT:
	case SIO_MODE::NORMAL_32BIT:
	{
		// Process based on START_BIT and SHIFT_CLK
		auto& sioCnt = pGBA_peripherals->mSIOCNT.mSIOCNT_SPFields;
		if (sioCnt.START_BIT == SET)
		{
			if (static_cast<SIO_PARTY>(sioCnt.SHIFT_CLK) == SIO_PARTY::MASTER)
			{
				pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.sioCounter += sioCycles;
			}

			TODO("For now, we will fix the cyclesToTxRxData");
			// Set cycles to transfer data
			cyclesToTxRxData = (sioMode == SIO_MODE::NORMAL_8BIT) ? (EIGHT * EIGHT) : (EIGHT * THIRTYTWO);

			if (pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.sioCounter >= cyclesToTxRxData)
			{
				pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.sioCounter -= cyclesToTxRxData;

				// Raise interrupt if enabled
				interruptRequested = (sioCnt.IRQ_EN == SET);
				if (interruptRequested)
				{
					requestInterrupts(GBA_INTERRUPT::IRQ_SERIAL);
				}

				// Reset START_BIT
				sioCnt.START_BIT = RESET;
			}
		}
		else
		{
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.sioCounter = RESET;
		}

		BREAK;
	}
	case SIO_MODE::MULTIPLAY_16BIT:
	{
		// Check START_BUSY_BIT for MULTIPLAY_16BIT mode
		if (pGBA_peripherals->mSIOCNT.mSIOCNT_MPFields.START_BUSY_BIT == SET)
		{
			interruptRequested = (pGBA_peripherals->mSIOCNT.mSIOCNT_MPFields.IRQ_EN == SET);
		}

		BREAK;
	}
	case SIO_MODE::UART:
	{
		// Handle for UART mode
		interruptRequested = (pGBA_peripherals->mSIOCNT.mSCCNTFields.IRQ_EN == SET);
		BREAK;
	}
	case SIO_MODE::GP:
	{
		BREAK;
	}
	case SIO_MODE::JOYBUS:
	{
		// Handle for JoyBus mode
		interruptRequested = (pGBA_peripherals->mJOYCNTHalfWord.mJOYCNTFields.DEV_RESET == SET);
		BREAK;
	}
	default:
	{
		FATAL("Unknown SIO Mode");
	}
	}
}

void GBA_t::processBackup()
{
	// Refer : https://dillonbeliveau.com/2020/06/05/GBA-FLASH.html
	// Refer : http://problemkaputt.de/gbatek-gba-cart-backup-flash-rom.htm

	switch (pGBA_instance->GBA_state.emulatorStatus.backup.backupType)
	{
	case BACKUP_TYPE::FLASH64K:
	case BACKUP_TYPE::FLASH128K:
	{
		ID currentMemoryBank = RESET;
		if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentMemoryBank == BACKUP_FLASH_MEMORY_BANK::BANK1)
		{
			currentMemoryBank = ONE;
		}

		switch (pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState)
		{
		case BACKUP_FLASH_FSM::STATE0:
		{
			if (pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY2 - GAMEPAK_SRAM_START_ADDRESS]
				== TO_UINT8(BACKUP_FLASH_CMDS::CMD_1))
			{
				pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState = BACKUP_FLASH_FSM::STATE1;
				pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY2 - GAMEPAK_SRAM_START_ADDRESS] = RESET;
			}
			BREAK;
		}
		case BACKUP_FLASH_FSM::STATE1:
		{
			if (pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY3 - GAMEPAK_SRAM_START_ADDRESS]
				== TO_UINT8(BACKUP_FLASH_CMDS::CMD_2))
			{
				pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState = BACKUP_FLASH_FSM::STATE2;
				pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY3 - GAMEPAK_SRAM_START_ADDRESS] = RESET;
			}
			BREAK;
		}
		case BACKUP_FLASH_FSM::STATE2:
		{
			// Handle the 4KB erase
			if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.previousFlashCommand
				== BACKUP_FLASH_CMDS::START_ERASE_CMD
				&&
				pGBA_instance->GBA_state.emulatorStatus.backup.flash.erase4kbPageNumber != INVALID)
			{
				pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

				pGBA_instance->GBA_state.emulatorStatus.backup.flash.erase4kbPageNumber = INVALID;

				GBA_WORD eraseStartAddr = pGBA_instance->GBA_state.emulatorStatus.backup.flash.erase4kbStartAddr;
				pGBA_instance->GBA_state.emulatorStatus.backup.flash.erase4kbStartAddr = RESET;

				if (eraseStartAddr >= GAMEPAK_SRAM_START_ADDRESS + 0x10000)
				{
					FATAL("Invalid address for 4kb sector erase");
				}

				memset(
					&(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][eraseStartAddr - GAMEPAK_SRAM_START_ADDRESS])
					, 0xFF
					, 0x1000
				);

				memset(
					&(pGBA_instance->GBA_state.emulatorStatus.backup.flash.isErased[currentMemoryBank][eraseStartAddr - GAMEPAK_SRAM_START_ADDRESS])
					, YES
					, 0x1000
				);
			}
			else
			{
				pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentFlashCommand =
					(BACKUP_FLASH_CMDS)(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY2 - GAMEPAK_SRAM_START_ADDRESS]);

				pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY2 - GAMEPAK_SRAM_START_ADDRESS] = RESET;

				switch (pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentFlashCommand)
				{
				case BACKUP_FLASH_CMDS::NO_OPERATION:
				{
					BREAK;
				}
				case BACKUP_FLASH_CMDS::ENTER_CHIP_INDENTIFICATION_MODE:
				{
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

					if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.previousFlashCommand
						== BACKUP_FLASH_CMDS::START_ERASE_CMD)
					{
						BREAK;
					}

					pGBA_instance->GBA_state.emulatorStatus.backup.flash.ogByteAtFlashAccessMem0 =
						pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY0 - GAMEPAK_SRAM_START_ADDRESS];
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.ogByteAtFlashAccessMem1 =
						pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY1 - GAMEPAK_SRAM_START_ADDRESS];

					if (pGBA_instance->GBA_state.emulatorStatus.backup.backupType == BACKUP_TYPE::FLASH64K)
					{
						pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][ZERO] = 0x32; // Panasonic manufacturer ID
						pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][ONE] = 0x1B; // Panasonic device ID
					}
					else if (pGBA_instance->GBA_state.emulatorStatus.backup.backupType == BACKUP_TYPE::FLASH128K)
					{
						pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][ZERO] = 0x62; // Sanyo manufacturer ID
						pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][ONE] = 0x13; // Sanyo device ID
					}

					BREAK;
				}
				case BACKUP_FLASH_CMDS::EXIT_CHIP_INDENTIFICATION_MODE:
				{
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

					if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.previousFlashCommand
						== BACKUP_FLASH_CMDS::START_ERASE_CMD)
					{
						BREAK;
					}

					pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY0 - GAMEPAK_SRAM_START_ADDRESS]
						= pGBA_instance->GBA_state.emulatorStatus.backup.flash.ogByteAtFlashAccessMem0;
					pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][FLASH_ACCESS_MEMORY1 - GAMEPAK_SRAM_START_ADDRESS] =
						pGBA_instance->GBA_state.emulatorStatus.backup.flash.ogByteAtFlashAccessMem1;

					BREAK;
				}
				case BACKUP_FLASH_CMDS::START_ERASE_CMD:
				{
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

					if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.previousFlashCommand
						== BACKUP_FLASH_CMDS::START_ERASE_CMD)
					{
						BREAK;
					}

					BREAK;
				}
				case BACKUP_FLASH_CMDS::ERASE_ENTIRE_CHIP:
				{
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

					if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.previousFlashCommand
						== BACKUP_FLASH_CMDS::START_ERASE_CMD)
					{
						if (pGBA_instance->GBA_state.emulatorStatus.backup.backupType == BACKUP_TYPE::FLASH64K)
						{
							memset(
								pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank]
								, 0xFF
								, 0x10000
							);

							memset(
								pGBA_instance->GBA_state.emulatorStatus.backup.flash.isErased[currentMemoryBank]
								, YES
								, 0x10000
							);
						}
						else if (pGBA_instance->GBA_state.emulatorStatus.backup.backupType == BACKUP_TYPE::FLASH128K)
						{
							if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentMemoryBank == BACKUP_FLASH_MEMORY_BANK::BANK0)
							{
								memset(
									pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank]
									, 0xFF
									, 0x10000
								);

								memset(
									pGBA_instance->GBA_state.emulatorStatus.backup.flash.isErased[currentMemoryBank]
									, YES
									, 0x10000
								);
							}
							else
							{
								memset(
									&(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[currentMemoryBank][0x10000])
									, 0xFF
									, 0x10000
								);

								memset(
									&(pGBA_instance->GBA_state.emulatorStatus.backup.flash.isErased[currentMemoryBank][0x10000])
									, YES
									, 0x10000
								);
							}
						}
					}

					BREAK;
				}
				case BACKUP_FLASH_CMDS::START_1BYTE_WRITE_CMD:
				{
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

					pGBA_instance->GBA_state.emulatorStatus.backup.flash.allowFlashWrite = YES;

					// Refer : http://problemkaputt.de/gbatek-gba-cart-backup-flash-rom.htm
					TODO("Need to use the \"allowFlashWrite\" flag to decide whether we should allow flash write");
					TODO("Need to use the \"isErased\" flag to decide whether we should allow flash write");

					BREAK;
				}
				case BACKUP_FLASH_CMDS::SET_MEMORY_BANK:
				{
					pGBA_instance->GBA_state.emulatorStatus.backup.flash.flashFsmState = BACKUP_FLASH_FSM::STATE0;

					pGBA_instance->GBA_state.emulatorStatus.backup.flash.chooseMemoryBank = YES;

					BREAK;
				}

				default:
				{
					FATAL("Unknown flash command");
				}
				}
			}

			if (pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentFlashCommand != BACKUP_FLASH_CMDS::NO_OPERATION)
			{
				pGBA_instance->GBA_state.emulatorStatus.backup.flash.previousFlashCommand
					= pGBA_instance->GBA_state.emulatorStatus.backup.flash.currentFlashCommand;
			}

			BREAK;
		}
		default:
		{
			FATAL("Unknown Flash State");
		}
		}
		BREAK;
	}
	default:
	{
		WARN("This backup type is not implemented yet");
	}
	}
}

#if (DEACTIVATED)
void GBA_t::dumpIO()
{
	static uint64_t ioDumpCounter = 0;
	if (ioDumpCounter == 1000)
	{
		std::ofstream fs("ioDump.txt");
		if (!fs)
		{
			std::cerr << "Cannot open the output file." << std::endl;
			RETURN;
		}
		auto addr = 0x04000000;
		for (int ii = 0; ii < (sizeof(pGBA_memory->mGBAMemoryMap.mIO.mIOMemory16bit) / 2); ii++)
		{
			fs << std::hex << std::setfill('0') << addr << " = " << std::setw(4) << pGBA_memory->mGBAMemoryMap.mIO.mIOMemory16bit[ii] << std::endl;
			addr += 2;
		}
		fs.close();
	}
	++ioDumpCounter;
}
#endif

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

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.ppuCounter = RESET;
	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.lcdCounter = RESET;

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

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_count_accurate.apuCounter = RESET;

	// Refer : http://problemkaputt.de/gbatek-gba-sound-control-registers.htm
	pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASHalfWord = 0x200;

	// Setup the volume for audio
	pGBA_audio->emulatorVolume = pt.get<std::float_t>("gba._volume");
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), pGBA_audio->emulatorVolume);
}

// TODO: Below function is not used for now...
void GBA_t::reInitializeAudio()
{
	if (_ENABLE_AUDIO == YES)
	{
		;
	}
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

	pGBA_instance->GBA_state.dma.currentlyActiveDMA = DMA::NONE;

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

#if (DEACTIVATED)
void GBA_t::runDebugger()
{

}
#endif
#pragma endregion EMULATION_DEFINITIONS