#include "nes.h"

void NES_t::runCPUPipeline()
{
	// Setup lamda functions if necessary

	auto INCREMENT_PC_BY_ONE = [&]()
		{
			pNES_cpuRegisters->pc++;
		};

	auto DECREMENT_PC_BY_ONE = [&]()
		{
			pNES_cpuRegisters->pc--;
		};

	// Set/Reset the Unused bits

	processUnusedFlags(ONE);

	// Get the original pc, opcode and cycles (only for logging)
	auto originalPC = pNES_cpuRegisters->pc;
	auto originalOp0 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc];
	auto originalOp1 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc + ONE];
	auto originalOp2 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc + TWO];
	auto originalCycles = pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter;

	if (ROM_TYPE == ROM::NES)
	{
		EXCEPTION_EVENT_TYPE event = EXCEPTION_EVENT_TYPE::EVENT_NONE;
		event = processNMI();
		if (event == EXCEPTION_EVENT_TYPE::EVENT_NMI)
		{
			CPUEVENT("%4X NMI $%04X", originalPC, pNES_cpuRegisters->pc);
			originalPC = pNES_cpuRegisters->pc;
			originalOp0 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc];
			originalOp1 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc + ONE];
			originalOp2 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc + TWO];
			originalCycles = pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter;
		}
		event = processIRQ();
		if (event == EXCEPTION_EVENT_TYPE::EVENT_NMI)
		{
			CPUEVENT("%4X NMI (IRQ)", originalPC);
			originalPC = pNES_cpuRegisters->pc;
			originalOp0 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc];
			originalOp1 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc + ONE];
			originalOp2 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc + TWO];
			originalCycles = pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter;
		}
		else if (event == EXCEPTION_EVENT_TYPE::EVENT_IRQ)
		{
			CPUEVENT("%4X IRQ $%04X", originalPC, pNES_cpuRegisters->pc);
			originalPC = pNES_cpuRegisters->pc;
			originalOp0 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc];
			originalOp1 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc + ONE];
			originalOp2 = pNES_cpuMemory->NESRawMemory[pNES_cpuRegisters->pc + TWO];
			originalCycles = pNES_instance->NES_state.emulatorStatus.ticks.cpuCounter;
		}

		// Decrement the "NMI instruction delay counter" 
		if (pNES_instance->NES_state.interrupts.nmiDelayInInstructions > RESET)
		{
			--pNES_instance->NES_state.interrupts.nmiDelayInInstructions;
		}

		// Decrement the "taken non-page-crossing branch's instruction delay counter"
		if (pNES_instance->NES_state.interrupts.irqDelayInInstructions > RESET)
		{
			--pNES_instance->NES_state.interrupts.irqDelayInInstructions;
		}

		// NOTE : To handle "5-branch_delays_irq.nes", refer implementation of BCC instruction (0x90) below for more details
		// Check whether there was any NMI or IRQ during the execution of last opcode
		pNES_instance->NES_state.interrupts.wasNMI = pNES_instance->NES_state.interrupts.isNMI;
		pNES_instance->NES_state.interrupts.wasIRQ = (pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE);
	}

	// Fetch Instruction
	pNES_cpuInstance->opcode = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
	INCREMENT_PC_BY_ONE();
	cpuTickT(CYCLES_TYPE::READ_CYCLE);

	// Fetch Data and Execute
	auto operationResult = 0;
	auto dataFromMemory = 0;
	auto lowerDataFromMemory = 0;
	auto higherDataFromMemory = 0;

	auto STP = [&]()
		{
			// Implemented based on bus cycles depicted in "Tom Harte" logs
			CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
				, originalPC
				, originalOp0
				, pNES_cpuRegisters->a
				, pNES_cpuRegisters->x
				, pNES_cpuRegisters->y
				, pNES_cpuRegisters->p.p
				, pNES_cpuRegisters->sp
				, originalCycles
			);
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			discard = readCpuRawMemory(IRQ_BRK_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			discard = readCpuRawMemory(IRQ_BRK_VECTOR_START_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			discard = readCpuRawMemory(IRQ_BRK_VECTOR_START_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			discard = readCpuRawMemory(IRQ_BRK_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			discard = readCpuRawMemory(IRQ_BRK_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			discard = readCpuRawMemory(IRQ_BRK_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			discard = readCpuRawMemory(IRQ_BRK_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			discard = readCpuRawMemory(IRQ_BRK_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			discard = readCpuRawMemory(IRQ_BRK_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			DISASSEMBLY("%4X *STP", originalPC);
		};

	switch (pNES_cpuInstance->opcode)
	{
	case 0x00:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);

		FLAG jumpToNMIResetVector = NO;
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// PC increment is suppressed
		// Push PC hi to stack
		stackPush((pNES_cpuRegisters->pc & 0xFF00) >> EIGHT);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Push PC low to stack
		stackPush(pNES_cpuRegisters->pc & 0x00FF);
		// Handle Interrupt hijacking
		// Refer to "Interrupt hijacking" section in https://www.nesdev.org/wiki/CPU_interrupts
		if (pNES_instance->NES_state.interrupts.isNMI == YES)
		{
			pNES_instance->NES_state.interrupts.isNMI = NO;
			jumpToNMIResetVector = YES;
		}
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Push P to stack
		auto p = pNES_cpuRegisters->p;
		p.flagFields.FCAUSE = ONE; // For BRK (only during stack push)
		p.flagFields.FORCED_TO_ONE = ONE;
		stackPush(p.p);
		// Set the interrupt disable flag in P
		// Refer "I: Interrupt Disable" in https://www.nesdev.org/wiki/Status_flags 
		pNES_cpuRegisters->p.flagFields.INTERRUPT_DISABLE = SET;
		pNES_cpuRegisters->p.flagFields.FORCED_TO_ONE = SET;
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (jumpToNMIResetVector == YES)
		{
			// Read PC low from vector
			pNES_cpuRegisters->pc = readCpuRawMemory(NMI_VECTOR_START_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			// Read PC high from vector
			pNES_cpuRegisters->pc |= (readCpuRawMemory(NMI_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU) << EIGHT);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			DISASSEMBLY("%4X NMI (BRK)", originalPC);
		}
		else
		{
			// Read PC low from vector
			pNES_cpuRegisters->pc = readCpuRawMemory(IRQ_BRK_VECTOR_START_ADDRESS, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			// Read PC high from vector
			pNES_cpuRegisters->pc |= (readCpuRawMemory(IRQ_BRK_VECTOR_END_ADDRESS, MEMORY_ACCESS_SOURCE::CPU) << EIGHT);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
			DISASSEMBLY("%4X BRK", originalPC);
		}
		// NOTE : From Mesen :- Apparently, post BRK, NMI cannot run immediatly, it will be delayed by one instruction
		// Refer : https://github.com/SourMesen/Mesen2/blob/master/Core/NES/NesCpu.cpp#L232
		pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE;
		BREAK;
	}
	case 0x01:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X ORA ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x02:
	{
		STP();
		BREAK;
	}
	case 0x03:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SLO ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x04:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X", originalPC, op);
		BREAK;
	}
	case 0x05:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X ORA $%02X", originalPC, address);
		BREAK;
	}
	case 0x06:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op << ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ASL $%02X", originalPC, address);
		BREAK;
	}
	case 0x07:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SLO $%02X", originalPC, address);
		BREAK;
	}
	case 0x08:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Refer : https://www.reddit.com/r/EmuDev/comments/j8osyv/nes_help_me_understand_nestest_pla_instruction/
		// Also refer : https://wiki.nesdev.com/w/index.php/Status_flags#The_B_flag
		// Also refer : https://github.com/OneLoneCoder/olcNES/issues/34
		// So, FCause field will be set only on the data that we push to stack, but the FCause bit in P will still remain zero
		auto pToStack = pNES_cpuRegisters->p;
		pToStack.flagFields.FCAUSE = SET;
		stackPush(pToStack.p);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X PHP", originalPC);
		BREAK;
	}
	case 0x09:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = cpuReadRegister(REGISTER_TYPE::RT_A) | op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X ORA #$%02X", originalPC, op);
		BREAK;
	}
	case 0x0A:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A));
		byte result = (op << ONE);
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X ASL A", originalPC);
		BREAK;
	}
	case 0x0B:
	{
		// Refer https://www.oxyron.de/html/opcodes02.html
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, result);
		DISASSEMBLY("%4X *ANC #$%02X", originalPC, op);
		BREAK;
	}
	case 0x0C:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%04X", originalPC, address);
		BREAK;
	}
	case 0x0D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X ORA $%04X", originalPC, address);
		BREAK;
	}
	case 0x0E:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op << ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ASL $%04X", originalPC, address);
		BREAK;
	}
	case 0x0F:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SLO $%04X", originalPC, address);
		BREAK;
	}
	case 0x10:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		SBYTE op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (pNES_flags->FNEGATIVE == RESET)
		{
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);

			if ((pNES_cpuRegisters->pc & 0xFF00) != ((pNES_cpuRegisters->pc + (SBYTE)op) & 0xFF00))
			{
				auto dummyAddress = ((pNES_cpuRegisters->pc & 0xFF00) | ((pNES_cpuRegisters->pc + (SBYTE)op) & 0x00FF));
				auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
			}
			// NOTE : To handle "5-branch_delays_irq.nes", refer implementation of BCC instruction (0x90) below for more details
			else
			{
				if (pNES_instance->NES_state.interrupts.wasNMI == NO && pNES_instance->NES_state.interrupts.isNMI == YES)
				{
					pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE;
				}
				if (pNES_instance->NES_state.interrupts.wasIRQ == NO && pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE)
				{
					pNES_instance->NES_state.interrupts.irqDelayInInstructions = ONE;
				}
			}
			pNES_cpuRegisters->pc += op;
		}
		DISASSEMBLY("%4X BPL $%02X", originalPC, (originalPC + TWO + op));
		BREAK;
	}
	case 0x11:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X ORA ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0x12:
	{
		STP();
		BREAK;
	}
	case 0x13:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		// Note: Based on Tom Harte test, SLO with indirect zero page will always take 8 cycles
		//if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(yEffAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(yEffAddress, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SLO ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0x14:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory((op + cpuReadRegister(REGISTER_TYPE::RT_X)) & 0xFF, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X,X", originalPC, op);
		BREAK;
	}
	case 0x15:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X ORA $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x16:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op << ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ASL $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x17:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SLO $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x18:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		pNES_flags->FCARRY = RESET;
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X CLC", originalPC);
		BREAK;
	}
	case 0x19:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X ORA $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0x1A:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP", originalPC);
		BREAK;
	}
	case 0x1B:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// Note: Based on Tom Harte test, SLO with indirect zero page will always take 8 cycles
		//if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SLO $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0x1C:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x1D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X ORA $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x1E:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op << ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ASL $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x1F:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) | shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SLO $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x20:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://retrocomputing.stackexchange.com/questions/19543/why-does-the-6502-jsr-instruction-only-increment-the-return-address-by-2-bytes
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// NOTE: PC should point to the last byte of the JSR instruction, not he byte next to the last byte of JSR instruction
		// So we push after 2 increments of PC instead of 3
		auto discard = readCpuRawMemory(pNES_cpuRegisters->sp + 0x100, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE); // "Store ADL" stage; Refer : https://archive.org/details/6500-50a_mcs6500pgmmanjan76/page/n121/mode/2up
		stackPush((pNES_cpuRegisters->pc & 0xFF00) >> EIGHT);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		stackPush(pNES_cpuRegisters->pc & 0x00FF);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		pNES_cpuRegisters->pc = lo | (hi << EIGHT);
		DISASSEMBLY("%4X JSR $%02X", originalPC, pNES_cpuRegisters->pc);
		BREAK;
	}
	case 0x21:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X AND ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x22:
	{
		STP();
		BREAK;
	}
	case 0x23:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp1, ZERO);
		}
		else
		{
			UNSETBIT(shiftedOp1, ZERO);
		}
		pNES_flags->FCARRY = GETBIT(SEVEN, shiftedOp1);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RLA ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x24:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op1 = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = readCpuRawMemory(op1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = cpuReadRegister(REGISTER_TYPE::RT_A) & op2;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op2);
		pNES_flags->FOVERFLOW = GETBIT(SIX, op2);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X BIT $%02X", originalPC, op1);
		BREAK;
	}
	case 0x25:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X AND $%02X", originalPC, address);
		BREAK;
	}
	case 0x26:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, ZERO);
		}
		else
		{
			UNSETBIT(result, ZERO);
		}
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ROL $%02X", originalPC, address);
		BREAK;
	}
	case 0x27:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp1, ZERO);
		}
		else
		{
			UNSETBIT(shiftedOp1, ZERO);
		}
		pNES_flags->FCARRY = GETBIT(SEVEN, shiftedOp1);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RLA $%02X", originalPC, address);
		BREAK;
	}
	case 0x28:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory(pNES_cpuRegisters->sp + 0x100, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = stackPop();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		FLAG statusOfIBeforeSettingFlag = (FLAG)pNES_flags->INTERRUPT_DISABLE;
		cpuSetRegister(REGISTER_TYPE::RT_P, op);
		pNES_flags->FORCED_TO_ONE = ONE;
		pNES_flags->FCAUSE = ZERO;
		if (ROM_TYPE == ROM::NES)
		{
			CPUTODO("Currently, to simulate Interrupt delay we are delaying the \"I\" flag itself; ideally \"I\"  flag should not be delayed, just the interrupt triggering/inhibition needs to be delayed");
			// To handle for I flag latency post a PLP
			// Refer https://forums.nesdev.org/viewtopic.php?p=19655#p19655
			if ((FLAG)pNES_flags->INTERRUPT_DISABLE == !statusOfIBeforeSettingFlag)
			{
				pNES_flags->INTERRUPT_DISABLE = ((pNES_flags->INTERRUPT_DISABLE == SET) ? RESET : SET);
				pNES_instance->NES_state.interrupts.plpDelayInCpuCycles = ONE;
			}
		}
		else
		{
			MASQ_UNUSED(statusOfIBeforeSettingFlag);
		}
		DISASSEMBLY("%4X PLP", originalPC);
		BREAK;
	}
	case 0x29:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = cpuReadRegister(REGISTER_TYPE::RT_A) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X AND #$%02X", originalPC, op);
		BREAK;
	}
	case 0x2A:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A));
		byte result = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, ZERO);
		}
		else
		{
			UNSETBIT(result, ZERO);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X ROL A", originalPC);
		BREAK;
	}
	case 0x2B:
	{
		// Refer https://www.oxyron.de/html/opcodes02.html
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, result);
		DISASSEMBLY("%4X *ANC #$%02X", originalPC, op);
		BREAK;
	}
	case 0x2C:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = cpuReadRegister(REGISTER_TYPE::RT_A) & op;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FOVERFLOW = GETBIT(SIX, op);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X BIT $%04X", originalPC, address);
		BREAK;
	}
	case 0x2D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X AND $%04X", originalPC, address);
		BREAK;
	}
	case 0x2E:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, ZERO);
		}
		else
		{
			UNSETBIT(result, ZERO);
		}
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ROL $%04X", originalPC, address);
		BREAK;
	}
	case 0x2F:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp1, ZERO);
		}
		else
		{
			UNSETBIT(shiftedOp1, ZERO);
		}
		pNES_flags->FCARRY = GETBIT(SEVEN, shiftedOp1);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RLA $%04X", originalPC, address);
		BREAK;
	}
	case 0x30:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		SBYTE op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (pNES_flags->FNEGATIVE == SET)
		{
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);

			if ((pNES_cpuRegisters->pc & 0xFF00) != ((pNES_cpuRegisters->pc + (SBYTE)op) & 0xFF00))
			{
				auto dummyAddress = ((pNES_cpuRegisters->pc & 0xFF00) | ((pNES_cpuRegisters->pc + (SBYTE)op) & 0x00FF));
				auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
			}
			// NOTE : To handle "5-branch_delays_irq.nes", refer implementation of BCC instruction (0x90) below for more details
			else
			{
				if (pNES_instance->NES_state.interrupts.wasNMI == NO && pNES_instance->NES_state.interrupts.isNMI == YES)
				{
					pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE;
				}
				if (pNES_instance->NES_state.interrupts.wasIRQ == NO && pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE)
				{
					pNES_instance->NES_state.interrupts.irqDelayInInstructions = ONE;
				}
			}
			pNES_cpuRegisters->pc += op;
		}
		DISASSEMBLY("%4X BMI $%02X", originalPC, (originalPC + TWO + op));
		BREAK;
	}
	case 0x31:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X AND ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0x32:
	{
		STP();
		BREAK;
	}
	case 0x33:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		// Note: Based on Tom Harte test, RLA with indirect zero page will always take 8 cycles
		//if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(yEffAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp1, ZERO);
		}
		else
		{
			UNSETBIT(shiftedOp1, ZERO);
		}
		pNES_flags->FCARRY = GETBIT(SEVEN, shiftedOp1);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(yEffAddress, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RLA ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0x34:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory((op + cpuReadRegister(REGISTER_TYPE::RT_X)) & 0xFF, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X,X", originalPC, op);
		BREAK;
	}
	case 0x35:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X AND $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x36:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, ZERO);
		}
		else
		{
			UNSETBIT(result, ZERO);
		}
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ROL $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x37:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp1, ZERO);
		}
		else
		{
			UNSETBIT(shiftedOp1, ZERO);
		}
		pNES_flags->FCARRY = GETBIT(SEVEN, shiftedOp1);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RLA $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x38:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		pNES_flags->FCARRY = SET;
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X SEC", originalPC);
		BREAK;
	}
	case 0x39:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X AND $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0x3A:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP", originalPC);
		BREAK;
	}
	case 0x3B:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// Note: Based on Tom Harte test, RLA with indirect zero page will always take 8 cycles
		//if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp1, ZERO);
		}
		else
		{
			UNSETBIT(shiftedOp1, ZERO);
		}
		pNES_flags->FCARRY = GETBIT(SEVEN, shiftedOp1);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RLA $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0x3C:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x3D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X AND $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x3E:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, ZERO);
		}
		else
		{
			UNSETBIT(result, ZERO);
		}
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ROL $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x3F:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op << ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp1, ZERO);
		}
		else
		{
			UNSETBIT(shiftedOp1, ZERO);
		}
		pNES_flags->FCARRY = GETBIT(SEVEN, shiftedOp1);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RLA $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x40:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer : https://www.scribd.com/document/562551470/mcs6500-family-programming-manual
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory(pNES_cpuRegisters->sp + 0x100, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		p_t p = { ZERO };
		p.p = stackPop();
		p.flagFields.FORCED_TO_ONE = ONE;
		p.flagFields.FCAUSE = ZERO;
		cpuSetRegister(REGISTER_TYPE::RT_P, p.p);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto lo = stackPop();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = stackPop();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		pNES_cpuRegisters->pc = lo | (hi << EIGHT);
		DISASSEMBLY("%4X RTI", originalPC);
		BREAK;
	}
	case 0x41:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X EOR ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x42:
	{
		STP();
		BREAK;
	}
	case 0x43:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op >> ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SRE ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x44:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X", originalPC, op);
		BREAK;
	}
	case 0x45:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X EOR $%02X", originalPC, address);
		BREAK;
	}
	case 0x46:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op >> ONE);
		pNES_flags->FNEGATIVE = ZERO;
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X LSR $%02X", originalPC, address);
		BREAK;
	}
	case 0x47:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op >> ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SRE $%02X", originalPC, address);
		BREAK;
	}
	case 0x48:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		stackPush(pNES_cpuRegisters->a);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X PHA", originalPC);
		BREAK;
	}
	case 0x49:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = cpuReadRegister(REGISTER_TYPE::RT_A) ^ op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X EOR #$%02X", originalPC, op);
		BREAK;
	}
	case 0x4A:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A));
		byte result = (op >> ONE);
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = ZERO;
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		DISASSEMBLY("%4X LSR A", originalPC);
		BREAK;
	}
	case 0x4B:
	{
		// Refer https://www.oxyron.de/html/opcodes02.html
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) & op;
		auto result = op1 >> ONE;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = ZERO;
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op1);
		DISASSEMBLY("%4X *ALR #$%02X", originalPC, op);
		BREAK;
	}
	case 0x4C:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		pNES_cpuRegisters->pc = lo | (hi << EIGHT);
		DISASSEMBLY("%4X JMP $%04X", originalPC, pNES_cpuRegisters->pc);
		BREAK;
	}
	case 0x4D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X EOR $%04X", originalPC, address);
		BREAK;
	}
	case 0x4E:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op >> ONE);
		pNES_flags->FNEGATIVE = ZERO;
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X LSR $%04X", originalPC, address);
		BREAK;
	}
	case 0x4F:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op >> ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SRE $%04X", originalPC, address);
		BREAK;
	}
	case 0x50:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		SBYTE op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (pNES_flags->FOVERFLOW == RESET)
		{
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);

			if ((pNES_cpuRegisters->pc & 0xFF00) != ((pNES_cpuRegisters->pc + (SBYTE)op) & 0xFF00))
			{
				auto dummyAddress = ((pNES_cpuRegisters->pc & 0xFF00) | ((pNES_cpuRegisters->pc + (SBYTE)op) & 0x00FF));
				auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
			}
			// NOTE : To handle "5-branch_delays_irq.nes", refer implementation of BCC instruction (0x90) below for more details
			else
			{
				if (pNES_instance->NES_state.interrupts.wasNMI == NO && pNES_instance->NES_state.interrupts.isNMI == YES)
				{
					pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE;
				}
				if (pNES_instance->NES_state.interrupts.wasIRQ == NO && pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE)
				{
					pNES_instance->NES_state.interrupts.irqDelayInInstructions = ONE;
				}
			}
			pNES_cpuRegisters->pc += op;
		}
		DISASSEMBLY("%4X BVC $%02X", originalPC, (originalPC + TWO + op));
		BREAK;
	}
	case 0x51:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X EOR ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0x52:
	{
		STP();
		BREAK;
	}
	case 0x53:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		// Note: Based on Tom Harte test, SRE with indirect zero page will always take 8 cycles
		//if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(yEffAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op >> ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(yEffAddress, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SRE ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0x54:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory((op + cpuReadRegister(REGISTER_TYPE::RT_X)) & 0xFF, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X,X", originalPC, op);
		BREAK;
	}
	case 0x55:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X EOR $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x56:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op >> ONE);
		pNES_flags->FNEGATIVE = ZERO;
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X LSR $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x57:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op >> ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SRE $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x58:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (ROM_TYPE == ROM::NES)
		{
			CPUTODO("Currently, to simulate Interrupt delay we are delaying the \"I\" flag itself; ideally \"I\"  flag should not be delayed, just the interrupt triggering/inhibition needs to be delayed");
			// To handle for I flag latency post a CLI
			// Refer : https://forums.nesdev.org/viewtopic.php?p=19655#p19655
			pNES_instance->NES_state.interrupts.cliDelayInCpuCycles = ONE;
		}
		else
		{
			pNES_flags->INTERRUPT_DISABLE = RESET;
		}
		DISASSEMBLY("%4X CLI", originalPC);
		BREAK;
	}
	case 0x59:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X EOR $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0x5A:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP", originalPC);
		BREAK;
	}
	case 0x5B:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// Note: Based on Tom Harte test, SRE with indirect zero page will always take 8 cycles
		//if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op >> ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SRE $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0x5C:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x5D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X EOR $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x5E:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op >> ONE);
		pNES_flags->FNEGATIVE = ZERO;
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X LSR $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x5F:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp1 = (op >> ONE);
		auto result = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)) ^ shiftedOp1;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, shiftedOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SRE $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x60:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://retrocomputing.stackexchange.com/questions/19543/why-does-the-6502-jsr-instruction-only-increment-the-return-address-by-2-bytes
		// Also refer to https://archive.org/details/6500-50a_mcs6500pgmmanjan76/page/n123/mode/2up
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory(pNES_cpuRegisters->sp + 0x100, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto lo = stackPop();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = stackPop();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		pNES_cpuRegisters->pc = lo | (hi << EIGHT);
		discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X RTS", originalPC);
		BREAK;
	}
	case 0x61:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand

			// Add lower nibbles
			uint8_t low_sum = a_low + op_low + c;
			uint8_t low_carry = 0;
			if (low_sum > 9)
			{
				low_sum += 6;        // BCD adjust
				low_carry = 1;
			}

			// Add upper nibbles  
			uint8_t high_sum = a_high + op_high + low_carry;
			if (high_sum > 9)
			{
				high_sum += 6;       // BCD adjust
				pNES_flags->FCARRY = 1;
			}
			else
			{
				pNES_flags->FCARRY = 0;
			}

			result = ((high_sum & 0x0F) << 4) | (low_sum & 0x0F);

			// V flag: Check if high nibble addition (as 4-bit signed) overflows
			// Treat high nibbles as signed 4-bit numbers (-8 to +7)
			int8_t signed_a_high = (a_high >= 8) ? (a_high - 16) : a_high;
			int8_t signed_op_high = (op_high >= 8) ? (op_high - 16) : op_high;
			int8_t signed_result = signed_a_high + signed_op_high + low_carry;

			// Overflow if result is outside -8 to +7 range
			pNES_flags->FOVERFLOW = (signed_result < -8 || signed_result > 7) ? 1 : 0;
		}
		else
		{
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X ADC ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x62:
	{
		STP();
		BREAK;
	}
	case 0x63:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp, SEVEN);
		}
		else
		{
			UNSETBIT(shiftedOp, SEVEN);
		}
		// NOTE: RRA's ROR first sets the carry and this carry is used next by the RRA's ADC operation; Refer "nestest-bus-cycles-no-disasm-v2.log"
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)shiftedOp + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ shiftedOp)) & (a ^ (byte)result)) >> SEVEN);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), shiftedOp, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RRA ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x64:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X", originalPC, op);
		BREAK;
	}
	case 0x65:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand

			// Add lower nibbles
			uint8_t low_sum = a_low + op_low + c;
			uint8_t low_carry = 0;
			if (low_sum > 9)
			{
				low_sum += 6;        // BCD adjust
				low_carry = 1;
			}

			// Add upper nibbles  
			uint8_t high_sum = a_high + op_high + low_carry;
			if (high_sum > 9)
			{
				high_sum += 6;       // BCD adjust
				pNES_flags->FCARRY = 1;
			}
			else
			{
				pNES_flags->FCARRY = 0;
			}

			result = ((high_sum & 0x0F) << 4) | (low_sum & 0x0F);

			// V flag: Check if high nibble addition (as 4-bit signed) overflows
			// Treat high nibbles as signed 4-bit numbers (-8 to +7)
			int8_t signed_a_high = (a_high >= 8) ? (a_high - 16) : a_high;
			int8_t signed_op_high = (op_high >= 8) ? (op_high - 16) : op_high;
			int8_t signed_result = signed_a_high + signed_op_high + low_carry;

			// Overflow if result is outside -8 to +7 range
			pNES_flags->FOVERFLOW = (signed_result < -8 || signed_result > 7) ? 1 : 0;
		}
		else
		{
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X ADC $%02X", originalPC, address);
		BREAK;
	}
	case 0x66:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, SEVEN);
		}
		else
		{
			UNSETBIT(result, SEVEN);
		}
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ROR $%02X", originalPC, address);
		BREAK;
	}
	case 0x67:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp, SEVEN);
		}
		else
		{
			UNSETBIT(shiftedOp, SEVEN);
		}
		// NOTE: RRA's ROR first sets the carry and this carry is used next by the RRA's ADC operation; Refer "nestest-bus-cycles-no-disasm-v2.log"
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)shiftedOp + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ shiftedOp)) & (a ^ (byte)result)) >> SEVEN);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, shiftedOp, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RRA $%02X", originalPC, address);
		BREAK;
	}
	case 0x68:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory(pNES_cpuRegisters->sp + 0x100, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = stackPop();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X PLA", originalPC);
		BREAK;
	}
	case 0x69:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand

			// Add lower nibbles
			uint8_t low_sum = a_low + op_low + c;
			uint8_t low_carry = 0;
			if (low_sum > 9)
			{
				low_sum += 6;        // BCD adjust
				low_carry = 1;
			}

			// Add upper nibbles  
			uint8_t high_sum = a_high + op_high + low_carry;
			if (high_sum > 9)
			{
				high_sum += 6;       // BCD adjust
				pNES_flags->FCARRY = 1;
			}
			else
			{
				pNES_flags->FCARRY = 0;
			}

			result = ((high_sum & 0x0F) << 4) | (low_sum & 0x0F);

			// V flag: Check if high nibble addition (as 4-bit signed) overflows
			// Treat high nibbles as signed 4-bit numbers (-8 to +7)
			int8_t signed_a_high = (a_high >= 8) ? (a_high - 16) : a_high;
			int8_t signed_op_high = (op_high >= 8) ? (op_high - 16) : op_high;
			int8_t signed_result = signed_a_high + signed_op_high + low_carry;

			// Overflow if result is outside -8 to +7 range
			pNES_flags->FOVERFLOW = (signed_result < -8 || signed_result > 7) ? 1 : 0;
		}
		else
		{
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X ADC #$%02X", originalPC, op);
		BREAK;
	}
	case 0x6A:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A));
		byte result = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, SEVEN);
		}
		else
		{
			UNSETBIT(result, SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X ROR A", originalPC);
		BREAK;
	}
	case 0x6B:
	{
		// Refer https://www.oxyron.de/html/opcodes02.html
		// Note that this instruction is implemented based on https://discord.com/channels/465585922579103744/465586161067229195/1045785570812641390
		CPUTODO("Find the source for *ARR instruction other than the discord source provided above");
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		byte a = TO_UINT8(cpuReadRegister(REGISTER_TYPE::RT_A));
		auto op1 = a & op;
		auto result = op1 >> ONE;
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, SEVEN);
		}
		else
		{
			UNSETBIT(result, SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(SEVEN, op1);
		pNES_flags->FOVERFLOW = (GETBIT(SEVEN, op1) ^ GETBIT(SIX, op1));
		DISASSEMBLY("%4X *ARR #$%02X", originalPC, op);
		BREAK;
	}
	case 0x6C:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto ilo = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t nextAddress = address + ONE;
		// For quirks, refer https://www.reddit.com/r/EmuDev/comments/91qchl/6502nes_simple_question_about_the_jump/
		if ((address & 0xFF00) != (nextAddress & 0xFF00))
		{
			nextAddress = (address & 0xFF00);
		}
		auto ihi = readCpuRawMemory(nextAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		pNES_cpuRegisters->pc = (ilo | (ihi << EIGHT));
		DISASSEMBLY("%4X JMP ($%04X)", originalPC, pNES_cpuRegisters->pc);
		BREAK;
	}
	case 0x6D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand

			// Add lower nibbles
			uint8_t low_sum = a_low + op_low + c;
			uint8_t low_carry = 0;
			if (low_sum > 9)
			{
				low_sum += 6;        // BCD adjust
				low_carry = 1;
			}

			// Add upper nibbles  
			uint8_t high_sum = a_high + op_high + low_carry;
			if (high_sum > 9)
			{
				high_sum += 6;       // BCD adjust
				pNES_flags->FCARRY = 1;
			}
			else
			{
				pNES_flags->FCARRY = 0;
			}

			result = ((high_sum & 0x0F) << 4) | (low_sum & 0x0F);

			// V flag: Check if high nibble addition (as 4-bit signed) overflows
			// Treat high nibbles as signed 4-bit numbers (-8 to +7)
			int8_t signed_a_high = (a_high >= 8) ? (a_high - 16) : a_high;
			int8_t signed_op_high = (op_high >= 8) ? (op_high - 16) : op_high;
			int8_t signed_result = signed_a_high + signed_op_high + low_carry;

			// Overflow if result is outside -8 to +7 range
			pNES_flags->FOVERFLOW = (signed_result < -8 || signed_result > 7) ? 1 : 0;
		}
		else
		{
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X ADC $%04X", originalPC, address);
		BREAK;
	}
	case 0x6E:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, SEVEN);
		}
		else
		{
			UNSETBIT(result, SEVEN);
		}
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ROR $%04X", originalPC, address);
		BREAK;
	}
	case 0x6F:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp, SEVEN);
		}
		else
		{
			UNSETBIT(shiftedOp, SEVEN);
		}
		// NOTE: RRA's ROR first sets the carry and this carry is used next by the RRA's ADC operation; Refer "nestest-bus-cycles-no-disasm-v2.log"
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + shiftedOp + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ shiftedOp)) & (a ^ (byte)result)) >> SEVEN);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, shiftedOp, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RRA $%04X", originalPC, address);
		BREAK;
	}
	case 0x70:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		SBYTE op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (pNES_flags->FOVERFLOW == SET)
		{
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);

			if ((pNES_cpuRegisters->pc & 0xFF00) != ((pNES_cpuRegisters->pc + (SBYTE)op) & 0xFF00))
			{
				auto dummyAddress = ((pNES_cpuRegisters->pc & 0xFF00) | ((pNES_cpuRegisters->pc + (SBYTE)op) & 0x00FF));
				auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
			}
			// NOTE : To handle "5-branch_delays_irq.nes", refer implementation of BCC instruction (0x90) below for more details
			else
			{
				if (pNES_instance->NES_state.interrupts.wasNMI == NO && pNES_instance->NES_state.interrupts.isNMI == YES)
				{
					pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE;
				}
				if (pNES_instance->NES_state.interrupts.wasIRQ == NO && pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE)
				{
					pNES_instance->NES_state.interrupts.irqDelayInInstructions = ONE;
				}
			}
			pNES_cpuRegisters->pc += op;
		}
		DISASSEMBLY("%4X BVS $%02X", originalPC, (originalPC + TWO + op));
		BREAK;
	}
	case 0x71:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand

			// Add lower nibbles
			uint8_t low_sum = a_low + op_low + c;
			uint8_t low_carry = 0;
			if (low_sum > 9)
			{
				low_sum += 6;        // BCD adjust
				low_carry = 1;
			}

			// Add upper nibbles  
			uint8_t high_sum = a_high + op_high + low_carry;
			if (high_sum > 9)
			{
				high_sum += 6;       // BCD adjust
				pNES_flags->FCARRY = 1;
			}
			else
			{
				pNES_flags->FCARRY = 0;
			}

			result = ((high_sum & 0x0F) << 4) | (low_sum & 0x0F);

			// V flag: Check if high nibble addition (as 4-bit signed) overflows
			// Treat high nibbles as signed 4-bit numbers (-8 to +7)
			int8_t signed_a_high = (a_high >= 8) ? (a_high - 16) : a_high;
			int8_t signed_op_high = (op_high >= 8) ? (op_high - 16) : op_high;
			int8_t signed_result = signed_a_high + signed_op_high + low_carry;

			// Overflow if result is outside -8 to +7 range
			pNES_flags->FOVERFLOW = (signed_result < -8 || signed_result > 7) ? 1 : 0;
		}
		else
		{
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X ADC ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0x72:
	{
		STP();
		BREAK;
	}
	case 0x73:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		// Note: Based on Tom Harte test, RRA with indirect zero page will always take 8 cycles
		//if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(yEffAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp, SEVEN);
		}
		else
		{
			UNSETBIT(shiftedOp, SEVEN);
		}
		// NOTE: RRA's ROR first sets the carry and this carry is used next by the RRA's ADC operation; Refer "nestest-bus-cycles-no-disasm-v2.log"
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)shiftedOp + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ shiftedOp)) & (a ^ (byte)result)) >> SEVEN);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(yEffAddress, shiftedOp, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RRA ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0x74:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory((op + cpuReadRegister(REGISTER_TYPE::RT_X)) & 0xFF, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X,X", originalPC, op);
		BREAK;
	}
	case 0x75:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand

			// Add lower nibbles
			uint8_t low_sum = a_low + op_low + c;
			uint8_t low_carry = 0;
			if (low_sum > 9)
			{
				low_sum += 6;        // BCD adjust
				low_carry = 1;
			}

			// Add upper nibbles  
			uint8_t high_sum = a_high + op_high + low_carry;
			if (high_sum > 9)
			{
				high_sum += 6;       // BCD adjust
				pNES_flags->FCARRY = 1;
			}
			else
			{
				pNES_flags->FCARRY = 0;
			}

			result = ((high_sum & 0x0F) << 4) | (low_sum & 0x0F);

			// V flag: Check if high nibble addition (as 4-bit signed) overflows
			// Treat high nibbles as signed 4-bit numbers (-8 to +7)
			int8_t signed_a_high = (a_high >= 8) ? (a_high - 16) : a_high;
			int8_t signed_op_high = (op_high >= 8) ? (op_high - 16) : op_high;
			int8_t signed_result = signed_a_high + signed_op_high + low_carry;

			// Overflow if result is outside -8 to +7 range
			pNES_flags->FOVERFLOW = (signed_result < -8 || signed_result > 7) ? 1 : 0;
		}
		else
		{
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X ADC $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x76:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, SEVEN);
		}
		else
		{
			UNSETBIT(result, SEVEN);
		}
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ROR $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x77:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp, SEVEN);
		}
		else
		{
			UNSETBIT(shiftedOp, SEVEN);
		}
		// NOTE: RRA's ROR first sets the carry and this carry is used next by the RRA's ADC operation; Refer "nestest-bus-cycles-no-disasm-v2.log"
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)shiftedOp + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ shiftedOp)) & (a ^ (byte)result)) >> SEVEN);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), shiftedOp, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RRA $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x78:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (ROM_TYPE == ROM::NES)
		{
			CPUTODO("Currently, to simulate Interrupt delay we are delaying the \"I\" flag itself; ideally \"I\"  flag should not be delayed, just the interrupt triggering/inhibition needs to be delayed");
			// To handle for I flag latency post a SEI
			// Refer : https://forums.nesdev.org/viewtopic.php?p=19655#p19655
			pNES_instance->NES_state.interrupts.seiDelayInCpuCycles = ONE;
		}
		else
		{
			pNES_flags->INTERRUPT_DISABLE = SET;
		}
		DISASSEMBLY("%4X SEI", originalPC);
		BREAK;
	}
	case 0x79:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand

			// Add lower nibbles
			uint8_t low_sum = a_low + op_low + c;
			uint8_t low_carry = 0;
			if (low_sum > 9)
			{
				low_sum += 6;        // BCD adjust
				low_carry = 1;
			}

			// Add upper nibbles  
			uint8_t high_sum = a_high + op_high + low_carry;
			if (high_sum > 9)
			{
				high_sum += 6;       // BCD adjust
				pNES_flags->FCARRY = 1;
			}
			else
			{
				pNES_flags->FCARRY = 0;
			}

			result = ((high_sum & 0x0F) << 4) | (low_sum & 0x0F);

			// V flag: Check if high nibble addition (as 4-bit signed) overflows
			// Treat high nibbles as signed 4-bit numbers (-8 to +7)
			int8_t signed_a_high = (a_high >= 8) ? (a_high - 16) : a_high;
			int8_t signed_op_high = (op_high >= 8) ? (op_high - 16) : op_high;
			int8_t signed_result = signed_a_high + signed_op_high + low_carry;

			// Overflow if result is outside -8 to +7 range
			pNES_flags->FOVERFLOW = (signed_result < -8 || signed_result > 7) ? 1 : 0;
		}
		else
		{
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X ADC $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0x7A:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP", originalPC);
		BREAK;
	}
	case 0x7B:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// Note: Based on Tom Harte test, RRA with indirect zero page will always take 8 cycles
		//if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp, SEVEN);
		}
		else
		{
			UNSETBIT(shiftedOp, SEVEN);
		}
		// NOTE: RRA's ROR first sets the carry and this carry is used next by the RRA's ADC operation; Refer "nestest-bus-cycles-no-disasm-v2.log"
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)shiftedOp + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ shiftedOp)) & (a ^ (byte)result)) >> SEVEN);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, shiftedOp, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RRA $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0x7C:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x7D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand

			// Add lower nibbles
			uint8_t low_sum = a_low + op_low + c;
			uint8_t low_carry = 0;
			if (low_sum > 9)
			{
				low_sum += 6;        // BCD adjust
				low_carry = 1;
			}

			// Add upper nibbles  
			uint8_t high_sum = a_high + op_high + low_carry;
			if (high_sum > 9)
			{
				high_sum += 6;       // BCD adjust
				pNES_flags->FCARRY = 1;
			}
			else
			{
				pNES_flags->FCARRY = 0;
			}

			result = ((high_sum & 0x0F) << 4) | (low_sum & 0x0F);

			// V flag: Check if high nibble addition (as 4-bit signed) overflows
			// Treat high nibbles as signed 4-bit numbers (-8 to +7)
			int8_t signed_a_high = (a_high >= 8) ? (a_high - 16) : a_high;
			int8_t signed_op_high = (op_high >= 8) ? (op_high - 16) : op_high;
			int8_t signed_result = signed_a_high + signed_op_high + low_carry;

			// Overflow if result is outside -8 to +7 range
			pNES_flags->FOVERFLOW = (signed_result < -8 || signed_result > 7) ? 1 : 0;
		}
		else
		{
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X ADC $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x7E:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte result = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(result, SEVEN);
		}
		else
		{
			UNSETBIT(result, SEVEN);
		}
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X ROR $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x7F:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte shiftedOp = (op >> ONE);
		if (pNES_flags->FCARRY == SET)
		{
			SETBIT(shiftedOp, SEVEN);
		}
		else
		{
			UNSETBIT(shiftedOp, SEVEN);
		}
		// NOTE: RRA's ROR first sets the carry and this carry is used next by the RRA's ADC operation; Refer "nestest-bus-cycles-no-disasm-v2.log"
		pNES_flags->FCARRY = GETBIT(ZERO, op);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)shiftedOp + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ shiftedOp)) & (a ^ (byte)result)) >> SEVEN);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, shiftedOp, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *RRA $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x80:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X", originalPC, op);
		BREAK;
	}
	case 0x81:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STA ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x82:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X", originalPC, op);
		BREAK;
	}
	case 0x83:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Refer https://stardot.org.uk/forums/viewtopic.php?p=30514&sid=db02bbda91e1e6ecddcd501f1207fb4a#p30514
		writeCpuRawMemory((lo + (hi << EIGHT)), (cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(REGISTER_TYPE::RT_X)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SAX ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0x84:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(op, static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_Y)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STY $%02X", originalPC, op);
		BREAK;
	}
	case 0x85:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(op, static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STA $%02X", originalPC, op);
		BREAK;
	}
	case 0x86:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(op, static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STX $%02X", originalPC, op);
		BREAK;
	}
	case 0x87:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Refer https://stardot.org.uk/forums/viewtopic.php?p=30514&sid=db02bbda91e1e6ecddcd501f1207fb4a#p30514
		writeCpuRawMemory(op, (cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(REGISTER_TYPE::RT_X)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SAX $%02X", originalPC, op);
		BREAK;
	}
	case 0x88:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = (uint16_t)(static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_Y))) - ONE;
		cpuSetRegister(REGISTER_TYPE::RT_Y, (op & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = ((op & 0xFF) == ZERO);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X DEY", originalPC);
		BREAK;
	}
	case 0x89:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X", originalPC, op);
		BREAK;
	}
	case 0x8A:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X));
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X TXA", originalPC);
		BREAK;
	}
	case 0x8B:
	{
		// Refer to https://www.nesdev.org/wiki/Visual6502wiki/6502_Opcode_8B_(XAA,_ANE)
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto magic = 0xEE; // Note: As per https://www.nesdev.org/wiki/Visual6502wiki/6502_Opcode_8B_(XAA,_ANE), this can be 0xFF or 0xEE; 0xEE helps pass Tom Harte Tests
		auto result = ((static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A))) | magic) & (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X))) & op;
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X *XAA #$%02X", originalPC, op);
		BREAK;
	}
	case 0x8C:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		writeCpuRawMemory(address, static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_Y)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STY $%04X", originalPC, address);
		BREAK;
	}
	case 0x8D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		writeCpuRawMemory(address, static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STA $%04X", originalPC, address);
		BREAK;
	}
	case 0x8E:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		writeCpuRawMemory(address, static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STX $%04X", originalPC, address);
		BREAK;
	}
	case 0x8F:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		// Refer https://stardot.org.uk/forums/viewtopic.php?p=30514&sid=db02bbda91e1e6ecddcd501f1207fb4a#p30514
		writeCpuRawMemory(address, (cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(REGISTER_TYPE::RT_X)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SAX $%04X", originalPC, address);
		BREAK;
	}
	case 0x90:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		SBYTE op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (pNES_flags->FCARRY == RESET)
		{
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);

			if ((pNES_cpuRegisters->pc & 0xFF00) != ((pNES_cpuRegisters->pc + (SBYTE)op) & 0xFF00))
			{
				auto dummyAddress = ((pNES_cpuRegisters->pc & 0xFF00) | ((pNES_cpuRegisters->pc + (SBYTE)op) & 0x00FF));
				auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
			}
			// NOTE : To handle "5-branch_delays_irq.nes"
			// Below "else if" to handle "Taken branch delays interrupts"
			// Refer : https://forums.nesdev.org/viewtopic.php?t=6510
			// The below log is mentioned in above link
			//
			//  test_branch_taken
			//  T+ CK PC
			//  00 02 04 CLC
			//  01 01 04
			//  02 03 07 BCC
			//  03 02 07
			//  04 05 0A LDA $100 *** This is the special case
			//  05 04 0A
			//  06 03 0A
			//  07 02 0A
			//  08 01 0A
			//  09 03 0A JMP
			//
			// NOTE THAT IN C6502, IRQ JUMP DECISION IS TAKEN IN LAST CPU CYCLE OF CURRENTLY EXECUTING INSTRUCTION
			// 
			// How to read this log is as follows:
			// T+ columns is just the time ticking
			// PC can be assumed as the current instruction. The value itself can be ignored as it is relative
			// So, we can assume 04 -> CLC instruction, 07 -> BCC instruction and 0A -> LDA instruction
			// CK is the time difference in clocks from time when IRQ was triggered to time at which IRQ actually jumps to handler
			// 
			// As as we mentioned before IRQ jump desision is taken in last cpu cycle of an instruction
			// But apparently, as per the new research done by Blargg, this is not the case for "Taken branch"
			// i.e. this IRQ jump is skipped in the last cycle for "Taken branch" instructions!
			// 
			// Note that if IRQ was already generated in previous instruction to that of "Taken branch", then after "Taken branch" is executed
			// IRQ jump happens normally
			// 
			// But assume a case where IRQ was not generated in previous instruction to that of "Taken branch" and it was generated in
			// one of the cycles of the "Taken branch" instruction, then in the last cycle of "Taken branch", we delay the jump further by 1 instruction
			// 
			// This is what I understood after I experimented by moving "previousCycleIRQState", 1 cpu cycle at a time... away from the last cpu cycle of previousCycleIRQState
			else
			{
				if (pNES_instance->NES_state.interrupts.wasNMI == NO && pNES_instance->NES_state.interrupts.isNMI == YES)
				{
					pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE;
				}
				if (pNES_instance->NES_state.interrupts.wasIRQ == NO && pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE)
				{
					pNES_instance->NES_state.interrupts.irqDelayInInstructions = ONE;
				}
			}
			pNES_cpuRegisters->pc += op;
		}
		DISASSEMBLY("%4X BCC $%02X", originalPC, (originalPC + TWO + op));
		BREAK;
	}
	case 0x91:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		auto discard = readCpuRawMemory(((effAddress & 0xFF00) | (yEffAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(yEffAddress, static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STA ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0x92:
	{
		STP();
		BREAK;
	}
	case 0x93:
	{
		// Implemented based on bus cycles as indicated in Tom Harte logs and https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
		// Refer to "65xx Processor Data.txt"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((address & 0xFF), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t hi = readCpuRawMemory(((address + ONE) & 0xFF), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		auto discard = readCpuRawMemory(((effAddress & 0xFF00) | (yEffAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
		CPUTODO("Need to find reliable source for using (hi + 1) instead of (hi) @ %d in file %s", __LINE__, __FILE__);
		byte result = cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(REGISTER_TYPE::RT_X) & (hi + ONE);
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
			CPUTODO("Need to find reliable source for the code snippet to get \"newAddress\" @ %d in file %s", __LINE__, __FILE__);
			auto newAddress = ((result << EIGHT) | (yEffAddress & 0x00FF));
			writeCpuRawMemory(newAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		else
		{
			writeCpuRawMemory(yEffAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *AHX $%02X", originalPC, address);
		BREAK;
	}
	case 0x94:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// op is always read from zero page, hence casting
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_Y)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STY $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x95:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// op is always read from zero page, hence casting
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STA $%02X,X", originalPC, address);
		BREAK;
	}
	case 0x96:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// op is always read from zero page, hence casting
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STX $%02X,Y", originalPC, address);
		BREAK;
	}
	case 0x97:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// op is always read from zero page, hence casting
		// Refer https://stardot.org.uk/forums/viewtopic.php?p=30514&sid=db02bbda91e1e6ecddcd501f1207fb4a#p30514
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), (cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(REGISTER_TYPE::RT_X)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SAX $%02X,Y", originalPC, address);
		BREAK;
	}
	case 0x98:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_Y));
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X TYA", originalPC);
		BREAK;
	}
	case 0x99:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STA $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0x9A:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X));
		cpuSetRegister(REGISTER_TYPE::RT_SP, op);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X TXS", originalPC);
		BREAK;
	}
	case 0x9B:
	{
		// Implemented based on bus cycles as indicated in Tom Harte logs
		// Also refer to "65xx Processor Data.txt"
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		uint16_t lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		auto discard = readCpuRawMemory(((effAddress & 0xFF00) | (yEffAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
		CPUTODO("Need to find reliable source for using (hi + 1) instead of (hi) @ %d in file %s", __LINE__, __FILE__);
		byte result = cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(REGISTER_TYPE::RT_X) & (hi + ONE);
		cpuSetRegister(REGISTER_TYPE::RT_SP, (cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(REGISTER_TYPE::RT_X)));
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
			CPUTODO("Need to find reliable source for the code snippet to get \"newAddress\" @ %d in file %s", __LINE__, __FILE__);
			auto newAddress = ((result << EIGHT) | (yEffAddress & 0x00FF));
			writeCpuRawMemory(newAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		else
		{
			writeCpuRawMemory(yEffAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *TAS", originalPC);
		BREAK;
	}
	case 0x9C:
	{
		// Implemented based on bus cycles as indicated in Tom Harte logs
		// Also refer to "65xx Processor Data.txt"
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		uint16_t lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t xEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_X) + effAddress;
		auto discard = readCpuRawMemory(((effAddress & 0xFF00) | (xEffAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
		CPUTODO("Need to find reliable source for using (hi + 1) instead of (hi) @ %d in file %s", __LINE__, __FILE__);
		byte result = cpuReadRegister(REGISTER_TYPE::RT_Y) & (hi + ONE);
		if ((xEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
			CPUTODO("Need to find reliable source for the code snippet to get \"newAddress\" @ %d in file %s", __LINE__, __FILE__);
			auto newAddress = ((result << EIGHT) | (xEffAddress & 0x00FF));
			writeCpuRawMemory(newAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		else
		{
			writeCpuRawMemory(xEffAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SHY", originalPC);
		BREAK;
	}
	case 0x9D:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));;
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory((addressIndexed & 0x00FF) | (address & 0xFF00), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// op is always read from zero page, hence casting
		writeCpuRawMemory(addressIndexed, static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X STA $%04X,X", originalPC, address);
		BREAK;
	}
	case 0x9E:
	{
		// Implemented based on bus cycles as indicated in Tom Harte logs
		// Also refer to "65xx Processor Data.txt"
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		uint16_t lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		auto discard = readCpuRawMemory(((effAddress & 0xFF00) | (yEffAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
		CPUTODO("Need to find reliable source for using (hi + 1) instead of (hi) @ %d in file %s", __LINE__, __FILE__);
		byte result = cpuReadRegister(REGISTER_TYPE::RT_X) & (hi + ONE);
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
			CPUTODO("Need to find reliable source for the code snippet to get \"newAddress\" @ %d in file %s", __LINE__, __FILE__);
			auto newAddress = ((result << EIGHT) | (yEffAddress & 0x00FF));
			writeCpuRawMemory(newAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		else
		{
			writeCpuRawMemory(yEffAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *SHX", originalPC);
		BREAK;
	}
	case 0x9F:
	{
		// Implemented based on bus cycles as indicated in Tom Harte logs
		// Also refer to "65xx Processor Data.txt"
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		uint16_t lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		auto discard = readCpuRawMemory(((effAddress & 0xFF00) | (yEffAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
		CPUTODO("Need to find reliable source for using (hi + 1) instead of (hi) @ %d in file %s", __LINE__, __FILE__);
		byte result = cpuReadRegister(REGISTER_TYPE::RT_A) & cpuReadRegister(REGISTER_TYPE::RT_X) & (hi + ONE);
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1467
			CPUTODO("Need to find reliable source for the code snippet to get \"newAddress\" @ %d in file %s", __LINE__, __FILE__);
			auto newAddress = ((result << EIGHT) | (yEffAddress & 0x00FF));
			writeCpuRawMemory(newAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		else
		{
			writeCpuRawMemory(yEffAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		}
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *AHX", originalPC);
		BREAK;
	}
	case 0xA0:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_Y, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDY #$%02X", originalPC, op);
		BREAK;
	}
	case 0xA1:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDA ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0xA2:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDX #$%02X", originalPC, op);
		BREAK;
	}
	case 0xA3:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = op + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (lo | (hi << EIGHT));
		auto result = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		cpuSetRegister(REGISTER_TYPE::RT_X, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X *LAX ($%02X,X)", originalPC, op);
		BREAK;
	}
	case 0xA4:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_Y, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDY $%02X", originalPC, address);
		BREAK;
	}
	case 0xA5:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDA $%02X", originalPC, address);
		BREAK;
	}
	case 0xA6:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDX $%02X", originalPC, address);
		BREAK;
	}
	case 0xA7:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto result = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		cpuSetRegister(REGISTER_TYPE::RT_X, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X *LAX $%02X", originalPC, op);
		BREAK;
	}
	case 0xA8:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A));
		cpuSetRegister(REGISTER_TYPE::RT_Y, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X TAY", originalPC);
		BREAK;
	}
	case 0xA9:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDA #$%02X", originalPC, op);
		BREAK;
	}
	case 0xAA:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A));
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X TAX", originalPC);
		BREAK;
	}
	case 0xAB:
	{
		// Implemented based on bus cycles as indicated in Tom Harte logs
		// Also refer to "65xx Processor Data.txt"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// Refer https://sourceforge.net/p/vice-emu/code/27740/tree/trunk/vice/src/6510core.c#l1489
		CPUTODO("Need to find reliable source for using below code snippet @ %d in file %s", __LINE__, __FILE__);
		
		// Refer to https://github.com/TASEmulators/fceux/blob/master/src/ops.inc
		// Only opcode which needs conditional implementation for SST and NES CPU tests
		// According to Blargg testing, this needs to be 0xFF for NES 6502
		auto magic = 0xFF;
		if (ROM_TYPE == ROM::TEST_SST)
		{
			magic = 0xEE; // Refer to implementation of 0x8B
		}
		auto result = ((cpuReadRegister(REGISTER_TYPE::RT_A) | magic) & op);
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		cpuSetRegister(REGISTER_TYPE::RT_X, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X *LAX #$%02X", originalPC, op);
		BREAK;
	}
	case 0xAC:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_Y, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDY $%04X", originalPC, address);
		BREAK;
	}
	case 0xAD:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDA $%04X", originalPC, address);
		BREAK;
	}
	case 0xAE:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDX $%04X", originalPC, address);
		BREAK;
	}
	case 0xAF:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto result = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		cpuSetRegister(REGISTER_TYPE::RT_X, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X *LAX $%04X", originalPC, address);
		BREAK;
	}
	case 0xB0:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		SBYTE op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (pNES_flags->FCARRY == SET)
		{
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);

			if ((pNES_cpuRegisters->pc & 0xFF00) != ((pNES_cpuRegisters->pc + (SBYTE)op) & 0xFF00))
			{
				auto dummyAddress = ((pNES_cpuRegisters->pc & 0xFF00) | ((pNES_cpuRegisters->pc + (SBYTE)op) & 0x00FF));
				auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
			}
			// NOTE : To handle "5-branch_delays_irq.nes", refer implementation of BCC instruction (0x90) below for more details
			else
			{
				if (pNES_instance->NES_state.interrupts.wasNMI == NO && pNES_instance->NES_state.interrupts.isNMI == YES)
				{
					pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE;
				}
				if (pNES_instance->NES_state.interrupts.wasIRQ == NO && pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE)
				{
					pNES_instance->NES_state.interrupts.irqDelayInInstructions = ONE;
				}
			}
			pNES_cpuRegisters->pc += op;
		}
		DISASSEMBLY("%4X BCS $%02X", originalPC, (originalPC + TWO + op));
		BREAK;
	}
	case 0xB1:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDA ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0xB2:
	{
		STP();
		BREAK;
	}
	case 0xB3:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		// Note: Based on SST, LAX with indirect zero page will always take 6 cycles
		if ((ROM_TYPE == ROM::TEST_SST) || (yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X *LAX ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0xB4:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// op is always read from zero page, hence casting
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_Y, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDY $%02X,X", originalPC, address);
		BREAK;
	}
	case 0xB5:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// op is always read from zero page, hence casting
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDA $%02X,X", originalPC, address);
		BREAK;
	}
	case 0xB6:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// op is always read from zero page, hence casting
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDX $%02X,Y", originalPC, address);
		BREAK;
	}
	case 0xB7:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// op is always read from zero page, hence casting
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X *LAX $%02X,Y", originalPC, address);
		BREAK;
	}
	case 0xB8:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		pNES_flags->FOVERFLOW = RESET;
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X CLV", originalPC);
		BREAK;
	}
	case 0xB9:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDA $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0xBA:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = (byte)cpuReadRegister(REGISTER_TYPE::RT_SP);
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X TSX", originalPC);
		BREAK;
	}
	case 0xBB:
	{
		// Implemented based on bus cycles as indicated in Tom Harte logs
		// Also refer to "65xx Processor Data.txt"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		uint16_t lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		auto op = readCpuRawMemory(((effAddress & 0xFF00) | (yEffAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		}
		auto result = cpuReadRegister(REGISTER_TYPE::RT_SP) & op;
		cpuSetRegister(REGISTER_TYPE::RT_SP, result);
		cpuSetRegister(REGISTER_TYPE::RT_A, result);
		cpuSetRegister(REGISTER_TYPE::RT_X, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (result == ZERO);
		DISASSEMBLY("%4X *LAS $%04X,Y", originalPC, effAddress);
		BREAK;
	}
	case 0xBC:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_Y, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDY $%04X,X", originalPC, address);
		BREAK;
	}
	case 0xBD:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDA $%04X,X", originalPC, address);
		BREAK;
	}
	case 0xBE:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X LDX $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0xBF:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// Note: Based on SST, LAX with indirect zero page will always take 6 cycles
		if ((ROM_TYPE == ROM::TEST_SST) || (effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		cpuSetRegister(REGISTER_TYPE::RT_A, op);
		cpuSetRegister(REGISTER_TYPE::RT_X, op);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = (op == ZERO);
		DISASSEMBLY("%4X *LAX $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0xC0:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op2 = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_Y));
		int16_t result = op1 - op2;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 >= op2);
		DISASSEMBLY("%4X CPY #$%02X", originalPC, op2);
		BREAK;
	}
	case 0xC1:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		byte result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CMP ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0xC2:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X", originalPC, op);
		BREAK;
	}
	case 0xC3:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), op1, MEMORY_ACCESS_SOURCE::CPU);
		byte decOp1 = op1 - ONE;
		byte op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		int16_t result = op2 - decOp1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op2 == decOp1);
		pNES_flags->FCARRY = (decOp1 <= op2);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), decOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *DCP ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0xC4:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_Y)));
		int16_t result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CPY $%02X", originalPC, address);
		BREAK;
	}
	case 0xC5:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		byte result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CMP $%02X", originalPC, address);
		BREAK;
	}
	case 0xC6:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		uint16_t result = (op - ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));;
		pNES_flags->FZERO = (op == ONE);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X DEC $%02X", originalPC, address);
		BREAK;
	}
	case 0xC7:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte decOp1 = (op - ONE);
		byte op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		int16_t result = op2 - decOp1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op2 == decOp1);
		pNES_flags->FCARRY = (decOp1 <= op2);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, decOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *DCP $%02X", originalPC, address);
		BREAK;
	}
	case 0xC8:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = (uint16_t)(static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_Y))) + ONE;
		cpuSetRegister(REGISTER_TYPE::RT_Y, (op & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = ((op & 0xFF) == ZERO);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X INY", originalPC);
		BREAK;
	}
	case 0xC9:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op1 = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		byte result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CMP #$%02X", originalPC, op1);
		BREAK;
	}
	case 0xCA:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = (uint16_t)(static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X))) - ONE;
		cpuSetRegister(REGISTER_TYPE::RT_X, (op & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = ((op & 0xFF) == ZERO);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X DEX", originalPC);
		BREAK;
	}
	case 0xCB:
	{
		// Implemented based on bus cycles as indicated in Tom Harte logs
		// Also refer to "65xx Processor Data.txt"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op2 = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A))) & (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X)));
		int16_t result = op1 - op2;
		cpuSetRegister(REGISTER_TYPE::RT_X, result);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 >= op2);
		DISASSEMBLY("%4X *AXS #$%02X", originalPC, op2);
		BREAK;
	}
	case 0xCC:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op1 = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_Y)));
		int16_t result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CPY $%04X", originalPC, address);
		BREAK;
	}
	case 0xCD:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op1 = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		byte result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CMP $%04X", originalPC, address);
		BREAK;
	}
	case 0xCE:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		uint16_t result = (op - ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));;
		pNES_flags->FZERO = (op == ONE);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X DEC $%04X", originalPC, address);
		BREAK;
	}
	case 0xCF:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte decOp1 = (op - ONE);
		byte op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		int16_t result = op2 - decOp1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op2 == decOp1);
		pNES_flags->FCARRY = (decOp1 <= op2);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, decOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *DCP $%04X", originalPC, address);
		BREAK;
	}
	case 0xD0:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		SBYTE op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (pNES_flags->FZERO == RESET)
		{
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);

			if ((pNES_cpuRegisters->pc & 0xFF00) != ((pNES_cpuRegisters->pc + (SBYTE)op) & 0xFF00))
			{
				auto dummyAddress = ((pNES_cpuRegisters->pc & 0xFF00) | ((pNES_cpuRegisters->pc + (SBYTE)op) & 0x00FF));
				auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
			}
			// NOTE : To handle "5-branch_delays_irq.nes", refer implementation of BCC instruction (0x90) below for more details
			else
			{
				if (pNES_instance->NES_state.interrupts.wasNMI == NO && pNES_instance->NES_state.interrupts.isNMI == YES)
				{
					pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE;
				}
				if (pNES_instance->NES_state.interrupts.wasIRQ == NO && pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE)
				{
					pNES_instance->NES_state.interrupts.irqDelayInInstructions = ONE;
				}
			}
			pNES_cpuRegisters->pc += op;
		}
		DISASSEMBLY("%4X BNE $%02X", originalPC, (originalPC + TWO + op));
		BREAK;
	}
	case 0xD1:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op1 = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		byte result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CMP ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0xD2:
	{
		STP();
		BREAK;
	}
	case 0xD3:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		// Note: Based on Tom Harte test, DCP with indirect zero page will always take 8 cycles
		//if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op1 = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(yEffAddress, op1, MEMORY_ACCESS_SOURCE::CPU);
		byte decOp1 = (op1 - ONE);
		byte op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		int16_t result = op2 - decOp1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op2 == decOp1);
		pNES_flags->FCARRY = (decOp1 <= op2);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(yEffAddress, decOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *DCP ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0xD4:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory((op + cpuReadRegister(REGISTER_TYPE::RT_X)) & 0xFF, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X,X", originalPC, op);
		BREAK;
	}
	case 0xD5:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		byte result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CMP ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0xD6:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		uint16_t result = (op - ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));;
		pNES_flags->FZERO = (op == ONE);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X DEC $%02X,X", originalPC, address);
		BREAK;
	}
	case 0xD7:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// casting as the read needs to be within zero page
		auto op1 = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op1, MEMORY_ACCESS_SOURCE::CPU);
		byte decOp1 = (op1 - ONE);
		byte op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		int16_t result = op2 - decOp1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op2 == decOp1);
		pNES_flags->FCARRY = (decOp1 <= op2);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), decOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *DCP $%02X,X", originalPC, address);
		BREAK;
	}
	case 0xD8:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		pNES_flags->DECIMAL = RESET;
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X CLD", originalPC);
		BREAK;
	}
	case 0xD9:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op1 = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		byte result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CMP $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0xDA:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP", originalPC);
		BREAK;
	}
	case 0xDB:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// Note: Based on Tom Harte test, DCP with indirect zero page will always take 8 cycles
		//if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op1 = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op1, MEMORY_ACCESS_SOURCE::CPU);
		byte decOp1 = (op1 - ONE);
		byte op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		int16_t result = op2 - decOp1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op2 == decOp1);
		pNES_flags->FCARRY = (decOp1 <= op2);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, decOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *DCP $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0xDC:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%04X,X", originalPC, address);
		BREAK;
	}
	case 0xDD:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op1 = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		byte result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CMP $%04X,X", originalPC, address);
		BREAK;
	}
	case 0xDE:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		uint16_t result = (op - ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));;
		pNES_flags->FZERO = (op == ONE);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X DEC $%04X,X", originalPC, address);
		BREAK;
	}
	case 0xDF:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op1, MEMORY_ACCESS_SOURCE::CPU);
		byte decOp1 = (op1 - ONE);
		byte op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_A)));
		int16_t result = op2 - decOp1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op2 == decOp1);
		pNES_flags->FCARRY = (decOp1 <= op2);;
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, decOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *DCP $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0xE0:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op2 = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X));
		byte result = op1 - op2;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 >= op2);
		DISASSEMBLY("%4X CPX #$%02X", originalPC, op2);
		BREAK;
	}
	case 0xE1:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		byte op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			// BCD Subtraction
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand
			uint8_t borrow = (c == 0) ? 1 : 0;  // Inverted carry for SBC

			// Subtract lower nibbles
			int8_t low_diff = a_low - op_low - borrow;
			uint8_t low_borrow = 0;
			if (low_diff < 0)
			{
				low_diff += 10;      // BCD adjust
				low_borrow = 1;
			}

			// Subtract upper nibbles  
			int8_t high_diff = a_high - op_high - low_borrow;
			if (high_diff < 0)
			{
				high_diff += 10;     // BCD adjust
				pNES_flags->FCARRY = 0;  // Borrow occurred
			}
			else
			{
				pNES_flags->FCARRY = 1;  // No borrow
			}

			result = ((high_diff & 0x0F) << 4) | (low_diff & 0x0F);

			uint8_t inverted_op = op ^ 0xFF;
			uint16_t binary_result = a + inverted_op + c;
			pNES_flags->FOVERFLOW = (((~(a ^ inverted_op)) & (a ^ binary_result)) >> SEVEN);
		}
		else
		{
			op ^= 0x00FF; // Complete code snippet for SBC is same as ADC except for this
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X SBC ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0xE2:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X", originalPC, op);
		BREAK;
	}
	case 0xE3:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		// discard is always read from zero page, hence casting
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// lo is always read from zero page, hence casting
		auto lo = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// hi is always read from zero page, hence casting
		auto hi = readCpuRawMemory((uint8_t)(addressIndexed + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		byte op = readCpuRawMemory((lo + (hi << EIGHT)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), op, MEMORY_ACCESS_SOURCE::CPU);
		byte incOp1 = (op + ONE);
		byte originalIncOp1 = incOp1;
		incOp1 ^= 0x00FF;
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)incOp1 + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ incOp1)) & (a ^ (byte)result)) >> SEVEN);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory((lo + (hi << EIGHT)), originalIncOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *ISB ($%02X,X)", originalPC, address);
		BREAK;
	}
	case 0xE4:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op1 = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X)));
		byte result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CPX $%02X", originalPC, address);
		BREAK;
	}
	case 0xE5:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		byte op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			// BCD Subtraction
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand
			uint8_t borrow = (c == 0) ? 1 : 0;  // Inverted carry for SBC

			// Subtract lower nibbles
			int8_t low_diff = a_low - op_low - borrow;
			uint8_t low_borrow = 0;
			if (low_diff < 0)
			{
				low_diff += 10;      // BCD adjust
				low_borrow = 1;
			}

			// Subtract upper nibbles  
			int8_t high_diff = a_high - op_high - low_borrow;
			if (high_diff < 0)
			{
				high_diff += 10;     // BCD adjust
				pNES_flags->FCARRY = 0;  // Borrow occurred
			}
			else
			{
				pNES_flags->FCARRY = 1;  // No borrow
			}

			result = ((high_diff & 0x0F) << 4) | (low_diff & 0x0F);
			
			uint8_t inverted_op = op ^ 0xFF;
			uint16_t binary_result = a + inverted_op + c;
			pNES_flags->FOVERFLOW = (((~(a ^ inverted_op)) & (a ^ binary_result)) >> SEVEN);
		}
		else
		{
			op ^= 0x00FF; // Complete code snippet for SBC is same as ADC except for this
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X SBC $%02X", originalPC, address);
		BREAK;
	}
	case 0xE6:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		uint16_t result = (op + ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));;
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X INC $%02X", originalPC, address);
		BREAK;
	}
	case 0xE7:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		byte op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte incOp1 = (op + ONE);
		byte originalIncOp1 = incOp1;
		incOp1 ^= 0x00FF;
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)incOp1 + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ incOp1)) & (a ^ (byte)result)) >> SEVEN);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, originalIncOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *ISB $%02X", originalPC, address);
		BREAK;
	}
	case 0xE8:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto op = (uint16_t)(static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X))) + ONE;
		cpuSetRegister(REGISTER_TYPE::RT_X, (op & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, op);
		pNES_flags->FZERO = ((op & 0xFF) == ZERO);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X INX", originalPC);
		BREAK;
	}
	case 0xE9:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		byte op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			// BCD Subtraction
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand
			uint8_t borrow = (c == 0) ? 1 : 0;  // Inverted carry for SBC

			// Subtract lower nibbles
			int8_t low_diff = a_low - op_low - borrow;
			uint8_t low_borrow = 0;
			if (low_diff < 0)
			{
				low_diff += 10;      // BCD adjust
				low_borrow = 1;
			}

			// Subtract upper nibbles  
			int8_t high_diff = a_high - op_high - low_borrow;
			if (high_diff < 0)
			{
				high_diff += 10;     // BCD adjust
				pNES_flags->FCARRY = 0;  // Borrow occurred
			}
			else
			{
				pNES_flags->FCARRY = 1;  // No borrow
			}

			result = ((high_diff & 0x0F) << 4) | (low_diff & 0x0F);
			
			uint8_t inverted_op = op ^ 0xFF;
			uint16_t binary_result = a + inverted_op + c;
			pNES_flags->FOVERFLOW = (((~(a ^ inverted_op)) & (a ^ binary_result)) >> SEVEN);
		}
		else
		{
			op ^= 0x00FF; // Complete code snippet for SBC is same as ADC except for this
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X SBC #$%02X", originalPC, op);
		BREAK;
	}
	case 0xEA:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X NOP", originalPC);
		BREAK;
	}
	case 0xEB:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		byte op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		op ^= 0x00FF; // Complete code snippet for SBC is same as ADC except for this
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)op + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ result)) >> SEVEN);
		DISASSEMBLY("%4X *SBC #$%02X", originalPC, op);
		BREAK;
	}
	case 0xEC:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op1 = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		byte op2 = (static_cast<byte>(cpuReadRegister(REGISTER_TYPE::RT_X)));
		int16_t result = op2 - op1;
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, result);
		pNES_flags->FZERO = (op1 == op2);
		pNES_flags->FCARRY = (op1 <= op2);
		DISASSEMBLY("%4X CPX $%04X", originalPC, address);
		BREAK;
	}
	case 0xED:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		byte op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			// BCD Subtraction
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand
			uint8_t borrow = (c == 0) ? 1 : 0;  // Inverted carry for SBC

			// Subtract lower nibbles
			int8_t low_diff = a_low - op_low - borrow;
			uint8_t low_borrow = 0;
			if (low_diff < 0)
			{
				low_diff += 10;      // BCD adjust
				low_borrow = 1;
			}

			// Subtract upper nibbles  
			int8_t high_diff = a_high - op_high - low_borrow;
			if (high_diff < 0)
			{
				high_diff += 10;     // BCD adjust
				pNES_flags->FCARRY = 0;  // Borrow occurred
			}
			else
			{
				pNES_flags->FCARRY = 1;  // No borrow
			}

			result = ((high_diff & 0x0F) << 4) | (low_diff & 0x0F);
			
			uint8_t inverted_op = op ^ 0xFF;
			uint16_t binary_result = a + inverted_op + c;
			pNES_flags->FOVERFLOW = (((~(a ^ inverted_op)) & (a ^ binary_result)) >> SEVEN);
		}
		else
		{
			op ^= 0x00FF; // Complete code snippet for SBC is same as ADC except for this
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X SBC $%04X", originalPC, address);
		BREAK;
	}
	case 0xEE:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		auto op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		uint16_t result = (op + ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));;
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X INC $%04X", originalPC, address);
		BREAK;
	}
	case 0xEF:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		byte op = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(address, op, MEMORY_ACCESS_SOURCE::CPU);
		byte incOp1 = (op + ONE);
		byte originalIncOp1 = incOp1;
		incOp1 ^= 0x00FF;
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)incOp1 + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ incOp1)) & (a ^ (byte)result)) >> SEVEN);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(address, originalIncOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *ISB $%04X", originalPC, address);
		BREAK;
	}
	case 0xF0:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		SBYTE op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		if (pNES_flags->FZERO == SET)
		{
			auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);

			if ((pNES_cpuRegisters->pc & 0xFF00) != ((pNES_cpuRegisters->pc + (SBYTE)op) & 0xFF00))
			{
				auto dummyAddress = ((pNES_cpuRegisters->pc & 0xFF00) | ((pNES_cpuRegisters->pc + (SBYTE)op) & 0x00FF));
				auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
				cpuTickT(CYCLES_TYPE::READ_CYCLE);
			}
			// NOTE : To handle "5-branch_delays_irq.nes", refer implementation of BCC instruction (0x90) below for more details
			else
			{
				if (pNES_instance->NES_state.interrupts.wasNMI == NO && pNES_instance->NES_state.interrupts.isNMI == YES)
				{
					pNES_instance->NES_state.interrupts.nmiDelayInInstructions = ONE;
				}
				if (pNES_instance->NES_state.interrupts.wasIRQ == NO && pNES_instance->NES_state.interrupts.isIRQ.signal != NES_IRQ_SRC_NONE)
				{
					pNES_instance->NES_state.interrupts.irqDelayInInstructions = ONE;
				}
			}
			pNES_cpuRegisters->pc += op;
		}
		DISASSEMBLY("%4X BEQ $%02X", originalPC, (originalPC + TWO + op));
		BREAK;
	}
	case 0xF1:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		byte op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			// BCD Subtraction
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand
			uint8_t borrow = (c == 0) ? 1 : 0;  // Inverted carry for SBC

			// Subtract lower nibbles
			int8_t low_diff = a_low - op_low - borrow;
			uint8_t low_borrow = 0;
			if (low_diff < 0)
			{
				low_diff += 10;      // BCD adjust
				low_borrow = 1;
			}

			// Subtract upper nibbles  
			int8_t high_diff = a_high - op_high - low_borrow;
			if (high_diff < 0)
			{
				high_diff += 10;     // BCD adjust
				pNES_flags->FCARRY = 0;  // Borrow occurred
			}
			else
			{
				pNES_flags->FCARRY = 1;  // No borrow
			}

			result = ((high_diff & 0x0F) << 4) | (low_diff & 0x0F);
			
			uint8_t inverted_op = op ^ 0xFF;
			uint16_t binary_result = a + inverted_op + c;
			pNES_flags->FOVERFLOW = (((~(a ^ inverted_op)) & (a ^ binary_result)) >> SEVEN);
		}
		else
		{
			op ^= 0x00FF; // Complete code snippet for SBC is same as ADC except for this
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X SBC ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0xF2:
	{
		STP();
		BREAK;
	}
	case 0xF3:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer https://www.c64-wiki.com/wiki/Indirect-indexed_addressing
		// Also refer https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1452s&ab_channel=javidx9
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t lo = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		// casting because we need to read from next "page zero" memory location
		uint16_t hi = readCpuRawMemory((uint8_t)(address + ONE), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t effAddress = (uint16_t)(lo | (hi << EIGHT));
		uint16_t yEffAddress = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_Y) + effAddress;
		// Note: Based on Tom Harte test, ISB with indirect zero page will always take 8 cycles
		//if ((yEffAddress & 0xFF00) != (effAddress & 0xFF00))
		{
			auto dummyAddress = ((effAddress & 0xFF00) | (yEffAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		byte op = readCpuRawMemory(yEffAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(yEffAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte incOp1 = (op + ONE);
		byte originalIncOp1 = incOp1;
		incOp1 ^= 0x00FF;
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)incOp1 + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ incOp1)) & (a ^ (byte)result)) >> SEVEN);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(yEffAddress, originalIncOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *ISB ($%02X),Y", originalPC, address);
		BREAK;
	}
	case 0xF4:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		// Refer "nestest-bus-cycles.log" for the info on these undocumented opcodes
		auto op = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(op, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		discard = readCpuRawMemory((op + cpuReadRegister(REGISTER_TYPE::RT_X)) & 0xFF, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%02X,X", originalPC, op);
		BREAK;
	}
	case 0xF5:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory((uint8_t)address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		byte op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			// BCD Subtraction
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand
			uint8_t borrow = (c == 0) ? 1 : 0;  // Inverted carry for SBC

			// Subtract lower nibbles
			int8_t low_diff = a_low - op_low - borrow;
			uint8_t low_borrow = 0;
			if (low_diff < 0)
			{
				low_diff += 10;      // BCD adjust
				low_borrow = 1;
			}

			// Subtract upper nibbles  
			int8_t high_diff = a_high - op_high - low_borrow;
			if (high_diff < 0)
			{
				high_diff += 10;     // BCD adjust
				pNES_flags->FCARRY = 0;  // Borrow occurred
			}
			else
			{
				pNES_flags->FCARRY = 1;  // No borrow
			}

			result = ((high_diff & 0x0F) << 4) | (low_diff & 0x0F);
			
			uint8_t inverted_op = op ^ 0xFF;
			uint16_t binary_result = a + inverted_op + c;
			pNES_flags->FOVERFLOW = (((~(a ^ inverted_op)) & (a ^ binary_result)) >> SEVEN);
		}
		else
		{
			op ^= 0x00FF; // Complete code snippet for SBC is same as ADC except for this
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X SBC $%02X,X", originalPC, address);
		BREAK;
	}
	case 0xF6:
	{
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		uint16_t result = (op + ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));;
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X INC $%02X,X", originalPC, address);
		BREAK;
	}
	case 0xF7:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X    A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto address = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto discard = readCpuRawMemory(address, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto addressIndexed = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		byte op = readCpuRawMemory(static_cast<uint8_t>(addressIndexed), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), op, MEMORY_ACCESS_SOURCE::CPU);
		byte incOp1 = (op + ONE);
		byte originalIncOp1 = incOp1;
		incOp1 ^= 0x00FF;
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)incOp1 + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ incOp1)) & (a ^ (byte)result)) >> SEVEN);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(static_cast<uint8_t>(addressIndexed), originalIncOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *ISB $%02X,X", originalPC, address);
		BREAK;
	}
	case 0xF8:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		pNES_flags->DECIMAL = SET;
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X SED", originalPC);
		BREAK;
	}
	case 0xF9:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		byte op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			// BCD Subtraction
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand
			uint8_t borrow = (c == 0) ? 1 : 0;  // Inverted carry for SBC

			// Subtract lower nibbles
			int8_t low_diff = a_low - op_low - borrow;
			uint8_t low_borrow = 0;
			if (low_diff < 0)
			{
				low_diff += 10;      // BCD adjust
				low_borrow = 1;
			}

			// Subtract upper nibbles  
			int8_t high_diff = a_high - op_high - low_borrow;
			if (high_diff < 0)
			{
				high_diff += 10;     // BCD adjust
				pNES_flags->FCARRY = 0;  // Borrow occurred
			}
			else
			{
				pNES_flags->FCARRY = 1;  // No borrow
			}

			result = ((high_diff & 0x0F) << 4) | (low_diff & 0x0F);
			
			uint8_t inverted_op = op ^ 0xFF;
			uint16_t binary_result = a + inverted_op + c;
			pNES_flags->FOVERFLOW = (((~(a ^ inverted_op)) & (a ^ binary_result)) >> SEVEN);
		}
		else
		{
			op ^= 0x00FF; // Complete code snippet for SBC is same as ADC except for this
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X SBC $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0xFA:
	{
		CPUDEBUG("%04X  %02X       A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto discard = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP", originalPC);
		BREAK;
	}
	case 0xFB:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_Y);
		// Note: Based on Tom Harte test, ISB with indirect zero page will always take 8 cycles
		//if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		byte op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte incOp1 = (op + ONE);
		byte originalIncOp1 = incOp1;
		incOp1 ^= 0x00FF;
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)incOp1 + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ incOp1)) & (a ^ (byte)result)) >> SEVEN);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, originalIncOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *ISB $%04X,Y", originalPC, address);
		BREAK;
	}
	case 0xFC:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		DISASSEMBLY("%4X *NOP $%04X,X", originalPC, address);
		BREAK;
	}
	case 0xFD:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		if ((effAddress & 0xFF00) != (address & 0xFF00))
		{
			auto dummyAddress = ((address & 0xFF00) | (effAddress & 0x00FF));
			auto discard = readCpuRawMemory(dummyAddress, MEMORY_ACCESS_SOURCE::CPU);
			cpuTickT(CYCLES_TYPE::READ_CYCLE);
		}
		byte op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = ZERO;
		if (pNES_flags->DECIMAL == SET && ROM_TYPE == ROM::TEST_ROM_BIN)
		{
			// BCD Subtraction
			uint8_t a_low = a & 0x0F;           // Lower nibble of A
			uint8_t a_high = (a >> 4) & 0x0F;   // Upper nibble of A  
			uint8_t op_low = op & 0x0F;         // Lower nibble of operand
			uint8_t op_high = (op >> 4) & 0x0F; // Upper nibble of operand
			uint8_t borrow = (c == 0) ? 1 : 0;  // Inverted carry for SBC

			// Subtract lower nibbles
			int8_t low_diff = a_low - op_low - borrow;
			uint8_t low_borrow = 0;
			if (low_diff < 0)
			{
				low_diff += 10;      // BCD adjust
				low_borrow = 1;
			}

			// Subtract upper nibbles  
			int8_t high_diff = a_high - op_high - low_borrow;
			if (high_diff < 0)
			{
				high_diff += 10;     // BCD adjust
				pNES_flags->FCARRY = 0;  // Borrow occurred
			}
			else
			{
				pNES_flags->FCARRY = 1;  // No borrow
			}

			result = ((high_diff & 0x0F) << 4) | (low_diff & 0x0F);
			
			uint8_t inverted_op = op ^ 0xFF;
			uint16_t binary_result = a + inverted_op + c;
			pNES_flags->FOVERFLOW = (((~(a ^ inverted_op)) & (a ^ binary_result)) >> SEVEN);
		}
		else
		{
			op ^= 0x00FF; // Complete code snippet for SBC is same as ADC except for this
			result = (uint16_t)(a + (uint16_t)op + c);
			pNES_flags->FCARRY = (result > 255);
			// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
			pNES_flags->FOVERFLOW = (((~(a ^ op)) & (a ^ (byte)result)) >> SEVEN);
		}
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		DISASSEMBLY("%4X SBC $%04X,X", originalPC, address);
		BREAK;
	}
	case 0xFE:
	{
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		uint16_t result = (op + ONE);
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));;
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, static_cast<byte>(result), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X INC $%04X,X", originalPC, address);
		BREAK;
	}
	case 0xFF:
	{
		// Implemented based on bus cycles depicted in "nestest-bus-cycles.log"
		CPUDEBUG("%04X  %02X %02X %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu"
			, originalPC
			, originalOp0
			, originalOp1
			, originalOp2
			, pNES_cpuRegisters->a
			, pNES_cpuRegisters->x
			, pNES_cpuRegisters->y
			, pNES_cpuRegisters->p.p
			, pNES_cpuRegisters->sp
			, originalCycles
		);
		auto lo = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		auto hi = readCpuRawMemory(pNES_cpuRegisters->pc, MEMORY_ACCESS_SOURCE::CPU);
		INCREMENT_PC_BY_ONE();
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		uint16_t address = (lo | (hi << EIGHT));
		uint16_t effAddress = address + cpuReadRegister(REGISTER_TYPE::RT_X);
		auto discard = readCpuRawMemory(((address & 0xFF00) | (effAddress & 0x00FF)), MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		byte op = readCpuRawMemory(effAddress, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::READ_CYCLE);
		writeCpuRawMemory(effAddress, op, MEMORY_ACCESS_SOURCE::CPU);
		byte incOp1 = (op + ONE);
		byte originalIncOp1 = incOp1;
		incOp1 ^= 0x00FF;
		auto a = (uint16_t)cpuReadRegister(REGISTER_TYPE::RT_A);
		auto c = (uint16_t)pNES_flags->FCARRY;
		uint16_t result = (uint16_t)(a + (uint16_t)incOp1 + c);
		cpuSetRegister(REGISTER_TYPE::RT_A, (result & 0xFF));
		pNES_flags->FNEGATIVE = GETBIT(SEVEN, (result & 0xFF));
		pNES_flags->FZERO = ((result & 0xFF) == ZERO);
		pNES_flags->FCARRY = (result > 255);
		// Refer for overflow : https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
		pNES_flags->FOVERFLOW = (((~(a ^ incOp1)) & (a ^ (byte)result)) >> SEVEN);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		writeCpuRawMemory(effAddress, originalIncOp1, MEMORY_ACCESS_SOURCE::CPU);
		cpuTickT(CYCLES_TYPE::WRITE_CYCLE);
		DISASSEMBLY("%4X *ISB $%04X,X", originalPC, address);
		BREAK;
	}
	default:
	{
		unimplementedInstruction();
		BREAK;
	}
	}

	processUnusedFlags(ONE);
}