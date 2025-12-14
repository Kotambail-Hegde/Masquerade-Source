#pragma region PACMAN_SPECIFIC_INCLUDES
#include "pacMan.h"
#pragma endregion PACMAN_SPECIFIC_INCLUDES

#pragma region PACMAN_SPECIFIC_MACROS
#define FLIPX											true
#define FLIPY											true
#define FLIPBOTH										true

#define PAC_MAN_CODE_ROM_MAX_CHIP_SIZE					0x1000

#define VRAM_START_ADDRESS								0x4000
#define VRAM_END_ADDRESS								0x47FF
#define CHARACTER_VRAM_START_ADDRESS					VRAM_START_ADDRESS
#define CHARACTER_VRAM_END_ADDRESS						0x43FF
#define CHARACTER_CRAM_START_ADDRESS					0x4400
#define CHARACTER_CRAM_END_ADDRESS						VRAM_END_ADDRESS

#define MAX_COLUMN_TILES								28
#define MAX_ROW_TILES									36

#define VOICE1											ZERO
#define VOICE2											ONE
#define VOICE3											TWO
#define TOTAL_VOICES									THREE
#define PACMAN_AUDIO_SCALING							50.0f
#pragma endregion PACMAN_SPECIFIC_MACROS

#pragma region PACMAN_SPECIFIC_DECLARATIONS
// CPU cycles per single pacman frame
static uint32_t const CPU_CYCLES_PER_FRAME = 51200;	// 3.072 MHz
// APU cycles per single pacman frame
static uint32_t const APU_CYCLES_PER_FRAME = 32;
// maximum number of inputs
static uint32_t const MAX_INPUT_LEN = (uint32_t)(PACMAN_AUDIO_SAMPLING_RATE / PACMAN_FPS);
// length of filter than can be handled
static uint32_t const MAX_FLT_LEN = 25;
// buffer to hold all of the input samples
static uint32_t const BUFFER_LEN = (MAX_FLT_LEN - 1 + MAX_INPUT_LEN);
// coefficients for the FIR from https://www.arc.id.au/FilterDesign.html; cutoff 15000 Hz and 90 db attenuation
static double const antiAliasingCoeffs[MAX_FLT_LEN] =
{
	-0.000018,
	-0.000213,
	-0.000302,
	0.001173,
	0.004734,
	0.005232,
	-0.006585,
	-0.029041,
	-0.034750,
	0.015836,
	0.130668,
	0.257006,
	0.312500,
	0.257006,
	0.130668,
	0.015836,
	-0.034750,
	-0.029041,
	-0.006585,
	0.005232,
	0.004734,
	0.001173,
	-0.000302,
	-0.000213,
	-0.000018
};
// double type buffers to hold input and output during FIR
static double doubleInput[(uint32_t)(PACMAN_AUDIO_SAMPLING_RATE / PACMAN_FPS)];
static double doubleOutput[(uint32_t)(PACMAN_AUDIO_SAMPLING_RATE / PACMAN_FPS)];

static std::string _JSON_LOCATION;
static boost::property_tree::ptree testCase;

static uint32_t pacman_texture;
static uint32_t matrix_texture;
static uint32_t matrix[16] = { 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
#pragma endregion PACMAN_SPECIFIC_DECLARATIONS

pacMan_t::pacMan_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config)
{
	isBiosEnabled = NO;
	INC8 indexToCheck = RESET;

	setEmulationID(EMULATION_ID::PACMAN_ID);

	this->pt = config;

	if (nFiles == TEST_ROMS)
	{
		++indexToCheck;
		std::transform(rom[indexToCheck].begin(), rom[indexToCheck].end(), rom[indexToCheck].begin(), ::tolower);

		std::size_t pos = rom[indexToCheck].find_last_of('.');
		if (pos != std::string::npos)
		{
			std::string ext = rom[indexToCheck].substr(pos + ONE);

			if (ext == "com")
			{
				ROM_TYPE = ROM::TEST_ROM_COM;
			}
			else if (ext == "cim")
			{
				ROM_TYPE = ROM::TEST_ROM_CIM;
			}
			else if (ext == "tap")
			{
				ROM_TYPE = ROM::TEST_ROM_TAP;
			}
		}
		else
		{
			ROM_TYPE = ROM::NO_ROM; // no extension at all
		}
	}
	else if (nFiles == SST_ROMS)
	{
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_WARN);
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);
		SETBIT(ENABLE_LOGS, LOG_VERBOSITY_EVENT);

		INFO("Running in sst Cpu Test Mode!");
		_JSON_LOCATION = rom[ONE];

		ROM_TYPE = ROM::TEST_SST;
	}
	else if (nFiles == PAC_MAN_ROMS)
	{
		ROM_TYPE = ROM::PAC_MAN;
	}
	else if (nFiles == MS_PAC_MAN_ROMS)
	{
		ROM_TYPE = ROM::MS_PAC_MAN;
	}

#ifndef __EMSCRIPTEN__
	_SAVE_LOCATION = pt.get<std::string>("pacman._save_location");
#else
	_SAVE_LOCATION = "assets/saves";
#endif
	_TEST_NUMBER = pt.get<std::int32_t>("pacman._test_to_run");

	// check if directory mentioned by "_SAVE_LOCATION" exists, if not we need to explicitly create it
	ifNoDirectoryThenCreate(_SAVE_LOCATION);

	for (INC8 ii = RESET; ii < (MAX_NUMBER_ROMS_PER_PLATFORM - indexToCheck); ii++)
	{
		this->rom[ii] = rom[ii + indexToCheck];
	}
}

void pacMan_t::setupTheCoreOfEmulation(void* masqueradeInstance, void* audio, void* network)
{
	INC8 indexToCheck = RESET;

	if (ROM_TYPE == ROM::PAC_MAN)
	{
		indexToCheck = PAC_MAN_ROMS - ONE;
	}
	else if (ROM_TYPE == ROM::MS_PAC_MAN)
	{
		indexToCheck = MS_PAC_MAN_ROMS - ONE;
	}
	else if (ROM_TYPE == ROM::TEST_ROM_COM || ROM_TYPE == ROM::TEST_ROM_CIM || ROM_TYPE == ROM::TEST_ROM_TAP)
	{
		indexToCheck = RESET;
	}

	if ((!rom[indexToCheck].empty()) || (ROM_TYPE == ROM::TEST_SST))
	{
		if (!initializeEmulator())
		{
			LOG("memory allocation failure");
			throw std::runtime_error("memory allocation failure");
		}

		if (ROM_TYPE != ROM::TEST_SST)
		{
			loadRom(rom);

			// initialize the graphics

			initializeGraphics();

			// initialize the audio

			initializeAudio();
		}
	}
	else
	{
		LOG("un-supported rom");
		throw std::runtime_error("un-supported rom");
	}
}

uint32_t pacMan_t::getScreenWidth()
{
	RETURN this->screen_width;
}

uint32_t pacMan_t::getScreenHeight()
{
	RETURN this->screen_height;
}

uint32_t pacMan_t::getPixelWidth()
{
	RETURN this->pixel_width;
}

uint32_t pacMan_t::getPixelHeight()
{
	RETURN this->pixel_height;
}

uint32_t pacMan_t::getTotalScreenWidth()
{
	RETURN this->screen_width;
}

uint32_t pacMan_t::getTotalScreenHeight()
{
	RETURN this->screen_height;
}

uint32_t pacMan_t::getTotalPixelWidth()
{
	RETURN this->pixel_width;
}

uint32_t pacMan_t::getTotalPixelHeight()
{
	RETURN this->pixel_height;
}

const char* pacMan_t::getEmulatorName()
{
	RETURN this->NAME;
}

void pacMan_t::setEmulationID(EMULATION_ID ID)
{
	myID = ID;
}

EMULATION_ID pacMan_t::getEmulationID()
{
	RETURN myID;
}

float pacMan_t::getEmulationFPS()
{
	RETURN this->myFPS;
}

void pacMan_t::cpuTickT()
{
	pPacMan_cpuInstance->cpuCounter += ONE;
	if (ROM_TYPE == ROM::TEST_SST)
	{
		++pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst;
	}
	else
	{
		syncOtherGBModuleTicks();
	}
}

void pacMan_t::syncOtherGBModuleTicks()
{
	apuTick();
}

void pacMan_t::apuTick()
{
	pPacMan_instance->pacMan_state.audio.apuCounter += ONE;
}

uint32_t pacMan_t::getEmulatedAPUCycle()
{
	RETURN pPacMan_instance->pacMan_state.audio.apuCounter;
}

void pacMan_t::setEmulatedAPUCycle(uint32_t cycles)
{
	pPacMan_instance->pacMan_state.audio.apuCounter = cycles;
}

