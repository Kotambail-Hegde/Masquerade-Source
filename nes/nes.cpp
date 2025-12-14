#pragma region NES_SPECIFIC_INCLUDES
#include "nes.h"
#pragma endregion NES_SPECIFIC_INCLUDES

#pragma region CONDITIONAL_INCLUDES
#pragma endregion CONDITIONAL_INCLUDES

#pragma region NES_SPECIFIC_MACROS
#define KEY_A											(ZERO)
#define KEY_B											(ONE)
#define KEY_SELECT										(TWO)
#define KEY_START										(THREE)
#define KEY_UP											(FOUR)
#define KEY_DOWN										(FIVE)
#define KEY_LEFT										(SIX)
#define KEY_RIGHT										(SEVEN)

#define FIRST_WRITE										(NO)
#define SECOND_WRITE									(YES)

#define DISABLE_FIRST_PULSE_CHANNEL						(NO)
#define DISABLE_SECOND_PULSE_CHANNEL					(NO)
#define DISABLE_TRIANGLE_CHANNEL						(NO)
#define DISABLE_NOISE_CHANNEL							(NO)
#define DISABLE_DMC_CHANNEL								(NO)
#define ENABLE_ACCURATE_AUDIO							(NO) // Drawback -> Computationally intensive and hence has noticable latency

#define ENABLE_CHR_RAM_BANKING_MMC1						(NO)
#define ENABLE_CHR_RAM_BANKING_MMC3						(NO)

#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif
#pragma endregion NES_SPECIFIC_MACROS

#pragma region NES_SPECIFIC_DECLARATIONS
// For debug
static COUNTER64 logCounter = ZERO;
static COUNTER64 emulationCounter[100] = { ZERO };

// for audio
// maximum number of inputs
static uint32_t const MAX_INPUT_LEN = (uint32_t)(EMULATED_AUDIO_SAMPLING_RATE_FOR_NES / CEIL(NES_FPS));
// length of filter than can be handled
static uint32_t const MAX_FLT_LEN = 103;
// buffer to hold all of the input samples
static uint32_t const BUFFER_LEN = (MAX_FLT_LEN - 1 + MAX_INPUT_LEN);
// coefficients for the FIR from https://www.arc.id.au/FilterDesign.html
// 
// double type buffers to hold input and output during FIR
static double doubleInput[(uint32_t)(EMULATED_AUDIO_SAMPLING_RATE_FOR_NES / CEIL(NES_FPS))];
static double doubleOutput[(uint32_t)(EMULATED_AUDIO_SAMPLING_RATE_FOR_NES / CEIL(NES_FPS))];

static std::string _JSON_LOCATION;
static boost::property_tree::ptree testCase;

static uint32_t nes_texture;
static uint32_t matrix_texture;
static uint32_t matrix[16] = { 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
#pragma endregion NES_SPECIFIC_DECLARATIONS

#pragma region INFRASTRUCTURE_DEFINITIONS
NES_t::NES_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config, CheatEngine_t *ce)
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

	setEmulationID(EMULATION_ID::NES_ID);

	this->pt = config;

	this->ceNES = ce;

	this->ceNES->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, EMULATION_ID::NES_ID);

	if (nFiles == SST_ROMS)
	{
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_WARN);
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_EVENT);

		INFO("Running in sst Cpu Test Mode!");
		_JSON_LOCATION = rom[ONE];

		ROM_TYPE = ROM::TEST_SST;
	}
	else
	{
		//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUDEBUG);
		//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_DISASSEMBLY);

		auto getExt = [](const std::string& filename) -> std::string 
			{
				std::string f = filename;
				std::transform(f.begin(), f.end(), f.begin(), ::tolower);
				std::size_t pos = f.find_last_of('.');
				RETURN (pos != std::string::npos) ? f.substr(pos + 1) : "";
			};

		// Determine ROM_TYPE
		ROM_TYPE = (getExt(rom[ZERO]) == "nes") ? ROM::NES : (rom.size() > ONE && getExt(rom[ONE]) == "bin") ? ROM::TEST_ROM_BIN : ROM::NO_ROM;

#ifndef __EMSCRIPTEN__
		_SAVE_LOCATION = pt.get<std::string>("nes._save_location");
#else
		_SAVE_LOCATION = "assets/saves";
#endif

		// check if directory mentioned by "_SAVE_LOCATION" exists, if not we need to explicitly create it
		ifNoDirectoryThenCreate(_SAVE_LOCATION);

		this->rom[ZERO] = rom[(ROM_TYPE == ROM::NES) ? ZERO : ONE];

#if (NES_ENABLE_AUDIO == NO)
		_ENABLE_AUDIO = NO;
#endif

		if (debugConfig._DEBUG_PPU_VIEWER_GUI == YES)
		{
			LOG("Debugger : Enabled\n");
		}
		else
		{
			LOG("Debugger : Disabled\n");
		}

		if (!_ENABLE_REWIND)
		{
			_REWIND_BUFFER_SIZE = 0;
			LOG("Rewind : Disabled\n");
		}
		else if (_REWIND_BUFFER_SIZE <= 0 && _ENABLE_REWIND == YES)
		{
			_ENABLE_REWIND = NO;
			LOG("Rewind : Disabled\n");
		}
		else
		{
			LOG("Rewind : Enabled\n");
		}

		LOG_NEW_LINE;

#if (NESTEST_AUTOMATED_MODE == YES)
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_CPUDEBUG);
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_DISASSEMBLY);
#endif
	}
}

NES_t::~NES_t()
{
	; // Do nothing for now!
}

void NES_t::setupTheCoreOfEmulation(void* masqueradeInstance, void* audio, void* network)
{
	uint8_t indexToCheck = 0;

	if (!rom[indexToCheck].empty() || (ROM_TYPE == ROM::TEST_SST))
	{
		if (!initializeEmulator())
		{
			LOG("memory allocation failure\n");
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
		LOG("un-supported rom\n");
		throw std::runtime_error("un-supported rom");
	}
}

uint32_t NES_t::getScreenWidth()
{
	RETURN this->screen_width;
}

uint32_t NES_t::getScreenHeight()
{
	RETURN this->screen_height;
}

uint32_t NES_t::getPixelWidth()
{
	RETURN this->pixel_width;
}

uint32_t NES_t::getPixelHeight()
{
	RETURN this->pixel_height;
}

void NES_t::setEmulationWindowOffsets(uint32_t x, uint32_t y, bool isEnabled)
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

uint32_t NES_t::getTotalScreenWidth()
{
	if (debugConfig._DEBUG_PPU_VIEWER_GUI == ENABLED)
	{
		RETURN this->debugger_screen_width;
	}
	else
	{
		RETURN this->screen_width;
	}
}

uint32_t NES_t::getTotalScreenHeight()
{
	if (debugConfig._DEBUG_PPU_VIEWER_GUI == ENABLED)
	{
		RETURN this->debugger_screen_height;
	}
	else
	{
		RETURN this->screen_height;
	}
}

uint32_t NES_t::getTotalPixelWidth()
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

uint32_t NES_t::getTotalPixelHeight()
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

void NES_t::setEmulationID(EMULATION_ID ID)
{
	myID = ID;
}

EMULATION_ID NES_t::getEmulationID()
{
	RETURN myID;
}

const char* NES_t::getEmulatorName()
{
	RETURN this->NAME;
}

float NES_t::getEmulationFPS()
{
	RETURN this->myFPS;
}
#pragma endregion INFRASTRUCTURE_DEFINITIONS

#pragma region RP2C02_DEFINITIONS
void NES_t::clockMMC3IRQ(uint16_t address, MEMORY_ACCESS_SOURCE source, FLAG isWriteOperation)
{
	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC3)
	{
		if (address & 0x1000 // If A12 is high
			&&
			pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.unfilteredA12RiseEvent == NO) // If A12 was previously low
		{
			pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.unfilteredA12RiseEvent = YES;

			// Refer https://forums.nesdev.org/viewtopic.php?p=92322#p92322
			if (pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterMMC3A12 >= SIXTEEN)
			{
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.filteredA12RiseEvent = YES;

				if (pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter == RESET
					|| pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqCounterReloadEnabled == YES)
				{
					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter
						= pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqReload_evenCk;

					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqCounterReloadEnabled = CLEAR;
				}
				else if (pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter > RESET)
				{
					--pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter;
				}

				if (pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter == RESET
					&& pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqEnable == ENABLED)
				{
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC3 = SET;
					pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = RESET;
				}
			}
			else
			{
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.filteredA12RiseEvent = NO;
			}

			pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterMMC3A12 = RESET;
		}
		// If A12 is low
		else if ((address & 0x1000) == RESET)
		{
			// Clear the unfilteredA12RiseEvent and filteredA12RiseEvent
			pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.unfilteredA12RiseEvent = NO;
			pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.filteredA12RiseEvent = NO;
		}
	}
}

byte NES_t::readPpuRawMemory(uint16_t address, MEMORY_ACCESS_SOURCE source)
{
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType = TYPE_OF_MEMORY_ACCESS::PPU_READ;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousPPUAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentPPUAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentPPUAccessType = TYPE_OF_MEMORY_ACCESS::PPU_READ;

	if (address >= 0x4000)
	{
		PPUWARN("Invalid address 0x%X", address);
	}

	address &= 0x3FFF;

	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC3)
	{
		// Refer https://forums.nesdev.org/viewtopic.php?p=243424#p243424 AND
		// https://forums.nesdev.org/viewtopic.php?p=243432#p243432 AND
		// https://forums.nesdev.org/viewtopic.php?p=243434#p243434
		// for reasons to ignore palette RAM Read Access from only PPU but allow from CPU
		// But below we ignore CPU as well because we handle the access via CPU in readCpuRawMemory
		if (source == MEMORY_ACCESS_SOURCE::CPU)
		{
			; // Don't do anything here!
		}
		else if (source == MEMORY_ACCESS_SOURCE::PPU
			&& (IF_ADDRESS_WITHIN(address, PALETTE_RAM_INDEXES_START_ADDRESS, PALETTE_RAM_INDEXES_MIRROR_END_ADDRESS)))
		{
			; // Don't do anything here!
		}
		else if ((pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET)
			|| (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET))
		{
			clockMMC3IRQ(address, source, NO);
		}
	}

	if (source == MEMORY_ACCESS_SOURCE::PPU || source == MEMORY_ACCESS_SOURCE::CPU)
	{
		switch (pNES_instance->NES_state.catridgeInfo.mapper)
		{
		case MAPPER::NROM:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
			{
				RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS];
			}
			else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS];
			}
			BREAK;
		}
		case MAPPER::MMC1:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
				{
#if (ENABLE_CHR_RAM_BANKING_MMC1 == NO)
					if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS];
					}
					else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS];
					}
#endif
					if (pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.fields1.c == RESET)
					{
						if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
						{
							RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS];
						}
						else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
						{
							RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS];
						}
					}
					// In 4KB mode, CHR bank 0 can support CHR RAM Banking
					// Refer https://www.nesdev.org/wiki/MMC1#CHR_bank_0_(internal,_$A000-$BFFF)
					else
					{
						if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
						{
							if ((pNES_instance->NES_state.catridgeInfo.mmc1.chrBank4Lo & 0x01) == ZERO)
							{
								RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS];
							}
							else
							{
								RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE0_START_ADDRESS];
							}
						}
						else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
						{
							RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS];
						}
					}
				}
				else
				{
					// 8KB mode
					if (pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.fields1.c == RESET)
					{
						auto index = pNES_instance->NES_state.catridgeInfo.mmc1.chrBank8 * 0x1000;
						index += ((address - PATTERN_TABLE0_START_ADDRESS) & 0x1FFF);
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
					}
					// 4KB mode
					else
					{
						if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
						{
							auto index = pNES_instance->NES_state.catridgeInfo.mmc1.chrBank4Lo * 0x1000;
							index += ((address - PATTERN_TABLE0_START_ADDRESS) & 0x0FFF);
							RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
						}
						if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
						{
							auto index = pNES_instance->NES_state.catridgeInfo.mmc1.chrBank4Hi * 0x1000;
							index += ((address - PATTERN_TABLE1_START_ADDRESS) & 0x0FFF);
							RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
						}
					}
				}
			}
			BREAK;
		}
		case MAPPER::UxROM_002:
		case MAPPER::AxROM:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
			{
				RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS];
			}
			else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS];
			}
			BREAK;
		}
		case MAPPER::CNROM:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
				{
					if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS];
					}
					else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS];
					}
				}
				else
				{
					auto index = pNES_instance->NES_state.catridgeInfo.cnrom.chrBank8 * 0x2000;
					index += ((address - PATTERN_TABLE0_START_ADDRESS) & 0x1FFF);
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
				}
			}
			BREAK;
		}
		case MAPPER::MMC3:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
#if (ENABLE_CHR_RAM_BANKING_MMC3 == NO)
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
				{
					if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS];
					}
					else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS];
					}
				}
#endif
				BIT chrRomBankMode = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.chrRomMode;

				auto startAddr1 = PATTERN_TABLE0_START_ADDRESS;
				auto endAddr1 = startAddr1 + 0x07FF; // 2KB
				auto startAddr2 = endAddr1 + ONE;
				auto endAddr2 = startAddr2 + 0x07FF; // 2KB
				auto startAddr3 = endAddr2 + ONE;
				auto endAddr3 = startAddr3 + 0x03FF; // 1KB
				auto startAddr4 = endAddr3 + ONE;
				auto endAddr4 = startAddr4 + 0x03FF; // 1KB
				auto startAddr5 = endAddr4 + ONE;
				auto endAddr5 = startAddr5 + 0x03FF; // 1KB
				auto startAddr6 = endAddr5 + ONE;
				auto endAddr6 = startAddr6 + 0x03FF; // 1KB

				if (chrRomBankMode == SET)
				{
					startAddr1 = PATTERN_TABLE0_START_ADDRESS;
					endAddr1 = startAddr1 + 0x03FF; // 1KB
					startAddr2 = endAddr1 + ONE;
					endAddr2 = startAddr2 + 0x03FF; // 1KB
					startAddr3 = endAddr2 + ONE;
					endAddr3 = startAddr3 + 0x03FF; // 1KB
					startAddr4 = endAddr3 + ONE;
					endAddr4 = startAddr4 + 0x03FF; // 1KB
					startAddr5 = endAddr4 + ONE;
					endAddr5 = startAddr5 + 0x07FF; // 2KB
					startAddr6 = endAddr5 + ONE;
					endAddr6 = startAddr6 + 0x07FF; // 2KB
				}

				if (IF_ADDRESS_WITHIN(address, startAddr1, endAddr1))
				{
					auto wrapAround = 0x07FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a * 0x0400;
					if (chrRomBankMode == SET)
					{
						wrapAround = 0x03FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a * 0x0400;
					}
					index += ((address - startAddr1) & wrapAround);
					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index];
					}
					else
					{
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
					}
				}
				if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
				{
					auto wrapAround = 0x07FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b * 0x0400;
					if (chrRomBankMode == SET)
					{
						wrapAround = 0x03FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b * 0x0400;
					}
					index += ((address - startAddr2) & wrapAround);
					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index];
					}
					else
					{
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
					}
				}
				if (IF_ADDRESS_WITHIN(address, startAddr3, endAddr3))
				{
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a * 0x0400;
					if (chrRomBankMode == SET)
					{
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c * 0x0400;
					}
					index += ((address - startAddr3) & 0x3FF);
					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index];
					}
					else
					{
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
					}
				}
				if (IF_ADDRESS_WITHIN(address, startAddr4, endAddr4))
				{
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b * 0x0400;
					if (chrRomBankMode == SET)
					{
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d * 0x0400;
					}
					index += ((address - startAddr4) & 0x3FF);
					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index];
					}
					else
					{
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
					}
				}
				if (IF_ADDRESS_WITHIN(address, startAddr5, endAddr5))
				{
					auto wrapAround = 0x03FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c * 0x0400;
					if (chrRomBankMode == SET)
					{
						wrapAround = 0x07FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a * 0x0400;
					}
					index += ((address - startAddr5) & wrapAround);
					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index];
					}
					else
					{
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
					}
				}
				if (IF_ADDRESS_WITHIN(address, startAddr6, endAddr6))
				{
					auto wrapAround = 0x03FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d * 0x0400;
					if (chrRomBankMode == SET)
					{
						wrapAround = 0x07FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b * 0x0400;
					}
					index += ((address - startAddr6) & wrapAround);
					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index];
					}
					else
					{
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
					}
				}
				FATAL("Invalid CHR ROM/RAM address in MMC3");
			}
			BREAK;
		}
		case MAPPER::GxROM:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
				{
					if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS];
					}
					else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS];
					}
				}
				else
				{
					auto index = pNES_instance->NES_state.catridgeInfo.gxrom.chrBank * 0x2000;
					index += ((address - PATTERN_TABLE0_START_ADDRESS) & 0x1FFF);
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
				}
			}
			BREAK;
		}
		case MAPPER::NANJING_FC001:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
			{
				RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS];
			}
			else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS];
			}
			BREAK;
		}
		default:
		{
			FATAL("Read performed for unsupported mapper");
			BREAK;
		}
		}

		if ((IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			|| (IF_ADDRESS_WITHIN(address, PPU_UNUSED_START_ADDRESS, PPU_UNUSED_END_ADDRESS)))
		{
			if (IF_ADDRESS_WITHIN(address, PPU_UNUSED_START_ADDRESS, PPU_UNUSED_END_ADDRESS))
			{
				// Address from 0x3000 is memory mapped to 0x2000, so subtract by 0x1000 and access the name table buffer
				// Subtracting by 0x1000 to bring the address to same range as nametable
				address -= 0x1000;
			}

			if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::HORIZONTAL_MIRROR)
			{
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[address - NAME_TABLE0_START_ADDRESS];
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[address - NAME_TABLE1_START_ADDRESS];
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[address - NAME_TABLE2_START_ADDRESS];
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE3_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[address - NAME_TABLE3_START_ADDRESS];
				}
			}
			else if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::VERTICAL_MIRROR)
			{
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[address - NAME_TABLE0_START_ADDRESS];
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[address - NAME_TABLE1_START_ADDRESS];
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[address - NAME_TABLE2_START_ADDRESS];
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE3_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[address - NAME_TABLE3_START_ADDRESS];
				}
			}
			else if ((pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR)
				|| (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR))
			{
				BYTE* nameTable = pNES_ppuMemory->NESMemoryMap.nameTable0;
				if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR)
				{
					nameTable = pNES_ppuMemory->NESMemoryMap.nameTable1;
				}

				if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::AxROM)
				{
					if (pNES_instance->NES_state.catridgeInfo.axrom.vramPage == YES)
					{
						nameTable = pNES_ppuMemory->NESMemoryMap.nameTable1;
					}
					else
					{
						nameTable = pNES_ppuMemory->NESMemoryMap.nameTable0;
					}
				}

				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					RETURN nameTable[address - NAME_TABLE0_START_ADDRESS];
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					RETURN nameTable[address - NAME_TABLE1_START_ADDRESS];
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					RETURN nameTable[address - NAME_TABLE2_START_ADDRESS];
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE3_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
				{
					RETURN nameTable[address - NAME_TABLE3_START_ADDRESS];
				}
			}
			else
			{
				FATAL("Unsupported Nametable Arrangement");
			}
		}
		else if (IF_ADDRESS_WITHIN(address, PALETTE_RAM_INDEXES_START_ADDRESS, PALETTE_RAM_INDEXES_MIRROR_END_ADDRESS))
		{
			address -= PALETTE_RAM_INDEXES_START_ADDRESS;
			auto index = address % THIRTYTWO;
			auto data = pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index];

			// Refer to "Color control" in https://www.nesdev.org/wiki/PPU_registers#PPUMASK_-_Rendering_settings_($2001_write)
			if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.GREYSCALE == SET)
			{
				data &= 0x30;
			}

			RETURN data;
		}
	}

	FATAL("Unknown Memory Access Source : %d", TO_UINT(source));
	RETURN (byte)ZERO;
}

void NES_t::writePpuRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source)
{
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType = TYPE_OF_MEMORY_ACCESS::PPU_WRITE;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousPPUAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentPPUAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentPPUAccessType = TYPE_OF_MEMORY_ACCESS::PPU_WRITE;

	if (address >= 0x4000)
	{
		PPUWARN("Invalid address 0x%X", address);
	}

	address &= 0x3FFF;

	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC3)
	{
		// Refer https://forums.nesdev.org/viewtopic.php?p=243424#p243424 AND
		// https://forums.nesdev.org/viewtopic.php?p=243432#p243432 AND
		// https://forums.nesdev.org/viewtopic.php?p=243434#p243434
		// for reasons to allow both CPU and PPU write access
		// But below, we ignore the CPU access as this is handled in writeCpuRawMemory
		if (source == MEMORY_ACCESS_SOURCE::CPU)
		{
			; // Don't do anything here!
		}
		else if ((pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET)
			|| (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET))
		{
			clockMMC3IRQ(address, source, YES);
		}
	}

	if (source == MEMORY_ACCESS_SOURCE::PPU || source == MEMORY_ACCESS_SOURCE::CPU || source == MEMORY_ACCESS_SOURCE::DMA)
	{
		switch (pNES_instance->NES_state.catridgeInfo.mapper)
		{
		case MAPPER::NROM:
		case MAPPER::UxROM_002:
		case MAPPER::CNROM:
		case MAPPER::AxROM:
		case MAPPER::GxROM:
		{
			// If size of chr rom is 0, then use this as chr ram
			if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
			{
				if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS] = data;
				}
				else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS] = data;
				}
			}
			BREAK;
		}
		case MAPPER::NANJING_FC001:
		{
			// For mapper 163, size of chr rom is always 0, hence we always use this memory as chr ram
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
			{
				pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS] = data;
			}
			else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS] = data;
			}
			BREAK;
		}
		case MAPPER::MMC1:
		{
			if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
			{
#if (ENABLE_CHR_RAM_BANKING_MMC1 == NO)
				if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS] = data;
					RETURN;
				}
				else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS] = data;
					RETURN;
				}
#endif
				if (pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.fields1.c == RESET)
				{
					if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
					{
						pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS] = data;
						RETURN;
					}
					else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
					{
						pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS] = data;
						RETURN;
					}
				}
				// In 4KB mode, CHR bank 0 can support CHR RAM Banking
				// Refer https://www.nesdev.org/wiki/MMC1#CHR_bank_0_(internal,_$A000-$BFFF)
				else
				{
					if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
					{
						if ((pNES_instance->NES_state.catridgeInfo.mmc1.chrBank4Lo & 0x01) == ZERO)
						{
							pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS] = data;
							RETURN;
						}
						else
						{
							pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE0_START_ADDRESS] = data;
							RETURN;
						}
					}
					else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
					{
						pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS] = data;
						RETURN;
					}
				}
			}
			BREAK;
		}
		case MAPPER::MMC3:
		{
			if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
			{
#if (ENABLE_CHR_RAM_BANKING_MMC3 == NO)
				if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[address - PATTERN_TABLE0_START_ADDRESS] = data;
					RETURN;
				}
				else if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE1_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[address - PATTERN_TABLE1_START_ADDRESS] = data;
					RETURN;
				}
