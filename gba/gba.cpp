#pragma region GBA_SPECIFIC_INCLUDES
#include "gba.h"
#include "gba.inl"
#include "gba_opcodes.inl"
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
COUNTER64 gbaEmulationCounter[100] = { ZERO };

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
uint32_t gameboyAdvance_texture;
uint32_t gameboyAdvance_matrix_texture;
static uint32_t gameboyAdvance_matrix[16] = { 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
#pragma endregion GBA_SPECIFIC_DECLARATIONS

#pragma region INFRASTRUCTURE_DEFINITIONS
GBA_t::GBA_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, MasqConfig_t& config, CheatEngine_t* ce)
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
		//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_WARN);
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_EVENT);

		INFO("Running in sst Cpu Test Mode!");
		_JSON_LOCATION = rom[ONE];

#if (ENABLE_ARM7TDMI_SST == YES)
		ROM_TYPE = ROM::TEST_SST;
#else
		FATAL("SSTs are not supported in this build");
		RETURN;
#endif
	}
	else if (nFiles == COMPARE_OR_REPLAY_ROM_FILE)
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

	this->ceGBA = ce;

	this->ceGBA->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, EMULATION_ID::GBA_ID);

#ifndef __EMSCRIPTEN__
	_SAVE_LOCATION = pt.get<std::string>("gba._save_location", "");
	if (_SAVE_LOCATION.empty())
	{
		FATAL("Could not locate the save directory");
	}
#else
	_SAVE_LOCATION = "assets/saves";
#endif
	// check if directory mentioned by "_SAVE_LOCATION" exists, if not we need to explicitly create it
	ifNoDirectoryThenCreate(_SAVE_LOCATION);
	_LOAD_GBA_BIOS = to_bool(config.get<std::string>("gba._load_gba_bios", _LOAD_GBA_BIOS ? "true" : "false"));
	_ENABLE_GBA_BIOS = to_bool(config.get<std::string>("gba._use_gba_bios", _ENABLE_GBA_BIOS ? "true" : "false"));

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

