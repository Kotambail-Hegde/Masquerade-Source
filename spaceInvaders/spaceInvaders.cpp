#pragma region SPACEINVADERS_SPECIFIC_INCLUDES
#include "spaceInvaders.h"
#pragma endregion SPACEINVADERS_SPECIFIC_INCLUDES

#pragma region SPACEINVADERS_SPECIFIC_MACROS
#define VRAM_START_ADDRESS 0x2400
#define VRAM_END_ADDRESS 0x3FFF
#pragma endregion SPACEINVADERS_SPECIFIC_MACROS

#pragma region CONDITIONAL_INCLUDES
#if (I8080_IN_SST_MODE == YES)
#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#endif
#pragma endregion CONDITIONAL_INCLUDES

#pragma region SPACEINVADERS_SPECIFIC_DECLARATIONS
// CPU cycles per single space invaders frame
static const uint32_t CPU_CYCLES_PER_FRAME = 16666 * TWO;	// 2 MHz

#if (I8080_IN_SST_MODE == YES)
static std::string _JSON_LOCATION;
static boost::property_tree::ptree testCase;
#endif

static uint32_t spaceInvaders_texture;
static uint32_t matrix_texture;
static uint32_t matrix[16] = { 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
#pragma endregion SPACEINVADERS_SPECIFIC_DECLARATIONS

spaceInvaders_t::spaceInvaders_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config)
{
	isBiosEnabled = NO;
	INC8 indexToCheck = RESET;

	setEmulationID(EMULATION_ID::SPACE_INVADERS_ID);

	this->pt = config;

#if (I8080_IN_SST_MODE == YES)
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_WARN);
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_EVENT);

	INFO("Running in sst Cpu Test Mode!");
	_JSON_LOCATION = pt.get<std::string>("spaceinvaders._sst_location");

	ROM_TYPE = ROM::TEST_SST;
#else
	if (nFiles == TEST_ROMS)
	{
		++indexToCheck;
		ROM_TYPE = ROM::TEST_ROM_COM;
	}
	else if (nFiles == SI_ROMS)
	{
		ROM_TYPE = ROM::SPACE_INVADERS;
	}

#ifndef __EMSCRIPTEN__
	_SAVE_LOCATION = pt.get<std::string>("spaceinvaders._save_location");
#else
	_SAVE_LOCATION = "assets/saves";
#endif
	_TEST_NUMBER = pt.get<std::int32_t>("spaceinvaders._test_to_run");

	// check if directory mentioned by "_SAVE_LOCATION" exists, if not we need to explicitly create it
	ifNoDirectoryThenCreate(_SAVE_LOCATION);

	for (INC8 ii = RESET; ii < (MAX_NUMBER_ROMS_FOR_SI - indexToCheck); ii++)
	{
		this->rom[ii] = rom[ii + indexToCheck];
	}
#endif
}

void spaceInvaders_t::setupTheCoreOfEmulation(void* masqueradeInstance, void* audio, void* network)
{
	INC8 indexToCheck = RESET;

	if (ROM_TYPE == ROM::SPACE_INVADERS)
	{
		indexToCheck = SI_ROMS - ONE;
	}
	else if (ROM_TYPE == ROM::TEST_ROM_COM)
	{
		indexToCheck = TEST_ROMS - ONE;
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
		}
	}
}

uint32_t spaceInvaders_t::getScreenWidth()
{
	RETURN this->screen_width;
}

uint32_t spaceInvaders_t::getScreenHeight()
{
	RETURN this->screen_height;
}

uint32_t spaceInvaders_t::getPixelWidth()
{
	RETURN this->pixel_width;
}

uint32_t spaceInvaders_t::getPixelHeight()
{
	RETURN this->pixel_height;
}

uint32_t spaceInvaders_t::getTotalScreenWidth()
{
	RETURN this->screen_width;
}

uint32_t spaceInvaders_t::getTotalScreenHeight()
{
	RETURN this->screen_height;
}

uint32_t spaceInvaders_t::getTotalPixelWidth()
{
	RETURN this->pixel_width;
}

uint32_t spaceInvaders_t::getTotalPixelHeight()
{
	RETURN this->pixel_height;
}

FLAG spaceInvaders_t::runEmulationAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

FLAG spaceInvaders_t::runEmulationLoopAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

FLAG spaceInvaders_t::runEmulationAtFixedRate(uint32_t currentFrame)
{
	bool status = true;

	loadQuirks();

	captureIO();

	displayCompleteScreen();

	RETURN status;
}

FLAG spaceInvaders_t::runEmulationLoopAtFixedRate(uint32_t currentFrame)
{
	bool vblank = false;

#if (I8080_IN_SST_MODE == YES)
#define SST_DEBUG_PRINT (NO)
	COUNTER32 opcode = ZERO;
	while (FOREVER)
	{
		if (opcode > 0xFF)
		{
			INFO("Completed Running all Tom Harte i8080 (v1) tests");
			BREAK;
		}

		// Get the input
		std::string testCaseName = std::format("{:02x}", opcode);
		testCaseName = _JSON_LOCATION + "\\" + testCaseName + ".json";

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
#if (SST_DEBUG_PRINT == YES)
			std::cout << "Name: " << name << std::endl;
#endif

			CPUTODO("Implement once the tests are available")
		}

		++opcode;
	}