#endif
				if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
				{
					BIT chrRomBankMode = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.chrRomMode;

					auto startAddr1 = PATTERN_TABLE0_START_ADDRESS;
					auto endAddr1 = startAddr1 + 0x07FF; // 2KB
					auto startAddr2 = endAddr1 + ONE;
					auto endAddr2 = startAddr2 + 0x07FF; // 2KB
					auto startAddr3 = endAddr2 + ONE;
					auto endAddr3 = startAddr3 + 0x03FF; // 1KB
					auto startAddr4 = endAddr3 + ONE;
					auto endAddr4 = startAddr4 + 0x03FF; // 1KB
					auto startAddr5 = endAddr4 + ONE;
					auto endAddr5 = startAddr5 + 0x03FF; // 1KB
					auto startAddr6 = endAddr5 + ONE;
					auto endAddr6 = startAddr6 + 0x03FF; // 1KB

					if (chrRomBankMode == SET)
					{
						startAddr1 = PATTERN_TABLE0_START_ADDRESS;
						endAddr1 = startAddr1 + 0x03FF; // 1KB
						startAddr2 = endAddr1 + ONE;
						endAddr2 = startAddr2 + 0x03FF; // 1KB
						startAddr3 = endAddr2 + ONE;
						endAddr3 = startAddr3 + 0x03FF; // 1KB
						startAddr4 = endAddr3 + ONE;
						endAddr4 = startAddr4 + 0x03FF; // 1KB
						startAddr5 = endAddr4 + ONE;
						endAddr5 = startAddr5 + 0x07FF; // 2KB
						startAddr6 = endAddr5 + ONE;
						endAddr6 = startAddr6 + 0x07FF; // 2KB
					}

					if (IF_ADDRESS_WITHIN(address, startAddr1, endAddr1))
					{
						auto wrapAround = 0x07FF;
						auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a * 0x0400;
						if (chrRomBankMode == SET)
						{
							wrapAround = 0x03FF;
							index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a * 0x0400;
						}
						index += ((address - startAddr1) & wrapAround);
						pNES_ppuMemory->NESMemoryMap.patternTable.raw[index] = data;
						RETURN;
					}
					if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
					{
						auto wrapAround = 0x07FF;
						auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b * 0x0400;
						if (chrRomBankMode == SET)
						{
							wrapAround = 0x03FF;
							index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b * 0x0400;
						}
						index += ((address - startAddr2) & wrapAround);
						pNES_ppuMemory->NESMemoryMap.patternTable.raw[index] = data;
						RETURN;
					}
					if (IF_ADDRESS_WITHIN(address, startAddr3, endAddr3))
					{
						auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a * 0x0400;
						if (chrRomBankMode == SET)
						{
							index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c * 0x0400;
						}
						index += ((address - startAddr3) & 0x3FF);
						pNES_ppuMemory->NESMemoryMap.patternTable.raw[index] = data;
						RETURN;
					}
					if (IF_ADDRESS_WITHIN(address, startAddr4, endAddr4))
					{
						auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b * 0x0400;
						if (chrRomBankMode == SET)
						{
							index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d * 0x0400;
						}
						index += ((address - startAddr4) & 0x3FF);
						pNES_ppuMemory->NESMemoryMap.patternTable.raw[index] = data;
						RETURN;
					}
					if (IF_ADDRESS_WITHIN(address, startAddr5, endAddr5))
					{
						auto wrapAround = 0x03FF;
						auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c * 0x0400;
						if (chrRomBankMode == SET)
						{
							wrapAround = 0x07FF;
							index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a * 0x0400;
						}
						index += ((address - startAddr5) & wrapAround);
						pNES_ppuMemory->NESMemoryMap.patternTable.raw[index] = data;
						RETURN;
					}
					if (IF_ADDRESS_WITHIN(address, startAddr6, endAddr6))
					{
						auto wrapAround = 0x03FF;
						auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d * 0x0400;
						if (chrRomBankMode == SET)
						{
							wrapAround = 0x07FF;
							index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b * 0x0400;
						}
						index += ((address - startAddr6) & wrapAround);
						pNES_ppuMemory->NESMemoryMap.patternTable.raw[index] = data;
						RETURN;
					}
					FATAL("Invalid CHR ROM/RAM address in MMC3");
				}
			}
			BREAK;
		}
		default:
		{
			FATAL("Read performed for unsupported mapper");
			BREAK;
		}
		}

		if ((IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			|| (IF_ADDRESS_WITHIN(address, PPU_UNUSED_START_ADDRESS, PPU_UNUSED_END_ADDRESS)))
		{
			if (IF_ADDRESS_WITHIN(address, PPU_UNUSED_START_ADDRESS, PPU_UNUSED_END_ADDRESS))
			{
				// Address from 0x3000 is memory mapped to 0x2000, so subtract by 0x1000 and access the name table buffer
				// Subtracting by 0x1000 to bring the address to same range as nametable
				address -= 0x1000;
			}

			if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::HORIZONTAL_MIRROR)
			{
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.nameTable0[address - NAME_TABLE0_START_ADDRESS] = data;
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.nameTable0[address - NAME_TABLE1_START_ADDRESS] = data;
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.nameTable1[address - NAME_TABLE2_START_ADDRESS] = data;
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE3_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.nameTable1[address - NAME_TABLE3_START_ADDRESS] = data;
				}
			}
			else if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::VERTICAL_MIRROR)
			{
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.nameTable0[address - NAME_TABLE0_START_ADDRESS] = data;
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.nameTable1[address - NAME_TABLE1_START_ADDRESS] = data;
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.nameTable0[address - NAME_TABLE2_START_ADDRESS] = data;
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE3_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
				{
					pNES_ppuMemory->NESMemoryMap.nameTable1[address - NAME_TABLE3_START_ADDRESS] = data;
				}
			}
			else if ((pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR)
				|| (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR))
			{
				BYTE* nameTable = pNES_ppuMemory->NESMemoryMap.nameTable0;
				if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR)
				{
					nameTable = pNES_ppuMemory->NESMemoryMap.nameTable1;
				}

				if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::AxROM)
				{
					if (pNES_instance->NES_state.catridgeInfo.axrom.vramPage == YES)
					{
						nameTable = pNES_ppuMemory->NESMemoryMap.nameTable1;
					}
					else
					{
						nameTable = pNES_ppuMemory->NESMemoryMap.nameTable0;
					}
				}

				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					nameTable[address - NAME_TABLE0_START_ADDRESS] = data;
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					nameTable[address - NAME_TABLE1_START_ADDRESS] = data;
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					nameTable[address - NAME_TABLE2_START_ADDRESS] = data;
				}
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE3_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
				{
					nameTable[address - NAME_TABLE3_START_ADDRESS] = data;
				}
			}
			else
			{
				FATAL("Unsupported Nametable Arrangement");
			}
		}
		else if (IF_ADDRESS_WITHIN(address, PALETTE_RAM_INDEXES_START_ADDRESS, PALETTE_RAM_INDEXES_MIRROR_END_ADDRESS))
		{
			address -= PALETTE_RAM_INDEXES_START_ADDRESS;
			auto index = address % THIRTYTWO;

			// Refer : https://www.nesdev.org/wiki/PPU_palettes
			// Note that entry 0 of each palette is also unique in that its color value is shared between the background and sprite palettes,
			// so writing to either one updates the same internal storage. 
			// This means that the backdrop color can be written through both $3F00 and $3F10.

			if (index == ZERO || index == (ZERO + SIXTEEN))
			{
				index = ZERO;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index] = data;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index + SIXTEEN] = data;
			}
			if (index == FOUR || index == (FOUR + SIXTEEN))
			{
				index = FOUR;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index] = data;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index + SIXTEEN] = data;
			}
			if (index == EIGHT || index == (EIGHT + SIXTEEN))
			{
				index = EIGHT;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index] = data;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index + SIXTEEN] = data;
			}
			if (index == TWELVE || index == (TWELVE + SIXTEEN))
			{
				index = TWELVE;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index] = data;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index + SIXTEEN] = data;
			}
			else
			{
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index] = data;
			}
		}
	}
	else
	{
		FATAL("Unknow Memory Access Source : %d", TO_UINT(source));
		RETURN;
	}
}
#pragma endregion RP2C02_DEFINITIONS

#pragma region RP2A03_DEFINITIONS
void NES_t::cpuSetRegister(REGISTER_TYPE rt, uint16_t u16parameter)
{
	switch (rt)
	{
		// Normal Register access
	case REGISTER_TYPE::RT_A:
	{
		pNES_cpuRegisters->a = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_X:
	{
		pNES_cpuRegisters->x = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_Y:
	{
		pNES_cpuRegisters->y = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_P:
	{
		pNES_cpuRegisters->p.p = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_SP:
	{
		pNES_cpuRegisters->sp = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_PC:
	{
		pNES_cpuRegisters->pc = u16parameter & 0xFFFF; BREAK;
	}
	case REGISTER_TYPE::RT_NONE:
	{
		BREAK;
	}
	default:
	{
		BREAK;
	}
	}
}

uint16_t NES_t::cpuReadRegister(REGISTER_TYPE rt)
{
	switch (rt)
	{
		// Normal Register access
	case REGISTER_TYPE::RT_A:
	{
		RETURN(pNES_cpuRegisters->a & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_X:
	{
		RETURN(pNES_cpuRegisters->x & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_Y:
	{
		RETURN(pNES_cpuRegisters->y & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_P:
	{
		RETURN(pNES_cpuRegisters->p.p & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_SP:
	{
		RETURN(pNES_cpuRegisters->sp & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_PC:
	{
		RETURN(pNES_cpuRegisters->pc & 0xFFFF); BREAK;
	}

	case REGISTER_TYPE::RT_NONE:
	{
		RETURN(uint16_t)NULL;  BREAK;
	}
	default:
	{
		RETURN(uint16_t)NULL; BREAK;
	}
	}
}

byte NES_t::readCpuRawMemory(uint16_t address, MEMORY_ACCESS_SOURCE source)
{
	if (ROM_TYPE == ROM::TEST_ROM_BIN)
	{
		RETURN pNES_cpuMemory->NESRawMemory[address];
	}

	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType = TYPE_OF_MEMORY_ACCESS::CPU_READ;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousCPUAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentCPUAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentCPUAccessType = TYPE_OF_MEMORY_ACCESS::CPU_READ;

	if (ROM_TYPE == ROM::TEST_SST)
	{

		auto data = pNES_cpuMemory->NESRawMemory[address];

		auto index = pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.indexer;
		pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].address = address;
		pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].data = data;
		pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].isRead = YES;
		++pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.indexer;

		RETURN data;
	}
	else
	{
		if (source == MEMORY_ACCESS_SOURCE::CPU || source == MEMORY_ACCESS_SOURCE::DMA || source == MEMORY_ACCESS_SOURCE::DEBUG_PORT)
		{
			if (IF_ADDRESS_WITHIN(address, RAM_START_ADDRESS, RAM_MIRROR3_END_ADDRESS))
			{
				address -= RAM_START_ADDRESS;
				auto index = address & (CPU_RAM_SIZE - ONE); // % 0x0800
				RETURN pNES_cpuMemory->NESMemoryMap.wram[index];
			}
			else if (IF_ADDRESS_WITHIN(address, PPU_START_ADDRESS, PPU_MIRROR_END_ADDRESS))
			{
				address -= PPU_START_ADDRESS;
				auto index = address & (PPU_CTRL_REG_SIZE - ONE); // % 0x0008

				// First process anything that needs to be done in PPU when CPU reads the PPU registers 

				switch (index + PPU_START_ADDRESS)
				{
				case PPU_CTRL_ADDRESS:
				{
					RETURN pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue;
				}
				case PPU_MASK_ADDRESS:
				{
					RETURN pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue;
				}
				case PPU_STATUS_ADDRESS:
				{
					pNES_ppuRegisters->ppuInternalRegisters.w = FIRST_WRITE; // Reading PPU status register clears the PPU internal W register

					// First, fill the open bus values
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS.raw = pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue;

					// Next, override the appropriate status bits with actual values
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS.ppustatus.VBLANK = pNES_ppuRegisters->vblank;
					pNES_ppuRegisters->vblank = CLEAR;
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS.ppustatus.SPRITE_0_HIT = pNES_ppuRegisters->sprite0hit;
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS.ppustatus.SPRITE_OVERFLOW = pNES_ppuRegisters->spriteOverflow;

					// Refer : https://www.nesdev.org/wiki/PPU_frame_timing
					SCOUNTER64 cycle = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;
					SCOUNTER64 ly = pNES_instance->NES_state.display.currentScanline;

					// NOTE: VBLANK is set @ ly == POST_RENDER_SCANLINE && cycle == ONE
					// For 5.nmi_suppression.nes
					if (ly == NES_POST_RENDER_SCANLINE && cycle >= ONE && cycle <= THREE)
					{
						// ly = POST_RENDER_SCANLINE && cycle == 1 : Reading flag 1 PPU clock before set should suppress NMI
						// ly = POST_RENDER_SCANLINE && cycle == 2 : Reading flag when it's set should suppress NMI
						// ly = POST_RENDER_SCANLINE && cycle == 3 : Reading flag 1 PPU clock after set should suppress NMI
						pNES_instance->NES_state.interrupts.isNMI = CLEAR;
						pNES_instance->NES_state.interrupts.nmiDelayInInstructions = RESET;
					}
					// For 2.vbl_timing.nes
					if (ly == NES_POST_RENDER_SCANLINE && cycle == ONE)
					{
						// ly = POST_RENDER_SCANLINE && cycle == 1 : Reading 1 PPU clock before VBL should suppress setting
						pNES_ppuRegisters->ppuStatusReadQuirkEnable = YES;
					}

					pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS.raw;
					RETURN pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS.raw;
				}
				case OAM_ADDR_ADDRESS:
				{
					RETURN pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue;
				}
				case OAM_DATA_ADDRESS:
				{
					auto data = pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR];

					// To handle 10th test of ppu_open_bus.nes
					// OAMADDR can go from 0 - 255, each sprite has 4 bytes allocated to it
					// So, we have 64 entries of 4 bytes (0th, 1st, 2nd and 3rd byte)
					// For any entry, if we access "2nd byte", then bits 2-4 should be cleared to zero
					// Therefore, OAMADDR of 2, 6, 10 ... should have their bits 2-4 should be cleared to zero

					// We check for this by doing OAMADDR % 4 == 2 (checking for remainder 2)
					// Therefore ((OAMADDR & 0x03) == 2)

					if ((pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR & 0x03) == TWO)
					{
						// Can also refer "Byte 2" section of https://www.nesdev.org/wiki/PPU_OAM
						data &= 0xE3;
					}

					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMDATA = data;

					pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMDATA;
					RETURN pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMDATA;
				}
				case PPU_SCROLL_ADDRESS:
				{
					RETURN pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue;
				}
				case PPU_ADDR_ADDRESS:
				{
					clockMMC3IRQ(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU, NO);
					RETURN pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue;
				}
				case PPU_DATA_ADDRESS:
				{
					// Refer : https://www.nesdev.org/wiki/PPU_registers#PPUDATA
					auto data = pNES_ppuRegisters->ppuInternalRegisters.cpu2ppu;

					pNES_ppuRegisters->ppuInternalRegisters.cpu2ppu
						= readPpuRawMemory(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU);
					if (IF_ADDRESS_WITHIN(pNES_ppuRegisters->ppuInternalRegisters.v.raw, PALETTE_RAM_INDEXES_START_ADDRESS, PALETTE_RAM_INDEXES_MIRROR_END_ADDRESS))
					{
						data = pNES_ppuRegisters->ppuInternalRegisters.cpu2ppu;

						// This is needed to handle vram_access.nes
						// Implemented based on https://forums.nesdev.org/viewtopic.php?p=79492#p79492
						// This is hinted @ "Reading palette RAM" section of https://www.nesdev.org/wiki/PPU_registers#PPUDATA_-_VRAM_data_($2007_read/write)

						pNES_ppuRegisters->ppuInternalRegisters.cpu2ppu
							= readPpuRawMemory(pNES_ppuRegisters->ppuInternalRegisters.v.raw - 0x1000, MEMORY_ACCESS_SOURCE::CPU);

						// For PPU Openbus
						// Extract bits 6 and 7 from openBusValue
						auto bits6And7 = (pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue & 0xC0);
						// Clear bits 6 and 7 in data
						data &= 0x3F;
						// Insert the extracted bits 6 and 7 to corresponding position in data
						data |= bits6And7;
					}

					if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.VRAM_ADDRESS_INCREMENT == RESET)
					{
						pNES_ppuRegisters->ppuInternalRegisters.v.raw += ONE;
						clockMMC3IRQ(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU, NO);
					}
					else
					{
						pNES_ppuRegisters->ppuInternalRegisters.v.raw += THIRTYTWO;
						clockMMC3IRQ(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU, NO);
					}

					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUDATA = data;

					pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUDATA;
					RETURN pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUDATA;
				}
				default:
				{
					FATAL("Unknown PPU register");
				}
				}

				pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue = pNES_cpuMemory->NESMemoryMap.ppuCtrl.raw[index];
				RETURN pNES_cpuMemory->NESMemoryMap.ppuCtrl.raw[index];
			}
			else if (IF_ADDRESS_WITHIN(address, APU_AND_IO_START_ADDRESS, APU_AND_IO_END_ADDRESS))
			{
				if (address >= APU_AND_IO_START_ADDRESS && address <= 0x4014)
				{
					// To handle test_cpu_exec_space_apu.nes
					// NOTE : "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be RETURNed
					// Maybe this is why test_cpu_exec_space_apu.nes is passing. But this needs further investigation
					RETURN(address >> EIGHT);
				}
				else if (address == APU_STATUS_ADDRESS)
				{
					SND_CHN_t SND_CHN = { RESET };

					// Get the status of DMC_INTR
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.DMC_INTR = pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_DMC;

					SND_CHN.raw = pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.raw;
					if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET)
					{
						SND_CHN.PULSE1 = NO;
					}
					if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET)
					{
						SND_CHN.PULSE2 = NO;
					}
					if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
					{
						SND_CHN.TRIANGLE = NO;
					}
					if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET)
					{
						SND_CHN.NOISE = NO;
					}
					if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].lengthCounter == RESET)
					{
						SND_CHN.DMC_ENABLE = NO;
					}
					auto apuStatus = SND_CHN.raw;

					// Reading 0x4015 clears the FRAME_INTR (and hence the corresponding bit in the status registers as well)
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_FRAMECTR = RESET;
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.FRAME_INTR = CLEAR;

					RETURN apuStatus;
				}
				if (address == JOYSTICK1_ADDRESS)
				{
					if (pNES_instance->NES_state.controller.startPolling == YES && pNES_instance->NES_state.controller.endPolling == NO)
					{
						// Refer https://www.nesdev.org/wiki/Standard_controller for reasons to OR with MSB of address
						RETURN((byte)ImGui::IsKeyDown(ImGuiKey_Z) | (address >> EIGHT));
					}
					else if ((pNES_instance->NES_state.controller.startPolling == NO && pNES_instance->NES_state.controller.endPolling == YES)
						&& (pNES_instance->NES_state.controller.keyID >= KEY_A && pNES_instance->NES_state.controller.keyID <= KEY_RIGHT))
					{
						auto data = GETBIT(pNES_instance->NES_state.controller.keyID, pNES_instance->NES_state.controller.keyStatus);
						++pNES_instance->NES_state.controller.keyID;
						// Refer https://www.nesdev.org/wiki/Standard_controller for reasons to OR with MSB of address
						RETURN(data | (address >> EIGHT));
					}
					else
					{
						pNES_instance->NES_state.controller.keyID = INVALID;
						// To handle test_cpu_exec_space_apu.nes
						// "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be RETURNed
						// Also refer https://www.nesdev.org/wiki/Standard_controller for reasons to OR with MSB of address
						RETURN((address >> EIGHT) | 0x01);
					}
				}
				if (address == JOYSTICK2_OR_FRAMECFG_ADDRESS)
				{
					if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::GxROM)
					{
						typedef union
						{
							struct
							{
								byte S : 1; // bit 0
								byte unused0 : 2; // bits 1 - 2
								byte W : 1; // bit 3
								byte T : 1; // bit 4
								byte unused1 : 3; // bits 5 - 7
							} fields;
							byte raw;
						} zapper_t;

						zapper_t zapper = { ZERO };

						float x = 0.0f, y = 0.0f;
						getMouseRelPosIfDocked(&x, &y, getScreenWidth(), getScreenHeight());

						ID paletteID = pNES_instance->NES_state.display.gfxColorID[(uint8_t)(x)][(uint8_t)(y)];

						// NOTE : As per https://www.nesdev.org/wiki/Zapper
						// We need to detect the white color generated by the game
						// Now based on PPU palettes (https://www.nesdev.org/wiki/PPU_palettes), white is index 0x20 and 0x30
						// So, we check for these 2 palettes as a way to detect the white color!

						if (paletteID == 0x20 || paletteID == 0x30)
						{
							zapper.fields.W = RESET;
						}
						else
						{
							zapper.fields.W = SET;
						}

						if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
						{
							zapper.fields.T = RESET;
						}
						else
						{
							zapper.fields.T = SET;
						}

						RETURN zapper.raw;
					}
					else
					{
						// To handle test_cpu_exec_space_apu.nes
						// "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be RETURNed
						// Refer https://www.nesdev.org/wiki/Standard_controller for reasons to OR with MSB of address
						RETURN(address >> EIGHT);
					}
				}

				FATAL("Invalid APU Address");
			}
			else if (IF_ADDRESS_WITHIN(address, OTHER_APU_AND_IO_START_ADDRESS, OTHER_APU_AND_IO_END_ADDRESS))
			{
				// To handle test_cpu_exec_space_apu.nes
				// NOTE : "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be RETURNed
				// Maybe this is why test_cpu_exec_space_apu.nes is passing. But this needs further investigation
				RETURN(address >> EIGHT);
			}
			else if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, UNMAPPED_END_ADDRESS))
			{
				// To handle test_cpu_exec_space_apu.nes
				// NOTE : "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be RETURNed
				// Maybe this is why test_cpu_exec_space_apu.nes is passing. But this needs further investigation
				if (address >= 0x4020 && address <= 0x40FF)
				{
					RETURN(address >> EIGHT);
				}

				int16_t modedData = RESET;
				int16_t compareForMod = RESET;
				uint32_t index = RESET;

				switch (pNES_instance->NES_state.catridgeInfo.mapper)
				{
				case MAPPER::NROM:
				{
					if ((ceNES->interceptCPURead(address, &modedData, &compareForMod))
						&&
						((compareForMod == INVALID)
							||
							((compareForMod != INVALID)
								&& ((BYTE)compareForMod == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS]))))
					{
						RETURN TO_UINT8(modedData);
					}
					else
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}
				}
				case MAPPER::MMC1:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.mmc1.prgRamEnable == YES)
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
						else
						{
							RETURN(address >> EIGHT);
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						switch (pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.fields1.pp)
						{
						case ZERO:
						case ONE:
						{
							// NOTE: "prgBank32 * 0x4000" is used instead of "prgBank32 * 0x8000" as bank IDs are based on 16KB mode even when 32KB mode is selected
							index = pNES_instance->NES_state.catridgeInfo.mmc1.prgBank32 * 0x4000;
							index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x7FFF);
							BREAK;
						}
						case TWO:
						case THREE:
						{
							if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
							{
								index = pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Lo * 0x4000;
								index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x3FFF);
							}
							else if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK1_START_ADDRESS, CATRIDGE_ROM_BANK1_END_ADDRESS))
							{
								index = pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Hi * 0x4000;
								index += ((address - CATRIDGE_ROM_BANK1_START_ADDRESS) & 0x3FFF);
							}
							else
							{
								FATAL("Invalid Memory Region for MMC1");
							}
							BREAK;
						}
						}

						if ((ceNES->interceptCPURead(address, &modedData, &compareForMod))
							&&
							((compareForMod == INVALID)
								||
								((compareForMod != INVALID)
									&& ((BYTE)compareForMod == pNES_catridgeMemory->maxCatridgePRGROM[index]))))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
					}
					BREAK;
				}
				case MAPPER::UxROM_002:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
					{
						index = pNES_instance->NES_state.catridgeInfo.uxrom_002.prgBank16 * 0x4000;
						index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x3FFF);
						if ((ceNES->interceptCPURead(address, &modedData, &compareForMod))
							&&
							((compareForMod == INVALID)
								||
								((compareForMod != INVALID)
									&& ((BYTE)compareForMod == pNES_catridgeMemory->maxCatridgePRGROM[index]))))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK1_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if ((ceNES->interceptCPURead(address, &modedData, &compareForMod))
							&&
							((compareForMod == INVALID)
								||
								((compareForMod != INVALID)
									&& ((BYTE)compareForMod == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS]))))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
					}
					BREAK;
				}
				case MAPPER::CNROM:
				{
					if ((ceNES->interceptCPURead(address, &modedData, &compareForMod))
						&&
						((compareForMod == INVALID)
							||
							((compareForMod != INVALID)
								&& ((BYTE)compareForMod == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS]))))
					{
						RETURN TO_UINT8(modedData);
					}
					else
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}
				}
				case MAPPER::MMC3:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.fields.prgRamEnable == SET)
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
						RETURN(address >> EIGHT);
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						BIT prgRomBankMode = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.prgRomMode;

						auto startAddr1 = CATRIDGE_ROM_BANK0_START_ADDRESS;
						auto endAddr1 = startAddr1 + 0x1FFF;
						auto startAddr2 = CATRIDGE_ROM_BANK1_START_ADDRESS;
						auto endAddr2 = startAddr2 + 0x1FFF;

						if (prgRomBankMode == SET)
						{
							startAddr1 = CATRIDGE_ROM_BANK1_START_ADDRESS;
							endAddr1 = startAddr1 + 0x1FFF;
							startAddr2 = CATRIDGE_ROM_BANK0_START_ADDRESS;
							endAddr2 = startAddr2 + 0x1FFF;
						}

						if (IF_ADDRESS_WITHIN(address, startAddr1, endAddr1))
						{
							index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8a * 0x2000;
							index += ((address - startAddr1) & 0x1FFF);

						}
						else if (IF_ADDRESS_WITHIN(address, 0xA000, 0xBFFF))
						{
							index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8b * 0x2000;
							index += ((address - 0xA000) & 0x1FFF);
						}
						else if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
						{
							// Note that sizeOfPrgRom is in 16KB units
							// sizeOfPrgRom - ONE is the start address of last 16 KB bank
							// Hence sizeOfPrgRom - ONE is also the start address of second last 8KB bank
							index = (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB - ONE) * 0x4000;
							index += ((address - startAddr2) & 0x1FFF);
						}
						else if (IF_ADDRESS_WITHIN(address, 0xE000, 0xFFFF))
						{
							// Same as what we did for last but 1 bank, but add another 0x2000 to get the last bank
							index = (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB - ONE) * 0x4000;
							index += 0x2000;
							index += ((address - 0xE000) & 0x1FFF);
						}
						else
						{
							FATAL("Invalid PRG ROM address in MMC3");
						}

						if ((ceNES->interceptCPURead(address, &modedData, &compareForMod))
							&&
							((compareForMod == INVALID)
								||
								((compareForMod != INVALID)
									&& ((BYTE)compareForMod == pNES_catridgeMemory->maxCatridgePRGROM[index]))))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
					}
					BREAK;
				}
				case MAPPER::AxROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						index = pNES_instance->NES_state.catridgeInfo.axrom.prgBank * 0x8000;
						index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x7FFF);
						if ((ceNES->interceptCPURead(address, &modedData, &compareForMod))
							&&
							((compareForMod == INVALID)
								||
								((compareForMod != INVALID)
									&& ((BYTE)compareForMod == pNES_catridgeMemory->maxCatridgePRGROM[index]))))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
					}
					BREAK;
				}
				case MAPPER::GxROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						index = pNES_instance->NES_state.catridgeInfo.gxrom.prgBank * 0x8000;
						index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x7FFF);
						if ((ceNES->interceptCPURead(address, &modedData, &compareForMod))
							&&
							((compareForMod == INVALID)
								||
								((compareForMod != INVALID)
									&& ((BYTE)compareForMod == pNES_catridgeMemory->maxCatridgePRGROM[index]))))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
					}
					BREAK;
				}
				case MAPPER::NANJING_FC001:
				{
					if (address == 0x5000)
					{
						FATAL("Reading from write only register ??? for mapper 163 at %d of %s", __LINE__, __FILE__);
					}
					else if (IF_ADDRESS_WITHIN(address, 0x5100, 0x5101))
					{
						FATAL("Reading from write only register ??? for mapper 163 at %d of %s", __LINE__, __FILE__);
					}
					else if (address == 0x5200)
					{
						FATAL("Reading from write only register ??? for mapper 163 at %d of %s", __LINE__, __FILE__);
					}
					else if (address == 0x5300)
					{
						FATAL("Reading from write only register ??? for mapper 163 at %d of %s", __LINE__, __FILE__);
					}
					else if (IF_ADDRESS_WITHIN(address, 0x5500, 0x5501))
					{
						// Refer to https://www.nesdev.org/wiki/INES_Mapper_163#Feedback_Read_($5500-$5501,_read)
						if (pNES_instance->NES_state.catridgeInfo.nanjing_fc001.F)
						{
							RETURN ZERO;
						}
						RETURN 0x04;
					}
					// PRG RAM
					else if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}
					// PRG ROM
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						index = pNES_instance->NES_state.catridgeInfo.nanjing_fc001.prgRomBank.raw * 0x8000;
						index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x7FFF);
						if ((ceNES->interceptCPURead(address, &modedData, &compareForMod))
							&& compareForMod != INVALID
							&& (BYTE)compareForMod == pNES_catridgeMemory->maxCatridgePRGROM[index])
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
					}
					BREAK;
				}
				default:
				{
					FATAL("Read performed for unsupported mapper");
				}
				}
			}
		}

		FATAL("Unknown Memory Access Source : %d", TO_UINT(source));
		RETURN(byte)ZERO;
	}
}