void GBA_t::setupTheCoreOfEmulation(void* masqueradeInstance, void* audio, void* input, void* network)
{
	uint8_t indexToCheck = 0;

#if (ENABLE_ARM7TDMI_SST == YES)
	if (!rom[indexToCheck].empty() || ROM_TYPE == ROM::TEST_SST)
#else
	if (!rom[indexToCheck].empty())
#endif
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
OPT_SIZE

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

		if (gbaEmulationCounter[ZERO] == 100)
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

OPT_DEFAULT
#pragma endregion ARM7TDMI_DEFINITIONS

#pragma region EMULATION_DEFINITIONS
OPT_SPEED

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
		{
			CONTINUE;
		}

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

void GBA_t::processDMA()
{
	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter = RESET;

	if (IsAnyDMARunning() == NO)
	{
		RETURN;
	}

	cpuTick(TICK_TYPE::DMA_TICK);

	// Handles the loop for all DMA IDs
	do
	{
		RunDMAChannel();
	}
	while (IsAnyDMARunning());

	cpuTick(TICK_TYPE::DMA_TICK);
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
						DelayedDMAActivate(DMA::DMA3);
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
						DelayedDMAActivate(DMA::DMA3);
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
						DelayedDMAActivate(DMA::DMA3);
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

OPT_DEFAULT

OPT_SIZE

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
	pGBA_audio->emulatorVolume = pt.get<std::float_t>("gba._volume", 0.1f);
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

#if (ENABLE_ARM7TDMI_SST == YES)
	if (ROM_TYPE == ROM::TEST_SST) MASQ_UNLIKELY
	{
		static FLAG SST_DEBUG_PRINT = NO;

	// ------------------------------------------------------------
	// Move large arrays to heap with a struct
	// ------------------------------------------------------------
	struct TestState {
		uint32_t R[16] = {};
		uint32_t R_fiq[7] = {};
		uint32_t R_svc[2] = {};
		uint32_t R_abt[2] = {};
		uint32_t R_irq[2] = {};
		uint32_t R_und[2] = {};
		uint32_t spsr[5] = {};
		uint32_t pipeline[2] = {};
		uint32_t cpsr = 0;
		uint32_t access = 0;
	};

	// ------------------------------------------------------------
	// Enumerate all JSON files
	// ------------------------------------------------------------
	std::vector<std::string> testFiles;
	try
	{
		std::filesystem::path testDir(_JSON_LOCATION);
		for (const auto& entry : std::filesystem::directory_iterator(testDir))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".json")
			{
				testFiles.push_back(entry.path().filename().string());
			}
		}
		std::sort(testFiles.begin(), testFiles.end());
	}
	catch (const std::exception& e)
	{
		FATAL("Failed to enumerate test files: %s", e.what());
		RETURN false;
	}

	INFO("Found %zu GBA SST test files", testFiles.size());

	// ------------------------------------------------------------
	// Helper: read JSON array into C array
	// ------------------------------------------------------------
	auto readArray = [](const rapidjson::Value& parent,
		const char* key,
		uint32_t* out,
		size_t count)
		{
			if (!parent.HasMember(key) || !parent[key].IsArray()) return;

			const rapidjson::Value& arr = parent[key];
			size_t i = 0;
			for (rapidjson::SizeType idx = 0; idx < arr.Size() && i < count; ++idx)
			{
				out[i++] = arr[idx].GetUint();
			}
		};

	COUNTER32 testSetIndex = 0;
	static COUNTER32 DEBUG_START_TEST_SET = ZERO;

	std::vector<ARM7TDMI_SST_t::Transaction> transactions;
	transactions.reserve(100);

	// ------------------------------------------------------------
	// Iterate test files
	// ------------------------------------------------------------
	for (const auto& testFileName : testFiles)
	{
		std::filesystem::path fullPath = std::filesystem::path(_JSON_LOCATION) / testFileName;

		testSetIndex++;

		INFO("================================================================================");
		if ((std::strcmp(testFileName.c_str(), "arm_cdp.json") == 0)
			|| (std::strcmp(testFileName.c_str(), "arm_mcr_mrc.json") == 0)
			|| (std::strcmp(testFileName.c_str(), "arm_stc_ldc.json") == 0))
		{
			INFO("Skipping: %s", testFileName.c_str());
			CONTINUE;
		}
		else
		{
			INFO("Loading: %s", testFileName.c_str());
		}
		INFO("================================================================================");

		if (testSetIndex < DEBUG_START_TEST_SET)
			CONTINUE;

		{
			// Read entire file into string
			std::ifstream ifs(fullPath);
			if (!ifs.is_open())
			{
				WARN("Failed to open %s", testFileName.c_str());
				CONTINUE;
			}

			std::string jsonStr((std::istreambuf_iterator<char>(ifs)),
				std::istreambuf_iterator<char>());
			ifs.close();

			// Parse from string
			rapidjson::Document testCase;
			testCase.Parse(jsonStr.c_str());

			if (testCase.HasParseError())
			{
				WARN("Failed to parse %s: error code %u at offset %zu",
					testFileName.c_str(),
					(unsigned)testCase.GetParseError(),
					testCase.GetErrorOffset());
				CONTINUE;
			}

			if (!testCase.IsArray())
			{
				WARN("%s does not contain a JSON array", testFileName.c_str());
				CONTINUE;
			}

			COUNTER32 passedCount = 0;
			COUNTER32 failedCount = 0;
			COUNTER32 testIndex = 0;
			static COUNTER32 DEBUG_START_TEST = 0;

			// --------------------------------------------------------
			// Iterate each JSON entry
			// --------------------------------------------------------
			for (rapidjson::SizeType itemIdx = 0; itemIdx < testCase.Size(); ++itemIdx)
			{
				const rapidjson::Value& item = testCase[itemIdx];

				// ======== ALLOCATE ON HEAP ========
				auto initial = std::make_unique<TestState>();
				auto final = std::make_unique<TestState>();

				// Reset state
				sst.SST_Reset();
				for (int i = 0; i < 8; ++i) pGBA_registers->unbankedLORegisters[i] = 0;
				pGBA_registers->cpsr.psrMemory = 0;
				for (uint8_t rb = RB_USR_SYS; rb < RB_TOTAL; ++rb)
				{
					for (int i = 0; i < 7; ++i)
						pGBA_registers->bankedHIRegisters[rb][i] = 0;

					pGBA_registers->spsr[rb].psrMemory = 0;
				}
				pGBA_registers->pc = 0;

				if (testIndex++ < DEBUG_START_TEST)
					CONTINUE;

				FLAG quitThisRun = NO;

				// ================= INITIAL =================
				if (!item.HasMember("initial") || !item["initial"].IsObject())
				{
					WARN("Test %u missing 'initial' object", testIndex - 1);
					CONTINUE;
				}

				const rapidjson::Value& initialJson = item["initial"];

				readArray(initialJson, "R", initial->R, 16);
				readArray(initialJson, "R_fiq", initial->R_fiq, 7);
				readArray(initialJson, "R_svc", initial->R_svc, 2);
				readArray(initialJson, "R_abt", initial->R_abt, 2);
				readArray(initialJson, "R_irq", initial->R_irq, 2);
				readArray(initialJson, "R_und", initial->R_und, 2);
				readArray(initialJson, "SPSR", initial->spsr, 5);
				readArray(initialJson, "pipeline", initial->pipeline, 2);

				initial->cpsr = initialJson.HasMember("CPSR") ? initialJson["CPSR"].GetUint() : 0;
				initial->access = initialJson.HasMember("access") ? initialJson["access"].GetUint() : 0;

				// ================= FINAL =================
				if (!item.HasMember("final") || !item["final"].IsObject())
				{
					WARN("Test %u missing 'final' object", testIndex - 1);
					CONTINUE;
				}

				const rapidjson::Value& finalJson = item["final"];

				readArray(finalJson, "R", final->R, 16);
				readArray(finalJson, "R_fiq", final->R_fiq, 7);
				readArray(finalJson, "R_svc", final->R_svc, 2);
				readArray(finalJson, "R_abt", final->R_abt, 2);
				readArray(finalJson, "R_irq", final->R_irq, 2);
				readArray(finalJson, "R_und", final->R_und, 2);
				readArray(finalJson, "SPSR", final->spsr, 5);
				readArray(finalJson, "pipeline", final->pipeline, 2);

				final->cpsr = finalJson.HasMember("CPSR") ? finalJson["CPSR"].GetUint() : 0;
				final->access = finalJson.HasMember("access") ? finalJson["access"].GetUint() : 0;

				// ================= OPCODE =================
				uint32_t opcode = item.HasMember("opcode") ? item["opcode"].GetUint() : 0;
				uint32_t baseAddr = item.HasMember("base_addr") ? item["base_addr"].GetUint() : 0;

				// ================= TRANSACTIONS =================
				transactions.clear();
				if (item.HasMember("transactions") && item["transactions"].IsArray())
				{
					const rapidjson::Value& txArray = item["transactions"];
					for (rapidjson::SizeType i = 0; i < txArray.Size(); ++i)
					{
						const rapidjson::Value& t = txArray[i];
						transactions.push_back({
							t.HasMember("kind") ? t["kind"].GetUint() : 0,
							t.HasMember("size") ? t["size"].GetUint() : 0,
							t.HasMember("addr") ? t["addr"].GetUint() : 0,
							t.HasMember("data") ? t["data"].GetUint() : 0,
							t.HasMember("cycle") ? t["cycle"].GetUint() : 0,
							t.HasMember("access") ? t["access"].GetUint() : 0
							});
					}
				}

				sst.transactions = transactions;

				if (SST_DEBUG_PRINT)
				{
					std::cout << "\n--- Initial State ---\n";
					for (int i = 0; i < 16; ++i) std::cout << "R[" << i << "]: 0x" << std::hex << initial->R[i] << "\n";
					for (int i = 0; i < 7; ++i) std::cout << "R_fiq[" << i << "]: 0x" << std::hex << initial->R_fiq[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "R_svc[" << i << "]: 0x" << std::hex << initial->R_svc[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "R_abt[" << i << "]: 0x" << std::hex << initial->R_abt[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "R_irq[" << i << "]: 0x" << std::hex << initial->R_irq[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "R_und[" << i << "]: 0x" << std::hex << initial->R_und[i] << "\n";
					for (int i = 0; i < 5; ++i) std::cout << "SPSR[" << i << "]: 0x" << std::hex << initial->spsr[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "Pipeline[" << i << "]: 0x" << std::hex << initial->pipeline[i] << "\n";
					std::cout << "CPSR: 0x" << std::hex << initial->cpsr << "\n";
					std::cout << "Access: " << std::dec << initial->access << "\n";

					std::cout << "\n--- Final State ---\n";
					for (int i = 0; i < 16; ++i) std::cout << "R[" << i << "]: 0x" << std::hex << final->R[i] << "\n";
					for (int i = 0; i < 7; ++i) std::cout << "R_fiq[" << i << "]: 0x" << std::hex << final->R_fiq[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "R_svc[" << i << "]: 0x" << std::hex << final->R_svc[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "R_abt[" << i << "]: 0x" << std::hex << final->R_abt[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "R_irq[" << i << "]: 0x" << std::hex << final->R_irq[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "R_und[" << i << "]: 0x" << std::hex << final->R_und[i] << "\n";
					for (int i = 0; i < 5; ++i) std::cout << "SPSR[" << i << "]: 0x" << std::hex << final->spsr[i] << "\n";
					for (int i = 0; i < 2; ++i) std::cout << "Pipeline[" << i << "]: 0x" << std::hex << final->pipeline[i] << "\n";
					std::cout << "CPSR: 0x" << std::hex << final->cpsr << "\n";
					std::cout << "Access: " << std::dec << final->access << "\n";

					std::cout << "\n--- Opcode/Base ---\n";
					std::cout << "Opcode: 0x" << std::hex << opcode << "\n";
					std::cout << "Base Address: 0x" << std::hex << baseAddr << "\n";

					std::cout << "\n--- Transactions (" << transactions.size() << ") ---\n";
					for (size_t i = 0; i < transactions.size(); ++i)
					{
						std::cout << "[" << i << "] kind=" << transactions[i].kind
							<< " size=" << transactions[i].size
							<< " addr=0x" << std::hex << transactions[i].addr
							<< " data=0x" << transactions[i].data
							<< " cycle=" << std::dec << transactions[i].cycle
							<< " access=" << transactions[i].access << "\n";
					}
				}

				// === Setup initial state ===
				pGBA_registers->unbankedLORegisters[0] = initial->R[0];
				pGBA_registers->unbankedLORegisters[1] = initial->R[1];
				pGBA_registers->unbankedLORegisters[2] = initial->R[2];
				pGBA_registers->unbankedLORegisters[3] = initial->R[3];
				pGBA_registers->unbankedLORegisters[4] = initial->R[4];
				pGBA_registers->unbankedLORegisters[5] = initial->R[5];
				pGBA_registers->unbankedLORegisters[6] = initial->R[6];
				pGBA_registers->unbankedLORegisters[7] = initial->R[7];
				pGBA_registers->cpsr.psrMemory = initial->cpsr;
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_USR_SYS][0] = initial->R[8];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_USR_SYS][1] = initial->R[9];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_USR_SYS][2] = initial->R[10];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_USR_SYS][3] = initial->R[11];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_USR_SYS][4] = initial->R[12];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_USR_SYS][5] = initial->R[13];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_USR_SYS][6] = initial->R[14];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_FIQ][0] = initial->R_fiq[0];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_FIQ][1] = initial->R_fiq[1];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_FIQ][2] = initial->R_fiq[2];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_FIQ][3] = initial->R_fiq[3];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_FIQ][4] = initial->R_fiq[4];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_FIQ][5] = initial->R_fiq[5];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_FIQ][6] = initial->R_fiq[6];
				pGBA_registers->spsr[REGISTER_BANK_TYPE::RB_FIQ].psrMemory = initial->spsr[0];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_SVC][0] = initial->R[8];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_SVC][1] = initial->R[9];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_SVC][2] = initial->R[10];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_SVC][3] = initial->R[11];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_SVC][4] = initial->R[12];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_SVC][5] = initial->R_svc[0];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_SVC][6] = initial->R_svc[1];
				pGBA_registers->spsr[REGISTER_BANK_TYPE::RB_SVC].psrMemory = initial->spsr[1];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_ABT][0] = initial->R[8];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_ABT][1] = initial->R[9];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_ABT][2] = initial->R[10];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_ABT][3] = initial->R[11];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_ABT][4] = initial->R[12];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_ABT][5] = initial->R_abt[0];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_ABT][6] = initial->R_abt[1];
				pGBA_registers->spsr[REGISTER_BANK_TYPE::RB_ABT].psrMemory = initial->spsr[2];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_IRQ][0] = initial->R[8];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_IRQ][1] = initial->R[9];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_IRQ][2] = initial->R[10];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_IRQ][3] = initial->R[11];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_IRQ][4] = initial->R[12];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_IRQ][5] = initial->R_irq[0];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_IRQ][6] = initial->R_irq[1];
				pGBA_registers->spsr[REGISTER_BANK_TYPE::RB_IRQ].psrMemory = initial->spsr[3];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_UND][0] = initial->R[8];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_UND][1] = initial->R[9];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_UND][2] = initial->R[10];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_UND][3] = initial->R[11];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_UND][4] = initial->R[12];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_UND][5] = initial->R_und[0];
				pGBA_registers->bankedHIRegisters[REGISTER_BANK_TYPE::RB_UND][6] = initial->R_und[1];
				pGBA_registers->spsr[REGISTER_BANK_TYPE::RB_UND].psrMemory = initial->spsr[4];

				pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = initial->pipeline[0];
				pGBA_registers->pc = initial->R[15];

				pGBA_memory->setNextMemoryAccessType = (initial->access & ARM7TDMI_SST_t::Sequential) ? MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE : MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;

				// === Execute processSOC() ===
				processSOC();

				// === Verify final state (using macro to avoid lambda) ===
				#define CHECK_VALUE(name, actual, expected) \
						if ((actual) != (expected)) { \
							if (!((std::strcmp(name, "PC") == 0) && (((expected) & 0xFFFFFFFC) == ((actual) & 0xFFFFFFFC)))) { \
								INFO("test number %d", testIndex); \
								INFO("Actual %u", (uint32_t)(actual)); \
								INFO("Expected %u", (uint32_t)(expected)); \
								FATAL("%s Mismatch", name); \
								quitThisRun = YES; \
							} \
						}

					// Unbanked LO registers
					for (int i = 0; i < 8; ++i)
						CHECK_VALUE(("R[" + std::to_string(i) + "]").c_str(), pGBA_registers->unbankedLORegisters[i], final->R[i]);

					// CPSR
					CHECK_VALUE("CPSR", pGBA_registers->cpsr.psrMemory, final->cpsr);

					// Banked HI Registers and SPSRs
					struct BankCheck {
						REGISTER_BANK_TYPE type;
						uint32_t* R_final_arr;
						uint32_t SPSR_val;
						const char* name;
					};

					BankCheck banks[] = {
						{REGISTER_BANK_TYPE::RB_USR_SYS, &final->R[8], 0, "RB_USR_SYS"},
						{REGISTER_BANK_TYPE::RB_FIQ, final->R_fiq, final->spsr[0], "RB_FIQ"},
						{REGISTER_BANK_TYPE::RB_SVC, final->R_svc, final->spsr[1], "RB_SVC"},
						{REGISTER_BANK_TYPE::RB_ABT, final->R_abt, final->spsr[2], "RB_ABT"},
						{REGISTER_BANK_TYPE::RB_IRQ, final->R_irq, final->spsr[3], "RB_IRQ"},
						{REGISTER_BANK_TYPE::RB_UND, final->R_und, final->spsr[4], "RB_UND"}
					};

					for (auto& b : banks)
					{
						int j = 0;
						for (int i = ((b.type == REGISTER_BANK_TYPE::RB_USR_SYS || b.type == REGISTER_BANK_TYPE::RB_FIQ) ? 0 : 5); i < 7; ++i)
						{
							char buf[64];
							snprintf(buf, sizeof(buf), "%s[%d]", b.name, i);
							CHECK_VALUE(buf, pGBA_registers->bankedHIRegisters[b.type][i], b.R_final_arr[j++]);
						}

						if (b.SPSR_val != 0)
						{
							char buf[64];
							snprintf(buf, sizeof(buf), "SPSR_%s", b.name);
							CHECK_VALUE(buf, pGBA_registers->spsr[b.type].psrMemory, b.SPSR_val);
						}
					}

					// PC
					CHECK_VALUE("PC", pGBA_registers->pc, final->R[15]);

					// Transaction
					uint32_t count = std::min<uint32_t>((uint32_t)transactions.size(), (uint32_t)sst.index);

					for (uint32_t i = 0; i < count; ++i)
					{
						char buf[64];

						snprintf(buf, sizeof(buf), "TX[%u].kind", i);
						CHECK_VALUE(buf, (uint32_t)sst.internal[i].kind, transactions[i].kind);

						snprintf(buf, sizeof(buf), "TX[%u].size", i);
						CHECK_VALUE(buf, (uint32_t)sst.internal[i].size, transactions[i].size);

						snprintf(buf, sizeof(buf), "TX[%u].addr", i);
						CHECK_VALUE(buf, (uint32_t)sst.internal[i].addr, transactions[i].addr);

						snprintf(buf, sizeof(buf), "TX[%u].data", i);
						CHECK_VALUE(buf, (uint32_t)sst.internal[i].data, transactions[i].data);

						TODO("SST: Cycle matching is disabled");
						//snprintf(buf, sizeof(buf), "TX[%u].cycle", i);
						//CHECK_VALUE(buf, (uint32_t)sst.internal[i].cycle, transactions[i].cycle);

						snprintf(buf, sizeof(buf), "TX[%u].access", i);
						CHECK_VALUE(buf, (uint32_t)(sst.internal[i].access), transactions[i].access);

						if (quitThisRun == YES)
						{
							volatile int bp = 0;
						}
					}

					#undef CHECK_VALUE

					// Count mismatch is also an error
					if (transactions.size() != sst.index)
					{
						FATAL("Transaction count mismatch: live=%u ref=%zu\n", sst.index, transactions.size());
						quitThisRun = YES;
					}

					if (quitThisRun == YES)
					{
						++failedCount;
						BREAK;
					}

					++passedCount;
				}

				INFO("Test file %s : %u passed, %u failed",
					testFileName.c_str(), passedCount, failedCount);
			}
		}

		INFO("================================================================================");
		INFO("GBA SST Testing Complete");
		INFO("================================================================================");
		PAUSE;
	}
	else
