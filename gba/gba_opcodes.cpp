#include "gba.h"

#pragma region ARM7TDMI_SPECIFIC_MACROS
#define THUMB_SOFTWARE_INTERRUPT_MASK					0x0000FF00
#define THUMB_SOFTWARE_INTERRUPT_INSTRUCTION			0x0000DF00
#define UNCONDITIONAL_BRANCH_MASK						0x0000F800
#define UNCONDITIONAL_BRANCH_INSTRUCTION				0x0000E000
#define CONDITIONAL_BRANCH_MASK							0x0000F000
#define CONDITIONAL_BRANCH_INSTRUCTION					0x0000D000
#define MULTIPLE_LOAD_STORE_MASK						0x0000F000
#define MULTIPLE_LOAD_STORE_INSTRUCTION					0x0000C000
#define LONG_BRANCH_WITH_LINK_MASK						0x0000F000
#define LONG_BRANCH_WITH_LINK_INSTRUCTION				0x0000F000
#define OFFSET_TO_STACK_POINTER_MASK					0x0000FF00
#define OFFSET_TO_STACK_POINTER_INSTRUCTION				0x0000B000
#define PUSH_POP_REGISTERS_MASK							0x0000F600
#define PUSH_POP_REGISTERS_INSTRUCTION					0x0000B400
#define LOAD_STORE_HALFWORD_MASK						0x0000F000
#define LOAD_STORE_HALFWORD_INSTRUCTION					0x00008000
#define SP_RELATIVE_LOAD_STORE_MASK						0x0000F000
#define SP_RELATIVE_LOAD_STORE_INSTRUCTION				0x00009000
#define LOAD_ADDRESS_MASK								0x0000F000
#define LOAD_ADDRESS_INSTRUCTION						0x0000A000
#define LOAD_STORE_IMMEDIATE_OFFSET_MASK				0x0000E000
#define LOAD_STORE_IMMEDIATE_OFFSET_INSTRUCTION			0x00006000
#define LOAD_STORE_REGISTER_OFFSET_MASK					0x0000F200
#define LOAD_STORE_REGISTER_OFFSET_INSTRUCTION			0x00005000
#define LOAD_STORE_SIGN_EXT_BYTE_HALFWORD_MASK			0x0000F200
#define LOAD_STORE_SIGN_EXT_BYTE_HALFWORD_INSTRUCTION	0x00005200
#define PC_RELATIVE_LOAD_MASK							0x0000F800
#define PC_RELATIVE_LOAD_INSTRUCTION					0x00004800
#define HI_REGISTER_OP_OR_BRANCH_EXCHANGE_MASK			0x0000FC00
#define HI_REGISTER_OP_OR_BRANCH_EXCHANGE_INSTRUCTION	0x00004400
#define ALU_OPERATIONS_MASK								0x0000FC00
#define ALU_OPERATIONS_INSTRUCTION						0x00004000
#define MOV_CMP_ADD_SUB_IMMEDIATE_MASK					0x0000E000
#define MOV_CMP_ADD_SUB_IMMEDIATE_INSTRUCTION			0x00002000
#define ADD_SUBTRACT_MASK								0x0000F800
#define ADD_SUBTRACT_INSTRUCTION						0x00001800
#define MOVE_SHIFTED_REGISTER_MASK						0x0000E000
#define MOVE_SHIFTED_REGISTER_INSTRUCTION				0x00000000

#define BRANCH_AND_BRANCH_EXCHANGE_MASK					0x0FFFFFF0
#define BRANCH_AND_BRANCH_EXCHANGE_INSTRUCTION			0x012FFF10
#define BLOCK_DATA_TRANSFER_MASK						0x0E000000
#define BLOCK_DATA_TRANSFER_INSTRUCTION					0x08000000
#define BRANCH_AND_BRANCH_WITH_LINK_MASK				0x0F000000
#define BRANCH_INSTRUCTION								0x0A000000
#define BRANCH_WITH_LINK_INSTRUCTION					0x0B000000
#define SOFTWARE_INTERRUPT_MASK							0x0F000000
#define SOFTWARE_INTERRUPT_INSTRUCTION					0x0F000000
#define SOFTWARE_INTERRUPT_SWI_SVC_HANDLER				0x00000008
#define UNDEFINED_MASK									0x0E000010
#define UNDEFINED_INSTRUCTION							0x06000010
#define SINGLE_DATA_TRANSFER_MASK						0x0C000000
#define SINGLE_DATA_TRANSFER_INSTRUCTION				0x04000000
#define SINGLE_DATA_SWAP_MASK							0x0F800FF0
#define SINGLE_DATA_SWAP_INSTRUCTION					0x01000090
#define MULTIPLY_AND_MULTIPLY_ACCUMULATE_MASK			0x0F8000F0
#define MULTIPLY_INSTRUCTION							0x00000090
#define MULTIPLY_ACCUMULATE_INSTRUCTION					0x00800090
#define HALF_WORD_DATA_TRANSFER_REGISTER_MASK			0x0E400F90
#define HALF_WORD_DATA_TRANSFER_REGISTER_INSTRUCTION	0x00000090
#define HALF_WORD_DATA_TRANSFER_IMMEDIATE_MASK			0x0E400090
#define HALF_WORD_DATA_TRANSFER_IMMEDIATE_INSTRUCTION	0x00400090
#define MRS_MASK										0x0FBF0000
#define MRS_INSTRUCTION									0x010F0000
#define MSR_MASK										0x0DB0F000
#define MSR_INSTRUCTION									0x0120F000
#define DATA_PROCESSING_MASK							0x0C000000
#define DATA_PROCESSING_INSTRUCTION						0x00000000
#pragma endregion ARM7TDMI_SPECIFIC_MACROS

bool GBA_t::TickMultiply(FLAG isSigned, uint64_t multiplier)
{
	uint32_t mask = 0xFFFFFF00;
	bool full = false;

	cpuIdleCycles();

	while (TRUE)
	{
		multiplier &= mask;

		if (multiplier == ZERO)
		{
			BREAK;
		}

		if (isSigned == YES)
		{
			if (multiplier == mask)
			{
				BREAK;
			}
		}

		mask <<= EIGHT;
		full = true;
		cpuIdleCycles();
	}

	return full;
}

bool GBA_t::MultiplyCarrySimple(uint32_t multiplier)
{
	// Carry comes directly from final injected booth carry bit.
	// Final booth addend is negative only if upper 2 bits are 10.
	return (multiplier >> 30) == 2;
}

bool GBA_t::MultiplyCarryLo(
	uint32_t multiplicand,
	uint32_t multiplier,
	uint32_t accum /* = 0 */
)
{
	// Set low bit of multiplicand to cause negation to invert the upper bits.
	// This bit cannot propagate to the resulting carry bit.
	multiplicand |= 1;

	// Optimized first iteration.
	uint32_t booth = (int32_t)(multiplier << 31) >> 31;
	uint32_t carry = multiplicand * booth;
	uint32_t sum = carry + accum;

	int shift = 29;

	do
	{
		// Process 8 multiplier bits using 4 booth iterations.
		for (int i = 0; i < 4; i++, shift -= 2)
		{
			// Get next booth factor (-2 to 2, shifted left by 30-shift).
			uint32_t next_booth = (int32_t)(multiplier << shift) >> shift;
			uint32_t factor = next_booth - booth;
			booth = next_booth;

			// Get scaled value of booth addend.
			uint32_t addend = multiplicand * factor;

			// Accumulate addend with carry-save add.
			accum ^= carry ^ addend;
			sum += addend;
			carry = sum - accum;
		}
	}
	while (booth != multiplier);

	// Carry flag comes from bit 31 of carry-save adder's final carry.
	return (carry >> 31);
}

bool GBA_t::MultiplyCarryHi(
	bool sign_extend,
	uint32_t multiplicand,
	uint32_t multiplier,
	uint32_t accum_hi /* = 0 */
)
{
	// Only last 3 booth iterations are relevant to output carry.
	// Reduce scale of both inputs to get upper bits of 64-bit booth addends
	// in upper bits of 32-bit values, while handling sign extension.
	if (sign_extend)
	{
		multiplicand = (int32_t)multiplicand >> 6;
		multiplier = (int32_t)multiplier >> 26;
	}
	else
	{
		multiplicand >>= 6;
		multiplier >>= 26;
	}

	// Set low bit of multiplicand to cause negation to invert the upper bits.
	// This bit cannot propagate to the resulting carry bit.
	multiplicand |= 1;

	// Pre-populate magic bit 61 for carry.
	uint32_t carry = ~accum_hi & 0x20000000;

	// Pre-populate magic bits 63-60 for accum (with carry magic pre-added).
	uint32_t accum = accum_hi - 0x08000000;

	// Get factors for last 3 booth iterations.
	uint32_t booth0 = (int32_t)(multiplier << 27) >> 27;
	uint32_t booth1 = (int32_t)(multiplier << 29) >> 29;
	uint32_t booth2 = (int32_t)(multiplier << 31) >> 31;

	uint32_t factor0 = multiplier - booth0;
	uint32_t factor1 = booth0 - booth1;
	uint32_t factor2 = booth1 - booth2;

	// Get scaled value of 3rd-last booth addend.
	uint32_t addend = multiplicand * factor2;

	// Finalize bits 61-60 of accum magic using its sign.
	accum -= addend & 0x10000000;

	// Get scaled value of 2nd-last booth addend.
	addend = multiplicand * factor1;

	// Finalize bits 63-62 of accum magic using its sign.
	accum -= addend & 0x40000000;

	// Get carry from carry-save add in bit 61 and propagate it to bit 62.
	uint32_t sum = accum + (addend & 0x20000000);

	// Subtract out carry magic to get actual accum magic.
	accum -= carry;

	// Get scaled value of last booth addend.
	addend = multiplicand * factor0;

	// Add to bit 62 and propagate carry.
	sum += addend & 0x40000000;

	// Cancel out accum magic bit 63 to get carry bit 63.
	return (sum ^ accum) >> 31;
}

