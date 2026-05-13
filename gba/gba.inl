#pragma once

#pragma region GBA_SPECIFIC_INCLUDES
#include "gba.h"
#include "gba_opcodes.inl"
#pragma endregion GBA_SPECIFIC_INCLUDES

#pragma region GBA_SPECIFIC_DECLARATIONS
extern COUNTER64 gbaEmulationCounter[100];
extern uint32_t gameboyAdvance_texture;
extern uint32_t gameboyAdvance_matrix_texture;
#pragma endregion GBA_SPECIFIC_DECLARATIONS

OPT_SPEED

#pragma region ARM7TDMI_DEFINITIONS
MASQ_INLINE void GBA_t::cpuSetRegister(REGISTER_BANK_TYPE rb, REGISTER_TYPE rt, STATE_TYPE st, uint32_t u32parameter)
{
	uint8_t registerType = ((uint8_t)rt);
	uint8_t registerBank = ((uint8_t)rb);
	if (registerType < (LO_GP_REGISTERS)) // lower 8 registers are not banked
	{
		pGBA_registers->unbankedLORegisters[registerType] = u32parameter;
		RETURN;
	}
	else if (registerType < (LO_GP_REGISTERS + HI_GP_REGISTERS))
	{
		if ((registerType == SP) || (registerType == LR))
		{
			pGBA_registers->bankedHIRegisters[registerBank][(registerType - LO_GP_REGISTERS)] = u32parameter;
			RETURN;
		}
		else if (registerType == PC) // PC is not banked
		{
			if (getARMState() == STATE_TYPE::ST_THUMB)
			{
				pGBA_cpuInstance->registers.pc = (u32parameter & 0xFFFFFFFE);
			}
			else if (getARMState() == STATE_TYPE::ST_ARM)
			{
				pGBA_cpuInstance->registers.pc = (u32parameter & 0xFFFFFFFC);
			}
			loadPipeline(u32parameter);
			RETURN;
		}
		else // R8 to R12
		{
			if (rb == REGISTER_BANK_TYPE::RB_FIQ) // banked if FIQ mode
			{
				pGBA_registers->bankedHIRegisters[registerBank][(registerType - LO_GP_REGISTERS)] = u32parameter;
			}
			else // not banked if not FIQ mode
			{
				pGBA_registers->bankedHIRegisters[ZERO][(registerType - LO_GP_REGISTERS)] = u32parameter;
				pGBA_registers->bankedHIRegisters[TWO][(registerType - LO_GP_REGISTERS)] = u32parameter;
				pGBA_registers->bankedHIRegisters[THREE][(registerType - LO_GP_REGISTERS)] = u32parameter;
				pGBA_registers->bankedHIRegisters[FOUR][(registerType - LO_GP_REGISTERS)] = u32parameter;
				pGBA_registers->bankedHIRegisters[FIVE][(registerType - LO_GP_REGISTERS)] = u32parameter;
			}
			RETURN;
		}
	}
	else if (registerType < (TOTAL_GP_REGISTERS))
	{
		if (registerType == CPSR) // CPSR is not banked
		{
			// ARM7TDMI: bit 4 is always forced to 1
			u32parameter |= 0x00000010;
			pGBA_registers->cpsr.psrMemory = u32parameter;
			setARMMode((OP_MODE_TYPE)pGBA_registers->cpsr.psrFields.psrModeBits);
			RETURN;
		}
		else if (registerType == SPSR)
		{
			if ((pGBA_registers->cpsr.psrFields.psrStateBit == RESET) && ((u32parameter & (1u << 5)) != 0))
			{
				WARN("MSR: Attempt to set Thumb bit (T)");
			}

			if (registerBank == ZERO) RETURN;
			pGBA_registers->spsr[registerBank].psrMemory = u32parameter;
			RETURN;
		}
	}

	FATAL("Writing to Unknown Register");
}

MASQ_INLINE uint32_t GBA_t::cpuReadRegister(REGISTER_BANK_TYPE rb, REGISTER_TYPE rt)
{
	uint8_t registerType = ((uint8_t)rt);
	uint8_t registerBank = ((uint8_t)rb);
	if (registerType < (LO_GP_REGISTERS)) // lower 8 registers are not banked
	{
		RETURN pGBA_registers->unbankedLORegisters[registerType];
	}
	else if (registerType < (LO_GP_REGISTERS + HI_GP_REGISTERS))
	{
		if ((registerType == SP) || (registerType == LR))
		{
			RETURN pGBA_registers->bankedHIRegisters[registerBank][(registerType - LO_GP_REGISTERS)];
		}
		else if (registerType == PC) // PC is not banked
		{
			RETURN pGBA_registers->pc;
		}
		else // R8 to R12
		{
			if (rb == REGISTER_BANK_TYPE::RB_FIQ) // banked if FIQ mode
			{
				RETURN pGBA_registers->bankedHIRegisters[registerBank][(registerType - LO_GP_REGISTERS)];
			}
			else // not banked if not FIQ mode (so RETURN from any one mode other than FIQ mode)
			{
				RETURN pGBA_registers->bankedHIRegisters[ZERO][(registerType - LO_GP_REGISTERS)];
			}
		}
	}
	else if (registerType < (TOTAL_GP_REGISTERS))
	{
		if (registerType == CPSR) // CPSR is not banked
		{
			RETURN pGBA_registers->cpsr.psrMemory;
		}
		else if (registerType == SPSR)
		{
			if (rb == REGISTER_BANK_TYPE::RB_USR_SYS)
			{
				// NOTE: In system/user mode reading from SPSR RETURNs the current CPSR value.
				// However writes to SPSR appear to do nothing.
				// Refer 3.7.1 of https://www.dwedit.org/files/ARM7TDMI.pdf and Nano 

				RETURN pGBA_registers->cpsr.psrMemory;
			}
			else
			{
				RETURN pGBA_registers->spsr[registerBank].psrMemory;
			}
		}
	}

	FATAL("Reading from Unknown Register");
	RETURN((uint32_t)NULL);
}