#else
	if (ROM_TYPE == ROM::TEST_ROM_COM)
	{
		LOG("Starting the test");
		pSi_registers->pc = 0x0100;

		// NOTE: Works only for 8080EXM.COM
		// TODO: Add check for 8080EXM.COM
#if ENABLED
		uint32_t singleTestAddress = 0;

		if (_TEST_NUMBER != INVALID)
		{
			singleTestAddress = 0x100 + 58 + (_TEST_NUMBER * 2);
			pSi_instance->si_state.memory.rawMemory[(0x100 + 58)] =
				pSi_instance->si_state.memory.rawMemory[singleTestAddress];
			pSi_instance->si_state.memory.rawMemory[(0x100 + 58 + 1)] =
				pSi_instance->si_state.memory.rawMemory[singleTestAddress + 1];

			pSi_instance->si_state.memory.rawMemory[(0x100 + 58 + 2)] = 0x00;
			pSi_instance->si_state.memory.rawMemory[(0x100 + 58 + 3)] = 0x00;
		}
#endif

		pSi_instance->si_state.memory.rawMemory[0x0000] = 0xD3;
		pSi_instance->si_state.memory.rawMemory[0x0001] = 0x00;
		pSi_instance->si_state.memory.rawMemory[0x0005] = 0xD3;
		pSi_instance->si_state.memory.rawMemory[0x0006] = 0x01;
		pSi_instance->si_state.memory.rawMemory[0x0007] = 0xC9;

#if (LOGGER_ENABLED == true)
		std::ofstream outfile;
		outfile.open("undertest.txt", std::ios_base::app);
#endif

		while (!testFinished)
		{
			++testROMCycles;
			pSi_cpuInstance->opcode = pSi_instance->si_state.memory.rawMemory[pSi_registers->pc];

#if (LOGGER_ENABLED == true)
			if (testROMCycles > 0)
			{
				outfile << "--------------";
				outfile << "\n";
				outfile << testROMCycles;
				outfile << "\n";
				outfile << "a=";
				outfile << (unsigned int)cpuReadRegister(REGISTER_TYPE::RT_A);
				outfile << "\n";
				outfile << "b=";
				outfile << (unsigned int)cpuReadRegister(REGISTER_TYPE::RT_B);
				outfile << "\n";
				outfile << "c=";
				outfile << (unsigned int)cpuReadRegister(REGISTER_TYPE::RT_C);
				outfile << "\n";
				outfile << "d=";
				outfile << (unsigned int)cpuReadRegister(REGISTER_TYPE::RT_D);
				outfile << "\n";
				outfile << "e=";
				outfile << (unsigned int)cpuReadRegister(REGISTER_TYPE::RT_E);
				outfile << "\n";
				outfile << "h=";
				outfile << (unsigned int)cpuReadRegister(REGISTER_TYPE::RT_H);
				outfile << "\n";
				outfile << "l=";
				outfile << (unsigned int)cpuReadRegister(REGISTER_TYPE::RT_L);
				outfile << "\n";
				outfile << "pc=";
				outfile << (unsigned int)cpuReadRegister(REGISTER_TYPE::RT_PC);
				outfile << "\n";
				outfile << "sp=";
				outfile << (unsigned int)cpuReadRegister(REGISTER_TYPE::RT_SP);
				outfile << "\n";
				outfile << "sf=";
				outfile << (unsigned int)(pSi_flags->FSIGN);
				outfile << "\n";
				outfile << "zf=";
				outfile << (unsigned int)(pSi_flags->FZERO);
				outfile << "\n";
				outfile << "hf=";
				outfile << (unsigned int)(pSi_flags->FAuxCARRY);
				outfile << "\n";
				outfile << "pf=";
				outfile << (unsigned int)(pSi_flags->FPARITY);
				outfile << "\n";
				outfile << "cf=";
				outfile << (unsigned int)(pSi_flags->FCARRY);
				outfile << "\n";
				outfile << "opcode=";
				outfile << (unsigned int)(pSi_cpuInstance->opcode);
				outfile << "\n";
				outfile << "--------------";
			}
#endif

			performOperation(pSi_cpuInstance->opcode);
		}

		LOG("Run: %" PRId64, testROMCycles);
		LOG("Test complete");
#if (LOGGER_ENABLED == true)
		outfile.close();
#endif

		delete this;
		exit(0);
	}
	else
	{
		vblank = processCPU();
	}
#endif

	RETURN vblank;
}

const char* spaceInvaders_t::getEmulatorName()
{
	RETURN this->NAME;
}

float spaceInvaders_t::getEmulationFPS()
{
	RETURN this->myFPS;
}

void spaceInvaders_t::setEmulationID(EMULATION_ID ID)
{
	myID = ID;
}

EMULATION_ID spaceInvaders_t::getEmulationID()
{
	RETURN myID;
}

void spaceInvaders_t::cpuTickT()
{
	// Mid-Screen and VBlank is based on CPU T cycles, hence we can assume 1 cpu cycle = 4 ppu cycles
	pSi_cpuInstance->cpuCounter += ONE;
}