bool GBA_t::didConditionalCheckPass(uint32_t opCodeConditionalBits, uint32_t cpsr)
{
	CPUDEBUG("Condition: 0x%X", (uint32_t)opCodeConditionalBits);

	psr_t currentCPSR = { ZERO };
	currentCPSR.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR);

	switch ((CPSR_CONDITION_CODE)opCodeConditionalBits)
	{
	case CPSR_CONDITION_CODE::CPSR_EQ_SetZ:
	{
		RETURN((currentCPSR.psrFields.psrZeroBit == SET) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_NE_ClearZ:
	{
		RETURN((currentCPSR.psrFields.psrZeroBit == RESET) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_CS_SetC:
	{
		RETURN((currentCPSR.psrFields.psrCarryBorrowExtBit == SET) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_CS_ClearC:
	{
		RETURN((currentCPSR.psrFields.psrCarryBorrowExtBit == RESET) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_MI_SetN:
	{
		RETURN((currentCPSR.psrFields.psrNegativeBit == SET) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_PL_ClearN:
	{
		RETURN((currentCPSR.psrFields.psrNegativeBit == RESET) ? YES : NO);;
	}
	case CPSR_CONDITION_CODE::CPSR_VS_SetV:
	{
		RETURN((currentCPSR.psrFields.psrOverflowBit == SET) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_VC_ClearV:
	{
		RETURN((currentCPSR.psrFields.psrOverflowBit == RESET) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_HI_SetC_AND_ClearZ:
	{
		RETURN(((currentCPSR.psrFields.psrCarryBorrowExtBit == SET) && (currentCPSR.psrFields.psrZeroBit == RESET)) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_LS_ClearC_OR_SetZ:
	{
		RETURN(((currentCPSR.psrFields.psrCarryBorrowExtBit == RESET) || (currentCPSR.psrFields.psrZeroBit == SET)) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_GE_NEqualsV:
	{
		RETURN(((currentCPSR.psrFields.psrNegativeBit == currentCPSR.psrFields.psrOverflowBit)) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_LT_NNotEqualsV:
	{
		RETURN(((currentCPSR.psrFields.psrNegativeBit != currentCPSR.psrFields.psrOverflowBit)) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_CS_ClearZ_AND_NEqualsV:
	{
		RETURN(((currentCPSR.psrFields.psrZeroBit == RESET) && ((currentCPSR.psrFields.psrNegativeBit == currentCPSR.psrFields.psrOverflowBit))) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_CS_SetZ_OR_NNotEqualsV:
	{
		RETURN(((currentCPSR.psrFields.psrZeroBit == SET) || ((currentCPSR.psrFields.psrNegativeBit != currentCPSR.psrFields.psrOverflowBit))) ? YES : NO);
	}
	case CPSR_CONDITION_CODE::CPSR_AL_IgnoreConditions:
	{
		RETURN YES;
	}
	default:
	{
		RETURN NO;
	}
	}
}

GBA_WORD GBA_t::performShiftOperation(bool updateFlag, SHIFT_TYPE shiftType, uint32_t shiftAmount, uint32_t dataToBeShifted, bool quirkEnabled)
{
	uint32_t originalData = dataToBeShifted;

	if (shiftAmount == ZERO)
	{
		if (quirkEnabled == DISABLED)
		{
			RETURN dataToBeShifted;
		}
		else
		{
			switch (shiftType)
			{
			case SHIFT_TYPE::LSL:
			{
				RETURN dataToBeShifted;
			}
			case SHIFT_TYPE::LSR:
			case SHIFT_TYPE::ASR:
			{
				shiftAmount = THIRTYTWO;
				BREAK;
			}
			case SHIFT_TYPE::ROR:
			{
				BYTE oldC = pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit;

				if (updateFlag)
				{
					uint32_t carryBit = GETBIT(ZERO, originalData);
					pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
				}

				dataToBeShifted = ((oldC << THIRTYONE) | (originalData >> ONE));
				RETURN dataToBeShifted;
			}
			default:
			{
				FATAL("Unknown Shift Type (shift amount = 0)");
				RETURN dataToBeShifted;
			}
			}
		}
	}

	switch (shiftType)
	{
	case SHIFT_TYPE::LSL:
	{
		if (shiftAmount < THIRTYTWO)
		{
			if (updateFlag)
			{
				uint32_t bitToCheckForCarryFlag = THIRTYTWO - shiftAmount;
				uint32_t carryBit = GETBIT(bitToCheckForCarryFlag, originalData);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
			}
			dataToBeShifted = originalData << shiftAmount;
			RETURN dataToBeShifted;
		}
		else
		{
			if (updateFlag)
			{
				if (shiftAmount == THIRTYTWO)
				{
					uint32_t carryBit = GETBIT(ZERO, originalData);
					pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
				}
				else
				{
					pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ZERO;
				}
			}
			dataToBeShifted = ZERO;
			RETURN dataToBeShifted;
		}
	}
	case SHIFT_TYPE::LSR:
	{
		if (shiftAmount < THIRTYTWO)
		{
			if (updateFlag)
			{
				uint32_t bitToCheckForCarryFlag = shiftAmount - ONE;
				uint32_t carryBit = GETBIT(bitToCheckForCarryFlag, originalData);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
			}
			dataToBeShifted = originalData >> shiftAmount;
			RETURN dataToBeShifted;
		}
		else
		{
			if (updateFlag)
			{
				if (shiftAmount == THIRTYTWO)
				{
					uint32_t carryBit = GETBIT(THIRTYONE, originalData);
					pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
				}
				else
				{
					pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ZERO;
				}
			}
			dataToBeShifted = ZERO;
			RETURN dataToBeShifted;
		}
	}
	case SHIFT_TYPE::ASR:
	{
		if (shiftAmount < THIRTYTWO)
		{
			if (updateFlag)
			{
				uint32_t bitToCheckForCarryFlag = shiftAmount - ONE;
				uint32_t carryBit = GETBIT(bitToCheckForCarryFlag, originalData);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
			}

			int32_t signedData = originalData;
			signedData >>= shiftAmount; // Shifting signed data results in sign extention
			dataToBeShifted = signedData;
			RETURN dataToBeShifted;
		}
		else
		{
			if (updateFlag)
			{
				uint32_t carryBit = GETBIT(THIRTYONE, originalData);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
			}
			int32_t signedData = originalData;
			signedData >>= THIRTYONE; // Filled with bit 31
			dataToBeShifted = signedData;
			RETURN dataToBeShifted;
		}
	}
	case SHIFT_TYPE::ROR:
	{
		if (shiftAmount == THIRTYTWO)
		{
			if (updateFlag)
			{
				uint32_t carryBit = GETBIT(THIRTYONE, originalData);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
			}
			RETURN originalData;
		}
		else
		{
			while (shiftAmount > THIRTYTWO)
			{
				shiftAmount -= THIRTYTWO;
			}

			if (updateFlag)
			{
				uint32_t bitToCheckForCarryFlag = shiftAmount - ONE;
				uint32_t carryBit = GETBIT(bitToCheckForCarryFlag, originalData);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
			}

			dataToBeShifted = (originalData >> shiftAmount) | (originalData << (((shiftAmount) * (-ONE)) & 0x1F));
			RETURN dataToBeShifted;
		}
	}
	default:
	{
		FATAL("Unknown Shift Type");
		RETURN dataToBeShifted;
	}
	}
}

// Thumb Instructions
bool GBA_t::ThumbSoftwareInterrupt()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & THUMB_SOFTWARE_INTERRUPT_MASK);

	if (strippedOpCode == THUMB_SOFTWARE_INTERRUPT_INSTRUCTION)
	{
		// Need to go to supervisor mode
		// Save the pc in lr_svc
		// Save the cpsr contents to svc_spsr
		psr_t currentCPSR;
		currentCPSR.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR);

		// Now, since the cpsr is saved in svc_spsr, modify the current status of cpsr
		setARMMode(OP_MODE_TYPE::OP_SVC);
		setARMState(STATE_TYPE::ST_ARM);

		// Disable the irq
		pGBA_registers->cpsr.psrFields.psrIRQDisBit = ONE;

		// PC points to fetch stage, i.e PC+4 after Thumb SWI instruction
		// We have to save the immediate next instruction to BL, hence we store PC-2 in thumb mode
		uint32_t pcToStoreInLR = pGBA_cpuInstance->registers.pc - TWO;
		cpuSetRegister(REGISTER_BANK_TYPE::RB_SVC, (REGISTER_TYPE)LR, getARMState(), pcToStoreInLR);
		// Save SPSR as we will change ARM Mode
		cpuSetRegister(REGISTER_BANK_TYPE::RB_SVC, (REGISTER_TYPE)SPSR, getARMState(), currentCPSR.psrMemory);

		// Update the PC
		cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), SOFTWARE_INTERRUPT_SWI_SVC_HANDLER);
		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::UnconditionalBranch()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & UNCONDITIONAL_BRANCH_MASK);
	if (strippedOpCode == UNCONDITIONAL_BRANCH_INSTRUCTION)
	{
		uint32_t offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.UNCONDITIONAL_BRANCH.offset;
		int32_t soffset = signExtend32(offset, ELEVEN);
		soffset <<= ONE; // multiply by 2

		uint32_t pc = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
		pc += (soffset);
		pc |= ONE; // For thumb mode
		cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), pc);

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::ConditionalBranch()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & CONDITIONAL_BRANCH_MASK);
	if (strippedOpCode == CONDITIONAL_BRANCH_INSTRUCTION)
	{
		if (didConditionalCheckPass(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.CONDITIONAL_BRANCH.cond, pGBA_cpuInstance->registers.cpsr.psrMemory))
		{
			uint32_t offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.CONDITIONAL_BRANCH.soffset;
			int32_t soffset = signExtend32(offset, EIGHT);
			soffset <<= ONE; // multiply by 2

			uint32_t pc = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
			pc += (soffset);
			pc |= ONE; // For thumb mode
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), pc);
		}
		else
		{
			// Increment the PC
			pGBA_cpuInstance->registers.pc += TWO;
			pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;
		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::MultipleLoadStore()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & MULTIPLE_LOAD_STORE_MASK);
	if (strippedOpCode == MULTIPLE_LOAD_STORE_INSTRUCTION)
	{
		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		uint32_t registerList = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MULTIPLE_LOAD_STORE.rlist;
		uint32_t rb = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MULTIPLE_LOAD_STORE.rb;
		bool l = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MULTIPLE_LOAD_STORE.l;

		uint32_t address = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rb);

		if (registerList == ZERO)
		{
			CPUWARN("Empty Register List for Multiple Load Store");

			if (l == YES)
			{
				auto memory2register = readRawMemory<GBA_WORD>(address, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), memory2register);
			}
			else
			{
				auto pc = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
				// write PC to [SP]
				writeRawMemory<GBA_WORD>(address, pc, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
			}

			// Increment rb (but this should behave as though all registers were loaded/stored, so increment 16*4 times)
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rb, getARMState(), address + 0x40);
		}
		else
		{
			// NOTE: Lowest register maps to lowest address

			if (l == YES)
			{
				uint32_t memory2register = ZERO;

				for (uint8_t rt = (uint8_t)REGISTER_TYPE::RT_0; rt <= (uint8_t)REGISTER_TYPE::RT_7; rt++) // looping from lower to higher registers
				{
					if ((registerList >> rt) & 0x01) // if "rt" is part of register list
					{
						// read from memory
						memory2register = readRawMemory<GBA_WORD>(address, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);

						// Even before the first loop is completed, if write back is enabled, base register is updated
						// so, if rb is part of the list, then in further loops, rb will be updated and hence the writeback is overriden

						// write to register
						cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rt, getARMState(), memory2register);

						address += sizeof(GBA_WORD);
					}
				}

				// NOTE: Writebacks most probably happens at 2nd cycles of the instruction
				// The below code snippet gives the same output as that of mentioned in "BLOCK DATA TRANSFER"'s load operation (rlist != 0)
				// i.e. if we had used the same method mentioned in "BLOCK DATA TRANSFER"
				// we would have still achieved the following:
				// "Even before the first loop is completed, if write back is enabled, base register is updated
				// so, if rb is part of the list, then in further loops, rb will be updated and hence the writeback is overriden"
				// Yet we have moved out this logic from above for loop
				// Only reason I can think of is to handle the internal Cycles which was not there for "BLOCK DATA TRANSFER"

				cpuIdleCycles();

				// If rb is not part of register list, then perform writeback
				if (~registerList & (1 << rb))
				{
					CPUTODO("Find proper source for code snippet at line %d in file %s", __LINE__, __FILE__);
					cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rb, getARMState(), address);
				}
			}
			else
			{
				uint32_t register2memory = ZERO;
				bool firstTransfer = YES;

				uint32_t new_base = address
#if defined(_MSC_VER)
					+__popcnt(registerList)
#elif defined(__EMSCRIPTEN__) || defined(__clang__) || defined(__GNUC__)
					+__builtin_popcountll(registerList)
#else
					+ countSetBits(registerList)
#endif
					* sizeof(GBA_WORD);

				for (uint8_t rt = (uint8_t)REGISTER_TYPE::RT_0; rt <= (uint8_t)REGISTER_TYPE::RT_7; rt++)	// looping from lower to higher registers
				{
					if ((registerList >> rt) & 0x01) // if "rt" is part of register list
					{
						// read from register
						register2memory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rt);

						// write to memory
						writeRawMemory<GBA_WORD>(address, register2memory, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);

						// NOTE:
						// As mentioned in http://problemkaputt.de/gbatek-thumb-opcodes-memory-multiple-load-store-push-pop-and-ldm-stm.htm
						// Writeback with Rb included in Rlist: Store OLD base if Rb is FIRST entry in Rlist, 
						// Otherwise store NEW base (STM/ARMv4)

						// NOTE:
						// Also mentioned in ARM DOC
						// Writing to memory before the writeback will handle the condition mentioned in 4.11.6 of https://www.dwedit.org/files/ARM7TDMI.pdf

						// To handle the above mentioned note, the sequence inside the for loop is as follows:
						// Reason being, if rb is first in list, then its not modified during the first loop
						// And write back is done before writeRawMemory, so old [rb] will be written to memory
						// then write back happens to rb with updated value, now if rb is part of list
						// then from second loop onwards cpuReadRegister of rb gives new value
						if (firstTransfer == YES)
						{
							cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rb, getARMState(), new_base);
						}

						address += sizeof(GBA_WORD);

						firstTransfer = NO;
					}
				}
			}

		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::LongBranchWithLink()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & LONG_BRANCH_WITH_LINK_MASK);
	if (strippedOpCode == LONG_BRANCH_WITH_LINK_INSTRUCTION)
	{
		uint32_t offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LONG_BRANCH_WITH_LINK.offset;
		bool h = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LONG_BRANCH_WITH_LINK.h;

		if (h == YES)
		{
			// Operation to be done : PC = LR + (nn SHL 1), and LR = PC + 2 OR 1
			offset <<= ONE; // handling "nn SHL 1"
			auto oldLR = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)LR);
			auto oldPC = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
			// LR = PC + 2 OR 1, but our PC is already incremented by 4 as it is always pointing to fetch stage... so PC - 2 OR 1
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)LR, getARMState(), ((oldPC - TWO) | ONE));
			uint32_t newPC = oldLR + offset;
			newPC &= 0xFFFFFFFE;
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), newPC);
		}
		else
		{
			// Operation to be done : LR = PC + 4 + (nn SHL 12)
			offset = signExtend32(offset, ELEVEN); // since this is upper 11 bits, sign bit would be present
			offset <<= TWELVE; // handling "nn SHL 12"
			auto pc = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
			// LR = PC + 4 + (nn SHL 12), but our PC is already incremented by 4 as it is always pointing to fetch stage... so PC + (nn SHL 12)
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)LR, getARMState(), pc + offset);

			// Increment the PC
			pGBA_cpuInstance->registers.pc += TWO;
			pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;
		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::AddOffsetToStackPointer()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & OFFSET_TO_STACK_POINTER_MASK);
	if (strippedOpCode == OFFSET_TO_STACK_POINTER_INSTRUCTION)
	{
		uint32_t offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ADD_OFFSET_TO_STACK_POINTER.sword7;
		bool s = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ADD_OFFSET_TO_STACK_POINTER.s;

		auto sp = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP);

		if (s == YES)
		{
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP, getARMState(), sp - (offset << TWO));
		}
		else
		{
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP, getARMState(), sp + (offset << TWO));
		}

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::PushPopRegisters()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & PUSH_POP_REGISTERS_MASK);
	if (strippedOpCode == PUSH_POP_REGISTERS_INSTRUCTION)
	{
		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		uint32_t registerList = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.PUSH_POP_REGISTERS.rlist;
		bool l = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.PUSH_POP_REGISTERS.l;
		bool r = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.PUSH_POP_REGISTERS.r;

		// The instructions in this group allow registers 0 - 7 and optionally LR to be pushed onto
		// the stack, and registers 0 - 7 and optionally PC to be popped off the stack.

		// In thumb mode, stack grows downwards, hence push means SP is decremented and vice versa for pop
		// Based on the examples in http://problemkaputt.de/gbatek-thumb-opcodes-memory-multiple-load-store-push-pop-and-ldm-stm.htm
		// The usage is as follows:
		// PUSH {R0-R4,LR} :
		// Store R0,R1,R2,R3,R4 and R14 (LR) at the stack pointed to by R13(SP) and update R13.
		// Useful at start of a sub - routine to save workspace and RETURN address.
		// POP {R2,R6,PC} :
		// Load R2,R6 and R15 (PC) from the stack pointed to by R13(SP) and update R13.
		// Useful to restore workspace and RETURN from sub - routine.

		// As mentioned in http://problemkaputt.de/gbatek-thumb-opcodes-memory-multiple-load-store-push-pop-and-ldm-stm.htm
		// Note: When calling to a sub - routine, the RETURN address is stored in LR register, when calling further sub - routines, PUSH{ LR } must be used to save higher RETURN address on stack.
		// If so, POP{ PC } can be later used to RETURN from the sub - routine.

		// So, LR is stored in Higher Address!

		// register list is zero and LR/PC bit is also not set.. i.e. register list is completely empty
		// Refer to http://problemkaputt.de/gbatek-thumb-opcodes-memory-multiple-load-store-push-pop-and-ldm-stm.htm
		// Empty Rlist : R15 loaded / stored(ARMv4 only), and Rb = Rb + 40h(ARMv4 - v5)
		if (registerList == ZERO && r == RESET)
		{
			CPUWARN("Empty Register List for Push Pop Registers");
			if (l == YES)
			{
				// POP to PC from [SP]

				// Get SP
				auto sp = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP);
				// Get [SP]
				auto popedData = readRawMemory<GBA_WORD>(sp, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
				// Set [SP] to PC
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), popedData);
				// Increment SP (but this should behave as though all registers were poped, so increment 16*4 times)
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), sp + 0x40);
			}
			else
			{
				// PUSH PC to [SP]

				// Get SP
				auto sp = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP);
				// Decrement SP (but this should behave as though all registers were poped, so decrement 16*4 times)
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), sp - 0x40);
				// Get PC (PC pointing towards fetch stage ?)
				auto pc = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
				// write PC to [SP]
				writeRawMemory<GBA_WORD>(sp, pc, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
			}
		}
		else
		{
			// POP
			if (l == YES)
			{
				// get the current SP
				uint32_t baseAddr = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP);

				for (uint8_t rt = (uint8_t)REGISTER_TYPE::RT_0; rt <= (uint8_t)REGISTER_TYPE::RT_7; rt++) // looping from lower to higher registers
				{
					if ((registerList >> rt) & 0x01) // if "rt" is part of register list
					{
						// get [SP]
						GBA_WORD memoryToRegister = readRawMemory<GBA_WORD>(baseAddr, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
						// set [SP] to rt
						cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rt, getARMState(), memoryToRegister);
						// increment sp
						baseAddr += sizeof(GBA_WORD);
					}
				}

				// NOTE: The weird code if else code snippet below (which could have been optimized but is not) is to handle internal cycles
				// Handle PC
				if (r == SET)
				{
					GBA_WORD memoryToRegister = readRawMemory<GBA_WORD>(baseAddr, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
					// update SP
					cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP, getARMState(), baseAddr + sizeof(GBA_WORD));
					cpuIdleCycles();
					// update PC
					cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), memoryToRegister);
				}
				else
				{
					cpuIdleCycles();
					// update SP
					cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP, getARMState(), baseAddr);
				}
			}
			// PUSH
			else
			{
				uint32_t numberOfRegisters;

#if defined(_MSC_VER)
				numberOfRegisters = __popcnt(registerList); // for 32-bit
#elif defined(__GNUC__) || defined(__clang__) || defined(__EMSCRIPTEN__)
				numberOfRegisters = __builtin_popcountll(registerList); // for 64-bit
#else
				numberOfRegisters = countSetBits(registerList);
#endif

				// get the lowest SP value as we are storing registers in incremental order
				auto sp = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP);
				uint32_t baseAddr = sp - (sizeof(GBA_WORD) * numberOfRegisters);

				// Handle LR
				if (r == SET)
				{
					baseAddr -= sizeof(GBA_WORD);
				}

				// update sp with new baseAddr
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP, getARMState(), baseAddr);

				for (uint8_t rt = (uint8_t)REGISTER_TYPE::RT_0; rt <= (uint8_t)REGISTER_TYPE::RT_7; rt++) // looping from lower to higher registers
				{
					if ((registerList >> rt) & 0x01) // if "rt" is part of register list
					{
						GBA_WORD registerToMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rt);
						writeRawMemory<GBA_WORD>(baseAddr, registerToMemory, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
						baseAddr += sizeof(GBA_WORD);
					}
				}

				// Handle LR
				if (r == SET)
				{
					GBA_WORD registerToMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)LR);
					writeRawMemory<GBA_WORD>(baseAddr, registerToMemory, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
				}
			}
		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::LoadStoreHalfword()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & LOAD_STORE_HALFWORD_MASK);
	if (strippedOpCode == LOAD_STORE_HALFWORD_INSTRUCTION)
	{
		uint32_t offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_HALFWORD.offset;
		uint32_t rb = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_HALFWORD.rb;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_HALFWORD.rd;
		bool l = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_HALFWORD.l;

		uint32_t address = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rb) + (offset << ONE);

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		if (l == YES)
		{
			GBA_WORD dataToBeWritten = readRawMemory<GBA_HALFWORD>(address, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU);
			// Refer "INFORMATION_001" for the reason to perform ROR below
			dataToBeWritten = performShiftOperation(
				NO
				, SHIFT_TYPE::ROR
				, ((address & ONE) << THREE)
				, dataToBeWritten
				, DISABLED
			);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);
			cpuIdleCycles();
		}
		else
		{
			GBA_WORD dataToBeWritten = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd);
			writeRawMemory<GBA_HALFWORD>(address, static_cast<GBA_HALFWORD>(dataToBeWritten), MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU);
		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::SPRelativeLoadStore()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & SP_RELATIVE_LOAD_STORE_MASK);
	if (strippedOpCode == SP_RELATIVE_LOAD_STORE_INSTRUCTION)
	{
		uint32_t offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.SP_RELATIVE_LOAD_STORE.word8;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.SP_RELATIVE_LOAD_STORE.rd;
		bool l = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.SP_RELATIVE_LOAD_STORE.l;

		auto sp = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP);
		uint32_t relativeAddr = sp + (offset << TWO);

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		if (l == YES)
		{
			GBA_WORD dataToBeWritten = readRawMemory<GBA_WORD>(relativeAddr, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);

			// Refer : http://problemkaputt.de/gbatek-arm-cpu-memory-alignments.htm
			dataToBeWritten = performShiftOperation(
				NO
				, SHIFT_TYPE::ROR
				, ((relativeAddr & THREE) << THREE)
				, dataToBeWritten
				, DISABLED
			);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);
			cpuIdleCycles();
		}
		else
		{
			GBA_WORD dataToBeWritten = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd);
			writeRawMemory<GBA_WORD>(relativeAddr, dataToBeWritten, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::LoadAddress()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & LOAD_ADDRESS_MASK);
	if (strippedOpCode == LOAD_ADDRESS_INSTRUCTION)
	{
		isThisTheInstruction = YES;

		GBA_HALFWORD rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_ADDRESS.rd;
		// uOffset should be 10-bit with [1:0] set to zero, so left shift (Refer to 5.12.1 of https://www.dwedit.org/files/ARM7TDMI.pdf)
		GBA_HALFWORD uOffset = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_ADDRESS.word8 << TWO);
		GBA_HALFWORD subOpCode = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_ADDRESS.spOrPc;

		if (subOpCode == RESET) // PC
		{
			GBA_WORD pcData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);

			// Refer to 5.12.1 of https ://www.dwedit.org/files/ARM7TDMI.pdf
			// By the time we come here, PC is already ahead by 4 from current instruction to fill the pipeline
			// On top of this, we need to force bit 1 to zero
			pcData &= 0xFFFFFFFD;
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), (pcData + uOffset));
		}
		// SP
		else
		{
			GBA_WORD spData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SP);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), (spData + uOffset));
		}

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::LoadStoreWithImmediateOffset()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & LOAD_STORE_IMMEDIATE_OFFSET_MASK);
	if (strippedOpCode == LOAD_STORE_IMMEDIATE_OFFSET_INSTRUCTION)
	{
		uint32_t offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_IO.offset;
		uint32_t rb = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_IO.rb;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_IO.rd;
		bool l = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_IO.l;
		bool b = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_IO.b;

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		offset = ((b == YES) ? offset : (offset << TWO));

		uint32_t address = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rb) + offset;

		if (l == YES)
		{
			if (b == YES)
			{
				GBA_WORD dataToBeWritten = readRawMemory<BYTE>(address, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);
				cpuIdleCycles();
			}
			else
			{
				GBA_WORD dataToBeWritten = readRawMemory<GBA_WORD>(address, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
				// Refer "INFORMATION_001" for the reason to perform ROR below
				dataToBeWritten = performShiftOperation(
					NO
					, SHIFT_TYPE::ROR
					, ((address & THREE) << THREE)
					, dataToBeWritten
					, DISABLED
				);
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);
				cpuIdleCycles();
			}
		}
		else
		{
			GBA_WORD dataToBeWritten = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd);
			if (b == YES)
			{
				writeRawMemory<BYTE>(address, static_cast<BYTE>(dataToBeWritten), MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
			}
			else
			{
				writeRawMemory<GBA_WORD>(address, dataToBeWritten, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
			}
		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::LoadStoreWithRegisterOffset()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & LOAD_STORE_REGISTER_OFFSET_MASK);
	if (strippedOpCode == LOAD_STORE_REGISTER_OFFSET_INSTRUCTION)
	{
		uint32_t ro = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_RO.ro;
		uint32_t rb = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_RO.rb;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_RO.rd;
		bool l = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_RO.l;
		bool b = (bool)pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_RO.b;

		uint32_t address = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rb) + cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)ro);

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		if (l == YES)
		{
			if (b == YES)
			{
				GBA_WORD dataToBeWritten = readRawMemory<BYTE>(address, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);
				cpuIdleCycles();
			}
			else
			{
				GBA_WORD dataToBeWritten = readRawMemory<GBA_WORD>(address, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
				// Refer "INFORMATION_001" for the reason to perform ROR below
				dataToBeWritten = performShiftOperation(
					NO
					, SHIFT_TYPE::ROR
					, ((address & THREE) << THREE)
					, dataToBeWritten
					, DISABLED
				);
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);
				cpuIdleCycles();
			}
		}
		else
		{
			GBA_WORD dataToBeWritten = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd);
			if (b == YES)
			{
				writeRawMemory<BYTE>(address, static_cast<BYTE>(dataToBeWritten), MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
			}
			else
			{
				writeRawMemory<GBA_WORD>(address, dataToBeWritten, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
			}
		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::LoadStoreSignExtendedByteHalfword()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & LOAD_STORE_SIGN_EXT_BYTE_HALFWORD_MASK);
	if (strippedOpCode == LOAD_STORE_SIGN_EXT_BYTE_HALFWORD_INSTRUCTION)
	{
		uint32_t ro = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_SIGNED_BYTE_HALFWORD.ro;
		uint32_t rb = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_SIGNED_BYTE_HALFWORD.rb;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_SIGNED_BYTE_HALFWORD.rd;
		uint32_t opcode = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.LOAD_STORE_SIGNED_BYTE_HALFWORD.opcode;

		uint32_t address = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rb) + cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)ro);

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		switch (opcode)
		{
		case ZERO:
		{
			GBA_HALFWORD dataToBeWritten = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd);
			writeRawMemory<GBA_HALFWORD>(address, (GBA_HALFWORD)dataToBeWritten, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU);
			BREAK;
		}
		case ONE:
		{
			GBA_WORD dataToBeWritten = readRawMemory<BYTE>(address, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
			dataToBeWritten = signExtend32(dataToBeWritten, EIGHT);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);
			cpuIdleCycles();
			BREAK;
		}
		case TWO:
		{
			GBA_WORD dataToBeWritten = readRawMemory<GBA_HALFWORD>(address, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU);
			// Refer "INFORMATION_001" for the reason to perform ROR below
			dataToBeWritten = performShiftOperation(
				NO
				, SHIFT_TYPE::ROR
				, ((address & ONE) << THREE)
				, dataToBeWritten
				, DISABLED
			);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);
			cpuIdleCycles();
			BREAK;
		}
		case THREE:
		{
			GBA_WORD dataToBeWritten = ZERO;
			// If address is halfword aligned, then bit 15 is sign bit, so [sssssssssssssssssddddddddddddddd] where s is sign bit and d is valid data 
			if ((address & 0x01) == ZERO)
			{
				dataToBeWritten = readRawMemory<GBA_HALFWORD>(address, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU);
				dataToBeWritten = signExtend32(dataToBeWritten, SIXTEEN);
			}
			// If address is not halfword aligned, then bit 7 is sign bit, so [sssssssssssssssssssssssssddddddd] where s is sign bit and d is valid data
			// Refer "INFORMATION_001" (especially the part which talks about signed data) for the reason why bit 7 is sign extended instead of bit 15 when the address is not 16 bit aligned
			else
			{
				dataToBeWritten = readRawMemory<BYTE>(address, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
				dataToBeWritten = signExtend32(dataToBeWritten, EIGHT);
			}
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);
			cpuIdleCycles();
			BREAK;
		}
		default:
		{
			FATAL("Load Store Sign Extended Byte / Halfword; Unknown Sub-Opcode");
		}
		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::PCRelativeLoad()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & PC_RELATIVE_LOAD_MASK);
	if (strippedOpCode == PC_RELATIVE_LOAD_INSTRUCTION)
	{
		uint32_t offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.PC_RELATIVE_LOAD.word8;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.PC_RELATIVE_LOAD.rd;

		auto pc = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);

		// Refer to 5.12.1 of https ://www.dwedit.org/files/ARM7TDMI.pdf
		// By the time we come here, PC is already ahead by 4 from current instruction to fill the pipeline
		// On top of this, we need to force bit 1 to zero

		pc &= 0xFFFFFFFD;
		uint32_t relativeAddr = pc + (offset << TWO);

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		uint32_t dataToBeWritten = readRawMemory<GBA_WORD>(relativeAddr, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
		cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), dataToBeWritten);

		cpuIdleCycles();

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::HiRegisterOperationsBranchExchange()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & HI_REGISTER_OP_OR_BRANCH_EXCHANGE_MASK);
	if (strippedOpCode == HI_REGISTER_OP_OR_BRANCH_EXCHANGE_INSTRUCTION)
	{
		isThisTheInstruction = YES;

		GBA_HALFWORD rdhd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.HIGH_REGISTER_OPERATIONS.rdhd;
		GBA_HALFWORD rshs = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.HIGH_REGISTER_OPERATIONS.rshs;
		GBA_HALFWORD isHd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.HIGH_REGISTER_OPERATIONS.h1;
		GBA_HALFWORD isHs = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.HIGH_REGISTER_OPERATIONS.h2;
		GBA_HALFWORD subOpCode = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.HIGH_REGISTER_OPERATIONS.opcode;

		if ((isHs == RESET && isHd == RESET) && ((subOpCode == 0x00) || (subOpCode == 0x01) || (subOpCode == 0x02)))
		{
			WARN("Hi Register OP / Branch Exchange; isHs == 0 && isHd == 0 when subOpCode == ADD or CMP or MOV");
		}

		switch (subOpCode)
		{
		case 0x00: // ADD
		{
			GBA_WORD actualRd = ((isHd == ONE) ? (rdhd + EIGHT) : (rdhd));
			GBA_WORD actualRs = ((isHs == ONE) ? (rshs + EIGHT) : (rshs));

			GBA_WORD rdData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)actualRd);
			GBA_WORD rsData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)actualRs);

			GBA_WORD subOpCodeResult = rdData + rsData;

			if (actualRd == PC)
			{
				// Retaining thumb mode (so keep LSB set)
				subOpCodeResult |= ONE;
			}

			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)actualRd, getARMState(), subOpCodeResult);

			if (actualRd != PC) // if "actualRd == PC", then pipeline flush would have occured and new PC would have been set
			{
				// Increment the PC
				pGBA_cpuInstance->registers.pc += TWO;
				pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;
			}

			BREAK;
		}
		case 0x01: // CMP
		{
			GBA_WORD actualRd = ((isHd == ONE) ? (rdhd + EIGHT) : (rdhd));
			GBA_WORD actualRs = ((isHs == ONE) ? (rshs + EIGHT) : (rshs));

			GBA_WORD rdData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)actualRd);
			GBA_WORD rsData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)actualRs);

			GBA_WORD subOpCodeResult = rdData - rsData;

			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((rdData >= rsData) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((rdData ^ rsData) & (rdData ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);

			// Increment the PC
			pGBA_cpuInstance->registers.pc += TWO;
			pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

			BREAK;
		}
		case 0x02: // MOV
		{
			GBA_WORD actualRd = ((isHd == ONE) ? (rdhd + EIGHT) : (rdhd));
			GBA_WORD actualRs = ((isHs == ONE) ? (rshs + EIGHT) : (rshs));

			GBA_WORD rsData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)actualRs);

			if (actualRd == PC)
			{
				// Retaining thumb mode (so keep LSB set)
				rsData |= ONE;
			}

			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)actualRd, getARMState(), rsData);

			if (actualRd != PC) // if "actualRd == PC", then pipeline flush would have occured and new PC would have been set
			{
				// Increment the PC
				pGBA_cpuInstance->registers.pc += TWO;
				pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;
			}

			BREAK;
		}
		case 0x03: // BX
		{
			if (isHd != RESET)
			{
				FATAL("isHd != 0 when subOpCode == BX");
			}

			GBA_WORD actualRs = ((isHs == ONE) ? (rshs + EIGHT) : (rshs));
			GBA_WORD rsData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)actualRs);

			if (GETBIT(ZERO, rsData) == RESET)
			{
				CPUEVENT("[THUMB] -> [ARM]");

				setARMState(STATE_TYPE::ST_ARM);
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), rsData);
			}
			// If bit 0 is 1, then we remain in thumb
			else
			{
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), rsData);
			}

			BREAK;
		}
		default:
		{
			FATAL("Hi Reg Operation / Branch Exchange; Unknown Sub-Opcode");
		}
		}
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::ALUOperations()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & ALU_OPERATIONS_MASK);
	if (strippedOpCode == ALU_OPERATIONS_INSTRUCTION)
	{
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ALU_OPERATIONS.rd;
		uint32_t rs = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ALU_OPERATIONS.rs;
		uint32_t subOpCode = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ALU_OPERATIONS.opcode;

		GBA_WORD op1 = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd);
		GBA_WORD op2 = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rs);
		GBA_WORD subOpCodeResult = ZERO;

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		GBA_WORD cOriginal = pGBA_registers->cpsr.psrFields.psrCarryBorrowExtBit;

		switch ((ALU_SUBCODES)subOpCode)
		{
		case ALU_SUBCODES::ALU_AND:
		{
			subOpCodeResult = op1 & op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_XOR:
		{
			subOpCodeResult = op1 ^ op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_LSL:
		{
			cpuIdleCycles();
			subOpCodeResult = performShiftOperation(
				YES
				, SHIFT_TYPE::LSL
				, op2
				, op1
				, DISABLED
			);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_LSR:
		{
			cpuIdleCycles();
			subOpCodeResult = performShiftOperation(
				YES
				, SHIFT_TYPE::LSR
				, op2
				, op1
				, DISABLED
			);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_ASR:
		{
			cpuIdleCycles();
			subOpCodeResult = performShiftOperation(
				YES
				, SHIFT_TYPE::ASR
				, op2
				, op1
				, DISABLED
			);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_ADC:
		{
			uint64_t op2c = (uint64_t)op2 + (uint64_t)cOriginal;
			uint64_t subOpCodeResult64 = (uint64_t)op1 + (uint64_t)op2c;
			subOpCodeResult = (GBA_WORD)subOpCodeResult64;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((subOpCodeResult64 > 0xFFFFFFFF) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op2 ^ subOpCodeResult) & (~(op1 ^ op2))) >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_SBC:
		{
			uint64_t temp = (uint64_t)op2 + (uint64_t)(cOriginal == ONE ? ZERO : ONE);
			uint64_t result64 = (uint64_t)op1 - temp;
			subOpCodeResult = (uint32_t)result64;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((op1 >= temp) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op1 ^ op2) & (op1 ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_ROR:
		{
			cpuIdleCycles();
			subOpCodeResult = performShiftOperation(
				YES
				, SHIFT_TYPE::ROR
				, op2
				, op1
				, DISABLED
			);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_TST:
		{
			subOpCodeResult = op1 & op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			BREAK;
		}
		case ALU_SUBCODES::ALU_NEG:
		{
			subOpCodeResult = ZERO - op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((op1 >= op2) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op1 ^ op2) & (op1 ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_CMP:
		{
			subOpCodeResult = op1 - op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((op1 >= op2) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op1 ^ op2) & (op1 ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);
			BREAK;
		}
		case ALU_SUBCODES::ALU_CMN:
		{
			subOpCodeResult = op1 + op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			uint64_t result = (uint64_t)op1 + (uint64_t)op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((result > 0xFFFFFFFF) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op2 ^ subOpCodeResult) & (~(op1 ^ op2))) >> THIRTYONE) ? ONE : ZERO);
			BREAK;
		}
		case ALU_SUBCODES::ALU_ORR:
		{
			subOpCodeResult = op1 | op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_MUL:
		{
			// op1 -> dest
			// op2 -> src

			bool full = TickMultiply(YES, op1);
			subOpCodeResult = op1 * op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			// Carry flag logic
			if (full)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = MultiplyCarrySimple(op1) ? ONE : ZERO;
			}
			else
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = MultiplyCarryLo(op2, op1) ? ONE : ZERO;
			}
			BREAK;
		}
		case ALU_SUBCODES::ALU_BIC:
		{
			subOpCodeResult = op1 & (~op2);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		case ALU_SUBCODES::ALU_MVN:
		{
			subOpCodeResult = ~op2;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);
			BREAK;
		}
		default:
		{
			FATAL("ALU Operation; Unknown Sub-Opcode");
		}
		}

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::MoveCompareAddSubtractImmediate()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & MOV_CMP_ADD_SUB_IMMEDIATE_MASK);
	if (strippedOpCode == MOV_CMP_ADD_SUB_IMMEDIATE_INSTRUCTION)
	{
		isThisTheInstruction = YES;

		GBA_HALFWORD rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MOVE_COMPARE_ADD_SUBTRACT_IMMEDIATE.rd;
		GBA_HALFWORD uOffset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MOVE_COMPARE_ADD_SUBTRACT_IMMEDIATE.offset;
		GBA_HALFWORD subOpCode = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MOVE_COMPARE_ADD_SUBTRACT_IMMEDIATE.opcode;

		switch (subOpCode)
		{
		case 0x00: // MOV
		{
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), uOffset);

			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((uOffset == ZERO) ? ONE : ZERO);

			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((((GBA_WORD)uOffset) >> THIRTYONE) ? ONE : ZERO);

			BREAK;
		}
		case 0x01: // CMP
		{
			GBA_WORD rdData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd);
			GBA_WORD subOpCodeResult = rdData - uOffset;

			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((rdData >= uOffset) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((rdData ^ uOffset) & (rdData ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);

			BREAK;
		}
		case 0x02: // ADD
		{
			GBA_WORD rdData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd);
			GBA_WORD subOpCodeResult = rdData + uOffset;

			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			uint64_t result = (uint64_t)rdData + (uint64_t)uOffset;
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((result > 0xFFFFFFFF) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((uOffset ^ subOpCodeResult) & (~(rdData ^ uOffset))) >> THIRTYONE) ? ONE : ZERO);

			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);

			BREAK;
		}
		case 0x03: // SUB
		{
			GBA_WORD rdData = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd);
			GBA_WORD subOpCodeResult = rdData - uOffset;

			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((rdData >= uOffset) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((rdData ^ uOffset) & (rdData ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);

			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), subOpCodeResult);

			BREAK;
		}
		default:
		{
			FATAL("Move Compare Add Subtract Immediate; Unknown Sub-Opcode");
		}
		}

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::AddSubtract()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & ADD_SUBTRACT_MASK);
	if (strippedOpCode == ADD_SUBTRACT_INSTRUCTION)
	{
		uint32_t subOpcode = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ADD_SUBTRACT.op;
		uint32_t rn_or_offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ADD_SUBTRACT.rn_or_offset;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ADD_SUBTRACT.rd;
		uint32_t rs = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ADD_SUBTRACT.rs;

		bool i = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.ADD_SUBTRACT.i);

		uint32_t op1 = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rs));

		uint32_t op2 = ZERO;
		if (i == YES)
		{
			op2 = rn_or_offset;
		}
		else
		{
			op2 = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rn_or_offset));
		}

		uint64_t result = ZERO;

		switch (subOpcode)
		{
		case ZERO: // ADD
		{
			result = (uint64_t)op1 + (uint64_t)op2;

			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((result > 0xFFFFFFFF) ? ONE : ZERO);

			result = (uint32_t)result; // cast it back to 32 bit once we are done checking for carry flag

			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((result == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((result >> THIRTYONE) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op2 ^ ((uint32_t)result)) & (~(op1 ^ op2))) >> THIRTYONE) ? ONE : ZERO);

			BREAK;
		}
		case ONE: // SUB
		{
			result = op1 - op2;

			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((result == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((result >> THIRTYONE) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((op1 >= op2) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op1 ^ op2) & (op1 ^ ((uint32_t)result))) >> THIRTYONE) ? ONE : ZERO);

			BREAK;
		}
		default:
		{
			FATAL("Add Subtract; Unknown Sub-Opcode");
		}
		}

		cpuSetRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rd), getARMState(), (uint32_t)result);

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::MoveShiftedRegister()
{
	bool isThisTheInstruction = NO;

	uint16_t strippedOpCode = ((static_cast<GBA_HALFWORD>(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode)) & MOVE_SHIFTED_REGISTER_MASK);
	if (strippedOpCode == MOVE_SHIFTED_REGISTER_INSTRUCTION)
	{
		uint32_t subOpcode = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MOVE_SHIFTED_REGISTER.opcode;
		uint32_t offset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MOVE_SHIFTED_REGISTER.offset;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MOVE_SHIFTED_REGISTER.rd;
		uint32_t rs = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.thumb.MOVE_SHIFTED_REGISTER.rs;

		GBA_WORD dataAfterShifting = ZERO;
		SHIFT_TYPE shiftType = SHIFT_TYPE::LSL;

		switch (subOpcode)
		{
		case ZERO:
		{
			shiftType = SHIFT_TYPE::LSL;
			BREAK;
		}
		case ONE:
		{
			shiftType = SHIFT_TYPE::LSR;
			BREAK;
		}
		case TWO:
		{
			shiftType = SHIFT_TYPE::ASR;
			BREAK;
		}
		default:
		{
			FATAL("Move Shifted Register; Unknown Sub-Opcode");
		}
		}

		dataAfterShifting = performShiftOperation(
			YES
			, shiftType
			, offset
			, cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rs))
			, ENABLED
		);

		pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((dataAfterShifting == ZERO) ? ONE : ZERO);
		pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((dataAfterShifting >> THIRTYONE) ? ONE : ZERO);

		cpuSetRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rd), getARMState(), dataAfterShifting);

		// Increment the PC
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		isThisTheInstruction = YES;
	}

	RETURN isThisTheInstruction;
}