void NES_t::writeCpuRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source)
{
	if (ROM_TYPE == ROM::TEST_ROM_BIN)
	{
		pNES_cpuMemory->NESRawMemory[address] = data;
		RETURN;
	}

	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType = TYPE_OF_MEMORY_ACCESS::CPU_WRITE;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousCPUAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentCPUAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentCPUAccessType = TYPE_OF_MEMORY_ACCESS::CPU_WRITE;

	if (ROM_TYPE == ROM::TEST_SST)
	{

		auto index = pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.indexer;
		pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].address = address;
		pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].data = data;
		pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].isRead = NO;
		++pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.indexer;

		pNES_cpuMemory->NESRawMemory[address] = data;
	}
	else
	{ 
		if (source == MEMORY_ACCESS_SOURCE::CPU || source == MEMORY_ACCESS_SOURCE::DMA || source == MEMORY_ACCESS_SOURCE::DEBUG_PORT)
		{
			if (IF_ADDRESS_WITHIN(address, RAM_START_ADDRESS, RAM_MIRROR3_END_ADDRESS))
			{
				address -= RAM_START_ADDRESS;
				auto index = address & (CPU_RAM_SIZE - ONE); // % 0x0800
				pNES_cpuMemory->NESMemoryMap.wram[index] = data;
			}
			else if (IF_ADDRESS_WITHIN(address, PPU_START_ADDRESS, PPU_MIRROR_END_ADDRESS))
			{
				address -= PPU_START_ADDRESS;
				auto index = address & (PPU_CTRL_REG_SIZE - ONE); // % 0x0008

				FLAG wasNmiSet = ((pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.VBLANK_NMI_ENABLE == SET) ? YES : NO);

				// Update this in CPU's perspective 

				auto originalStatus = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS;
				if ((index + PPU_START_ADDRESS) != PPU_STATUS_ADDRESS)
				{
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.raw[index] = data;
				}

				// Now update in PPU's perspective as well...

				switch (index + PPU_START_ADDRESS)
				{
				case PPU_CTRL_ADDRESS:
				{
					// Refer "$2000 (PPUCTRL) write" of https://www.nesdev.org/wiki/PPU_scrolling
					pNES_ppuRegisters->ppuInternalRegisters.t.fields.nameTblSelectH
						= pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.BASE_NAMETABLE_ADDR_H;

					pNES_ppuRegisters->ppuInternalRegisters.t.fields.nameTblSelectV
						= pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.BASE_NAMETABLE_ADDR_V;

					// Needed for 9th test of 7.nmi_timing.nes, i.e. "NMI enabled when VBL already set should delay 1 instruction"
					// Also needed for 11th test of 04-nmi_control.nes, i.e. "Immediate occurence should be after NEXT instruction"
					if (wasNmiSet == NO && pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.VBLANK_NMI_ENABLE == SET // Rising Edge (but actually is a falling edge in HW)
						&& pNES_ppuRegisters->vblank == SET && pNES_instance->NES_state.interrupts.isNMI == NO)
					{
						pNES_instance->NES_state.interrupts.isNMI = YES;
						pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE; // Delay by 1 instruction
					}

					// NOTE: Masquerade (NES) provides only cpu cycle accuracy; micro accuracy is not currently possible
					// What this means is, basic unit is 1 cpu cycle, but since in actual NES, basic unit is 1 ppu cycle AND 1 cpu cycle = 3 ppu cycle
					// With the current method of implementation, there is always a possibility of cpu and ppu being out of sync by atmost +/- 3 ppu cycles
					// The NMI on tests validates timing @ ppu cycle accuracy
					// i.e. as per readme:-
					//  ppu cycle offset		NMI occurence
					//	00						N
					//	01						N
					//	02						N
					//	03						N
					//	04						N
					//	05						-
					//	06						-
					//	07						-
					//	08						-
					// With current design, we can control with minimum resolution of about 3 ppu cycles
					// If any even occurs @ 00 offset (in our case, VBL is cleared), we can react to it only @ cycles 02 or 05 or 08 and so on...
					// But since this test is expecting even lower resolution i.e. @ 04 cycle...
					// We need to implement hacks such as below to pass the test
					// From experiment, we observed nmi suppression @ cycle 1 (same cycle as when VBL gets cleared within ppu tick) helps in passing 07-nmi_on_timing.nes
					// Refer https://forums.nesdev.org/viewtopic.php?p=160705#p160705
					// Refer https://forums.nesdev.org/viewtopic.php?p=160582#p160582
					// FYI -> Condition for LY is added as the hack is needed only when VBL was just cleared which happens in LY = -1
					if (pNES_instance->NES_state.display.currentScanline == NES_PRE_RENDER_SCANLINE
						&& pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY == ONE)
					{
						pNES_instance->NES_state.interrupts.isNMI = wasNmiSet; // Revert the NMI to original state
					}

					BREAK;
				}
				case PPU_MASK_ADDRESS:
				{
					BREAK;
				}
				case PPU_STATUS_ADDRESS:
				{
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS = originalStatus; // We have made sure above that status cannot be written, but just as a precaution
					BREAK;
				}
				case OAM_ADDR_ADDRESS:
				{
					BREAK;
				}
				case OAM_DATA_ADDRESS:
				{
					if (pNES_instance->NES_state.display.currentScanline >= NES_PRE_RENDER_SCANLINE
						&&
						pNES_instance->NES_state.display.currentScanline <= NES_LAST_VISIBLE_PPU_SCANLINE
						&&
						(pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET
							|| pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET)
						)
					{
						// As mentioned in https://www.nesdev.org/wiki/PPU_registers#OAMDATA
						// For emulation purpose, its best to completely ignore the writes when rendering is enabled
					}
					else
					{
						pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR]
							= pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMDATA;

						pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR += ONE;
						pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR &= 0xFF;
					}
					BREAK;
				}
				case PPU_SCROLL_ADDRESS:
				{
					if (pNES_ppuRegisters->ppuInternalRegisters.w == FIRST_WRITE)
					{
						pNES_ppuRegisters->ppuInternalRegisters.t.fields.coarseXScroll = (data >> THREE);
						pNES_ppuRegisters->ppuInternalRegisters.x = (data & 0x07);
						pNES_ppuRegisters->ppuInternalRegisters.w = SECOND_WRITE;
					}
					else if (pNES_ppuRegisters->ppuInternalRegisters.w == SECOND_WRITE)
					{
						pNES_ppuRegisters->ppuInternalRegisters.t.fields.coarseYScroll = (data >> THREE);
						pNES_ppuRegisters->ppuInternalRegisters.t.fields.fineYScroll = (data & 0x07);
						pNES_ppuRegisters->ppuInternalRegisters.w = FIRST_WRITE;
					}
					BREAK;
				}
				case PPU_ADDR_ADDRESS:
				{
					if (pNES_ppuRegisters->ppuInternalRegisters.w == FIRST_WRITE)
					{
						pNES_ppuRegisters->ppuInternalRegisters.t.addr.hi = (data & 0x3F);
						pNES_ppuRegisters->ppuInternalRegisters.w = SECOND_WRITE;
					}
					else if (pNES_ppuRegisters->ppuInternalRegisters.w == SECOND_WRITE)
					{
						pNES_ppuRegisters->ppuInternalRegisters.t.addr.lo = data;
						pNES_ppuRegisters->ppuInternalRegisters.v.raw = pNES_ppuRegisters->ppuInternalRegisters.t.raw;
						clockMMC3IRQ(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU, YES);
						pNES_ppuRegisters->ppuInternalRegisters.w = FIRST_WRITE;
					}

					BREAK;
				}
				case PPU_DATA_ADDRESS:
				{
					writePpuRawMemory(pNES_ppuRegisters->ppuInternalRegisters.v.raw, data, MEMORY_ACCESS_SOURCE::CPU);
					if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.VRAM_ADDRESS_INCREMENT == RESET)
					{
						pNES_ppuRegisters->ppuInternalRegisters.v.raw += ONE;
						clockMMC3IRQ(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU, YES);
					}
					else
					{
						pNES_ppuRegisters->ppuInternalRegisters.v.raw += THIRTYTWO;
						clockMMC3IRQ(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU, YES);
					}
					BREAK;
				}
				default:
				{
					FATAL("Unknown PPU register");
				}
				}

				pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue = data;
				pNES_ppuRegisters->ppuInternalRegisters.openBus.lastRefreshTimeInMs
					= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}
			else if (IF_ADDRESS_WITHIN(address, APU_AND_IO_START_ADDRESS, APU_AND_IO_END_ADDRESS))
			{
				if (address == 0x4000)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_VOL.raw = data;
					pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::PULSE_1)] = ONE;
					RETURN;
				}
				if (address == 0x4001)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_SWEEP.raw = data;
					// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].sweep.reload = YES;
					RETURN;
				}
				if (address == 0x4002)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_LO = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].frequencyPeriod.fields.lo = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].frequencyPeriod.fields.unused = RESET;
					RETURN;
				}
				if (address == 0x4003)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_HI.raw = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].frequencyPeriod.fields.hi =
						pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_HI.HIGH;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].frequencyPeriod.fields.unused = RESET;
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.PULSE1 == SET)
					{
						if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FOUR_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01))
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03)))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_HI.LENGTH_COUNTER & 0x1F];
							}
						}
						else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10))
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12)))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_HI.LENGTH_COUNTER & 0x1F];
							}
						}
					}
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].envelope.startFlag = YES;
					// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].frequencyCounter
						= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].frequencyPeriod.raw;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].dutyCounter = RESET;
					RETURN;
				}
				if (address == 0x4004)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_VOL.raw = data;
					pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::PULSE_2)] = ONE;
					RETURN;
				}
				if (address == 0x4005)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_SWEEP.raw = data;
					// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].sweep.reload = YES;
					RETURN;
				}
				if (address == 0x4006)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_LO = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].frequencyPeriod.fields.lo = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].frequencyPeriod.fields.unused = RESET;
					RETURN;
				}
				if (address == 0x4007)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_HI.raw = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].frequencyPeriod.fields.hi =
						pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_HI.HIGH;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].frequencyPeriod.fields.unused = RESET;
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.PULSE2 == SET)
					{
						if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FOUR_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01))
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03)))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_HI.LENGTH_COUNTER & 0x1F];
							}
						}
						else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10))
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12)))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_HI.LENGTH_COUNTER & 0x1F];
							}
						}
					}
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].envelope.startFlag = YES;
					// Refer https://forums.nesdev.org/viewtopic.php?p=163102#p163102
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].frequencyCounter
						= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].frequencyPeriod.raw;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].dutyCounter = RESET;
					RETURN;
				}
				if (address == 0x4008)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_LINEAR.raw = data;
					pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)] = ONE;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.linearCounter.linearCounterReload
						= pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_LINEAR.LINEAR_COUNTER;
					RETURN;
				}
				if (address == 0x400A)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_LO = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyPeriod.fields.lo = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyPeriod.fields.unused = RESET;
					RETURN;
				}
				if (address == 0x400B)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_HI.raw = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyPeriod.fields.hi =
						pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_HI.HIGH;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].frequencyPeriod.fields.unused = RESET;
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.TRIANGLE == SET)
					{
						if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FOUR_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01))
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03)))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_HI.LENGTH_COUNTER & 0x1F];
							}
						}
						else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10))
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12)))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_HI.LENGTH_COUNTER & 0x1F];
							}
						}
					}
					// Refer https://forums.nesdev.org/viewtopic.php?p=163155#p163155 for pseudocode
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].triangle.linearCounter.linearReload = YES;
					RETURN;
				}
				if (address == 0x400C)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_VOL.raw = data;
					pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::NOISE)] = ONE;
					RETURN;
				}
				if (address == 0x400E)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_PERIOD.raw = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseFrequencyPeriod
						= NOISE_PERIOD_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_PERIOD.LINEAR_COUNTER];
					RETURN;
				}
				if (address == 0x400F)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_LENGTH_COUNTER.raw = data;
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.NOISE == SET)
					{
						if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FOUR_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01))
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03)))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_LENGTH_COUNTER.LENGTH_COUNTER & 0x1F];
							}
						}
						else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12))
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10))
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12)))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_LENGTH_COUNTER.LENGTH_COUNTER & 0x1F];
							}
						}
					}
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].envelope.startFlag = YES;
					RETURN;
				}
				if (address == 0x4010)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_FREQ.raw = data;
					// "Frequency Period = DMC_PERIOD_LUT[v.3210] - 1" instead of "Frequency Period = DMC_PERIOD_LUT[v.3210]" helps in passing "8-dmc_rates.nes" test
					APUTODO("Find the reason for need to subtract DMC_PERIOD_LUT[v.3210] by 1");
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcFrequencyPeriod
						= DMC_PERIOD_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_FREQ.FREQUENCY_INDEX] - ONE;
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_FREQ.IRQ_ENABLE == RESET)
					{
						pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_DMC = RESET;
					}
					RETURN;
				}
				if (address == 0x4011)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_RAW.raw = data;
					// Refer : https://www.nesdev.org/wiki/APU_DMC
					APUTODO("Need to handle edge case for DMC Direct Load");
					RETURN;
				}
				if (address == 0x4012)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_START = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleAddress
						= (0xC000 | (data << SIX));
					RETURN;
				}
				if (address == 0x4013)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.DMC_LEN = data;
					pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleLength
						= ((data << FOUR) + ONE);
					RETURN;
				}
				if (address == OAM_DMA_ADDRESS)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.OAMDMA = data;
					pNES_instance->NES_state.oamDMA.sourceAddress = ((pNES_cpuMemory->NESMemoryMap.apuAndIO.OAMDMA << EIGHT) & 0xFF00);
					pNES_instance->NES_state.oamDMA.DMAInProgress = YES;
					RETURN;
				}
				if (address == APU_STATUS_ADDRESS)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.rw = (data & 0x1F);

					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.PULSE1 == RESET)
					{
						pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter = RESET;
					}
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.PULSE2 == RESET)
					{
						pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter = RESET;
					}
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.TRIANGLE == RESET)
					{
						pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter = RESET;
					}
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.NOISE == RESET)
					{
						pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter = RESET;
					}
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.DMC_ENABLE == SET)
					{
						if (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].lengthCounter == RESET)
						{
							pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].lengthCounter
								= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleLength;
							pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcAddress
								= pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleAddress;
							pNES_instance->NES_state.dmcDMA.sourceAddress = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleAddress;
						}
					}
					else
					{
						pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].lengthCounter = RESET;
					}
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_DMC = RESET;
					RETURN;
				}
				else if (address == JOYSTICK1_ADDRESS)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY1 = data;
					pNES_instance->NES_state.controller.startPolling = ((data & 0x01) ? YES : NO);
					pNES_instance->NES_state.controller.endPolling = ((data & 0x01) ? NO : YES);
					if (pNES_instance->NES_state.controller.startPolling == YES && pNES_instance->NES_state.controller.endPolling == NO)
					{
						pNES_instance->NES_state.controller.keyID = KEY_A;
						captureIO();
					}
					RETURN;
				}
				else if (address == JOYSTICK2_OR_FRAMECFG_ADDRESS)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.raw = data;
					if (pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.FRAME_CONFIG.DIS_FRAME_INTR == SET)
					{
						pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.FRAME_INTR = RESET;
						pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_FRAMECTR = RESET;
					}

					// Handles 04.clock_jitter.nes of blargg apu tests
					if (GETBIT(ZERO, pNES_instance->NES_state.emulatorStatus.ticks.apuCounter) == SET)
					{
						pNES_instance->NES_state.audio.cyclesToSequencerModeChange = ONE;
					}
					else
					{
						pNES_instance->NES_state.audio.cyclesToSequencerModeChange = RESET;
						pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer = RESET;
						pNES_instance->NES_state.audio.frameSequencerMode
							= (FRAME_SEQUENCER_MODE)pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.FRAME_CONFIG.FRAME_SEQ_MODE;
					}
					RETURN;
				}

				pNES_cpuMemory->NESMemoryMap.apuAndIO.raw[address - APU_AND_IO_START_ADDRESS] = data;
				RETURN;
			}
			else if (IF_ADDRESS_WITHIN(address, OTHER_APU_AND_IO_START_ADDRESS, OTHER_APU_AND_IO_END_ADDRESS))
			{
				pNES_cpuMemory->NESMemoryMap.otherApuAndIO[address - OTHER_APU_AND_IO_START_ADDRESS] = data;
			}
			else if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, UNMAPPED_END_ADDRESS))
			{
				switch (pNES_instance->NES_state.catridgeInfo.mapper)
				{
				case MAPPER::NROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					BREAK;
				}
				case MAPPER::MMC1:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.mmc1.prgRamEnable == YES)
						{
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (GETBIT(SEVEN, data) == SET)
						{
							pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.raw = RESET;
							pNES_instance->NES_state.catridgeInfo.mmc1.clrWriteCount = RESET;
							pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.fields1.pp = THREE; // Resetting to prg rom bank 3
						}
						else
						{
							if ((pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType == pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousAccessType)
								&& pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType == TYPE_OF_MEMORY_ACCESS::CPU_WRITE)
							{
								// Note : This is to support the "Consecutive-cycle writes" section mentioned in https://www.nesdev.org/wiki/MMC1
							}
							else
							{
								++pNES_instance->NES_state.catridgeInfo.mmc1.clrWriteCount;
								pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.raw >>= ONE;
								pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.fields1.data4 = GETBIT(ZERO, data);

								if (pNES_instance->NES_state.catridgeInfo.mmc1.clrWriteCount == FIVE)
								{
									BYTE targetRegister = (address >> THIRTEEN) & THREE;

									switch (targetRegister)
									{
									case ZERO: // CONTROL $8000-$9FFF
									{
										pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.raw
											= pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.fields2.shiftValue;

										switch (pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.fields1.mm)
										{
										case ZERO:
										{
											pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
											BREAK;
										}
										case ONE:
										{
											pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR;
											BREAK;
										}
										case TWO:
										{
											pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
											BREAK;
										}
										case THREE:
										{
											pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
											BREAK;
										}
										}
										BREAK;
									}
									case ONE: // CHR bank 0 $A000-$BFFF
									{
										// 8KB mode
										if (pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.fields1.c == RESET)
										{
											pNES_instance->NES_state.catridgeInfo.mmc1.chrBank8
												= (pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.fields2.shiftValue & 0x1E);
										}
										// 4KB mode
										else
										{
											pNES_instance->NES_state.catridgeInfo.mmc1.chrBank4Lo
												= (pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.fields2.shiftValue & 0x1F);
										}

										BREAK;
									}
									case TWO: // CHR bank 1 $C000-$DFFF
									{
										if (pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.fields1.c == SET)
										{
											pNES_instance->NES_state.catridgeInfo.mmc1.chrBank4Hi
												= (pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.fields2.shiftValue & 0x1F);
										}

										BREAK;
									}
									case THREE: // PRG bank $E000-$FFFF
									{
										TODO("Find source for enabling of prgRamEnable on write to $E000-$FFFF without checking whether 4th bit is set as mentioned in wiki");
	#if (DISABLED)
										pNES_instance->NES_state.catridgeInfo.mmc1.prgRamEnable
											= (FLAG)((pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.fields2.shiftValue & 0x10) == 0x10);
	#else
										pNES_instance->NES_state.catridgeInfo.mmc1.prgRamEnable = YES;
	#endif

										switch (pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.fields1.pp)
										{
										case ZERO:
										case ONE:
										{
											pNES_instance->NES_state.catridgeInfo.mmc1.prgBank32
												= (pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.fields2.shiftValue & 0x0E);
											BREAK;
										}
										case TWO:
										{
											// Refer https://www.nesdev.org/wiki/MMC1#Consecutive-cycle_writes for 2: fix first bank at $8000 and switch 16 KB bank at $C000
											pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Lo = ZERO;
											pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Hi
												= (pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.fields2.shiftValue & 0x0F);
											BREAK;
										}
										case THREE:
										{
											// Refer https://www.nesdev.org/wiki/MMC1#Consecutive-cycle_writes for 3: fix last bank at $C000 and switch 16 KB bank at $8000
											pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Lo
												= (pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.fields2.shiftValue & 0x0F);

											// Last bank
											pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Hi = pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB - ONE;

											BREAK;
										}
										default:
										{
											FATAL("Invalid \"pp\"");
										}
										}

										BREAK;
									}
									}

									pNES_instance->NES_state.catridgeInfo.mmc1.clrWriteCount = RESET;
									pNES_instance->NES_state.catridgeInfo.mmc1.intfShiftReg.raw = RESET;
								}
							}
						}
					}
					BREAK;
				}
				case MAPPER::UxROM_002:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						pNES_instance->NES_state.catridgeInfo.uxrom_002.prgBank16 = data & 0x0F;
					}
					BREAK;
				}
				case MAPPER::CNROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						pNES_instance->NES_state.catridgeInfo.cnrom.chrBank8 = data;
					}
					BREAK;
				}
				case MAPPER::MMC3:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if ((pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.fields.prgRamEnable == SET)
							&& pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.fields.denyWrite == RESET)
						{
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						switch ((address & 0xF000))
						{
						case 0x8000:
						case 0x9000:
						{
							if (IS_EVEN(address))
							{
								pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.raw = data;
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankData_odd8k = data;

								switch (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.bankRegSel)
								{
								case ZERO:
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a = (data & 0xFE);
									BREAK;
								}
								case ONE:
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b = (data & 0xFE);
									BREAK;
								}
								case TWO:
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a = data;
									BREAK;
								}
								case THREE:
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b = data;
									BREAK;
								}
								case FOUR:
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c = data;
									BREAK;
								}
								case FIVE:
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d = data;
									BREAK;
								}
								case SIX:
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8a = (data & 0x3F);
									BREAK;
								}
								case SEVEN:
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8b = (data & 0x3F);
									BREAK;
								}
								default:
								{
									FATAL("Invalid MMC3 Bank Select");
									BREAK;
								}
								}
							}
							BREAK;
						}
						case 0xA000:
						case 0xB000:
						{
							if (IS_EVEN(address))
							{
								pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.mirroring_evenAk.raw = data;

								if (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.mirroring_evenAk.fields.isHorizontal == SET)
								{
									pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
								}
								else
								{
									pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
								}

								// Override to 1 screen mode is alternate Nametable is enabled in MMC3
								if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.alternativeNametable == SET)
								{
									pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
								}
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.raw = data;
							}
							BREAK;
						}
						case 0xC000:
						case 0xD000:
						{
							if (IS_EVEN(address))
							{
								pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqReload_evenCk = data;
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqReload_oddCk = data;
								pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter = RESET;
								pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqCounterReloadEnabled = YES;
							}
							BREAK;
						}
						case 0xE000:
						case 0xF000:
						{
							if (IS_EVEN(address))
							{
								pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqDisable_evenEk = data;
								pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqEnable = DISABLED;
								pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC3 = RESET;
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqEnable_oddEk = data;
								pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqEnable = ENABLED;
							}
							BREAK;
						}
						default:
						{
							FATAL("Invalid Address");
						}
						}
					}
					BREAK;
				}
				case MAPPER::AxROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						// Note: Nesdev mentions only first 3 bits to be considered for prgBank,
						// But according to https://forums.nesdev.org/viewtopic.php?p=79826#p79826, first four bits needs to be considered
						// Both Mesen and Fceux64 do the same as well
						pNES_instance->NES_state.catridgeInfo.axrom.prgBank = (data & 0x0F);
						pNES_instance->NES_state.catridgeInfo.axrom.vramPage = (((data & 0x10) == 0x10) ? YES : NO);
					}
					BREAK;
				}
				case MAPPER::GxROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						pNES_instance->NES_state.catridgeInfo.gxrom.chrBank = (data & 0x03);
						pNES_instance->NES_state.catridgeInfo.gxrom.prgBank = ((data >> FOUR) & 0x03);
					}
					BREAK;
				}
				case MAPPER::NANJING_FC001:
				{
					auto modifiedData01 = data;
					if (pNES_instance->NES_state.catridgeInfo.nanjing_fc001.B == (FLAG)SET)
					{
						modifiedData01 = (data & ~3) | ((data & 1) << 1) | ((data & 2) >> 1);
					}

					if (address == 0x5000)
					{
						pNES_instance->NES_state.catridgeInfo.nanjing_fc001.prgRomBank.fields.lo = (modifiedData01 & 0x0F);
						pNES_instance->NES_state.catridgeInfo.nanjing_fc001.chrRamAutoSwitch = ((modifiedData01 & 0x80) > ZERO);

						if (pNES_instance->NES_state.catridgeInfo.nanjing_fc001.A == (FLAG)RESET)
						{
							pNES_instance->NES_state.catridgeInfo.nanjing_fc001.prgRomBank.fields.lo |= 0x03;
						}

						BREAK;
					}
					else if (address == 0x5100)
					{
						// Refer to https://www.nesdev.org/wiki/INES_Mapper_163#Feedback_Write_($5100-$5101,_write)
						// if A = 0 (i.e. 0x5100), we simply latch E and F
						pNES_instance->NES_state.catridgeInfo.nanjing_fc001.E = (FLAG)((modifiedData01 & 0x01) > ZERO);
						pNES_instance->NES_state.catridgeInfo.nanjing_fc001.F = (FLAG)((modifiedData01 & 0x04) > ZERO);
						BREAK;
					}
					else if (address == 0x5101)
					{
						// Refer to https://www.nesdev.org/wiki/INES_Mapper_163#Feedback_Write_($5100-$5101,_write)
						// Latch the new E and then if A = 1 (i.e. 0x5101), if E is 1, flip F

						pNES_instance->NES_state.catridgeInfo.nanjing_fc001.E = (FLAG)((modifiedData01 & 0x01) > ZERO);
						if (pNES_instance->NES_state.catridgeInfo.nanjing_fc001.E == (FLAG)SET)
						{
							if (pNES_instance->NES_state.catridgeInfo.nanjing_fc001.F == NO)
							{
								pNES_instance->NES_state.catridgeInfo.nanjing_fc001.F = YES;
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.nanjing_fc001.F = NO;
							}
						}
						BREAK;
					}
					else if (address == 0x5200)
					{
						pNES_instance->NES_state.catridgeInfo.nanjing_fc001.prgRomBank.fields.hi = (modifiedData01 & 0x03);
						BREAK;
					}
					else if (address == 0x5300)
					{
						pNES_instance->NES_state.catridgeInfo.nanjing_fc001.B = (FLAG)((data & 0x01) > ZERO);
						pNES_instance->NES_state.catridgeInfo.nanjing_fc001.A = (FLAG)((data & 0x04) > ZERO);

						if (pNES_instance->NES_state.catridgeInfo.nanjing_fc001.A == (FLAG)RESET)
						{
							pNES_instance->NES_state.catridgeInfo.nanjing_fc001.prgRomBank.fields.lo |= 0x03;
						}

						BREAK;
					}
					else if (IF_ADDRESS_WITHIN(address, 0x5500, 0x5501))
					{
						FATAL("Writing to read only register ??? for mapper 163 at %d of %s", __LINE__, __FILE__);
					}

					// Write to actual memory as well after the above processing is done!
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					BREAK;
				}
				default:
				{
					FATAL("Read performed for unsupported mapper");
				}
				}
			}
		}
		else
		{
			FATAL("Unknown Memory Access Source : %d", TO_UINT(source));
			RETURN;
		}
	}
}

