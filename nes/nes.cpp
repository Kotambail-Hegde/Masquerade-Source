#pragma region NES_SPECIFIC_INCLUDES
#include "nes.h"
#pragma endregion NES_SPECIFIC_INCLUDES

#pragma region CONDITIONAL_INCLUDES
#pragma endregion CONDITIONAL_INCLUDES

#pragma region NES_SPECIFIC_MACROS
#pragma region WIP
#define ENABLE_OAM_CORRUPTION							(YES)	// May not be accurate; read the TODO added in the code for more details.
#pragma endregion WIP

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
#define DISABLE_MMC5_CHANNELS							(NO)
#define DISABLE_VRC6_CHANNELS							(NO)
#define DISABLE_NAMCO163_CHANNELS						(NO)
#define ENABLE_AUDIO_MIXER_LUT							(YES) // If not enabled, falls back to less-accurate linear approximation.

#ifdef _MSC_VER  
#define __packed  
#pragma pack(1)  
#endif
#pragma endregion NES_SPECIFIC_MACROS

#pragma region NES_SPECIFIC_DECLARATIONS
// For debug
static COUNTER64 logCounter = ZERO;
static COUNTER64 nesEmulationCounter[100] = { ZERO };

static std::string _JSON_LOCATION;
static MasqConfig_t testCase;

static uint32_t nes_texture;
static uint32_t matrix_texture;
static uint32_t matrix[16] = { 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
#pragma endregion NES_SPECIFIC_DECLARATIONS

#pragma region INFRASTRUCTURE_DEFINITIONS
NES_t::NES_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, MasqConfig_t& config, CheatEngine_t* ce)
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
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_WARN);
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
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_WARN);
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

#if (ENABLE_R2A03_SST == YES)
		ROM_TYPE = ROM::TEST_SST;
#else
		FATAL("SSTs are not supported in this build");
		RETURN;
#endif
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
		_SAVE_LOCATION = pt.get<std::string>("nes._save_location", "");
		if (_SAVE_LOCATION.empty())
		{
			FATAL("Could not locate the save directory");
		}
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

void NES_t::setupTheCoreOfEmulation(void* masqueradeInstance, void* audio, void* input, void* network, void* camera)
{
	uint8_t indexToCheck = 0;

#if (ENABLE_R2A03_SST == YES)
	if (!rom[indexToCheck].empty() || ROM_TYPE == ROM::TEST_SST)
#else
	if (!rom[indexToCheck].empty())
#endif
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
	const auto mapper = pNES_instance->NES_state.catridgeInfo.mapper;

	// Mapper 014 only runs the MMC3 IRQ counter while its supervisor
	// register has switched the chip into MMC3 mode -- in VRC2 mode there
	// is no IRQ hardware active at all, and this counter must stay frozen.
	if (mapper == MAPPER::INES_MAPPER_014 && (pNES_instance->NES_state.catridgeInfo.ines014.supervisorReg & 0x10) == ZERO)
	{
		RETURN;
	}

	if (mapper != MAPPER::MMC3
		&& mapper != MAPPER::INES_MAPPER_014
		&& mapper != MAPPER::INES_MAPPER_037
		&& mapper != MAPPER::INES_MAPPER_047
		&& mapper != MAPPER::INES_MAPPER_119
		&& mapper != MAPPER::INES_MAPPER_118
		&& mapper != MAPPER::RAMBO1
		&& mapper != MAPPER::INES_MAPPER_158
		&& mapper != MAPPER::INES_MAPPER_268)
	{
		RETURN;
	}

	// RAMBO-1/158 in CPU cycle mode: A12 events don't affect IRQ counter
	if ((mapper == MAPPER::RAMBO1 || mapper == MAPPER::INES_MAPPER_158)
		&& pNES_instance->NES_state.catridgeInfo.mmc3.rambo1.irqCycleMode == YES)
	{
		RETURN;
	}

	// -----------------------------------------------------------------------
	//  KEY FIX: Only track A12 for CHR bus addresses ($0000-$1FFF).
	//  NT/AT addresses ($2000-$3FFF) go to CIRAM, not the CHR address bus.
	//  Allowing them to reset ppuCounterMMC3A12 causes sprite pattern fetches
	//  ($1000-$1FFF, which arrive 4-6 PPU ticks after dummy NT reads) to be
	//  filtered out by the >= 16 tick threshold, suppressing valid IRQ clocks.
	// -----------------------------------------------------------------------
	const bool isPatternTableAddress = (address < 0x2000);
	if (!isPatternTableAddress)
	{
		// Not a CHR bus address — do not touch ppuCounterMMC3A12 or the
		// A12 rise tracking. Just return.
		RETURN;
	}

	if (address & 0x1000) // A12 high
	{
		if (pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.unfilteredA12RiseEvent == NO)
		{
			pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.unfilteredA12RiseEvent = YES;

			if (pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterMMC3A12 >= SIXTEEN)
			{
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.filteredA12RiseEvent = YES;

				// --- RAMBO-1 scanline mode IRQ ---
				if (mapper == MAPPER::RAMBO1 || mapper == MAPPER::INES_MAPPER_158)
				{
					auto& rb = pNES_instance->NES_state.catridgeInfo.mmc3.rambo1;
					if (rb.needReload)
					{
						rb.irqCounter = (rb.irqReloadValue <= 1)
							? (rb.irqReloadValue + 1)
							: (rb.irqReloadValue + 2);
						rb.needReload = NO;
					}
					else if (rb.irqCounter == RESET)
					{
						rb.irqCounter = rb.irqReloadValue + 1;
					}
					if (rb.irqCounter > RESET)
					{
						--rb.irqCounter;
					}
					if (rb.irqCounter == RESET && rb.irqEnabled == YES)
					{
						rb.irqDelay = 2;
					}
				}
				else
				{
					// --- Standard MMC3 / 118 / 037 / 047 / 119 IRQ ---
					const BYTE count = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter;
					const FLAG reload = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqCounterReloadEnabled;

					if (count == RESET || reload == YES)
					{
						pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter
							= pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.irqReload_evenCk;
						pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqCounterReloadEnabled = CLEAR;
					}
					else
					{
						--pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter;
					}

					if (pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.mmc3IrqEnable == ENABLED)
					{
						if (pNES_instance->NES_state.catridgeInfo.mmc3.isRevA == YES)
						{
							// Rev A: counter must have decremented or reloaded TO zero (not already been zero)
							if ((count > RESET || reload == YES)
								&& pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter == RESET)
							{
								pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC3 = SET;
								pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = RESET;
							}
						}
						else
						{
							// Rev B: fire whenever counter is zero after update
							if (pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.currentMMC3IrqCounter == RESET)
							{
								pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC3 = SET;
								pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = RESET;
							}
						}
					}
				}
			}
			else
			{
				pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.filteredA12RiseEvent = NO;
			}
			pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterMMC3A12 = RESET;
		}
	}
	else // A12 low
	{
		pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.unfilteredA12RiseEvent = NO;
		pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.filteredA12RiseEvent = NO;
		// Note: ppuCounterMMC3A12 continues counting — it is only reset on
		// a detected rising edge (A12 going high), not on A12 going low.
		// This matches the hardware: the filter counts time since the last
		// CHR A12 rising edge, not time since A12 went low.
	}
}

void NES_t::clockRambo1CpuIRQ()
{
	const auto mapper = pNES_instance->NES_state.catridgeInfo.mapper;
	if (mapper != MAPPER::RAMBO1 && mapper != MAPPER::INES_MAPPER_158)
		RETURN;

	auto& rb = pNES_instance->NES_state.catridgeInfo.mmc3.rambo1;

	// Tick the RAMBO-1 IRQ fire delay (counts down to assert)
	if (rb.irqDelay > RESET)
	{
		--rb.irqDelay;
		if (rb.irqDelay == RESET)
		{
			pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC3 = SET;
			pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = RESET;
		}
	}

	// CPU cycle mode: prescaler fires every 4 CPU clocks
	if (rb.irqCycleMode == YES || rb.forceClock == YES)
	{
		rb.cpuClockCounter = (rb.cpuClockCounter + 1) & 0x03;
		if (rb.cpuClockCounter == RESET)
		{
			// Clock the IRQ counter
			if (rb.needReload)
			{
				rb.irqCounter = (rb.irqReloadValue <= 1)
					? (rb.irqReloadValue + 1)
					: (rb.irqReloadValue + 2);
				rb.needReload = NO;
			}
			else if (rb.irqCounter == RESET)
			{
				rb.irqCounter = rb.irqReloadValue + 1;
			}

			if (rb.irqCounter > RESET)
			{
				--rb.irqCounter;
			}

			if (rb.irqCounter == RESET && rb.irqEnabled == YES)
			{
				// CPU cycle mode: 1 M2 delay
				rb.irqDelay = 1;
			}

			rb.forceClock = NO;
		}
	}
}

void NES_t::clockNamco163IRQ()
{
	if (pNES_instance->NES_state.catridgeInfo.mapper != MAPPER::INES_MAPPER_019
		&& pNES_instance->NES_state.catridgeInfo.mapper != MAPPER::INES_MAPPER_210)
	{
		RETURN;
	}

	auto& n163 = pNES_instance->NES_state.catridgeInfo.namco163;

	// Only tick when enable bit (bit 15) is set AND not already at terminal
	if ((n163.irqCounter & 0x8000) && ((n163.irqCounter & 0x7FFF) != 0x7FFF))
	{
		n163.irqCounter++;
		if ((n163.irqCounter & 0x7FFF) == 0x7FFF)
		{
			pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_NAMECO163 = SET;
		}
	}
}

void NES_t::updateMMC5ChrA()
{
    auto& mmc5 = pNES_instance->NES_state.catridgeInfo.mmc5;
    const bool largeSprites = (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.SPRITE_SIZE == SET);
    if (!largeSprites) mmc5.lastChrReg = 0; // 8x8 sprites reset the "last CHR register" selection.
	mmc5.chrA = !largeSprites
		|| (mmc5.ppuInFrame && mmc5.splitTileNumber >= 32 && mmc5.splitTileNumber < 40)
		|| (!mmc5.ppuInFrame && mmc5.lastChrReg <= 0x5127);
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

	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC3
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_014
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_037
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_047
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_119
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_118  // TxSROM
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::RAMBO1
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_158
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_268)
	{
		// Refer https://forums.nesdev.org/viewtopic.php?p=243424#p243424 AND
		// https://forums.nesdev.org/viewtopic.php?p=243432#p243432 AND
		// https://forums.nesdev.org/viewtopic.php?p=243434#p243434
		// for reasons to ignore palette RAM Read Access from only PPU but allow from CPU
		// But below we ignore CPU as well because we handle the access via CPU in readCpuRawMemory
		if (source == MEMORY_ACCESS_SOURCE::CPU || source == MEMORY_ACCESS_SOURCE::DEBUG_PORT)
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

	if (source == MEMORY_ACCESS_SOURCE::PPU || source == MEMORY_ACCESS_SOURCE::CPU || source == MEMORY_ACCESS_SOURCE::DEBUG_PORT)
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
				auto& mmc1 = pNES_instance->NES_state.catridgeInfo.mmc1;

				// Determine effective CHR size based on whether board uses CHR-RAM or CHR-ROM
				const uint64_t totalChrBytes = pNES_instance->NES_state.catridgeInfo.hasChrRam
					? pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes
					: pNES_instance->NES_state.catridgeInfo.chrRomSizeBytes;

				uint32_t index = 0;

				// -------------------------------------------------------------
				// 8 KB CHR Mode (Control Reg Bit 4 == 0)
				// -------------------------------------------------------------
				if (mmc1.intfControlReg.fields1.c == RESET)
				{
					const uint32_t total8kBanks = static_cast<uint32_t>(totalChrBytes / 0x2000);

					if (total8kBanks > 0)
					{
						// Bit 0 of $A000 is IGNORED in 8 KB CHR mode by MMC1 hardware.
						// Shift right by 1 to get the 8 KB bank index (0, 1, 2, 3...)
						const uint32_t bank8 = ((mmc1.chrBank8 & 0x1E) >> 1) % total8kBanks;
						index = (bank8 * 0x2000) | (address & 0x1FFF);
					}
					else
					{
						index = address & 0x1FFF; // Fallback for 8 KB single CHR-RAM
					}
				}
				// -------------------------------------------------------------
				// 4 KB CHR Mode (Control Reg Bit 4 == 1)
				// -------------------------------------------------------------
				else
				{
					const uint32_t total4kBanks = static_cast<uint32_t>(totalChrBytes / 0x1000);
					const uint32_t patternTable = (address >> 12) & 1; // 0 = $0000-$0FFF, 1 = $1000-$1FFF
					const uint32_t rawBank = (patternTable == 0) ? mmc1.chrBank4Lo : mmc1.chrBank4Hi;

					if (total4kBanks > 0)
					{
						const uint32_t bank4 = (rawBank & 0x1F) % total4kBanks;
						index = (bank4 * 0x1000) | (address & 0x0FFF);
					}
					else
					{
						index = address & 0x1FFF; // Fallback
					}
				}

				// Hardware Whitelist Aliasing: Standard 8 KB CHR-RAM chips (SNROM/SGROM)
				// only have 13 address lines (A0-A12). Alias all bank switches to $0000-$1FFF.
				if (pNES_instance->NES_state.catridgeInfo.hasChrRam && totalChrBytes <= 0x2000)
				{
					index &= 0x1FFF;
				}

				// Final safety guard against out-of-bounds buffer reads
				if (totalChrBytes > 0)
				{
					index %= totalChrBytes;
				}

				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_105:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
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
			BREAK;
		}
		case MAPPER::UxROM_002:
		case MAPPER::INES_MAPPER_180:
		case MAPPER::AxROM:
		case MAPPER::INES_MAPPER_232:
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
		case MAPPER::INES_MAPPER_218:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				BYTE* nt = nullptr;

				switch (pNES_instance->NES_state.catridgeInfo.nameTblMir)
				{
				case NAMETABLE_MIRROR::VERTICAL_MIRROR:
					nt = (address & 0x0400)
						? pNES_ppuMemory->NESMemoryMap.nameTable1
						: pNES_ppuMemory->NESMemoryMap.nameTable0;
					BREAK;
				case NAMETABLE_MIRROR::HORIZONTAL_MIRROR:
					nt = (address & 0x0800)
						? pNES_ppuMemory->NESMemoryMap.nameTable1
						: pNES_ppuMemory->NESMemoryMap.nameTable0;
					BREAK;
				case NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR:
					// $A8: CIRAM A10 <- PPU A12 -> pattern table0 = BLK0, pattern table1 = BLK1
					nt = (address & 0x1000)
						? pNES_ppuMemory->NESMemoryMap.nameTable1
						: pNES_ppuMemory->NESMemoryMap.nameTable0;
					BREAK;
				case NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR:
					// $A9: CIRAM A10 <- PPU A13 -> always BLK0 (A13 is always 0 within $0000-$1FFF)
					nt = pNES_ppuMemory->NESMemoryMap.nameTable0;
					BREAK;
				}

				if (nt == nullptr)
				{
					FATAL("Unknown Nametable Mirror Type %d", pNES_instance->NES_state.catridgeInfo.nameTblMir);
					RETURN ZERO;
				}
				else
				{
					RETURN nt[address & 0x03FF];
				}
			}
			BREAK;
		}
		case MAPPER::CNROM:
		case MAPPER::J87:
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
					const uint32_t index = (pNES_instance->NES_state.catridgeInfo.cnrom.chrBank8 << 13) | (address & 0x1FFF);
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
				}
			}
			BREAK;
		}
		case MAPPER::MMC3:
		case MAPPER::INES_MAPPER_037:
		case MAPPER::INES_MAPPER_047:
		case MAPPER::INES_MAPPER_118:
		case MAPPER::INES_MAPPER_119:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint64_t totalChrRam = pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes;
				const uint64_t chrRomBytes = pNES_instance->NES_state.catridgeInfo.chrRomSizeBytes;

				// No CHR ROM and no CHR RAM -> Return Open Bus
				if (chrRomBytes == ZERO && totalChrRam == ZERO)
				{
					RETURN pNES_cpuRegisters->openbus;
				}

				// NOTE: Mapper 119 (TQROM) — bit 6 of CHR bank is chip select: 0=ROM, 1=RAM.
				// TQROM CS=1 reads from patternTable.raw (8KB RAM on board).
				// Refer https://www.nesdev.org/wiki/INES_Mapper_119
				//
				// For large pure CHR-RAM (>8KB, no CHR ROM): CS is forced to 1 and
				// maxCatridgeCHRROM is reused as the backing store (safe since CHR ROM size is zero).
				const bool isTQROM = (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_119);
				const bool isPureChrRam = (!isTQROM && chrRomBytes == ZERO && totalChrRam > ZERO);

				// NOTE: Mapper 037 CHR base — Q bit selects 128KB CHR window.
				// 0 for plain MMC3/119, so safe as a shared declaration.
				// Refer https://www.nesdev.org/wiki/INES_Mapper_037
				const uint32_t chrBase =
					(pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_037)
					? (((pNES_instance->NES_state.catridgeInfo.mmc3.ines037.outerBank >> 2) & 0x01) ? 0x20000u : 0x00000u)
					: (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_047)
					? (uint32_t)(pNES_instance->NES_state.catridgeInfo.mmc3.ines047.multicart & 0x01) << 17
					: 0x00000u;

				const uint32_t chrRomMask = (chrRomBytes > ZERO) ? (uint32_t)(chrRomBytes - ONE) : ZERO;

				// RAM address mask for large CHR-RAM: mask to full declared size so upper banks are reachable.
				// Not used for TQROM (always 0x1FFF there).
				const uint32_t largeChrRamMask = (totalChrRam > ZERO) ? (uint32_t)(totalChrRam - ONE) : 0x1FFFu;

				BIT currentBankCS = RESET;

				BIT chrA12Inversion = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.chrA12Inversion;

				uint32_t startAddr1 = PATTERN_TABLE0_START_ADDRESS;
				uint32_t endAddr1 = startAddr1 + 0x07FF; // 2KB
				uint32_t startAddr2 = endAddr1 + ONE;
				uint32_t endAddr2 = startAddr2 + 0x07FF; // 2KB
				uint32_t startAddr3 = endAddr2 + ONE;
				uint32_t endAddr3 = startAddr3 + 0x03FF; // 1KB
				uint32_t startAddr4 = endAddr3 + ONE;
				uint32_t endAddr4 = startAddr4 + 0x03FF; // 1KB
				uint32_t startAddr5 = endAddr4 + ONE;
				uint32_t endAddr5 = startAddr5 + 0x03FF; // 1KB
				uint32_t startAddr6 = endAddr5 + ONE;
				uint32_t endAddr6 = startAddr6 + 0x03FF; // 1KB

				if (chrA12Inversion == SET)
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
					// Only read CS bits from ines119 registers for actual TQROM;
					// for large CHR-RAM, CS is implicitly 1 (all banks are RAM).
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2aCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						wrapAround = 0x03FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1aCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr1) & wrapAround);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF];
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask];
					}
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[(chrBase + index) & chrRomMask];
				}
				if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
				{
					auto wrapAround = 0x07FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2bCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						wrapAround = 0x03FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1bCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr2) & wrapAround);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF];
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask];
					}
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[(chrBase + index) & chrRomMask];
				}
				if (IF_ADDRESS_WITHIN(address, startAddr3, endAddr3))
				{
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1aCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1cCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr3) & 0x3FF);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF];
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask];
					}
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[(chrBase + index) & chrRomMask];
				}
				if (IF_ADDRESS_WITHIN(address, startAddr4, endAddr4))
				{
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1bCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1dCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr4) & 0x3FF);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF];
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask];
					}
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[(chrBase + index) & chrRomMask];
				}
				if (IF_ADDRESS_WITHIN(address, startAddr5, endAddr5))
				{
					auto wrapAround = 0x03FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1cCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						wrapAround = 0x07FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2aCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr5) & wrapAround);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF];
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask];
					}
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[(chrBase + index) & chrRomMask];
				}
				if (IF_ADDRESS_WITHIN(address, startAddr6, endAddr6))
				{
					auto wrapAround = 0x03FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1dCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						wrapAround = 0x07FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2bCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr6) & wrapAround);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF];
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask];
					}
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[(chrBase + index) & chrRomMask];
				}
				FATAL("Invalid CHR ROM/RAM address in MMC3");
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_268:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				uint64_t chrRamSizeBytes = ZERO, chrNvRamSizeBytes = ZERO;
				if (isNES2)
				{
					BYTE chrRamShift = hdr.flags_8to15.nes2p0.flag11.fields.chrVolRam;
					BYTE chrNvRamShift = hdr.flags_8to15.nes2p0.flag11.fields.chrNonVolRam;
					chrRamSizeBytes = (chrRamShift == ZERO) ? ZERO : (64ULL << chrRamShift);
					chrNvRamSizeBytes = (chrNvRamShift == ZERO) ? ZERO : (64ULL << chrNvRamShift);
				}
				const uint64_t totalChrRam = chrRamSizeBytes + chrNvRamSizeBytes;
				const bool hasChrRom = (hdr.sizeOfChrRomIn8KB != ZERO);
				const bool isLargeChrRam = (!hasChrRom && totalChrRam > 0x2000);
				const bool isSmallChrRam = (!hasChrRom && totalChrRam <= 0x2000);

				if (isSmallChrRam)
				{
					RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[address - PATTERN_TABLE0_START_ADDRESS];
				}

				const BIT chrA12Inversion = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.chrA12Inversion;

				uint32_t startAddr1 = PATTERN_TABLE0_START_ADDRESS, endAddr1 = startAddr1 + 0x07FF;
				uint32_t startAddr2 = endAddr1 + ONE, endAddr2 = startAddr2 + 0x07FF;
				uint32_t startAddr3 = endAddr2 + ONE, endAddr3 = startAddr3 + 0x03FF;
				uint32_t startAddr4 = endAddr3 + ONE, endAddr4 = startAddr4 + 0x03FF;
				uint32_t startAddr5 = endAddr4 + ONE, endAddr5 = startAddr5 + 0x03FF;
				uint32_t startAddr6 = endAddr5 + ONE, endAddr6 = startAddr6 + 0x03FF;

				if (chrA12Inversion == SET)
				{
					endAddr1 = startAddr1 + 0x03FF;
					startAddr2 = endAddr1 + ONE; endAddr2 = startAddr2 + 0x03FF;
					startAddr3 = endAddr2 + ONE; endAddr3 = startAddr3 + 0x03FF;
					startAddr4 = endAddr3 + ONE; endAddr4 = startAddr4 + 0x03FF;
					startAddr5 = endAddr4 + ONE; endAddr5 = startAddr5 + 0x07FF;
					startAddr6 = endAddr5 + ONE; endAddr6 = startAddr6 + 0x07FF;
				}

				BYTE nativeV1k = ZERO;
				uint32_t pageBase = ZERO;
				const auto& reg = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters;

				if (IF_ADDRESS_WITHIN(address, startAddr1, endAddr1))
				{
					pageBase = startAddr1;
					nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1a
						: (BYTE)(reg.chrBank2a + (((address - startAddr1) >> 10) & 1));
				}
				else if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
				{
					pageBase = startAddr2;
					nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1b
						: (BYTE)(reg.chrBank2b + (((address - startAddr2) >> 10) & 1));
				}
				else if (IF_ADDRESS_WITHIN(address, startAddr3, endAddr3))
				{
					pageBase = startAddr3;
					nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1c : reg.chrBank1a;
				}
				else if (IF_ADDRESS_WITHIN(address, startAddr4, endAddr4))
				{
					pageBase = startAddr4;
					nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1d : reg.chrBank1b;
				}
				else if (IF_ADDRESS_WITHIN(address, startAddr5, endAddr5))
				{
					pageBase = startAddr5;
					nativeV1k = (chrA12Inversion == SET)
						? (BYTE)(reg.chrBank2a + (((address - startAddr5) >> 10) & 1))
						: reg.chrBank1c;
				}
				else
				{
					pageBase = startAddr6;
					nativeV1k = (chrA12Inversion == SET)
						? (BYTE)(reg.chrBank2b + (((address - startAddr6) >> 10) & 1))
						: reg.chrBank1d;
				}

				const auto& outer = pNES_instance->NES_state.catridgeInfo.mmc3.ines268;
				uint32_t page1k = mapper268ComputeChrPage(nativeV1k, (uint16_t)address, outer.reg);
				uint32_t index = (page1k * 0x0400u) + ((address - pageBase) & 0x03FFu);

				if (isLargeChrRam)
				{
					const uint32_t largeChrRamMask = (uint32_t)(totalChrRam - ONE);
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask];
				}
				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index % ((uint32_t)hdr.sizeOfChrRomIn8KB * 0x2000u)];
			}
			BREAK;
		}
		case MAPPER::RAMBO1:
		case MAPPER::INES_MAPPER_158:
		{
			if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
			{
				// Pure CHR-RAM
				if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
				{
					RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[address - PATTERN_TABLE0_START_ADDRESS];
				}
			}

			if (!IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
				BREAK;

			{
				const auto& rb = pNES_instance->NES_state.catridgeInfo.mmc3.rambo1;
				const BYTE curReg = rb.currentRegister;
				const BIT chrA12Inv = (curReg >> 7) & 1;   // bit 7
				const BIT kMode = (curReg >> 5) & 1;   // bit 5: full 1KB mode

				// Get total CHR 1KB bank count for wrapping
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8kBanks = isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
					: hdr.sizeOfChrRomIn8KB;
				const uint32_t totalChr1kBanks = totalChr8kBanks * 8;

				// RAMBO-1 CHR layout (A12=0 i.e. chrA12Inv==0):
				//   $0000-$07FF : R0 (2KB, K=0) or R0/R8 (1KB each, K=1)
				//   $0800-$0FFF : R1 (2KB, K=0) or R1/R9 (1KB each, K=1)
				//   $1000-$13FF : R2 (1KB)
				//   $1400-$17FF : R3 (1KB)
				//   $1800-$1BFF : R4 (1KB)
				//   $1C00-$1FFF : R5 (1KB)
				// When chrA12Inv==1, the two halves swap.
				//
				// Helper: resolve a 1KB CHR bank index and local offset to a ROM byte.
				auto readChr1k = [&](BYTE bankReg, uint32_t localOffset) -> BYTE
					{
						uint32_t bankIdx = rb.reg[bankReg] % (totalChr1kBanks ? totalChr1kBanks : 1);
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[bankIdx * 0x400 + (localOffset & 0x3FF)];
					};

				// Determine which half of CHR address space we are in
				const bool inUpperHalf = (address >= 0x1000); // PPU $1000-$1FFF
				// After A12 inversion, does this address land in the 2KB/1KB zone or the 1KB-only zone?
				// The "2KB/1KB zone" is whichever half does NOT contain the fixed-1KB banks.
				// Without inversion: lower half ($0000) = 2KB/K zone, upper half ($1000) = 1KB zone.
				// With inversion:    upper half ($1000) = 2KB/K zone, lower half ($0000) = 1KB zone.
				const bool inKZone = (chrA12Inv ? !inUpperHalf : inUpperHalf);

				// Relative offset within the 8KB pattern table window
				const uint32_t relAddr = address - PATTERN_TABLE0_START_ADDRESS;

				if (!inKZone)
				{
					// This is the "2KB or 1KB" half (registers R0, R1, R8, R9)
					// $0000-$03FF or $0800-$0BFF after inversion mapping
					const uint32_t halfBase = chrA12Inv ? 0x1000u : 0x0000u;
					const uint32_t offsetInHalf = address - halfBase; // 0-$0FFF

					if (kMode == SET)
					{
						// K=1: four 1KB banks (R0, R8, R1, R9) fill the 4KB
						const uint32_t slot = offsetInHalf >> 10;  // 0-3
						const uint32_t lo = offsetInHalf & 0x3FF;
						static const BYTE kRegs[4] = { 0, 8, 1, 9 };
						RETURN readChr1k(kRegs[slot], lo);
					}
					else
					{
						// K=0: two 2KB banks (R0, R1)
						// Bit 0 of register value is ignored; PPU A10 passes through.
						const bool isSecond2k = (offsetInHalf >= 0x800);
						const BYTE baseReg = isSecond2k ? 1 : 0;
						const uint32_t bankBase = (rb.reg[baseReg] & 0xFE) * 0x400;
						const uint32_t lo = offsetInHalf & 0x7FF;
						RETURN pNES_catridgeMemory->maxCatridgeCHRROM[
							(bankBase + lo) % (totalChr8kBanks * 0x2000 ? totalChr8kBanks * 0x2000 : 1)];
					}
				}
				else
				{
					// This is the fixed four 1KB half (R2, R3, R4, R5)
					const uint32_t fourKBase = chrA12Inv ? 0x0000u : 0x1000u;
					const uint32_t offsetIn4k = address - fourKBase; // 0-$0FFF
					const uint32_t slot = offsetIn4k >> 10;           // 0-3
					const uint32_t lo = offsetIn4k & 0x3FF;
					static const BYTE fRegs[4] = { 2, 3, 4, 5 };
					RETURN readChr1k(fRegs[slot], lo);
				}
			}
			BREAK;
		}
		case MAPPER::MMC5:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				auto& mmc5 = pNES_instance->NES_state.catridgeInfo.mmc5;

				const FLAG isActualPpuFetch = (source == MEMORY_ACCESS_SOURCE::PPU);
				const uint64_t ppuCycle = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;
				const FLAG isSpriteFetchWindow = isActualPpuFetch && ppuCycle >= TWOFIFTYSEVEN && ppuCycle <= THREETWENTY;
				// MMC5 monitors all PPU VRAM reads to determine whether the PPU
				// is still rendering. CHR/pattern-table reads must refresh the
				// 3-CPU-cycle in-frame watchdog just like nametable/attribute reads.
				if (isActualPpuFetch)
				{
					mmc5.ppuIdleCounter = 3;
					mmc5.lastPpuReadAddr = address;
				}

				// --- Vertical split CHR override ---
				if (isActualPpuFetch
					&& !isSpriteFetchWindow
					&& mmc5.verticalSplitEnabled == YES
					&& mmc5.ppuInFrame == YES
					&& mmc5.splitInSplitRegion == YES
					&& mmc5.extendedRamMode <= 1)
				{
					const uint32_t chrAddr = ((uint32_t)mmc5.verticalSplitBank << 12)
						| ((uint32_t)address & 0x0FF8u)
						| ((uint32_t)mmc5.verticalSplitScanlineCounter & 0x07u);
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
					const uint32_t totalChr8k = isNES2
						? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
						: hdr.sizeOfChrRomIn8KB;
					const uint32_t chrRomBytes = (totalChr8k == 0) ? 0x2000u : (totalChr8k * 0x2000u);
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[chrAddr % chrRomBytes];
				}

				// --- Extended attribute mode (extendedRamMode == 1) ---
				// CHR tile fetches for BG tiles are overridden by ExRAM bank data.
				// CHR low and high byte fetches for extended attribute mode
				// CHR substitution applies to background pattern fetches only.
				if (isActualPpuFetch && !isSpriteFetchWindow
					&& mmc5.extendedRamMode == 1 && mmc5.ppuInFrame == YES
					&& (mmc5.splitTileNumber < 32 || mmc5.splitTileNumber >= 40)
					&& mmc5.exAttrFetchCounter >= 1 && mmc5.exAttrFetchCounter <= 2)
				{
					// Decrement first, then return — counter goes 2->1 (CHR low)
					// then 1->0 (CHR high).
					mmc5.exAttrFetchCounter--;
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[(uint32_t)mmc5.exAttrSelectedChrBank * 0x1000+ (address & 0x0FFF)];
				}

				// --- Determine chrA (which set of CHR registers to use) ---
				// MMC5 CHR set selection.
				//
				// 8x8 sprites:
				//     Set A is always active.
				//
				// 8x16 sprites while rendering:
				//     Background fetches use Set B.
				//     Sprite fetches use Set A.
				//     MMC5 identifies the sprite-fetch window through its
				//     internal tile counter (32..39).
				//
				// Outside rendering:
				//     PPUDATA ($2007) uses whichever CHR register set
				//     was written last.
				const FLAG largeSprites = (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUCTRL.ppuctrl.SPRITE_SIZE == SET);

				if (!largeSprites)
				{
					mmc5.lastChrReg = 0;
				}
				
				const FLAG chrA = !largeSprites 
					|| (mmc5.ppuInFrame && mmc5.splitTileNumber >= 32 && mmc5.splitTileNumber < 40)
					|| (!mmc5.ppuInFrame && mmc5.lastChrReg <= 0x5127);

				uint32_t index = 0;

				switch (mmc5.chrMode)
				{
				case 0: // 8KB: single bank from last reg of each set
				{
					// Set A -> chrBanks[7] ($5127), Set B -> chrBanks[11] ($512B)
					index = (uint32_t)mmc5.chrBanks[chrA ? 7 : 11] * 0x2000 + (address & 0x1FFF);
					BREAK;
				}
				case 1: // 4KB: two halves
				{
					if (address < 0x1000)
					{
						// $0000-$0FFF: Set A -> chrBanks[3] ($5123), Set B -> chrBanks[11] ($512B)
						index = (uint32_t)mmc5.chrBanks[chrA ? 3 : 11] * 0x1000 + (address & 0x0FFF);
					}
					else
					{
						// $1000-$1FFF: Set A -> chrBanks[7] ($5127), Set B -> chrBanks[11] ($512B)
						index = (uint32_t)mmc5.chrBanks[chrA ? 7 : 11] * 0x1000 + (address & 0x0FFF);
					}
					BREAK;
				}
				case 2: // 2KB: four quarters
				{
					if (address < 0x0800)
					{
						// $0000-$07FF: Set A -> chrBanks[1] ($5121), Set B -> chrBanks[9] ($5129)
						index = (uint32_t)mmc5.chrBanks[chrA ? 1 : 9] * 0x0800 + (address & 0x07FF);
					}
					else if (address < 0x1000)
					{
						// $0800-$0FFF: Set A -> chrBanks[3] ($5123), Set B -> chrBanks[11] ($512B)
						index = (uint32_t)mmc5.chrBanks[chrA ? 3 : 11] * 0x0800 + (address & 0x07FF);
					}
					else if (address < 0x1800)
					{
						// $1000-$17FF: Set A -> chrBanks[5] ($5125), Set B -> chrBanks[9] ($5129)
						index = (uint32_t)mmc5.chrBanks[chrA ? 5 : 9] * 0x0800 + (address & 0x07FF);
					}
					else
					{
						// $1800-$1FFF: Set A -> chrBanks[7] ($5127), Set B -> chrBanks[11] ($512B)
						index = (uint32_t)mmc5.chrBanks[chrA ? 7 : 11] * 0x0800 + (address & 0x07FF);
					}
					BREAK;
				}
				case 3: // 1KB: eight 1KB slots
				{
					// Set A: slot maps directly to chrBanks[0..7]
					// Set B: only 4 registers ($5128-$512B = indices 8-11); slots wrap mod 4
					const uint32_t slot = (address >> 10) & 0x07;
					const uint32_t bankIdx = chrA ? slot : (8 + (slot & 0x03));
					index = (uint32_t)mmc5.chrBanks[bankIdx] * 0x0400 + (address & 0x03FF);
					BREAK;
				}
				default:
				{
					FATAL("Invalid MMC5 CHR mode");
					BREAK;
				}
				}

				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const FLAG isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8k = isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
					: hdr.sizeOfChrRomIn8KB;
				const uint32_t chrRomBytes = (totalChr8k == 0) ? 0x2000u : (totalChr8k * 0x2000u);
				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index % chrRomBytes];
			}
			BREAK;
		}
		case MAPPER::MMC2:
		case MAPPER::MMC4:
		{
			if (address <= PATTERN_TABLE1_END_ADDRESS)
			{
				const FLAG isMMC4 = (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC4);

				auto& chrBankFD = (isMMC4)
					?
					pNES_instance->NES_state.catridgeInfo.mmc4.chrBankFD
					:
					pNES_instance->NES_state.catridgeInfo.mmc2.chrBankFD;

				auto& chrBankFE = (isMMC4)
					?
					pNES_instance->NES_state.catridgeInfo.mmc4.chrBankFE
					:
					pNES_instance->NES_state.catridgeInfo.mmc2.chrBankFE;

				auto& chrBankLatch = (isMMC4)
					?
					pNES_instance->NES_state.catridgeInfo.mmc4.chrBankLatch
					:
					pNES_instance->NES_state.catridgeInfo.mmc2.chrBankLatch;

				const uint32_t chrRomBytes = (uint32_t)pNES_instance->NES_state.catridgeInfo.chrRomSizeBytes;
				const uint32_t chrMask = (chrRomBytes > ZERO) ? (chrRomBytes - ONE) : 0x1FFFu; // fallback or full size mask
				const uint32_t patternTable = (address >> 12) & 1;
				const uint32_t bank = (chrBankLatch[patternTable] == 0xFD) ? chrBankFD[patternTable] : chrBankFE[patternTable];
				const uint32_t index = (bank << 12) | (address & 0x0FFF); // << 12 is same as * 0x1000
				const uint32_t safeIndex = index & chrMask; // Prevents out-of-bounds crash
				const BYTE value = pNES_catridgeMemory->maxCatridgeCHRROM[safeIndex];

				// Update latch AFTER fetch
				if (
					(isMMC4 && IF_ADDRESS_WITHIN(address, 0x0FD8, 0x0FDF))
					||
					(!isMMC4 && address == 0x0FD8)
					)
				{
					chrBankLatch[0] = 0xFD;
				}
				else if (
					(isMMC4 && IF_ADDRESS_WITHIN(address, 0x0FE8, 0x0FEF))
					||
					(!isMMC4 && address == 0x0FE8)
					)
				{
					chrBankLatch[0] = 0xFE;
				}
				else if (IF_ADDRESS_WITHIN(address, 0x1FD8, 0x1FDF))
				{
					chrBankLatch[1] = 0xFD;
				}
				else if (IF_ADDRESS_WITHIN(address, 0x1FE8, 0x1FEF))
				{
					chrBankLatch[1] = 0xFE;
				}

				RETURN value;
			}
			BREAK;
		}
		case MAPPER::COLOR_DREAMS:
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
					const uint32_t index = (pNES_instance->NES_state.catridgeInfo.colorDreams.chrBank8 << 13) | (address & 0x1FFF); // << 13 is same as * 0x2000
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
				}
			}
			BREAK;
		}
		case MAPPER::CPROM:
		{
			/*
			 * ============================================================================
			 * NES-CPROM (Mapper 13) Memory Mapping Table
			 * ============================================================================
			 * The physical CPROM board has 16 KiB of CHR-RAM, split into four 4 KiB chunks
			 * (Chunks 0, 1, 2, 3).
			 *
			 * Hardware Layout:
			 * - PPU $0000-$0FFF (Table 0) is permanently hardwired to Chunk 0.
			 * - PPU $1000-$1FFF (Table 1) selects a chunk based on the 'chrBank' register.
			 *   Due to board routing logic (74HC08), Bank 3 wraps around to Chunk 0.
			 *
			 * Emulation Layout (Using: index = (bank << 12) | (address & 0x0FFF)):
			 * Because your array is oversized, our indexing maps each bank linearly.
			 *
			 * +----------+---------------+-------------------+-----------------------+
			 * | chrBank  | PPU Table 0   | PPU Table 1       | Net Hardware Behavior |
			 * | Register | ($0000-$0FFF) | ($1000-$1FFF)     |                       |
			 * +----------+---------------+-------------------+-----------------------+
			 * |    0     | Array[0x0000] | Array[0x0000]*    | Table 0 = Chunk 0     |
			 * |          | (Chunk 0)     | (Chunk 0)*        | Table 1 = Chunk 1     |
			 * +----------+---------------+-------------------+-----------------------+
			 * |    1     | Array[0x0000] | Array[0x1000]     | Table 0 = Chunk 0     |
			 * |          | (Chunk 0)     | (Chunk 1)         | Table 1 = Chunk 2     |
			 * +----------+---------------+-------------------+-----------------------+
			 * |    2     | Array[0x0000] | Array[0x2000]     | Table 0 = Chunk 0     |
			 * |          | (Chunk 0)     | (Chunk 2)         | Table 1 = Chunk 3     |
			 * +----------+---------------+-------------------+-----------------------+
			 * |    3     | Array[0x0000] | Array[0x3000]**   | Both Tables point     |
			 * |          | (Chunk 0)     | (Chunk 0)**       | to Chunk 0 (Mirror)   |
			 * +----------+---------------+-------------------+-----------------------+
			 *
			 * *  CRITICAL BUG FIX NOTE:
			 *    When chrBank = 0, our shifted formula maps Table 1 to Array[0x0000].
			 *    To prevent Table 1 from overwriting Table 0's distinct UI tiles,
			 *    both PPU Read and PPU Write MUST use the exact same indexing math
			 *    so they safely share the data without dropping tiles or cross-corrupting.
			 *
			 * ** Hardware Mirroring Note:
			 *    On real hardware, selecting Bank 3 loops the internal memory lines back
			 *    to Chunk 0. In our emulator, it utilizes Array[0x3000]. Videomation
			 *    uses Bank 3 as a safe blank workspace, meaning this array separation
			 *    works perfectly.
			 * ============================================================================
			 */
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				uint32_t index = 0;

				if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
				{
					// Pattern Table 0 ($0000-$0FFF) always maps to the first 4KB of your array
					index = address & 0x0FFF;
				}
				else // PATTERN_TABLE1 ($1000-$1FFF)
				{
					// Pattern Table 1 maps to bank 0, 1, 2, or 3 based on the register
					BYTE bank = pNES_instance->NES_state.catridgeInfo.cprom.chrBank;
					index = (bank << 12) | (address & 0x0FFF);
				}

				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_014:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				auto& reg014 = pNES_instance->NES_state.catridgeInfo.ines014;
				const bool isMMC3Mode = (reg014.supervisorReg & 0x10) != ZERO;
				const uint32_t chrA18Offset = mapper014ChrA18Offset(reg014.supervisorReg, (uint16_t)address);

				uint16_t nativeV1k = ZERO;

				if (isMMC3Mode)
				{
					// Identical 6-region native-bank decode to your shared MMC3 CHR
					// case (chrA12Inversion-dependent), yielding nativeV1k from
					// chrBank2a/2b/1a-1d -- paste that logic here.
					const BIT chrA12Inversion = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.chrA12Inversion;

					uint32_t startAddr1 = PATTERN_TABLE0_START_ADDRESS, endAddr1 = startAddr1 + 0x07FF;
					uint32_t startAddr2 = endAddr1 + ONE, endAddr2 = startAddr2 + 0x07FF;
					uint32_t startAddr3 = endAddr2 + ONE, endAddr3 = startAddr3 + 0x03FF;
					uint32_t startAddr4 = endAddr3 + ONE, endAddr4 = startAddr4 + 0x03FF;
					uint32_t startAddr5 = endAddr4 + ONE, endAddr5 = startAddr5 + 0x03FF;
					uint32_t startAddr6 = endAddr5 + ONE, endAddr6 = startAddr6 + 0x03FF;

					if (chrA12Inversion == SET)
					{
						endAddr1 = startAddr1 + 0x03FF;
						startAddr2 = endAddr1 + ONE; endAddr2 = startAddr2 + 0x03FF;
						startAddr3 = endAddr2 + ONE; endAddr3 = startAddr3 + 0x03FF;
						startAddr4 = endAddr3 + ONE; endAddr4 = startAddr4 + 0x03FF;
						startAddr5 = endAddr4 + ONE; endAddr5 = startAddr5 + 0x07FF;
						startAddr6 = endAddr5 + ONE; endAddr6 = startAddr6 + 0x07FF;
					}

					const auto& reg = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters;

					if (IF_ADDRESS_WITHIN(address, startAddr1, endAddr1))
						nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1a : (BYTE)(reg.chrBank2a + (((address - startAddr1) >> 10) & 1));
					else if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
						nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1b : (BYTE)(reg.chrBank2b + (((address - startAddr2) >> 10) & 1));
					else if (IF_ADDRESS_WITHIN(address, startAddr3, endAddr3))
						nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1c : reg.chrBank1a;
					else if (IF_ADDRESS_WITHIN(address, startAddr4, endAddr4))
						nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1d : reg.chrBank1b;
					else if (IF_ADDRESS_WITHIN(address, startAddr5, endAddr5))
						nativeV1k = (chrA12Inversion == SET) ? (BYTE)(reg.chrBank2a + (((address - startAddr5) >> 10) & 1)) : reg.chrBank1c;
					else
						nativeV1k = (chrA12Inversion == SET) ? (BYTE)(reg.chrBank2b + (((address - startAddr6) >> 10) & 1)) : reg.chrBank1d;
				}
				else
				{
					const uint32_t bankIndex = (address >> 10) & 0x07;
					nativeV1k = pNES_instance->NES_state.catridgeInfo.vrc24.chrBank[bankIndex];
				}

				uint32_t index = chrA18Offset + ((uint32_t)nativeV1k * 0x0400u) + (address & 0x03FFu);

				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8kBanks = isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << EIGHT))
					: hdr.sizeOfChrRomIn8KB;
				const uint64_t totalChrBytes = (uint64_t)totalChr8kBanks * 0x2000ULL;
				if (totalChrBytes > ZERO)
				{
					index = (uint32_t)(index % totalChrBytes);
				}

				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_015:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[address - PATTERN_TABLE0_START_ADDRESS];
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_016:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS)) // $0000-$1FFF
			{
				// CHR-RAM Case (Unbacked by CHR-ROM)
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

				// CHR-ROM Bank Swapped Case (8 x 1 KiB Pages)
				uint32_t subWindow = (address - PATTERN_TABLE0_START_ADDRESS) / 0x0400;
				uint32_t windowOffset = (address - PATTERN_TABLE0_START_ADDRESS) & 0x03FF;

				uint32_t targetBank = pNES_instance->NES_state.catridgeInfo.ines016.chrBank[subWindow];
				uint32_t index = (targetBank * 0x0400) + windowOffset;

				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}
			BREAK; // Let Nametables ($2000-$2FFF) pass straight through to generic VRAM mirrors
		}
		case MAPPER::INES_MAPPER_018:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				auto& j18 = pNES_instance->NES_state.catridgeInfo.jaleco18;
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8kBanks =
					isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << EIGHT))
					: hdr.sizeOfChrRomIn8KB;
				const uint32_t totalChr1kBanks = (totalChr8kBanks == 0) ? 8 : (totalChr8kBanks << THREE);

				// 1KB window index 0..7 directly from address bits [12:10]
				const uint32_t bankIndex = (address >> 10) & 0x07;
				const uint32_t bank = j18.chrBank[bankIndex] % totalChr1kBanks;
				const uint32_t index = (bank * 0x0400) + (address & 0x03FF);

				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}
			BREAK;
		}
		case MAPPER::VRC2_022:
		case MAPPER::VRC4_021:
		case MAPPER::VRC2_VRC4_023:
		case MAPPER::VRC2_VRC4_025:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8kBanks =
					isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << EIGHT))
					: hdr.sizeOfChrRomIn8KB;
				const uint32_t totalChr1kBanks = totalChr8kBanks << THREE;

				// A 1KB window corresponds to shifting the address right by 10 bits (0x0400 = 1 << 10)
				// This gives an array index 0-7 directly from the address range 0x0000 - 0x1FFF
				uint32_t bank_index = (address >> 10) & 0x07;
				uint32_t bank = pNES_instance->NES_state.catridgeInfo.vrc24.chrBank[bank_index];

				// Refer to https://www.nesdev.org/wiki/VRC2_and_VRC4#CHR_Select_0_low($B000),_high($B001)
				// In case of VRC2a, low bit is ignored (value right shifted by one)
				if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::VRC2_022)
				{
					bank >>= ONE;
				}

				bank %= totalChr1kBanks;

				// Combine bank layout with the 1KB offset within the page
				uint32_t index = (bank * 0x0400) + (address & 0x03FF);

				// Note: Since the CHR RAM and CHR ROM code blocks were identical, 
				// this single return statement safely handles both conditions.
				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}

			BREAK;
		}
		case MAPPER::VRC6_024:
		case MAPPER::VRC6_026:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				auto& vrc6 = pNES_instance->NES_state.catridgeInfo.vrc6;
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8kBanks =
					isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << EIGHT))
					: hdr.sizeOfChrRomIn8KB;

				const uint32_t totalChr1kBanks = (totalChr8kBanks == 0) ? 8 : (totalChr8kBanks << THREE);

				uint32_t bank_index = (address >> 10) & 0x07;
				uint32_t bank = vrc6.chrBank[bank_index] % totalChr1kBanks;

				uint32_t index = (bank * 0x0400) + (address & 0x03FF);
				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}

			BREAK;
		}
		case MAPPER::INES_MAPPER_028:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint32_t chrSizeBytes = static_cast<uint32_t>(pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes);
				// Guard against zero size to prevent division by zero
				if (chrSizeBytes == 0) MASQ_UNLIKELY
				{
					RETURN pNES_cpuRegisters->openbus;
				}
				const uint32_t bank8 = pNES_instance->NES_state.catridgeInfo.ines028.reg00_chrBank & 0x03;
				const uint32_t rawIndex = (bank8 * 0x2000u) + (address & 0x1FFFu);
				const uint32_t index = rawIndex % chrSizeBytes;
				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_029:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint32_t bank8 = pNES_instance->NES_state.catridgeInfo.ines029.chrBank8;
				const uint32_t index = (bank8 * 0x2000u) + (address & 0x1FFFu);
				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index]; // CHR-RAM backing, same convention as your other large-CHR mappers
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_030:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint32_t bank8 = pNES_instance->NES_state.catridgeInfo.ines030.chrBank8;
				const uint32_t index = (bank8 * 0x2000u) + (address & 0x1FFFu);
				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_034:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint32_t patternTable = (address >> 12) & 1;
				const uint32_t offset = address & 0x0FFF;

				if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO
					|| pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::BNROM) // BNROM doesnt have any banking, just blindly reads CHR ROM/RAM
				{
					if (patternTable == 0)
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0[offset];
					}
					else
					{
						RETURN pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1[offset];
					}
				}
				else
				{
					const uint32_t bank = (patternTable == 0) ? pNES_instance->NES_state.catridgeInfo.ines034.chrBank4Lo : pNES_instance->NES_state.catridgeInfo.ines034.chrBank4Hi;
					const uint32_t index = (bank << 12) | offset; // << 12 is same as * 0x1000
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
				}
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
					const uint32_t index = (pNES_instance->NES_state.catridgeInfo.gxrom.chrBank << 13) | (address & 0x1FFF);
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
				}
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_067:
		case MAPPER::INES_MAPPER_068:
		{
			// --- PATTERN TABLES ($0000 - $1FFF) ---
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

				const uint32_t totalChr8kBanks = isNES2
					? (hdr.sizeOfChrRomIn8KB | ((hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB & 0x0F) << EIGHT))
					: hdr.sizeOfChrRomIn8KB;

				// 1 8KB bank = 4 2KB banks
				const uint32_t totalChr2kBanks = totalChr8kBanks << TWO;

				// Determine which 2KB window is being accessed (0, 1, 2, or 3)
				const uint32_t window = (address & 0x1F00) >> 11;
				const uint32_t offset = address & 0x07FF;

				// Fetch the correct register bank based on the window
				uint32_t selectedBank = 0;
				auto& regs = pNES_instance->NES_state.catridgeInfo.ines_067_068;
				switch (window)
				{
				case 0: selectedBank = regs.chrBank0; BREAK;
				case 1: selectedBank = regs.chrBank1; BREAK;
				case 2: selectedBank = regs.chrBank2; BREAK;
				case 3: selectedBank = regs.chrBank3; BREAK;
				}

				// Calculate final index 
				uint32_t index = (selectedBank * 0x0800) + offset;

				// Wrap around correctly based on total size of CHR-ROM in bytes
				if (totalChr2kBanks > 0)
				{
					index %= (totalChr2kBanks * 0x0800);
				}

				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_069:
		{
			// --- PATTERN TABLES ($0000 - $1FFF) ---
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;

				// NES 2.0 identification: bits 2-3 of flag 7 must equal 2
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

				// Calculate total 8KB banks, accounting for NES 2.0 MSB nibble if applicable
				const uint32_t totalChr8kBanks = isNES2
					? (hdr.sizeOfChrRomIn8KB | ((hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB & 0x0F) << EIGHT))
					: hdr.sizeOfChrRomIn8KB;

				const uint32_t totalChr1kBanks = totalChr8kBanks << THREE;

				// Calculate the 1KB bank index (0 to 7) based on the current PPU address
				// Mapper 69 allows independent mapping of eight 1KB banks
				const uint32_t bank = address >> 10;
				const uint32_t offset = address & 0x03FF;

				// Fetch the physical bank selected by the mapper registers, scale to bytes, and add offset
				uint32_t index = (pNES_instance->NES_state.catridgeInfo.ines069.chrBank[bank] * 0x0400) + offset;

				if (hdr.sizeOfChrRomIn8KB == ZERO)
				{
					// 8KB CHR-RAM
					index &= 0x1FFF;
					RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[index];
				}
				else
				{
					// CHR-ROM bank wrapping
					index %= (totalChr1kBanks * 0x0400);
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
				}
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_070:
		case MAPPER::INES_MAPPER_152:
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
					const uint32_t index = (pNES_instance->NES_state.catridgeInfo.ines_070_152.chrReg << 13) | (address & 0x1FFF);
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
				}
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_078:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8kBanks = isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
					: hdr.sizeOfChrRomIn8KB;

				const uint32_t bank8 = pNES_instance->NES_state.catridgeInfo.ines078.chrBank8 % totalChr8kBanks;
				const uint32_t index = (bank8 * 0x2000u) + (address & 0x1FFFu);

				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
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
		case MAPPER::INES_MAPPER_019:
		case MAPPER::INES_MAPPER_210:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				auto& n163 = pNES_instance->NES_state.catridgeInfo.namco163;

				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool     isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr1k = isNES2
					? ((hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8)) * 8)
					: (hdr.sizeOfChrRomIn8KB * 8);

				// 8 x 1KB slots covering $0000-$1FFF
				const uint8_t slot = (uint8_t)(address >> 10);  // 0..7
				const BYTE    bankVal = n163.chrBanks[slot];

				// lowChrNtMode  (bit6 of $E800): YES = slots 0-3 treat $E0-$FF as CHR-ROM, NOT CIRAM
				// highChrNtMode (bit7 of $E800): YES = slots 4-7 treat $E0-$FF as CHR-ROM, NOT CIRAM
				// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#CHR_and_NT_Select
				const bool ntModeDisabled = (slot < 4)
					? (n163.lowChrNtMode == YES)
					: (n163.highChrNtMode == YES);

				// Redirect to CIRAM when: Namco163 variant, value >= 0xE0, mode not disabled
				// bit 0 of bankVal selects CIRAM bank 0 (nameTable0) or bank 1 (nameTable1)
				if (n163.variant == 0 && bankVal >= 0xE0 && !ntModeDisabled)
				{
					const uint16_t ntOffset = address & 0x3FF;
					if ((bankVal & 0x01) == 0)
						RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset];
					else
						RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset];
				}

				// CHR-RAM game
				if (hdr.sizeOfChrRomIn8KB == ZERO)
					RETURN pNES_ppuMemory->NESMemoryMap.patternTable.raw[address & 0x1FFF];

				// CHR-ROM
				const uint32_t chrIndex = ((uint32_t)bankVal % (totalChr1k > 0 ? totalChr1k : 1)) * 0x400
					+ (address & 0x3FF);
				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[chrIndex];
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
				address -= 0x1000;
			}

			if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC5)
			{
				auto& mmc5 = pNES_instance->NES_state.catridgeInfo.mmc5;

				const bool isActualPpuFetch = (source == MEMORY_ACCESS_SOURCE::PPU);

				uint8_t  ntIndex = 0;
				uint16_t ntOffset = 0;
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0; ntOffset = (uint16_t)(address - NAME_TABLE0_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1; ntOffset = (uint16_t)(address - NAME_TABLE1_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2; ntOffset = (uint16_t)(address - NAME_TABLE2_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE3_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
				{
					ntIndex = 3; ntOffset = (uint16_t)(address - NAME_TABLE3_START_ADDRESS);
				}

				const bool isNTFetch = (ntOffset < 0x3C0);
				const bool isAttrFetch = (ntOffset >= 0x3C0);

				// STEP 1: splitTileNumber++ and ppuInFrame transition FIRST (Mesen order)
				if (isNTFetch)
				{
					mmc5.splitTileNumber++;
					if (mmc5.needInFrame == YES)
					{
						mmc5.needInFrame = NO;
						mmc5.ppuInFrame = YES;
						mmc5.verticalSplitScanlineCounter = mmc5.verticalSplitScroll;
					}
					updateMMC5ChrA();
				}

				// STEP 2: scanline detection AFTER splitTileNumber update (Mesen's DetectScanlineStart)
				if (mmc5.ntReadCounter >= 2)
				{
					mmc5.ntReadCounter = 0;
					mmc5.splitTileNumber = 0;
					if (mmc5.ppuInFrame == YES)
					{
						mmc5.verticalSplitScanlineCounter = (mmc5.verticalSplitScanlineCounter + 1) & 0x00FF;
					}
					if (!mmc5.ppuInFrame && !mmc5.needInFrame)
					{
						mmc5.needInFrame = YES;
						mmc5.scanlineCounter = 0;
					}
					else
					{
						mmc5.scanlineCounter++;
						if (mmc5.scanlineCounter == mmc5.irqCounterTarget)
						{
							mmc5.irqPending = YES;
							if (mmc5.irqEnabled == YES)
								pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC5 = SET;
						}
					}
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
				{
					if (mmc5.lastPpuReadAddr == address)
					{
						mmc5.ntReadCounter++;
						if (mmc5.ntReadCounter >= 2)
							mmc5.splitTileNumber = 0;
					}
				}

				// Reset counter on any address change (NT or non-NT)
				if (mmc5.lastPpuReadAddr != address)
					mmc5.ntReadCounter = 0;

				// STEP 3: idle counter and lastPpuReadAddr LAST
				if (isActualPpuFetch)
				{
					mmc5.ppuIdleCounter = 3;
					mmc5.lastPpuReadAddr = address;
				}

				// --- Vertical split mode ($5200) ---
				if (mmc5.verticalSplitEnabled == YES
					&& mmc5.ppuInFrame == YES
					&& mmc5.extendedRamMode <= 1)
				{
					const uint8_t  scanline = (mmc5.splitTileNumber >= 41)
						? (uint8_t)(mmc5.scanlineCounter + 1)
						: (uint8_t)mmc5.scanlineCounter;
					const uint32_t vertScrollY = ((uint32_t)scanline + mmc5.verticalSplitScroll) % 240;
					const uint8_t  column = (uint8_t)((mmc5.splitTileNumber + 2) % 42);

					if (isNTFetch)
					{
						if (column == 0)
							mmc5.splitInSplitRegion = (mmc5.verticalSplitRightSide == NO) ? YES : NO;

						if (column == mmc5.verticalSplitDelimiterTile && mmc5.splitTileNumber < 42)
							mmc5.splitInSplitRegion = (mmc5.splitInSplitRegion == YES) ? NO : YES;
						else if (column > 32)
							mmc5.splitInSplitRegion = NO;

						if (mmc5.splitInSplitRegion == YES)
						{
							mmc5.splitTile = ((vertScrollY & 0xF8) << 2) | column;
							RETURN mmc5.exRam[mmc5.splitTile & 0x3FF];
						}
					}
					else
					{
						if (mmc5.splitInSplitRegion == YES)
						{
							const uint8_t  shift = (uint8_t)(((mmc5.splitTile >> 4) & 0x04) | (mmc5.splitTile & 0x02));
							const uint16_t atAddr = (uint16_t)(0x3C0 | ((mmc5.splitTile & 0x380) >> 4) | ((mmc5.splitTile & 0x01F) >> 2));
							const uint8_t  palette = (mmc5.exRam[atAddr & 0x3FF] >> shift) & 0x03;
							RETURN(uint8_t)(palette * 0x55);
						}
					}
				}

				// --- Extended attribute mode ($5104 == 1) ---
				if (mmc5.extendedRamMode == 1 && mmc5.ppuInFrame == YES
					&& (mmc5.splitTileNumber < 32 || mmc5.splitTileNumber >= 40))
				{
					if (isNTFetch)
					{
						mmc5.exAttrLastNTFetch = ntOffset;
						mmc5.exAttrFetchCounter = 3;
						// fall through: return actual NT tile byte normally
					}
					else if (isAttrFetch && mmc5.exAttrFetchCounter == 3)
					{
						mmc5.exAttrFetchCounter = 2;
						const uint8_t exVal = mmc5.exRam[mmc5.exAttrLastNTFetch & 0x3FF];
						mmc5.exAttrSelectedChrBank = (exVal & 0x3F) | ((uint8_t)(mmc5.chrUpperBits << 6));
						const uint8_t palette = (exVal & 0xC0) >> 6;
						RETURN (uint8_t)(palette * 0x55);
					}
				}

				// --- Per-nametable routing ---
				const uint8_t ntSel = (mmc5.nametableMapping >> (ntIndex * 2)) & 0x03;
				switch (ntSel)
				{
				case 0: RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset];
				case 1: RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset];
				case 2:
				{
					if (mmc5.extendedRamMode <= 1)
					{
						// Modes 0/1: ExRAM is available to PPUDATA.
						// Extended-attribute mode has already been intercepted above.
						RETURN mmc5.exRam[ntOffset];
					}
					// Modes 2/3: ExRAM is not available as a PPU nametable.
					RETURN 0;
				}
				case 3:
				{
					if (ntOffset < 0x3C0)
						RETURN mmc5.fillTile;
					const uint8_t c = mmc5.fillColor & 0x03;
					RETURN (uint8_t)(c | (c << 2) | (c << 4) | (c << 6));
				}
				}
			}
			else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_068
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				BYTE ntControl = pNES_instance->NES_state.catridgeInfo.ines_067_068.ntControl;
				bool useChrRomForNt = (ntControl & 0x10) != 0;

				if (useChrRomForNt)
				{
					uint32_t targetBank = 0;
					uint16_t offset = 0;

					// 1. Resolve which physical bank (ntBank0 or ntBank1) applies based on the current mirroring mode setup
					if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::VERTICAL_MIRROR)
					{
						if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS) ||
							IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
						{
							targetBank = pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank0; // Lower CIRAM page equivalent
						}
						else // NAME_TABLE1 or NAME_TABLE3
						{
							targetBank = pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank1; // Upper CIRAM page equivalent
						}
					}
					else if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::HORIZONTAL_MIRROR)
					{
						if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS) ||
							IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
						{
							targetBank = pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank0;
						}
						else // NAME_TABLE2 or NAME_TABLE3
						{
							targetBank = pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank1;
						}
					}
					else if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR)
					{
						targetBank = pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank0;
					}
					else if (pNES_instance->NES_state.catridgeInfo.nameTblMir == NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR)
					{
						targetBank = pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank1;
					}

					// 2. Extract the relative 1 KiB tile offset from the matching PPU window
					if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
					{
						offset = address - NAME_TABLE0_START_ADDRESS;
					}
					else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
					{
						offset = address - NAME_TABLE1_START_ADDRESS;
					}
					else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
					{
						offset = address - NAME_TABLE2_START_ADDRESS;
					}
					else if (IF_ADDRESS_WITHIN(address, NAME_TABLE3_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
					{
						offset = address - NAME_TABLE3_START_ADDRESS;
					}

					// 3. Return directly from CHR-ROM, completely bypassing standard VRAM lookups below
					uint32_t index = (targetBank * 0x0400) + offset;
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
				}
			}
			else if ((pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::VRC6_024
				|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::VRC6_026)
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				auto& vrc6 = pNES_instance->NES_state.catridgeInfo.vrc6;

				// Refer to https://www.nesdev.org/wiki/VRC6#Banking_modes
				// Full banking-mode decode from $B003 — Mesen VRC6::UpdatePpuBanking()
				const uint8_t bm = vrc6.b003_reg;
				const uint8_t bmMasked = bm & 0x2F;   // collapse don't-care bits
				const uint8_t slot = (uint8_t)((address - NAME_TABLE0_START_ADDRESS) >> 10); // 0..3
				const uint16_t slotOff = (address - NAME_TABLE0_START_ADDRESS) & 0x3FF;

				if (bm & 0x10) // CHR-ROM backed nametables
				{
					const auto& hdrNT = pINES->iNES_Fields.iNES_header.fields;
					const bool isNES2NT = ((hdrNT.flag7.raw & 0x0C) == 0x08);
					const uint32_t totalChr8kNT =
						isNES2NT
						? (hdrNT.sizeOfChrRomIn8KB | (hdrNT.flags_8to15.nes2p0.flag9.fields.chrRomMSB << EIGHT))
						: hdrNT.sizeOfChrRomIn8KB;
					const uint32_t totalChr1kNT = (totalChr8kNT == 0) ? 8 : (totalChr8kNT << THREE);

					uint8_t ntPage[4];
					switch (bmMasked)
					{
					case 0x20: case 0x27:
						ntPage[0] = vrc6.chrBank[6] & 0xFE;
						ntPage[1] = (vrc6.chrBank[6] & 0xFE) | 1;
						ntPage[2] = vrc6.chrBank[7] & 0xFE;
						ntPage[3] = (vrc6.chrBank[7] & 0xFE) | 1;
						BREAK;
					case 0x23: case 0x24:
						ntPage[0] = vrc6.chrBank[6] & 0xFE;
						ntPage[1] = vrc6.chrBank[7] & 0xFE;
						ntPage[2] = (vrc6.chrBank[6] & 0xFE) | 1;
						ntPage[3] = (vrc6.chrBank[7] & 0xFE) | 1;
						BREAK;
					case 0x28: case 0x2F:
						ntPage[0] = ntPage[1] = vrc6.chrBank[6] & 0xFE;
						ntPage[2] = ntPage[3] = vrc6.chrBank[7] & 0xFE;
						BREAK;
					case 0x2B: case 0x2C:
						ntPage[0] = ntPage[1] = (vrc6.chrBank[6] & 0xFE) | 1;
						ntPage[2] = ntPage[3] = (vrc6.chrBank[7] & 0xFE) | 1;
						BREAK;
					default:
						switch (bm & 0x07)
						{
						case 0: case 6: case 7:
							ntPage[0] = ntPage[1] = vrc6.chrBank[6];
							ntPage[2] = ntPage[3] = vrc6.chrBank[7];
							BREAK;
						case 1: case 5:
							ntPage[0] = vrc6.chrBank[4];
							ntPage[1] = vrc6.chrBank[5];
							ntPage[2] = vrc6.chrBank[6];
							ntPage[3] = vrc6.chrBank[7];
							BREAK;
						default: // 2, 3, 4
							ntPage[0] = ntPage[2] = vrc6.chrBank[6];
							ntPage[1] = ntPage[3] = vrc6.chrBank[7];
							BREAK;
						}
						BREAK;
					}

					const uint32_t chrIdx = (ntPage[slot] % totalChr1kNT) * 0x0400 + slotOff;
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[chrIdx];
				}
				else // CIRAM-backed nametables
				{
					uint8_t ntSel[4]; // 0 = nameTable0, 1 = nameTable1
					switch (bmMasked)
					{
					case 0x20: case 0x27: // Vertical
						ntSel[0] = ntSel[2] = 0;
						ntSel[1] = ntSel[3] = 1;
						BREAK;
					case 0x23: case 0x24: // Horizontal
						ntSel[0] = ntSel[1] = 0;
						ntSel[2] = ntSel[3] = 1;
						BREAK;
					case 0x28: case 0x2F: // Single-screen A (lower)
						ntSel[0] = ntSel[1] = ntSel[2] = ntSel[3] = 0;
						BREAK;
					case 0x2B: case 0x2C: // Single-screen B (upper)
						ntSel[0] = ntSel[1] = ntSel[2] = ntSel[3] = 1;
						BREAK;
					default:
						switch (bm & 0x07)
						{
						case 0: case 6: case 7:
							ntSel[0] = ntSel[1] = vrc6.chrBank[6] & 0x01;
							ntSel[2] = ntSel[3] = vrc6.chrBank[7] & 0x01;
							BREAK;
						case 1: case 5:
							ntSel[0] = vrc6.chrBank[4] & 0x01;
							ntSel[1] = vrc6.chrBank[5] & 0x01;
							ntSel[2] = vrc6.chrBank[6] & 0x01;
							ntSel[3] = vrc6.chrBank[7] & 0x01;
							BREAK;
						default: // 2, 3, 4
							ntSel[0] = ntSel[2] = vrc6.chrBank[6] & 0x01;
							ntSel[1] = ntSel[3] = vrc6.chrBank[7] & 0x01;
							BREAK;
						}
						BREAK;
					}

					BYTE* nt = ntSel[slot]
						? pNES_ppuMemory->NESMemoryMap.nameTable1
						: pNES_ppuMemory->NESMemoryMap.nameTable0;
					RETURN nt[slotOff];
				}
			}
			else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_118
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				const auto& txs = pNES_instance->NES_state.catridgeInfo.mmc3.txsrom;

				uint16_t ntOffset = 0;
				uint8_t  ntIndex = 0;
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0; ntOffset = address - NAME_TABLE0_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1; ntOffset = address - NAME_TABLE1_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2; ntOffset = address - NAME_TABLE2_START_ADDRESS;
				}
				else
				{
					ntIndex = 3; ntOffset = address - NAME_TABLE3_START_ADDRESS;
				}

				if (txs.ntPage[ntIndex] == ONE)
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset];
				}
				else
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset];
				}
			}
			else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_158
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				// Mapper 158 reuses the same txsrom.ntPage[] storage — it is populated
				// by the RAMBO-1 $8001 write handler below.
				const auto& txs = pNES_instance->NES_state.catridgeInfo.mmc3.txsrom;

				uint16_t ntOffset = 0;
				uint8_t  ntIndex = 0;
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0; ntOffset = address - NAME_TABLE0_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1; ntOffset = address - NAME_TABLE1_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2; ntOffset = address - NAME_TABLE2_START_ADDRESS;
				}
				else
				{
					ntIndex = 3; ntOffset = address - NAME_TABLE3_START_ADDRESS;
				}

				if (txs.ntPage[ntIndex] == ONE)
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset];
				}
				else
				{
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset];
				}
			}
			else if (isNamco163() && IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				auto& n163 = pNES_instance->NES_state.catridgeInfo.namco163;

				uint8_t  ntIndex = 0;
				uint16_t ntOffset = 0;
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0; ntOffset = (uint16_t)(address - NAME_TABLE0_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1; ntOffset = (uint16_t)(address - NAME_TABLE1_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2; ntOffset = (uint16_t)(address - NAME_TABLE2_START_ADDRESS);
				}
				else
				{
					ntIndex = 3; ntOffset = (uint16_t)(address - NAME_TABLE3_START_ADDRESS);
				}

				// Slots 8-11 map PPU $2000-$2FFF (one slot per nametable)
				// $C000-$DFFF range: NT redirect ALWAYS allowed regardless of lowChrNtMode/highChrNtMode
				// (those flags only gate slots 0-7 in the pattern table range)
				// Ref: Mesen Namco163::WriteRegister() case 0xC000 — no mode check here
				const uint8_t slot = ntIndex + 8;
				const BYTE    bankVal = n163.chrBanks[slot];

				if (n163.variant == 0 && bankVal >= 0xE0)
				{
					// bit 0 selects CIRAM bank
					if ((bankVal & 0x01) == 0)
						RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset];
					else
						RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset];
				}

				// CHR-ROM nametable (value < 0xE0, or non-Namco163 variant)
				// Used by some Namco163 games that point nametable slots at CHR-ROM for
				// read-only background attribute data
				if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB > ZERO)
				{
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const bool     isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
					const uint32_t totalChr1k = isNES2
						? ((hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8)) * 8)
						: (hdr.sizeOfChrRomIn8KB * 8);
					const uint32_t chrIndex = ((uint32_t)bankVal % (totalChr1k > 0 ? totalChr1k : 1)) * 0x400 + ntOffset;
					RETURN pNES_catridgeMemory->maxCatridgeCHRROM[chrIndex];
				}

				// CHR-RAM fallback: treat as standard CIRAM via bit 0
				if ((bankVal & 0x01) == 0)
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset];
				else
					RETURN pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset];
			}
			else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_030
				&& pNES_instance->NES_state.catridgeInfo.ines030.ntMode == INES030_NT_MODE::FOUR_SCREEN_CART_VRAM
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

				uint64_t chrRamSizeBytes = ZERO;
				uint64_t chrNvRamSizeBytes = ZERO;
				uint64_t totalChrRam = ZERO;
				if (isNES2)
				{
					BYTE chrRamShift = hdr.flags_8to15.nes2p0.flag11.fields.chrVolRam;
					BYTE chrNvRamShift = hdr.flags_8to15.nes2p0.flag11.fields.chrNonVolRam;
					chrRamSizeBytes = (chrRamShift == ZERO) ? ZERO : (64ULL << chrRamShift);
					chrNvRamSizeBytes = (chrNvRamShift == ZERO) ? ZERO : (64ULL << chrNvRamShift);
				}
				totalChrRam = chrRamSizeBytes + chrNvRamSizeBytes;

				// Plain iNES 1.0 (no NES 2.0 size fields) or a header that under-reports:
				// wiki says default to 32KB for this board regardless.
				if (totalChrRam < 0x2000)
				{
					totalChrRam = 0x8000ULL;
				}

				const uint32_t last8kBase = (uint32_t)(totalChrRam - 0x2000ULL);

				uint16_t ntOffset = 0;
				uint8_t  ntIndex = 0;
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0; ntOffset = address - NAME_TABLE0_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1; ntOffset = address - NAME_TABLE1_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2; ntOffset = address - NAME_TABLE2_START_ADDRESS;
				}
				else
				{
					ntIndex = 3; ntOffset = address - NAME_TABLE3_START_ADDRESS;
				}

				const uint32_t index = last8kBase + (ntIndex * 0x0400u) + ntOffset;
				RETURN pNES_catridgeMemory->maxCatridgeCHRROM[index];
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

	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC3
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_014
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_037
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_047
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_119
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_118  // TxSROM
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::RAMBO1
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_158
		|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_268)
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
		case MAPPER::INES_MAPPER_180:
		case MAPPER::CNROM:
		case MAPPER::J87:
		case MAPPER::AxROM:
		case MAPPER::GxROM:
		case MAPPER::COLOR_DREAMS:
		case MAPPER::INES_MAPPER_034:
		case MAPPER::INES_MAPPER_067:
		case MAPPER::INES_MAPPER_068:
		case MAPPER::INES_MAPPER_070:
		case MAPPER::INES_MAPPER_152:
		case MAPPER::MMC5:
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
		case MAPPER::INES_MAPPER_069:
		{
			if (pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO)
			{
				if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
				{
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;

					// Calculate the 1KB bank index (0 to 7) based on the current PPU address
					// Mapper 69 allows independent mapping of eight 1KB banks
					const uint32_t bank = address >> 10;
					const uint32_t offset = address & 0x03FF;

					// Fetch the physical bank selected by the mapper registers, scale to bytes, and add offset
					uint32_t index = (pNES_instance->NES_state.catridgeInfo.ines069.chrBank[bank] * 0x0400) + offset;

					// 8KB CHR-RAM
					index &= 0x1FFF;
					pNES_ppuMemory->NESMemoryMap.patternTable.raw[index] = data;
				}
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_218:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				BYTE* nt = nullptr;
				switch (pNES_instance->NES_state.catridgeInfo.nameTblMir)
				{
				case NAMETABLE_MIRROR::VERTICAL_MIRROR:
					nt = (address & 0x0400)
						? pNES_ppuMemory->NESMemoryMap.nameTable1
						: pNES_ppuMemory->NESMemoryMap.nameTable0;
					BREAK;
				case NAMETABLE_MIRROR::HORIZONTAL_MIRROR:
					nt = (address & 0x0800)
						? pNES_ppuMemory->NESMemoryMap.nameTable1
						: pNES_ppuMemory->NESMemoryMap.nameTable0;
					BREAK;
				case NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR:
					// $A8: CIRAM A10 <- PPU A12 -> pattern table0 = BLK0, pattern table1 = BLK1
					nt = (address & 0x1000)
						? pNES_ppuMemory->NESMemoryMap.nameTable1
						: pNES_ppuMemory->NESMemoryMap.nameTable0;
					BREAK;
				case NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR:
					// $A9: CIRAM A10 <- PPU A13 -> always BLK0 (A13 is always 0 within $0000-$1FFF)
					nt = pNES_ppuMemory->NESMemoryMap.nameTable0;
					BREAK;
				}

				if (nt != nullptr)
				{
					nt[address & 0x03FF] = data;
				}
			}
			BREAK;
		}
		case MAPPER::CPROM:
		{
			/*
			 * ============================================================================
			 * NES-CPROM (Mapper 13) Memory Mapping Table
			 * ============================================================================
			 * The physical CPROM board has 16 KiB of CHR-RAM, split into four 4 KiB chunks
			 * (Chunks 0, 1, 2, 3).
			 *
			 * Hardware Layout:
			 * - PPU $0000-$0FFF (Table 0) is permanently hardwired to Chunk 0.
			 * - PPU $1000-$1FFF (Table 1) selects a chunk based on the 'chrBank' register.
			 *   Due to board routing logic (74HC08), Bank 3 wraps around to Chunk 0.
			 *
			 * Emulation Layout (Using: index = (bank << 12) | (address & 0x0FFF)):
			 * Because your array is oversized, our indexing maps each bank linearly.
			 *
			 * +----------+---------------+-------------------+-----------------------+
			 * | chrBank  | PPU Table 0   | PPU Table 1       | Net Hardware Behavior |
			 * | Register | ($0000-$0FFF) | ($1000-$1FFF)     |                       |
			 * +----------+---------------+-------------------+-----------------------+
			 * |    0     | Array[0x0000] | Array[0x0000]*    | Table 0 = Chunk 0     |
			 * |          | (Chunk 0)     | (Chunk 0)*        | Table 1 = Chunk 1     |
			 * +----------+---------------+-------------------+-----------------------+
			 * |    1     | Array[0x0000] | Array[0x1000]     | Table 0 = Chunk 0     |
			 * |          | (Chunk 0)     | (Chunk 1)         | Table 1 = Chunk 2     |
			 * +----------+---------------+-------------------+-----------------------+
			 * |    2     | Array[0x0000] | Array[0x2000]     | Table 0 = Chunk 0     |
			 * |          | (Chunk 0)     | (Chunk 2)         | Table 1 = Chunk 3     |
			 * +----------+---------------+-------------------+-----------------------+
			 * |    3     | Array[0x0000] | Array[0x3000]**   | Both Tables point     |
			 * |          | (Chunk 0)     | (Chunk 0)**       | to Chunk 0 (Mirror)   |
			 * +----------+---------------+-------------------+-----------------------+
			 *
			 * *  CRITICAL BUG FIX NOTE:
			 *    When chrBank = 0, our shifted formula maps Table 1 to Array[0x0000].
			 *    To prevent Table 1 from overwriting Table 0's distinct UI tiles,
			 *    both PPU Read and PPU Write MUST use the exact same indexing math
			 *    so they safely share the data without dropping tiles or cross-corrupting.
			 *
			 * ** Hardware Mirroring Note:
			 *    On real hardware, selecting Bank 3 loops the internal memory lines back
			 *    to Chunk 0. In our emulator, it utilizes Array[0x3000]. Videomation
			 *    uses Bank 3 as a safe blank workspace, meaning this array separation
			 *    works perfectly.
			 * ============================================================================
			 */
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				uint32_t index = 0;

				if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE0_END_ADDRESS))
				{
					index = address & 0x0FFF;
				}
				else // PATTERN_TABLE1 ($1000-$1FFF)
				{
					BYTE bank = pNES_instance->NES_state.catridgeInfo.cprom.chrBank;
					index = (bank << 12) | (address & 0x0FFF);
				}

				pNES_catridgeMemory->maxCatridgeCHRROM[index] = data;
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_014:
		{
			// CHR is pure ROM in both VRC2 and MMC3 mode -- no write path exists
			// on real hardware, so this is intentionally a no-op.
			BREAK;
		}
		case MAPPER::INES_MAPPER_015:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const auto& reg = pNES_instance->NES_state.catridgeInfo.ines015;
				if ((reg.latchedAddr & 0x03) != THREE) // only mode 3 write-protects, per the compromise noted above
				{
					pNES_ppuMemory->NESMemoryMap.patternTable.raw[address - PATTERN_TABLE0_START_ADDRESS] = data;
				}
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_016:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS)) // $0000-$1FFF
			{
				// CHR-RAM Case (Writable pattern tables)
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
					RETURN;
				}

				// CHR-ROM Case (Ignore writes to read-only pattern hardware)
				RETURN;
			}
			BREAK; // Fall through directly to your standard nametable routing blocks below
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
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				auto& mmc1 = pNES_instance->NES_state.catridgeInfo.mmc1;

				// Allow PPU writes if the cartridge uses CHR-RAM (or has 0 CHR-ROM banks)
				const bool isChrRam = pNES_instance->NES_state.catridgeInfo.hasChrRam ||
					(pINES->iNES_Fields.iNES_header.fields.sizeOfChrRomIn8KB == ZERO);

				if (isChrRam)
				{
					const uint64_t totalChrBytes = pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes > 0
						? pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes
						: pNES_instance->NES_state.catridgeInfo.chrRomSizeBytes;

					uint32_t index = 0;

					// -------------------------------------------------------------
					// 8 KB CHR Mode (Control Reg Bit 4 == 0)
					// -------------------------------------------------------------
					if (mmc1.intfControlReg.fields1.c == RESET)
					{
						const uint32_t total8kBanks = static_cast<uint32_t>(totalChrBytes / 0x2000);

						if (total8kBanks > 0)
						{
							// Bit 0 of $A000 is IGNORED in 8 KB mode.
							// Shift right by 1 to convert register value to 8 KB bank index.
							const uint32_t bank8 = ((mmc1.chrBank8 & 0x1E) >> 1) % total8kBanks;
							index = (bank8 * 0x2000) | (address & 0x1FFF);
						}
						else
						{
							index = address & 0x1FFF;
						}
					}
					// -------------------------------------------------------------
					// 4 KB CHR Mode (Control Reg Bit 4 == 1)
					// -------------------------------------------------------------
					else
					{
						const uint32_t total4kBanks = static_cast<uint32_t>(totalChrBytes / 0x1000);
						const uint32_t patternTable = (address >> 12) & 1;
						const uint32_t rawBank = (patternTable == 0) ? mmc1.chrBank4Lo : mmc1.chrBank4Hi;

						if (total4kBanks > 0)
						{
							const uint32_t bank4 = (rawBank & 0x1F) % total4kBanks;
							index = (bank4 * 0x1000) | (address & 0x0FFF);
						}
						else
						{
							index = address & 0x1FFF;
						}
					}

					// Only force 8 KB masking if CHR-RAM size is strictly 8 KB or less (SNROM/SGROM hardware pin limit)
					if (totalChrBytes <= 0x2000)
					{
						index &= 0x1FFF;
					}
					else if (totalChrBytes > 0)
					{
						index %= totalChrBytes;
					}

					pNES_catridgeMemory->maxCatridgeCHRROM[index] = data;
				}
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_105:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
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
			BREAK;
		}
		case MAPPER::INES_MAPPER_018:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8kBanks =
					isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << EIGHT))
					: hdr.sizeOfChrRomIn8KB;

				// Only write if CHR RAM (no CHR ROM present)
				if (totalChr8kBanks == 0)
				{
					auto& j18 = pNES_instance->NES_state.catridgeInfo.jaleco18;
					const uint32_t totalChr1kBanks = 8; // 8KB CHR RAM = 8 x 1KB banks
					const uint32_t bankIndex = (address >> 10) & 0x07;
					const uint32_t bank = j18.chrBank[bankIndex] % totalChr1kBanks;
					const uint32_t index = (bank * 0x0400) + (address & 0x03FF);
					pNES_catridgeMemory->maxCatridgeCHRROM[index] = data;
				}
			}
			BREAK;
		}
		case MAPPER::VRC2_022:
		case MAPPER::VRC4_021:
		case MAPPER::VRC2_VRC4_023:
		case MAPPER::VRC2_VRC4_025:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8kBanks =
					isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << EIGHT))
					: hdr.sizeOfChrRomIn8KB;

				// CHR RAM only — CHR ROM is read-only silicon, writes are silently ignored
				if (totalChr8kBanks == 0)
				{
					const uint32_t totalChr1kBanks = 8; // 8KB CHR RAM = 8 x 1KB banks
					uint32_t bank_index = (address >> 10) & 0x07;
					uint32_t bank = pNES_instance->NES_state.catridgeInfo.vrc24.chrBank[bank_index];
					if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::VRC2_022)
					{
						bank >>= ONE;
					}
					bank %= totalChr1kBanks;
					const uint32_t index = (bank * 0x0400) + (address & 0x03FF);
					pNES_catridgeMemory->maxCatridgeCHRROM[index] = data;
				}
			}
			BREAK;
		}
		case MAPPER::VRC6_024:
		case MAPPER::VRC6_026:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
				const uint32_t totalChr8kBanks =
					isNES2
					? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << EIGHT))
					: hdr.sizeOfChrRomIn8KB;

				// CHR RAM only — CHR ROM is read-only silicon, writes are silently ignored
				if (totalChr8kBanks == 0)
				{
					auto& vrc6 = pNES_instance->NES_state.catridgeInfo.vrc6;
					const uint32_t totalChr1kBanks = 8;
					const uint32_t bank_index = (address >> 10) & 0x07;
					const uint32_t bank = vrc6.chrBank[bank_index] % totalChr1kBanks;
					const uint32_t index = (bank * 0x0400) + (address & 0x03FF);
					pNES_catridgeMemory->maxCatridgeCHRROM[index] = data;
				}
			}
			BREAK;
		}
		case MAPPER::MMC3:
		case MAPPER::INES_MAPPER_037:
		case MAPPER::INES_MAPPER_047:
		case MAPPER::INES_MAPPER_118:
		case MAPPER::INES_MAPPER_119:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint64_t totalChrRam = pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes;
				const uint64_t chrRomBytes = pNES_instance->NES_state.catridgeInfo.chrRomSizeBytes;

				// CHR-ROM is read-only / no CHR RAM available -> Ignore write
				if (chrRomBytes > ZERO || totalChrRam == ZERO)
				{
					BREAK;
				}

				// NOTE: Mapper 119 (TQROM) — bit 6 of CHR bank is chip select: 0=ROM, 1=RAM.
				// TQROM CS=1 writes to patternTable.raw (8KB RAM on board).
				// Refer https://www.nesdev.org/wiki/INES_Mapper_119
				//
				// For large pure CHR-RAM (>8KB, no CHR ROM): CS is forced to 1 and
				// maxCatridgeCHRROM is reused as the backing store (safe since CHR ROM size is zero).
				const bool isTQROM = (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_119);
				const bool isPureChrRam = (!isTQROM && chrRomBytes == ZERO && totalChrRam > ZERO);

				// RAM address mask for large CHR-RAM: mask to full declared size so upper banks are reachable.
				// Not used for TQROM (always 0x1FFF there).
				const uint32_t largeChrRamMask = (totalChrRam > ZERO) ? (uint32_t)(totalChrRam - ONE) : 0x1FFFu;

				BIT currentBankCS = RESET;

				BIT chrA12Inversion = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.chrA12Inversion;

				uint32_t startAddr1 = PATTERN_TABLE0_START_ADDRESS;
				uint32_t endAddr1 = startAddr1 + 0x07FF; // 2KB
				uint32_t startAddr2 = endAddr1 + ONE;
				uint32_t endAddr2 = startAddr2 + 0x07FF; // 2KB
				uint32_t startAddr3 = endAddr2 + ONE;
				uint32_t endAddr3 = startAddr3 + 0x03FF; // 1KB
				uint32_t startAddr4 = endAddr3 + ONE;
				uint32_t endAddr4 = startAddr4 + 0x03FF; // 1KB
				uint32_t startAddr5 = endAddr4 + ONE;
				uint32_t endAddr5 = startAddr5 + 0x03FF; // 1KB
				uint32_t startAddr6 = endAddr5 + ONE;
				uint32_t endAddr6 = startAddr6 + 0x03FF; // 1KB

				if (chrA12Inversion == SET)
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
					// Only read CS bits from ines119 registers for actual TQROM;
					// for large CHR-RAM, CS is implicitly 1 (all banks are RAM).
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2aCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						wrapAround = 0x03FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1aCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr1) & wrapAround);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF] = data; BREAK;
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask] = data;
					}
					BREAK;
				}
				if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
				{
					auto wrapAround = 0x07FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2bCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						wrapAround = 0x03FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1bCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr2) & wrapAround);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF] = data; BREAK;
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask] = data;
					}
					BREAK;
				}
				if (IF_ADDRESS_WITHIN(address, startAddr3, endAddr3))
				{
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1aCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1cCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr3) & 0x3FF);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF] = data; BREAK;
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask] = data;
					}
					BREAK;
				}
				if (IF_ADDRESS_WITHIN(address, startAddr4, endAddr4))
				{
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1bCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1dCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr4) & 0x3FF);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF] = data; BREAK;
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask] = data;
					}
					BREAK;
				}
				if (IF_ADDRESS_WITHIN(address, startAddr5, endAddr5))
				{
					auto wrapAround = 0x03FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1cCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						wrapAround = 0x07FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2aCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr5) & wrapAround);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF] = data; BREAK;
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask] = data;
					}
					BREAK;
				}
				if (IF_ADDRESS_WITHIN(address, startAddr6, endAddr6))
				{
					auto wrapAround = 0x03FF;
					auto index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d * 0x0400;
					if (isTQROM)
					{
						currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1dCS;
					}
					else if (isPureChrRam)
					{
						currentBankCS = ONE;
					}
					if (chrA12Inversion == SET)
					{
						wrapAround = 0x07FF;
						index = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b * 0x0400;
						if (isTQROM)
						{
							currentBankCS = pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2bCS;
						}
						else if (isPureChrRam)
						{
							currentBankCS = ONE;
						}
					}
					index += ((address - startAddr6) & wrapAround);
					if (currentBankCS == ONE)
					{
						// TQROM: 8KB CHR RAM lives in patternTable.raw
						if (isTQROM || totalChrRam <= 0x2000)
						{
							pNES_ppuMemory->NESMemoryMap.patternTable.raw[index & 0x1FFF] = data; BREAK;
						}
						// Large CHR-RAM: maxCatridgeCHRROM reused as backing store (CHR ROM size is zero)
						pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask] = data;
					}
					BREAK;
				}
				FATAL("Invalid CHR ROM/RAM address in MMC3");
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_268:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint64_t totalChrRam = pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes;
				const uint64_t chrRomBytes = pNES_instance->NES_state.catridgeInfo.chrRomSizeBytes;

				if (chrRomBytes > ZERO || totalChrRam == ZERO)
				{
					BREAK; // CHR-ROM is read-only / no CHR RAM available
				}

				const BIT chrA12Inversion = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.chrA12Inversion;

				uint32_t startAddr1 = PATTERN_TABLE0_START_ADDRESS, endAddr1 = startAddr1 + 0x07FF;
				uint32_t startAddr2 = endAddr1 + ONE, endAddr2 = startAddr2 + 0x07FF;
				uint32_t startAddr3 = endAddr2 + ONE, endAddr3 = startAddr3 + 0x03FF;
				uint32_t startAddr4 = endAddr3 + ONE, endAddr4 = startAddr4 + 0x03FF;
				uint32_t startAddr5 = endAddr4 + ONE, endAddr5 = startAddr5 + 0x03FF;
				uint32_t startAddr6 = endAddr5 + ONE, endAddr6 = startAddr6 + 0x03FF;

				if (chrA12Inversion == SET)
				{
					endAddr1 = startAddr1 + 0x03FF;
					startAddr2 = endAddr1 + ONE; endAddr2 = startAddr2 + 0x03FF;
					startAddr3 = endAddr2 + ONE; endAddr3 = startAddr3 + 0x03FF;
					startAddr4 = endAddr3 + ONE; endAddr4 = startAddr4 + 0x03FF;
					startAddr5 = endAddr4 + ONE; endAddr5 = startAddr5 + 0x07FF;
					startAddr6 = endAddr5 + ONE; endAddr6 = startAddr6 + 0x07FF;
				}

				BYTE nativeV1k = ZERO;
				uint32_t pageBase = ZERO;
				const auto& reg = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters;

				if (IF_ADDRESS_WITHIN(address, startAddr1, endAddr1))
				{
					pageBase = startAddr1;
					nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1a
						: (BYTE)(reg.chrBank2a + (((address - startAddr1) >> 10) & 1));
				}
				else if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
				{
					pageBase = startAddr2;
					nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1b
						: (BYTE)(reg.chrBank2b + (((address - startAddr2) >> 10) & 1));
				}
				else if (IF_ADDRESS_WITHIN(address, startAddr3, endAddr3))
				{
					pageBase = startAddr3;
					nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1c : reg.chrBank1a;
				}
				else if (IF_ADDRESS_WITHIN(address, startAddr4, endAddr4))
				{
					pageBase = startAddr4;
					nativeV1k = (chrA12Inversion == SET) ? reg.chrBank1d : reg.chrBank1b;
				}
				else if (IF_ADDRESS_WITHIN(address, startAddr5, endAddr5))
				{
					pageBase = startAddr5;
					nativeV1k = (chrA12Inversion == SET)
						? (BYTE)(reg.chrBank2a + (((address - startAddr5) >> 10) & 1))
						: reg.chrBank1c;
				}
				else
				{
					pageBase = startAddr6;
					nativeV1k = (chrA12Inversion == SET)
						? (BYTE)(reg.chrBank2b + (((address - startAddr6) >> 10) & 1))
						: reg.chrBank1d;
				}

				const auto& outer = pNES_instance->NES_state.catridgeInfo.mmc3.ines268;
				uint32_t page1k = mapper268ComputeChrPage(nativeV1k, (uint16_t)address, outer.reg);
				uint32_t index = (page1k * 0x0400u) + ((address - pageBase) & 0x03FFu);
				const uint32_t largeChrRamMask = (uint32_t)(totalChrRam - ONE);

				pNES_catridgeMemory->maxCatridgeCHRROM[index & largeChrRamMask] = data;
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_019:
		case MAPPER::INES_MAPPER_210:
		case MAPPER::RAMBO1:
		case MAPPER::INES_MAPPER_158:
		case MAPPER::MMC2:
		case MAPPER::MMC4:
		case MAPPER::INES_MAPPER_232:
		{
			// Pattern table writes only land when CHR-RAM (no CHR-ROM)
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
			// CHR-ROM: pattern table writes are silently ignored (ROM is read-only)
			BREAK;
		}
		case MAPPER::INES_MAPPER_078:
		{
			BREAK; // CHR-ROM only, no CHR-RAM on either board
		}
		case MAPPER::INES_MAPPER_028:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint32_t chrRamSizeBytes = static_cast<uint32_t>(pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes);
				if (chrRamSizeBytes > ZERO)
				{
					const uint32_t bank8 = pNES_instance->NES_state.catridgeInfo.ines028.reg00_chrBank & 0x03;
					const uint32_t index = ((bank8 * 0x2000u) + (address & 0x1FFFu)) % chrRamSizeBytes;
					pNES_catridgeMemory->maxCatridgeCHRROM[index] = data;
				}
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_029:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint32_t bank8 = pNES_instance->NES_state.catridgeInfo.ines029.chrBank8;
				const uint32_t index = (bank8 * 0x2000u) + (address & 0x1FFFu);
				pNES_catridgeMemory->maxCatridgeCHRROM[index] = data;
			}
			BREAK;
		}
		case MAPPER::INES_MAPPER_030:
		{
			if (IF_ADDRESS_WITHIN(address, PATTERN_TABLE0_START_ADDRESS, PATTERN_TABLE1_END_ADDRESS))
			{
				const uint32_t bank8 = pNES_instance->NES_state.catridgeInfo.ines030.chrBank8;
				const uint32_t index = (bank8 * 0x2000u) + (address & 0x1FFFu);
				pNES_catridgeMemory->maxCatridgeCHRROM[index] = data;
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

			if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC5)
			{
				auto& mmc5 = pNES_instance->NES_state.catridgeInfo.mmc5;

				uint8_t ntIndex = 0;
				uint16_t ntOffset = 0;

				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0;
					ntOffset = (uint16_t)(address - NAME_TABLE0_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1;
					ntOffset = (uint16_t)(address - NAME_TABLE1_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2;
					ntOffset = (uint16_t)(address - NAME_TABLE2_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE3_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
				{
					ntIndex = 3;
					ntOffset = (uint16_t)(address - NAME_TABLE3_START_ADDRESS);
				}

				const uint8_t ntSel =
					(mmc5.nametableMapping >> (ntIndex * 2)) & 0x03;

				switch (ntSel)
				{
				case 0:
					// $5105 = %00 : CIRAM / nametable 0
					pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset] = data;
					RETURN;

				case 1:
					// $5105 = %01 : CIRAM / nametable 1
					pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset] = data;
					RETURN;

				case 2:
					// $5105 = %10 : ExRAM
					//
					// ExRAM is writable through the PPU nametable path
					// when $5104 is 0 or 1.
					//
					// In mode 2, ExRAM is general-purpose CPU RAM and
					// is not writable through the PPU nametable path.
					//
					// In mode 3, ExRAM is read-only and PPU writes are ignored.
					if (mmc5.extendedRamMode <= 1)
					{
						mmc5.exRam[ntOffset] = data;
					}

					RETURN;

				case 3:
					// $5105 = %11 : Fill mode
					//
					// Fill-mode nametable is generated from $5106/$5107.
					// PPU writes do not modify fillTile/fillColor.
					RETURN;
				}
			}
			else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_068
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				BYTE ntControl = pNES_instance->NES_state.catridgeInfo.ines_067_068.ntControl;
				bool useChrRomForNt = (ntControl & 0x10) != 0;

				if (useChrRomForNt)
				{
					// Rom Nametable Mode is enabled. The physical backing memory is CHR-ROM.
					// Because ROM is read-only, PPU writes here are completely ignored by the hardware.
					RETURN;
				}

				// If useChrRomForNt is false, it drops down here, exits this block, 
				// and naturally falls through to your normal internal VRAM RAM mode logic below.
			}
			else if ((pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::VRC6_024
				|| pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::VRC6_026)
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				auto& vrc6 = pNES_instance->NES_state.catridgeInfo.vrc6;

				// Refer to https://www.nesdev.org/wiki/VRC6#Banking_modes
				// Full banking-mode decode from $B003 — Mesen VRC6::UpdatePpuBanking()
				const uint8_t bm = vrc6.b003_reg;
				const uint8_t bmMasked = bm & 0x2F;
				const uint8_t slot = (uint8_t)((address - NAME_TABLE0_START_ADDRESS) >> 10); // 0..3
				const uint16_t slotOff = (address - NAME_TABLE0_START_ADDRESS) & 0x3FF;

				if (bm & 0x10)
				{
					// CHR-ROM backed nametables are read-only — writes are silently discarded
					RETURN;
				}

				// CIRAM-backed nametables
				uint8_t ntSel[4]; // 0 = nameTable0, 1 = nameTable1
				switch (bmMasked)
				{
				case 0x20: case 0x27: // Vertical
					ntSel[0] = ntSel[2] = 0;
					ntSel[1] = ntSel[3] = 1;
					BREAK;
				case 0x23: case 0x24: // Horizontal
					ntSel[0] = ntSel[1] = 0;
					ntSel[2] = ntSel[3] = 1;
					BREAK;
				case 0x28: case 0x2F: // Single-screen A (lower)
					ntSel[0] = ntSel[1] = ntSel[2] = ntSel[3] = 0;
					BREAK;
				case 0x2B: case 0x2C: // Single-screen B (upper)
					ntSel[0] = ntSel[1] = ntSel[2] = ntSel[3] = 1;
					BREAK;
				default:
					switch (bm & 0x07)
					{
					case 0: case 6: case 7:
						ntSel[0] = ntSel[1] = vrc6.chrBank[6] & 0x01;
						ntSel[2] = ntSel[3] = vrc6.chrBank[7] & 0x01;
						BREAK;
					case 1: case 5:
						ntSel[0] = vrc6.chrBank[4] & 0x01;
						ntSel[1] = vrc6.chrBank[5] & 0x01;
						ntSel[2] = vrc6.chrBank[6] & 0x01;
						ntSel[3] = vrc6.chrBank[7] & 0x01;
						BREAK;
					default: // 2, 3, 4
						ntSel[0] = ntSel[2] = vrc6.chrBank[6] & 0x01;
						ntSel[1] = ntSel[3] = vrc6.chrBank[7] & 0x01;
						BREAK;
					}
					BREAK;
				}

				BYTE* nt = ntSel[slot]
					? pNES_ppuMemory->NESMemoryMap.nameTable1
					: pNES_ppuMemory->NESMemoryMap.nameTable0;
				nt[slotOff] = data;
				RETURN;
			}
			else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_118
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				const auto& txs = pNES_instance->NES_state.catridgeInfo.mmc3.txsrom;

				uint16_t ntOffset = 0;
				uint8_t  ntIndex = 0;
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0; ntOffset = address - NAME_TABLE0_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1; ntOffset = address - NAME_TABLE1_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2; ntOffset = address - NAME_TABLE2_START_ADDRESS;
				}
				else
				{
					ntIndex = 3; ntOffset = address - NAME_TABLE3_START_ADDRESS;
				}

				if (txs.ntPage[ntIndex] == ONE)
				{
					pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset] = data;
				}
				else
				{
					pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset] = data;
				}
				RETURN;
			}
			else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_158
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				const auto& txs = pNES_instance->NES_state.catridgeInfo.mmc3.txsrom;

				uint16_t ntOffset = 0;
				uint8_t  ntIndex = 0;
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0; ntOffset = address - NAME_TABLE0_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1; ntOffset = address - NAME_TABLE1_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2; ntOffset = address - NAME_TABLE2_START_ADDRESS;
				}
				else
				{
					ntIndex = 3; ntOffset = address - NAME_TABLE3_START_ADDRESS;
				}

				if (txs.ntPage[ntIndex] == ONE)
				{
					pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset] = data;
				}
				else
				{
					pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset] = data;
				}
				RETURN;
			}
			else if (isNamco163() && IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				auto& n163 = pNES_instance->NES_state.catridgeInfo.namco163;

				uint8_t  ntIndex = 0;
				uint16_t ntOffset = 0;
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0; ntOffset = (uint16_t)(address - NAME_TABLE0_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1; ntOffset = (uint16_t)(address - NAME_TABLE1_START_ADDRESS);
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2; ntOffset = (uint16_t)(address - NAME_TABLE2_START_ADDRESS);
				}
				else
				{
					ntIndex = 3; ntOffset = (uint16_t)(address - NAME_TABLE3_START_ADDRESS);
				}

				const uint8_t slot = ntIndex + 8;
				const BYTE    bankVal = n163.chrBanks[slot];

				// Only CIRAM-backed slots are writable
				// CHR-ROM slots silently drop the write (ROM is read-only)
				if (n163.variant == 0 && bankVal >= 0xE0)
				{
					if ((bankVal & 0x01) == 0)
						pNES_ppuMemory->NESMemoryMap.nameTable0[ntOffset] = data;
					else
						pNES_ppuMemory->NESMemoryMap.nameTable1[ntOffset] = data;
				}
				RETURN;
			}
			else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_030
				&& pNES_instance->NES_state.catridgeInfo.ines030.ntMode == INES030_NT_MODE::FOUR_SCREEN_CART_VRAM
				&& IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE3_END_ADDRESS))
			{
				const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
				const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

				uint64_t chrRamSizeBytes = ZERO;
				uint64_t chrNvRamSizeBytes = ZERO;
				uint64_t totalChrRam = ZERO;
				if (isNES2)
				{
					BYTE chrRamShift = hdr.flags_8to15.nes2p0.flag11.fields.chrVolRam;
					BYTE chrNvRamShift = hdr.flags_8to15.nes2p0.flag11.fields.chrNonVolRam;
					chrRamSizeBytes = (chrRamShift == ZERO) ? ZERO : (64ULL << chrRamShift);
					chrNvRamSizeBytes = (chrNvRamShift == ZERO) ? ZERO : (64ULL << chrNvRamShift);
				}
				totalChrRam = chrRamSizeBytes + chrNvRamSizeBytes;

				// Plain iNES 1.0 (no NES 2.0 size fields) or a header that under-reports:
				// wiki says default to 32KB for this board regardless.
				if (totalChrRam < 0x2000)
				{
					totalChrRam = 0x8000ULL;
				}

				const uint32_t last8kBase = (uint32_t)(totalChrRam - 0x2000ULL);

				uint16_t ntOffset = 0;
				uint8_t  ntIndex = 0;
				if (IF_ADDRESS_WITHIN(address, NAME_TABLE0_START_ADDRESS, NAME_TABLE0_END_ADDRESS))
				{
					ntIndex = 0; ntOffset = address - NAME_TABLE0_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE1_START_ADDRESS, NAME_TABLE1_END_ADDRESS))
				{
					ntIndex = 1; ntOffset = address - NAME_TABLE1_START_ADDRESS;
				}
				else if (IF_ADDRESS_WITHIN(address, NAME_TABLE2_START_ADDRESS, NAME_TABLE2_END_ADDRESS))
				{
					ntIndex = 2; ntOffset = address - NAME_TABLE2_START_ADDRESS;
				}
				else
				{
					ntIndex = 3; ntOffset = address - NAME_TABLE3_START_ADDRESS;
				}

				const uint32_t index = last8kBase + (ntIndex * 0x0400u) + ntOffset;
				pNES_catridgeMemory->maxCatridgeCHRROM[index] = data;
				RETURN;
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
			else if (index == FOUR || index == (FOUR + SIXTEEN))
			{
				index = FOUR;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index] = data;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index + SIXTEEN] = data;
			}
			else if (index == EIGHT || index == (EIGHT + SIXTEEN))
			{
				index = EIGHT;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index] = data;
				pNES_ppuMemory->NESMemoryMap.paletteRamIndex[index + SIXTEEN] = data;
			}
			else if (index == TWELVE || index == (TWELVE + SIXTEEN))
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
		RETURN (pNES_cpuRegisters->a & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_X:
	{
		RETURN (pNES_cpuRegisters->x & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_Y:
	{
		RETURN (pNES_cpuRegisters->y & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_P:
	{
		RETURN (pNES_cpuRegisters->p.p & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_SP:
	{
		RETURN (pNES_cpuRegisters->sp & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_PC:
	{
		RETURN (pNES_cpuRegisters->pc & 0xFFFF); BREAK;
	}

	case REGISTER_TYPE::RT_NONE:
	{
		RETURN (uint16_t)NULL;  BREAK;
	}
	default:
	{
		RETURN (uint16_t)NULL; BREAK;
	}
	}
}

inline byte NES_t::readCpuRawMemoryInternal(uint16_t address, MEMORY_ACCESS_SOURCE source)
{
	if (ROM_TYPE == ROM::TEST_ROM_BIN)
	{
		RETURN pNES_cpuMemory->NESRawMemory[address];
	}

	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentAccessType = TYPE_OF_MEMORY_ACCESS::CPU_READ;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.previousCPUAccessType = pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentCPUAccessType;
	pNES_instance->NES_state.emulatorStatus.memoryAccessType.currentCPUAccessType = TYPE_OF_MEMORY_ACCESS::CPU_READ;

#if (ENABLE_R2A03_SST == YES)
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
#endif
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
				auto& openBus = pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue;

				address -= PPU_START_ADDRESS;
				auto index = address & (PPU_CTRL_REG_SIZE - ONE); // % 0x0008

				// First process anything that needs to be done in PPU when CPU reads the PPU registers 

				switch (index + PPU_START_ADDRESS)
				{
				case PPU_CTRL_ADDRESS:
				case PPU_MASK_ADDRESS:
				case OAM_ADDR_ADDRESS:
				case PPU_SCROLL_ADDRESS:
				{
					RETURN applyOpenBusDecay();
				}
				case PPU_STATUS_ADDRESS:
				{
					pNES_ppuRegisters->ppuInternalRegisters.w = FIRST_WRITE; // Reading PPU status register clears the PPU internal W register

					// First, fill the open bus values
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS.raw = openBus;

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

					auto statusVal = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUSTATUS.raw;

					// bits 7:5 are driven, bits 4:0 are open bus
					RETURN applyOpenBus(0x1F, statusVal);
				}
				case OAM_DATA_ADDRESS:
				{
					SCOUNTER64 ppuCycle = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;
					SCOUNTER32 scanline = pNES_instance->NES_state.display.currentScanline;
					// Refer to https://forums.nesdev.org/viewtopic.php?f=3&t=15763 -- sprite evaluation (and thus the
					// $2004 read-during-rendering glitch) is active on the pre-render scanline too, not just the
					// visible ones, matching the state machine driving oamByte (see the block gated on
					// "ly >= NES_PRE_RENDER_SCANLINE" elsewhere in this file). The old isVisibleScanline check here
					// excluded the pre-render line, so reads during dots 65-256 of that line incorrectly fell
					// through to a plain direct-memory read instead of exposing the glitched evaluation state.
					bool isEvaluatingSprites = (scanline >= NES_PRE_RENDER_SCANLINE && scanline <= NES_LAST_VISIBLE_PPU_SCANLINE);
					bool isRendering = (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET
						|| pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET);

					uint8_t data;

					if (isRendering && isEvaluatingSprites)
					{
						// Refer to https://www.nesdev.org/wiki/PPU_registers#OAMDATA
						// "It mentions the following : "Reading OAMDATA while the PPU is rendering will expose internal OAM accesses during sprite evaluation and loading"
						if (ppuCycle >= ONE && ppuCycle <= SIXTYFOUR)
						{
							// Refer to `Cycles 1-64` of https://www.nesdev.org/wiki/PPU_sprite_evaluation
							// Secondary OAM being cleared — always reads $FF
							data = 0xFF;
						}
						else if (ppuCycle >= SIXTYFIVE && ppuCycle <= TWOFIFTYSIX)
						{
							// Sprite evaluation active — return oamByte (current read from primary OAM)
							data = pNES_ppuRegisters->oamByte;
						}
						else
						{
							// Cycles 257-320 sprite fetch — return $FF
							data = 0xFF;
						}
					}
					else
					{
						// Outside rendering — normal read
						data = pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR];
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
					}
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMDATA = data;
					RETURN applyOpenBus(0x00, data); // all bits driven
				}
				case PPU_ADDR_ADDRESS:
				{
					clockMMC3IRQ(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU, NO);
					applyOpenBusDecay();
					RETURN pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue;
				}
				case PPU_DATA_ADDRESS:
				{
					// Refer : https://www.nesdev.org/wiki/PPU_registers#PPUDATA
					if (pNES_ppuRegisters->ppuInternalRegisters.ignoreVramRead > ZERO)
					{
						// 2 reads to $2007 in quick succession causes 2nd read to be ignored
						RETURN applyOpenBus(0xFF, ZERO);
					}

					auto data = pNES_ppuRegisters->ppuInternalRegisters.cpu2ppu;
					pNES_ppuRegisters->ppuInternalRegisters.cpu2ppu
						= readPpuRawMemory(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU);

					bool isPaletteRead = IF_ADDRESS_WITHIN(pNES_ppuRegisters->ppuInternalRegisters.v.raw,
						PALETTE_RAM_INDEXES_START_ADDRESS, PALETTE_RAM_INDEXES_MIRROR_END_ADDRESS);

					if (isPaletteRead)
					{
						data = pNES_ppuRegisters->ppuInternalRegisters.cpu2ppu;

						// This is needed to handle vram_access.nes
						// Implemented based on https://forums.nesdev.org/viewtopic.php?p=79492#p79492
						pNES_ppuRegisters->ppuInternalRegisters.cpu2ppu
							= readPpuRawMemory(pNES_ppuRegisters->ppuInternalRegisters.v.raw - 0x1000, MEMORY_ACCESS_SOURCE::CPU);

						// For PPU Openbus: palette reads only drive bits 5:0
						data &= 0x3F;
					}

					// Defer VRAM address increment to next PPU cycle (matches Mesen _needVideoRamIncrement)
					pNES_ppuRegisters->ppuInternalRegisters.needVideoRamIncrement = YES;

					// Ignore next back-to-back read (6 PPU cycles worth)
					pNES_ppuRegisters->ppuInternalRegisters.ignoreVramRead = SIX;

					pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUDATA = data;

					RETURN isPaletteRead ? applyOpenBus(0xC0, data) : applyOpenBus(0x00, data);
				}
				default:
				{
					FATAL("Unknown PPU register");
					RETURN pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue;
				}
				}
			}
			else if (IF_ADDRESS_WITHIN(address, APU_AND_IO_START_ADDRESS, APU_AND_IO_END_ADDRESS))
			{
				if (address >= APU_AND_IO_START_ADDRESS && address <= 0x4014)
				{
					// To handle test_cpu_exec_space_apu.nes
					// NOTE : "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be returned
					// Maybe this is why test_cpu_exec_space_apu.nes is passing. But this needs further investigation
					RETURN (address >> EIGHT);
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
						RETURN ((byte)ImGui::IsKeyDown(ImGuiKey_Z) | (address >> EIGHT));
					}
					else if ((pNES_instance->NES_state.controller.startPolling == NO && pNES_instance->NES_state.controller.endPolling == YES)
						&& (pNES_instance->NES_state.controller.keyID >= KEY_A && pNES_instance->NES_state.controller.keyID <= KEY_RIGHT))
					{
						auto data = GETBIT(pNES_instance->NES_state.controller.keyID, pNES_instance->NES_state.controller.keyStatus);
						++pNES_instance->NES_state.controller.keyID;
						// Refer https://www.nesdev.org/wiki/Standard_controller for reasons to OR with MSB of address
						RETURN (data | (address >> EIGHT));
					}
					else
					{
						pNES_instance->NES_state.controller.keyID = INVALID;
						// To handle test_cpu_exec_space_apu.nes
						// "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be RETURNed
						// Also refer https://www.nesdev.org/wiki/Standard_controller for reasons to OR with MSB of address
						RETURN ((address >> EIGHT) | 0x01);
					}
				}
				if (address == JOYSTICK2_OR_FRAMECFG_ADDRESS)
				{
					if (enableZapper)
					{
						typedef union
						{
							struct
							{
								byte S : 1;       // bit 0 — always 0, zapper has no shift register
								byte unused0 : 2; // bits 1 - 2
								byte W : 1;       // bit 3 — 0 = light detected, 1 = no light
								byte T : 1;       // bit 4 — 0 = trigger pulled, 1 = released
								byte unused1 : 3; // bits 5 - 7
							} fields;
							byte raw;
						} zapper_t;
						zapper_t zapper = { ZERO };
						float x = 0.0f, y = 0.0f;
						FLAG isInsideScreen = getMouseRelPosIfDocked(&x, &y, getScreenWidth(), getScreenHeight());

						zapper.fields.W = SET; // default: no light
						if (isInsideScreen == YES)
						{
							const int32_t cursorX = (int32_t)x;
							const int32_t cursorY = (int32_t)y;
							const int32_t currentLy = (int32_t)pNES_instance->NES_state.display.currentScanline;
							const int32_t currentCy = (int32_t)pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;

							// NOTE: Zapper light detection matches Mesen's logic:
							// - Check a small radius around the cursor (real sensor isn't a single pixel)
							// - Beam must have already passed the target pixel (scanline+cycle check)
							// - Scanline window: 20 lines behind beam
							// - Brightness threshold: luminance >= 85/255 on rendered RGB
							// Refer https://www.nesdev.org/wiki/Zapper
							static constexpr int32_t ZAPPER_SCANLINE_WINDOW = 20;
							static constexpr int32_t ZAPPER_RADIUS = 2;
							static constexpr uint8_t ZAPPER_BRIGHTNESS = 85;

							bool lightFound = false;
							for (int32_t yOffset = -ZAPPER_RADIUS; yOffset <= ZAPPER_RADIUS && !lightFound; ++yOffset)
							{
								const int32_t yPos = cursorY + yOffset;
								if (yPos < ZERO || yPos >= (int32_t)getScreenHeight())
								{
									continue;
								}
								for (int32_t xOffset = -ZAPPER_RADIUS; xOffset <= ZAPPER_RADIUS && !lightFound; ++xOffset)
								{
									const int32_t xPos = cursorX + xOffset;
									if (xPos < ZERO || xPos >= (int32_t)getScreenWidth())
									{
										continue;
									}
									// Beam must have already passed this pixel:
									// scanline must be at or past yPos, within window,
									// and if on the same scanline the cycle must be past xPos
									const bool beamPastPixel = (currentLy >= yPos)
										&& ((currentLy - yPos) <= ZAPPER_SCANLINE_WINDOW)
										&& (currentLy != yPos || currentCy > xPos);
									if (beamPastPixel)
									{
										const Pixel& p = pNES_instance->NES_state.display.imGuiBuffer.imGuiBuffer2D[yPos][xPos];
										// Standard luminance formula (BT.601)
										const uint8_t luminance = (uint8_t)(0.299f * p.r + 0.587f * p.g + 0.114f * p.b);
										if (luminance >= ZAPPER_BRIGHTNESS)
										{
											lightFound = true;
										}
									}
								}
							}
							zapper.fields.W = lightFound ? RESET : SET;
						}

						// T=1 when mouse is held (half-pulled), T=0 when released
						zapper.fields.T = ImGui::IsMouseDown(ImGuiMouseButton_Left) ? SET : RESET;

						RETURN zapper.raw;
					}
					else
					{
						// To handle test_cpu_exec_space_apu.nes
						// "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be RETURNed
						// Refer https://www.nesdev.org/wiki/Standard_controller for reasons to OR with MSB of address
						RETURN (address >> EIGHT);
					}
				}

				FATAL("Invalid APU Address");
			}
			else if (IF_ADDRESS_WITHIN(address, OTHER_APU_AND_IO_START_ADDRESS, OTHER_APU_AND_IO_END_ADDRESS))
			{
				// To handle test_cpu_exec_space_apu.nes
				// NOTE : "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be RETURNed
				// Maybe this is why test_cpu_exec_space_apu.nes is passing. But this needs further investigation
				RETURN (address >> EIGHT);
			}
			else if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, UNMAPPED_END_ADDRESS))
			{
				// To handle test_cpu_exec_space_apu.nes
				// NOTE : "CPU open bus" section of https://www.nesdev.org/wiki/Open_bus_behavior mentions that "high byte of address" should be RETURNed
				// Maybe this is why test_cpu_exec_space_apu.nes is passing. But this needs further investigation
				if (address >= 0x4020 && address <= 0x40FF)
				{
					RETURN (address >> EIGHT);
				}

				uint32_t modedData = 0;
				uint32_t compareVal = 0;
				FLAG     hasCompare = NO;
				uint32_t index = RESET;

				switch (pNES_instance->NES_state.catridgeInfo.mapper)
				{
				case MAPPER::NROM:
				case MAPPER::CNROM:
				case MAPPER::J87:
				case MAPPER::CPROM:
				{
					if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
						&&
						(!hasCompare || (BYTE)compareVal == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS]))
					{
						RETURN TO_UINT8(modedData);
					}
					else
					{
						if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
						{
							if (pNES_instance->NES_state.catridgeInfo.hasPrgRam == NO)
							{
								RETURN pNES_cpuRegisters->openbus;
							}
						}
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}
					BREAK;
				}
				case MAPPER::MMC1:
				case MAPPER::INES_MAPPER_105:
				{
					auto& mmc1 = pNES_instance->NES_state.catridgeInfo.mmc1;

					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						const auto subMapper = pNES_instance->NES_state.catridgeInfo.subMapper;

						if (pNES_instance->NES_state.catridgeInfo.mmc1.prgRamEnable == YES
							&& subMapper != SUB_MAPPER::SGROM
							&& subMapper != SUB_MAPPER::SFROM
							&& subMapper != SUB_MAPPER::SLROM
							&& subMapper != SUB_MAPPER::SBROM
							&& subMapper != SUB_MAPPER::SHROM
							&& subMapper != SUB_MAPPER::SCROM)
						{
							// Compute bank offset: SOROM uses bit 0 (2 banks), SXROM uses bits 0-1 (4 banks)
							uint32_t ramBank = pNES_instance->NES_state.catridgeInfo.mmc1.surom_sxrom.prgRamBank8_sxrom;

							// Mask bank number according to submapper
							if (subMapper == SUB_MAPPER::SOROM)
							{
								ramBank &= 0x01; // 16 KB total (2 x 8 KB banks)
							}
							else if (subMapper != SUB_MAPPER::SXROM)
							{
								ramBank = 0;     // Standard MMC1 / SUROM / SNROM (fixed 8 KB bank)
							}
							// SXROM uses all 2 bits (ramBank & 0x03)

							const uint32_t ramOffset = (ramBank * 0x2000) + (address - UNMAPPED_START_ADDRESS);
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[ramOffset];
						}
						else
						{
							RETURN (address >> EIGHT);
						}
					}
					else if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SEROM_SHROM_SH1ROM)
						{
							// SEROM / SHROM / SH1ROM:
							// Fixed 32KB PRG. MMC1 PRG banking output is not connected.
							index = address - CATRIDGE_ROM_BANK0_START_ADDRESS;
						}
						else
						{
							switch (mmc1.intfControlReg.fields1.pp)
							{
							case ZERO:
							case ONE:
							{
								// NOTE: "prgBank32 * 0x4000" is used instead of "prgBank32 * 0x8000" as bank IDs are based on 16KB mode even when 32KB mode is selected
								index = (mmc1.prgBank32 | mmc1.surom_sxrom.prgBank256) * 0x4000;
								index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x7FFF);
								BREAK;
							}
							case TWO:
							case THREE:
							{
								if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
								{
									index = (mmc1.prgBank16Lo | mmc1.surom_sxrom.prgBank256) * 0x4000;
									index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x3FFF);
								}
								else if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK1_START_ADDRESS, CATRIDGE_ROM_BANK1_END_ADDRESS))
								{
									index = (mmc1.prgBank16Hi | mmc1.surom_sxrom.prgBank256) * 0x4000;
									index += ((address - CATRIDGE_ROM_BANK1_START_ADDRESS) & 0x3FFF);
								}
								else
								{
									FATAL("Invalid Memory Region for MMC1");
								}
								BREAK;
							}
							}

							// MMC1A A17-bypass: if bit 4 of $E000 is set, bit 3 directly drives A17 across all $8000-$FFFF
							if (mmc1.isMmc1A == YES && (mmc1.prgBank16Lo & 0x10) /* bit 4 of last $E000 write */)
							{
								// bit 3 of prgBank16Lo/Hi forces the upper 256KB half
								// prgBank256 equivalent: (prgBank16Lo >> 3) & 1, applied to both banks
								const uint32_t a17 = (mmc1.prgBank16Lo & 0x08) ? 0x10 : 0x00; // drives A17
								// index already computed above — add the outer-bank offset
								index += (a17 * 0x4000);
							}

							if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_105)
							{
								auto& mmc1 = pNES_instance->NES_state.catridgeInfo.mmc1;
								auto& ev = mmc1.nes_event;

								const bool oBit = (mmc1.chrBank4Lo & 0x08) != 0; // bit 3 = O (chip select)
								const BYTE aReg = (mmc1.chrBank4Lo & 0x06) >> 1;  // bits 2:1 = AA (32KB bank in lower 128KB)
								const bool pMode = (mmc1.intfControlReg.fields1.pp >= 2); // MMC1 P bit (pp mode 2 or 3)
								const bool slotSel = (mmc1.intfControlReg.fields1.pp == 3); // S: pp==3 means slot 0 is swappable

								const uint32_t offset = address - CATRIDGE_ROM_BANK0_START_ADDRESS;

								if (ev.initState < 2 || !oBit)
								{
									// Uninitialized OR O=0: 32KB from lower 128KB, bank = aReg (0 if uninitialized)
									const BYTE bank32 = (ev.initState < 2) ? 0 : aReg;
									index = (bank32 * 0x8000) + (offset & 0x7FFF);
								}
								else
								{
									// O=1: upper 128KB (add 0x20000 base), MMC1-style banking using $E000 reg
									const uint32_t upperBase = 0x20000U; // 128KB offset
									const BYTE bReg = mmc1.prgBank16Lo & 0x07; // lower 3 bits of $E000
									const BYTE prgReg = bReg | 0x08;           // always within upper 128KB (banks 8..15)

									if (!pMode) // pp = 0 or 1: 32KB swap
									{
										index = upperBase + ((prgReg & 0xFE) * 0x4000) + (offset & 0x7FFF);
									}
									else if (!slotSel) // pp = 2: fix bank 8 at $8000, swap at $C000
									{
										if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
										{
											index = upperBase + (0x08 * 0x4000) + (offset & 0x3FFF);
										}
										else
										{
											index = upperBase + (prgReg * 0x4000) + (offset & 0x3FFF);
										}
									}
									else // pp = 3: swap at $8000, fix bank 0x0F at $C000
									{
										if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
										{
											index = upperBase + (prgReg * 0x4000) + (offset & 0x3FFF);
										}
										else
										{
											index = upperBase + (0x0F * 0x4000) + (offset & 0x3FFF);
										}
									}
								}
							}
						}
						
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
				case MAPPER::INES_MAPPER_180:
				{
					const auto readProgrammableBank = [&](uint16_t address) -> BYTE
						{
							uint32_t index = pNES_instance->NES_state.catridgeInfo.uxrom_002.prgBank16 * 0x4000;
							index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x3FFF);

							if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
								&&
								(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
							{
								RETURN TO_UINT8(modedData);
							}
							else
							{
								RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
							}
						};

					const auto readFixedBank = [&](uint16_t address) -> BYTE
						{
							const uint32_t mappedIndex = address - UNMAPPED_START_ADDRESS;

							if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
								&&
								(!hasCompare || (BYTE)compareVal == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[mappedIndex]))
							{
								RETURN TO_UINT8(modedData);
							}
							else
							{
								RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[mappedIndex];
							}
						};

					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
						else
						{
							RETURN pNES_cpuRegisters->openbus;
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
					{
						RETURN (pNES_instance->NES_state.catridgeInfo.mapper == INES_MAPPER_180) ? readFixedBank(address) : readProgrammableBank(address);
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK1_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						RETURN (pNES_instance->NES_state.catridgeInfo.mapper == INES_MAPPER_180) ? readProgrammableBank(address) : readFixedBank(address);
					}
					BREAK;
				}
				case MAPPER::MMC3:
				case MAPPER::INES_MAPPER_037:
				case MAPPER::INES_MAPPER_047:
				case MAPPER::INES_MAPPER_118:
				case MAPPER::INES_MAPPER_119:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::MMC6
							&& pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.mm6PrgRamEnable == SET
							&& pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							if (address >= 0x7000)
							{
								const uint16_t ramAddr = (address - UNMAPPED_START_ADDRESS) & 0x03FF; // 1KB PRG-RAM bank mirroring

								const bool upperHalf = (ramAddr & 0x0200) != ZERO;

								const bool enR01FF = (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.mmc6Fields.enR01FF == SET);
								const bool enR23FF = (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.mmc6Fields.enR23FF == SET);

								if (upperHalf ? enR23FF : enR01FF)
								{
									RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[ramAddr];
								}

								// If any one of them is enabled, the disabled half returns 0 instead of open bus.
								if (enR01FF != enR23FF)
								{
									RETURN ZERO;
								}
							}
						}
						else if (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.fields.prgRamEnable == SET 
							&& pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							if (pNES_instance->NES_state.catridgeInfo.mapper != MAPPER::INES_MAPPER_047)
							{
								RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
							}
						}
						RETURN pNES_cpuRegisters->openbus;
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
							const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
							const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
							const uint32_t totalPrg16kBanks = isNES2
								? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
								: hdr.sizeOfPrgRomIn16KB;

							// totalPrg16kBanks - ONE is the start of the last 16KB bank,
							// which is also the start of the second-to-last 8KB bank
							index = (totalPrg16kBanks - ONE) * 0x4000;
							index += ((address - startAddr2) & 0x1FFF);
						}
						else if (IF_ADDRESS_WITHIN(address, 0xE000, 0xFFFF))
						{
							const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
							const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
							const uint32_t totalPrg16kBanks = isNES2
								? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
								: hdr.sizeOfPrgRomIn16KB;

							// Add 0x2000 past the second-to-last bank to get the last 8KB bank
							index = (totalPrg16kBanks - ONE) * 0x4000;
							index += 0x2000;
							index += ((address - 0xE000) & 0x1FFF);
						}
						else
						{
							FATAL("Invalid PRG ROM address in MMC3");
						}

						// NOTE: Mapper 037 outer bank PRG transform
						// Refer https://www.nesdev.org/wiki/INES_Mapper_037
						if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_037)
						{
							const uint8_t outerBank = pNES_instance->NES_state.catridgeInfo.mmc3.ines037.outerBank;
							const uint8_t BB = outerBank & 0x03;
							const uint8_t Q = (outerBank >> 2) & 0x01;
							const uint32_t A17 = (uint32_t)Q << 17;
							const uint32_t A16 = (BB == 3) ? 0x10000u : ((index & 0x10000u) & ((uint32_t)Q << 16));
							index = A17 | A16 | (index & 0xFFFF);
						}
						else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_047)
						{
							// NOTE: Mapper 047 PRG transform — single bit B selects 128KB PRG block.
							// Refer https://www.nesdev.org/wiki/INES_Mapper_047
							const uint32_t prgBase = (uint32_t)(pNES_instance->NES_state.catridgeInfo.mmc3.ines047.multicart & 0x01) << 17;
							index = prgBase | (index & 0x1FFFF);
						}

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
				case MAPPER::INES_MAPPER_268:
				{
					const BYTE submapperRaw = (BYTE)pNES_instance->NES_state.catridgeInfo.subMapper;
					const bool isMindkids = (submapperRaw & 0x01) != 0;

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						const BYTE a001Raw = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.raw;
						if (mapper268WramReadable(a001Raw))
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
						RETURN pNES_cpuRegisters->openbus;
					}

					if (isMindkids && IF_ADDRESS_WITHIN(address, 0x5000, 0x5FFF))
					{
						RETURN pNES_cpuRegisters->openbus; // write-only register window
					}

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						const BIT prgRomBankMode = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.prgRomMode;

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

						uint16_t cpuWindowAddr = ZERO;
						BYTE nativeV = ZERO;
						uint32_t pageBase = ZERO;

						if (IF_ADDRESS_WITHIN(address, startAddr1, endAddr1))
						{
							nativeV = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8a;
							pageBase = startAddr1; cpuWindowAddr = 0x8000;
						}
						else if (IF_ADDRESS_WITHIN(address, 0xA000, 0xBFFF))
						{
							nativeV = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8b;
							pageBase = 0xA000; cpuWindowAddr = 0xA000;
						}
						else if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
						{
							nativeV = 0xFE;
							pageBase = startAddr2; cpuWindowAddr = 0xC000;
						}
						else if (IF_ADDRESS_WITHIN(address, 0xE000, 0xFFFF))
						{
							nativeV = 0xFF;
							pageBase = 0xE000; cpuWindowAddr = 0xE000;
						}
						else
						{
							FATAL("Invalid PRG ROM address in Mapper 268");
						}

						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;
						const uint64_t totalPrgBytes = (uint64_t)totalPrg16kBanks * 0x4000ULL;
						const bool supports64MiB = (totalPrgBytes > 0x2000000ULL);

						const auto& outer = pNES_instance->NES_state.catridgeInfo.mmc3.ines268;
						uint32_t page = mapper268ComputePrgPage(nativeV, cpuWindowAddr, outer.reg, submapperRaw, supports64MiB);
						uint32_t index = (page * 0x2000u) + ((address - pageBase) & 0x1FFF);

						if (totalPrgBytes > ZERO)
						{
							index = (uint32_t)(index % totalPrgBytes);
						}

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
					}
					BREAK;
				}
				case MAPPER::RAMBO1:
				case MAPPER::INES_MAPPER_158:
				{
					// RAMBO-1 has no PRG-RAM at $6000-$7FFF; return open bus.
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuRegisters->openbus;
					}

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						const auto& rb = pNES_instance->NES_state.catridgeInfo.mmc3.rambo1;
						const BYTE curReg = rb.currentRegister;
						const BIT prgMode = (curReg >> 6) & 1; // bit 6

						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;
						const uint32_t totalPrg8kBanks = totalPrg16kBanks * 2;

						uint32_t prgIndex = RESET;

						// PRG map:
						//   prgMode=0: $8000=R6, $A000=R7, $C000=RF, $E000=last
						//   prgMode=1: $8000=RF, $A000=R7, $C000=R6, $E000=last
						if (IF_ADDRESS_WITHIN(address, 0x8000, 0x9FFF))
						{
							const BYTE bankReg = prgMode ? 15 : 6;
							prgIndex = (rb.reg[bankReg] % totalPrg8kBanks) * 0x2000;
							prgIndex += (address - 0x8000) & 0x1FFF;
						}
						else if (IF_ADDRESS_WITHIN(address, 0xA000, 0xBFFF))
						{
							prgIndex = (rb.reg[7] % totalPrg8kBanks) * 0x2000;
							prgIndex += (address - 0xA000) & 0x1FFF;
						}
						else if (IF_ADDRESS_WITHIN(address, 0xC000, 0xDFFF))
						{
							const BYTE bankReg = prgMode ? 6 : 15;
							prgIndex = (rb.reg[bankReg] % totalPrg8kBanks) * 0x2000;
							prgIndex += (address - 0xC000) & 0x1FFF;
						}
						else // $E000-$FFFF : fixed to last 8KB bank
						{
							prgIndex = (totalPrg8kBanks - 1) * 0x2000;
							prgIndex += (address - 0xE000) & 0x1FFF;
						}

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[prgIndex]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[prgIndex];
					}
					BREAK;
				}
				case MAPPER::MMC5:
				{
					auto& mmc5 = pNES_instance->NES_state.catridgeInfo.mmc5;

					// $5000-$5FFF: MMC5 internal registers and ExRAM
					if (IF_ADDRESS_WITHIN(address, 0x5000, 0x5FFF))
					{
						if (IF_ADDRESS_WITHIN(address, 0x5C00, 0x5FFF))
						{
							// ExRAM reads
							// Mode 0/1: write-only from CPU perspective — return open bus
							// Mode 2/3: readable
							if (mmc5.extendedRamMode >= 2)
							{
								RETURN mmc5.exRam[address - 0x5C00];
							}
							RETURN pNES_cpuRegisters->openbus;
						}

						switch (address)
						{
						case 0x5010:
						{
							uint8_t val = 0;
							if (mmc5.mmc5Audio.pcmIrqPending == YES)
							{
								val |= 0x80;
								mmc5.mmc5Audio.pcmIrqPending = NO;
							}
							RETURN val;
						}
						case 0x5015:
						{
							uint8_t val = 0;
							if (mmc5.mmc5Audio.pulse[0].lengthCounter > 0) val |= 0x01;
							if (mmc5.mmc5Audio.pulse[1].lengthCounter > 0) val |= 0x02;
							RETURN val;
						}
						case 0x5204:
						{
							// IRQ status: bit 7 = pending, bit 6 = in-frame
							// Reading clears the pending flag and IRQ source
							const BYTE val = (BYTE)((mmc5.irqPending ? 0x80 : 0x00) | (mmc5.ppuInFrame ? 0x40 : 0x00));
							mmc5.irqPending = NO;
							pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC5 = RESET;
							RETURN val;
						}
						case 0x5205:
							RETURN (BYTE)((mmc5.multiplier1 * mmc5.multiplier2) & 0xFF);
						case 0x5206:
							RETURN (BYTE)((mmc5.multiplier1 * mmc5.multiplier2) >> 8);
						case 0xFFFA:
						case 0xFFFB:
							mmc5.ppuInFrame = NO;
							updateMMC5ChrA();
							mmc5.lastPpuReadAddr = RESET;
							mmc5.scanlineCounter = RESET;
							mmc5.irqPending = NO;
							pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC5 = RESET;
							BREAK;
						default:
							RETURN pNES_cpuRegisters->openbus;
						}
					}

					// $6000-$7FFF: PRG-RAM via $5113 (always RAM regardless of bit 7)
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						const uint32_t ramBank = mmc5.prgBanks[0] & 0x07;
						const uint32_t idx = ramBank * 0x2000 + (address - CATRIDGE_RAM_START_ADDRESS);
						RETURN mmc5.prgRam[idx & 0xFFFF];
					}

					// $8000-$FFFF: PRG-ROM or PRG-RAM depending on prgMode and bank register bit 7
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;
						const uint32_t totalPrg8kBanks = totalPrg16kBanks * 2;

						// NMI vector reads ($FFFA/$FFFB) must clear in-frame state
						// Refer https://www.nesdev.org/wiki/MMC5#Scanline_Detection_and_Counting
						if (address == 0xFFFA || address == 0xFFFB)
						{
							mmc5.ppuInFrame = NO;
							mmc5.lastPpuReadAddr = 0;
							mmc5.scanlineCounter = 0;
							mmc5.irqPending = NO;
							pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC5 = RESET;
						}

						uint32_t index = 0;
						bool     isRAM = false;

						// PRG banking modes — compute ROM index or RAM bank+offset
						// Refer https://www.nesdev.org/wiki/MMC5#PRG_Registers
						const uint32_t prgRomSize = totalPrg8kBanks * 0x2000;

						switch (mmc5.prgMode)
						{
						case 0: // 32KB ROM at $8000-$FFFF via $5117
						{
							// $5117: 32KB-aligned bank
							const uint32_t bankBase = (uint32_t)(mmc5.prgBanks[4] & 0x7C) * 0x2000;
							index = (bankBase + (address - 0x8000)) % prgRomSize;
							BREAK;
						}
						case 1: // 16KB + 16KB
						{
							if (address <= 0xBFFF)
							{
								// $8000-$BFFF: $5115, 16KB-aligned
								if ((mmc5.prgBanks[2] & 0x80) == 0)
								{
									isRAM = true;
									index = (uint32_t)(mmc5.prgBanks[2] & 0x06) * 0x2000 + (address - 0x8000);
								}
								else
								{
									const uint32_t bankBase = (uint32_t)(mmc5.prgBanks[2] & 0x7E) * 0x2000;
									index = (bankBase + (address - 0x8000)) % prgRomSize;
								}
							}
							else
							{
								// $C000-$FFFF: $5117, always ROM, 16KB-aligned
								const uint32_t bankBase = (uint32_t)(mmc5.prgBanks[4] & 0x7E) * 0x2000;
								index = (bankBase + (address - 0xC000)) % prgRomSize;
							}
							BREAK;
						}
						case 2: // 16KB + 8KB + 8KB
						{
							if (address <= 0xBFFF)
							{
								// $8000-$BFFF: $5115, 16KB-aligned
								if ((mmc5.prgBanks[2] & 0x80) == 0)
								{
									isRAM = true;
									index = (uint32_t)(mmc5.prgBanks[2] & 0x06) * 0x2000 + (address - 0x8000);
								}
								else
								{
									const uint32_t bankBase = (uint32_t)(mmc5.prgBanks[2] & 0x7E) * 0x2000;
									index = (bankBase + (address - 0x8000)) % prgRomSize;
								}
							}
							else if (address <= 0xDFFF)
							{
								// $C000-$DFFF: $5116, 8KB
								if ((mmc5.prgBanks[3] & 0x80) == 0)
								{
									isRAM = true;
									index = (uint32_t)(mmc5.prgBanks[3] & 0x07) * 0x2000 + (address - 0xC000);
								}
								else
								{
									const uint32_t bankBase = (uint32_t)(mmc5.prgBanks[3] & 0x7F) * 0x2000;
									index = (bankBase + (address - 0xC000)) % prgRomSize;
								}
							}
							else
							{
								// $E000-$FFFF: $5117, always ROM, 8KB
								const uint32_t bankBase = (uint32_t)(mmc5.prgBanks[4] & 0x7F) * 0x2000;
								index = (bankBase + (address - 0xE000)) % prgRomSize;
							}

							BREAK;
						}

						case 3: // 8KB + 8KB + 8KB + 8KB
						{
							if (address <= 0x9FFF)
							{
								// $8000-$9FFF: $5114
								if ((mmc5.prgBanks[1] & 0x80) == 0)
								{
									isRAM = true;
									index = (uint32_t)(mmc5.prgBanks[1] & 0x07) * 0x2000 + (address - 0x8000);
								}
								else
								{
									const uint32_t bankBase = (uint32_t)(mmc5.prgBanks[1] & 0x7F) * 0x2000;
									index = (bankBase + (address - 0x8000)) % prgRomSize;
								}
							}
							else if (address <= 0xBFFF)
							{
								// $A000-$BFFF: $5115
								if ((mmc5.prgBanks[2] & 0x80) == 0)
								{
									isRAM = true;
									index = (uint32_t)(mmc5.prgBanks[2] & 0x07) * 0x2000 + (address - 0xA000);
								}
								else
								{
									const uint32_t bankBase =(uint32_t)(mmc5.prgBanks[2] & 0x7F) * 0x2000;
									index = (bankBase + (address - 0xA000)) % prgRomSize;
								}
							}
							else if (address <= 0xDFFF)
							{
								// $C000-$DFFF: $5116
								if ((mmc5.prgBanks[3] & 0x80) == 0)
								{
									isRAM = true;
									index = (uint32_t)(mmc5.prgBanks[3] & 0x07) * 0x2000 + (address - 0xC000);
								}
								else
								{
									const uint32_t bankBase = (uint32_t)(mmc5.prgBanks[3] & 0x7F) * 0x2000;
									index = (bankBase + (address - 0xC000)) % prgRomSize;
								}
							}
							else
							{
								// $E000-$FFFF: $5117, always ROM
								const uint32_t bankBase = (uint32_t)(mmc5.prgBanks[4] & 0x7F) * 0x2000;
								index = (bankBase + (address - 0xE000)) % prgRomSize;
							}
							BREAK;
						}

						default:
						{
							FATAL("Invalid MMC5 PRG mode");
							BREAK;
						}
						}

						if (isRAM)
						{
							RETURN mmc5.prgRam[index & 0xFFFF];
						}

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
					}
					BREAK;
				}
				case MAPPER::MMC2:
				case MAPPER::MMC4:
				{
					const FLAG isMMC4 = (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC4);

					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}

					const uint32_t switchableBankEndAddress = (isMMC4) ? CATRIDGE_ROM_BANK0_END_ADDRESS : (CATRIDGE_ROM_BANK0_START_ADDRESS + 0x1FFF);
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, switchableBankEndAddress))
					{
						const uint32_t bank = (isMMC4) ? pNES_instance->NES_state.catridgeInfo.mmc4.prgBank16 : pNES_instance->NES_state.catridgeInfo.mmc2.prgBank;

						const uint32_t bankShift = (isMMC4) ? 14 : 13;
						const uint32_t addressMask = (isMMC4) ? 0x3FFF : 0x1FFF;

						index = (bank << bankShift) | (address & addressMask);

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
					}

					const uint32_t fixedBankStartAddress = (isMMC4) ? CATRIDGE_ROM_BANK1_START_ADDRESS : (CATRIDGE_ROM_BANK0_START_ADDRESS + 0x2000);
					if (IF_ADDRESS_WITHIN(address, fixedBankStartAddress, UNMAPPED_END_ADDRESS))
					{
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS]))
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
				case MAPPER::AxROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
						else
						{
							RETURN pNES_cpuRegisters->openbus;
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						index = pNES_instance->NES_state.catridgeInfo.axrom.prgBank * 0x8000;
						index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x7FFF);
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
				case MAPPER::INES_MAPPER_218:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						index = address - CATRIDGE_ROM_BANK0_START_ADDRESS;
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
				case MAPPER::COLOR_DREAMS:
				{
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						index = pNES_instance->NES_state.catridgeInfo.colorDreams.prgBank32 * 0x8000;
						index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x7FFF);
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
				case MAPPER::INES_MAPPER_014:
				{
					auto& reg014 = pNES_instance->NES_state.catridgeInfo.ines014;
					auto& vrc24 = pNES_instance->NES_state.catridgeInfo.vrc24;
					const bool isMMC3Mode = (reg014.supervisorReg & 0x10) != ZERO;

					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (isMMC3Mode)
						{
							if (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.fields.prgRamEnable == SET)
							{
								RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
							}
							RETURN pNES_cpuRegisters->openbus;
						}

						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						FLAG hasPrgRam = NO;
						if (isNES2)
						{
							hasPrgRam = (hdr.flags_8to15.nes2p0.flag10.fields.prgVolRam > ZERO)
								|| (hdr.flags_8to15.nes2p0.flag10.fields.prgNonVolRam > ZERO);
						}
						else
						{
							hasPrgRam = (hdr.flags_8to15.ines.flag10.fields.prgRamNotPresent != ONE);
						}

						if (!hasPrgRam && IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, 0x6FFF))
						{
							RETURN(vrc24.latch & 0x01) | (pNES_cpuRegisters->openbus & 0xFE);
						}
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << EIGHT))
							: hdr.sizeOfPrgRomIn16KB;
						const uint32_t totalPrg8kBanks = totalPrg16kBanks << ONE;

						uint32_t index = ZERO;

						if (isMMC3Mode)
						{
							const BIT prgRomBankMode = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.prgRomMode;

							auto startAddr1 = CATRIDGE_ROM_BANK0_START_ADDRESS, endAddr1 = startAddr1 + 0x1FFF;
							auto startAddr2 = CATRIDGE_ROM_BANK1_START_ADDRESS, endAddr2 = startAddr2 + 0x1FFF;
							if (prgRomBankMode == SET)
							{
								startAddr1 = CATRIDGE_ROM_BANK1_START_ADDRESS; endAddr1 = startAddr1 + 0x1FFF;
								startAddr2 = CATRIDGE_ROM_BANK0_START_ADDRESS; endAddr2 = startAddr2 + 0x1FFF;
							}

							uint32_t prgBank8k = ZERO;
							uint32_t pageBase = ZERO;

							if (IF_ADDRESS_WITHIN(address, startAddr1, endAddr1))
							{
								prgBank8k = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8a % totalPrg8kBanks;
								pageBase = startAddr1;
							}
							else if (IF_ADDRESS_WITHIN(address, 0xA000, 0xBFFF))
							{
								prgBank8k = pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8b % totalPrg8kBanks;
								pageBase = 0xA000;
							}
							else if (IF_ADDRESS_WITHIN(address, startAddr2, endAddr2))
							{
								prgBank8k = totalPrg8kBanks - TWO;
								pageBase = startAddr2;
							}
							else
							{
								prgBank8k = totalPrg8kBanks - ONE;
								pageBase = 0xE000;
							}

							index = (prgBank8k * 0x2000u) + ((address - pageBase) & 0x1FFFu);
						}
						else // VRC2 mode -- identical to your existing VRC2_022/etc. PRG read path
						{
							uint32_t prgBank = ZERO;
							if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, 0x9FFF))
							{
								prgBank = vrc24.prgBank0 % totalPrg8kBanks;
							}
							else if (IF_ADDRESS_WITHIN(address, 0xA000, CATRIDGE_ROM_BANK0_END_ADDRESS))
							{
								prgBank = vrc24.prgBank1 % totalPrg8kBanks;
							}
							else if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK1_START_ADDRESS, 0xDFFF))
							{
								prgBank = totalPrg8kBanks - TWO;
							}
							else
							{
								prgBank = totalPrg8kBanks - ONE;
							}
							index = (prgBank * 0x2000u) + (address & 0x1FFFu);
						}

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_015:
				{
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						// Real hardware has no PRG-RAM here -- see note above on why we
						// provide it unconditionally anyway (mapper-hack compatibility).
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						const auto& reg = pNES_instance->NES_state.catridgeInfo.ines015;
						const BYTE slot = (BYTE)((address - CATRIDGE_ROM_BANK0_START_ADDRESS) >> 13); // 0-3

						uint32_t index = (mapper015ComputePrgBank8k(reg.latchedData, reg.latchedAddr, slot) * 0x2000u) + (address & 0x1FFFu);

						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const uint64_t totalPrgBytes = (uint64_t)hdr.sizeOfPrgRomIn16KB * 0x4000ULL;
						if (totalPrgBytes > ZERO)
						{
							index = (uint32_t)(index % totalPrgBytes);
						}

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_016:
				{
					const auto submapper = pNES_instance->NES_state.catridgeInfo.subMapper;
					auto& m016 = pNES_instance->NES_state.catridgeInfo.ines016;

					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						// 1. Serial EEPROM Read Port Interception ($6000-$7FFF)
						// Only Submapper 5 (LZ93D50 with serial EEPROM) returns bits shifted out via its buffer
						if (submapper == SUB_MAPPER::BANDAI_LZ93D50_24C01 || submapper == SUB_MAPPER::BANDAI_LZ93D50_24C02)
						{
							if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
							{
								// Shifting out bit 4 from data outBuffer, combining with open bus baseline logic
								BYTE eepromBit = (m016.outBuffer & 0x01) << 4;

								// Replicates Mesen's: return output | GetOpenBus(0xE7);
								RETURN (eepromBit | (pNES_cpuRegisters->openbus & 0xE7));
							}
						}

						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}

					// 2. PRG-ROM Translation Pipeline ($8000-$FFFF)
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						// Window 0: $8000-$BFFF (Swappable bank)
						if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
						{
							const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
							const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
							const uint32_t totalPrg16kBanks = isNES2
								? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
								: hdr.sizeOfPrgRomIn16KB;

							uint32_t targetBank = m016.prgBank;
							targetBank %= totalPrg16kBanks; // Mask index against total page counts

							uint32_t index = targetBank * 0x4000;
							index += (address - CATRIDGE_ROM_BANK0_START_ADDRESS);

							if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
								&&
								(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
							{
								RETURN TO_UINT8(modedData);
							}
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
						// Window 1: $C000-$FFFF (Mesen Alignment: 0x0F | _prgBankSelect)
						else if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK1_START_ADDRESS, UNMAPPED_END_ADDRESS))
						{
							if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
								&&
								(!hasCompare || (BYTE)compareVal == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS]))
							{
								RETURN TO_UINT8(modedData);
							}
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_018:
				{
					auto& j18 = pNES_instance->NES_state.catridgeInfo.jaleco18;
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const FLAG isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

					const uint32_t totalPrg16kBanks =
						isNES2
						? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << EIGHT))
						: hdr.sizeOfPrgRomIn16KB;
					const uint32_t totalPrg8kBanks = totalPrg16kBanks << ONE;

					// WRAM $6000-$7FFF — always accessible if present, no runtime enable gate
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						FLAG hasPrgRam = NO;
						if (isNES2)
						{
							const uint8_t vs = hdr.flags_8to15.nes2p0.flag10.fields.prgVolRam;
							const uint8_t nv = hdr.flags_8to15.nes2p0.flag10.fields.prgNonVolRam;
							hasPrgRam = (vs > ZERO) || (nv > ZERO);
						}
						else
						{
							hasPrgRam = !(hdr.flags_8to15.ines.flag10.fields.prgRamNotPresent == ONE);
						}

						if (hasPrgRam && IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
						RETURN pNES_cpuRegisters->openbus;
					}

					// PRG ROM $8000-$FFFF
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						uint32_t prgBank = ZERO;
						uint16_t bankOffset = ZERO;

						if (address < 0xA000)       // $8000-$9FFF — prgBank[0]
						{
							prgBank = j18.prgBank[0] % totalPrg8kBanks;
							bankOffset = address - 0x8000;
						}
						else if (address < 0xC000)  // $A000-$BFFF — prgBank[1]
						{
							prgBank = j18.prgBank[1] % totalPrg8kBanks;
							bankOffset = address - 0xA000;
						}
						else if (address < 0xE000)  // $C000-$DFFF — prgBank[2]
						{
							prgBank = j18.prgBank[2] % totalPrg8kBanks;
							bankOffset = address - 0xC000;
						}
						else                        // $E000-$FFFF — fixed last bank
						{
							prgBank = (totalPrg8kBanks - ONE);
							bankOffset = address - 0xE000;
						}

						const uint32_t romIndex = (prgBank * 0x2000) + bankOffset;

						if ((ceNES->interceptCPURead(
							CheatEngine_t::CHEATING_ENGINE::GAMEGENIE,
							address, &modedData, &compareVal, &hasCompare))
							&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[romIndex]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[romIndex];
					}

					BREAK;
				}
				case MAPPER::VRC2_022:
				case MAPPER::VRC4_021:
				case MAPPER::VRC2_VRC4_023:
				case MAPPER::VRC2_VRC4_025:
				{
					auto& vrc24 = pNES_instance->NES_state.catridgeInfo.vrc24;
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const FLAG isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

					const uint32_t totalPrg16kBanks =
						isNES2
						? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << EIGHT))
						: hdr.sizeOfPrgRomIn16KB;

					const uint32_t totalPrg8kBanks = totalPrg16kBanks << ONE;

					// Calculate PRG RAM presence using your exact union layout fields
					FLAG hasPrgRam = false;
					if (isNES2)
					{
						const uint8_t volatileShift = hdr.flags_8to15.nes2p0.flag10.fields.prgVolRam;
						const uint8_t nvShift = hdr.flags_8to15.nes2p0.flag10.fields.prgNonVolRam;
						hasPrgRam = (volatileShift > ZERO) || (nvShift > ZERO);
					}
					else
					{
						const FLAG explicitlyNotPresent = (hdr.flags_8to15.ines.flag10.fields.prgRamNotPresent == ONE);
						hasPrgRam = !explicitlyNotPresent;
					}

					// -------------------------------------------------
					// WRAM / VRC2 Latch
					// -------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (isVRC2() == YES)
						{
							if (!hasPrgRam && IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, 0x6FFF))
							{
								RETURN (vrc24.latch & 0x01) | (pNES_cpuRegisters->openbus & 0xFE);
							}
							else
							{
								RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
							}
						}
						else if (isVRC4() == YES)
						{
							if (hasPrgRam && vrc24.wramEnable)
							{
								RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
							}
						}

						RETURN pNES_cpuRegisters->openbus;
					}

					// -------------------------------------------------
					// PRG ROM
					// -------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						uint32_t prgBank = ZERO;

						// -----------------------------------------
						// $8000-$9FFF
						// -----------------------------------------
						if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, 0x9FFF))
						{
							if (vrc24.swapMode == RESET)
							{
								prgBank = (vrc24.prgBank0 % totalPrg8kBanks);
							}
							else
							{
								prgBank = (totalPrg8kBanks - TWO);
							}
						}

						// -----------------------------------------
						// $A000-$BFFF
						// -----------------------------------------
						else if (IF_ADDRESS_WITHIN(address, 0xA000, CATRIDGE_ROM_BANK0_END_ADDRESS))
						{
							prgBank = (vrc24.prgBank1 % totalPrg8kBanks);
						}

						// -----------------------------------------
						// $C000-$DFFF
						// -----------------------------------------
						else if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK1_START_ADDRESS, 0xDFFF))
						{
							if (vrc24.swapMode == RESET)
							{
								prgBank = (totalPrg8kBanks - TWO);
							}
							else
							{
								prgBank = (vrc24.prgBank0 % totalPrg8kBanks);
							}
						}

						// -----------------------------------------
						// $E000-$FFFF
						// -----------------------------------------
						else
						{
							prgBank = (totalPrg8kBanks - ONE);
						}

						const uint32_t index = (prgBank * 0x2000) + (address & 0x1FFF);

						if ((ceNES->interceptCPURead(
							CheatEngine_t::CHEATING_ENGINE::GAMEGENIE,
							address,
							&modedData,
							&compareVal,
							&hasCompare))
							&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
				case MAPPER::VRC6_024:
				case MAPPER::VRC6_026:
				{
					auto& vrc6 = pNES_instance->NES_state.catridgeInfo.vrc6;
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const FLAG isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

					const uint32_t totalPrg16kBanks =
						isNES2
						? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << EIGHT))
						: hdr.sizeOfPrgRomIn16KB;

					const uint32_t totalPrg8kBanks = totalPrg16kBanks << ONE;

					// -------------------------------------------------
					// WRAM (Cartridge RAM) Space: $6000 - $7FFF
					// -------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						FLAG hasPrgRam = false;
						if (isNES2)
						{
							const uint8_t volatileShift = hdr.flags_8to15.nes2p0.flag10.fields.prgVolRam;
							const uint8_t nvShift = hdr.flags_8to15.nes2p0.flag10.fields.prgNonVolRam;
							hasPrgRam = (volatileShift > ZERO) || (nvShift > ZERO);
						}
						else
						{
							const FLAG explicitlyNotPresent = (hdr.flags_8to15.ines.flag10.fields.prgRamNotPresent == ONE);
							hasPrgRam = !explicitlyNotPresent;
						}

						// Bit 7 of $B003 is the runtime WRAM enable gate (NESdev VRC6 wiki)
						const FLAG wramEnabled = (vrc6.b003_reg & 0x80) != ZERO;
						if (hasPrgRam && wramEnabled
							&& IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
						RETURN pNES_cpuRegisters->openbus;
					}

					// -------------------------------------------------
					// PRG ROM Space: $8000 - $FFFF
					// -------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						// -------------------------------------------------------------
						// DYNAMICALLY PROGRAMMED BANKS BLOCK WINDOW: $8000-$DFFF
						// -------------------------------------------------------------
						if (address < 0xE000)
						{
							uint32_t prgBank = ZERO;
							uint16_t windowOffset = ZERO;

							if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
							{
								windowOffset = address - CATRIDGE_ROM_BANK0_START_ADDRESS; // Local 16KB index: $0000-$3FFF

								if (windowOffset < 0x2000) // $8000-$9FFF (Lower 8KB)
								{
									prgBank = ((vrc6.prgBank0 << ONE) % totalPrg8kBanks);
								}
								else                       // $A000-$BFFF (Upper 8KB)
								{
									prgBank = (((vrc6.prgBank0 << ONE) | ONE) % totalPrg8kBanks);
								}
							}
							else // Area must be $C000-$DFFF inside CATRIDGE_ROM_BANK1 region
							{
								windowOffset = address - CATRIDGE_ROM_BANK1_START_ADDRESS; // Local 16KB index: $0000-$3FFF
								prgBank = (vrc6.prgBank1 % totalPrg8kBanks);
							}

							const uint32_t dynamicIndex = (prgBank * 0x2000) + (windowOffset & 0x1FFF);

							if ((ceNES->interceptCPURead(
								CheatEngine_t::CHEATING_ENGINE::GAMEGENIE,
								address,
								&modedData,
								&compareVal,
								&hasCompare))
								&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[dynamicIndex]))
							{
								RETURN TO_UINT8(modedData);
							}
							else
							{
								RETURN pNES_catridgeMemory->maxCatridgePRGROM[dynamicIndex];
							}
						}

						// -------------------------------------------------------------
						// FIXED BASE BANK WINDOW: $E000-$FFFF
						// -------------------------------------------------------------
						else
						{
							const uint32_t fixedMappedIndex = address - UNMAPPED_START_ADDRESS;

							if ((ceNES->interceptCPURead(
								CheatEngine_t::CHEATING_ENGINE::GAMEGENIE,
								address,
								&modedData,
								&compareVal,
								&hasCompare))
								&& (!hasCompare || (BYTE)compareVal == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[fixedMappedIndex]))
							{
								RETURN TO_UINT8(modedData);
							}
							else
							{
								RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[fixedMappedIndex];
							}
						}
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_028:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuRegisters->openbus;
					}
					else if (IF_ADDRESS_WITHIN(address, 0x8000, 0xFFFF))
					{
						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;

						const auto& reg028 = pNES_instance->NES_state.catridgeInfo.ines028;
						const bool isC000Window = IF_ADDRESS_WITHIN(address, 0xC000, 0xFFFF);
						const BYTE innerBits4 = reg028.reg01_innerBank & 0x0F;

						uint32_t bank16 = action53ComputePrgBank16k(reg028.reg80_mode, reg028.reg81_outerBank, innerBits4, isC000Window);
						bank16 %= totalPrg16kBanks;
						const uint32_t index = (bank16 * 0x4000u) + (address & 0x3FFFu);

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_029:
				{
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}

					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
					const uint32_t totalPrg16kBanks = isNES2
						? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
						: hdr.sizeOfPrgRomIn16KB;

					uint32_t index = ZERO;

					if (IF_ADDRESS_WITHIN(address, 0x8000, 0xBFFF))
					{
						const uint32_t bank16 = pNES_instance->NES_state.catridgeInfo.ines029.prgBank16 % totalPrg16kBanks;
						index = (bank16 * 0x4000u) + (address & 0x3FFFu);
					}
					else if (IF_ADDRESS_WITHIN(address, 0xC000, 0xFFFF))
					{
						index = ((totalPrg16kBanks - ONE) * 0x4000u) + (address & 0x3FFFu);
					}
					else
					{
						BREAK;
					}

					if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
						&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
					{
						RETURN TO_UINT8(modedData);
					}
					RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
				}
				case MAPPER::INES_MAPPER_030:
				{
					if (IF_ADDRESS_WITHIN(address, 0x8000, 0xFFFF))
					{
						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;

						uint32_t index = ZERO;

						if (IF_ADDRESS_WITHIN(address, 0x8000, 0xBFFF))
						{
							const uint32_t bank16 = pNES_instance->NES_state.catridgeInfo.ines030.prgBank16 % totalPrg16kBanks;
							index = (bank16 * 0x4000u) + (address & 0x3FFFu);
						}
						else // 0xC000-0xFFFF
						{
							index = ((totalPrg16kBanks - ONE) * 0x4000u) + (address & 0x3FFFu);
						}

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
					}

					// $6000-$7FFF and anything else this case doesn't own: mapper 030 has
					// no PRG-RAM ("PRG RAM capacity: None" per the wiki), so this is
					// genuinely unmapped -- open bus, not a crash.
					RETURN pNES_cpuRegisters->openbus;
				}
				case MAPPER::INES_MAPPER_034:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuRegisters->openbus;
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						index = pNES_instance->NES_state.catridgeInfo.ines034.prgBank32 * 0x8000;
						index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x7FFF);
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
						if (pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
						else
						{
							RETURN pNES_cpuRegisters->openbus;
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						index = pNES_instance->NES_state.catridgeInfo.gxrom.prgBank * 0x8000;
						index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x7FFF);
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
				case MAPPER::INES_MAPPER_067:
				case MAPPER::INES_MAPPER_068:
				{
					// --- Cartridge RAM / WRAM Space ($6000 - $7FFF) ---
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (address >= CATRIDGE_RAM_START_ADDRESS && pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_067)
						{
							RETURN pNES_cpuRegisters->openbus;
						}
						if (address >= CATRIDGE_RAM_START_ADDRESS && !pNES_instance->NES_state.catridgeInfo.ines_067_068.prgRamEnable)
						{
							RETURN pNES_cpuRegisters->openbus;
						}
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}

					// --- PRG ROM Bank 0 ($8000 - $BFFF): 16KB Switchable ---
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
					{
						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;

						// ADDED: Rollover safe bound check before index calculation
						uint32_t bank = pNES_instance->NES_state.catridgeInfo.ines_067_068.prgBank % totalPrg16kBanks;
						index = (bank * 0x4000) + (address - CATRIDGE_ROM_BANK0_START_ADDRESS);

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
					}

					// --- PRG ROM Bank 1 ($C000 - $FFFF): Fixed to Last 16KB Bank ---
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK1_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS]))
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
				case MAPPER::INES_MAPPER_069:
				{
					// Cache total banks inline once per execution entry to save execution cycles
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
					const uint32_t totalPrg16kBanks = isNES2
						? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
						: hdr.sizeOfPrgRomIn16KB;
					const uint32_t totalPrg8kBanks = totalPrg16kBanks * 2;

					// --- Cartridge RAM / PRG ROM Space ($6000 - $7FFF) ---
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (address >= CATRIDGE_RAM_START_ADDRESS)
						{
							if (pNES_instance->NES_state.catridgeInfo.ines069.ramMode)
							{
								if (pNES_instance->NES_state.catridgeInfo.ines069.prgRamEnable)
								{
									RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
								}
								else
								{
									RETURN pNES_cpuRegisters->openbus;
								}
							}
							else
							{
								// Unrolled Bank 0 inline read
								uint32_t bank = pNES_instance->NES_state.catridgeInfo.ines069.prgBank[0] % totalPrg8kBanks;
								const uint32_t index = (bank * 0x2000) + (address & 0x1FFF);

								if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
									&&
									(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
								{
									RETURN TO_UINT8(modedData);
								}
								RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
							}
						}
						else
						{
							RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
						}
					}

					// --- PRG ROM Banks ($8000 - $DFFF): 8KB Switchable Windows ---
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, 0xDFFF))
					{
						uint32_t bankId = ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) >> 13) + ONE;
						uint32_t bank = pNES_instance->NES_state.catridgeInfo.ines069.prgBank[bankId] % totalPrg8kBanks;
						const uint32_t index = (bank * 0x2000) + (address & 0x1FFF);

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
					}

					// --- PRG ROM Bank 1 ($E000 - $FFFF): Fixed to Last 8KB Bank ---
					if (IF_ADDRESS_WITHIN(address, 0xE000, UNMAPPED_END_ADDRESS))
					{
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS]))
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
				case MAPPER::INES_MAPPER_070:
				case MAPPER::INES_MAPPER_152:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
					{
						index = pNES_instance->NES_state.catridgeInfo.ines_070_152.prgReg * 0x4000;
						index += ((address - CATRIDGE_ROM_BANK0_START_ADDRESS) & 0x3FFF);
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS]))
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
				case MAPPER::INES_MAPPER_078:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuRegisters->openbus;
					}

					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
					const uint32_t totalPrg16kBanks = isNES2
						? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
						: hdr.sizeOfPrgRomIn16KB;

					uint32_t index = ZERO;

					if (IF_ADDRESS_WITHIN(address, 0x8000, 0xBFFF))
					{
						const uint32_t bank16 = pNES_instance->NES_state.catridgeInfo.ines078.prgBank16 % totalPrg16kBanks;
						index = (bank16 * 0x4000u) + (address & 0x3FFFu);
					}
					else if (IF_ADDRESS_WITHIN(address, 0xC000, 0xFFFF))
					{
						index = ((totalPrg16kBanks - ONE) * 0x4000u) + (address & 0x3FFFu);
					}
					else
					{
						BREAK;
					}

					if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
						&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
					{
						RETURN TO_UINT8(modedData);
					}
					RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
				}
				case MAPPER::NANJING_FC001:
				{
					if (address == 0x5000
						|| IF_ADDRESS_WITHIN(address, 0x5100, 0x5101)
						|| address == 0x5200
						|| address == 0x5300)
					{
						RETURN (address >> EIGHT);
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
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
				case MAPPER::INES_MAPPER_019:
				case MAPPER::INES_MAPPER_210:
				{
					auto& n163 = pNES_instance->NES_state.catridgeInfo.namco163;

					// -----------------------------------------------------------
					// $4800-$4FFF: Audio RAM data port (read + optional auto-increment)
					// Ref: https://www.nesdev.org/wiki/Namco_163_audio#Data_Port
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0x4800, 0x4FFF))
					{
						const BYTE val = n163.audioRam[n163.audioRamAddr & 0x7F];
						if (n163.audioAutoInc == YES)
							n163.audioRamAddr = (n163.audioRamAddr + 1) & 0x7F;
						RETURN val;
					}

					// -----------------------------------------------------------
					// $5000-$57FF: IRQ counter low byte
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#IRQ_Counter_(low)
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0x5000, 0x57FF))
					{
						RETURN(BYTE)(n163.irqCounter & 0xFF);
					}

					// -----------------------------------------------------------
					// $5800-$5FFF: IRQ counter high byte (includes enable bit)
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#IRQ_Counter_(high)
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0x5800, 0x5FFF))
					{
						RETURN(BYTE)(n163.irqCounter >> 8);
					}

					// -----------------------------------------------------------
					// $6000-$7FFF: WRAM
					// Namco163 and Namco175 expose WRAM; Namco340/Unknown = open bus
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (n163.variant == 0 || n163.variant == 1)
						{
							const uint16_t offset = (uint16_t)(address - CATRIDGE_RAM_START_ADDRESS);
							RETURN n163.prgRam[offset & 0x1FFF];
						}
						RETURN pNES_cpuRegisters->openbus;
					}

					// -----------------------------------------------------------
					// $8000-$FFFF: PRG-ROM (3 switchable 8KB banks + fixed last bank)
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#Banks
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool     isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16k = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;
						const uint32_t totalPrg8k = totalPrg16k * 2;

						uint32_t bank = 0;
						uint32_t offset = 0;

						if (IF_ADDRESS_WITHIN(address, 0x8000, 0x9FFF))
						{
							bank = n163.prgBanks[0] & 0x3F;
							offset = address - 0x8000;
						}
						else if (IF_ADDRESS_WITHIN(address, 0xA000, 0xBFFF))
						{
							bank = n163.prgBanks[1] & 0x3F;
							offset = address - 0xA000;
						}
						else if (IF_ADDRESS_WITHIN(address, 0xC000, 0xDFFF))
						{
							bank = n163.prgBanks[2] & 0x3F;
							offset = address - 0xC000;
						}
						else // $E000-$FFFF: fixed last bank
						{
							bank = (totalPrg8k > 0) ? (totalPrg8k - 1) : 0;
							offset = address - 0xE000;
						}

						const uint32_t prgIndex = ((uint32_t)bank % (totalPrg8k > 0 ? totalPrg8k : 1)) * 0x2000 + offset;

						// Game Genie intercept — same pattern as MMC5
						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE,
							address, &modedData, &compareVal, &hasCompare))
							&& (!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[prgIndex]))
						{
							RETURN TO_UINT8(modedData);
						}
						RETURN pNES_catridgeMemory->maxCatridgePRGROM[prgIndex];
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_232:
				{
					// --- Cartridge RAM / WRAM Space ($6000 - $7FFF) ---
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						RETURN pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS];
					}

					// --- PRG ROM Bank 0 ($8000 - $BFFF): 16KB Switchable ---
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, CATRIDGE_ROM_BANK0_END_ADDRESS))
					{
						index = (pNES_instance->NES_state.catridgeInfo.ines232.prgBank8000 * 0x4000) + (address - CATRIDGE_ROM_BANK0_START_ADDRESS);

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pNES_catridgeMemory->maxCatridgePRGROM[index];
						}
					}

					// --- PRG ROM Bank 1 ($C000 - $FFFF): 16KB Switchable ---
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK1_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						index = (pNES_instance->NES_state.catridgeInfo.ines232.prgBankC000 * 0x4000) + (address - CATRIDGE_ROM_BANK1_START_ADDRESS);

						if ((ceNES->interceptCPURead(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, address, &modedData, &compareVal, &hasCompare))
							&&
							(!hasCompare || (BYTE)compareVal == pNES_catridgeMemory->maxCatridgePRGROM[index]))
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
		RETURN pNES_cpuRegisters->openbus;
	}
}

byte NES_t::readCpuRawMemory(uint16_t address, MEMORY_ACCESS_SOURCE source)
{
	pNES_cpuRegisters->openbus = readCpuRawMemoryInternal(address, source);
	RETURN pNES_cpuRegisters->openbus;
}

inline void NES_t::writeCpuRawMemoryInternal(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source)
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

#if (ENABLE_R2A03_SST == YES)
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
#endif
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

				// Update this in CPU's perspective (except for PPU_STATUS)

				if ((index + PPU_START_ADDRESS) != PPU_STATUS_ADDRESS)
				{
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.raw[index] = data;
				}

				// Now update in PPU's perspective as well...

				switch (index + PPU_START_ADDRESS)
				{
				case PPU_CTRL_ADDRESS:
				{
					pNES_cpuMemory->NESMemoryMap.ppuCtrl.raw[index] = data;

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

					refreshOpenBus(data);

					BREAK;
				}
				case PPU_MASK_ADDRESS:
				{
					// MMC5 listens only to the actual CPU $2001 address,
					// not to the PPU register mirrors ($2009, $2011, ...).
					//
					// When both BG and sprite rendering are disabled,
					// MMC5 immediately leaves "in frame" state and disables
					// its PPU data substitutions.

					if ((pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC5) && (address == (PPU_MASK_ADDRESS - PPU_START_ADDRESS)) && ((data & 0x18) == 0x00))
					{
						auto& mmc5 = pNES_instance->NES_state.catridgeInfo.mmc5;

						mmc5.ppuInFrame = NO;
						mmc5.needInFrame = NO;
						mmc5.ppuIdleCounter = ZERO;

						mmc5.lastPpuReadAddr = RESET;
						mmc5.ntReadCounter = ZERO;
						mmc5.scanlineCounter = RESET;
						mmc5.splitTileNumber = RESET;

						mmc5.splitInSplitRegion = NO;
						mmc5.exAttrFetchCounter = RESET;

						mmc5.irqPending = NO;

						pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC5 = RESET;

						updateMMC5ChrA();
					}

					refreshOpenBus(data);

					BREAK;
				}
				case PPU_STATUS_ADDRESS:
				{
					refreshOpenBus(data);

					BREAK;
				}
				case OAM_ADDR_ADDRESS:
				{
					refreshOpenBus(data);

					BREAK;
				}
				case OAM_DATA_ADDRESS:
				{
					if (pNES_instance->NES_state.display.currentScanline >= NES_PRE_RENDER_SCANLINE
						&&
						pNES_instance->NES_state.display.currentScanline <= NES_LAST_VISIBLE_PPU_SCANLINE
						&&
						(pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET
							|| pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET))
					{
						// Don't write to OAM, but do glitchy increment of OAMADDR by 4
						// Refer: https://www.nesdev.org/wiki/PPU_registers#OAMDATA
						// "do perform a glitchy increment of OAMADDR, bumping only the high 6 bits"
						pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR
							= (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR + FOUR) & 0xFC;
					}
					else
					{
						pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR]
							= pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMDATA;

						pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR += ONE;
						pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR &= 0xFF;
					}

					refreshOpenBus(data);

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

					refreshOpenBus(data);

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

						// Delay v update by 3 PPU cycles (matches Mesen _updateVramAddrDelay)
						pNES_ppuRegisters->ppuInternalRegisters.vramAddrUpdateDelay = THREE;
						pNES_ppuRegisters->ppuInternalRegisters.vramAddrPendingValue = pNES_ppuRegisters->ppuInternalRegisters.t.raw;
						// DO NOT set v.raw or call clockMMC3IRQ here

						refreshOpenBus(data);
						pNES_ppuRegisters->ppuInternalRegisters.w = FIRST_WRITE;
					}

					refreshOpenBus(data);

					BREAK;
				}
				case PPU_DATA_ADDRESS:
				{
					writePpuRawMemory(pNES_ppuRegisters->ppuInternalRegisters.v.raw, data, MEMORY_ACCESS_SOURCE::CPU);

					// Defer v increment to ppuTick (matches Mesen _needVideoRamIncrement on writes)
					// ppuTick will do xInc()+yInc() during rendering, or v+=1/32 outside rendering
					pNES_ppuRegisters->ppuInternalRegisters.needVideoRamIncrement = YES;

					refreshOpenBus(data);

					BREAK;
				}
				default:
				{
					FATAL("Unknown PPU register");
				}
				}
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
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM01())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM03())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM01())
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM03()))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ1_HI.LENGTH_COUNTER & 0x1F];
							}
						}
						else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM10())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM12())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_1)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM10())
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM12()))
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
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM01())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM03())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM01())
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM03()))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.SQ2_HI.LENGTH_COUNTER & 0x1F];
							}
						}
						else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM10())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM12())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::PULSE_2)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM10())
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM12()))
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
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM01())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM03())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM01())
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM03()))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.TRI_HI.LENGTH_COUNTER & 0x1F];
							}
						}
						else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM10())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM12())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::TRIANGLE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM10())
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM12()))
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
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM01())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM03())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM01())
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM03()))
								|| (pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET))
							{
								pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter
									= LENGTH_COUNTER_LUT[pNES_cpuMemory->NESMemoryMap.apuAndIO.NOISE_LENGTH_COUNTER.LENGTH_COUNTER & 0x1F];
							}
						}
						else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
						{
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM10())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer == (frameSeqM12())
								&& pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].lengthCounter == RESET)
							{
								pNES_instance->NES_state.audio.skipClockingLengthCounter = YES;
							}
							if ((pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM10())
								&& pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer != (frameSeqM12()))
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
				case MAPPER::INES_MAPPER_105:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						const auto subMapper = pNES_instance->NES_state.catridgeInfo.subMapper;

						if (pNES_instance->NES_state.catridgeInfo.mmc1.prgRamEnable == YES
							&& subMapper != SUB_MAPPER::SGROM
							&& subMapper != SUB_MAPPER::SFROM
							&& subMapper != SUB_MAPPER::SLROM
							&& subMapper != SUB_MAPPER::SBROM
							&& subMapper != SUB_MAPPER::SHROM
							&& subMapper != SUB_MAPPER::SCROM)
						{
							// Compute bank offset: SOROM uses bit 0 (2 banks), SXROM uses bits 0-1 (4 banks)
							uint32_t ramBank = pNES_instance->NES_state.catridgeInfo.mmc1.surom_sxrom.prgRamBank8_sxrom;

							// Mask bank number according to submapper
							if (subMapper == SUB_MAPPER::SOROM)
							{
								ramBank &= 0x01; // 16 KB total (2 x 8 KB banks)
							}
							else if (subMapper != SUB_MAPPER::SXROM)
							{
								ramBank = 0;     // Standard MMC1 / SUROM / SNROM (fixed 8 KB bank)
							}
							// SXROM uses all 2 bits (ramBank & 0x03)

							const uint32_t ramOffset = (ramBank * 0x2000) + (address - UNMAPPED_START_ADDRESS);
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[ramOffset] = data;
						}
					}
					else if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
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
										auto& mmc1 = pNES_instance->NES_state.catridgeInfo.mmc1;
										const BYTE shiftVal = mmc1.intfShiftReg.fields2.shiftValue;

										if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_105)
										{
											// NES-EVENT remapping of chrBank4Lo bits
											auto& ev = mmc1.nes_event;
											mmc1.chrBank4Lo = shiftVal; // store raw (CHR-RAM is fixed 8KB, not really banked)

											// I bit (bit 4): IRQ control
											const bool iBit = (shiftVal & 0x10) != 0;
											if (iBit)
											{
												// I=1: reset counter, clear IRQ, disable counting
												ev.irqCounter = 0;
												ev.irqEnabled = NO;
												pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC1 = RESET;
											}
											else
											{
												// I=0: enable counting
												ev.irqEnabled = YES;
											}

											// Update initState machine
											// (needed to "unlock" PRG swapping)
											if (ev.initState == 0 && !iBit)
											{
												ev.initState = 1;
											}
											else if (ev.initState == 1 && iBit)
											{
												ev.initState = 2; // unlocked
											}

											// PRG banking is now applied in the read path (see item 11)
										}
										else if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SUROM) MASQ_UNLIKELY
										{
											mmc1.surom_sxrom.prgBank256 = (shiftVal & 0x10);

											// Bit 0 (C) controls 4 KB CHR-RAM bank
											if (mmc1.intfControlReg.fields1.c == RESET)
											{
												mmc1.chrBank8 = 0; // 8 KB mode ignores Bit 0 completely
											}
											else
											{
												mmc1.chrBank4Lo = (shiftVal & 0x01); // 4 KB mode uses Bit 0 only
											}
										}
										else if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SOROM) MASQ_UNLIKELY
										{
											// Bit 2 (S): 8 KB PRG-RAM bank select (0 or 1)
											mmc1.surom_sxrom.prgRamBank8_sxrom = (shiftVal >> 2) & 0x01;

											// Bit 0 (C): 4 KB CHR-RAM bank
											if (mmc1.intfControlReg.fields1.c == RESET)
											{
												mmc1.chrBank8 = 0;
											}
											else
											{
												mmc1.chrBank4Lo = (shiftVal & 0x01);
											}
										}
										else if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SXROM) MASQ_UNLIKELY
										{
											// Bit 4 (P): PRG-ROM outer bank
											mmc1.surom_sxrom.prgBank256 = (shiftVal & 0x10);

											// Bits 2-3 (SS): 8 KB PRG-RAM bank select (0 to 3)
											mmc1.surom_sxrom.prgRamBank8_sxrom = (shiftVal >> 2) & 0x03;

											// Bit 0 (C): 4 KB CHR-RAM bank
											if (mmc1.intfControlReg.fields1.c == RESET)
											{
												mmc1.chrBank8 = 0;
											}
											else
											{
												mmc1.chrBank4Lo = (shiftVal & 0x01);
											}
										}
										else
										{
											// 8KB CHR mode
											if (mmc1.intfControlReg.fields1.c == RESET)
											{
												mmc1.chrBank8 = (shiftVal & 0x1E);
											}
											// 4KB CHR mode
											else
											{
												mmc1.chrBank4Lo = (shiftVal & 0x1F);
											}
										}

										BREAK;
									}
									case TWO: // CHR bank 1 $C000-$DFFF
									{
										auto& mmc1 = pNES_instance->NES_state.catridgeInfo.mmc1;
										const BYTE shiftVal = mmc1.intfShiftReg.fields2.shiftValue;
										
										if (mmc1.intfControlReg.fields1.c == SET)
										{
											if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SUROM) MASQ_UNLIKELY
											{
												mmc1.surom_sxrom.prgBank256 = (shiftVal & 0x10);
												// Bit 0 (C) controls 4 KB CHR-RAM bank at $1000-$1FFF
												mmc1.chrBank4Hi = (shiftVal & 0x01);
											}
											else if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SOROM) MASQ_UNLIKELY
											{
												mmc1.surom_sxrom.prgRamBank8_sxrom = (shiftVal >> 2) & 0x01;
												// Bit 0 (C) controls 4 KB CHR-RAM bank at $1000-$1FFF
												mmc1.chrBank4Hi = (shiftVal & 0x01);
											}
											else if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SXROM) MASQ_UNLIKELY
											{
												mmc1.surom_sxrom.prgBank256 = (shiftVal & 0x10);
												mmc1.surom_sxrom.prgRamBank8_sxrom = (shiftVal >> 2) & 0x03;
												// Bit 0 (C) controls 4 KB CHR-RAM bank at $1000-$1FFF
												mmc1.chrBank4Hi = (shiftVal & 0x01);
											}
											else
											{
												mmc1.chrBank4Hi = (shiftVal & 0x1F);
											}
										}

										BREAK;
									}
									case THREE: // PRG bank $E000-$FFFF
									{
										auto& mmc1 = pNES_instance->NES_state.catridgeInfo.mmc1;
										const BYTE shiftVal = mmc1.intfShiftReg.fields2.shiftValue;

										if (mmc1.isMmc1A == YES)
										{
											// MMC1A: PRG-RAM always enabled; bit 4 directly forces A17 when set
											mmc1.prgRamEnable = YES;
											// (A17 bypass logic handled in PRG read — see item below)
										}
										else
										{
											// MMC1B: bit 4 disables PRG-RAM
											mmc1.prgRamEnable = (FLAG)((shiftVal & 0x10) == 0x00);
										}

										const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
										const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
										const uint32_t totalPrg16kBanks = isNES2
											? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
											: hdr.sizeOfPrgRomIn16KB;
										switch (mmc1.intfControlReg.fields1.pp)
										{
										case ZERO:
										case ONE:
										{
											mmc1.prgBank32
												= (mmc1.intfShiftReg.fields2.shiftValue & 0x0E) % totalPrg16kBanks;
											BREAK;
										}
										case TWO:
										{
											// Refer https://www.nesdev.org/wiki/MMC1#Consecutive-cycle_writes for 2: fix first bank at $8000 and switch 16 KB bank at $C000
											mmc1.prgBank16Lo = ZERO;
											mmc1.prgBank16Hi
												= (mmc1.intfShiftReg.fields2.shiftValue & 0x0F) % totalPrg16kBanks;
											BREAK;
										}
										case THREE:
										{
											// Refer https://www.nesdev.org/wiki/MMC1#Consecutive-cycle_writes for 3: fix last bank at $C000 and switch 16 KB bank at $8000
											mmc1.prgBank16Lo
												= (mmc1.intfShiftReg.fields2.shiftValue & 0x0F) % totalPrg16kBanks;
											mmc1.prgBank16Hi = ((totalPrg16kBanks - ONE) & 0x0F);
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
				case MAPPER::INES_MAPPER_180:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.isBusConflictPresent)
						{
							data &= readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
						}

						BYTE mask = 0x0F;
						if (pINES->iNES_Fields.iNES_header.fields.flag7.fields.nes2p0 == 0x02)
						{
							mask = 0xFF;
						}
						if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_180)
						{
							mask = 0x07;
						}

						{
							const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
							const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
							const uint32_t totalPrg16kBanks = isNES2
								? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
								: hdr.sizeOfPrgRomIn16KB;
							pNES_instance->NES_state.catridgeInfo.uxrom_002.prgBank16 = (data & mask) % totalPrg16kBanks;
						}
					}
					BREAK;
				}
				case MAPPER::CNROM:
				case MAPPER::J87:
				{
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
					const uint32_t totalChr8kBanks = isNES2
						? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
						: hdr.sizeOfChrRomIn8KB;

					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::J87)
						{
							pNES_instance->NES_state.catridgeInfo.cnrom.chrBank8 = (data & 0x03) % totalChr8kBanks;
						}
						else if (pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
					}
					if ((pNES_instance->NES_state.catridgeInfo.mapper != MAPPER::J87)
						&& IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.isBusConflictPresent)
						{
							data &= readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
						}

						pNES_instance->NES_state.catridgeInfo.cnrom.chrBank8 = (data & 0x03) % totalChr8kBanks;
					}
					BREAK;
				}
				case MAPPER::MMC3:
				case MAPPER::INES_MAPPER_037:
				case MAPPER::INES_MAPPER_047:
				case MAPPER::INES_MAPPER_118:
				case MAPPER::INES_MAPPER_119:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::MMC6
							&& pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.mm6PrgRamEnable == SET
							&& pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							if (address >= 0x7000)
							{
								const uint16_t ramAddr = (address - UNMAPPED_START_ADDRESS) & 0x03FF; // 1KB PRG-RAM bank mirroring

								// Bit 9 selects the upper 512-byte half (0x200-0x3FF); clear selects the lower half (0x000-0x1FF)
								if ((ramAddr & 0x0200)
									? (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.mmc6Fields.enW23FF == SET)
									: (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.mmc6Fields.enW01FF == SET))
								{
									pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[ramAddr] = data;
								}
							}
						}
						else if ((pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.fields.prgRamEnable == SET)
							&& pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.fields.denyWrite == RESET)
						{
							if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_037)
							{
								// NOTE: Outer bank register — write only, not RAM.
								// Refer https://www.nesdev.org/wiki/INES_Mapper_037
								pNES_instance->NES_state.catridgeInfo.mmc3.ines037.outerBank = data & 0x07;
							}
							else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_047)
							{
								if (address >= CATRIDGE_RAM_START_ADDRESS)
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.ines047.multicart = data & 0x01;
								}
							}
							else if (pNES_instance->NES_state.catridgeInfo.hasPrgRam)
							{
								// MMC3 and INES_MAPPER_119 have normal PRG-RAM here
								pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
							}
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

								// Bank counts for % guard
								const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
								const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

								// --- PRG Banks ---
								const uint32_t totalPrg16kBanks = isNES2
									? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
									: hdr.sizeOfPrgRomIn16KB;
								const uint32_t totalPrg8kBanks = totalPrg16kBanks * 2;

								// --- CHR Banks ---
								const uint32_t totalChr8kBanks = isNES2
									? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
									: hdr.sizeOfChrRomIn8KB;

								// Frequently used sub-bank sizes for fine-grained mapper switching guards
								uint32_t totalChr1kBanks = totalChr8kBanks * 8;

								// Special CHR RAM handling
								if (pNES_instance->NES_state.catridgeInfo.chrRomSizeBytes == ZERO)
								{
									if (pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes >= 0x2000)
									{
										totalChr1kBanks = TO_UINT32((pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes / 0x0400)); // 1KB banks
									}
								}

								const bool isTxSROM = (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_118);
								const bool isTQROM = (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_119);

								BYTE chr01Mask = 0xFE;
								BYTE chr25Mask = 0xFF;
								BYTE chr67Mask = 0x3F;
								if (isTQROM)
								{
									chr01Mask = 0x3E;
									chr25Mask = 0x3F;
									chr67Mask = 0x3F;
								}

								switch (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.bankRegSel)
								{
								case ZERO:
								{
									if (isTQROM)
									{
										// NOTE: Mapper 119 — bit 6 is chip select (0=ROM, 1=RAM), not part of bank number.
										// Apply % guard only for ROM banks (CS=0); RAM banks are guarded by & 0x1FFF at access time.
										// Refer https://www.nesdev.org/wiki/INES_Mapper_119
										pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2aCS = (data >> 6) & 0x01;
										const BYTE bankNum = (data & chr01Mask);
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a =
											(pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2aCS == ONE)
											? bankNum
											: bankNum % totalChr1kBanks;
									}
									else if (totalChr1kBanks > ZERO)
									{
										// 2KB CHR bank at PPU $0000 (or $1000); & 0xFE enforces 2KB alignment
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a = (data & chr01Mask) % totalChr1kBanks; // Bank numbers still follow 1kb format, hence % totalChr1kBanks and not % totalChr2kBanks
									}
									BREAK;
								}
								case ONE:
								{
									if (isTQROM)
									{
										// NOTE: Mapper 119 — bit 6 is chip select (0=ROM, 1=RAM), not part of bank number.
										// Apply % guard only for ROM banks (CS=0); RAM banks are guarded by & 0x1FFF at access time.
										// Refer https://www.nesdev.org/wiki/INES_Mapper_119
										pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2bCS = (data >> 6) & 0x01;
										const BYTE bankNum = (data & chr01Mask);
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b =
											(pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank2bCS == ONE)
											? bankNum
											: bankNum % totalChr1kBanks;
									}
									else if (totalChr1kBanks > ZERO)
									{
										// 2KB CHR bank at PPU $0800 (or $1800); & 0xFE enforces 2KB alignment
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b = (data & chr01Mask) % totalChr1kBanks; // Bank numbers still follow 1kb format, hence % totalChr1kBanks and not % totalChr2kBanks
									}
									BREAK;
								}
								case TWO:
								{
									if (totalChr1kBanks > ZERO)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a = (data & chr25Mask) % totalChr1kBanks;
									}
									if (isTQROM)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1aCS = (data >> 6) & 0x01;
									}
									BREAK;
								}
								case THREE:
								{
									if (totalChr1kBanks > ZERO)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b = (data & chr25Mask) % totalChr1kBanks;
									}
									if (isTQROM)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1bCS = (data >> 6) & 0x01;
									}
									BREAK;
								}
								case FOUR:
								{
									if (totalChr1kBanks > ZERO)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c = (data & chr25Mask) % totalChr1kBanks;
									}
									if (isTQROM)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1cCS = (data >> 6) & 0x01;
									}
									BREAK;
								}
								case FIVE:
								{
									if (totalChr1kBanks > ZERO)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d = (data & chr25Mask) % totalChr1kBanks;
									}
									if (isTQROM)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.ines119.chrBank1dCS = (data >> 6) & 0x01;
									}
									BREAK;
								}
								case SIX:
								{
									if (totalPrg8kBanks > ZERO)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8a = (data & chr67Mask) % totalPrg8kBanks;
									}
									BREAK;
								}
								case SEVEN:
								{
									if (totalPrg8kBanks > ZERO)
									{
										pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8b = (data & chr67Mask) % totalPrg8kBanks;
									}
									BREAK;
								}
								default:
								{
									FATAL("Invalid MMC3 Bank Select");
									BREAK;
								}
								}

								// -------------------------------------------------------
								// Mapper 118 (TxSROM) nametable update.
								// Runs only on odd writes ($8001/$8003/...) when mapper is 118.
								// Bit 7 of the bank data byte selects which CIRAM page (0 or 1)
								// backs the nametable slot(s) that the written CHR bank covers.
								// -------------------------------------------------------
								if (isTxSROM)
								{
									const BYTE ntPage = (data >> 7) & 1;
									const BYTE regSel = pNES_instance->NES_state.catridgeInfo.mmc3
										.exRegisters.bankRegisterSelect_even8k.fields.bankRegSel;
									const BIT ChrMode = pNES_instance->NES_state.catridgeInfo.mmc3
										.exRegisters.bankRegisterSelect_even8k.fields.chrA12Inversion;

									auto& txs = pNES_instance->NES_state.catridgeInfo.mmc3.txsrom;

									if (ChrMode == RESET)
									{
										// ChrMode == 0: lower half is $0000-$0FFF, mapped by R0 (2KB) and R1 (2KB).
										// R2-R5 map into the UPPER half ($1000-$1FFF) -> ignored, no NT write.
										switch (regSel)
										{
										case ZERO:
											txs.ntPage[0] = ntPage;
											txs.ntPage[1] = ntPage;
											BREAK;
										case ONE:
											txs.ntPage[2] = ntPage;
											txs.ntPage[3] = ntPage;
											BREAK;
										default:
											// R2-R5, R6, R7 all map into $1000-$1FFF or are PRG — ignored.
											BREAK;
										}
									}
									else
									{
										// ChrMode == 1 (chrA12Inv set): lower half is $0000-$0FFF,
										// mapped by R2/R3/R4/R5 (1KB each).
										// R0 and R1 map into the UPPER half ($1000-$1FFF) -> ignored.
										switch (regSel)
										{
										case TWO:   txs.ntPage[0] = ntPage; BREAK;
										case THREE: txs.ntPage[1] = ntPage; BREAK;
										case FOUR:  txs.ntPage[2] = ntPage; BREAK;
										case FIVE:  txs.ntPage[3] = ntPage; BREAK;
										default:
											// R0, R1, R6, R7 -> ignored.
											BREAK;
										}
									}
								}

								if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::MMC6
									&& pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.mm6PrgRamEnable == RESET)
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.raw = ZERO; // MMC6 continuously sets $A001 to $00
								}
							}

							BREAK;
						}
						case 0xA000:
						case 0xB000:
						{
							if (IS_EVEN(address))
							{
								if (pNES_instance->NES_state.catridgeInfo.mapper != MAPPER::INES_MAPPER_118)
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
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.raw = data;

								if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::MMC6
									&& pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.mm6PrgRamEnable == RESET)
								{
									pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.raw = RESET; // MMC6: writes to $A001 are disabled when PRG RAM is disabled
								}
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
				case MAPPER::INES_MAPPER_268:
				{
					const BYTE submapperRaw = (BYTE)pNES_instance->NES_state.catridgeInfo.subMapper;
					const bool isMindkids = (submapperRaw & 0x01) != 0;
					auto& outer = pNES_instance->NES_state.catridgeInfo.mmc3.ines268;
					const BYTE a001Raw = pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.raw;

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS)) // $6000-$7FFF
					{
						if (!isMindkids)
						{
							// Coolboy: outer registers overlay the WHOLE $6000-$7FFF range.
							if (mapper268WramWritable(a001Raw))
							{
								pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
							}
							mapper268WriteReg(outer.reg, address, data, submapperRaw);
						}
						else if (mapper268WramWritable(a001Raw))
						{
							// Mindkids: this range is PURE WRAM. Registers never live here.
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
					}
					else if (isMindkids && IF_ADDRESS_WITHIN(address, 0x5000, 0x5FFF))
					{
						mapper268WriteReg(outer.reg, address, data, submapperRaw);
					}

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						switch (address & 0xF000)
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

								// Bank numbers stored RAW/unmodulo'd — mapper268ComputePrgPage
								// and mapper268ComputeChrPage do all size-relative masking;
								// the only wrap happens once, at final indexing.
								switch (pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.bankRegisterSelect_even8k.fields.bankRegSel)
								{
								case ZERO:  pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a = data & 0xFE; BREAK;
								case ONE:   pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b = data & 0xFE; BREAK;
								case TWO:   pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a = data; BREAK;
								case THREE: pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b = data; BREAK;
								case FOUR:  pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c = data; BREAK;
								case FIVE:  pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d = data; BREAK;
								case SIX:   pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8a = data; BREAK;
								case SEVEN: pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8b = data; BREAK;
								default: FATAL("Invalid MMC3 Bank Select"); BREAK;
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
								pNES_instance->NES_state.catridgeInfo.nameTblMir = mapper268ResolveMirroring(data, outer.reg, submapperRaw);
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
							FATAL("Invalid Address");
						}
					}
					BREAK;
				}
				case MAPPER::RAMBO1:
				case MAPPER::INES_MAPPER_158:
				{
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						auto& rb = pNES_instance->NES_state.catridgeInfo.mmc3.rambo1;

						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;
						const uint32_t totalPrg8kBanks = totalPrg16kBanks * 2;
						const uint32_t totalChr8kBanks = isNES2
							? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
							: hdr.sizeOfChrRomIn8KB;
						const uint32_t totalChr1kBanks = totalChr8kBanks * 8;

						switch (address & 0xE001)
						{
						case 0x8000:
						{
							rb.currentRegister = data;
							BREAK;
						}
						case 0x8001:
						{
							const BYTE regSel = rb.currentRegister & 0x0F;

							// Store into the register array with wrapping guards
							if (regSel <= 5)
							{
								// CHR 1KB registers R0-R5
								rb.reg[regSel] = (totalChr1kBanks > RESET)
									? (data % totalChr1kBanks)
									: data;
							}
							else if (regSel == 6 || regSel == 7)
							{
								// PRG 8KB registers R6, R7
								rb.reg[regSel] = (totalPrg8kBanks > RESET)
									? (data & 0x3F) % totalPrg8kBanks
									: (data & 0x3F);
							}
							else if (regSel == 8 || regSel == 9)
							{
								// Extra 1KB CHR registers R8, R9 (K=1 mode only)
								rb.reg[regSel] = (totalChr1kBanks > RESET)
									? (data % totalChr1kBanks)
									: data;
							}
							else if (regSel == 0x0F)
							{
								// RF — third switchable PRG 8KB bank
								rb.reg[15] = (totalPrg8kBanks > RESET)
									? (data & 0x3F) % totalPrg8kBanks
									: (data & 0x3F);
							}

							// -------------------------------------------------------
							// Mapper 158 only: bit 7 of bank data drives CIRAM A10,
							// selecting which nametable CIRAM page (0 or 1) backs the
							// nametable slots covered by the written CHR bank.
							// Same logic as TxSROM (mapper 118), same ntPage[] storage.
							// -------------------------------------------------------
							if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_158)
							{
								const BYTE ntPage = (data >> 7) & 1;
								const BIT chrMode = (rb.currentRegister >> 7) & 1;
								const BYTE regSel3 = rb.currentRegister & 0x07;  // low 3 bits only, matching Mesen

								auto& txs = pNES_instance->NES_state.catridgeInfo.mmc3.txsrom;

								if (chrMode == SET)
								{
									// _currentRegister & 0x80 is set: A12 inverted.
									// Lower half ($0000-$0FFF) = R2/R3/R4/R5.
									switch (regSel3)
									{
									case 2: txs.ntPage[0] = ntPage; BREAK;
									case 3: txs.ntPage[1] = ntPage; BREAK;
									case 4: txs.ntPage[2] = ntPage; BREAK;
									case 5: txs.ntPage[3] = ntPage; BREAK;
									default:
										BREAK;
									}
								}
								else
								{
									// _currentRegister & 0x80 is clear: A12 not inverted.
									// Lower half ($0000-$0FFF) = R0 (2KB) and R1 (2KB).
									switch (regSel3)
									{
									case 0:
										txs.ntPage[0] = ntPage;
										txs.ntPage[1] = ntPage;
										BREAK;
									case 1:
										txs.ntPage[2] = ntPage;
										txs.ntPage[3] = ntPage;
										BREAK;
									default:
										BREAK;
									}
								}
							}

							BREAK;
						}
						case 0xA000:
						{
							// Mirroring — mapper 64 only.
							// Mapper 158 ignores this write entirely (mirroring is
							// driven by CHR A17 via the $8001 ntPage[] writes above).
							if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::RAMBO1)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir =
									(data & 0x01)
									? NAMETABLE_MIRROR::HORIZONTAL_MIRROR
									: NAMETABLE_MIRROR::VERTICAL_MIRROR;
							}
							BREAK;
						}
						case 0xA001:
						{
							// Not implemented on RAMBO-1 (no PRG-RAM).
							BREAK;
						}
						case 0xC000:
						{
							// IRQ latch — reload value for IRQ counter
							rb.irqReloadValue = data;
							BREAK;
						}
						case 0xC001:
						{
							// IRQ mode select & reload
							// Skull & Crossbones fix: switching from cycle mode to
							// scanline mode forces one extra clock before the switch.
							if (rb.irqCycleMode == YES && (data & 0x01) == RESET)
							{
								rb.forceClock = YES;
							}
							rb.irqCycleMode = (data & 0x01) ? YES : NO;
							if (rb.irqCycleMode == YES)
							{
								rb.cpuClockCounter = RESET;
							}
							rb.needReload = YES;
							BREAK;
						}
						case 0xE000:
						{
							// IRQ disable and acknowledge
							rb.irqEnabled = NO;
							rb.irqDelay = RESET;
							pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC3 = RESET;
							BREAK;
						}
						case 0xE001:
						{
							// IRQ enable — does NOT acknowledge if already pending
							rb.irqEnabled = YES;
							BREAK;
						}
						default:
							BREAK;
						}
					}
					BREAK;
				}
				case MAPPER::MMC5:
				{
					auto& mmc5 = pNES_instance->NES_state.catridgeInfo.mmc5;

					// $5000-$5FFF: MMC5 internal registers and ExRAM
					if (IF_ADDRESS_WITHIN(address, 0x5000, 0x5FFF))
					{
						if (IF_ADDRESS_WITHIN(address, 0x5C00, 0x5FFF))
						{
							// ExRAM writes
							if (mmc5.extendedRamMode == 3)
							{
								BREAK;  // Read-only
							}
							BYTE writeData = data;
							if (mmc5.extendedRamMode <= 1 && mmc5.ppuInFrame == NO)
							{
								writeData = 0;  // Mode 0/1: writes 0 outside rendering
							}
							mmc5.exRam[address - 0x5C00] = writeData;
							BREAK;
						}

						// MMC5 Pulse 1: $5000-$5003
						// MMC5 Pulse 2: $5004-$5007
						if (IF_ADDRESS_WITHIN(address, 0x5000, 0x5007))
						{
							const uint8_t pulseIdx = (address >= 0x5004) ? 1 : 0;
							auto& p = mmc5.mmc5Audio.pulse[pulseIdx];
							const uint8_t reg = address & 0x03;
							switch (reg)
							{
							case 0: // $5000/$5004 — duty, envelope
								p.duty = (data >> 6) & 0x03;
								p.envelopeLoop = (data & 0x20) ? YES : NO;
								p.constantVolume = (data & 0x10) ? YES : NO;
								p.envelopePeriod = data & 0x0F;
								p.envelope.startFlag = YES;
								BREAK;
							case 1: // $5001/$5005 — no sweep on MMC5, ignore
								BREAK;
							case 2: // $5002/$5006 — frequency low
								p.frequencyPeriod.fields.lo = data;
								BREAK;
							case 3: // $5003/$5007 — frequency high + length counter load
								p.frequencyPeriod.fields.hi = data & 0x07;
								if (p.enabled == YES)
								{
									static const uint8_t LENGTH_TABLE[32] = {
										10, 254, 20, 2,  40, 4,  80, 6,
										160, 8, 60, 10, 14, 12, 26, 14,
										12, 16, 24, 18, 48, 20, 96, 22,
										192, 24, 72, 26, 16, 28, 32, 30
									};
									p.lengthCounter = LENGTH_TABLE[(data >> 3) & 0x1F];
								}
								p.dutyCounter = 0;
								p.frequencyCounter = p.frequencyPeriod.raw; // reset timer
								p.envelope.startFlag = YES;
								BREAK;
							}
							BREAK;
						}

						switch (address)
						{
						case 0x5010: // PCM mode
							mmc5.mmc5Audio.pcmIrqEnable = (data & 0x80) ? YES : NO;
							mmc5.mmc5Audio.pcmReadMode = (data & 0x01) ? YES : NO;
							BREAK;

						case 0x5011: // PCM raw sample
							if (mmc5.mmc5Audio.pcmReadMode == NO && data != 0)
							{
								mmc5.mmc5Audio.pcmRawSample = data;
							}
							BREAK;

						case 0x5015: // channel enable/status
							mmc5.mmc5Audio.status = data & 0x03;
							mmc5.mmc5Audio.pulse[0].enabled = (data & 0x01) ? YES : NO;
							mmc5.mmc5Audio.pulse[1].enabled = (data & 0x02) ? YES : NO;
							if (mmc5.mmc5Audio.pulse[0].enabled == NO) mmc5.mmc5Audio.pulse[0].lengthCounter = 0;
							if (mmc5.mmc5Audio.pulse[1].enabled == NO) mmc5.mmc5Audio.pulse[1].lengthCounter = 0;
							BREAK;

						case 0x5100: mmc5.prgMode = data & 0x03; BREAK;
						case 0x5101:
							mmc5.chrMode = data & 0x03;
							updateMMC5ChrA();
							BREAK;
						case 0x5102: mmc5.prgRamProtect1 = data & 0x03; BREAK;
						case 0x5103: mmc5.prgRamProtect2 = data & 0x03; BREAK;
						case 0x5104:
							mmc5.extendedRamMode = data & 0x03;
							BREAK;
						case 0x5105:
							mmc5.nametableMapping = data;
							BREAK;
						case 0x5106: mmc5.fillTile = data;        BREAK;
						case 0x5107: mmc5.fillColor = data & 0x03; BREAK;
						case 0x5113: mmc5.prgBanks[0] = data;      BREAK;
						case 0x5114: mmc5.prgBanks[1] = data;      BREAK;
						case 0x5115: mmc5.prgBanks[2] = data;      BREAK;
						case 0x5116: mmc5.prgBanks[3] = data;      BREAK;
						case 0x5117:
							mmc5.prgBanks[4] = data | 0x80; // Always ROM
							BREAK;
						case 0x5120: case 0x5121: case 0x5122: case 0x5123:
						case 0x5124: case 0x5125: case 0x5126: case 0x5127:
						case 0x5128: case 0x5129: case 0x512A: case 0x512B:
						{
							const uint8_t  bankIdx = (uint8_t)(address - 0x5120);
							const uint16_t newValue = (uint16_t)data | ((uint16_t)mmc5.chrUpperBits << 8);
							if (newValue == mmc5.chrBanks[bankIdx] && mmc5.lastChrReg == address)
							{
								BREAK;
							}
							mmc5.chrBanks[bankIdx] = newValue;
							mmc5.lastChrReg = address;
							updateMMC5ChrA();
							BREAK;
						}
						case 0x5130: mmc5.chrUpperBits = data & 0x03; BREAK;
						case 0x5200:
						{
							mmc5.verticalSplitEnabled = (data & 0x80) ? YES : NO;
							mmc5.verticalSplitRightSide = (data & 0x40) ? YES : NO;
							mmc5.verticalSplitDelimiterTile = (data & 0x1F);
							BREAK;
						}
						case 0x5201: // Vertical split scroll
						{
							mmc5.verticalSplitScroll = data;
							BREAK;
						}
						case 0x5202: // Vertical split bank
						{
							mmc5.verticalSplitBank = data;
							BREAK;
						}
						case 0x5203: mmc5.irqCounterTarget = data; BREAK;
						case 0x5204:
							mmc5.irqEnabled = ((data & 0x80) != 0) ? YES : NO;
							if (mmc5.irqEnabled == NO)
							{
								pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC5 = RESET;
							}
							else if (mmc5.irqEnabled == YES && mmc5.irqPending == YES)
							{
								pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC5 = SET;
							}
							BREAK;
						case 0x5205: mmc5.multiplier1 = data; BREAK;
						case 0x5206: mmc5.multiplier2 = data; BREAK;
						default: BREAK;
						}
						BREAK;
					}

					// $6000-$7FFF: PRG-RAM via $5113 (always RAM)
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						const bool canWrite = (mmc5.prgRamProtect1 == 0x02 && mmc5.prgRamProtect2 == 0x01);
						if (canWrite)
						{
							const uint32_t ramBank = mmc5.prgBanks[0] & 0x07;
							const uint32_t idx = ramBank * 0x2000 + (address - CATRIDGE_RAM_START_ADDRESS);
							mmc5.prgRam[idx & 0xFFFF] = data;
						}
						BREAK;
					}

					// $8000-$FFFF: write only to RAM-mapped banks (ROM banks are read-only)
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						const bool canWrite = (mmc5.prgRamProtect1 == 0x02 && mmc5.prgRamProtect2 == 0x01);
						if (!canWrite)
						{
							BREAK;
						}

						uint32_t idx = 0;
						bool     isRAM = false;

						switch (mmc5.prgMode)
						{
						case 0:
							// Mode 0: always ROM, no writable banks above $6000
							BREAK;
						case 1:
							if (address <= 0xBFFF && (mmc5.prgBanks[2] & 0x80) == 0)
							{
								isRAM = true;
								idx = (uint32_t)(mmc5.prgBanks[2] & 0x06) * 0x2000 + (address - 0x8000);
							}
							BREAK;
						case 2:
							if (address <= 0xBFFF && (mmc5.prgBanks[2] & 0x80) == 0)
							{
								isRAM = true;
								idx = (uint32_t)(mmc5.prgBanks[2] & 0x06) * 0x2000 + (address - 0x8000);
							}
							else if (address <= 0xDFFF && (mmc5.prgBanks[3] & 0x80) == 0)
							{
								isRAM = true;
								idx = (uint32_t)(mmc5.prgBanks[3] & 0x07) * 0x2000 + (address - 0xC000);
							}
							BREAK;
						case 3:
							if (address <= 0x9FFF && (mmc5.prgBanks[1] & 0x80) == 0)
							{
								isRAM = true;
								idx = (uint32_t)(mmc5.prgBanks[1] & 0x07) * 0x2000 + (address - 0x8000);
							}
							else if (address <= 0xBFFF && (mmc5.prgBanks[2] & 0x80) == 0)
							{
								isRAM = true;
								idx = (uint32_t)(mmc5.prgBanks[2] & 0x07) * 0x2000 + (address - 0xA000);
							}
							else if (address <= 0xDFFF && (mmc5.prgBanks[3] & 0x80) == 0)
							{
								isRAM = true;
								idx = (uint32_t)(mmc5.prgBanks[3] & 0x07) * 0x2000 + (address - 0xC000);
							}
							BREAK;
						}

						if (isRAM)
						{
							mmc5.prgRam[idx & 0xFFFF] = data;
						}
						BREAK;
					}
					BREAK;
				}
				case MAPPER::MMC2:
				case MAPPER::MMC4:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					if (IF_ADDRESS_WITHIN(address, 0xA000, UNMAPPED_END_ADDRESS))
					{
						const FLAG isMMC4 = (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC4);

						auto* mmc = (isMMC4) ? (void*)&pNES_instance->NES_state.catridgeInfo.mmc4 : (void*)&pNES_instance->NES_state.catridgeInfo.mmc2;

						switch (address & 0xF000)
						{
						case 0xA000:
							isMMC4
								? ((decltype(&pNES_instance->NES_state.catridgeInfo.mmc4))mmc)->prgBank16
								: ((decltype(&pNES_instance->NES_state.catridgeInfo.mmc2))mmc)->prgBank;
							((isMMC4
								? ((decltype(&pNES_instance->NES_state.catridgeInfo.mmc4))mmc)->prgBank16
								: ((decltype(&pNES_instance->NES_state.catridgeInfo.mmc2))mmc)->prgBank) = (data & 0x0F));
							BREAK;

						case 0xB000:
							((decltype(&pNES_instance->NES_state.catridgeInfo.mmc2))mmc)->chrBankFD[0] = (data & 0x1F);
							BREAK;

						case 0xC000:
							((decltype(&pNES_instance->NES_state.catridgeInfo.mmc2))mmc)->chrBankFE[0] = (data & 0x1F);
							BREAK;

						case 0xD000:
							((decltype(&pNES_instance->NES_state.catridgeInfo.mmc2))mmc)->chrBankFD[1] = (data & 0x1F);
							BREAK;

						case 0xE000:
							((decltype(&pNES_instance->NES_state.catridgeInfo.mmc2))mmc)->chrBankFE[1] = (data & 0x1F);
							BREAK;

						case 0xF000:
							pNES_instance->NES_state.catridgeInfo.nameTblMir
								= ((data & 0x01) == RESET) ? NAMETABLE_MIRROR::VERTICAL_MIRROR : NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
							BREAK;

						default:
							BREAK;
						}
					}
					BREAK;
				}
				case MAPPER::AxROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						// Note: Nesdev mentions only first 3 bits to be considered for prgBank,
						// But according to https://forums.nesdev.org/viewtopic.php?p=79826#p79826, first four bits needs to be considered
						// Both Mesen and Fceux64 do the same as well
						{
							const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
							const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
							const uint32_t totalPrg16kBanks = isNES2
								? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
								: hdr.sizeOfPrgRomIn16KB;
							// AxROM switches 32KB banks
							const uint32_t totalPrg32kBanks = totalPrg16kBanks / 2;
							pNES_instance->NES_state.catridgeInfo.axrom.prgBank = (data & 0x0F) % totalPrg32kBanks;
						}
						pNES_instance->NES_state.catridgeInfo.axrom.vramPage = (((data & 0x10) == 0x10) ? YES : NO);
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_218:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					// Writes to $8000-$FFFF do nothing.
					BREAK;
				}
				case MAPPER::COLOR_DREAMS:
				{
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.isBusConflictPresent)
						{
							data &= readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
						}

						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;
						const uint32_t totalChr8kBanks = isNES2
							? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
							: hdr.sizeOfChrRomIn8KB;
						// COLOR_DREAMS switches 32KB PRG and 8KB CHR banks
						const uint32_t totalPrg32kBanks = totalPrg16kBanks / 2;
						pNES_instance->NES_state.catridgeInfo.colorDreams.chrBank8 = ((data >> FOUR) & 0x0F) % totalChr8kBanks;
						pNES_instance->NES_state.catridgeInfo.colorDreams.prgBank32 = (data & 0x03) % totalPrg32kBanks;
					}
					BREAK;
				}
				case MAPPER::CPROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.isBusConflictPresent)
						{
							data &= readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
						}
						pNES_instance->NES_state.catridgeInfo.cprom.chrBank = (data & 0x03);
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_014:
				{
					auto& reg014 = pNES_instance->NES_state.catridgeInfo.ines014;
					auto& vrc24 = pNES_instance->NES_state.catridgeInfo.vrc24;

					// Supervisor register. Wiki lists "Mask: $FFFF?" (uncertain) -- treated
					// as exact-address-only here; the ONLY known cart (Samurai Spirits 8CH)
					// should confirm whether that's right.
					if (address == 0xA131)
					{
						reg014.supervisorReg = data;
					}

					const bool isMMC3Mode = (reg014.supervisorReg & 0x10) != ZERO;

					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (isMMC3Mode)
						{
							if ((pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.fields.prgRamEnable == SET)
								&& pNES_instance->NES_state.catridgeInfo.mmc3.exRegisters.prgRamProtect_oddAk.fields.denyWrite == RESET)
							{
								pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
							}
						}
						else // VRC2 mode -- identical semantics to your existing VRC2_022/etc. case
						{
							const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
							const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
							FLAG hasPrgRam = NO;
							if (isNES2)
							{
								hasPrgRam = (hdr.flags_8to15.nes2p0.flag10.fields.prgVolRam > ZERO)
									|| (hdr.flags_8to15.nes2p0.flag10.fields.prgNonVolRam > ZERO);
							}
							else
							{
								hasPrgRam = (hdr.flags_8to15.ines.flag10.fields.prgRamNotPresent != ONE);
							}

							if (!hasPrgRam && IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, 0x6FFF))
							{
								vrc24.latch = data & 0x01;
							}
							else
							{
								pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
							}
						}
					}

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (isMMC3Mode)
						{
							// Identical to your shared MMC3 $8000-$FFFF register-write case
							// body (bank select, mirroring, PRG-RAM protect, IRQ) -- paste
							// that block here unmodified; $A131 itself also lands in the
							// $A000/$B000 branch of that same switch and is handled fine
							// since it only cares about IS_EVEN(address), which $A131 (odd) is not.
							switch (address & 0xF000)
							{
							case 0x8000:
							case 0x9000:
								/* ... same as your MMC3 case body ... */
								BREAK;
							case 0xA000:
							case 0xB000:
								/* ... same as your MMC3 case body ... */
								BREAK;
							case 0xC000:
							case 0xD000:
								/* ... same as your MMC3 case body ... */
								BREAK;
							case 0xE000:
							case 0xF000:
								/* ... same as your MMC3 case body ... */
								BREAK;
							default:
								FATAL("Invalid Address");
							}
						}
						else // VRC2 mode -- VRC2b address convention specifically (A0=bit0, A1=bit1),
							 // per the wiki's explicit "(A0/A1, VRC2b)" callout, NOT the VRC2_022 (VRC2a) convention.
						{
							const uint32_t A0 = GETBIT(ZERO, address);
							const uint32_t A1 = GETBIT(ONE, address);
							const uint32_t decodedAddress = (address & ~0x03) | (A1 << ONE) | A0;

							switch (decodedAddress & 0xF000)
							{
							case 0x8000:
								vrc24.prgBank0 = data & 0x1F;
								BREAK;
							case 0xA000:
								vrc24.prgBank1 = data & 0x1F;
								BREAK;
							case 0x9000:
								// VRC2 has no $9002 swap/WRAM register -- mirroring only.
								switch (data & 0x01)
								{
								case ZERO: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR; BREAK;
								case ONE:  pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR; BREAK;
								}
								BREAK;
							case 0xB000:
							case 0xC000:
							case 0xD000:
							case 0xE000:
								if (IF_ADDRESS_WITHIN(decodedAddress, 0xB000, 0xEFFF))
								{
									const uint32_t chrRegion = (decodedAddress >> TWELVE) - 0x0B;
									if (chrRegion <= THREE)
									{
										const uint32_t subIndex = (decodedAddress >> ONE) & 0x01;
										const uint32_t reg = (chrRegion * TWO) + subIndex;

										if ((decodedAddress & 0x01) == ZERO)
										{
											vrc24.chrBank[reg] = (vrc24.chrBank[reg] & 0x0F0) | (data & 0x0F);
										}
										else
										{
											vrc24.chrBank[reg] = (vrc24.chrBank[reg] & 0x00F) | ((data & 0x0F) << FOUR);
											vrc24.chrBank[reg] &= 0x0FF;
										}
									}
								}
								BREAK;
							default:
								BREAK; // no IRQ registers in VRC2 mode
							}
						}
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_015:
				{
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						BREAK;
					}

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						auto& reg = pNES_instance->NES_state.catridgeInfo.ines015;
						reg.latchedAddr = address;
						reg.latchedData = data;

						pNES_instance->NES_state.catridgeInfo.nameTblMir =
							(data & 0x40) ? NAMETABLE_MIRROR::HORIZONTAL_MIRROR : NAMETABLE_MIRROR::VERTICAL_MIRROR;
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_016:
				{
					const auto submapper = pNES_instance->NES_state.catridgeInfo.subMapper;
					bool isRegisterWrite = false;
					uint16_t regOffset = 0;

					// Submapper 4 (FCG-1/2): Strictly lives within Cartridge RAM space $6000-$7FFF
					if (submapper == SUB_MAPPER::BANDAI_FCG_1_2 &&
						IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						isRegisterWrite = true;
						regOffset = address & 0x000F;
					}
					// Submapper 5 (LZ93D50): Strictly lives within ROM space $8000-$FFFF
					else if ((submapper == SUB_MAPPER::BANDAI_LZ93D50_24C01 || submapper == SUB_MAPPER::BANDAI_LZ93D50_24C02) &&
						IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						isRegisterWrite = true;
						regOffset = address & 0x000F;
					}

					if (isRegisterWrite)
					{
						auto& m016 = pNES_instance->NES_state.catridgeInfo.ines016;

						// CHR Registers (0x00 - 0x07)
						if (regOffset <= 0x07)
						{
							m016.chrBank[regOffset] = data;

							// Replicate Mesen's page count tracking check (GetPrgPageCount() >= 0x20)
							const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
							const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
							const uint32_t totalPrg16kBanks = isNES2
								? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
								: hdr.sizeOfPrgRomIn16KB;

							if (totalPrg16kBanks >= 0x20) // Oversized games (512KB+) accumulate bit 0 across CHR regs
							{
								m016.prgBankSelectUpper = 0;
								for (int i = 0; i < 8; i++)
								{
									m016.prgBankSelectUpper |= (m016.chrBank[i] & 0x01) << 4;
								}

								// Refresh execution windows immediately with the newly accumulated configuration bits
								m016.prgBank = m016.prgPage | m016.prgBankSelectUpper;
							}
							else
							{
								m016.prgBankSelectUpper = 0;
							}
							RETURN;
						}

						switch (regOffset)
						{
						case 0x08: // PRG Bank Select (16KB)
						{
							m016.prgPage = data & 0x0F;
							m016.prgBank = m016.prgPage | m016.prgBankSelectUpper;
							BREAK;
						}

						case 0x09: // Mirroring Mode Configuration
						{
							BYTE mirroringMode = data & 0x03;
							m016.mirroringMode = mirroringMode;

							if (mirroringMode == 0)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
							}
							else if (mirroringMode == 1)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
							}
							else if (mirroringMode == 2)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
							}
							else if (mirroringMode == 3)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR;
							}
							BREAK;
						}

						case 0x0A: // IRQ Control
						{
							m016.irqCountingEnable = (data & 0x01) == SET;
							pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_INES016 = RESET;

							if (m016.irqCountingEnable == SET && m016.irqCounter == RESET)
							{
								pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_INES016 = SET;
							}

							// Submapper 5 (LZ93D50) explicitly clones latch to counter on activation write
							if (submapper != SUB_MAPPER::BANDAI_FCG_1_2)
							{
								m016.irqCounter = m016.irqLatch;
							}
							BREAK;
						}

						case 0x0B: // IRQ Counter Low Byte
						{
							if (submapper != SUB_MAPPER::BANDAI_FCG_1_2) // Submapper 5 updates Latch
							{
								m016.irqLatch = (m016.irqLatch & 0xFF00) | data;
							}
							else // Submapper 4 updates running Counter directly
							{
								m016.irqCounter = (m016.irqCounter & 0xFF00) | data;
							}
							BREAK;
						}

						case 0x0C: // IRQ Counter High Byte
						{
							if (submapper != SUB_MAPPER::BANDAI_FCG_1_2) // Submapper 5 updates Latch
							{
								m016.irqLatch = (m016.irqLatch & 0x00FF) | ((uint16_t)data << 8);
							}
							else // Submapper 4 updates running Counter directly
							{
								m016.irqCounter = (m016.irqCounter & 0x00FF) | ((uint16_t)data << 8);
							}
							BREAK;
						}

						case 0x0D: // Serial EEPROM Hardware Interface Link
						{
							if (submapper != SUB_MAPPER::BANDAI_FCG_1_2)
							{
								bool scl = (data & 0x20) != ZERO;
								bool sda = (data & 0x40) != ZERO;
								m016.lastScl = scl;
								m016.lastSda = sda;
							}
							BREAK;
						}
						}
						RETURN;
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_018:
				{
					auto& j18 = pNES_instance->NES_state.catridgeInfo.jaleco18;
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const FLAG isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

					// WRAM $6000-$7FFF
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						FLAG hasPrgRam = NO;
						if (isNES2)
						{
							const uint8_t vs = hdr.flags_8to15.nes2p0.flag10.fields.prgVolRam;
							const uint8_t nv = hdr.flags_8to15.nes2p0.flag10.fields.prgNonVolRam;
							hasPrgRam = (vs > ZERO) || (nv > ZERO);
						}
						else
						{
							hasPrgRam = !(hdr.flags_8to15.ines.flag10.fields.prgRamNotPresent == ONE);
						}

						if (hasPrgRam)
						{
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
						BREAK;
					}

					if (!IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						BREAK;
					}

					// Refer: https://www.nesdev.org/wiki/INES_Mapper_018
					// All registers use A0 to select nibble: A0=0 -> low nibble, A0=1 -> high nibble
					// data is masked to 4 bits (low nibble of the written byte)
					const FLAG      updateHi = (address & 0x01) ? YES : NO;
					const uint8_t   nibble = data & 0x0F;

					auto updatePrgBank = [&](uint8_t idx)
						{
							if (updateHi == NO)
								j18.prgBank[idx] = (j18.prgBank[idx] & 0xF0) | nibble;
							else
								j18.prgBank[idx] = (j18.prgBank[idx] & 0x0F) | (nibble << 4);
						};

					auto updateChrBank = [&](uint8_t idx)
						{
							if (updateHi == NO)
								j18.chrBank[idx] = (j18.chrBank[idx] & 0xF0) | nibble;
							else
								j18.chrBank[idx] = (j18.chrBank[idx] & 0x0F) | (nibble << 4);
						};

					switch (address & 0xF003)
					{
						// PRG banking
					case 0x8000: case 0x8001: updatePrgBank(0); BREAK;
					case 0x8002: case 0x8003: updatePrgBank(1); BREAK;
					case 0x9000: case 0x9001: updatePrgBank(2); BREAK;
						// $9002/$9003 unused per NESdev

						// CHR banking
					case 0xA000: case 0xA001: updateChrBank(0); BREAK;
					case 0xA002: case 0xA003: updateChrBank(1); BREAK;
					case 0xB000: case 0xB001: updateChrBank(2); BREAK;
					case 0xB002: case 0xB003: updateChrBank(3); BREAK;
					case 0xC000: case 0xC001: updateChrBank(4); BREAK;
					case 0xC002: case 0xC003: updateChrBank(5); BREAK;
					case 0xD000: case 0xD001: updateChrBank(6); BREAK;
					case 0xD002: case 0xD003: updateChrBank(7); BREAK;

						// IRQ reload nibbles
						// $E000=nibble0(bits3:0), $E001=nibble1(bits7:4), $E002=nibble2(bits11:8), $E003=nibble3(bits15:12)
					case 0xE000: j18.irqReload[0] = nibble; BREAK;
					case 0xE001: j18.irqReload[1] = nibble; BREAK;
					case 0xE002: j18.irqReload[2] = nibble; BREAK;
					case 0xE003: j18.irqReload[3] = nibble; BREAK;

					case 0xF000:
						// Acknowledge IRQ + reload counter from irqReload nibbles
						pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_VRC467 = RESET;
						j18.irqCounter = (uint16_t)(
							j18.irqReload[0]
							| (j18.irqReload[1] << 4)
							| (j18.irqReload[2] << 8)
							| (j18.irqReload[3] << 12));
						BREAK;

					case 0xF001:
						// Acknowledge IRQ + set counter size + enable flag
						pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_VRC467 = RESET;
						j18.irqEnabled = (nibble & 0x01) ? YES : NO;
						if (nibble & 0x08) j18.irqCounterSize = 3; // 4-bit
						else if (nibble & 0x04) j18.irqCounterSize = 2; // 8-bit
						else if (nibble & 0x02) j18.irqCounterSize = 1; // 12-bit
						else                    j18.irqCounterSize = 0; // 16-bit
						BREAK;

					case 0xF002:
						// Mirroring
						switch (nibble & 0x03)
						{
						case 0: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;  BREAK;
						case 1: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;    BREAK;
						case 2: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR; BREAK;
						case 3: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR; BREAK;
						}
						BREAK;

					case 0xF003:
						// Expansion sound — not supported
						BREAK;

					default:
						BREAK;
					}

					BREAK;
				}
				case MAPPER::VRC2_022:
				case MAPPER::VRC4_021:
				case MAPPER::VRC2_VRC4_023:
				case MAPPER::VRC2_VRC4_025:
				{
					auto& vrc24 = pNES_instance->NES_state.catridgeInfo.vrc24;
					const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
					const FLAG isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);

					// Calculate PRG RAM presence using your exact union layout fields
					FLAG hasPrgRam = false;
					if (isNES2)
					{
						const uint8_t volatileShift = hdr.flags_8to15.nes2p0.flag10.fields.prgVolRam;
						const uint8_t nvShift = hdr.flags_8to15.nes2p0.flag10.fields.prgNonVolRam;
						hasPrgRam = (volatileShift > ZERO) || (nvShift > ZERO);
					}
					else
					{
						const FLAG explicitlyNotPresent = (hdr.flags_8to15.ines.flag10.fields.prgRamNotPresent == ONE);
						hasPrgRam = !explicitlyNotPresent;
					}

					// -------------------------------------------------
					// WRAM / VRC2 Latch
					// -------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (isVRC2() == YES)
						{
							if (!hasPrgRam && IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, 0x6FFF))
							{
								// Any VRC2 configuration without prgRam writes to the 1-bit hardware latch, NOT to memory arrays.
								vrc24.latch = data & 0x01;
							}
							else
							{
								pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
							}
						}
						else if (isVRC4() == YES)
						{
							if (hasPrgRam && vrc24.wramEnable)
							{
								// VRC4 configuration: Standard PRG RAM behavior.
								pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
							}
						}
					}

					// -------------------------------------------------
					// Registers
					// -------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						uint32_t A0 = ZERO;
						uint32_t A1 = ZERO;
						switch (pNES_instance->NES_state.catridgeInfo.mapper)
						{
						case MAPPER::VRC2_022:
						{
							A0 = GETBIT(ONE, address);
							A1 = GETBIT(ZERO, address);
							BREAK;
						}
						case MAPPER::VRC4_021:
						{
							switch (pNES_instance->NES_state.catridgeInfo.subMapper)
							{
							case SUB_MAPPER::VRC4A:
							{
								A0 = GETBIT(ONE, address);
								A1 = GETBIT(TWO, address);
								BREAK;
							}
							case SUB_MAPPER::VRC4C:
							{
								A0 = GETBIT(SIX, address);
								A1 = GETBIT(SEVEN, address);
								BREAK;
							}
							default:
							{
								FATAL("Unknown VRC4_021 submapper : %d", pNES_instance->NES_state.catridgeInfo.subMapper);
							}
							}
							BREAK;
						}
						case MAPPER::VRC2_VRC4_023:
						{
							switch (pNES_instance->NES_state.catridgeInfo.subMapper)
							{
							case SUB_MAPPER::VRC4F:
							case SUB_MAPPER::VRC2B:
							{
								A0 = GETBIT(ZERO, address);
								A1 = GETBIT(ONE, address);
								BREAK;
							}
							case SUB_MAPPER::VRC4E:
							{
								A0 = GETBIT(TWO, address);
								A1 = GETBIT(THREE, address);
								BREAK;
							}
							default:
							{
								FATAL("Unknown VRC2_VRC4_023 submapper : %d", pNES_instance->NES_state.catridgeInfo.subMapper);
							}
							}
							BREAK;
						}
						case MAPPER::VRC2_VRC4_025:
						{
							switch (pNES_instance->NES_state.catridgeInfo.subMapper)
							{
							case SUB_MAPPER::VRC4B:
							case SUB_MAPPER::VRC2C:
							{
								A0 = GETBIT(ONE, address);
								A1 = GETBIT(ZERO, address);
								BREAK;
							}
							case SUB_MAPPER::VRC4D:
							{
								A0 = GETBIT(THREE, address);
								A1 = GETBIT(TWO, address);
								BREAK;
							}
							default:
							{
								FATAL("Unknown VRC2_VRC4_025 submapper : %d", pNES_instance->NES_state.catridgeInfo.subMapper);
							}
							}
							BREAK;
						}
						default:
						{
							BREAK;
						}
						}

						const uint32_t decodedAddress = (address & ~0x03) | (A1 << ONE) | A0;

						switch (decodedAddress & 0xF000)
						{
						case 0x8000:
						{
							vrc24.prgBank0 = data & 0x1F;
							BREAK;
						}
						case 0xA000:
						{
							vrc24.prgBank1 = data & 0x1F;
							BREAK;
						}
						case 0x9000:
						{
							if (isVRC4())
							{
								if ((decodedAddress & 0xF003) == 0x9000)
								{
									switch (data & 0x03)
									{
									case ZERO:  pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR; BREAK;
									case ONE:   pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR; BREAK;
									case TWO:   pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR; BREAK;
									case THREE: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR; BREAK;
									}
								}
								else if ((decodedAddress & 0xF003) == 0x9002)
								{
									vrc24.swapMode = (data >> ONE) & 0x01;
									vrc24.wramEnable = data & 0x01;
								}
							}
							else
							{
								vrc24.swapMode = ZERO;
								switch (data & 0x01)
								{
								case ZERO: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR; BREAK;
								case ONE:  pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR; BREAK;
								}
							}
							BREAK;
						}
						case 0xB000:
						case 0xC000:
						case 0xD000:
						case 0xE000:
						{
							if (IF_ADDRESS_WITHIN(decodedAddress, 0xB000, 0xEFFF))
							{
								const uint32_t chrRegion = (decodedAddress >> TWELVE) - 0x0B;
								if (chrRegion <= THREE)
								{
									const uint32_t subIndex = (decodedAddress >> ONE) & 0x01;
									const uint32_t reg = (chrRegion * TWO) + subIndex;

									if ((decodedAddress & 0x01) == ZERO)
									{
										// update low
										if (isVRC4() == YES)
										{
											vrc24.chrBank[reg] = (vrc24.chrBank[reg] & 0x1F0) | (data & 0x0F);
										}
										else
										{
											vrc24.chrBank[reg] = (vrc24.chrBank[reg] & 0x0F0) | (data & 0x0F);
										}
									}
									else
									{
										// update high
										// 5 bits
										if (isVRC4() == YES)
										{
											vrc24.chrBank[reg] = (vrc24.chrBank[reg] & 0x00F) | ((data & 0x1F) << FOUR);
											vrc24.chrBank[reg] &= 0x1FF;
										}
										// 4 bits
										else
										{
											vrc24.chrBank[reg] = (vrc24.chrBank[reg] & 0x00F) | ((data & 0x0F) << FOUR);
											vrc24.chrBank[reg] &= 0x0FF;
										}
									}
								}
							}
							BREAK;
						}
						case 0xF000:
						{
							if (isVRC4())
							{
								switch ((decodedAddress & 0xF003))
								{
								case 0xF000:
									vrc24.irqLatch = (vrc24.irqLatch & 0xF0) | (data & 0x0F);
									BREAK;
								case 0xF001:
									vrc24.irqLatch = (vrc24.irqLatch & 0x0F) | ((data & 0x0F) << 4);
									BREAK;
								case 0xF002:
									vrc24.prescaler = NES_TOTAL_PPU_CYCLES_PER_SCANLINE;
									vrc24.irqControl = data;
									if (GETBIT(ONE, data))
									{
										vrc24.irqCounter = vrc24.irqLatch;
									}
									pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_VRC467 = RESET;
									BREAK;
								case 0xF003:
									// Copy bit 0 to bit 1
									vrc24.irqControl = (vrc24.irqControl & ~0x02) | ((vrc24.irqControl & 0x01) << 1);
									pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_VRC467 = RESET;
									BREAK;
								default:
									BREAK;
								}
							}
						}
						default:
						{
							BREAK;
						}
						}
						BREAK;
					}
					BREAK;
				}
				case MAPPER::VRC6_024:
				case MAPPER::VRC6_026:
				{
					auto& vrc6 = pNES_instance->NES_state.catridgeInfo.vrc6;
					const MAPPER curMapper = pNES_instance->NES_state.catridgeInfo.mapper;
					const auto& hdrW = pINES->iNES_Fields.iNES_header.fields;
					const FLAG isNES2W = ((hdrW.flag7.raw & 0x0C) == 0x08);

					// -------------------------------------------------
					// WRAM (Cartridge RAM) Space: $6000 - $7FFF
					// Bit 7 of $B003 is the runtime WRAM enable gate
					// -------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						FLAG hasPrgRamW = false;
						if (isNES2W)
						{
							const uint8_t volatileShift = hdrW.flags_8to15.nes2p0.flag10.fields.prgVolRam;
							const uint8_t nvShift = hdrW.flags_8to15.nes2p0.flag10.fields.prgNonVolRam;
							hasPrgRamW = (volatileShift > ZERO) || (nvShift > ZERO);
						}
						else
						{
							hasPrgRamW = !(hdrW.flags_8to15.ines.flag10.fields.prgRamNotPresent == ONE);
						}

						const FLAG wramEnabledW = (vrc6.b003_reg & 0x80) != ZERO;
						if (hasPrgRamW && wramEnabledW)
						{
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
						BREAK;
					}

					// Normalize pin inversions between Mapper 24 and Mapper 26
					uint8_t regLine = address & 0x03;
					if (curMapper == MAPPER::VRC6_026)
					{
						// If address is $B002, (address & 0x01) is 0, (address & 0x02) is 2.
						// This turns it into: (0 << 1) | (2 >> 1) = 0 | 1 = 1 ($B001).
						regLine = ((address & 0x01) << 1) | ((address & 0x02) >> 1);
					}

					uint16_t baseRegisterRange = address & 0xF000;

					switch (baseRegisterRange)
					{
					case 0x8000: // PRG 16KB Block Selection ($8000-$8003 all select the same 16KB bank)
					{
						vrc6.prgBank0 = data & 0x0F;
						RETURN;
					}
					case 0x9000: // VRC6 Audio: Pulse 1 ($9000-$9002) / Halt+FreqShift ($9003)
					case 0xA000: // VRC6 Audio: Pulse 2 ($A000-$A002)
					{
						writeVRC6Audio(baseRegisterRange, regLine, data, address);
						RETURN;
					}
					case 0xB000:
					{
						if (regLine == 3) // $B003: Banking-mode / mirroring control register
						{
							vrc6.b003_reg = data;
							RETURN;
						}
						// $B000-$B002: VRC6 Audio sawtooth registers
						writeVRC6Audio(baseRegisterRange, regLine, data, address);
						RETURN;
					}
					case 0xC000: // PRG 8KB Selector Block Selection
					{
						vrc6.prgBank1 = data & 0x1F;
						RETURN;
					}
					case 0xD000: // CHR Bank Assignments 0 to 3
					{
						vrc6.chrBank[regLine] = data;
						RETURN;
					}
					case 0xE000: // CHR Bank Assignments 4 to 7
					{
						vrc6.chrBank[4 + regLine] = data;
						RETURN;
					}
					case 0xF000: // IRQ Subsystem Register Access Ports
					{
						switch (regLine)
						{
						case 0: // $F000: IRQ Latch
							vrc6.irqLatch = data;
							RETURN;
						case 1: // $F001: IRQ Control Config Enable
							vrc6.irqControl = data & 0x07;
							if (GETBIT(ONE, vrc6.irqControl))
							{
								vrc6.irqCounter = vrc6.irqLatch;
								vrc6.prescaler = NES_TOTAL_PPU_CYCLES_PER_SCANLINE;
							}
							pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_VRC467 = RESET;
							RETURN;
						case 2: // $F002: IRQ Acknowledge — copy E(bit0) into M(bit1), keep E, clear IRQ
							vrc6.irqControl = (vrc6.irqControl & ~0x02) | ((vrc6.irqControl & 0x01) << 1);
							pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_VRC467 = RESET;
							RETURN;
						}
						BREAK;
					}
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_028:
				{
					if (IF_ADDRESS_WITHIN(address, 0x5000, 0x5FFF))
					{
						// "7654 3210 / S       R" -- only bit7 (supervisor select) and bit0
						// (register within that bank) matter; this happens to exactly match
						// the register's own name (0x00/0x01/0x80/0x81).
						pNES_instance->NES_state.catridgeInfo.ines028.selectedReg = data & 0x81;
						BREAK;
					}

					if (IF_ADDRESS_WITHIN(address, 0x8000, 0xFFFF))
					{
						auto& reg028 = pNES_instance->NES_state.catridgeInfo.ines028;

						switch (reg028.selectedReg)
						{
						case 0x00: reg028.reg00_chrBank = data; BREAK;
						case 0x01: reg028.reg01_innerBank = data; BREAK;
						case 0x80: reg028.reg80_mode = data; BREAK;
						case 0x81: reg028.reg81_outerBank = data; BREAK;
						default: BREAK; // no register selected yet -- write has no effect
						}

						// Mirroring resolution -- shared by writes to any of the 4
						// registers, since $80 can change the mode directly and $00/$01 can
						// override bit 0 while in one of the 1-screen modes.
						const BYTE mirrorModeBits = reg028.reg80_mode & 0x03;
						if (mirrorModeBits == ZERO || mirrorModeBits == ONE)
						{
							BYTE bit0 = mirrorModeBits & 0x01;
							if (reg028.selectedReg == 0x00 || reg028.selectedReg == 0x01)
							{
								bit0 = (data >> 4) & 0x01;
								reg028.reg80_mode = (reg028.reg80_mode & (BYTE)~0x01) | bit0;
							}
							pNES_instance->NES_state.catridgeInfo.nameTblMir =
								bit0 ? NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR : NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
						}
						else if (mirrorModeBits == TWO)
						{
							pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
						}
						else
						{
							pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
						}
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_029:
				{
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						BREAK;
					}

					if (IF_ADDRESS_WITHIN(address, 0x8000, 0xFFFF))
					{
						auto& reg029 = pNES_instance->NES_state.catridgeInfo.ines029;
						reg029.prgBank16 = (data >> 2) & 0x07;
						reg029.chrBank8 = data & 0x03;
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_030:
				{
					auto& reg030 = pNES_instance->NES_state.catridgeInfo.ines030;
					const BYTE submapperRaw = (BYTE)pNES_instance->NES_state.catridgeInfo.subMapper;

					// TODO: confirm the actual field name for the iNES "battery present"
					// bit (byte 6, bit 1) in your header struct -- placeholder below.
					const bool batteryBitSet = (pINES->iNES_Fields.iNES_header.fields.flag6.raw & 0x02) != ZERO;

					const bool mainRegAtC000Only = mapper030MainRegAtC000Only(submapperRaw, batteryBitSet);
					const bool hasBusConflicts = mapper030HasBusConflicts(submapperRaw, batteryBitSet);

					// $8000-$BFFF: only meaningful once the main register has moved to
					// $C000-only (i.e. a flashable/battery config).
					if (mainRegAtC000Only && IF_ADDRESS_WITHIN(address, 0x8000, 0xBFFF))
					{
						if (submapperRaw == FOUR)
						{
							reg030.ledReg = data; // cosmetic only -- see caveats
						}
						// Flash ROM self-reprogramming (submappers 0/1/4 with battery bit)
						// is NOT implemented: it would require emulating the flash chip's
						// command/erase sequence and persisting the rewritten PRG image.
						// Writes here are silently dropped rather than corrupting anything.
						BREAK;
					}

					if ((mainRegAtC000Only && IF_ADDRESS_WITHIN(address, 0xC000, 0xFFFF))
						|| (!mainRegAtC000Only && IF_ADDRESS_WITHIN(address, 0x8000, 0xFFFF)))
					{
						if (hasBusConflicts)
						{
							data &= readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
						}

						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;

						reg030.prgBank16 = (BYTE)((data & 0x1F) % totalPrg16kBanks);
						reg030.chrBank8 = (data >> 5) & 0x03;
						reg030.nametableBit = (data >> 7) & 0x01;

						if (reg030.ntMode == INES030_NT_MODE::ONESCREEN_SWITCHABLE)
						{
							pNES_instance->NES_state.catridgeInfo.nameTblMir =
								reg030.nametableBit ? NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR : NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
						}
						else if (reg030.ntMode == INES030_NT_MODE::SUBMAPPER3_HV_SWITCHABLE)
						{
							pNES_instance->NES_state.catridgeInfo.nameTblMir =
								reg030.nametableBit ? NAMETABLE_MIRROR::VERTICAL_MIRROR : NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
						}
						// FIXED_VERTICAL / FIXED_HORIZONTAL / FOUR_SCREEN_CART_VRAM: N has
						// no effect, nameTblMir was already latched once at init.
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_034:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;

						if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::NINA)
						{
							if (address == 0x7FFD || address == 0x7FFE || address == 0x7FFF)
							{
								const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
								const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
								const uint32_t totalPrg16kBanks = isNES2
									? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
									: hdr.sizeOfPrgRomIn16KB;
								const uint32_t totalChr8kBanks = isNES2
									? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
									: hdr.sizeOfChrRomIn8KB;
								// INES_MAPPER_034 switches 32KB PRG and 4KB CHR banks
								const uint32_t totalPrg32kBanks = totalPrg16kBanks / 2;
								const uint32_t totalChr4kBanks = totalChr8kBanks * 2;

								switch (address)
								{
								case 0x7FFD:
								{
									pNES_instance->NES_state.catridgeInfo.ines034.prgBank32 = (data & 0x03) % totalPrg32kBanks;
									BREAK;
								}
								case 0x7FFE:
								{
									pNES_instance->NES_state.catridgeInfo.ines034.chrBank4Lo = (data & 0x0F) % totalChr4kBanks;
									BREAK;
								}
								case 0x7FFF:
								{
									pNES_instance->NES_state.catridgeInfo.ines034.chrBank4Hi = (data & 0x0F) % totalChr4kBanks;
									BREAK;
								}
								}
							}
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::BNROM)
						{
							if (pNES_instance->NES_state.catridgeInfo.isBusConflictPresent)
							{
								data &= readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
							}

							const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
							const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
							const uint32_t totalPrg16kBanks = isNES2
								? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
								: hdr.sizeOfPrgRomIn16KB;
							// INES_MAPPER_034 switches 32KB PRG
							const uint32_t totalPrg32kBanks = totalPrg16kBanks / 2;
							pNES_instance->NES_state.catridgeInfo.ines034.prgBank32 = (data & 0x03) % totalPrg32kBanks;
						}
					}
					BREAK;
				}
				case MAPPER::GxROM:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.hasPrgRam)
						{
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;
						const uint32_t totalChr8kBanks = isNES2
							? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
							: hdr.sizeOfChrRomIn8KB;
						// GxROM switches 32KB PRG and 8KB CHR banks
						const uint32_t totalPrg32kBanks = totalPrg16kBanks / 2;
						pNES_instance->NES_state.catridgeInfo.gxrom.chrBank = (data & 0x03) % totalChr8kBanks;
						pNES_instance->NES_state.catridgeInfo.gxrom.prgBank = ((data >> FOUR) & 0x03) % totalPrg32kBanks;
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_067:
				{
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if ((address & 0x8800) == 0x8000)
						{
							// IRQ Acknowledge
							// Clear pending IRQ and deassert mapper IRQ line.
							pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_SUNSOFT = CLEAR;
						}

						switch (address & 0xF800)
						{
						case 0x8800:
							// CHR bank 0 ($0000-$07FF)
							pNES_instance->NES_state.catridgeInfo.ines_067_068.chrBank0 = data & 0x3F;
							BREAK;
						case 0x9800:
							// CHR bank 1 ($0800-$0FFF)
							pNES_instance->NES_state.catridgeInfo.ines_067_068.chrBank1 = data & 0x3F;
							BREAK;
						case 0xA800:
							// CHR bank 2 ($1000-$17FF)
							pNES_instance->NES_state.catridgeInfo.ines_067_068.chrBank2 = data & 0x3F;
							BREAK;
						case 0xB800:
							// CHR bank 3 ($1800-$1FFF)
							pNES_instance->NES_state.catridgeInfo.ines_067_068.chrBank3 = data & 0x3F;
							BREAK;
						case 0xC800:
							// IRQ Load (write high byte then low byte)
							if (pNES_instance->NES_state.catridgeInfo.ines_067_068.writeState == FALSE)
							{
								pNES_instance->NES_state.catridgeInfo.ines_067_068.irqCounter
									= (pNES_instance->NES_state.catridgeInfo.ines_067_068.irqCounter & 0x00FF) | ((uint16_t)data << 8);
								pNES_instance->NES_state.catridgeInfo.ines_067_068.writeState = TRUE;

							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.ines_067_068.irqCounter
									= (pNES_instance->NES_state.catridgeInfo.ines_067_068.irqCounter & 0xFF00) | data;
								pNES_instance->NES_state.catridgeInfo.ines_067_068.writeState = FALSE;
							}
							BREAK;
						case 0xD800:
							// IRQ Enable / Counter Control
							pNES_instance->NES_state.catridgeInfo.ines_067_068.irqCounterEnable = (data & 0x10) ? YES : NO;
							// - Reset IRQ load latch state.
							// - Do NOT acknowledge IRQ here.
							pNES_instance->NES_state.catridgeInfo.ines_067_068.writeState = FALSE;
							BREAK;
						case 0xE800:
						{
							BYTE mirroringMode = data & 0x03;

							if (mirroringMode == 0)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
							}
							else if (mirroringMode == 1)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
							}
							else if (mirroringMode == 2)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR;
							}
							BREAK;
						}
						case 0xF800:
							pNES_instance->NES_state.catridgeInfo.ines_067_068.prgBank = data & 0x0F;
							BREAK;
						}
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_068:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (address >= CATRIDGE_RAM_START_ADDRESS && !pNES_instance->NES_state.catridgeInfo.ines_067_068.prgRamEnable)
						{
							RETURN;
						}
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						switch ((address & 0xF000))
						{
						case 0x8000:
							pNES_instance->NES_state.catridgeInfo.ines_067_068.chrBank0 = data;
							BREAK;
						case 0x9000:
							pNES_instance->NES_state.catridgeInfo.ines_067_068.chrBank1 = data;
							BREAK;
						case 0xA000:
							pNES_instance->NES_state.catridgeInfo.ines_067_068.chrBank2 = data;
							BREAK;
						case 0xB000:
							pNES_instance->NES_state.catridgeInfo.ines_067_068.chrBank3 = data;
							BREAK;
						case 0xC000:
							// Hardware treats D7 as 1; forces bank choice into upper 128KB
							pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank0 = data | 0x80;
							BREAK;
						case 0xD000:
							pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank1 = data | 0x80;
							BREAK;
						case 0xE000:
						{
							pNES_instance->NES_state.catridgeInfo.ines_067_068.ntControl = data;

							// Mirroring mode is determined by bits 0 and 1 of ntControl
							BYTE mirroringMode = data & 0x03;
							if (mirroringMode == 0)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
							}
							else if (mirroringMode == 1)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
							}
							else if (mirroringMode == 2)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
							}
							else if (mirroringMode == 3)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR;
							}
							BREAK;
						}
						case 0xF000:
							pNES_instance->NES_state.catridgeInfo.ines_067_068.prgBank = data & 0x0F;
							pNES_instance->NES_state.catridgeInfo.ines_067_068.prgRamEnable = (data & 0x10) ? YES : NO;
							BREAK;
						}
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_069:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						if (address >= CATRIDGE_RAM_START_ADDRESS)
						{
							if (pNES_instance->NES_state.catridgeInfo.ines069.ramMode)
							{
								if (pNES_instance->NES_state.catridgeInfo.ines069.prgRamEnable)
								{
									pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
								}
							}
						}
						else
						{
							pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
						}
					}

					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, 0x9FFF))
						{
							pNES_instance->NES_state.catridgeInfo.ines069.command = data & 0x0F;
						}
						else if (IF_ADDRESS_WITHIN(address, 0xA000, CATRIDGE_ROM_BANK0_END_ADDRESS))
						{
							auto& ines069 = pNES_instance->NES_state.catridgeInfo.ines069;
							switch (ines069.command)
							{
							case 0: case 1: case 2: case 3:
							case 4: case 5: case 6: case 7:
								ines069.chrBank[ines069.command] = data;
								BREAK;
							case 8:
								ines069.prgBank[ZERO] = data & 0x3F;
								ines069.ramMode = GETBIT(SIX, data);
								ines069.prgRamEnable = GETBIT(SEVEN, data);
								BREAK;
							case 9: case 0x0A: case 0x0B:
								ines069.prgBank[ines069.command - EIGHT] = data & 0x3F;
								BREAK;
							case 0x0C:
								ines069.nm = data & 0x03;
								if (ines069.nm == 0)
								{
									pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
								}
								else if (ines069.nm == 1)
								{
									pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
								}
								else if (ines069.nm == 2)
								{
									pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
								}
								else if (ines069.nm == 3)
								{
									pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR;
								}
								BREAK;
							case 0x0D:
								ines069.irqEnable = GETBIT(ZERO, data);
								ines069.irqCounterEnable = GETBIT(SEVEN, data);
								pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_SUNSOFT = CLEAR;
								BREAK;
							case 0x0E:
								ines069.irqCounter = (ines069.irqCounter & 0xFF00) | data;
								BREAK;
							case 0x0F:
								ines069.irqCounter = (ines069.irqCounter & 0x00FF) | ((uint16_t)data << 8);
								BREAK;
							default:
								FATAL("Unknown Sunsoft_FME-7 command");
								BREAK;
							}
						}
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_070:
				case MAPPER::INES_MAPPER_152:
				{
					if (IF_ADDRESS_WITHIN(address, UNMAPPED_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[address - UNMAPPED_START_ADDRESS] = data;
					}
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.isBusConflictPresent)
						{
							data &= readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
						}

						{
							const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
							const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
							const uint32_t totalPrg16kBanks = isNES2
								? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
								: hdr.sizeOfPrgRomIn16KB;
							const uint32_t totalChr8kBanks = isNES2
								? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
								: hdr.sizeOfChrRomIn8KB;

							pNES_instance->NES_state.catridgeInfo.ines_070_152.prgReg = ((data >> 4) & 0x07) % totalPrg16kBanks;
							pNES_instance->NES_state.catridgeInfo.ines_070_152.chrReg = (data & 0x0F) % totalChr8kBanks;
							pNES_instance->NES_state.catridgeInfo.ines_070_152.m = (data >> 7) & 0x01;

							if (pNES_instance->NES_state.catridgeInfo.ines_070_152.m)
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR;
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
							}
						}
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_078:
				{
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_ROM_BANK0_START_ADDRESS, UNMAPPED_END_ADDRESS))
					{
						if (pNES_instance->NES_state.catridgeInfo.isBusConflictPresent)
						{
							data &= readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::DEBUG_PORT);
						}

						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;
						const uint32_t totalChr8kBanks = isNES2
							? (hdr.sizeOfChrRomIn8KB | (hdr.flags_8to15.nes2p0.flag9.fields.chrRomMSB << 8))
							: hdr.sizeOfChrRomIn8KB;

						auto& reg078 = pNES_instance->NES_state.catridgeInfo.ines078;
						reg078.prgBank16 = (BYTE)((data & 0x07) % totalPrg16kBanks);
						reg078.chrBank8 = (BYTE)(((data >> 4) & 0x0F) % totalChr8kBanks);

						const BIT mBit = (data >> 3) & 0x01;
						const BYTE submapperRaw = (BYTE)pNES_instance->NES_state.catridgeInfo.subMapper;

						if (submapperRaw == THREE)
						{
							// Holy Diver: M is a straight H/V select.
							pNES_instance->NES_state.catridgeInfo.nameTblMir =
								mBit ? NAMETABLE_MIRROR::VERTICAL_MIRROR : NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
						}
						else
						{
							// Submapper 1 (Uchuusen/Cosmo Carrier), or plain iNES 1.0 with no
							// submapper info at all: AxROM-style fixed single-screen select.
							// The wiki notes these two mirroring schemes are NOT
							// cross-compatible -- Nestopia 1.4.0/FCEUX 2.1.5 both default to
							// this one-screen interpretation for the ambiguous case, so
							// that's the default here too.
							pNES_instance->NES_state.catridgeInfo.nameTblMir =
								mBit ? NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR : NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
						}
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
				case MAPPER::INES_MAPPER_019:
				case MAPPER::INES_MAPPER_210:
				{
					auto& n163 = pNES_instance->NES_state.catridgeInfo.namco163;

					// Helper lambda: auto-detect variant; mirrors Mesen SetVariant()
					// - If autoDetect is NO, never changes variant
					// - If notNamco340=YES, refuses to set variant=2 (Namco340)
					// Ref: Mesen Namco163::SetVariant()
					const auto setVariant = [&](BYTE v)
						{
							if (n163.autoDetect == NO) return;
							if (n163.notNamco340 == YES && v == 2) return;
							n163.variant = v;
						};

					// -----------------------------------------------------------
					// $4800-$4FFF: Audio RAM data port (write + optional auto-increment)
					// Writing here signals this is a Namco163 (not 175/340)
					// Ref: Mesen Namco163::WriteRegister() case 0x4800
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0x4800, 0x4FFF))
					{
						setVariant(0); // confirms Namco163
						n163.audioRam[n163.audioRamAddr & 0x7F] = data;
						if (n163.audioAutoInc == YES)
							n163.audioRamAddr = (n163.audioRamAddr + 1) & 0x7F;
						BREAK;
					}

					// -----------------------------------------------------------
					// $5000-$57FF: IRQ counter low byte + acknowledge
					// Also signals Namco163
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#IRQ_Counter_(low)
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0x5000, 0x57FF))
					{
						setVariant(0);
						n163.irqCounter = (n163.irqCounter & 0xFF00) | data;
						pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_NAMECO163 = RESET;
						BREAK;
					}

					// -----------------------------------------------------------
					// $5800-$5FFF: IRQ counter high byte + enable bit + acknowledge
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#IRQ_Counter_(high)
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0x5800, 0x5FFF))
					{
						setVariant(0);
						n163.irqCounter = (n163.irqCounter & 0x00FF) | ((uint16_t)data << 8);
						pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_NAMECO163 = RESET;
						BREAK;
					}

					// -----------------------------------------------------------
					// $6000-$7FFF: WRAM write
					// Any write here proves it is NOT Namco340 (Namco340 has no WRAM)
					// Ref: Mesen Namco163::WriteRam()
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, CATRIDGE_RAM_START_ADDRESS, CATRIDGE_RAM_END_ADDRESS))
					{
						n163.notNamco340 = YES;
						if (n163.variant == 2)   // was Namco340 — demote to Unknown
							setVariant(3);

						if (n163.variant == 0 || n163.variant == 1)
						{
							// Check write-protect per-variant
							bool canWrite = false;
							if (n163.variant == 0)   // Namco163: global enable + per-window bits
							{
								const bool globalEn = (n163.writeProtect & 0x40) != 0;
								if (globalEn)
								{
									const uint8_t windowBit = (uint8_t)((address - CATRIDGE_RAM_START_ADDRESS) >> 11); // 0..3
									canWrite = (n163.writeProtect & (1u << windowBit)) == 0; // 0 = writable
								}
							}
							else                     // Namco175: single bit
							{
								canWrite = (n163.writeProtect & 0x01) != 0;
							}

							if (canWrite)
							{
								const uint16_t offset = (uint16_t)(address - CATRIDGE_RAM_START_ADDRESS);
								n163.prgRam[offset & 0x1FFF] = data;
							}
						}
						BREAK;
					}

					// -----------------------------------------------------------
					// $8000-$9FFF: CHR banks 0–3 (PPU $0000-$0FFF)
					// One 1KB bank per $0800 address step (4 registers)
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#CHR_and_NT_Select
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0x8000, 0x9FFF))
					{
						const uint8_t slot = (uint8_t)((address - 0x8000) >> 11); // 0..3
						n163.chrBanks[slot] = data;
						BREAK;
					}

					// -----------------------------------------------------------
					// $A000-$BFFF: CHR banks 4–7 (PPU $1000-$1FFF)
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0xA000, 0xBFFF))
					{
						const uint8_t slot = (uint8_t)(((address - 0xA000) >> 11) + 4); // 4..7
						n163.chrBanks[slot] = data;
						BREAK;
					}

					// -----------------------------------------------------------
					// $C000-$DFFF: CHR banks 8–11 (PPU $2000-$2FFF = nametables)
					// Variant detection:
					//   $C800-$DFFF write => must be Namco163 (has 4 NT bank regs)
					//   $C000-$C7FF write + not already Namco163 => Namco175
					// On Namco175, $C000 is the WRAM write-protect register (not a CHR bank)
					// Ref: Mesen Namco163::WriteRegister() case 0xC000
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0xC000, 0xDFFF))
					{
						if (address >= 0xC800)
							setVariant(0); // confirmed Namco163
						else if (n163.variant != 0)
							setVariant(1); // likely Namco175

						if (n163.variant == 1)  // Namco175: $C000 = WRAM write-protect
						{
							n163.writeProtect = data;
						}
						else  // Namco163 (or auto-detect): CHR/NT bank registers
						{
							const uint8_t slot = (uint8_t)(((address - 0xC000) >> 11) + 8); // 8..11
							n163.chrBanks[slot] = data;
						}
						BREAK;
					}

					// -----------------------------------------------------------
					// $E000-$E7FF: PRG bank 0 ($8000-$9FFF) + sound enable + Namco340 detect
					// bits[5:0] = 8KB PRG bank; bit 6 = disable audio (Namco163 only)
					// bit 7 = 1 or bit 6 = 1 (not already Namco163) => Namco340 detect
					// Namco340: bits[7:6] also control mirroring
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#PRG_Select_1
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0xE000, 0xE7FF))
					{
						if ((data & 0x80) != 0)
							setVariant(2); // Namco340 detection: bit 7 set
						else if ((data & 0x40) != 0 && n163.variant != 0)
							setVariant(2); // bit 6 set + not confirmed Namco163

						n163.prgBanks[0] = data & 0x3F;

						if (n163.variant == 2)  // Namco340: mirroring via bits[7:6]
						{
							switch ((data & 0xC0) >> 6)
							{
							case 0: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR; BREAK;
							case 1: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;        BREAK;
							case 2: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;      BREAK;
							case 3: pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR; BREAK;
							}
						}
						else if (n163.variant == 0)  // Namco163: bit 6 = audio disable
						{
							// Ref: https://www.nesdev.org/wiki/Namco_163_audio#Sound_Enable
							n163.audioDisable = ((data & 0x40) != 0) ? YES : NO;
						}
						BREAK;
					}

					// -----------------------------------------------------------
					// $E800-$EFFF: PRG bank 1 ($A000-$BFFF) + CHR-NT mode flags
					// bits[5:0] = bank; bit 6 = lowChrNtMode; bit 7 = highChrNtMode
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#PRG_Select_2
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0xE800, 0xEFFF))
					{
						n163.prgBanks[1] = data & 0x3F;
						if (n163.variant == 0)
						{
							n163.lowChrNtMode = ((data & 0x40) != 0) ? YES : NO;
							n163.highChrNtMode = ((data & 0x80) != 0) ? YES : NO;
						}
						BREAK;
					}

					// -----------------------------------------------------------
					// $F000-$F7FF: PRG bank 2 ($C000-$DFFF)
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#PRG_Select_3
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0xF000, 0xF7FF))
					{
						n163.prgBanks[2] = data & 0x3F;
						BREAK;
					}

					// -----------------------------------------------------------
					// $F800-$FFFF: Write-protect register + audio address port
					// This also confirms Namco163 variant
					// bits[6:0] = audio RAM address; bit 7 = auto-increment enable
					// writeProtect stores the full byte for WRAM gating
					// Ref: https://www.nesdev.org/wiki/INES_Mapper_019#Write_Protect
					// Ref: https://www.nesdev.org/wiki/Namco_163_audio#Address_Port
					// -----------------------------------------------------------
					if (IF_ADDRESS_WITHIN(address, 0xF800, 0xFFFF))
					{
						setVariant(0);
						n163.writeProtect = data;
						n163.audioRamAddr = data & 0x7F;
						n163.audioAutoInc = ((data & 0x80) != 0) ? YES : NO;
						BREAK;
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_232:
				{
					if (address >= CATRIDGE_ROM_BANK0_START_ADDRESS)
					{
						auto& m232 = pNES_instance->NES_state.catridgeInfo.ines232;

						if (address <= 0xBFFF)
						{
							// Outer Register: Bits 3 and 4 select 64 KiB outer PRG block
							m232.outerBank = (data >> 3) & 0x03;
						}
						else // 0xC000 - 0xFFFF
						{
							// Inner Register: Bits 0 and 1 select 16 KiB inner bank for $8000-$BFFF
							m232.innerBank = data & 0x03;
						}

						const auto& hdr = pINES->iNES_Fields.iNES_header.fields;
						const bool isNES2 = ((hdr.flag7.raw & 0x0C) == 0x08);
						const uint32_t totalPrg16kBanks = isNES2
							? (hdr.sizeOfPrgRomIn16KB | (hdr.flags_8to15.nes2p0.flag9.fields.prgRomMSB << 8))
							: hdr.sizeOfPrgRomIn16KB;

						const uint32_t maxBanks = totalPrg16kBanks > ZERO ? totalPrg16kBanks : ONE;

						// $8000-$BFFF points to selected 16 KiB page in block
						m232.prgBank8000 = ((m232.outerBank * 4) + m232.innerBank) % maxBanks;
						// $C000-$FFFF is hardwired to the last (3rd) 16 KiB page of the selected block
						m232.prgBankC000 = ((m232.outerBank * 4) + 3) % maxBanks;
					}
					BREAK;
				}
				default:
				{
					FATAL("Write performed for unsupported mapper");
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

void NES_t::writeCpuRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source)
{
	writeCpuRawMemoryInternal(address, data, source);
	pNES_cpuRegisters->openbus = data;
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

	// NMI edge trigger requires VBL to be set, or have been set just moments before to generate the edge trigger
	if (pNES_instance->NES_state.interrupts.isNMI != YES)
	{
		RETURN EXCEPTION_EVENT_TYPE::EVENT_NONE;
	}
	// NMI suppression: VBLANK_NMI_ENABLE was cleared right at the VBL edge � cancel NMI
	else if (isNMI_SuppressedAtVBLEdge(ly, ppuCycle))
	{
		// Reset counters
		pNES_instance->NES_state.interrupts.nmiDelayInInstructions = RESET;
		// Clear the NMI flag
		pNES_instance->NES_state.interrupts.isNMI = CLEAR;
		RETURN EXCEPTION_EVENT_TYPE::EVENT_NONE;
	}
	// Handle NMI
	// Interrupt is checked only during the final tick of an opcode, hence can be executed only when we are about to fetch the next opcode
	// Refer https://www.reddit.com/r/EmuDev/comments/16y1ilc/comment/k362vo1/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
	// Refer https://forums.nesdev.org/viewtopic.php?p=177408#p177408
	// Refer https://forums.nesdev.org/viewtopic.php?p=177414#p177414
	// Refer https://forums.nesdev.org/viewtopic.php?p=177516#p177516 (this is an interesting post)
	// Also handles for 7.nmi_timing.nes, 04-nmi_control and 05-nmi_timing.nes
	else if (isNMI_ReadyToDispatch(ly, ppuCycle))
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
	CPUWARN("CPU Panic; unknown opcode! %02X", pNES_cpuInstance->opcode);
}
#pragma endregion RP2A03_DEFINITIONS

#pragma region EMULATION_DEFINITIONS
void NES_t::clockVRC467IRQ(void)
{
	if (!(isVRC4() || isVRC6() || isVRC7()))
	{
		RETURN;
	}

	if (isVRC4())
	{
		auto& vrc24 = pNES_instance->NES_state.catridgeInfo.vrc24;

		// E bit
		if (!GETBIT(ONE, vrc24.irqControl))
		{
			RETURN;
		}

		vrc24.prescaler -= THREE;

		const FLAG cycleMode = GETBIT(TWO, vrc24.irqControl);

		if (cycleMode || (vrc24.prescaler <= ZERO))
		{
			if (vrc24.irqCounter == 0xFF)
			{
				vrc24.irqCounter = vrc24.irqLatch;

				pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_VRC467 = SET;
			}
			else
			{
				++vrc24.irqCounter;
			}

			vrc24.prescaler += NES_TOTAL_PPU_CYCLES_PER_SCANLINE;
		}
	}
	else if (isVRC6())
	{
		auto& vrc6 = pNES_instance->NES_state.catridgeInfo.vrc6;

		// E Bit (Bit 1) - Subsystem Execution Interrupt Enable Check
		if (!GETBIT(ONE, vrc6.irqControl))
		{
			RETURN;
		}

		// Process down step via architectural CPU tick cycle weights
		vrc6.prescaler -= THREE;

		// M Bit (Bit 2) - Coarse Mode Timing Select Flag
		const FLAG cycleMode = GETBIT(TWO, vrc6.irqControl);

		if (cycleMode || (vrc6.prescaler <= ZERO))
		{
			if (vrc6.irqCounter == 0xFF)
			{
				vrc6.irqCounter = vrc6.irqLatch;

				// Flags the target line directly shared with your VRC4 layout components
				pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_VRC467 = SET;
			}
			else
			{
				++vrc6.irqCounter;
			}

			// Reload internal scanline pool counters
			vrc6.prescaler += NES_TOTAL_PPU_CYCLES_PER_SCANLINE;
		}
	}
}

void NES_t::clockJalecoIRQ(void)
{
	if (!(isJaleco18()))
	{
		RETURN;
	}

	if (isJaleco18())
	{
		auto& j18 = pNES_instance->NES_state.catridgeInfo.jaleco18;

		if (j18.irqEnabled == NO)
		{
			RETURN;
		}

		// IRQ mask table: index 0=16-bit, 1=12-bit, 2=8-bit, 3=4-bit
		static constexpr uint16_t IRQ_MASK[4] = { 0xFFFF, 0x0FFF, 0x00FF, 0x000F };
		const uint16_t mask = IRQ_MASK[j18.irqCounterSize];

		// Decrement only the active bits; preserve the upper bits unchanged
		uint16_t activeBits = j18.irqCounter & mask;

		if (activeBits == 0)
		{
			// Already at zero — don't wrap through, just stay (fire already sent)
			RETURN;
		}

		--activeBits;

		j18.irqCounter = (j18.irqCounter & ~mask) | (activeBits & mask);

		if (activeBits == 0)
		{
			pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_VRC467 = SET;
		}

		RETURN;
	}
}

void NES_t::clockINES016IRQ(void)
{
	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_016)
	{
		auto& m016 = pNES_instance->NES_state.catridgeInfo.ines016;

		if (m016.irqCountingEnable)
		{
			if (m016.irqCounter == ZERO)
			{
				pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_INES016 = SET;
				m016.irqCounter = 0xFFFF;
			}
			else
			{
				--m016.irqCounter;
			}
		}
	}
}

void NES_t::clockINES067IRQ(void)
{
	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_067)
	{
		auto& m067 = pNES_instance->NES_state.catridgeInfo.ines_067_068;

		if (m067.irqCounterEnable)
		{
			if (m067.irqCounter == ZERO)
			{
				pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_SUNSOFT = SET;
				m067.irqCounterEnable = NO;
				m067.irqCounter = 0xFFFF;
			}
			else
			{
				--m067.irqCounter;
			}
		}
	}
}

void NES_t::clockINES069IRQ(void)
{
	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_069)
	{
		auto& m069 = pNES_instance->NES_state.catridgeInfo.ines069;

		if (m069.irqCounterEnable)
		{
			if (m069.irqCounter == ZERO)
			{
				pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_SUNSOFT = m069.irqEnable;
				m069.irqCounter = 0xFFFF;
			}
			else
			{
				--m069.irqCounter;
			}
		}
	}
}

void NES_t::clockINES105IRQ(void)
{
	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_105)
	{
		auto& mmc1 = pNES_instance->NES_state.catridgeInfo.mmc1;
		auto& ev = mmc1.nes_event;

		if (ev.irqEnabled == YES)
		{
			++ev.irqCounter;

			const uint32_t threshold = 0x20000000U | ((uint32_t)(ev.dipSwitches & 0x0F) << 25);
			if (ev.irqCounter >= threshold)
			{
				// Fire IRQ
				pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC1 = SET;
				ev.irqEnabled = NO; // counter stops after firing
			}
		}
	}
}

// clockMMC5 now only handles the PPU idle counter; 
// audio is fully driven by apuTick()
void NES_t::clockMMC5(void)
{
	if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC5)
	{
		auto& mmc5 = pNES_instance->NES_state.catridgeInfo.mmc5;

		if (mmc5.ppuIdleCounter > ZERO)
		{
			mmc5.ppuIdleCounter--;
			if (mmc5.ppuIdleCounter == ZERO)
			{
				mmc5.ppuInFrame = NO;
				updateMMC5ChrA();
			}
		}
		// Audio: all ticking now happens inside apuTick()
	}
}

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

		// Handle DMC DMA (Refer : https://www.nesdev.org/wiki/DMA, https://forums.nesdev.org/viewtopic.php?t=14120)
		// TODO: Simplified vs. real hardware: does not yet model the "DMC lands mid-OAM-DMA -> only 2 cycles" overlap case,
		// only the standalone 3-4 cycle stall. Mirrors the OAM DMA halt pattern below using the same dmaGetPutCounter.
		auto& dmc = pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc;
		if (dmc.dmcDmaPending == YES && cycleType == CYCLES_TYPE::READ_CYCLE)
		{
			// Halt-attempt cycle: this READ_CYCLE itself is the hijacked read (point 1 of the forum thread above)
			++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;
			clockVRC467IRQ();
			clockJalecoIRQ();
			clockINES016IRQ();
			clockINES067IRQ();
			clockINES069IRQ();
			clockINES105IRQ();
			clockRambo1CpuIRQ();
			clockNamco163IRQ();
			clockMMC5();
			tickPpuForOneCpuCycle();
			apuTick();
			joypadTick();

			// DMC-only dummy read (point 2)
			++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;
			clockVRC467IRQ();
			clockJalecoIRQ();
			clockINES016IRQ();
			clockINES067IRQ();
			clockINES069IRQ();
			clockINES105IRQ();
			clockRambo1CpuIRQ();
			clockNamco163IRQ();
			clockMMC5();
			tickPpuForOneCpuCycle();
			apuTick();
			joypadTick();

			// Alignment cycle if landed on a "put" cycle (point 3)
			if (GETBIT(ZERO, pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter) == SET)
			{
				++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;
				clockVRC467IRQ();
				clockJalecoIRQ();
				clockINES016IRQ();
				clockINES067IRQ();
				clockINES069IRQ();
				clockINES105IRQ();
				clockRambo1CpuIRQ();
				clockNamco163IRQ();
				clockMMC5();
				tickPpuForOneCpuCycle();
				apuTick();
				joypadTick();
			}

			// Actual DMA read (point 4)
			++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;
			completeDmcDmaFetch();
			clockVRC467IRQ();
			clockJalecoIRQ();
			clockINES016IRQ();
			clockINES067IRQ();
			clockINES069IRQ();
			clockINES105IRQ();
			clockRambo1CpuIRQ();
			clockNamco163IRQ();
			clockMMC5();
			tickPpuForOneCpuCycle();
			apuTick();
			joypadTick();

			++pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter;
			RETURN;
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
			clockVRC467IRQ();
			clockJalecoIRQ();
			clockINES016IRQ();
			clockINES067IRQ();
			clockINES069IRQ();
			clockINES105IRQ();
			clockRambo1CpuIRQ();
			clockNamco163IRQ();
			clockMMC5();
			tickPpuForOneCpuCycle();
			apuTick();
			joypadTick();

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
				clockVRC467IRQ();
				clockJalecoIRQ();
				clockINES016IRQ();
				clockINES067IRQ();
				clockINES069IRQ();
				clockINES105IRQ();
				clockRambo1CpuIRQ();
				clockNamco163IRQ();
				clockMMC5();
				tickPpuForOneCpuCycle();
				apuTick();
				joypadTick();
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
				clockVRC467IRQ();
				clockJalecoIRQ();
				clockINES016IRQ();
				clockINES067IRQ();
				clockINES069IRQ();
				clockINES105IRQ();
				clockRambo1CpuIRQ();
				clockNamco163IRQ();
				clockMMC5();
				tickPpuForOneCpuCycle();
				apuTick();
				joypadTick();

				// As mentioned in point 1 of https://forums.nesdev.org/viewtopic.php?t=14120
				// "Values are written on 'put' cycles"
				pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR] = pNES_instance->NES_state.oamDMA.dataToTx;
				++pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR;
				// DMA unit alternates between 'get' cycles and 'put' cycles as mentioned in https://forums.nesdev.org/viewtopic.php?p=169070#p169070
				++pNES_instance->NES_state.emulatorStatus.ticks.dmaGetPutCounter;
				++pNES_instance->NES_state.emulatorStatus.ticks.oamDmaCounter;
				clockVRC467IRQ();
				clockJalecoIRQ();
				clockINES016IRQ();
				clockINES067IRQ();
				clockINES069IRQ();
				clockINES105IRQ();
				clockRambo1CpuIRQ();
				clockNamco163IRQ();
				clockMMC5();
				tickPpuForOneCpuCycle();
				apuTick();
				joypadTick();

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
					clockVRC467IRQ();
					clockJalecoIRQ();
					clockINES016IRQ();
					clockINES067IRQ();
					clockINES069IRQ();
					clockINES105IRQ();
					clockRambo1CpuIRQ();
					clockNamco163IRQ();
					clockMMC5();
					tickPpuForOneCpuCycle();
					apuTick();
					joypadTick();

					// Increment the CPU Counter
					++pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter;

					// Refer : https://discord.com/channels/465585922579103744/465586161067229195/863885418143416351
					// Accoring to above link, IRQ was buffered during the CPU's READ_CYCLE which triggered OAMDMA; post the completion of OAMDMA to compensate for "Dummy DMA read", CPU will try to do another CPU_READ. Will the buffered IRQ get triggered now?
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

	// NTSC: 1 CPU Tick = 3 PPU Ticks. PAL: 1 CPU Tick = 3.2 PPU Ticks on average (see tickPpuForOneCpuCycle()).
	// 1 CPU Tick = 0.5 APU Tick excepts for Triangle channel's timer (APU generally operates at half the frequency of the CPU),
	// But, 1 CPU Tick = 1 APU Tick for Triangle channel's timer
	// 1 APU Tick = 6 PPU Ticks most of the time, 
	// But, in case of Triangle channel's timer, this becomes 3 PPU ticks
	// Therefore, based on above information, we will call 1 "apuTick" for 1 "cpuTickT"

	clockVRC467IRQ();
	clockJalecoIRQ();
	clockINES016IRQ();
	clockINES067IRQ();
	clockINES069IRQ();
	clockINES105IRQ();
	clockRambo1CpuIRQ();
	clockNamco163IRQ();
	clockMMC5();
	tickPpuForOneCpuCycle();
	apuTick();
	joypadTick();
}

// Refer : https://www.nesdev.org/wiki/Cycle_reference_chart#Clock_rates
// Applies the region selected via setTVSystem() to every runtime timing member. Called once, at the top of initializeEmulator().
void NES_t::applyTVSystemTimingConfig()
{
	palPpuTickAccumulator = RESET;

	if (tvSystem == NES_TV_SYSTEM::PAL)
	{
		cpuClockHz = (float)NES_PAL_CPU_CLOCK_HZ;
		ppuClockHz = (float)NES_PAL_PPU_CLOCK_HZ;
		nesLastPpuScanline = NES_PAL_LAST_PPU_SCANLINE;
		nesTotalPpuScanline = NES_PAL_TOTAL_PPU_SCANLINE;
		nesFrameDots = NES_PAL_FRAME_DOTS;
		myFPS = (float)NES_PAL_FPS;

		// Refer : https://forums.nesdev.org/viewtopic.php?t=2124
		static const uint16_t palNoise[SIXTEEN] = { 4,7,14,30,60,88,118,148,188,236,354,472,708,944,1890,3778 };
		static const uint16_t palDmc[SIXTEEN] = { 398,354,316,298,276,236,210,198,176,148,132,118,98,78,66,50 };
		memcpy(NOISE_PERIOD_LUT, palNoise, sizeof(palNoise));
		memcpy(DMC_PERIOD_LUT, palDmc, sizeof(palDmc));
	}
	else
	{
		cpuClockHz = (float)NES_NTSC_CPU_CLOCK_HZ;
		ppuClockHz = (float)NES_NTSC_PPU_CLOCK_HZ;
		nesLastPpuScanline = NES_NTSC_LAST_PPU_SCANLINE;
		nesTotalPpuScanline = NES_NTSC_TOTAL_PPU_SCANLINE;
		nesFrameDots = NES_NTSC_FRAME_DOTS;
		myFPS = (float)NES_NTSC_FPS;

		static const uint16_t ntscNoise[SIXTEEN] = { 4,8,16,32,64,96,128,160,202,254,380,508,762,1016,2034,4068 };
		static const uint16_t ntscDmc[SIXTEEN] = { 428,380,340,320,286,254,226,214,190,160,142,128,106,84,72,54 };
		memcpy(NOISE_PERIOD_LUT, ntscNoise, sizeof(ntscNoise));
		memcpy(DMC_PERIOD_LUT, ntscDmc, sizeof(ntscDmc));
	}
}

// Refer : https://www.nesdev.org/wiki/Cycle_reference_chart#Clock_rates
// NTSC: exactly 3 PPU dots per CPU cycle, no drift.
// PAL : exactly 3.2 PPU dots per CPU cycle on average (16 PPU dots per 5 CPU cycles),
//       implemented here as 3 dots on 4 of every 5 CPU cycles and 4 dots on the 5th
//       ("the PPU outputs three dots for each CPU cycle, unless you're emulating a PAL NES,
//       in which case it outputs an extra dot every fifth CPU cycle" -- nesdev forums,
//       https://forums.nesdev.org/viewtopic.php?t=10266).
void NES_t::tickPpuForOneCpuCycle()
{
	uint8_t ppuTicksThisCpuCycle = THREE;

	if (tvSystem == NES_TV_SYSTEM::PAL)
	{
		++palPpuTickAccumulator;
		if (palPpuTickAccumulator >= FIVE)
		{
			palPpuTickAccumulator = RESET;
			ppuTicksThisCpuCycle = FOUR;
		}
	}

	for (uint8_t i = RESET; i < ppuTicksThisCpuCycle; ++i)
	{
		ppuTick();
	}
}

void NES_t::ppuTick()
{
	if (pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter >= NES_PPU_WAIT_CPU_CYCLES_POST_RESET)
	{
		SCOUNTER64 cycle = pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;
		SCOUNTER32 ly = pNES_instance->NES_state.display.currentScanline;

		// MMC5: entering post-render scanline 240 immediately ends
		// the "in-frame" state.
		//
		// IMPORTANT:
		// This must be OUTSIDE the normal rendering block because
		// NES_LAST_VISIBLE_PPU_SCANLINE == 239.
		if ((pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC5) && (ly == (NES_POST_RENDER_SCANLINE - ONE)) && (cycle == ONE))
		{
			auto& mmc5 = pNES_instance->NES_state.catridgeInfo.mmc5;

			mmc5.ppuInFrame = NO;
			mmc5.needInFrame = NO;
			mmc5.ppuIdleCounter = ZERO;

			mmc5.lastPpuReadAddr = RESET;
			mmc5.ntReadCounter = ZERO;
			mmc5.scanlineCounter = RESET;
			mmc5.splitTileNumber = RESET;

			mmc5.splitInSplitRegion = NO;
			mmc5.exAttrFetchCounter = RESET;

			mmc5.irqPending = NO;

			pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_MMC5 = RESET;

			updateMMC5ChrA();
		}

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
					// Vblank runs from scanline NES_POST_RENDER_SCANLINE (241) to nesLastPpuScanline inclusive, so its
					// length in scanlines is (nesLastPpuScanline - NES_POST_RENDER_SCANLINE + 1); NTSC = 20, PAL = 70.
					uint64_t expectedVblSetToClearDots = (uint64_t)(nesLastPpuScanline - NES_POST_RENDER_SCANLINE + ONE) * NES_TOTAL_PPU_CYCLES_PER_SCANLINE;
					if (diff != expectedVblSetToClearDots)
					{
						FATAL("VBL set time - VBL clear clear is %llu! (expected %llu)", diff, expectedVblSetToClearDots);
					}
				}

				pNES_ppuRegisters->vblank = CLEAR;
				pNES_ppuRegisters->sprite0hit = CLEAR;
				pNES_ppuRegisters->spriteOverflow = CLEAR;

#if (ENABLE_OAM_CORRUPTION == YES)
				// Mesen: ProcessScanlineFirstCycle -> if(_renderingEnabled) ProcessOamCorruption()
				if (checkIfRenderring() == YES)
				{
					for (COUNTER8 i = ZERO; i < THIRTYTWO; i++)
					{
						if (pNES_ppuRegisters->oamCorruptionRows[i] == YES)
						{
							if (i > ZERO)
							{
								memcpy(
									&pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[i * EIGHT],
									&pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[ZERO],
									EIGHT);
							}
							pNES_ppuRegisters->oamCorruptionRows[i] = NO;
						}
					}
				}
#endif
			}

#if (ENABLE_OAM_CORRUPTION == YES)
			PPUTODO("OAM corruption row selection is empirical. "
				"Mesen's implementation uses the Window-A PPU cycle to select the "
				"corrupted OAM row (cycle >> 1), but Mesen explicitly documents this "
				"behavior as still requiring further hardware research. "
				"AccuracyCoin verifies the delayed corruption behavior but does not "
				"define the exact hardware row-selection algorithm.");
			// Refer : https://forums.nesdev.org/viewtopic.php?p=284030#p284030
			// Refer : https://forums.nesdev.org/viewtopic.php?p=80985#p80985
			// Mesen reference: SetOamCorruptionFlags() / ProcessOamCorruption()
			//
			// When rendering transitions from enabled to disabled during visible scanlines,
			// flag OAM rows for corruption only during the two hardware windows:
			//   Window A: cycles 0-63  (secondary OAM clear)
			//   Window B: cycles 256-319 (sprite tile fetching)
			if (pNES_ppuRegisters->prevRendering == YES
				&& checkIfRenderring() == NO
				&& ly >= NES_FIRST_VISIBLE_SCANLINE
				&& ly <= NES_LAST_VISIBLE_PPU_SCANLINE)
			{
				// Window A: cycle 1-64 (your cycle is 1-based, Mesen's is 0-based)
				if (cycle >= ONE && cycle <= SIXTYFOUR)
				{
					PPUTODO("Determine the hardware-accurate Window A OAM corruption row selection. "
						"Current implementation intentionally flags OAM row 1 for the entire secondary-OAM-clear "
						"window (PPU cycles 1-64) to satisfy both the AccuracyCoin OAM corruption test and "
						"5.Emulator.nes sprite-overflow test. The cycle-to-row mapping is not yet understood.");
					pNES_ppuRegisters->oamCorruptionRows[ONE] = YES;
				}
				// Window B: cycle 257-320 (shift by 1 for 1-based)
				else if (cycle >= TWOFIFTYSEVEN && cycle <= THREETWENTY)
				{
					uint8_t base = TO_UINT8((cycle - TWOFIFTYSEVEN) >> THREE);
					uint8_t offset = TO_UINT8(((cycle - TWOFIFTYSEVEN) & SEVEN) > THREE ? THREE : ((cycle - TWOFIFTYSEVEN) & SEVEN));
					pNES_ppuRegisters->oamCorruptionRows[base * FOUR + offset] = YES;
				}
			}

			// Apply any pending corruption when rendering is re-enabled mid-screen
			// Mesen: UpdateState() -> _prevRenderingEnabled becomes true -> ProcessOamCorruption()
			if (pNES_ppuRegisters->prevRendering == NO
				&& checkIfRenderring() == YES
				&& ly >= NES_FIRST_VISIBLE_SCANLINE
				&& ly <= NES_LAST_VISIBLE_PPU_SCANLINE)
			{
				for (COUNTER8 i = ZERO; i < THIRTYTWO; i++)
				{
					if (pNES_ppuRegisters->oamCorruptionRows[i] == YES)
					{
						if (i > ZERO)
						{
							memcpy(
								&pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[i * EIGHT],
								&pNES_ppuMemory->NESMemoryMap.primaryOam.oamB[ZERO],
								EIGHT);
						}
						pNES_ppuRegisters->oamCorruptionRows[i] = NO;
					}
				}
			}

			pNES_ppuRegisters->prevRendering = checkIfRenderring();
#endif

			// Refer "Tile and attribute fetching" in https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
			// NOTE: when "((cycle >= THREETWENTYONE) && (cycle <= THREETHIRTYSIX))" is triggerred, "Y" of v is already incremented
			// So, we are fetching the first 2 tiles of the next scanline!
			if (checkIfRenderring() == YES && (((cycle >= ONE) && (cycle <= TWOFIFTYSIX)) || ((cycle >= THREETWENTYONE) && (cycle <= THREETHIRTYSIX))))
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

					// Handle left most 8 pixels for bg
					if (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.BG_IN_LEFTMOST_8PIXELS == NO && cycle < NINE)
					{
						bgColorID = RESET;
						bgPaletteID = RESET;
					}

					// Handle left most 8 pixels for sprites
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

						const BYTE emphasis =
							(pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.EMP_RED << ZERO)
							| (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.EMP_GREEN << ONE)
							| (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.EMP_BLUE << TWO);

						if (emphasis != ZERO)
						{
							const float redFactor = (emphasis & 0x01) ? 1.0f : 0.75f;
							const float greenFactor = (emphasis & 0x02) ? 1.0f : 0.75f;
							const float blueFactor = (emphasis & 0x04) ? 1.0f : 0.75f;

							p.r = (uint8_t)(p.r * redFactor);
							p.g = (uint8_t)(p.g * greenFactor);
							p.b = (uint8_t)(p.b * blueFactor);
						}

						// Needed for Zapper Support
						pNES_instance->NES_state.display.gfxColorID[cycle - ONE][ly] = paletteID;

						// Needed for VIDEO_FILTERS::CRT_FILTER's composite/NTSC-artifact decode
						pNES_instance->NES_state.display.gfxEmphasisBits[cycle - ONE][ly] = emphasis;

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
					// Refer : https://www.nesdev.org/wiki/PPU_registers#OAMADDR
					// OAMADDR is reset to zero if rendering is enabled in cycles 257 - 320 for visible scanlines; this is implemented below
					// Now, for some reason this reset has not happened, then we start from a non-zero OAMADDR as mentioned in the above link.
					// Previously, we were hardcoding pn and pm to zero, HW doesnt have a concept of zero, it always should be from OAMADDR, and
					// HW mostly ensured OAMADDR would be zero and hence we never noticed any issues... in most of the cases of ofcourse.
					pNES_ppuRegisters->pn = (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR >> TWO) & 0x3F;
					pNES_ppuRegisters->pm = pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR & 0x03;
					pNES_ppuRegisters->sn = RESET;
					pNES_ppuRegisters->sm = RESET;
					pNES_ppuRegisters->oamByte = RESET;
					pNES_ppuRegisters->startSpriteOverflowEvaluation = CLEAR;
					pNES_ppuRegisters->stopSpriteEvaluation = CLEAR;

					pNES_instance->NES_state.display.obj.isSprite0PresentInSecondaryOam = CLEAR;

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
								// Refer to https://forums.nesdev.org/viewtopic.php?t=25552
								// "Sprite 0 is not a property of a sprite's position in OAM ($00 in OAM is *not* necessarily sprite 0) nor even the order of evaluation (the first sprite evaluated is *not* necessarily sprite 0). 
								// Rather, it is a property of the time at which a sprite is evaluated. S
								// pecifically, on dot 66 during rendering, a 'sprite 0 on next scanline' flag is set if the current sprite is in range and cleared otherwise."
								// This helps pass arbitrary_sprite_0_test and accuracy coin's sprite evaluation tests
								if (cycle == (SIXTYFOUR + TWO))
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
				if (checkIfRenderring() == YES)
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
						const uint16_t dummyNameTblAddr = NAME_TABLE0_START_ADDRESS | (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0FFF);
						const auto discard = readPpuRawMemory(dummyNameTblAddr, MEMORY_ACCESS_SOURCE::PPU);

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
						const uint16_t dummyAttrTblAddr = (NAME_TABLE0_START_ADDRESS + 0x03C0)
							| (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0C00)
							| ((pNES_ppuRegisters->ppuInternalRegisters.v.raw >> FOUR) & 0x0038)
							| ((pNES_ppuRegisters->ppuInternalRegisters.v.raw >> TWO) & 0x0007);
						const auto discard = readPpuRawMemory(dummyAttrTblAddr, MEMORY_ACCESS_SOURCE::PPU);

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

					if (ly <= NES_LAST_VISIBLE_PPU_SCANLINE)
					{
						// Refer https://www.nesdev.org/wiki/PPU_registers#OAMADDR
						pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.OAMADDR = ZERO;
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
					// NT fetch only occurs while rendering is enabled.
					if (checkIfRenderring() == YES)
					{
						const uint16_t dummyNameTblAddr = NAME_TABLE0_START_ADDRESS | (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0FFF);
						const auto discard = readPpuRawMemory(dummyNameTblAddr, MEMORY_ACCESS_SOURCE::PPU);
					}

					BREAK;
				}
				case THREETHIRTYNINE:
				{
					// If rendering is disabled at dot 339, all sprite X positions become 0
					// Refer : https://github.com/100thCoin/AccuracyCoin#stale-bg-shift-registers
					if (checkIfRenderring() == NO)
					{
						for (COUNTER8 i = ZERO; i < EIGHT; i++)
						{
							pNES_instance->NES_state.display.obj.shifter[i].xSubtractor = ZERO;
						}
					}
					BREAK;
				}
				case THREEFORTY:
				{
					// NT fetch only occurs while rendering is enabled.
					if (checkIfRenderring() == YES)
					{
						const uint16_t dummyNameTblAddr = NAME_TABLE0_START_ADDRESS | (pNES_ppuRegisters->ppuInternalRegisters.v.raw & 0x0FFF);
						const auto discard = readPpuRawMemory(dummyNameTblAddr, MEMORY_ACCESS_SOURCE::PPU);
					}
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

		// Refer : Mesen _updateVramAddrDelay - deferred $2006 2nd-write v update
		if (pNES_ppuRegisters->ppuInternalRegisters.vramAddrUpdateDelay > ZERO)
		{
			pNES_ppuRegisters->ppuInternalRegisters.vramAddrUpdateDelay--;

			if (pNES_ppuRegisters->ppuInternalRegisters.vramAddrUpdateDelay == ZERO)
			{
				pNES_ppuRegisters->ppuInternalRegisters.v.raw
					= pNES_ppuRegisters->ppuInternalRegisters.vramAddrPendingValue;

				bool isNotRendering = !((pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET)
					|| (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET));
				bool isOutsideVisible = !((pNES_instance->NES_state.display.currentScanline >= NES_PRE_RENDER_SCANLINE)
					&& (pNES_instance->NES_state.display.currentScanline <= NES_LAST_VISIBLE_PPU_SCANLINE));

				if (isNotRendering || isOutsideVisible)
				{
					clockMMC3IRQ(pNES_ppuRegisters->ppuInternalRegisters.v.raw, MEMORY_ACCESS_SOURCE::CPU, NO);
				}
			}
		}

		// Refer : Mesen _ignoreVramRead - suppress back-to-back $2007 reads within 6 PPU cycles
		if (pNES_ppuRegisters->ppuInternalRegisters.ignoreVramRead > ZERO)
		{
			pNES_ppuRegisters->ppuInternalRegisters.ignoreVramRead--;
		}

		// Refer : Mesen _needVideoRamIncrement - defer $2007 v increment by 1 PPU cycle
		// This prevents open bus test's rapid $2007 reads from corrupting v during rendering tests
		if (pNES_ppuRegisters->ppuInternalRegisters.needVideoRamIncrement == YES)
		{
			pNES_ppuRegisters->ppuInternalRegisters.needVideoRamIncrement = NO;

			bool isRendering = (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_BG_RENDERING == SET)
				|| (pNES_cpuMemory->NESMemoryMap.ppuCtrl.ppuCtrl.PPUMASK.ppumask.ENABLE_SPRITE_RENDERING == SET);
			bool isVisibleOrPrerender = (pNES_instance->NES_state.display.currentScanline >= NES_PRE_RENDER_SCANLINE)
				&& (pNES_instance->NES_state.display.currentScanline <= NES_LAST_VISIBLE_PPU_SCANLINE);

			if (isRendering && isVisibleOrPrerender)
			{
				xInc();
				yInc();
			}
			else
			{
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
			}
		}

		// Tick the counters
		++pNES_instance->NES_state.emulatorStatus.ticks.ppuCounter;
		++pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY;
		++pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerFrame;
		++pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterMMC3A12;

		// Refer : https://www.nesdev.org/wiki/PPU_frame_timing 
		// Refer : https://forums.nesdev.org/viewtopic.php?t=1368
		// Refer : https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
		// Refer : https://www.nesdev.org/wiki/PAL_video ("No short line" -- PAL/2C07 never skips this dot, on odd or even frames)
		if (
			(tvSystem == NES_TV_SYSTEM::NTSC)
			&&
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

			if (pNES_instance->NES_state.display.currentScanline >= nesTotalPpuScanline)
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

#ifndef __RPI_PICO__
	debugSyncScreenIfNeeded();
#endif
}

void NES_t::apuTick()
{
	// -----------------------------------------------------------------------
	// Called once per CPU cycle
	// Clock domains:
	//   Odd CPU cycles only : NES pulse 1/2, noise, MMC5 pulse 1/2  (÷2 domain)
	//   Every CPU cycle     : triangle, DMC, VRC6 (own dividers, full-rate domain)
	// -----------------------------------------------------------------------

	if (ENABLED)
	{
		// To handle 09.reset_timing.nes
		if (pNES_instance->NES_state.audio.isReset == YES)
		{
			pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer = TEN;
			pNES_instance->NES_state.audio.isReset = CLEAR;
		}

		// Refer : https://forums.nesdev.org/viewtopic.php?p=64359#p64359 for the apuSequencer magic numbers
		if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FOUR_STEP_MODE)
		{
			uint64_t seq0 = pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer;

			if (seq0 == frameSeqM00())
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);
				processLinearCounter();
			}
			else if (seq0 == frameSeqM01())
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
			}
			else if (seq0 == frameSeqM02())
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);
				processLinearCounter();
			}
			else if (seq0 == (frameSeqM03() - ONE))
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.FRAME_CONFIG.DIS_FRAME_INTR == RESET
					&& pNES_instance->NES_state.interrupts.irqDelayInCpuCycles == RESET)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.FRAME_INTR = SET;
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_FRAMECTR = SET;
					pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = ONE;
				}
			}
			else if (seq0 == frameSeqM03())
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.FRAME_CONFIG.DIS_FRAME_INTR == RESET
					&& pNES_instance->NES_state.interrupts.irqDelayInCpuCycles == RESET)
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
			}
			else if (seq0 == (frameSeqM03() + ONE))
			{
				if (pNES_cpuMemory->NESMemoryMap.apuAndIO.JOY2_OR_FRAME_CONFIG.FRAME_CONFIG.DIS_FRAME_INTR == RESET
					&& pNES_instance->NES_state.interrupts.irqDelayInCpuCycles == RESET)
				{
					pNES_cpuMemory->NESMemoryMap.apuAndIO.SND_CHN.FRAME_INTR = SET;
					pNES_instance->NES_state.interrupts.isIRQ.fields.IRQ_SRC_FRAMECTR = SET;
					pNES_instance->NES_state.interrupts.irqDelayInCpuCycles = ZERO;
				}
			}
		}
		else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
		{
			uint64_t seq1 = pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer;

			if (seq1 == frameSeqM10())
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
			}
			else if (seq1 == frameSeqM11())
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);
				processLinearCounter();
			}
			else if (seq1 == frameSeqM12())
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
			}
			else if (seq1 == frameSeqM13())
			{
				processEnvelope(AUDIO_CHANNELS::PULSE_1);
				processEnvelope(AUDIO_CHANNELS::PULSE_2);
				processEnvelope(AUDIO_CHANNELS::NOISE);
				processLinearCounter();
			}
		}
		else
		{
			FATAL("Unknown Frame Sequencer Mode");
		}

		// MMC5 frame sequencer clocked HERE — before ++apuSequencer — so it
		// reads the same step value the NES channels above just processed.
		// Calling it after the increment (as clockMMC5() previously did) caused
		// MMC5 envelope and length counters to fire one step late.
		if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC5)
		{
			clockMMC5FrameSequencer();
		}

		++pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer;

		if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FOUR_STEP_MODE)
		{
			if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer >= (frameSeqM04()))
			{
				pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer = (frameSeqM00());
			}
		}
		else if (pNES_instance->NES_state.audio.frameSequencerMode == FRAME_SEQUENCER_MODE::FIVE_STEP_MODE)
		{
			if (pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer >= (frameSeqM14()))
			{
				pNES_instance->NES_state.emulatorStatus.ticks.apuSequencer = (frameSeqM10());
			}
		}
	}

	if (ENABLED)
	{
		// ---- ÷2 clock domain (odd CPU cycles only) ----
		// NES pulse 1/2, noise, and MMC5 pulse 1/2 all share this domain.
		// Their freqPeriod values are calibrated for a half-rate clock.
		if (GETBIT(ZERO, pNES_instance->NES_state.emulatorStatus.ticks.apuCounter) == SET)
		{
			tickPulse(AUDIO_CHANNELS::PULSE_1);
			tickPulse(AUDIO_CHANNELS::PULSE_2);
			tickNoise();

			if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC5)
			{
				tickMMC5Pulse(0);
				tickMMC5Pulse(1);
			}
		}

		// ---- Full CPU-rate clock domain ----
		// Triangle and DMC tick every CPU cycle per NES spec.
		// VRC6 also ticks every CPU cycle — its freqPeriod values (0..4095)
		// are calibrated for per-cycle decrement. Gating it on odd cycles
		// would halve its effective clock and pitch everything an octave low.
		tickTriangle();
		tickDMC();

		if (isVRC6())
		{
			tickVRC6Audio();
		}

		if (isNamco163WithAudio())
		{
			tickNamco163Audio();
		}
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

		// Expansion audio output snapshots — generated here so all channel
		// dacInputs are consistent before captureDownsampledAudioSamples reads them.
		if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC5)
		{
			generateMMC5PulseOutput(0);
			generateMMC5PulseOutput(1);
		}
		if (isVRC6())
		{
			generateVRC6AudioOutput();
		}
		if (isNamco163WithAudio())
		{
			generateNamco163Output();
		}

		captureDownsampledAudioSamples();
	}

	++pNES_instance->NES_state.emulatorStatus.ticks.apuCounter;
}

static constexpr uint32_t KEY_SAMPLING_MASK = next_pow2(NES_TOTAL_PPU_CYCLES_PER_SCANLINE) - ONE; // = 511

void NES_t::joypadTick()
{
	if (_ENABLE_ACCURATE_INPUT_SAMPLING == YES)
	{
		auto& counter = pNES_instance->NES_state.emulatorStatus.ticks.keySamplingCounter;
		counter = (counter + ONE) & KEY_SAMPLING_MASK;
		if (counter == RESET)
		{
			inputHintCallback();
		}
	}
}

void NES_t::updateKeyStatus()
{
	auto& keys = pNES_instance->NES_state.emulatorStatus.controllerInput;

	byte status = 0;

	status |= (byte)(keys.keyA == YES);
	status |= (byte)(keys.keyB == YES) << ONE;
	status |= (byte)(keys.keySELECT == YES) << TWO;
	status |= (byte)(keys.keySTART == YES) << THREE;
	status |= (byte)(keys.keyUP == YES) << FOUR;
	status |= (byte)(keys.keyDOWN == YES) << FIVE;
	status |= (byte)(keys.keyLEFT == YES) << SIX;
	status |= (byte)(keys.keyRIGHT == YES) << SEVEN;

	pNES_instance->NES_state.controller.keyStatus = status;
}

void NES_t::captureIO()
{
	DO_NOTHING;
}

void NES_t::captureDownsampledAudioSamples()
{
	pNES_instance->NES_state.audio.downSamplingRatioCounter += ONE;

	if (pNES_instance->NES_state.audio.downSamplingRatioCounter >= ((int32_t)(cpuClockHz / EMULATED_AUDIO_SAMPLING_RATE_FOR_NES)))
	{
		pNES_instance->NES_state.audio.downSamplingRatioCounter -= ((int32_t)(cpuClockHz / EMULATED_AUDIO_SAMPLING_RATE_FOR_NES));

		NES_AUDIO_SAMPLE_TYPE pulse_out = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE tnd_out = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE expansion_out = MUTE_AUDIO;

		NES_AUDIO_SAMPLE_TYPE pulse1 = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE pulse2 = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE triangle = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE noise = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE dmc = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE mmc5pulse1 = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE mmc5pulse2 = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE mmc5pcm = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE vrc6pulse1 = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE vrc6pulse2 = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE vrc6saw = MUTE_AUDIO;
		NES_AUDIO_SAMPLE_TYPE n163out = MUTE_AUDIO;

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
		if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::MMC5 && DISABLE_MMC5_CHANNELS == NO)
		{
			auto& mmc5audio = pNES_instance->NES_state.catridgeInfo.mmc5.mmc5Audio;
			mmc5pulse1 = mmc5audio.pulse[0].dacInput;
			mmc5pulse2 = mmc5audio.pulse[1].dacInput;

			// MMC5 PCM ($5011) is an independent DAC equivalent in volume to the
			// NES DMC DAC ($4011). It is NOT part of the NES tnd path — it is a
			// separate linear term. Writing $00 has no effect per the MMC5 wiki spec.
			if (mmc5audio.pcmRawSample > 0)
			{
				// pcmReadMode == NO means write mode ($5011 writes update the DAC).
				// pcmReadMode == YES means read mode ($8000-BFFF reads update the DAC).
				// In both modes the dacInput is the current pcmRawSample value.
				mmc5pcm = static_cast<NES_AUDIO_SAMPLE_TYPE>(mmc5audio.pcmRawSample);
			}
		}
		if (isVRC6() && DISABLE_VRC6_CHANNELS == NO)
		{
			auto& aud = pNES_instance->NES_state.catridgeInfo.vrc6.vrc6Audio;
			vrc6pulse1 = aud.pulse[0].dacInput;
			vrc6pulse2 = aud.pulse[1].dacInput;
			vrc6saw = aud.sawtooth.dacInput;
		}
		if (isNamco163WithAudio() && DISABLE_NAMCO163_CHANNELS == NO)
		{
			n163out = pNES_instance->NES_state.catridgeInfo.namco163.audioDacInput;
		}

		// -----------------------------------------------------------------------
		// APU Mixer — Refer: https://www.nesdev.org/wiki/APU_Mixer
		//
		// NES internal channels use a nonlinear DAC (pulse group and tnd group).
		// Expansion audio (MMC5, VRC6) connects to the analog summing network
		// AFTER the NES DAC and is mixed linearly.
		//
		// MMC5 pulse channels: equivalent volume to NES pulse channels.
		//   Scale = 0.00752 per unit (same as NES linear approximation constant).
		//   Refer: https://www.nesdev.org/wiki/MMC5_audio — "equivalent in volume
		//   to the corresponding APU channels".
		//
		// MMC5 PCM ($5011): equivalent volume to NES DMC ($4011), range 0–255.
		//   Scale = 0.00335 per unit (same as NES DMC linear constant).
		//
		// VRC6: 6-bit DAC summing two 4-bit pulse outputs and high-5-bit saw.
		//   Pulse range 0–15, saw range 0–31.
		//   At max volume, VRC6 pulse ≡ NES pulse -> scale = 0.00752 per unit.
		//   Sawtooth max = 31 ≈ 2× a pulse channel -> scale = 0.00752 / 2 per unit.
		//   Refer: https://www.nesdev.org/wiki/VRC6_audio#Output — "roughly
		//   equivalent to the pulse channels of the 2A03".
		// -----------------------------------------------------------------------

#if (ENABLE_AUDIO_MIXER_LUT == YES)
		// ---- NES pulse group: LUT formula ----
		// pulse_table[n] = 95.52 / (8128.0 / n + 100)
		// n = pulse1 + pulse2 only (NES two-pulse DAC; MMC5 is separate)
		if ((pulse1 > MUTE_AUDIO) || (pulse2 > MUTE_AUDIO))
		{
			const NES_AUDIO_SAMPLE_TYPE n = pulse1 + pulse2;
			pulse_out = 95.52f / ((8128.0f / n) + 100.0f);
		}

		// ---- NES tnd group: LUT approximation ----
		// tnd_table[n] = 163.67 / (24329.0 / n + 100)
		// n = 3 * triangle + 2 * noise + dmc
		// (approximation within 4% of the exact formula per NESdev wiki)
		if ((triangle > MUTE_AUDIO) || (noise > MUTE_AUDIO) || (dmc > MUTE_AUDIO))
		{
			const NES_AUDIO_SAMPLE_TYPE n = (3.0f * triangle) + (2.0f * noise) + dmc;
			tnd_out = 163.67f / ((24329.0f / n) + 100.0f);
		}
#else
		// ---- Linear approximation fallback ----
		// Refer: https://www.nesdev.org/wiki/APU_Mixer#Linear_Approximation
		if ((pulse1 > MUTE_AUDIO) || (pulse2 > MUTE_AUDIO))
		{
			pulse_out = 0.00752f * (pulse1 + pulse2);
		}
		if ((triangle > MUTE_AUDIO) || (noise > MUTE_AUDIO) || (dmc > MUTE_AUDIO))
		{
			tnd_out = (0.00851f * triangle) + (0.00494f * noise) + (0.00335f * dmc);
		}
#endif

		// ---- Expansion audio: linear additive post-DAC mix ----
		// All expansion channels use the same linear scale regardless of
		// whether the NES path used LUT or linear approximation.
		expansion_out =
			// MMC5 pulses: 0.00752 per unit, range 0–15 each
			(0.00752f * (mmc5pulse1 + mmc5pulse2))
			// MMC5 PCM: 0.00335 per unit, range 0–255 (DMC-equivalent volume)
			+ (0.00335f * mmc5pcm)
			// VRC6 pulses: 0.00752 per unit, range 0–15 each
			+ (0.00752f * (vrc6pulse1 + vrc6pulse2))
			// VRC6 sawtooth: 0.00376 per unit (half-pulse scale), range 0–31
			+ (0.00376f * vrc6saw)
			// N163: linear additive, same 0.00752 scale as NES pulse channels
			// Output is already divided by (numCh+1) in generateNamco163Output()
			// Range after divide: approx ±15 per unit at single-channel max volume
			// Ref: https://www.nesdev.org/wiki/Namco_163_audio#Mixing
			+ (0.00752f * n163out);

		NES_AUDIO_SAMPLE_TYPE sample = pulse_out + tnd_out + expansion_out;

		// -----------------------------------------------------------------------
		// Filtering — Refer: https://forums.nesdev.org/viewtopic.php?p=163208#p163208
		// Chain: Low Pass (14 kHz) -> High Pass A (440 Hz) -> High Pass B (90 Hz)
		// -----------------------------------------------------------------------

		// 1) Low pass filter (~14 kHz)
		pNES_instance->NES_state.audio.LP_In = sample;
		pNES_instance->NES_state.audio.LP_Out = (pNES_instance->NES_state.audio.LP_In - pNES_instance->NES_state.audio.LP_Out) * 0.815686f;

		// 2) High pass filter A (~440 Hz)
		pNES_instance->NES_state.audio.HPA_Out = pNES_instance->NES_state.audio.HPA_Out * 0.996039f + pNES_instance->NES_state.audio.LP_Out - pNES_instance->NES_state.audio.HPA_Prev;
		pNES_instance->NES_state.audio.HPA_Prev = pNES_instance->NES_state.audio.LP_Out;

		// 3) High pass filter B (~90 Hz)
		pNES_instance->NES_state.audio.HPB_Out = pNES_instance->NES_state.audio.HPB_Out * 0.999835f + pNES_instance->NES_state.audio.HPA_Out - pNES_instance->NES_state.audio.HPB_Prev;
		pNES_instance->NES_state.audio.HPB_Prev = pNES_instance->NES_state.audio.HPA_Out;

		sample = pNES_instance->NES_state.audio.HPB_Out;

#if DISABLED
		sample = std::clamp(sample, (float)(-1.0), (float)(1.0)) * 100.0f;
#else
		sample *= 100.0f;
#endif

		if (pNES_instance->NES_state.audio.accumulatedTone >= AUDIO_BUFFER_SIZE_FOR_NES)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) == YES)
			{
				auto gain = getEmulationVolume();
				gain += 0.05f;
				gain = std::clamp(gain, 0.0001f, 0.9998f);
				setEmulationVolume(gain);
			}
			if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) == YES)
			{
				auto gain = getEmulationVolume();
				gain -= 0.05f;
				gain = std::clamp(gain, 0.0001f, 0.9998f);
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

	// Apply filtering only when it changes (optimization)
	static GLint prevFilterNES = -1;
	if (filter != prevFilterNES)
	{
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
		prevFilterNES = filter;
	}

	// 2. Render nes_texture into framebuffer (masquerade_texture target)
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
	GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// Pass 1: Render base texture (Game Boy framebuffer)
	FLAG useCrtFilter = (currEnVFilter == VIDEO_FILTERS::CRT_FILTER) && ntscResourcesInitialized;
	if (useCrtFilter)
	{
#if (ENABLE_SINGLE_PALETTE_NTSC_FILTER_DEBUG == YES)
		if (ntscDebugPaletteIndex >= 0)
		{
			BYTE index = (BYTE)ntscDebugPaletteIndex;
			for (uint32_t y = ZERO; y < getScreenHeight(); ++y)
			{
				for (uint32_t x = ZERO; x < getScreenWidth(); ++x)
				{
					pNES_instance->NES_state.display.gfxColorID[x][y] = index;
					pNES_instance->NES_state.display.gfxEmphasisBits[x][y] = ZERO;
				}
			}
		}
#endif
		// Pack palette-index (0-63) and emphasis bits (0-7) into a normalized RG8 texture -- avoids GL_R8UI, which
		// isn't available under the ES2 path this file already guards against (see the #if above this function).
		static BYTE ntscPackedBuffer[screen_height][screen_width][TWO];
		for (uint32_t y = ZERO; y < getScreenHeight(); ++y)
		{
			for (uint32_t x = ZERO; x < getScreenWidth(); ++x)
			{
				ntscPackedBuffer[y][x][ZERO] = pNES_instance->NES_state.display.gfxColorID[x][y];
				ntscPackedBuffer[y][x][ONE] = pNES_instance->NES_state.display.gfxEmphasisBits[x][y];
			}
		}

		if (ntscIndexTexture == ZERO)
		{
			GL_CALL(glGenTextures(1, &ntscIndexTexture));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, ntscIndexTexture));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, getScreenWidth(), getScreenHeight(), 0, GL_RG, GL_UNSIGNED_BYTE, NULL));
		}

		GL_CALL(glBindTexture(GL_TEXTURE_2D, ntscIndexTexture));
		GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RG, GL_UNSIGNED_BYTE, (GLvoid*)ntscPackedBuffer));

		GL_CALL(glUseProgram(shaderProgramNTSC));
		GL_CALL(glUniform1i(glGetUniformLocation(shaderProgramNTSC, "u_IndexTexture"), 0));
		GL_CALL(glUniform1f(glGetUniformLocation(shaderProgramNTSC, "u_ScreenWidth"), (float)getScreenWidth()));
		GL_CALL(glUniform1f(glGetUniformLocation(shaderProgramNTSC, "u_ScreenHeight"), (float)getScreenHeight()));
		GL_CALL(glUniform1i(glGetUniformLocation(shaderProgramNTSC, "u_OddFrame"), (pNES_instance->NES_state.display.isOddFrame == YES) ? 1 : 0));
		GL_CALL(glUniform1f(glGetUniformLocation(shaderProgramNTSC, "u_PhaseOffsetDegrees"), ntscPhaseOffsetDegrees));
	}
	else
	{
		GL_CALL(glUseProgram(shaderProgramBasic));
	}
	GL_CALL(glActiveTexture(GL_TEXTURE0));

	if (useCrtFilter)
	{
		// ntscIndexTexture was already uploaded and bound above -- this unconditional block used to always
		// rebind nes_texture here, silently discarding that bind before the draw call. That was the actual
		// bug: the CRT shader ran, but always sampled plain RGB through a sampler it declared as an index
		// texture, so nothing written into gfxColorID (including the debug strip) ever reached the GPU.
		GL_CALL(glBindTexture(GL_TEXTURE_2D, ntscIndexTexture));
	}
	else
	{
		// Bind once (no redundant state changes)
		GL_CALL(glBindTexture(GL_TEXTURE_2D, nes_texture));

		// Ensure correct filter is applied only when needed
		static GLint prevFilterSrcNES = -1;
		static GLuint prevTexNES = 0;

		if (filter != prevFilterSrcNES || prevTexNES != nes_texture)
		{
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
			prevFilterSrcNES = filter;
			prevTexNES = nes_texture;
		}

		// Set uniform
		GL_CALL(glUniform1i(glGetUniformLocation(shaderProgramBasic, "u_Texture"), 0));
	}

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

	// Apply filtering only when it changes
	static GLint prevFilterFinalNES = -1;
	if (filter != prevFilterFinalNES)
	{
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
		prevFilterFinalNES = filter;
	}
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

	// in your PPU reset / init
	memset(pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusDecayStamp, 0,
		sizeof(pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusDecayStamp));

	pNES_ppuRegisters->ppuInternalRegisters.openBus.openBusValue = 0;
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

	pNES_instance->NES_state.audio.emulatorVolume = pt.get<std::float_t>("nes._volume", 0.1f);
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), pNES_instance->NES_state.audio.emulatorVolume);

	// Refer to https://forums.nesdev.org/viewtopic.php?p=163157&sid=d5d3c2ba788e71c4b0d23d7651bb7dd5#p163157
	pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::NOISE)].noise.noiseShiftRegister.raw = ONE;

	// Refer to https://forums.nesdev.org/viewtopic.php?p=163287#p163287
	pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.bitsInOutputUnit = ONE;

	// Needed by DmcPowerOnBuzz.nes
	pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleAddress = 0xC000;
	pNES_instance->NES_state.audio.apuInternalRegisters[TO_UINT8(AUDIO_CHANNELS::DMC)].dmc.dmcSampleLength = ONE;
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

	RETURN status;
}

bool NES_t::runEmulationLoopAtFixedRate(uint32_t currentFrame)
{
#ifndef __RPI_PICO__
	if (nesDebugger.ppu.paused == YES && nesDebugger.ppu.stepRequested == NO)
	{
		RETURN true;
	}
	nesDebugger.ppu.stepRequested = NO;
#endif

	pNES_instance->NES_state.display.wasVblankJustTriggerred = NO;

	FLAG shouldYieldForSampleMode = NO;

#if (ENABLE_R2A03_SST == YES)
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
				opcode = RESET;
				INFO("Completed Running all Tom Harte nes6502 (v1) tests");
				PAUSE;
			}

			// --------------------------------------------------------
			// Build file path
			// --------------------------------------------------------
			std::string testCaseName = std::format("{:02x}", opcode);
			std::filesystem::path fullPath = std::filesystem::path(_JSON_LOCATION) / (testCaseName + ".json");

			LOG_NEW_LINE;
			INFO("Running : %s", fullPath.string().c_str());

			// --------------------------------------------------------
			// Read entire file into string, then parse with RapidJSON
			// --------------------------------------------------------
			{
				std::ifstream ifs(fullPath);
				if (!ifs.is_open())
				{
					WARN("Failed to open %s", fullPath.string().c_str());
					++opcode;
					CONTINUE;
				}

				std::string jsonStr((std::istreambuf_iterator<char>(ifs)),
					std::istreambuf_iterator<char>());
				ifs.close();

				rapidjson::Document testCase;
				testCase.Parse(jsonStr.c_str());

				if (testCase.HasParseError())
				{
					WARN("Failed to parse %s: error code %u at offset %zu",
						fullPath.string().c_str(),
						(unsigned)testCase.GetParseError(),
						testCase.GetErrorOffset());
					++opcode;
					CONTINUE;
				}

				if (!testCase.IsArray())
				{
					WARN("%s does not contain a JSON array", fullPath.string().c_str());
					++opcode;
					CONTINUE;
				}

				// --------------------------------------------------------
				// Iterate each test case in the JSON array
				// --------------------------------------------------------
				for (rapidjson::SizeType itemIdx = 0; itemIdx < testCase.Size(); ++itemIdx)
				{
					const rapidjson::Value& item = testCase[itemIdx];
					FLAG quitThisRun = NO;

					// ================= NAME =================
					std::string name = (item.HasMember("name") && item["name"].IsString())
						? item["name"].GetString() : "";

					if (SST_DEBUG_PRINT)
						std::cout << "Name: " << name << std::endl;

					// ================= INITIAL =================
					if (!item.HasMember("initial") || !item["initial"].IsObject())
					{
						WARN("Test '%s' missing 'initial' object", name.c_str());
						CONTINUE;
					}

					const rapidjson::Value& initialJson = item["initial"];

					int initial_pc = initialJson.HasMember("pc") ? initialJson["pc"].GetInt() : 0;
					int initial_s = initialJson.HasMember("s") ? initialJson["s"].GetInt() : 0;
					int initial_a = initialJson.HasMember("a") ? initialJson["a"].GetInt() : 0;
					int initial_x = initialJson.HasMember("x") ? initialJson["x"].GetInt() : 0;
					int initial_y = initialJson.HasMember("y") ? initialJson["y"].GetInt() : 0;
					int initial_p = initialJson.HasMember("p") ? initialJson["p"].GetInt() : 0;

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

					// ================= INITIAL RAM =================
					if (SST_DEBUG_PRINT)
						std::cout << "Initial RAM:" << std::endl;

					if (initialJson.HasMember("ram") && initialJson["ram"].IsArray())
					{
						const rapidjson::Value& ramArray = initialJson["ram"];
						for (rapidjson::SizeType i = 0; i < ramArray.Size(); ++i)
						{
							const rapidjson::Value& entry = ramArray[i];
							if (!entry.IsArray() || entry.Size() < 2) CONTINUE;

							int address = entry[0].GetInt();
							int value = entry[1].GetInt();

							if (SST_DEBUG_PRINT)
								std::cout << "  Address: " << address << ", Value: " << value << std::endl;

							pNES_cpuMemory->NESRawMemory[address] = value;
						}
					}

					// ================= RUN =================
					processSOC();

					// ================= FINAL =================
					if (!item.HasMember("final") || !item["final"].IsObject())
					{
						WARN("Test '%s' missing 'final' object", name.c_str());
						CONTINUE;
					}

					const rapidjson::Value& finalJson = item["final"];

					int final_pc = finalJson.HasMember("pc") ? finalJson["pc"].GetInt() : 0;
					int final_s = finalJson.HasMember("s") ? finalJson["s"].GetInt() : 0;
					int final_a = finalJson.HasMember("a") ? finalJson["a"].GetInt() : 0;
					int final_x = finalJson.HasMember("x") ? finalJson["x"].GetInt() : 0;
					int final_y = finalJson.HasMember("y") ? finalJson["y"].GetInt() : 0;
					int final_p = finalJson.HasMember("p") ? finalJson["p"].GetInt() : 0;

					if (SST_DEBUG_PRINT)
					{
						std::cout << "Final PC: " << final_pc << ", S: " << final_s
							<< ", A: " << final_a << ", X: " << final_x
							<< ", Y: " << final_y << ", P: " << final_p << std::endl;
					}

					// ================= REGISTER CHECKS =================
					if (pNES_cpuRegisters->pc != final_pc)
					{
						FATAL("PC Mismatch"); quitThisRun = YES;
					}
					if (pNES_cpuRegisters->sp != final_s)
					{
						FATAL("SP Mismatch"); quitThisRun = YES;
					}
					if (pNES_cpuRegisters->a != final_a)
					{
						FATAL("A Mismatch");  quitThisRun = YES;
					}
					if (pNES_cpuRegisters->x != final_x)
					{
						FATAL("X Mismatch");  quitThisRun = YES;
					}
					if (pNES_cpuRegisters->y != final_y)
					{
						FATAL("Y Mismatch");  quitThisRun = YES;
					}
					if (pNES_cpuRegisters->p.p != final_p)
					{
						FATAL("P Mismatch");  quitThisRun = YES;
					}

					pNES_cpuRegisters->pc = RESET;
					pNES_cpuRegisters->sp = RESET;
					pNES_cpuRegisters->a = RESET;
					pNES_cpuRegisters->x = RESET;
					pNES_cpuRegisters->y = RESET;
					pNES_cpuRegisters->p.p = RESET;

					// ================= FINAL RAM =================
					if (SST_DEBUG_PRINT)
						std::cout << "Final RAM:" << std::endl;

					if (finalJson.HasMember("ram") && finalJson["ram"].IsArray())
					{
						const rapidjson::Value& ramArray = finalJson["ram"];
						for (rapidjson::SizeType i = 0; i < ramArray.Size(); ++i)
						{
							const rapidjson::Value& entry = ramArray[i];
							if (!entry.IsArray() || entry.Size() < 2) CONTINUE;

							int address = entry[0].GetInt();
							int value = entry[1].GetInt();

							if (SST_DEBUG_PRINT)
								std::cout << "  Address: " << address << ", Value: " << value << std::endl;

							if (pNES_cpuMemory->NESRawMemory[address] != value)
							{
								FATAL("RAM Mismatch");
								quitThisRun = YES;
							}

							pNES_cpuMemory->NESRawMemory[address] = RESET;
						}
					}

					// ================= CYCLES =================
					if (SST_DEBUG_PRINT)
						std::cout << "Cycles:" << std::endl;

					pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.indexer = RESET;
					INC8 indexer = RESET;

					if (item.HasMember("cycles") && item["cycles"].IsArray())
					{
						const rapidjson::Value& cyclesArray = item["cycles"];
						for (rapidjson::SizeType i = 0; i < cyclesArray.Size(); ++i)
						{
							const rapidjson::Value& cycle = cyclesArray[i];
							if (!cycle.IsArray() || cycle.Size() < 3) CONTINUE;

							int         cycle_address = cycle[0].GetInt();
							int         cycle_value = cycle[1].GetInt();
							std::string cycle_type = cycle[2].GetString();

							if (SST_DEBUG_PRINT)
							{
								std::cout << "Cycle Address: " << cycle_address
									<< ", Value: " << cycle_value
									<< ", Type: " << cycle_type << std::endl;
							}

							if (cycle_address != pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].address)
							{
								FATAL("Address Cycle Mismatch");
								quitThisRun = YES;
							}

							if (cycle_value != pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].data)
							{
								FATAL("Data Cycle Mismatch");
								quitThisRun = YES;
							}

							std::string temp = "write";
							if (pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].isRead == YES)
								temp = "read";

							if (cycle_type.compare(temp))
							{
								FATAL("Operation Cycle Mismatch");
								quitThisRun = YES;
							}

							pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].reset();
							++indexer;
						}
					}

					if (quitThisRun == YES)
						BREAK;

					// ================= UPDATE STATS =================
					++pNES_instance->NES_state.emulatorStatus.debugger.tomHarte.testCount[opcode];
				}
			}

			++opcode;
		}
	}
	else
#endif
	{
	processSOC();

#if (DISABLED)
	runDebugger();
#endif
	}

	if (nesReset && pNES_instance->NES_state.display.wasVblankJustTriggerred)
	{
		pNES_instance->NES_state.display.wasVblankJustTriggerred = CLEAR;

		pNES_cpuRegisters->pc = 0x0000;
		pNES_cpuRegisters->sp = 0x00;
		pNES_cpuRegisters->p.p = 0x00;
		pNES_instance->NES_state.controller.keyID = INVALID;

		loadRom(rom);
	}

#ifndef __RPI_PICO__
	if (nesDebugger.ppu.runToBreakpointArmed == YES
		&& (int)pNES_instance->NES_state.display.currentScanline == nesDebugger.ppu.breakpointScanline
		&& (int)pNES_instance->NES_state.emulatorStatus.ticks.ppuCounterPerLY == nesDebugger.ppu.breakpointDot)
	{
		nesDebugger.ppu.runToBreakpointArmed = NO;
		nesDebugger.ppu.paused = YES;
	}

	if (nesDebugger.ppu.enabled == YES && nesDebugger.ppu.runToBreakpointArmed == NO && nesDebugger.ppu.paused == NO)
	{
		if (nesDebugger.ppu.pixelOutputSampleMode == NES_DEBUG_PIXEL_SAMPLE_MODE::PER_DOT)
		{
			shouldYieldForSampleMode = YES;
		}
		else if (nesDebugger.ppu.pixelOutputSampleMode == NES_DEBUG_PIXEL_SAMPLE_MODE::PER_LY
			&& pNES_instance->NES_state.display.currentScanline != nesDebugger.ppu.debugLastScanlineSeenByLoop)
		{
			shouldYieldForSampleMode = YES;
			nesDebugger.ppu.debugLastScanlineSeenByLoop = pNES_instance->NES_state.display.currentScanline;
		}
	}
#endif

	RETURN (pNES_instance->NES_state.display.wasVblankJustTriggerred
#ifndef __RPI_PICO__
		|| nesDebugger.ppu.paused == YES
		|| shouldYieldForSampleMode == YES
#endif
	);
}

FLAG NES_t::onKeyEvent(EmuKey key, EmuKeyAction action)
{
	bool pressed = (action == EmuKeyAction::PRESSED);

	switch (key)
	{
	case EmuKey::A:      pNES_instance->NES_state.emulatorStatus.controllerInput.keyA = pressed; BREAK;
	case EmuKey::B:      pNES_instance->NES_state.emulatorStatus.controllerInput.keyB = pressed; BREAK;
	case EmuKey::START:  pNES_instance->NES_state.emulatorStatus.controllerInput.keySTART = pressed; BREAK;
	case EmuKey::SELECT: pNES_instance->NES_state.emulatorStatus.controllerInput.keySELECT = pressed; BREAK;
	case EmuKey::UP:     pNES_instance->NES_state.emulatorStatus.controllerInput.keyUP = pressed; BREAK;
	case EmuKey::DOWN:   pNES_instance->NES_state.emulatorStatus.controllerInput.keyDOWN = pressed; BREAK;
	case EmuKey::LEFT:   pNES_instance->NES_state.emulatorStatus.controllerInput.keyLEFT = pressed; BREAK;
	case EmuKey::RIGHT:  pNES_instance->NES_state.emulatorStatus.controllerInput.keyRIGHT = pressed; BREAK;
	default:             RETURN NO;
	}

	updateKeyStatus();

	RETURN YES;
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

	// check whether to enable the db or not
	pAbsolute_NES_instance->absolute_NES_state.enable_nes_db = to_bool(pt.get<std::string>("nes._enable_nes_db", "false"));

	// get forced TV settings
	forceNTSC = to_bool(pt.get<std::string>("nes._force_ntsc", "false"));
	forcePAL = to_bool(pt.get<std::string>("nes._force_pal", "false"));

	if (forceNTSC == YES && forcePAL == YES)
	{
		WARN("_force_ntsc and _force_pal cannot be enabled at the same time. Reverting to auto-detect");
		forceNTSC = NO;
		pt.put<std::string>("nes._force_ntsc", "false");
		forcePAL = NO;
		pt.put<std::string>("nes._force_pal", "false");
	}

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
			FATAL("Error: Framebuffer is not complete!");
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

		// VIDEO_FILTERS::CRT_FILTER -- NTSC composite-artifact simulation. Refer : https://www.nesdev.org/wiki/NTSC_video
		shaderProgramSource_t ntscShader = parseShader(shaderPath + "/shaders/ntsc.shaders");
		shaderProgramNTSC = createShader(ntscShader.vertexSource, ntscShader.fragmentSource);
		ntscResourcesInitialized = (shaderProgramNTSC != ZERO);
		if (!ntscResourcesInitialized)
		{
			WARN("CRT filter shader failed to compile/link -- falling back to shaderProgramBasic when CRT_FILTER is selected");
		}

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
				outSRAM.write(reinterpret_cast<const char*>(&ramByte), ONE);
			}

			outSRAM.flush();
		}

		outSRAM.close();
	}

	logCounter = ZERO;
	memset(nesEmulationCounter, ZERO, ((sizeof(nesEmulationCounter[100])) / sizeof(nesEmulationCounter[0])));

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
	nes_texture = 0;

	glDeleteTextures(1, &matrix_texture);
	matrix_texture = 0;
#else
	// 1. Delete and zero out Textures
	glDeleteTextures(1, &nes_texture);
	nes_texture = 0;

	glDeleteTextures(1, &matrix_texture);
	matrix_texture = 0;
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

			crc32_init();

			if (ENABLED)
			{
				// If database search is enabled, then try to get info if available in database...
				FLAG dbFound = NO;
				if (pAbsolute_NES_instance->absolute_NES_state.enable_nes_db == YES)
				{
					const std::string nes20db_path = "/assets/nes/db/nes20db.json";
					if (loadNESDB(_EXE_LOCATION + nes20db_path) == SUCCESS)
					{
						const bool hasTrainerDB = (pINES->completeROM[6] & 0x04) != 0;
						const uint32_t offset = 16u + (hasTrainerDB ? 512u : 0u);
						const uint32_t crc_db = crc32_compute(
							pINES->completeROM + offset,
							pAbsolute_NES_instance->absolute_NES_state.aboutRom.codeRomSize - offset);
						dbFound = lookupNESDB(crc_db);
					}
				}

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
					WARN("Handle alternate nametable arrangement");
				}

				// check whether trainer is present
				FLAG isTrainerPresent = (FLAG)pINES->iNES_Fields.iNES_header.fields.flag6.fields.trainerPresent;

				// nes 2.0
				FLAG is2p0 = pINES->iNES_Fields.iNES_header.fields.flag7.fields.nes2p0;

				// decode TV system information		
				if (is2p0 == NO)
				{
					if (pINES->iNES_Fields.iNES_header.fields.flags_8to15.ines.flag9.fields.tvSystem == TO_UINT8(NES_TV_SYSTEM::PAL)
						|| pINES->iNES_Fields.iNES_header.fields.flags_8to15.ines.flag10.fields.tvSystem == TWO
						|| pINES->iNES_Fields.iNES_header.fields.flags_8to15.ines.flag10.fields.tvSystem == THREE)
					{
						setTVSystem(NES_TV_SYSTEM::PAL);
					}
					else
					{
						setTVSystem(NES_TV_SYSTEM::NTSC);
					}
				}
				else
				{
					if (pINES->iNES_Fields.iNES_header.fields.flags_8to15.nes2p0.flag12.fields.variant == TO_UINT8(NES_TV_SYSTEM::PAL))
					{
						setTVSystem(NES_TV_SYSTEM::PAL);
					}
					else if (pINES->iNES_Fields.iNES_header.fields.flags_8to15.nes2p0.flag12.fields.variant == TO_UINT8(NES_TV_SYSTEM::NTSC))
					{
						setTVSystem(NES_TV_SYSTEM::NTSC);
					}
					else
					{
						FATAL("Unknown TV system variant in NES 2.0 header");
					}
				}
				// Must run before anything below derives timing from cpuClockHz/ppuClockHz/nesLastPpuScanline/nesTotalPpuScanline/nesFrameDots/myFPS.
				// tvSystem itself is set via setTVSystem().
				applyTVSystemTimingConfig();

				BYTE* romData = nullptr;

				if (isTrainerPresent == YES)
				{
					romData = pINES->iNES_Fields.remaining.withTrainer.romData;
				}
				else
				{
					romData = pINES->iNES_Fields.remaining.withoutTrainer.romData;
				}

				// Display some of the Cartridge information

				LOG_NEW_LINE;
				LOG("==================================================");
				LOG(" NES Cartridge Information");
				LOG("==================================================");

				const auto& header = pINES->iNES_Fields.iNES_header.fields;

				// ---------------------------------------------------------------------
				// FORMAT DETECTION
				// ---------------------------------------------------------------------

				const bool isNES2 =
					((header.flag7.raw & 0x0C) == 0x08);

				const bool isArchaic =
					((header.flag7.raw & 0x0C) == 0x04);

				LOG(" Format : %s",
					isNES2 ? "NES 2.0" :
					isArchaic ? "Archaic iNES" :
					"iNES");

				// ---------------------------------------------------------------------
				// CONSOLE TYPE
				// ---------------------------------------------------------------------

				if (isNES2)
				{
					switch (header.flag7.raw & 0x03)
					{
					case 0: LOG(" Console Type : Nintendo Entertainment System / Famicom"); BREAK;
					case 1: LOG(" Console Type : Nintendo Vs. System");                     BREAK;
					case 2: LOG(" Console Type : Nintendo PlayChoice-10");                  BREAK;
					case 3: LOG(" Console Type : Extended Console Type");                   BREAK;
					}
				}
				else
				{
					if (header.flag7.fields.vsUnisystem) LOG(" Console Type : Nintendo Vs. System");
					else if (header.flag7.fields.playChoice)  LOG(" Console Type : Nintendo PlayChoice-10");
					else                                       LOG(" Console Type : Nintendo Entertainment System / Famicom");
				}

				// ---------------------------------------------------------------------
				// MAPPER
				// ---------------------------------------------------------------------

				uint32_t mapper = ZERO;

				if (isNES2)
				{
					mapper =
						(header.flag6.fields.mapperLo)
						| (header.flag7.fields.mapperHi << 4)
						| (header.flags_8to15.nes2p0.flag8.fields.mapperNBHi << 8);
				}
				else
				{
					mapper =
						(header.flag6.fields.mapperLo)
						| (header.flag7.fields.mapperHi << 4);
				}

				LOG(" Mapper : %u", mapper);

				if (isNES2)
				{
					LOG(" Submapper : %u", header.flags_8to15.nes2p0.flag8.fields.subMapper);
					pNES_instance->NES_state.catridgeInfo.subMapper
						= static_cast<SUB_MAPPER>(header.flags_8to15.nes2p0.flag8.fields.subMapper);
				}
				else
				{
					pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SUB_MAPPER_NOT_APPLICABLE;
				}

				// ---------------------------------------------------------------------
				// PRG-ROM SIZE
				// ---------------------------------------------------------------------

				uint64_t prgRomBytes = ZERO;

				if (isNES2)
				{
					const uint32_t msb = header.flags_8to15.nes2p0.flag9.fields.prgRomMSB;
					if (msb != 0x0F)
					{
						prgRomBytes = static_cast<uint64_t>(header.sizeOfPrgRomIn16KB | (msb << 8)) * 16ULL * 1024ULL;
					}
					else
					{
						const BYTE lsb = header.sizeOfPrgRomIn16KB;
						const uint32_t exp = (lsb >> 2);
						const uint32_t multi = ((lsb & 0x03) * 2) + 1;
						prgRomBytes = (1ULL << exp) * multi;
					}
				}
				else
				{
					prgRomBytes = static_cast<uint64_t>(header.sizeOfPrgRomIn16KB) * 16ULL * 1024ULL;
				}

				LOG(" PRG-ROM : %llu KB (%llu bytes)", prgRomBytes / 1024ULL, prgRomBytes);

				// ---------------------------------------------------------------------
				// CHR-ROM SIZE
				// ---------------------------------------------------------------------

				uint64_t chrRomBytes = ZERO;

				if (isNES2)
				{
					const uint32_t msb = header.flags_8to15.nes2p0.flag9.fields.chrRomMSB;
					if (msb != 0x0F)
					{
						chrRomBytes = static_cast<uint64_t>(header.sizeOfChrRomIn8KB | (msb << 8)) * 8ULL * 1024ULL;
					}
					else
					{
						const BYTE lsb = header.sizeOfChrRomIn8KB;
						const uint32_t exp = (lsb >> 2);
						const uint32_t multi = ((lsb & 0x03) * 2) + 1;
						chrRomBytes = (1ULL << exp) * multi;
					}
				}
				else
				{
					chrRomBytes = static_cast<uint64_t>(header.sizeOfChrRomIn8KB) * 8ULL * 1024ULL;
				}

				LOG(" CHR-ROM : %llu KB (%llu bytes)", chrRomBytes / 1024ULL, chrRomBytes);
				LOG(" CHR Memory : %s", (header.sizeOfChrRomIn8KB == ZERO) ? "CHR-RAM" : "CHR-ROM");

				// ---------------------------------------------------------------------
				// RAM SIZES
				// ---------------------------------------------------------------------

				FLAG isPrgRamAvailable = NO;
				FLAG isChrRamAvailable = NO;

				if (isNES2)
				{
					const BYTE prgRamShift = header.flags_8to15.nes2p0.flag10.fields.prgVolRam;
					const BYTE prgNvRamShift = header.flags_8to15.nes2p0.flag10.fields.prgNonVolRam;
					const BYTE chrRamShift = header.flags_8to15.nes2p0.flag11.fields.chrVolRam;
					const BYTE chrNvRamShift = header.flags_8to15.nes2p0.flag11.fields.chrNonVolRam;

					const uint64_t prgRamSize = (prgRamShift == ZERO) ? ZERO : (64ULL << prgRamShift);
					const uint64_t prgNvRamSize = (prgNvRamShift == ZERO) ? ZERO : (64ULL << prgNvRamShift);
					const uint64_t chrRamSize = (chrRamShift == ZERO) ? ZERO : (64ULL << chrRamShift);
					const uint64_t chrNvRamSize = (chrNvRamShift == ZERO) ? ZERO : (64ULL << chrNvRamShift);

					LOG(" PRG-RAM : %llu KB", prgRamSize / 1024ULL);
					LOG(" PRG-NVRAM : %llu KB", prgNvRamSize / 1024ULL);
					LOG(" CHR-RAM : %llu KB", chrRamSize / 1024ULL);
					LOG(" CHR-NVRAM : %llu KB", chrNvRamSize / 1024ULL);

					if (prgRamSize > 0ULL || prgNvRamSize > 0ULL) isPrgRamAvailable = YES;
					if (chrRamSize > 0ULL || chrNvRamSize > 0ULL) isChrRamAvailable = YES;
				}
				else
				{
					const uint32_t prgRamSizeKB =
						(header.flags_8to15.ines.prgRamSize == ZERO)
						? 0u
						: (header.flags_8to15.ines.prgRamSize * 8u);
					LOG(" PRG-RAM : %u KB", prgRamSizeKB);
					if (prgRamSizeKB > 0u) isPrgRamAvailable = YES;
				}

				// ---------------------------------------------------------------------
				// MIRRORING / NAMETABLES
				// ---------------------------------------------------------------------

				switch (header.flag6.fields.nametableArrangement)
				{
				case 0:
					LOG(" Nametable Layout : Vertical Arrangement");
					LOG("                     (Horizontal Mirroring)");
					BREAK;
				case 1:
					LOG(" Nametable Layout : Horizontal Arrangement");
					LOG("                     (Vertical Mirroring)");
					BREAK;
				}

				LOG(" Alternative Nametable Layout : %s",
					header.flag6.fields.alternativeNametable ? "YES" : "NO");

				// ---------------------------------------------------------------------
				// TRAINER / BATTERY / TV SYSTEM / BUS CONFLICTS / MISC
				// ---------------------------------------------------------------------

				LOG(" Trainer Present : %s",
					header.flag6.fields.trainerPresent ? "YES (512 bytes @ $7000-$71FF)" : "NO");

				LOG(" Battery / Persistent Memory : %s",
					header.flag6.fields.hasPersistantMemory ? "YES" : "NO");

				if (isNES2)
				{
					switch (header.flags_8to15.nes2p0.flag12.fields.variant)
					{
					case 0: LOG(" TV System : NTSC");         BREAK;
					case 1: LOG(" TV System : PAL");          BREAK;
					case 2: LOG(" TV System : Multi-Region"); BREAK;
					case 3: LOG(" TV System : Dendy");        BREAK;
					}
				}
				else
				{
					switch (header.flags_8to15.ines.flag9.fields.tvSystem)
					{
					case 0: LOG(" TV System : NTSC"); BREAK;
					case 1: LOG(" TV System : PAL");  BREAK;
					}
				}

				if (isNES2 == NO)
				{
					LOG(" Bus Conflicts : %s",
						header.flags_8to15.ines.flag10.fields.busConflict ? "YES" : "NO");
				}

				LOG(" Vs. Unisystem : %s", header.flag7.fields.vsUnisystem ? "YES" : "NO");
				LOG(" PlayChoice-10 : %s", header.flag7.fields.playChoice ? "YES" : "NO");

				if (isNES2)
				{
					LOG(" Miscellaneous ROMs : %u", header.flags_8to15.nes2p0.flag14.fields.miscRoms);
					LOG(" Default Expansion Device : %u", header.flags_8to15.nes2p0.flag15.fields.expDev);
				}

				LOG("==================================================");

				// -----------------------------------------------------------------------------
				// ROM POINTERS / SIZES
				// -----------------------------------------------------------------------------

				const bool trainerPresent = (header.flag6.fields.trainerPresent == YES);

				// -----------------------------------------------------------------------------
				// DECODE PRG-ROM / CHR-ROM SIZE
				// -----------------------------------------------------------------------------

				uint64_t prgRomSizeBytes = ZERO;
				uint64_t chrRomSizeBytes = ZERO;

				bool prgExponentEncoding = NO;
				bool chrExponentEncoding = NO;

				if (isNES2)
				{
					const BYTE prgMsb = header.flags_8to15.nes2p0.flag9.fields.prgRomMSB;
					const BYTE chrMsb = header.flags_8to15.nes2p0.flag9.fields.chrRomMSB;

					if (prgMsb == 0x0F)
					{
						prgExponentEncoding = YES;
						const BYTE lsb = header.sizeOfPrgRomIn16KB;
						const uint32_t exp = (lsb >> 2);
						const uint32_t multi = ((lsb & 0x03) * 2) + 1;
						prgRomSizeBytes = (1ULL << exp) * multi;
					}
					else
					{
						prgRomSizeBytes =
							static_cast<uint64_t>(header.sizeOfPrgRomIn16KB | (prgMsb << 8)) * 0x4000ULL;
					}

					if (chrMsb == 0x0F)
					{
						chrExponentEncoding = YES;
						const BYTE lsb = header.sizeOfChrRomIn8KB;
						const uint32_t exp = (lsb >> 2);
						const uint32_t multi = ((lsb & 0x03) * 2) + 1;
						chrRomSizeBytes = (1ULL << exp) * multi;
					}
					else
					{
						chrRomSizeBytes =
							static_cast<uint64_t>(header.sizeOfChrRomIn8KB | (chrMsb << 8)) * 0x2000ULL;
					}
				}
				else
				{
					prgRomSizeBytes = static_cast<uint64_t>(header.sizeOfPrgRomIn16KB) * 0x4000ULL;
					chrRomSizeBytes = static_cast<uint64_t>(header.sizeOfChrRomIn8KB) * 0x2000ULL;
				}

				if (prgExponentEncoding)
				{
					FATAL("NES 2.0 PRG exponent/multiplier encoding not supported");
				}
				if (chrExponentEncoding)
				{
					FATAL("NES 2.0 CHR exponent/multiplier encoding not supported");
				}

				// -----------------------------------------------------------------------------
				// RAM / NVRAM
				// -----------------------------------------------------------------------------

				bool hasPrgRam = NO;
				bool hasChrRam = NO;
				bool hasBatteryBackedMemory = NO;

				uint64_t prgRamSizeBytes = ZERO;
				uint64_t prgNvRamSizeBytes = ZERO;
				uint64_t chrRamSizeBytes = ZERO;
				uint64_t chrNvRamSizeBytes = ZERO;

				if (isNES2)
				{
					const BYTE prgRamShift = header.flags_8to15.nes2p0.flag10.fields.prgVolRam;
					const BYTE prgNvRamShift = header.flags_8to15.nes2p0.flag10.fields.prgNonVolRam;
					const BYTE chrRamShift = header.flags_8to15.nes2p0.flag11.fields.chrVolRam;
					const BYTE chrNvRamShift = header.flags_8to15.nes2p0.flag11.fields.chrNonVolRam;

					prgRamSizeBytes = (prgRamShift == ZERO) ? ZERO : (64ULL << prgRamShift);
					prgNvRamSizeBytes = (prgNvRamShift == ZERO) ? ZERO : (64ULL << prgNvRamShift);
					chrRamSizeBytes = (chrRamShift == ZERO) ? ZERO : (64ULL << chrRamShift);
					chrNvRamSizeBytes = (chrNvRamShift == ZERO) ? ZERO : (64ULL << chrNvRamShift);

					hasPrgRam = (prgRamSizeBytes > ZERO) || (prgNvRamSizeBytes > ZERO);
					hasChrRam = (chrRamSizeBytes > ZERO) || (chrNvRamSizeBytes > ZERO);
					hasBatteryBackedMemory = (prgNvRamSizeBytes > ZERO) || (chrNvRamSizeBytes > ZERO);
				}
				else
				{
					prgRamSizeBytes =
						((header.flags_8to15.ines.prgRamSize == ZERO)
							? 8ULL
							: static_cast<uint64_t>(header.flags_8to15.ines.prgRamSize * 8ULL))
						* 1024ULL;

					hasPrgRam = (prgRamSizeBytes > ZERO);
					hasChrRam = (chrRomSizeBytes == ZERO);
					chrRamSizeBytes = (chrRomSizeBytes == ZERO && chrRamSizeBytes == ZERO) ? 0x2000 : chrRamSizeBytes;
					hasBatteryBackedMemory = header.flag6.fields.hasPersistantMemory;
				}

				pNES_instance->NES_state.catridgeInfo.hasPrgRam = hasPrgRam;
				pNES_instance->NES_state.catridgeInfo.hasChrRam = hasChrRam;
				pNES_instance->NES_state.catridgeInfo.hasBatteryBackedMemory = hasBatteryBackedMemory;
				pNES_instance->NES_state.catridgeInfo.prgRamSizeBytes = prgRamSizeBytes + prgNvRamSizeBytes;
				pNES_instance->NES_state.catridgeInfo.chrRamSizeBytes = chrRamSizeBytes + chrNvRamSizeBytes;
				pNES_instance->NES_state.catridgeInfo.prgRomSizeBytes = prgRomSizeBytes;
				pNES_instance->NES_state.catridgeInfo.chrRomSizeBytes = chrRomSizeBytes;

				// -----------------------------------------------------------------------------
				// APPLY MAPPER FROM HEADER  (may be overridden by DB below)
				// -----------------------------------------------------------------------------

				pNES_instance->NES_state.catridgeInfo.mapper = static_cast<MAPPER>(mapper);
				pNES_instance->NES_state.catridgeInfo.mapperID = mapper;

				// -----------------------------------------------------------------------------
				// DB OVERRIDE — DB is authoritative; WARN on any mismatch with iNES header
				// -----------------------------------------------------------------------------

				if (dbFound == YES)
				{
					const NES20DBEntry_t& db = pAbsolute_NES_instance->absolute_NES_state.dbEntry;

					// ---- PRG-ROM size ----
					const uint64_t dbPrgBytes = static_cast<uint64_t>(db.prgromSize);
					if (dbPrgBytes != ZERO && dbPrgBytes != prgRomSizeBytes)
					{
						WARN("NES20DB PRG-ROM size mismatch: iNES header=%llu bytes  DB=%llu bytes  -> using DB.",
							prgRomSizeBytes, dbPrgBytes);
						prgRomSizeBytes = dbPrgBytes;
					}

					// ---- CHR-ROM size ----
					const uint64_t dbChrBytes = static_cast<uint64_t>(db.chrromSize);
					if (dbChrBytes != ZERO && dbChrBytes != chrRomSizeBytes)
					{
						WARN("NES20DB CHR-ROM size mismatch: iNES header=%llu bytes  DB=%llu bytes  -> using DB.",
							chrRomSizeBytes, dbChrBytes);
						chrRomSizeBytes = dbChrBytes;
					}

					// ---- PRG-RAM / PRG-NVRAM ----
					const uint64_t dbPrgRamBytes = static_cast<uint64_t>(db.prgramSize);
					const uint64_t dbPrgNvRamBytes = static_cast<uint64_t>(db.prgnvramSize);
					if (dbPrgRamBytes != prgRamSizeBytes)
					{
						WARN("NES20DB PRG-RAM size mismatch: iNES header=%llu bytes  DB=%llu bytes  -> using DB.",
							prgRamSizeBytes, dbPrgRamBytes);
						prgRamSizeBytes = dbPrgRamBytes;
					}
					if (dbPrgNvRamBytes != prgNvRamSizeBytes)
					{
						WARN("NES20DB PRG-NVRAM size mismatch: iNES header=%llu bytes  DB=%llu bytes  -> using DB.",
							prgNvRamSizeBytes, dbPrgNvRamBytes);
						prgNvRamSizeBytes = dbPrgNvRamBytes;
					}
					if (dbPrgRamBytes > ZERO || dbPrgNvRamBytes > ZERO) hasPrgRam = YES;

					// ---- CHR-RAM / CHR-NVRAM ----
					const uint64_t dbChrRamBytes = static_cast<uint64_t>(db.chrramSize);
					const uint64_t dbChrNvRamBytes = static_cast<uint64_t>(db.chrnvramSize);
					if (dbChrRamBytes != chrRamSizeBytes)
					{
						WARN("NES20DB CHR-RAM size mismatch: iNES header=%llu bytes  DB=%llu bytes  -> using DB.",
							chrRamSizeBytes, dbChrRamBytes);
						chrRamSizeBytes = dbChrRamBytes;
					}
					if (dbChrNvRamBytes != chrNvRamSizeBytes)
					{
						WARN("NES20DB CHR-NVRAM size mismatch: iNES header=%llu bytes  DB=%llu bytes  -> using DB.",
							chrNvRamSizeBytes, dbChrNvRamBytes);
						chrNvRamSizeBytes = dbChrNvRamBytes;
					}
					if (dbChrRamBytes > ZERO || dbChrNvRamBytes > ZERO) hasChrRam = YES;

					// ---- mapper ----
					if (db.mapper != mapper)
					{
						WARN("NES20DB mapper mismatch: iNES header=%u  DB=%u  -> using DB.", mapper, db.mapper);
						mapper = db.mapper;
						pNES_instance->NES_state.catridgeInfo.mapper = static_cast<MAPPER>(mapper);
						pNES_instance->NES_state.catridgeInfo.mapperID = mapper;
					}

					// ---- submapper ----
					{
						const uint32_t headerSub = static_cast<uint32_t>(pNES_instance->NES_state.catridgeInfo.subMapper);
						if (isNES2 && (db.subMapper != headerSub))
						{
							WARN("NES20DB submapper mismatch: iNES header=%u  DB=%u  -> using DB.", headerSub, db.subMapper);
						}
						pNES_instance->NES_state.catridgeInfo.subMapper = static_cast<SUB_MAPPER>(db.subMapper);
					}

					// ---- mirroring ----
					if (db.mirroring != '4')
					{
						const NAMETABLE_MIRROR dbMirror =
							(db.mirroring == 'V')
							? NAMETABLE_MIRROR::VERTICAL_MIRROR
							: NAMETABLE_MIRROR::HORIZONTAL_MIRROR;

						if (dbMirror != pNES_instance->NES_state.catridgeInfo.nameTblMir)
						{
							WARN("NES20DB mirroring mismatch: iNES header=%d  DB=%c  -> using DB.",
								static_cast<int>(pNES_instance->NES_state.catridgeInfo.nameTblMir),
								db.mirroring);
							pNES_instance->NES_state.catridgeInfo.nameTblMir = dbMirror;
						}
					}

					// ---- battery ----
					{
						const uint8_t headerBattery = (uint8_t)header.flag6.fields.hasPersistantMemory;
						if (db.battery != headerBattery)
						{
							WARN("NES20DB battery mismatch: iNES header=%u  DB=%u  -> using DB.",
								headerBattery, db.battery);
							hasBatteryBackedMemory = (db.battery != 0u);
						}
					}

					// ---- console metadata (informational) ----
					if (db.consoleType != ZERO || db.consoleRegion != ZERO || db.expansionType != ZERO)
					{
						LOG(" [DB] consoleType=%u  consoleRegion=%u  expansionType=%u", db.consoleType, db.consoleRegion, db.expansionType);
						NES_TV_SYSTEM tvSystem = (db.consoleRegion == 1) ? NES_TV_SYSTEM::PAL : NES_TV_SYSTEM::NTSC;
						if (getTVSystem() != tvSystem)
						{
							WARN("NES20DB TV system mismatch: iNES header=%d  consoleRegion=%u  -> using DB.",
								static_cast<int>(getTVSystem()),
								static_cast<int>(tvSystem));
						}
						setTVSystem((db.consoleRegion == 1) ? NES_TV_SYSTEM::PAL : NES_TV_SYSTEM::NTSC);
						applyTVSystemTimingConfig();
					}
					if (db.vsHardware != ZERO || db.vsPpu != ZERO)
					{
						LOG(" [DB] vsHardware=%u  vsPpu=%u", db.vsHardware, db.vsPpu);
					}
					if (db.miscRoms != ZERO)
					{
						WARN("NES20DB miscROMs=%u — not handled.", db.miscRoms);
					}

					LOG(" [DB] mapper=%u submapper=%u mirror=%c battery=%u prgRom=%llu chrRom=%llu",
						db.mapper, db.subMapper, db.mirroring, db.battery, prgRomSizeBytes, chrRomSizeBytes);
					LOG("==================================================");
				}

				// -----------------------------------------------------------------------------
				// ROM POINTERS
				// -----------------------------------------------------------------------------

				const BYTE* prgRom = romData + (trainerPresent ? 512 : 0);
				const BYTE* chrRom = prgRom + prgRomSizeBytes;

				// -----------------------------------------------------------------------------
				// BANK COUNTS
				// -----------------------------------------------------------------------------

				const uint32_t prg16kBanks = static_cast<uint32_t>(prgRomSizeBytes / 0x4000ULL);
				const uint32_t chr8kBanks = static_cast<uint32_t>(chrRomSizeBytes / 0x2000ULL);

				// -----------------------------------------------------------------------------
				// CPU/PPU WINDOWS
				// -----------------------------------------------------------------------------

				BYTE* cpuCart = &(pNES_instance->NES_state.cpuMemory.NESMemoryMap.catridgeMappedMemory[0x3FE0]);
				BYTE* ppuChr = pNES_instance->NES_state.ppuMemory.NESMemoryMap.patternTable.raw;

				// -----------------------------------------------------------------------------
				// STORE GLOBAL FLAGS
				// -----------------------------------------------------------------------------

				pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;

				if (isNES2 == NO)
				{
					pNES_instance->NES_state.catridgeInfo.isBusConflictPresent =
						header.flags_8to15.ines.flag10.fields.busConflict;
				}

				// -----------------------------------------------------------------------------
				// CLEAR CART MEMORY
				// -----------------------------------------------------------------------------

				memset(pNES_catridgeMemory->maxCatridgePRGROM, 0, sizeof(pNES_catridgeMemory->maxCatridgePRGROM));
				memset(pNES_catridgeMemory->maxCatridgeCHRROM, 0, sizeof(pNES_catridgeMemory->maxCatridgeCHRROM));

				// -----------------------------------------------------------------------------
				// COPY FULL ROMS
				// -----------------------------------------------------------------------------

				if (prgRomSizeBytes > ZERO)
				{
					memcpy_portable(pNES_catridgeMemory->maxCatridgePRGROM, prgRomSizeBytes, prgRom, prgRomSizeBytes);
				}

				if (chrRomSizeBytes > ZERO)
				{
					memcpy_portable(pNES_catridgeMemory->maxCatridgeCHRROM, chrRomSizeBytes, chrRom, chrRomSizeBytes);
				}

				// -----------------------------------------------------------------------------
				// Handling of case where prg16kBanks = 0 but prgRomSizeBytes != 0
				// -----------------------------------------------------------------------------

				FLAG zeroBanksHandled = NO;
				if (prg16kBanks == ZERO && prgRomSizeBytes > ZERO)
				{
					const uint32_t mirrors = static_cast<uint32_t>(0x4000ULL / prgRomSizeBytes);
					for (uint32_t i = 0; i < (2 * mirrors); ++i)
					{
						memcpy_portable(&(cpuCart[i * prgRomSizeBytes]), prgRomSizeBytes, prgRom, prgRomSizeBytes);
					}
					zeroBanksHandled = YES;
				}

				// Handle forces TV type
				if (forcePAL)
				{
					WARN("TV System overriden to PAL via CONFIG.ini");
					setTVSystem(NES_TV_SYSTEM::PAL);
					applyTVSystemTimingConfig();
				}
				else if (forceNTSC)
				{
					WARN("TV System overriden to NTSC via CONFIG.ini");
					setTVSystem(NES_TV_SYSTEM::NTSC);
					applyTVSystemTimingConfig();
				}

				// -----------------------------------------------------------------------------
				// MAPPER INIT
				// -----------------------------------------------------------------------------

				switch (pNES_instance->NES_state.catridgeInfo.mapper)
				{
				case MAPPER::NROM:
				case MAPPER::CPROM:
				{
					if ((prg16kBanks != ONE) && (prg16kBanks != TWO) && (zeroBanksHandled == NO))
					{
						FATAL("Invalid PRG-ROM size for NROM / CPROM");
					}

					if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::CPROM)
					{
						if (isNES2)
						{
							const BYTE sub = header.flags_8to15.nes2p0.flag8.fields.subMapper;
							if (sub == 0 || sub == 2) pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = YES;
							else if (sub == 1)             pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;
						}
						else
						{
							pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = YES;
						}

						memset(pNES_catridgeMemory->maxCatridgeCHRROM, 0, 0x4000);
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[0x4000]), 0x4000);
						}
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_105:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.mmc1), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.mmc1));

					pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.raw = 0x1C;
					pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Lo = ZERO;
					pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Hi = (prg16kBanks > ZERO) ? (prg16kBanks - ONE) : ZERO;
					pNES_instance->NES_state.catridgeInfo.mmc1.prgBank32 = ZERO;
					pNES_instance->NES_state.catridgeInfo.mmc1.prgRamEnable = hasPrgRam ? YES : NO;

					if (prg16kBanks < ONE && zeroBanksHandled == NO)
					{
						FATAL("Invalid MMC1 PRG-ROM");
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
						}
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					auto& ev = pNES_instance->NES_state.catridgeInfo.mmc1.nes_event;
					ev.initState = 0;
					ev.irqCounter = 0;
					ev.irqEnabled = NO;
					ev.dipSwitches = 0xB; // TODO: Populated from config/DIP setting

					// NES-EVENT powers up with I bit = 1 (chrBank4Lo bit 4 forced high)
					pNES_instance->NES_state.catridgeInfo.mmc1.chrBank4Lo |= 0x10;

					// Until unlocked, force 32KB bank 0 at $8000-$FFFF
					// (prgBank32 = 0, pp = 0/1 mode)
					BREAK;
				}
				case MAPPER::MMC1:
				case MAPPER::INES_MAPPER_155:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.mmc1), ZERO,
						sizeof(pNES_instance->NES_state.catridgeInfo.mmc1));

					const auto& header = pINES->iNES_Fields.iNES_header.fields;
					const bool isNES2 = ((header.flag7.raw & 0x0C) == 0x08);

					// Calculate actual PRG RAM size in bytes
					size_t prgRamSizeBytes = ZERO;
					if (isNES2)
					{
						const uint8_t volShift = header.flags_8to15.nes2p0.flag10.fields.prgVolRam;
						const uint8_t nonVolShift = header.flags_8to15.nes2p0.flag10.fields.prgNonVolRam;
						if (volShift > ZERO) prgRamSizeBytes += (64ULL << volShift);
						if (nonVolShift > ZERO) prgRamSizeBytes += (64ULL << nonVolShift);
					}
					else
					{
						// iNES 1.0 fallback: assume standard 8 KB PRG-RAM unless Byte 10 explicitly specifies none
						const bool ramDisabled = (header.flags_8to15.ines.flag10.fields.prgRamNotPresent == SET);
						if (!ramDisabled)
						{
							prgRamSizeBytes = (header.flags_8to15.ines.prgRamSize > ZERO)
								? ((size_t)header.flags_8to15.ines.prgRamSize * 0x2000U)
								: 0x2000U;
						}
					}

					// Heuristic Submapper Identification (runs if submapper isn't explicitly set by header/database)
					if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SUB_MAPPER_NOT_APPLICABLE ||
						pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::VRC2A) // 0
					{
						if (prgRomSizeBytes == 0x80000U)
						{
							// 512 KB PRG-ROM + 8 KB PRG-RAM -> SUROM
							pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SUROM;
						}
						else if (prgRamSizeBytes == 0x8000U)
						{
							// 32 KB PRG-RAM -> SXROM
							pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SXROM;
						}
						else if (prgRamSizeBytes == 0x4000U)
						{
							// 16 KB PRG-RAM -> SOROM or SZROM
							pNES_instance->NES_state.catridgeInfo.subMapper = (chrRomSizeBytes > ZERO) ? SUB_MAPPER::SZROM : SUB_MAPPER::SOROM;
						}
						else if (prgRamSizeBytes == 0x2000U)
						{
							// 8 KB PRG-RAM
							if (chrRomSizeBytes == ZERO)
							{
								// CHR-RAM + PRG-RAM -> SNROM
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SNROM;
							}
							else if (chrRomSizeBytes == 0x20000U)
							{
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SKROM;
							}
							else if (prgRomSizeBytes == 0x8000U)
							{
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SIROM;
							}
							else if (prgRomSizeBytes == 0x10000U)
							{
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SAROM;
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SJROM;
							}
						}
						else if (prgRamSizeBytes == ZERO)
						{
							// No PRG-RAM
							if (chrRomSizeBytes == ZERO)
							{
								// CHR-RAM + No PRG-RAM -> SGROM
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SGROM;
							}
							else if (chrRomSizeBytes == 0x20000U)
							{
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SLROM;
							}
							else if (prgRomSizeBytes == 0x8000U)
							{
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SHROM;
							}
							else if (prgRomSizeBytes == 0x10000U)
							{
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SBROM;
							}
							else
							{
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::SFROM;
							}
						}

						if (pNES_instance->NES_state.catridgeInfo.subMapper != SUB_MAPPER::SUB_MAPPER_NOT_APPLICABLE)
						{
							auto getSubmapperName = [](SUB_MAPPER sm) -> const char* 
								{
									switch (sm)
									{
									case SUB_MAPPER::SGROM:    RETURN "SGROM";
									case SUB_MAPPER::SAROM:    RETURN "SAROM";
									case SUB_MAPPER::SBROM:    RETURN "SBROM";
									case SUB_MAPPER::SCROM:    RETURN "SCROM";
									case SUB_MAPPER::SC1ROM:   RETURN "SC1ROM";
									case SUB_MAPPER::SFROM:    RETURN "SFROM";
									case SUB_MAPPER::SF1ROM:   RETURN "SF1ROM";
									case SUB_MAPPER::SFEXPROM: RETURN "SFEXPROM";
									case SUB_MAPPER::SHROM:    RETURN "SHROM";
									case SUB_MAPPER::SH1ROM:   RETURN "SH1ROM";
									case SUB_MAPPER::SIROM:    RETURN "SIROM";
									case SUB_MAPPER::SIEPROM:  RETURN "SIEPROM";
									case SUB_MAPPER::SJROM:    RETURN "SJROM";
									case SUB_MAPPER::SKROM:    RETURN "SKROM";
									case SUB_MAPPER::SKEPROM:  RETURN "SKEPROM";
									case SUB_MAPPER::SLROM:    RETURN "SLROM";
									case SUB_MAPPER::SL1ROM:   RETURN "SL1ROM";
									case SUB_MAPPER::SL2ROM:   RETURN "SL2ROM";
									case SUB_MAPPER::SL3ROM:   RETURN "SL3ROM";
									case SUB_MAPPER::SLRROM:   RETURN "SLRROM";
									case SUB_MAPPER::SMROM:    RETURN "SMROM";
									case SUB_MAPPER::SNWEPROM: RETURN "SNWEPROM";
									case SUB_MAPPER::SZROM:    RETURN "SZROM";
									case SUB_MAPPER::SNROM:    RETURN "SNROM";
									case SUB_MAPPER::SOROM:    RETURN "SOROM";
									case SUB_MAPPER::SUROM:    RETURN "SUROM";
									case SUB_MAPPER::SXROM:    RETURN "SXROM";
									default:                   RETURN "UNKNOWN";
									}
								};

							LOG(" Submapper : %s", getSubmapperName(pNES_instance->NES_state.catridgeInfo.subMapper));
							LOG("==================================================");
						}
					}

					// Refer : https://www.nesdev.org/wiki/Programming_MMC1 -- "there is not much of a reason to use
					// 4 KB bankswitching with CHR-RAM, it is wise for programs to just set 8 KB bankswitching mode" --
					// meaning well-behaved CHR-RAM games (like this one) may never write the control register at all,
					// trusting the power-on default to already be 8KB mode (C=0). 0x1C (C=1, 4KB mode) left both
					// halves of CHR-RAM aliased onto the same 4KB when a game never explicitly selects a CHR mode.
					pNES_instance->NES_state.catridgeInfo.mmc1.intfControlReg.raw = 0x0C;
					pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Lo = ZERO;
					pNES_instance->NES_state.catridgeInfo.mmc1.prgBank16Hi = (prg16kBanks > ZERO) ? (prg16kBanks - ONE) : ZERO;
					pNES_instance->NES_state.catridgeInfo.mmc1.prgBank32 = ZERO;
					pNES_instance->NES_state.catridgeInfo.mmc1.prgRamEnable = (prgRamSizeBytes > ZERO) ? YES : NO;

					if (prg16kBanks < ONE && zeroBanksHandled == NO)
					{
						FATAL("Invalid MMC1 PRG-ROM");
					}

					if (zeroBanksHandled == NO)
					{
						if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SEROM_SHROM_SH1ROM)
						{
							// SEROM / SHROM / SH1ROM: fixed 32KB PRG.
							memcpy_portable(&(cpuCart[0x0000]), 0x8000, prgRom, 0x8000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

							if (prg16kBanks == ONE)
							{
								memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
							}
							else
							{
								memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
							}
						}
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					pNES_instance->NES_state.catridgeInfo.mmc1.isMmc1A =
						(pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_155) ? YES : NO;

					// MMC1A: PRG-RAM always enabled, not gated by $E000 bit 4
					if (pNES_instance->NES_state.catridgeInfo.mmc1.isMmc1A == YES)
					{
						pNES_instance->NES_state.catridgeInfo.mmc1.prgRamEnable = YES;
					}

					BREAK;
				}
				case MAPPER::UxROM_002:
				case MAPPER::INES_MAPPER_180:
				{
					if (isNES2)
					{
						const BYTE sub = header.flags_8to15.nes2p0.flag8.fields.subMapper;
						if (sub == 0 || sub == 2) pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = YES;
						else if (sub == 1)             pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;
					}
					else
					{
						pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = YES;
					}

					memset(&(pNES_instance->NES_state.catridgeInfo.uxrom_002), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.uxrom_002));

					if (prg16kBanks < ONE && zeroBanksHandled == NO)
					{
						FATAL("Invalid UxROM PRG-ROM");
					}

					pNES_instance->NES_state.catridgeInfo.uxrom_002.prgBank16 = ZERO;

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
						}
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::CNROM:
				case MAPPER::J87:
				{
					if (pNES_instance->NES_state.catridgeInfo.mapper != MAPPER::J87)
					{
						if (isNES2)
						{
							const BYTE sub = header.flags_8to15.nes2p0.flag8.fields.subMapper;
							if (sub == 0 || sub == 2) pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = YES;
							else if (sub == 1)             pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;
						}
						else
						{
							pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = YES;
						}
					}

					memset(&(pNES_instance->NES_state.catridgeInfo.cnrom), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.cnrom));

					if ((prg16kBanks != ONE) && (prg16kBanks != TWO) && (zeroBanksHandled == NO))
					{
						FATAL("Invalid PRG-ROM size for CNROM");
					}
					if (chrRomSizeBytes <= ZERO)
					{
						FATAL("CNROM requires CHR-ROM");
					}

					pNES_instance->NES_state.catridgeInfo.cnrom.chrBank8 = ZERO;

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[0x4000]), 0x4000);
						}
					}

					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::MMC3:
				case MAPPER::INES_MAPPER_037:
				case MAPPER::INES_MAPPER_047:
				case MAPPER::INES_MAPPER_119:
				case MAPPER::INES_MAPPER_118:
				case MAPPER::INES_MAPPER_268:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.mmc3), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.mmc3));

					pNES_instance->NES_state.catridgeInfo.mmc3
						.exRegisters.prgRamProtect_oddAk.fields.prgRamEnable = hasPrgRam ? YES : NO;

					// CHR Bank Registers (R0 - R5)
					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2a = 0; // R0 (2KB bank at $0000)
					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank2b = 2; // R1 (2KB bank at $0800)
					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1a = 4; // R2 (1KB bank at $1000)
					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1b = 5; // R3 (1KB bank at $1100)
					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1c = 6; // R4 (1KB bank at $1200)
					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.chrBank1d = 7; // R5 (1KB bank at $1300)

					// PRG Bank Registers (R6 - R7)
					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8a = 0; // R6 (8KB bank at $8000)
					pNES_instance->NES_state.catridgeInfo.mmc3.inRegisters.prgBank8b = 1; // R7 (8KB bank at $A000)

					if (ENABLED)
					{
						const bool hasTrainerDB = (pINES->completeROM[6] & 0x04) != 0;
						const uint32_t offset = 16u + (hasTrainerDB ? 512u : 0u);
						const uint32_t crc_db = crc32_compute(
							pINES->completeROM + offset,
							pAbsolute_NES_instance->absolute_NES_state.aboutRom.codeRomSize - offset);

						if ((crc_db == 0xF312D1DEu) || (crc_db == 0x633AFe6Fu))
						{
							pNES_instance->NES_state.catridgeInfo.mmc3.isRevA = YES;
						}
						else if (crc_db == 0xA512BDF6u)
						{
							pNES_instance->NES_state.catridgeInfo.mmc3.isRevA = YES;
							pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::MMC6;
						}
					}

					// Heuristic TxROM Board Identification (runs if submapper isn't explicitly set)
					if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SUB_MAPPER_NOT_APPLICABLE ||
						pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::VRC2A) // 0
					{
						const uint32_t prgRomSizeBytes = prg16kBanks * 0x4000U;

						if (prgRomSizeBytes == 0x10000U) // 64 KB PRG-ROM
						{
							pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::TBROM;
						}
						else if (prgRomSizeBytes == 0x8000U) // 32 KB PRG-ROM
						{
							pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::TEROM;
						}
						else // 128 KB, 256 KB, or 512 KB PRG-ROM
						{
							if (chrRomSizeBytes == ZERO)
							{
								// Uses CHR-RAM (8 KB)
								// TGROM has PRG-RAM + CHR-RAM. TNROM has PRG-RAM + CHR-RAM with battery/extra wiring.
								// Default to TGROM for standard CHR-RAM MMC3 boards.
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::TGROM;
							}
							else
							{
								// Uses CHR-ROM
								if (hasPrgRam)
								{
									// TKROM has battery-backed PRG-RAM, TSROM does not
									pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::TKROM;
								}
								else
								{
									// TLROM has no PRG-RAM
									pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::TLROM;
								}
							}
						}

						// Specific multi-mapper / board overrides (from explicit iNES mappers)
						if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_118)
						{
							pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::TLSROM;
						}
						else if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_119)
						{
							pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::TQROM;
						}

						if (pNES_instance->NES_state.catridgeInfo.subMapper != SUB_MAPPER::SUB_MAPPER_NOT_APPLICABLE)
						{
							auto getTxRomBoardName = [](SUB_MAPPER sm) -> const char*
								{
									switch (sm)
									{
									case SUB_MAPPER::TBROM:   RETURN "TBROM";
									case SUB_MAPPER::TEROM:   RETURN "TEROM";
									case SUB_MAPPER::TFROM:   RETURN "TFROM";
									case SUB_MAPPER::TGROM:   RETURN "TGROM";
									case SUB_MAPPER::TKROM:   RETURN "TKROM";
									case SUB_MAPPER::TK1ROM:  RETURN "TK1ROM";
									case SUB_MAPPER::TKSROM:  RETURN "TKSROM";
									case SUB_MAPPER::TKEPROM: RETURN "TKEPROM";
									case SUB_MAPPER::TLROM:   RETURN "TLROM";
									case SUB_MAPPER::TL1ROM:  RETURN "TL1ROM";
									case SUB_MAPPER::TL2ROM:  RETURN "TL2ROM";
									case SUB_MAPPER::TLBROM:  RETURN "TLBROM";
									case SUB_MAPPER::TLSROM:  RETURN "TLSROM";
									case SUB_MAPPER::TNROM:   RETURN "TNROM";
									case SUB_MAPPER::TQROM:   RETURN "TQROM";
									case SUB_MAPPER::TR1ROM:  RETURN "TR1ROM";
									case SUB_MAPPER::TSROM:   RETURN "TSROM";
									case SUB_MAPPER::TVROM:   RETURN "TVROM";
									default:                  RETURN "UNKNOWN TxROM";
									}
								};

							LOG(" TxROM Board : %s", getTxRomBoardName(pNES_instance->NES_state.catridgeInfo.subMapper));
							LOG("==================================================");
						}
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);
						memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					// Sanity check: large CHR RAM (> 8KB) is only valid when no CHR ROM is present.
					// maxCatridgeCHRROM is reused as the CHR RAM backing store in this case, which
					// is only safe if CHR ROM size is zero (otherwise ROM data would be corrupted).
					// A well-formed iNES/NES 2.0 header should never have both simultaneously.
					if ((chrRamSizeBytes + chrNvRamSizeBytes) > 0x2000ULL && chrRomSizeBytes > ZERO)
					{
						FATAL("Invalid header: large CHR RAM and CHR ROM both present — maxCatridgeCHRROM cannot be safely reused");
					}

					// -----------------------------------------------------------------
					// Mapper 118 (TxSROM) only: seed txsrom.ntPage[] from the iNES
					// header mirroring flag.
					//
					// iNES flag6 bit 0: 0 = vertical mirroring, 1 = horizontal
					// -----------------------------------------------------------------
					if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_118)
					{
						auto& txs = pNES_instance->NES_state.catridgeInfo.mmc3.txsrom;

						const bool isHorizontal =
							(pINES->iNES_Fields.iNES_header.fields.flag6.fields.nametableArrangement == SET);

						if (isHorizontal)
						{
							// Horizontal: NT0->page0, NT1->page0, NT2->page1, NT3->page1
							txs.ntPage[0] = 0;
							txs.ntPage[1] = 0;
							txs.ntPage[2] = 1;
							txs.ntPage[3] = 1;
						}
						else
						{
							// Vertical: NT0->page0, NT1->page1, NT2->page0, NT3->page1
							txs.ntPage[0] = 0;
							txs.ntPage[1] = 1;
							txs.ntPage[2] = 0;
							txs.ntPage[3] = 1;
						}
					}

					BREAK;
				}
				case MAPPER::RAMBO1:
				case MAPPER::INES_MAPPER_158:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.mmc3), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.mmc3));

					auto& rb = pNES_instance->NES_state.catridgeInfo.mmc3.rambo1;
					rb.irqEnabled = NO;
					rb.irqCycleMode = NO;
					rb.needReload = NO;
					rb.irqCounter = RESET;
					rb.irqReloadValue = RESET;
					rb.cpuClockCounter = RESET;
					rb.forceClock = NO;
					rb.irqDelay = RESET;
					rb.currentRegister = RESET;
					memset(rb.reg, 0, sizeof(rb.reg));

					// Mapper 158 only: seed txsrom.ntPage[] from iNES header.
					// Mapper 64 uses nameTblMir (set globally from header) — no seed needed.
					if (pNES_instance->NES_state.catridgeInfo.mapper == MAPPER::INES_MAPPER_158)
					{
						auto& txs = pNES_instance->NES_state.catridgeInfo.mmc3.txsrom;
						const bool isHorizontal =
							(pINES->iNES_Fields.iNES_header.fields.flag6.fields.nametableArrangement == SET);
						if (isHorizontal)
						{
							txs.ntPage[0] = 0; txs.ntPage[1] = 0;
							txs.ntPage[2] = 1; txs.ntPage[3] = 1;
						}
						else
						{
							txs.ntPage[0] = 0; txs.ntPage[1] = 1;
							txs.ntPage[2] = 0; txs.ntPage[3] = 1;
						}
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::MMC5:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.mmc5), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.mmc5));

					// NOTE: Per Mesen and nesdev, Romance of the 3 Kingdoms 2 expects 8KB PRG mode on power-up
					// Refer https://www.nesdev.org/wiki/MMC5#PRG_Mode_Register_($5100)
					pNES_instance->NES_state.catridgeInfo.mmc5.prgMode = 3;
					pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[0x5100 - UNMAPPED_START_ADDRESS] = 0xFF;

					// NOTE: Games expect $5117 = 0xFF on power-up (last ROM page at $E000-$FFFF)
					// Force bit 7 high since $5117 is always ROM
					pNES_instance->NES_state.catridgeInfo.mmc5.prgBanks[4] = 0xFF;
					pNES_cpuMemory->NESMemoryMap.catridgeMappedMemory[0x5117 - UNMAPPED_START_ADDRESS] = 0xFF;

					// NOTE: PRG-RAM writes require protect registers unlocked.
					// Initialize locked (protect1 != 2 or protect2 != 1 means no writes)
					pNES_instance->NES_state.catridgeInfo.mmc5.prgRamProtect1 = ZERO;
					pNES_instance->NES_state.catridgeInfo.mmc5.prgRamProtect2 = ZERO;

					// Default nametable: all slots map to CIRAM bank 0 (nametableMapping = 0x00)
					// Games must configure $5105 before use

					// Copy last PRG bank to both halves of cpuCart for initial NMI vector access
					// (runtime reads use the full dynamic banking logic above)
					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);
						memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
					}

					// CHR-ROM loaded into maxCatridgeCHRROM (done before this switch)
					// PRG-ROM loaded into maxCatridgePRGROM (done before this switch)

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					// PRG-RAM starts zeroed (from memset above)

					memset(&pNES_instance->NES_state.catridgeInfo.mmc5.mmc5Audio, 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.mmc5.mmc5Audio));
					// Shift register init for noise is NES standard; MMC5 pulses start silent
					pNES_instance->NES_state.catridgeInfo.mmc5.mmc5Audio.pulse[0].dacInput = MUTE_AUDIO;
					pNES_instance->NES_state.catridgeInfo.mmc5.mmc5Audio.pulse[1].dacInput = MUTE_AUDIO;
					pNES_instance->NES_state.catridgeInfo.mmc5.mmc5Audio.pcmRawSample = 0;

					BREAK;
				}
				case MAPPER::AxROM:
				{
					if (isNES2)
					{
						const BYTE sub = header.flags_8to15.nes2p0.flag8.fields.subMapper;
						pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = (sub == 2) ? YES : NO;
					}
					else
					{
						pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;
					}

					memset(&(pNES_instance->NES_state.catridgeInfo.axrom), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.axrom));

					pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;

					if (prgRomSizeBytes < 0x8000ULL && zeroBanksHandled == NO)
					{
						WARN("AxROM requires >= 32KB PRG-ROM");
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
							memcpy_portable(&(pNES_catridgeMemory->maxCatridgePRGROM[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[0x4000]), 0x4000);
						}
					}

					if ((hasChrRam == NO) && (chrRomSizeBytes == ZERO))
					{
						WARN("AxROM has no CHR memory");
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_218:
				{
					pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;

					// Mapper 218 uses the iNES header differently:
					// bit 3 = single-screen mode
					// bit 0 = variant selection
					if (header.flag6.fields.alternativeNametable == SET)
					{
						// $A8 / $A9
						if (header.flag6.fields.nametableArrangement == SET)
						{
							// $A9 - Single-screen B (CIRAM A10 <- PPU A13)
							pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_HI_MIRROR;
						}
						else
						{
							// $A8 - Single-screen A (CIRAM A10 <- PPU A12)
							pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
						}
					}
					else
					{
						// $A0 / $A1
						if (header.flag6.fields.nametableArrangement == SET)
						{
							// $A1 - CIRAM A10 <- PPU A10 ("Vertical mirroring")
							pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
						}
						else
						{
							// $A0 - CIRAM A10 <- PPU A11 ("Horizontal mirroring")
							pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
						}
					}

					if (prgRomSizeBytes < 0x8000ULL && zeroBanksHandled == NO)
					{
						WARN("Mapper 218 requires >= 32KB PRG-ROM");
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
							memcpy_portable(&(pNES_catridgeMemory->maxCatridgePRGROM[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[0x4000]), 0x4000);
						}
					}

					// Mapper 218 has no cartridge CHR-ROM/CHR-RAM.
					// Pattern tables are backed by the console's internal CIRAM.

					BREAK;
				}
				case MAPPER::MMC2:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.mmc2), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.mmc2));

					pNES_instance->NES_state.catridgeInfo.mmc2.chrBankLatch[0] = 0xFD;
					pNES_instance->NES_state.catridgeInfo.mmc2.chrBankLatch[1] = 0xFD;

					const uint32_t prg8kBanks = prg16kBanks << 1;
					if (prg8kBanks < ONE && zeroBanksHandled == NO)
					{
						FATAL("Invalid MMC2 PRG-ROM");
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x2000, prgRom, 0x2000);

						switch (prg8kBanks)
						{
						case ZERO: case ONE: case TWO: case THREE:
							FATAL("Invalid MMC2 PRG-ROM");
							BREAK;
						default:
							for (uint32_t i = 0; i < 3; ++i)
							{
								memcpy_portable(
									&(cpuCart[0x2000 + (i * 0x2000)]),
									0x2000,
									&(prgRom[((prg8kBanks - 3 + i) * 0x2000)]),
									0x2000);
							}
							BREAK;
						}
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::MMC4:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.mmc4), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.mmc4));

					pNES_instance->NES_state.catridgeInfo.mmc4.chrBankLatch[0] = 0xFD;
					pNES_instance->NES_state.catridgeInfo.mmc4.chrBankLatch[1] = 0xFD;
					pNES_instance->NES_state.catridgeInfo.mmc4.prgBank16 = ZERO;

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
						}
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::COLOR_DREAMS:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.colorDreams), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.colorDreams));

					pNES_instance->NES_state.catridgeInfo.colorDreams.prgBank32 = ZERO;

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x8000, prgRom, 0x8000);
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_014:
				{
					memset(&pNES_instance->NES_state.catridgeInfo.ines014, 0, sizeof(pNES_instance->NES_state.catridgeInfo.ines014));
					memset(&pNES_instance->NES_state.catridgeInfo.vrc24, 0, sizeof(pNES_instance->NES_state.catridgeInfo.vrc24));
					memset(&pNES_instance->NES_state.catridgeInfo.mmc3, 0, sizeof(pNES_instance->NES_state.catridgeInfo.mmc3));

					// Cold-boot: supervisorReg=0 -> VRC2 mode, CHR A18=0 for all three
					// regions. The wiki doesn't document mapper 014's own reset state, but
					// "mode bit clear" is the natural default and matches mode 0 = VRC2.
					pNES_instance->NES_state.catridgeInfo.ines014.supervisorReg = ZERO;

					// VRC2 register reset state -- CHR bank registers power on to $FF per
					// the mapper 116 wiki note ("The VRC2 CHR-ROM registers are initialized
					// on power-up to $FF"), which applies to this same Huang-1 ASIC.
					for (BYTE i = ZERO; i < EIGHT; ++i)
					{
						pNES_instance->NES_state.catridgeInfo.vrc24.chrBank[i] = 0xFF;
					}
					pNES_instance->NES_state.catridgeInfo.vrc24.prgBank0 = ZERO;
					pNES_instance->NES_state.catridgeInfo.vrc24.prgBank1 = ONE;
					pNES_instance->NES_state.catridgeInfo.vrc24.swapMode = ZERO;

					pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;

					BREAK;
				}
				case MAPPER::INES_MAPPER_015:
				{
					// Real K-1029/K-1030P hardware has no PRG-RAM and no header-declared
					// CHR-RAM (CHR is a fixed 8KB unbanked chip on the board). But per the
					// wiki's own note, almost every ROM claiming mapper 15 is actually a
					// mapper-hack expecting 8KB of PRG-RAM at $6000-$7FFF to just exist --
					// so force it on regardless of what the header says, matching every
					// mainstream emulator's compromise here.

					// Standard PRG/CHR-ROM setup -- same as every other mapper case above.
					// (If your existing cases do this via a shared helper/memcpy rather than
					// inline, call that here instead; this is just the minimal set mapper 15
					// itself needs.)
					memset(&pNES_instance->NES_state.catridgeInfo.ines015, 0, sizeof(pNES_instance->NES_state.catridgeInfo.ines015));

					// Power-on / reset latch state: "all bits clear" per the wiki, i.e.
					// mode SS=0 (NROM-256) and P=0. Using 0x8000 as the sentinel address
					// (rather than a bare 0) just keeps `latchedAddr & 0x03` meaningful
					// without needing a separate "has this ever been written" flag.
					pNES_instance->NES_state.catridgeInfo.ines015.latchedAddr = 0x8000;
					pNES_instance->NES_state.catridgeInfo.ines015.latchedData = ZERO;

					// Mirroring reflects the power-on latch value too: M bit (data bit 6) is
					// 0, so vertical.
					pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;

					BREAK;
				}
				case MAPPER::INES_MAPPER_016:
				{
					pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;

					auto& m016 = pNES_instance->NES_state.catridgeInfo.ines016;

					memset(m016.chrBank, 0, sizeof(m016.chrBank));
					m016.prgPage = ZERO;
					m016.prgBankSelectUpper = ZERO;

					// Mesen defaults Window 0 to page 0, and Window 1 to page 0x0F at boot
					m016.prgBank = ZERO;

					m016.irqCountingEnable = false;
					m016.irqCounter = ZERO;
					m016.irqLatch = ZERO;
					m016.mirroringMode = ZERO;

					if (prg16kBanks < ONE && zeroBanksHandled == NO)
					{
						FATAL("Invalid Mapper 016 PRG-ROM size");
					}

					// Fallback heuristics if the database didn't explicitly populate subMapper ahead of time
					if (pNES_instance->NES_state.catridgeInfo.subMapper == SUB_MAPPER::SUB_MAPPER_NOT_APPLICABLE ||
						pNES_instance->NES_state.catridgeInfo.subMapper == static_cast<SUB_MAPPER>(ZERO))
					{
						if (isNES2)
						{
							// Accesses the submapper field directly via your custom nested union layout
							const BYTE sub = header.flags_8to15.nes2p0.flag8.fields.subMapper;
							pNES_instance->NES_state.catridgeInfo.subMapper = static_cast<SUB_MAPPER>(sub);
						}

						// Heuristics for standard legacy iNES files or unassigned NES 2.0 submappers
						if (pNES_instance->NES_state.catridgeInfo.subMapper == static_cast<SUB_MAPPER>(ZERO))
						{
							// Check the persistent memory flag (battery) using your exact structural path
							if (header.flag6.fields.hasPersistantMemory)
							{
								// Default to 24C02 (256B EEPROM) as the baseline standard fallback for saves
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::BANDAI_LZ93D50_24C02;
							}
							else
							{
								// Boards without battery/persistent flags lack a serial data chip entirely
								pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::BANDAI_FCG_1_2;
							}
						}

						LOG(" Mapper 016 Assigned Submapper : %d", static_cast<int16_t>(pNES_instance->NES_state.catridgeInfo.subMapper));
						LOG("==================================================");
					}

					// Power-On Defaults: 
					// $8000-$BFFF is switchable and defaults to the first bank (0)
					// $C000-$FFFF is hardwired/fixed to the last 16KB PRG-ROM bank
					pNES_instance->NES_state.catridgeInfo.ines016.prgBank = ZERO;

					// Map initial 16KB CPU memory bank views
					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
						}
					}

					// Map initial 8KB of CHR-ROM sequentially to PPU pattern tables ($0000-$1FFF)
					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					// Initialize the Serial EEPROM non-volatile data storage array to an unprogrammed state (0xFF)
					memset(pNES_instance->NES_state.catridgeInfo.ines016.dataArray, 0xFF, 256);

					BREAK;
				}
				case MAPPER::INES_MAPPER_018:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.jaleco18), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.jaleco18));

					auto& j18 = pNES_instance->NES_state.catridgeInfo.jaleco18;

					// PRG: banks 0/1/2 start at 0/1/2; last bank fixed in read path
					j18.prgBank[0] = 0;
					j18.prgBank[1] = 1;
					j18.prgBank[2] = 2;

					// CHR: identity map
					for (uint32_t i = 0; i < 8; ++i)
					{
						j18.chrBank[i] = (uint8_t)i;
					}

					j18.irqEnabled = NO;
					j18.irqCounter = 0;
					j18.irqCounterSize = 0;

					BREAK;
				}
				case MAPPER::VRC2_022:
				case MAPPER::VRC4_021:
				case MAPPER::VRC2_VRC4_023:
				case MAPPER::VRC2_VRC4_025:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.vrc24), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.vrc24));

					auto& vrc24 = pNES_instance->NES_state.catridgeInfo.vrc24;

					const MAPPER     curMapper = pNES_instance->NES_state.catridgeInfo.mapper;
					const SUB_MAPPER curSubMapper = pNES_instance->NES_state.catridgeInfo.subMapper;

					// CRC32 submapper fallback for iNES 1.0 ROMs not found in DB.
					// If DB found this ROM, subMapper is already set and this block is skipped.
					if (curSubMapper == SUB_MAPPER::SUB_MAPPER_NOT_APPLICABLE)
					{
						FATAL("Unknow Submapper : %d", pNES_instance->NES_state.catridgeInfo.subMapper);
					}

					FLAG isVRC4 = SET;

					switch (curMapper)
					{
					case MAPPER::VRC2_022: isVRC4 = RESET; BREAK;
					case MAPPER::VRC4_021: isVRC4 = SET;   BREAK;

					case MAPPER::VRC2_VRC4_023:
					{
						if (curSubMapper == SUB_MAPPER::SUB_MAPPER_NOT_APPLICABLE)
							isVRC4 = (chr8kBanks <= 32) ? RESET : SET;
						else
							isVRC4 = (curSubMapper != SUB_MAPPER::VRC2B);
						BREAK;
					}
					case MAPPER::VRC2_VRC4_025:
					{
						if (curSubMapper == SUB_MAPPER::SUB_MAPPER_NOT_APPLICABLE)
							isVRC4 = (chr8kBanks <= 32) ? RESET : SET;
						else
							isVRC4 = (curSubMapper != SUB_MAPPER::VRC2C);
						BREAK;
					}
					default: BREAK;
					}

					vrc24.swapMode = RESET;
					vrc24.wramEnable = RESET;

					const uint32_t prg8kBanks = prg16kBanks << 1;
					if (prg8kBanks < TWO && zeroBanksHandled == NO)
					{
						FATAL("Invalid VRC2/VRC4 PRG-ROM");
					}

					vrc24.prgBank0 = ZERO;
					vrc24.prgBank1 = ONE;

					for (uint32_t i = 0; i < EIGHT; ++i)
					{
						vrc24.chrBank[i] = TO_UINT8(i);
					}

					vrc24.latch = RESET;
					vrc24.wramEnable = isPrgRamAvailable;

					BREAK;
				}
				case MAPPER::VRC6_024:
				case MAPPER::VRC6_026:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.vrc6), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.vrc6));

					auto& vrc6 = pNES_instance->NES_state.catridgeInfo.vrc6;

					pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;

					vrc6.prgBank0 = ZERO;
					vrc6.prgBank1 = ZERO;
					vrc6.b003_reg = ZERO;

					for (uint32_t i = 0; i < EIGHT; ++i)
					{
						vrc6.chrBank[i] = TO_UINT8(i);
					}

					vrc6.irqLatch = ZERO;
					vrc6.irqControl = ZERO;
					vrc6.irqCounter = ZERO;
					vrc6.prescaler = ZERO;

					// VRC6 audio: globalHalt=0 means channels are enabled by default
					// freqCounters/accumulators already zeroed by memset above
					vrc6.vrc6Audio.pulse[0].enabled = YES;
					vrc6.vrc6Audio.pulse[1].enabled = YES;
					vrc6.vrc6Audio.sawtooth.enabled = YES;

					// -----------------------------------------------------------------
					// Setup Fixed Base Bank Memory Spaces:
					// $E000-$FFFF is hardwired fixed to the LAST 8KB PRG ROM block
					// -----------------------------------------------------------------
					const uint32_t prg8kBanks = prg16kBanks << ONE;

					// cpuCart base = catridgeMappedMemory[0x3FE0] = CPU $8000
					// $E000-$FFFF offset from $8000 = 0x6000
					// Fixed window = last 8KB PRG ROM bank
					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x6000]), 0x2000, &(prgRom[(prg8kBanks - ONE) * 0x2000]), 0x2000);
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_028:
				{
					if (nesReset == NO)
					{
						memset(&pNES_instance->NES_state.catridgeInfo.ines028, 0, sizeof(pNES_instance->NES_state.catridgeInfo.ines028));

						// Guarantee "$C000-$FFFF = last 16KB bank" without special-casing the
						// read path: PP=3 (Fixed $C000), SS=0, outer=0xFF. The fixed-window
						// formula gives (0xFF<<1)|1 = 0x1FF (max possible 16KB index); the
						// read path's existing `% totalPrg16kBanks` wrap then naturally lands
						// on the true last bank for any power-of-two ROM size.
						pNES_instance->NES_state.catridgeInfo.ines028.reg80_mode = 0x0C;
						pNES_instance->NES_state.catridgeInfo.ines028.reg81_outerBank = 0xFF;
						memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);

						pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR; // arbitrary -- "rest of state is unspecified"
					}

					nesReset = CLEAR;
					BREAK;
				}
				case MAPPER::INES_MAPPER_029:
				{
					memset(&pNES_instance->NES_state.catridgeInfo.ines029, 0, sizeof(pNES_instance->NES_state.catridgeInfo.ines029));
					pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR; // hardwired, no register bit controls this
					pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;
					BREAK;
				}
				case MAPPER::INES_MAPPER_030:
				{
					memset(&pNES_instance->NES_state.catridgeInfo.ines030, 0, sizeof(pNES_instance->NES_state.catridgeInfo.ines030));

					const BYTE submapperRaw = (BYTE)pNES_instance->NES_state.catridgeInfo.subMapper;
					auto& reg030 = pNES_instance->NES_state.catridgeInfo.ines030;

					if (submapperRaw == THREE)
					{
						reg030.ntMode = INES030_NT_MODE::SUBMAPPER3_HV_SWITCHABLE;
						pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR; // N=0 default
					}
					else
					{
						const BIT fourScreenBit = (pINES->iNES_Fields.iNES_header.fields.flag6.raw >> 3) & 0x01;
						const BIT mirrorBit = pINES->iNES_Fields.iNES_header.fields.flag6.raw & 0x01;

						if (fourScreenBit && mirrorBit)
						{
							reg030.ntMode = INES030_NT_MODE::FOUR_SCREEN_CART_VRAM;
						}
						else if (fourScreenBit)
						{
							reg030.ntMode = INES030_NT_MODE::ONESCREEN_SWITCHABLE;
							pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;
						}
						else if (mirrorBit) // bit0=1 -> VERTICAL mirroring (was wrongly HORIZONTAL)
						{
							reg030.ntMode = INES030_NT_MODE::FIXED_VERTICAL;
							pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::VERTICAL_MIRROR;
						}
						else // bit0=0 -> HORIZONTAL mirroring (was wrongly VERTICAL)
						{
							reg030.ntMode = INES030_NT_MODE::FIXED_HORIZONTAL;
							pNES_instance->NES_state.catridgeInfo.nameTblMir = NAMETABLE_MIRROR::HORIZONTAL_MIRROR;
						}
					}
					BREAK;
				}
				case MAPPER::INES_MAPPER_034:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.ines034), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.ines034));

					pNES_instance->NES_state.catridgeInfo.ines034.prgBank32 = ZERO;

					if (pNES_instance->NES_state.catridgeInfo.subMapper == static_cast<SUB_MAPPER>(ZERO))
					{
						if (chrRomSizeBytes <= 0x2000ULL)
							pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::BNROM;
						else
							pNES_instance->NES_state.catridgeInfo.subMapper = SUB_MAPPER::NINA;

						LOG(" Submapper : %u", pNES_instance->NES_state.catridgeInfo.subMapper);
						LOG("==================================================");
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x8000, prgRom, 0x8000);
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::GxROM:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.gxrom), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.gxrom));

					if (prgRomSizeBytes < 0x8000ULL && zeroBanksHandled == NO)
					{
						FATAL("Invalid GxROM PRG-ROM");
					}
					if (chrRomSizeBytes <= ZERO)
					{
						FATAL("GxROM requires CHR-ROM");
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x8000, prgRom, 0x8000);
					}

					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_067:
				case MAPPER::INES_MAPPER_068:
				{
					// Sunsoft-4 boards do not suffer from physical hardware bus conflicts
					pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;

					// Clear the mapper's internal register struct state
					memset(&(pNES_instance->NES_state.catridgeInfo.ines_067_068), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.ines_067_068));

					// Sunsoft-4 expects at least 16KB banks to map correctly
					if (prg16kBanks < ONE && zeroBanksHandled == NO)
					{
						FATAL("Invalid Mapper 067/068 PRG-ROM size");
					}

					// Power-on State: 
					// - $8000-$BFFF maps the first 16KB PRG bank (Bank 0)
					// - $C000-$FFFF is hardwired/fixed to the LAST 16KB PRG bank
					pNES_instance->NES_state.catridgeInfo.ines_067_068.prgBank = ZERO;

					// Map CPU $8000-$BFFF (Bank 0)
					// Map CPU $C000-$FFFF (Fixed Last Bank)
					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
						}
					}

					// Sunsoft-4 uses pure CHR-ROM. Power-on defaults usually map the first 8KB of CHR-ROM 
					// into the PPU pattern tables ($0000-$1FFF) split into 1KB pages or as a contiguous block.
					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}
					else
					{
						FATAL("Mapper 067/068 requires CHR-ROM, but size is 0");
					}

					// Initial NT/Mirroring state defaults to standard hardwired configuration 
					// until overwritten by CPU writes to $E000
					pNES_instance->NES_state.catridgeInfo.ines_067_068.ntControl = ZERO;
					pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank0 = ZERO;
					pNES_instance->NES_state.catridgeInfo.ines_067_068.ntBank1 = ZERO;

					BREAK;
				}
				case MAPPER::INES_MAPPER_069:
				{
					// Sunsoft-FME-7 boards do not suffer from physical hardware bus conflicts
					pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;

					// Clear the mapper's internal register struct state
					memset(&(pNES_instance->NES_state.catridgeInfo.ines069), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.ines069));

					// Power-on State: 
					// - $E000-$FFFF is hardwired/fixed to the LAST 16KB PRG bank

					// Map CPU $E000-$FFFF (Fixed Last Bank)
					const uint32_t prg8kBanks = prg16kBanks * 2;
					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x6000]), 0x2000, &(prgRom[(prg8kBanks - ONE) * 0x2000]), 0x2000);
					}

					// Sunsoft-FME-7 uses pure CHR-ROM. Power-on defaults usually map the first 8KB of CHR-ROM 
					// into the PPU pattern tables ($0000-$1FFF) split into 1KB pages or as a contiguous block.
					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}
					else
					{
						// CHR-RAM powers on as whatever your normal RAM initialization
						// establishes. Mapper 69 bank registers are already zeroed above.
						memset(
							pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0,
							0,
							sizeof(pNES_ppuMemory->NESMemoryMap.patternTable.patternTable0));

						memset(
							pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1,
							0,
							sizeof(pNES_ppuMemory->NESMemoryMap.patternTable.patternTable1));
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_070:
				case MAPPER::INES_MAPPER_078:
				{
					memset(&pNES_instance->NES_state.catridgeInfo.ines078, 0, sizeof(pNES_instance->NES_state.catridgeInfo.ines078));

					pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = YES; // always, regardless of submapper

					const BYTE submapperRaw = (BYTE)pNES_instance->NES_state.catridgeInfo.subMapper;
					pNES_instance->NES_state.catridgeInfo.nameTblMir = (submapperRaw == THREE)
						? NAMETABLE_MIRROR::HORIZONTAL_MIRROR
						: NAMETABLE_MIRROR::ONESCREEN_LO_MIRROR;

					BREAK;
				}
				case MAPPER::INES_MAPPER_152:
				{
					if (isNES2)
					{
						const BYTE sub = header.flags_8to15.nes2p0.flag8.fields.subMapper;
						if (sub == 0 || sub == 2) pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = YES;
						else if (sub == 1)             pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = NO;
					}
					else
					{
						pNES_instance->NES_state.catridgeInfo.isBusConflictPresent = YES;
					}

					memset(&(pNES_instance->NES_state.catridgeInfo.ines_070_152), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.ines_070_152));

					if (prg16kBanks < ONE && zeroBanksHandled == NO)
					{
						FATAL("Invalid 070/152 PRG-ROM");
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);

						if (prg16kBanks == ONE)
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, prgRom, 0x4000);
						}
						else
						{
							memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
						}
					}

					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::NANJING_FC001:
				{
					memset(&(pNES_instance->NES_state.catridgeInfo.nanjing_fc001), 0,
						sizeof(pNES_instance->NES_state.catridgeInfo.nanjing_fc001));

					if (prgRomSizeBytes < 0x8000ULL && zeroBanksHandled == NO)
					{
						FATAL("Invalid NANJING_FC001 PRG-ROM");
					}
					if (chrRomSizeBytes > ZERO)
					{
						FATAL("NANJING_FC001 doesn't support CHR-ROM");
					}

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x8000, prgRom, 0x8000);
					}

					pNES_instance->NES_state.catridgeInfo.nanjing_fc001.A = (FLAG)SET;

					BREAK;
				}
				case MAPPER::INES_MAPPER_019:
				case MAPPER::INES_MAPPER_210:
				{
					auto& n163 = pNES_instance->NES_state.catridgeInfo.namco163;
					memset(&n163, 0, sizeof(n163));
					n163.audioCurrentCh = 7;

					// -----------------------------------------------------------
					// Variant detection
					// Mapper 19 defaults to Namco163 with auto-detect enabled
					// (auto-detect watches register writes to confirm the variant)
					// Mapper 210 uses submapper to select known variant
					// Ref: Mesen Namco163::InitMapper()
					// -----------------------------------------------------------
					const MAPPER     curMapper = pNES_instance->NES_state.catridgeInfo.mapper;
					const SUB_MAPPER curSubMapper = pNES_instance->NES_state.catridgeInfo.subMapper;

					if (curMapper == MAPPER::INES_MAPPER_019)
					{
						n163.variant = 0;   // default Namco163
						n163.autoDetect = YES; // refine from DB board string if available
						// If NES 2.0 sub 2 = audio explicitly disabled
						if (curSubMapper == SUB_MAPPER::NAMCO340)
							n163.audioDisable = YES;
					}
					else // MAPPER::INES_MAPPER_210
					{
						switch (curSubMapper)
						{
						case SUB_MAPPER::NAMCO175:
							n163.variant = 1; n163.autoDetect = NO; BREAK;
						case SUB_MAPPER::NAMCO340:
							n163.variant = 2; n163.autoDetect = NO; BREAK;
						default:
							n163.variant = 3; n163.autoDetect = YES; BREAK;
						}
					}

					// -----------------------------------------------------------
					// PRG init: fixed last bank to $E000-$FFFF
					// Copy to cpuCart for initial NMI vector access
					// -----------------------------------------------------------

					// All three switchable banks start at 0 (first bank)
					n163.prgBanks[0] = 0;
					n163.prgBanks[1] = 0;
					n163.prgBanks[2] = 0;
					// Fixed last bank: $E000-$FFFF (handled dynamically in CPU read)

					if (zeroBanksHandled == NO)
					{
						memcpy_portable(&(cpuCart[0x0000]), 0x4000, prgRom, 0x4000);
						memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[(prg16kBanks - ONE) * 0x4000]), 0x4000);
					}

					// -----------------------------------------------------------
					// CHR init: copy first 8KB if CHR-ROM present
					// -----------------------------------------------------------
					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				case MAPPER::INES_MAPPER_232:
				{
					auto& m232 = pNES_instance->NES_state.catridgeInfo.ines232;
					memset(&m232, 0, sizeof(m232));

					// Power-on / Reset Defaults:
					// Outer bank defaults to block 3 (last 64 KiB block), inner bank defaults to 0
					m232.outerBank = THREE;
					m232.innerBank = ZERO;

					// Calculate initial 16 KiB PRG bank indices
					// $8000-$BFFF: (outerBank * 4) + innerBank
					// $C000-$FFFF: (outerBank * 4) + 3 (fixed to last bank of selected block)
					const uint32_t totalBanks = prg16kBanks > ZERO ? prg16kBanks : ONE;
					m232.prgBank8000 = ((m232.outerBank * 4) + m232.innerBank) % totalBanks;
					m232.prgBankC000 = ((m232.outerBank * 4) + 3) % totalBanks;

					// Map initial PRG banks to CPU cartridge view ($8000-$BFFF and $C000-$FFFF)
					memcpy_portable(&(cpuCart[0x0000]), 0x4000, &(prgRom[m232.prgBank8000 * 0x4000]), 0x4000);
					memcpy_portable(&(cpuCart[0x4000]), 0x4000, &(prgRom[m232.prgBankC000 * 0x4000]), 0x4000);

					// Map initial 8 KiB CHR ROM/RAM pattern table
					if (chrRomSizeBytes > ZERO)
					{
						const uint64_t copySize = (chrRomSizeBytes > 0x2000ULL) ? 0x2000ULL : chrRomSizeBytes;
						memcpy_portable(ppuChr, copySize, chrRom, copySize);
					}

					BREAK;
				}
				default:
				{
					FATAL("Unsupported mapper : %u", pNES_instance->NES_state.catridgeInfo.mapperID);
				}
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
						inSRAM.read(reinterpret_cast<char*>(&ramByte), ONE);
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