// ARM Instructions
bool GBA_t::BranchAndBranchExchange()
{
	bool isThisTheInstruction = NO;

	// Branch and Branch Exchange
	uint32_t strippedOpCode = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & BRANCH_AND_BRANCH_EXCHANGE_MASK);
	if (strippedOpCode == BRANCH_AND_BRANCH_EXCHANGE_INSTRUCTION)
	{
		isThisTheInstruction = YES;

		uint32_t rn = 0x00;
		uint32_t newPC = 0x00;
		if ((pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & 0xF0) == 0x10)
		{
			rn = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BRANCH_EXCHANGE.rn;
			newPC = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rn));

			// check if we need to transition to thumb state
			bool isThumb = (bool)(newPC & 0x01);

			if (isThumb)
			{
				CPUEVENT("[ARM] -> [THUMB]");
				setARMState(STATE_TYPE::ST_THUMB);
				newPC &= 0xFFFFFFFE;
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), newPC);
			}
			else
			{
				setARMState(STATE_TYPE::ST_ARM);
				newPC &= 0xFFFFFFFC;
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), newPC);
			}
		}
		else if ((pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & 0xF0) == 0x20)
		{
			FATAL("Branch and Branch Exchange; TBD");
		}
		else if ((pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & 0xF0) == 0x30)
		{
			FATAL("Branch and Branch Exchange; TBD");
		}
		else
		{
			FATAL("Branch and Branch Exchange; TBD");
		}
	}
	RETURN isThisTheInstruction;
}