void pacMan_t::cpuSetRegister(pacMan_t::REGISTER_TYPE rt, uint16_t u16parameter)
{
	switch (rt) {

		// Normal Register access
	case REGISTER_TYPE::RT_A: { pPacMan_registers->af.aAndFRegisters.a = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_F: { pPacMan_registers->af.aAndFRegisters.f.flagMemory = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_B: { pPacMan_registers->bc.bAndCRegisters.b = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_C: { pPacMan_registers->bc.bAndCRegisters.c = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_D: { pPacMan_registers->de.dAndERegisters.d = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_E: { pPacMan_registers->de.dAndERegisters.e = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_H: { pPacMan_registers->hl.hAndLRegisters.h = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_L: { pPacMan_registers->hl.hAndLRegisters.l = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_I: { pPacMan_registers->i = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_R: { pPacMan_registers->r = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_IXH: { pPacMan_registers->ix.ixRegisters.ixh = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_IXL: { pPacMan_registers->ix.ixRegisters.ixl = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_IYH: { pPacMan_registers->iy.iyRegisters.iyh = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_IYL: { pPacMan_registers->iy.iyRegisters.iyl = u16parameter & 0x00FF; BREAK; }

	case REGISTER_TYPE::RT_PC: { pPacMan_registers->pc = u16parameter & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_SP: { pPacMan_registers->sp = u16parameter & 0xFFFF; BREAK; }

	case REGISTER_TYPE::RT_IX: { pPacMan_registers->ix.ix_u16memory = u16parameter & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_IY: { pPacMan_registers->iy.iy_u16memory = u16parameter & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_AF: { pPacMan_registers->af.af_u16memory = u16parameter & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_BC: { pPacMan_registers->bc.bc_u16memory = u16parameter & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_DE: { pPacMan_registers->de.de_u16memory = u16parameter & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_HL: { pPacMan_registers->hl.hl_u16memory = u16parameter & 0xFFFF; BREAK; }

	// Shadow Register access
	case REGISTER_TYPE::RT_SHADOW_A: { pPacMan_registers->shadow_af.aAndFRegisters.a = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_F: { pPacMan_registers->shadow_af.aAndFRegisters.f.flagMemory = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_B: { pPacMan_registers->shadow_bc.bAndCRegisters.b = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_C: { pPacMan_registers->shadow_bc.bAndCRegisters.c = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_D: { pPacMan_registers->shadow_de.dAndERegisters.d = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_E: { pPacMan_registers->shadow_de.dAndERegisters.e = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_H: { pPacMan_registers->shadow_hl.hAndLRegisters.h = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_L: { pPacMan_registers->shadow_hl.hAndLRegisters.l = u16parameter & 0x00FF; BREAK; }

	case REGISTER_TYPE::RT_SHADOW_AF: { pPacMan_registers->shadow_af.af_u16memory = u16parameter & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_BC: { pPacMan_registers->shadow_bc.bc_u16memory = u16parameter & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_DE: { pPacMan_registers->shadow_de.de_u16memory = u16parameter & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_SHADOW_HL: { pPacMan_registers->shadow_hl.hl_u16memory = u16parameter & 0xFFFF; BREAK; }

	case REGISTER_TYPE::RT_WZ: { pPacMan_registers->wz = u16parameter & 0xFFFF; BREAK; }

	case REGISTER_TYPE::RT_P: { pPacMan_registers->p = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_Q: { pPacMan_registers->q = u16parameter & 0x00FF; BREAK; }

	case REGISTER_TYPE::RT_IFF1: { pPacMan_registers->iff1 = u16parameter & 0x00FF; BREAK; }
	case REGISTER_TYPE::RT_IFF2: { pPacMan_registers->iff2 = u16parameter & 0x00FF; BREAK; }

	case REGISTER_TYPE::RT_NONE: { BREAK; }
	default: { BREAK; }
	}
}

uint16_t pacMan_t::cpuReadRegister(pacMan_t::REGISTER_TYPE rt)
{
	switch (rt) {

		// Normal Register access
	case REGISTER_TYPE::RT_A: { RETURN (pPacMan_registers->af.aAndFRegisters.a & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_F: { RETURN (pPacMan_registers->af.aAndFRegisters.f.flagMemory & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_B: { RETURN (pPacMan_registers->bc.bAndCRegisters.b & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_C: { RETURN (pPacMan_registers->bc.bAndCRegisters.c & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_D: { RETURN (pPacMan_registers->de.dAndERegisters.d & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_E: { RETURN (pPacMan_registers->de.dAndERegisters.e & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_H: { RETURN (pPacMan_registers->hl.hAndLRegisters.h & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_L: { RETURN (pPacMan_registers->hl.hAndLRegisters.l & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_I: { RETURN (pPacMan_registers->i & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_R: { RETURN (pPacMan_registers->r & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_IXH: { RETURN (pPacMan_registers->ix.ixRegisters.ixh & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_IXL: { RETURN (pPacMan_registers->ix.ixRegisters.ixl & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_IYH: { RETURN (pPacMan_registers->iy.iyRegisters.iyh & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_IYL: { RETURN (pPacMan_registers->iy.iyRegisters.iyl & 0x00FF); BREAK; }

	case REGISTER_TYPE::RT_PC: { RETURN (pPacMan_registers->pc & 0xFFFF); BREAK; }
	case REGISTER_TYPE::RT_SP: { RETURN (pPacMan_registers->sp & 0xFFFF); BREAK; }

	case REGISTER_TYPE::RT_IX: { RETURN (pPacMan_registers->ix.ix_u16memory) & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_IY: { RETURN (pPacMan_registers->iy.iy_u16memory) & 0xFFFF; BREAK; }
	case REGISTER_TYPE::RT_AF: { RETURN (pPacMan_registers->af.af_u16memory & 0xFFFF); BREAK; }
	case REGISTER_TYPE::RT_BC: { RETURN (pPacMan_registers->bc.bc_u16memory & 0xFFFF); BREAK; }
	case REGISTER_TYPE::RT_DE: { RETURN (pPacMan_registers->de.de_u16memory & 0xFFFF); BREAK; }
	case REGISTER_TYPE::RT_HL: { RETURN (pPacMan_registers->hl.hl_u16memory & 0xFFFF); BREAK; }

	// Shadow Register access
	case REGISTER_TYPE::RT_SHADOW_A: { RETURN (pPacMan_registers->shadow_af.aAndFRegisters.a & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_F: { RETURN (pPacMan_registers->shadow_af.aAndFRegisters.f.flagMemory & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_B: { RETURN (pPacMan_registers->shadow_bc.bAndCRegisters.b & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_C: { RETURN (pPacMan_registers->shadow_bc.bAndCRegisters.c & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_D: { RETURN (pPacMan_registers->shadow_de.dAndERegisters.d & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_E: { RETURN (pPacMan_registers->shadow_de.dAndERegisters.e & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_H: { RETURN (pPacMan_registers->shadow_hl.hAndLRegisters.h & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_L: { RETURN (pPacMan_registers->shadow_hl.hAndLRegisters.l & 0x00FF); BREAK; }

	case REGISTER_TYPE::RT_SHADOW_AF: { RETURN (pPacMan_registers->shadow_af.af_u16memory & 0xFFFF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_BC: { RETURN (pPacMan_registers->shadow_bc.bc_u16memory & 0xFFFF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_DE: { RETURN (pPacMan_registers->shadow_de.de_u16memory & 0xFFFF); BREAK; }
	case REGISTER_TYPE::RT_SHADOW_HL: { RETURN (pPacMan_registers->shadow_hl.hl_u16memory & 0xFFFF); BREAK; }

	case REGISTER_TYPE::RT_WZ: { RETURN (pPacMan_registers->wz & 0xFFFF); BREAK;}

	case REGISTER_TYPE::RT_P: { RETURN(pPacMan_registers->p & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_Q: { RETURN(pPacMan_registers->q & 0x00FF); BREAK; }

	case REGISTER_TYPE::RT_IFF1: { RETURN(pPacMan_registers->iff1 & 0x00FF); BREAK; }
	case REGISTER_TYPE::RT_IFF2: { RETURN(pPacMan_registers->iff2 & 0x00FF); BREAK; }

	case REGISTER_TYPE::RT_NONE: { RETURN ZERO;  BREAK; }
	default: { RETURN ZERO; BREAK; }
	}
}

void pacMan_t::cpuWritePointer(pacMan_t::POINTER_TYPE mrt, uint16_t u16parameter)
{
	switch (mrt) {

		// Memory pointed by Register access
	case POINTER_TYPE::RT_M_HL: { writeRawMemoryFromCPU(pPacMan_registers->hl.hl_u16memory, u16parameter & 0x00FF); BREAK; }
	case POINTER_TYPE::RT_M_DE: { writeRawMemoryFromCPU(pPacMan_registers->de.de_u16memory, u16parameter & 0x00FF); BREAK; }
	case POINTER_TYPE::RT_M_BC: { writeRawMemoryFromCPU(pPacMan_registers->bc.bc_u16memory, u16parameter & 0x00FF); BREAK; }
	case POINTER_TYPE::RT_M_WZ: { writeRawMemoryFromCPU(pPacMan_registers->wz, u16parameter & 0x00FF); BREAK; }

	case POINTER_TYPE::RT_M_NONE: { BREAK; }
	default: { BREAK; }
	}
}

BYTE pacMan_t::cpuReadPointer(pacMan_t::POINTER_TYPE mrt)
{
	switch (mrt) {

		// Memory pointed by Register access
	case POINTER_TYPE::RT_M_HL: { RETURN (readRawMemoryFromCPU(pPacMan_registers->hl.hl_u16memory) & 0x00FF); BREAK; }
	case POINTER_TYPE::RT_M_DE: { RETURN (readRawMemoryFromCPU(pPacMan_registers->de.de_u16memory) & 0x00FF); BREAK; }
	case POINTER_TYPE::RT_M_BC: { RETURN (readRawMemoryFromCPU(pPacMan_registers->bc.bc_u16memory) & 0x00FF); BREAK; }
	case POINTER_TYPE::RT_M_WZ: { RETURN (readRawMemoryFromCPU(pPacMan_registers->wz) & 0x00FF); BREAK; }

	case POINTER_TYPE::RT_M_NONE: { RETURN ZERO;  BREAK; }
	default: { RETURN ZERO; BREAK; }
	}
}

// Should be called only from the following functions:
// 1) processOpcode
// 2) performOperation
// 3) stackPop
// 4) stackPush
// 5) processinterrupts?
BYTE pacMan_t::readRawMemoryFromCPU(uint16_t address, FLAG opcodeFetch)
{
	if (ROM_TYPE == ROM::TEST_SST)
	{
		auto data = pPacMan_memory->pacManRawMemory[address];

		while (pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst != ZERO)
		{
			auto index = pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer;
			pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].address = address;
			pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = YES;
			pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
			pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = data;

			// handle the first 4 cycles (opcode fetch cycles)
			if (opcodeFetch == YES)
			{
				// Index zero represents the first read cycle of opcode fetch
				if ((index & 0x03) == ZERO) // mod 3 index == 0
				{
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
				}
				// Index one represents the second read cycle of opcode fetch
				else if ((index & 0x03) == ONE) // mod 3 index == 1
				{
					// This is to simulate 2 cycles needed for reading an address
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = YES;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
				}
				// Index two represents the first refresh cycle
				else if ((index & 0x03) == TWO) // mod 3 index == 2
				{
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].address = (cpuReadRegister(REGISTER_TYPE::RT_I) << 8) | (cpuReadRegister(REGISTER_TYPE::RT_R) & 0x7F);
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
				}
				// Index three represents the second refresh cycle
				else if ((index & 0x03) == THREE) // mod 3 index == 3
				{
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].address = (cpuReadRegister(REGISTER_TYPE::RT_I) << 8) | (cpuReadRegister(REGISTER_TYPE::RT_R) & 0x7F);
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
				}
			}
			else
			{
				// Any memory read cycle other than the opcode fetch will take 3 cycles, anything more, the difference is an internal cycle

				// Actual Memory Cycle
				if (pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst <= THREE)
				{
					auto rIndex = pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst;

					// The first non-opcode read cycle
					if (rIndex == THREE)
					{
						pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
						pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
						pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
					}
					// The second non-opcode read cycle
					else if (rIndex == TWO)
					{
						pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = YES;
						pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
						pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
					}
					// The third non-opcode read cycle
					else if (rIndex == ONE)
					{
						pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
						pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
					}
				}
				// Internal Cycle
				else
				{
					// Internal cycles are padded with previous address
					auto index = pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].address
						= pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index - ONE].address;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
				}
			}

			++pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer;
			--pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst;
		}

		RETURN data;
	}
	else
	{
		if (ROM_TYPE == ROM::TEST_ROM_COM || ROM_TYPE == ROM::TEST_ROM_CIM || ROM_TYPE == ROM::TEST_ROM_TAP)
		{
			RETURN pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[address];
		}

		/*
		 * Ms. Pac-Man is simply a Pac-Man PCB which an auxiliary daughterboard attached via
		 * a ribbon cable to the original Z80 CPU slot. It is dormant until a 1 is written to
		 * the 0x5002 address, at which point it becomes enabled. The daughterboard handles
		 * intercepting reads/writes to certain addresses so it can patch areas of the original
		 * Pac-Man ROM with jumps to the extra Ms. Pac-Man ROMs on the daughterboard.
		 */

		if (
			ROM_TYPE == ROM::MS_PAC_MAN
			&&
			pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.auxBoardEnable
			)
		{
			if (address < 0x4000)
			{
				// Read the patched ROM
				RETURN pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomMemory[address];
			}
			else if (address >= 0x8800 && address <= 0x8FFF)
			{
				// When accessing 0x8800 to 0x8FFF, we read part of U6's memory address...
				// Based on one of the weird logic of Daughter Board i.e. connected via ribbon cable...
				address += 0x1000;
				RETURN pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[address];
			}
		}

		if (address >= 0x5000 && address <= 0x503F)
		{
			// Since all bytes from 0x5000 to 0x503F read same value
			// we will always return the contents of 0x5000 for any address b/w 0x5000 to 0x503F

			RETURN pIN0Memory->IN0Memory[ZERO];
		}
		else if (address >= 0x5040 && address <= 0x507F)
		{
			// Since all bytes from 0x5040 to 0x507F read same value
			// we will always return the contents of 0x5040 for any address b/w 0x5040 to 0x507F

			RETURN pIN1Memory->IN1Memory[ZERO];
		}
		else if (address >= 0x5080 && address <= 0x50BF)
		{
			// Since all bytes from 0x5080 to 0x50BF read same value
			// we will always return the contents of 0x5080 for any address b/w 0x5080 to 0x50BF

			RETURN pPacMan_memory->pacManMemoryMap.DIPMemory.DIPMemory[ZERO];
		}

		RETURN pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[address];
	}
}

// Should be called only from the following functions:
// 1) processDisplay
// 2) displayCompleteScreen
BYTE pacMan_t::readRawMemoryFromGFX(uint16_t address)
{
	if (ROM_TYPE == ROM::TEST_ROM_COM || ROM_TYPE == ROM::TEST_ROM_CIM || ROM_TYPE == ROM::TEST_ROM_TAP)
	{
		RETURN pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[address];
	}

	if (address >= 0x5080 && address <= 0x50BF)
	{
		// Since all bytes from 0x5080 to 0x50BF read same value
		// we will always return the contents of 0x5080 for any address b/w 0x5080 to 0x50BF

		RETURN pPacMan_memory->pacManMemoryMap.DIPMemory.DIPMemory[ZERO];
	}

	if (
		ROM_TYPE == ROM::MS_PAC_MAN
		&&
		pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.auxBoardEnable
		)
	{
		if (address < 0x4000)
		{
			RETURN pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomMemory[address];
		}
	}

	RETURN pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[address];
}

// Should be called only from the following functions:
// 1) processOpcode
// 2) performOperation
// 3) stackPop
// 4) stackPush
// 5) processinterrupts?
void pacMan_t::writeRawMemoryFromCPU(uint16_t address, BYTE data)
{
	if (ROM_TYPE == ROM::TEST_SST)
	{
		while (pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst != ZERO)
		{
			auto index = pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer;
			pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].address = address;
			pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
			pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = YES;
			pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = data;

			// Any memory write cycle will take 3 cycles, anything more, the difference is an internal cycle

			// Actual Memory Cycle
			if (pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst <= THREE)
			{
				auto wIndex = pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst;

				// The first write cycle
				if (wIndex == THREE)
				{
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
				}
				// The second write cycle
				else if (wIndex == TWO)
				{
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = YES;
				}
				// The third write cycle
				else if (wIndex == ONE)
				{
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
				}
			}
			// Internal Cycle
			else
			{
				// Internal cycles are padded with previous address
				auto index = pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer;
				pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].address
					= pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index - ONE].address;
				pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
				pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
				pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
			}

			++pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer;
			--pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst;
		}

		pPacMan_memory->pacManRawMemory[address] = data;
	}
	else
	{
		if (ROM_TYPE == ROM::TEST_ROM_COM || ROM_TYPE == ROM::TEST_ROM_CIM || ROM_TYPE == ROM::TEST_ROM_TAP)
		{
			pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[address] = data;
			RETURN;
		}

		if (address >= 0x0000 && address <= 0x3FFF)
		{
			LOG("If you tried to drag and drop the roms to masquerade.exe and encountered this issue, please try the following:");
			LOG("1) Open masquerade.exe and then try to drag and drop the roms");
			LOG("2) Use the masquerade's open rom option to load the roms");
			FATAL("Trying to write to a read only memory 0x%X", address);
		}

		if (
			(address >= 0x5008 && address <= 0x503F)
			||
			(address >= 0x5070 && address <= 0x50BF)
			||
			(address >= 0x5100)
			)
		{
			// NOTE: Apparently a bunch of these writes occur during bootup / self-test.
			// Ignore writes to these address range;
			RETURN;
		}

		if (
			ROM_TYPE == ROM::MS_PAC_MAN
			&&
			address == 0x5002
			)
		{
			if ((data & 0x01) == ONE)
			{
				pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.auxBoardEnable = ONE;
			}

			if (pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.auxBoardEnable)
			{
				LOG("Auxillary Board Connected!");
			}
			else
			{
				LOG("Auxillary Board Not Connected!");
			}

			RETURN;
		}

		pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[(address & 0xFFFF)] = data;
	}
}

void pacMan_t::stackPush(BYTE data)
{
	(pPacMan_registers->sp)--;
	writeRawMemoryFromCPU(pPacMan_registers->sp, data);
}

BYTE pacMan_t::stackPop()
{
	BYTE popedData = readRawMemoryFromCPU(pPacMan_registers->sp);
	(pPacMan_registers->sp)++;
	RETURN popedData;
}

void pacMan_t::processSZPFlags
(
	byte value
)
{
	pPacMan_cpuInstance->possFlag = YES;

	if ((value & 0x80) == 0x80)
	{
		pPacMan_flags->FSIGN = ONE;
	}
	else
	{
		pPacMan_flags->FSIGN = ZERO;
	}

	if ((value & 0xFF) == 0x00)
	{
		pPacMan_flags->FZERO = ONE;
	}
	else
	{
		pPacMan_flags->FZERO = ZERO;
	}

	if (parityLUT[value])
	{
		pPacMan_flags->F_OF_PARITY = ONE;
	}
	else
	{
		pPacMan_flags->F_OF_PARITY = ZERO;
	}
}

void pacMan_t::processFlagsForLogicalOperation
(
	byte value,
	FLAG isOperationAND
)
{
	pPacMan_cpuInstance->possFlag = YES;

	pPacMan_flags->FHALFCARRY = ((isOperationAND == true) ? ONE : ZERO);
	pPacMan_flags->FNEGATIVE = ZERO;
	pPacMan_flags->FCARRY = ZERO;

	processSZPFlags(value);
}

void pacMan_t::processFlagsFor8BitAdditionOperation
(
	byte value1,
	byte value2,
	FLAG includeCarryInOperation,
	FLAG affectsCarryFlag
)
{
	pPacMan_cpuInstance->possFlag = YES;

	auto unsignedResult1 = 0;
	auto unsignedResult2 = 0;
	auto signedResult1 = 0;

	// 8 bit unsigned addition
	if (includeCarryInOperation == true)
	{
		unsignedResult1 =
			(uint8_t)value1
			+ (uint8_t)value2
			+ (uint8_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);
	}
	else
	{
		unsignedResult1 =
			(uint8_t)value1
			+ (uint8_t)value2;
	}

	// 4 bit signed addition
	if (includeCarryInOperation == true)
	{
		unsignedResult2 =
			((uint8_t)value1 & 0x0F)
			+ ((uint8_t)value2 & 0x0F)
			+ (uint8_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);

	}
	else
	{
		unsignedResult2 =
			((uint8_t)value1 & 0x0F)
			+ ((uint8_t)value2 & 0x0F);
	}

	// 8 bit signed addition
	if (includeCarryInOperation == true)
	{
		signedResult1 =
			(int8_t)value1
			+ (int8_t)value2
			+ (int8_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);
	}
	else
	{
		signedResult1 =
			(int8_t)value1
			+ (int8_t)value2;
	}

	// ARITHMETIC OPERATION TYPE Flag

	pPacMan_flags->FNEGATIVE = ZERO;

	// SIGN flag

	if ((unsignedResult1 & 0x80) == 0x80)
	{
		pPacMan_flags->FSIGN = ONE;
	}
	else
	{
		pPacMan_flags->FSIGN = ZERO;
	}

	// ZERO flag

	if ((unsignedResult1 & 0xFF) == ZERO)
	{
		pPacMan_flags->FZERO = ONE;
	}
	else
	{
		pPacMan_flags->FZERO = ZERO;
	}

	// CARRY Flag

	if (affectsCarryFlag == true)
	{
		if (unsignedResult1 > 255)
		{
			pPacMan_flags->FCARRY = ONE;
		}
		else
		{
			pPacMan_flags->FCARRY = ZERO;
		}
	}

	// HALF CARRY Flag

	if (unsignedResult2 & 0x10)
	{
		pPacMan_flags->FHALFCARRY = ONE;
	}
	else
	{
		pPacMan_flags->FHALFCARRY = ZERO;
	}

	// OVERFLOW Flag

	if (signedResult1 > 127 || signedResult1 < -128)
	{
		pPacMan_flags->F_OF_PARITY = ONE;
	}
	else
	{
		pPacMan_flags->F_OF_PARITY = ZERO;
	}
}

void pacMan_t::processFlagsFor16BitAdditionOperation
(
	uint16_t value1,
	uint16_t value2,
	FLAG includeCarryInOperation,
	FLAG setSZPoF
)
{
	pPacMan_cpuInstance->possFlag = YES;

	auto unsignedResult1 = 0;
	auto unsignedResult2 = 0;
	auto signedResult1 = 0;

	// 16 bit unsigned addition
	if (includeCarryInOperation == true)
	{
		unsignedResult1 =
			(uint16_t)value1
			+ (uint16_t)value2
			+ (uint16_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);
	}
	else
	{
		unsignedResult1 =
			(uint16_t)value1
			+ (uint16_t)value2;
	}

	// 12 bit signed addition
	if (includeCarryInOperation == true)
	{
		unsignedResult2 =
			((uint16_t)value1 & 0x0FFF)
			+ ((uint16_t)value2 & 0x0FFF)
			+ (uint16_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);

	}
	else
	{
		unsignedResult2 =
			((uint16_t)value1 & 0x0FFF)
			+ ((uint16_t)value2 & 0x0FFF);
	}

	// 16 bit signed addition
	if (includeCarryInOperation == true)
	{
		// 2's compliment to convert uint16_t to signed based on the sign bit
		auto signedValue1 = ((value1 & 0x8000) == 0x8000) ? (value1 - 65536) : value1;
		auto signedValue2 = ((value2 & 0x8000) == 0x8000) ? (value2 - 65536) : value2;

		signedResult1 =
			signedValue1
			+ signedValue2
			+ (uint16_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);
	}
	else
	{
		// 2's compliment to convert uint16_t to signed based on the sign bit
		int32_t signedValue1 = ((value1 & 0x8000) == 0x8000) ? (value1 - 65536) : value1;
		int32_t signedValue2 = ((value2 & 0x8000) == 0x8000) ? (value2 - 65536) : value2;

		signedResult1 =
			signedValue1
			+ signedValue2;
	}

	// ARITHMETIC OPERATION TYPE Flag

	pPacMan_flags->FNEGATIVE = ZERO;

	// SIGN flag

	if (setSZPoF == true)
	{
		if ((unsignedResult1 & 0x8000) == 0x8000)
		{
			pPacMan_flags->FSIGN = ONE;
		}
		else
		{
			pPacMan_flags->FSIGN = ZERO;
		}

		// ZERO flag

		if ((unsignedResult1 & 0xFFFF) == ZERO)
		{
			pPacMan_flags->FZERO = ONE;
		}
		else
		{
			pPacMan_flags->FZERO = ZERO;
		}
	}

	// CARRY Flag

	if (unsignedResult1 > 65535)
	{
		pPacMan_flags->FCARRY = ONE;
	}
	else
	{
		pPacMan_flags->FCARRY = ZERO;
	}

	// HALF CARRY Flag

	if (unsignedResult2 & 0x1000)
	{
		pPacMan_flags->FHALFCARRY = ONE;
	}
	else
	{
		pPacMan_flags->FHALFCARRY = ZERO;
	}

	// OVERFLOW Flag

	if (setSZPoF == true)
	{
		if (signedResult1 > 32767 || signedResult1 < -32768)
		{
			pPacMan_flags->F_OF_PARITY = ONE;
		}
		else
		{
			pPacMan_flags->F_OF_PARITY = ZERO;
		}
	}
}

void pacMan_t::processFlagsFor8BitSubtractionOperation
(
	byte value1,
	byte value2,
	FLAG includeCarryInOperation,
	FLAG affectsCarryFlag 
)
{
	pPacMan_cpuInstance->possFlag = YES;

	auto result1 = 0;
	auto result2 = 0;
	auto signedResult1 = 0;

	// 8 bit subtraction
	if (includeCarryInOperation == true)
	{
		result1 =
			value1
			- value2
			- (uint8_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);
	}
	else
	{
		result1 =
			value1
			- value2;
	}

	// 4 bit subraction
	if (includeCarryInOperation == true)
	{
		result2 =
			(value1 & 0x0F)
			- (value2 & 0x0F)
			- (uint8_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);

	}
	else
	{
		result2 =
			(value1 & 0x0F)
			- (value2 & 0x0F);
	}

	// 8 bit signed subraction
	if (includeCarryInOperation == true)
	{
		signedResult1 =
			(int8_t)value1
			- (int8_t)value2
			- (int8_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);
	}
	else
	{
		signedResult1 =
			(int8_t)value1
			- (int8_t)value2;
	}

	// ARITHMETIC OPERATION TYPE Flag

	pPacMan_flags->FNEGATIVE = ONE;

	// SIGN flag

	if ((result1 & 0x80) == 0x80)
	{
		pPacMan_flags->FSIGN = ONE;
	}
	else
	{
		pPacMan_flags->FSIGN = ZERO;
	}

	// ZERO flag

	if ((result1 & 0xFF) == ZERO)
	{
		pPacMan_flags->FZERO = ONE;
	}
	else
	{
		pPacMan_flags->FZERO = ZERO;
	}

	// CARRY Flag

	if (affectsCarryFlag == true)
	{
		if ((includeCarryInOperation == true) && pPacMan_flags->FCARRY)
		{
			if (value2 >= value1)
			{
				pPacMan_flags->FCARRY = ONE;
			}
			else
			{
				pPacMan_flags->FCARRY = ZERO;
			}
		}
		else
		{
			if (value2 > value1)
			{
				pPacMan_flags->FCARRY = ONE;
			}
			else
			{
				pPacMan_flags->FCARRY = ZERO;
			}
		}
	}

	// HALF CARRY Flag

	if (result2 & 0x10)
	{
		pPacMan_flags->FHALFCARRY = ONE;
	}
	else
	{
		pPacMan_flags->FHALFCARRY = ZERO;
	}

	// OVERFLOW Flag

	if (signedResult1 > 127 || signedResult1 < -128)
	{
		pPacMan_flags->F_OF_PARITY = ONE;
	}
	else
	{
		pPacMan_flags->F_OF_PARITY = ZERO;
	}
}

void pacMan_t::processFlagsFor16BitSubtractionOperation
(
	uint16_t value1,
	uint16_t value2,
	FLAG includeCarryInOperation,
	FLAG affectsCarryFlag
)
{
	pPacMan_cpuInstance->possFlag = YES;

	auto result1 = 0;
	auto result2 = 0;
	auto signedResult1 = 0;

	// 16 bit subtraction
	if (includeCarryInOperation == true)
	{
		result1 =
			value1
			- value2
			- (uint16_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);
	}
	else
	{
		result1 =
			value1
			- value2;
	}

	// 12 bit subraction
	if (includeCarryInOperation == true)
	{
		result2 =
			(value1 & 0x0FFF)
			- (value2 & 0x0FFF)
			- (uint16_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);

	}
	else
	{
		result2 =
			(value1 & 0x0FFF)
			- (value2 & 0x0FFF);
	}

	// 16 bit signed subraction
	if (includeCarryInOperation == true)
	{
		// 2's compliment to convert uint16_t to signed based on the sign bit
		auto signedValue1 = ((value1 & 0x8000) == 0x8000) ? (value1 - 65536) : value1;
		auto signedValue2 = ((value2 & 0x8000) == 0x8000) ? (value2 - 65536) : value2;

		signedResult1 =
			signedValue1
			- signedValue2
			- (uint16_t)((pPacMan_flags->FCARRY == 0) ? ZERO : ONE);
	}
	else
	{
		// 2's compliment to convert uint16_t to signed based on the sign bit
		auto signedValue1 = ((value1 & 0x8000) == 0x8000) ? (value1 - 65536) : value1;
		auto signedValue2 = ((value2 & 0x8000) == 0x8000) ? (value2 - 65536) : value2;

		signedResult1 =
			signedValue1
			- signedValue2;
	}

	// ARITHMETIC OPERATION TYPE Flag

	pPacMan_flags->FNEGATIVE = ONE;

	// SIGN flag

	if ((result1 & 0x8000) == 0x8000)
	{
		pPacMan_flags->FSIGN = ONE;
	}
	else
	{
		pPacMan_flags->FSIGN = ZERO;
	}

	// ZERO flag

	if ((result1 & 0xFFFF) == ZERO)
	{
		pPacMan_flags->FZERO = ONE;
	}
	else
	{
		pPacMan_flags->FZERO = ZERO;
	}

	// CARRY Flag

	if (affectsCarryFlag == true)
	{
		if ((includeCarryInOperation == true) && pPacMan_flags->FCARRY)
		{
			if (value2 >= value1)
			{
				pPacMan_flags->FCARRY = ONE;
			}
			else
			{
				pPacMan_flags->FCARRY = ZERO;
			}
		}
		else
		{
			if (value2 > value1)
			{
				pPacMan_flags->FCARRY = ONE;
			}
			else
			{
				pPacMan_flags->FCARRY = ZERO;
			}
		}
	}

	// HALF CARRY Flag

	if (result2 & 0x1000)
	{
		pPacMan_flags->FHALFCARRY = ONE;
	}
	else
	{
		pPacMan_flags->FHALFCARRY = ZERO;
	}

	// OVERFLOW Flag

	if (signedResult1 > 32767 || signedResult1 < -32768)
	{
		pPacMan_flags->F_OF_PARITY = ONE;
	}
	else
	{
		pPacMan_flags->F_OF_PARITY = ZERO;
	}
}

// Wrapper on top of everything thing that the CPU Core does
FLAG pacMan_t::processCPU()
{
	FLAG vblank = NO;

	if (pPacMan_cpuInstance->cpuCounter < CPU_CYCLES_PER_FRAME)
	{
		processOpcode();
	}
	else
	{
		pPacMan_cpuInstance->cpuCounter -= CPU_CYCLES_PER_FRAME;

		if (processInterrupts() == NO)
		{
			processOpcode();
		}

		vblank = YES;
	}

	RETURN vblank;
}

FLAG pacMan_t::processOpcode()
{
	FLAG status = true;

	++pAbsolute_pacMan_instance->absolute_pacMan_state.pacMan_instance.pacMan_state.others.hostCPUCycle;

	if (pPacMan_instance->pacMan_state.HaltEnabled == true)
	{
		// NOP cycles
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		RETURN status;
	}

	performOperation();

	if (debugConfig._DEBUG_REGISTERS == true)
	{
		LOG_NEW_LINE;
		LOG("--------------------------------------------");
		LOG("instruction cycle: \t%" PRId64, pPacMan_cpuInstance->cpuCounter);
		LOG("executed opcode: \t%02x", pPacMan_cpuInstance->opcode);
		LOG("to be executed opcode: \t%02x", readRawMemoryFromCPU(pPacMan_registers->pc));
		LOG("a register: \t\t%02x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.a);

		LOG("f register: \t\t%02x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagMemory);
		LOG("FSIGN: \t\t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FSIGN);
		LOG("FZERO: \t\t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FZERO);
		LOG("FIFTH: \t\t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FIFTH);
		LOG("FHALFCARRY: \t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FHALFCARRY);
		LOG("THIRD: \t\t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.THIRD);
		LOG("F_OF_PARITY: \t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.F_OF_PARITY);
		LOG("FNEGATIVE: \t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FNEGATIVE);
		LOG("FCARRY: \t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FCARRY);

		LOG("b register: \t\t%02x", pPacMan_instance->pacMan_state.registers.bc.bAndCRegisters.b);
		LOG("c register: \t\t%02x", pPacMan_instance->pacMan_state.registers.bc.bAndCRegisters.c);
		LOG("d register: \t\t%02x", pPacMan_instance->pacMan_state.registers.de.dAndERegisters.d);
		LOG("e register: \t\t%02x", pPacMan_instance->pacMan_state.registers.de.dAndERegisters.e);
		LOG("h register: \t\t%02x", pPacMan_instance->pacMan_state.registers.hl.hAndLRegisters.h);
		LOG("l register: \t\t%02x", pPacMan_instance->pacMan_state.registers.hl.hAndLRegisters.l);

		LOG("(BC): \t\t\t%02x", readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_BC)));
		LOG("(DE): \t\t\t%02x", readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_DE)));
		LOG("(HL): \t\t\t%02x", readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL)));
		LOG("(SP): \t\t\t%02x", readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_SP)));

		LOG("i register: \t\t%02x", pPacMan_instance->pacMan_state.registers.i);
		LOG("r register: \t\t%02x", pPacMan_instance->pacMan_state.registers.r);
		LOG("ixh register: \t\t%02x", pPacMan_instance->pacMan_state.registers.ix.ixRegisters.ixh);
		LOG("ixl register: \t\t%02x", pPacMan_instance->pacMan_state.registers.ix.ixRegisters.ixl);
		LOG("iyh register: \t\t%02x", pPacMan_instance->pacMan_state.registers.iy.iyRegisters.iyh);
		LOG("iyl register: \t\t%02x", pPacMan_instance->pacMan_state.registers.iy.iyRegisters.iyl);

		LOG("program counter: \t%04x", pPacMan_instance->pacMan_state.registers.pc);
		LOG("stack pointer: \t\t%04x", pPacMan_instance->pacMan_state.registers.sp);
		LOG("--------------------------------------------");
		LOG("Shadow Registers");
		LOG("shadow a register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.a);

		LOG("shadow f register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagMemory);
		LOG("shadow FSIGN: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FSIGN);
		LOG("shadow FZERO: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FZERO);
		LOG("shadow FIFTH: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FIFTH);
		LOG("shadow FHALFCARRY: \t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FHALFCARRY);
		LOG("shadow THIRD: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.THIRD);
		LOG("shadow F_OF_PARITY: \t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.F_OF_PARITY);
		LOG("shadow NEGATIVE: \t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FNEGATIVE);
		LOG("shadow FCARRY: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FCARRY);

		LOG("shadow b register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_bc.bAndCRegisters.b);
		LOG("shadow c register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_bc.bAndCRegisters.c);
		LOG("shadow d register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_de.dAndERegisters.d);
		LOG("shadow e register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_de.dAndERegisters.e);
		LOG("shadow h register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_hl.hAndLRegisters.h);
		LOG("shadow l register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_hl.hAndLRegisters.l);
		LOG("--------------------------------------------");
	}
	if (debugConfig._DEBUG_KEYPAD == true)
	{
		LOG("Not Supported");
	}

	RETURN status;
}

void pacMan_t::unimplementedInstruction()
{
	LOG("CPU Panic; unknown opcode! %02X", pPacMan_cpuInstance->opcode);
	LOG_NEW_LINE;
	LOG("--------------------------------------------");
	LOG("emulated cpu cycle: \t%" PRId64, pPacMan_cpuInstance->cpuCounter);
	LOG("to be executed opcode: \t%02x", pPacMan_cpuInstance->opcode);
	LOG("a register: \t\t%02x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.a);

	LOG("f register: \t\t%02x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagMemory);
	LOG("FSIGN: \t\t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FSIGN);
	LOG("FZERO: \t\t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FZERO);
	LOG("FIFTH: \t\t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FIFTH);
	LOG("FHALFCARRY: \t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FHALFCARRY);
	LOG("THIRD: \t\t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.THIRD);
	LOG("F_OF_PARITY: \t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.F_OF_PARITY);
	LOG("FNEGATIVE: \t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FNEGATIVE);
	LOG("FCARRY: \t\t%01x", pPacMan_instance->pacMan_state.registers.af.aAndFRegisters.f.flagFields.FCARRY);

	LOG("b register: \t\t%02x", pPacMan_instance->pacMan_state.registers.bc.bAndCRegisters.b);
	LOG("c register: \t\t%02x", pPacMan_instance->pacMan_state.registers.bc.bAndCRegisters.c);
	LOG("d register: \t\t%02x", pPacMan_instance->pacMan_state.registers.de.dAndERegisters.d);
	LOG("e register: \t\t%02x", pPacMan_instance->pacMan_state.registers.de.dAndERegisters.e);
	LOG("h register: \t\t%02x", pPacMan_instance->pacMan_state.registers.hl.hAndLRegisters.h);
	LOG("l register: \t\t%02x", pPacMan_instance->pacMan_state.registers.hl.hAndLRegisters.l);

	LOG("(BC): \t\t\t%02x", readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_BC)));
	LOG("(DE): \t\t\t%02x", readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_DE)));
	LOG("(HL): \t\t\t%02x", readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL)));
	LOG("(SP): \t\t\t%02x", readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_SP)));

	LOG("i register: \t\t%02x", pPacMan_instance->pacMan_state.registers.i);
	LOG("r register: \t\t%02x", pPacMan_instance->pacMan_state.registers.r);
	LOG("ixh register: \t\t%02x", pPacMan_instance->pacMan_state.registers.ix.ixRegisters.ixh);
	LOG("ixl register: \t\t%02x", pPacMan_instance->pacMan_state.registers.ix.ixRegisters.ixl);
	LOG("iyh register: \t\t%02x", pPacMan_instance->pacMan_state.registers.iy.iyRegisters.iyh);
	LOG("iyl register: \t\t%02x", pPacMan_instance->pacMan_state.registers.iy.iyRegisters.iyl);

	LOG("program counter: \t%04x", pPacMan_instance->pacMan_state.registers.pc);
	LOG("stack pointer: \t\t%04x", pPacMan_instance->pacMan_state.registers.sp);
	LOG("--------------------------------------------");
	LOG("Shadow Registers");
	LOG("shadow a register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.a);

	LOG("shadow f register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagMemory);
	LOG("shadow FSIGN: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FSIGN);
	LOG("shadow FZERO: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FZERO);
	LOG("shadow FIFTH: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FIFTH);
	LOG("shadow FHALFCARRY: \t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FHALFCARRY);
	LOG("shadow THIRD: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.THIRD);
	LOG("shadow F_OF_PARITY: \t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.F_OF_PARITY);
	LOG("shadow NEGATIVE: \t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FNEGATIVE);
	LOG("shadow FCARRY: \t\t%01x", pPacMan_instance->pacMan_state.registers.shadow_af.aAndFRegisters.f.flagFields.FCARRY);

	LOG("shadow b register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_bc.bAndCRegisters.b);
	LOG("shadow c register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_bc.bAndCRegisters.c);
	LOG("shadow d register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_de.dAndERegisters.d);
	LOG("shadow e register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_de.dAndERegisters.e);
	LOG("shadow h register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_hl.hAndLRegisters.h);
	LOG("shadow l register: \t%02x", pPacMan_instance->pacMan_state.registers.shadow_hl.hAndLRegisters.l);
	LOG("--------------------------------------------");
	FATAL("Unimplemented or unknown instruction");
}

FLAG pacMan_t::processInterrupts()
{
	/*
	*
	*	Refer for Interrupt cycles in Z80
	*
	*	NMI
	*	It takes 11 clock cycles to get to #0066:
	*	M1 cycle: 5 T states to do an opcode read and decrement SP
	*	M2 cycle: 3 T states write high byte of PC to the stack and decrement SP
	*	M3 cycle: 3 T states write the low byte of PC and jump to #0066.
	*
	*	INT and interrupt mode 0 set
	*	In this mode, timing depends on the instruction put on the bus. The interrupt processing last 2 clock cycles more than this instruction usually needs.
	*	Two typical examples follow:
	*	a RST n on the data bus, it takes 13 cycles to get to 'n':
	*	M1 cycle: 7 ticks
	*	acknowledge interrupt and decrement SP
	*	M2 cycle: 3 ticks
	*	write high byte and decrement SP
	*	M3 cycle: 3 ticks
	*	write low byte and jump to 'n'
	*	With a CALL nnnn on the data bus, it takes 19 cycles:
	*	M1 cycle: 7 ticks
	*	acknowledge interrupt
	*	M2 cycle: 3 ticks
	*	read low byte of 'nnnn' from data bus
	*	M3 cycle: 3 ticks
	*	read high byte of 'nnnn' and decrement SP
	*	M4 cycle: 3 ticks
	*	write high byte of PC to the stack and decrement SP
	*	M5 cycle: 3 ticks
	*	write low byte of PC and jump to 'nnnn'.
	*
	*
	*
	*	INT and interrupt mode 1 set
	*	It takes 13 clock cycles to reach #0038:
	*	M1 cycle: 7 ticks
	*	acknowledge interrupt and decrement SP
	*	M2 cycle: 3 ticks
	*	write high byte of PC onto the stack and decrement SP
	*	M3 cycle: 3 ticks
	*	write low byte onto the stack and to set PC to #0038.
	*
	*	INT and interrupt mode 2 set
	*	It takes 19 clock cycles to get to the interrupt routine:
	*	M1 cycle: 7 ticks
	*	acknowledge interrupt and decrement SP
	*	M2 cycle: 3 ticks
	*	write high byte of PC onto stack and decrement SP
	*	M3 cycle: 3 ticks
	*	write low byte onto the stack
	*	M4 cycle: 3 ticks
	*	read low byte from the interrupt vector
	*	M5 cycle: 3 ticks
	*	read high byte from bus and jump to interrupt routine
	*
	*/

	if ((pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.interruptEnable & 0x01) == 0x01)
	{
		// disable the interrupt
		pPacMan_registers->iff1 = RESET;
		pPacMan_registers->iff2 = RESET;
		pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.interruptEnable &= 0xFE;

		// clear the Halt Enabled Flag

		pPacMan_instance->pacMan_state.HaltEnabled = false;

		if (pPacMan_instance->pacMan_state.interruptMode == INTERRUPT_MODE::INTERRUPT_MODE_0)
		{
			// Only 2 cycles here, remaining will be performed within performOperation itself..
			cpuTickT();
			cpuTickT();

			performOperation((pPacMan_instance->pacMan_state.port0Data & 0xFF));
		}
		else if (pPacMan_instance->pacMan_state.interruptMode == INTERRUPT_MODE::INTERRUPT_MODE_1)
		{
			RUN_FOR_(SEVEN, cpuTickT());

			BYTE higherData = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE lowerData = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(higherData);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(lowerData);

			pPacMan_registers->pc = 0x0038;
			performOperation();
		}
		else if (pPacMan_instance->pacMan_state.interruptMode == INTERRUPT_MODE::INTERRUPT_MODE_2)
		{
			RUN_FOR_(SEVEN, cpuTickT());

			BYTE higherData = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE lowerData = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(higherData);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(lowerData);

			uint16_t iRegisterContent = (uint16_t)pPacMan_registers->i;
			iRegisterContent <<= EIGHT; // * 256
			uint16_t pointerTo8bitAddress = iRegisterContent + pPacMan_instance->pacMan_state.port0Data;

			cpuTickT(); cpuTickT(); cpuTickT();
			pPacMan_registers->pc = (readRawMemoryFromCPU(pointerTo8bitAddress));
			cpuTickT(); cpuTickT(); cpuTickT();
			pPacMan_registers->pc |= (readRawMemoryFromCPU(pointerTo8bitAddress + 1) << 8);

			performOperation();
		}
		else
		{
			FATAL("Unknown Interrupt Mode!");
		}
	}

	RETURN true;
}

void pacMan_t::processTimers()
{
	;
}

void pacMan_t::captureIO()
{
	pIN1Memory->IN1Fields[ZERO].START1 = (ImGui::IsKeyDown(ImGuiKey_Keypad1) == true ? 0 : 1);
	pIN1Memory->IN1Fields[ZERO].START2 = (ImGui::IsKeyDown(ImGuiKey_Keypad2) == true ? 0 : 1);
	pIN1Memory->IN1Fields[ZERO].UP2 = (ImGui::IsKeyDown(ImGuiKey_W) == true ? 0 : 1);
	pIN1Memory->IN1Fields[ZERO].LEFT2 = (ImGui::IsKeyDown(ImGuiKey_A) == true ? 0 : 1);
	pIN1Memory->IN1Fields[ZERO].RIGHT2 = (ImGui::IsKeyDown(ImGuiKey_D) == true ? 0 : 1);
	pIN1Memory->IN1Fields[ZERO].DOWN2 = (ImGui::IsKeyDown(ImGuiKey_S) == true ? 0 : 1);

	pIN0Memory->IN0Fields[ZERO].COIN1 = (ImGui::IsKeyDown(ImGuiKey_Keypad5) == true ? 1 : 0);
	pIN0Memory->IN0Fields[ZERO].UP1 = (ImGui::IsKeyDown(ImGuiKey_UpArrow) == true ? 0 : 1);
	pIN0Memory->IN0Fields[ZERO].LEFT1 = (ImGui::IsKeyDown(ImGuiKey_LeftArrow) == true ? 0 : 1);
	pIN0Memory->IN0Fields[ZERO].RIGHT1 = (ImGui::IsKeyDown(ImGuiKey_RightArrow) == true ? 0 : 1);
	pIN0Memory->IN0Fields[ZERO].DOWN1 = (ImGui::IsKeyDown(ImGuiKey_DownArrow) == true ? 0 : 1);
	pIN0Memory->IN0Fields[ZERO].COIN2 = (ImGui::IsKeyDown(ImGuiKey_Keypad6) == true ? 1 : 0);

	pIN0Memory->IN0Fields[ZERO].CREDIT = (ImGui::IsKeyDown(ImGuiKey_Keypad3) == true ? 0 : 1);
	pIN0Memory->IN0Fields[ZERO].RACK_ADV = (ImGui::IsKeyDown(ImGuiKey_Keypad0) == true ? 0 : 1);
}

void pacMan_t::processAudio()
{
	if (getEmulatedAPUCycle() >= APU_CYCLES_PER_FRAME)
	{
		setEmulatedAPUCycle(getEmulatedAPUCycle() - APU_CYCLES_PER_FRAME);

		PACMAN_AUDIO_SAMPLE_TYPE voiceMix = MUTE_AUDIO;

		if (pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.soundEnable == ONE)
		{
			int32_t voice1Frequency = ZERO;
			int32_t voice2Frequency = ZERO;
			int32_t voice3Frequency = ZERO;
			PACMAN_AUDIO_SAMPLE_TYPE voice1Sample = ZERO;
			PACMAN_AUDIO_SAMPLE_TYPE voice2Sample = ZERO;
			PACMAN_AUDIO_SAMPLE_TYPE voice3Sample = ZERO;
			int32_t toneItterator = ZERO;

			// 1) Get the 20 bit (voice 1) and 16 bit frequencies (voice 2 and voice 3)

			voice1Frequency =
				(
					(pAudioRegisters->V1_frequency[0] & 0x0F)
					| ((pAudioRegisters->V1_frequency[1] & 0x0F) << 4)
					| ((pAudioRegisters->V1_frequency[2] & 0x0F) << 8)
					| ((pAudioRegisters->V1_frequency[3] & 0x0F) << 12)
					| ((pAudioRegisters->V1_frequency[4] & 0x0F) << 16)
					);

			voice2Frequency =
				(
					(pAudioRegisters->V2_frequency[0] & 0x0F)
					| ((pAudioRegisters->V2_frequency[1] & 0x0F) << 4)
					| ((pAudioRegisters->V2_frequency[2] & 0x0F) << 8)
					| ((pAudioRegisters->V2_frequency[3] & 0x0F) << 12)
					);

			voice3Frequency =
				(
					(pAudioRegisters->V3_frequency[0] & 0x0F)
					| ((pAudioRegisters->V3_frequency[1] & 0x0F) << 4)
					| ((pAudioRegisters->V3_frequency[2] & 0x0F) << 8)
					| ((pAudioRegisters->V3_frequency[3] & 0x0F) << 12)
					);

			// 2) Add the accumulator to the frequency components

			pPacMan_instance->pacMan_state.audio.voice1Accumulator =
				((pPacMan_instance->pacMan_state.audio.voice1Accumulator + voice1Frequency) & 0xFFFFF);
			pPacMan_instance->pacMan_state.audio.voice2Accumulator =
				((pPacMan_instance->pacMan_state.audio.voice2Accumulator + voice2Frequency) & 0xFFFFF);
			pPacMan_instance->pacMan_state.audio.voice3Accumulator =
				((pPacMan_instance->pacMan_state.audio.voice3Accumulator + voice3Frequency) & 0xFFFFF);

			// 3) Update the new accumulator in memory

			pAudioRegisters->SV1_accumulator[0] = ((pPacMan_instance->pacMan_state.audio.voice1Accumulator & 0x0000F) >> ZERO);
			pAudioRegisters->SV1_accumulator[1] = ((pPacMan_instance->pacMan_state.audio.voice1Accumulator & 0x000F0) >> FOUR);
			pAudioRegisters->SV1_accumulator[2] = ((pPacMan_instance->pacMan_state.audio.voice1Accumulator & 0x00F00) >> EIGHT);
			pAudioRegisters->SV1_accumulator[3] = ((pPacMan_instance->pacMan_state.audio.voice1Accumulator & 0x0F000) >> TWELVE);
			pAudioRegisters->SV1_accumulator[4] = ((pPacMan_instance->pacMan_state.audio.voice1Accumulator & 0xF0000) >> SIXTEEN);

			pAudioRegisters->SV2_accumulator[0] = ((pPacMan_instance->pacMan_state.audio.voice2Accumulator & 0x000F) >> ZERO);
			pAudioRegisters->SV2_accumulator[1] = ((pPacMan_instance->pacMan_state.audio.voice2Accumulator & 0x00F0) >> FOUR);
			pAudioRegisters->SV2_accumulator[2] = ((pPacMan_instance->pacMan_state.audio.voice2Accumulator & 0x0F00) >> EIGHT);
			pAudioRegisters->SV2_accumulator[3] = ((pPacMan_instance->pacMan_state.audio.voice2Accumulator & 0xF000) >> TWELVE);

			pAudioRegisters->SV3_accumulator[0] = ((pPacMan_instance->pacMan_state.audio.voice3Accumulator & 0x000F) >> ZERO);
			pAudioRegisters->SV3_accumulator[1] = ((pPacMan_instance->pacMan_state.audio.voice3Accumulator & 0x00F0) >> FOUR);
			pAudioRegisters->SV3_accumulator[2] = ((pPacMan_instance->pacMan_state.audio.voice3Accumulator & 0x0F00) >> EIGHT);
			pAudioRegisters->SV3_accumulator[3] = ((pPacMan_instance->pacMan_state.audio.voice3Accumulator & 0xF000) >> TWELVE);

			// 4) Get the waveform (32 samples) and select the particular sample

			voice1Sample =
				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSoundWaveform
				[(pAudioRegisters->SV1_waveform & 0x07)][(((pPacMan_instance->pacMan_state.audio.voice1Accumulator) & 0xF8000) >> 15)];

			voice2Sample =
				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSoundWaveform
				[(pAudioRegisters->SV2_waveform & 0x07)][(((pPacMan_instance->pacMan_state.audio.voice2Accumulator) & 0x0F800) >> 11)];

			voice3Sample =
				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSoundWaveform
				[(pAudioRegisters->SV3_waveform & 0x07)][(((pPacMan_instance->pacMan_state.audio.voice3Accumulator) & 0x0F800) >> 11)];


			// 5) Multiply with the volume nibble

			pPacMan_instance->pacMan_state.audio.voice1Sample = (PACMAN_AUDIO_SAMPLE_TYPE)(voice1Sample * (pAudioRegisters->V1_volume & 0x0F));
			pPacMan_instance->pacMan_state.audio.voice2Sample = (PACMAN_AUDIO_SAMPLE_TYPE)(voice2Sample * (pAudioRegisters->V2_volume & 0x0F));
			pPacMan_instance->pacMan_state.audio.voice3Sample = (PACMAN_AUDIO_SAMPLE_TYPE)(voice3Sample * (pAudioRegisters->V3_volume & 0x0F));

			// 6) Accumulate these samples

			voiceMix = (pPacMan_instance->pacMan_state.audio.voice1Sample) + (pPacMan_instance->pacMan_state.audio.voice2Sample) + (pPacMan_instance->pacMan_state.audio.voice3Sample);
			voiceMix /= THREE;
		}

		if (pPacMan_instance->pacMan_state.audio.accumulatedAudioSamples >= AUDIO_BUFFER_SIZE_FOR_PACMAN)
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

			if (SDL_PutAudioStreamData(audioStream, pAbsolute_pacMan_instance->absolute_pacMan_state.audioBuffer, sizeof(pAbsolute_pacMan_instance->absolute_pacMan_state.audioBuffer)) == FAILURE)
			{
				SDL_Log("Could not put data on Audio stream, %s", SDL_GetError());
			}
			pPacMan_instance->pacMan_state.audio.accumulatedAudioSamples = RESET;
		}
		else
		{
			pAbsolute_pacMan_instance->absolute_pacMan_state.audioBuffer[pPacMan_instance->pacMan_state.audio.accumulatedAudioSamples] = (PACMAN_AUDIO_SAMPLE_TYPE)(voiceMix);
			++pPacMan_instance->pacMan_state.audio.accumulatedAudioSamples;
		}
	}
}

void pacMan_t::playTheAudioFrame()
{
	;
}

FLAG pacMan_t::processDisplay(uint32_t vramAddress)
{
	// origin of the display is the top left pixel...

	int VRAMOffset = 0x0000;

	uint32_t row = 0;
	uint32_t column = 2;	// 1st 2 columns are not used...
	uint32_t xOffirstPixelOfTile = 0;
	uint32_t yOffirstPixelOfTile = 0;

	uint32_t spriteID = 0;
	uint32_t tileID = 0;
	uint32_t paletteID = 0;

	FLAG flipXneeded = false;
	FLAG flipYneeded = false;

	uint32_t spriteOriginX = 0;
	uint32_t spriteOriginY = 0;

	// Process the address 
	if (vramAddress >= CHARACTER_VRAM_START_ADDRESS && vramAddress <= CHARACTER_VRAM_END_ADDRESS)
	{
		VRAMOffset = vramAddress - CHARACTER_VRAM_START_ADDRESS;
	}
	else if (vramAddress >= CHARACTER_CRAM_START_ADDRESS && vramAddress <= CHARACTER_CRAM_END_ADDRESS)
	{
		VRAMOffset = vramAddress - CHARACTER_CRAM_START_ADDRESS;
	}
	else if
		(
			(vramAddress >= 0x4FF0 && vramAddress <= 0x4FFF)
			||
			(vramAddress >= 0x5060 && vramAddress <= 0x506F)
			)
	{
		VRAMOffset = INVALID;
	}
	else
	{
		RETURN false;
	}

	if (VRAMOffset != INVALID)
	{
		// Get the coordinates of tile in display

		if (VRAMOffset < 0x0020)
		{
			if (
				((VRAMOffset == 0x0000) || (VRAMOffset == 0x0001))
				||
				((VRAMOffset == 0x001E) || (VRAMOffset == 0x001F))
				)
			{
				RETURN false;
			}
			else
			{
				uint32_t temporary = VRAMOffset;

				row = 34;

				column = column + ((MAX_COLUMN_TILES - 1) - ((temporary - 0x0000) - 2));
			}
		}
		else if (VRAMOffset < 0x0040)
		{
			if (
				((VRAMOffset == 0x0020) || (VRAMOffset == 0x0021))
				||
				((VRAMOffset == 0x003E) || (VRAMOffset == 0x003F))
				)
			{
				RETURN false;
			}
			else
			{
				uint32_t temporary = VRAMOffset;

				row = 35;

				column = column + ((MAX_COLUMN_TILES - 1) - ((temporary - 0x0020) - 2));
			}
		}
		else if (VRAMOffset < 0x03C0)
		{
			uint32_t COLUMN_TO_COLUMN_DIFF = 0x0020;
			uint32_t INITIAL_COLUMN_OFFSET = 0x0040;

			uint32_t temporary = VRAMOffset;
			temporary = temporary - INITIAL_COLUMN_OFFSET;

			uint32_t columnFromBackwards = (uint32_t)((float)temporary / (float)COLUMN_TO_COLUMN_DIFF);
			column = column + ((MAX_COLUMN_TILES - 1) - columnFromBackwards);

			row = 2; // start from row number 2 as actual rows 0 and 1 is impossible...
			row += temporary - (columnFromBackwards * COLUMN_TO_COLUMN_DIFF);

		}
		else if (VRAMOffset < 0x03E0)
		{
			if (
				((VRAMOffset == 0x03C0) || (VRAMOffset == 0x03C1))
				||
				((VRAMOffset == 0x03DE) || (VRAMOffset == 0x03DF))
				)
			{
				RETURN false;
			}
			else
			{
				uint32_t temporary = VRAMOffset;

				row = 0;

				column = column + ((MAX_COLUMN_TILES - 1) - ((temporary - 0x03C0) - 2));
			}
		}
		else
		{
			if (
				((VRAMOffset == 0x03E0) || (VRAMOffset == 0x03E1))
				||
				((VRAMOffset == 0x03FE) || (VRAMOffset == 0x03FF))
				)
			{
				RETURN false;
			}
			else
			{
				uint32_t temporary = VRAMOffset;

				row = 1;

				column = column + ((MAX_COLUMN_TILES - 1) - ((temporary - 0x03E0) - 2));
			}
		}

		// Get tile ID and palette ID

		tileID = pPacMan_memory->pacManMemoryMap.mVram.vramFields.videoRam[VRAMOffset];
		paletteID = (pPacMan_memory->pacManMemoryMap.mVram.vramFields.colorRam[VRAMOffset] & 0x3F);

		// Display Tiles

		xOffirstPixelOfTile = column * 8;
		yOffirstPixelOfTile = row * 8;

		for (uint8_t y = 0; y < 8; y++)
		{
			for (uint8_t x = 0; x < 8; x++)
			{
				//pacmanGameEngine->Draw(xOffirstPixelOfTile + x, yOffirstPixelOfTile + y,
				//	pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedTiles[tileID][paletteID].cPixel[x][y]);

				pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer2D[yOffirstPixelOfTile + y][xOffirstPixelOfTile + x]
					= pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedTiles[tileID][paletteID].cPixel[x][y];
			}
		}
	}
	else
	{
#if DEACTIVATED
		for (uint8_t id = 0; id < 64; id++)
		{
			for (uint8_t y = 0; y < 16; y++)
			{
				for (uint8_t x = 0; x < 16; x++)
				{
					Draw(spriteOriginX + x, spriteOriginY + y,
						//pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[id][9].sCPixel[x][y]);
						pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[id][9].sCPixel[x][y]);
					//pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[id][9].sCPixel[x][y]);
					//pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[id][9].sCPixel[x][y]);
				}
			}
			if ((id + 1) % 8 == 0)
			{
				// Row is finished, wrap back around.
				spriteOriginX = 0;
				spriteOriginY += 16;
			}
			else
			{
				// Next column.
				spriteOriginX += 16;
			}
		}
#else
		// Get the coordinates of sprite in display

		uint32_t SPRITE_ID_TO_SPRITE_XY = 0x0070;
		uint32_t convertedX = 0;
		uint32_t convertedY = 0;

		if (vramAddress >= 0x4FF0 && vramAddress <= 0x4FFF)
		{
			// input was sprite number
			if ((vramAddress & 0x01) == 0x00)
			{
				spriteID = ((readRawMemoryFromGFX(vramAddress) >> 2) & 0x3F);
				paletteID = (readRawMemoryFromGFX(vramAddress + 1) & 0x3F);

				flipXneeded = ((readRawMemoryFromGFX(vramAddress) & 0b00000010) == 0x02 ? true : false);
				flipYneeded = ((readRawMemoryFromGFX(vramAddress) & 0b00000001) == 0x01 ? true : false);

				spriteOriginX = readRawMemoryFromGFX(vramAddress + SPRITE_ID_TO_SPRITE_XY);
				spriteOriginY = readRawMemoryFromGFX(vramAddress + SPRITE_ID_TO_SPRITE_XY + 1);
			}
			// input was sprite palette ID
			else
			{
				spriteID = ((readRawMemoryFromGFX(vramAddress - 1) >> 2) & 0x3F);
				paletteID = (readRawMemoryFromGFX(vramAddress) & 0x3F);

				flipXneeded = ((readRawMemoryFromGFX(vramAddress - 1) & 0b00000010) == 0x02 ? true : false);
				flipYneeded = ((readRawMemoryFromGFX(vramAddress - 1) & 0b00000001) == 0x01 ? true : false);

				spriteOriginX = readRawMemoryFromGFX(vramAddress - 1 + SPRITE_ID_TO_SPRITE_XY);
				spriteOriginY = readRawMemoryFromGFX(vramAddress - 1 + SPRITE_ID_TO_SPRITE_XY + 1);
			}
		}
		else
		{
			// input was x location of sprite
			if ((vramAddress & 0x01) == 0x00)
			{
				spriteOriginX = readRawMemoryFromGFX(vramAddress);
				spriteOriginY = readRawMemoryFromGFX(vramAddress + 1);

				spriteID = ((readRawMemoryFromGFX(vramAddress - SPRITE_ID_TO_SPRITE_XY) >> 2) & 0x3F);
				paletteID = (readRawMemoryFromGFX(vramAddress - SPRITE_ID_TO_SPRITE_XY + 1) & 0x3F);

				flipXneeded = ((readRawMemoryFromGFX(vramAddress - SPRITE_ID_TO_SPRITE_XY) & 0b00000010) == 0x02 ? true : false);
				flipYneeded = ((readRawMemoryFromGFX(vramAddress - SPRITE_ID_TO_SPRITE_XY) & 0b00000001) == 0x01 ? true : false);
			}
			// input was y location of sprite
			else
			{
				spriteOriginX = readRawMemoryFromGFX(vramAddress - 1);
				spriteOriginY = readRawMemoryFromGFX(vramAddress);

				spriteID = ((readRawMemoryFromGFX(vramAddress - SPRITE_ID_TO_SPRITE_XY - 1) >> 2) & 0x3F);
				paletteID = (readRawMemoryFromGFX(vramAddress - SPRITE_ID_TO_SPRITE_XY) & 0x3F);

				flipXneeded = ((readRawMemoryFromGFX(vramAddress - SPRITE_ID_TO_SPRITE_XY - 1) & 0b00000010) == 0x02 ? true : false);
				flipYneeded = ((readRawMemoryFromGFX(vramAddress - SPRITE_ID_TO_SPRITE_XY - 1) & 0b00000001) == 0x01 ? true : false);
			}
		}

		// Display Sprites 

		/*
		*
		*	Sprite co-ordinate system used by pacman
		*
		*	(239, 256) .............. (31, 256)
		*		:						  :
		*		:						  :
		*		:						  :
		*		:						  :
		*   (239, 16)  .............. (31, 16)
		*
		*
		*	Our co-ordinate system
		*
		*	(31, 16)  .............. (239, 16)
		* 		:						  :
		* 		:						  :
		* 		:						  :
		* 		:						  :
		* 	(31, 256) .............. (239, 256)
		*
		*/

		convertedX = getScreenWidth() - spriteOriginX;

		// Sprite doesn't show up in top 2 or bottom 2 rows eventhough tiles can show up here...
		// The memory mapping for these 2 rows are also different...
		// These 2 rows individually account for 16 pixels...
		// Since sprites don't show up here, i.e. 0 - 15 cooridnates of y axis, we need to offset that in coverted Y 
		convertedY = getScreenHeight() - 16 - spriteOriginY;

		for (uint8_t y = 0; y < 16; y++)
		{
			for (uint8_t x = 0; x < 16; x++)
			{

				if ((convertedX + x < 0) || convertedX + x >= getScreenWidth())
				{
					continue;
				}

				if ((convertedY + y < 0) || convertedY + y >= getScreenHeight())
				{
					continue;
				}

				if (!flipXneeded && !flipYneeded)
				{
					if (pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y].a == 0)
					{
						continue;
					}

					pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer2D[convertedY + y][convertedX + x]
						= pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y];
				}
				if (flipXneeded && !flipYneeded)
				{
					if (pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y].a == 0)
					{
						continue;
					}

					pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer2D[convertedY + y][convertedX + x]
						= pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y];
				}
				if (!flipXneeded && flipYneeded)
				{
					if (pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y].a == 0)
					{
						continue;
					}

					pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer2D[convertedY + y][convertedX + x]
						= pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y];
				}
				if (flipXneeded && flipYneeded)
				{
					if (pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y].a == 0)
					{
						continue;
					}

					pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer2D[convertedY + y][convertedX + x]
						= pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y];
				}
			}
		}
#endif
	}

	pPacMan_instance->pacMan_state.display.waitingForRefresh = false;

	RETURN true;
}

void pacMan_t::clearCompleteScreen()
{
	for (uint32_t y = 0; y < getScreenHeight(); y++)
	{
		for (uint32_t x = 0; x < getScreenWidth(); x++)
		{
			pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer2D[y][x] = BLACK;
		}
	}
}

void pacMan_t::displayCompleteScreen()
{
	//clearCompleteScreen();

	for (uint32_t vramAddress = CHARACTER_VRAM_START_ADDRESS; vramAddress <= CHARACTER_VRAM_END_ADDRESS; vramAddress++)
	{
		processDisplay(vramAddress);
	}

	for (uint32_t spriteAddress = 0x4FFE; spriteAddress >= 0x4FF0; spriteAddress -= 2)
	{
		processDisplay(spriteAddress);
	}

	// BLACKEN the first 16 and last 16 columns as they seem to cause "no clip" effect
	// TODO: confirm whether this is a valid implementation or just a workaround

	for (uint32_t y = 0; y < getScreenHeight(); y++)
	{
		for (uint32_t x1 = 0; x1 < 16; x1++)
		{
			pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer2D[y][x1] = BLACK;
		}

		for (uint32_t x2 = getScreenWidth() - 16; x2 < getScreenWidth(); x2++)
		{
			pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer2D[y][x2] = BLACK;
		}
	}

	// Display the remaining part of the screen
	
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glDisable(GL_BLEND);

	// Handle for gameboy system's texture

	glBindTexture(GL_TEXTURE_2D, pacman_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer1D);

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
	// 1. Upload emulator framebuffer to pacman_texture
	GL_CALL(glBindTexture(GL_TEXTURE_2D, pacman_texture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
		(GLvoid*)pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer1D));

	// Choose filtering mode (NEAREST or LINEAR)
	GLint filter = (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER) ? GL_LINEAR : GL_NEAREST;
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));

	// 2. Render gameboy_texture into framebuffer (masquerade_texture target)
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
	GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// Pass 1: Render base texture (PacMan framebuffer)
	GL_CALL(glUseProgram(shaderProgramBasic));
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, pacman_texture));
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
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
#endif
}

void pacMan_t::initializeGraphics()
{
	// 1) initialize colors

	uint8_t colorROMByte;
	FLAG isBLACK = false;
	uint8_t r = 0, g = 0, b = 0;

	for (uint32_t colorID = 0; colorID < sizeof(pPacMan_instance->pacMan_state.color_rom.colorROMMemory); colorID++)
	{
		colorROMByte = pPacMan_instance->pacMan_state.color_rom.colorROMMemory[colorID];
		if (colorROMByte == 0x00) { isBLACK = true; }
		else
		{
			if ((colorROMByte & 0b00000001) == 0x01) { r += 0x21; }
			if ((colorROMByte & 0b00000010) == 0x02) { r += 0x47; }
			if ((colorROMByte & 0b00000100) == 0x04) { r += 0x97; }
			if ((colorROMByte & 0b00001000) == 0x08) { g += 0x21; }
			if ((colorROMByte & 0b00010000) == 0x10) { g += 0x47; }
			if ((colorROMByte & 0b00100000) == 0x20) { g += 0x97; }
			if ((colorROMByte & 0b01000000) == 0x40) { b += 0x51; }
			if ((colorROMByte & 0b10000000) == 0x80) { b += 0xAE; }
		}

		if (isBLACK)
		{
			pAbsolute_pacMan_instance->absolute_pacMan_state.decodedColorROM[colorID] = BLACK;
		}
		else
		{
			pAbsolute_pacMan_instance->absolute_pacMan_state.decodedColorROM[colorID] = Pixel(r, g, b);
		}

		isBLACK = false;
		r = g = b = 0;
	}

	// 2) initialize palettes

	uint8_t paletteID = 0;
	for (uint32_t ii = 0; ii < sizeof(pPacMan_instance->pacMan_state.palette_rom.paletteROMMemory); ii += 4)
	{
		pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[paletteID].color_0 =
			pAbsolute_pacMan_instance->absolute_pacMan_state.decodedColorROM
			[
				pPacMan_instance->pacMan_state.palette_rom.paletteROMMemory[ii + 0]
			];
		pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[paletteID].color_1 =
			pAbsolute_pacMan_instance->absolute_pacMan_state.decodedColorROM
			[
				pPacMan_instance->pacMan_state.palette_rom.paletteROMMemory[ii + 1]
			];
		pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[paletteID].color_2 =
			pAbsolute_pacMan_instance->absolute_pacMan_state.decodedColorROM
			[
				pPacMan_instance->pacMan_state.palette_rom.paletteROMMemory[ii + 2]
			];
		pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[paletteID].color_3 =
			pAbsolute_pacMan_instance->absolute_pacMan_state.decodedColorROM
			[
				pPacMan_instance->pacMan_state.palette_rom.paletteROMMemory[ii + 3]
			];
		++paletteID;
	}

	// 3) initialize tiles

	/*
	*
	*   Mapping of tile data from tile rom to display (referred from justin-credible; lomont's PacManEmulation pdf gives slightly different format)
	*
	*	63  59  55  51  47  43	39	35
	*   62  58  54  50  46  42	38	34
	*   61  57  53  49  45  41	37	33
	*   60  56  52  48  44  40  36  32
	*	31  27  23  19  15  11	7	3
	*	30  26  22  18  14  10	6	2
	*	29  25  21  17  13  9	5	1
	*   28  24  20  16  12  8	4	0
	*
	*/

	/*
	*
	*	This is how prerenderedTile is going to save the tile info in its 2d buffer...
	*
	*	0,0		1,0		2,0		3,0		4,0		5,0		6,0		7,0
	*	0,1		1,1		2,1		3,1		4,1		5,1		6,1		7,1
	*	0,2		1,2		2,2		3,2		4,2		5,2		6,2		7,2
	*	0,3		1,3		2,3		3,3		4,3		5,3		6,3		7,3
	*
	*	0,4		1,4		2,4		3,4		4,4		5,4		6,4		7,4
	*	0,5		1,5		2,5		3,5		4,5		5,5		6,5		7,5
	*	0,6		1,6		2,6		3,6		4,6		5,6		6,6		7,6
	*	0,7		1,7		2,7		3,7		4,7		5,7		6,7		7,7
	*
	*/

	auto getColorForTile = [&](uint8_t idPalette, uint8_t bits2color)
	{
		if (bits2color == 0)
		{
			RETURN pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_0;
		}
		else if (bits2color == 1)
		{
			RETURN pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_1;
		}
		else if (bits2color == 2)
		{
			RETURN pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_2;
		}
		else
		{
			RETURN pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_3;
		}
	};

	uint8_t tileROMByte;
	uint8_t tileID = 0;
	uint8_t x = 0, y = 0;
	for (paletteID = 0; paletteID < 64; paletteID++) // 64 loops
	{
		tileID = 0;
		for (uint32_t ii = 0; ii < sizeof(pPacMan_instance->pacMan_state.tile_rom.tileROMMemory); ii += 16)	// 256 loops
		{
			x = 7;
			y = 4;
			for (int jj = 0; jj < 16; jj++)	// 16 loops
			{

				tileROMByte = pPacMan_instance->pacMan_state.tile_rom.tileROMMemory[ii + jj];

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedTiles[tileID][paletteID].cPixel[x][y++] =
					getColorForTile(paletteID, (((tileROMByte & 0b00001000) >> 3) | ((tileROMByte & 0b10000000) >> 6)));

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedTiles[tileID][paletteID].cPixel[x][y++] =
					getColorForTile(paletteID, (((tileROMByte & 0b00000100) >> 2) | ((tileROMByte & 0b01000000) >> 5)));

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedTiles[tileID][paletteID].cPixel[x][y++] =
					getColorForTile(paletteID, (((tileROMByte & 0b00000010) >> 1) | ((tileROMByte & 0b00100000) >> 4)));

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedTiles[tileID][paletteID].cPixel[x][y++] =
					getColorForTile(paletteID, (((tileROMByte & 0b00000001) >> 0) | ((tileROMByte & 0b00010000) >> 3)));

				if (x == 0)
				{
					x = 7;
				}
				else
				{
					--x;
				}

				if (jj >= 7)
				{
					y = 0;
				}
				else
				{
					y = 4;
				}

			}
			++tileID;
		}
	}

	// 4) initialize sprites

	/*
	*
	*   Mapping of sprite data from tile rom to display (referred from justin-credible; lomont's PacManEmulation pdf gives slightly different format)
	*
	* 	191		187		183		179		175		171		167		163		63		59		55		51		47		43		39		35
	* 	190		186		182		178		174		170		166		162		62		58		54		50		46		42		38		34
	* 	189		185		181		177		173		169		165		161		61		57		53		49		45		41		37		33
	*   188		184		180		176		172		168		164		160		60		56		52		48		44		40		36		32
	*
	* 	223		219		215		211		207		203		199		195		95		91		87		83		79		75		71		67
	*  	222		218		214		210		206		202		198		194		94		90		86		82		78		74		70		66
	* 	221		217		213		209		205		201		197		193		93		89		85		81		77		73		69		65
	*   220		216		212		208		204		200		196		192		92		88		84		80		76		72		68		64
	*
	* 	255		251		247		243		239		235		231		227		127		123		119		115		111		107		103		99
	*  	254		250		246		242		238		234		230		226		126		122		118		114		110		106		102		98
	* 	253		249		245		241		237		233		229		225		125		121		117		113		109		105		101		97
	*	252		248		244		240		236		232		228		224		124		120		116		112		108		104		100		96
	*
	* 	159		155		151		147		143		139		135		131		31		27		23		19		15		11		7		3
	* 	158		154		150		146		142		138		134		130		30		26		22		18		14		10		6		2
	* 	157		153		149		145		141		137		133		129		29		25		21		17		13		9		5		1
	* 	156		152		148		144		140		136		132		128		28		24		20		16		12		8		4		0
	*
	*/

	/*
	*
	*	This is how prerenderedSprite is going to save the tile info in its 2d buffer...
	*
	*	0,0		1,0		2,0		3,0		4,0		5,0		6,0		7,0		8,0		9,0		10,0	11,0	12,0	13,0	14,0	15,0
	*	0,1		1,1		2,1		3,1		4,1		5,1		6,1		7,1		8,1		9,1		10,1	11,1	12,1	13,1	14,1	15,1
	*	0,2		1,2		2,2		3,2		4,2		5,2		6,2		7,2		8,2		9,2		10,2	11,2	12,2	13,2	14,2	15,2
	*	0,3		1,3		2,3		3,3		4,3		5,3		6,3		7,3		8,3		9,3		10,3	11,3	12,3	13,3	14,3	15,3
	*
	*	0,4		1,4		2,4		3,4		4,4		5,4		6,4		7,4		8,4		9,4		10,4	11,4	12,4	13,4	14,4	15,4
	*	0,5		1,5		2,5		3,5		4,5		5,5		6,5		7,5		8,5		9,5		10,5	11,5	12,5	13,5	14,5	15,5
	*	0,6		1,6		2,6		3,6		4,6		5,6		6,6		7,6		8,6		9,6		10,6	11,6	12,6	13,6	14,6	15,6
	*	0,7		1,7		2,7		3,7		4,7		5,7		6,7		7,7		8,7		9,7		10,7	11,7	12,7	13,7	14,7	15,7
	*
	*	0,8		1,8		2,8		3,8		4,8		5,8		6,8 	7,8		8,8		9,8		10,8	11,8	12,8	13,8	14,8	15,8
	*	0,9		1,9		2,9		3,9		4,9		5,9		6,9		7,9		8,9		9,9		10,9	11,9	12,9	13,9	14,9	15,9
	*	0,10	1,10	2,10	3,10	4,10	5,10	6,10	7,10	8,10	9,10	10,10	11,10	12,10	13,10	14,10	15,10
	*	0,11	1,11	2,11	3,11	4,11	5,11	6,11	7,11	8,11	9,11	10,11	11,11	12,11	13,11	14,11	15,11
	*
	*	0,12	1,12	2,12	3,12	4,12	5,12	6,12	7,12	8,12	9,12	10,12	11,12	12,12	13,12	14,12	15,12
	*	0,13	1,13	2,13	3,13	4,13	5,13	6,13	7,13	8,13	9,13	10,13	11,13	12,13	13,13	14,13	15,13
	*	0,14	1,14	2,14	3,14	4,14	5,14	6,14	7,14	8,14	9,14	10,14	11,14	12,14	13,14	14,14	15,14
	*	0,15	1,15	2,15	3,15	4,15	5,15	6,15	7,15	8,15	9,15	10,15	11,15	12,15	13,15	14,15	15,15
	*
	*/

	auto getColorForSprite = [&](uint8_t idPalette, uint8_t bits2color)
	{
		if (bits2color == 0)
		{
			pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_0.a = 0;
			RETURN pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_0;
		}
		else if (bits2color == 1)
		{
			if (idPalette == 0)
			{
				pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_1.a = 0;
			}
			else
			{
				pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_1.a = 255;
			}
			RETURN pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_1;
		}
		else if (bits2color == 2)
		{
			if (idPalette == 0)
			{
				pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_2.a = 0;
			}
			else
			{
				pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_2.a = 255;
			}
			RETURN pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_2;
		}
		else if (bits2color == 3)
		{
			if (idPalette == 0)
			{
				pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_3.a = 0;
			}
			else
			{
				pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_3.a = 255;
			}
			RETURN pAbsolute_pacMan_instance->absolute_pacMan_state.decodedPaletteROM[idPalette].color_3;
		}
		else
		{
			LOG("Unknown color ID");
			exit(0);
		}
	};

	uint8_t spriteROMByte;
	uint8_t spriteID = 0;
	x = 0;
	y = 0;
	for (paletteID = 0; paletteID < 64; paletteID++) // 64 loops
	{
		spriteID = 0;
		for (uint32_t ii = 0; ii < sizeof(pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory); ii += 64)	// 64 loops
		{

			// a) No Flip

			x = 15;
			y = 12;

			for (int jj = 0; jj < 32; jj++)	// 32 loops
			{

				spriteROMByte = pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory[ii + jj];

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y++] =
					getColorForSprite(paletteID, (((spriteROMByte & 0b00001000) >> 3) | ((spriteROMByte & 0b10000000) >> 6)));

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y++] =
					getColorForSprite(paletteID, (((spriteROMByte & 0b00000100) >> 2) | ((spriteROMByte & 0b01000000) >> 5)));

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y++] =
					getColorForSprite(paletteID, (((spriteROMByte & 0b00000010) >> 1) | ((spriteROMByte & 0b00100000) >> 4)));

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y++] =
					getColorForSprite(paletteID, (((spriteROMByte & 0b00000001) >> 0) | ((spriteROMByte & 0b00010000) >> 3)));

				if (x == 8)
				{
					x = 15;
				}
				else
				{
					--x;
				}

				if (jj >= 0 + 23)
				{
					y = 8;
				}
				else if (jj >= 0 + 15)
				{
					y = 4;
				}
				else if (jj >= 0 + 7)
				{
					y = 0;
				}
				else
				{
					y = 12;
				}
			}

			x = 7;
			y = 12;

			for (int jj = 32; jj < 64; jj++)	// 32 loops
			{

				spriteROMByte = pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory[ii + jj];

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y++] =
					getColorForSprite(paletteID, (((spriteROMByte & 0b00001000) >> 3) | ((spriteROMByte & 0b10000000) >> 6)));

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y++] =
					getColorForSprite(paletteID, (((spriteROMByte & 0b00000100) >> 2) | ((spriteROMByte & 0b01000000) >> 5)));

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y++] =
					getColorForSprite(paletteID, (((spriteROMByte & 0b00000010) >> 1) | ((spriteROMByte & 0b00100000) >> 4)));

				pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSprites[spriteID][paletteID].sCPixel[x][y++] =
					getColorForSprite(paletteID, (((spriteROMByte & 0b00000001) >> 0) | ((spriteROMByte & 0b00010000) >> 3)));

				if (x == 0)
				{
					x = 7;
				}
				else
				{
					--x;
				}

				if (jj >= 32 + 23)
				{
					y = 8;
				}
				else if (jj >= 32 + 15)
				{
					y = 4;
				}
				else if (jj >= 32 + 7)
				{
					y = 0;
				}
				else
				{
					y = 12;
				}
			}

			// b) Flip X

			if (FLIPX)
			{
				x = 0;
				y = 12;

				for (int jj = 0; jj < 32; jj++)	// 32 loops
				{

					spriteROMByte = pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory[ii + jj];

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y++] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00001000) >> 3) | ((spriteROMByte & 0b10000000) >> 6)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y++] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000100) >> 2) | ((spriteROMByte & 0b01000000) >> 5)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y++] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000010) >> 1) | ((spriteROMByte & 0b00100000) >> 4)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y++] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000001) >> 0) | ((spriteROMByte & 0b00010000) >> 3)));

					if (x == 7)
					{
						x = 0;
					}
					else
					{
						++x;
					}

					if (jj >= 0 + 23)
					{
						y = 8;
					}
					else if (jj >= 0 + 15)
					{
						y = 4;
					}
					else if (jj >= 0 + 7)
					{
						y = 0;
					}
					else
					{
						y = 12;
					}
				}

				x = 8;
				y = 12;

				for (int jj = 32; jj < 64; jj++)	// 32 loops
				{

					spriteROMByte = pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory[ii + jj];

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y++] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00001000) >> 3) | ((spriteROMByte & 0b10000000) >> 6)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y++] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000100) >> 2) | ((spriteROMByte & 0b01000000) >> 5)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y++] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000010) >> 1) | ((spriteROMByte & 0b00100000) >> 4)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipX[spriteID][paletteID].sCPixel[x][y++] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000001) >> 0) | ((spriteROMByte & 0b00010000) >> 3)));

					if (x == 15)
					{
						x = 8;
					}
					else
					{
						++x;
					}

					if (jj >= 32 + 23)
					{
						y = 8;
					}
					else if (jj >= 32 + 15)
					{
						y = 4;
					}
					else if (jj >= 32 + 7)
					{
						y = 0;
					}
					else
					{
						y = 12;
					}
				}
			}

			// c) Flip Y
			if (FLIPY)
			{
				x = 15;
				y = 3;

				for (int jj = 0; jj < 32; jj++)	// 32 loops
				{

					spriteROMByte = pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory[ii + jj];

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00001000) >> 3) | ((spriteROMByte & 0b10000000) >> 6)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000100) >> 2) | ((spriteROMByte & 0b01000000) >> 5)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000010) >> 1) | ((spriteROMByte & 0b00100000) >> 4)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000001) >> 0) | ((spriteROMByte & 0b00010000) >> 3)));

					if (x == 8)
					{
						x = 15;
					}
					else
					{
						--x;
					}

					if (jj >= 0 + 23)
					{
						y = 7;
					}
					else if (jj >= 0 + 15)
					{
						y = 11;
					}
					else if (jj >= 0 + 7)
					{
						y = 15;
					}
					else
					{
						y = 3;
					}
				}

				x = 7;
				y = 3;

				for (int jj = 32; jj < 64; jj++)	// 32 loops
				{

					spriteROMByte = pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory[ii + jj];

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00001000) >> 3) | ((spriteROMByte & 0b10000000) >> 6)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000100) >> 2) | ((spriteROMByte & 0b01000000) >> 5)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000010) >> 1) | ((spriteROMByte & 0b00100000) >> 4)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipY[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000001) >> 0) | ((spriteROMByte & 0b00010000) >> 3)));

					if (x == 0)
					{
						x = 7;
					}
					else
					{
						--x;
					}

					if (jj >= 32 + 23)
					{
						y = 7;
					}
					else if (jj >= 32 + 15)
					{
						y = 11;
					}
					else if (jj >= 32 + 7)
					{
						y = 15;
					}
					else
					{
						y = 3;
					}
				}
			}

			// d) Flip X and Y
			if (FLIPBOTH)
			{
				x = 0;
				y = 3;

				for (int jj = 0; jj < 32; jj++)	// 32 loops
				{

					spriteROMByte = pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory[ii + jj];

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00001000) >> 3) | ((spriteROMByte & 0b10000000) >> 6)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000100) >> 2) | ((spriteROMByte & 0b01000000) >> 5)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000010) >> 1) | ((spriteROMByte & 0b00100000) >> 4)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000001) >> 0) | ((spriteROMByte & 0b00010000) >> 3)));

					if (x == 7)
					{
						x = 0;
					}
					else
					{
						++x;
					}

					if (jj >= 0 + 23)
					{
						y = 7;
					}
					else if (jj >= 0 + 15)
					{
						y = 11;
					}
					else if (jj >= 0 + 7)
					{
						y = 15;
					}
					else
					{
						y = 3;
					}
				}

				x = 8;
				y = 3;

				for (int jj = 32; jj < 64; jj++)	// 32 loops
				{

					spriteROMByte = pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory[ii + jj];

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00001000) >> 3) | ((spriteROMByte & 0b10000000) >> 6)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000100) >> 2) | ((spriteROMByte & 0b01000000) >> 5)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000010) >> 1) | ((spriteROMByte & 0b00100000) >> 4)));

					pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSpritesFlipBoth[spriteID][paletteID].sCPixel[x][y--] =
						getColorForSprite(paletteID, (((spriteROMByte & 0b00000001) >> 0) | ((spriteROMByte & 0b00010000) >> 3)));

					if (x == 15)
					{
						x = 8;
					}
					else
					{
						++x;
					}

					if (jj >= 32 + 23)
					{
						y = 7;
					}
					else if (jj >= 32 + 15)
					{
						y = 11;
					}
					else if (jj >= 32 + 7)
					{
						y = 15;
					}
					else
					{
						y = 3;
					}
				}
			}

			++spriteID;
		}
	}
}

float pacMan_t::getEmulationVolume() 
{
	pPacMan_instance->pacMan_state.audio.emulatorVolume = SDL_GetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream)) * PACMAN_AUDIO_SCALING;
	RETURN pPacMan_instance->pacMan_state.audio.emulatorVolume;
}

void pacMan_t::setEmulationVolume(float volume)
{
	pPacMan_instance->pacMan_state.audio.emulatorVolume = volume;
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), volume / PACMAN_AUDIO_SCALING);
	pt.put("pacman._volume", volume);
	boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, pt);
}

void pacMan_t::initializeAudio()
{
	/*
	*
	* Using little bit of Justin-Credible's work for reference as this is first time complete audio synthesis is done in emulator...
	*
	* There are 2 Sound ROMs of 256 bytes each
	* Only the 1st nibble in each byte is used as a sound sample
	* So, 512 sound samples of 4 bit each...
	* There are organised into 16 waveforms (0 to 15 is the sound ID for these waveforms), each 32 samples long
	*
	* "pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSoundWaveform[].soundSamples[]" is the array which will
	* contain the 16 prerendered sound waveforms
	*
	*/

	uint8_t soundROMByte = 0;
	uint8_t waveformID = 0;

	for (uint32_t ii = 0; ii < sizeof(pPacMan_instance->pacMan_state.sound_rom.soundROMMemory); ii += 32)	// 16 loops
	{
		for (uint32_t sample = 0; sample < 32; sample++)	// 32 loops
		{
			pAbsolute_pacMan_instance->absolute_pacMan_state.prerenderedSoundWaveform[waveformID][sample] =
				(pPacMan_instance->pacMan_state.sound_rom.soundROMMemory[ii + sample] & 0x0F);
		}
		++waveformID;
	}

	// setup the volume for audio
		
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	const SDL_AudioSpec AudioSettings{ SDL_AUDIO_F32, ONE, TO_UINT(EMULATED_AUDIO_SAMPLING_RATE_FOR_PACMAN) };
	audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &AudioSettings, NULL, NULL);
	SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));

	pPacMan_instance->pacMan_state.audio.emulatorVolume = pt.get<std::float_t>("pacman._volume");

	if (_MUTE_AUDIO == YES)
	{
		SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), MUTE_AUDIO);
	}
	else
	{
		SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), pPacMan_instance->pacMan_state.audio.emulatorVolume / PACMAN_AUDIO_SCALING);
	}
}

uint32_t pacMan_t::decryptAddressMethod1(uint32_t in)
{
	uint32_t out;

	out = (in & 0x807);
	out = out | (in & 0x400) >> 7;
	out = out | (in & 0x200) >> 2;
	out = out | (in & 0x080) << 3;
	out = out | (in & 0x040) << 2;
	out = out | (in & 0x138) << 1;

	RETURN out;
}

uint32_t pacMan_t::decryptAddressMethod2(uint32_t in)
{
	uint32_t out;

	out = (in & 0x807);
	out = out | (in & 0x040) << 4;
	out = out | (in & 0x100) >> 3;
	out = out | (in & 0x080) << 2;
	out = out | (in & 0x600) >> 2;
	out = out | (in & 0x028) << 1;
	out = out | (in & 0x010) >> 1;

	RETURN out;
}

BYTE pacMan_t::decryptData(uint32_t in)
{
	uint32_t out;

	out = (in & 0xC0) >> 3;
	out = out | (in & 0x10) << 2;
	out = out | (in & 0x0E) >> 1;
	out = out | (in & 0x01) << 7;
	out = out | (in & 0x20);

	RETURN (BYTE)out;
}

FLAG pacMan_t::runEmulationAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

FLAG pacMan_t::runEmulationLoopAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

FLAG pacMan_t::runEmulationAtFixedRate(uint32_t currentFrame)
{
	FLAG status = true;

	loadQuirks();

	captureIO();

	playTheAudioFrame();

	displayCompleteScreen();

	RETURN status;
}

FLAG pacMan_t::runEmulationLoopAtFixedRate(uint32_t currentFrame)
{
	pPacMan_instance->pacMan_state.display.isVblank = NO;

	if (ROM_TYPE == ROM::TEST_SST)
	{
		static FLAG SST_DEBUG_PRINT = NO;
		const COUNTER32 init_test_opcode = 0x00;
		const COUNTER32 init_test_subopcode = 0x00;
		const COUNTER32 init_test_subopcodeL2 = 0x00;
		COUNTER32 opcode = init_test_opcode;
		COUNTER32 subopcode = init_test_subopcode;
		COUNTER32 subopcodeL2 = init_test_subopcodeL2;
		FLAG cbMode = NO;
		FLAG edMode = NO;
		FLAG ddMode = NO;
		FLAG fdMode = NO;
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

			if (cbMode || edMode || ddMode || fdMode)
			{
				if ((init_test_subopcode != ZERO) && (subopcode == init_test_subopcode))
				{
					SST_DEBUG_PRINT = YES;
				}
				else
				{
					SST_DEBUG_PRINT = NO;
				}
			}

			if (opcode > 0xFF)
			{
				INFO("Completed Running all Tom Harte z80 (v1) tests");
				PAUSE;
			}

			if (opcode == 0xCB)
			{
				cbMode = YES;
			}

			if (opcode == 0xED)
			{
				edMode = YES;
			}

			if (opcode == 0xDD)
			{
				ddMode = YES;

				if (subopcode == 0xCB)
				{
					cbMode = YES;
				}
			}

			if (opcode == 0xFD)
			{
				fdMode = YES;

				if (subopcode == 0xCB)
				{
					cbMode = YES;
				}
			}

			// Opcodes that needs to be excluded from tests
			if (!cbMode && !edMode && !ddMode && !fdMode)
			{
				while (
					opcode == 0xD3
					|| opcode == 0xDB
					)
				{
					++opcode;
				}
			}

			auto unimplementedED = [](uint8_t value)
				{
					static constexpr std::array<uint8_t, 222> unimplementedED = {
						0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
						0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
						0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
						0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
						0x40,0x41,0x45,0x48,0x49,0x4C,0x4D,0x4E,0x50,0x51,0x54,0x55,0x58,0x59,0x5C,0x5D,
						0x60,0x61,0x64,0x65,0x66,0x68,0x69,0x6C,0x6D,0x6E,0x70,0x71,0x74,0x75,0x76,0x77,
						0x78,0x79,0x7C,0x7D,0x7E,0x7F,0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
						0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
						0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xAA,0xAB,0xAC,0xAD,
						0xAE,0xAF,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1,
						0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,
						0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,0xE0,0xE1,
						0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,
						0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
					};

					RETURN std::binary_search(unimplementedED.begin(), unimplementedED.end(), value);
				};

			if (edMode == YES)
			{
				while (unimplementedED(subopcode))
				{
					++subopcode;
				}
			}

			auto unimplementedDDFD = [](uint8_t value)
				{
					static constexpr std::array<uint8_t, 149> unimplementedDDFD = {
						0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
						0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x27,
						0x28, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x37, 0x38, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x76, 0x80,
						0x81, 0x82, 0x83, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x97, 0x98, 0x99,
						0x9A, 0x9B, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAF, 0xB0, 0xB1, 0xB2,
						0xB3, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8,
						0xC9, 0xCA, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9,
						0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE2, 0xE4, 0xE6, 0xE7, 0xE8, 0xEA, 0xEB, 0xEC, 0xED,
						0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD,
						0xFE, 0xFF
					};

					return std::binary_search(unimplementedDDFD.begin(), unimplementedDDFD.end(), value);
				};

			if (ddMode == YES || fdMode == YES)
			{
				while (unimplementedDDFD(subopcode))
				{
					++subopcode;

					if (subopcode == 0xCB)
					{
						cbMode = YES;
					}
				}
			}

			// Get the input
			std::string testCaseName = std::format("{:02x}", opcode);
			if (!cbMode && !edMode && !ddMode && !fdMode)
			{
				testCaseName = _JSON_LOCATION + "\\" + testCaseName + ".json";
				//std::string testCaseName = _JSON_LOCATION + "\\" +  "test.json";
			}
			else
			{
				if ((ddMode == YES || fdMode == YES) && cbMode == YES)
				{
					std::string subtestCaseName = std::format("{:02x}", subopcode);
					std::string subtestCaseNameL2 = std::format("{:02x}", subopcodeL2);
					testCaseName = _JSON_LOCATION + "\\" + testCaseName + " " + subtestCaseName + " __ " + subtestCaseNameL2 + ".json";
					//std::string testCaseName = _JSON_LOCATION + "\\" +  "test.json";
				}
				else
				{
					std::string subtestCaseName = std::format("{:02x}", subopcode);
					testCaseName = _JSON_LOCATION + "\\" + testCaseName + " " + subtestCaseName + ".json";
					//std::string testCaseName = _JSON_LOCATION + "\\" +  "test.json";
				}
			}

			LOG_NEW_LINE;
			INFO("Running : %s\n", testCaseName.c_str());
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
				volatile bool quitThisRun = NO;

				// Accessing top-level fields
				std::string name = item.second.get<std::string>("name");
				if (SST_DEBUG_PRINT)
				{
					std::cout << "Name: " << name << std::endl;
				}

				// Accessing initial state
				auto initial = item.second.get_child("initial");
				int initial_pc = initial.get<int>("pc");
				int initial_sp = initial.get<int>("sp");
				int initial_a = initial.get<int>("a");
				int initial_b = initial.get<int>("b");
				int initial_c = initial.get<int>("c");
				int initial_d = initial.get<int>("d");
				int initial_e = initial.get<int>("e");
				int initial_f = initial.get<int>("f");
				int initial_h = initial.get<int>("h");
				int initial_l = initial.get<int>("l");
				int initial_i = initial.get<int>("i");
				int initial_r = initial.get<int>("r");
				int initial_ei = initial.get<int>("ei");
				int initial_wz = initial.get<int>("wz");
				int initial_ix = initial.get<int>("ix");
				int initial_iy = initial.get<int>("iy");
				int initial_af_ = initial.get<int>("af_");
				int initial_bc_ = initial.get<int>("bc_");
				int initial_de_ = initial.get<int>("de_");
				int initial_hl_ = initial.get<int>("hl_");
				int initial_im = initial.get<int>("im");
				int initial_p = initial.get<int>("p");
				int initial_q = initial.get<int>("q");
				int initial_iff1 = initial.get<int>("iff1");
				int initial_iff2 = initial.get<int>("iff2");

				if (SST_DEBUG_PRINT)
				{
					std::cout << "Initial PC: " << initial_pc << ", SP: " << initial_sp
						<< ", A: " << initial_a << ", B: " << initial_b
						<< ", C: " << initial_c << ", D: " << initial_d
						<< ", E: " << initial_e << ", F: " << initial_f
						<< ", H: " << initial_h << ", L: " << initial_l
						<< ", I: " << initial_i << ", R: " << initial_r
						<< ", EI: " << initial_ei << ", WZ: " << initial_wz
						<< ", IX: " << initial_ix << ", IY: " << initial_iy
						<< ", AF': " << initial_af_ << ", BC': " << initial_bc_
						<< ", DE': " << initial_de_ << ", HL': " << initial_hl_
						<< ", IM: " << initial_im << ", P: " << initial_p
						<< ", Q: " << initial_q << ", IFF1: " << initial_iff1
						<< ", IFF2: " << initial_iff2
						<< std::endl;
				}

				pPacMan_registers->pc = initial_pc;
				pPacMan_registers->sp = initial_sp;

				// Main registers
				pPacMan_registers->af.aAndFRegisters.a = initial_a;
				pPacMan_registers->af.aAndFRegisters.f.flagMemory = initial_f;

				pPacMan_registers->bc.bAndCRegisters.b = initial_b;
				pPacMan_registers->bc.bAndCRegisters.c = initial_c;

				pPacMan_registers->de.dAndERegisters.d = initial_d;
				pPacMan_registers->de.dAndERegisters.e = initial_e;

				pPacMan_registers->hl.hAndLRegisters.h = initial_h;
				pPacMan_registers->hl.hAndLRegisters.l = initial_l;

				// Index registers
				pPacMan_registers->ix.ix_u16memory = initial_ix;
				pPacMan_registers->iy.iy_u16memory = initial_iy;

				// Special registers
				pPacMan_registers->i = initial_i;
				pPacMan_registers->r = initial_r;
				MASQ_UNUSED(initial_ei);
				pPacMan_instance->pacMan_state.interruptMode = (INTERRUPT_MODE)initial_im;
				pPacMan_registers->wz = initial_wz;

				// Shadow registers
				pPacMan_registers->shadow_af.aAndFRegisters.a = (initial_af_ >> 8) & 0xFF;
				pPacMan_registers->shadow_af.aAndFRegisters.f.flagMemory = initial_af_ & 0xFF;

				pPacMan_registers->shadow_bc.bAndCRegisters.b = (initial_bc_ >> 8) & 0xFF;
				pPacMan_registers->shadow_bc.bAndCRegisters.c = initial_bc_ & 0xFF;

				pPacMan_registers->shadow_de.dAndERegisters.d = (initial_de_ >> 8) & 0xFF;
				pPacMan_registers->shadow_de.dAndERegisters.e = initial_de_ & 0xFF;

				pPacMan_registers->shadow_hl.hAndLRegisters.h = (initial_hl_ >> 8) & 0xFF;
				pPacMan_registers->shadow_hl.hAndLRegisters.l = initial_hl_ & 0xFF;

				// Other registers
				pPacMan_registers->p = initial_p;
				pPacMan_registers->q = initial_q;

				// Other interrupt-related registers
				pPacMan_registers->iff1 = initial_iff1;
				pPacMan_registers->iff2 = initial_iff2;

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
					pPacMan_memory->pacManRawMemory[address] = value;
				}

				// Run the CPU
				performOperation();

				// Internal cycles are padded with previous address
				while (pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst != RESET)
				{
					auto index = pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].address
						= pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index - ONE].address;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isRead = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].isWrite = NO;
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[index].data = RESET;
					++pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer;
					--pPacMan_instance->pacMan_state.others.tomHarte.cycles.cyclePerInst;
				}

				// Accessing final state
				auto final = item.second.get_child("final");
				int final_pc = final.get<int>("pc");
				int final_sp = final.get<int>("sp");
				int final_a = final.get<int>("a");
				int final_b = final.get<int>("b");
				int final_c = final.get<int>("c");
				int final_d = final.get<int>("d");
				int final_e = final.get<int>("e");
				int final_f = final.get<int>("f");
				int final_h = final.get<int>("h");
				int final_l = final.get<int>("l");
				int final_i = final.get<int>("i");
				int final_r = final.get<int>("r");
				int final_ei = final.get<int>("ei");
				int final_wz = final.get<int>("wz");
				int final_ix = final.get<int>("ix");
				int final_iy = final.get<int>("iy");
				int final_af_ = final.get<int>("af_");
				int final_bc_ = final.get<int>("bc_");
				int final_de_ = final.get<int>("de_");
				int final_hl_ = final.get<int>("hl_");
				int final_im = final.get<int>("im");
				int final_p = final.get<int>("p");
				int final_q = final.get<int>("q");
				int final_iff1 = final.get<int>("iff1");
				int final_iff2 = final.get<int>("iff2");

				if (SST_DEBUG_PRINT)
				{
					std::cout << "Final PC: " << final_pc << ", SP: " << final_sp
						<< ", A: " << final_a << ", B: " << final_b
						<< ", C: " << final_c << ", D: " << final_d
						<< ", E: " << final_e << ", F: " << final_f
						<< ", H: " << final_h << ", L: " << final_l
						<< ", I: " << final_i << ", R: " << final_r
						<< ", EI: " << final_ei << ", WZ: " << final_wz
						<< ", IX: " << final_ix << ", IY: " << final_iy
						<< ", AF': " << final_af_ << ", BC': " << final_bc_
						<< ", DE': " << final_de_ << ", HL': " << final_hl_
						<< ", IM: " << final_im << ", P: " << final_p
						<< ", Q: " << final_q << ", IFF1: " << final_iff1
						<< ", IFF2: " << final_iff2
						<< std::endl;
				}

				if (pPacMan_registers->pc != final_pc)
				{
					WARN("PC Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->sp != final_sp)
				{
					WARN("SP Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->af.aAndFRegisters.a != final_a)
				{
					WARN("A Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->bc.bAndCRegisters.b != final_b)
				{
					WARN("B Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->bc.bAndCRegisters.c != final_c)
				{
					WARN("C Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->de.dAndERegisters.d != final_d)
				{
					WARN("D Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->de.dAndERegisters.e != final_e)
				{
					WARN("E Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->af.aAndFRegisters.f.flagMemory != final_f)
				{
					WARN("F Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->hl.hAndLRegisters.h != final_h)
				{
					WARN("H Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->hl.hAndLRegisters.l != final_l)
				{
					WARN("L Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->i != final_i)
				{
					WARN("I Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->r != final_r)
				{
					WARN("R Mismatch");
					quitThisRun = YES;
				}
				MASQ_UNUSED(final_ei);
				if (pPacMan_instance->pacMan_state.interruptMode != (INTERRUPT_MODE)final_im)
				{
					WARN("IM Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->wz != final_wz)
				{
					WARN("WZ Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->ix.ix_u16memory != final_ix)
				{
					WARN("IX Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->iy.iy_u16memory != final_iy)
				{
					WARN("IY Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->shadow_af.af_u16memory != final_af_)
				{
					WARN("Shadow AF Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->shadow_bc.bc_u16memory != final_bc_)
				{
					WARN("Shadow BC Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->shadow_de.de_u16memory != final_de_)
				{
					WARN("Shadow DE Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->shadow_hl.hl_u16memory != final_hl_)
				{
					WARN("Shadow HL Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->p != final_p)
				{
					WARN("P Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->q != final_q)
				{
					WARN("Q Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->iff1 != final_iff1)
				{
					WARN("IFF1 Mismatch");
					quitThisRun = YES;
				}
				if (pPacMan_registers->iff2 != final_iff2)
				{
					WARN("IFF2 Mismatch");
					quitThisRun = YES;
				}

				pPacMan_registers->pc = RESET;
				pPacMan_registers->sp = RESET;

				// Main registers
				pPacMan_registers->af.aAndFRegisters.a = RESET;
				pPacMan_registers->af.aAndFRegisters.f.flagMemory = RESET;

				pPacMan_registers->bc.bAndCRegisters.b = RESET;
				pPacMan_registers->bc.bAndCRegisters.c = RESET;

				pPacMan_registers->de.dAndERegisters.d = RESET;
				pPacMan_registers->de.dAndERegisters.e = RESET;

				pPacMan_registers->hl.hAndLRegisters.h = RESET;
				pPacMan_registers->hl.hAndLRegisters.l = RESET;

				pPacMan_registers->ix.ixRegisters.ixh = RESET;
				pPacMan_registers->ix.ixRegisters.ixl = RESET;

				pPacMan_registers->iy.iyRegisters.iyh = RESET;
				pPacMan_registers->iy.iyRegisters.iyl = RESET;

				pPacMan_registers->i = RESET;
				pPacMan_registers->r = RESET;

				pPacMan_instance->pacMan_state.interruptMode = INTERRUPT_MODE::INTERRUPT_MODE_0;

				pPacMan_registers->wz = RESET;

				// Shadow registers
				pPacMan_registers->shadow_af.aAndFRegisters.a = RESET;
				pPacMan_registers->shadow_af.aAndFRegisters.f.flagMemory = RESET;

				pPacMan_registers->shadow_bc.bAndCRegisters.b = RESET;
				pPacMan_registers->shadow_bc.bAndCRegisters.c = RESET;

				pPacMan_registers->shadow_de.dAndERegisters.d = RESET;
				pPacMan_registers->shadow_de.dAndERegisters.e = RESET;

				pPacMan_registers->shadow_hl.hAndLRegisters.h = RESET;
				pPacMan_registers->shadow_hl.hAndLRegisters.l = RESET;

				pPacMan_registers->p = RESET;
				pPacMan_registers->q = RESET;

				pPacMan_registers->iff1 = RESET;
				pPacMan_registers->iff2 = RESET;

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

					if (pPacMan_memory->pacManRawMemory[address] != value)
					{
						WARN("RAM Mismatch");
						quitThisRun = YES;
					}

					pPacMan_memory->pacManRawMemory[address] = RESET;
				}

#if (ENABLED)
				// Accessing cycles
				if (SST_DEBUG_PRINT)
				{
					std::cout << "Cycles:" << std::endl;
				}
				pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer = RESET;
				INC8 indexer = RESET;
				for (const auto& cycle : item.second.get_child("cycles"))
				{
					auto it = cycle.second.begin();
					int cycle_address = it->second.get_value<int>(); // First element
					++it; // Move to the second element
					int cycle_value = it->second.get_value<int>(0);
					++it; // Move to the third element
					std::string cycle_type = it->second.get_value<std::string>(); // Third element
					if (SST_DEBUG_PRINT)
					{
						std::cout << "Cycle Address: " << cycle_address << ", Value: " << cycle_value << ", Type: " << cycle_type << std::endl;
					}

					std::string temp = "----";
					if (pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[indexer].isWrite == YES)
					{
						temp = "-wm-";
					}
					if (pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[indexer].isRead == YES)
					{
						temp = "r-m-";
					}

					if (cycle_type.compare(temp))
					{
						WARN("Operation Cycle Mismatch");
						quitThisRun = YES;
					}

					if (cycle_address != pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[indexer].address)
					{
						WARN("Address Cycle Mismatch");
						quitThisRun = YES;
					}

					if (cycle_value != pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[indexer].data)
					{
						WARN("Data Cycle Mismatch");
						quitThisRun = YES;
					}

					++indexer;
				}

				for (INC8 ii = ZERO; ii < TWENTY; ii++)
				{
					pPacMan_instance->pacMan_state.others.tomHarte.cycles.cycles[ii].reset();
				}
#else
				pPacMan_instance->pacMan_state.others.tomHarte.cycles.indexer = RESET;
#endif

				if (quitThisRun == YES)
				{
					FATAL("SST Failure");
				}

				// Update Stats
				if (cbMode == YES)
				{
					++pPacMan_instance->pacMan_state.others.tomHarte.cbtestCount[subopcode];
				}
				if (edMode == YES)
				{
					++pPacMan_instance->pacMan_state.others.tomHarte.edtestCount[subopcode];
				}
				if (ddMode == YES)
				{
					++pPacMan_instance->pacMan_state.others.tomHarte.ddtestCount[subopcode];
				}
				if (fdMode == YES)
				{
					++pPacMan_instance->pacMan_state.others.tomHarte.fdtestCount[subopcode];
				}

				if (!cbMode || !edMode || !ddMode || !fdMode)
				{
					++pPacMan_instance->pacMan_state.others.tomHarte.testCount[opcode];
				}

#if _DEBUG
				if (pPacMan_instance->pacMan_state.others.tomHarte.testCount[0x02] == 419)
				{
					volatile int breakpoint = 0;
				}

				if (pPacMan_instance->pacMan_state.others.tomHarte.edtestCount[0x57] == 4)
				{
					volatile int breakpoint = 0;
				}
#endif
			}

#if 1
			// TODO: Temporary code as we have not implemented 0xDDFF, 0xFDFF and 0xEDFF
			if (ddMode == YES || fdMode == YES)
			{
				if (subopcode == 0xE9)
				{
					ddMode = NO;
					fdMode = NO;
				}
			}

			if (edMode == YES)
			{
				if (subopcode == 0xB9)
				{
					edMode = NO;
				}
			}
#endif

			if (subopcodeL2 == 0xFF)
			{
				cbMode = NO;
			}

			if (subopcode == 0xFF)
			{
				cbMode = NO;
				edMode = NO;
				ddMode = NO;
				fdMode = NO;
			}

			if (!cbMode && !edMode && !ddMode && !fdMode)
			{
				++opcode;
				subopcode = RESET;
				subopcodeL2 = RESET;
			}
			else
			{
				if ((ddMode == YES || fdMode == YES) && cbMode == YES)
				{
					++subopcodeL2;
				}
				else
				{
					++subopcode;
					subopcodeL2 = RESET;
				}
			}
		}
	}
	else
	{
		if (ROM_TYPE == ROM::TEST_ROM_COM || ROM_TYPE == ROM::TEST_ROM_CIM || ROM_TYPE == ROM::TEST_ROM_TAP)
		{
			LOG("Starting the test\n\n");
			pPacMan_registers->pc = 0x0100;

			uint32_t singleTestAddress = 0;

			if (_TEST_NUMBER != INVALID)
			{
				singleTestAddress = 0x100 + 58 + (_TEST_NUMBER * 2);
				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[(0x100 + 58)] =
					pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[singleTestAddress];
				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[(0x100 + 58 + 1)] =
					pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[singleTestAddress + 1];

				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[(0x100 + 58 + 2)] = 0x00;
				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[(0x100 + 58 + 3)] = 0x00;
			}

			if (ROM_TYPE == ROM::TEST_ROM_COM)
			{
				// CP/M BIOS/BDOS stub to handle COM files

				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x0006] = 0x00;
				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x0007] = 0xF0;

				while (!testFinished)
				{
					++testROMCycles;

					//if (testROMCycles == 895)
					//{
					//	volatile int a = 0;
					//}

					performOperation();

#ifdef NDEBUG
					//if (testROMCycles > 3227169747)
					//{
					//	LOG("Test number %" PRId64, testROMCycles);
					//	LOG("Cycles till now is %" PRId64, pPacMan_cpuInstance->cpuCounter);
					//}
#endif

					if (pPacMan_registers->pc == 0x0005)
					{
						if (cpuReadRegister(REGISTER_TYPE::RT_C) == 2)
						{
							std::cout << (char)(cpuReadRegister(REGISTER_TYPE::RT_A));
						}
						else if (cpuReadRegister(REGISTER_TYPE::RT_C) == 9)
						{
							uint16_t readAddress = cpuReadRegister(REGISTER_TYPE::RT_DE);
							while (((char)pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[readAddress]) != '$')
							{
								std::cout << (char)(pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[readAddress]);
								readAddress++;
							}
						}

						BYTE lowerData = stackPop();
						BYTE higherData = stackPop();
						uint16_t operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
						pPacMan_registers->pc = operationResult;
					}

					if (pPacMan_registers->pc == 0x0000)
					{
						testFinished = true;
					}
				}
			}
			else if (ROM_TYPE == ROM::TEST_ROM_CIM)
			{
				// CP/M BIOS/BDOS stub to handle CIM files
				// CIM expects an OUT (0), IN (0) hook to terminate/print

				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x0000] = 0xD3;
				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x0001] = 0x00;
				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x0005] = 0xDB;
				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x0006] = 0x00;
				pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x0007] = 0xC9;

				while (!testFinished)
				{
					++testROMCycles;

					//if (testROMCycles == 3228849330)
					//{
					//	volatile int a = 0;
					//}

					performOperation();

#ifdef NDEBUG
					//if (testROMCycles > 3227169747)
					//{
					//	LOG("Test number %" PRId64, testROMCycles);
					//	LOG("Cycles till now is %" PRId64, pPacMan_cpuInstance->cpuCounter);
					//}
#endif

				// --- CIM handling (I/O trap at port 0)

					if (pPacMan_cpuInstance->opcode == 0xDB && pPacMan_cpuInstance->port == 0)
					{
						if (cpuReadRegister(REGISTER_TYPE::RT_C) == 2)
						{
							std::cout << (char)(cpuReadRegister(REGISTER_TYPE::RT_E));
						}
						else if (cpuReadRegister(REGISTER_TYPE::RT_C) == 9)
						{
							uint16_t readAddress = cpuReadRegister(REGISTER_TYPE::RT_DE);
							while (((char)pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[readAddress]) != '$')
							{
								std::cout << (char)(pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[readAddress]);
								readAddress++;
							}
						}
					}

					if (pPacMan_cpuInstance->opcode == 0xD3 && pPacMan_cpuInstance->port == 0)
					{
						testFinished = true;
					}
				}
			}

			LOG_NEW_LINE;
			LOG("Number of tests: %" PRId64, testROMCycles);
			LOG("Total cycles: %" PRId64, pPacMan_cpuInstance->cpuCounter);
			LOG("Test complete");
			exit(0);
		}
		else
		{
			pPacMan_instance->pacMan_state.display.isVblank = processCPU();

			processAudio();
		}
	}

	RETURN pPacMan_instance->pacMan_state.display.isVblank;
}

FLAG pacMan_t::setupEmulatorForMsPacMan(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, uint32_t* totalAuxilaryRomSize)
{
	// Load and decrypt the Ms. Pac-Man daughterboard ROMs and apply patches to the base Pac-Man ROMs.
	// The patches will be applied to a seperate memory instance because we still need the original,
	// unmodified Pac-Man code ROMs present to boot and pass the self-test.

	// Fill 0xFF in SPARE (easier for debugging)

	for (uint32_t ii = 0; ii < sizeof(pPacMan_memory->pacManMemoryMap.SPARE); ii++)
	{
		pPacMan_memory->pacManMemoryMap.SPARE[ii] = 0xFF;
	}

	for (uint32_t ii = 0; ii < sizeof(pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.SPARE0); ii++)
	{
		pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.SPARE0[ii] = 0xFF;
	}

	for (uint32_t ii = 0; ii < sizeof(pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.SPARE1); ii++)
	{
		pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.SPARE1[ii] = 0xFF;
	}

	FILE* fp = NULL;
	uint32_t totalRomSize = 0;

	// Load, Decrypt and Store u7, u6 and u5 in cloned memory for now

	// create temporary aux memory to hold the load data (before decryption)...

	typedef struct
	{
		BYTE u7[0x1000];
		BYTE u6[0x1000];
		BYTE u5[0x1000];
	} placeholderMemFields_t;

	typedef union
	{
		placeholderMemFields_t placeholderMemFields;
		BYTE placeholderMemory[sizeof(placeholderMemFields_t)];
	} placeholderForMsPacMan_t;

	auto placeholderToLoad = new placeholderForMsPacMan_t;

	// Load the Auxillary ROMs

	// file u5

	errno_t err = fopen_portable(&fp, rom[TEN].c_str(), "rb");

	if (!err && (fp != NULL))
	{
		// get the size of the complete rom
		fseek(fp, 0, SEEK_END);
		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize = ftell(fp);
		*totalAuxilaryRomSize += pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize;

		// read the complete rom
		rewind(fp);
		fread(placeholderToLoad->placeholderMemFields.u5, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize, 1, fp);
		LOG("file u5 loaded");

		// close the rom for now
		fclose(fp);

		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
	}
	else
	{
		LOG("Unable to load the rom file");
		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
		RETURN false;
	}

	// file u6

	err = fopen_portable(&fp, rom[ELEVEN].c_str(), "rb");

	if (!err && (fp != NULL))
	{
		// get the size of the complete rom
		fseek(fp, 0, SEEK_END);
		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize = ftell(fp);
		*totalAuxilaryRomSize += pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize;

		// read the complete rom
		rewind(fp);
		fread(placeholderToLoad->placeholderMemFields.u6, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize, 1, fp);
		LOG("file u6 loaded");

		// close the rom for now
		fclose(fp);

		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;

	}
	else
	{
		LOG("Unable to load the rom file");
		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
		RETURN false;
	}

	// file u7

	err = fopen_portable(&fp, rom[TWELVE].c_str(), "rb");

	if (!err && (fp != NULL))
	{
		// get the size of the complete rom
		fseek(fp, 0, SEEK_END);
		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize = ftell(fp);
		*totalAuxilaryRomSize += pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize;

		// read the complete rom
		rewind(fp);
		fread(placeholderToLoad->placeholderMemFields.u7, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize, 1, fp);
		LOG("file u7 loaded");

		// close the rom for now
		fclose(fp);

		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = true;

	}
	else
	{
		LOG("Unable to load the rom file");
		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
		RETURN false;
	}

	pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.auxillaryCodeRomSize = (*totalAuxilaryRomSize);

	// Decrypt the Auxillary ROMs 

	// create temporary aux memory to hold the decrypted data...

	auto placeholderAfterDecrypt = new placeholderForMsPacMan_t;

#ifndef PAC_MAN_CODE_ROM_MAX_CHIP_SIZE
#define PAC_MAN_CODE_ROM_MAX_CHIP_SIZE (0x1000)
#endif

	for (uint32_t ii = 0; ii < PAC_MAN_CODE_ROM_MAX_CHIP_SIZE; ii++)
	{
		if (ii < 0x0800)
		{
			placeholderAfterDecrypt->placeholderMemFields.u5[decryptAddressMethod2(ii)] =
				decryptData(placeholderToLoad->placeholderMemFields.u5[ii]);
		}

		placeholderAfterDecrypt->placeholderMemFields.u6[decryptAddressMethod1(ii)] =
			decryptData(placeholderToLoad->placeholderMemFields.u6[ii]);

		placeholderAfterDecrypt->placeholderMemFields.u7[decryptAddressMethod1(ii)] =
			decryptData(placeholderToLoad->placeholderMemFields.u7[ii]);

	}

	// Copy the original PacMan ROMs to the cloned address

	for (uint32_t ii = 0; ii < PAC_MAN_CODE_ROM_MAX_CHIP_SIZE; ii++)
	{
		// 6e, 6f and 6h are copied as it is to the cloned memory

		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6e[ii] =
			pPacMan_memory->pacManMemoryMap.mCodeRom.codeRomFields.code_rom_6e[ii];

		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[ii] =
			pPacMan_memory->pacManMemoryMap.mCodeRom.codeRomFields.code_rom_6f[ii];

		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[ii] =
			pPacMan_memory->pacManMemoryMap.mCodeRom.codeRomFields.code_rom_6h[ii];

		// 6j is completely replaced by decrypted U7 in the cloned memory

		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6j[ii] =
			placeholderAfterDecrypt->placeholderMemFields.u7[ii];

		// Copy the placeholder data to its actual position in the original memory map...

		if (ii < 0x0800)
		{
			pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[ii] =
				placeholderAfterDecrypt->placeholderMemFields.u5[ii];
		}

		pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u6[ii] =
			placeholderAfterDecrypt->placeholderMemFields.u6[ii];

		pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u7[ii] =
			placeholderAfterDecrypt->placeholderMemFields.u7[ii];

	}

#undef PAC_MAN_CODE_ROM_MAX_CHIP_SIZE

	// Delete all the placeholders

	delete(placeholderToLoad);
	delete(placeholderAfterDecrypt);

	// Patch 6e, 6f, 6h using u5
	// FYI - HW actually latches any read from these address, which we simulate as patch...

	for (uint8_t ii = 0; ii < 8; ii++)
	{
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6e[0x0410 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0008 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6e[0x08E0 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x01D8 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6e[0x0A30 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0118 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6e[0x0BD0 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x00D8 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6e[0x0C20 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0120 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6e[0x0E58 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0168 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6e[0x0EA8 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0198 + ii];

		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x0000 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0020 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x0008 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0010 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x0288 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0098 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x0348 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0048 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x0688 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0088 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x06B0 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0188 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x06D8 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x00C8 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x06F8 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x01C8 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x09A8 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x00A8 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6f[0x09B8 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x01A8 + ii];

		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0060 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0148 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0108 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0018 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x01A0 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x01A0 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0298 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x00A0 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x03E0 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x00E8 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0418 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0000 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0448 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0058 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0470 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0140 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0488 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0080 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x04B0 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0180 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x04D8 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x00C0 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x04F8 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x01C0 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0748 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0050 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0780 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0090 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x07B8 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0190 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0800 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0028 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0B20 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0100 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0B30 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0110 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0BF0 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x01D0 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0CC0 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x00D0 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0CD8 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x00E0 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0CF0 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x01E0 + ii];
		pPacMan_memory->pacManMemoryMap.mPatched.mCodeRomPatched.codeRomFields.code_rom_6h[0x0D60 + ii] = pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5[0x0160 + ii];
	}

	RETURN true;
}

FLAG pacMan_t::initializeEmulator()
{
	FLAG status = true;

	pAbsolute_pacMan_instance = std::make_shared<absolute_pacMan_instance_t>();

	// for readability

	pPacMan_instance = (pacMan_instance_t*)&(pAbsolute_pacMan_instance->absolute_pacMan_state.pacMan_instance);
	pPacMan_registers = &(pPacMan_instance->pacMan_state.registers);
	pPacMan_cpuInstance = &(pPacMan_instance->pacMan_state.cpuInstance);
	pPacMan_memory = &(pPacMan_instance->pacMan_state.pacManMemory);
	pPacMan_flags = &(pPacMan_registers->af.aAndFRegisters.f.flagFields);
	pAudioRegisters = &(pPacMan_instance->pacMan_state.pacManMemory.pacManMemoryMap.audioANDSpriteCoordinatesMemoryMap.audioANDSpriteCoordinates);
	pIN0Memory = &(pPacMan_instance->pacMan_state.IN0Memory);
	pIN1Memory = &(pPacMan_instance->pacMan_state.IN1Memory);

	if (ROM_TYPE == ROM::TEST_ROM_COM || ROM_TYPE == ROM::TEST_ROM_CIM || ROM_TYPE == ROM::TEST_ROM_TAP)
	{
		RETURN status;
	}

	// other initializations

	pPacMan_instance->pacMan_state.others.hostCPUCycle = ZERO;
	pPacMan_cpuInstance->cpuCounter = ZERO;

	pPacMan_registers->pc = 0x0000;

	pPacMan_instance->pacMan_state.interruptMode = INTERRUPT_MODE::INTERRUPT_MODE_0;
	pPacMan_instance->pacMan_state.port0Data = ZERO;
	pPacMan_instance->pacMan_state.HaltEnabled = false;

	pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.auxBoardEnable = ZERO;

	// topmost address of RAM as stack is of the decrementing type

	pPacMan_registers->sp = 0x4FEF; // Don't keep it at 0x4FF0 as sprite rom begins there and some instruction use sp without decrementing first!

	// initialize DIP and INI switches

	pPacMan_memory->pacManMemoryMap.DIPMemory.DIP[ZERO].DIP0 = 1;		// 1 coin per game
	pPacMan_memory->pacManMemoryMap.DIPMemory.DIP[ZERO].DIP1 = 0;		// 1 coin per game
	pPacMan_memory->pacManMemoryMap.DIPMemory.DIP[ZERO].DIP2 = 0;		// 3 lives
	pPacMan_memory->pacManMemoryMap.DIPMemory.DIP[ZERO].DIP3 = 1;		// 3 lives
	pPacMan_memory->pacManMemoryMap.DIPMemory.DIP[ZERO].DIP4 = 0;		// 10000 points
	pPacMan_memory->pacManMemoryMap.DIPMemory.DIP[ZERO].DIP5 = 0;		// 10000 points
	pPacMan_memory->pacManMemoryMap.DIPMemory.DIP[ZERO].SOLDER2 = 1;	// Normal difficulty
	pPacMan_memory->pacManMemoryMap.DIPMemory.DIP[ZERO].SOLDER1 = 1;	// Normal ghost names

	pIN1Memory->IN1Fields[ZERO].BOARDTEST = 1;					// Disable Board Test
	pIN1Memory->IN1Fields[ZERO].CABINET = 1;					// Upright Mode

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

		glGenTextures(1, &pacman_texture);
		glBindTexture(GL_TEXTURE_2D, pacman_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer1D);
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

		// 3. PacMan texture (used to upload emulated framebuffer)
		GL_CALL(glGenTextures(1, &pacman_texture));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, pacman_texture));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pPacMan_instance->pacMan_state.display.imGuiBuffer.imGuiBuffer1D));
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

void pacMan_t::destroyEmulator()
{
	// reset globals

	memset(doubleInput, ZERO, static_cast<size_t>((PACMAN_AUDIO_SAMPLING_RATE / PACMAN_FPS)));
	memset(doubleOutput, ZERO, static_cast<size_t>((PACMAN_AUDIO_SAMPLING_RATE / PACMAN_FPS)));

	// deallocate memory

	pAbsolute_pacMan_instance.reset();

	if (isCLI() == NO)
	{
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
		glDeleteTextures(1, &pacman_texture);
		glDeleteTextures(1, &matrix_texture);
#else
		glDeleteTextures(1, &pacman_texture);
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
}

FLAG pacMan_t::getRomLoadedStatus()
{
	RETURN pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded;
}

FLAG pacMan_t::loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom)
{
	// open the rom file

	FILE* fp = NULL;
	uint32_t totalRomSize = 0;
	uint32_t totalAuxilaryRomSize = 0;

	if ((ROM_TYPE == ROM::TEST_ROM_COM || ROM_TYPE == ROM::TEST_ROM_CIM) && !rom[ZERO].empty())
	{
		// Test ROM

		errno_t err = fopen_portable(&fp, rom[ZERO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize = ftell(fp);
			totalRomSize += pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize;

			// read the complete rom
			rewind(fp);
			fread(&pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x0100], pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize, 1, fp);
			LOG("test rom loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
		}
	}
	else
	{
		// file 6e

		errno_t err = fopen_portable(&fp, rom[ZERO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize = ftell(fp);
			totalRomSize += pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize;

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.pacManMemory.pacManMemoryMap.mCodeRom.codeRomFields.code_rom_6e, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize, 1, fp);
			LOG("file 6e loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
		}

		// file 6f

		err = fopen_portable(&fp, rom[ONE].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize = ftell(fp);
			totalRomSize += pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize;

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.pacManMemory.pacManMemoryMap.mCodeRom.codeRomFields.code_rom_6f, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize, 1, fp);
			LOG("file 6f loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
		}

		// file 6h

		err = fopen_portable(&fp, rom[TWO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize = ftell(fp);
			totalRomSize += pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize;

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.pacManMemory.pacManMemoryMap.mCodeRom.codeRomFields.code_rom_6h, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize, 1, fp);
			LOG("file 6h loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
		}

		// file 6j

		err = fopen_portable(&fp, rom[THREE].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize = ftell(fp);
			totalRomSize += pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize;

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.pacManMemory.pacManMemoryMap.mCodeRom.codeRomFields.code_rom_6j, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize, 1, fp);
			LOG("file 6j loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
			RETURN false;
		}

		// file 7f

		err = fopen_portable(&fp, rom[FOUR].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.colorRomSize = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.color_rom.colorROMMemory, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.colorRomSize, 1, fp);
			LOG("file 7f loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
			RETURN false;
		}

		// file 4a

		err = fopen_portable(&fp, rom[FIVE].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.paletteRomSize = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.palette_rom.paletteROMMemory, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.paletteRomSize, 1, fp);
			LOG("file 4a loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
			RETURN false;
		}

		// file 5e

		err = fopen_portable(&fp, rom[SIX].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.tileRomSize = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.tile_rom.tileROMMemory, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.tileRomSize, 1, fp);
			LOG("file 5e loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
			RETURN false;
		}

		// file 5f

		err = fopen_portable(&fp, rom[SEVEN].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.spriteRomSize = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.spriteRomSize, 1, fp);
			LOG("file 5f loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
			RETURN false;
		}

		// file 1m

		err = fopen_portable(&fp, rom[EIGHT].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.soundRom1Size = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.sound_rom.soundROM.sound_rom1, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.soundRom1Size, 1, fp);
			LOG("file 1m loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
			RETURN false;
		}

		// file 3m

		err = fopen_portable(&fp, rom[NINE].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.soundRom2Size = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pPacMan_instance->pacMan_state.sound_rom.soundROM.sound_rom2, pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.soundRom2Size, 1, fp);
			LOG("file 3m loaded");

			// close the rom for now
			fclose(fp);

			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false; // true;

		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.areRomsLoaded = false;
			RETURN false;
		}

		if (ROM_TYPE == ROM::MS_PAC_MAN)
		{
			if (!setupEmulatorForMsPacMan(rom, &totalAuxilaryRomSize))
			{
				RETURN false;
			}
		}

		pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize = totalRomSize;

		// Fill "SPARE" with 0xFF so as to make debugging easier...

		for (uint32_t ii = 0; ii < sizeof(pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.SPARE); ii++)
		{
			pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.SPARE[ii] = 0xFF;
		}

		for (uint32_t ii = 0; ii < sizeof(pPacMan_memory->pacManMemoryMap.audioANDSpriteCoordinatesMemoryMap.audioANDSpriteCoordinates.SPARE); ii++)
		{
			pPacMan_memory->pacManMemoryMap.audioANDSpriteCoordinatesMemoryMap.audioANDSpriteCoordinates.SPARE[ii] = 0xFF;
		}
	}

	RETURN true;
}

void pacMan_t::dumpRom()
{
	uint32_t scanner = 0;
	uint32_t addressField = 0x10;

	LOG("Code ROM DUMP");
	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (int ii = 0; ii < (int)pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.codeRomSize; ii++)
	{
		LOG("0x%02x\t", pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	LOG_NEW_LINE;
	scanner = 0;
	addressField = 0x10;

	LOG("Color ROM DUMP");
	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (int ii = 0; ii < (int)pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.colorRomSize; ii++)
	{
		LOG("0x%02x\t", pPacMan_instance->pacMan_state.color_rom.colorROMMemory[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	LOG_NEW_LINE;
	scanner = 0;
	addressField = 0x10;

	LOG("Palette ROM DUMP");
	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (int ii = 0; ii < (int)pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.paletteRomSize; ii++)
	{
		LOG("0x%02x\t", pPacMan_instance->pacMan_state.palette_rom.paletteROMMemory[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	LOG_NEW_LINE;
	scanner = 0;
	addressField = 0x10;

	LOG("Tile ROM DUMP");
	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (int ii = 0; ii < (int)pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.tileRomSize; ii++)
	{
		LOG("0x%02x\t", pPacMan_instance->pacMan_state.tile_rom.tileROMMemory[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	LOG_NEW_LINE;
	scanner = 0;
	addressField = 0x10;

	LOG("Sprite ROM DUMP");
	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (int ii = 0; ii < (int)pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.spriteRomSize; ii++)
	{
		LOG("0x%02x\t", pPacMan_instance->pacMan_state.sprite_rom.spriteROMMemory[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	LOG_NEW_LINE;
	scanner = 0;
	addressField = 0x10;

	LOG("Sound ROM 1 DUMP");
	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (int ii = 0; ii < (int)pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.soundRom1Size; ii++)
	{
		LOG("0x%02x\t", pPacMan_instance->pacMan_state.sound_rom.soundROM.sound_rom1[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	LOG_NEW_LINE;
	scanner = 0;
	addressField = 0x10;

	LOG("Sound ROM 2 DUMP");
	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (int ii = 0; ii < (int)pAbsolute_pacMan_instance->absolute_pacMan_state.aboutRom.soundRom2Size; ii++)
	{
		LOG("0x%02x\t", pPacMan_instance->pacMan_state.sound_rom.soundROM.sound_rom2[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	if (ROM_TYPE == ROM::MS_PAC_MAN)
	{
		// Auxillary ROMs

		LOG_NEW_LINE;

		LOG("Decrypted Auxillary Code ROM U5 DUMP");
		LOG("Address\t\t");
		for (int ii = 0; ii < 0x10; ii++)
		{
			LOG("%02x\t", ii);
		}
		LOG_NEW_LINE;
		LOG("00000000\t");
		for (int ii = 0; ii < sizeof(pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u5); ii++)
		{
			LOG("0x%02x\t", pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x8000 + ii]);
			if (++scanner == 0x10)
			{
				scanner = 0;
				LOG_NEW_LINE;
				LOG("%08x\t", addressField);
				addressField += 0x10;
			}
		}

		LOG_NEW_LINE;

		LOG("Decrypted Auxillary Code ROM U6 DUMP");
		LOG("Address\t\t");
		for (int ii = 0; ii < 0x10; ii++)
		{
			LOG("%02x\t", ii);
		}
		LOG_NEW_LINE;
		LOG("00000000\t");
		for (int ii = 0; ii < sizeof(pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u6); ii++)
		{
			LOG("0x%02x\t", pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0x9000 + ii]);
			if (++scanner == 0x10)
			{
				scanner = 0;
				LOG_NEW_LINE;
				LOG("%08x\t", addressField);
				addressField += 0x10;
			}
		}

		LOG_NEW_LINE;

		LOG("Decrypted Auxillary Code ROM U7 DUMP");
		LOG("Address\t\t");
		for (int ii = 0; ii < 0x10; ii++)
		{
			LOG("%02x\t", ii);
		}
		LOG_NEW_LINE;
		LOG("00000000\t");
		for (int ii = 0; ii < sizeof(pPacMan_memory->pacManMemoryMap.mAuxCodeRom.auxillaryCodeRomFields.auxCode_rom_u7); ii++)
		{
			LOG("0x%02x\t", pPacMan_instance->pacMan_state.pacManMemory.pacManRawMemory[0xB000 + ii]);
			if (++scanner == 0x10)
			{
				scanner = 0;
				LOG_NEW_LINE;
				LOG("%08x\t", addressField);
				addressField += 0x10;
			}
		}
	}

	LOG_NEW_LINE;
}