void NES_t::stackPush(BYTE data)
{
	writeCpuRawMemory(pNES_cpuRegisters->sp + 0x100, data, MEMORY_ACCESS_SOURCE::CPU);
	if (pNES_cpuRegisters->sp == 0x00)
	{
		pNES_cpuRegisters->sp = 0xFF;
	}
	else
	{
		(pNES_cpuRegisters->sp)--;
	}
}

BYTE NES_t::stackPop()
{
	if (pNES_cpuRegisters->sp == 0xFF)
	{
		pNES_cpuRegisters->sp = 0x00;
	}
	else
	{
		(pNES_cpuRegisters->sp)++;
	}
	BYTE popedData = readCpuRawMemory(pNES_cpuRegisters->sp + 0x100, MEMORY_ACCESS_SOURCE::CPU);
	RETURN popedData;
}

void NES_t::processUnusedFlags(BYTE result)
{
	pNES_flags->FORCED_TO_ONE = result;
}

bool NES_t::processSOC()
{
	bool status = true;

	runCPUPipeline();

	RETURN status;
}

NES_t::EXCEPTION_EVENT_TYPE NES_t::processNMI()
{
	// Refer : https://www.nesdev.org/wiki/PPU_frame_timing 
	// For 6.nmi_disable.nes and 08-nmi_off_timing
	SCOUNTER64 ppuCycle = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;
	SCOUNTER32 ly = pNES_instance->NES_state.display.currentScanline;
	if (pNES_instance->NES_state.interrupts.isNMI == YES	// To ensure NMI was triggered which inturn ensures VBL is set or atleast was set just few cycles before to generate the edge trigger
		&& ly == NES_POST_RENDER_SCANLINE	// To ensure "just as VBL flag is set" condition
		&& ppuCycle <= SIX	// Refer 08-nmi_off_timing documentation; This is the method used achieve the PPU cycle accuracy expected by the test in our emulator where CPU cycle is min resolution
		&& pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.VBLANK_NMI_ENABLE == RESET)
	{
		pNES_instance->NES_state.interrupts.isNMI = CLEAR;
		pNES_instance->NES_state.interrupts.nmiDelayInInstructions = RESET;
		RETURN EXCEPTION_EVENT_TYPE::EVENT_NONE;
	}

	// Handle NMI
	// Interrupt is checked only during the final tick of an opcode, hence can be executed only when we are about to fetch the next opcode
	// Refer https://www.reddit.com/r/EmuDev/comments/16y1ilc/comment/k362vo1/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
	// Refer https://forums.nesdev.org/viewtopic.php?p=177408#p177408
	// Refer https://forums.nesdev.org/viewtopic.php?p=177414#p177414
	// Refer https://forums.nesdev.org/viewtopic.php?p=177516#p177516 (this is an interesting post)
	// Also handles for 7.nmi_timing.nes, 04-nmi_control and 05-nmi_timing.nes
	else if
		(
			pNES_instance->NES_state.interrupts.isNMI == YES	// To ensure NMI was triggered which inturn ensures VBL is set or atleast was set just few cycles before to generate the edge trigger 
			&&
			(
				pNES_instance->NES_state.interrupts.nmiDelayInInstructions <= RESET	// NOTE 1 : This handled for NMI latency assuming VBL was already set sometime before 
				&&
				(
					(
						ly == NES_POST_RENDER_SCANLINE	// To ensure "just as VBL flag is set" condition 
						&&
						ppuCycle > SIX	// NOTE 2 : This handles for NMI latency assuming VBL is set just few PPU clocks before; This is the method used achieve the PPU cycle accuracy expected by the test in our emulator where CPU cycle is min resolution
						)
					||
					(
						ly != NES_POST_RENDER_SCANLINE	// To handle condition other than "just as VBL flag is set"
						)
					)
				)
			)
	{
		// For debug
		pNES_ppuRegisters->startOfFrameToNMIHandlerPPUCycles = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerFrame;
		// Reset counters
		pNES_instance->NES_state.interrupts.nmiDelayInInstructions = RESET;
		// Clear the NMI flag
		pNES_instance->NES_state.interrupts.isNMI = NO;
		// Get the current opcode that was fetched...this needs to be subtituted
		pNES_cpuInstance->opcode = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		// PC increment is suppressed when NMI is available
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		pNES_cpuInstance->opcode = 0x00; // Substitute opcode with BRK
		// Try to read next byte from opcode, but PC increment is suppressed, so we read the opcode again
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// PC increment is suppressed
		// Push PC hi to stack
		stackPush(((pNES_cpuRegisters->pc & 0xFF00) >> EIGHT));
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Push PC low to stack
		stackPush((pNES_cpuRegisters->pc & 0x00FF));
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Push P to stack
		auto p = pNES_cpuRegisters->p;
		p.flagFields.FCAUSE = CLEAR; // For NMI and IRQ
		p.flagFields.FORCED_TO_ONE = SET;
		stackPush(p.p);
		// Set the interrupt disable flag in P
		// Refer "I: Interrupt Disable" in https://www.nesdev.org/wiki/Status_flags 
		pNES_cpuRegisters->p.flagFields.INTERRUPT_DISABLE = SET;
		pNES_cpuRegisters->p.flagFields.FZERO = CLEAR;
		pNES_cpuRegisters->p.flagFields.FORCED_TO_ONE = SET;
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Read PC low from vector
		pNES_cpuRegisters->pc = readCpuRawMemory(NMI_VECTOR_START_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Read PC high from vector
		pNES_cpuRegisters->pc |= (readCpuRawMemory(NMI_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU) << EIGHT);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		RETURN EXCEPTION_EVENT_TYPE::EVENT_NMI;
	}

	RETURN EXCEPTION_EVENT_TYPE::EVENT_NONE;
}

NES_t::EXCEPTION_EVENT_TYPE NES_t::processIRQ()
{
	// Handle IRQ
	if (pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE
#if (DEACTIVATED) 
		// Refer : https://forums.nesdev.org/viewtopic.php?t=6464
		// Since its very tough to achieve PPU cycle accuracy in masquerade, thought of experimenting with below code snippet, just to see how off we are
		// Below code helps in passing scanline_timing.nes's first 9 test
		// But this is a HACK and hence will not be implemented
		&& (pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC3 == SET
			&& (
				(pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY > 266 
					&& pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.SPRITE_PATTER_TABLE_ADDR_8x8 == SET)
				||
				(pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY > 10 
					&& pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.BG_PATTERN_TABLE_ADDR == SET)
				)
			)
#endif
		&& pNES_instance->NES_state.interrupts.irqDelayInCpuCycles <= RESET		// NOTE 1 : This handles for IRQ latency assuming IRQ source is set just few CPU clocks before
		&& pNES_instance->NES_state.interrupts.irqDelayInInstructions <= RESET)	// NOTE 2 : This handles for IRQ latency of instruction for taken non-page-crossing branch
	{
		// Reset the "IRQ delay counter"
		pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = RESET;
		pNES_instance->NES_state.interrupts.irqDelayInInstructions = RESET;

		if (pNES_cpuRegisters->p.flagFields.INTERRUPT_DISABLE == SET)
		{
			RETURN EXCEPTION_EVENT_TYPE::EVENT_NONE;
		}
		else
		{
			FLAG jumpToNMIResetVector = NO;
			// We don't clear the IRQ flag as IRQ is level triggered!
			// Refer : https://www.nesdev.org/wiki/CPU_interrupts
			// Get the current opcode that was fetched...this needs to be subtituted
			pNES_cpuInstance->opcode = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			// PC increment is suppressed when IRQ is available
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			pNES_cpuInstance->opcode = 0x00; // Substitute opcode with BRK
			// Try to read next byte from opcode, but PC increment is suppressed, so we read the opcode again
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			// PC increment is suppressed
			// Push PC hi to stack
			stackPush((pNES_cpuRegisters->pc & 0xFF00) >> EIGHT);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			// Push PC low to stack
			stackPush(pNES_cpuRegisters->pc & 0x00FF);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			// Push P to stack
			auto p = pNES_cpuRegisters->p;
			p.flagFields.FCAUSE = CLEAR; // For NMI and IRQ
			p.flagFields.FORCED_TO_ONE = SET;
			stackPush(p.p);
			// Set the interrupt disable flag in P
			// Refer "I: Interrupt Disable" in https://www.nesdev.org/wiki/Status_flags 
			pNES_cpuRegisters->p.flagFields.INTERRUPT_DISABLE = SET;
			pNES_cpuRegisters->p.flagFields.FZERO = CLEAR;
			pNES_cpuRegisters->p.flagFields.FORCED_TO_ONE = SET;
			// Handle Interrupt hijacking
			// Refer to "Interrupt hijacking" section in https://www.nesdev.org/wiki/CPU_interrupts
			if (pNES_instance->NES_state.interrupts.isNMI == YES)
			{
				pNES_instance->NES_state.interrupts.isNMI = NO;
				jumpToNMIResetVector = YES;
			}
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			if (jumpToNMIResetVector == YES)
			{
				// Read PC low from vector
				pNES_cpuRegisters->pc = readCpuRawMemory(NMI_VECTOR_START_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
				// Read PC high from vector
				pNES_cpuRegisters->pc |= (readCpuRawMemory(NMI_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU) << EIGHT);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
				RETURN EXCEPTION_EVENT_TYPE::EVENT_NMI;
			}
			else
			{
				// Read PC low from vector
				pNES_cpuRegisters->pc = readCpuRawMemory(IRQ_BRK_VECTOR_START_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
				// Read PC high from vector
				pNES_cpuRegisters->pc |= (readCpuRawMemory(IRQ_BRK_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU) << EIGHT);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
				RETURN EXCEPTION_EVENT_TYPE::EVENT_IRQ;
			}
		}
	}
	else
	{
		RETURN EXCEPTION_EVENT_TYPE::EVENT_NONE;
	}
}

void NES_t::unimplementedInstruction()
{
	WARN("CPU Panic; unknown opcode! %02X", pNES_cpuInstance->opcode);
}
#pragma endregion RP2A03_DEFINITIONS

#pragma region EMULATION_DEFINITIONS
void NES_t::cpuTickT(CYCLES_TYPE cycleType)
{
	if (ROM_TYPE == ROM::NES)
	{
		// DMA unit alternates between 'get' cycles and 'put' cycles as mentioned in https://forums.nesdev.org/viewtopic.php?p=169070#p169070
		++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;

		// Decrement the "CLI cpu cycle delay counter"
		if (pNES_instance->NES_state.interrupts.cliDelayInCpuCycles > RESET)
		{
			--pNES_instance->NES_state.interrupts.cliDelayInCpuCycles;
			if (pNES_instance->NES_state.interrupts.cliDelayInCpuCycles == RESET)
			{
				CPUTODO("Currently, to simulate Interrupt delay we are delaying the \"I\" flag itself; ideally \"I\"  flag should not be delayed, just the interrupt triggering/inhibition needs to be delayed");
				pNES_flags->INTERRUPT_DISABLE = RESET;
			}
		}

		// Decrement the "SEI cpu cycle delay counter"
		if (pNES_instance->NES_state.interrupts.seiDelayInCpuCycles > RESET)
		{
			--pNES_instance->NES_state.interrupts.seiDelayInCpuCycles;
			if (pNES_instance->NES_state.interrupts.seiDelayInCpuCycles == RESET)
			{
				CPUTODO("Currently, to simulate Interrupt delay we are delaying the \"I\" flag itself; ideally \"I\"  flag should not be delayed, just the interrupt triggering/inhibition needs to be delayed");
				pNES_flags->INTERRUPT_DISABLE = SET;
			}
		}

		// Decrement the "PLP cpu cycle delay counter"
		if (pNES_instance->NES_state.interrupts.plpDelayInCpuCycles > RESET)
		{
			--pNES_instance->NES_state.interrupts.plpDelayInCpuCycles;
			if (pNES_instance->NES_state.interrupts.plpDelayInCpuCycles == RESET)
			{
				CPUTODO("Currently, to simulate Interrupt delay we are delaying the \"I\" flag itself; ideally \"I\"  flag should not be delayed, just the interrupt triggering/inhibition needs to be delayed");
				pNES_flags->INTERRUPT_DISABLE = ((pNES_flags->INTERRUPT_DISABLE == SET) ? RESET : SET);
			}
		}

		// Decrement the "IRQ cpu cycle delay counter". This also handles the APU's IRQ tests
		if (pNES_instance->NES_state.interrupts.irqDelayInCpuCycles > RESET)
		{
			--pNES_instance->NES_state.interrupts.irqDelayInCpuCycles;
		}

		// Handle DMA
		if (pNES_instance->NES_state.oamDMA.DMAInProgress == YES && cycleType == CYCLES_TYPE::READ_CYCLE)
		{
			TODO("Maybe, we need to re-think the implementation of \"OAM DMA halts the read operation of CPU\"");
			// Unfortunately in our emulator, cpuTickT occurs after all the memory operation is done
			// Let's assume a case where cycleType is Write Cycle
			// Then dma anyways can't halt cpu, so we will just let this go...
			// Assume a case where cycleType is Read Cycle
			// Then dma should ideally halt the memory read operation
			// but in our emulator, the read operation is already done by the time we come here
			// Is this a problem??? How do we handle this ??? Can identify this particular "supposed to be halted read" somehow using "DMAInProgress" flag ???

			uint16_t target = FIVEHUNDREDTHIRTEEN;

			// halt attempt takes 1 cycle; Refer to point 1 of first comment https://forums.nesdev.org/viewtopic.php?t=14120
			// DMA unit alternates between 'get' cycles and 'put' cycles as mentioned in https://forums.nesdev.org/viewtopic.php?p=169070#p169070
			++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;
			++pNES_instance->NES_state.emulatorStatus.ticks.oamDmaCounter;
			ppuTick();
			ppuTick();
			ppuTick();
			apuTick();

			// "Aligment Cycle" to make sure DMA read happens in GET cycle only
			// Refer to point 1 and 3 of first comment https://forums.nesdev.org/viewtopic.php?t=14120
			// Odd cycles are "put cycles" as mentioned in above link, so if we are in PUT cycle, we need to perform "dummy read?"
			TODO("DMA alignment cycle should be on \"cpuCounter\" or \"dmaGetPutCounter\"?");
#if (DISABLED)
			if (GETBIT(ZERO, pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter) == SET)
#else
			if (GETBIT(ZERO, pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter) == SET)
#endif
			{
				target = FIVEHUNDREDFOURTEEN;
				// DMA unit alternates between 'get' cycles and 'put' cycles as mentioned in https://forums.nesdev.org/viewtopic.php?p=169070#p169070
				++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;
				++pNES_instance->NES_state.emulatorStatus.ticks.oamDmaCounter;
				ppuTick();
				ppuTick();
				ppuTick();
				apuTick();
			}

			while (pNES_instance->NES_state.oamDMA.DMAInProgress)
			{
				// As mentioned in point 1 of https://forums.nesdev.org/viewtopic.php?t=14120
				// "Values are read on 'get' cycles"
				pNES_instance->NES_state.oamDMA.dataToTx = readCpuRawMemory(pNES_instance->NES_state.oamDMA.sourceAddress, MEMORY_ACCESS_SOURCE::DMA);
				++pNES_instance->NES_state.oamDMA.sourceAddress;
				// DMA unit alternates between 'get' cycles and 'put' cycles as mentioned in https://forums.nesdev.org/viewtopic.php?p=169070#p169070
				++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;
				++pNES_instance->NES_state.emulatorStatus.ticks.oamDmaCounter;
				ppuTick();
				ppuTick();
				ppuTick();
				apuTick();

				// As mentioned in point 1 of https://forums.nesdev.org/viewtopic.php?t=14120
				// "Values are written on 'put' cycles"
				pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR]
					= pNES_instance->NES_state.oamDMA.dataToTx;
				++pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR;
				// DMA unit alternates between 'get' cycles and 'put' cycles as mentioned in https://forums.nesdev.org/viewtopic.php?p=169070#p169070
				++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;
				++pNES_instance->NES_state.emulatorStatus.ticks.oamDmaCounter;
				ppuTick();
				ppuTick();
				ppuTick();
				apuTick();

				if (pNES_instance->NES_state.emulatorStatus.ticks.oamDmaCounter == target)
				{
					if ((pNES_instance->NES_state.oamDMA.sourceAddress & 0xFF) != 0x00)
					{
						FATAL("OAM DMA terminated even before the complete page was transferred");
					}
					pNES_instance->NES_state.oamDMA.DMAInProgress = NO;

					// NOTE : To compensate for the hijacked read operation!
					// Refer : https://forums.nesdev.org/viewtopic.php?p=169070#p169070
					// And especially refer to https://forums.nesdev.org/viewtopic.php?p=169128#p169128
					// Ideally in the HW, DMA starts during a CPU read cycle, i.e. this particular CPU read is hijacked by DMA and hence read data is discarded
					// Once the DMA is complete, the same read which was discarded is performed CPU again to compensate for the previous hijack by DMA
					// In our emulator, what we have done is that our DMA operation starts AFTER the CPU read which was supposed to be discarded is complete, so basically discard doesn't happen, read data is saved
					// And when DMA operation is complete, we directly proceed from the steps after this read instead of performing "re-read" as the previous read data is saved!
					// But in HW since "re-read" happens, an extra CPU cycle is associated with this!
					// To account for this extra "re-read" CPU cycle, we will add one additional CPU tick and its corresponding PPU and APU ticks here, once the DMA is complete
					ppuTick();
					ppuTick();
					ppuTick();
					apuTick();
					// Increment the CPU Counter
					++pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter;

					// Refer : https://discord.com/channels/465585922579103744/465586161067229195/863885418143416351
					// Accoring to above link, IRQ was buffered during the CPU's READ_CYCLE which triggered OAMDMA; post the completeion of OAMDMA to compensate for "Dummy DMA read", CPU will try to do another CPU_READ. Will the buffered IRQ get triggered now?
					// Emulating this behaviour seems to help pass "4-irq_and_dma.nes"
					TODO("Need to find few more reliable sources for behaviour mentioned in line %d of file %s", __LINE__, __FILE__);
					pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = RESET; // Reset the cycles to simulate the IRQ getting triggered during the "Dummy DMA read"
					processIRQ();
				}
			}

			pNES_instance->NES_state.emulatorStatus.ticks.oamDmaCounter = RESET;
		}
		else
		{
			// Increment the CPU Counter
			++pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter;
			syncOtherGBModuleTicks();
		}
	}
	else
	{
		// Increment the CPU Counter
		++pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter;
	}
}

void NES_t::syncOtherGBModuleTicks()
{
	// SOC Timing Sequence

	// Note: For PPU sequence, refer to https://www.nesdev.org/wiki/Cycle_reference_chart
	// Note: For APU sequence, refer to "Glossary" of https://www.nesdev.org/wiki/APU

	// 1 CPU Tick = 3 PPU Ticks
	// 1 CPU Tick = 0.5 APU Tick excepts for Triangle channel's timer (APU generally operates at half the frequency of the CPU),
	// But, 1 CPU Tick = 1 APU Tick for Triangle channel's timer
	// 1 APU Tick = 6 PPU Ticks most of the time, 
	// But, in case of Triangle channel's timer, this becomes 3 PPU ticks
	// Therefore, based on above information, we will call 1 "apuTick" for 1 "cpuTickT"

	ppuTick();
	ppuTick();
	ppuTick();

	apuTick();
}

void NES_t::ppuTick()
{
	auto resetPPUState = [&]()
		{
			pNES_instance->NES_state.display.bg.nameTblAddr = RESET;
			pNES_instance->NES_state.display.bg.nameTblByte = RESET;
			pNES_instance->NES_state.display.bg.attrTblAddr = RESET;
			pNES_instance->NES_state.display.bg.attrTblByte = RESET;
			pNES_instance->NES_state.display.bg.patternTableLAddr = RESET;
			pNES_instance->NES_state.display.bg.patternTblLByte = RESET;
			pNES_instance->NES_state.display.bg.patternTblMByte = RESET;
			pNES_ppuRegisters->pn = RESET;
			pNES_ppuRegisters->pm = RESET;
			pNES_ppuRegisters->sn = RESET;
			pNES_ppuRegisters->sm = RESET;
			pNES_ppuRegisters->oamByte = RESET;
			pNES_ppuRegisters->stopSpriteEvaluation = CLEAR;
			pNES_instance->NES_state.display.obj.spriteYCoordinate = RESET;
			pNES_instance->NES_state.display.obj.patternTableLAddr = RESET;
			pNES_instance->NES_state.display.obj.patternTableMAddr = RESET;
			pNES_instance->NES_state.display.obj.patternTblLByte = RESET;
			pNES_instance->NES_state.display.obj.patternTblMByte = RESET;
			pNES_instance->NES_state.display.obj.spriteAttribute.raw = RESET;
			pNES_instance->NES_state.display.obj.spriteXCoordinate = RESET;
			pNES_instance->NES_state.display.obj.tileNumber = RESET;
			pNES_instance->NES_state.display.obj.paletteID = RESET;
		};

	auto checkIfRenderring = [&]()
		{
			if ((pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET)
				|| (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET))
			{
				RETURN YES;
			}
			else
			{
				RETURN NO;
			}
		};

	auto xInc = [&]()
		{
			// if coarse X == 31
			if ((pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x001F) == THIRTYONE)
			{
				pNES_ppuRegisters->ppuInternalRegisters.v.raw &= (~0x001F);						// coarse X = 0
				pNES_ppuRegisters->ppuInternalRegisters.v.raw ^= 0x0400;						// switch horizontal nametable
			}
			else
			{
				pNES_ppuRegisters->ppuInternalRegisters.v.raw += ONE;							// increment coarse X
			}
		};

	auto yInc = [&]()
		{
			// if fine Y < 7
			if ((pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x7000) != 0x7000)
			{
				pNES_ppuRegisters->ppuInternalRegisters.v.raw += 0x1000;						// increment fine Y
			}
			else
			{
				pNES_ppuRegisters->ppuInternalRegisters.v.raw &= (~0x7000);						// fine Y = 0
				uint16_t y = (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x03E0) >> FIVE;	// let y = coarse Y
				if (y == TWENTYNINE)
				{
					y = ZERO;																	// coarse Y = 0
					pNES_ppuRegisters->ppuInternalRegisters.v.raw ^= 0x0800;					// switch vertical nametable
				}
				else if (y == THIRTYONE)
				{
					y = ZERO;																	// coarse Y = 0, nametable not switched
				}
				else
				{
					y += ONE;																	// increment coarse Y
				}
				// put coarse Y back into v
				pNES_ppuRegisters->ppuInternalRegisters.v.raw = (pNES_ppuRegisters->ppuInternalRegisters.v.raw & (~0x03E0)) | (y << FIVE);
			}
		};

	auto populatePixelShiftRegisters = [&]()
		{
			// Loading the shift registers

			pNES_instance->NES_state.display.bg.loPatternShifter.loPatternShiftSplit[LO]
				= pNES_instance->NES_state.display.bg.patternTblLByte;
			pNES_instance->NES_state.display.bg.hiPatternShifter.hiPatternShiftSplit[LO]
				= pNES_instance->NES_state.display.bg.patternTblMByte;

			// Even though ideally this needs to be 1 bit latch, we will still use 8 bit register and set all bits to same value...
			// NOTE: bit[n] Xly 256 sets all 8 bits to bit[n] 

			pNES_instance->NES_state.display.bg.loAttrShifter.loAttrShiftSplit[LO]
				= GETBIT(LO, pNES_instance->NES_state.display.bg.paletteID) * 0xFF;

			pNES_instance->NES_state.display.bg.hiAttrShifter.hiAttrShiftSplit[LO]
				= GETBIT(HI, pNES_instance->NES_state.display.bg.paletteID) * 0xFF;
		};

	auto shiftThePixelShiftRegisters = [&](SCOUNTER64 cycle)
		{
			if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET)
			{
				pNES_instance->NES_state.display.bg.loPatternShifter.loPatternShift <<= ONE;
				pNES_instance->NES_state.display.bg.hiPatternShifter.hiPatternShift <<= ONE;
				pNES_instance->NES_state.display.bg.loAttrShifter.loAttrShift <<= ONE;
				pNES_instance->NES_state.display.bg.hiAttrShifter.hiAttrShift <<= ONE;
			}

			// We check for cycles because we don't want sprite shifter to run during ((cycle >= THREETWENTYONE) && (cycle <= THREETHIRTYSIX))
			if ((cycle >= ONE) && (cycle <= TWOFIFTYSIX))
			{
				if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET)
				{
					for (COUNTER8 spriteI = ZERO; spriteI < pNES_instance->NES_state.display.obj.spriteCountPerScanline; spriteI++)
					{
						if (pNES_instance->NES_state.display.obj.shifter[spriteI].xSubtractor > ZERO)
						{
							pNES_instance->NES_state.display.obj.shifter[spriteI].xSubtractor--;
						}
						else
						{
							pNES_instance->NES_state.display.obj.shifter[spriteI].loPatternShifter <<= ONE;
							pNES_instance->NES_state.display.obj.shifter[spriteI].hiPatternShifter <<= ONE;
						}
					}
				}
			}
		};

	if (pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter >= NES_PPU_WAIT_CPU_CYCLES_POST_RESET)
	{
		SCOUNTER64 cycle = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;
		SCOUNTER32 ly = pNES_instance->NES_state.display.currentScanline;

		if ((ly >= NES_PRE_RENDER_SCANLINE) && (ly <= NES_LAST_SCANLINE_PER_FRAME))
		{
			if ((ly == NES_PRE_RENDER_SCANLINE) && (cycle == ONE))
			{
				// For debug
				if (pNES_ppuRegisters->vblank == SET)
				{
					pNES_ppuRegisters->vblClearPPUCycle = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounter;
					auto diff = (pNES_ppuRegisters->vblClearPPUCycle - pNES_ppuRegisters->vblSetPPUCycle);
					// Refer to 2nd point of "VBL Flag Timing" in https://www.nesdev.org/wiki/PPU_frame_timing
					if ((pNES_ppuRegisters->vblClearPPUCycle - pNES_ppuRegisters->vblSetPPUCycle) != 6820)
					{
						FATAL("VBL set time - VBL clear clear is %llu!", diff);
					}
				}

				pNES_ppuRegisters->vblank = CLEAR;
				pNES_ppuRegisters->sprite0hit = CLEAR;
				pNES_ppuRegisters->spriteOverflow = CLEAR;
			}

			// Refer "Tile and attribute fetching" in https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
			// NOTE: when "((cycle >= THREETWENTYONE) && (cycle <= THREETHIRTYSIX))" is triggerred, "Y" of v is already incremented
			// So, we are fetching the first 2 tiles of the next scanline!
			if (((cycle >= ONE) && (cycle <= TWOFIFTYSIX)) || ((cycle >= THREETWENTYONE) && (cycle <= THREETHIRTYSIX)))
			{
				PPU_BG_FSM fsmState = (PPU_BG_FSM)((cycle - ONE) & SEVEN);	// ((cycle - 1) % 8)
				switch (fsmState)
				{
				case PPU_BG_FSM::RELOAD_SHIFTERS:
				{
					// Refer "Cycles 1-256" in https://www.nesdev.org/wiki/PPU_rendering
					// "The shifters are reloaded during ticks 9, 17, 25, ..., 257"
					if (cycle >= NINE && cycle < TWOFIFTYSEVEN)
					{
						populatePixelShiftRegisters();
					}

					BREAK;
				}
				case PPU_BG_FSM::FETCH_NAMETABLE_BYTE:
				{
					// Refer to https://www.nesdev.org/wiki/PPU_scrolling#Tile_and_attribute_fetching
					pNES_instance->NES_state.display.bg.nameTblAddr
						= NAME_TABLE0_START_ADDRESS
						| (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0FFF);

					pNES_instance->NES_state.display.bg.nameTblByte
						= readPpuRawMemory(pNES_instance->NES_state.display.bg.nameTblAddr, MEMORY_ACCESS_SOURCE::PPU);

					BREAK;
				}
				case PPU_BG_FSM::FETCH_ATTRTABLE_BYTE:
				{
					// Refer to https://www.nesdev.org/wiki/PPU_scrolling#Tile_and_attribute_fetching
					pNES_instance->NES_state.display.bg.attrTblAddr
						= (NAME_TABLE0_START_ADDRESS + 0x03C0)
						| (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0C00)
						| ((pNES_ppuRegisters->ppuInternalRegisters.v.raw >> FOUR) & 0x0038)
						| ((pNES_ppuRegisters->ppuInternalRegisters.v.raw >> TWO) & 0x0007);

					pNES_instance->NES_state.display.bg.attrTblByte
						= readPpuRawMemory(pNES_instance->NES_state.display.bg.attrTblAddr, MEMORY_ACCESS_SOURCE::PPU);

					// To deduce the quadrant
					// 
					// Basics:
					// Assume an n bit number, and lets assume we increment this number for every event E
					// so, bit 0 of this number will toggle at every 1 E
					// bit 1 of this number will toggle at every 2 E's
					// bit 2 of this number will toggle at every 4 E's
					// bit 3 of this number will toggle at every 8 E's
					// bit 4 of this number will toggle at every 16 E's
					// bit 5 of this number will toggle at every 32 E's
					// bit 6 of this number will toggle at every 64 E's
					// 
					// coarseXScroll is incremented one tile
					// We need to figure when it transitions of every 2 tiles, so we consider bit 1 of coarseXScroll
					// coarseYScroll is incremented at every 8 increment of fineYScroll which itself is incremented every scanline
					// We need to figure out every 16 increments of fineyScoll so as to detect 16 scanline increments
					// So we consider bit 1 of coarseYScroll 

					if (pNES_ppuRegisters->ppuInternalRegisters.v.fields.coarseYScroll & 0x02)
					{
						pNES_instance->NES_state.display.bg.attrTblByte >>= FOUR;
					}
					if (pNES_ppuRegisters->ppuInternalRegisters.v.fields.coarseXScroll & 0x02)
					{
						pNES_instance->NES_state.display.bg.attrTblByte >>= TWO;
					}

					pNES_instance->NES_state.display.bg.paletteID = (pNES_instance->NES_state.display.bg.attrTblByte & 0x03);

					BREAK;
				}
				case PPU_BG_FSM::FETCH_PATTTABLE_LBYTE:
				{
					pNES_instance->NES_state.display.bg.patternTableLAddr
						= (PATTERN_TABLE0_START_ADDRESS + (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.BG_PATTERN_TABLE_ADDR << TWELVE)) // Xlied by 0x1000 using shift 12
						| (pNES_instance->NES_state.display.bg.nameTblByte << FOUR) // Xlied by 16 using shift 4
						| pNES_ppuRegisters->ppuInternalRegisters.v.fields.fineYScroll; // fine y basically represents "y per tile" (ly % 8)

					if (pNES_instance->NES_state.catridgeInfo.nanjing_fc001.chrRamAutoSwitch == YES
						&& pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::NANJING_FC001)
					{
						// Refer to https://www.nesdev.org/wiki/INES_Mapper_163#Feedback_Write_($5100-$5101,_write)
						// NOTE : 
						// Left pattern table refers to patternTable0
						// Right pattern table refers to patternTable1
						// And also since we are hardwired to be in vertical name table mirroring
						// https://www.nesdev.org/wiki/Mirroring#Nametable_Mirroring
						// top left block is nameTable0
						// top right block is nameTable1
						// bottom left block is nameTable0
						// botton right block is nameTable1

						auto normNameTblAddr = ((pNES_instance->NES_state.display.bg.nameTblAddr - NAME_TABLE0_START_ADDRESS) & 1023); // & 1023 == % 1024 == % 0x400 (size of single nametable memory)
						if (normNameTblAddr <= 0x1FF)
						{
							if (pNES_instance->NES_state.display.bg.patternTableLAddr >= 0x1000)
							{
								pNES_instance->NES_state.display.bg.patternTableLAddr -= 0x1000;
							}
						}
						else if (normNameTblAddr <= 0x3FF)
						{
							if (pNES_instance->NES_state.display.bg.patternTableLAddr < 0x1000)
							{
								pNES_instance->NES_state.display.bg.patternTableLAddr += 0x1000;
							}
						}
						else
						{
							FATAL("Invalid name table address encountered in mapper 163 when in automatic chrram switch mode");
						}
					}

					pNES_instance->NES_state.display.bg.patternTblLByte
						= readPpuRawMemory(pNES_instance->NES_state.display.bg.patternTableLAddr, MEMORY_ACCESS_SOURCE::PPU);

					BREAK;
				}
				case PPU_BG_FSM::FETCH_PATTTABLE_HBYTE:
				{
					pNES_instance->NES_state.display.bg.patternTableMAddr
						= pNES_instance->NES_state.display.bg.patternTableLAddr + EIGHT;

					pNES_instance->NES_state.display.bg.patternTblMByte
						= readPpuRawMemory(pNES_instance->NES_state.display.bg.patternTableMAddr, MEMORY_ACCESS_SOURCE::PPU);

					// Refer to "Between dot 328 of a scanline, and 256 of the next scanline" of https://www.nesdev.org/wiki/PPU_scrolling
					// Increment X and populate the shift registers @ cycles 8, 16, 24... 240, 248, 256
					// Also, since this if condition also runs from cycles 321 to 336 AND (321 - 1) % 8 == 7 and (326 - 1) % 8 == 7
					// All the conditions mentioned in above link is satisfied!
					if (checkIfRenderring() == YES)
					{
						xInc();
					}

					BREAK;
				}
				default:
				{
					BREAK;
				}
				}

				ID bgColorID = RESET;
				ID bgPaletteID = RESET;
				ID spriteColorID = ZERO;
				ID spritePaletteID = ZERO;
				FLAG bgOverSprite = YES;
				FLAG isSpriteZeroPixel = NO;
				ID finalPixelID = ZERO;
				ID finalPaletteID = ZERO;
				uint16_t paletteRamAddress = RESET;

				// Start rendering
				if (
					((cycle >= ONE) && (cycle <= TWOFIFTYSIX))
					&&
					((ly >= NES_FIRST_VISIBLE_SCANLINE) && (ly <= NES_LAST_SCANLINE_PER_FRAME))
					)
				{
					// Render bg
					if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET)
					{
						bgColorID
							= (GETBIT(FIFTEEN - pNES_ppuRegisters->ppuInternalRegisters.x, pNES_instance->NES_state.display.bg.hiPatternShifter.hiPatternShift) << ONE)
							| GETBIT(FIFTEEN - pNES_ppuRegisters->ppuInternalRegisters.x, pNES_instance->NES_state.display.bg.loPatternShifter.loPatternShift);
						bgPaletteID
							= (GETBIT(FIFTEEN - pNES_ppuRegisters->ppuInternalRegisters.x, pNES_instance->NES_state.display.bg.hiAttrShifter.hiAttrShift) << ONE)
							| GETBIT(FIFTEEN - pNES_ppuRegisters->ppuInternalRegisters.x, pNES_instance->NES_state.display.bg.loAttrShifter.loAttrShift);
					}

					// Render sprites
					if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET)
					{
						for (COUNTER8 spriteI = ZERO; spriteI < pNES_instance->NES_state.display.obj.spriteCountPerScanline; spriteI++)
						{
							if (pNES_instance->NES_state.display.obj.shifter[spriteI].xSubtractor == ZERO)
							{
								spriteColorID
									= (GETBIT(SEVEN, pNES_instance->NES_state.display.obj.shifter[spriteI].hiPatternShifter) << ONE)
									| GETBIT(SEVEN, pNES_instance->NES_state.display.obj.shifter[spriteI].loPatternShifter);
								spritePaletteID = pNES_instance->NES_state.display.obj.shifter[spriteI].spriteAttribute.fields.palette;
								bgOverSprite = pNES_instance->NES_state.display.obj.shifter[spriteI].spriteAttribute.fields.priority;
								isSpriteZeroPixel = pNES_instance->NES_state.display.obj.shifter[spriteI].isSpriteZero;

								if (spriteColorID != ZERO)
								{
									BREAK;
								}
							}
						}
					}

					// Handle left most 8 pixels

					if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.BG_IN_LEFTMOST_8PIXELS == NO && cycle < NINE)
					{
						bgColorID = RESET;
						bgPaletteID = RESET;
					}

					if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.SPRITE_IN_LEFTMOST_8PIXELS == NO && cycle < NINE)
					{
						spriteColorID = RESET;
						spritePaletteID = RESET;
					}

					uint16_t spritePaletteOffset = RESET;

					// Evaluate BG and Sprite priorities
					if (bgColorID == ZERO && spriteColorID == ZERO)
					{
						// The background pixel is transparent
						// The foreground pixel is transparent
						// No winner, draw "backdrop" colour
						finalPixelID = ZERO;
						finalPaletteID = ZERO;
						isSpriteZeroPixel = CLEAR;
					}
					else if (bgColorID == ZERO && spriteColorID > ZERO)
					{
						// The background pixel is transparent
						// The foreground pixel is visible
						// Foreground wins!
						finalPixelID = spriteColorID;
						finalPaletteID = spritePaletteID;
						spritePaletteOffset = SIXTEEN;
						isSpriteZeroPixel = CLEAR;
					}
					else if (bgColorID > ZERO && spriteColorID == ZERO)
					{
						// The background pixel is visible
						// The foreground pixel is transparent
						// Background wins!
						finalPixelID = bgColorID;
						finalPaletteID = bgPaletteID;
						isSpriteZeroPixel = CLEAR;
					}
					else if (bgColorID > ZERO && spriteColorID > ZERO)
					{
						if (bgOverSprite == CLEAR)
						{
							finalPixelID = spriteColorID;
							finalPaletteID = spritePaletteID;
							spritePaletteOffset = SIXTEEN;
						}
						else
						{
							finalPixelID = bgColorID;
							finalPaletteID = bgPaletteID;
						}

						// Handle sprite 0 hit
						if (isSpriteZeroPixel == YES
							&&
							pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET
							&&
							pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET
							)
						{
							if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.BG_IN_LEFTMOST_8PIXELS == YES
								&& pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.SPRITE_IN_LEFTMOST_8PIXELS == YES)
							{
								if (cycle >= ONE && cycle <= TWOFIFTYFIVE)
								{
									pNES_ppuRegisters->sprite0hit = YES;
								}
							}
							else
							{
								if (cycle >= NINE && cycle <= TWOFIFTYFIVE)
								{
									pNES_ppuRegisters->sprite0hit = YES;
								}
							}
						}
					}

					uint16_t paletteRamAddress
						= PALETTE_RAM_INDEXES_START_ADDRESS
						+ spritePaletteOffset
						+ ((finalPaletteID & 0x03) << TWO) // Xly by 4 implemented by shift 2
						+ (finalPixelID & 0x03);

					// Final sanity check
					if (((cycle >= ONE) && (cycle <= TWOFIFTYSIX)) && ((ly >= NES_FIRST_VISIBLE_SCANLINE) && (ly <= NES_LAST_SCANLINE_PER_FRAME)))
					{
						// Render pixel
						ID paletteID = readPpuRawMemory(paletteRamAddress, MEMORY_ACCESS_SOURCE::PPU) & 0x003F;
						Pixel p = palScreen[paletteID];

						// Needed for Zapper Support
						pNES_instance->NES_state.display.gfxColorID[cycle - ONE][ly] = paletteID;

						// Update the ImGui Buffer
						pNES_instance->NES_state.display.imGuiBuffer.imGuiBuffer2D[ly][cycle - ONE] = p;
					}
				}

				if (checkIfRenderring() == YES)
				{
					shiftThePixelShiftRegisters(cycle);
				}
			}

			// Refer https://forums.nesdev.org/viewtopic.php?p=195567#p195567 for condition to check if rendering is enabled
			if ((cycle >= ONE) && (cycle <= SIXTYFOUR) && (checkIfRenderring() == YES))
			{
				PPUTODO("Get the actual sequence in which secondary oam clear is performed");
				// Odd cycle
				if (GETBIT(ZERO, cycle) == SET)
				{
					;
				}
				// Even cycle
				else
				{
					pNES_ppuMemory->NESMemoryMap.secondaryOam.oamB[(cycle / TWO) - ONE] = 0xFF;
				}

				PPUTODO("At which cycle of PPU should the OAM internal fetch registers be cleared");
				if (cycle == SIXTYFOUR)
				{
					pNES_ppuRegisters->pn = RESET;
					pNES_ppuRegisters->pm = RESET;
					pNES_ppuRegisters->sn = RESET;
					pNES_ppuRegisters->sm = RESET;
					pNES_ppuRegisters->oamByte = RESET;
					pNES_ppuRegisters->startSpriteOverflowEvaluation = CLEAR;
					pNES_ppuRegisters->stopSpriteEvaluation = CLEAR;

					memset(pNES_ppuMemory->NESMemoryMap.overflowOam.oamB, RESET, sizeof(pNES_ppuMemory->NESMemoryMap.overflowOam.oamB));
				}
			}

			// Refer https://forums.nesdev.org/viewtopic.php?p=195567#p195567 for condition to check if rendering is enabled
			// Refer to difference b/w pre-render ly and normal ly mentioned in https://forums.nesdev.org/viewtopic.php?p=40598#p40598
			if (
				((cycle >= SIXTYFIVE) && (cycle <= TWOFIFTYSIX))
				&&
				((ly >= NES_PRE_RENDER_SCANLINE) && (ly <= NES_LAST_SCANLINE_PER_FRAME))
				&&
				(checkIfRenderring() == YES)
				)
			{
				if (pNES_ppuRegisters->stopSpriteEvaluation == NO)
				{
					// Odd cycle
					if (GETBIT(ZERO, cycle) == SET)
					{
						pNES_ppuRegisters->oamByte
							= pNES_ppuMemory->NESMemoryMap.primaryOam.oam2B[pNES_ppuRegisters->pn][pNES_ppuRegisters->pm];
					}
					// Even cycle
					else
					{
						if (pNES_ppuRegisters->startSpriteOverflowEvaluation == NO && pNES_ppuRegisters->stopSpriteEvaluation == NO)
						{
							pNES_ppuMemory->NESMemoryMap.secondaryOam.oam2B[pNES_ppuRegisters->sn][pNES_ppuRegisters->sm]
								= pNES_ppuRegisters->oamByte;

							// Check if this sprite needs to be considered for next scanline
							uint16_t yMin = pNES_ppuMemory->NESMemoryMap.secondaryOam.oamW[pNES_ppuRegisters->sn].yPosition;
							uint16_t yMax = (yMin + EIGHT - ONE);
							uint16_t y = ly;

							// Handle 8x16 sprites
							if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.SPRITE_SIZE == SET)
							{
								yMax += EIGHT;
							}

							if (y >= yMin && y <= yMax)
							{
								// Handle sprite 0 hit
								// If we came here and pNES_ppuRegisters->pn = 0, this means sprite 0 of primary oam was triggered and will be copied to first location of secondary oam
								if (pNES_ppuRegisters->pn == ZERO)
								{
									pNES_instance->NES_state.display.obj.isSprite0PresentInSecondaryOam = YES;
								}

								pNES_ppuRegisters->pm += ONE;
								pNES_ppuRegisters->sm += ONE;
							}
							else
							{
								pNES_ppuRegisters->pn += ONE;
							}

							// Handle overflows
							if (pNES_ppuRegisters->pm > THREE)
							{
								++pNES_ppuRegisters->pn;
								pNES_ppuRegisters->pm = RESET;
							}
							if (pNES_ppuRegisters->sm > THREE)
							{
								++pNES_ppuRegisters->sn;
								pNES_ppuRegisters->sm = RESET;
							}

							if (pNES_ppuRegisters->sn >= EIGHT)
							{
								pNES_ppuRegisters->sn = RESET;
								pNES_ppuRegisters->sm = RESET;
								pNES_ppuRegisters->oamByte = RESET;
								pNES_ppuRegisters->startSpriteOverflowEvaluation = YES;
							}

							if (pNES_ppuRegisters->pn >= SIXTYFOUR)
							{
								pNES_ppuRegisters->pn = RESET;
								pNES_ppuRegisters->pm = RESET;
								pNES_ppuRegisters->sn = RESET;
								pNES_ppuRegisters->sm = RESET;
								pNES_ppuRegisters->oamByte = RESET;
								pNES_ppuRegisters->startSpriteOverflowEvaluation = CLEAR;
								pNES_ppuRegisters->stopSpriteEvaluation = YES;
							}
						}
						else if (pNES_ppuRegisters->startSpriteOverflowEvaluation == YES && pNES_ppuRegisters->stopSpriteEvaluation == NO)
						{
							// Refer to point 1b of https://www.nesdev.org/wiki/PPU_sprite_evaluation#Details
							PPUTODO("Perform the dummy secondary OAM read operation at line %d of file %s", __LINE__, __FILE__);

							// Start evaluating sprite overflow
							pNES_ppuMemory->NESMemoryMap.overflowOam.oam2B[pNES_ppuRegisters->sn][pNES_ppuRegisters->sm]
								= pNES_ppuRegisters->oamByte;

							// Check if this sprite needs to be considered for next scanline
							uint16_t yMin = pNES_ppuMemory->NESMemoryMap.overflowOam.oamW[pNES_ppuRegisters->sn].yPosition;
							uint16_t yMax = (yMin + EIGHT - ONE);
							uint16_t y = ly;

							// Handle 8x16 sprites
							if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.SPRITE_SIZE == SET)
							{
								yMax += EIGHT;
							}

							if (y >= yMin && y <= yMax)
							{
								if (checkIfRenderring() == YES)
								{
									pNES_ppuRegisters->spriteOverflow = YES;
								}

								pNES_ppuRegisters->pm += ONE;
								pNES_ppuRegisters->sm += ONE;

								// Handle overflows
								if (pNES_ppuRegisters->pm > THREE)
								{
									++pNES_ppuRegisters->pn;
									pNES_ppuRegisters->pm = RESET;
								}
							}
							else
							{
								pNES_ppuRegisters->pn += ONE;
								pNES_ppuRegisters->pm += ONE; // Sprite Overflow Bug

								// Handle overflows
								// Refer "4.Obscure.nes" of Blargg's "sprite_overflow_tests" source code for expected behaviour
								if (pNES_ppuRegisters->pm > THREE)
								{
									pNES_ppuRegisters->pm = RESET;
								}
							}

							// Handle overflows
							if (pNES_ppuRegisters->sm > THREE)
							{
								++pNES_ppuRegisters->sn;
								pNES_ppuRegisters->sm = RESET;
							}

							if ((pNES_ppuRegisters->sn >= SIXTYFOUR) || (pNES_ppuRegisters->pn >= SIXTYFOUR))
							{
								pNES_ppuRegisters->pn = RESET;
								pNES_ppuRegisters->pm = RESET;
								pNES_ppuRegisters->sn = RESET;
								pNES_ppuRegisters->sm = RESET;
								pNES_ppuRegisters->oamByte = RESET;
								pNES_ppuRegisters->startSpriteOverflowEvaluation = CLEAR;
								pNES_ppuRegisters->stopSpriteEvaluation = YES;
							}
						}
					}
				}
			}

			// Refer https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
			// Also refer "At dot 256 of each scanline" in https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
			if (cycle == TWOFIFTYSIX)
			{
				if (checkIfRenderring() == YES)
				{
					yInc();
				}
			}

			// Refer https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
			// Also refer "At dot 257 of each scanline" in https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
			if (cycle == TWOFIFTYSEVEN)
			{
				populatePixelShiftRegisters(); // Refer "Cycles 1-256" in https://www.nesdev.org/wiki/PPU_rendering ("The shifters are reloaded during ticks 9, 17, 25, ..., 257")
				if (checkIfRenderring() == YES)
				{
					pNES_ppuRegisters->ppuInternalRegisters.v.fields.coarseXScroll = pNES_ppuRegisters->ppuInternalRegisters.t.fields.coarseXScroll;
					pNES_ppuRegisters->ppuInternalRegisters.v.fields.nameTblSelectH = pNES_ppuRegisters->ppuInternalRegisters.t.fields.nameTblSelectH;
				}

				PPUTODO("At which PPU cycle do we clear the internal sprite shift registers");
				for (COUNTER8 spriteI = ZERO; spriteI < EIGHT; spriteI++)
				{
					pNES_instance->NES_state.display.obj.shifter[spriteI].hiPatternShifter = RESET;
					pNES_instance->NES_state.display.obj.shifter[spriteI].loPatternShifter = RESET;
					pNES_instance->NES_state.display.obj.shifter[spriteI].spriteAttribute.fields.palette = RESET;
					pNES_instance->NES_state.display.obj.shifter[spriteI].spriteAttribute.fields.priority = RESET;
					pNES_instance->NES_state.display.obj.shifter[spriteI].isSpriteZero = NO;
				}
				pNES_instance->NES_state.display.obj.spriteCountPerScanline = RESET;
			}

			// Refer : https://forums.nesdev.org/viewtopic.php?t=17327
			// Sprite fetches for the next scanline is done here!
			// Refer to difference b/w pre-render ly and normal ly mentioned in https://forums.nesdev.org/viewtopic.php?p=40598#p40598
			if (
				((cycle >= TWOFIFTYSEVEN) && (cycle <= THREETWENTY))
				&&
				((ly >= NES_PRE_RENDER_SCANLINE) && (ly <= NES_LAST_SCANLINE_PER_FRAME))
				)
			{
				auto stateIdx = ((cycle - TWOFIFTYSEVEN) & SEVEN); // ((cycle - 257) % 8)
				auto objIdx = ((cycle - TWOFIFTYSEVEN) / EIGHT);

				switch (stateIdx)
				{
				case ZERO:
				{
					pNES_instance->NES_state.display.obj.spriteYCoordinate
						= pNES_ppuMemory->NESMemoryMap.secondaryOam.oamW[objIdx].yPosition;
					BREAK;
				}
				case ONE:
				{
					// Refer to https://www.nesdev.org/wiki/PPU_rendering#Cycles_257-320
					auto dummyNameTblAddr = NAME_TABLE0_START_ADDRESS | (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0FFF);
					auto discard = readPpuRawMemory(pNES_instance->NES_state.display.bg.nameTblAddr, MEMORY_ACCESS_SOURCE::PPU);

					pNES_instance->NES_state.display.obj.tileNumber
						= pNES_ppuMemory->NESMemoryMap.secondaryOam.oamW[objIdx].tileID;
					BREAK;
				}
				case TWO:
				{
					pNES_instance->NES_state.display.obj.spriteAttribute.raw
						= pNES_ppuMemory->NESMemoryMap.secondaryOam.oamW[objIdx].attributes.raw;
					BREAK;
				}
				case THREE:
				{
					// Refer to https://www.nesdev.org/wiki/PPU_rendering#Cycles_257-320
					auto dummyNameTblAddr = (NAME_TABLE0_START_ADDRESS + 0x03C0)
						| (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0C00)
						| ((pNES_ppuRegisters->ppuInternalRegisters.v.raw >> FOUR) & 0x0038)
						| ((pNES_ppuRegisters->ppuInternalRegisters.v.raw >> TWO) & 0x0007);
					auto discard = readPpuRawMemory(pNES_instance->NES_state.display.bg.attrTblAddr, MEMORY_ACCESS_SOURCE::PPU);

					pNES_instance->NES_state.display.obj.spriteXCoordinate
						= pNES_ppuMemory->NESMemoryMap.secondaryOam.oamW[objIdx].xPosition;
					BREAK;
				}
				case FOUR:
				{
					// Dummy Read as mentioned in point 3b of https://www.nesdev.org/wiki/PPU_sprite_evaluation#Details
					pNES_instance->NES_state.display.obj.spriteXCoordinate
						= pNES_ppuMemory->NESMemoryMap.secondaryOam.oamW[objIdx].xPosition;
					BREAK;
				}
				case FIVE:
				{
					auto flipY = ((pNES_instance->NES_state.display.obj.spriteAttribute.fields.flipVertically == SET) ? YES : NO);

					if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.SPRITE_SIZE == SET)
					{
						auto tileNumber = pNES_instance->NES_state.display.obj.tileNumber & 0xFE;
						// Note: Below mod operation is needed because
						// from the equations below, we know that to figure out the pattern Table Address, we needed "y per tile"
						// In case of 8*16 tiles, the way we normally get "y per tile" is not enough
						// as ly - spriteY can be => 8
						// This is because spriteY always indicates the top tiles y value, so for bottom tile, ly - spriteY becomes >= 8
						// So we take mod of this diff, which essentially is same as moving the spriteY to the lower tile
						auto yPerTile = (ly - pNES_instance->NES_state.display.obj.spriteYCoordinate) & 0x07;

						if (flipY)
						{
							// Check if we need the higher half of 8x16 sprite
							if ((ly - pNES_instance->NES_state.display.obj.spriteYCoordinate) < EIGHT)
							{
								tileNumber += ONE;
							}

							pNES_instance->NES_state.display.obj.patternTableLAddr
								= (PATTERN_TABLE0_START_ADDRESS + (GETBIT(ZERO, pNES_instance->NES_state.display.obj.tileNumber) << TWELVE)) // Xlied by 0x1000 using shift 12
								| (tileNumber << FOUR) // Xlied by 16 using shift 4
								| (SEVEN - yPerTile); // Represents "y per tile"
						}
						else
						{
							// Check if we need the lower half of 8x16 sprite
							if ((ly - pNES_instance->NES_state.display.obj.spriteYCoordinate) >= EIGHT)
							{
								tileNumber += ONE;
							}

							pNES_instance->NES_state.display.obj.patternTableLAddr
								= (PATTERN_TABLE0_START_ADDRESS + (GETBIT(ZERO, pNES_instance->NES_state.display.obj.tileNumber) << TWELVE)) // Xlied by 0x1000 using shift 12
								| (tileNumber << FOUR) // Xlied by 16 using shift 4
								| (yPerTile); // Represents "y per tile"
						}
					}
					else
					{
						if (flipY)
						{
							pNES_instance->NES_state.display.obj.patternTableLAddr
								= (PATTERN_TABLE0_START_ADDRESS + (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.SPRITE_PATTER_TABLE_ADDR_8x8 << TWELVE)) // Xlied by 0x1000 using shift 12
								| (pNES_instance->NES_state.display.obj.tileNumber << FOUR) // Xlied by 16 using shift 4
								| (SEVEN - (ly - pNES_instance->NES_state.display.obj.spriteYCoordinate)); // Represents "y per tile"
						}
						else
						{
							pNES_instance->NES_state.display.obj.patternTableLAddr
								= (PATTERN_TABLE0_START_ADDRESS + (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.SPRITE_PATTER_TABLE_ADDR_8x8 << TWELVE)) // Xlied by 0x1000 using shift 12
								| (pNES_instance->NES_state.display.obj.tileNumber << FOUR) // Xlied by 16 using shift 4
								| (ly - pNES_instance->NES_state.display.obj.spriteYCoordinate); // Represents "y per tile"
						}
					}

					if (pNES_instance->NES_state.catridgeInfo.nanjing_fc001.chrRamAutoSwitch == YES
						&& pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::NANJING_FC001)
					{
						// Refer to https://www.nesdev.org/wiki/INES_Mapper_163#Feedback_Write_($5100-$5101,_write)
						// NOTE : 
						// Left pattern table refers to patternTable0
						// Right pattern table refers to patternTable1
						// And also since we are hardwired to be in vertical name table mirroring
						// https://www.nesdev.org/wiki/Mirroring#Nametable_Mirroring
						// top left block is nameTable0
						// top right block is nameTable1
						// bottom left block is nameTable0
						// botton right block is nameTable1

						auto normNameTblAddr = ((pNES_instance->NES_state.display.bg.nameTblAddr - NAME_TABLE0_START_ADDRESS) & 1023); // & 1023 == % 1024 == % 0x400 (size of single nametable memory)
						if (normNameTblAddr <= 0x1FF)
						{
							if (pNES_instance->NES_state.display.bg.patternTableLAddr >= 0x1000)
							{
								pNES_instance->NES_state.display.bg.patternTableLAddr -= 0x1000;
							}
						}
						else if (normNameTblAddr <= 0x3FF)
						{
							if (pNES_instance->NES_state.display.bg.patternTableLAddr < 0x1000)
							{
								pNES_instance->NES_state.display.bg.patternTableLAddr += 0x1000;
							}
						}
						else
						{
							FATAL("Invalid name table address encountered in mapper 163 when in automatic chrram switch mode");
						}
					}

					pNES_instance->NES_state.display.obj.patternTblLByte
						= readPpuRawMemory(pNES_instance->NES_state.display.obj.patternTableLAddr, MEMORY_ACCESS_SOURCE::PPU);

					// Dummy Read as mentioned in point 3b of https://www.nesdev.org/wiki/PPU_sprite_evaluation#Details
					pNES_instance->NES_state.display.obj.spriteXCoordinate
						= pNES_ppuMemory->NESMemoryMap.secondaryOam.oamW[objIdx].xPosition;
					BREAK;
				}
				case SIX:
				{
					// Dummy Read as mentioned in point 3b of https://www.nesdev.org/wiki/PPU_sprite_evaluation#Details
					pNES_instance->NES_state.display.obj.spriteXCoordinate
						= pNES_ppuMemory->NESMemoryMap.secondaryOam.oamW[objIdx].xPosition;
					BREAK;
				}
				case SEVEN:
				{
					pNES_instance->NES_state.display.obj.patternTableMAddr
						= pNES_instance->NES_state.display.obj.patternTableLAddr + EIGHT;

					pNES_instance->NES_state.display.obj.patternTblMByte
						= readPpuRawMemory(pNES_instance->NES_state.display.obj.patternTableMAddr, MEMORY_ACCESS_SOURCE::PPU);

					// Populate the shift registers
					// Refer : https://stackoverflow.com/a/2602885
					auto flipbyte = [](BYTE b)
						{
							b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
							b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
							b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
							RETURN b;
						};

					// Populate the sprite subtractor
					pNES_instance->NES_state.display.obj.shifter[objIdx].xSubtractor = pNES_instance->NES_state.display.obj.spriteXCoordinate;

					// Check if horizontal flip is needed 
					auto flipX = ((pNES_instance->NES_state.display.obj.spriteAttribute.fields.flipHorizontally == SET) ? YES : NO);
					if (flipX == YES)
					{
						pNES_instance->NES_state.display.obj.patternTblLByte =
							flipbyte(pNES_instance->NES_state.display.obj.patternTblLByte);

						pNES_instance->NES_state.display.obj.patternTblMByte =
							flipbyte(pNES_instance->NES_state.display.obj.patternTblMByte);

					}

					PPUTODO("Below if condition's functionality at line %d of file %s is needed as \"last column sprite glitch\" is not there when we add this", __LINE__, __FILE__);
					// But we don't know how PPU actually achieves this functionality
					// We probably need to figure out the behaviour of sprite rendering when primary OAM is completely empty!
					// Refer : https://www.nesdev.org/wiki/PPU_sprite_evaluation (For the first empty sprite slot, this will consist of sprite #63's Y-coordinate followed by 3 $FF bytes; for subsequent empty sprite slots, this will be four $FF bytes)
					// Discarding dummy 64th sprite entry
					if (pNES_instance->NES_state.display.obj.spriteXCoordinate == 0xFF
						&& pNES_instance->NES_state.display.obj.spriteAttribute.raw == 0xFF
						&& pNES_instance->NES_state.display.obj.tileNumber == 0xFF)
					{
						// If sprite zero is present in secondary OAM and is valid, then it will occupy the first place in our secondary OAM
						if (pNES_instance->NES_state.display.obj.isSprite0PresentInSecondaryOam == YES
							&& objIdx == ZERO)
						{
							pNES_instance->NES_state.display.obj.isSprite0PresentInSecondaryOam = CLEAR;
						}
					}
					else if (pNES_instance->NES_state.display.obj.spriteCountPerScanline < EIGHT)
					{
						// Load the shift registers
						pNES_instance->NES_state.display.obj.shifter
							[pNES_instance->NES_state.display.obj.spriteCountPerScanline].loPatternShifter = pNES_instance->NES_state.display.obj.patternTblLByte;

						pNES_instance->NES_state.display.obj.shifter
							[pNES_instance->NES_state.display.obj.spriteCountPerScanline].hiPatternShifter = pNES_instance->NES_state.display.obj.patternTblMByte;

						pNES_instance->NES_state.display.obj.shifter
							[pNES_instance->NES_state.display.obj.spriteCountPerScanline].spriteAttribute.raw = pNES_instance->NES_state.display.obj.spriteAttribute.raw;

						// If sprite zero is present in secondary OAM and is valid, then it will occupy the first place in our secondary OAM
						if (pNES_instance->NES_state.display.obj.isSprite0PresentInSecondaryOam == YES
							&& objIdx == ZERO)
						{
							pNES_instance->NES_state.display.obj.shifter
								[pNES_instance->NES_state.display.obj.spriteCountPerScanline].isSpriteZero = YES;
							pNES_instance->NES_state.display.obj.isSprite0PresentInSecondaryOam = CLEAR;
						}

						// Increment the sprite count
						++pNES_instance->NES_state.display.obj.spriteCountPerScanline;
					}

					// Clear the internal registers
					pNES_instance->NES_state.display.obj.spriteXCoordinate = RESET;
					pNES_instance->NES_state.display.obj.spriteYCoordinate = RESET;
					pNES_instance->NES_state.display.obj.patternTableLAddr = RESET;
					pNES_instance->NES_state.display.obj.patternTableMAddr = RESET;
					pNES_instance->NES_state.display.obj.patternTblLByte = RESET;
					pNES_instance->NES_state.display.obj.patternTblMByte = RESET;
					pNES_instance->NES_state.display.obj.spriteAttribute.raw = RESET;
					pNES_instance->NES_state.display.obj.tileNumber = RESET;
					pNES_instance->NES_state.display.obj.paletteID = RESET;

					// Dummy Read as mentioned in point 3b of https://www.nesdev.org/wiki/PPU_sprite_evaluation#Details
					pNES_instance->NES_state.display.obj.spriteXCoordinate
						= pNES_ppuMemory->NESMemoryMap.secondaryOam.oamW[objIdx].xPosition;
					BREAK;
				}
				default:
				{
					FATAL("Unknown Sprite Fetch Stage");
				}
				}
			}

			// Refer https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
			// Also refer "During dots 280 to 304 of the pre-render scanline (end of vblank)" in https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
			// NOTE: The below code snippet is needed to reset the "Y" parts of the V register after scanline -1
			// else, "Y" during scanline 0 would index scanline 1's data and in this way, all data would be shifted down by 1 scanline and we would actually miss the last visible scanline!
			// Refer to difference b/w pre-render ly and normal ly mentioned in https://forums.nesdev.org/viewtopic.php?p=40598#p40598
			if ((pNES_instance->NES_state.display.currentScanline == NES_PRE_RENDER_SCANLINE) && (cycle >= TWOHUNDREDEIGHTY) && (cycle <= THREEHUNDREDFOUR))
			{
				if (checkIfRenderring() == YES)
				{
					pNES_ppuRegisters->ppuInternalRegisters.v.fields.coarseYScroll = pNES_ppuRegisters->ppuInternalRegisters.t.fields.coarseYScroll;
					pNES_ppuRegisters->ppuInternalRegisters.v.fields.nameTblSelectV = pNES_ppuRegisters->ppuInternalRegisters.t.fields.nameTblSelectV;
					pNES_ppuRegisters->ppuInternalRegisters.v.fields.fineYScroll = pNES_ppuRegisters->ppuInternalRegisters.t.fields.fineYScroll;
				}
			}

			// Refer to point 4 of https://www.nesdev.org/wiki/PPU_sprite_evaluation#Details 
			if ((cycle >= THREETWENTYONE) && (cycle <= THREEFORTY))
			{
				PPUTODO("Perform the first byte read of secondary OAM at line %d of file %s", __LINE__, __FILE__);
			}

			PPUTODO("Figure out in which cycle does the populating of shift register takes place for cycles beyond 321");
			// Below if condition is to handle the populating of shift registers for 2 tiles fetched during cycles 321-326
			// 1st tile of next scanline (cycle found empirically)
			if (cycle == THREETWENTYEIGHT)
			{
				populatePixelShiftRegisters();
			}
			// 2nd tile of next scanline (cycle found empirically)
			if (cycle == THREETHIRTYSIX)
			{
				populatePixelShiftRegisters();
			}

			// Cycles 337-340
			if ((cycle >= THREETHIRTYSEVEN) && (cycle <= THREEFORTY))
			{
				switch (cycle)
				{
				case THREETHIRTYSEVEN:
				{
					BREAK;
				}
				case THREETHIRTYEIGHT:
				{
					auto discard = readPpuRawMemory(NAME_TABLE0_START_ADDRESS | (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0FFF), MEMORY_ACCESS_SOURCE::PPU);
					BREAK;
				}
				case THREETHIRTYNINE:
				{
					BREAK;
				}
				case THREEFORTY:
				{
					auto discard = readPpuRawMemory(NAME_TABLE0_START_ADDRESS | (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0FFF), MEMORY_ACCESS_SOURCE::PPU);
					BREAK;
				}
				}
			}
		}

		if ((ly == NES_POST_RENDER_SCANLINE) && (cycle == ONE))
		{
			if (pNES_ppuRegisters->ppuStatusReadQuirkEnable == NO)
			{
				pNES_ppuRegisters->vblank = YES;
				pNES_ppuRegisters->vblSetPPUCycle = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounter;

				if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.VBLANK_NMI_ENABLE == SET
					&& pNES_instance->NES_state.interrupts.isNMI == NO)
				{
					pNES_instance->NES_state.interrupts.isNMI = YES;
					pNES_instance->NES_state.interrupts.nmiDelayInInstructions = RESET;
					// For debug
					pNES_ppuRegisters->startOfFrameToNMITriggerPPUCycles = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerFrame;
				}
			}

			// Clear the quirk 
			// Refer : https://www.nesdev.org/wiki/PPU_frame_timing
			pNES_ppuRegisters->ppuStatusReadQuirkEnable = CLEAR;

			pNES_instance->NES_state.display.wasVblankJustTriggerred = YES;
		}

		// Tick the counters
		++pNES_instance->NES_state.emulatorStatus.ticks.ppuCounter;
		++pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;
		++pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerFrame;
		++pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterMMC3A12;

		// Refer : https://www.nesdev.org/wiki/PPU_frame_timing 
		// Refer : https://forums.nesdev.org/viewtopic.php?t=1368
		// Refer : https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
		if (
			(ly == NES_PRE_RENDER_SCANLINE)
			&&
			(checkIfRenderring() == YES)
			&&
			(pNES_instance->NES_state.display.isOddFrame == YES)
			&&
			(pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY == (NES_LAST_PPU_CYCLE_PER_SCANLINE - ONE))) // For 10-even_odd_timing.nes
		{
			++pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY; // For 1.frame_basics.nes and 10-even_odd_timing.nes
		}

		if (pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY >= NES_TOTAL_PPU_CYCLES_PER_SCANLINE)
		{
			resetPPUState();
			pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY = RESET;
			++pNES_instance->NES_state.display.currentScanline;

			if (pNES_instance->NES_state.display.currentScanline >= NES_TOTAL_PPU_SCANLINE)
			{
				pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerFrame = RESET;
				pNES_instance->NES_state.display.currentScanline = NES_PRE_RENDER_SCANLINE;
				pNES_instance->NES_state.display.isOddFrame = !pNES_instance->NES_state.display.isOddFrame;
			}
		}
	}
	else
	{
		// Do nothing and wait...
	}
}