bool GBA_t::BlockDataTransfer()
{
	bool isThisTheInstruction = NO;

	// Block Data Transfer
	uint32_t strippedOpCode = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & BLOCK_DATA_TRANSFER_MASK);
	if (strippedOpCode == BLOCK_DATA_TRANSFER_INSTRUCTION)
	{
		isThisTheInstruction = YES;
		OP_MODE_TYPE originalOpMode = getARMMode(); // when S bit is set; keep a copy of the original operating mode so that we can revert to it once the opcode processing is done

		uint32_t rn = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BLOCK_DATA_TRANSFER.rn;
		uint32_t old_base = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rn));
		uint32_t address = old_base;	// "address" will be properly reinitialized w.r.t "u" field and will also be used as incrementor
		uint32_t new_base = old_base;

		bool p = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BLOCK_DATA_TRANSFER.p);
		bool u = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BLOCK_DATA_TRANSFER.u);
		bool s = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BLOCK_DATA_TRANSFER.s);
		bool w = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BLOCK_DATA_TRANSFER.w);

		// Refer 4.11.5 of https://www.dwedit.org/files/ARM7TDMI.pdf
		if (rn == PC)
		{
			// "gang-ldmstm.gba" uses this condition, hence 'FATAL' is replaced with 'CPUWARN'
			CPUWARN(" Block Data Transfer; rn = PC");
		}

		uint32_t registerList = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BLOCK_DATA_TRANSFER.rlist;
		uint32_t numberOfRegisters = ZERO;