#endif
	{
		if (ROM_TYPE == ROM::COMPARE)
		{
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
	}

	if (pGBA_display->wasVblankJustTriggered)
	{
		// at GBA VBlank — hardware accurate cheat writes
		// ROM patch entries (address >= GAMEPAK_ROM_WS0_START_ADDRESS) are skipped here;
		// those are handled by interceptCPURead in the read path.
		for (auto engine : { CheatEngine_t::CHEATING_ENGINE::GAMESHARK,
							 CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3,
							 CheatEngine_t::CHEATING_ENGINE::CODEBREAKER })
		{
			auto writes = ceGBA->getCheatWrites(engine);
			for (auto& w : writes)
			{
				if (w.address >= GAMEPAK_ROM_WS0_START_ADDRESS) continue;

				switch (w.width)
				{
				case CheatEngine_t::CheatWidth::U8:
					writeRawMemory<uint8_t>(
						w.address,
						static_cast<uint8_t>(w.data),
						MEMORY_ACCESS_WIDTH::EIGHT_BIT,
						MEMORY_ACCESS_SOURCE::CPU,
						MEMORY_ACCESS_TYPE::AUTOMATIC);
					BREAK;
				case CheatEngine_t::CheatWidth::U16:
					writeRawMemory<uint16_t>(
						w.address,
						static_cast<uint16_t>(w.data),
						MEMORY_ACCESS_WIDTH::SIXTEEN_BIT,
						MEMORY_ACCESS_SOURCE::CPU,
						MEMORY_ACCESS_TYPE::AUTOMATIC);
					BREAK;
				case CheatEngine_t::CheatWidth::U32:
					writeRawMemory<uint32_t>(
						w.address,
						static_cast<uint32_t>(w.data),
						MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT,
						MEMORY_ACCESS_SOURCE::CPU,
						MEMORY_ACCESS_TYPE::AUTOMATIC);
					BREAK;
				}
			}
		}
	}

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

	// Initialize the backup interface

	pGBA_instance->GBA_state.emulatorStatus.backup.flash.erase4kbPageNumber = (SSTATE32)INVALID;

	// Setup the logger interface

	pGBA_instance->GBA_state.emulatorStatus.debugger.loggerInterface.logger = ENABLE_LOGS;

	// Initialize few memory variables

	pGBA_memory->previouslyAccessedMemory = RESET;
	pGBA_memory->setNextMemoryAccessType = MEMORY_ACCESS_TYPE::AUTOMATIC;
	pGBA_memory->setNextPipelineAccessType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;

	// Initialize memory

	memset(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ZERO], 0xFF, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ZERO]));
	memset(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ONE], 0xFF, sizeof(pGBA_memory->mGBAMemoryMap.mGamePakBackup.mGamePakFlash.mExtFlash8bit[ONE]));

	if (isCLI() == NO)
	{
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
		glGenTextures(1, &gameboyAdvance_matrix_texture);

		glBindTexture(GL_TEXTURE_2D, gameboyAdvance_matrix_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)gameboyAdvance_matrix);
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
		GL_CALL(glGenTextures(1, &gameboyAdvance_matrix_texture));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboyAdvance_matrix_texture));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)gameboyAdvance_matrix));
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
		shaderPath = pt.get<std::string>("internal._working_directory", "");
		if (shaderPath.empty())
		{
			FATAL("Could not locate the shaders");
		}
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
	}

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
					uint32_t address = GAMEPAK_SRAM_START_ADDRESS + (banks * sizeOfSRAMSlot) + ii;

					BYTE ramByte = readRawMemoryInternal<BYTE>(
						address,
						MEMORY_ACCESS_WIDTH::EIGHT_BIT,
						MEMORY_ACCESS_SOURCE::HOST,
						MEMORY_ACCESS_TYPE::AUTOMATIC
					);

					outSRAM.write(reinterpret_cast<const char*>(&ramByte), ONE);
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
	memset(gbaEmulationCounter, ZERO, ((sizeof(gbaEmulationCounter[100])) / sizeof(gbaEmulationCounter[0])));

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
	glDeleteTextures(1, &gameboyAdvance_matrix_texture);
#else
	glDeleteTextures(1, &gameboyAdvance_texture);
	glDeleteTextures(1, &gameboyAdvance_matrix_texture);
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

			pAbsolute_GBA_instance->absolute_GBA_state.aboutRom.romMaxAddressMask = (32 * 1024 * 1024) - 1; // 32 MiB Mask

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

OPT_DEFAULT
#pragma endregion EMULATION_DEFINITIONS