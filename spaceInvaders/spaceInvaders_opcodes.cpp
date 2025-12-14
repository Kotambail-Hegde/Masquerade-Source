#include "spaceInvaders.h"

FLAG spaceInvaders_t::performOperation(int32_t anySpecificOpcode)
{
	auto INCREMENT_PC_BY_ONE = [&]()
		{
			pSi_registers->pc++;
		};

	FLAG status = true;

	pSi_cpuInstance->previousOpcode = pSi_cpuInstance->opcode;
	pSi_cpuInstance->opcode = pSi_memory->rawMemory[pSi_registers->pc];
	// Memory read usually takes 3 T cycles in 8080
	cpuTickT(); 
	cpuTickT(); 
	cpuTickT(); 
	BYTE opcode = pSi_cpuInstance->opcode;

	// Below if condition is used only in TEST MODE
	if (anySpecificOpcode != INVALID)
	{
		opcode = (anySpecificOpcode & 0xFF);
	}

	INCREMENT_PC_BY_ONE();
	// Register increment/decrement usually takes 1 T cycle in 8080
	cpuTickT(); 

	// Reset the non-modifiable bits to default just in case it got modified

	pSi_flags->ONE1 = ONE;
	pSi_flags->ZERO1 = ZERO;
	pSi_flags->ZERO2 = ZERO;

	auto operationResult = RESET;
	auto dataFromMemory = RESET;
	auto lowerDataFromMemory = RESET;
	auto higherDataFromMemory = RESET;

	switch (opcode)
	{
	case 0x00:
	{
		BREAK;
	}
	case 0x01:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_BC, UWORD_FROM_UBYTES(higherDataFromMemory, lowerDataFromMemory));
		BREAK;
	}
	case 0x02:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_BC);
		pSi_memory->rawMemory[operationResult] = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x03:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_BC) + ONE;
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_BC, operationResult);
		BREAK;
	}
	case 0x04:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B) + ONE;
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_B), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		BREAK;
	}
	case 0x05:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B) - ONE;
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_B), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		BREAK;
	}
	case 0x06:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		BREAK;
	}
	case 0x07:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		if (registerAContent & 0x80)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		operationResult = ((registerAContent >> 7) | (registerAContent << 1));
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x08:
	{
		BREAK;
	}
	case 0x09:
	{
		//	T-Cycle breakdown is as follows
		//
		//	| M-Cycle | T-State | Description                       |
		//	| ------- | ------- | --------------------------------- |
		//	| **M1**  | T1      | Place PC on address bus, ALE high |
		//	|         | T2      | RD low, memory puts opcode        |
		//	|         | T3      | Opcode latched (`0x09`), RD high  |
		//	|         | T4      | **PC incremented**                |
		//	| **M2**  | T5傍10  | Internal 16-bit ALU operation:    |
		//	|         | T5      | Read L and C                      |
		//	|         | T6      | Add L + C                         |
		//	|         | T7      | Read H and B                      |
		//	|         | T8      | Add H + B + carry from low byte   |
		//	|         | T9      | Store result in HL (low byte)     |
		//	|         | T10     | Store result in HL (high byte)    |

		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL) + (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_BC);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		if (operationResult & 0xFFFF0000)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);

		BREAK;
	}
	case 0x0A:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->bc.bc_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		BREAK;
	}
	case 0x0B:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_BC) - 1;
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_BC, operationResult);
		BREAK;
	}
	case 0x0C:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C) + 1;
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_C), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		BREAK;
	}
	case 0x0D:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C) - 1;
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_C), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		BREAK;
	}
	case 0x0E:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		BREAK;
	}
	case 0x0F:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		if (registerAContent & 0x01)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		operationResult = ((registerAContent << 7) | (registerAContent >> 1));
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x10:
	{
		BREAK;
	}
	case 0x11:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_DE, UWORD_FROM_UBYTES(higherDataFromMemory, lowerDataFromMemory));
		BREAK;
	}
	case 0x12:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_DE);
		pSi_memory->rawMemory[operationResult] = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x13:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_DE) + 1;
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_DE, operationResult);
		BREAK;
	}
	case 0x14:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D) + 1;
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_D), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		BREAK;
	}
	case 0x15:
	{
		BYTE operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D) - 1;
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_D), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		BREAK;
	}
	case 0x16:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		BREAK;
	}
	case 0x17:
	{
		BYTE FCarry = pSi_flags->FCARRY;
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		if (registerAContent & 0x80)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		operationResult = ((registerAContent << 1) | FCarry);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x18:
	{
		BREAK;
	}
	case 0x19:
	{
		//	T-Cycle breakdown is as follows
		//
		//	| M-Cycle | T-State | Description                       |
		//	| ------- | ------- | --------------------------------- |
		//	| **M1**  | T1      | Place PC on address bus, ALE high |
		//	|         | T2      | RD low, memory puts opcode        |
		//	|         | T3      | Opcode latched (`0x09`), RD high  |
		//	|         | T4      | **PC incremented**                |
		//	| **M2**  | T5傍10  | Internal 16-bit ALU operation:    |
		//	|         | T5      | Read L and E                      |
		//	|         | T6      | Add L + E                         |
		//	|         | T7      | Read H and D                      |
		//	|         | T8      | Add H + D + carry from low byte   |
		//	|         | T9      | Store result in HL (low byte)     |
		//	|         | T10     | Store result in HL (high byte)    |

		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL) + (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_DE);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		if (operationResult & 0xFFFF0000)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		BREAK;
	}
	case 0x1A:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->de.de_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		BREAK;
	}
	case 0x1B:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_DE) - 1;
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_DE, operationResult);
		BREAK;
	}
	case 0x1C:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E) + 1;
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_E), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		BREAK;
	}
	case 0x1D:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E) - 1;
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_E), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		BREAK;
	}
	case 0x1E:
	{
		BYTE dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		BREAK;
	}
	case 0x1F:
	{
		BYTE FCarry = pSi_flags->FCARRY;
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		if (registerAContent & 0x01)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		operationResult = ((registerAContent >> 1) | (FCarry << 7));
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x20:
	{
		BREAK;
	}
	case 0x21:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_HL, UWORD_FROM_UBYTES(higherDataFromMemory, lowerDataFromMemory));
		BREAK;
	}
	case 0x22:
	{
		lowerDataFromMemory = (uint16_t)(pSi_memory->rawMemory[pSi_registers->pc]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = (uint16_t)(pSi_memory->rawMemory[pSi_registers->pc + ONE]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		pSi_memory->rawMemory[operationResult] = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_memory->rawMemory[operationResult + 1] = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x23:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL) + 1;
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, operationResult);
		BREAK;
	}
	case 0x24:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H) + 1;
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_H), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		BREAK;
	}
	case 0x25:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H) - 1;
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_H), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		BREAK;
	}
	case 0x26:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		BREAK;
	}
	case 0x27:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A);
		if ((operationResult & 0x0F) > 0x09 || pSi_flags->FAuxCARRY)
		{
			if (((operationResult & 0x0F) + 0x06) & 0xF0)
			{
				pSi_flags->FAuxCARRY = 1;
			}
			else
			{
				pSi_flags->FAuxCARRY = 0;
			}

			operationResult += 0x06;

			if (operationResult & 0xFF00)
			{
				pSi_flags->FCARRY = 1;
			}
		}
		if ((operationResult & 0xF0) > 0x90 || pSi_flags->FCARRY)
		{
			operationResult += 0x60;

			if (operationResult & 0xFF00)
			{
				pSi_flags->FCARRY = 1;
			}
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x28:
	{
		BREAK;
	}
	case 0x29:
	{
		//	T-Cycle breakdown is as follows
		//
		//	| M-Cycle | T-State | Description                       |
		//	| ------- | ------- | --------------------------------- |
		//	| **M1**  | T1      | Place PC on address bus, ALE high |
		//	|         | T2      | RD low, memory puts opcode        |
		//	|         | T3      | Opcode latched (`0x09`), RD high  |
		//	|         | T4      | **PC incremented**                |
		//	| **M2**  | T5傍10  | Internal 16-bit ALU operation:    |
		//	|         | T5      | Read L and L                      |
		//	|         | T6      | Add L + L                         |
		//	|         | T7      | Read H and H                      |
		//	|         | T8      | Add H + H + carry from low byte   |
		//	|         | T9      | Store result in HL (low byte)     |
		//	|         | T10     | Store result in HL (high byte)    |

		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL) + (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		if (operationResult & 0xFFFF0000)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		BREAK;
	}
	case 0x2A:
	{
		uint16_t lowerDataFromMemory = (uint16_t)(pSi_memory->rawMemory[pSi_registers->pc]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)(pSi_memory->rawMemory[pSi_registers->pc + ONE]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		uint16_t operationResult = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_L, (uint16_t)pSi_memory->rawMemory[operationResult]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, (uint16_t)pSi_memory->rawMemory[operationResult + 1]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x2B:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL) - 1;
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, operationResult);
		BREAK;
	}
	case 0x2C:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L) + 1;
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_L), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		BREAK;
	}
	case 0x2D:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L) - 1;
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_L), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		BREAK;
	}
	case 0x2E:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		BREAK;
	}
	case 0x2F:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		operationResult = ~operationResult;
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x30:
	{
		BREAK;
	}
	case 0x31:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		BREAK;
	}
	case 0x32:
	{
		lowerDataFromMemory = (uint16_t)(pSi_memory->rawMemory[pSi_registers->pc]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		uint16_t higherDataFromMemory = (uint16_t)(pSi_memory->rawMemory[pSi_registers->pc + ONE]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		uint16_t operationResult = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		pSi_memory->rawMemory[operationResult] = TO_UINT8(cpuReadRegister(REGISTER_TYPE::RT_A));
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x33:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_SP) + 1;
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		BREAK;
	}
	case 0x34:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = dataFromMemory + 1;
		process_AC(dataFromMemory, 1);
		process_SZP(operationResult);
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = operationResult;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x35:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = dataFromMemory - 1;
		process_AB(dataFromMemory, 1);
		process_SZP(operationResult);
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = operationResult;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x36:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = dataFromMemory;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x37:
	{
		pSi_flags->FCARRY = 1;
		BREAK;
	}
	case 0x38:
	{
		BREAK;
	}
	case 0x39:
	{
		//	T-Cycle breakdown is as follows
		//
		//	| M-Cycle | T-State | Description								|
		//	| ------- | ------- | ------------------------------------------|
		//	| **M1**  | T1      | Place PC on address bus, ALE high			|
		//	|         | T2      | RD low, memory puts opcode				|
		//	|         | T3      | Opcode latched (`0x09`), RD high			|
		//	|         | T4      | **PC incremented**						|
		//	| **M2**  | T5傍10  | Internal 16-bit ALU operation:			|
		//	|         | T5      | Read L and SP low							|
		//	|         | T6      | Add L + SP low							|
		//	|         | T7      | Read H and SP high						|
		//	|         | T8      | Add H + SP high + carry from low byte		|
		//	|         | T9      | Store result in HL (low byte)				|
		//	|         | T10     | Store result in HL (high byte)			|

		uint32_t operationResult = (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_HL) + (uint32_t)cpuReadRegister(REGISTER_TYPE::RT_SP);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		if (operationResult & 0xFFFF0000)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_HL, (uint16_t)operationResult);
		BREAK;
	}
	case 0x3A:
	{
		// 1) get the next 16 bit address immediately after the LDA opcode
		lowerDataFromMemory = (uint16_t)(pSi_memory->rawMemory[pSi_registers->pc]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = (uint16_t)(pSi_memory->rawMemory[pSi_registers->pc + ONE]);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		uint16_t address = ((higherDataFromMemory << 8) | lowerDataFromMemory);
		// 2) read the data from obtained 16 bit address
		operationResult = pSi_memory->rawMemory[address];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		// 3) set the data to destination register
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x3B:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_SP) - 1;
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		BREAK;
	}
	case 0x3C:
	{
		BYTE operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) + 1;
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x3D:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) - 1;
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), 1);
		process_SZP(operationResult);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x3E:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		BREAK;
	}
	case 0x3F:
	{
		pSi_flags->FCARRY = (pSi_flags->FCARRY == 1 ? 0 : 1);
		BREAK;
	}
	case 0x40:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		BREAK;
	}
	case 0x41:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		BREAK;
	}
	case 0x42:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		BREAK;
	}
	case 0x43:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		BREAK;
	}
	case 0x44:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		BREAK;
	}
	case 0x45:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		BREAK;
	}
	case 0x46:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, dataFromMemory);
		BREAK;
	}
	case 0x47:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_B, operationResult);
		BREAK;
	}
	case 0x48:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		BREAK;
	}
	case 0x49:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		BREAK;
	}
	case 0x4A:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		BREAK;
	}
	case 0x4B:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		BREAK;
	}
	case 0x4C:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		BREAK;
	}
	case 0x4D:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		BREAK;
	}
	case 0x4E:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, dataFromMemory);
		BREAK;
	}
	case 0x4F:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_C, operationResult);
		BREAK;
	}
	case 0x50:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		BREAK;
	}
	case 0x51:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		BREAK;
	}
	case 0x52:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		BREAK;
	}
	case 0x53:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		BREAK;
	}
	case 0x54:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		BREAK;
	}
	case 0x55:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		BREAK;
	}
	case 0x56:
	{
		BYTE dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, dataFromMemory);
		BREAK;
	}
	case 0x57:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_D, operationResult);
		BREAK;
	}
	case 0x58:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		BREAK;
	}
	case 0x59:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		BREAK;
	}
	case 0x5A:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		BREAK;
	}
	case 0x5B:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		BREAK;
	}
	case 0x5C:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		BREAK;
	}
	case 0x5D:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		BREAK;
	}
	case 0x5E:
	{
		BYTE dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, dataFromMemory);
		BREAK;
	}
	case 0x5F:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_E, operationResult);
		BREAK;
	}
	case 0x60:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		BREAK;
	}
	case 0x61:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		BREAK;
	}
	case 0x62:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		BREAK;
	}
	case 0x63:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		BREAK;
	}
	case 0x64:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		BREAK;
	}
	case 0x65:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		BREAK;
	}
	case 0x66:
	{
		BYTE dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, dataFromMemory);
		BREAK;
	}
	case 0x67:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_H, operationResult);
		BREAK;
	}
	case 0x68:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		BREAK;
	}
	case 0x69:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		BREAK;
	}
	case 0x6A:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		BREAK;
	}
	case 0x6B:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		BREAK;
	}
	case 0x6C:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		BREAK;
	}
	case 0x6D:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		BREAK;
	}
	case 0x6E:
	{
		BYTE dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, dataFromMemory);
		BREAK;
	}
	case 0x6F:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, operationResult);
		BREAK;
	}
	case 0x70:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = operationResult;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x71:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = operationResult;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x72:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = operationResult;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x73:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = operationResult;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x74:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = operationResult;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x75:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = operationResult;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x76:
	{
		LOG("HALT");
		cpuTickT();
		cpuTickT();
		cpuTickT();
		unimplementedInstruction();
		BREAK;
	}
	case 0x77:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] = operationResult;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0x78:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x79:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x7A:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x7B:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x7C:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x7D:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x7E:
	{
		BYTE dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, dataFromMemory);
		BREAK;
	}
	case 0x7F:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x80:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_B);
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x81:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_C);
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x82:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_D);
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x83:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_E);
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x84:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_H);

		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x85:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_L);

		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L));
		process_SZP((BYTE)operationResult);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);

		BREAK;
	}
	case 0x86:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		uint16_t operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory;
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x87:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + cpuReadRegister(REGISTER_TYPE::RT_A);
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x88:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_B)
			+ (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AC_withCarry((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B));
		}
		else
		{
			process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B));
		}
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x89:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_C)
			+ (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AC_withCarry((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C));
		}
		else
		{
			process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C));
		}
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x8A:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_D)
			+ (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AC_withCarry((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D));
		}
		else
		{
			process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D));
		}
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x8B:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_E)
			+ (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AC_withCarry((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E));
		}
		else
		{
			process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E));
		}
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x8C:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_H)
			+ (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AC_withCarry((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H));
		}
		else
		{
			process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H));
		}
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x8D:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_L)
			+ (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AC_withCarry((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L));
		}
		else
		{
			process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L));
		}
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x8E:
	{
		dataFromMemory = (BYTE)(pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory] & 0xFF);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ (uint16_t)dataFromMemory
			+ (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AC_withCarry((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		}
		else
		{
			process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		}
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x8F:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			+ cpuReadRegister(REGISTER_TYPE::RT_A)
			+ (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AC_withCarry((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		}
		else
		{
			process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		}
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x90:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_B);
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A)) && cpuReadRegister(REGISTER_TYPE::RT_B))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x91:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_C);
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A)) && cpuReadRegister(REGISTER_TYPE::RT_C))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x92:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_D);
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A)) && cpuReadRegister(REGISTER_TYPE::RT_D))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x93:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_E);
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A)) && cpuReadRegister(REGISTER_TYPE::RT_E))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x94:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_H);
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A)) && cpuReadRegister(REGISTER_TYPE::RT_H))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x95:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_L);
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A)) && cpuReadRegister(REGISTER_TYPE::RT_L))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x96:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - (uint16_t)dataFromMemory;

		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A)) && dataFromMemory)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x97:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - cpuReadRegister(REGISTER_TYPE::RT_A);
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A)) && cpuReadRegister(REGISTER_TYPE::RT_A))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x98:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_B)
			- (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AB_withBorrow((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B));
		}
		else
		{
			process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B));
		}
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A))
			&& (cpuReadRegister(REGISTER_TYPE::RT_B) | pSi_flags->FCARRY))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x99:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_C)
			- (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AB_withBorrow((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C));
		}
		else
		{
			process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C));
		}
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A))
			&& (cpuReadRegister(REGISTER_TYPE::RT_C) | pSi_flags->FCARRY))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x9A:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_D)
			- (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AB_withBorrow((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D));
		}
		else
		{
			process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D));
		}
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A))
			&& (cpuReadRegister(REGISTER_TYPE::RT_D) | pSi_flags->FCARRY))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x9B:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_E)
			- (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AB_withBorrow((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E));
		}
		else
		{
			process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E));
		}
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A))
			&& (cpuReadRegister(REGISTER_TYPE::RT_E) | pSi_flags->FCARRY))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x9C:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_H)
			- (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AB_withBorrow((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H));
		}
		else
		{
			process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H));
		}
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A))
			&& (cpuReadRegister(REGISTER_TYPE::RT_H) | pSi_flags->FCARRY))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x9D:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_L)
			- (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AB_withBorrow((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L));
		}
		else
		{
			process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L));
		}
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A))
			&& (cpuReadRegister(REGISTER_TYPE::RT_L) | pSi_flags->FCARRY))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x9E:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- (uint16_t)dataFromMemory
			- (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AB_withBorrow((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		}
		else
		{
			process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		}
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A))
			&& (dataFromMemory | pSi_flags->FCARRY))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0x9F:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A)
			- cpuReadRegister(REGISTER_TYPE::RT_A)
			- (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AB_withBorrow((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		}
		else
		{
			process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A));
		}
		if (((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A))
			&& (cpuReadRegister(REGISTER_TYPE::RT_A) | pSi_flags->FCARRY))
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA0:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		if (((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B)) & 0x08)
		{
			pSi_flags->FAuxCARRY = 1;
		}
		else
		{
			pSi_flags->FAuxCARRY = 0;
		}
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA1:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		if (((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C)) & 0x08)
		{
			pSi_flags->FAuxCARRY = 1;
		}
		else
		{
			pSi_flags->FAuxCARRY = 0;
		}
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA2:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		if (((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D)) & 0x08)
		{
			pSi_flags->FAuxCARRY = 1;
		}
		else
		{
			pSi_flags->FAuxCARRY = 0;
		}
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA3:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		if (((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E)) & 0x08)
		{
			pSi_flags->FAuxCARRY = 1;
		}
		else
		{
			pSi_flags->FAuxCARRY = 0;
		}
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);

		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA4:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);

		if (((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H)) & 0x08)
		{
			pSi_flags->FAuxCARRY = 1;
		}
		else
		{
			pSi_flags->FAuxCARRY = 0;
		}
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA5:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		if (((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L)) & 0x08)
		{
			pSi_flags->FAuxCARRY = 1;
		}
		else
		{
			pSi_flags->FAuxCARRY = 0;
		}
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA6:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & dataFromMemory;
		if (((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)dataFromMemory) & 0x08)
		{
			pSi_flags->FAuxCARRY = 1;
		}
		else
		{
			pSi_flags->FAuxCARRY = 0;
		}
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA7:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		if (((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A)) & 0x08)
		{
			pSi_flags->FAuxCARRY = 1;
		}
		else
		{
			pSi_flags->FAuxCARRY = 0;
		}
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA8:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_B);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xA9:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_C);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xAA:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_D);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xAB:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_E);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xAC:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_H);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xAD:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_L);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xAE:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ dataFromMemory;

		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xAF:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) ^ cpuReadRegister(REGISTER_TYPE::RT_A);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xB0:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xB1:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xB2:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xB3:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xB4:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xB5:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xB6:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) | dataFromMemory;

		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xB7:
	{
		operationResult = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xB8:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_B);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;

		if (((operationResult & 0x00FF) >= registerAContent) && otherRegisterContent)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB(registerAContent, otherRegisterContent);
		process_SZP((BYTE)operationResult);
		BREAK;
	}
	case 0xB9:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_C);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;

		if (((operationResult & 0x00FF) >= registerAContent) && otherRegisterContent)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB(registerAContent, otherRegisterContent);
		process_SZP((BYTE)operationResult);
		BREAK;
	}
	case 0xBA:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_D);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;

		if (((operationResult & 0x00FF) >= registerAContent) && otherRegisterContent)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB(registerAContent, otherRegisterContent);
		process_SZP((BYTE)operationResult);
		BREAK;
	}
	case 0xBB:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_E);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;

		if (((operationResult & 0x00FF) >= registerAContent) && otherRegisterContent)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB(registerAContent, otherRegisterContent);
		process_SZP((BYTE)operationResult);
		BREAK;
	}
	case 0xBC:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;

		if (((operationResult & 0x00FF) >= registerAContent) && otherRegisterContent)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB(registerAContent, otherRegisterContent);
		process_SZP((BYTE)operationResult);
		BREAK;
	}
	case 0xBD:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;

		if (((operationResult & 0x00FF) >= registerAContent) && otherRegisterContent)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB(registerAContent, otherRegisterContent);
		process_SZP((BYTE)operationResult);
		BREAK;
	}
	case 0xBE:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE dataFromMemory = pSi_memory->rawMemory[pSi_registers->hl.hl_u16memory];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = (uint16_t)registerAContent - (uint16_t)dataFromMemory;

		if (((operationResult & 0x00FF) >= registerAContent) && dataFromMemory)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB(registerAContent, dataFromMemory);
		process_SZP((BYTE)operationResult);
		BREAK;
	}
	case 0xBF:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE otherRegisterContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		operationResult = (uint16_t)registerAContent - (uint16_t)otherRegisterContent;

		if (((operationResult & 0x00FF) >= registerAContent) && otherRegisterContent)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB(registerAContent, otherRegisterContent);
		process_SZP((BYTE)operationResult);
		BREAK;
	}
	case 0xC0:
	{
		cpuTickT();
		if (pSi_flags->FZERO == 0)
		{
			BYTE lowerData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			BYTE higherData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xC1:
	{
		BYTE lowerData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BYTE higherData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		cpuSetRegister(REGISTER_TYPE::RT_BC, operationResult);
		BREAK;
	}
	case 0xC2:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pSi_flags->FZERO == 0)
		{
			// last operation was not equal to zero, so jump
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xC3:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xC4:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		cpuTickT();
		if (pSi_flags->FZERO == 0)
		{
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			stackPush(lowerData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xC5:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_BC);
		BYTE higherData = (BYTE)(operationResult >> 8);
		BYTE lowerData = (BYTE)operationResult;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0xC6:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		operationResult = (cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory);
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xC7:
	{
		// RST (CALL $0) opcode
		operationResult = 0x0000;
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xC8:
	{
		cpuTickT();
		if (pSi_flags->FZERO)
		{
			BYTE lowerData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			BYTE higherData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			uint16_t operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xC9:
	{
		BYTE lowerData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BYTE higherData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		uint16_t operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xCA:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pSi_flags->FZERO)
		{
			// last operation was equal to zero, so jump
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xCB:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xCC:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		cpuTickT();
		if (pSi_flags->FZERO)
		{
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			stackPush(lowerData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xCD:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		cpuTickT();
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xCE:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) + (uint16_t)dataFromMemory + (uint16_t)pSi_flags->FCARRY;
		if (pSi_flags->FCARRY)
		{
			process_AC_withCarry((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		}
		else
		{
			process_AC((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		}
		if (operationResult & 0xFF00)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xCF:
	{
		// RST (CALL $08) opcode
		operationResult = 0x0008;
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xD0:
	{
		cpuTickT();
		if (pSi_flags->FCARRY == 0)
		{
			BYTE lowerData = stackPop();
			cpuTickT();
		cpuTickT();
		cpuTickT();
			BYTE higherData = stackPop();
			cpuTickT();
		cpuTickT();
		cpuTickT();
			operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xD1:
	{
		BYTE lowerData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BYTE higherData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		cpuSetRegister(REGISTER_TYPE::RT_DE, operationResult);
		BREAK;
	}
	case 0xD2:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pSi_flags->FCARRY == 0)
		{
			// last operation resulted in not setting the carry
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xD3:
	{
		// OUT

		if (ROM_TYPE == ROM::TEST_ROM_COM)
		{
			if (pSi_memory->rawMemory[pSi_registers->pc] == 0)
			{
				testFinished = true;
			}
			else if (pSi_memory->rawMemory[pSi_registers->pc] == 1)
			{
				if (cpuReadRegister(REGISTER_TYPE::RT_C) == 2)
				{
					printf("%c", cpuReadRegister(REGISTER_TYPE::RT_E));
				}
				else if (cpuReadRegister(REGISTER_TYPE::RT_C) == 9)
				{
					uint16_t readAddress = cpuReadRegister(REGISTER_TYPE::RT_DE);
					do
					{
						printf("%c", pSi_instance->si_state.memory.rawMemory[readAddress++]);
					}
					while (((char)pSi_instance->si_state.memory.rawMemory[readAddress]) != '$');
				}
			}

			INCREMENT_PC_BY_ONE();

			RETURN status;
		}

		captureIO();

		uint16_t operationResult = cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE port = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		if (port == 2)
		{
			pSi_io->oPort2 = (operationResult & 0x07);
		}
		else if (port == 3)
		{
			if ((pSi_io->oPort3.oPort3Fields.UFO == 0) && operationResult & 0x01)
			{
				SDL_PutAudioStreamData(audioStream, UFO, UFO_length);
			}
			else if ((pSi_io->oPort3.oPort3Fields.SHOT == 0) && operationResult & 0x02)
			{
				SDL_PutAudioStreamData(audioStream, Shot, Shot_length);
			}
			else if ((pSi_io->oPort3.oPort3Fields.PLY_DIE == 0) && operationResult & 0x04)
			{
				SDL_PutAudioStreamData(audioStream, PlayerDies, PlayerDies_length);
			}
			else if ((pSi_io->oPort3.oPort3Fields.INV_DIE == 0) && operationResult & 0x08)
			{
				SDL_PutAudioStreamData(audioStream, InvaderDies, InvaderDies_length);
			}
			else
			{
				;
			}

			pSi_io->oPort3.oPort3Memory = (BYTE)operationResult;
		}
		else if (port == 4)
		{
			pSi_io->shiftRegister = (operationResult << 8 | pSi_io->shiftRegister >> 8);
		}
		else if (port == 5)
		{
			if ((pSi_io->oPort5.oPort5Fields.FLT_M1 == 0) && operationResult & 0x01)
			{
				SDL_PutAudioStreamData(audioStream, FleetMovement1, FleetMovement1_length);
			}
			else if ((pSi_io->oPort5.oPort5Fields.FLT_M2 == 0) && operationResult & 0x02)
			{
				SDL_PutAudioStreamData(audioStream, FleetMovement2, FleetMovement2_length);
			}
			else if ((pSi_io->oPort5.oPort5Fields.FLT_M3 == 0) && operationResult & 0x04)
			{
				SDL_PutAudioStreamData(audioStream, FleetMovement3, FleetMovement3_length);
			}
			else if ((pSi_io->oPort5.oPort5Fields.FLT_M4 == 0) && operationResult & 0x08)
			{
				SDL_PutAudioStreamData(audioStream, FleetMovement4, FleetMovement4_length);
			}
			else if ((pSi_io->oPort5.oPort5Fields.UFO_HIT == 0) && operationResult & 0x10)
			{
				SDL_PutAudioStreamData(audioStream, UFOHit, UFOHit_length);
			}
			else
			{
				;
			}

			pSi_io->oPort5.oPort5Memory = (BYTE)operationResult;
		}
		else if (port == 6)
		{
			;
		}
		else
		{
			unimplementedInstruction();
		}
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0xD4:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		cpuTickT();
		if (pSi_flags->FCARRY == 0)
		{
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			stackPush(lowerData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			// 4) set the pc to 16 bit address
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xD5:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_DE);
		BYTE higherData = (BYTE)(operationResult >> 8);
		BYTE lowerData = (BYTE)operationResult;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0xD6:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_A) - (uint16_t)dataFromMemory;
		if ((operationResult & 0x00FF) >= cpuReadRegister(REGISTER_TYPE::RT_A) && dataFromMemory)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xD7:
	{
		// RST (CALL $10) opcode
		operationResult = 0x0010;
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xD8:
	{
		cpuTickT();
		if (pSi_flags->FCARRY)
		{
			BYTE lowerData = stackPop();
			cpuTickT();
		cpuTickT();
		cpuTickT();
			BYTE higherData = stackPop();
			cpuTickT();
		cpuTickT();
		cpuTickT();
			uint16_t operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xD9:
	{
		BYTE lowerData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BYTE higherData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		uint16_t operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xDA:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pSi_flags->FCARRY)
		{
			// last operation resulted in setting the carry
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xDB:
	{
		// IN

		if (ROM_TYPE == ROM::TEST_ROM_COM)
		{
			INCREMENT_PC_BY_ONE();
			RETURN status;
		}

		captureIO();

		BYTE port = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		if (port == 0)
		{
			BYTE operationResult = pSi_io->iPort0.iPort0Memory;
			cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		}
		else if (port == 1)
		{
			BYTE operationResult = pSi_io->iPort1.iPort1Memory;
			cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		}
		else if (port == 2)
		{
			BYTE operationResult = pSi_io->iPort2.iPort2Memory;
			cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		}
		else if (port == 3)
		{
			if (pSi_io->oPort2 > 7)
			{
				unimplementedInstruction();
			}
			BYTE operationResult = (BYTE)(pSi_io->shiftRegister >> (8 - pSi_io->oPort2));
			cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		}
		else
		{
			unimplementedInstruction();
		}
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0xDC:
	{
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		// 3) check if the condition is met
		cpuTickT();
		cpuTickT();
		cpuTickT();
		if (pSi_flags->FCARRY)
		{
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			stackPush(lowerData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			// 4) set the pc to 16 bit address
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xDD:
	{
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		// 3) set the pc to 16 bit address
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xDE:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		operationResult = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A)
			- (uint16_t)dataFromMemory - (uint16_t)pSi_flags->FCARRY;

		if (pSi_flags->FCARRY)
		{
			process_AB_withBorrow((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		}
		else
		{
			process_AB((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A), dataFromMemory);
		}
		if (
			((operationResult & 0x00FF) >= (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A))
			&&
			(dataFromMemory | pSi_flags->FCARRY)
			)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_SZP((BYTE)operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xDF:
	{
		// RST (CALL $18) opcode
		operationResult = 0x0018;
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xE0:
	{
		cpuTickT();
		if (pSi_flags->FPARITY == 0)
		{
			BYTE lowerData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			BYTE higherData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xE1:
	{
		BYTE lowerData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BYTE higherData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		cpuSetRegister(REGISTER_TYPE::RT_HL, operationResult);
		BREAK;
	}
	case 0xE2:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pSi_flags->FPARITY == 0)
		{
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xE3:
	{
		BYTE olderL = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_L);
		BYTE olderH = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_H);
		cpuTickT();
		BYTE newL = pSi_memory->rawMemory[pSi_registers->sp];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BYTE newH = pSi_memory->rawMemory[pSi_registers->sp + 1];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_L, newL);
		cpuSetRegister(REGISTER_TYPE::RT_H, newH);
		cpuTickT();
		pSi_memory->rawMemory[pSi_registers->sp] = olderL;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_memory->rawMemory[pSi_registers->sp + 1] = olderH;
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0xE4:
	{
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		// 3) check if the condition is met
		cpuTickT();
		if (pSi_flags->FPARITY == 0)
		{
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			stackPush(lowerData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			// 4) set the pc to 16 bit address
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xE5:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL);
		BYTE higherData = (BYTE)(operationResult >> 8);
		BYTE lowerData = (BYTE)operationResult;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0xE6:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		operationResult = ((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) & dataFromMemory);

		// NOTE: Undocumeted behaviour for Half Carry
		// The 8080 logical AND instructions set the flag to reflect the logical OR of bit 3 of the values involved in the AND operation.
		pSi_flags->FAuxCARRY =
			((((cpuReadRegister(REGISTER_TYPE::RT_A)) | (dataFromMemory)) & 0x08) != 0);
		pSi_flags->FCARRY = 0;
		process_SZP(operationResult);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		BREAK;
	}
	case 0xE7:
	{
		// RST (CALL $20) opcode
		operationResult = 0x0020;
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xE8:
	{
		cpuTickT();
		if (pSi_flags->FPARITY)
		{
			BYTE lowerData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			BYTE higherData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xE9:
	{
		pSi_registers->pc = cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuTickT();
		BREAK;
	}
	case 0xEA:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pSi_flags->FPARITY)
		{
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xEB:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuSetRegister(REGISTER_TYPE::RT_HL, cpuReadRegister(REGISTER_TYPE::RT_DE));
		cpuSetRegister(REGISTER_TYPE::RT_DE, operationResult);
		cpuTickT();
		BREAK;
	}
	case 0xEC:
	{
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		// 3) check if the condition is met
		cpuTickT();
		if (pSi_flags->FPARITY)
		{
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			stackPush(lowerData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			// 4) set the pc to 16 bit address
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xED:
	{
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		// 3) set the pc to 16 bit address
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xEE:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		operationResult = ((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) ^ dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP(operationResult);
		BREAK;
	}
	case 0xEF:
	{
		// RST (CALL $28) opcode
		operationResult = 0x0028;
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xF0:
	{
		cpuTickT();
		if (pSi_flags->FSIGN == 0)
		{
			BYTE lowerData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			BYTE higherData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xF1:
	{
		BYTE lowerData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BYTE higherData = stackPop();
		cpuTickT();
		cpuTickT();
		cpuTickT();
		operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
		cpuSetRegister(REGISTER_TYPE::RT_AF, operationResult);
		BREAK;
	}
	case 0xF2:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pSi_flags->FSIGN == 0)
		{
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xF3:
	{
		pSi_instance->si_state.registers.ie = INTERRUPT_DISABLED;
		BREAK;
	}
	case 0xF4:
	{
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		// 3) check if the condition is met
		cpuTickT();
		if (pSi_flags->FSIGN == 0)
		{
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			stackPush(lowerData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			// 4) set the pc to 16 bit address
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xF5:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_AF);
		BYTE higherData = (BYTE)(operationResult >> 8);
		BYTE lowerData = (BYTE)operationResult;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		BREAK;
	}
	case 0xF6:
	{
		dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		operationResult = ((BYTE)cpuReadRegister(REGISTER_TYPE::RT_A) | dataFromMemory);
		cpuSetRegister(REGISTER_TYPE::RT_A, operationResult);
		pSi_flags->FAuxCARRY = 0;
		pSi_flags->FCARRY = 0;
		process_SZP(operationResult);
		BREAK;
	}
	case 0xF7:
	{
		// RST (CALL $30) opcode
		operationResult = 0x0030;
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xF8:
	{
		cpuTickT();
		if (pSi_flags->FSIGN)
		{
			BYTE lowerData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			BYTE higherData = stackPop();
			cpuTickT();
			cpuTickT();
			cpuTickT();
			uint16_t operationResult = ((((uint16_t)higherData) << 8) | ((uint16_t)lowerData));
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xF9:
	{
		operationResult = cpuReadRegister(REGISTER_TYPE::RT_HL);
		cpuTickT();
		cpuSetRegister(REGISTER_TYPE::RT_SP, operationResult);
		BREAK;
	}
	case 0xFA:
	{
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		if (pSi_flags->FSIGN)
		{
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xFB:
	{
		pSi_instance->si_state.registers.ie = INTERRUPT_ENABLED;
		BREAK;
	}
	case 0xFC:
	{
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		// 3) check if the condition is met
		cpuTickT();
		if (pSi_flags->FSIGN)
		{
			BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
			BYTE lowerData = (BYTE)pSi_registers->pc;
			stackPush(higherData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			stackPush(lowerData);
			cpuTickT();
			cpuTickT();
			cpuTickT();
			// 4) set the pc to 16 bit address
			pSi_registers->pc = operationResult;
		}
		BREAK;
	}
	case 0xFD:
	{
		// 1) get the next 16 bit address immediately after the CALL opcode
		lowerDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		higherDataFromMemory = pSi_memory->rawMemory[pSi_registers->pc + ONE];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		INCREMENT_PC_BY_ONE();
		operationResult = ((((uint16_t)higherDataFromMemory) << 8) | ((uint16_t)lowerDataFromMemory));
		// 2) get the PC value pointing to next sequential data after the 16 bit address and push it to stack 
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		// 3) set the pc to 16 bit address
		pSi_registers->pc = operationResult;
		BREAK;
	}
	case 0xFE:
	{
		BYTE registerAContent = (BYTE)cpuReadRegister(REGISTER_TYPE::RT_A);
		BYTE dataFromMemory = pSi_memory->rawMemory[pSi_registers->pc];
		cpuTickT();
		cpuTickT();
		cpuTickT();
		INCREMENT_PC_BY_ONE();
		operationResult = (uint16_t)registerAContent - (uint16_t)dataFromMemory;

		if (((operationResult & 0x00FF) >= registerAContent) && dataFromMemory)
		{
			pSi_flags->FCARRY = 1;
		}
		else
		{
			pSi_flags->FCARRY = 0;
		}
		process_AB(registerAContent, dataFromMemory);
		process_SZP((BYTE)operationResult);
		BREAK;
	}
	case 0xFF:
	{
		// RST (CALL $38) opcode
		operationResult = 0x0038;
		BYTE higherData = (BYTE)(pSi_registers->pc >> 8);
		BYTE lowerData = (BYTE)pSi_registers->pc;
		cpuTickT();
		stackPush(higherData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		stackPush(lowerData);
		cpuTickT();
		cpuTickT();
		cpuTickT();
		pSi_registers->pc = operationResult;
		BREAK;
	}
	default:
	{
		status = false;
		unimplementedInstruction();
		BREAK;
	}
	}

	// Reset the non-modifiable bits to default just in case it got modified

	pSi_flags->ONE1 = ONE;
	pSi_flags->ZERO1 = ZERO;
	pSi_flags->ZERO2 = ZERO;

	RETURN status;
}