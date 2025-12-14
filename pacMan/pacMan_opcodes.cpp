#include "pacMan.h"

#pragma region PACMAN_SPECIFIC_MACROS
#define VRAM_START_ADDRESS 0x4000
#define VRAM_END_ADDRESS 0x47FF
#define CHARACTER_VRAM_START_ADDRESS VRAM_START_ADDRESS
#define CHARACTER_VRAM_END_ADDRESS 0x43FF
#define CHARACTER_CRAM_START_ADDRESS 0x4400
#define CHARACTER_CRAM_END_ADDRESS VRAM_END_ADDRESS
#define FLIPX true
#define FLIPY true
#define FLIPBOTH true
#define MAX_COLUMN_TILES 28
#define MAX_ROW_TILES 36
#pragma endregion PACMAN_SPECIFIC_MACROS

void pacMan_t::incrementR()
{
	BYTE r = TO_UINT8(cpuReadRegister(REGISTER_TYPE::RT_R));
	BYTE operationResult = (BYTE)((r & TO_UINT8(0x80)) | (((r & TO_UINT8(0x7F)) + 1) & TO_UINT8(0x7F)));
	cpuSetRegister(REGISTER_TYPE::RT_R, operationResult);
}

void pacMan_t::updateXY(uint8_t opcode, uint8_t src)
{
	pPacMan_flags->THIRD = GETBIT(THREE, src);
	pPacMan_flags->FIFTH = GETBIT(FIVE, src);
}