GBA_HALFWORD GBA_t::readIO(uint32_t address, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType)
{
	// NOTE: Below if condition to handle "MEMORY_ACCESS_SOURCE::HOST" is very important for following reasons
	// * We to handle non 16 bit writes to IO basically do multiple 16 bit reads and combine data and write back
	// * Problem arises when the 16 bit read we do happens to be on a write only register, our standard read will give open-bus data, etc
	// * The combined data which we write in the end will mess the contents of write only registers
	// * Hence, to handle reads used for combine purpose, we should directly read the memory without handling for open-bus, etc
	// * To differentiate this read, we have created "MEMORY_ACCESS_SOURCE::HOST"
	if (source == MEMORY_ACCESS_SOURCE::HOST)
	{
		RETURN pGBA_memory->mGBAMemoryMap.mIO.mIOMemory16bit[(address - IO_START_ADDRESS) / TWO];
	}

	switch (address)
	{
	case IO_DISPCNT:
	{
		RETURN pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTHalfWord;
	}
	case IO_GREENSWAP:
	{
		RETURN pGBA_peripherals->mGREENSWAPHalfWord.mGREENSWAPHalfWord;
	}
	case IO_DISPSTAT:
	{
		RETURN pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord;
	}
	case IO_VCOUNT:
	{
		RETURN pGBA_peripherals->mVCOUNTHalfWord.mVCOUNTHalfWord;
	}
	case IO_BG0CNT:
	{
		pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT = RESET;
		RETURN pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord;
	}
	case IO_BG1CNT:
	{
		pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTFields.BG2_BG3_DISP_AREA_OVERFLOW_OR_NDS_BG0_BG1_EXT_PALETTE_SLOT = RESET;
		RETURN pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord;
	}
	case IO_BG2CNT:
	{
		RETURN pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord;
	}
	case IO_BG3CNT:
	{
		RETURN pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord;
	}
	case IO_BG0HOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG0VOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG1HOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG1VOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2HOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2VOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3HOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3VOFS:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2PA:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2PB:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2PC:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2PD:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2X_L:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2X_H:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2Y_L:
	{
		RETURN 0xDEAD;
	}
	case IO_BG2Y_H:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3PA:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3PB:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3PC:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3PD:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3X_L:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3X_H:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3Y_L:
	{
		RETURN 0xDEAD;
	}
	case IO_BG3Y_H:
	{
		RETURN 0xDEAD;
	}
	case IO_WIN0H:
	{
		RETURN 0xDEAD;
	}
	case IO_WIN1H:
	{
		RETURN 0xDEAD;
	}
	case IO_WIN0V:
	{
		RETURN 0xDEAD;
	}
	case IO_WIN1V:
	{
		RETURN 0xDEAD;
	}
	case IO_WININ:
	{
		pGBA_peripherals->mWININHalfWord.mWININFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mWININHalfWord.mWININFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mWININHalfWord.mWININHalfWord;
	}
	case IO_WINOUT:
	{
		pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mWINOUTHalfWord.mWINOUTFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mWINOUTHalfWord.mWINOUTHalfWord;
	}
	case IO_MOSAIC:
	{
		RETURN 0xDEAD;
	}
	case IO_400004E:
	{
		RETURN 0xDEAD;
	}
	case IO_BLDCNT:
	{
		pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTFields.NOT_USED_0 = RESET;
		RETURN pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord;
	}
	case IO_BLDALPHA:
	{
		pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAHalfWord;
	}
	case IO_BLDY:
	{
		RETURN 0xDEAD;
	}
	case IO_4000056:
	case IO_4000058:
	case IO_400005A:
	case IO_400005C:
	case IO_400005E:
	{
		RETURN 0xDEAD;
	}
	case IO_SOUND1CNT_L:
	{
		pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.NOT_USED_0 = RESET;
		RETURN pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord;
	}
	case IO_SOUND1CNT_H:
	{
		mSOUND1CNT_HHalfWord_t data = pGBA_peripherals->mSOUND1CNT_HHalfWord;
		data.mSOUND1CNT_HFields.SOUND_LENGTH = RESET;
		RETURN data.mSOUND1CNT_HHalfWord;
	}
	case IO_SOUND1CNT_X:
	{
		pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.NOT_USED_0 = RESET;
		mSOUND1CNT_XHalfWord_t data = pGBA_peripherals->mSOUND1CNT_XHalfWord;
		data.mSOUND1CNT_XFields.FREQ = RESET;
		data.mSOUND1CNT_XFields.NOT_USED_0 = RESET;
		data.mSOUND1CNT_XFields.INITIAL = RESET;
		RETURN data.mSOUND1CNT_XHalfWord;
	}
	case IO_4000066:
	{
		RETURN 0x0000;
	}
	case IO_SOUND2CNT_L:
	{
		mSOUND2CNT_LHalfWord_t data = pGBA_peripherals->mSOUND2CNT_LHalfWord;
		data.mSOUND2CNT_LFields.SOUND_LENGTH = RESET;
		RETURN data.mSOUND2CNT_LHalfWord;
	}
	case IO_400006A:
	{
		RETURN 0x0000;
	}
	case IO_SOUND2CNT_H:
	{
		pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.NOT_USED_0 = RESET;
		mSOUND2CNT_HHalfWord_t data = pGBA_peripherals->mSOUND2CNT_HHalfWord;
		data.mSOUND2CNT_HFields.FREQ = RESET;
		data.mSOUND2CNT_HFields.NOT_USED_0 = RESET;
		data.mSOUND2CNT_HFields.INITIAL = RESET;
		RETURN data.mSOUND2CNT_HHalfWord;
	}
	case IO_400006E:
	{
		RETURN 0x0000;
	}
	case IO_SOUND3CNT_L:
	{
		pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord;
	}
	case IO_SOUND3CNT_H:
	{
		mSOUND3CNT_HHalfWord_t data = pGBA_peripherals->mSOUND3CNT_HHalfWord;
		data.mSOUND3CNT_HFields.SOUND_LENGTH = RESET;
		data.mSOUND3CNT_HFields.NOT_USED_0 = RESET;
		RETURN data.mSOUND3CNT_HHalfWord;
	}
	case IO_SOUND3CNT_X:
	{
		pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.NOT_USED_0 = RESET;
		mSOUND3CNT_XHalfWord_t data = pGBA_peripherals->mSOUND3CNT_XHalfWord;
		data.mSOUND3CNT_XFields.SAMPLE_RATE = RESET;
		data.mSOUND3CNT_XFields.NOT_USED_0 = RESET;
		data.mSOUND3CNT_XFields.INITIAL = RESET;
		RETURN data.mSOUND3CNT_XHalfWord;
	}
	case IO_4000076:
	{
		RETURN 0x0000;
	}
	case IO_SOUND4CNT_L:
	{
		pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.NOT_USED_0 = RESET;
		mSOUND4CNT_LHalfWord_t data = pGBA_peripherals->mSOUND4CNT_LHalfWord;
		data.mSOUND4CNT_LFields.SOUND_LENGTH = RESET;
		data.mSOUND4CNT_LFields.NOT_USED_0 = RESET;
		RETURN data.mSOUND4CNT_LHalfWord;
	}
	case IO_400007A:
	{
		RETURN 0x0000;
	}
	case IO_SOUND4CNT_H:
	{
		pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.NOT_USED_0 = RESET;
		mSOUND4CNT_HHalfWord_t data = pGBA_peripherals->mSOUND4CNT_HHalfWord;
		data.mSOUND4CNT_HFields.NOT_USED_0 = RESET;
		data.mSOUND4CNT_HFields.INITIAL = RESET;
		RETURN data.mSOUND4CNT_HHalfWord;
	}
	case IO_400007E:
	{
		RETURN 0x0000;
	}
	case IO_SOUNDCNT_L:
	{
		pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LFields.NOT_USED_1 = RESET;
		mSOUNDCNT_LHalfWord_t data = pGBA_peripherals->mSOUNDCNT_LHalfWord;
		data.mSOUNDCNT_LFields.NOT_USED_0 = RESET;
		data.mSOUNDCNT_LFields.NOT_USED_1 = RESET;
		RETURN data.mSOUNDCNT_LHalfWord;
	}
	case IO_SOUNDCNT_H:
	{
		pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.NOT_USED_0 = RESET;
		mSOUNDCNT_HHalfWord_t data = pGBA_peripherals->mSOUNDCNT_HHalfWord;
		data.mSOUNDCNT_HFields.NOT_USED_0 = RESET;
		data.mSOUNDCNT_HFields.DMA_SOUND_A_RESET_FIFO = RESET;
		data.mSOUNDCNT_HFields.DMA_SOUND_B_RESET_FIFO = RESET;
		RETURN data.mSOUNDCNT_HHalfWord;
	}
	case IO_SOUNDCNT_X:
	{
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.NOT_USED_0 = RESET;
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.NOT_USED_1 = RESET;
		RETURN pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XHalfWord;
	}
	case IO_4000086:
	{
		RETURN 0x0000;
	}
	case IO_SOUNDBIAS:
	{
		pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASFields.NOT_USED_0 = RESET;
		RETURN pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASHalfWord;
	}
	case IO_400008A:
	{
		RETURN 0x0000;
	}
	case IO_400008C:
	case IO_400008E:
	{
		RETURN 0xDEAD;
	}
	case (IO_WAVERAM_START_ADDRESS + 0):
	case (IO_WAVERAM_START_ADDRESS + 2):
	case (IO_WAVERAM_START_ADDRESS + 4):
	case (IO_WAVERAM_START_ADDRESS + 6):
	case (IO_WAVERAM_START_ADDRESS + 8):
	case (IO_WAVERAM_START_ADDRESS + 10):
	case (IO_WAVERAM_START_ADDRESS + 12):
	case (IO_WAVERAM_START_ADDRESS + 14):
	{
		RETURN pGBA_peripherals->mWAVERAM16[(address - IO_WAVERAM_START_ADDRESS) / TWO].waveRamHalfWord;
	}
	case IO_FIFO_A_L:
	{
		RETURN 0xDEAD;
	}
	case IO_FIFO_A_H:
	{
		RETURN 0xDEAD;
	}
	case IO_FIFO_B_L:
	{
		RETURN 0xDEAD;
	}
	case IO_FIFO_B_H:
	{
		RETURN 0xDEAD;
	}
	case IO_40000A8:
	case IO_40000AA:
	case IO_40000AC:
	case IO_40000AE:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA0SAD_L:
	case IO_DMA0SAD_H:
	case IO_DMA0DAD_L:
	case IO_DMA0DAD_H:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA0CNT_L:
	{
		RETURN 0x0000;
	}
	case IO_DMA0CNT_H:
	{
		mDMAnCNT_HHalfWord_t data = pGBA_peripherals->mDMA0CNT_H;
		data.mDMAnCNT_HFields.NOT_USED_0 = RESET;
		data.mDMAnCNT_HFields.GAME_PAK_DRQ = RESET;
		RETURN data.mDMAnCNT_HHalfWord;
	}
	case IO_DMA1SAD_L:
	case IO_DMA1SAD_H:
	case IO_DMA1DAD_L:
	case IO_DMA1DAD_H:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA1CNT_L:
	{
		RETURN 0x0000;
	}
	case IO_DMA1CNT_H:
	{
		mDMAnCNT_HHalfWord_t data = pGBA_peripherals->mDMA1CNT_H;
		data.mDMAnCNT_HFields.NOT_USED_0 = RESET;
		data.mDMAnCNT_HFields.GAME_PAK_DRQ = RESET;
		RETURN data.mDMAnCNT_HHalfWord;
	}
	case IO_DMA2SAD_L:
	case IO_DMA2SAD_H:
	case IO_DMA2DAD_L:
	case IO_DMA2DAD_H:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA2CNT_L:
	{
		RETURN 0x0000;
	}
	case IO_DMA2CNT_H:
	{
		mDMAnCNT_HHalfWord_t data = pGBA_peripherals->mDMA2CNT_H;
		data.mDMAnCNT_HFields.NOT_USED_0 = RESET;
		data.mDMAnCNT_HFields.GAME_PAK_DRQ = RESET;
		RETURN data.mDMAnCNT_HHalfWord;
	}
	case IO_DMA3SAD_L:
	case IO_DMA3SAD_H:
	case IO_DMA3DAD_L:
	case IO_DMA3DAD_H:
	{
		RETURN 0xDEAD;
	}
	case IO_DMA3CNT_L:
	{
		RETURN 0x0000;
	}
	case IO_DMA3CNT_H:
	{
		mDMAnCNT_HHalfWord_t data = pGBA_peripherals->mDMA3CNT_H;
		data.mDMAnCNT_HFields.NOT_USED_0 = RESET;
		RETURN data.mDMAnCNT_HHalfWord;
	}
	case IO_40000E0:
	case IO_40000E2:
	case IO_40000E4:
	case IO_40000E6:
	case IO_40000E8:
	case IO_40000EA:
	case IO_40000EC:
	case IO_40000EE:
	case IO_40000F0:
	case IO_40000F2:
	case IO_40000F4:
	case IO_40000F6:
	case IO_40000F8:
	case IO_40000FA:
	case IO_40000FC:
	case IO_40000FE:
	{
		RETURN 0xDEAD;
	}
	case IO_TM0CNT_L:
	{
		RETURN pGBA_peripherals->mTIMER0CNT_L;
	}
	case IO_TM0CNT_H:
	{
		RETURN pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HHalfWord;
	}
	case IO_TM1CNT_L:
	{
		RETURN pGBA_peripherals->mTIMER1CNT_L;
	}
	case IO_TM1CNT_H:
	{
		RETURN pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HHalfWord;
	}
	case IO_TM2CNT_L:
	{
		RETURN pGBA_peripherals->mTIMER2CNT_L;
	}
	case IO_TM2CNT_H:
	{
		RETURN pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HHalfWord;
	}
	case IO_TM3CNT_L:
	{
		RETURN pGBA_peripherals->mTIMER3CNT_L;
	}
	case IO_TM3CNT_H:
	{
		RETURN pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HHalfWord;
	}
	case IO_SIOMULTI0:
	{
		RETURN pGBA_peripherals->mSIOMULTI0;
	}
	case IO_SIOMULTI1:
	{
		RETURN pGBA_peripherals->mSIOMULTI1;
	}
	case IO_SIOMULTI2:
	{
		RETURN pGBA_peripherals->mSIOMULTI2;
	}
	case IO_SIOMULTI3:
	{
		RETURN pGBA_peripherals->mSIOMULTI3;
	}
	case IO_SIOCNT:
	{
		RETURN pGBA_peripherals->mSIOCNT.mSIOCNTHalfWord;
	}
	case IO_SIO_DATA8_MLTSEND:
	{
		RETURN pGBA_peripherals->mSIO_DATA8_MLTSEND;
	}
	case IO_KEYINPUT:
	{
		RETURN pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTHalfWord;
	}
	case IO_KEYCNT:
	{
		RETURN pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTHalfWord;
	}
	case IO_RCNT:
	{
		RETURN pGBA_peripherals->mRCNTHalfWord.mRCNTHalfWord;
	}
	case IO_IR:
	{
		RETURN 0x0000;
	}
	case IO_JOYCNT:
	{
		RETURN pGBA_peripherals->mJOYCNTHalfWord.mJOYCNTHalfWord;
	}
	case IO_4000142:
	{
		RETURN 0x0000;
	}
	case IO_JOY_RECV_L:
	{
		RETURN pGBA_peripherals->mJOY_RECV_L;
	}
	case IO_JOY_RECV_H:
	{
		RETURN pGBA_peripherals->mJOY_RECV_H;
	}
	case IO_JOY_TRANS_L:
	{
		RETURN pGBA_peripherals->mJOY_TRANS_L;
	}
	case IO_JOY_TRANS_H:
	{
		RETURN pGBA_peripherals->mJOY_TRANS_H;
	}
	case IO_JOYSTAT:
	{
		RETURN pGBA_peripherals->mJOYSTATHalfWord.mJOYSTATHalfWord;
	}
	case IO_400015A:
	{
		RETURN 0x0000;
	}
	case IO_IE:
	{
		RETURN pGBA_peripherals->mIEHalfWord.mIEHalfWord;
	}
	case IO_IF:
	{
		RETURN pGBA_peripherals->mIFHalfWord.mIFHalfWord;
	}
	case IO_WAITCNT:
	{
		RETURN pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTHalfWord;
	}
	case IO_4000206:
	{
		RETURN 0x0000;
	}
	case IO_IME:
	{
		RETURN pGBA_peripherals->mIMEHalfWord.mIMEHalfWord;
	}
	case IO_400020A:
	{
		RETURN 0x0000;
	}
	case IO_POSTFLG:
	case IO_HALTCNT:
	{
		typedef struct
		{
			mPOSTFLGByte_t mPOSTFLGByte;
			mHALTCNTByte_t mHALTCNTByte;
		} mPOSTFLG_HALTCNT_Fields_t;

		mPOSTFLG_HALTCNT_Fields_t mPOSTFLG_HALTCNT_Fields;
		mPOSTFLG_HALTCNT_Fields.mHALTCNTByte = pGBA_peripherals->mHALTCNTByte;
		mPOSTFLG_HALTCNT_Fields.mPOSTFLGByte = pGBA_peripherals->mPOSTFLGByte;
		RETURN static_cast<GBA_HALFWORD>(mPOSTFLG_HALTCNT_Fields.mPOSTFLGByte.mPOSTFLGByte);
	}
	case IO_4000302:
	{
		RETURN 0x0000;
	}
	default:
	{
		RETURN readOpenBus<GBA_HALFWORD>(address, accessWidth, source, accessType);
	}
	}
}