void NES_t::apuTick()
{
	// As mentioned in "Glossary" of https://www.nesdev.org/wiki/APU
	// 
	// Frame Sequencer: 
	// Sequencer is ticked at 240 Hz, i.e. 4 times the frame rate (60 Hz)
	// In our emulator, 1 frame is when VBLANK is set
	// We have pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerFrame... 
	// which is derived from CPU ticks AND also remains ~constant per frame
	// 
	// Timer: 
	// Triangle channel's timer is clocked every cpu cycle
	// Pulse, Noise and DMC timers are clocked in every 2nd CPU cycle; so, whenever "apuCounter" is odd
	//
	// To emulate the 240 Hz ticks, we need to check "apuCounter" against some magic numbers
	// Refer https://github.com/ObaraEmmanuel/NES/blob/master/src/apu.c#L185
	// Refer https://github.com/amaiorano/nes-emu/blob/master/src/Apu.cpp
	// Refer https://github.com/amaiorano/nes-emu/blob/master/src/Nes.cpp
	// Refer https://github.com/kevinbchen/nes-emu/blob/main/src/nes/apu.cpp#L153

	// runProfiler();
	// Based on _DEBUG_PROFILER, we are coming to this function @ 1786830 Hz rate
	// This is expected as this is function is called every time CPU tick is called and CPU rate in NES is NES_CPU_CLOCK_HZ, which is 1786830 Hz
	// So the frequency @ which this function is called is 1.78 MHz
	// When using odd cycles of apuCounter, this frequency comes down to 893.415 KHz which is NES_APU_CLOCK_HZ

	if (ENABLED)
	{
		// To handle 09.reset_timing.nes
		// As the documentation states, the offset can be somewhere around 9 - 12 clocks (This is verified in Masquerade as well)
		// We have arbitrarilly chosen 10 to be the offset (and apparently 09.reset_timing.nes also uses 10 clock as reference)
		if (pNES_instance->NES_state.audio.isReset == YES)
		{
			pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer = TEN;
			pNES_instance->NES_state.audio.isReset = CLEAR;
		}

		// Refer : https://forums.nesdev.org/viewtopic.php?p=64359#p64359 for the apuSequencer magic numbers in below switch case
		if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FOUR_STEP_MODE)
		{
			switch (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer)
			{
			case (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M00)):
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);

				processLinearCounter();
				BREAK;
			}
			case (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M01)):
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);

				processSweep(AUDIO_CHANNELS::PULSE_1);
				processSweep(AUDIO_CHANNELS::PULSE_2);

				processLinearCounter();

				processLengthCounter(AUDIO_CHANNELS::PULSE_1);
				processLengthCounter(AUDIO_CHANNELS::PULSE_2);
				processLengthCounter(AUDIO_CHANNELS::TRIANGLE);
				processLengthCounter(AUDIO_CHANNELS::NOISE);
				BREAK;
			}
			case (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M02)):
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);

				processLinearCounter();
				BREAK;
			}
			case ((TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03)) - ONE):
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.FRAME_CONFIG.DIS_FRAME_INTR == RESET
					&& pNES_instance->NES_state.interrupts.irqDelayInCpuCycles == RESET)	// TODO : Is this check needed?
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.FRAME_INTR = SET;
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_FRAMECTR = SET;
					pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = ONE;
				}
				BREAK;
			}
			case (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03)):
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.FRAME_CONFIG.DIS_FRAME_INTR == RESET
					&& pNES_instance->NES_state.interrupts.irqDelayInCpuCycles == RESET)	// TODO : Is this check needed?
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.FRAME_INTR = SET;
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_FRAMECTR = SET;
					pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = ONE;
				}

				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);

				processSweep(AUDIO_CHANNELS::PULSE_1);
				processSweep(AUDIO_CHANNELS::PULSE_2);

				processLinearCounter();

				processLengthCounter(AUDIO_CHANNELS::PULSE_1);
				processLengthCounter(AUDIO_CHANNELS::PULSE_2);
				processLengthCounter(AUDIO_CHANNELS::TRIANGLE);
				processLengthCounter(AUDIO_CHANNELS::NOISE);
				BREAK;
			}
			case ((TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M03)) + ONE):
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.FRAME_CONFIG.DIS_FRAME_INTR == RESET
					&& pNES_instance->NES_state.interrupts.irqDelayInCpuCycles == RESET)	// TODO : Is this check needed?
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.FRAME_INTR = SET;
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_FRAMECTR = SET;
					pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = ZERO;
				}
				BREAK;
			}
			}
		}
		else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
		{
			switch (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer)
			{
			case (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10)):
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);

				processSweep(AUDIO_CHANNELS::PULSE_1);
				processSweep(AUDIO_CHANNELS::PULSE_2);

				processLinearCounter();

				processLengthCounter(AUDIO_CHANNELS::PULSE_1);
				processLengthCounter(AUDIO_CHANNELS::PULSE_2);
				processLengthCounter(AUDIO_CHANNELS::TRIANGLE);
				processLengthCounter(AUDIO_CHANNELS::NOISE);
				BREAK;
			}
			case (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M11)):
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);

				processLinearCounter();
				BREAK;
			}
			case (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M12)):
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);

				processSweep(AUDIO_CHANNELS::PULSE_1);
				processSweep(AUDIO_CHANNELS::PULSE_2);

				processLinearCounter();

				processLengthCounter(AUDIO_CHANNELS::PULSE_1);
				processLengthCounter(AUDIO_CHANNELS::PULSE_2);
				processLengthCounter(AUDIO_CHANNELS::TRIANGLE);
				processLengthCounter(AUDIO_CHANNELS::NOISE);
				BREAK;
			}
			case (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M13)):
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);

				processLinearCounter();
				BREAK;
			}
			}
		}
		else
		{
			FATAL("Unknown Frame Sequencer Mode");
		}

		++pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer;

		if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FOUR_STEP_MODE)
		{
			if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer >= (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M04)))
			{
				pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer = (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_0::STEP_M00));
			}
		}
		else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
		{
			if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer >= (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M14)))
			{
				pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer = (TO_UINT(FRAME_SEQUENCER_CYCLES_MODE_1::STEP_M10));
			}
		}
	}

	if (ENABLED)
	{
		// On every odd cycles
		if (GETBIT(ZERO, pNES_instance->NES_state.emulatorStatus.ticks.apuCounter) == SET)
		{
			tickPulse(AUDIO_CHANNELS::PULSE_1);
			tickPulse(AUDIO_CHANNELS::PULSE_2);
			tickNoise();
		}

		tickTriangle();
		tickDMC();
	}

	// Handle Quirks!
	if (ENABLED)
	{
		// Handles 04.clock_jitter.nes of blargg apu tests
		if (pNES_instance->NES_state.audio.cyclesToSequencerModeChange > RESET)
		{
			--pNES_instance->NES_state.audio.cyclesToSequencerModeChange;

			if (-pNES_instance->NES_state.audio.cyclesToSequencerModeChange == RESET)
			{
				pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer = RESET;
				pNES_instance->NES_state.audio.frameSequencerMode
					= (FRAME_SEQUENCER_MODE)pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.FRAME_CONFIG.FRAME_SEQ_MODE;
			}
		}

		if (pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::PULSE_1)] > RESET)
		{
			--pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::PULSE_1)];

			pNES_instance->NES_state.audio.effectivelengthCounterHaltFlag[TO_UINT8(AUDIO_CHANNELS::PULSE_1)]
				= (FLAG)(pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_VOL.LOOP_ENV_OR_DIS_LENGTH_COUNTER);
		}
		if (pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::PULSE_2)] > RESET)
		{
			--pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::PULSE_2)];

			pNES_instance->NES_state.audio.effectivelengthCounterHaltFlag[TO_UINT8(AUDIO_CHANNELS::PULSE_2)]
				= (FLAG)(pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_VOL.LOOP_ENV_OR_DIS_LENGTH_COUNTER);
		}
		if (pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)] > RESET)
		{
			--pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)];

			pNES_instance->NES_state.audio.effectivelengthCounterHaltFlag[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)]
				= (FLAG)(pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_LINEAR.DIS_LENGTH_COUNTER);
		}
		if (pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::NOISE)] > RESET)
		{
			--pNES_instance->NES_state.audio.lengthCounterHaltDelay[TO_UINT8(AUDIO_CHANNELS::NOISE)];

			pNES_instance->NES_state.audio.effectivelengthCounterHaltFlag[TO_UINT8(AUDIO_CHANNELS::NOISE)]
				= (FLAG)(pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_VOL.LOOP_ENV_OR_DIS_LENGTH_COUNTER);
		}
	}

	if (ENABLED)
	{
		generateLogicalOutput(AUDIO_CHANNELS::PULSE_1);
		generateLogicalOutput(AUDIO_CHANNELS::PULSE_2);
		generateLogicalOutput(AUDIO_CHANNELS::TRIANGLE);
		generateLogicalOutput(AUDIO_CHANNELS::NOISE);
		generateLogicalOutput(AUDIO_CHANNELS::DMC);
		captureDownsampledAudioSamples();
	}

	++pNES_instance->NES_state.emulatorStatus.ticks.apuCounter;
}