// Should be called only from "processOpcode" or from "processInterrupts" (Except for test ROM)
FLAG pacMan_t::performOperation(int32_t anySpecificOpcode)
{
	auto INCREMENT_PC_BY_ONE = [&]()
		{
			pPacMan_registers->pc++;
		};

	FLAG status = true;

	pPacMan_cpuInstance->opcode = RESET;
	pPacMan_cpuInstance->prefix1 = RESET;
	pPacMan_cpuInstance->prefix2 = RESET;

	auto operationResult = 0;
	auto dataFromMemory = 0;
	auto lowerDataFromMemory = 0;
	auto higherDataFromMemory = 0;

	// Clear the P register at the start of new opcode
	cpuSetRegister(REGISTER_TYPE::RT_P, ZERO);

	// M1: opcode fetch
	cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();

	if (anySpecificOpcode != INVALID)
	{
		pPacMan_cpuInstance->opcode = (anySpecificOpcode & 0xFF);
	}
	else
	{
		pPacMan_cpuInstance->opcode = readRawMemoryFromCPU(pPacMan_registers->pc, YES);
	}

	incrementR();

	pPacMan_registers->pc++;

	pPacMan_cpuInstance->possFlag = NO;

	switch (pPacMan_cpuInstance->opcode)
	{
	case 0x00:
	{ // NOP (4T)
		BREAK;
	}

	case 0x01:
	{ // LD BC,nn (10T = 4+3+3)
		// M2: low byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_C, low);

		// M3: high byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_B, high);

		BREAK;
	}

	case 0x02:
	{ // LD (BC),A (7T = 4+3)
		// M2
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_BC),
			(BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		auto temp = (uint32_t)((cpuReadRegister(REGISTER_TYPE::RT_A) << EIGHT) | (BYTE)(cpuReadRegister(REGISTER_TYPE::RT_C) + ONE));
		cpuSetRegister(REGISTER_TYPE::RT_WZ, temp);

		BREAK;
	}

	case 0x03:
	{ // INC BC (6T = 4+2 internal)
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) + 1);
		BREAK;
	}

	case 0x04:
	{ // INC B (4T)
		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		BYTE res = old + 1;
		processFlagsFor8BitAdditionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_B, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x05:
	{ // DEC B (4T)
		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		BYTE res = old - 1;
		processFlagsFor8BitSubtractionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_B, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x06:
	{ // LD B,n (7T = 4+3)

		cpuTickT(); cpuTickT(); cpuTickT();
		int val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_B, val);
		BREAK;
	}

	case 0x07:
	{ // RLCA (4T)
		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		pPacMan_flags->FCARRY = (a & 0x80) ? 1 : 0;
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		BYTE res = (a << 1) | (a >> 7);
		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x08:
	{ // EX AF,AF' (4T)
		auto tmp = cpuReadRegister(REGISTER_TYPE::RT_AF);
		cpuSetRegister(REGISTER_TYPE::RT_AF, cpuReadRegister(REGISTER_TYPE::RT_SHADOW_AF));
		cpuSetRegister(REGISTER_TYPE::RT_SHADOW_AF, tmp);
		BREAK;
	}

	case 0x09:
	{ // ADD HL,BC (11T = 4+7)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t res = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL) +
			(uint32_t)cpuReadRegister(REGISTER_TYPE::RT_BC);
		processFlagsFor16BitAdditionOperation(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_BC),
			false, false);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)res);
		updateXY(pPacMan_cpuInstance->opcode, sizeof(res) > ONE ? res >> EIGHT : res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x0A:
	{ // LD A,(BC) (7T = 4+3)
		cpuTickT(); cpuTickT(); cpuTickT();
		int val = cpuReadPointer(POINTER_TYPE::RT_M_BC);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_BC) + ONE));
		BREAK;
	}

	case 0x0B:
	{ // DEC BC (6T = 4+2 internal)
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) - 1);
		BREAK;
	}

	case 0x0C:
	{ // INC C (4T)
		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		BYTE res = old + 1;
		processFlagsFor8BitAdditionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_C, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x0D:
	{ // DEC C (4T)
		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		BYTE res = old - 1;
		processFlagsFor8BitSubtractionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_C, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x0E:
	{ // LD C,n (7T = 4+3)
		cpuTickT(); cpuTickT(); cpuTickT();
		int val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_C, val);
		BREAK;
	}

	case 0x0F:
	{ // RRCA (4T)
		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		pPacMan_flags->FCARRY = (a & 0x01) ? 1 : 0;
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		BYTE res = (a >> 1) | (a << 7);
		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x10:
	{ // DJNZ e		// M2: fetch displacement (3T)
		cpuTickT(); // Internal cycle (1T)

		// Memory read takes about 3T
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE offset = (SBYTE)readRawMemoryFromCPU(pPacMan_registers->pc);

		pPacMan_registers->pc++;

		// Decrement B and test
		int b = cpuReadRegister(REGISTER_TYPE::RT_B) - 1;
		cpuSetRegister(REGISTER_TYPE::RT_B, b);

		if (b != 0)
		{
			// M3: branch taken (5T)
			cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
			pPacMan_registers->pc += offset;
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		}
		BREAK;
	}

	case 0x11:
	{ // LD DE,nn
		// 10 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_E, low);

		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_D, high);
		BREAK;
	}

	case 0x12:
	{ // LD (DE),A
		// 7 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_DE),
			(BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		auto temp = (uint32_t)((cpuReadRegister(REGISTER_TYPE::RT_A) << EIGHT) | (BYTE)(cpuReadRegister(REGISTER_TYPE::RT_E) + ONE));
		cpuSetRegister(REGISTER_TYPE::RT_WZ, temp);

		BREAK;
	}

	case 0x13:
	{ // INC DE
		// 6 cycles
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_DE, cpuReadRegister(REGISTER_TYPE::RT_DE) + 1);
		BREAK;
	}

	case 0x14:
	{ // INC D
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		BYTE res = old + 1;
		processFlagsFor8BitAdditionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_D, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x15:
	{ // DEC D
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		BYTE res = old - 1;
		processFlagsFor8BitSubtractionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_D, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x16:
	{ // LD D,n
		// 7 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_D, val);
		BREAK;
	}

	case 0x17:
	{ // RLA
		// 4 cycles

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE carry = pPacMan_flags->FCARRY;
		pPacMan_flags->FCARRY = (a & 0x80) ? 1 : 0;
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		cpuSetRegister(REGISTER_TYPE::RT_A, (a << 1) | carry);
		updateXY(pPacMan_cpuInstance->opcode, ((a << 1) | carry));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x18:
	{ // JR e
		// 12 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE offset = (SBYTE)readRawMemoryFromCPU(pPacMan_registers->pc);

		pPacMan_registers->pc++;

		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc += offset;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		BREAK;
	}

	case 0x19:
	{ // ADD HL,DE
		// 11 cycles

		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t res = cpuReadRegister(REGISTER_TYPE::RT_HL) +
			cpuReadRegister(REGISTER_TYPE::RT_DE);
		processFlagsFor16BitAdditionOperation(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_DE),
			false, false);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)res);
		updateXY(pPacMan_cpuInstance->opcode, sizeof(res) > ONE ? res >> EIGHT : res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x1A:
	{ // LD A,(DE)
		// 7 cycles
		cpuTickT(); cpuTickT(); cpuTickT();
		int val = cpuReadPointer(POINTER_TYPE::RT_M_DE);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_DE) + ONE));
		BREAK;
	}

	case 0x1B:
	{ // DEC DE
		// 6 cycles
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_DE, cpuReadRegister(REGISTER_TYPE::RT_DE) - 1);
		BREAK;
	}

	case 0x1C:
	{ // INC E
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		BYTE res = old + 1;
		processFlagsFor8BitAdditionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_E, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x1D:
	{ // DEC E
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		BYTE res = old - 1;
		processFlagsFor8BitSubtractionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_E, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x1E:
	{ // LD E,n
		// 7 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_E, val);
		BREAK;
	}

	case 0x1F:
	{ // RRA
		// 4 cycles

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE carry = pPacMan_flags->FCARRY;
		pPacMan_flags->FCARRY = (a & 0x01) ? 1 : 0;
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		cpuSetRegister(REGISTER_TYPE::RT_A, (a >> 1) | (carry << 7));
		updateXY(pPacMan_cpuInstance->opcode, (a >> 1) | (carry << 7));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x20:
	{ // JR NZ,e
		// 7 cycles if not taken, 12 cycles if taken

		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE offset = (SBYTE)readRawMemoryFromCPU(pPacMan_registers->pc);

		pPacMan_registers->pc++;

		if (!pPacMan_flags->FZERO)
		{
			cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
			pPacMan_registers->pc += offset;
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		}
		BREAK;
	}

	case 0x21:
	{ // LD HL,nn
		// 10 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		cpuSetRegister(REGISTER_TYPE::RT_HL, (high << 8) | low);
		BREAK;
	}

	case 0x22:
	{ // LD (nn),HL
		// 16 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		uint16_t addr = (high << 8) | low;

		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(addr, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(addr + 1, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H));

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr + 1);

		BREAK;
	}

	case 0x23:
	{ // INC HL
		// 6 cycles

		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) + 1);
		BREAK;
	}

	case 0x24:
	{ // INC H
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		BYTE res = old + 1;
		processFlagsFor8BitAdditionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_H, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x25:
	{ // DEC H
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		BYTE res = old - 1;
		processFlagsFor8BitSubtractionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_H, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x26:
	{ // LD H,n
		// 7 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_H, val);
		BREAK;
	}

	case 0x27:
	{ // DAA (4T)

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE correction = 0;

		// low nibble adjustment
		if ((a & 0x0F) > 0x09 || pPacMan_flags->FHALFCARRY)
			correction += 0x06;

		// high nibble adjustment
		if (a > 0x99 || pPacMan_flags->FCARRY)
		{
			correction += 0x60;
			pPacMan_flags->FCARRY = ONE;
		}

		if (pPacMan_flags->FNEGATIVE)
		{
			// subtraction
			pPacMan_flags->FHALFCARRY = pPacMan_flags->FHALFCARRY & ((a & 0x0F) < 0x06);
			cpuSetRegister(REGISTER_TYPE::RT_A, a - correction);
		}
		else
		{
			// addition
			pPacMan_flags->FHALFCARRY = (a & 0x0F) > 0x09;
			cpuSetRegister(REGISTER_TYPE::RT_A, a + correction);
		}

		// use existing helper
		processSZPFlags((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x28:
	{ // JR Z,e
		// 7 cycles if not taken, 12 cycles if taken

		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE offset = (SBYTE)readRawMemoryFromCPU(pPacMan_registers->pc);

		pPacMan_registers->pc++;

		if (pPacMan_flags->FZERO)
		{
			cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
			pPacMan_registers->pc += offset;
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		}
		BREAK;
	}

	case 0x29:
	{ // ADD HL,HL
		// 11 cycles

		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t hl = cpuReadRegister(REGISTER_TYPE::RT_HL);
		uint32_t res = hl + hl;
		processFlagsFor16BitAdditionOperation(hl, hl, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)res);
		updateXY(pPacMan_cpuInstance->opcode, sizeof(res) > ONE ? res >> EIGHT : res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x2A:
	{ // LD HL,(nn)
		// 16 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		uint16_t addr = (high << 8) | low;

		cpuTickT(); cpuTickT(); cpuTickT();
		int l = readRawMemoryFromCPU(addr);
		cpuTickT(); cpuTickT(); cpuTickT();
		int h = readRawMemoryFromCPU(addr + 1);

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr + 1);

		cpuSetRegister(REGISTER_TYPE::RT_HL, (h << 8) | l);
		BREAK;
	}

	case 0x2B:
	{ // DEC HL
		// 6 cycles
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) - 1);
		BREAK;
	}

	case 0x2C:
	{ // INC L
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE res = old + 1;
		processFlagsFor8BitAdditionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_L, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x2D:
	{ // DEC L
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE res = old - 1;
		processFlagsFor8BitSubtractionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_L, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x2E:
	{ // LD L,n
		// 7 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_L, val);
		BREAK;
	}

	case 0x2F:
	{ // CPL (4T)

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_A, a ^ 0xFF);  // complement A
		pPacMan_flags->FNEGATIVE = 1;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (a ^ 0xFF));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x30:
	{ // JR NC,e
		// 7 cycles if not taken, 12 cycles if taken

		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE offset = (SBYTE)readRawMemoryFromCPU(pPacMan_registers->pc);

		pPacMan_registers->pc++;

		if (!pPacMan_flags->FCARRY)
		{
			cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
			pPacMan_registers->pc += offset;
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		}
		BREAK;
	}

	case 0x31:
	{ // LD SP,nn
		// 10 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		cpuSetRegister(REGISTER_TYPE::RT_SP, (high << 8) | low);
		BREAK;
	}

	case 0x32:
	{ // LD (nn),A
		// 13 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		uint16_t addr = (high << 8) | low;

		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(addr, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		auto temp = (uint32_t)(cpuReadRegister(REGISTER_TYPE::RT_A) << EIGHT) | (uint32_t)((addr + ONE) & 0xFF);
		cpuSetRegister(REGISTER_TYPE::RT_WZ, temp);

		BREAK;
	}

	case 0x33:
	{ // INC SP
		// 6 cycles
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_SP, cpuReadRegister(REGISTER_TYPE::RT_SP) + 1);
		BREAK;
	}

	case 0x34:
	{ // INC (HL)
		// 11 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t addr = cpuReadRegister(REGISTER_TYPE::RT_HL);
		int val = readRawMemoryFromCPU(addr);

		cpuTickT();
		BYTE res = (BYTE)val + 1;
		processFlagsFor8BitAdditionOperation((BYTE)val, 1, false, false);

		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(addr, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x35:
	{ // DEC (HL)
		// 11 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t addr = cpuReadRegister(REGISTER_TYPE::RT_HL);
		int val = readRawMemoryFromCPU(addr);

		cpuTickT();
		BYTE res = (BYTE)val - 1;
		processFlagsFor8BitSubtractionOperation((BYTE)val, 1, false, false);

		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(addr, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x36:
	{ // LD (HL),n
		// 10 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)val);
		BREAK;
	}

	case 0x37:
	{ // SCF
		// 4 cycles

		pPacMan_flags->FCARRY = ONE;
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		// Refer https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
		updateXY(pPacMan_cpuInstance->opcode, (cpuReadRegister(REGISTER_TYPE::RT_Q) ^ cpuReadRegister(REGISTER_TYPE::RT_F)) | cpuReadRegister(REGISTER_TYPE::RT_A));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x38:
	{ // JR C,e
		// 7 cycles if not taken, 12 cycles if taken

		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE offset = (SBYTE)readRawMemoryFromCPU(pPacMan_registers->pc);

		pPacMan_registers->pc++;

		if (pPacMan_flags->FCARRY)
		{
			cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
			pPacMan_registers->pc += offset;
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		}
		BREAK;
	}

	case 0x39:
	{ // ADD HL,SP
		// 11 cycles
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t res = cpuReadRegister(REGISTER_TYPE::RT_HL) +
			cpuReadRegister(REGISTER_TYPE::RT_SP);
		processFlagsFor16BitAdditionOperation(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_SP),
			false, false);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)res);
		updateXY(pPacMan_cpuInstance->opcode, sizeof(res) > ONE ? res >> EIGHT : res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x3A:
	{ // LD A,(nn)
		// 13 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		uint16_t addr = (high << 8) | low;

		cpuTickT(); cpuTickT(); cpuTickT();
		int val = readRawMemoryFromCPU(addr);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(addr + ONE));
		BREAK;
	}

	case 0x3B:
	{ // DEC SP
		// 6 cycles
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_SP, cpuReadRegister(REGISTER_TYPE::RT_SP) - 1);
		BREAK;
	}

	case 0x3C:
	{ // INC A
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE res = old + 1;
		processFlagsFor8BitAdditionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x3D:
	{ // DEC A
		// 4 cycles

		BYTE old = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE res = old - 1;
		processFlagsFor8BitSubtractionOperation(old, 1, false, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x3E:
	{ // LD A,n
		// 7 cycles

		cpuTickT(); cpuTickT(); cpuTickT();
		int val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		BREAK;
	}

	case 0x3F:
	{ // CCF
		// 4 cycles
		BYTE oldCarry = pPacMan_flags->FCARRY;
		pPacMan_flags->FCARRY = oldCarry ^ 1;  // complement carry
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = oldCarry;
		// Refer https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
		updateXY(pPacMan_cpuInstance->opcode, (cpuReadRegister(REGISTER_TYPE::RT_Q) ^ cpuReadRegister(REGISTER_TYPE::RT_F)) | cpuReadRegister(REGISTER_TYPE::RT_A));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x40:
	{ // LD B,B
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_B, val);
		BREAK;
	}

	case 0x41:
	{ // LD B,C
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_B, val);
		BREAK;
	}

	case 0x42:
	{ // LD B,D
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_B, val);
		BREAK;
	}

	case 0x43:
	{ // LD B,E
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_B, val);
		BREAK;
	}

	case 0x44:
	{ // LD B,H
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_B, val);
		BREAK;
	}

	case 0x45:
	{ // LD B,L
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_B, val);
		BREAK;
	}

	case 0x46:
	{ // LD B,(HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuSetRegister(REGISTER_TYPE::RT_B, val);
		BREAK;
	}

	case 0x47:
	{ // LD B,A
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_B, val);
		BREAK;
	}

	case 0x48:
	{ // LD C,B
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_C, val);
		BREAK;
	}

	case 0x49:
	{ // LD C,C
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_C, val);
		BREAK;
	}

	case 0x4A:
	{ // LD C,D
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_C, val);
		BREAK;
	}

	case 0x4B:
	{ // LD C,E
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_C, val);
		BREAK;
	}

	case 0x4C:
	{ // LD C,H
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_C, val);
		BREAK;
	}

	case 0x4D:
	{ // LD C,L
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_C, val);
		BREAK;
	}

	case 0x4E:
	{ // LD C,(HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuSetRegister(REGISTER_TYPE::RT_C, val);
		BREAK;
	}

	case 0x4F:
	{ // LD C,A
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_C, val);
		BREAK;
	}

	case 0x50:
	{ // LD D,B
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_D, val);
		BREAK;
	}

	case 0x51:
	{ // LD D,C
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_D, val);
		BREAK;
	}

	case 0x52:
	{ // LD D,D
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_D, val);
		BREAK;
	}

	case 0x53:
	{ // LD D,E
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_D, val);
		BREAK;
	}

	case 0x54:
	{ // LD D,H
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_D, val);
		BREAK;
	}

	case 0x55:
	{ // LD D,L
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_D, val);
		BREAK;
	}

	case 0x56:
	{ // LD D,(HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuSetRegister(REGISTER_TYPE::RT_D, val);
		BREAK;
	}

	case 0x57:
	{ // LD D,A
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_D, val);
		BREAK;
	}

	case 0x58:
	{ // LD E,B
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_E, val);
		BREAK;
	}

	case 0x59:
	{ // LD E,C
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_E, val);
		BREAK;
	}

	case 0x5A:
	{ // LD E,D
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_E, val);
		BREAK;
	}

	case 0x5B:
	{ // LD E,E
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_E, val);
		BREAK;
	}

	case 0x5C:
	{ // LD E,H
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_E, val);
		BREAK;
	}

	case 0x5D:
	{ // LD E,L
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_E, val);
		BREAK;
	}

	case 0x5E:
	{ // LD E,(HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuSetRegister(REGISTER_TYPE::RT_E, val);
		BREAK;
	}

	case 0x5F:
	{ // LD E,A
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_E, val);
		BREAK;
	}

	case 0x60:
	{ // LD H,B
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_H, val);
		BREAK;
	}

	case 0x61:
	{ // LD H,C
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_H, val);
		BREAK;
	}

	case 0x62:
	{ // LD H,D
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_H, val);
		BREAK;
	}

	case 0x63:
	{ // LD H,E
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_H, val);
		BREAK;
	}

	case 0x64:
	{ // LD H,H
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_H, val);
		BREAK;
	}

	case 0x65:
	{ // LD H,L
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_H, val);
		BREAK;
	}

	case 0x66:
	{ // LD H,(HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuSetRegister(REGISTER_TYPE::RT_H, val);
		BREAK;
	}

	case 0x67:
	{ // LD H,A
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_H, val);
		BREAK;
	}

	case 0x68:
	{ // LD L,B
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_L, val);
		BREAK;
	}

	case 0x69:
	{ // LD L,C
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_L, val);
		BREAK;
	}

	case 0x6A:
	{ // LD L,D
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_L, val);
		BREAK;
	}

	case 0x6B:
	{ // LD L,E
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_L, val);
		BREAK;
	}

	case 0x6C:
	{ // LD L,H
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_L, val);
		BREAK;
	}

	case 0x6D:
	{ // LD L,L
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_L, val);
		BREAK;
	}

	case 0x6E:
	{ // LD L,(HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuSetRegister(REGISTER_TYPE::RT_L, val);
		BREAK;
	}

	case 0x6F:
	{ // LD L,A
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_L, val);
		BREAK;
	}

	case 0x70:
	{ // LD (HL),B
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuWritePointer(POINTER_TYPE::RT_M_HL, val);
		BREAK;
	}

	case 0x71:
	{ // LD (HL),C
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuWritePointer(POINTER_TYPE::RT_M_HL, val);
		BREAK;
	}

	case 0x72:
	{ // LD (HL),D
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuWritePointer(POINTER_TYPE::RT_M_HL, val);
		BREAK;
	}

	case 0x73:
	{ // LD (HL),E
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuWritePointer(POINTER_TYPE::RT_M_HL, val);
		BREAK;
	}

	case 0x74:
	{ // LD (HL),H
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuWritePointer(POINTER_TYPE::RT_M_HL, val);
		BREAK;
	}

	case 0x75:
	{ // LD (HL),L
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuWritePointer(POINTER_TYPE::RT_M_HL, val);
		BREAK;
	}

	case 0x76:
	{ // HALT
		pPacMan_instance->pacMan_state.HaltEnabled = YES;
		BREAK;
	}

	case 0x77:
	{ // LD (HL),A
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuWritePointer(POINTER_TYPE::RT_M_HL, val);
		BREAK;
	}

	case 0x78:
	{ // LD A,B
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		BREAK;
	}

	case 0x79:
	{ // LD A,C
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		BREAK;
	}

	case 0x7A:
	{ // LD A,D
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		BREAK;
	}

	case 0x7B:
	{ // LD A,E
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		BREAK;
	}

	case 0x7C:
	{ // LD A,H
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		BREAK;
	}

	case 0x7D:
	{ // LD A,L
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		BREAK;
	}

	case 0x7E:
	{ // LD A,(HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE val = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		BREAK;
	}

	case 0x7F:
	{ // LD A,A
		BYTE val = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_A, val);
		BREAK;
	}

	case 0x80:
	{ // ADD A,B
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_B);

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x81:
	{ // ADD A,C
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_C);

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x82:
	{ // ADD A,D
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_D);

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x83:
	{ // ADD A,E
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_E);

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x84:
	{ // ADD A,H
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_H);

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x85:
	{ // ADD A,L
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_L);

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x86:
	{ // ADD A,(HL)
		// M2: memory read (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);

		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x87:
	{ // ADD A,A
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_A);

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x88:
	{ // ADC A,B
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_B) + (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x89:
	{ // ADC A,C
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_C) + (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x8A:
	{ // ADC A,D
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_D) + (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x8B:
	{ // ADC A,E
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_E) + (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x8C:
	{ // ADC A,H
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_H) + (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x8D:
	{ // ADC A,L
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_L) + (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x8E:
	{ // ADC A,(HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);

		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory + (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x8F:
	{ // ADC A,A
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x90:
	{ // SUB B
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_B);

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x91:
	{ // SUB C
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_C);

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x92:
	{ // SUB D
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_D);

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x93:
	{ // SUB E
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_E);

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x94:
	{ // SUB H
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_H);

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x95:
	{ // SUB L
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_L);

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x96:
	{ // SUB (HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);

		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - (uint16_t)dataFromMemory;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x97:
	{ // SUB A
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_A);

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x98:
	{ // SBC A,B
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_B) - (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x99:
	{ // SBC A,C
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_C) - (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x9A:
	{ // SBC A,D
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_D) - (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x9B:
	{ // SBC A,E
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_E) - (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x9C:
	{ // SBC A,H
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_H) - (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x9D:
	{ // SBC A,L
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_L) - (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x9E:
	{ // SBC A,(HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);

		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - (uint16_t)dataFromMemory - (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0x9F:
	{ // SBC A,A
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_A) - (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA0:
	{ // AND B
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		processFlagsForLogicalOperation((BYTE)operationResult, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA1:
	{ // AND C
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		processFlagsForLogicalOperation((BYTE)operationResult, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA2:
	{ // AND D
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		processFlagsForLogicalOperation((BYTE)operationResult, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA3:
	{ // AND E
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		processFlagsForLogicalOperation((BYTE)operationResult, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA4:
	{ // AND H
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		processFlagsForLogicalOperation((BYTE)operationResult, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA5:
	{ // AND L
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		processFlagsForLogicalOperation((BYTE)operationResult, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA6:
	{ // AND (HL)
		// M2: memory read (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);

		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & dataFromMemory;
		processFlagsForLogicalOperation((BYTE)operationResult, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA7:
	{ // AND A
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		processFlagsForLogicalOperation((BYTE)operationResult, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA8:
	{ // XOR B
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xA9:
	{ // XOR C
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xAA:
	{ // XOR D
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xAB:
	{ // XOR E
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		BREAK;
	}

	case 0xAC:
	{ // XOR H
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xAD:
	{ // XOR L
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xAE:
	{ // XOR (HL)
		// M2: memory read (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);

		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ dataFromMemory;
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xAF:
	{ // XOR A
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB0:
	{ // OR A,B
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB1:
	{ // OR A,C
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB2:
	{ // OR A,D
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB3:
	{ // OR A,E
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB4:
	{ // OR A,H
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB5:
	{ // OR A,L
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB6:
	{ // OR A,(HL)
		// M2: memory read
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);

		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | dataFromMemory;
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB7:
	{ // OR A,A
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		processFlagsForLogicalOperation((BYTE)operationResult, false);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB8:
	{ // CP B
		BYTE regA = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE regB = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		operationResult = (uint16_t)regA - (uint16_t)regB;
		processFlagsFor8BitSubtractionOperation(regA, regB, false, true);
		updateXY(pPacMan_cpuInstance->opcode, regB);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xB9:
	{ // CP C
		BYTE regA = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE regC = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		operationResult = (uint16_t)regA - (uint16_t)regC;
		processFlagsFor8BitSubtractionOperation(regA, regC, false, true);
		updateXY(pPacMan_cpuInstance->opcode, regC);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xBA:
	{ // CP D
		BYTE regA = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE regD = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		operationResult = (uint16_t)regA - (uint16_t)regD;
		processFlagsFor8BitSubtractionOperation(regA, regD, false, true);
		updateXY(pPacMan_cpuInstance->opcode, regD);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xBB:
	{ // CP E
		BYTE regA = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE regE = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		operationResult = (uint16_t)regA - (uint16_t)regE;
		processFlagsFor8BitSubtractionOperation(regA, regE, false, true);
		updateXY(pPacMan_cpuInstance->opcode, regE);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xBC:
	{ // CP H
		BYTE regA = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE regH = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		operationResult = (uint16_t)regA - (uint16_t)regH;
		processFlagsFor8BitSubtractionOperation(regA, regH, false, true);
		updateXY(pPacMan_cpuInstance->opcode, regH);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xBD:
	{ // CP L
		BYTE regA = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE regL = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		operationResult = (uint16_t)regA - (uint16_t)regL;
		processFlagsFor8BitSubtractionOperation(regA, regL, false, true);
		updateXY(pPacMan_cpuInstance->opcode, regL);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xBE:
	{ // CP (HL)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);

		BYTE regA = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		operationResult = (uint16_t)regA - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation(regA, dataFromMemory, false, true);
		updateXY(pPacMan_cpuInstance->opcode, dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xBF:
	{ // CP A
		BYTE regA = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		operationResult = 0; // A - A
		processFlagsFor8BitSubtractionOperation(regA, regA, false, true);
		updateXY(pPacMan_cpuInstance->opcode, regA);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xC0:
	{ // RET NZ (11T if taken, 5T if not taken)
		if (pPacMan_flags->FZERO == 0)
		{
			// Condition met - perform return
			cpuTickT(); // Internal cycle (1T)

			// Pop low byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE low = stackPop();

			// Pop high byte from stack  
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE high = stackPop();

			// Set PC to popped address
			uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
			pPacMan_registers->pc = addr;

			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		}
		else
		{
			// Condition not met - just internal cycle
			cpuTickT(); // Internal cycle (1T)
		}

		BREAK;
	}

	case 0xC1:
	{ // POP BC (10T = 4+3+3)
		// Pop low byte from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE low = stackPop();

		// Pop high byte from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE high = stackPop();

		uint16_t val = (((uint16_t)high) << 8) | ((uint16_t)low);
		cpuSetRegister(REGISTER_TYPE::RT_BC, val);
		BREAK;
	}

	case 0xC2:
	{ // JP NZ,nn (10T = 4+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		if (pPacMan_flags->FZERO == 0)
		{
			// Condition met - jump
			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xC3:
	{ // JP nn (10T = 4+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		pPacMan_registers->pc = addr;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xC4:
	{ // CALL NZ,nn (17T if taken, 10T if not taken)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);

		if (pPacMan_flags->FZERO == 0)
		{
			// Condition met - perform call
			cpuTickT(); // Internal cycle (1T)

			// Push current PC to stack
			BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE pcLow = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcHigh);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcLow);

			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xC5:
	{ // PUSH BC (11T = 4+1+3+3)
		cpuTickT(); // Internal cycle (1T)

		uint16_t val = cpuReadRegister(REGISTER_TYPE::RT_BC);
		BYTE high = (BYTE)(val >> 8);
		BYTE low = (BYTE)val;

		// Push high byte first
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(high);

		// Push low byte
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(low);

		BREAK;
	}

	case 0xC6:
	{ // ADD A,n (7T = 4+3)
		// Read immediate byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE res = a + val;

		processFlagsFor8BitAdditionOperation(a, val, false, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xC7:
	{ // RST 00H (11T = 4+1+3+3)
		pPacMan_instance->pacMan_state.HaltEnabled = false;

		cpuTickT(); // Internal cycle (1T)

		// Push current PC to stack
		BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
		BYTE pcLow = (BYTE)pPacMan_registers->pc;
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcHigh);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcLow);

		pPacMan_registers->pc = 0x0000;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		BREAK;
	}

	case 0xC8:
	{ // RET Z (11T if taken, 5T if not taken)
		if (pPacMan_flags->FZERO)
		{
			// Condition met - perform return
			cpuTickT(); // Internal cycle (1T)

			// Pop low byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE low = stackPop();

			// Pop high byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE high = stackPop();

			uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
			pPacMan_registers->pc = addr;

			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		}
		else
		{
			// Condition not met - just internal cycle
			cpuTickT(); // Internal cycle (1T)
		}

		BREAK;
	}

	case 0xC9:
	{ // RET (10T = 4+3+3)
		// Pop low byte from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE low = stackPop();

		// Pop high byte from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE high = stackPop();

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		pPacMan_registers->pc = addr;

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xCA:
	{ // JP Z,nn (10T = 4+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		if (pPacMan_flags->FZERO)
		{
			// Condition met - jump
			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xCB:
	{
		pPacMan_cpuInstance->prefix1 = 0xCB;
		process0xCB();
		BREAK;
	}

	case 0xCC:
	{ // CALL Z,nn (17T if taken, 10T if not taken)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);

		if (pPacMan_flags->FZERO)
		{
			// Condition met - perform call
			cpuTickT(); // Internal cycle (1T)

			// Push current PC to stack
			BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE pcLow = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcHigh);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcLow);

			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xCD:
	{ // CALL nn (17T = 4+3+3+1+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);

		cpuTickT(); // Internal cycle (1T)

		// Push current PC to stack
		BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
		BYTE pcLow = (BYTE)pPacMan_registers->pc;
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcHigh);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcLow);

		pPacMan_registers->pc = addr;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xCE:
	{ // ADC A,n (7T = 4+3)
		// Read immediate byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE res = a + val + (BYTE)pPacMan_flags->FCARRY;

		processFlagsFor8BitAdditionOperation(a, val, true, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xCF:
	{ // RST 08H (11T = 4+1+3+3)
		pPacMan_instance->pacMan_state.HaltEnabled = false;

		cpuTickT(); // Internal cycle (1T)

		// Push current PC to stack
		BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
		BYTE pcLow = (BYTE)pPacMan_registers->pc;
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcHigh);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcLow);

		pPacMan_registers->pc = 0x0008;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		BREAK;
	}

	case 0xD0:
	{ // RET NC (11T if taken, 5T if not taken)
		if (pPacMan_flags->FCARRY == 0)
		{
			// Condition met - perform return
			cpuTickT(); // Internal cycle (1T)

			// Pop low byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE low = stackPop();

			// Pop high byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE high = stackPop();

			uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
			pPacMan_registers->pc = addr;

			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		}
		else
		{
			// Condition not met - just internal cycle
			cpuTickT(); // Internal cycle (1T)
		}

		BREAK;
	}

	case 0xD1:
	{ // POP DE (10T = 4+3+3)
		// Pop low byte from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE low = stackPop();

		// Pop high byte from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE high = stackPop();

		uint16_t val = (((uint16_t)high) << 8) | ((uint16_t)low);
		cpuSetRegister(REGISTER_TYPE::RT_DE, val);
		BREAK;
	}

	case 0xD2:
	{ // JP NC,nn (10T = 4+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		if (pPacMan_flags->FCARRY == 0)
		{
			// Condition met - jump
			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xD3:
	{ // OUT (n),A (11T = 4+3+4)
		// Read port number
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE port = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		BYTE aVal = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);

		// I/O operation
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); // I/O write (4T)

		// Save the port number
		pPacMan_cpuInstance->port = port;

		// Port 0 sets the ISR address
		if (port == 0)
		{
			pPacMan_instance->pacMan_state.port0Data = (uint16_t)aVal;
		}
		else if (ROM_TYPE != ROM::TEST_ROM_CIM)
		{
			unimplementedInstruction();
		}

		BREAK;
	}

	case 0xD4:
	{ // CALL NC,nn (17T if taken, 10T if not taken)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);

		if (pPacMan_flags->FCARRY == 0)
		{
			// Condition met - perform call
			cpuTickT(); // Internal cycle (1T)

			// Push current PC to stack
			BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE pcLow = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcHigh);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcLow);

			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xD5:
	{ // PUSH DE (11T = 4+1+3+3)
		cpuTickT(); // Internal cycle (1T)

		uint16_t val = cpuReadRegister(REGISTER_TYPE::RT_DE);
		BYTE high = (BYTE)(val >> 8);
		BYTE low = (BYTE)val;

		// Push high byte first
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(high);

		// Push low byte
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(low);

		BREAK;
	}

	case 0xD6:
	{ // SUB n (7T = 4+3)
		// Read immediate byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE res = a - val;

		processFlagsFor8BitSubtractionOperation(a, val, false, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xD7:
	{ // RST 10H (11T = 4+1+3+3)
		pPacMan_instance->pacMan_state.HaltEnabled = false;

		cpuTickT(); // Internal cycle (1T)

		// Push current PC to stack
		BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
		BYTE pcLow = (BYTE)pPacMan_registers->pc;
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcHigh);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcLow);

		pPacMan_registers->pc = 0x0010;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		BREAK;
	}

	case 0xD8:
	{ // RET C (11T if taken, 5T if not taken)
		if (pPacMan_flags->FCARRY)
		{
			// Condition met - perform return
			cpuTickT(); // Internal cycle (1T)

			// Pop low byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE low = stackPop();

			// Pop high byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE high = stackPop();

			uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
			pPacMan_registers->pc = addr;

			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		}
		else
		{
			// Condition not met - just internal cycle
			cpuTickT(); // Internal cycle (1T)
		}

		BREAK;
	}

	case 0xD9:
	{ // EXX (4T)
		// Exchange BC with BC'
		uint16_t tmpBC = cpuReadRegister(REGISTER_TYPE::RT_BC);
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_SHADOW_BC));
		cpuSetRegister(REGISTER_TYPE::RT_SHADOW_BC, tmpBC);

		// Exchange DE with DE'
		uint16_t tmpDE = cpuReadRegister(REGISTER_TYPE::RT_DE);
		cpuSetRegister(REGISTER_TYPE::RT_DE, cpuReadRegister(REGISTER_TYPE::RT_SHADOW_DE));
		cpuSetRegister(REGISTER_TYPE::RT_SHADOW_DE, tmpDE);

		// Exchange HL with HL'
		uint16_t tmpHL = cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_SHADOW_HL));
		cpuSetRegister(REGISTER_TYPE::RT_SHADOW_HL, tmpHL);

		BREAK;
	}

	case 0xDA:
	{ // JP C,nn (10T = 4+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		if (pPacMan_flags->FCARRY)
		{
			// Condition met - jump
			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xDB:
	{ // IN A,(n) (11T = 4+3+4)
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE port = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// I/O operation  
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); // I/O read (4T)

		// Save the port number
		pPacMan_cpuInstance->port = port;

		if (ROM_TYPE != ROM::TEST_ROM_CIM)
		{
			unimplementedInstruction();
		}

		BREAK;
	}

	case 0xDC:
	{ // CALL C,nn (17T if taken, 10T if not taken)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);

		if (pPacMan_flags->FCARRY)
		{
			// Condition met - perform call
			cpuTickT(); // Internal cycle (1T)

			// Push current PC to stack
			BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE pcLow = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcHigh);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcLow);

			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xDD:
	{
		pPacMan_cpuInstance->prefix1 = 0xDD;
		process0xDD0xFD(REGISTER_TYPE::RT_IX, REGISTER_TYPE::RT_IXH, REGISTER_TYPE::RT_IXL);
		BREAK;
	}

	case 0xDE:
	{ // SBC A,n (7T = 4+3)
		// Read immediate byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE res = a - val - (BYTE)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation(a, val, true, true);
		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xDF:
	{ // RST 18H (11T = 4+1+3+3)
		pPacMan_instance->pacMan_state.HaltEnabled = false;

		cpuTickT(); // Internal cycle (1T)

		// Push current PC to stack
		BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
		BYTE pcLow = (BYTE)pPacMan_registers->pc;
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcHigh);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcLow);

		pPacMan_registers->pc = 0x0018;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		BREAK;
	}

	case 0xE0:
	{ // RET PO (11T if taken, 5T if not taken)
		if (pPacMan_flags->F_OF_PARITY == 0)
		{
			// Condition met - perform return
			cpuTickT(); // Internal cycle (1T)

			// Pop low byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE low = stackPop();

			// Pop high byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE high = stackPop();

			uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
			pPacMan_registers->pc = addr;

			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		}
		else
		{
			// Condition not met - just internal cycle
			cpuTickT(); // Internal cycle (1T)
		}

		BREAK;
	}

	case 0xE1:
	{ // POP HL (10T = 4+3+3)
		// Pop low byte from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE low = stackPop();

		// Pop high byte from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE high = stackPop();

		uint16_t val = (((uint16_t)high) << 8) | ((uint16_t)low);
		cpuSetRegister(REGISTER_TYPE::RT_HL, val);
		BREAK;
	}

	case 0xE2:
	{ // JP PO,nn (10T = 4+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		if (pPacMan_flags->F_OF_PARITY == 0)
		{
			// Condition met - jump
			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xE3:
	{ // EX (SP),HL (19T = 4+3+4+3+3+1+1)
		// Read low byte from stack
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE stackLow = readRawMemoryFromCPU(pPacMan_registers->sp);

		// Read high byte from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE stackHigh = readRawMemoryFromCPU(pPacMan_registers->sp + 1);

		// Get current HL values
		cpuTickT();
		BYTE hlLow = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE hlHigh = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);

		// Write HL to stack
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory write (3T)
		writeRawMemoryFromCPU(pPacMan_registers->sp, hlLow);

		cpuTickT(); cpuTickT(); cpuTickT(); // Memory write (3T)
		writeRawMemoryFromCPU(pPacMan_registers->sp + 1, hlHigh);

		// Internal cycles
		cpuTickT(); // Internal (1T)
		cpuTickT(); // Internal (1T)

		// Set HL to stack values
		cpuSetRegister(REGISTER_TYPE::RT_L, stackLow);
		cpuSetRegister(REGISTER_TYPE::RT_H, stackHigh);

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, cpuReadRegister(REGISTER_TYPE::RT_HL));

		BREAK;
	}

	case 0xE4:
	{ // CALL PO,nn (17T if taken, 10T if not taken)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);

		if (pPacMan_flags->F_OF_PARITY == 0)
		{
			// Condition met - perform call
			cpuTickT(); // Internal cycle (1T)

			// Push current PC to stack
			BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE pcLow = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcHigh);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcLow);

			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xE5:
	{ // PUSH HL (11T = 4+1+3+3)
		cpuTickT(); // Internal cycle (1T)

		uint16_t val = cpuReadRegister(REGISTER_TYPE::RT_HL);
		BYTE high = (BYTE)(val >> 8);
		BYTE low = (BYTE)val;

		// Push high byte first
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(high);

		// Push low byte
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(low);

		BREAK;
	}

	case 0xE6:
	{ // AND n (7T = 4+3)
		// Read immediate byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE res = a & val;

		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		processFlagsForLogicalOperation(res, true);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xE7:
	{ // RST 20H (11T = 4+1+3+3)
		pPacMan_instance->pacMan_state.HaltEnabled = false;

		cpuTickT(); // Internal cycle (1T)

		// Push current PC to stack
		BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
		BYTE pcLow = (BYTE)pPacMan_registers->pc;
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcHigh);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcLow);

		pPacMan_registers->pc = 0x0020;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		BREAK;
	}

	case 0xE8:
	{ // RET PE (11T if taken, 5T if not taken)
		if (pPacMan_flags->F_OF_PARITY)
		{
			// Condition met - perform return
			cpuTickT(); // Internal cycle (1T)

			// Pop low byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE low = stackPop();

			// Pop high byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE high = stackPop();

			uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
			pPacMan_registers->pc = addr;

			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		}
		else
		{
			// Condition not met - just internal cycle
			cpuTickT(); // Internal cycle (1T)
		}

		BREAK;
	}

	case 0xE9:
	{ // JP (HL) (4T)
		pPacMan_registers->pc = cpuReadRegister(REGISTER_TYPE::RT_HL);
		BREAK;
	}

	case 0xEA:
	{ // JP PE,nn (10T = 4+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		if (pPacMan_flags->F_OF_PARITY)
		{
			// Condition met - jump
			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xEB:
	{ // EX DE,HL (4T)
		uint16_t tmp = cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_DE));
		cpuSetRegister(REGISTER_TYPE::RT_DE, tmp);
		BREAK;
	}

	case 0xEC:
	{ // CALL PE,nn (17T if taken, 10T if not taken)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);

		if (pPacMan_flags->F_OF_PARITY)
		{
			// Condition met - perform call
			cpuTickT(); // Internal cycle (1T)

			// Push current PC to stack
			BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE pcLow = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcHigh);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcLow);

			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xED:
	{
		pPacMan_cpuInstance->prefix1 = 0xED;
		process0xED();
		BREAK;
	}

	case 0xEE:
	{ // XOR n (7T = 4+3)
		// Read immediate byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE res = a ^ val;

		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		processFlagsForLogicalOperation(res, false);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xEF:
	{ // RST 28H (11T = 4+1+3+3)
		pPacMan_instance->pacMan_state.HaltEnabled = false;

		cpuTickT(); // Internal cycle (1T)

		// Push current PC to stack
		BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
		BYTE pcLow = (BYTE)pPacMan_registers->pc;
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcHigh);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcLow);

		pPacMan_registers->pc = 0x0028;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		BREAK;
	}

	case 0xF0:
	{ // RET P (11T if taken, 5T if not taken)
		if (pPacMan_flags->FSIGN == 0)
		{
			// Condition met - perform return
			cpuTickT(); // Internal cycle (1T)

			// Pop low byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE low = stackPop();

			// Pop high byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE high = stackPop();

			uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
			pPacMan_registers->pc = addr;

			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		}
		else
		{
			// Condition not met - just internal cycle
			cpuTickT(); // Internal cycle (1T)
		}

		BREAK;
	}

	case 0xF1:
	{ // POP AF (10T = 4+3+3)
		// Pop low byte (flags) from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE low = stackPop();

		// Pop high byte (A register) from stack
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE high = stackPop();

		uint16_t val = (((uint16_t)high) << 8) | ((uint16_t)low);
		cpuSetRegister(REGISTER_TYPE::RT_AF, val);
		BREAK;
	}

	case 0xF2:
	{ // JP P,nn (10T = 4+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		if (pPacMan_flags->FSIGN == 0)
		{
			// Condition met - jump
			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xF3:
	{ // DI (4T)
		pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.interruptEnable &= 0xFE;
		pPacMan_registers->iff1 = RESET;
		pPacMan_registers->iff2 = RESET;
		BREAK;
	}

	case 0xF4:
	{ // CALL P,nn (17T if taken, 10T if not taken)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);

		if (pPacMan_flags->FSIGN == 0)
		{
			// Condition met - perform call
			cpuTickT(); // Internal cycle (1T)

			// Push current PC to stack
			BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE pcLow = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcHigh);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcLow);

			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xF5:
	{ // PUSH AF (11T = 4+1+3+3)
		cpuTickT(); // Internal cycle (1T)

		uint16_t val = cpuReadRegister(REGISTER_TYPE::RT_AF);
		BYTE high = (BYTE)(val >> 8);
		BYTE low = (BYTE)val;

		// Push high byte (A register) first
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(high);

		// Push low byte (flags)
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(low);

		BREAK;
	}

	case 0xF6:
	{ // OR n (7T = 4+3)
		// Read immediate byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE res = a | val;

		cpuSetRegister(REGISTER_TYPE::RT_A, res);
		processFlagsForLogicalOperation(res, false);
		updateXY(pPacMan_cpuInstance->opcode, res);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	case 0xF7:
	{ // RST 30H (11T = 4+1+3+3)
		pPacMan_instance->pacMan_state.HaltEnabled = false;

		cpuTickT(); // Internal cycle (1T)

		// Push current PC to stack
		BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
		BYTE pcLow = (BYTE)pPacMan_registers->pc;
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcHigh);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcLow);

		pPacMan_registers->pc = 0x0030;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		BREAK;
	}

	case 0xF8:
	{ // RET M (11T if taken, 5T if not taken)
		if (pPacMan_flags->FSIGN)
		{
			// Condition met - perform return
			cpuTickT(); // Internal cycle (1T)

			// Pop low byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE low = stackPop();

			// Pop high byte from stack
			cpuTickT(); cpuTickT(); cpuTickT();
			BYTE high = stackPop();

			uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
			pPacMan_registers->pc = addr;

			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		}
		else
		{
			// Condition not met - just internal cycle
			cpuTickT(); // Internal cycle (1T)
		}

		BREAK;
	}

	case 0xF9:
	{ // LD SP,HL (6T = 4+2)
		uint16_t hlVal = cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuSetRegister(REGISTER_TYPE::RT_SP, hlVal);

		cpuTickT(); cpuTickT(); // Internal cycles (2T)
		BREAK;
	}

	case 0xFA:
	{ // JP M,nn (10T = 4+3+3)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);
		if (pPacMan_flags->FSIGN)
		{
			// Condition met - jump
			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xFB:
	{ // EI (4T)
		pPacMan_memory->pacManMemoryMap.miscellaneousMemoryMap.miscellaneousFields.interruptEnable |= 0x01;
		pPacMan_registers->iff1 = SET;
		pPacMan_registers->iff2 = SET;
		BREAK;
	}

	case 0xFC:
	{ // CALL M,nn (17T if taken, 10T if not taken)
		// Read low byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		// Read high byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t addr = (((uint16_t)high) << 8) | ((uint16_t)low);

		if (pPacMan_flags->FSIGN)
		{
			// Condition met - perform call
			cpuTickT(); // Internal cycle (1T)

			// Push current PC to stack
			BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
			BYTE pcLow = (BYTE)pPacMan_registers->pc;
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcHigh);
			cpuTickT(); cpuTickT(); cpuTickT();
			stackPush(pcLow);

			pPacMan_registers->pc = addr;
		}
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, addr);
		BREAK;
	}

	case 0xFD:
	{
		pPacMan_cpuInstance->prefix1 = 0xFD;
		process0xDD0xFD(REGISTER_TYPE::RT_IY, REGISTER_TYPE::RT_IYH, REGISTER_TYPE::RT_IYL);
		BREAK;
	}

	case 0xFE:
	{ // CP n (7T = 4+3)
		// Read immediate byte
		cpuTickT(); cpuTickT(); cpuTickT(); // Memory read (3T)
		BYTE val = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		BYTE a = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		// Compare operation - subtract but don't store result
		processFlagsFor8BitSubtractionOperation(a, val, false, true);
		updateXY(pPacMan_cpuInstance->opcode, val);
		BREAK;
	}

	case 0xFF:
	{ // RST 38H (11T = 4+1+3+3)
		pPacMan_instance->pacMan_state.HaltEnabled = false;

		cpuTickT(); // Internal cycle (1T)

		// Push current PC to stack
		BYTE pcHigh = (BYTE)(pPacMan_registers->pc >> 8);
		BYTE pcLow = (BYTE)pPacMan_registers->pc;
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcHigh);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(pcLow);

		pPacMan_registers->pc = 0x0038;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc);
		BREAK;
	}

	default:
	{
		status = false;
		unimplementedInstruction();
		BREAK;
	}
	}

	cpuSetRegister(REGISTER_TYPE::RT_Q, (pPacMan_cpuInstance->possFlag == YES) ? cpuReadRegister(REGISTER_TYPE::RT_F) : ZERO);
	pPacMan_cpuInstance->possFlag = CLEAR;

	RETURN status;
}

// Should be called only from "performOperation"
void pacMan_t::process0xCB()
{
	auto operationResult = 0;
	auto dataFromMemory = 0;
	auto lowerDataFromMemory = 0;
	auto higherDataFromMemory = 0;

	cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();	// M2: Read CB opcode (4T)
	operationResult = readRawMemoryFromCPU(pPacMan_registers->pc, YES);
	incrementR();
	pPacMan_registers->pc += 1;

	BYTE bitUnderTest = 0;
	BYTE theBit = 0;

	if (
		(operationResult >= 0x40 && operationResult <= 0x47)
		||
		(operationResult >= 0x80 && operationResult <= 0x87)
		||
		(operationResult >= 0xC0 && operationResult <= 0xC7)
		)
	{
		bitUnderTest = BIT0_SET;
		theBit = BIT0;
	}
	else if (
		(operationResult >= 0x48 && operationResult <= 0x4F)
		||
		(operationResult >= 0x88 && operationResult <= 0x8F)
		||
		(operationResult >= 0xC8 && operationResult <= 0xCF)
		)
	{
		bitUnderTest = BIT1_SET;
		theBit = BIT1;
	}
	else if (
		(operationResult >= 0x50 && operationResult <= 0x57)
		||
		(operationResult >= 0x90 && operationResult <= 0x97)
		||
		(operationResult >= 0xD0 && operationResult <= 0xD7)
		)
	{
		bitUnderTest = BIT2_SET;
		theBit = BIT2;
	}
	else if (
		(operationResult >= 0x58 && operationResult <= 0x5F)
		||
		(operationResult >= 0x98 && operationResult <= 0x9F)
		||
		(operationResult >= 0xD8 && operationResult <= 0xDF)
		)
	{
		bitUnderTest = BIT3_SET;
		theBit = BIT3;
	}
	else if (
		(operationResult >= 0x60 && operationResult <= 0x67)
		||
		(operationResult >= 0xA0 && operationResult <= 0xA7)
		||
		(operationResult >= 0xE0 && operationResult <= 0xE7)
		)
	{
		bitUnderTest = BIT4_SET;
		theBit = BIT4;
	}
	else if (
		(operationResult >= 0x68 && operationResult <= 0x6F)
		||
		(operationResult >= 0xA8 && operationResult <= 0xAF)
		||
		(operationResult >= 0xE8 && operationResult <= 0xEF)
		)
	{
		bitUnderTest = BIT5_SET;
		theBit = BIT5;
	}
	else if (
		(operationResult >= 0x70 && operationResult <= 0x77)
		||
		(operationResult >= 0xB0 && operationResult <= 0xB7)
		||
		(operationResult >= 0xF0 && operationResult <= 0xF7)
		)
	{
		bitUnderTest = BIT6_SET;
		theBit = BIT6;
	}
	else if (
		(operationResult >= 0x78 && operationResult <= 0x7F)
		||
		(operationResult >= 0xB8 && operationResult <= 0xBF)
		||
		(operationResult >= 0xF8 && operationResult <= 0xFF)
		)
	{
		bitUnderTest = BIT7_SET;
		theBit = BIT7;
	}

	switch (operationResult)
	{
	case 0x00: // RLC B (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | (bit7 >> 7));
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x01: // RLC C (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | (bit7 >> 7));
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x02: // RLC D (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | (bit7 >> 7));
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x03: // RLC E (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | (bit7 >> 7));
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x04: // RLC H (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | (bit7 >> 7));
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x05: // RLC L (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | (bit7 >> 7));
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x06: // RLC (HL) (15T = 4+4+4+3)
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));

		cpuTickT();
		BYTE bit7 = dataFromMemory & 0b10000000;
		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		processSZPFlags((byte)dataFromMemory);

		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x07: // RLC A (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | (bit7 >> 7));
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	// RRC instructions (0x08-0x0F)
	case 0x08: // RRC B (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		BYTE bit0 = registerContent & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit0 << 7);
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x09: // RRC C (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		BYTE bit0 = registerContent & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit0 << 7);
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x0A: // RRC D (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		BYTE bit0 = registerContent & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit0 << 7);
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x0B: // RRC E (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		BYTE bit0 = registerContent & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit0 << 7);
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x0C: // RRC H (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		BYTE bit0 = registerContent & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit0 << 7);
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x0D: // RRC L (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE bit0 = registerContent & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit0 << 7);
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x0E: // RRC (HL) (15T = 4+4+4+3)
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));

		cpuTickT();
		BYTE bit0 = dataFromMemory & 0b00000001;
		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);
		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x0F: // RRC A (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE bit0 = registerContent & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit0 << 7);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}

	// RL instructions (0x10-0x17) 
	case 0x10: // RL B (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | FCARRY);
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x11: // RL C (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | FCARRY);
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x12: // RL D (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | FCARRY);
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x13: // RL E (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | FCARRY);
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x14: // RL H (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | FCARRY);
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x15: // RL L (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | FCARRY);
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x16: // RL (HL) (15T = 4+4+4+3)
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));

		cpuTickT();
		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = dataFromMemory & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | FCARRY);
		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x17: // RL A (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent << 1) | FCARRY);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x18:	// RR B (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		BYTE FCARRY = pPacMan_flags->FCARRY;

		pPacMan_flags->FCARRY = registerContent & 0b00000001;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | FCARRY << 7);
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x19:	// RR C (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		BYTE FCARRY = pPacMan_flags->FCARRY;

		pPacMan_flags->FCARRY = registerContent & 0b00000001;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | FCARRY << 7);
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x1A:	// RR D (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		BYTE FCARRY = pPacMan_flags->FCARRY;

		pPacMan_flags->FCARRY = registerContent & 0b00000001;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | FCARRY << 7);
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x1B:	// RR E (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		BYTE FCARRY = pPacMan_flags->FCARRY;

		pPacMan_flags->FCARRY = registerContent & 0b00000001;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | FCARRY << 7);
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x1C:	// RR H (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		BYTE FCARRY = pPacMan_flags->FCARRY;

		pPacMan_flags->FCARRY = registerContent & 0b00000001;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | FCARRY << 7);
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x1D:	// RR L (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE FCARRY = pPacMan_flags->FCARRY;

		pPacMan_flags->FCARRY = registerContent & 0b00000001;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | FCARRY << 7);
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x1E: // RR (HL) (15T = 4+4+4+3)
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		
		cpuTickT();
		BYTE FCARRY = pPacMan_flags->FCARRY;
		pPacMan_flags->FCARRY = dataFromMemory & 0b00000001;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);
		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x1F:	// RR A (8T total)
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE FCARRY = pPacMan_flags->FCARRY;

		pPacMan_flags->FCARRY = registerContent & 0b00000001;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | FCARRY << 7);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x20:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1);
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x21:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1);
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x22:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1);
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x23:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1);
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x24:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1);
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x25:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1);
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x26:
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		
		cpuTickT();
		BYTE bit7 = dataFromMemory & 0b10000000;
		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1);
		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x27:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x28:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		BYTE bit7 = registerContent & 0b10000000;

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit7);
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x29:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		BYTE bit7 = registerContent & 0b10000000;

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit7);
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x2A:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		BYTE bit7 = registerContent & 0b10000000;

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit7);
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x2B:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		BYTE bit7 = registerContent & 0b10000000;

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit7);
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x2C:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		BYTE bit7 = registerContent & 0b10000000;

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit7);
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x2D:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE bit7 = registerContent & 0b10000000;

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit7);
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x2E:
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		
		cpuTickT();
		BYTE bit7 = dataFromMemory & 0b10000000;
		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit7);
		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x2F:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE bit7 = registerContent & 0b10000000;

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = ((registerContent >> 1) | bit7);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x30:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1) | 0x01;
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x31:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1) | 0x01;
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x32:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1) | 0x01;
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x33:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1) | 0x01;
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x34:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1) | 0x01;
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;;
	}
	case 0x35:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1) | 0x01;
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x36:
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		
		cpuTickT();
		BYTE bit7 = dataFromMemory & 0b10000000;
		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1) | 0x01;
		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x37:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE bit7 = registerContent & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent << 1) | 0x01;
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x38:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent >> 1);
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x39:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent >> 1);
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x3A:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent >> 1);
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x3B:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent >> 1);
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x3C:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent >> 1);
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x3D:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent >> 1);
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x3E:
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));

		cpuTickT();
		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory >> 1);
		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x3F:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);

		if (registerContent & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		registerContent = (registerContent >> 1);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		processSZPFlags((byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x40:
	case 0x50:
	case 0x60:
	case 0x70:
	case 0x48:
	case 0x58:
	case 0x68:
	case 0x78:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		processSZPFlags((registerContent& bitUnderTest)); // SST needs this (so Parity and Sign bits are set, its just not documented properly)
		if ((registerContent & bitUnderTest) == bitUnderTest)
		{
			pPacMan_flags->FZERO = 0;
		}
		else
		{
			pPacMan_flags->FZERO = 1;
		}
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x41:
	case 0x51:
	case 0x61:
	case 0x71:
	case 0x49:
	case 0x59:
	case 0x69:
	case 0x79:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		processSZPFlags((registerContent& bitUnderTest)); // SST needs this (so Parity and Sign bits are set, its just not documented properly)
		if ((registerContent & bitUnderTest) == bitUnderTest)
		{
			pPacMan_flags->FZERO = 0;
		}
		else
		{
			pPacMan_flags->FZERO = 1;
		}
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x42:
	case 0x52:
	case 0x62:
	case 0x72:
	case 0x4A:
	case 0x5A:
	case 0x6A:
	case 0x7A:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		processSZPFlags((registerContent& bitUnderTest)); // SST needs this (so Parity and Sign bits are set, its just not documented properly)
		if ((registerContent & bitUnderTest) == bitUnderTest)
		{
			pPacMan_flags->FZERO = 0;
		}
		else
		{
			pPacMan_flags->FZERO = 1;
		}
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x43:
	case 0x53:
	case 0x63:
	case 0x73:
	case 0x4B:
	case 0x5B:
	case 0x6B:
	case 0x7B:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		processSZPFlags((registerContent& bitUnderTest)); // SST needs this (so Parity and Sign bits are set, its just not documented properly)
		if ((registerContent & bitUnderTest) == bitUnderTest)
		{
			pPacMan_flags->FZERO = 0;
		}
		else
		{
			pPacMan_flags->FZERO = 1;
		}
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x44:
	case 0x54:
	case 0x64:
	case 0x74:
	case 0x4C:
	case 0x5C:
	case 0x6C:
	case 0x7C:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		processSZPFlags((registerContent& bitUnderTest)); // SST needs this (so Parity and Sign bits are set, its just not documented properly)
		if ((registerContent & bitUnderTest) == bitUnderTest)
		{
			pPacMan_flags->FZERO = 0;
		}
		else
		{
			pPacMan_flags->FZERO = 1;
		}
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x45:
	case 0x55:
	case 0x65:
	case 0x75:
	case 0x4D:
	case 0x5D:
	case 0x6D:
	case 0x7D:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		processSZPFlags((registerContent& bitUnderTest)); // SST needs this (so Parity and Sign bits are set, its just not documented properly)
		if ((registerContent & bitUnderTest) == bitUnderTest)
		{
			pPacMan_flags->FZERO = 0;
		}
		else
		{
			pPacMan_flags->FZERO = 1;
		}
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x46:
	case 0x56:
	case 0x66:
	case 0x76:
	case 0x4E:
	case 0x5E:
	case 0x6E:
	case 0x7E:
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));

		cpuTickT();
		processSZPFlags((dataFromMemory & bitUnderTest)); // SST needs this (so Parity and Sign bits are set, its just not documented properly)
		if ((dataFromMemory & bitUnderTest) == bitUnderTest)
		{
			pPacMan_flags->FZERO = 0;
		}
		else
		{
			pPacMan_flags->FZERO = 1;
		}
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)(cpuReadRegister(REGISTER_TYPE::RT_WZ) >> EIGHT));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x47:
	case 0x57:
	case 0x67:
	case 0x77:
	case 0x4F:
	case 0x5F:
	case 0x6F:
	case 0x7F:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		processSZPFlags((registerContent& bitUnderTest)); // SST needs this (so Parity and Sign bits are set, its just not documented properly)
		if ((registerContent & bitUnderTest) == bitUnderTest)
		{
			pPacMan_flags->FZERO = 0;
		}
		else
		{
			pPacMan_flags->FZERO = 1;
		}
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (byte)registerContent);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x80:
	case 0x90:
	case 0xA0:
	case 0xB0:
	case 0x88:
	case 0x98:
	case 0xA8:
	case 0xB8:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		UNSETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		BREAK;
	}
	case 0x81:
	case 0x91:
	case 0xA1:
	case 0xB1:
	case 0x89:
	case 0x99:
	case 0xA9:
	case 0xB9:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		UNSETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		BREAK;
	}
	case 0x82:
	case 0x92:
	case 0xA2:
	case 0xB2:
	case 0x8A:
	case 0x9A:
	case 0xAA:
	case 0xBA:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		UNSETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		BREAK;
	}
	case 0x83:
	case 0x93:
	case 0xA3:
	case 0xB3:
	case 0x8B:
	case 0x9B:
	case 0xAB:
	case 0xBB:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		UNSETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		BREAK;
	}
	case 0x84:
	case 0x94:
	case 0xA4:
	case 0xB4:
	case 0x8C:
	case 0x9C:
	case 0xAC:
	case 0xBC:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		UNSETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		BREAK;
	}
	case 0x85:
	case 0x95:
	case 0xA5:
	case 0xB5:
	case 0x8D:
	case 0x9D:
	case 0xAD:
	case 0xBD:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		UNSETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		BREAK;
	}
	case 0x86:
	case 0x96:
	case 0xA6:
	case 0xB6:
	case 0x8E:
	case 0x9E:
	case 0xAE:
	case 0xBE:
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));

		cpuTickT();
		UNSETBIT(dataFromMemory, theBit);

		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		BREAK;
	}
	case 0x87:
	case 0x97:
	case 0xA7:
	case 0xB7:
	case 0x8F:
	case 0x9F:
	case 0xAF:
	case 0xBF:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		UNSETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		BREAK;
	}
	case 0xC0:
	case 0xD0:
	case 0xE0:
	case 0xF0:
	case 0xC8:
	case 0xD8:
	case 0xE8:
	case 0xF8:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		SETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
		BREAK;
	}
	case 0xC1:
	case 0xD1:
	case 0xE1:
	case 0xF1:
	case 0xC9:
	case 0xD9:
	case 0xE9:
	case 0xF9:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		SETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
		BREAK;
	}
	case 0xC2:
	case 0xD2:
	case 0xE2:
	case 0xF2:
	case 0xCA:
	case 0xDA:
	case 0xEA:
	case 0xFA:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		SETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
		BREAK;
	}
	case 0xC3:
	case 0xD3:
	case 0xE3:
	case 0xF3:
	case 0xCB:
	case 0xDB:
	case 0xEB:
	case 0xFB:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		SETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
		BREAK;
	}
	case 0xC4:
	case 0xD4:
	case 0xE4:
	case 0xF4:
	case 0xCC:
	case 0xDC:
	case 0xEC:
	case 0xFC:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		SETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
		BREAK;
	}
	case 0xC5:
	case 0xD5:
	case 0xE5:
	case 0xF5:
	case 0xCD:
	case 0xDD:
	case 0xED:
	case 0xFD:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		SETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
		BREAK;
	}
	case 0xC6:
	case 0xD6:
	case 0xE6:
	case 0xF6:
	case 0xCE:
	case 0xDE:
	case 0xEE:
	case 0xFE:
	{
		// Memory read (4T)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));

		cpuTickT();
		SETBIT(dataFromMemory, theBit);

		// Memory write (3T)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		BREAK;
	}
	case 0xC7:
	case 0xD7:
	case 0xE7:
	case 0xF7:
	case 0xCF:
	case 0xDF:
	case 0xEF:
	case 0xFF:
	{
		BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		SETBIT(registerContent, theBit);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
		BREAK;
	}
	default:
	{
		unimplementedInstruction();
		BREAK;
	}
	}
}