MASQ_INLINE void GBA_t::writeIO(uint32_t address, GBA_HALFWORD data, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType)
{
#define IO8_WRONG_HANDLER() FATAL("Should be handled by writeIO8, Line: %d in File %s", __LINE__, __FILE__)
#define REJECT_IO8(width) if ((width) == MEMORY_ACCESS_WIDTH::EIGHT_BIT) MASQ_UNLIKELY IO8_WRONG_HANDLER()

	switch (address)
	{
	case IO_DISPCNT:
	{
		pGBA_peripherals->mDISPCNTHalfWord.mDISPCNTHalfWord = data;
		RETURN;
	}
	case IO_GREENSWAP:
	{
		pGBA_peripherals->mGREENSWAPHalfWord.mGREENSWAPHalfWord = data;
		RETURN;
	}
	case IO_DISPSTAT:
	{
		uint16_t dispstatBeforeUpdate = pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord;
		pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord = data;
		GBA_HALFWORD retainMask = dispstatBeforeUpdate & 0x07; // extract last 3 bits
		pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord &= 0xFFF8; // clear last 3 bits
		pGBA_peripherals->mDISPSTATHalfWord.mDISPSTATHalfWord |= retainMask; // set last 3 bits to extracted value
		RETURN;
	}
	case IO_VCOUNT:
	{
		RETURN;
	}
	case IO_BG0CNT:
	{
		pGBA_peripherals->mBG0CNTHalfWord.mBGnCNTHalfWord = data;
		RETURN;
	}
	case IO_BG1CNT:
	{
		pGBA_peripherals->mBG1CNTHalfWord.mBGnCNTHalfWord = data;
		RETURN;
	}
	case IO_BG2CNT:
	{
		pGBA_peripherals->mBG2CNTHalfWord.mBGnCNTHalfWord = data;
		RETURN;
	}
	case IO_BG3CNT:
	{
		pGBA_peripherals->mBG3CNTHalfWord.mBGnCNTHalfWord = data;
		RETURN;
	}
	case IO_BG0HOFS:
	{
		pGBA_peripherals->mBG0HOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG0VOFS:
	{
		pGBA_peripherals->mBG0VOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG1HOFS:
	{
		pGBA_peripherals->mBG1HOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG1VOFS:
	{
		pGBA_peripherals->mBG1VOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG2HOFS:
	{
		pGBA_peripherals->mBG2HOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG2VOFS:
	{
		pGBA_peripherals->mBG2VOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG3HOFS:
	{
		pGBA_peripherals->mBG3HOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG3VOFS:
	{
		pGBA_peripherals->mBG3VOFSHalfWord.mBGniOFSHalfWord = data;
		RETURN;
	}
	case IO_BG2PA:
	{
		pGBA_peripherals->mBG2PAHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG2PB:
	{
		pGBA_peripherals->mBG2PBHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG2PC:
	{
		pGBA_peripherals->mBG2PCHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG2PD:
	{
		pGBA_peripherals->mBG2PDHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG2X_L:
	{
		pGBA_peripherals->mBG2XWord.mBGniHalfWords.mBGniWord_L = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG2].internalRefPointRegisters.bgxOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG2X_H:
	{
		pGBA_peripherals->mBG2XWord.mBGniHalfWords.mBGniWord_H = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG2].internalRefPointRegisters.bgxOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG2Y_L:
	{
		pGBA_peripherals->mBG2YWord.mBGniHalfWords.mBGniWord_L = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG2].internalRefPointRegisters.bgyOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG2Y_H:
	{
		pGBA_peripherals->mBG2YWord.mBGniHalfWords.mBGniWord_H = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG2].internalRefPointRegisters.bgyOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG3PA:
	{
		pGBA_peripherals->mBG3PAHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG3PB:
	{
		pGBA_peripherals->mBG3PBHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG3PC:
	{
		pGBA_peripherals->mBG3PCHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG3PD:
	{
		pGBA_peripherals->mBG3PDHalfWord.mBGnPxHalfWord = data;
		RETURN;
	}
	case IO_BG3X_L:
	{
		pGBA_peripherals->mBG3XWord.mBGniHalfWords.mBGniWord_L = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG3].internalRefPointRegisters.bgxOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG3X_H:
	{
		pGBA_peripherals->mBG3XWord.mBGniHalfWords.mBGniWord_H = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG3].internalRefPointRegisters.bgxOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG3Y_L:
	{
		pGBA_peripherals->mBG3YWord.mBGniHalfWords.mBGniWord_L = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG3].internalRefPointRegisters.bgyOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_BG3Y_H:
	{
		pGBA_peripherals->mBG3YWord.mBGniHalfWords.mBGniWord_H = data;
		// NOTE: We need to detect any writes to few of these these registers
		// BG2 and BG3 registers are defined in our IO memory array as follows:
		// Address		|		8 bit		|  16 bit		| 32 bit		| Info
		// _________________________________________________________________|________________
		// 0x4000024	|	index 36		| index 18		| index 9		|
		// 0x4000025	|	index 37		| NA			| NA			|
		// 0x4000026	|	index 38		| index 19		| NA			|
		// 0x4000027	|	index 39		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000028	|	index 40		| index 20		| index 10		| BG2 X
		// 0x4000029	|	index 41		| NA			| NA			|
		// 0x400002A	|	index 42		| index 21		| NA			|
		// 0x400002B	|	index 43		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x400002C	|	index 44		| index 22		| index 11		| BG2 Y
		// 0x400002D	|	index 45		| NA			| NA			|
		// 0x400002E	|	index 46		| index 23		| NA			|
		// 0x400002F	|	index 47		| NA			| NA			|
		// _________________________________________________________________|________________
		// 0x4000030	|	index 44		| index 24		| index 12		|
		// 0x4000031	|	index 45		| NA			| NA			|
		// 0x4000032	|	index 46		| index 25		| NA			|
		// 0x4000033	|	index 47		| NA			| NA			|
		pGBA_display->bgCache[BG3].internalRefPointRegisters.bgyOverwrittenByCPU = YES;
		RETURN;
	}
	case IO_WIN0H:
	{
		pGBA_peripherals->mWIN0HHalfWord.mWINniHalfWord = data;
		RETURN;
	}
	case IO_WIN1H:
	{
		pGBA_peripherals->mWIN1HHalfWord.mWINniHalfWord = data;
		RETURN;
	}
	case IO_WIN0V:
	{
		pGBA_peripherals->mWIN0VHalfWord.mWINniHalfWord = data;
		RETURN;
	}
	case IO_WIN1V:
	{
		pGBA_peripherals->mWIN1VHalfWord.mWINniHalfWord = data;
		RETURN;
	}
	case IO_WININ:
	{
		pGBA_peripherals->mWININHalfWord.mWININHalfWord = data;
		RETURN;
	}
	case IO_WINOUT:
	{
		pGBA_peripherals->mWINOUTHalfWord.mWINOUTHalfWord = data;
		RETURN;
	}
	case IO_MOSAIC:
	{
		pGBA_peripherals->mMOSAICHalfWord.mMOSAICHalfWord = data;
		RETURN;
	}
	case IO_400004E:
	{
		RETURN;
	}
	case IO_BLDCNT:
	{
		pGBA_peripherals->mBLDCNTHalfWord.mBLDCNTHalfWord = data;
		RETURN;
	}
	case IO_BLDALPHA:
	{
		pGBA_peripherals->mBLDALPHAHalfWord.mBLDALPHAHalfWord = data;
		RETURN;
	}
	case IO_BLDY:
	{
		REJECT_IO8(accessWidth);
		pGBA_peripherals->mBLDYHalfWord.mBLDYHalfWord = data;
		RETURN;
	}
	case IO_4000056:
	case IO_4000058:
	case IO_400005A:
	case IO_400005C:
	case IO_400005E:
	{
		RETURN;
	}
	case IO_SOUND1CNT_L:
	{
		REJECT_IO8(accessWidth);
		BYTE nr10 = pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord & 0xFF;
		pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord = data;

		// writing to Sound channel 1 sweep
		//if (nr10 != (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord |= nr10;
			}
			// NOTE: One of the weird quirks of frequency sweep
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			else if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_FREQ_DIR == ZERO
				&& pGBA_instance->GBA_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger == YES)
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = ZERO;
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.INITIAL = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
			}
		}

		RETURN;
	}
	case IO_SOUND1CNT_H:
	{
		BYTE nr11 = pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord & 0xFF;
		BYTE nr12 = (pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord = data;

		// writing to Sound channel 1 length timer & duty cycle
		//if (nr11 != (pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord |= nr11;
			}
			else
			{

				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer
					= 64 - pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.SOUND_LENGTH;
			}
		}

		// writing to Sound channel 1 volume & envelope
		//if (nr12 != ((pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HHalfWord |= (nr12 << EIGHT);
			}
			else
			{
				FLAG wasVolumePeriodZero = ((nr12 & 0b111) == ZERO);
				FLAG wasEnvelopeInSubtractMode = ((nr12 & 0b1000) == ZERO);
				// NOTE: One of the weird quirks of APU called the "Zombie Mode"
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (isAudioChannelEnabled(AUDIO_CHANNELS::CHANNEL_1) == YES)
				{
					if (wasVolumePeriodZero == YES
						&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isVolumeEnvelopeStillDoingAutomaticUpdates == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
					}
					else if (wasEnvelopeInSubtractMode == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
					}
					if (
						(wasEnvelopeInSubtractMode == YES
							&& pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_DIR == ONE)
						||
						(wasEnvelopeInSubtractMode == NO
							&& pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_DIR == ZERO))
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume
							= SIXTEEN - pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume;
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume
						&= 0x0F;
				}
				continousDACCheck();
			}
		}

		RETURN;
	}
	case IO_SOUND1CNT_X:
	{
		BYTE nr13 = pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord & 0xFF;
		BYTE nr14 = (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord = data;

		// writing to Sound channel 1 period low
		//if (nr13 != (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord |= nr13;
			}
		}

		// writing to Sound channel 1 period high & control
		//if (nr14 != ((pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XHalfWord |= (nr14 << EIGHT);
			}
			else
			{
				FLAG wasLengthEnableBitZero = ((nr14 & 0b1000000) == ZERO);
				// NOTE: One of the weird quirks of length counter
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (wasLengthEnableBitZero == YES
					&& pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.LENGTH_FLAG == ONE
					&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE
					&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer > ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer;
					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer == ZERO)
					{
						pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = ZERO;
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
					}
				}
				if (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.INITIAL == ONE)
				{
					enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_1);
					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer == ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer = 64;
						// NOTE: One of the weird quirks of length counter
						// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
						if (pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.LENGTH_FLAG == ONE
							&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
						{
							--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer;
						}
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer
						= pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_STEP_TIME;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].currentVolume
						= pGBA_peripherals->mSOUND1CNT_HHalfWord.mSOUND1CNT_HFields.ENVP_INIT_VOL;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isVolumeEnvelopeStillDoingAutomaticUpdates = YES;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].shadowFrequency
						= getChannelPeriod(AUDIO_CHANNELS::CHANNEL_1);

					if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME > ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepTimer
							= pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME;
					}
					else
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepTimer = EIGHT;
					}
					if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_TIME > ZERO
						|| pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_SHIFT > ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepEnabled = ENABLED;
					}
					else
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].sweepEnabled = DISABLED;
					}
					pGBA_instance->GBA_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger = NO;
					if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_SHIFT > ZERO)
					{
						performOverFlowCheck();
					}

					// Source : https://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu
					uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_1)) * FOUR;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].frequencyTimer
						= (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].frequencyTimer & THREE)
						+ resetFrequencyTimer;
				}
			}
		}

		RETURN;
	}
	case IO_4000066:
	{
		RETURN;
	}
	case IO_SOUND2CNT_L:
	{
		BYTE nr21 = pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord & 0xFF;
		BYTE nr22 = (pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord = data;

		// writing to Sound channel 2 length timer & duty cycle
		//if (nr21 != (pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord |= nr21;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer
					= 64 - pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.SOUND_LENGTH;
			}
		}

		// writing to Sound channel 2 volume & envelope
		//if (nr22 != ((pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LHalfWord |= (nr22 << EIGHT);
			}
			else
			{
				FLAG wasVolumePeriodZero = ((nr22 & 0b111) == ZERO);
				FLAG wasEnvelopeInSubtractMode = ((nr22 & 0b1000) == ZERO);
				// NOTE: One of the weird quirks of APU called the "Zombie Mode"
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (isAudioChannelEnabled(AUDIO_CHANNELS::CHANNEL_2) == YES)
				{
					if (wasVolumePeriodZero == YES
						&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isVolumeEnvelopeStillDoingAutomaticUpdates == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
					}
					else if (wasEnvelopeInSubtractMode == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
					}
					if (
						(wasEnvelopeInSubtractMode == YES
							&& pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_DIR == ONE)
						||
						(wasEnvelopeInSubtractMode == NO
							&& pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_DIR == ZERO))
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume
							= SIXTEEN - pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume;
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume &= 0x0F;
				}
				continousDACCheck();
			}
		}

		RETURN;
	}
	case IO_400006A:
	{
		RETURN;
	}
	case IO_SOUND2CNT_H:
	{
		BYTE nr23 = pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord & 0xFF;
		BYTE nr24 = (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord = data;

		// writing to Sound channel 2 period low
		//if (nr23 != (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord |= nr23;
			}
		}

		// writing to Sound channel 2 period high & control
		//if (nr24 != ((pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HHalfWord |= (nr24 << EIGHT);
			}
			else
			{
				FLAG wasLengthEnableBitZero = ((nr24 & 0b1000000) == ZERO);
				// NOTE: One of the weird quirks of length counter
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (wasLengthEnableBitZero == YES
					&& pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.LENGTH_FLAG == ONE
					&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE
					&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer > ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer;

					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer == ZERO)
					{
						pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = ZERO;
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
					}
				}
				if (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.INITIAL == ONE)
				{
					enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_2);
					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer == ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer = 64;
						// NOTE: One of the weird quirks of length counter
						// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
						if (pGBA_peripherals->mSOUND2CNT_HHalfWord.mSOUND2CNT_HFields.LENGTH_FLAG == ONE
							&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
						{
							--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer;
						}
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer
						= pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_STEP_TIME;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].currentVolume
						= pGBA_peripherals->mSOUND2CNT_LHalfWord.mSOUND2CNT_LFields.ENVP_INIT_VOL;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isVolumeEnvelopeStillDoingAutomaticUpdates = YES;

					// Source : https://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu
					uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_2)) * FOUR;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].frequencyTimer
						= (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].frequencyTimer & THREE)
						+ resetFrequencyTimer;
				}
			}
		}

		RETURN;
	}
	case IO_400006E:
	{
		RETURN;
	}
	case IO_SOUND3CNT_L:
	{
		BYTE nr30 = pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord & 0xFF;
		BYTE currentWB = pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_BANK_NUMBER;
		BYTE currentWD = pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_DIMENSION;
		pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord = data;

		// writing to Sound channel 3 DAC enable
		//if (nr30 != (pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LHalfWord |= nr30;
			}
			else
			{
				continousDACCheck();

				if (currentWB != pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_BANK_NUMBER)
				{
					std::swap_ranges(std::begin(pGBA_peripherals->mWAVERAM8), std::end(pGBA_peripherals->mWAVERAM8), std::begin(pGBA_memory->mBankedWAVERAM.mWAVERAM8));
				}

				if (currentWD != pGBA_peripherals->mSOUND3CNT_LHalfWord.mSOUND3CNT_LFields.WAVE_RAM_DIMENSION)
				{
					;
				}
			}
		}

		RETURN;
	}
	case IO_SOUND3CNT_H:
	{
		BYTE nr31 = pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord & 0xFF;
		BYTE nr32 = (pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord = data;

		// writing to Sound channel 3 length timer
		//if (nr31 != (pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord |= nr31;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer = 256 - nr31;
			}
		}

		// Sound channel 3 output level
		//if (nr32 != ((pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HHalfWord |= (nr32 << EIGHT);
			}
			else
			{
				if (pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HFields.FORCE_VOL == ONE)
				{
					pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = THREE;
				}
				else
				{
					switch (pGBA_peripherals->mSOUND3CNT_HHalfWord.mSOUND3CNT_HFields.SOUND_VOL)
					{
					case ZERO:
						pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = ZERO;
						BREAK;
					case ONE:
						pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = FOUR;
						BREAK;
					case TWO:
						pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = TWO;
						BREAK;
					case THREE:
						pGBA_instance->GBA_state.audio.channel3OutputLevelAndShift = ONE;
						BREAK;
					}
				}
			}
		}

		RETURN;
	}
	case IO_SOUND3CNT_X:
	{
		BYTE nr33 = pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord & 0xFF;
		BYTE nr34 = (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord = data;

		// Sound channel 3 period low
		//if (nr33 != (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord |= nr33;
			}
		}

		// writing to Sound channel 3 period high & control
		//if (nr34 != ((pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XHalfWord |= (nr34 << EIGHT);
			}
			else
			{
				FLAG wasLengthEnableBitZero = ((nr34 & 0b1000000) == ZERO);
				// NOTE: One of the weird quirks of length counter
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (wasLengthEnableBitZero == YES
					&& pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.LENGTH_FLAG == ONE
					&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE
					&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer > ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer;

					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer == ZERO)
					{
						pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = ZERO;
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
					}
				}
				if (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.INITIAL == ONE)
				{
					enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_3);

					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer == ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer = 256;

						// NOTE: One of the weird quirks of length counter
						// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
						if (pGBA_peripherals->mSOUND3CNT_XHalfWord.mSOUND3CNT_XFields.LENGTH_FLAG == ONE
							&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
						{
							--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer;
						}
					}

					// Period needs to be reloaded as per https://gbdev.io/pandocs/Audio_Registers.html#ff1e--nr34-channel-3-period-high--control
					uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_3)) * TWO;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].frequencyTimer = resetFrequencyTimer;
					// As per https://forums.nesdev.org/viewtopic.php?p=188035#p188035, another 6 cycle delay is needed
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].frequencyTimer += SIX;

					pGBA_instance->GBA_state.audio.waveRamCurrentIndex = RESET;
					pGBA_instance->GBA_state.audio.didChannel3ReadWaveRamPostTrigger = NO;
				}
			}
		}

		RETURN;
	}
	case IO_4000076:
	{
		RETURN;
	}
	case IO_SOUND4CNT_L:
	{
		BYTE nr41 = pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord & 0xFF;
		BYTE nr42 = (pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord = data;

		// writing to Sound channel 4 length timer
		//if (nr41 != (pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord |= nr41;
			}
			else
			{
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer
					= 64 - pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.SOUND_LENGTH;
			}
		}

		// writing to Sound channel 4 volume & envelope
		//if (nr42 != ((pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LHalfWord |= (nr42 << EIGHT);
			}
			else
			{
				FLAG wasVolumePeriodZero = ((nr42 & 0b111) == ZERO);
				FLAG wasEnvelopeInSubtractMode = ((nr42 & 0b1000) == ZERO);
				// NOTE: One of the weird quirks of APU called the "Zombie Mode"
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (isAudioChannelEnabled(AUDIO_CHANNELS::CHANNEL_4) == YES)
				{
					if (wasVolumePeriodZero == YES
						&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isVolumeEnvelopeStillDoingAutomaticUpdates == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
					}
					else if (wasEnvelopeInSubtractMode == YES)
					{
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
						++pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
					}
					if (
						(wasEnvelopeInSubtractMode == YES
							&& pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_DIR == ONE)
						||
						(wasEnvelopeInSubtractMode == NO
							&& pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_DIR == ZERO))
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume
							= SIXTEEN - pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume;
					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume &= 0x0F;
				}
				continousDACCheck();
			}
		}

		RETURN;
	}
	case IO_400007A:
	{
		RETURN;
	}
	case IO_SOUND4CNT_H:
	{
		BYTE nr43 = pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord & 0xFF;
		BYTE nr44 = (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord = data;

		// Sound channel 4 frequency & randomness
		//if (nr43 != (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord |= nr43;
			}
		}

		// writing to Sound channel 4 control
		//if (nr44 != ((pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HHalfWord |= (nr44 << EIGHT);
			}
			else
			{
				FLAG wasLengthEnableBitZero = ((nr44 & 0b1000000) == ZERO);
				// NOTE: One of the weird quirks of length counter
				// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				if (wasLengthEnableBitZero == YES
					&& pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.LENGTH_FLAG == ONE
					&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE
					&& pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer > ZERO)
				{
					--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer;

					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer == ZERO)
					{
						pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = ZERO;
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
					}
				}
				if (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.INITIAL == ONE)
				{
					enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_4);
					if (pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer == ZERO)
					{
						pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer = 64;
						// NOTE: One of the weird quirks of length counter
						// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
						if (pGBA_peripherals->mSOUND4CNT_HHalfWord.mSOUND4CNT_HFields.LENGTH_FLAG == ONE
							&& pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
						{
							--pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer;
						}

					}
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer
						= pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_STEP_TIME;

					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].currentVolume
						= pGBA_peripherals->mSOUND4CNT_LHalfWord.mSOUND4CNT_LFields.ENVP_INIT_VOL;
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isVolumeEnvelopeStillDoingAutomaticUpdates = YES;
					// Refer "trigger event" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].LFSR = 0x7FFF;

					// Source : https://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu
					pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].frequencyTimer += EIGHT;
				}
			}
		}

		RETURN;
	}
	case IO_400007E:
	{
		RETURN;
	}
	case IO_SOUNDCNT_L:
	{
		BYTE nr50 = pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord & 0xFF;
		BYTE nr51 = (pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord >> EIGHT) & 0xFF;
		pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord = data;

		// writing to Master volume & VIN panning
		//if (nr50 != (pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord |= nr50;
			}
		}

		// writing to Sound panning
		//if (nr51 != ((pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord >> EIGHT) & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord &= 0x00FF;
				pGBA_peripherals->mSOUNDCNT_LHalfWord.mSOUNDCNT_LHalfWord |= (nr51 << EIGHT);
			}
		}

		RETURN;
	}
	case IO_SOUNDCNT_H:
	{
		pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HHalfWord = data;
		pGBA_audio->FIFO[DIRECT_SOUND_A].timer = pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_A_TIMER_SEL;
		pGBA_audio->FIFO[DIRECT_SOUND_B].timer = pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_B_TIMER_SEL;

		if (pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_A_RESET_FIFO == ONE)
		{
			pGBA_audio->FIFO[DIRECT_SOUND_A].position = RESET;
			pGBA_audio->FIFO[DIRECT_SOUND_A].size = RESET;
		}
		if (pGBA_peripherals->mSOUNDCNT_HHalfWord.mSOUNDCNT_HFields.DMA_SOUND_B_RESET_FIFO == ONE)
		{
			pGBA_audio->FIFO[DIRECT_SOUND_B].position = RESET;
			pGBA_audio->FIFO[DIRECT_SOUND_B].size = RESET;
		}

		RETURN;
	}
	case IO_SOUNDCNT_X:
	{
		BYTE nr52 = pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XHalfWord & 0xFF;
		pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XHalfWord = data;

		// writing to Sound ON/OFF
		//if (nr52 != (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XHalfWord & 0xFF))
		{
			BYTE APU_POWER_WAS = pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN;
			pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN = GETBIT(7, data);
			// Channel ON -> Channel OFF
			if (APU_POWER_WAS == ONE && pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				for (uint32_t address = 0x04000060; address <= 0x040000AE /*0x04000075*/; address++)
				{
					pGBA_memory->mGBAMemoryMap.mIO.mIOMemory8bit[address - 0x04000000] = ZERO;
				}
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND2_ON_FLAG = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND3_ON_FLAG = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND4_ON_FLAG = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
			}
			// Channel OFF -> Channel ON
			else if (APU_POWER_WAS == ZERO && pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
			{
				// Reset the length counters in CGB mode during power up
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].lengthTimer = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_2].lengthTimer = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_3].lengthTimer = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_4].lengthTimer = ZERO;

				pGBA_instance->GBA_state.audio.wasPowerCycled = YES;

				// NOTE: As we are resetting the frame sequencer, next half period WILL clock the length counter
				pGBA_instance->GBA_state.audio.nextHalfWillNotClockLengthCounter = FALSE;
			}
		}

		RETURN;
	}
	case IO_4000086:
	{
		RETURN;
	}
	case IO_SOUNDBIAS:
	{
		pGBA_peripherals->mSOUNDBIASHalfWord.mSOUNDBIASHalfWord = data & ~1;
		RETURN;
	}
	case IO_400008A:
	{
		RETURN;
	}
	case IO_400008C:
	case IO_400008E:
	{
		RETURN;
	}
	case (IO_WAVERAM_START_ADDRESS + 0):
	case (IO_WAVERAM_START_ADDRESS + 2):
	case (IO_WAVERAM_START_ADDRESS + 4):
	case (IO_WAVERAM_START_ADDRESS + 6):
	case (IO_WAVERAM_START_ADDRESS + 8):
	case (IO_WAVERAM_START_ADDRESS + 10):
	case (IO_WAVERAM_START_ADDRESS + 12):
	case (IO_WAVERAM_START_ADDRESS + 14):
	{
		pGBA_peripherals->mWAVERAM16[(address - IO_WAVERAM_START_ADDRESS) / TWO].waveRamHalfWord = data;
		RETURN;
	}
	case IO_FIFO_A_L:
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
		{
			pGBA_peripherals->mFIFOA_L = data;
			pGBA_audio->FIFO[DIRECT_SOUND_A].fifoByteWrite((data >> ZERO) & 0xFF);
			pGBA_audio->FIFO[DIRECT_SOUND_A].fifoByteWrite((data >> EIGHT) & 0xFF);
		}
		RETURN;
	}
	case IO_FIFO_A_H:
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
		{
			pGBA_peripherals->mFIFOA_H = data;
			pGBA_audio->FIFO[DIRECT_SOUND_A].fifoByteWrite((data >> ZERO) & 0xFF);
			pGBA_audio->FIFO[DIRECT_SOUND_A].fifoByteWrite((data >> EIGHT) & 0xFF);
		}
		RETURN;
	}
	case IO_FIFO_B_L:
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
		{
			pGBA_peripherals->mFIFOB_L = data;
			pGBA_audio->FIFO[DIRECT_SOUND_B].fifoByteWrite((data >> ZERO) & 0xFF);
			pGBA_audio->FIFO[DIRECT_SOUND_B].fifoByteWrite((data >> EIGHT) & 0xFF);
		}
		RETURN;
	}
	case IO_FIFO_B_H:
	{
		if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ONE)
		{
			pGBA_peripherals->mFIFOB_H = data;
			pGBA_audio->FIFO[DIRECT_SOUND_B].fifoByteWrite((data >> ZERO) & 0xFF);
			pGBA_audio->FIFO[DIRECT_SOUND_B].fifoByteWrite((data >> EIGHT) & 0xFF);
		}
		RETURN;
	}
	case IO_40000A8:
	case IO_40000AA:
	case IO_40000AC:
	case IO_40000AE:
	{
		RETURN;
	}
	case IO_DMA0SAD_L:
	{
		pGBA_peripherals->mDMA0SAD_L = data;
		RETURN;
	}
	case IO_DMA0SAD_H:
	{
		pGBA_peripherals->mDMA0SAD_H = data;
		RETURN;
	}
	case IO_DMA0DAD_L:
	{
		pGBA_peripherals->mDMA0DAD_L = data;
		RETURN;
	}
	case IO_DMA0DAD_H:
	{
		pGBA_peripherals->mDMA0DAD_H = data;
		RETURN;
	}
	case IO_DMA0CNT_L:
	{
		pGBA_peripherals->mDMA0CNT_L = data;
		RETURN;
	}
	case IO_DMA0CNT_H:
	{
		//if (source == MEMORY_ACCESS_SOURCE::DMA)
		//{
		//	INFO("DMA updating DMA");
		//}

		FLAG oldEnable = pGBA_peripherals->mDMA0CNT_H.mDMAnCNT_HFields.DMA_EN;
		pGBA_peripherals->mDMA0CNT_H.mDMAnCNT_HHalfWord = data;
		FLAG newEnable = pGBA_peripherals->mDMA0CNT_H.mDMAnCNT_HFields.DMA_EN;

		OnDMAChannelWritten(DMA::DMA0, oldEnable, newEnable);
		RETURN;
	}
	case IO_DMA1SAD_L:
	{
		pGBA_peripherals->mDMA1SAD_L = data;
		RETURN;
	}
	case IO_DMA1SAD_H:
	{
		pGBA_peripherals->mDMA1SAD_H = data;
		RETURN;
	}
	case IO_DMA1DAD_L:
	{
		pGBA_peripherals->mDMA1DAD_L = data;
		RETURN;
	}
	case IO_DMA1DAD_H:
	{
		pGBA_peripherals->mDMA1DAD_H = data;
		RETURN;
	}
	case IO_DMA1CNT_L:
	{
		pGBA_peripherals->mDMA1CNT_L = data;
		RETURN;
	}
	case IO_DMA1CNT_H:
	{
		//if (source == MEMORY_ACCESS_SOURCE::DMA)
		//{
		//	INFO("DMA updating DMA");
		//}

		FLAG oldEnable = pGBA_peripherals->mDMA1CNT_H.mDMAnCNT_HFields.DMA_EN;
		pGBA_peripherals->mDMA1CNT_H.mDMAnCNT_HHalfWord = data;
		FLAG newEnable = pGBA_peripherals->mDMA1CNT_H.mDMAnCNT_HFields.DMA_EN;

		OnDMAChannelWritten(DMA::DMA1, oldEnable, newEnable);
		RETURN;
	}
	case IO_DMA2SAD_L:
	{
		pGBA_peripherals->mDMA2SAD_L = data;
		RETURN;
	}
	case IO_DMA2SAD_H:
	{
		pGBA_peripherals->mDMA2SAD_H = data;
		RETURN;
	}
	case IO_DMA2DAD_L:
	{
		pGBA_peripherals->mDMA2DAD_L = data;
		RETURN;
	}
	case IO_DMA2DAD_H:
	{
		pGBA_peripherals->mDMA2DAD_H = data;
		RETURN;
	}
	case IO_DMA2CNT_L:
	{
		pGBA_peripherals->mDMA2CNT_L = data;
		RETURN;
	}
	case IO_DMA2CNT_H:
	{
		//if (source == MEMORY_ACCESS_SOURCE::DMA)
		//{
		//	INFO("DMA updating DMA");
		//}

		FLAG oldEnable = pGBA_peripherals->mDMA2CNT_H.mDMAnCNT_HFields.DMA_EN;
		pGBA_peripherals->mDMA2CNT_H.mDMAnCNT_HHalfWord = data;
		FLAG newEnable = pGBA_peripherals->mDMA2CNT_H.mDMAnCNT_HFields.DMA_EN;

		OnDMAChannelWritten(DMA::DMA2, oldEnable, newEnable);
		RETURN;
	}
	case IO_DMA3SAD_L:
	{
		pGBA_peripherals->mDMA3SAD_L = data;
		RETURN;
	}
	case IO_DMA3SAD_H:
	{
		pGBA_peripherals->mDMA3SAD_H = data;
		RETURN;
	}
	case IO_DMA3DAD_L:
	{
		pGBA_peripherals->mDMA3DAD_L = data;
		RETURN;
	}
	case IO_DMA3DAD_H:
	{
		pGBA_peripherals->mDMA3DAD_H = data;
		RETURN;
	}
	case IO_DMA3CNT_L:
	{
		pGBA_peripherals->mDMA3CNT_L = data;
		RETURN;
	}
	case IO_DMA3CNT_H:
	{
		//if (source == MEMORY_ACCESS_SOURCE::DMA)
		//{
		//	INFO("DMA updating DMA");
		//}

		FLAG oldEnable = pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN;
		pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HHalfWord = data;
		FLAG newEnable = pGBA_peripherals->mDMA3CNT_H.mDMAnCNT_HFields.DMA_EN;

		OnDMAChannelWritten(DMA::DMA3, oldEnable, newEnable);
		RETURN;
	}
	case IO_40000E0:
	case IO_40000E2:
	case IO_40000E4:
	case IO_40000E6:
	case IO_40000E8:
	case IO_40000EA:
	case IO_40000EC:
	case IO_40000EE:
	case IO_40000F0:
	case IO_40000F2:
	case IO_40000F4:
	case IO_40000F6:
	case IO_40000F8:
	case IO_40000FA:
	case IO_40000FC:
	case IO_40000FE:
	{
		RETURN;
	}