void NES_t::captureIO()
{
	pNES_instance->NES_state.controller.keyStatus = (
		(byte)ImGui::IsKeyDown(ImGuiKey_Z)
		| ((byte)ImGui::IsKeyDown(ImGuiKey_X) << ONE)
		| ((byte)ImGui::IsKeyDown(ImGuiKey_Space) << TWO)
		| ((byte)ImGui::IsKeyDown(ImGuiKey_Enter) << THREE)
		| ((byte)ImGui::IsKeyDown(ImGuiKey_UpArrow) << FOUR)
		| ((byte)ImGui::IsKeyDown(ImGuiKey_DownArrow) << FIVE)
		| ((byte)ImGui::IsKeyDown(ImGuiKey_LeftArrow) << SIX)
		| ((byte)ImGui::IsKeyDown(ImGuiKey_RightArrow) << SEVEN)
		);
}

void NES_t::captureDownsampledAudioSamples()
{
	pNES_instance->NES_state.audio.downSamplingRatioCounter += ONE;

	if (pNES_instance->NES_state.audio.downSamplingRatioCounter >= ((uint32_t)(NES_CPU_CLOCK_HZ / EMULATED_AUDIO_SAMPLING_RATE_FOR_NES)))
	{
		pNES_instance->NES_state.audio.downSamplingRatioCounter -= ((uint32_t)(NES_CPU_CLOCK_HZ / EMULATED_AUDIO_SAMPLING_RATE_FOR_NES));

		NES_AUDIO_SAMPLE_TYPE sample = MUTE_AUDIO;

		NES_AUDIO_SAMPLE_TYPE pulse_out = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE tnd_out = MUTE_AUDIO;

		NES_AUDIO_SAMPLE_TYPE pulse1 = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE pulse2 = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE triangle = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE noise = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE dmc = MUTE_AUDIO;

		if (DISABLE_FIRST_PULSE_CHANNEL == NO)
		{
			pulse1 = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].dacInput;
		}
		if (DISABLE_SECOND_PULSE_CHANNEL == NO)
		{
			pulse2 = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].dacInput;
		}
		if (DISABLE_TRIANGLE_CHANNEL == NO)
		{
			triangle = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].dacInput;
		}
		if (DISABLE_NOISE_CHANNEL == NO)
		{
			noise = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].dacInput;
		}
		if (DISABLE_DMC_CHANNEL == NO)
		{
			dmc = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dacInput;
		}

		// APU Mixer (Refer : https://www.nesdev.org/wiki/APU_Mixer)