void spaceInvaders_t::cpuSetRegister(REGISTER_TYPE rt, uint16_t u16parameter)
{
	switch (rt) {

	case REGISTER_TYPE::RT_M_HL: { pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = u16parameter & 0x00FF; break; }
	case REGISTER_TYPE::RT_M_DE: { pSi_memory->rawMemory[pSi_registers->de.de_u16memory] = u16parameter & 0x00FF; break; }
	case REGISTER_TYPE::RT_M_BC: { pSi_memory->rawMemory[pSi_registers->bc.bc_u16memory] = u16parameter & 0x00FF; break; }

	case REGISTER_TYPE::RT_A: { pSi_registers->af.aAndFRegisters.a = u16parameter & 0x00FF; break; }
	case REGISTER_TYPE::RT_F: { pSi_registers->af.aAndFRegisters.f.flagMemory = u16parameter & 0x00FF; break; }
	case REGISTER_TYPE::RT_B: { pSi_registers->bc.bAndCRegisters.b = u16parameter & 0x00FF; break; }
	case REGISTER_TYPE::RT_C: { pSi_registers->bc.bAndCRegisters.c = u16parameter & 0x00FF; break; }
	case REGISTER_TYPE::RT_D: { pSi_registers->de.dAndERegisters.d = u16parameter & 0x00FF; break; }
	case REGISTER_TYPE::RT_E: { pSi_registers->de.dAndERegisters.e = u16parameter & 0x00FF; break; }
	case REGISTER_TYPE::RT_H: { pSi_registers->hl.hAndLRegisters.h = u16parameter & 0x00FF; break; }
	case REGISTER_TYPE::RT_L: { pSi_registers->hl.hAndLRegisters.l = u16parameter & 0x00FF; break; }

	case REGISTER_TYPE::RT_PC: { pSi_registers->pc = u16parameter & 0xFFFF; break; }
	case REGISTER_TYPE::RT_SP: { pSi_registers->sp = u16parameter & 0xFFFF; break; }

	case REGISTER_TYPE::RT_AF: { pSi_registers->af.af_u16memory = u16parameter & 0xFFFF; break; }
	case REGISTER_TYPE::RT_BC: { pSi_registers->bc.bc_u16memory = u16parameter & 0xFFFF; break; }
	case REGISTER_TYPE::RT_DE: { pSi_registers->de.de_u16memory = u16parameter & 0xFFFF; break; }
	case REGISTER_TYPE::RT_HL: { pSi_registers->hl.hl_u16memory = u16parameter & 0xFFFF; break; }

	case REGISTER_TYPE::RT_NONE: { break; }
	default: { break; }
	}
}

uint16_t spaceInvaders_t::cpuReadRegister(REGISTER_TYPE rt) 
{
	switch (rt) {

	case REGISTER_TYPE::RT_M_HL: { RETURN (pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] & 0x00FF); break; }
	case REGISTER_TYPE::RT_M_DE: { RETURN (pSi_memory->rawMemory[pSi_registers->de.de_u16memory] & 0x00FF); break; }
	case REGISTER_TYPE::RT_M_BC: { RETURN (pSi_memory->rawMemory[pSi_registers->bc.bc_u16memory] & 0x00FF); break; }

	case REGISTER_TYPE::RT_A: { RETURN (pSi_registers->af.aAndFRegisters.a & 0x00FF); break; }
	case REGISTER_TYPE::RT_F: { RETURN (pSi_registers->af.aAndFRegisters.f.flagMemory & 0x00FF); break; }
	case REGISTER_TYPE::RT_B: { RETURN (pSi_registers->bc.bAndCRegisters.b & 0x00FF); break; }
	case REGISTER_TYPE::RT_C: { RETURN (pSi_registers->bc.bAndCRegisters.c & 0x00FF); break; }
	case REGISTER_TYPE::RT_D: { RETURN (pSi_registers->de.dAndERegisters.d & 0x00FF); break; }
	case REGISTER_TYPE::RT_E: { RETURN (pSi_registers->de.dAndERegisters.e & 0x00FF); break; }
	case REGISTER_TYPE::RT_H: { RETURN (pSi_registers->hl.hAndLRegisters.h & 0x00FF); break; }
	case REGISTER_TYPE::RT_L: { RETURN (pSi_registers->hl.hAndLRegisters.l & 0x00FF); break; }

	case REGISTER_TYPE::RT_PC: { RETURN (pSi_registers->pc & 0xFFFF); break; }
	case REGISTER_TYPE::RT_SP: { RETURN (pSi_registers->sp & 0xFFFF); break; }

	case REGISTER_TYPE::RT_AF: { RETURN (pSi_registers->af.af_u16memory & 0xFFFF); break; }
	case REGISTER_TYPE::RT_BC: { RETURN (pSi_registers->bc.bc_u16memory & 0xFFFF); break; }
	case REGISTER_TYPE::RT_DE: { RETURN (pSi_registers->de.de_u16memory & 0xFFFF); break; }
	case REGISTER_TYPE::RT_HL: { RETURN (pSi_registers->hl.hl_u16memory & 0xFFFF); break; }

	case REGISTER_TYPE::RT_NONE: { RETURN ZERO;  break; }
	default: { RETURN ZERO; break; }
	}
}

void spaceInvaders_t::process_SZP(BYTE value)
{
	if (value & 0x80)
	{
		pSi_flags->FSIGN = 1;
	}
	else
	{
		pSi_flags->FSIGN = 0;
	}

	if (value == 0)
	{
		pSi_flags->FZERO = 1;
	}
	else
	{
		pSi_flags->FZERO = 0;
	}

	if (parityLUT[value])
	{
		pSi_flags->FPARITY = 1;
	}
	else
	{
		pSi_flags->FPARITY = 0;
	}
}

void spaceInvaders_t::process_AC(BYTE value1, BYTE value2)
{
	if (((value1 & 0x0F) + (value2 & 0x0F)) > 0x0F)
	{
		pSi_flags->FAuxCARRY = 1;
	}
	else
	{
		pSi_flags->FAuxCARRY = 0;
	}
}

void spaceInvaders_t::process_AC_withCarry(BYTE value1, BYTE value2)
{
	if (((value1 & 0x0F) + (value2 & 0x0F)) >= 0x0F)
	{
		pSi_flags->FAuxCARRY = 1;
	}
	else
	{
		pSi_flags->FAuxCARRY = 0;
	}
}