#if defined(_MSC_VER)
		numberOfRegisters = __popcnt(registerList);  // MSVC intrinsic for 32-bit
#elif defined(__GNUC__) || defined(__clang__) || defined(__EMSCRIPTEN__)
		numberOfRegisters = __builtin_popcount(registerList);  // GCC/Clang/Emscripten
#else
		// Optional fallback if you're targeting unknown compilers
		numberOfRegisters = countSetBits(registerList);
#endif

		if (registerList == ZERO)
		{
			CPUWARN("Block Data Transfer but register list is empty");

			// Refer: http://problemkaputt.de/gbatek.htm#armopcodesmemoryblockdatatransferldmstm

			// Strange Effects on Invalid Rlist's
			// Empty Rlist : R15 loaded / stored(ARMv4 only), and Rb = Rb + / -40h(ARMv4 - v5).
			// Writeback with Rb included in Rlist : Store OLD base if Rb is FIRST entry in Rlist, otherwise store NEW base(STM / ARMv4), 
			// always store OLD base(STM / ARMv5), no writeback(LDM / ARMv4), 
			// writeback if Rb is "the ONLY register, or NOT the LAST register" in Rlist(LDM / ARMv5).

			// Only PC is loaded or stored as mentioned above
			registerList = (1 << PC);

			// As mentioned above [base register] = [base register] + / -40h(ARMv4 - v5) (40h = 64 i.e. all 16 registers)
			// i.e. [base register] = [base register] +/- (4 * 16)
#if defined(_MSC_VER)
			numberOfRegisters = __popcnt(0xFFFF); // MSVC intrinsic for 32-bit
#elif defined(__GNUC__) || defined(__clang__) || defined(__EMSCRIPTEN__)
			numberOfRegisters = __builtin_popcount(0xFFFF); // GCC/Clang/Emscripten
#else
			numberOfRegisters = countSetBits(0xFFFF); // Portable fallback
#endif
		}

		if (u == YES) // move up the memory from base address
		{
			; // do nothing to "address"
			new_base = address + (FOUR * numberOfRegisters);
		}
		else // move down the memory from base address
		{
			// Few Notes (verified):
			// 1) Final address is same irrespective of Post-decrement or Pre-decrement
			// TWO) Since we are going in reverse, Pre-decrement results to Post-decrement and vice-versa
			// 3) Since we are going in reverse, the first write-back address represents the final write-back address

			address -= (FOUR * numberOfRegisters); // updated initial address
			new_base = address;
			p = !p;	// invert the "p" flag (because of point 2 mentioned above)
		}

		int32_t nonZeroIfPreIncrement = ZERO;
		int32_t nonZeroIfPostIncrement = ZERO;
		if (p == YES) // pre-increment
		{
			nonZeroIfPreIncrement = FOUR; // 4 bytes increment per register read/write
			nonZeroIfPostIncrement = ZERO;
		}
		else // post-increment
		{
			nonZeroIfPreIncrement = ZERO;
			nonZeroIfPostIncrement = FOUR; // 4 bytes increment per register read/write
		}

		// Increment the PC
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		if (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BLOCK_DATA_TRANSFER.l)
		{
			cpuIdleCycles();

			// Refer 4.11.4 of https://www.dwedit.org/files/ARM7TDMI.pdf (3rd condition)
			// R15 not in list and S bit set (User bank transfer)
			// For both LDM and STM instructions, the User bank registers are transferred rather than the register bank corresponding to the current mode.
			// Hence, we switch to USR mode just before the register loading begins
			if (s == YES && (((registerList >> PC) & 0x01) == ZERO))
			{
				setARMMode(OP_MODE_TYPE::OP_USR);
			}

			uint32_t memory2register = ZERO;
			bool firstTransfer = YES;

			for (uint8_t rt = (uint8_t)REGISTER_TYPE::RT_0; rt <= (uint8_t)REGISTER_TYPE::RT_15; rt++) // looping from lower to higher registers
			{
				if ((registerList >> rt) & 0x01) // if "rt" is part of register list
				{
					address += nonZeroIfPreIncrement;

					// NOTE: Writebacks happens at 2nd cycles of the instruction
					// Even before the first loop is completed, if write back is enabled, base register is updated
					// So, if rb is part of the list, then in further loops, rb will be updated and hence the writeback is overriden
					if (w == YES && firstTransfer == YES)
					{
						CPUTODO("Find proper source for code snippet at line %d in file %s", __LINE__, __FILE__);
						cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rn, getARMState(), new_base);
					}

					// read from memory
					memory2register = readRawMemory<GBA_WORD>(address, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);

					// write to register
					cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rt, getARMState(), memory2register);

					address += nonZeroIfPostIncrement;

					firstTransfer = NO;
				}
			}
		}
		else
		{
			// Refer 4.11.4 of https://www.dwedit.org/files/ARM7TDMI.pdf (2nd and 3rd condition)
			// STM with R15 in transfer list and S bit set (User bank transfer)
			// The registers transferred are taken from the User bank rather than the bank corresponding to the current mode.
			// R15 not in list and S bit set (User bank transfer)
			// For both LDM and STM instructions, the User bank registers are transferred rather than the register bank corresponding to the current mode.
			// Hence, we switch to USR mode just before the register transfer begins
			if (s == YES)
			{
				setARMMode(OP_MODE_TYPE::OP_USR);
			}

			uint32_t register2memory = ZERO;
			bool firstTransfer = YES;

			for (uint8_t rt = (uint8_t)REGISTER_TYPE::RT_0; rt <= (uint8_t)REGISTER_TYPE::RT_15; rt++)	// looping from lower to higher registers
			{
				if ((registerList >> rt) & 0x01) // if "rt" is part of register list
				{
					if (rt == PC && s == YES)
					{
						CPUWARN("Base write-back should not be used (STM with R15 in transfer list and S bit set)");
					}

					address += nonZeroIfPreIncrement;

					// read from register
					register2memory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rt);

					// write to memory
					writeRawMemory<GBA_WORD>(address, register2memory, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
					address += nonZeroIfPostIncrement;

					// NOTE:
					// Also mentioned in ARM DOC
					// Writing to memory before the writeback will handle the condition mentioned in 4.11.6 of https://www.dwedit.org/files/ARM7TDMI.pdf

					// To handle the above mentioned note, the sequence inside the for loop is as follows:
					// Reason being, if rb is first in list, then its not modified during the first loop
					// And write back is done before writeRawMemory, so old [rb] will be written to memory
					// then write back happens to rb with updated value, now if rb is part of list
					// then from second loop onwards cpuReadRegister of rb gives new value
					if (w == YES && firstTransfer == YES)
					{
						cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rn, getARMState(), new_base);
					}

					firstTransfer = NO;
				}
			}
		}

		if (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BLOCK_DATA_TRANSFER.l)
		{
			cpuIdleCycles();

			// Refer 4.11.4 of https://www.dwedit.org/files/ARM7TDMI.pdf (1st condition)
			// LDM with R15 in transfer list and S bit set (Mode changes)
			// If the instruction is a LDM then SPSR_<mode> is transferred to CPSR at the same time as R15 is loaded.
			// At this point, all registers including R15 is loaded, so we can transfer SPSR_<mode> to CPSR
			if ((s == YES) && (((registerList >> PC) & 0x01) == ONE))
			{
				psr_t spsr;
				spsr.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SPSR);
				setARMMode((OP_MODE_TYPE)spsr.psrFields.psrModeBits); // NOTE: Need to change the mode to whatever SPSR is indicating before CPSR is modified (else, we go to UNKNOWN ARM Mode)
				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR, getARMState(), spsr.psrMemory);
			}
		}

		// After conditions mentioned in 4.11.4 of https://www.dwedit.org/files/ARM7TDMI.pdf is met, we revert back to original mode
		if (s == YES)
		{
			setARMMode(originalOpMode); // revert back to original mode!
		}
	}
	RETURN isThisTheInstruction;
}

