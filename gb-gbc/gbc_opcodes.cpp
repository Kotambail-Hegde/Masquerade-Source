#include "gbc.h"

void GBc_t::runCPUPipeline()
{
	auto process0xCBPrefixInstructions = [&](uint16_t originalPC)
	{
		auto operationResult = 0;
		auto dataFromMemory = 0;
		auto lowerDataFromMemory = 0;
		auto higherDataFromMemory = 0;
		BYTE bitUnderTest = 0;
		BYTE theBit = 0;

		operationResult = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);

		INCREMENT_PC_BY_ONE();

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

		pGBc_cpuInstance->opcode = operationResult;
		switch (pGBc_cpuInstance->opcode)
		{
		case 0x00:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | (bit7 >> 7));
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RLC B", originalPC);
			BREAK;
		}
		case 0x01:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | (bit7 >> 7));
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RLC C", originalPC);
			BREAK;
		}
		case 0x02:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | (bit7 >> 7));
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RLC D", originalPC);
			BREAK;
		}
		case 0x03:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | (bit7 >> 7));
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RLC E", originalPC);
			BREAK;
		}
		case 0x04:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | (bit7 >> 7));
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RLC H", originalPC);
			BREAK;
		}
		case 0x05:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | (bit7 >> 7));
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RLC L", originalPC);
			BREAK;
		}
		case 0x06:
		{
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			BYTE bit7 = dataFromMemory & 0b10000000;
			dataFromMemory = ((dataFromMemory << 1) | (bit7 >> 7));
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			processZeroFlag((byte)dataFromMemory);
			cpuTickM();
			DISASSEMBLY("%04X RLC (HL)", originalPC);
			BREAK;
		}
		case 0x07:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | (bit7 >> 7));
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RLC A", originalPC);
			BREAK;
		}
		case 0x08:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			BYTE bit0 = registerContent & 0b00000001;
			pGBc_flags->FCARRY = bit0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit0 << 7);
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RRC B", originalPC);
			BREAK;
		}
		case 0x09:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			BYTE bit0 = registerContent & 0b00000001;
			pGBc_flags->FCARRY = bit0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit0 << 7);
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RRC C", originalPC);
			BREAK;
		}
		case 0x0A:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			BYTE bit0 = registerContent & 0b00000001;
			pGBc_flags->FCARRY = bit0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit0 << 7);
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RRC D", originalPC);
			BREAK;
		}
		case 0x0B:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			BYTE bit0 = registerContent & 0b00000001;
			pGBc_flags->FCARRY = bit0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit0 << 7);
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RRC E", originalPC);
			BREAK;
		}
		case 0x0C:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			BYTE bit0 = registerContent & 0b00000001;
			pGBc_flags->FCARRY = bit0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit0 << 7);
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RRC H", originalPC);
			BREAK;
		}
		case 0x0D:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			BYTE bit0 = registerContent & 0b00000001;
			pGBc_flags->FCARRY = bit0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit0 << 7);
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RRC L", originalPC);
			BREAK;
		}
		case 0x0E:
		{
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			BYTE bit0 = dataFromMemory & 0b00000001;
			dataFromMemory = ((dataFromMemory >> 1) | bit0 << 7);
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
			pGBc_flags->FCARRY = bit0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			processZeroFlag((byte)dataFromMemory);
			cpuTickM();
			DISASSEMBLY("%04X RRC (HL)", originalPC);
			BREAK;
		}
		case 0x0F:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			BYTE bit0 = registerContent & 0b00000001;
			pGBc_flags->FCARRY = bit0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit0 << 7);
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			processZeroFlag((byte)registerContent);
			DISASSEMBLY("%04X RRC A", originalPC);
			BREAK;
		}
		case 0x10:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			BYTE FCARRY = pGBc_flags->FCARRY;
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | FCARRY);
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RL B", originalPC);
			BREAK;
		}
		case 0x11:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			BYTE FCARRY = pGBc_flags->FCARRY;
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | FCARRY);
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RL C", originalPC);
			BREAK;
		}
		case 0x12:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			BYTE FCARRY = pGBc_flags->FCARRY;
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | FCARRY);
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RL D", originalPC);
			BREAK;
		}
		case 0x13:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			BYTE FCARRY = pGBc_flags->FCARRY;
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | FCARRY);
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RL E", originalPC);
			BREAK;
		}
		case 0x14:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			BYTE FCARRY = pGBc_flags->FCARRY;
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | FCARRY);
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RL H", originalPC);
			BREAK;
		}
		case 0x15:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			BYTE FCARRY = pGBc_flags->FCARRY;
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | FCARRY);
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RL L", originalPC);
			BREAK;
		}
		case 0x16:
		{
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			BYTE FCARRY = pGBc_flags->FCARRY;
			BYTE bit7 = dataFromMemory & 0b10000000;
			dataFromMemory = ((dataFromMemory << 1) | FCARRY);
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			processZeroFlag((BYTE)dataFromMemory);
			cpuTickM();
			DISASSEMBLY("%04X RL (HL)", originalPC);
			BREAK;
		}
		case 0x17:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			BYTE FCARRY = pGBc_flags->FCARRY;
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent << 1) | FCARRY);
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RL A", originalPC);
			BREAK;
		}
		case 0x18:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			BYTE FCARRY = pGBc_flags->FCARRY;
			pGBc_flags->FCARRY = registerContent & 0b00000001;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | FCARRY << 7);
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RR B", originalPC);
			BREAK;
		}
		case 0x19:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			BYTE FCARRY = pGBc_flags->FCARRY;
			pGBc_flags->FCARRY = registerContent & 0b00000001;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | FCARRY << 7);
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RR C", originalPC);
			BREAK;
		}
		case 0x1A:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			BYTE FCARRY = pGBc_flags->FCARRY;
			pGBc_flags->FCARRY = registerContent & 0b00000001;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | FCARRY << 7);
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RR D", originalPC);
			BREAK;
		}
		case 0x1B:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			BYTE FCARRY = pGBc_flags->FCARRY;
			pGBc_flags->FCARRY = registerContent & 0b00000001;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | FCARRY << 7);
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RR E", originalPC);
			BREAK;
		}
		case 0x1C:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			BYTE FCARRY = pGBc_flags->FCARRY;
			pGBc_flags->FCARRY = registerContent & 0b00000001;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | FCARRY << 7);
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RR H", originalPC);
			BREAK;
		}
		case 0x1D:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			BYTE FCARRY = pGBc_flags->FCARRY;
			pGBc_flags->FCARRY = registerContent & 0b00000001;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | FCARRY << 7);
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RR L", originalPC);
			BREAK;
		}
		case 0x1E:
		{
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			BYTE FCARRY = pGBc_flags->FCARRY;
			BYTE bit0 = dataFromMemory & 0b00000001;
			dataFromMemory = ((dataFromMemory >> 1) | FCARRY << 7);
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
			pGBc_flags->FCARRY = bit0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			processZeroFlag((BYTE)dataFromMemory);
			cpuTickM();
			DISASSEMBLY("%04X RR (HL)", originalPC);
			BREAK;
		}
		case 0x1F:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			BYTE FCARRY = pGBc_flags->FCARRY;
			pGBc_flags->FCARRY = registerContent & 0b00000001;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | FCARRY << 7);
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X RR A", originalPC);
			BREAK;
		}
		case 0x20:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent << 1);
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SLA B", originalPC);
			BREAK;
		}
		case 0x21:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent << 1);
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SLA C", originalPC);
			BREAK;
		}
		case 0x22:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent << 1);
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SLA D", originalPC);
			BREAK;
		}
		case 0x23:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent << 1);
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SLA E", originalPC);
			BREAK;
		}
		case 0x24:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent << 1);
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SLA H", originalPC);
			BREAK;
		}
		case 0x25:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent << 1);
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SLA L", originalPC);
			BREAK;
		}
		case 0x26:
		{
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			BYTE bit7 = dataFromMemory & 0b10000000;
			dataFromMemory = (dataFromMemory << 1);
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			processZeroFlag((BYTE)dataFromMemory);
			cpuTickM();
			DISASSEMBLY("%04X SLA (HL)", originalPC);
			BREAK;
		}
		case 0x27:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			BYTE bit7 = registerContent & 0b10000000;
			pGBc_flags->FCARRY = (bit7 >> 7);
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent << 1);
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SLA A", originalPC);
			BREAK;
		}
		case 0x28:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			BYTE bit7 = registerContent & 0b10000000;
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit7);
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRA B", originalPC);
			BREAK;
		}
		case 0x29:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			BYTE bit7 = registerContent & 0b10000000;
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit7);
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRA C", originalPC);
			BREAK;
		}
		case 0x2A:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			BYTE bit7 = registerContent & 0b10000000;
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit7);
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRA D", originalPC);
			BREAK;
		}
		case 0x2B:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			BYTE bit7 = registerContent & 0b10000000;
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit7);
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRA E", originalPC);
			BREAK;
		}
		case 0x2C:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			BYTE bit7 = registerContent & 0b10000000;
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit7);
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRA H", originalPC);
			BREAK;
		}
		case 0x2D:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			BYTE bit7 = registerContent & 0b10000000;
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit7);
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRA L", originalPC);
			BREAK;
		}
		case 0x2E:
		{
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			BYTE bit7 = dataFromMemory & 0b10000000;
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)((dataFromMemory >> 1) | bit7));
			if (dataFromMemory & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			dataFromMemory = ((dataFromMemory >> 1) | bit7);
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			processZeroFlag((BYTE)dataFromMemory);
			cpuTickM();
			DISASSEMBLY("%04X SRA (HL)", originalPC);
			BREAK;
		}
		case 0x2F:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			BYTE bit7 = registerContent & 0b10000000;
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = ((registerContent >> 1) | bit7);
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRA A", originalPC);
			BREAK;
		}
		case 0x30:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			BYTE higherNibbleToLower = ((registerContent >> 4) & 0x0F);
			BYTE lowerNibbleToHigher = ((registerContent << 4) & 0xF0);
			registerContent = higherNibbleToLower | lowerNibbleToHigher;
			pGBc_flags->FCARRY = 0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SWAP B", originalPC);
			BREAK;
		}
		case 0x31:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			BYTE higherNibbleToLower = ((registerContent >> 4) & 0x0F);
			BYTE lowerNibbleToHigher = ((registerContent << 4) & 0xF0);
			registerContent = higherNibbleToLower | lowerNibbleToHigher;
			pGBc_flags->FCARRY = 0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SWAP C", originalPC);
			BREAK;
		}
		case 0x32:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			BYTE higherNibbleToLower = ((registerContent >> 4) & 0x0F);
			BYTE lowerNibbleToHigher = ((registerContent << 4) & 0xF0);
			registerContent = higherNibbleToLower | lowerNibbleToHigher;
			pGBc_flags->FCARRY = 0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SWAP D", originalPC);
			BREAK;
		}
		case 0x33:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			BYTE higherNibbleToLower = ((registerContent >> 4) & 0x0F);
			BYTE lowerNibbleToHigher = ((registerContent << 4) & 0xF0);
			registerContent = higherNibbleToLower | lowerNibbleToHigher;
			pGBc_flags->FCARRY = 0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SWAP E", originalPC);
			BREAK;
		}
		case 0x34:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			BYTE higherNibbleToLower = ((registerContent >> 4) & 0x0F);
			BYTE lowerNibbleToHigher = ((registerContent << 4) & 0xF0);
			registerContent = higherNibbleToLower | lowerNibbleToHigher;
			pGBc_flags->FCARRY = 0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SWAP H", originalPC);
			BREAK;
		}
		case 0x35:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			BYTE higherNibbleToLower = ((registerContent >> 4) & 0x0F);
			BYTE lowerNibbleToHigher = ((registerContent << 4) & 0xF0);
			registerContent = higherNibbleToLower | lowerNibbleToHigher;
			pGBc_flags->FCARRY = 0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SWAP L", originalPC);
			BREAK;
		}
		case 0x36:
		{
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			BYTE higherNibbleToLower = ((dataFromMemory >> 4) & 0x0F);
			BYTE lowerNibbleToHigher = ((dataFromMemory << 4) & 0xF0);
			dataFromMemory = higherNibbleToLower | lowerNibbleToHigher;
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
			pGBc_flags->FCARRY = 0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			processZeroFlag((BYTE)dataFromMemory);
			cpuTickM();
			DISASSEMBLY("%04X SWAP (HL)", originalPC);
			BREAK;
		}
		case 0x37:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			BYTE higherNibbleToLower = ((registerContent >> 4) & 0x0F);
			BYTE lowerNibbleToHigher = ((registerContent << 4) & 0xF0);
			registerContent = higherNibbleToLower | lowerNibbleToHigher;
			pGBc_flags->FCARRY = 0;
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SWAP A", originalPC);
			BREAK;
		}
		case 0x38:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent >> 1);
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRL B", originalPC);
			BREAK;
		}
		case 0x39:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent >> 1);
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRL C", originalPC);
			BREAK;
		}
		case 0x3A:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent >> 1);
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRL D", originalPC);
			BREAK;
		}
		case 0x3B:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent >> 1);
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRL E", originalPC);
			BREAK;
		}
		case 0x3C:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent >> 1);
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRL H", originalPC);
			BREAK;
		}
		case 0x3D:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent >> 1);
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRL L", originalPC);
			BREAK;
		}
		case 0x3E:
		{
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)((dataFromMemory >> 1)));
			if (dataFromMemory & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			dataFromMemory = (dataFromMemory >> 1);
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			processZeroFlag((BYTE)dataFromMemory);
			cpuTickM();
			DISASSEMBLY("%04X SRL (HL)", originalPC);
			BREAK;
		}
		case 0x3F:
		{
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			if (registerContent & 0x01)
			{
				pGBc_flags->FCARRY = 1;
			}
			else
			{
				pGBc_flags->FCARRY = 0;
			}
			pGBc_flags->FHALFCARRY = 0;
			pGBc_flags->FSUB = 0;
			registerContent = (registerContent >> 1);
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			processZeroFlag((BYTE)registerContent);
			DISASSEMBLY("%04X SRL A", originalPC);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			if ((registerContent & bitUnderTest) == bitUnderTest)
			{
				pGBc_flags->FZERO = 0;
			}
			else
			{
				pGBc_flags->FZERO = 1;
			}
			pGBc_flags->FSUB = 0;
			pGBc_flags->FHALFCARRY = 1;
			DISASSEMBLY("%04X BIT %d, B", originalPC, bitUnderTest);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			if ((registerContent & bitUnderTest) == bitUnderTest)
			{
				pGBc_flags->FZERO = 0;
			}
			else
			{
				pGBc_flags->FZERO = 1;
			}
			pGBc_flags->FSUB = 0;
			pGBc_flags->FHALFCARRY = 1;
			DISASSEMBLY("%04X BIT %d, C", originalPC, bitUnderTest);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			if ((registerContent & bitUnderTest) == bitUnderTest)
			{
				pGBc_flags->FZERO = 0;
			}
			else
			{
				pGBc_flags->FZERO = 1;
			}
			pGBc_flags->FSUB = 0;
			pGBc_flags->FHALFCARRY = 1;
			DISASSEMBLY("%04X BIT %d, D", originalPC, bitUnderTest);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			if ((registerContent & bitUnderTest) == bitUnderTest)
			{
				pGBc_flags->FZERO = 0;
			}
			else
			{
				pGBc_flags->FZERO = 1;
			}
			pGBc_flags->FSUB = 0;
			pGBc_flags->FHALFCARRY = 1;
			DISASSEMBLY("%04X BIT %d, E", originalPC, bitUnderTest);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			if ((registerContent & bitUnderTest) == bitUnderTest)
			{
				pGBc_flags->FZERO = 0;
			}
			else
			{
				pGBc_flags->FZERO = 1;
			}
			pGBc_flags->FSUB = 0;
			pGBc_flags->FHALFCARRY = 1;
			DISASSEMBLY("%04X BIT %d, H", originalPC, bitUnderTest);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			if ((registerContent & bitUnderTest) == bitUnderTest)
			{
				pGBc_flags->FZERO = 0;
			}
			else
			{
				pGBc_flags->FZERO = 1;
			}
			pGBc_flags->FSUB = 0;
			pGBc_flags->FHALFCARRY = 1;
			DISASSEMBLY("%04X BIT %d, L", originalPC, bitUnderTest);
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
			cpuTickM();
			dataFromMemory = readRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), MEMORY_ACCESS_SOURCE::CPU);
			cpuTickM();
			if ((dataFromMemory & bitUnderTest) == bitUnderTest)
			{
				pGBc_flags->FZERO = 0;
			}
			else
			{
				pGBc_flags->FZERO = 1;
			}
			pGBc_flags->FSUB = 0;
			pGBc_flags->FHALFCARRY = 1;
			DISASSEMBLY("%04X BIT %d, (HL)", originalPC, bitUnderTest);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			if ((registerContent & bitUnderTest) == bitUnderTest)
			{
				pGBc_flags->FZERO = 0;
			}
			else
			{
				pGBc_flags->FZERO = 1;
			}
			pGBc_flags->FSUB = 0;
			pGBc_flags->FHALFCARRY = 1;
			DISASSEMBLY("%04X BIT %d, A", originalPC, bitUnderTest);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			UNSETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			DISASSEMBLY("%04X RES %d, B", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			UNSETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			DISASSEMBLY("%04X RES %d, C", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			UNSETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			DISASSEMBLY("%04X RES %d, D", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			UNSETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			DISASSEMBLY("%04X RES %d, E", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			UNSETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			DISASSEMBLY("%04X RES %d, H", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			UNSETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			DISASSEMBLY("%04X RES %d, L", originalPC, theBit);
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
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			UNSETBIT(dataFromMemory, theBit);
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickM();
			DISASSEMBLY("%04X RES %d, (HL)", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			UNSETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			DISASSEMBLY("%04X RES %d, A", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
			SETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_B, registerContent);
			DISASSEMBLY("%04X SET %d, B", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
			SETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_C, registerContent);
			DISASSEMBLY("%04X SET %d, C", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
			SETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_D, registerContent);
			DISASSEMBLY("%04X SET %d, D", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
			SETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_E, registerContent);
			DISASSEMBLY("%04X SET %d, E", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
			SETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_H, registerContent);
			DISASSEMBLY("%04X SET %d, H", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
			SETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_L, registerContent);
			DISASSEMBLY("%04X SET %d, L", originalPC, theBit);
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
			cpuTickM();
			dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
			SETBIT(dataFromMemory, theBit);
			cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
			writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickM();
			DISASSEMBLY("%04X SET %d, (HL)", originalPC, theBit);
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
			cpuTickM();
			BYTE registerContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
			SETBIT(registerContent, theBit);
			cpuSetRegister(REGISTER_TYPE::RT_A, registerContent);
			DISASSEMBLY("%04X SET %d, A", originalPC, theBit);
			BREAK;
		}
		default:
		{
			cpuTickM();
			unimplementedInstruction();
			BREAK;
		}
		}
	};

	// Set/Reset the Unused bits

	processUnusedFlags(ZERO);
	if (ROM_TYPE != ROM::TEST_SST)
	{
		processUnusedJoyPadBits(ONE);
		processUnusedIFBits(ONE);
	}

	// Fetch Instruction

	pGBc_cpuInstance->previousOpcode = pGBc_cpuInstance->opcode;
	pGBc_cpuInstance->opcode = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);

	FLAG isPCRepeated = INCREMENT_PC_BY_ONE();

	// Fetch Data and Execute

	auto operationResult = RESET;
	auto dataFromMemory = RESET;
	auto lowerDataFromMemory = RESET;
	auto higherDataFromMemory = RESET;

	// Get the original pc, opcode and cycles (only for logging)

	auto originalPC = GET_PC();
	auto originalOp0 = pGBc_cpuInstance->opcode;

	switch (pGBc_cpuInstance->opcode)
	{
	case 0x00:
	{
		cpuTickM();
		DISASSEMBLY("%04X NOP", originalPC);
		BREAK;
	}
	case 0x01:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_BC, UWORD_FROM_UBYTES(higherDataFromMemory, lowerDataFromMemory));
		DISASSEMBLY("%04X LD BC, $%04X", originalPC, UWORD_FROM_UBYTES(higherDataFromMemory, lowerDataFromMemory));
		BREAK;
	}
	case 0x02:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_BC);
		cpuTickM(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		writeRawMemory(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (BC), A", originalPC);
		BREAK;
	}
	case 0x03:
	{
		cpuTickM();
		auto bc = cpuReadRegister(REGISTER_TYPE::RT_BC);
		operationResult = bc + ONE;
		handleOAMCorruption(bc, OAM_ACCESS_TYPE::WRITE);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_BC, operationResult);
		DISASSEMBLY("%04X INC BC", originalPC);
		BREAK;
	}
	case 0x04:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B) + ONE;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		DISASSEMBLY("%04X INC B", originalPC);
		BREAK;
	}
	case 0x05:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B) - 1;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		DISASSEMBLY("%04X DEC B", originalPC);
		BREAK;
	}
	case 0x06:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		DISASSEMBLY("%04X LD B, $%02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0x07:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		if (registerAContent & 0x80)
		{
			pGBc_flags->FCARRY = 1;
		}
		else
		{
			pGBc_flags->FCARRY = 0;
		}
		pGBc_flags->FHALFCARRY = ZERO;
		pGBc_flags->FSUB = ZERO;
		pGBc_flags->FZERO = ZERO;
		operationResult = ((registerAContent >> 7) | (registerAContent << 1));
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X RLCA", originalPC);
		BREAK;
	}
	case 0x08:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		cpuTickM(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_SP));
		writeRawMemory(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_SP), MEMORY_ACCESS_SOURCE::CPU);
		writeRawMemory(operationResult + ONE, (BYTE)((cpuReadRegister(REGISTER_TYPE::RT_SP) & 0xFF00) >> 8), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD ($%04X), SP", originalPC, operationResult);
		BREAK;
	}
	case 0x09:
	{
		cpuTickM();
		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL)
			+ (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_BC);
		cpuTickM();
		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_BC),
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		DISASSEMBLY("%04X ADD HL, BC", originalPC);
		BREAK;
	}
	case 0x0A:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_BC);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		DISASSEMBLY("%04X LD A, (BC)", originalPC);
		BREAK;
	}
	case 0x0B:
	{
		cpuTickM();
		auto bc = cpuReadRegister(REGISTER_TYPE::RT_BC);
		operationResult = bc - ONE;
		handleOAMCorruption(bc, OAM_ACCESS_TYPE::WRITE);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_BC, operationResult);
		DISASSEMBLY("%04X DEC BC", originalPC);
		BREAK;
	}
	case 0x0C:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C) + ONE;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		DISASSEMBLY("%04X INC C", originalPC);
		BREAK;
	}
	case 0x0D:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C) - 1;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		DISASSEMBLY("%04X DEC C", originalPC);
		BREAK;
	}
	case 0x0E:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		DISASSEMBLY("%04X LD C, $%02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0x0F:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		if (registerAContent & 0x01)
		{
			pGBc_flags->FCARRY = 1;
		}
		else
		{
			pGBc_flags->FCARRY = 0;
		}
		pGBc_flags->FHALFCARRY = ZERO;
		pGBc_flags->FSUB = ZERO;
		pGBc_flags->FZERO = ZERO;
		operationResult = ((registerAContent << 7) | (registerAContent >> 1));
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X RRCA", originalPC);
		BREAK;
	}
	case 0x10:
	{
		cpuTickM();
#if (ENABLE_SM83_SST == YES)
		if (ROM_TYPE != ROM::TEST_SST)
#endif
		{
			auto handleCGBSpeedSwitch = [&]()
				{
					if (ROM_TYPE == ROM::GAME_BOY_COLOR)
					{
						pGBc_peripherals->KEY1.KEY1Fields.PrepareSpeedSwitch = RESET;

						toggleCGBSpeedMode();

						if (isCGBDoubleSpeedEnabled() == NO)
						{
							CPUINFO("CGB Double Speed : Disabled");
							pGBc_peripherals->KEY1.KEY1Fields.CurrentSpeed = ZERO;
						}
						else
						{
							CPUINFO("CGB Double Speed : Enabled");
							pGBc_peripherals->KEY1.KEY1Fields.CurrentSpeed = ONE;
						}
					}
				};

			FLAG isSpeedSwitchRequested = (ROM_TYPE == ROM::GAME_BOY_COLOR) && (pGBc_peripherals->KEY1.KEY1Fields.PrepareSpeedSwitch == SET);
			FLAG isInterruptPending = isInterruptReadyToBeServed();
			FLAG isIMEEnabled = pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn;

			// If a button is being held (1 -> not pressed)
			if ((pGBc_peripherals->P1_JOYP.joyPadMemory & 0x0F) != 0x0F)
			{
				if (isInterruptPending == NO)
				{
					cpuTickM();
					INCREMENT_PC_BY_ONE();
					pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = YES;
					pGBc_instance->GBc_state.emulatorStatus.isCPUJustHalted = YES;
				}
			}
			// None of the buttons are held
			else
			{
				if (isSpeedSwitchRequested == YES)
				{
					if (isInterruptPending == YES)
					{
						if (isIMEEnabled == YES)
						{
							handleCGBSpeedSwitch(); // API checks whether CGB or not before switch speed
							pGBc_peripherals->DIV.divMemory = RESET;
						}
						else
						{
							FATAL("CPU glitch because of STOP");
						}
					}
					else
					{
						cpuTickM();
						INCREMENT_PC_BY_ONE();
						pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = YES;
						pGBc_instance->GBc_state.emulatorStatus.isCPUJustHalted = YES;
						CPUTODO("Pandocs mentions \"exitHaltInTCycles\" = 0x20000, but sameboy uses 0x20008, and we pass the speed switch tests with 0x20004");
						// Maybe the above cpuTickM handles the 4 out of 0x20008 and hence we need only 0x20004 to be set here. But we need to investigate this further.
						// Refer (Pandocs) : https://gbdev.io/pandocs/Reducing_Power_Consumption.html?highlight=STOP#the-bizarre-case-of-the-game-boy-stop-instruction-before-even-considering-timing
						// Refer (Sameboy) : https://github.com/LIJI32/SameBoy/blob/master/Core/sm83_cpu.c#L434
						pGBc_instance->GBc_state.emulatorStatus.exitHaltInTCycles = 0x20004;
						pGBc_peripherals->DIV.divMemory = RESET;
						handleCGBSpeedSwitch(); // API checks whether CGB or not before switch speed
					}
				}
				else
				{
					if (isInterruptPending == NO)
					{
						cpuTickM();
						INCREMENT_PC_BY_ONE();
					}
					pGBc_instance->GBc_state.emulatorStatus.isCPUStopped = YES;
					pGBc_peripherals->DIV.divMemory = RESET;
					// Pandocs "Using the STOP instruction" (GBC): STOP with the LCD enabled
					// blanks the display to black, UNLESS the PPU was already in Mode 3 at
					// the moment STOP executed, in which case it keeps rendering the current
					// frame uninterrupted instead. This is decided once and latched for the
					// whole STOP duration -- mirrors SameBoy's cgb_palettes_ppu_blocked, which
					// is set once in enter_stop_mode() and never re-evaluated as the PPU
					// naturally cycles through modes afterward.
					pGBc_instance->GBc_state.emulatorStatus.stopLCD = YES;
					pGBc_instance->GBc_state.emulatorStatus.stopLCDDone = NO;
					pGBc_instance->GBc_state.emulatorStatus.stopKeepDrawingMode3 = ((ROM_TYPE == ROM::GAME_BOY_COLOR) && (pGBc_display->currentLCDMode == LCD_MODES::MODE_LCD_DISPLAY_PIXELS)) ? YES : NO;
				}
			}
		}

		DISASSEMBLY("%04X STOP", originalPC);
		BREAK;
	}
	case 0x11:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_DE, UWORD_FROM_UBYTES(higherDataFromMemory, lowerDataFromMemory));
		DISASSEMBLY("%04X LD DE, $%04X", originalPC, UWORD_FROM_UBYTES(higherDataFromMemory, lowerDataFromMemory));
		BREAK;
	}
	case 0x12:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_DE);
		cpuTickM(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		writeRawMemory(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (DE), A", originalPC);
		BREAK;
	}
	case 0x13:
	{
		cpuTickM();
		auto de = cpuReadRegister(REGISTER_TYPE::RT_DE);
		operationResult = de + ONE;
		handleOAMCorruption(de, OAM_ACCESS_TYPE::WRITE);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_DE, operationResult);
		DISASSEMBLY("%04X INC DE", originalPC);
		BREAK;
	}
	case 0x14:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D) + ONE;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		DISASSEMBLY("%04X INC D", originalPC);
		BREAK;
	}
	case 0x15:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D) - 1;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		DISASSEMBLY("%04X DEC D", originalPC);
		BREAK;
	}
	case 0x16:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		DISASSEMBLY("%04X LD D, $%02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0x17:
	{
		cpuTickM();
		BYTE FCarry = pGBc_flags->FCARRY;
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		if (registerAContent & 0x80)
		{
			pGBc_flags->FCARRY = 1;
		}
		else
		{
			pGBc_flags->FCARRY = 0;
		}
		pGBc_flags->FHALFCARRY = ZERO;
		pGBc_flags->FSUB = ZERO;
		pGBc_flags->FZERO = ZERO;
		operationResult = ((registerAContent << 1) | FCarry);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X RLA", originalPC);
		BREAK;
	}
	case 0x18:
	{
		cpuTickM();
		SBYTE relativeJump = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		SET_PC(GET_PC() + relativeJump);
		cpuTickM();
		DISASSEMBLY("%04X JR $%+d", originalPC, relativeJump);
		BREAK;
	}
	case 0x19:
	{
		cpuTickM();
		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL)
			+ (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_DE);
		cpuTickM();
		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_DE),
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		DISASSEMBLY("%04X ADD HL, DE", originalPC);
		BREAK;
	}
	case 0x1A:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_DE);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		DISASSEMBLY("%04X LD A, (DE)", originalPC);
		BREAK;
	}
	case 0x1B:
	{
		cpuTickM();
		auto de = cpuReadRegister(REGISTER_TYPE::RT_DE);
		operationResult = de - ONE;
		handleOAMCorruption(de, OAM_ACCESS_TYPE::WRITE);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_DE, operationResult);
		DISASSEMBLY("%04X DEC DE", originalPC);
		BREAK;
	}
	case 0x1C:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E) + ONE;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		DISASSEMBLY("%04X INC E", originalPC);
		BREAK;
	}
	case 0x1D:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E) - 1;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		DISASSEMBLY("%04X DEC E", originalPC);
		BREAK;
	}
	case 0x1E:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		DISASSEMBLY("%04X LD E, $%02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0x1F:
	{
		cpuTickM();
		BYTE FCarry = pGBc_flags->FCARRY;
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		if (registerAContent & 0x01)
		{
			pGBc_flags->FCARRY = 1;
		}
		else
		{
			pGBc_flags->FCARRY = 0;
		}
		pGBc_flags->FHALFCARRY = ZERO;
		pGBc_flags->FSUB = ZERO;
		pGBc_flags->FZERO = ZERO;
		operationResult = ((registerAContent >> 1) | (FCarry << 7));
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X RRA", originalPC);
		BREAK;
	}
	case 0x20:
	{
		cpuTickM();
		SBYTE relativeJump = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		if (pGBc_flags->FZERO == 0)
		{
			SET_PC(GET_PC() + relativeJump);
			cpuTickM();
		}
		DISASSEMBLY("%04X JR NZ, $%02X", originalPC, relativeJump);
		BREAK;
	}
	case 0x21:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_HL, UWORD_FROM_UBYTES(higherDataFromMemory, lowerDataFromMemory));
		DISASSEMBLY("%04X LD HL, $%04X", originalPC, UWORD_FROM_UBYTES(higherDataFromMemory, lowerDataFromMemory));
		BREAK;
	}
	case 0x22:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (operationResult + ONE));
		FLAG wasBlocked = handleOAMCorruption(operationResult, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		if (!wasBlocked) // TODO: Confirm this behaviour; Source : Sameboy
		{
			writeRawMemory(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), MEMORY_ACCESS_SOURCE::CPU);
		}
		cpuTickM();
		DISASSEMBLY("%04X LD (HL+), A", originalPC);
		BREAK;
	}
	case 0x23:
	{
		cpuTickM();
		auto hl = cpuReadRegister(REGISTER_TYPE::RT_HL);
		operationResult = hl + ONE;
		handleOAMCorruption(hl, OAM_ACCESS_TYPE::WRITE);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_HL, operationResult);
		DISASSEMBLY("%04X INC HL", originalPC);
		BREAK;
	}
	case 0x24:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H) + ONE;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		DISASSEMBLY("%04X INC H", originalPC);
		BREAK;
	}
	case 0x25:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H) - 1;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		DISASSEMBLY("%04X DEC H", originalPC);
		BREAK;
	}
	case 0x26:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		DISASSEMBLY("%04X LD H, $%02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0x27:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A);
		if (pGBc_flags->FSUB == ZERO)
		{
			if (pGBc_flags->FCARRY || operationResult > 0x99)
			{
				operationResult += 0x60;
				pGBc_flags->FCARRY = ONE;
			}

			if (pGBc_flags->FHALFCARRY || (operationResult & 0x0F) > 0x09)
			{
				operationResult += 0x06;
			}
		}
		else
		{
			if (pGBc_flags->FCARRY)
			{
				operationResult -= 0x60;
			}

			if (pGBc_flags->FHALFCARRY)
			{
				operationResult -= 0x06;
			}
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		pGBc_flags->FHALFCARRY = ZERO;
		processZeroFlag((byte)operationResult);
		DISASSEMBLY("%04X DAA", originalPC);
		BREAK;
	}
	case 0x28:
	{
		cpuTickM();
		SBYTE relativeJump = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		if (pGBc_flags->FZERO != 0)
		{
			SET_PC(GET_PC() + relativeJump);
			cpuTickM();
		}
		DISASSEMBLY("%04X JR Z, $%02X", originalPC, relativeJump);
		BREAK;
	}
	case 0x29:
	{
		cpuTickM();
		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL)
			+ (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuTickM();
		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		DISASSEMBLY("%04X ADD HL, HL", originalPC);
		BREAK;
	}
	case 0x2A:
	{
		cpuTickM();
		auto hl = cpuReadRegister(REGISTER_TYPE::RT_HL);
		handleOAMCorruption(hl, OAM_ACCESS_TYPE::READ);
		dataFromMemory = readRawMemory(hl, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_HL, (hl + ONE));
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		DISASSEMBLY("%04X LD A, (HL+)", originalPC);
		BREAK;
	}
	case 0x2B:
	{
		cpuTickM();
		auto hl = cpuReadRegister(REGISTER_TYPE::RT_HL);
		operationResult = hl - ONE;
		handleOAMCorruption(hl, OAM_ACCESS_TYPE::WRITE);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_HL, operationResult);
		DISASSEMBLY("%04X DEC HL", originalPC);
		BREAK;
	}
	case 0x2C:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L) + ONE;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		DISASSEMBLY("%04X INC L", originalPC);
		BREAK;
	}
	case 0x2D:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L) - 1;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		DISASSEMBLY("%04X DEC L", originalPC);
		BREAK;
	}
	case 0x2E:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		DISASSEMBLY("%04X LD L, $%02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0x2F:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		operationResult = ~operationResult;
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		pGBc_flags->FSUB = ONE;
		pGBc_flags->FHALFCARRY = ONE;
		DISASSEMBLY("%04X CPL", originalPC);
		BREAK;
	}
	case 0x30:
	{
		cpuTickM();
		SBYTE relativeJump = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		if (pGBc_flags->FCARRY == 0)
		{
			SET_PC(GET_PC() + relativeJump);
			cpuTickM();
		}
		DISASSEMBLY("%04X JR NC, $%02X", originalPC, relativeJump);
		BREAK;
	}
	case 0x31:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		DISASSEMBLY("%04X LD SP, $%04X", originalPC, operationResult);
		BREAK;
	}
	case 0x32:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (operationResult - 1));
		FLAG wasBlocked = handleOAMCorruption(operationResult, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		if (!wasBlocked) // TODO: Confirm this behaviour; Source : Sameboy
		{
			writeRawMemory(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), MEMORY_ACCESS_SOURCE::CPU);
		}
		cpuTickM();
		DISASSEMBLY("%04X LD (HL-), A", originalPC);
		BREAK;
	}
	case 0x33:
	{
		cpuTickM();
		auto sp = cpuReadRegister(REGISTER_TYPE::RT_SP);
		operationResult = sp + ONE;
		handleOAMCorruption(sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		DISASSEMBLY("%04X INC SP", originalPC);
		BREAK;
	}
	case 0x34:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		operationResult = dataFromMemory + ONE;
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult);
		processFlagsFor8BitAdditionOperation
		(
			(byte)dataFromMemory,
			(byte)1,
			false,
			false
		);
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X INC (HL)", originalPC);
		BREAK;
	}
	case 0x35:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		operationResult = dataFromMemory - 1;
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)dataFromMemory,
			(byte)1,
			false,
			false
		);
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X DEC (HL)", originalPC);
		BREAK;
	}
	case 0x36:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory);
		INCREMENT_PC_BY_ONE();
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), (BYTE)dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (HL), $%02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0x37:
	{
		cpuTickM();
		pGBc_flags->FCARRY = ONE;
		pGBc_flags->FHALFCARRY = ZERO;
		pGBc_flags->FSUB = ZERO;
		DISASSEMBLY("%04X SCF", originalPC);
		BREAK;
	}
	case 0x38:
	{
		cpuTickM();
		SBYTE relativeJump = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		if (pGBc_flags->FCARRY != 0)
		{
			SET_PC(GET_PC() + relativeJump);
			cpuTickM();
		}
		DISASSEMBLY("%04X JR C, $%02X", originalPC, relativeJump);
		BREAK;
	}
	case 0x39:
	{
		cpuTickM();
		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL)
			+ (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_SP);
		cpuTickM();
		processFlagsFor16BitAdditionOperation
		(
			cpuReadRegister(REGISTER_TYPE::RT_HL),
			cpuReadRegister(REGISTER_TYPE::RT_SP),
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		DISASSEMBLY("%04X ADD HL, SP", originalPC);
		BREAK;
	}
	case 0x3A:
	{
		cpuTickM();
		auto hl = cpuReadRegister(REGISTER_TYPE::RT_HL);
		handleOAMCorruption(hl, OAM_ACCESS_TYPE::READ);
		dataFromMemory = readRawMemory(hl, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_HL, (hl - ONE));
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		DISASSEMBLY("%04X LD A, (HL-)", originalPC);
		BREAK;
	}
	case 0x3B:
	{
		cpuTickM();
		auto sp = cpuReadRegister(REGISTER_TYPE::RT_SP);
		operationResult = sp - 1;
		handleOAMCorruption(sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		DISASSEMBLY("%04X DEC SP", originalPC);
		BREAK;
	}
	case 0x3C:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) + ONE;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X INC A", originalPC);
		BREAK;
	}
	case 0x3D:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) - 1;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)1,
			false,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X DEC A", originalPC);
		BREAK;
	}
	case 0x3E:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		DISASSEMBLY("%04X LD A, $%02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0x3F:
	{
		cpuTickM();
		pGBc_flags->FCARRY = (pGBc_flags->FCARRY > ZERO ? ZERO : ONE);
		pGBc_flags->FHALFCARRY = ZERO;
		pGBc_flags->FSUB = ZERO;
		DISASSEMBLY("%04X CCF", originalPC);
		BREAK;
	}
	case 0x40:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		DISASSEMBLY("%04X LD B, B", originalPC);
		BREAK;
	}
	case 0x41:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		DISASSEMBLY("%04X LD B, C", originalPC);
		BREAK;
	}
	case 0x42:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		DISASSEMBLY("%04X LD B, D", originalPC);
		BREAK;
	}
	case 0x43:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		DISASSEMBLY("%04X LD B, E", originalPC);
		BREAK;
	}
	case 0x44:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		DISASSEMBLY("%04X LD B, H", originalPC);
		BREAK;
	}
	case 0x45:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		DISASSEMBLY("%04X LD B, L", originalPC);
		BREAK;
	}
	case 0x46:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		DISASSEMBLY("%04X LD B, (HL)", originalPC);
		BREAK;
	}
	case 0x47:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		DISASSEMBLY("%04X LD B, A", originalPC);
		BREAK;
	}
	case 0x48:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		DISASSEMBLY("%04X LD C, B", originalPC);
		BREAK;
	}
	case 0x49:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		DISASSEMBLY("%04X LD C, C", originalPC);
		BREAK;
	}
	case 0x4A:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		DISASSEMBLY("%04X LD C, D", originalPC);
		BREAK;
	}
	case 0x4B:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		DISASSEMBLY("%04X LD C, E", originalPC);
		BREAK;
	}
	case 0x4C:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		DISASSEMBLY("%04X LD C, H", originalPC);
		BREAK;
	}
	case 0x4D:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		DISASSEMBLY("%04X LD C, L", originalPC);
		BREAK;
	}
	case 0x4E:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		DISASSEMBLY("%04X LD C, (HL)", originalPC);
		BREAK;
	}
	case 0x4F:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		DISASSEMBLY("%04X LD C, A", originalPC);
		BREAK;
	}
	case 0x50:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		DISASSEMBLY("%04X LD D, B", originalPC);
		BREAK;
	}
	case 0x51:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		DISASSEMBLY("%04X LD D, C", originalPC);
		BREAK;
	}
	case 0x52:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		DISASSEMBLY("%04X LD D, D", originalPC);
		BREAK;
	}
	case 0x53:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		DISASSEMBLY("%04X LD D, E", originalPC);
		BREAK;
	}
	case 0x54:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		DISASSEMBLY("%04X LD D, H", originalPC);
		BREAK;
	}
	case 0x55:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		DISASSEMBLY("%04X LD D, L", originalPC);
		BREAK;
	}
	case 0x56:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		DISASSEMBLY("%04X LD D, (HL)", originalPC);
		BREAK;
	}
	case 0x57:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		DISASSEMBLY("%04X LD D, A", originalPC);
		BREAK;
	}
	case 0x58:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		DISASSEMBLY("%04X LD E, B", originalPC);
		BREAK;
	}
	case 0x59:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		DISASSEMBLY("%04X LD E, C", originalPC);
		BREAK;
	}
	case 0x5A:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		DISASSEMBLY("%04X LD E, D", originalPC);
		BREAK;
	}
	case 0x5B:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		DISASSEMBLY("%04X LD E, E", originalPC);
		BREAK;
	}
	case 0x5C:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		DISASSEMBLY("%04X LD E, H", originalPC);
		BREAK;
	}
	case 0x5D:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		DISASSEMBLY("%04X LD E, L", originalPC);
		BREAK;
	}
	case 0x5E:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		DISASSEMBLY("%04X LD E, (HL)", originalPC);
		BREAK;
	}
	case 0x5F:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		DISASSEMBLY("%04X LD E, A", originalPC);
		BREAK;
	}
	case 0x60:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		DISASSEMBLY("%04X LD H, B", originalPC);
		BREAK;
	}
	case 0x61:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		DISASSEMBLY("%04X LD H, C", originalPC);
		BREAK;
	}
	case 0x62:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		DISASSEMBLY("%04X LD H, D", originalPC);
		BREAK;
	}
	case 0x63:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		DISASSEMBLY("%04X LD H, E", originalPC);
		BREAK;
	}
	case 0x64:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		DISASSEMBLY("%04X LD H, H", originalPC);
		BREAK;
	}
	case 0x65:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		DISASSEMBLY("%04X LD H, L", originalPC);
		BREAK;
	}
	case 0x66:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		DISASSEMBLY("%04X LD H, (HL)", originalPC);
		BREAK;
	}
	case 0x67:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		DISASSEMBLY("%04X LD H, A", originalPC);
		BREAK;
	}
	case 0x68:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		DISASSEMBLY("%04X LD L, B", originalPC);
		BREAK;
	}
	case 0x69:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		DISASSEMBLY("%04X LD L, C", originalPC);
		BREAK;
	}
	case 0x6A:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		DISASSEMBLY("%04X LD L, D", originalPC);
		BREAK;
	}
	case 0x6B:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		DISASSEMBLY("%04X LD L, E", originalPC);
		BREAK;
	}
	case 0x6C:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		DISASSEMBLY("%04X LD L, H", originalPC);
		BREAK;
	}
	case 0x6D:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		DISASSEMBLY("%04X LD L, L", originalPC);
		BREAK;
	}
	case 0x6E:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		DISASSEMBLY("%04X LD L, (HL)", originalPC);
		BREAK;
	}
	case 0x6F:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		DISASSEMBLY("%04X LD L, A", originalPC);
		BREAK;
	}
	case 0x70:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult);
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (HL), B", originalPC);
		BREAK;
	}
	case 0x71:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult);
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (HL), C", originalPC);
		BREAK;
	}
	case 0x72:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult);
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (HL), D", originalPC);
		BREAK;
	}
	case 0x73:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult);
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (HL), E", originalPC);
		BREAK;
	}
	case 0x74:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult);
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (HL), H", originalPC);
		BREAK;
	}
	case 0x75:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult);
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (HL), L", originalPC);
		BREAK;
	}
	case 0x76:
	{
		cpuTickM();
		// Halt logic is documented here at https://gbdev.io/pandocs/halt.html
		// But, for docboy test suite's halt_ime1_interrupt1_ret.gb to pass along with already passing halt tests
		// there was some tweaking needed in addition to items mentioned above
		// Source : Sameboy; Refer https://github.com/LIJI32/SameBoy/blob/master/Core/sm83_cpu.c#L1032
		CPUTODO("Find documentation for below HALT behaviour ?");

		pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = YES;
		if (isInterruptReadyToBeServed() == YES)
		{
			pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = NO;
			if (pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn == ENABLED)
			{
				// Weird behaviour #2
				DECREMENT_PC_BY_ONE();
			}
			else
			{
				// Weird behaviour #1
				pGBc_instance->GBc_state.emulatorStatus.isHaltBugActivated = HALT_BUG_STATE::HALT_BUG_ENABLED;
			}
		}
		// Set "just halted" to same state as "halted"
		pGBc_instance->GBc_state.emulatorStatus.isCPUJustHalted = pGBc_instance->GBc_state.emulatorStatus.isCPUHalted;
		DISASSEMBLY("%04X HALT", originalPC);
		BREAK;
	}
	case 0x77:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickM(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult);
		writeRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (HL), A", originalPC);
		BREAK;
	}
	case 0x78:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X LD A, B", originalPC);
		BREAK;
	}
	case 0x79:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X LD A, C", originalPC);
		BREAK;
	}
	case 0x7A:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X LD A, D", originalPC);
		BREAK;
	}
	case 0x7B:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X LD A, E", originalPC);
		BREAK;
	}
	case 0x7C:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X LD A, H", originalPC);
		BREAK;
	}
	case 0x7D:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X LD A, L", originalPC);
		BREAK;
	}
	case 0x7E:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		DISASSEMBLY("%04X LD A, (HL)", originalPC);
		BREAK;
	}
	case 0x7F:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X LD A, A", originalPC);
		BREAK;
	}
	case 0x80:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_B);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADD A, B", originalPC);
		BREAK;
	}
	case 0x81:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_C);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADD A, C", originalPC);
		BREAK;
	}
	case 0x82:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_D);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADD A, D", originalPC);
		BREAK;
	}
	case 0x83:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_E);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADD A, E", originalPC);
		BREAK;
	}
	case 0x84:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_H);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADD A, H", originalPC);
		BREAK;
	}
	case 0x85:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_L);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADD A, L", originalPC);
		BREAK;
	}
	case 0x86:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADD A, (HL)", originalPC);
		BREAK;
	}
	case 0x87:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_A);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADD A, A", originalPC);
		BREAK;
	}
	case 0x88:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_B)
			+ (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADC A, B", originalPC);
		BREAK;
	}
	case 0x89:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_C)
			+ (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADC A, C", originalPC);
		BREAK;
	}
	case 0x8A:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_D)
			+ (uint16_t)pGBc_flags->FCARRY;

		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADC A, D", originalPC);
		BREAK;
	}
	case 0x8B:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_E)
			+ (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADC A, E", originalPC);
		BREAK;
	}
	case 0x8C:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_H)
			+ (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADC A, H", originalPC);
		BREAK;
	}
	case 0x8D:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_L)
			+ (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADC A, L", originalPC);
		BREAK;
	}
	case 0x8E:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ (uint16_t)dataFromMemory
			+ (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADC A, (HL)", originalPC);
		BREAK;
	}
	case 0x8F:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_A)
			+ (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADC A, A", originalPC);
		BREAK;
	}
	case 0x90:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_B);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SUB B", originalPC);
		BREAK;
	}
	case 0x91:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_C);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SUB C", originalPC);
		BREAK;
	}
	case 0x92:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_D);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SUB D", originalPC);
		BREAK;
	}
	case 0x93:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_E);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SUB E", originalPC);
		BREAK;
	}
	case 0x94:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_H);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SUB H", originalPC);
		BREAK;
	}
	case 0x95:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_L);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SUB L", originalPC);
		BREAK;
	}
	case 0x96:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SUB (HL)", originalPC);
		BREAK;
	}
	case 0x97:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_A);
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SUB A", originalPC);
		BREAK;
	}
	case 0x98:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_B)
			- (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_B),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SBC A, B", originalPC);
		BREAK;
	}
	case 0x99:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_C)
			- (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_C),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SBC A, C", originalPC);
		BREAK;
	}
	case 0x9A:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_D)
			- (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_D),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SBC A, D", originalPC);
		BREAK;
	}
	case 0x9B:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_E)
			- (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_E),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SBC A, E", originalPC);
		BREAK;
	}
	case 0x9C:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_H)
			- (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_H),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SBC A, H", originalPC);
		BREAK;
	}
	case 0x9D:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_L)
			- (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_L),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SBC A, L", originalPC);
		BREAK;
	}
	case 0x9E:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- (uint16_t)dataFromMemory
			- (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SBC A, (HL)", originalPC);
		BREAK;
	}
	case 0x9F:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_A)
			- (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SBC A, A", originalPC);
		BREAK;
	}
	case 0xA0:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X AND A, B", originalPC);
		BREAK;
	}
	case 0xA1:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X AND A, C", originalPC);
		BREAK;
	}
	case 0xA2:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X AND A, D", originalPC);
		BREAK;
	}
	case 0xA3:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X AND A, E", originalPC);
		BREAK;
	}
	case 0xA4:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X AND A, H", originalPC);
		BREAK;
	}
	case 0xA5:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X AND A, L", originalPC);
		BREAK;
	}
	case 0xA6:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & dataFromMemory;
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X AND A, (HL)", originalPC);
		BREAK;
	}
	case 0xA7:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X AND A, A", originalPC);
		BREAK;
	}
	case 0xA8:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_B);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X XOR A, B", originalPC);
		BREAK;
	}
	case 0xA9:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_C);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X XOR A, C", originalPC);
		BREAK;
	}
	case 0xAA:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_D);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X XOR A, D", originalPC);
		BREAK;
	}
	case 0xAB:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_E);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X XOR A, E", originalPC);
		BREAK;
	}
	case 0xAC:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_H);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X XOR A, H", originalPC);
		BREAK;
	}
	case 0xAD:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_L);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X XOR A, L", originalPC);
		BREAK;
	}
	case 0xAE:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ dataFromMemory;
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X XOR A, (HL)", originalPC);
		BREAK;
	}
	case 0xAF:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_A);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X XOR A, A", originalPC);
		BREAK;
	}
	case 0xB0:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X OR A, B", originalPC);
		BREAK;
	}
	case 0xB1:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X OR A, C", originalPC);
		BREAK;
	}
	case 0xB2:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X OR A, D", originalPC);
		BREAK;
	}
	case 0xB3:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X OR A, E", originalPC);
		BREAK;
	}
	case 0xB4:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X OR A, H", originalPC);
		BREAK;
	}
	case 0xB5:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X OR A, L", originalPC);
		BREAK;
	}
	case 0xB6:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) | dataFromMemory;
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X OR A, (HL)", originalPC);
		BREAK;
	}
	case 0xB7:
	{
		cpuTickM();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X OR A, A", originalPC);
		BREAK;
	}
	case 0xB8:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)otherRegisterContent,
			false,
			true
		);
		DISASSEMBLY("%04X CP A, B", originalPC);
		BREAK;
	}
	case 0xB9:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)otherRegisterContent,
			false,
			true
		);
		DISASSEMBLY("%04X CP A, C", originalPC);
		BREAK;
	}
	case 0xBA:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)otherRegisterContent,
			false,
			true
		);
		DISASSEMBLY("%04X CP A, D", originalPC);
		BREAK;
	}
	case 0xBB:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)otherRegisterContent,
			false,
			true
		);
		DISASSEMBLY("%04X CP A, E", originalPC);
		BREAK;
	}
	case 0xBC:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)otherRegisterContent,
			false,
			true
		);
		DISASSEMBLY("%04X CP A, H", originalPC);
		BREAK;
	}
	case 0xBD:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)otherRegisterContent,
			false,
			true
		);
		DISASSEMBLY("%04X CP A, L", originalPC);
		BREAK;
	}
	case 0xBE:
	{
		cpuTickM();
		dataFromMemory = cpuReadPointer(POINTER_TYPE::RT_M_HL);
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		operationResult = (uint16_t)registerAContent - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)dataFromMemory,
			false,
			true
		);
		DISASSEMBLY("%04X CP A, (HL)", originalPC);
		BREAK;
	}
	case 0xBF:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)otherRegisterContent,
			false,
			true
		);
		DISASSEMBLY("%04X CP A, A", originalPC);
		BREAK;
	}
	case 0xC0:
	{
		cpuTickM();
		cpuTickM(INVALID, INVALID, CPU_TICK_TYPE::DUMMY);
		if (pGBc_flags->FZERO == 0)
		{
			BYTE lowerData = stackPop();
			cpuTickM();
			BYTE higherData = stackPop();
			cpuTickM();
			operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			SET_PC(operationResult);
			cpuTickM();
		}
		DISASSEMBLY("%04X RET NZ", originalPC);
		BREAK;
	}
	case 0xC1:
	{
		cpuTickM();
		BYTE lowerData = stackPop(YES);
		cpuTickM();
		BYTE higherData = stackPop();
		cpuTickM();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		cpuSetRegister(REGISTER_TYPE::RT_BC, operationResult);
		DISASSEMBLY("%04X POP BC", originalPC);
		BREAK;
	}
	case 0xC2:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pGBc_flags->FZERO == 0)
		{
			// last operation was not equal to zero, so jump
			SET_PC(operationResult);
			cpuTickM();
		}
		DISASSEMBLY("%04X JP NZ, %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xC3:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		SET_PC(operationResult);
		cpuTickM();
		DISASSEMBLY("%04X JP %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xC4:
	{
		cpuTickM();
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		// 3) check if the condition is met
		if (pGBc_flags->FZERO == 0)
		{
			BYTE higherData = (BYTE)(GET_PC() >> 8);
			BYTE lowerData = (BYTE)GET_PC();
			handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
			cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
			stackPush(higherData);
			cpuTickM(pGBc_registers->sp - ONE, lowerData);
			stackPush(lowerData);
			cpuTickM();
			// 4) set the pc to 16 bit address
			SET_PC(operationResult);
		}
		DISASSEMBLY("%04X CALL NZ, %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xC5:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_BC);
		BYTE higherData = (BYTE)(operationResult >> 8);
		BYTE lowerData = (BYTE)operationResult;
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();
		DISASSEMBLY("%04X PUSH BC", originalPC);
		BREAK;
	}
	case 0xC6:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = (cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory);
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADD A, %02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0xC7:
	{
		cpuTickM();
		// RST (CALL $0) opcode
		operationResult = 0x0000;

		BYTE higherData = (BYTE)(GET_PC() >> 8);
		BYTE lowerData = (BYTE)GET_PC();
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();

		SET_PC(operationResult);
		DISASSEMBLY("%04X RST 00H", originalPC);
		BREAK;
	}
	case 0xC8:
	{
		cpuTickM();
		cpuTickM(INVALID, INVALID, CPU_TICK_TYPE::DUMMY);
		if (pGBc_flags->FZERO)
		{
			BYTE lowerData = stackPop();
			cpuTickM();
			BYTE higherData = stackPop();
			cpuTickM();
			operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			SET_PC(operationResult);
			cpuTickM();
		}
		DISASSEMBLY("%04X RET Z", originalPC);
		BREAK;
	}
	case 0xC9:
	{
		cpuTickM();
		BYTE lowerData = stackPop();
		cpuTickM();
		BYTE higherData = stackPop();
		cpuTickM();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		SET_PC(operationResult);
		cpuTickM();
		DISASSEMBLY("%04X RET", originalPC);
		BREAK;
	}
	case 0xCA:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pGBc_flags->FZERO)
		{
			// last operation was equal to zero, so jump
			SET_PC(operationResult);
			cpuTickM();
		}
		DISASSEMBLY("%04X JP Z, %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xCB:
	{
		cpuTickM();
		process0xCBPrefixInstructions(originalPC);
		BREAK;
	}
	case 0xCC:
	{
		cpuTickM();
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		// 3) check if the condition is met
		if (pGBc_flags->FZERO)
		{
			BYTE higherData = (BYTE)(GET_PC() >> 8);
			BYTE lowerData = (BYTE)GET_PC();
			handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
			cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
			stackPush(higherData);
			cpuTickM(pGBc_registers->sp - ONE, lowerData);
			stackPush(lowerData);
			cpuTickM();
			// 4) set the pc to 16 bit address
			SET_PC(operationResult);
		}
		DISASSEMBLY("%04X CALL Z, %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xCD:
	{
		cpuTickM();
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		BYTE higherData = (BYTE)(GET_PC() >> 8);
		BYTE lowerData = (BYTE)GET_PC();
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();
		// 3) set the pc to 16 bit address
		SET_PC(operationResult);
		DISASSEMBLY("%04X CALL %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xCE:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory + (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitAdditionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X ADC A, %02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0xCF:
	{
		cpuTickM();
		// RST (CALL $08) opcode
		operationResult = 0x0008;

		BYTE higherData = (BYTE)(GET_PC() >> 8);
		BYTE lowerData = (BYTE)GET_PC();
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();

		SET_PC(operationResult);
		DISASSEMBLY("%04X RST 08H", originalPC);
		BREAK;
	}
	case 0xD0:
	{
		cpuTickM();
		cpuTickM(INVALID, INVALID, CPU_TICK_TYPE::DUMMY);
		if (pGBc_flags->FCARRY == 0)
		{
			BYTE lowerData = stackPop();
			cpuTickM();
			BYTE higherData = stackPop();
			cpuTickM();
			operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			SET_PC(operationResult);
			cpuTickM();
		}
		DISASSEMBLY("%04X RET NC", originalPC);
		BREAK;
	}
	case 0xD1:
	{
		cpuTickM();
		BYTE lowerData = stackPop(YES);
		cpuTickM();
		BYTE higherData = stackPop();
		cpuTickM();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		cpuSetRegister(REGISTER_TYPE::RT_DE, operationResult);
		DISASSEMBLY("%04X POP DE", originalPC);
		BREAK;
	}
	case 0xD2:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pGBc_flags->FCARRY == 0)
		{
			// last operation resulted in not setting the carry
			SET_PC(operationResult);
			cpuTickM();
		}
		DISASSEMBLY("%04X JP NC, %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xD3:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X OUT (n), A", originalPC);
		BREAK;
	}
	case 0xD4:
	{
		cpuTickM();
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		// 3) check if the condition is met
		if (pGBc_flags->FCARRY == 0)
		{
			BYTE higherData = (BYTE)(GET_PC() >> 8);
			BYTE lowerData = (BYTE)GET_PC();
			handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
			cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
			stackPush(higherData);
			cpuTickM(pGBc_registers->sp - ONE, lowerData);
			stackPush(lowerData);
			cpuTickM();
			// 4) set the pc to 16 bit address
			SET_PC(operationResult);
		}
		DISASSEMBLY("%04X CALL NC, %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xD5:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_DE);
		BYTE higherData = (BYTE)(operationResult >> 8);
		BYTE lowerData = (BYTE)operationResult;
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();
		DISASSEMBLY("%04X PUSH DE", originalPC);
		BREAK;
	}
	case 0xD6:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			false,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SUB %02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0xD7:
	{
		cpuTickM();
		// RST (CALL $10) opcode
		operationResult = 0x0010;

		BYTE higherData = (BYTE)(GET_PC() >> 8);
		BYTE lowerData = (BYTE)GET_PC();
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();

		SET_PC(operationResult);
		DISASSEMBLY("%04X RST 10H", originalPC);
		BREAK;
	}
	case 0xD8:
	{
		cpuTickM();
		cpuTickM(INVALID, INVALID, CPU_TICK_TYPE::DUMMY);
		if (pGBc_flags->FCARRY)
		{
			BYTE lowerData = stackPop();
			cpuTickM();
			BYTE higherData = stackPop();
			cpuTickM();
			operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			SET_PC(operationResult);
			cpuTickM();
		}
		DISASSEMBLY("%04X RET C", originalPC);
		BREAK;
	}
	case 0xD9:
	{
		cpuTickM();
		BYTE lowerData = stackPop();
		cpuTickM();
		BYTE higherData = stackPop();
		cpuTickM();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		SET_PC(operationResult);
		cpuTickM();
		// enable IME
		pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn = ENABLED;
		pGBc_instance->GBc_state.emulatorStatus.eiEnState = EI_ENABLE_STATE::NOTHING_TO_BE_DONE;
		DISASSEMBLY("%04X RETI", originalPC);
		BREAK;
	}
	case 0xDA:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pGBc_flags->FCARRY)
		{
			// last operation resulted in setting the carry
			SET_PC(operationResult);
			cpuTickM();
		}
		DISASSEMBLY("%04X JP C, %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xDB:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X OUT (n), A", originalPC);
		BREAK;
	}
	case 0xDC:
	{
		cpuTickM();
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 			
		// 3) check if the condition is met
		if (pGBc_flags->FCARRY)
		{
			BYTE higherData = (BYTE)(GET_PC() >> 8);
			BYTE lowerData = (BYTE)GET_PC();
			handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
			cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
			stackPush(higherData);
			cpuTickM(pGBc_registers->sp - ONE, lowerData);
			stackPush(lowerData);
			cpuTickM();
			// 4) set the pc to 16 bit address
			SET_PC(operationResult);
		}
		DISASSEMBLY("%04X CALL C, %04X", originalPC, operationResult);
		BREAK;
	}
	case 0xDD:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X CALL/JP IX", originalPC);
		BREAK;
	}
	case 0xDE:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A)
			- (uint16_t)dataFromMemory - (uint16_t)pGBc_flags->FCARRY;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)cpuReadRegister(REGISTER_TYPE::RT_A),
			(byte)dataFromMemory,
			true,
			true
		);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		DISASSEMBLY("%04X SBC A, %02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0xDF:
	{
		cpuTickM();
		// RST (CALL $18) opcode
		operationResult = 0x0018;

		BYTE higherData = (BYTE)(GET_PC() >> 8);
		BYTE lowerData = (BYTE)GET_PC();
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();

		SET_PC(operationResult);
		DISASSEMBLY("%04X RST 18H", originalPC);
		BREAK;
	}
	case 0xE0:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM(0xFF00 + dataFromMemory, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		INCREMENT_PC_BY_ONE();
		writeRawMemory(0xFF00 + dataFromMemory, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LDH (a8), A", originalPC);
		BREAK;
	}
	case 0xE1:
	{
		cpuTickM();
		BYTE lowerData = stackPop(YES);
		cpuTickM();
		BYTE higherData = stackPop();
		cpuTickM();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		cpuSetRegister(REGISTER_TYPE::RT_HL, operationResult);
		DISASSEMBLY("%04X POP HL", originalPC);
		BREAK;
	}
	case 0xE2:
	{
		cpuTickM((0xFF00 + (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C)), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		writeRawMemory((0xFF00 + (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C)), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (C), A", originalPC);
		BREAK;
	}
	case 0xE3:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X UNIMPLEMENTED", originalPC);
		BREAK;
	}
	case 0xE4:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X UNIMPLEMENTED", originalPC);
		BREAK;
	}
	case 0xE5:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL);
		BYTE higherData = (BYTE)(operationResult >> 8);
		BYTE lowerData = (BYTE)operationResult;
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();
		DISASSEMBLY("%04X PUSH HL", originalPC);
		BREAK;
	}
	case 0xE6:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = ((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			true
		);
		DISASSEMBLY("%04X AND %02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0xE7:
	{
		cpuTickM();
		// RST (CALL $20) opcode
		operationResult = 0x0020;

		BYTE higherData = (BYTE)(GET_PC() >> 8);
		BYTE lowerData = (BYTE)GET_PC();
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();

		SET_PC(operationResult);
		DISASSEMBLY("%04X RST 20H", originalPC);
		BREAK;
	}
	case 0xE8:
	{
		cpuTickM();
		dataFromMemory = (SBYTE)readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = (int)cpuReadRegister(REGISTER_TYPE::RT_SP) + (SBYTE)dataFromMemory;
		cpuTickM();
		cpuTickM();
		processFlagFor0xE8And0xF8
		(
			(SBYTE)cpuReadRegister(REGISTER_TYPE::RT_SP),
			(SBYTE)dataFromMemory
		);
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		DISASSEMBLY("%04X ADD SP, %02X", originalPC, (BYTE)dataFromMemory);
		BREAK;
	}
	case 0xE9:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL);
		SET_PC(operationResult);
		DISASSEMBLY("%04X JP (HL)", originalPC);
		BREAK;
	}
	case 0xEA:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		cpuTickM(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		writeRawMemory(operationResult, (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		DISASSEMBLY("%04X LD (a16), A", originalPC);
		BREAK;
	}
	case 0xEB:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X UNIMPLEMENTED", originalPC);
		BREAK;
	}
	case 0xEC:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X UNIMPLEMENTED", originalPC);
		BREAK;
	}
	case 0xED:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X UNIMPLEMENTED", originalPC);
		BREAK;
	}
	case 0xEE:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = ((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		DISASSEMBLY("%04X XOR %02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0xEF:
	{
		cpuTickM();
		// RST (CALL $28) opcode
		operationResult = 0x0028;

		BYTE higherData = (BYTE)(GET_PC() >> 8);
		BYTE lowerData = (BYTE)GET_PC();
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();

		SET_PC(operationResult);
		DISASSEMBLY("%04X RST 28H", originalPC);
		BREAK;
	}
	case 0xF0:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = readRawMemory(0xFF00 + dataFromMemory, MEMORY_ACCESS_SOURCE::CPU);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		cpuTickM();
		DISASSEMBLY("%04X LDH A, (a8)", originalPC);
		BREAK;
	}
	case 0xF1:
	{
		cpuTickM();
		BYTE lowerData = stackPop(YES);
		cpuTickM();
		BYTE higherData = stackPop();
		cpuTickM();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		cpuSetRegister(REGISTER_TYPE::RT_AF, operationResult);
		DISASSEMBLY("%04X POP AF", originalPC);
		BREAK;
	}
	case 0xF2:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(0xFF00 + (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		DISASSEMBLY("%04X LD A, (C)", originalPC);
		BREAK;
	}
	case 0xF3:
	{
		cpuTickM();
		// disable IME

		pGBc_instance->GBc_state.emulatorStatus.eiEnState = EI_ENABLE_STATE::NOTHING_TO_BE_DONE;

		// DI is never delayed, https://github.com/LIJI32/SameBoy/blob/master/Core/sm83_cpu.c#L1347
		pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn = DISABLED;

		DISASSEMBLY("%04X DI", originalPC);
		BREAK;
	}
	case 0xF4:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X UNIMPLEMENTED", originalPC);
		BREAK;
	}
	case 0xF5:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_AF);
		BYTE higherData = (BYTE)(operationResult >> 8);
		BYTE lowerData = (BYTE)operationResult;
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();
		DISASSEMBLY("%04X PUSH AF", originalPC);
		BREAK;
	}
	case 0xF6:
	{
		cpuTickM();
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = ((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		processFlagsForLogicalOperation
		(
			(byte)operationResult,
			false
		);
		DISASSEMBLY("%04X OR %02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0xF7:
	{
		cpuTickM();
		// RST (CALL $30) opcode
		operationResult = 0x0030;

		BYTE higherData = (BYTE)(GET_PC() >> 8);
		BYTE lowerData = (BYTE)GET_PC();
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();

		SET_PC(operationResult);
		DISASSEMBLY("%04X RST 30H", originalPC);
		BREAK;
	}
	case 0xF8:
	{
		cpuTickM();
		dataFromMemory = (SBYTE)readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = (int)cpuReadRegister(REGISTER_TYPE::RT_SP) + dataFromMemory;
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_HL, operationResult);
		processFlagFor0xE8And0xF8
		(
			(SBYTE)cpuReadRegister(REGISTER_TYPE::RT_SP),
			(SBYTE)dataFromMemory
		);
		DISASSEMBLY("%04X LD HL, SP+%02X", originalPC, (BYTE)dataFromMemory);
		BREAK;
	}
	case 0xF9:
	{
		cpuTickM();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuTickM(INVALID, INVALID, CPU_TICK_TYPE::DUMMY);
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		DISASSEMBLY("%04X LD SP, HL", originalPC);
		BREAK;
	}
	case 0xFA:
	{
		cpuTickM();
		lowerDataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		higherDataFromMemory = readRawMemory(GET_PC() + ONE, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		dataFromMemory = readRawMemory(operationResult, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		DISASSEMBLY("%04X LD A, (a16)", originalPC);
		BREAK;
	}
	case 0xFB:
	{
		cpuTickM();
		pGBc_instance->GBc_state.emulatorStatus.eiEnState = EI_ENABLE_STATE::EI_TO_BE_ENABLED;
		DISASSEMBLY("%04X EI", originalPC);
		BREAK;
	}
	case 0xFC:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X UNIMPLEMENTED", originalPC);
		BREAK;
	}
	case 0xFD:
	{
		cpuTickM();
		unimplementedInstruction();
		DISASSEMBLY("%04X UNIMPLEMENTED", originalPC);
		BREAK;
	}
	case 0xFE:
	{
		cpuTickM();
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		dataFromMemory = readRawMemory(GET_PC(), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickM();
		INCREMENT_PC_BY_ONE();
		operationResult = (uint16_t)registerAContent - (uint16_t)dataFromMemory;
		processFlagsFor8BitSubtractionOperation
		(
			(byte)registerAContent,
			(byte)dataFromMemory,
			false,
			true
		);
		DISASSEMBLY("%04X CP %02X", originalPC, dataFromMemory);
		BREAK;
	}
	case 0xFF:
	{
		cpuTickM();
		// RST (CALL $38) opcode
		operationResult = 0x0038;

		BYTE higherData = (BYTE)(GET_PC() >> 8);
		BYTE lowerData = (BYTE)GET_PC();
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		cpuTickM(pGBc_registers->sp - ONE, higherData, CPU_TICK_TYPE::DUMMY);
		stackPush(higherData);
		cpuTickM(pGBc_registers->sp - ONE, lowerData);
		stackPush(lowerData);
		cpuTickM();

		SET_PC(operationResult);
		DISASSEMBLY("%04X RST 38H", originalPC);
		BREAK;
	}
	default:
	{
		cpuTickM();
		unimplementedInstruction();
		BREAK;
	}
	}

	processUnusedFlags(ZERO);
	if (ROM_TYPE != ROM::TEST_SST)
	{
		processUnusedJoyPadBits(ONE);
		processUnusedIFBits(ONE);
	}
}