void spaceInvaders_t::process_AB(BYTE value1, BYTE value2)
{
	if ((value2 & 0x0F) <= (value1 & 0x0F))
	{
		pSi_flags->FAuxCARRY = 1;
	}
	else
	{
		pSi_flags->FAuxCARRY = 0;
	}
}

void spaceInvaders_t::process_AB_withBorrow(BYTE value1, BYTE value2)
{
	if ((value2 & 0x0F) < (value1 & 0x0F))
	{
		pSi_flags->FAuxCARRY = 1;
	}
	else
	{
		pSi_flags->FAuxCARRY = 0;
	}
}

void spaceInvaders_t::stackPush(BYTE data)
{
	(pSi_registers->sp)--;
	pSi_instance->si_state.memory.rawMemory[pSi_registers->sp] = data;
}

BYTE spaceInvaders_t::stackPop()
{
	uint8_t popedData =  pSi_instance->si_state.memory.rawMemory[pSi_registers->sp];
	(pSi_registers->sp)++;
	RETURN popedData;
}

void spaceInvaders_t::writeRawMemory(uint16_t address, BYTE data)
{
	pSi_memory->rawMemory[address] = data;
}

BYTE spaceInvaders_t::readRawMemory(uint16_t address)
{
	RETURN pSi_memory->rawMemory[address];
}

FLAG spaceInvaders_t::processDisplay(uint32_t vramAddress)
{
	// columns = 224
	// rows = 256 
	// vram start = 0x2400
	// vram size in bits = 8 * 0x1C00

	// 2400 - 3FFF(1C00 bytes = 256 * 28) 28 * 8 = 224. Screen is 256x224 pixels.
	// The map below shows the raster layout. Take this map and rotate it counter clockwise once. Thus the first byte is lower left.First "row" ends upper left. Last byte is upper right.
	//     2400     2401     2402        ....   241F
	//     01234567 01234567 01234567    ....   01234567
	// 
	//     2420     2421     2422        ....   243F
	//     01234567 01234567 01234567    ....   01234567
	//    
	//     .                                    .
	//     .                                    .
	//     .                                    .
	//     .                                    .
	// 
	//     3FE0     3FE1     3FE2        ....   3FFF
	//     01234567 01234567 01234567    ....   01234567

	// gfx mapping
	// 1D to 2D mapping (below numbers indicate gfx array index)
	//	
	// n 2n  3n   ..    N
	// :  :   :			:
	// :  :   :			:
	// 1 n+2 2n+2	 (N-1)n+2
	// 0 n+1 2n+1 .. (N-1)n+1 

	// To get the column
	// Note: column needs to shift for every 0x20 starting from 0x2400
	// example 1, modified VRAM address is 0x2402
	// now, 0x2402  - vram start = 0x02
	// (uint32_t) 0x02 / 0x20 = 0 ... therefore, column number 0 
	// example 2, modified VRAM address is 0x2421
	// now, 0x2421  - vram start = 0x21
	// (uint32_t) 0x21 / 0x20 = 1 ... therefore, column number 1  
	// example 3, modified VRAM address is 0x3FE2
	// now, 0x3FE2  - vram start = 0x1BE2
	// (uint32_t) 0x1bE2 / 0x20 = 223 ... therefore, column number 223  

	// To get the row, we assume 2 kinds of row, MajRow and MinRow
	// Note: row needs to reset to 0 for every 0x20 starting from 0x2400
	// Since each bit is a row, the 0th bit of each byte becomes the MajRow
	// 1st to 7th bit becomes the MinRow
	// Number of MajRows is 256/8 = 32
	// Note: MajRow is encoded in least significant byte of the vram address
	// example 1, modified VRAM address is 0x2402
	// now, LSByte(0x2402) = 0x02 
	// 0x02 % 0x20 = 2
	// 31 - 2 = 29 ... therefore, MajRow number 29
	// example 2, modified VRAM address is 0x2421
	// now, LSByte(0x2421) = 0x21 
	// 0x21 % 0x20 = 1
	// 31 - 1 = 30 ... therefore, MajRow number 30
	// example 3, modified VRAM address is 0x3FE2
	// now, LSByte(0x3FFF) = 0xFF 
	// 0xFF % 0x20 = 0x1F
	// 31 - 31 = 1 ... therefore, MajRow number 1

	// For colour
	// ,_______________________________.
	// |WHITE            ^             |
	// |                32             |
	// |                 v             |
	// |-------------------------------|
	// |RED              ^             |
	// |                32             |
	// |                 v             |
	// |-------------------------------|
	// |WHITE                          |
	// |         < 224 >               |
	// |                               |
	// |                 ^             |
	// |                120            |
	// |                 v             |
	// |                               |
	// |                               |
	// |                               |
	// |-------------------------------|
	// |GREEN                          |
	// | ^                  ^          |
	// |56        ^        56          |
	// | v       72         v          |
	// |____      v      ______________|
	// |  ^  |          | ^            |
	// |<16> |  < 118 > |16   < 122 >  |
	// |  v  |          | v            |
	// |WHITE|          |         WHITE|
	// `-------------------------------'

	BYTE modifiedVRAMData = 0;
	uint8_t BITi = 0;
	uint8_t BITn = 7;
	uint8_t NEXT_LINE = 0x20;
	uint8_t INV_ROW = 31;
	uint8_t vramToColumn = (uint8_t)(((vramAddress)-VRAM_START_ADDRESS) / NEXT_LINE);
	uint8_t vramToRow = (uint8_t)(INV_ROW - (((vramAddress) & 0x00FF) % NEXT_LINE));
	uint8_t pixel = 0;

	modifiedVRAMData = pSi_instance->si_state.memory.rawMemory[vramAddress];

	for (BITi = 0; BITi <= BITn; BITi++)
	{
		pixel = (vramToColumn * getScreenHeight()) + (((INV_ROW - vramToRow) * 8) + (BITn - BITi));	//TODO: inversion of Byte performed here BITi -> (BITn - BITi). Figure out the reason...
		if (getBit(modifiedVRAMData, (BITn - BITi)))	//TODO: inversion of Byte performed here BITi -> (BITn - BITi). Figure out the reason...		
		{
			Pixel colorN = WHITE;	

			if ((vramToRow <= 3) || (vramToRow >= 8 && vramToRow <= 22))
			{
				colorN = WHITE;
			}
			else if (vramToRow <= 7)
			{
				colorN = DARK_RED;
			}
			else if (vramToRow >= 23 && vramToRow <= 29)
			{
				colorN = GREEN;
			}
			else
			{
				if (vramToColumn <= 15 || vramToColumn >= 134)
				{
					colorN = WHITE;
				}
				else
				{
					colorN = GREEN;
				}
			}

			pSi_instance->si_state.display.imGuiBuffer.imGuiBuffer2D[(vramToRow * 8) + (BITi)][vramToColumn] = colorN;	// TODO: Have to figure out if the proper index is filled or not within gfx buffer
		}
		else
		{
			pSi_instance->si_state.display.imGuiBuffer.imGuiBuffer2D[(vramToRow * 8) + (BITi)][vramToColumn] = BLACK;	// TODO: Have to figure out if the proper index is filled or not within gfx buffer
		}
	}

	RETURN true;
}