bool GBA_t::BranchAndBranchLink()
{
	bool isThisTheInstruction = NO;

	// Branch and Branch with Link
	uint32_t strippedOpCode = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & BRANCH_AND_BRANCH_WITH_LINK_MASK);
	if (strippedOpCode == BRANCH_INSTRUCTION || strippedOpCode == BRANCH_WITH_LINK_INSTRUCTION)
	{
		isThisTheInstruction = YES;
		bool isNegative = NO;
		uint32_t uOffset = ZERO;
		int32_t iOffset = ZERO;

		uOffset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BRANCH.offset;
		isNegative = (((uOffset & 0x800000) > ZERO) ? YES : NO);	// if offset is a 24-bit signed value

		if (isNegative)
		{
			// decode the 2's complement value (automatically produces sign extended value because we do ~uOffset)
			uOffset = ((~uOffset + ONE) & 0xFFFFFF);
		}
		uOffset <<= TWO; // left shift by 2

		iOffset = isNegative ? -ONE * (int32_t)uOffset : (int32_t)uOffset;

		if (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.BRANCH.link)
		{
			// PC points to fetch stage, i.e PC+8 after BL instruction
			// we have to save the immediate next instruction to BL, hence we store PC-4
			cpuSetRegister(getRegisterBankFromOperatingMode(getARMMode()), (REGISTER_TYPE)LR, getARMState(), pGBA_cpuInstance->registers.pc - FOUR);
		}

		auto pc = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
		pc += (iOffset);
		pc &= 0xFFFFFFFC;
		cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), pc);
	}
	RETURN isThisTheInstruction;
}

bool GBA_t::SoftwareInterrupt()
{
	bool isThisTheInstruction = NO;

	// Software Interrupt
	uint32_t strippedOpCode = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & SOFTWARE_INTERRUPT_MASK);
	if (strippedOpCode == SOFTWARE_INTERRUPT_INSTRUCTION)
	{
		isThisTheInstruction = YES;
		if (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SOFTWARE_INTERRUPT.opcode == 0x0F)
		{
			// Need to go to supervisor mode
			// Save the pc in lr_svc
			// Save the cpsr contents to svc_spsr
			psr_t currentCPSR;
			currentCPSR.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR);

			// Now, since the cpsr is saved in svc_spsr, modify the current status of cpsr
			setARMMode(OP_MODE_TYPE::OP_SVC);
			setARMState(STATE_TYPE::ST_ARM);
			// Disable the irq
			pGBA_registers->cpsr.psrFields.psrIRQDisBit = ONE;

			// PC points to fetch stage, i.e PC+8 after SWI instruction
			// We have to save the immediate next instruction to BL, hence we store PC-4 in ARM mode
			uint32_t pcToStoreInLR = pGBA_cpuInstance->registers.pc - FOUR;
			cpuSetRegister(REGISTER_BANK_TYPE::RB_SVC, (REGISTER_TYPE)LR, getARMState(), pcToStoreInLR);
			// Save SPSR as we will change ARM Mode
			cpuSetRegister(REGISTER_BANK_TYPE::RB_SVC, (REGISTER_TYPE)SPSR, getARMState(), currentCPSR.psrMemory);

			// Display the SWI comment
			uint32_t comment = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SOFTWARE_INTERRUPT.comment >> SIXTEEN;
			CPUEVENT("Adjusted PC: 0x%08X: SWI: 0x%2X [%s]", pcToStoreInLR, comment, SWI_NAMES[comment].c_str());

			// Update the PC
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), SOFTWARE_INTERRUPT_SWI_SVC_HANDLER);
		}
		else if (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SOFTWARE_INTERRUPT.opcode == 0x01)
		{
			FATAL(" Software Interrupt; TBD");
		}
		else
		{
			FATAL(" Software Interrupt; Unknown Sub-Opcode")
		}
	}
	RETURN isThisTheInstruction;
}

bool GBA_t::Undefined()
{
	bool isThisTheInstruction = NO;

	// Undefined
	uint32_t strippedOpCode = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & UNDEFINED_MASK);
	if (strippedOpCode == UNDEFINED_INSTRUCTION)
	{
		isThisTheInstruction = YES;

		FATAL("Undefined");

		// Increment the PC
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		cpuIdleCycles();
	}
	RETURN isThisTheInstruction;
}

bool GBA_t::SingleDataTransfer()
{
	bool isThisTheInstruction = NO;

	// Single Data Transfer : http://problemkaputt.de/gbatek.htm#armopcodesmemorysingleDataTransferldrstrpld
	uint32_t strippedOpCode = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & SINGLE_DATA_TRANSFER_MASK);
	if (strippedOpCode == SINGLE_DATA_TRANSFER_INSTRUCTION)
	{
		isThisTheInstruction = YES;
		uint32_t uOffset = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.offset;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.rd;
		uint32_t rn = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.rn;

		bool i = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.i);
		bool p = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.p);
		bool u = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.u);
		bool b = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.b);
		bool w = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.w);

		uint32_t address = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rn));

		if (w == YES && rn == PC)
		{
			FATAL("Single Data Transfer; w == 1 && rn == PC");
		}

		if (p == NO) // post-increment
		{
			// force w to one, otherwise post increment doesn't makes sense (http://problemkaputt.de/gbatek.htm#armopcodesmemorysingleDataTransferldrstrpld)
			w = YES;
		}

		typedef union {
			struct {
				uint16_t rm : FOUR;
				uint16_t reg_op : ONE;
				uint16_t shift_type : TWO;
				uint16_t shift_amount : FIVE;
			uint16_t: FOUR;
			};
			uint16_t raw;
		} offset_when_i_is_set_t;

		offset_when_i_is_set_t offset_when_i_is_set;
		int32_t uEffectiveOffset = ZERO;

		if (i == YES) // Shifted Register Mode
		{
			offset_when_i_is_set.raw = uOffset;

			if (offset_when_i_is_set.rm == PC)	// http://problemkaputt.de/gbatek.htm#armopcodesmemorysingleDataTransferldrstrpld
			{
				FATAL("Single Data Transfer; rm = PC");
			}

			if (offset_when_i_is_set.reg_op != ZERO)	// http://problemkaputt.de/gbatek.htm#armopcodesmemorysingleDataTransferldrstrpld
			{
				FATAL(" Single Data Transfee; reg_op != 0");
			}

			uEffectiveOffset = performShiftOperation(
				NO	// CPSR flags are not updated -> http://problemkaputt.de/gbatek.htm#armopcodesmemorysingleDataTransferldrstrpld
				, (SHIFT_TYPE)offset_when_i_is_set.shift_type
				, offset_when_i_is_set.shift_amount
				, cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)offset_when_i_is_set.rm))
				, ENABLED
			);
		}
		// Immediate Mode
		else
		{
			uEffectiveOffset = (int32_t)uOffset;
		}

		// Increment the PC
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		if (u == NO) // subtract from base, hence changing the sign
		{
			uEffectiveOffset = -uEffectiveOffset;
		}

		if (p == YES) // pre-increment
		{
			address += uEffectiveOffset; // adding offset before the memory read/write operation
		}

		if (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.l == RESET) // contents loaded from memory to registers
		{
			uint32_t register2memory = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rd));

			// NOTE: "rd == PC" case is already handled as PC is already incremented above. Refer to "Data Processing" for more info

			if (b == YES)
			{
				writeRawMemory<BYTE>(address, register2memory, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
			}
			else
			{
				writeRawMemory<GBA_WORD>(address, register2memory, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
			}
		}

		if (w == YES)
		{
			auto writeBackAddress = address;
			if (p == NO) // post-increment
			{
				writeBackAddress += uEffectiveOffset; // adding offset after the memory read/write operation
			}

			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rn, getARMState(), writeBackAddress);
		}

		if (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_TRANSFER.l == SET) // contents loaded from memory to registers
		{
			GBA_WORD memory2register = ZERO;

			if (b == YES)
			{
				memory2register = readRawMemory<BYTE>(address, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
			}
			else
			{
				memory2register = readRawMemory<GBA_WORD>(address, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);

				// Refer "INFORMATION_001" for the reason to perform ROR below
				memory2register = performShiftOperation(
					NO
					, SHIFT_TYPE::ROR
					, ((address & THREE) << THREE)
					, memory2register
					, DISABLED
				);
			}

			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), memory2register);

			cpuIdleCycles();
		}
	}
	RETURN isThisTheInstruction;
}

bool GBA_t::SingleDataSwap()
{
	bool isThisTheInstruction = NO;

	// Single Data Swap
	uint32_t strippedOpCode = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & SINGLE_DATA_SWAP_MASK);
	if (strippedOpCode == SINGLE_DATA_SWAP_INSTRUCTION)
	{
		isThisTheInstruction = YES;
		uint32_t rm = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_SWAP.rm;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_SWAP.rd;
		uint32_t rn = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.SINGLE_DATA_SWAP.rn;

		bool b = (bool)(GETBIT(TWENTYTWO, pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode));

		// Increment the PC
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		if (rn == PC || rd == PC || rm == PC)
		{
			FATAL(" Single Data Swap; rn == PC || rd == PC || rm == PC");
		}

		uint32_t swapAddress = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rn);
		uint32_t sourceRegisterContent = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rm);

		if (b == YES)
		{
			BYTE swapAddrContent = readRawMemory<BYTE>(swapAddress, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
			// Setting LOCK to YES (Source NBA)
			CPUTODO("Find a source for need of LOCK during memory write in SingleDataSwap at %d in %s", __LINE__, __FILE__);
			writeRawMemory<BYTE>(swapAddress, static_cast<BYTE>(sourceRegisterContent), MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU, MEMORY_ACCESS_TYPE::AUTOMATIC, YES);

			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), swapAddrContent);

			cpuIdleCycles();
		}
		else
		{
			GBA_WORD swapAddrContent = readRawMemory<GBA_WORD>(swapAddress, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU);
			// Refer "INFORMATION_001" for the reason to perform ROR below
			swapAddrContent = performShiftOperation(
				NO
				, SHIFT_TYPE::ROR
				, ((swapAddress & THREE) << THREE)
				, swapAddrContent
				, DISABLED
			);

			// Setting LOCK to YES (Source NBA)
			CPUTODO("Find a source for need of LOCK during memory write in SingleDataSwap at %d in %s", __LINE__, __FILE__);
			writeRawMemory<GBA_WORD>(swapAddress, sourceRegisterContent, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU, MEMORY_ACCESS_TYPE::AUTOMATIC, YES);

			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), swapAddrContent);

			cpuIdleCycles();
		}
	}
	RETURN isThisTheInstruction;
}