#if (GBA_ENABLE_DELAYED_TIMER_REG == YES)
	case IO_TM0CNT_L:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, ZERO);
		pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.io_tmxcnt_l = data;
		RETURN;
	}
	case IO_TM0CNT_H:
	{
		REJECT_IO8(accessWidth);
		SETBIT(pGBA_instance->GBA_state.timerPendMap, ONE);
		pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.io_tmxcnt_h = data;
		RETURN;
	}
	case IO_TM1CNT_L:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, TWO);
		pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.io_tmxcnt_l = data;
		RETURN;
	}
	case IO_TM1CNT_H:
	{
		REJECT_IO8(accessWidth);
		SETBIT(pGBA_instance->GBA_state.timerPendMap, THREE);
		pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.io_tmxcnt_h = data;
		RETURN;
	}
	case IO_TM2CNT_L:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, FOUR);
		pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.io_tmxcnt_l = data;
		RETURN;
	}
	case IO_TM2CNT_H:
	{
		REJECT_IO8(accessWidth);
		SETBIT(pGBA_instance->GBA_state.timerPendMap, FIVE);
		pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.io_tmxcnt_h = data;
		RETURN;
	}
	case IO_TM3CNT_L:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, SIX);
		pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.io_tmxcnt_l = data;
		RETURN;
	}
	case IO_TM3CNT_H:
	{
		REJECT_IO8(accessWidth);
		SETBIT(pGBA_instance->GBA_state.timerPendMap, SEVEN);
		pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.io_tmxcnt_h = data;
		RETURN;
	}