FLAG spaceInvaders_t::processInterrupts()
{
	// if interrupt is enabled
	if (pSi_registers->ie == INTERRUPT_ENABLED)
	{
		// disabled the interrupt

		pSi_registers->ie = INTERRUPT_DISABLED;

		// now set the pc the interrupt handler
		// Note: this is similar to "RST interrupt_number" instruction.

		// RST 0H -> 0x0000 to 0x0007
		// RST 1H -> 0x0008 to 0x000F
		// :
		// RST 7H -> 0x0038 to 0x003F

		// push the pc to stack

		if (pSi_instance->si_state.currentInterrupt == INTERRUPTS::INTERRUPT_1)
		{
			// RST (CALL $08) opcode

			uint16_t operationResult = 0x0008;
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			stackPush(lowerData);
			pSi_registers->pc = operationResult;

			CPUTODO("11 cpu cycles needs to be split properly across different stages of interrupt");
			RUN_FOR_(ELEVEN, cpuTickT());

			RETURN YES;
		}
		else if (pSi_instance->si_state.currentInterrupt == INTERRUPTS::INTERRUPT_2)
		{
			// RST (CALL $10) opcode

			uint16_t operationResult = 0x0010;
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			stackPush(lowerData);
			pSi_registers->pc = operationResult;

			CPUTODO("11 cpu cycles needs to be split properly across different stages of interrupt");
			RUN_FOR_(ELEVEN, cpuTickT());

			RETURN YES;
		}
		else
		{
			FATAL("Unhandled SI Interrupt");
		}
	}

	RETURN NO;
}

// Wrapper on top of everything thing that the CPU Core does
FLAG spaceInvaders_t::processCPU()
{
	FLAG vblank = NO;

	const uint32_t halfFrameCycles = CPU_CYCLES_PER_FRAME / TWO;

	if (pSi_cpuInstance->cpuCounter < halfFrameCycles)
	{
		processOpcode();
	}
	else
	{
		pSi_cpuInstance->cpuCounter -= halfFrameCycles;

		if (processInterrupts() == NO)
		{
			processOpcode();
		}

		vblank = (pSi_instance->si_state.currentInterrupt != INTERRUPTS::INTERRUPT_1);
		pSi_instance->si_state.currentInterrupt = (pSi_instance->si_state.currentInterrupt == INTERRUPTS::INTERRUPT_1) ? INTERRUPTS::INTERRUPT_2 : INTERRUPTS::INTERRUPT_1;
	}

	RETURN vblank;
}