#if (ENABLE_ACCURATE_AUDIO == YES)
		if ((pulse1 > MUTE_AUDIO) || (pulse2 > MUTE_AUDIO))
		{
			pulse_out = 95.88f / ((8128.0f / (pulse1 + pulse2)) + 100.0f);
		}
		if ((triangle > MUTE_AUDIO) || (noise > MUTE_AUDIO) || (dmc > MUTE_AUDIO))
		{
			tnd_out = 159.79f / ((1.0f / ((triangle / 8227.0f) + (noise / 12241.0f) + (dmc / 22638.0f))) + 100.0f);
		}
#else
		pulse_out = (0.00752f * (pulse1 + pulse2));
		tnd_out = ((0.00851f * triangle) + (0.00494f * noise) + (0.00335f * dmc));
#endif

		sample = pulse_out + tnd_out;

		// Filtering (Refer to https://forums.nesdev.org/viewtopic.php?p=163208&sid=d03a5db1dac3bb54d60b81c8ba009f66#p163208)
		// 1) Low Pass
		pNES_instance->NES_state.audio.LP_In = sample;
		pNES_instance->NES_state.audio.LP_Out = (pNES_instance->NES_state.audio.LP_In - pNES_instance->NES_state.audio.LP_Out) * 0.815686f;
		// 2) High Pass A
		pNES_instance->NES_state.audio.HPA_Out = pNES_instance->NES_state.audio.HPA_Out * 0.996039f + pNES_instance->NES_state.audio.LP_Out - pNES_instance->NES_state.audio.HPA_Prev;
		pNES_instance->NES_state.audio.HPA_Prev = pNES_instance->NES_state.audio.LP_Out;
		// 3) High Pass B
		pNES_instance->NES_state.audio.HPB_Out = pNES_instance->NES_state.audio.HPB_Out * 0.999835f + pNES_instance->NES_state.audio.HPA_Out - pNES_instance->NES_state.audio.HPB_Prev;
		pNES_instance->NES_state.audio.HPB_Prev = pNES_instance->NES_state.audio.HPA_Out;

		sample = pNES_instance->NES_state.audio.HPB_Out;

#if DISABLED
		sample = std::clamp(sample, (float)(-1.0), (float)(1.0)) * 100.0;
#else
		sample *= 100.0f;
#endif

		if (pNES_instance->NES_state.audio.accumulatedTone >= AUDIO_BUFFER_SIZE_FOR_NES)
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

			if (SDL_PutAudioStreamData(audioStream, pNES_instance->NES_state.audio.audioBuffer, sizeof(pNES_instance->NES_state.audio.audioBuffer)) == FAILURE)
			{
				SDL_Log("Could not put data on Audio stream, %s", SDL_GetError());
			}
			pNES_instance->NES_state.audio.accumulatedTone = RESET;
		}
		else
		{
			pNES_instance->NES_state.audio.audioBuffer[pNES_instance->NES_state.audio.accumulatedTone] = sample;
			++pNES_instance->NES_state.audio.accumulatedTone;
		}
	}
}

void NES_t::playTheAudioFrame()
{
	;
}

// NOTE: This function is used only when we a have a need to restore the graphics from scratch, for example load/save states
void NES_t::displayCompleteScreen()
{
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glDisable(GL_BLEND);

	// Handle for system's texture

	glBindTexture(GL_TEXTURE_2D, nes_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pNES_instance->NES_state.display.imGuiBuffer.imGuiBuffer1D);

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
	// 1. Upload emulator framebuffer to nes_texture
	GL_CALL(glBindTexture(GL_TEXTURE_2D, nes_texture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
		(GLvoid*)pNES_instance->NES_state.display.imGuiBuffer.imGuiBuffer1D));

	// Choose filtering mode (NEAREST or LINEAR)
	GLint filter = (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER) ? GL_LINEAR : GL_NEAREST;
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));

	// 2. Render nes_texture into framebuffer (masquerade_texture target)
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
	GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// Pass 1: Render base texture (Game Boy framebuffer)
	GL_CALL(glUseProgram(shaderProgramBasic));
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, nes_texture));
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

void NES_t::initializeGraphics()
{
	palScreen[0x00] = Pixel(84, 84, 84);
	palScreen[0x01] = Pixel(0, 30, 116);
	palScreen[0x02] = Pixel(8, 16, 144);
	palScreen[0x03] = Pixel(48, 0, 136);
	palScreen[0x04] = Pixel(68, 0, 100);
	palScreen[0x05] = Pixel(92, 0, 48);
	palScreen[0x06] = Pixel(84, 4, 0);
	palScreen[0x07] = Pixel(60, 24, 0);
	palScreen[0x08] = Pixel(32, 42, 0);
	palScreen[0x09] = Pixel(8, 58, 0);
	palScreen[0x0A] = Pixel(0, 64, 0);
	palScreen[0x0B] = Pixel(0, 60, 0);
	palScreen[0x0C] = Pixel(0, 50, 60);
	palScreen[0x0D] = Pixel(0, 0, 0);
	palScreen[0x0E] = Pixel(0, 0, 0);
	palScreen[0x0F] = Pixel(0, 0, 0);
	
	palScreen[0x10] = Pixel(152, 150, 152);
	palScreen[0x11] = Pixel(8, 76, 196);
	palScreen[0x12] = Pixel(48, 50, 236);
	palScreen[0x13] = Pixel(92, 30, 228);
	palScreen[0x14] = Pixel(136, 20, 176);
	palScreen[0x15] = Pixel(160, 20, 100);
	palScreen[0x16] = Pixel(152, 34, 32);
	palScreen[0x17] = Pixel(120, 60, 0);
	palScreen[0x18] = Pixel(84, 90, 0);
	palScreen[0x19] = Pixel(40, 114, 0);
	palScreen[0x1A] = Pixel(8, 124, 0);
	palScreen[0x1B] = Pixel(0, 118, 40);
	palScreen[0x1C] = Pixel(0, 102, 120);
	palScreen[0x1D] = Pixel(0, 0, 0);
	palScreen[0x1E] = Pixel(0, 0, 0);
	palScreen[0x1F] = Pixel(0, 0, 0);
	
	palScreen[0x20] = Pixel(236, 238, 236);
	palScreen[0x21] = Pixel(76, 154, 236);
	palScreen[0x22] = Pixel(120, 124, 236);
	palScreen[0x23] = Pixel(176, 98, 236);
	palScreen[0x24] = Pixel(228, 84, 236);
	palScreen[0x25] = Pixel(236, 88, 180);
	palScreen[0x26] = Pixel(236, 106, 100);
	palScreen[0x27] = Pixel(212, 136, 32);
	palScreen[0x28] = Pixel(160, 170, 0);
	palScreen[0x29] = Pixel(116, 196, 0);
	palScreen[0x2A] = Pixel(76, 208, 32);
	palScreen[0x2B] = Pixel(56, 204, 108);
	palScreen[0x2C] = Pixel(56, 180, 204);
	palScreen[0x2D] = Pixel(60, 60, 60);
	palScreen[0x2E] = Pixel(0, 0, 0);
	palScreen[0x2F] = Pixel(0, 0, 0);
	
	palScreen[0x30] = Pixel(236, 238, 236);
	palScreen[0x31] = Pixel(168, 204, 236);
	palScreen[0x32] = Pixel(188, 188, 236);
	palScreen[0x33] = Pixel(212, 178, 236);
	palScreen[0x34] = Pixel(236, 174, 236);
	palScreen[0x35] = Pixel(236, 174, 212);
	palScreen[0x36] = Pixel(236, 180, 176);
	palScreen[0x37] = Pixel(228, 196, 144);
	palScreen[0x38] = Pixel(204, 210, 120);
	palScreen[0x39] = Pixel(180, 222, 120);
	palScreen[0x3A] = Pixel(168, 226, 144);
	palScreen[0x3B] = Pixel(152, 226, 180);
	palScreen[0x3C] = Pixel(160, 214, 228);
	palScreen[0x3D] = Pixel(160, 162, 160);
	palScreen[0x3E] = Pixel(0, 0, 0);
	palScreen[0x3F] = Pixel(0, 0, 0);

	pNES_instance->NES_state.display.currentScanline = NES_PRE_RENDER_SCANLINE;
}

float NES_t::getEmulationVolume()
{
	pNES_instance->NES_state.audio.emulatorVolume = SDL_GetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream));
	RETURN pNES_instance->NES_state.audio.emulatorVolume;
}

void NES_t::setEmulationVolume(float volume)
{
	pNES_instance->NES_state.audio.emulatorVolume = volume;
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), volume);
	pt.put("nes._volume", volume);
	boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, pt);
}

void NES_t::initializeAudio()
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	const SDL_AudioSpec AudioSettings{ SDL_AUDIO_F32, ONE, TO_UINT(EMULATED_AUDIO_SAMPLING_RATE_FOR_NES) };
	audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &AudioSettings, NULL, NULL);
	SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));

	pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer = RESET;
	pNES_instance->NES_state.emulatorStatus.ticks.apuCounter = RESET; // APU ticks = 0
	pNES_instance->NES_state.audio.isReset = YES;

	pNES_instance->NES_state.audio.emulatorVolume = pt.get<std::float_t>("nes._volume");
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), pNES_instance->NES_state.audio.emulatorVolume);

	// Refer to https://forums.nesdev.org/viewtopic.php?p=163157&sid=d5d3c2ba788e71c4b0d23d7651bb7dd5#p163157
	pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseShiftRegister.raw = ONE;

	// Refer to https://forums.nesdev.org/viewtopic.php?p=163287#p163287
	pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.bitsInOutputUnit = ONE;
}

// TODO: Below function is not used for now...
void NES_t::reInitializeAudio()
{
	if (_ENABLE_AUDIO == YES)
	{
		;
	}
}

bool NES_t::runEmulationAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

bool NES_t::runEmulationLoopAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

bool NES_t::runEmulationAtFixedRate(uint32_t currentFrame)
{
	bool status = true;

#if (DISABLED)
	pNES_instance->NES_state.emulatorStatus.debugger.wasDebuggerJustTriggerred = CLEAR;
#endif

	loadQuirks();

	if (pNES_instance->NES_state.controller.startPolling == YES && pNES_instance->NES_state.controller.endPolling == NO)
	{
		pNES_instance->NES_state.controller.keyID = KEY_A;
		captureIO();
	}

	playTheAudioFrame();

	displayCompleteScreen();

	// To handle 3rd test of ppu_open_bus.nes 
	// Check for 600 ms decay rate for open bus
	auto currentTimeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if ((currentTimeInMs - pNES_ppuRegisters->ppuInternalRegisters.openBus.lastRefreshTimeInMs) > (600 / _XFPS))
	{
		pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue = RESET;
	}

	RETURN status;
}

bool NES_t::runEmulationLoopAtFixedRate(uint32_t currentFrame)
{
	pNES_instance->NES_state.display.wasVblankJustTriggerred = NO;

	if (ROM_TYPE == ROM::TEST_SST)
	{
		static FLAG SST_DEBUG_PRINT = NO;
		const COUNTER32 init_test_opcode = 0x00;
		COUNTER32 opcode = init_test_opcode;
		while (FOREVER)
		{
			if ((init_test_opcode != ZERO) && (opcode == init_test_opcode))
			{
				SST_DEBUG_PRINT = YES;
			}
			else
			{
				SST_DEBUG_PRINT = NO;
			}

			if (opcode > 0xFF)
			{
				INFO("Completed Running all Tom Harte nes6502 (v1) tests");
				PAUSE;
			}

			// Get the input
			std::string testCaseName = std::format("{:02x}", opcode);
			testCaseName = _JSON_LOCATION + "\\" + testCaseName + ".json";
			//std::string testCaseName = _JSON_LOCATION + "\\" +  "test.json";

			LOG_NEW_LINE;
			INFO("Running : %s", testCaseName.c_str());
			try
			{
				boost::property_tree::read_json(testCaseName, testCase);
			}
			catch (std::exception& ex)
			{
				std::cout << ex.what() << std::endl;
				RETURN false;
			}

			// Test the CPU!

			// Itterate over each test case in the JSON array
			for (const auto& item : testCase)
			{
				auto quitThisRun = NO;

				// Accessing top-level fields
				std::string name = item.second.get<std::string>("name");
				if (SST_DEBUG_PRINT)
				{
					std::cout << "Name: " << name << std::endl;
				}

				// Accessing initial state
				auto initial = item.second.get_child("initial");
				int initial_pc = initial.get<int>("pc");
				int initial_s = initial.get<int>("s");
				int initial_a = initial.get<int>("a");
				int initial_x = initial.get<int>("x");
				int initial_y = initial.get<int>("y");
				int initial_p = initial.get<int>("p");

				if (SST_DEBUG_PRINT)
				{
					std::cout << "Initial PC: " << initial_pc << ", S: " << initial_s
						<< ", A: " << initial_a << ", X: " << initial_x
						<< ", Y: " << initial_y << ", P: " << initial_p << std::endl;
				}

				pNES_cpuRegisters->pc = initial_pc;
				pNES_cpuRegisters->sp = initial_s;
				pNES_cpuRegisters->a = initial_a;
				pNES_cpuRegisters->x = initial_x;
				pNES_cpuRegisters->y = initial_y;
				pNES_cpuRegisters->p.p = initial_p;

				// Accessing RAM in initial state
				if (SST_DEBUG_PRINT)
				{
					std::cout << "Initial RAM:" << std::endl;
				}
				for (const auto& ram_entry : initial.get_child("ram"))
				{
					auto it = ram_entry.second.begin();
					int address = it->second.get_value<int>(); // First element is the address
					++it; // Move to the second element
					int value = it->second.get_value<int>(); // Second element is the value
					if (SST_DEBUG_PRINT)
					{
						std::cout << "  Address: " << address << ", Value: " << value << std::endl;
					}

					pNES_cpuMemory->NESRawMemory[address] = value;
				}

				// Run the CPU
				processSOC();

				// Accessing final state
				auto final = item.second.get_child("final");
				int final_pc = final.get<int>("pc");
				int final_s = final.get<int>("s");
				int final_a = final.get<int>("a");
				int final_x = final.get<int>("x");
				int final_y = final.get<int>("y");
				int final_p = final.get<int>("p");

				if (SST_DEBUG_PRINT)
				{
					std::cout << "Final PC: " << final_pc << ", S: " << final_s
						<< ", A: " << final_a << ", X: " << final_x
						<< ", Y: " << final_y << ", P: " << final_p << std::endl;
				}

				if (pNES_cpuRegisters->pc != final_pc)
				{
					WARN("PC Mismatch");
					quitThisRun = YES;
				}
				if (pNES_cpuRegisters->sp != final_s)
				{
					WARN("SP Mismatch");
					quitThisRun = YES;
				}
				if (pNES_cpuRegisters->a != final_a)
				{
					WARN("A Mismatch");
					quitThisRun = YES;
				}
				if (pNES_cpuRegisters->x != final_x)
				{
					WARN("X Mismatch");
					quitThisRun = YES;
				}
				if (pNES_cpuRegisters->y != final_y)
				{
					WARN("Y Mismatch");
					quitThisRun = YES;
				}
				if (pNES_cpuRegisters->p.p != final_p)
				{
					WARN("P Mismatch");
					quitThisRun = YES;
				}

				pNES_cpuRegisters->pc = RESET;
				pNES_cpuRegisters->sp = RESET;
				pNES_cpuRegisters->a = RESET;
				pNES_cpuRegisters->x = RESET;
				pNES_cpuRegisters->y = RESET;
				pNES_cpuRegisters->p.p = RESET;

				// Accessing RAM in final state
				if (SST_DEBUG_PRINT)
				{
					std::cout << "Final RAM:" << std::endl;
				}

				for (const auto& ram_entry : final.get_child("ram"))
				{
					auto it = ram_entry.second.begin();
					int address = it->second.get_value<int>(); // First element is the address
					++it; // Move to the second element
					int value = it->second.get_value<int>(); // Second element is the value
					if (SST_DEBUG_PRINT)
					{
						std::cout << "  Address: " << address << ", Value: " << value << std::endl;
					}

					if (pNES_cpuMemory->NESRawMemory[address] != value)
					{
						WARN("RAM Mismatch");
						quitThisRun = YES;
					}

					pNES_cpuMemory->NESRawMemory[address] = RESET;
				}

				// Accessing cycles
				if (SST_DEBUG_PRINT)
				{
					std::cout << "Cycles:" << std::endl;
				}
				pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.indexer = RESET;
				INC8 indexer = RESET;
				for (const auto& cycle : item.second.get_child("cycles"))
				{
					auto it = cycle.second.begin();
					int cycle_address = it->second.get_value<int>(); // First element
					++it; // Move to the second element
					int cycle_value = it->second.get_value<int>(); // Second element
					++it; // Move to the third element
					std::string cycle_type = it->second.get_value<std::string>(); // Third element
					if (SST_DEBUG_PRINT)
					{
						std::cout << "Cycle Address: " << cycle_address << ", Value: " << cycle_value << ", Type: " << cycle_type << std::endl;
					}

					if (cycle_address != pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].address)
					{
						WARN("Address Cycle Mismatch");
						quitThisRun = YES;
					}

					if (cycle_value != pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].data)
					{
						WARN("Data Cycle Mismatch");
						quitThisRun = YES;
					}

					std::string temp = "write";
					if (pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].isRead == YES)
					{
						temp = "read";
					}

					if (cycle_type.compare(temp))
					{
						WARN("Operation Cycle Mismatch");
						quitThisRun = YES;
					}

					pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].reset();
					++indexer;
				}

				if (quitThisRun == YES)
				{
					BREAK;
				}

				// Update Stats
				++pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.testCount[opcode];
			}

			++opcode;
		}
	}
	else
	{
		processSOC();

#if (DISABLED)
		runDebugger();
#endif
	}

	RETURN pNES_instance->NES_state.display.wasVblankJustTriggerred;
}