bool GBA_t::MultiplyAndMultiplyAccumulate()
{
	bool isThisTheInstruction = NO;

	// Multiply And Multiply Accumulate
	uint32_t strippedOpCode = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & MULTIPLY_AND_MULTIPLY_ACCUMULATE_MASK);

	if (strippedOpCode == MULTIPLY_INSTRUCTION)
	{
		isThisTheInstruction = YES;
		uint32_t rm = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY.rm;
		uint32_t rs = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY.rs;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY.rd;
		uint32_t rn = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY.rn;
		bool s = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY.s);
		bool a = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY.a);

		// Increment the PC
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		uint32_t op1 = ZERO;
		uint32_t op2 = ZERO;
		uint32_t accum = ZERO;
		uint32_t result = ZERO;

		if (rd == rm)
		{
			CPUWARN("Multiply and Multiply Accumulate; rd == rm");
		}

		if (rm == PC || rs == PC || rd == PC || rn == PC)
		{
			FATAL("Multiply and Multiply Accumulate; rn == PC || rs == PC || rd == PC || rm == PC");
		}

		op1 = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rm));
		op2 = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rs));

		result = op1 * op2;

		bool full = TickMultiply(YES, op2);

		if (a == YES)
		{
			accum = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rn));
			result += accum;
			cpuIdleCycles();
		}

		if (s == YES)
		{
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((result == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((result >> THIRTYONE) ? ONE : ZERO);

			// Carry flag logic
			if (full)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = MultiplyCarrySimple(op2) ? ONE : ZERO;
			}
			else
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = MultiplyCarryLo(op1, op2, accum) ? ONE : ZERO;
			}
		}

		cpuSetRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rd), getARMState(), result);
	}

	if (strippedOpCode == MULTIPLY_ACCUMULATE_INSTRUCTION)
	{
		isThisTheInstruction = YES;
		uint32_t rm = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY_LONG.rm;
		uint32_t rs = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY_LONG.rs;
		uint32_t rdHi = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY_LONG.rdhi;
		uint32_t rdLo = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY_LONG.rdlo;
		bool s = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY_LONG.s);
		bool a = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY_LONG.a);
		bool u = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.MULTIPLY_LONG.u);

		// Increment the PC
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		uint32_t op1 = ZERO;
		uint32_t op2 = ZERO;
		uint64_t result = ZERO;
		uint32_t accum_lo = ZERO;
		uint32_t accum_hi = ZERO;

		if (rdHi == rm || rdLo == rm || rdHi == rdLo)
		{
			FATAL("Multiply and Multiply Accumulate; rdHi == rm || rdLo == rm || rdHi == rdLo");
		}

		if (rm == PC || rs == PC || rdHi == PC || rdLo == PC)
		{
			FATAL("Multiply and Multiply Accumulate; rm == PC || rs == PC || rdHi == PC || rdLo == PC");
		}

		op1 = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rm));
		op2 = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rs));

		if (u == YES)
		{
			result = (int64_t)(int32_t)op1 * (int64_t)(int32_t)op2;
		}
		else
		{
			result = (uint64_t)op1 * (uint64_t)op2;
		}

		bool full = TickMultiply(u, op2);
		cpuIdleCycles();

		if (a == YES)
		{
			accum_hi = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rdHi));
			accum_lo = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rdLo));
			uint64_t value = (uint64_t)accum_hi << 32 | accum_lo;
			result += value;

			cpuIdleCycles();
		}

		uint32_t result_hi = result >> 32;

		if (s == YES)
		{
			pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((result == ZERO) ? ONE : ZERO);
			pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = (result_hi >> THIRTYONE);

			// Carry flag logic for long multiply
			if (full)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit =
					MultiplyCarryHi(u == YES, op1, op2, accum_hi) ? ONE : ZERO;
			}
			else
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit =
					MultiplyCarryLo(op1, op2, accum_lo) ? ONE : ZERO;
			}
		}

		cpuSetRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rdHi), getARMState(), result_hi);
		cpuSetRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rdLo), getARMState(), (uint32_t)(result & 0xFFFFFFFF));
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::HalfWordDataTransfer()
{
	bool isThisTheInstruction = NO;

	auto ROTATE_RIGHT = [&](uint32_t value, uint32_t amount)
		{
			amount &= THIRTYONE;
			RETURN(value >> amount) | (value << ((-ONE * amount) & THIRTYONE));
		};

	auto HALF_WORD_DATA_TRANSFER = [&](uint32_t uOffset, uint32_t rn, uint32_t rd, bool p, bool u, bool w, bool l, bool s, bool h)
		{
			if (p == NO && w == YES)
			{
				FATAL("Half-Word Data Transfer; p == 0 && w == 1");
			}

			if (w == YES && rn == PC)
			{
				FATAL("Half-Word Data Transfer; w == 1 && rn == PC");
			}

			if (s == NO && h == NO)	// https://www.gregorygaines.com/blog/decoding-the-arm7tdmi-instruction-set-game-boy-advance/
			{
				FATAL("Half-Word Data Transfer; s == 0 && h == 0");
			}

			if (p == NO) // post-increment
			{
				// force w to one, otherwise post increment doesn't makes sense (4.10.1 of https://www.dwedit.org/files/ARM7TDMI.pdf)
				w = YES;
			}

			uint32_t address = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rn));
			int32_t uEffectiveOffset = (int32_t)uOffset;

			// Increment the PC
			pGBA_cpuInstance->registers.pc += FOUR;
			pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

			if (u == NO) // subtract from base, hence changing the sign
			{
				uEffectiveOffset = -uEffectiveOffset;
			}

			if (p == YES) // pre-increment
			{
				address += uEffectiveOffset; // adding offset before the memory read/write operation
			}

			if (l == NO)
			{
				if (s == NO && h == YES) // STRH: Store unsigned halfword (zero extended)
				{
					uint32_t register2memory = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rd));

					if (rd == PC)
					{
						CPUWARN("Half-Word Data Transfer (STRH with rd == PC); Special Behaviour");
						register2memory += FOUR; // reason already mentioned in BLOCK_DATA_TRANSFER
					}

					writeRawMemory<GBA_HALFWORD>(address, register2memory, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU);
				}
				else
				{
					WARN("Half-Word Data Transfer; Invalid s and h combination");
				}
			}

			if (w == YES)
			{
				auto writeBackAddress = address;
				// post increment makes sense only if write back is enabled, otherwise who's gonna see the post increment...
				if (p == NO) // post-increment
				{
					writeBackAddress += uEffectiveOffset; // adding offset after the memory read/write operation
				}

				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rn, getARMState(), writeBackAddress);
			}

			if (l == YES)
			{
				uint32_t memory2register = ZERO;

				if (s == NO && h == YES) // LDRH: Load unsigned halfword (zero extended)
				{
					uint16_t value = readRawMemory<GBA_HALFWORD>(address, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU);
					// Refer "INFORMATION_001" for the reason to perform ROR below
					memory2register = ROTATE_RIGHT(value, (address & 0x01) << THREE);
				}
				else if (s == YES && h == NO) //LDRSB: Load signed byte (sign extended)
				{
					uint32_t value = readRawMemory<BYTE>(address, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
					memory2register = signExtend32(value, EIGHT);
				}
				else if (s == YES && h == YES) // LDRSH: Load signed halfword (sign extended)
				{
					if (address & 0x01)
					{
						// Refer "INFORMATION_001" (especially the part which talks about signed data) for the reason why bit 7 is sign extended instead of bit 15 when the address is not 16 bit aligned
						uint32_t value = readRawMemory<BYTE>(address, MEMORY_ACCESS_WIDTH::EIGHT_BIT, MEMORY_ACCESS_SOURCE::CPU);
						memory2register = signExtend32(value, EIGHT);
					}
					else
					{
						uint32_t value = readRawMemory<GBA_HALFWORD>(address, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU);
						memory2register = signExtend32(value, SIXTEEN);
					}
				}

				cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), memory2register);

				cpuIdleCycles();
			}
		};

	uint32_t strippedOpCode1 = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & HALF_WORD_DATA_TRANSFER_REGISTER_MASK);
	uint32_t strippedOpCode2 = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & HALF_WORD_DATA_TRANSFER_IMMEDIATE_MASK);

	// Half Word Data Transfer (Register) 
	if (strippedOpCode1 == HALF_WORD_DATA_TRANSFER_REGISTER_INSTRUCTION)
	{
		isThisTheInstruction = YES;

		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_RO.rd;
		uint32_t rn = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_RO.rn;
		uint32_t rm = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_RO.rm;

		if (rm == PC)
		{
			FATAL("Half-Word Data Transfer; rm == PC");
		}

		uint32_t uOffset = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rm);

		bool p = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_RO.p);
		bool u = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_RO.u);
		bool w = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_RO.w);
		bool l = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_RO.l);
		bool s = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_RO.s);
		bool h = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_RO.h);

		HALF_WORD_DATA_TRANSFER(uOffset, rn, rd, p, u, w, l, s, h);
	}
	// Half Word Data Transfer (Immediate) 
	else if (strippedOpCode2 == HALF_WORD_DATA_TRANSFER_IMMEDIATE_INSTRUCTION)
	{
		isThisTheInstruction = YES;

		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.rd;
		uint32_t rn = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.rn;
		uint32_t uOffset = ((pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.offset_high << FOUR) | (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.offset_low));

		bool p = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.p);
		bool u = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.u);
		bool w = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.w);
		bool l = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.l);
		bool s = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.s);
		bool h = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.HALFWORD_DATATRANSFER_IO.h);

		HALF_WORD_DATA_TRANSFER(uOffset, rn, rd, p, u, w, l, s, h);
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::psrTransfer()
{
	bool isThisTheInstruction = NO;

	uint32_t strippedOpCode1 = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & MRS_MASK);
	uint32_t strippedOpCode2 = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & MSR_MASK);

	uint32_t uOperand2 = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.operand2;
	uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.rd;
	uint32_t rn = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.rn;
	uint32_t subOpCode = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.opcode;

	bool i = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.immediate);

	bool isSPSR = (bool)(GETBIT(TWENTYTWO, pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode));

	if (strippedOpCode1 == MRS_INSTRUCTION) // Move CPSR or SPSR to another register
	{
		isThisTheInstruction = YES;

		if (i == YES)
		{
			FATAL("PSR Tranfer (MRS); i == 1");
		}
		else
		{
			psr_t psr = { ZERO };
			psr.psrMemory = ((isSPSR == YES) ? cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SPSR) : cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR));
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rd, getARMState(), psr.psrMemory);
		}

		// Increment the PC
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;
	}
	else if (strippedOpCode2 == MSR_INSTRUCTION)
	{
		isThisTheInstruction = YES;

		typedef union fieldMasks
		{
			struct
			{
				uint8_t c : 1; // Write to control field
				uint8_t x : 1; // Write to extension field
				uint8_t s : 1; // Write to status field
				uint8_t f : 1; // Write to flags field
			};
			uint8_t raw;
		} fieldMasks_t;

		fieldMasks_t fieldMasks = { ZERO };
		fieldMasks.raw = rn; // rn represents the fields mask

		uint32_t dataToBeSet = ZERO;

		if (i == YES)
		{
			typedef union msrImmediateFlags
			{
				struct
				{
					uint16_t imm : 8;
					uint16_t shift : 4;
				} arm;
				uint16_t raw;
			} msrImmediateFlags_t;

			msrImmediateFlags_t msrImmediateFlags = { ZERO };
			msrImmediateFlags.raw = uOperand2; // bits 0 to bit 11 ... same as operand 2
			uint32_t valueBeforeShift = msrImmediateFlags.arm.imm;
			uint32_t shiftAmount = msrImmediateFlags.arm.shift;
			shiftAmount *= TWO; // multiply by two
			shiftAmount &= 0x1F; // limit the value within (0, 31)

			// perform ROR shift
			if (shiftAmount != ZERO) // no point in doing shift 0
			{
				dataToBeSet = (valueBeforeShift >> shiftAmount) | (valueBeforeShift << ((-ONE * shiftAmount) & 0x1F));
			}
			else
			{
				dataToBeSet = valueBeforeShift;
			}
		}
		else
		{
			uint32_t rm = uOperand2 & 0x0F; // bits 0 - 3 represents rm register
			dataToBeSet = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)rm);;
		}

		// create the mask
		uint32_t actualMask = ZERO;
		if (fieldMasks.f)
		{
			actualMask |= 0xFF000000u;
		}
		if (fieldMasks.s)
		{
			actualMask |= 0x00FF0000u;
		}
		if (fieldMasks.x)
		{
			actualMask |= 0x0000FF00u;
		}
		if (fieldMasks.c)
		{
			actualMask |= 0x000000FFu;
		}

		if ((getARMMode() == OP_MODE_TYPE::OP_USR) && (isSPSR == NO))
		{
			// Refer to 4.6.1 of https://www.dwedit.org/files/ARM7TDMI.pdf
			actualMask &= 0xFF000000;
		}

		// now, retain only valid bits in "dataToBeSet"
		dataToBeSet &= actualMask;

		// update psr
		psr_t psr = { ZERO };
		psr.psrMemory = ((isSPSR == YES) ? cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SPSR) : cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR));
		psr.psrMemory = ((psr.psrMemory & ((~actualMask) & 0xFFFFFFFF)) | dataToBeSet);

		if (isSPSR == YES)
		{
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SPSR, getARMState(), psr.psrMemory);
		}
		else
		{
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR, getARMState(), psr.psrMemory);
		}

		// Increment the PC
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;
	}

	RETURN isThisTheInstruction;
}