#else
	case IO_TM0CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM0CNT_H:
	{
		BIT timer0EnBeforeUpdate = pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HHalfWord = data;

		// Handles loading of "reload" to "counter" when timer is enabled (0 -> 1)
		if (timer0EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER0CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.counter = pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.reload;
			pGBA_peripherals->mTIMER0CNT_L = pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.timerCounter[TIMER::TIMER0] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TIMER::TIMER0].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TIMER::TIMER0].startupDelay = TWO;
		}
		RETURN;
	}
	case IO_TM1CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM1CNT_H:
	{
		BIT timer1EnBeforeUpdate = pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HHalfWord = data;
		if (timer1EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER1CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.counter = pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.reload;
			pGBA_peripherals->mTIMER1CNT_L = pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.timerCounter[TIMER::TIMER1] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TIMER::TIMER1].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TIMER::TIMER1].startupDelay = TWO;
		}
		RETURN;
	}
	case IO_TM2CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM2CNT_H:
	{
		BIT timer2EnBeforeUpdate = pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HHalfWord = data;
		if (timer2EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER2CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.counter = pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.reload;
			pGBA_peripherals->mTIMER2CNT_L = pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.timerCounter[TIMER::TIMER2] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TIMER::TIMER2].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TIMER::TIMER2].startupDelay = TWO;
		}
		RETURN;
	}
	case IO_TM3CNT_L:
	{
		// Write should directly happen to "reload" instead of the actual mTIMERxCNT_L)
		pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.reload = data; // Store the new value in "reload"
		RETURN;
	}
	case IO_TM3CNT_H:
	{
		BIT timer3EnBeforeUpdate = pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP;
		pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HHalfWord = data;
		if (timer3EnBeforeUpdate == RESET && pGBA_peripherals->mTIMER3CNT_H.mTIMERnCNT_HFields.TIMER_START_STOP == SET)
		{
			pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.counter = pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.reload;
			pGBA_peripherals->mTIMER3CNT_L = pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.counter;
			pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.timerCounter[TIMER::TIMER3] = RESET;
			// Takes 2 cycles for CNTH to get applied after writing to control/reload
			// Refer : https://discordapp.com/channels/465585922579103744/465586361731121162/1034239922602782801
			pGBA_instance->GBA_state.timer[TIMER::TIMER3].currentState = DISABLED;
			TODO("For some reason, instead of 2, we need to put 3 for prescalar tests to pass in AGS");
			pGBA_instance->GBA_state.timer[TIMER::TIMER3].startupDelay = TWO;
		}
		RETURN;
	}