// Should be called only from "performOperation"
void pacMan_t::process0xDD0xFD(REGISTER_TYPE r, REGISTER_TYPE rh, REGISTER_TYPE rl)
{
	auto operationResult = 0;
	auto dataFromMemory = 0;
	auto lowerDataFromMemory = 0;
	auto higherDataFromMemory = 0;

	cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();	// M2: Read DD or FD opcode (4T)
	operationResult = readRawMemoryFromCPU(pPacMan_registers->pc, YES);
	incrementR();
	pPacMan_registers->pc += 1;

	switch (operationResult)
	{
	case 0x09:
	{ // ADD I,BC (15T = M1:4 + M2:4 + ALU:7)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(r) + ONE));

		operationResult = cpuReadRegister(r) + cpuReadRegister(REGISTER_TYPE::RT_BC);

		cpuTickT(); cpuTickT(); cpuTickT();

		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(r),
			cpuReadRegister(REGISTER_TYPE::RT_BC),
			false,
			false
		);
		cpuSetRegister(r, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x19:
	{ // ADD I,DE (15T = M1:4 + M2:4 + ALU:7)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(r) + ONE));

		operationResult = cpuReadRegister(r) + cpuReadRegister(REGISTER_TYPE::RT_DE);

		cpuTickT(); cpuTickT(); cpuTickT();

		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(r),
			cpuReadRegister(REGISTER_TYPE::RT_DE),
			false,
			false
		);
		cpuSetRegister(r, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x21:
	{ // LD I,nn (14T = M1:4 + M2:4 + M3:3 + M4:3)
		// M3: low byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		// M4: high byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		operationResult = ((high << 8) | low);
		cpuSetRegister(r, operationResult);
		BREAK;
	}
	case 0x22:
	{ // LD (nn),I (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// M3: low address byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int addrLow = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		// M4: high address byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int addrHigh = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		operationResult = ((addrHigh << 8) | addrLow);
		uint16_t contentsOfI = cpuReadRegister(r);
		// M5: write I low byte
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(operationResult, (BYTE)(contentsOfI & 0x00FF));
		// M6: write I high byte
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(operationResult + 1, (BYTE)((contentsOfI & 0xFF00) >> 8));
		// Based on SST
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(operationResult + ONE));
		BREAK;
	}
	case 0x23:
	{ // INC I (10T = M1:4 + M2:4 + internal:2)
		// Internal 16-bit increment (2T)
		cpuTickT(); cpuTickT();

		operationResult = cpuReadRegister(r) + 1;
		cpuSetRegister(r, operationResult);
		BREAK;
	}
	case 0x24:
	{ // INC IH (8T = M1:4 + M2:4)
		operationResult = cpuReadRegister(rh) + 1;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(rh),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(rh, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x25:
	{ // DEC IH (8T = M1:4 + M2:4)
		operationResult = cpuReadRegister(rh) - 1;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(rh),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(rh, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x26:
	{ // LD IH,n (11T = M1:4 + M2:4 + M3:3)
		// M3: immediate byte
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(rh, dataFromMemory);
		BREAK;
	}
	case 0x29:
	{ // ADD I,I (15T = M1:4 + M2:4 + ALU:7)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(r) + ONE));

		operationResult = cpuReadRegister(r) + cpuReadRegister(r);

		cpuTickT(); cpuTickT(); cpuTickT();

		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(r),
			cpuReadRegister(r),
			false,
			false
		);
		cpuSetRegister(r, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x2A:
	{ // LD I,(nn) (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// M3: low address byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int addrLow = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		// M4: high address byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int addrHigh = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		operationResult = ((addrHigh << 8) | addrLow);
		// M5: read low byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int low = readRawMemoryFromCPU(operationResult);
		// M6: read high byte
		cpuTickT(); cpuTickT(); cpuTickT();
		int high = readRawMemoryFromCPU(operationResult + 1);
		cpuSetRegister(r, ((high << 8) | low));
		// Based on SST
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(operationResult + ONE));
		BREAK;
	}
	case 0x2B:
	{ // DEC I (10T = M1:4 + M2:4 + internal:2)
		// Internal 16-bit decrement (2T)
		cpuTickT(); cpuTickT();

		operationResult = cpuReadRegister(r) - 1;
		cpuSetRegister(r, operationResult);
		BREAK;
	}
	case 0x2C:
	{ // INC IL (8T = M1:4 + M2:4)
		operationResult = cpuReadRegister(rl) + 1;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(rl),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(rl, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x2D:
	{ // DEC IL (8T = M1:4 + M2:4)
		operationResult = cpuReadRegister(rl) - 1;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(rl),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(rl, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x2E:
	{ // LD IL,n (11T = M1:4 + M2:4 + M3:3)
		// M3: immediate byte
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuSetRegister(rl, dataFromMemory);
		BREAK;
	}
	case 0x34:
	{ // INC (I+d) (23T)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		auto temp = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		processFlagsFor8BitAdditionOperation
		(
			(byte)temp,
			(byte)1,
			false,
			false
		);
		cpuTickT();
		operationResult = temp + 1;
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x35:
	{ // DEC (I+d) (23T)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		auto temp = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		processFlagsFor8BitSubtractionOperation
		(
			(byte)temp,
			(byte)1,
			false,
			false
		);
		cpuTickT();
		operationResult = temp - 1;
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x36:
	{ // LD (I+d),n (19T)
		// M3: displacement byte
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)dataFromMemory);
		BREAK;
	}
	case 0x39:
	{ // ADD I,SP (15T total, first 8T already handled)
		// M3: Internal 16-bit addition – 4T
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();

		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(r) + ONE));

		operationResult = cpuReadRegister(r) + cpuReadRegister(REGISTER_TYPE::RT_SP);

		// M4: Internal timing / write-back – 3T
		cpuTickT(); cpuTickT(); cpuTickT();

		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(r),
			cpuReadRegister(REGISTER_TYPE::RT_SP),
			false,
			false
		);

		cpuSetRegister(r, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x40:
	{
		cpuSetRegister(REGISTER_TYPE::RT_B, cpuReadRegister(REGISTER_TYPE::RT_B));
		BREAK;
	}
	case 0x41:
	{
		cpuSetRegister(REGISTER_TYPE::RT_B, cpuReadRegister(REGISTER_TYPE::RT_C));
		BREAK;
	}
	case 0x42:
	{
		cpuSetRegister(REGISTER_TYPE::RT_B, cpuReadRegister(REGISTER_TYPE::RT_D));
		BREAK;
	}
	case 0x43:
	{
		cpuSetRegister(REGISTER_TYPE::RT_B, cpuReadRegister(REGISTER_TYPE::RT_E));
		BREAK;
	}
	case 0x44:
	{
		cpuSetRegister(REGISTER_TYPE::RT_B, cpuReadRegister(rh));
		BREAK;
	}
	case 0x45:
	{
		cpuSetRegister(REGISTER_TYPE::RT_B, cpuReadRegister(rl));
		BREAK;
	}
	case 0x46:
	{ // LD B,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);

		cpuTickT(); cpuTickT();	cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));

		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		BREAK;
	}
	case 0x47:
	{
		cpuSetRegister(REGISTER_TYPE::RT_B, cpuReadRegister(REGISTER_TYPE::RT_A));
		BREAK;
	}
	case 0x48:
	{
		cpuSetRegister(REGISTER_TYPE::RT_C, cpuReadRegister(REGISTER_TYPE::RT_B));
		BREAK;
	}
	case 0x49:
	{
		cpuSetRegister(REGISTER_TYPE::RT_C, cpuReadRegister(REGISTER_TYPE::RT_C));
		BREAK;
	}
	case 0x4A:
	{
		cpuSetRegister(REGISTER_TYPE::RT_C, cpuReadRegister(REGISTER_TYPE::RT_D));
		BREAK;
	}
	case 0x4B:
	{
		cpuSetRegister(REGISTER_TYPE::RT_C, cpuReadRegister(REGISTER_TYPE::RT_E));
		BREAK;
	}
	case 0x4C:
	{
		cpuSetRegister(REGISTER_TYPE::RT_C, cpuReadRegister(rh));
		BREAK;
	}
	case 0x4D:
	{
		cpuSetRegister(REGISTER_TYPE::RT_C, cpuReadRegister(rl));
		BREAK;
	}
	case 0x4E:
	{ // LD C,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);

		cpuTickT(); cpuTickT();	cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));

		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		BREAK;
	}
	case 0x4F:
	{
		cpuSetRegister(REGISTER_TYPE::RT_C, cpuReadRegister(REGISTER_TYPE::RT_A));
		BREAK;
	}
	case 0x50:
	{
		cpuSetRegister(REGISTER_TYPE::RT_D, cpuReadRegister(REGISTER_TYPE::RT_B));
		BREAK;
	}
	case 0x51:
	{
		cpuSetRegister(REGISTER_TYPE::RT_D, cpuReadRegister(REGISTER_TYPE::RT_C));
		BREAK;
	}
	case 0x52:
	{
		cpuSetRegister(REGISTER_TYPE::RT_D, cpuReadRegister(REGISTER_TYPE::RT_D));
		BREAK;
	}
	case 0x53:
	{
		cpuSetRegister(REGISTER_TYPE::RT_D, cpuReadRegister(REGISTER_TYPE::RT_E));
		BREAK;
	}
	case 0x54:
	{
		cpuSetRegister(REGISTER_TYPE::RT_D, cpuReadRegister(rh));
		BREAK;
	}
	case 0x55:
	{
		cpuSetRegister(REGISTER_TYPE::RT_D, cpuReadRegister(rl));
		BREAK;
	}
	case 0x56:
	{ // LD D,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);

		cpuTickT(); cpuTickT();	cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));

		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		BREAK;
	}
	case 0x57:
	{
		cpuSetRegister(REGISTER_TYPE::RT_D, cpuReadRegister(REGISTER_TYPE::RT_A));
		BREAK;
	}
	case 0x58:
	{
		cpuSetRegister(REGISTER_TYPE::RT_E, cpuReadRegister(REGISTER_TYPE::RT_B));
		BREAK;
	}
	case 0x59:
	{
		cpuSetRegister(REGISTER_TYPE::RT_E, cpuReadRegister(REGISTER_TYPE::RT_C));
		BREAK;
	}
	case 0x5A:
	{
		cpuSetRegister(REGISTER_TYPE::RT_E, cpuReadRegister(REGISTER_TYPE::RT_D));
		BREAK;
	}
	case 0x5B:
	{
		cpuSetRegister(REGISTER_TYPE::RT_E, cpuReadRegister(REGISTER_TYPE::RT_E));
		BREAK;
	}
	case 0x5C:
	{
		cpuSetRegister(REGISTER_TYPE::RT_E, cpuReadRegister(rh));
		BREAK;
	}
	case 0x5D:
	{
		cpuSetRegister(REGISTER_TYPE::RT_E, cpuReadRegister(rl));
		BREAK;
	}
	case 0x5E:
	{ // LD E,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);

		cpuTickT(); cpuTickT();	cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));

		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		BREAK;
	}
	case 0x5F:
	{
		cpuSetRegister(REGISTER_TYPE::RT_E, cpuReadRegister(REGISTER_TYPE::RT_A));
		BREAK;
	}
	case 0x60:
	{
		cpuSetRegister(rh, cpuReadRegister(REGISTER_TYPE::RT_B));
		BREAK;
	}
	case 0x61:
	{
		cpuSetRegister(rh, cpuReadRegister(REGISTER_TYPE::RT_C));
		BREAK;
	}
	case 0x62:
	{
		cpuSetRegister(rh, cpuReadRegister(REGISTER_TYPE::RT_D));
		BREAK;
	}
	case 0x63:
	{
		cpuSetRegister(rh, cpuReadRegister(REGISTER_TYPE::RT_E));
		BREAK;
	}
	case 0x64:
	{
		cpuSetRegister(rh, cpuReadRegister(rh));
		BREAK;
	}
	case 0x65:
	{
		cpuSetRegister(rh, cpuReadRegister(rl));
		BREAK;
	}
	case 0x66:
	{ // LD H,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);

		cpuTickT(); cpuTickT();	cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));

		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		BREAK;
	}
	case 0x67:
	{
		cpuSetRegister(rh, cpuReadRegister(REGISTER_TYPE::RT_A));
		BREAK;
	}
	case 0x68:
	{
		cpuSetRegister(rl, cpuReadRegister(REGISTER_TYPE::RT_B));
		BREAK;
	}
	case 0x69:
	{
		cpuSetRegister(rl, cpuReadRegister(REGISTER_TYPE::RT_C));
		BREAK;
	}
	case 0x6A:
	{
		cpuSetRegister(rl, cpuReadRegister(REGISTER_TYPE::RT_D));
		BREAK;
	}
	case 0x6B:
	{
		cpuSetRegister(rl, cpuReadRegister(REGISTER_TYPE::RT_E));
		BREAK;
	}
	case 0x6C:
	{
		cpuSetRegister(rl, cpuReadRegister(rh));
		BREAK;
	}
	case 0x6D:
	{
		cpuSetRegister(rl, cpuReadRegister(rl));
		BREAK;
	}
	case 0x6E:
	{ // LD L,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);

		cpuTickT(); cpuTickT();	cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));

		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		BREAK;
	}
	case 0x6F:
	{
		cpuSetRegister(rl, cpuReadRegister(REGISTER_TYPE::RT_A));
		BREAK;
	}
	case 0x70:
	{// LD (I+d),B (19T total, first 8T already handled)
		// M3: Fetch displacement byte (d) – 3T
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)operationResult);

		BREAK;
	}
	case 0x71:
	{ // LD (I+d),C
		// M3: Fetch displacement byte (d) – 3T
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)operationResult);

		BREAK;
	}
	case 0x72:
	{ // LD (I+d),D
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)operationResult);

		BREAK;
	}
	case 0x73:
	{ // LD (I+d),E
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)operationResult);

		BREAK;
	}
	case 0x74:
	{ // LD (I+d),H
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)operationResult);

		BREAK;
	}
	case 0x75:
	{ // LD (I+d),L
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)operationResult);
		BREAK;
	}
	case 0x77:
	{ // LD (I+d),A
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)operationResult);
		BREAK;
	}
	case 0x78:
	{
		cpuSetRegister(REGISTER_TYPE::RT_A, cpuReadRegister(REGISTER_TYPE::RT_B));
		BREAK;
	}
	case 0x79:
	{
		cpuSetRegister(REGISTER_TYPE::RT_A, cpuReadRegister(REGISTER_TYPE::RT_C));
		BREAK;
	}
	case 0x7A:
	{
		cpuSetRegister(REGISTER_TYPE::RT_A, cpuReadRegister(REGISTER_TYPE::RT_D));
		BREAK;
	}
	case 0x7B:
	{
		cpuSetRegister(REGISTER_TYPE::RT_A, cpuReadRegister(REGISTER_TYPE::RT_E));
		BREAK;
	}
	case 0x7C:
	{
		cpuSetRegister(REGISTER_TYPE::RT_A, cpuReadRegister(rh));
		BREAK;
	}
	case 0x7D:
	{
		cpuSetRegister(REGISTER_TYPE::RT_A, cpuReadRegister(rl));
		BREAK;
	}
	case 0x7E:
	{ // LD A,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);

		cpuTickT(); cpuTickT();	cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));

		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		BREAK;
	}
	case 0x7F:
	{
		cpuSetRegister(REGISTER_TYPE::RT_A, cpuReadRegister(REGISTER_TYPE::RT_A));
		BREAK;
	}
	case 0x84:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(rh);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rh),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x85:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(rl);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rl),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x86:
	{ // ADD A,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + dataFromMemory;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x8C:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(rh)
			+ (uint16_t)pPacMan_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rh),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x8D:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(rl)
			+ (uint16_t)pPacMan_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rl),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x8E:
	{ // ADC A,(I+d) (19T total, first 8T already handled)

		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + dataFromMemory + (pPacMan_flags->FCARRY ? 1 : 0);;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			pPacMan_flags->FCARRY,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x94:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(rh);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rh),
			false,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x95:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(rl);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rl),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x96:
	{ // SUB A,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x9C:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(rh)
			- (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rh),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}
	case 0x9D:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(rl)
			- (uint16_t)pPacMan_flags->FCARRY;

		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rl),
			true,
			true
		);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x9E:
	{ // SBC A,(I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - dataFromMemory - (uint16_t)pPacMan_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xA4:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(rh);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xA5:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(rl);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xA6:
	{ // AND (I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) & dataFromMemory;
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xAC:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(rh);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xAD:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(rl);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xAE:
	{ // XOR (I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ dataFromMemory;
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xB4:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) | cpuReadRegister(rh);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xB5:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) | cpuReadRegister(rl);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xB6:
	{ // OR (I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) | dataFromMemory;
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xBC:
	{
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rh),
			false,
			true
		);
		updateXY(pPacMan_cpuInstance->opcode, (byte)cpuReadRegister(rh));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xBD:
	{
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(rl),
			false,
			true
		);
		updateXY(pPacMan_cpuInstance->opcode, (byte)cpuReadRegister(rl));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xBE:
	{ // CP (I+d) (19T total, first 8T already handled)
		cpuTickT(); cpuTickT(); cpuTickT();
		SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		pPacMan_registers->pc++;
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			false,
			true
		);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xCB:
	{
		pPacMan_cpuInstance->prefix2 = 0xCB;
		process0xDDCB0xFDCB(r, rh, rl);
		BREAK;
	}
	case 0xE1:
	{ // POP I (14T total, first 8T already handled)
		// M3: Pop lower byte from stack – 3T
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE lowerData = stackPop();

		// M4: Pop higher byte from stack – 3T
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE higherData = stackPop();

		// M5: Combine and store in I – 2T
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		cpuSetRegister(r, operationResult);
		BREAK;
	}
	case 0xE3:
	{ // EX (SP),I (23T total, first 8T handled outside)
		// M3: Read low byte from (SP) – 3T
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE newL = readRawMemoryFromCPU(pPacMan_registers->sp);

		// M4: Read high byte from (SP+1) – 4T
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE newH = readRawMemoryFromCPU(pPacMan_registers->sp + 1);

		cpuTickT();
		BYTE olderL = (BYTE)cpuReadRegister(rl);
		BYTE olderH = (BYTE)cpuReadRegister(rh);

		// M5: Write old I low to (SP) – 3T
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(pPacMan_registers->sp, olderL);

		// M6: Write old I high to (SP+1) – 5T
		cpuSetRegister(rh, newH);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(pPacMan_registers->sp + 1, olderH);

		// Internal cycles
		cpuTickT(); // Internal (1T)
		cpuTickT(); // Internal (1T)

		cpuSetRegister(rl, newL);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, cpuReadRegister(r));

		BREAK;
	}
	case 0xE5:
	{ // PUSH I (15T total, first 8T handled outside)
		// M3: Internal timing before memory writes – 1T
		cpuTickT();

		// M4: Write high byte to (SP-1) – 3T
		BYTE higherData = (BYTE)(cpuReadRegister(r) >> 8);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(higherData);

		// M5: Write low byte to (SP-2) – 3T
		BYTE lowerData = (BYTE)cpuReadRegister(r);
		cpuTickT(); cpuTickT(); cpuTickT();
		stackPush(lowerData);
		BREAK;
	}
	case 0xE9:
	{
		pPacMan_registers->pc = cpuReadRegister(r);
		BREAK;
	}
	default:
	{
		unimplementedInstruction();
		BREAK;
	}
	}
}