bool NES_t::initializeEmulator()
{
	bool status = true;

	pAbsolute_NES_instance = std::make_shared<absolute_NES_instance_t>();

	// for readability

	pNES_instance = (NES_instance_t*)&(pAbsolute_NES_instance->absolute_NES_state.NES_instance);
	pINES = (iNES_t*)&(pAbsolute_NES_instance->absolute_NES_state.aboutRom.iNES);
	pNES_cpuRegisters = &(pNES_instance->NES_state.cpuRegisters);
	pNES_cpuInstance = &(pNES_instance->NES_state.cpuInstance);
	pNES_cpuMemory = &(pNES_instance->NES_state.cpuMemory);
	pNES_ppuRegisters = &(pNES_instance->NES_state.ppuRegisters);
	pNES_ppuMemory = &(pNES_instance->NES_state.ppuMemory);
	pNES_catridgeMemory = &(pNES_instance->NES_state.catridgeMemory);
	pNES_flags = &(pNES_cpuRegisters->p.flagFields);

	pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter = RESET;

	// other initializations
	pNES_cpuRegisters->pc = 0x0000;
	pNES_cpuRegisters->sp = 0x00;
	pNES_cpuRegisters->p.p = 0x00;

	pNES_instance->NES_state.controller.keyID = INVALID;

	// initialize memory to zero
	memset(pAbsolute_NES_instance->NES_absoluteMemoryState, RESET, sizeof(pAbsolute_NES_instance->NES_absoluteMemoryState));
	memset(pNES_catridgeMemory->maxCatridgePRGROM, RESET, sizeof(pNES_catridgeMemory->maxCatridgePRGROM));
	memset(pNES_catridgeMemory->maxCatridgeCHRROM, RESET, sizeof(pNES_catridgeMemory->maxCatridgeCHRROM));

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

		glGenTextures(1, &nes_texture);
		glBindTexture(GL_TEXTURE_2D, nes_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pNES_instance->NES_state.display.imGuiBuffer.imGuiBuffer1D);
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

		// 3. NES texture (used to upload emulated framebuffer)
		GL_CALL(glGenTextures(1, &nes_texture));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, nes_texture));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pNES_instance->NES_state.display.imGuiBuffer.imGuiBuffer1D));
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
	}

	RETURN status;
}

void NES_t::destroyEmulator()
{
	bool status = true;

	// Saving PRG RAM

	if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.hasPersistantMemory == YES)
	{
		std::filesystem::path saveDirectory(_SAVE_LOCATION);
		if (!(std::filesystem::exists(saveDirectory)))
		{
			std::filesystem::create_directory(saveDirectory);
		}

		TODO("Need a better way to generate unique ID as current method wastes some time");
		std::string saveFileNameForThisROM = getSaveFileName(
			pNES_catridgeMemory->maxCatridgePRGROM
			, 0xFFFF
		);

		saveFileNameForThisROM = _SAVE_LOCATION + "\\" + saveFileNameForThisROM;

		std::cout << "\nSaving to " << saveFileNameForThisROM << std::endl;

		std::ofstream outSRAM(saveFileNameForThisROM.c_str(), std::ios_base::binary);

		if (outSRAM.fail() == NO)
		{
			for (INC16 address = CATRIDGE_RAM_START_ADDRESS; address < CATRIDGE_RAM_END_ADDRESS; address++)
			{
				BYTE ramByte = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
				outSRAM.write(reinterpret_cast<const char*> (&ramByte), ONE);
			}

			outSRAM.flush();
		}

		outSRAM.close();
	}

	logCounter = ZERO;
	memset(emulationCounter, ZERO, ((sizeof(emulationCounter[100])) / sizeof(emulationCounter[0])));

	pINES = nullptr;
	pNES_catridgeMemory = nullptr;
	pNES_instance = nullptr;
	pNES_cpuRegisters = nullptr;
	pNES_cpuInstance = nullptr;
	pNES_cpuMemory = nullptr;
	pNES_ppuRegisters = nullptr;
	pNES_ppuMemory = nullptr;
	pNES_flags = nullptr;

	pAbsolute_NES_instance.reset();

#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glDeleteTextures(1, &nes_texture);
	glDeleteTextures(1, &matrix_texture);
#else
	glDeleteTextures(1, &nes_texture);
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

bool NES_t::loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom)
{
	// open the rom file

	FILE* fp = NULL;
	uint32_t totalRomSize = 0;
	uint32_t totalAuxilaryRomSize = 0;

	if (ROM_TYPE == ROM::NES)
	{
		errno_t err = fopen_portable(&fp, rom[ZERO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_NES_instance->absolute_NES_state.aboutRom.codeRomSize = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pINES->completeROM + 0x0000, pAbsolute_NES_instance->absolute_NES_state.aboutRom.codeRomSize, 1, fp);
			fclose(fp);

			// decode mapper information
			pNES_instance->NES_state.catridgeInfo.mapperID
				= pINES->iNES_Fields.iNES_header.fields.flag6.fields.mapperLo
				| (pINES->iNES_Fields.iNES_header.fields.flag7.fields.mapperHi << FOUR);

			// decode nametable arrangement
			if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.nametableArrangement == RESET)
			{
				pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
			}
			else
			{
				pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
			}

			if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.alternativeNametable == SET)
			{
				FATAL("Handle alternate nametable arrangement");
			}

			// check whether trainer is present
			FLAG isTrainerPresent = (FLAG)pINES->iNES_Fields.iNES_header.fields.flag6.fields.trainerPresent;

			BYTE* romData = nullptr;

			if (isTrainerPresent == YES)
			{
				romData = pINES->iNES_Fields.remaining.withTrainer.romData;
			}
			else
			{
				romData = pINES->iNES_Fields.remaining.withoutTrainer.romData;
			}

			switch (pNES_instance->NES_state.catridgeInfo.mapper)
			{
			case MAPPER::NROM:
			{
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB == ONE)
				{
					memcpy_portable(
						&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
						, 0x4000
						, romData
						, 0x4000);

					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ONE)
					{
						memcpy_portable(
							pNES_instance->NES_state.ppuMemory.NESMemoryMap.patternTable.raw
							, 0x2000
							, &(romData[0x4000])
							, 0x2000);
					}

					// If PRG ROM is 16 KB, then crom0 and crom1 are mirrored

					memcpy_portable(
						&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
						, 0x4000
						, &(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
						, 0x4000);
				}
				else if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB == TWO)
				{
					memcpy_portable(
						&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
						, 0x4000
						, romData
						, 0x4000);

					memcpy_portable(
						&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
						, 0x4000
						, &(romData[0x4000])
						, 0x4000);

					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ONE)
					{
						memcpy_portable(
							pNES_instance->NES_state.ppuMemory.NESMemoryMap.patternTable.raw
							, 0x2000
							, &(romData[0x8000])
							, 0x2000);
					}
				}
				else
				{
					FATAL("Invalid PRG ROM size for NROM");
				}

				BREAK;
			}
			case MAPPER::MMC1:
			{
				pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.raw = 0x1C;
				pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Lo = ZERO;
				pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Hi = pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB - ONE;
				pNES_instance->NES_state.catridgeInfo.mmc1.prgBank32 = ZERO;
				pNES_instance->NES_state.catridgeInfo.mmc1.chrBank4Lo = ZERO;
				pNES_instance->NES_state.catridgeInfo.mmc1.chrBank4Hi = ZERO;
				pNES_instance->NES_state.catridgeInfo.mmc1.chrBank8 = ZERO;

				if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB >= ONE)
				{
					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
							, 0x4000
							, romData
							, 0x4000);

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB == ONE)
						{
							TODO("If PRG ROM is 16 KB, is crom0 and crom1 mirrored in mmc1 as well?");

							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
								, 0x4000
								, &(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
								, 0x4000);
						}
						else
						{
							// Storing the last bank

							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory
									[0x3FE0 + 0x4000])
								, 0x4000
								, &(romData[((pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB - ONE) * 0x4000)])
								, 0x4000);
						}

						// CHR ROM
						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
						{
							memcpy_portable(
								pNES_instance->NES_state.ppuMemory.NESMemoryMap.patternTable.raw
								, 0x2000
								, &(romData[pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000])
								, 0x2000);
						}
					}

					//

					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_catridgeMemory->maxCatridgePRGROM)
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000
							, romData
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000);

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
						{
							memcpy_portable(
								pNES_catridgeMemory->maxCatridgeCHRROM
								, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000
								, &(romData[pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000])
								, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000);
						}
					}
				}
				BREAK;
			}
			case MAPPER::UxROM_002:
			{
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB >= ONE)
				{
					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
							, 0x4000
							, romData
							, 0x4000);

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB == ONE)
						{
							TODO("If PRG ROM is 16 KB, is crom0 and crom1 mirrored in uxrom as well?");

							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
								, 0x4000
								, &(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
								, 0x4000);
						}
						else
						{
							// Storing the last bank

							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory
									[0x3FE0 + 0x4000])
								, 0x4000
								, &(romData[((pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB - ONE) * 0x4000)])
								, 0x4000);
						}

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
						{
							memcpy_portable(
								pNES_instance->NES_state.ppuMemory.NESMemoryMap.patternTable.raw
								, 0x2000
								, &(romData[pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000])
								, 0x2000);
						}
					}

					//

					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_catridgeMemory->maxCatridgePRGROM)
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000
							, romData
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000);

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
						{
							memcpy_portable(
								pNES_catridgeMemory->maxCatridgeCHRROM
								, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000
								, &(romData[pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000])
								, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000);
						}
					}
				}

				BREAK;
			}
			case MAPPER::CNROM:
			{
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB == ONE)
				{
					memcpy_portable(
						&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
						, 0x4000
						, romData
						, 0x4000);

					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
					{
						memcpy_portable(
							pNES_instance->NES_state.ppuMemory.NESMemoryMap.patternTable.raw
							, 0x2000
							, &(romData
								[0x4000])
							, 0x2000);
					}

					memcpy_portable(
						pNES_catridgeMemory->maxCatridgePRGROM
						, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000
						, romData
						, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000);

					memcpy_portable(
						pNES_catridgeMemory->maxCatridgeCHRROM
						, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000
						, &(romData
							[0x4000])
						, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000);

					TODO("If PRG ROM is 16 KB, is crom0 and crom1 mirrored in cnrom as well?");

					memcpy_portable(
						&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
						, 0x4000
						, &(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
						, 0x4000);
				}
				else if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB == TWO)
				{
					memcpy_portable(
						&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
						, 0x4000
						, romData
						, 0x4000);

					memcpy_portable(
						&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
						, 0x4000
						, &(romData[0x4000])
						, 0x4000);

					if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
					{
						memcpy_portable(
							pNES_instance->NES_state.ppuMemory.NESMemoryMap.patternTable.raw
							, 0x2000
							, &(romData[0x8000])
							, 0x2000);
					}

					memcpy_portable(
						pNES_catridgeMemory->maxCatridgePRGROM
						, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000
						, romData
						, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000);

					memcpy_portable(
						pNES_catridgeMemory->maxCatridgeCHRROM
						, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000
						, &(romData[0x8000])
						, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000);
				}
				else
				{
					FATAL("Invalid PRG ROM size for CNROM");
				}

				BREAK;
			}
			case MAPPER::MMC3:
			{
				pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankData_odd8k = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.raw = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqDisable_evenEk = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqEnable_oddEk = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqReload_evenCk = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqReload_oddCk = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.mirroring_evenAk.raw = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.raw = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.filteredA12RiseEvent = NO;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqCounterReloadEnabled = NO;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqEnable = NO;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8a = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8b = 0x00;
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.unfilteredA12RiseEvent = 0x00;

				if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB >= ONE)
				{
					if (ENABLED)
					{
						// MMC3 has 4 8KB banks; lower 2 store first 2 banks and for optimization, we directly load first 16KB for rom
						memcpy_portable(
							&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
							, 0x4000
							, romData
							, 0x4000);

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB == ONE)
						{
							TODO("If PRG ROM is 16 KB, is crom0 and crom1 mirrored in mmc3 as well?");

							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
								, 0x4000
								, &(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
								, 0x4000);
						}
						else
						{
							// Storing the last bank and second to last
							// MMC3 has 4 8KB banks; higher 2 store last 2 banks and for optimization, we directly load last 16KB of rom

							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
								, 0x4000
								, &(romData[((pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB - ONE) * 0x4000)])
								, 0x4000);
						}

						// CHR ROM
						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
						{
							memcpy_portable(
								pNES_instance->NES_state.ppuMemory.NESMemoryMap.patternTable.raw
								, 0x2000
								, &(romData[pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000])
								, 0x2000);
						}
					}

					//

					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_catridgeMemory->maxCatridgePRGROM)
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000
							, romData
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000);

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
						{
							memcpy_portable(
								pNES_catridgeMemory->maxCatridgeCHRROM
								, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000
								, &(romData[pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000])
								, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000);
						}
					}
				}
				BREAK;
			}
			case MAPPER::AxROM:
			{
				pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;

				if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB >= ONE)
				{
					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
							, 0x4000
							, romData
							, 0x4000);

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB == ONE)
						{
							TODO("If PRG ROM is 16 KB, is crom0 and crom1 mirrored in axrom as well?");

							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
								, 0x4000
								, &(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
								, 0x4000);
						}
						else
						{
							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
								, 0x4000
								, &(romData[0x4000])
								, 0x4000);
						}

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB > ZERO)
						{
							FATAL("Invalid CHR ROM size for AxROM");
						}
					}

					//

					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_catridgeMemory->maxCatridgePRGROM)
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000
							, romData
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000);
					}
				}

				BREAK;
			}
			case MAPPER::GxROM:
			{
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB >= ONE)
				{
					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
							, 0x4000
							, romData
							, 0x4000);

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB == ONE)
						{
							TODO("If PRG ROM is 16 KB, is crom0 and crom1 mirrored in gxrom as well?");

							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0 + 0x4000])
								, 0x4000
								, &(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
								, 0x4000);
						}
						else
						{
							memcpy_portable(
								&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory
									[0x3FE0 + 0x4000])
								, 0x4000
								, &(romData[0x4000])
								, 0x4000);
						}

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
						{
							memcpy_portable(
								pNES_instance->NES_state.ppuMemory.NESMemoryMap.patternTable.raw
								, 0x2000
								, &(romData[pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000])
								, 0x2000);
						}
					}

					//

					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_catridgeMemory->maxCatridgePRGROM)
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000
							, romData
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000);

						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
						{
							memcpy_portable(
								pNES_catridgeMemory->maxCatridgeCHRROM
								, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000
								, &(romData[pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000])
								, pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 0x2000);
						}
					}
				}

				BREAK;
			}
			case MAPPER::NANJING_FC001:
			{
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB >= ONE)
				{
					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0])
							, 0x8000
							, romData
							, 0x8000);


						// CHR ROM
						if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB >= ONE)
						{
							FATAL("NANJING_FC001 doesn't support CHR ROM");
						}
					}

					//

					if (ENABLED)
					{
						memcpy_portable(
							&(pNES_catridgeMemory->maxCatridgePRGROM)
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000
							, romData
							, pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 0x4000);
					}

					TODO("For NANJING_FC001, As mentioned in NesDev, ROM should be dumped with  $5300=$04 as this determines rom bank, but not sure how to map this initially");
					TODO("So, for now, $5300's A bit is by default set to 1, as keeping this zero make games boot from bank 3");
					pNES_instance->NES_state.catridgeInfo.nanjing_fc001.A = (FLAG)SET;

				}
				BREAK;
			}
			default:
			{
				FATAL("Unsupported Mapper : %u", pNES_instance->NES_state.catridgeInfo.mapperID);
			}
			}

			// Display some of the Cartridge information
			if (ENABLED)
			{
				LOG_NEW_LINE;
				LOG("Cartridge Loaded:\n");
				if (pINES->iNES_Fields.iNES_header.fields.flag10.fields.tvSystem == ONE
					|| pINES->iNES_Fields.iNES_header.fields.flag10.fields.tvSystem == THREE)
				{
					LOG(" TV System : %s", "NTSC / PAL");
				}
				else
				{
					if (pINES->iNES_Fields.iNES_header.fields.flag9.fields.tvSystem == ZERO)
					{
						LOG(" TV System : %s", "NTSC");
					}
					else
					{
						LOG(" TV System : %s", "PAL");
					}
				}
				LOG(" PRG ROM Size : %d KB", pINES->iNES_Fields.iNES_header.fields.sizeOfPrgRomIn16KB * 16);
				LOG(" CHR ROM Size : %d KB", pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB * 8);
				LOG(" PRG RAM Size : %d KB", pINES->iNES_Fields.iNES_header.fields.prgRamSize * 8);
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
				{
					LOG(" CHR RAM : %s", "YES");
				}
				else
				{
					LOG(" CHR RAM : %s", "NO");
				}
				LOG(" Mapper : %d", TO_UINT(pNES_instance->NES_state.catridgeInfo.mapper));
				if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.nametableArrangement == TO_UINT(NAMETABLE_MIRROR::VERTICAL_MIRROR))
				{
					LOG(" Name Table Arrangement : %s", "VERTICAL");
				}
				else if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.nametableArrangement == TO_UINT(NAMETABLE_MIRROR::HORIZONTAL_MIRROR))
				{
					LOG(" Name Table Arrangement : %s", "HORIZONTAL");
				}
				if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.hasPersistantMemory == YES)
				{
					LOG(" Battery Packed PRG RAM : %s", "YES");
				}
				else
				{
					LOG(" Battery Packed PRG RAM : %s", "NO");
				}
				if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.trainerPresent == YES)
				{
					LOG(" Trainer Present : %s", "YES");
				}
				else
				{
					LOG(" Trainer Present : %s", "NO");
				}
				if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.alternativeNametable == ZERO)
				{
					LOG(" Alternative Name Table : %s", "YES");
				}
				else
				{
					LOG(" Alternative Name Table : %s", "NO");
				}
				if (pINES->iNES_Fields.iNES_header.fields.flag7.fields.flags8_15Type == TWO)
				{
					LOG(" NES 2.0 : %s", "YES");
				}
				else
				{
					LOG(" NES 2.0 : %s", "NO");
				}
			}

			// Simulating the PLA ROM; Refer : https://www.pagetable.com/?p=410
			pNES_cpuRegisters->p.flagFields.FORCED_TO_ONE = ONE;
			pNES_cpuRegisters->p.flagFields.INTERRUPT_DISABLE = ONE;
			stackPush((pNES_cpuRegisters->pc & 0xFF00) >> EIGHT);
			stackPush(pNES_cpuRegisters->pc & 0x00FF);
			stackPush(pNES_cpuRegisters->p.p);

#if (NESTEST_AUTOMATED_MODE == YES)
			pNES_cpuRegisters->pc = 0xC000;
#else
			pNES_cpuRegisters->pc = (readCpuRawMemory(RESET_VECTOR_START_ADDRESS, MEMORY_ACCESS_SOURCE::CPU)
				+ (readCpuRawMemory(RESET_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU) << EIGHT));

			// Load PRG RAM

			if (pINES->iNES_Fields.iNES_header.fields.flag6.fields.hasPersistantMemory == YES)
			{
				TODO("Need a better way to generate unique ID as current method wastes time");
				std::string saveFileNameForThisROM = getSaveFileName(
					pNES_catridgeMemory->maxCatridgePRGROM
					, 0xFFFF
				);

				saveFileNameForThisROM = _SAVE_LOCATION + "\\" + saveFileNameForThisROM;

				std::cout << "\nAttempting to load " << saveFileNameForThisROM << std::endl;

				std::ifstream inSRAM(saveFileNameForThisROM.c_str(), std::ios::in | std::ios_base::binary);

				if (inSRAM.fail() == NO)
				{
					for (INC16 address = CATRIDGE_RAM_START_ADDRESS; address < CATRIDGE_RAM_END_ADDRESS; address++)
					{
						BYTE ramByte = ZERO;
						inSRAM.read(reinterpret_cast<char*> (&ramByte), ONE);
						writeCpuRawMemory(address, ramByte, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
					}
				}
			}
#endif
		}
		else
		{
			LOG("Failed to open file: %s", strerror(err));
		}
	}
	else if (ROM_TYPE == ROM::TEST_ROM_BIN)
	{
		errno_t err = fopen_portable(&fp, rom[ZERO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_NES_instance->absolute_NES_state.aboutRom.codeRomSize = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pNES_cpuMemory->NESRawMemory + 0x0000, pAbsolute_NES_instance->absolute_NES_state.aboutRom.codeRomSize, 1, fp);
			fclose(fp);

			pNES_cpuRegisters->sp = 0x00FF;
			pNES_cpuRegisters->pc = 0x0400;
		}
		else
		{
			LOG("Failed to open file: %s", strerror(err));
		}
	}
	else
	{
		RETURN FAILURE;
	}

	RETURN SUCCESS;
}

void NES_t::dumpRom()
{
	uint32_t scanner = 0;
	uint32_t addressField = 0x10;

	LOG("ROM DUMP\n");
	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (int ii = 0; ii < (int)pAbsolute_NES_instance->absolute_NES_state.aboutRom.codeRomSize; ii++)
	{
		LOG("0x%02x\t", pNES_instance->NES_state.cpuMemory.NESRawMemory[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	LOG("\n\n\n");
}

bool NES_t::getRomLoadedStatus()
{
	RETURN pAbsolute_NES_instance->absolute_NES_state.aboutRom.isRomLoaded;
}

#if (DISABLED)
void NES_t::runDebugger()
{

}
#endif
#pragma endregion EMULATION_DEFINITIONS