bool GBA_t::DataProcessing()
{
	bool isThisTheInstruction = NO;

	// Data Processing
	uint32_t strippedOpCode = (pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode & DATA_PROCESSING_MASK);
	if (strippedOpCode == DATA_PROCESSING_INSTRUCTION)
	{
		isThisTheInstruction = YES;
		uint32_t uOperand2 = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.operand2;
		uint32_t rd = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.rd;
		uint32_t rn = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.rn;
		uint32_t subOpCode = pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.opcode;

		bool i = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.immediate);
		bool s = (bool)(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.DATA_PROCESSING.s);

		// NOTE: This is most probably needed because, if this is not there... assume a case where subOpCode is ADC
		// The shift operation may change the C flag and original intent of ADC, if it was to detect C flag is not possible as C got modified by shift operation
		// Hence, we save the original C value to be used by subOpCodes
		uint32_t cOriginal = pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit;
		bool sOriginal = (bool)(GETBIT(TWENTY, pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode));

		// Refer to 4.5.1 of https://www.dwedit.org/files/ARM7TDMI.pdf
		if (rd == PC && s == SET)
		{
			CPUWARN("Data Processing; rd == PC && s == ONE");
			// NOTE: 
			// No need to clean "s" bit here as the condition "if (rd == PC && sOriginal)" below after performing the "Data Processing" will override the CPSR with SPSR.
			// So any flags we set will be overriden anyways...
		}

		typedef union
		{
			struct
			{
				GBA_HALFWORD rm : FOUR;
				GBA_HALFWORD r : ONE;
				GBA_HALFWORD shiftType : TWO;
				GBA_HALFWORD dontUseThisField : NINE;
			} op2ShiftReg;

			// When bit25 = 0 and bit4 = 0
			struct
			{
				GBA_HALFWORD rm : FOUR;
				GBA_HALFWORD r : ONE;
				GBA_HALFWORD shiftType : TWO;
				GBA_HALFWORD shiftAmount : FIVE;
				GBA_HALFWORD unused0 : FOUR;
			} op2ShiftRegType1;

			// When bit25 = 0 and bit4 = 1
			struct
			{
				GBA_HALFWORD rm : FOUR;
				GBA_HALFWORD r : ONE;
				GBA_HALFWORD shiftType : TWO;
				GBA_HALFWORD unused1 : ONE;
				GBA_HALFWORD rs : FOUR;
				GBA_HALFWORD unused2 : FOUR;
			} op2ShiftRegType2;

			// When bit25 = 1
			struct
			{
				GBA_HALFWORD imm : EIGHT;
				GBA_HALFWORD rorShiftApplied : FOUR;
				GBA_HALFWORD unused3 : FOUR;
			} op2Imm;
			GBA_HALFWORD raw;
		} operand2_t;

		operand2_t operand2;
		operand2.raw = static_cast<GBA_HALFWORD>(uOperand2);

		GBA_WORD op2 = ZERO;

		if (i == YES) // second operand is an immediate value
		{
			GBA_WORD preshiftedOperand2 = (operand2.op2Imm.imm & 0xFF);
			GBA_WORD shiftAmount = operand2.op2Imm.rorShiftApplied;
			shiftAmount *= TWO; // multiply by two
			shiftAmount &= 0x1F; // limit the value within (0, 31)

			// perform ROR shift
			if (shiftAmount != ZERO) // no point in doing shift 0
			{
				op2 = (preshiftedOperand2 >> shiftAmount) | (preshiftedOperand2 << ((-ONE * shiftAmount) & 0x1F));

				if (s == YES)
				{
					uint32_t carryBit = GETBIT(THIRTYONE, op2);
					pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = (carryBit == ZERO ? ZERO : ONE);
				}
			}
			else
			{
				op2 = preshiftedOperand2;
			}
		}
		else // second operand is a shifted register
		{
			SHIFT_TYPE typeOfShift = (SHIFT_TYPE)operand2.op2ShiftReg.shiftType;
			GBA_WORD shiftAmount = ZERO;
			GBA_WORD dataToBeShifted = ZERO;

			if (operand2.op2ShiftReg.r == ONE) // shift amount determined by rs register
			{
				if (operand2.op2ShiftRegType2.rs == PC)
				{
					// NOTE: armwrestler.gba exercises this condition for v5 testing
					FATAL("Data Processing; rs == PC");
				}

				shiftAmount = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)operand2.op2ShiftRegType2.rs));
				shiftAmount &= 0x00FF;

				/*
				* Using R15 (PC):
				* When using R15 as Destination (Rd), note below CPSR description and Execution time description.
				* When using R15 as operand (Rm or Rn), the returned value depends on the instruction: PC+12 if I=0,R=1 (shift by register), otherwise PC+8 (shift by immediate).
				*
				* The above statement is handled as follows in our code
				* Instead of blindly incrementing the op1 or dataToBeShifted by 4 whenever rn or rm is PC
				* If we look into the reason why this is the case, we see that the actual PC is incremented at this time... as part of its normal increment cycles
				* So, when we read the PC, for op1 or dataToBeShifted, it is already incremented
				*
				*/

				// Increment the PC
				pGBA_cpuInstance->registers.pc += FOUR;
				pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

				cpuIdleCycles();

				dataToBeShifted = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)operand2.op2ShiftRegType2.rm));

				op2 = performShiftOperation(s, typeOfShift, shiftAmount, dataToBeShifted, DISABLED);
			}
			else // shift amount determined by shift amount field
			{
				dataToBeShifted = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)operand2.op2ShiftRegType1.rm));

				shiftAmount = operand2.op2ShiftRegType1.shiftAmount;

				op2 = performShiftOperation(s, typeOfShift, shiftAmount, dataToBeShifted, ENABLED);
			}

		}

		GBA_WORD op1 = cpuReadRegister(getCurrentlyValidRegisterBank(), ((REGISTER_TYPE)rn));
		GBA_WORD subOpCodeResult = ZERO;
		GBA_WORD* validValueToSet = NULL;
		switch ((DATAPROCESSING_SUBCODES)subOpCode)
		{
		case DATAPROCESSING_SUBCODES::DP_AND:
		{
			subOpCodeResult = op1 & op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_XOR:
		{
			subOpCodeResult = op1 ^ op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_SUB:
		{
			subOpCodeResult = op1 - op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((op1 >= op2) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op1 ^ op2) & (op1 ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_RSB:
		{
			subOpCodeResult = op2 - op1;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((op2 >= op1) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op2 ^ op1) & (op2 ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_ADD:
		{
			subOpCodeResult = op1 + op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
				uint64_t result = (uint64_t)op1 + (uint64_t)op2;
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((result > 0xFFFFFFFF) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op2 ^ subOpCodeResult) & (~(op1 ^ op2))) >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_ADC:
		{
			uint64_t op2c = (uint64_t)op2 + (uint64_t)cOriginal;
			uint64_t subOpCodeResult64 = (uint64_t)op1 + (uint64_t)op2c;
			subOpCodeResult = (GBA_WORD)subOpCodeResult64;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((subOpCodeResult64 > 0xFFFFFFFF) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op2 ^ subOpCodeResult) & (~(op1 ^ op2))) >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_SBC:
		{
			uint64_t temp = (uint64_t)op2 + (uint64_t)(cOriginal == ONE ? ZERO : ONE);
			uint64_t result64 = (uint64_t)op1 - temp;
			subOpCodeResult = (uint32_t)result64;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((op1 >= temp) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op1 ^ op2) & (op1 ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_RSC:
		{
			uint64_t temp = (uint64_t)op1 - (uint64_t)cOriginal + (uint64_t)ONE;
			uint64_t result64 = (uint64_t)op2 - temp;
			subOpCodeResult = (uint32_t)result64;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((op2 >= temp) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op2 ^ op1) & (op2 ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_TST:
		{
			subOpCodeResult = op1 & op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			}
			else
			{
				CPUWARN("Data Processing (DP_TST); s == 0");
			}
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_TEQ:
		{
			subOpCodeResult = op1 ^ op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			}
			else
			{
				CPUWARN("Data Processing (DP_TEQ); s == 0");
			}
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_CMP:
		{
			subOpCodeResult = op1 - op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((op1 >= op2) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op1 ^ op2) & (op1 ^ subOpCodeResult)) >> THIRTYONE) ? ONE : ZERO);
			}
			else
			{
				CPUWARN("Data Processing (DP_CMP); s == 0");
			}
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_CMN:
		{
			subOpCodeResult = op1 + op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
				uint64_t result = (uint64_t)op1 + (uint64_t)op2;
				pGBA_cpuInstance->registers.cpsr.psrFields.psrCarryBorrowExtBit = ((result > 0xFFFFFFFF) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrOverflowBit = ((((op2 ^ subOpCodeResult) & (~(op1 ^ op2))) >> THIRTYONE) ? ONE : ZERO);
			}
			else
			{
				CPUWARN("Data Processing (DP_CMN); s == 0");
			}
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_ORR:
		{
			subOpCodeResult = op1 | op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_MOV:
		{
			subOpCodeResult = op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_BIC:
		{
			subOpCodeResult = op1 & (~op2);
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		case DATAPROCESSING_SUBCODES::DP_NOT:
		{
			subOpCodeResult = ~op2;
			if (s == YES)
			{
				pGBA_cpuInstance->registers.cpsr.psrFields.psrZeroBit = ((subOpCodeResult == ZERO) ? ONE : ZERO);
				pGBA_cpuInstance->registers.cpsr.psrFields.psrNegativeBit = ((subOpCodeResult >> THIRTYONE) ? ONE : ZERO);
			}
			validValueToSet = &subOpCodeResult;
			BREAK;
		}
		default:
		{
			FATAL("Data Processing; Unknown Sub-Opcode");
		}
		}

		if (rd == PC && sOriginal) // ARM Mode or State change may happen if rd == PC and flags is enabled
		{
			// http://problemkaputt.de/gbatek.htm#armopcodesmemorysingleDataTransferldrstrpld
			if (getARMMode() == OP_MODE_TYPE::OP_USR)
			{
				FATAL("Data Processing (ARM Mode = USR); TBD");
			}

			psr_t spsr = { ZERO };
			spsr.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)SPSR);
			setARMMode((OP_MODE_TYPE)spsr.psrFields.psrModeBits);
			cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR, getARMState(), spsr.psrMemory);

			if (getARMState() == STATE_TYPE::ST_THUMB && validValueToSet != NULL)
			{
				*validValueToSet |= ONE; // because PC that we are about to load should be in THUMB state
			}
		}

		if (validValueToSet != NULL)
		{
			// NOTE: We will not reach this point if the sub opcode was TST/TEQ/CMP/CMN
			cpuSetRegister(getRegisterBankFromOperatingMode(getARMMode()), (REGISTER_TYPE)rd, getARMState(), *validValueToSet);
		}

		// The possible cases where PC is not modified at all even at this point (i.e. end of the execution of current opcode)
		if (
			(validValueToSet == NULL) // In this case, rd was not set at all. Hence, even if rd == PC, PC would not be modified. So, PC needs to increment
			||
			((rd != PC) && ((i == YES) || ((i == NO) && (operand2.op2ShiftReg.r == ZERO)))) // In this case, rd probably was set, but rd != PC AND i = 1 and r = 1, so PC is not modified at all as of now. So, PC needs to increment
			)
		{
			// Increment the PC
			pGBA_cpuInstance->registers.pc += FOUR;
			pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;
		}
	}
	RETURN isThisTheInstruction;
}