#endif
	case IO_SIOMULTI0:
	{
		pGBA_peripherals->mSIOMULTI0 = data;
		RETURN;
	}
	case IO_SIOMULTI1:
	{
		pGBA_peripherals->mSIOMULTI1 = data;
		RETURN;
	}
	case IO_SIOMULTI2:
	{
		pGBA_peripherals->mSIOMULTI2 = data;
		RETURN;
	}
	case IO_SIOMULTI3:
	{
		pGBA_peripherals->mSIOMULTI3 = data;
		RETURN;
	}
	case IO_SIOCNT:
	{
		pGBA_peripherals->mSIOCNT.mSIOCNTHalfWord = data;
		RETURN;
	}
	case IO_SIO_DATA8_MLTSEND:
	{
		pGBA_peripherals->mSIO_DATA8_MLTSEND = data;
		RETURN;
	}
	case IO_KEYINPUT:
	{
		RETURN;
	}
	case IO_KEYCNT:
	{
		pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTHalfWord = data;
		handleKeypadInterrupts();
		RETURN;
	}
	case IO_RCNT:
	{
		pGBA_peripherals->mRCNTHalfWord.mRCNTHalfWord = data;
		RETURN;
	}
	case IO_IR:
	{
		pGBA_peripherals->mIRHalfWord.mIRHalfWord = data;
		RETURN;
	}
	case IO_JOYCNT:
	{
		pGBA_peripherals->mJOYCNTHalfWord.mJOYCNTHalfWord = data;
		RETURN;
	}
	case IO_4000142:
	{
		RETURN;
	}
	case IO_JOY_RECV_L:
	{
		pGBA_peripherals->mJOY_RECV_L = data;
		RETURN;
	}
	case IO_JOY_RECV_H:
	{
		pGBA_peripherals->mJOY_RECV_H = data;
		RETURN;
	}
	case IO_JOY_TRANS_L:
	{
		pGBA_peripherals->mJOY_TRANS_L = data;
		RETURN;
	}
	case IO_JOY_TRANS_H:
	{
		pGBA_peripherals->mJOY_TRANS_H = data;
		RETURN;
	}
	case IO_JOYSTAT:
	{
		pGBA_peripherals->mJOYSTATHalfWord.mJOYSTATHalfWord = data;
		RETURN;
	}
	case IO_400015A:
	{
		RETURN;
	}
	case IO_IE:
	{
#if (GBA_ENABLE_DELAYED_IRQ == YES)
		pGBA_instance->GBA_state.interrupt.iePend = data;
#else
		pGBA_peripherals->mIEHalfWord.mIEHalfWord = data;
#endif
		RETURN;
	}
	case IO_IF:
	{
		// For IF: Refer http://problemkaputt.de/gbatek-gba-interrupt-control.htm
		// Writing from CPU is always an ACK, so this is basically W1C register for CPU
		// Masters other than CPU should be able to write this without triggering W1C
#if (GBA_ENABLE_DELAYED_IRQ == YES)
		pGBA_instance->GBA_state.interrupt.ifPend &= ~data;
#else
		pGBA_peripherals->mIFHalfWord.mIFHalfWord &= ~data;
#endif
		RETURN;
	}
	case IO_WAITCNT:
	{
		// Refer to https://problemkaputt.de/gbatek-gba-system-control.htm

		pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTHalfWord = data;

		auto NonSequentialWaitStates = [&](uint32_t x) {
			switch (x)
			{
			case ZERO:  RETURN FIVE;  // 4 + 1
			case ONE:   RETURN FOUR;  // 3 + 1
			case TWO:   RETURN THREE; // 2 + 1
			case THREE: RETURN NINE;  // 8 + 1
			default: FATAL("Invalid nonsequential WAITCNT setting: %d!", x); RETURN ZERO;
			}
			};

		// WS0
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_NON_SEQ);
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= (pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_0_SEQ == 1) ? TWO : THREE;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			+ WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] * TWO;

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];

		// 8-bit uses same as 16-bit
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		// WS1
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_NON_SEQ);
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= (pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_1_SEQ == 1) ? TWO : FIVE;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			+ WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] * TWO;

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];

		// 8-bit uses same as 16-bit
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM0_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM1_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		// WS2
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_NON_SEQ);
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= (pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.WAIT_STATE_2_SEQ == 1) ? TWO : NINE;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT]
			+ WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] * TWO;

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT];

		// 8-bit uses same as 16-bit
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];
		WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_H][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT]
			= WAIT_CYCLES[MEMORY_REGIONS::REGION_FLASH_ROM2_L][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT];

		// SRAM (same timing for N and S)
		uint32_t sramCycles = NonSequentialWaitStates(pGBA_peripherals->mWAITCNTHalfWord.mWAITCNTFields.SRAM_WAIT_CTRL);

		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT] = sramCycles;

		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::EIGHT_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::SIXTEEN_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT] = sramCycles;
		WAIT_CYCLES[MEMORY_REGIONS::REGION_GAMEPAK_SRAM_MIRR][MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE][MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT] = sramCycles;

		RETURN;
	}
	case IO_4000206:
	{
		RETURN;
	}
	case IO_IME:
	{
#if (GBA_ENABLE_DELAYED_IRQ == YES)
		pGBA_instance->GBA_state.interrupt.imePend = data;
#else
		pGBA_peripherals->mIMEHalfWord.mIMEHalfWord = data;
		pGBA_peripherals->mIMEHalfWord.mIMEFields.NOT_USED_0 = ZERO;
#endif
		RETURN;
	}
	case IO_400020A:
	{
		RETURN;
	}
	case IO_4000302:
	{
		RETURN;
	}
	default:
	{
		pGBA_memory->mGBAMemoryMap.mIO.mIOMemory16bit[(address - IO_START_ADDRESS) / TWO] = data;
		RETURN;
	}
	}
}

MASQ_INLINE void GBA_t::writeIO8(uint32_t address, BYTE data, MEMORY_ACCESS_WIDTH accessWidth, MEMORY_ACCESS_SOURCE source, MEMORY_ACCESS_TYPE accessType)
{
	switch (address)
	{
	case IO_BLDY:
	{
		pGBA_peripherals->mBLDYHalfWord.mBLDYFields.EVY_COEFF = data & 0x1F;
		RETURN;
	}
	case IO_SOUND1CNT_L:
	{
		BYTE nr10 = pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord & 0xFF;
		pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord = data;

		// writing to Sound channel 1 sweep
		//if (nr10 != (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord & 0xFF))
		{
			if (pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.PSG_FIFO_MASTER_EN == ZERO)
			{
				pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord &= 0xFF00;
				pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LHalfWord |= nr10;
			}
			// NOTE: One of the weird quirks of frequency sweep
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			else if (pGBA_peripherals->mSOUND1CNT_LHalfWord.mSOUND1CNT_LFields.SWEEP_FREQ_DIR == ZERO
				&& pGBA_instance->GBA_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger == YES)
			{
				pGBA_peripherals->mSOUNDCNT_XHalfWord.mSOUNDCNT_XFields.SOUND1_ON_FLAG = ZERO;
				pGBA_peripherals->mSOUND1CNT_XHalfWord.mSOUND1CNT_XFields.INITIAL = ZERO;
				pGBA_instance->GBA_state.audio.audioChannelInstance[(uint8_t)AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
			}
		}

		RETURN;
	}
#if (GBA_ENABLE_DELAYED_TIMER_REG == YES)
	case IO_TM0CNT_H:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, ONE);
		pGBA_instance->GBA_state.timer[TIMER::TIMER0].cache.io_tmxcnt_h = data;
		RETURN;
	}
	case IO_TM1CNT_H:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, THREE);
		pGBA_instance->GBA_state.timer[TIMER::TIMER1].cache.io_tmxcnt_h = data;
		RETURN;
	}
	case IO_TM2CNT_H:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, FIVE);
		pGBA_instance->GBA_state.timer[TIMER::TIMER2].cache.io_tmxcnt_h = data;
		RETURN;
	}
	case IO_TM3CNT_H:
	{
		SETBIT(pGBA_instance->GBA_state.timerPendMap, SEVEN);
		pGBA_instance->GBA_state.timer[TIMER::TIMER3].cache.io_tmxcnt_h = data;
		RETURN;
	}
#endif
	case IO_POSTFLG:
	{
		CPUTODO("If condition of PC <= 0x3FFF at %d in %s is obtained from NBA. Find actual source of this information", __LINE__, __FILE__);

		if (cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC) <= 0x3FFF)
		{
			pGBA_peripherals->mPOSTFLGByte.mPOSTFLGFields.UNDOC = GETBIT(ZERO, data);
		}

		RETURN;
	}
	case IO_HALTCNT:
	{
		CPUTODO("If condition of PC <= 0x3FFF at %d in %s is obtained from NBA. Find actual source of this information", __LINE__, __FILE__);
		if (cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC) <= 0x3FFF)
		{
			if (data & 0x80)
			{
				pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::STOP;
				FATAL("Stop Mode is not supported");
			}
			else
			{
				// Refer "4000301h - HALTCNT - BYTE - Undocumented - Low Power Mode Control (W)" of  https://problemkaputt.de/gbatek-gba-system-control.htm
				CPUEVENT("[RUN] -> [HALT]");
				pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::HALT;
				busCycles();
			}
		}

		RETURN;
	}
	}
}

MASQ_INLINE void GBA_t::busCycles()
{
	cpuTick();

#if (ENABLE_ARM7TDMI_SST == YES)
	if (ROM_TYPE == ROM::TEST_SST) MASQ_UNLIKELY
	{
		// If this SST entry is already active, keep incrementing it
		if (sst.internal[sst.index].cycle != 0)
		{
			sst.internal[sst.index].cycle += ONE;
		}
	// First cycle of this SST entry
	else if (sst.index != RESET)
	{
		sst.internal[sst.index].cycle =
			sst.internal[sst.index - 1].cycle + ONE;
	}
	// Very first SST entry
	else
	{
		sst.internal[sst.index].cycle = ONE;
	}
	}
#endif
}

MASQ_INLINE void GBA_t::cpuIdleCycles()
{
	// Run DMA if any channel is active
	// Refer https://github.com/zaydlang/AGBEEG-Aging-Cartridge/blob/master/documentation/dma/cpu_runs_idles_throughout_dma.md
	if (IsAnyDMARunning() == YES)
	{
		dmaTick();
	}

	auto& ticks = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate;

	// Note that below method of using "free bus cycles" is inspired from NBA

	// Refill free internal cycles using DMA counter
	ticks.freeBusCyclesCounter += ticks.dmaCounter;
	ticks.dmaCounter = RESET;

	if (ticks.freeBusCyclesCounter == RESET)
	{
		// No DMA cycles pending, so execute a CPU cycle
		busCycles();
	}
	else
	{
		// There are pending DMA cycles to account for. 
		// Each unit in freeBusCyclesCounter represents a CPU cycle that should be skipped 
		// because a DMA transfer already consumed the bus. 
		// By decrementing this counter, we effectively delay CPU execution 
		// until all DMA cycles have been accounted for, ensuring correct timing 
		// between CPU and DMA activity.
		--ticks.freeBusCyclesCounter;
	}

	// Next access after internal cycle is always non-sequential
	// Ref: https://discord.com/channels/465585922579103744/465586361731121162/1269384605136322571
	pGBA_memory->setNextMemoryAccessType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
	pGBA_memory->setNextPipelineAccessType = MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE;
}