// Should be called only from "process0xDD0xFD"
void pacMan_t::process0xDDCB0xFDCB(REGISTER_TYPE r, REGISTER_TYPE rh, REGISTER_TYPE rl)
{
	auto operationResult = 0;
	auto dataFromMemory = 0;
	auto lowerDataFromMemory = 0;
	auto higherDataFromMemory = 0;

	cpuTickT(); cpuTickT(); cpuTickT();
	//incrementR();
	SBYTE relativeJump = readRawMemoryFromCPU(pPacMan_registers->pc);
	pPacMan_registers->pc += 1;
	cpuTickT(); cpuTickT(); cpuTickT();
	operationResult = readRawMemoryFromCPU(pPacMan_registers->pc);
	cpuTickT(); cpuTickT();
	pPacMan_registers->pc += 1;
	// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
	cpuSetRegister(REGISTER_TYPE::RT_WZ, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));

	BYTE bitUnderTest = 0;
	BYTE theBit = 0;

	if (
		(operationResult >= 0x40 && operationResult <= 0x47)
		||
		(operationResult >= 0x80 && operationResult <= 0x87)
		||
		(operationResult >= 0xC0 && operationResult <= 0xC7)
		)
	{
		bitUnderTest = BIT0_SET;
		theBit = BIT0;
	}
	else if (
		(operationResult >= 0x48 && operationResult <= 0x4F)
		||
		(operationResult >= 0x88 && operationResult <= 0x8F)
		||
		(operationResult >= 0xC8 && operationResult <= 0xCF)
		)
	{
		bitUnderTest = BIT1_SET;
		theBit = BIT1;
	}
	else if (
		(operationResult >= 0x50 && operationResult <= 0x57)
		||
		(operationResult >= 0x90 && operationResult <= 0x97)
		||
		(operationResult >= 0xD0 && operationResult <= 0xD7)
		)
	{
		bitUnderTest = BIT2_SET;
		theBit = BIT2;
	}
	else if (
		(operationResult >= 0x58 && operationResult <= 0x5F)
		||
		(operationResult >= 0x98 && operationResult <= 0x9F)
		||
		(operationResult >= 0xD8 && operationResult <= 0xDF)
		)
	{
		bitUnderTest = BIT3_SET;
		theBit = BIT3;
	}
	else if (
		(operationResult >= 0x60 && operationResult <= 0x67)
		||
		(operationResult >= 0xA0 && operationResult <= 0xA7)
		||
		(operationResult >= 0xE0 && operationResult <= 0xE7)
		)
	{
		bitUnderTest = BIT4_SET;
		theBit = BIT4;
	}
	else if (
		(operationResult >= 0x68 && operationResult <= 0x6F)
		||
		(operationResult >= 0xA8 && operationResult <= 0xAF)
		||
		(operationResult >= 0xE8 && operationResult <= 0xEF)
		)
	{
		bitUnderTest = BIT5_SET;
		theBit = BIT5;
	}
	else if (
		(operationResult >= 0x70 && operationResult <= 0x77)
		||
		(operationResult >= 0xB0 && operationResult <= 0xB7)
		||
		(operationResult >= 0xF0 && operationResult <= 0xF7)
		)
	{
		bitUnderTest = BIT6_SET;
		theBit = BIT6;
	}
	else if (
		(operationResult >= 0x78 && operationResult <= 0x7F)
		||
		(operationResult >= 0xB8 && operationResult <= 0xBF)
		||
		(operationResult >= 0xF8 && operationResult <= 0xFF)
		)
	{
		bitUnderTest = BIT7_SET;
		theBit = BIT7;
	}

	switch (operationResult)
	{
	case 0x00: // RL (I+d), B
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x01: // RL (I+d), C
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x02: // RL (I+d), D
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x03: // RL (I+d), E
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x04: // RL (I+d), H
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x05: // RL (I+d), L
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x06: // RL (I+d) -> write back
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));

		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x07: // RL (I+d), A
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x08:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit0 = dataFromMemory & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x09:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit0 = dataFromMemory & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x0A:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit0 = dataFromMemory & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x0B:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit0 = dataFromMemory & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x0C:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit0 = dataFromMemory & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x0D:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit0 = dataFromMemory & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x0E:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit0 = dataFromMemory & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);

		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x0F:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit0 = dataFromMemory & 0b00000001;

		pPacMan_flags->FCARRY = bit0;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (byte)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x10:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | FCARRY);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x11:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | FCARRY);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x12:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | FCARRY);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x13:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | FCARRY);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x14:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | FCARRY);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x15:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | FCARRY);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x16:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | FCARRY);

		cpuTickT(); cpuTickT(); cpuTickT();

		// (I+d) -> write back (copy of 0x14 but explicit)
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x17:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory << 1) | FCARRY);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x18:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x19:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x1A:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x1B:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x1C:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x1D:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x1E:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x1F:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE FCARRY = pPacMan_flags->FCARRY;
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x20:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x21:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x22:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x23:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x24:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x25:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x26:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1);

		cpuTickT(); cpuTickT(); cpuTickT();

		// Write back to (I+d)
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x27:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x28:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x29:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x2A:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x2B:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x2C:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x2D:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x2E:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit7);

		cpuTickT(); cpuTickT(); cpuTickT();

		// Write back to (I+d)
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x2F:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0b10000000;

		if (dataFromMemory & 0x01)
		{
			pPacMan_flags->FCARRY = 1;
		}
		else
		{
			pPacMan_flags->FCARRY = 0;
		}
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = ((dataFromMemory >> 1) | bit7);

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x30:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1) | 0x01;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x31:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1) | 0x01;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x32:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1) | 0x01;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x33:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1) | 0x01;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x34:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1) | 0x01;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x35:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1) | 0x01;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x36:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1) | 0x01;

		cpuTickT(); cpuTickT(); cpuTickT();

		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x37:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE bit7 = dataFromMemory & 0x80;

		pPacMan_flags->FCARRY = (bit7 >> 7);
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory = (dataFromMemory << 1) | 0x01;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x38:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory >>= 1;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x39:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory >>= 1;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x3A:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();

		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory >>= 1;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x3B:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory >>= 1;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x3C:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory >>= 1;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x3D:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory >>= 1;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x3E:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory >>= 1;

		cpuTickT(); cpuTickT(); cpuTickT();

		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x3F:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		BYTE lsb = dataFromMemory & 0x01;

		pPacMan_flags->FCARRY = lsb;
		pPacMan_flags->FHALFCARRY = 0;
		pPacMan_flags->FNEGATIVE = 0;

		dataFromMemory >>= 1;

		cpuTickT(); cpuTickT(); cpuTickT();

		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);

		processSZPFlags((byte)dataFromMemory);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)dataFromMemory);
		pPacMan_cpuInstance->possFlag = YES;

		BREAK;
	}

	case 0x40:
	case 0x50:
	case 0x60:
	case 0x70:
	case 0x48:
	case 0x58:
	case 0x68:
	case 0x78:
	case 0x41:
	case 0x51:
	case 0x61:
	case 0x71:
	case 0x49:
	case 0x59:
	case 0x69:
	case 0x79:
	case 0x42:
	case 0x52:
	case 0x62:
	case 0x72:
	case 0x4A:
	case 0x5A:
	case 0x6A:
	case 0x7A:
	case 0x43:
	case 0x53:
	case 0x63:
	case 0x73:
	case 0x4B:
	case 0x5B:
	case 0x6B:
	case 0x7B:
	case 0x44:
	case 0x54:
	case 0x64:
	case 0x74:
	case 0x4C:
	case 0x5C:
	case 0x6C:
	case 0x7C:
	case 0x45:
	case 0x55:
	case 0x65:
	case 0x75:
	case 0x4D:
	case 0x5D:
	case 0x6D:
	case 0x7D:
	case 0x46:
	case 0x56:
	case 0x66:
	case 0x76:
	case 0x4E:
	case 0x5E:
	case 0x6E:
	case 0x7E:
	case 0x47:
	case 0x57:
	case 0x67:
	case 0x77:
	case 0x4F:
	case 0x5F:
	case 0x6F:
	case 0x7F:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		processSZPFlags((dataFromMemory & bitUnderTest)); // SST needs this (so Parity and Sign bits are set, its just not documented properly)
		if ((dataFromMemory & bitUnderTest) == bitUnderTest)
		{
			pPacMan_flags->FZERO = 0;
		}
		else
		{
			pPacMan_flags->FZERO = 1;
		}
		pPacMan_flags->FNEGATIVE = 0;
		pPacMan_flags->FHALFCARRY = 1;
		updateXY(pPacMan_cpuInstance->opcode, (int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump) >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x80:
	case 0x90:
	case 0xA0:
	case 0xB0:
	case 0x88:
	case 0x98:
	case 0xA8:
	case 0xB8:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		UNSETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		BREAK;
	}
	case 0x81:
	case 0x91:
	case 0xA1:
	case 0xB1:
	case 0x89:
	case 0x99:
	case 0xA9:
	case 0xB9:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		UNSETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		BREAK;
	}
	case 0x82:
	case 0x92:
	case 0xA2:
	case 0xB2:
	case 0x8A:
	case 0x9A:
	case 0xAA:
	case 0xBA:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		UNSETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		BREAK;
	}
	case 0x83:
	case 0x93:
	case 0xA3:
	case 0xB3:
	case 0x8B:
	case 0x9B:
	case 0xAB:
	case 0xBB:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		UNSETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		BREAK;
	}
	case 0x84:
	case 0x94:
	case 0xA4:
	case 0xB4:
	case 0x8C:
	case 0x9C:
	case 0xAC:
	case 0xBC:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		UNSETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		BREAK;
	}
	case 0x85:
	case 0x95:
	case 0xA5:
	case 0xB5:
	case 0x8D:
	case 0x9D:
	case 0xAD:
	case 0xBD:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		UNSETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		BREAK;
	}
	case 0x86:
	case 0x96:
	case 0xA6:
	case 0xB6:
	case 0x8E:
	case 0x9E:
	case 0xAE:
	case 0xBE:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		UNSETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)dataFromMemory);
		BREAK;
	}
	case 0x87:
	case 0x97:
	case 0xA7:
	case 0xB7:
	case 0x8F:
	case 0x9F:
	case 0xAF:
	case 0xBF:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		UNSETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		BREAK;
	}
	case 0xC0:
	case 0xD0:
	case 0xE0:
	case 0xF0:
	case 0xC8:
	case 0xD8:
	case 0xE8:
	case 0xF8:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		SETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		BREAK;
	}
	case 0xC1:
	case 0xD1:
	case 0xE1:
	case 0xF1:
	case 0xC9:
	case 0xD9:
	case 0xE9:
	case 0xF9:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		SETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		BREAK;
	}
	case 0xC2:
	case 0xD2:
	case 0xE2:
	case 0xF2:
	case 0xCA:
	case 0xDA:
	case 0xEA:
	case 0xFA:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		SETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		BREAK;
	}
	case 0xC3:
	case 0xD3:
	case 0xE3:
	case 0xF3:
	case 0xCB:
	case 0xDB:
	case 0xEB:
	case 0xFB:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		SETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		BREAK;
	}
	case 0xC4:
	case 0xD4:
	case 0xE4:
	case 0xF4:
	case 0xCC:
	case 0xDC:
	case 0xEC:
	case 0xFC:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		SETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		BREAK;
	}
	case 0xC5:
	case 0xD5:
	case 0xE5:
	case 0xF5:
	case 0xCD:
	case 0xDD:
	case 0xED:
	case 0xFD:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		SETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		BREAK;
	}
	case 0xC6:
	case 0xD6:
	case 0xE6:
	case 0xF6:
	case 0xCE:
	case 0xDE:
	case 0xEE:
	case 0xFE:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		SETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), (BYTE)dataFromMemory);
		BREAK;
	}
	case 0xC7:
	case 0xD7:
	case 0xE7:
	case 0xF7:
	case 0xCF:
	case 0xDF:
	case 0xEF:
	case 0xFF:
	{
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump));
		cpuTickT();
		SETBIT(dataFromMemory, theBit);
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU((int32_t)((int32_t)cpuReadRegister(r) + (SBYTE)relativeJump), dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		BREAK;
	}
	default:
	{
		unimplementedInstruction();
		BREAK;
	}
	}
}