FLAG spaceInvaders_t::processOpcode()
{
	bool status = false;

	if (pSi_registers->pc >= pSi_registers->sp)
	{
		LOG_NEW_LINE;
		LOG("--------------------------------------------");
		LOG("emulated cpu cycle: \t%" PRId64, pSi_cpuInstance->cpuCounter);
		LOG("executed opcode: \t%02x", pSi_cpuInstance->opcode);
		LOG("a register: \t\t%02x", pSi_instance->si_state.registers.af.aAndFRegisters.a);

		LOG("f register: \t\t%02x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagMemory);
		LOG("FSIGN: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FSIGN);
		LOG("FZERO: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FZERO);
		LOG("ZERO: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.ZERO1);
		LOG("FAuxCARRY: \t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FAuxCARRY);
		LOG("ZERO: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.ZERO2);
		LOG("FPARITY: \t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FPARITY);
		LOG("ONE: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.ONE1);
		LOG("FCARRY: \t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FCARRY);

		LOG("b register: \t\t%02x", pSi_instance->si_state.registers.bc.bAndCRegisters.b);
		LOG("c register: \t\t%02x", pSi_instance->si_state.registers.bc.bAndCRegisters.c);
		LOG("d register: \t\t%02x", pSi_instance->si_state.registers.de.dAndERegisters.d);
		LOG("e register: \t\t%02x", pSi_instance->si_state.registers.de.dAndERegisters.e);
		LOG("h register: \t\t%02x", pSi_instance->si_state.registers.hl.hAndLRegisters.h);
		LOG("l register: \t\t%02x", pSi_instance->si_state.registers.hl.hAndLRegisters.l);
		LOG("program counter: \t%04x", pSi_instance->si_state.registers.pc);
		LOG("stack pointer: \t\t%04x", pSi_instance->si_state.registers.sp);
		LOG("interrupt Enable: \t%02x", pSi_instance->si_state.registers.ie);
		LOG("--------------------------------------------");

		FATAL("PC over SP; Stack Corruption");
	}
	
	status = performOperation();

	if (debugConfig._DEBUG_REGISTERS == true)
	{
		LOG_NEW_LINE;
		LOG("--------------------------------------------");
		LOG("emulated cpu cycle: \t%" PRId64, pSi_cpuInstance->cpuCounter);
		LOG("executed opcode: \t%02x", pSi_cpuInstance->opcode);
		LOG("to be executed opcode: \t%02x", pSi_instance->si_state.memory.rawMemory[pSi_registers->pc]);
		LOG("a register: \t\t%02x", pSi_instance->si_state.registers.af.aAndFRegisters.a);

		LOG("f register: \t\t%02x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagMemory);
		LOG("FSIGN: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FSIGN);
		LOG("FZERO: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FZERO);
		LOG("ZERO: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.ZERO1);
		LOG("FAuxCARRY: \t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FAuxCARRY);
		LOG("ZERO: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.ZERO2);
		LOG("FPARITY: \t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FPARITY);
		LOG("ONE: \t\t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.ONE1);
		LOG("FCARRY: \t\t%01x", pSi_instance->si_state.registers.af.aAndFRegisters.f.flagFields.FCARRY);

		LOG("b register: \t\t%02x", pSi_instance->si_state.registers.bc.bAndCRegisters.b);
		LOG("c register: \t\t%02x", pSi_instance->si_state.registers.bc.bAndCRegisters.c);
		LOG("d register: \t\t%02x", pSi_instance->si_state.registers.de.dAndERegisters.d);
		LOG("e register: \t\t%02x", pSi_instance->si_state.registers.de.dAndERegisters.e);
		LOG("h register: \t\t%02x", pSi_instance->si_state.registers.hl.hAndLRegisters.h);
		LOG("l register: \t\t%02x", pSi_instance->si_state.registers.hl.hAndLRegisters.l);
		LOG("program counter: \t%04x", pSi_instance->si_state.registers.pc);
		LOG("stack pointer: \t\t%04x", pSi_instance->si_state.registers.sp);
		LOG("interrupt Enable: \t%02x", pSi_instance->si_state.registers.ie);
		LOG("--------------------------------------------");
	}
	if (debugConfig._DEBUG_KEYPAD == true)
	{
		LOG_NEW_LINE;
		LOG("--------------------------------------------");
		LOG("cycle: \t%u", pSi_cpuInstance->cpuCounter);
		LOG("--------------------------------------------");
	}

	RETURN status;
}

void spaceInvaders_t::unimplementedInstruction()
{
	FATAL("CPU Panic; unknown opcode! %02X", pSi_cpuInstance->opcode);
}

void spaceInvaders_t::captureIO()
{
	loadQuirks();

	pSi_io->iPort0.iPort0Fields.ONE1 = 1;
	pSi_io->iPort0.iPort0Fields.ONE2 = 1;
	pSi_io->iPort0.iPort0Fields.ONE3 = 1;

	pSi_io->iPort1.iPort1Fields.CREDIT = (ImGui::IsKeyDown(ImGuiKey_Keypad5) == true ? 1 : 0);
	pSi_io->iPort1.iPort1Fields.P1START = (ImGui::IsKeyDown(ImGuiKey_Keypad1) == true ? 1 : 0);
	pSi_io->iPort1.iPort1Fields.P2START = (ImGui::IsKeyDown(ImGuiKey_Keypad2) == true ? 1 : 0);
	pSi_io->iPort1.iPort1Fields.ONE1 = 1;
	pSi_io->iPort1.iPort1Fields.P1SHOT = (ImGui::IsKeyDown(ImGuiKey_Space) == true ? 1 : 0);
	pSi_io->iPort1.iPort1Fields.P1LEFT = (ImGui::IsKeyDown(ImGuiKey_LeftArrow) == true ? 1 : 0);
	pSi_io->iPort1.iPort1Fields.P1RIGHT = (ImGui::IsKeyDown(ImGuiKey_RightArrow) == true ? 1 : 0);

	pSi_io->iPort2.iPort2Fields.DIP3 = (uint8_t)pSi_instance->si_state.quirks._DIP3;
	pSi_io->iPort2.iPort2Fields.DIP5 = (uint8_t)pSi_instance->si_state.quirks._DIP5;
	pSi_io->iPort2.iPort2Fields.TILT = (ImGui::IsKeyDown(ImGuiKey_T) == true ? 1 : 0);
	pSi_io->iPort2.iPort2Fields.DIP6 = (uint8_t)pSi_instance->si_state.quirks._DIP6;
	pSi_io->iPort2.iPort2Fields.P2SHOT = (ImGui::IsKeyDown(ImGuiKey_S) == true ? 1 : 0);
	pSi_io->iPort2.iPort2Fields.P2LEFT = (ImGui::IsKeyDown(ImGuiKey_Keypad4) == true ? 1 : 0);
	pSi_io->iPort2.iPort2Fields.P2RIGHT = (ImGui::IsKeyDown(ImGuiKey_Keypad6) == true ? 1 : 0);
	pSi_io->iPort2.iPort2Fields.DIP7 = (uint8_t)pSi_instance->si_state.quirks._DIP7;
}

void spaceInvaders_t::displayCompleteScreen()
{
	for (uint32_t vramAddress = VRAM_START_ADDRESS; vramAddress <= VRAM_END_ADDRESS; vramAddress++)
	{
		processDisplay(vramAddress);
	}

#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glDisable(GL_BLEND);

	// Handle for system's texture

	glBindTexture(GL_TEXTURE_2D, spaceInvaders_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pSi_instance->si_state.display.imGuiBuffer.imGuiBuffer1D);

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
	// 1. Upload emulator framebuffer to spaceInvaders_texture
	GL_CALL(glBindTexture(GL_TEXTURE_2D, spaceInvaders_texture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
		(GLvoid*)pSi_instance->si_state.display.imGuiBuffer.imGuiBuffer1D));

	// Choose filtering mode (NEAREST or LINEAR)
	GLint filter = (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER) ? GL_LINEAR : GL_NEAREST;
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));

	// 2. Render spaceInvaders_texture into framebuffer (masquerade_texture target)
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
	GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// Pass 1: Render base texture (Game Boy framebuffer)
	GL_CALL(glUseProgram(shaderProgramBasic));
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, spaceInvaders_texture));
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

void spaceInvaders_t::processTimers()
{
	;
}

float spaceInvaders_t::getEmulationVolume()
{
	pSi_instance->si_state.audio.emulatorVolume = SDL_GetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream));
	RETURN pSi_instance->si_state.audio.emulatorVolume;
}

void spaceInvaders_t::setEmulationVolume(float volume)
{
	pSi_instance->si_state.audio.emulatorVolume = volume;
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), volume);
	pt.put("spaceinvaders._volume", volume);
	boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, pt);
}

FLAG spaceInvaders_t::initializeEmulator()
{
	// create an instance

	pAbsolute_si_instance = std::make_shared<absolute_si_instance_t>();

	// for readability

	pSi_instance = &(pAbsolute_si_instance->absolute_si_state.si_instance);
	pSi_registers = &(pSi_instance->si_state.registers);
	pSi_cpuInstance = &(pSi_instance->si_state.cpuInstance);
	pSi_memory = &(pSi_instance->si_state.memory);
	pSi_flags = &(pSi_registers->af.aAndFRegisters.f.flagFields);
	pSi_io = &(pSi_instance->si_state.io);

	// initialize the graphics and bitmap

	uint32_t scanner = 0;
	for (uint32_t x = 0; x < getScreenWidth(); x++)
	{
		for (uint32_t y = (getScreenHeight() - 1); y >= 0; y--)
		{
			// For Buffer Per Frame mode
			pSi_instance->si_state.display.imGuiBuffer.imGuiBuffer1D[scanner++] = BLACK;
		}
	}

	// other initializations

	pSi_cpuInstance->cpuCounter = RESET;

	pSi_registers->pc = 0x0000;

	pSi_io->oPort2 = 0;
	pSi_io->shiftRegister = 0;

	pSi_instance->si_state.currentInterrupt = INTERRUPTS::INTERRUPT_1;

	// topmost address of RAM, as stack is of the decrementing type

	pSi_registers->sp = 0x2400;

#if 1
	// space invaders assumes 0xFF to be the default value of VRAM

	memset(
		pSi_instance->si_state.memory.memoryMap.mVram,
		0xFF,
		sizeof(pSi_instance->si_state.memory.memoryMap.mVram)
	);
#endif

#if DEACTIVATED
	for (uint32_t ii = 0; ii < sizeof(pSi_instance->si_state.memory.memoryMap.mVram); ii++)
	{
		pSi_instance->si_state.memory.memoryMap.mVram[ii] = 0x6E;
	}
#endif

	if (isCLI() == NO)
	{
		SDL_InitSubSystem(SDL_INIT_AUDIO);
		SDL_AudioSpec AudioSettings{ SDL_AUDIO_F32, ONE, TO_UINT(EMULATED_AUDIO_SAMPLING_RATE_FOR_SPACEINVADERS) };

#if (SPACE_INVADERS_AUDIO_AS_STATIC_BUFFERS == NO)
		SDL_LoadWAV((pt.get<std::string>("spaceinvaders._ufo")).c_str(), &AudioSettings, &UFO, &UFO_length);
		SDL_LoadWAV((pt.get<std::string>("spaceinvaders._shot")).c_str(), &AudioSettings, &Shot, &Shot_length);
		SDL_LoadWAV((pt.get<std::string>("spaceinvaders._player_dies")).c_str(), &AudioSettings, &PlayerDies, &PlayerDies_length);
		SDL_LoadWAV((pt.get<std::string>("spaceinvaders._invader_dies")).c_str(), &AudioSettings, &InvaderDies, &InvaderDies_length);
		SDL_LoadWAV((pt.get<std::string>("spaceinvaders._fleet_movement_1")).c_str(), &AudioSettings, &FleetMovement1, &FleetMovement1_length);
		SDL_LoadWAV((pt.get<std::string>("spaceinvaders._fleet_movement_2")).c_str(), &AudioSettings, &FleetMovement2, &FleetMovement2_length);
		SDL_LoadWAV((pt.get<std::string>("spaceinvaders._fleet_movement_3")).c_str(), &AudioSettings, &FleetMovement3, &FleetMovement3_length);
		SDL_LoadWAV((pt.get<std::string>("spaceinvaders._fleet_movement_4")).c_str(), &AudioSettings, &FleetMovement4, &FleetMovement4_length);
		SDL_LoadWAV((pt.get<std::string>("spaceinvaders._ufo_hit")).c_str(), &AudioSettings, &UFOHit, &UFOHit_length);
#endif

		audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &AudioSettings, NULL, NULL);
		SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));

		pSi_instance->si_state.audio.emulatorVolume = pt.get<std::float_t>("spaceinvaders._volume");
		SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), pSi_instance->si_state.audio.emulatorVolume);

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

		glGenTextures(1, &spaceInvaders_texture);
		glBindTexture(GL_TEXTURE_2D, spaceInvaders_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pSi_instance->si_state.display.imGuiBuffer.imGuiBuffer1D);
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

		// 3. Space Invaders texture (used to upload emulated framebuffer)
		GL_CALL(glGenTextures(1, &spaceInvaders_texture));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, spaceInvaders_texture));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pSi_instance->si_state.display.imGuiBuffer.imGuiBuffer1D));
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

	RETURN true;
}

void spaceInvaders_t::destroyEmulator()
{
	pAbsolute_si_instance.reset();

	if (isCLI() == NO)
	{

#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
		glDeleteTextures(1, &spaceInvaders_texture);
		glDeleteTextures(1, &matrix_texture);
#else
		glDeleteTextures(1, &spaceInvaders_texture);
		glDeleteTextures(1, &matrix_texture);
#endif

#if (SPACE_INVADERS_AUDIO_AS_STATIC_BUFFERS == NO)
		SDL_free(UFO);
		SDL_free(Shot);
		SDL_free(PlayerDies);
		SDL_free(InvaderDies);
		SDL_free(FleetMovement1);
		SDL_free(FleetMovement2);
		SDL_free(FleetMovement3);
		SDL_free(FleetMovement4);
		SDL_free(UFOHit);
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

FLAG spaceInvaders_t::getRomLoadedStatus()
{
	RETURN pAbsolute_si_instance->absolute_si_state.aboutRom.isRomLoaded;
}

FLAG spaceInvaders_t::loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom)
{
	// open the rom file

	FILE* fp = NULL;
	uint32_t totalRomSize = 0;

	if (ROM_TYPE == ROM::TEST_ROM_COM)
	{
		// Test ROM

		errno_t err = fopen_portable(&fp, rom[ZERO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_si_instance->absolute_si_state.aboutRom.romSize = ftell(fp);
			totalRomSize += pAbsolute_si_instance->absolute_si_state.aboutRom.romSize;

			// read the complete rom
			rewind(fp);
			fread(&pSi_instance->si_state.memory.rawMemory[0x0100], pAbsolute_si_instance->absolute_si_state.aboutRom.romSize, 1, fp);
			LOG("test rom loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_si_instance->absolute_si_state.aboutRom.isRomLoaded = false;
		}
	}
	else
	{

		// file e

		errno_t err = fopen_portable(&fp, rom[ZERO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_si_instance->absolute_si_state.aboutRom.romSize = ftell(fp);
			totalRomSize += pAbsolute_si_instance->absolute_si_state.aboutRom.romSize;

			// read the complete rom
			rewind(fp);
			fread(pSi_instance->si_state.memory.memoryMap.mRom.romFields.rom_e, pAbsolute_si_instance->absolute_si_state.aboutRom.romSize, 1, fp);
			LOG("file e loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_si_instance->absolute_si_state.aboutRom.isRomLoaded = false;
		}

		// file f

		err = fopen_portable(&fp, rom[ONE].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_si_instance->absolute_si_state.aboutRom.romSize = ftell(fp);
			totalRomSize += pAbsolute_si_instance->absolute_si_state.aboutRom.romSize;

			// read the complete rom
			rewind(fp);
			fread(pSi_instance->si_state.memory.memoryMap.mRom.romFields.rom_f, pAbsolute_si_instance->absolute_si_state.aboutRom.romSize, 1, fp);
			LOG("file f loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_si_instance->absolute_si_state.aboutRom.isRomLoaded = false;
		}

		// file g

		err = fopen_portable(&fp, rom[TWO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_si_instance->absolute_si_state.aboutRom.romSize = ftell(fp);
			totalRomSize += pAbsolute_si_instance->absolute_si_state.aboutRom.romSize;

			// read the complete rom
			rewind(fp);
			fread(pSi_instance->si_state.memory.memoryMap.mRom.romFields.rom_g, pAbsolute_si_instance->absolute_si_state.aboutRom.romSize, 1, fp);
			LOG("file g loaded");

			// close the rom for now
			fclose(fp);
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_si_instance->absolute_si_state.aboutRom.isRomLoaded = false;
		}

		// file h

		err = fopen_portable(&fp, rom[THREE].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_si_instance->absolute_si_state.aboutRom.romSize = ftell(fp);
			totalRomSize += pAbsolute_si_instance->absolute_si_state.aboutRom.romSize;

			// read the complete rom
			rewind(fp);
			fread(pSi_instance->si_state.memory.memoryMap.mRom.romFields.rom_h, pAbsolute_si_instance->absolute_si_state.aboutRom.romSize, 1, fp);
			LOG("file h loaded");

			// close the rom for now
			fclose(fp);

			pAbsolute_si_instance->absolute_si_state.aboutRom.isRomLoaded = true;
		}
		else
		{
			LOG("Unable to load the rom file");
			pAbsolute_si_instance->absolute_si_state.aboutRom.isRomLoaded = false;
			RETURN false;
		}
	}

	pAbsolute_si_instance->absolute_si_state.aboutRom.romSize = totalRomSize;

	RETURN true;
}

void spaceInvaders_t::dumpRom()
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
	for (int ii = 0; ii < (int)pAbsolute_si_instance->absolute_si_state.aboutRom.romSize; ii++)
	{
		LOG("0x%02x\t", pSi_instance->si_state.memory.rawMemory[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}
}