MASQ_INLINE void GBA_t::fetchAndDecode(uint32_t newPC)
{
	if (getARMState() == STATE_TYPE::ST_THUMB)
	{
		// New PC is being loaded, so we have the flush the contents of pipeline and reload all the stages again
		pGBA_cpuInstance->registers.pc = (newPC & 0xFFFFFFFE);

		// Stage 1:
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_HALFWORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU_INSTRUCTION_FETCH, MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE);
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		// Stage 2:
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_HALFWORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU_INSTRUCTION_FETCH, MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE);
		pGBA_cpuInstance->registers.pc += TWO;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFE;

		pGBA_memory->setNextPipelineAccessType = MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE;

		// Note: We still don't have the valid instruction in "executeStageOpCode"... for this, we need one more cycle i.e. Stage 3
		// We don't do Stage 3 here because by default, one cycle is always executed as part of of "runCPUPipeline"... so this can be considered Stage 3

		CPUINFO("[THUMB] Filling the instruction pipeline: {DECODE} 0x%04X: [0x%04X] | {FETCH} 0x%04X: [0x%04X]",
			pGBA_cpuInstance->registers.pc - TWO,
			pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode,
			pGBA_cpuInstance->registers.pc,
			pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode);
	}
	else if (getARMState() == STATE_TYPE::ST_ARM)
	{
		// New PC is being loaded, so we have the flush the contents of pipeline and reload all the stages again
		pGBA_cpuInstance->registers.pc = (newPC & 0xFFFFFFFC);

		// Stage 1:
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_WORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU_INSTRUCTION_FETCH, MEMORY_ACCESS_TYPE::NON_SEQUENTIAL_CYCLE);
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		// Stage 2:
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_WORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU_INSTRUCTION_FETCH, MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE);
		pGBA_cpuInstance->registers.pc += FOUR;
		pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

		pGBA_memory->setNextPipelineAccessType = MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE;

		// Note: We still don't have the valid instruction in "executeStageOpCode"... for this, we need one more cycle i.e. Stage 3
		// We don't do Stage 3 here because by default, one cycle is always executed as part of of "runCPUPipeline"... so this can be considered Stage 3

		CPUINFO("[ARM] Filling the instruction pipeline: {DECODE} 0x%08X: [0x%08X] | {FETCH} 0x%08X: [0x%08X]",
			pGBA_cpuInstance->registers.pc - FOUR,
			pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode,
			pGBA_cpuInstance->registers.pc,
			pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode);
	}
	else
	{
		FATAL("Unknown Operating Mode");
	}
}

MASQ_INLINE FLAG GBA_t::processSOC()
{
	FLAG status = true;

	INFRA("[Loop 0]: %" PRId64, gbaEmulationCounter[ZERO]);
	++gbaEmulationCounter[ZERO];

	INC64 cyclesInThisRun = RESET;
	INC64 dmaCyclesInThisRun = RESET;

	if (pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::RUN)
	{
		runCPUPipeline();
		cyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter;
	}
	else if (pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::HALT)
	{
		if (shouldUnHaltTheCPU() == NO && IsAnyDMARunning() == YES)
		{
			// Refer https://github.com/zaydlang/AGBEEG-Aging-Cartridge/blob/master/documentation/dma/cpu_runs_idles_throughout_dma.md

			// Cycles can never be zero; In actual device, clocks are always running
			// Since the emulation's clocks are based on CPU, even when other masters (eg: DMA) running, cycles can appear to be zero
			// Ideally, all master's should be running in parallel
			// However, our emulation works based on ticking the CPU, seeing how many cycles it ticked and ticking other masters by same cycles
			// But, when CPU is HALTED, we need an alternative source of cycles to tick other masters
			// We can use DMA for this if its is running, so basically when CPU is HALTED, "cpuCounter == dmaCounter (if dmaCounter != 0)"
			// If DMA is also not running, then we just tick by one until we reach vblank

			dmaTick();
		}

		dmaCyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter;

		// Refer to http://problemkaputt.de/gbatek-gba-system-control.htm
		if (shouldUnHaltTheCPU() == YES)
		{
			busCycles();
			cyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter;
			pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::RUN;
			CPUEVENT("[HALT] -> [RUN]");
		}

		// Some tick should happen to pull CPU out of HALT state... 
		// Hence we call busCycles incase both DMA and CPU ticks were zero
		if (dmaCyclesInThisRun == RESET && cyclesInThisRun == RESET)
		{
			busCycles();
			cyclesInThisRun = pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter;
		}
	}
	else
	{
		FATAL("CPU is in STOP mode");
	}

	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter = RESET;
	pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.dmaCounter = RESET;

	RETURN status;
}

MASQ_INLINE void GBA_t::runCPUPipeline()
{
	if (pGBA_cpuInstance->registers.pc >= GAMEPAK_ROM_WS0_START_ADDRESS)
	{
		pGBA_instance->GBA_state.emulatorStatus.isBiosExecutionDone = YES;
	}

	CPUINFRA("[Loop 1]: %" PRId64, gbaEmulationCounter[ONE]);
	++gbaEmulationCounter[ONE];

	handleInterruptsIfApplicable();

	CPUDEBUG("r0:  0x%08X   r1: 0x%08X   r2: 0x%08X   r3: 0x%08X", cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_0), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_1), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_2), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_3));
	CPUDEBUG("r4:  0x%08X   r5: 0x%08X   r6: 0x%08X   r7: 0x%08X", cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_4), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_5), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_6), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_7));
	CPUDEBUG("r8:  0x%08X   r9: 0x%08X  r10: 0x%08X  r11: 0x%08X", cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_8), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_9), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_10), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_11));
	CPUDEBUG("r12: 0x%08X  r13: 0x%08X  r14: 0x%08X  r15: 0x%08X", cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_12), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_13), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_14), cpuReadRegister(getCurrentlyValidRegisterBank(), REGISTER_TYPE::RT_15));
#if (DISABLED)
	CPUDEBUG("spsr [USR/SYS]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_USR_SYS, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [FIQ]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_FIQ, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [IRQ]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_IRQ, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [SVC]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_SVC, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [ABT]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_ABT, (REGISTER_TYPE)SPSR));
	CPUDEBUG("spsr [UND]: 0x%08X", cpuReadRegister(REGISTER_BANK_TYPE::RB_UND, (REGISTER_TYPE)SPSR));
#endif
	psr_t currentCPSR = { ZERO };
	currentCPSR.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR);
	CPUDEBUG("cpsr: 0x%08X [%s%s%s%s%s%s%s]", currentCPSR.psrMemory, CPSR_FLAG(currentCPSR.psrFields.psrNegativeBit, "N"), CPSR_FLAG(currentCPSR.psrFields.psrZeroBit, "Z"),
		CPSR_FLAG(currentCPSR.psrFields.psrCarryBorrowExtBit, "C"), CPSR_FLAG(currentCPSR.psrFields.psrOverflowBit, "V"), CPSR_FLAG(currentCPSR.psrFields.psrIRQDisBit, "I"),
		CPSR_FLAG(currentCPSR.psrFields.psrFIQDisBit, "F"), CPSR_FLAG(currentCPSR.psrFields.psrStateBit, "T"));

#if (GBA_ENABLE_AGS_PATCHED_TEST == YES)
	static const std::string chk1 = "TCHK01–";
	static const std::string chk2 = "AGB CHECKER";
	static const std::string test1 = reinterpret_cast<char*>(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.gameCode);
	static const std::string test2 = reinterpret_cast<char*>(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.gametitle);
	if (
		(!test1.compare(chk1))
		&&
		(!test2.compare(chk2))
		&&
		(pGBA_memory->mGBAMemoryMap.mGamePakRom.mWaitState.mWaitState0.mWaitState0Fields.cartridge_header_SB.cartridge_header_SB_fields.softwareVersion == SIXTEEN)
		)
	{
#if (DISABLED)
		static FLAG openFileDone = false;
		static std::ofstream out;
		if (openFileDone == false)
		{
			out.open("AGS_LOGs.txt");
			openFileDone = true;
		}
#endif
		if (pGBA_instance->GBA_state.emulatorStatus.isBiosExecutionDone == YES && ((gbaEmulationCounter[ONE] % 10000) == ZERO))
		{
			pGBA_instance->GBA_state.emulatorStatus.debugger.agbReturn = readRawMemory<GBA_HALFWORD>(0x0004, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::HOST);
			INFO("ERR : %d", pGBA_instance->GBA_state.emulatorStatus.debugger.agbReturn);
			//out << "ERR : " << (GBA_HALFWORD)pGBA_instance->GBA_state.emulatorStatus.debugger.agbReturn << std::endl;
		}
	}