// Should be called only from "performOperation"
void pacMan_t::process0xED()
{
	auto operationResult = 0;
	auto dataFromMemory = 0;
	auto lowerDataFromMemory = 0;
	auto higherDataFromMemory = 0;

	cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();	// M2: Read ED opcode (4T)
	auto opcode = readRawMemoryFromCPU(pPacMan_registers->pc, YES);
	incrementR();
	pPacMan_registers->pc += 1;

	switch (opcode)
	{
	case 0x42:
	{ // SBC HL,BC (15T = M1:4 + M2:4 + ALU:7)
		// Internal 16-bit ALU operation (7T)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL)
			- (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_BC)
			- (uint32_t)pPacMan_flags->FCARRY;
		cpuTickT(); cpuTickT(); cpuTickT();
		processFlagsFor16BitSubtractionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_BC),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x43:
	{ // LD (nn),BC (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// M3: low address byte
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t lowerDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		// M4: high address byte
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t pointerToMemory = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_BC);
		// M5: write BC low byte
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(pointerToMemory, (BYTE)(operationResult & 0x00FF));
		// M6: write BC high byte
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(pointerToMemory + 1, (BYTE)((operationResult & 0xFF00) >> 8));
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pointerToMemory + 1);
		BREAK;
	}
	case 0x44:
	{ // NEG (8T = M1:4 + M2:4)
		SBYTE operationResult = 0 - cpuReadRegister(REGISTER_TYPE::RT_A);
		processFlagsFor8BitSubtractionOperation
		(
			0,
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, (BYTE)operationResult);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x46:
	{ // IM 0 (8T = M1:4 + M2:4)
		pPacMan_instance->pacMan_state.interruptMode = INTERRUPT_MODE::INTERRUPT_MODE_0;
		BREAK;
	}
	case 0x47:
	{ // LD I,A (9T = M1:4 + M2:4 + internal:1)
		// Internal operation (1T)
		cpuTickT();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_I, operationResult);
		BREAK;
	}
	case 0x4A:
	{ // ADC HL,BC (15T = M1:4 + M2:4 + ALU:7)
		// Internal 16-bit ALU operation (7T)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL)
			+ (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_BC)
			+ (uint32_t)pPacMan_flags->FCARRY;
		cpuTickT(); cpuTickT(); cpuTickT();
		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_BC),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x4B:
	{ // LD BC,(nn) (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// M3: low address byte
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t lowerDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;
		// M4: high address byte
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc++;

		uint16_t pointerToMemory = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		// M5: read low byte
		cpuTickT(); cpuTickT(); cpuTickT();
		auto low = readRawMemoryFromCPU(pointerToMemory);
		// M6: read high byte
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = low | (readRawMemoryFromCPU(pointerToMemory + 1) << 8);
		cpuSetRegister(REGISTER_TYPE::RT_BC, operationResult);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pointerToMemory + 1);
		BREAK;
	}
	case 0x4F:
	{ // LD R,A (9T = M1:4 + M2:4 + internal:1)
		// Internal operation (1T)
		cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_R, operationResult);
		BREAK;
	}
	case 0x52:
	{ // SBC HL,DE (15T = M1:4 + M2:4 + ALU:7)
		// Internal 16-bit ALU operation (7T)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL)
			- cpuReadRegister(REGISTER_TYPE::RT_DE)
			- (uint16_t)pPacMan_flags->FCARRY;
		cpuTickT(); cpuTickT(); cpuTickT();
		processFlagsFor16BitSubtractionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_DE),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x53:
	{ // LD (nn),DE (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// Memory read cycles for address (M3:3 + M4:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t lowerDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		uint16_t pointerToMemory = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		// Memory write cycles (M5:3 + M6:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_DE);
		writeRawMemoryFromCPU(pointerToMemory, (BYTE)(operationResult & 0x00FF));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(pointerToMemory + 1, (BYTE)((operationResult & 0xFF00) >> 8));
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pointerToMemory + 1);
		BREAK;
	}
	case 0x56:
	{ // IM 1 (8T = M1:4 + M2:4)
		// No additional cycles needed (M1:4 + M2:4 already handled)
		pPacMan_instance->pacMan_state.interruptMode = INTERRUPT_MODE::INTERRUPT_MODE_1;
		BREAK;
	}
	case 0x57:
	{ // LD A,I (9T = M1:4 + M2:4 + internal:1)
		// Internal operation (1T)
		cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_I);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		if ((operationResult & 0x80) == 0x80)
		{
			pPacMan_flags->FSIGN = ONE;
		}
		else
		{
			pPacMan_flags->FSIGN = ZERO;
		}
		if ((operationResult & 0xFF) == 0x00)
		{
			pPacMan_flags->FZERO = ONE;
		}
		else
		{
			pPacMan_flags->FZERO = ZERO;
		}
		pPacMan_flags->F_OF_PARITY = pPacMan_registers->iff2;
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		updateXY(pPacMan_cpuInstance->opcode, operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_P, ONE);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x5A:
	{ // ADC HL,DE (15T = M1:4 + M2:4 + ALU:7)
		// Internal 16-bit ALU operation (7T)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL)
			+ (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_DE)
			+ (uint32_t)pPacMan_flags->FCARRY;
		cpuTickT(); cpuTickT(); cpuTickT();
		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_DE),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x5B:
	{ // LD DE,(nn) (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// Memory read cycles for address (M3:3 + M4:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t lowerDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		uint16_t pointerToMemory = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		// Memory read cycles for data (M5:3 + M6:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = readRawMemoryFromCPU(pointerToMemory);
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult |= (readRawMemoryFromCPU(pointerToMemory + 1) << 8);
		cpuSetRegister(REGISTER_TYPE::RT_DE, operationResult);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pointerToMemory + 1);
		BREAK;
	}
	case 0x5E:
	{ // IM 2 (8T = M1:4 + M2:4)
		// No additional cycles needed (M1:4 + M2:4 already handled)
		pPacMan_instance->pacMan_state.interruptMode = INTERRUPT_MODE::INTERRUPT_MODE_2;
		BREAK;
	}
	case 0x5F:
	{ // LD A,R (9T = M1:4 + M2:4 + internal:1)
		// Internal operation (1T)
		cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_R);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);

		if ((operationResult & 0x80) == 0x80)
		{
			pPacMan_flags->FSIGN = ONE;
		}
		else
		{
			pPacMan_flags->FSIGN = ZERO;
		}
		if ((operationResult & 0xFF) == 0x00)
		{
			pPacMan_flags->FZERO = ONE;
		}
		else
		{
			pPacMan_flags->FZERO = ZERO;
		}
		pPacMan_flags->F_OF_PARITY = pPacMan_registers->iff2;
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		updateXY(pPacMan_cpuInstance->opcode, operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_P, ONE);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x62:
	{ // SBC HL,HL (15T = M1:4 + M2:4 + ALU:7)
		// Internal 16-bit ALU operation (7T)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL)
			- (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL)
			- (uint32_t)pPacMan_flags->FCARRY;
		cpuTickT(); cpuTickT(); cpuTickT();
		processFlagsFor16BitSubtractionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x63:
	{ // LD (nn),HL (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// Memory read cycles for address (M3:3 + M4:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t lowerDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		uint16_t pointerToMemory = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		// Memory write cycles (M5:3 + M6:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL);
		writeRawMemoryFromCPU(pointerToMemory, (BYTE)(operationResult & 0x00FF));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(pointerToMemory + 1, (BYTE)((operationResult & 0xFF00) >> 8));
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pointerToMemory + 1);
		BREAK;
	}
	case 0x67:
	{ // RRD (18T)
		// Memory read cycle
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE registerAContent = TO_UINT8(cpuReadRegister(REGISTER_TYPE::RT_A));
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Memory write cycle
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		BYTE datatoMemory = ((dataFromMemory & 0xF0) >> 4) | ((registerAContent & 0x0F) << 4);
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), datatoMemory);
		// Internal processing
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE registerAContentToBeSet = (registerAContent & 0xF0) | (dataFromMemory & 0x0F);
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		processSZPFlags
		(
			(byte)registerAContentToBeSet
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerAContentToBeSet);
		updateXY(pPacMan_cpuInstance->opcode, registerAContentToBeSet);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x6A:
	{ // ADC HL,HL (15T = M1:4 + M2:4 + ALU:7)
		// Internal 16-bit ALU operation (7T)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL)
			+ (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL)
			+ (uint32_t)pPacMan_flags->FCARRY;
		cpuTickT(); cpuTickT(); cpuTickT();
		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x6B:
	{ // LD HL,(nn) (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// Memory read cycles for address (M3:3 + M4:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t lowerDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		uint16_t pointerToMemory = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		// Memory read cycles for data (M5:3 + M6:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = readRawMemoryFromCPU(pointerToMemory);
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult |= (readRawMemoryFromCPU(pointerToMemory + 1) << 8);
		cpuSetRegister(REGISTER_TYPE::RT_HL, operationResult);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pointerToMemory + 1);
		BREAK;
	}
	case 0x6F:
	{ // RLD (18T)
		// Memory read cycle
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE registerAContent = TO_UINT8(cpuReadRegister(REGISTER_TYPE::RT_A));
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Memory write cycle
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		BYTE datatoMemory = (registerAContent & 0x0F) | ((dataFromMemory & 0x0F) << 4);
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL), datatoMemory);
		// Internal processing
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE registerAContentToBeSet = (registerAContent & 0xF0) | ((dataFromMemory & 0xF0) >> 4);
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		processSZPFlags
		(
			(byte)registerAContentToBeSet
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, registerAContentToBeSet);
		updateXY(pPacMan_cpuInstance->opcode, registerAContentToBeSet);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x72:
	{ // SBC HL,SP (15T = M1:4 + M2:4 + ALU:7)
		// Internal 16-bit ALU operation (7T)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL)
			- (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_SP)
			- (uint32_t)pPacMan_flags->FCARRY;
		cpuTickT(); cpuTickT(); cpuTickT();
		processFlagsFor16BitSubtractionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_SP),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x73:
	{ // LD (nn),SP (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// Memory read cycles for address (M3:3 + M4:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t lowerDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		uint16_t pointerToMemory = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		// Memory write cycles (M5:3 + M6:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_SP);
		writeRawMemoryFromCPU(pointerToMemory, (BYTE)(operationResult & 0x00FF));
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(pointerToMemory + 1, (BYTE)((operationResult & 0xFF00) >> 8));
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pointerToMemory + 1);
		BREAK;
	}
	case 0x7A:
	{ // ADC HL,SP (15T = M1:4 + M2:4 + ALU:7)
		// Internal 16-bit ALU operation (7T)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, (uint16_t)(cpuReadRegister(REGISTER_TYPE::RT_HL) + ONE));
		uint32_t operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL)
			+ (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_SP)
			+ (uint32_t)pPacMan_flags->FCARRY;
		cpuTickT(); cpuTickT(); cpuTickT();
		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_SP),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		updateXY(pPacMan_cpuInstance->opcode, operationResult >> EIGHT);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0x7B:
	{ // LD SP,(nn) (20T = M1:4 + M2:4 + M3:3 + M4:3 + M5:3 + M6:3)
		// Memory read cycles for address (M3:3 + M4:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t lowerDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		cpuTickT(); cpuTickT(); cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)readRawMemoryFromCPU(pPacMan_registers->pc);
		pPacMan_registers->pc += 1;
		uint16_t pointerToMemory = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		// Memory read cycles for data (M5:3 + M6:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = readRawMemoryFromCPU(pointerToMemory);
		cpuTickT(); cpuTickT(); cpuTickT();
		operationResult |= (readRawMemoryFromCPU(pointerToMemory + 1) << 8);
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, pointerToMemory + 1);
		BREAK;
	}
	case 0xA0:
	{ // LDI (16T = M1:4 + M2:4 + M3:3 + M4:5)
		// Memory read cycle (M3:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Internal processing and register updates (M4:5)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_DE), (BYTE)dataFromMemory);
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) + 1);
		cpuSetRegister(REGISTER_TYPE::RT_DE, cpuReadRegister(REGISTER_TYPE::RT_DE) + 1);
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) - 1);
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != ZERO)
		{
			pPacMan_flags->F_OF_PARITY = ONE;
		}
		else
		{
			pPacMan_flags->F_OF_PARITY = ZERO;
		}
		uint16_t temp = (cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory);
		// Copy bit 1 to bit 5
		temp = (temp & ~(1 << 5)) | ((temp & (1 << 1)) << 4);
		updateXY(pPacMan_cpuInstance->opcode, TO_UINT8(temp));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xA1:
	{ // CPI (16T = M1:4 + M2:4 + M3:3 + M4:5)
		// Memory read cycle (M3:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Internal processing and register updates (M4:5)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (uint16_t)registerAContent - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)dataFromMemory,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) + 1);
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) - 1);
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != ZERO)
		{
			pPacMan_flags->F_OF_PARITY = ONE;
		}
		else
		{
			pPacMan_flags->F_OF_PARITY = ZERO;
		}

		uint16_t temp = (uint16_t)operationResult - (uint16_t)pPacMan_flags->FHALFCARRY;
		// Copy bit 1 to bit 5
		temp = (temp & ~(1 << 5)) | ((temp & (1 << 1)) << 4);
		updateXY(pPacMan_cpuInstance->opcode, TO_UINT8(temp));
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, cpuReadRegister(REGISTER_TYPE::RT_WZ) + ONE);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xA8:
	{ // LDD (16T = M1:4 + M2:4 + M3:3 + M4:5)
		// Memory read cycle (M3:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Internal processing and register updates
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_DE), (BYTE)dataFromMemory);
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) - 1);
		cpuSetRegister(REGISTER_TYPE::RT_DE, cpuReadRegister(REGISTER_TYPE::RT_DE) - 1);
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) - 1);
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != ZERO)
		{
			pPacMan_flags->F_OF_PARITY = ONE;
		}
		else
		{
			pPacMan_flags->F_OF_PARITY = ZERO;
		}
		uint16_t temp = (cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory);
		// Copy bit 1 to bit 5
		temp = (temp & ~(1 << 5)) | ((temp & (1 << 1)) << 4);
		updateXY(pPacMan_cpuInstance->opcode, TO_UINT8(temp));
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xA9:
	{ // CPD (16T = M1:4 + M2:4 + M3:3 + M4:5)
		// Memory read cycle (M3:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Internal processing and register updates (M4:5)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (uint16_t)registerAContent - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)dataFromMemory,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) - 1);
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) - 1);
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != ZERO)
		{
			pPacMan_flags->F_OF_PARITY = ONE;
		}
		else
		{
			pPacMan_flags->F_OF_PARITY = ZERO;
		}
		uint16_t temp = (uint16_t)operationResult - (uint16_t)pPacMan_flags->FHALFCARRY;
		// Copy bit 1 to bit 5
		temp = (temp & ~(1 << 5)) | ((temp & (1 << 1)) << 4);
		updateXY(pPacMan_cpuInstance->opcode, TO_UINT8(temp));
		// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
		cpuSetRegister(REGISTER_TYPE::RT_WZ, cpuReadRegister(REGISTER_TYPE::RT_WZ) - ONE);
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xB0:
	{ // LDIR (21T when BC!=0, 16T when BC=0)
		// Memory read cycle (M3:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Internal processing and register updates (M4:5)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_DE), (BYTE)dataFromMemory);
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) + 1);
		cpuSetRegister(REGISTER_TYPE::RT_DE, cpuReadRegister(REGISTER_TYPE::RT_DE) + 1);
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) - 1);
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != 0)
		{
			pPacMan_flags->F_OF_PARITY = ONE;
			// Additional cycles for repeat condition (M5:5)
			cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
			// PC was already incremented when we entered "case 0xED" of the main instruction set, so decrement PC by 1
			// PC is incremented when we entered 0xED 0xB0, so decrement PC by 1 again...
			pPacMan_registers->pc -= TWO;
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc + ONE);
			// Refer https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
			updateXY(pPacMan_cpuInstance->opcode, ((pPacMan_registers->pc & (1 << 13)) ? (1 << 5) : 0) | ((pPacMan_registers->pc & (1 << 11)) ? (1 << 3) : 0));
		}
		else
		{
			pPacMan_flags->F_OF_PARITY = ZERO;
			uint16_t temp = (cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory);
			// Copy bit 1 to bit 5
			temp = (temp & ~(1 << 5)) | ((temp & (1 << 1)) << 4);
			updateXY(pPacMan_cpuInstance->opcode, TO_UINT8(temp));
		}
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xB1:
	{ // CPIR (21T when BC!=0 and Z=0, 16T when BC=0 or Z=1)
		// Memory read cycle (M3:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Internal processing and register updates (M4:5)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (uint16_t)registerAContent - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)dataFromMemory,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) + 1);
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) - 1);
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != ZERO)
		{
			pPacMan_flags->F_OF_PARITY = ONE;
		}
		else
		{
			pPacMan_flags->F_OF_PARITY = ZERO;
		}
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != 0 && pPacMan_flags->FZERO == 0)
		{
			// Additional cycles for repeat condition (M5:5)
			cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
			// PC was already incremented when we entered "case 0xED" of the main instruction set, so decrement PC by 1
			// PC is incremented when we entered 0xED 0xB0, so decrement PC by 1 again...
			pPacMan_registers->pc -= TWO;
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc + ONE);
			// Refer https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
			updateXY(pPacMan_cpuInstance->opcode, ((pPacMan_registers->pc& (1 << 13)) ? (1 << 5) : 0) | ((pPacMan_registers->pc & (1 << 11)) ? (1 << 3) : 0));
		}
		else
		{
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, cpuReadRegister(REGISTER_TYPE::RT_WZ) + ONE);
			uint16_t temp = (uint16_t)operationResult - (uint16_t)pPacMan_flags->FHALFCARRY;
			// Copy bit 1 to bit 5
			temp = (temp & ~(1 << 5)) | ((temp & (1 << 1)) << 4);
			updateXY(pPacMan_cpuInstance->opcode, TO_UINT8(temp));
		}
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xB8:
	{ // LDDR (21T when BC!=0, 16T when BC=0)
		// Memory read cycle (M3:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Internal processing and register updates (M4:5)
		cpuTickT(); cpuTickT(); cpuTickT();
		writeRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_DE), (BYTE)dataFromMemory);
		cpuTickT(); cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) - 1);
		cpuSetRegister(REGISTER_TYPE::RT_DE, cpuReadRegister(REGISTER_TYPE::RT_DE) - 1);
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) - 1);
		pPacMan_flags->FNEGATIVE = ZERO;
		pPacMan_flags->FHALFCARRY = ZERO;
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != 0)
		{
			pPacMan_flags->F_OF_PARITY = ONE;
			// Additional cycles for repeat condition (M5:5)
			cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
			// PC was already incremented when we entered "case 0xED" of the main instruction set, so decrement PC by 1
			// PC is incremented when we entered 0xED 0xB0, so decrement PC by 1 again...
			pPacMan_registers->pc -= TWO;
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc + ONE);
			// Refer https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
			updateXY(pPacMan_cpuInstance->opcode, ((pPacMan_registers->pc& (1 << 13)) ? (1 << 5) : 0) | ((pPacMan_registers->pc & (1 << 11)) ? (1 << 3) : 0));
		}
		else
		{
			pPacMan_flags->F_OF_PARITY = ZERO;
			uint16_t temp = (cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory);
			// Copy bit 1 to bit 5
			temp = (temp & ~(1 << 5)) | ((temp & (1 << 1)) << 4);
			updateXY(pPacMan_cpuInstance->opcode, TO_UINT8(temp));
		}
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	case 0xB9:
	{ // CPDR (21T when BC!=0 and Z=0, 16T when BC=0 or Z=1)
		// Memory read cycle (M3:3)
		cpuTickT(); cpuTickT(); cpuTickT();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		dataFromMemory = readRawMemoryFromCPU(cpuReadRegister(REGISTER_TYPE::RT_HL));
		// Internal processing and register updates (M4:5)
		cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
		operationResult = (uint16_t)registerAContent - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)dataFromMemory,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_HL) - 1);
		cpuSetRegister(REGISTER_TYPE::RT_BC, cpuReadRegister(REGISTER_TYPE::RT_BC) - 1);
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != ZERO)
		{
			pPacMan_flags->F_OF_PARITY = ONE;
		}
		else
		{
			pPacMan_flags->F_OF_PARITY = ZERO;
		}
		if (cpuReadRegister(REGISTER_TYPE::RT_BC) != 0 && pPacMan_flags->FZERO == 0)
		{
			// Additional cycles for repeat condition (M5:5)
			cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT(); cpuTickT();
			// PC was already incremented when we entered "case 0xED" of the main instruction set, so decrement PC by 1
			// PC is incremented when we entered 0xED 0xB0, so decrement PC by 1 again...
			pPacMan_registers->pc -= TWO;
			// Refer https://retrocomputing.stackexchange.com/questions/6671/what-are-the-registers-w-and-z-inside-a-z80
			cpuSetRegister(REGISTER_TYPE::RT_WZ, pPacMan_registers->pc + ONE);
			// Refer https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
			updateXY(pPacMan_cpuInstance->opcode, ((pPacMan_registers->pc& (1 << 13)) ? (1 << 5) : 0) | ((pPacMan_registers->pc & (1 << 11)) ? (1 << 3) : 0));
		}
		else
		{
			uint16_t temp = (uint16_t)operationResult - (uint16_t)pPacMan_flags->FHALFCARRY;
			// Copy bit 1 to bit 5
			temp = (temp & ~(1 << 5)) | ((temp & (1 << 1)) << 4);
			updateXY(pPacMan_cpuInstance->opcode, TO_UINT8(temp));
		}
		pPacMan_cpuInstance->possFlag = YES;
		BREAK;
	}
	default:
	{
		unimplementedInstruction();
		BREAK;
	}
	}
}