#endif

	if (getARMState() == STATE_TYPE::ST_THUMB)
	{
		// PC increment OR Stage 3 (Note that we have 3 stage pipeline of which 2 stages are taken care in "fetchAndDecode" and last remaining stage is taken care below):
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		// NOTE: PC always point to fetchStageOpCode
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_HALFWORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU_INSTRUCTION_FETCH, pGBA_memory->setNextPipelineAccessType);
		pGBA_memory->setNextPipelineAccessType = MEMORY_ACCESS_TYPE::AUTOMATIC;

		uint32_t extension1 = TO_UINT32(11 - OP_MODE_NAMES[currentCPSR.psrFields.psrModeBits].length());
		DISASSEMBLY("[THUMB] [%s] %*c 0x%08X : [0x%04X]     [%-23s]", OP_MODE_NAMES[currentCPSR.psrFields.psrModeBits].c_str(), extension1, ' ', (GBA_WORD)(pGBA_cpuInstance->registers.pc - FOUR), pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode, disassembled.c_str());

		// Now, since all the pipelining is handled for this run, proceed to execute the opcode...

		if (ThumbSoftwareInterrupt() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::ThumbSoftwareInterrupt;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (UnconditionalBranch() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::UnconditionalBranch;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (ConditionalBranch() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::ConditionalBranch;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (MultipleLoadStore() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::MultipleLoadStore;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LongBranchWithLink() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::LongBranchWithLink;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (AddOffsetToStackPointer() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::AddOffsetToStackPointer;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (PushPopRegisters() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::PushPopRegisters;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreHalfword() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::LoadStoreHalfword;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (SPRelativeLoadStore() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::SPRelativeLoadStore;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadAddress() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::LoadAddress;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreWithImmediateOffset() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::LoadStoreWithImmediateOffset;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreWithRegisterOffset() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::LoadStoreWithRegisterOffset;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (LoadStoreSignExtendedByteHalfword() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::LoadStoreSignExtendedByteHalfword;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (PCRelativeLoad() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::PCRelativeLoad;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (HiRegisterOperationsBranchExchange() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::HiRegisterOperationsBranchExchange;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (ALUOperations() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::ALUOperations;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (MoveCompareAddSubtractImmediate() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::MoveCompareAddSubtractImmediate;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (AddSubtract() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::AddSubtract;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}
		if (MoveShiftedRegister() == YES)
		{
			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::MoveShiftedRegister;
			ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
			RETURN;
		}

		pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::UNKNOWN;

		FATAL("Unknown THUMB Instruction");
	}
	else if (getARMState() == STATE_TYPE::ST_ARM)
	{
		// PC increment OR Stage 3 (Note that we have 3 stage pipeline of which 2 stages are taken care in "fetchAndDecode" and last remaining stage is taken care below):
		pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode;
		pGBA_cpuInstance->pipeline.decodeStageOpCode.opCode.rawOpCode = pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode;
		// NOTE: PC always point to fetchStageOpCode
		pGBA_cpuInstance->pipeline.fetchStageOpCode.opCode.rawOpCode = readRawMemory<GBA_WORD>(pGBA_cpuInstance->registers.pc, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU_INSTRUCTION_FETCH, pGBA_memory->setNextPipelineAccessType);
		pGBA_memory->setNextPipelineAccessType = MEMORY_ACCESS_TYPE::AUTOMATIC;

		uint32_t extension1 = TO_UINT32(11 - OP_MODE_NAMES[currentCPSR.psrFields.psrModeBits].length());
		DISASSEMBLY("[ARM]   [%s] %*c 0x%08X : [0x%08X] [%-23s]", OP_MODE_NAMES[currentCPSR.psrFields.psrModeBits].c_str(), extension1, ' ', (GBA_WORD)(pGBA_cpuInstance->registers.pc - EIGHT), pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.rawOpCode, disassembled.c_str());

		// Now, since all the pipelining is handled for this run, proceed to execute the opcode...

		if (didConditionalCheckPass(pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.cond, pGBA_cpuInstance->registers.cpsr.psrMemory) == YES)
		{
			if (BranchAndBranchExchange() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::BranchAndBranchExchange;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (BlockDataTransfer() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::BlockDataTransfer;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (BranchAndBranchLink() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::BranchAndBranchLink;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (SoftwareInterrupt() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::SoftwareInterrupt;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (Undefined() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::Undefined;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (SingleDataTransfer() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::SingleDataTransfer;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (SingleDataSwap() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::SingleDataSwap;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (MultiplyAndMultiplyAccumulate() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::MultiplyAndMultiplyAccumulate;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (HalfWordDataTransfer() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::HalfWordDataTransfer;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (psrTransfer() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::PSRTransfer;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}
			if (DataProcessing() == YES)
			{
				pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::DataProcessing;
				ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
				RETURN;
			}

			pGBA_instance->GBA_state.emulatorStatus.lastOpcode = ARM7TDMI_Opcode::UNKNOWN;

			unimplementedInstruction();
			FATAL("Unknown ARM Instruction");
		}
		else
		{
			CPUINFO("Skipping instruction because condition %d was not met.", pGBA_cpuInstance->pipeline.executeStageOpCode.opCode.arm.cond);
			pGBA_cpuInstance->registers.pc += FOUR;
			pGBA_cpuInstance->registers.pc &= 0xFFFFFFFC;

			pGBA_memory->setNextPipelineAccessType = MEMORY_ACCESS_TYPE::SEQUENTIAL_CYCLE;
		}
	}
	else
	{
		FATAL("Unknown Operating Mode");
	}

	ASSERT(pGBA_instance->GBA_state.emulatorStatus.ticks.cycle_accurate.cpuCounter != RESET);
}
#pragma endregion ARM7TDMI_DEFINITIONS

#pragma region EMULATION_DEFINITIONS
MASQ_INLINE void GBA_t::dmaTick()
{
	processDMA();
}

MASQ_INLINE void GBA_t::timerTick()
{
	processTimer(ONE);
}

MASQ_INLINE void GBA_t::serialTick()
{
	processSIO(ONE);
}

MASQ_INLINE void GBA_t::apuTick()
{
	processAPU(ONE);
}

MASQ_INLINE void GBA_t::ppuTick()
{
	processPPU(ONE);
}

MASQ_INLINE void GBA_t::joypadTick()
{
	DO_NOTHING;
}

MASQ_INLINE void GBA_t::requestInterrupts(GBA_INTERRUPT interrupt)
{
#if (GBA_ENABLE_DELAYED_IRQ == YES)
	pGBA_instance->GBA_state.interrupt.ifPend |= (ONE << TO_UINT(interrupt));
#else
	pGBA_peripherals->mIFHalfWord.mIFHalfWord = (uint16_t)(ONE << TO_UINT16(interrupt));
#endif
}

MASQ_INLINE FLAG GBA_t::shouldUnHaltTheCPU()
{
#if (GBA_ENABLE_DELAYED_IRQ == YES)
	RETURN pGBA_instance->GBA_state.interrupt.irqAvailLatch;
#else
	if ((pGBA_peripherals->mIFHalfWord.mIFHalfWord & pGBA_peripherals->mIEHalfWord.mIEHalfWord & 0x3FFF) != ZERO)
	{
		RETURN YES;
	}

	RETURN NO;
#endif
}

MASQ_INLINE FLAG GBA_t::isInterruptReadyToBeServed()
{
#if (GBA_ENABLE_DELAYED_IRQ == YES)
	RETURN pGBA_instance->GBA_state.interrupt.irqAvailLatch;
#else
	if ((pGBA_peripherals->mIFHalfWord.mIFHalfWord & pGBA_peripherals->mIEHalfWord.mIEHalfWord & 0x3FFF) != ZERO)
	{
		RETURN YES;
	}

	RETURN NO;
#endif
}

MASQ_INLINE FLAG GBA_t::handleInterruptsIfApplicable()
{
	FLAG interruptWasServiced = NO;

#if (GBA_ENABLE_DELAYED_IRQ == YES)
	if (pGBA_instance->GBA_state.interrupt.cpsrIrqMaskLatch == DISABLED
		&& pGBA_instance->GBA_state.interrupt.irqLineLatch == ENABLED
		&& isInterruptReadyToBeServed() == YES)
#else
	if (pGBA_peripherals->mIMEHalfWord.mIMEFields.ENABLE_ALL_INTERRUPTS == ENABLED
		&& pGBA_registers->cpsr.psrFields.psrIRQDisBit == RESET
		&& isInterruptReadyToBeServed() == YES)
#endif
	{
		// Refer : http://problemkaputt.de/gbatek-arm-cpu-exceptions.htm for details on steps to be taken when interrupt occurs

		// Make sure Halt if cleared (It will be cleared...but just making sure)

		pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::RUN;

		// Prefetch the next instruction, even though this is not used, we still do it for timing

		if (getARMState() == STATE_TYPE::ST_THUMB)
		{
			(void)readRawMemory<GBA_HALFWORD>(cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC) & ~ONE, MEMORY_ACCESS_WIDTH::SIXTEEN_BIT, MEMORY_ACCESS_SOURCE::CPU_INSTRUCTION_FETCH, pGBA_memory->setNextPipelineAccessType);
		}
		else
		{
			(void)readRawMemory<GBA_WORD>(cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC) & ~THREE, MEMORY_ACCESS_WIDTH::THIRTYTWO_BIT, MEMORY_ACCESS_SOURCE::CPU_INSTRUCTION_FETCH, pGBA_memory->setNextPipelineAccessType);
		}

		pGBA_memory->setNextPipelineAccessType = MEMORY_ACCESS_TYPE::AUTOMATIC;

		// Save CPSR to SPSR.IRQ

		psr_t cpsr = { ZERO };
		cpsr.psrMemory = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)CPSR);
		cpuSetRegister(REGISTER_BANK_TYPE::RB_IRQ, (REGISTER_TYPE)SPSR, getARMState(), cpsr.psrMemory);

		// Switch to IRQ mode as CPSR is saved now

		setARMMode(OP_MODE_TYPE::OP_IRQ);

		// Disable IRQs

		pGBA_registers->cpsr.psrFields.psrIRQDisBit = SET; // https://gbadev.net/tonc/interrupts.html

		// Idle Cycles...

		cpuIdleCycles();

		// Switch to ARM state (if in thumb)

		setARMState(STATE_TYPE::ST_ARM);

		// Save PC in LR
		auto currentPC = cpuReadRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC);
		if ((STATE_TYPE)cpsr.psrFields.psrStateBit == STATE_TYPE::ST_THUMB)
		{
			// Refer to "INFORMATION_002" to understand why we store PC instead of PC - 4 in Thumb state
			currentPC = currentPC;
		}
		else
		{
			// Refer to "INFORMATION_002" to understand why we store PC - 4 instead of PC - 8 in ARM state
			currentPC = currentPC - FOUR;
		}
		cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)LR, getARMState(), currentPC);

		// Set PC to IRQ exception vector
		cpuSetRegister(getCurrentlyValidRegisterBank(), (REGISTER_TYPE)PC, getARMState(), 0x18);
		reloadPipeline(pGBA_registers->pc);

		interruptWasServiced = YES;
	}

	RETURN interruptWasServiced;
}

MASQ_INLINE void GBA_t::handleKeypadInterrupts()
{
	if (pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTFields.BUTTON_IRQ_EN == SET
		// Keypad interrupts are used to come out of STOP mode...
		|| pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::STOP)
	{
		if (pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTFields.BUTTON_IRQ_CONDITION == SET)
		{
			// Logical AND mode
			if
				(
					(
						((~(pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTHalfWord & 0x03FF)) & 0x03FF)
						==
						(pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTHalfWord & 0x03FF)
						)
					)
			{
				requestInterrupts(GBA_INTERRUPT::IRQ_KEYPAD);
			}
		}
		else
		{
			// Logical OR mode
			if
				(
					(
						((~(pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTHalfWord & 0x03FF)) & 0x03FF)
						&
						(pGBA_peripherals->mKEYCNTHalfWord.mKEYCNTHalfWord & 0x03FF)
						) != ZERO
					)
			{
				requestInterrupts(GBA_INTERRUPT::IRQ_KEYPAD);
			}
		}

		if (pGBA_cpuInstance->haltCntState == HALT_CONTROLLER::STOP)
		{
			pGBA_cpuInstance->haltCntState = HALT_CONTROLLER::RUN;
		}
	}
}

MASQ_INLINE void GBA_t::captureIO()
{
	// Refer http://problemkaputt.de/gbatek-gba-keypad-input.htm

	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.START = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_Enter) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.SELECT = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_Space) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);

	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.BUTTON_A = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_Z) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.BUTTON_B = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_X) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);

	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.LEFT = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_LeftArrow) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.RIGHT = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_RightArrow) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.UP = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_UpArrow) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.DOWN = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_DownArrow) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);

	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.BUTTON_R = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_S) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);
	pGBA_peripherals->mKEYINPUTHalfWord.mKEYINPUTFields.BUTTON_L = (GBA_HALFWORD)((ImGui::IsKeyDown(ImGuiKey_A) == YES) ? JOYPAD_STATES::PRESSED : JOYPAD_STATES::RELEASED);

	handleKeypadInterrupts();
}

MASQ_INLINE void GBA_t::playTheAudioFrame()
{
	RETURN;
}

MASQ_INLINE void GBA_t::displayCompleteScreen()
{
	if (debugConfig._DEBUG_PPU_VIEWER_GUI == ENABLED)
	{
		FATAL("_DEBUG_PPU_VIEWER_GUI is not supported yet");
	}
	else
	{
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

		glDisable(GL_BLEND);

		// Handle for gameboy system's texture

		glBindTexture(GL_TEXTURE_2D, gameboyAdvance_texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGBA_instance->GBA_state.display.imGuiBuffer.imGuiBuffer1D);

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

			glBindTexture(GL_TEXTURE_2D, gameboyAdvance_matrix_texture);

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
		// 1. Upload emulator framebuffer to gameboyAdvance_texture
		GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboyAdvance_texture));
		GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
			(GLvoid*)pGBA_instance->GBA_state.display.imGuiBuffer.imGuiBuffer1D));

		// Choose filtering mode (NEAREST or LINEAR)
		GLint filter = (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER) ? GL_LINEAR : GL_NEAREST;

		// Apply filtering only when it changes (optimization)
		static GLint prevFilterGBA = -1;
		if (filter != prevFilterGBA)
		{
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
			prevFilterGBA = filter;
		}

		// 2. Render gameboyAdvance_texture into framebuffer (masquerade_texture target)
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
		GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

		// Pass 1: Render base texture (Game Boy framebuffer)
		GL_CALL(glUseProgram(shaderProgramBasic));
		GL_CALL(glActiveTexture(GL_TEXTURE0));

		// Bind once (no redundant state changes)
		GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboyAdvance_texture));

		// Ensure correct filter is applied only when needed
		static GLint prevFilterSrcGBA = -1;
		static GLuint prevTexGBA = 0;

		if (filter != prevFilterSrcGBA || prevTexGBA != gameboyAdvance_texture)
		{
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
			prevFilterSrcGBA = filter;
			prevTexGBA = gameboyAdvance_texture;
		}

		// Set uniform
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
			GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboyAdvance_matrix_texture));
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
		static GLint prevFilterFinalGBA = -1;
		if (filter != prevFilterFinalGBA)
		{
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
			GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
			prevFilterFinalGBA = filter;
		}
#endif
	}
}

MASQ_INLINE float GBA_t::getEmulationVolume()
{
	pGBA_audio->emulatorVolume = SDL_GetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream));
	RETURN pGBA_audio->emulatorVolume;
}

MASQ_INLINE void GBA_t::setEmulationVolume(float volume)
{
	pGBA_audio->emulatorVolume = volume;
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), volume);
	pt.put("gba._volume", volume);
	boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, pt);
}
#pragma endregion EMULATION_DEFINITIONS

OPT_DEFAULT