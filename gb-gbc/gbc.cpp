#pragma region GB_GBC_SPECIFIC_INCLUDES
#include "gbc.h"
#pragma endregion GB_GBC_SPECIFIC_INCLUDES

#pragma region GB_GBC_SPECIFIC_MACROS
#pragma region WIP
#define GB_GBC_ENABLE_PPU_BG_MODE3_RUN_FOR_174_DOTS		(YES)	// Needed for sprite based PPU timing tests
#define GB_GBC_ENABLE_TILE_SEL_GLITCH					(NO)	// This is not working
#define GB_GBC_ENABLE_WIN_EN_GLITCH						(NO)	// Not yet implemented
#define GB_GBC_ENABLE_WINDESYNC_GLITCH					(NO)	// This is not working
#define GB_GBC_ENABLE_CGB_SCY_WRITE_DELAY				(NO)	// Enabling this causes pokemon "video.gbc" to have graphical artifacts; Check if this is valid in CGB double speed
#define GB_GBC_ENABLE_CGB_OBSCURE_TIMER_BEHAVIOUR		(NO)	// Enabling this causes rapid_toggle.gb to fail in CGB mode
#define GB_GBC_ENABLE_DMA_STAT_OAM_BOUNDARY_GLITCH		(YES)	// This is needed by docboy's "DMA check stat" tests 
#pragma endregion WIP

#define GB_GBC_REFERENCE_CLOCK_HZ						(4194304.0f)

#define ONE_MCYCLE_IN_T_CYCLES							(FOUR)

#define MBC1_ROM_MODE									(false)
#define MBC1_RAM_MODE									(true)

#define ROM_00_START_ADDRESS							0x0000
#define DMG_BIOS_START_ADDRESS							0x0000
#define DMG_BIOS_END_ADDRESS							0x0100
#define CGB_BIOS_START_ADDRESS_PART1					0x0000
#define CGB_BIOS_END_ADDRESS_PART1						0x0100
#define CGB_BIOS_START_ADDRESS_PART2					0x0200
#define CGB_BIOS_END_ADDRESS_PART2						0x0900
#define ROM_00_END_ADDRESS								0x3FFF
#define ROM_NN_START_ADDRESS							(ROM_00_END_ADDRESS + ONE)
#define ROM_NN_END_ADDRESS								0x7FFF
#define VRAM_START_ADDRESS								(ROM_NN_END_ADDRESS + ONE)
#define TILE_DATA_START_ADDRESS							VRAM_START_ADDRESS
#define TILE_DATA_END_ADDRESS							0x97FF
#define TILE_MAP_START_ADDRESS							(TILE_DATA_END_ADDRESS + ONE)
#define TILE_MAP_END_ADDRESS							0x9FFF
#define VRAM_END_ADDRESS								TILE_MAP_END_ADDRESS
#define EXTERNAL_RAM_START_ADDRESS						(VRAM_END_ADDRESS + ONE)
#define EXTERNAL_RAM_END_ADDRESS						0xBFFF
#define WORK_RAM_00_START_ADDRESS						(EXTERNAL_RAM_END_ADDRESS + ONE)
#define WORK_RAM_00_END_ADDRESS							0xCFFF
#define WORK_RAM_01_START_ADDRESS						(WORK_RAM_00_END_ADDRESS + ONE)
#define WORK_RAM_01_END_ADDRESS							0xDFFF
#define ECHO_RAM_START_ADDRESS							(WORK_RAM_01_END_ADDRESS + ONE)
#define ECHO_RAM_END_ADDRESS							0xFDFF
#define OAM_START_ADDRESS								(ECHO_RAM_END_ADDRESS + ONE)
#define OAM_END_ADDRESS									0xFE9F
#define RESTRICTED_MEMORY_START_ADDRESS					(OAM_END_ADDRESS + ONE)
#define RESTRICTED_MEMORY_END_ADDRESS					0xFEFF
#define GB_GBC_IO_MEMORY_START_ADDRESS					0xFF00
#define P1_JOYP_ADDRESS									GB_GBC_IO_MEMORY_START_ADDRESS
#define SB_ADDRESS										0xFF01
#define SC_ADDRESS										0xFF02
#define DIV_ADDRESS_LSB									0xFF03
#define DIV_ADDRESS_MSB									0xFF04
#define TIMA_ADDRESS									0xFF05
#define TMA_ADDRESS										0xFF06
#define TAC_ADDRESS										0xFF07
#define IF_ADDRESS										0xFF0F
#define NR10_ADDRESS									0xFF10
#define NR11_ADDRESS									0xFF11
#define NR12_ADDRESS									0xFF12
#define NR13_ADDRESS									0xFF13
#define NR14_ADDRESS									0xFF14
#define NR21_ADDRESS									0xFF16
#define NR22_ADDRESS									0xFF17
#define NR23_ADDRESS									0xFF18
#define NR24_ADDRESS									0xFF19
#define NR30_ADDRESS									0xFF1A
#define NR31_ADDRESS									0xFF1B
#define NR32_ADDRESS									0xFF1C
#define NR33_ADDRESS									0xFF1D
#define NR34_ADDRESS									0xFF1E
#define NR41_ADDRESS									0xFF20
#define NR42_ADDRESS									0xFF21
#define NR43_ADDRESS									0xFF22
#define NR44_ADDRESS									0xFF23
#define NR50_ADDRESS									0xFF24
#define NR51_ADDRESS									0xFF25
#define NR52_ADDRESS									0xFF26
#define WAVE_RAM_START_ADDRESS							0xFF30
#define WAVE_RAM_END_ADDRESS							0xFF3F
#define LCDC_ADDRESS									0xFF40
#define STAT_ADDRESS									0xFF41
#define SCY_ADDRESS										0xFF42
#define SCX_ADDRESS										0xFF43
#define LY_ADDRESS										0xFF44
#define LYC_ADDRESS										0xFF45
#define DMA_ADDRESS										0xFF46
#define BGP_ADDRESS										0xFF47
#define OBP0_ADDRESS									0xFF48
#define OBP1_ADDRESS									0xFF49
#define WY_ADDRESS										0xFF4A
#define WX_ADDRESS										0xFF4B
#define KEY0_ADDRESS									0xFF4C
#define KEY1_ADDRESS									0xFF4D
#define VRAM_BANK_SWITCH								0xFF4F
#define BANK_ADDRESS									0xFF50
#define HDMA1_SOURCE_HIGH_REGISTER_ADDRESS				0xFF51
#define HDMA2_SOURCE_LOW_REGISTER_ADDRESS				0xFF52
#define HDMA3_DEST_HIGH_REGISTER_ADDRESS				0xFF53
#define HDMA4_DEST_LOW_REGISTER_ADDRESS					0xFF54
#define HDMA5_CONFIG_REGISTER_ADDRESS					0xFF55
#define IR_PORT_ADDRESS									0xFF56
#define BCPS_ADDRESS									0xFF68
#define BCPD_ADDRESS									0xFF69
#define OCPS_ADDRESS									0xFF6A
#define OCPD_ADDRESS									0xFF6B
#define OBJECT_PRIORITY_MODE							0xFF6C
#define WRAM_BANK_SWITCH								0xFF70
#define PCM12_ADDRESS									0xFF76
#define PCM34_ADDRESS									0xFF77
#define GB_GBC_IO_MEMORY_END_ADDRESS					0xFF7F
#define HIRAM_START_ADDRESS								0xFF80
#define HIRAM_END_ADDRESS								0xFFFE
#define IE_ADDRESS										(HIRAM_END_ADDRESS + ONE)

#define DMA_DELAY										TWO

#define SERIAL_SLAVE									(ZERO)
#define SERIAL_MASTER									(ONE)

#define TD0x8000_TD0x8FFF								0x8000
#define TD0x8800_TD0x97FF								0x8800
#define TM0x9800_TM0x9BFF								0x9800
#define TM0x9C00_TM0x9FFF								0x9C00
#define PPU_CYCLES_PER_59_73HZ							(70224)		// 4.194304 MHz / 59.73 Hz => 70221 ~ 70224
#define COLOR_ID_ZERO									(ZERO)
#define VBLANK_SCANLINES								(TEN)

#define WAVE_TRIGGER_CORRUPTION_OFFSET_TICKS			TWO			// Needed to pass tests 9 and 12 of dmg_sound.gb
#define LFSR_WIDTH_IS_15_BITS							ZERO
#define LFSR_WIDTH_IS_7_BITS							ONE
#define APU_FRAME_SEQUENCER_RATE_HZ						(512)
#define APU_FRAME_SEQUENCER_CYCLES_PER_512_HZ			(8192)		// 4.194304 MHz / 512.0 Hz => 8192
#define DC_BIAS_FOR_AUDIO_SAMPLES						ZERO
#define DISABLE_FIRST_PULSE_CHANNEL						NO
#define DISABLE_SECOND_PULSE_CHANNEL					NO
#define DISABLE_WAVE_CHANNEL							NO
#define DISABLE_NOISE_CHANNEL							NO

#define RESET_TICK										FALSE
#define INVALID_TICK									NO
#define VALID_TICK										YES

#define initializeSerialClockSpeed						processSerialClockSpeedBit
#define performOverFlowCheck							getUpdatedFrequency
#pragma endregion GB_GBC_SPECIFIC_MACROS

#pragma region CONDITIONAL_INCLUDES
#pragma endregion CONDITIONAL_INCLUDES

#pragma region GB_GBC_SPECIFIC_DECLARATIONS
// PPU cycles per one gameboy/gameboy color frame
static uint32_t const PPU_CYCLES_PER_FRAME = PPU_CYCLES_PER_59_73HZ;
// maximum number of inputs
static uint32_t const MAX_INPUT_LEN = (uint32_t)(EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC / CEIL(GB_GBC_FPS));
// length of filter than can be handled
static uint32_t const MAX_FLT_LEN = 103;
// buffer to hold all of the input samples
static uint32_t const BUFFER_LEN = (MAX_FLT_LEN - 1 + MAX_INPUT_LEN);
// coefficients for the FIR from https://www.arc.id.au/FilterDesign.html
// 
// double type buffers to hold input and output during FIR
static double doubleInput[(uint32_t)(EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC / CEIL(GB_GBC_FPS))];
static double doubleOutput[(uint32_t)(EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC / CEIL(GB_GBC_FPS))];

static FLAG _DISABLE_BG = NO;
static FLAG _DISABLE_WIN = NO;
static FLAG _DISABLE_OBJ = NO;
static FLAG _ENABLE_DMG_BIOS = NO;
static FLAG _ENABLE_CGB_BIOS = NO;
static FLAG _ENABLE_AUDIO_HPF = NO;
static FLAG _FORCE_GB_FOR_GBC = NO;
static FLAG _FORCE_GB_GFX_FOR_GBC = NO;
static FLAG _FORCE_GBC_FOR_GB = NO;
static std::string _JSON_LOCATION;
static boost::property_tree::ptree testCase;

static uint32_t gameboy_texture;
static uint32_t matrix_texture;
static uint32_t matrix[16] = { 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };

#if _DEBUG
COUNTER32 OAM_STAT_TO_MODE_2_T_CYCLES = RESET;
#endif
#pragma endregion GB_GBC_SPECIFIC_DECLARATIONS

#pragma region INFRASTRUCTURE_DEFINITIONS
GBc_t::GBc_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree & config, CheatEngine_t* ce)
{
	setEmulationID(EMULATION_ID::GB_GBC_ID);

	this->pt = config;

	this->ceGBGBC = ce;

	this->ceGBGBC->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, EMULATION_ID::GB_GBC_ID);

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
		//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_DISASSEMBLY);

		isBiosEnabled = NO;

		std::transform(rom[ZERO].begin(), rom[ZERO].end(), rom[ZERO].begin(), ::tolower);

		if (rom[ZERO].substr(rom[ZERO].find_last_of(".") + ONE) == "gb")
		{
			ROM_TYPE = ROM::GAME_BOY;
		}
		else if (rom[ZERO].substr(rom[ZERO].find_last_of(".") + ONE) == "gbc")
		{
			ROM_TYPE = ROM::GAME_BOY_COLOR;
		}
		else
		{
			ROM_TYPE = ROM::NO_ROM;
		}

#ifndef __EMSCRIPTEN__
		_SAVE_LOCATION = pt.get<std::string>("gb-gbc._save_location");
#else
		_SAVE_LOCATION = "assets/saves";
#endif

		// check if directory mentioned by "_SAVE_LOCATION" exists, if not we need to explicitly create it
		ifNoDirectoryThenCreate(_SAVE_LOCATION);

		_ENABLE_DMG_BIOS = to_bool(pt.get<std::string>("gb-gbc._use_dmg_bios"));
		_ENABLE_CGB_BIOS = to_bool(pt.get<std::string>("gb-gbc._use_cgb_bios"));
		_FORCE_GB_FOR_GBC = to_bool(pt.get<std::string>("gb-gbc._force_gb_for_gbc"));
		_FORCE_GB_GFX_FOR_GBC = to_bool(pt.get<std::string>("gb-gbc._force_gb_gfx_for_gbc"));
		_FORCE_GBC_FOR_GB = to_bool(pt.get<std::string>("gb-gbc._force_gbc_for_gb"));
		_ENABLE_AUDIO_HPF = to_bool(config.get<std::string>("gb-gbc._enable_audio_hpf"));

		if (ROM_TYPE == ROM::GAME_BOY && _FORCE_GBC_FOR_GB == YES)
		{
			INFO("Running in Forced CGB mode");
			INFO("Forced CGB mode requires CGB bios to be loaded");
			ROM_TYPE = ROM::GAME_BOY_COLOR; // .gb loaded to GBC
			_ENABLE_DMG_BIOS = NO;
			_ENABLE_CGB_BIOS = YES;
		}
		else if (ROM_TYPE == ROM::GAME_BOY_COLOR && _FORCE_GB_FOR_GBC == YES)
		{
			INFO("Running in Forced DMG mode");
			INFO("Forced DMG mode requires DMG bios to be loaded");
			ROM_TYPE = ROM::GAME_BOY; // .gbc loaded to GB
			_ENABLE_CGB_BIOS = NO;
		}

		FLAG searchForBios = NO;

		if ((ROM_TYPE == ROM::GAME_BOY && _ENABLE_DMG_BIOS == YES)
			|| (ROM_TYPE == ROM::GAME_BOY_COLOR && _FORCE_GB_FOR_GBC == YES))
		{
			ROM_TYPE = ROM::GAME_BOY;
			INFO("Running in DMG mode");

#ifndef __EMSCRIPTEN__
			_BIOS_LOCATION = config.get<std::string>("gb-gbc._dmg_bios_location");
#else
			_BIOS_LOCATION = "assets/gb/bios/dmg_rom.bin";
#endif

			std::cout << "Searching for BIOS in " << _BIOS_LOCATION << '\n';
			dmg_cgb_bios.expectedBiosSize = 0x100;
			searchForBios = YES;
		}
		else if
			((ROM_TYPE == ROM::GAME_BOY_COLOR && _ENABLE_CGB_BIOS == YES)
				|| (ROM_TYPE == ROM::GAME_BOY && _FORCE_GBC_FOR_GB == YES))
		{
			ROM_TYPE = ROM::GAME_BOY_COLOR;
			INFO("Running in CGB mode");

#ifndef __EMSCRIPTEN__
			_BIOS_LOCATION = config.get<std::string>("gb-gbc._cgb_bios_location");
#else
			_BIOS_LOCATION = "assets/gbc/bios/cgb_boot.bin";
#endif

			std::cout << "Searching for BIOS in " << _BIOS_LOCATION << '\n';
			dmg_cgb_bios.expectedBiosSize = 0x900;
			searchForBios = YES;
		}
		else
		{
			INFO("By-passing BIOS");

			dmg_cgb_bios.biosFound = NO;
			dmg_cgb_bios.unMapBios = YES;
		}

		if (searchForBios == YES)
		{
			std::cout << "Expected Bios size " << dmg_cgb_bios.expectedBiosSize << '\n';

			// Get the list of files in bios directory
			dmg_cgb_bios.biosFound = NO;
			dmg_cgb_bios.unMapBios = YES;

			uint32_t sizeOfBios = ZERO;
			std::string maybeBiosFile;

#if DEACTIVATED
			for (const auto& entry : std::filesystem::directory_iterator(_BIOS_LOCATION))
			{
				maybeBiosFile = entry.path().string();
				std::transform(maybeBiosFile.begin(), maybeBiosFile.end(), maybeBiosFile.begin(), ::tolower);

				if (maybeBiosFile.substr(maybeBiosFile.find_last_of(".") + ONE) == "bin")
				{
					maybeBiosFile = entry.path().string();
					dmg_cgb_bios.biosFound = YES;
					dmg_cgb_bios.unMapBios = NO;
					BREAK;
				}
				dmg_cgb_bios.biosFound = NO;
				dmg_cgb_bios.unMapBios = YES;
			}
#else
			dmg_cgb_bios.biosFound = YES;
			dmg_cgb_bios.unMapBios = NO;
			maybeBiosFile = _BIOS_LOCATION;
#endif

			if (dmg_cgb_bios.biosFound == YES)
			{
				FILE* fp = NULL;
				std::cout << maybeBiosFile << std::endl;
				errno_t err = fopen_portable(&fp, maybeBiosFile.c_str(), "rb");
				if (!err && (fp != NULL))
				{
					fseek(fp, 0, SEEK_END);
					sizeOfBios = ftell(fp);

					if (sizeOfBios == dmg_cgb_bios.expectedBiosSize)
					{
						rewind(fp);
						fread(dmg_cgb_bios.biosImage + 0x0000, sizeOfBios, 1, fp);

#if ZERO
						uint32_t scanner = 0;
						uint32_t addressField = 0x10;
						LOG("BIOS DUMP");
						LOG("Address\t\t");
						for (uint32_t ii = 0; ii < 0x10; ii++)
						{
							LOG("%02x\t", ii);
						}
						LOG_NEW_LINE;
						LOG_NEW_LINE;
						LOG("00000000\t");
						for (uint32_t ii = 0; ii < (int)sizeOfBios; ii++)
						{
							LOG("0x%02x\t", bios.bios[0x0000 + ii]);
							if (++scanner == 0x10)
							{
								scanner = 0;
								LOG_NEW_LINE;
								LOG("%08x\t", addressField);
								addressField += 0x10;
							}
						}
						LOG_NEW_LINE;
						LOG_NEW_LINE;
						LOG_NEW_LINE;
#endif

					}
					else
					{
						dmg_cgb_bios.biosFound = false;
						dmg_cgb_bios.unMapBios = true;
					}
				}
			}

			if (dmg_cgb_bios.biosFound == true)
			{
				isBiosEnabled = YES;
				INFO("Using the above mentioned bios");
				LOG_NEW_LINE;
			}
		}

		this->rom[ZERO] = rom[ZERO];

		// Some Additional Information...

		if (_ENABLE_AUDIO_HPF == YES)
		{
			INFO("Audio HPF : Enabled");
		}
		else
		{
			INFO("Audio HPF : Disabled");
		}

		if (!_ENABLE_REWIND)
		{
			_REWIND_BUFFER_SIZE = 0;
			INFO("Rewind : Disabled");
		}
		else if (_REWIND_BUFFER_SIZE <= 0 && _ENABLE_REWIND == YES)
		{
			_ENABLE_REWIND = NO;
			INFO("Rewind : Disabled");
		}
		else
		{
			INFO("Rewind : Enabled");
		}

		LOG_NEW_LINE;
	}
}

void GBc_t::setupTheCoreOfEmulation(void* masqueradeInstance, void* audio, void* network)
{
	uint8_t indexToCheck = 0;

	if (!rom[indexToCheck].empty() || (ROM_TYPE == ROM::TEST_SST))
	{
		if (!initializeEmulator())
		{
			FATAL("memory allocation failure");
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
		FATAL("un-supported rom");
		throw std::runtime_error("un-supported rom");
	}
}

void GBc_t::setEmulationWindowOffsets(uint32_t x, uint32_t y, FLAG isEnabled)
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

uint32_t GBc_t::getTotalScreenWidth()
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

uint32_t GBc_t::getTotalScreenHeight()
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

uint32_t GBc_t::getTotalPixelWidth()
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

uint32_t GBc_t::getTotalPixelHeight()
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

void GBc_t::setEmulationID(EMULATION_ID ID)
{
	myID = ID;
}

EMULATION_ID GBc_t::getEmulationID()
{
	RETURN myID;
}

const char* GBc_t::getEmulatorName()
{
	RETURN this->NAME;
}

float GBc_t::getEmulationFPS()
{
	RETURN this->myFPS;
}

void GBc_t::blarggConsoleOutput()
{
	// blarggs test - serial output
	if (pGBc_peripherals->SC.scMemory == 0x81)
	{
		char c = pGBc_peripherals->SB;
		LOG("%c", c);
		pGBc_peripherals->SC.scMemory = 0x00;
	}
}
#pragma endregion INFRASTRUCTURE_DEFINITIONS

#pragma region EMULATION_DEFINITIONS
const char* GBc_t::cartridgeLicName()
{
	if (pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.newLicCode <= 0xA4)
	{
		RETURN LIC_CODE[pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.newLicCode];
	}

	RETURN "UNKNOWN";
}

const char* GBc_t::cartridgeTypeName()
{
	if (pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.cartridgeType <= 0x22)
	{
		RETURN ROM_TYPES[pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.cartridgeType];
	}

	RETURN "UNKNOWN";
}

FLAG GBc_t::isCGBDoubleSpeedEnabled()
{
	RETURN pGBc_emuStatus->isCGBDoubleSpeedMode;
}

void GBc_t::toggleCGBSpeedMode()
{
	pGBc_emuStatus->isCGBDoubleSpeedMode = !pGBc_emuStatus->isCGBDoubleSpeedMode;
}

FLAG GBc_t::isRTCAvailableInMBC3()
{
	RETURN pGBc_emuStatus->isRTCAvailableInMBC3;
}

void GBc_t::enableRTCAccess()
{
	if (isRTCAvailableInMBC3() == NO)
	{
		RETURN;
	}

	pGBc_emuStatus->enableRTCAccessTimer = true;
}

void GBc_t::disableRTCAccess()
{
	pGBc_emuStatus->enableRTCAccessTimer = false;
}

FLAG GBc_t::isRTCAccessEnabled()
{
	RETURN pGBc_emuStatus->enableRTCAccessTimer;
}

void GBc_t::setRTCFSM(uint8_t fsmState)
{
	pGBc_emuStatus->rtcFsm = fsmState;
}

uint8_t GBc_t::getRTCFSM()
{
	RETURN pGBc_emuStatus->rtcFsm;
}

uint8_t GBc_t::getRTCRegisterNumber()
{
	if (isRTCAccessEnabled() == true)
	{
		RETURN pGBc_emuStatus->currentRTCRegister;
	}
	else
	{
		FATAL("RTC is disabled");
		RETURN (uint8_t)INVALID;
	}
}

void GBc_t::shouldMapRTCToExternalRAM(FLAG shouldMapRTC)
{
	if (isRTCAccessEnabled() == true)
	{
		pGBc_emuStatus->mapRTCRegisters = shouldMapRTC;
	}
	else
	{
		pGBc_emuStatus->mapRTCRegisters = NO;
	}
}

FLAG GBc_t::isRTCMappedToExternalRAM()
{
	if (isRTCAccessEnabled() == true)
	{
		RETURN pGBc_emuStatus->mapRTCRegisters;
	}
	else
	{
		RETURN NO;
	}
}

void GBc_t::setRTCRegisterNumber(uint8_t rtcRegisterNumber)
{
	if (isRTCAccessEnabled() == true)
	{
		pGBc_emuStatus->currentRTCRegister = rtcRegisterNumber;
	}
}

int GBc_t::readFromRTCRegisterIfApplicable()
{
	if (isRTCAccessEnabled() == true)
	{
		if (getRTCRegisterNumber() == 0x08)
		{
			RETURN (pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_S & 0x3F);
		}
		else if (getRTCRegisterNumber() == 0x09)
		{
			RETURN (pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_M & 0x3F);
		}
		else if (getRTCRegisterNumber() == 0x0A)
		{
			RETURN (pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_H & 0x1F);
		}
		else if (getRTCRegisterNumber() == 0x0B)
		{
			RETURN pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DL;
		}
		else if (getRTCRegisterNumber() == 0x0C)
		{
			RETURN (pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DH.rtcDHMemory & 0xC1);
		}
	}

	RETURN INVALID;
}

uint16_t GBc_t::getRTCDayCounter()
{
	uint16_t dayCounter = pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DL;
	if (pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_MSB == ONE)
	{
		SETBIT(dayCounter, 8);
		RETURN dayCounter;
	}
	RETURN dayCounter;
}

void GBc_t::setRTCDayCounter(uint16_t dayCounterValue)
{
	pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DL = dayCounterValue & 0xFF;

	if (GETBIT(8, dayCounterValue) == ONE)
	{
		pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_MSB = ONE;
	}
	else
	{
		pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_MSB = ZERO;
	}
}

void GBc_t::writeToRTCRegisterIfApplicable(uint8_t data)
{
	if (isRTCAccessEnabled() == true)
	{
		if (getRTCRegisterNumber() == 0x08)
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_S = data;
			pGBc_instance->GBc_state.emulatorStatus.ticks.rtcCounter = ZERO; // This is needed by rtc3test.gb
		}
		else if (getRTCRegisterNumber() == 0x09)
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_M = data;
		}
		else if (getRTCRegisterNumber() == 0x0A)
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_H = data;
		}
		else if (getRTCRegisterNumber() == 0x0B)
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL = data;
		}
		else if (getRTCRegisterNumber() == 0x0C)
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHMemory = data;
		}
	}
}

void GBc_t::latchRTCRegisters()
{
	if (isRTCAccessEnabled())
	{
		pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_S = pGBc_instance->GBc_state.rtc.rtcFields.rtc_S;
		pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_M = pGBc_instance->GBc_state.rtc.rtcFields.rtc_M;
		pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_H = pGBc_instance->GBc_state.rtc.rtcFields.rtc_H;
		pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DL = pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL;
		pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DH.rtcDHMemory = pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHMemory;
	}
}

void GBc_t::setMBCType(uint16_t mbcType)
{
	switch (mbcType)
	{
	case 0x00:
		pGBc_emuStatus->no_mbc = true;
		pGBc_emuStatus->mbc1 = false;
		pGBc_emuStatus->mbc2 = false;
		pGBc_emuStatus->mbc3 = false;
		pGBc_emuStatus->mbc5 = false;
		BREAK;
	case 0x01:
		pGBc_emuStatus->no_mbc = false;
		pGBc_emuStatus->mbc1 = true;
		pGBc_emuStatus->mbc2 = false;
		pGBc_emuStatus->mbc3 = false;
		pGBc_emuStatus->mbc5 = false;
		BREAK;
	case 0x02:
		pGBc_emuStatus->no_mbc = false;
		pGBc_emuStatus->mbc1 = true;
		pGBc_emuStatus->mbc2 = false;
		pGBc_emuStatus->mbc3 = false;
		pGBc_emuStatus->mbc5 = false;
		BREAK;
	case 0x03:
		pGBc_emuStatus->no_mbc = false;
		pGBc_emuStatus->mbc1 = true;
		pGBc_emuStatus->mbc2 = false;
		pGBc_emuStatus->mbc3 = false;
		pGBc_emuStatus->mbc5 = false;
		BREAK;
	case 0x05:
		pGBc_emuStatus->no_mbc = false;
		pGBc_emuStatus->mbc1 = false;
		pGBc_emuStatus->mbc2 = true;
		pGBc_emuStatus->mbc3 = false;
		pGBc_emuStatus->mbc5 = false;
		BREAK;
	case 0x06:
		pGBc_emuStatus->no_mbc = false;
		pGBc_emuStatus->mbc1 = false;
		pGBc_emuStatus->mbc2 = true;
		pGBc_emuStatus->mbc3 = false;
		pGBc_emuStatus->mbc5 = false;
		BREAK;
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		pGBc_emuStatus->no_mbc = false;
		pGBc_emuStatus->mbc1 = false;
		pGBc_emuStatus->mbc2 = false;
		pGBc_emuStatus->mbc3 = true;
		pGBc_emuStatus->mbc5 = false;
		BREAK;
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
		pGBc_emuStatus->no_mbc = false;
		pGBc_emuStatus->mbc1 = false;
		pGBc_emuStatus->mbc2 = false;
		pGBc_emuStatus->mbc3 = false;
		pGBc_emuStatus->mbc5 = true;
		BREAK;
	default:
		WARN("This MBC type is not supported yet");
		BREAK;
	}
}

void GBc_t::setROMBankType(uint16_t romBankType)
{
	switch (romBankType)
	{
	case 0:
		pGBc_emuStatus->romBank32K = true;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 1:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = true;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 2:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = true;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 3:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = true;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 4:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = true;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 5:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = true;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 6:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = true;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 7:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = true;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 8:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = true;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 34:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = true;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 35:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = true;
		pGBc_emuStatus->romBank1_5M = false;
		BREAK;
	case 36:
		pGBc_emuStatus->romBank32K = false;
		pGBc_emuStatus->romBank64K = false;
		pGBc_emuStatus->romBank128K = false;
		pGBc_emuStatus->romBank256K = false;
		pGBc_emuStatus->romBank512K = false;
		pGBc_emuStatus->romBank1M = false;
		pGBc_emuStatus->romBank2M = false;
		pGBc_emuStatus->romBank4M = false;
		pGBc_emuStatus->romBank8M = false;
		pGBc_emuStatus->romBank1_1M = false;
		pGBc_emuStatus->romBank1_2M = false;
		pGBc_emuStatus->romBank1_5M = true;
		BREAK;
	default:
		WARN("Other ROM bank types are not supported yet");
		BREAK;
	}
}

void GBc_t::setROMBankNumber(uint16_t romBankNumber)
{
	pGBc_emuStatus->currentROMBankNumber.raw = romBankNumber;
}

uint16_t GBc_t::getROMBankNumber()
{
	RETURN ((pGBc_emuStatus->currentROMBankNumber.raw) % (getNumberOfROMBanksUsed()));
}

uint16_t GBc_t::getNumberOfROMBanksUsed()
{
	uint16_t numberOfROMSlots = ZERO;

	if (pGBc_emuStatus->romBank32K == true)
	{
		numberOfROMSlots = 2;
	}
	else if (pGBc_emuStatus->romBank64K == true)
	{
		numberOfROMSlots = 4;
	}
	else if (pGBc_emuStatus->romBank128K == true)
	{
		numberOfROMSlots = 8;
	}
	else if (pGBc_emuStatus->romBank256K == true)
	{
		numberOfROMSlots = 16;
	}
	else if (pGBc_emuStatus->romBank512K == true)
	{
		numberOfROMSlots = 32;
	}
	else if (pGBc_emuStatus->romBank1M == true)
	{
		numberOfROMSlots = 64;
	}
	else if (pGBc_emuStatus->romBank2M == true)
	{
		numberOfROMSlots = 128;
	}
	else if (pGBc_emuStatus->romBank4M == true)
	{
		numberOfROMSlots = 256;
	}
	else if (pGBc_emuStatus->romBank8M == true)
	{
		numberOfROMSlots = 512;
	}
	else if (pGBc_emuStatus->romBank1_1M == true)
	{
		numberOfROMSlots = 72;
	}
	else if (pGBc_emuStatus->romBank1_2M == true)
	{
		numberOfROMSlots = 80;
	}
	else if (pGBc_emuStatus->romBank1_5M == true)
	{
		numberOfROMSlots = 96;
	}

	RETURN numberOfROMSlots;
}

void GBc_t::setROMModeIfMBC1()
{
	if (pGBc_emuStatus->mbc1 == true)
	{
		pGBc_emuStatus->isMBC1_Mode1 = MBC1_ROM_MODE;
	}
	else
	{
		FATAL("Cartridge type is not MBC1");
	}
}

void GBc_t::setRAMModeInMBC1()
{
	if (pGBc_emuStatus->mbc1 == true)
	{
		pGBc_emuStatus->isMBC1_Mode1 = MBC1_RAM_MODE;
	}
	else
	{
		FATAL("Cartridge type is not MBC1");
	}
}

FLAG GBc_t::getROMOrRAMModeInMBC1()
{
	if (pGBc_emuStatus->mbc1 == true)
	{
		RETURN pGBc_emuStatus->isMBC1_Mode1;
	}
	else
	{
		FATAL("Cartridge type is not MBC1");
		RETURN pGBc_emuStatus->isMBC1_Mode1;
	}
}

void GBc_t::enableRAMBank()
{
	pGBc_emuStatus->enableRAMBanking = true;
}

void GBc_t::disableRAMBank()
{
	pGBc_emuStatus->enableRAMBanking = false;
}

FLAG GBc_t::isRAMBankEnabled()
{
	RETURN pGBc_emuStatus->enableRAMBanking;
}

void GBc_t::setRAMBankType(uint16_t ramBankType)
{
	switch (ramBankType)
	{
	case 0:
		pGBc_emuStatus->no_ramBank = true;
		pGBc_emuStatus->ramBank2K = false;
		pGBc_emuStatus->ramBank8K = false;
		pGBc_emuStatus->ramBank32K = false;
		pGBc_emuStatus->ramBank128K = false;
		pGBc_emuStatus->ramBank64K = false;
		BREAK;
	case 1:
		pGBc_emuStatus->no_ramBank = false;
		pGBc_emuStatus->ramBank2K = true;
		pGBc_emuStatus->ramBank8K = false;
		pGBc_emuStatus->ramBank32K = false;
		pGBc_emuStatus->ramBank128K = false;
		pGBc_emuStatus->ramBank64K = false;
		BREAK;
	case 2:
		pGBc_emuStatus->no_ramBank = false;
		pGBc_emuStatus->ramBank2K = false;
		pGBc_emuStatus->ramBank8K = true;
		pGBc_emuStatus->ramBank32K = false;
		pGBc_emuStatus->ramBank128K = false;
		pGBc_emuStatus->ramBank64K = false;
		BREAK;
	case 3:
		pGBc_emuStatus->no_ramBank = false;
		pGBc_emuStatus->ramBank2K = false;
		pGBc_emuStatus->ramBank8K = false;
		pGBc_emuStatus->ramBank32K = true;
		pGBc_emuStatus->ramBank128K = false;
		pGBc_emuStatus->ramBank64K = false;
		BREAK;
	case 4:
		pGBc_emuStatus->no_ramBank = false;
		pGBc_emuStatus->ramBank2K = false;
		pGBc_emuStatus->ramBank8K = false;
		pGBc_emuStatus->ramBank32K = false;
		pGBc_emuStatus->ramBank128K = true;
		pGBc_emuStatus->ramBank64K = false;
		BREAK;
	case 5:
		pGBc_emuStatus->no_ramBank = false;
		pGBc_emuStatus->ramBank2K = false;
		pGBc_emuStatus->ramBank8K = false;
		pGBc_emuStatus->ramBank32K = false;
		pGBc_emuStatus->ramBank128K = false;
		pGBc_emuStatus->ramBank64K = true;
		BREAK;
	default:
		WARN("Other RAM bank types are not supported yet");
		BREAK;
	}
}

uint8_t GBc_t::getRAMBankNumber()
{
	RETURN pGBc_emuStatus->currentRAMBankNumber;
}

void GBc_t::setRAMBankNumber(uint8_t ramBankNumber)
{
	pGBc_emuStatus->currentRAMBankNumber = ramBankNumber;
}

uint8_t GBc_t::getNumberOfRAMBanksUsed()
{
	uint8_t numberOfRAMSlots = ZERO;

	if (pGBc_emuStatus->no_ramBank == true)
	{
		numberOfRAMSlots = ONE;
	}
	else if (pGBc_emuStatus->ramBank2K == true)
	{
		numberOfRAMSlots = ONE;
	}
	else if (pGBc_emuStatus->ramBank8K == true)
	{
		numberOfRAMSlots = ONE;
	}
	else if (pGBc_emuStatus->ramBank32K == true)
	{
		numberOfRAMSlots = FOUR;
	}
	else if (pGBc_emuStatus->ramBank128K == true)
	{
		numberOfRAMSlots = SIXTEEN;
	}
	else if (pGBc_emuStatus->ramBank64K == true)
	{
		numberOfRAMSlots = EIGHT;
	}

	RETURN numberOfRAMSlots;
}

uint8_t GBc_t::getVRAMBankNumber()
{
	RETURN pGBc_emuStatus->currentVRAMBankNumber;
}

void GBc_t::setVRAMBankNumber(uint8_t vramBankNumber)
{
	pGBc_emuStatus->currentVRAMBankNumber = vramBankNumber;
}

uint8_t GBc_t::getWRAMBankNumber()
{
	RETURN pGBc_emuStatus->currentWRAMBankNumber;
}

void GBc_t::setWRAMBankNumber(uint8_t wramBankNumber)
{
	pGBc_emuStatus->currentWRAMBankNumber = wramBankNumber;
}

FLAG GBc_t::isCGBCompatibilityModeEnabled()
{
	RETURN pGBc_peripherals->KEY0.KEY0Fields.DMGCompatibility;
}

void GBc_t::cpuTickM(CPU_TICK_TYPE type)
{
	if (ROM_TYPE == ROM::TEST_SST)
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.cpuCounter++;

		if (type == CPU_TICK_TYPE::DUMMY)
		{
			++pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.indexer;
		}
	}
	else
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.cpuCounter++;

		// handle delayed write of 'actual' STAT in GB

		if (pGBc_instance->GBc_state.emulatorStatus.checkSTATTPlusOneCycle == YES)
		{
			pGBc_instance->GBc_state.emulatorStatus.checkSTATTPlusOneCycle = NO;
			BYTE readOnly = pGBc_instance->GBc_state.emulatorStatus.prevSTAT & 0x07;
			pGBc_instance->GBc_state.emulatorStatus.newSTAT &= 0x78;
			pGBc_instance->GBc_state.emulatorStatus.newSTAT |= readOnly;
			pGBc_peripherals->STAT.lcdStatusMemory = pGBc_instance->GBc_state.emulatorStatus.newSTAT;
			pGBc_peripherals->STAT.lcdStatusFields.UNUSED_07 = ONE;
			checkAllSTATInterrupts(NO);
		}

		// update flags if necessary

		pGBc_instance->GBc_state.emulatorStatus.isNewTimerCycle = YES;

		syncOtherGBModuleTicks();
	}
}

void GBc_t::gbCpuTick2T(FLAG isT2orT3)
{
	if (isT2orT3 == YES)
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.cpuCounter++;

		// handle delayed write of 'actual' STAT in GB

		if (pGBc_instance->GBc_state.emulatorStatus.checkSTATTPlusOneCycle == YES)
		{
			pGBc_instance->GBc_state.emulatorStatus.checkSTATTPlusOneCycle = NO;
			BYTE readOnly = pGBc_peripherals->STAT.lcdStatusMemory & 0x07;
			pGBc_instance->GBc_state.emulatorStatus.newSTAT &= 0x78;
			pGBc_instance->GBc_state.emulatorStatus.newSTAT |= readOnly;
			pGBc_peripherals->STAT.lcdStatusMemory = pGBc_instance->GBc_state.emulatorStatus.newSTAT;
			pGBc_peripherals->STAT.lcdStatusFields.UNUSED_07 = ONE;
			checkAllSTATInterrupts(NO);
		}

		// update flags if necessary

		pGBc_instance->GBc_state.emulatorStatus.isNewTimerCycle = YES;

		dmaTick();
	}
	timerTick();
	serialTick();
	rtcTick();
	ppuTick();
	apuTick();
	handleStopBasedHalt();
	timerTick();
	serialTick();
	rtcTick();
	ppuTick();
	apuTick();
	handleStopBasedHalt();
}

void GBc_t::syncOtherGBModuleTicks()
{
	// Helper Lamba Functions

	auto IS_VALID_TICK_FOR_DOUBLE_SPEED = [&]()
	{
		RETURN (pGBc_instance->GBc_state.emulatorStatus.ticks.isValidTickForDoubleSpeed == VALID_TICK);
	};

	auto SET_NEXT_TICK_FOR_DOUBLE_SPEED = [&]()
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.isValidTickForDoubleSpeed =
			!pGBc_instance->GBc_state.emulatorStatus.ticks.isValidTickForDoubleSpeed;
	};

	auto RESET_TICK_FOR_DOUBLE_SPEED = [&]()
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.isValidTickForDoubleSpeed = RESET_TICK;
	};

	// SOC Timing Sequence

	if (isCGBDoubleSpeedEnabled() == YES)
	{
		dmaTick();
		timerTick();
		serialTick();
		if (IS_VALID_TICK_FOR_DOUBLE_SPEED() == YES)
		{
			rtcTick();
			ppuTick();
			apuTick();
		}
		handleStopBasedHalt();
		timerTick();
		serialTick();
		if (IS_VALID_TICK_FOR_DOUBLE_SPEED() == YES)
		{
			rtcTick();
			ppuTick();
			apuTick();
		}
		handleStopBasedHalt();
		timerTick();
		serialTick();
		if (IS_VALID_TICK_FOR_DOUBLE_SPEED() == YES)
		{
			rtcTick();
			ppuTick();
			apuTick();
		}
		handleStopBasedHalt();
		timerTick();
		serialTick();
		if (IS_VALID_TICK_FOR_DOUBLE_SPEED() == YES)
		{
			rtcTick();
			ppuTick();
			apuTick();
		}
		handleStopBasedHalt();
		SET_NEXT_TICK_FOR_DOUBLE_SPEED();
	}
	else
	{
		RESET_TICK_FOR_DOUBLE_SPEED();
		dmaTick();
		timerTick();
		serialTick();
		rtcTick();
		ppuTick();
		apuTick();
		handleStopBasedHalt();
		timerTick();
		serialTick();
		rtcTick();
		ppuTick();
		apuTick();
		handleStopBasedHalt();
		timerTick();
		serialTick();
		rtcTick();
		ppuTick();
		apuTick();
		handleStopBasedHalt();
		timerTick();
		serialTick();
		rtcTick();
		ppuTick();
		apuTick();
		handleStopBasedHalt();
	}
}

void GBc_t::timerTick()
{
	// Refer : https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html#timer-overflow-behaviour

	// 1) divider

	pGBc_instance->GBc_state.emulatorStatus.ticks.dividerCounter = (uint16_t)((getGBDividerMSB() << EIGHT) | getGBDividerLSB());
	pGBc_instance->GBc_state.emulatorStatus.ticks.dividerCounter++;

	auto oldGBDividerMSB = getGBDividerMSB();

	setGBDividerLSB(pGBc_instance->GBc_state.emulatorStatus.ticks.dividerCounter & 0x00FF);				// sets the DIV LSByte
	setGBDividerMSB((pGBc_instance->GBc_state.emulatorStatus.ticks.dividerCounter & 0xFF00) >> EIGHT);	// sets the DIV MSByte

	// 2) div apu
	// process "DIV_APU" for APU
	// whenever 4th or 5th bit (depending on normal or double speed) of DIV MSByte goes from 1 -> 0, increment DIV APU
	BYTE bitToCheckForDivAPU = ((ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBDoubleSpeedEnabled() == YES) ? FIVE : FOUR);
	if (GETBIT(bitToCheckForDivAPU, oldGBDividerMSB) == ONE && GETBIT(bitToCheckForDivAPU, getGBDividerMSB()) == ZERO)
	{
		incrementDivAPU(ONE);
	}

	// 3) timer-overflow behaviour 
	// Refer : https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html#timer-overflow-behaviour

	if (pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow != INVALID)
	{
		++pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow;
	}

	// If 4 clocks elapsed after TIMA overflow, then request the Timer Interrupt
	if (pGBc_instance->GBc_state.emulatorStatus.waitingToRequestTimerInterrupt == YES)
	{
		if (pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow == FOUR)
		{
			pGBc_instance->GBc_state.emulatorStatus.waitingToRequestTimerInterrupt = NO;
			requestInterrupts(INTERRUPTS::TIMER_INTERRUPT);
			resetGBTimer(pGBc_peripherals->TMA); // reset TIMA to TMA
		}
	}

	if (pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow > EIGHT)
	{
		pGBc_instance->GBc_state.emulatorStatus.waitingToRequestTimerInterrupt = NO;
		pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow = INVALID;
	}

	TODO("Below deactivated timer obscure behaviour for GBC is mentioned in The \"Cycle-Accurate Game Boy Docs\", but this causes issues in mooneye timer tests");
#if (GB_GBC_ENABLE_CGB_OBSCURE_TIMER_BEHAVIOUR == YES)
	if (ROM_TYPE == ROM::GAME_BOY_COLOR)
	{
		FLAG timerEnable = (FLAG)pGBc_peripherals->TAC.timerControlFields.TIMER_ENABLE;
		SIGNAL muxout = (SIGNAL)getDIVSpecialBitStatus(getWhichGBTimerToUse());
		pGBc_instance->GBc_state.emulatorStatus.timaIncSignal = (timerEnable && ((!muxout) && pGBc_instance->GBc_state.emulatorStatus.fallingEdgeDetectorDelay));
		pGBc_instance->GBc_state.emulatorStatus.fallingEdgeDetectorDelay = muxout;
		pGBc_instance->GBc_state.emulatorStatus.instantTimerIF = NO;

	}
	else if (ROM_TYPE == ROM::GAME_BOY)
#else
	{
		pGBc_instance->GBc_state.emulatorStatus.timaIncSignal = ((!getTIMASignalForGB()) && pGBc_instance->GBc_state.emulatorStatus.fallingEdgeDetectorDelay);
		pGBc_instance->GBc_state.emulatorStatus.fallingEdgeDetectorDelay = getTIMASignalForGB();
	}
#endif

	if (pGBc_instance->GBc_state.emulatorStatus.timaIncSignal == HI)
	{
		// now, increment the actual GB timer

		BYTE timerValue = getGBTimer();	// get TIMA

		if (timerValue == 0xFF)
		{
			if (pGBc_instance->GBc_state.emulatorStatus.instantTimerIF == YES) // Strange overflow behaviour will not occur
			{
				requestInterrupts(INTERRUPTS::TIMER_INTERRUPT);
				resetGBTimer(pGBc_peripherals->TMA); // reset TIMA to TMA
				pGBc_instance->GBc_state.emulatorStatus.waitingToRequestTimerInterrupt = NO;
				pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow = INVALID;
				pGBc_instance->GBc_state.emulatorStatus.instantTimerIF = CLEAR;
			}
			else
			{
				pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow = RESET;
				pGBc_instance->GBc_state.emulatorStatus.waitingToRequestTimerInterrupt = YES;
				resetGBTimerToZero();
			}
		}
		else
		{
			timerValue++;
			setGBTimer(timerValue);	// set the incremented TIMA
		}
	}

	// reset the "New Timer Cycle" flag since the first timer cycle immediately after cpu cycle has ended
	pGBc_instance->GBc_state.emulatorStatus.isNewTimerCycle = NO;
}

void GBc_t::dmaTick()
{
	processDMA();

	if (ROM_TYPE == ROM::GAME_BOY_COLOR)
	{
		switch (pGBc_instance->GBc_state.emulatorStatus.cgbDMAMode)
		{
		case CGB_DMA_MODE::GPDMA:
		{
			processGPDMA();
			BREAK;
		}
		case CGB_DMA_MODE::HDMA:
		{
			processHDMA();
			BREAK;
		}
		default:
		{
			FATAL("DMA Mode Error");
		}
		}
	}
}

void GBc_t::serialTick()
{
	/*
	*   Supported Rates
	*   8192Hz		-	1KB/s	- Bit 1 cleared, Normal
	*	16384Hz		-	2KB/s	- Bit 1 cleared, Double Speed Mode
	*	262144Hz	-	32KB/s	- Bit 1 set,     Normal
	*	524288Hz	-	64KB/s	- Bit 1 set,     Double Speed Mode
	*/

	// NOTE: Don't handle for cgb double speed here as this is already taken care by the caller of "serialTick" i.e. "syncOtherGBModuleTicks"

	/*
	* For 8192Hz rate, we need to transfer 1 KBYTE every 8192 clocks
	* Now, we are comming to this function at 4194304 clocks
	* So, 4194304 / 8192 = 512
	* Hence, at every 512th entry to this function, we should transfer 1 byte out of the serial interface
	*/

	pGBc_instance->GBc_state.emulatorStatus.ticks.serialCounter++;

	if (pGBc_instance->GBc_state.emulatorStatus.ticks.serialCounter >= pGBc_instance->GBc_state.emulatorStatus.serialMaxClockPerTransfer)
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.serialCounter -= pGBc_instance->GBc_state.emulatorStatus.serialMaxClockPerTransfer;

		// Case where we are the Serial Master
		// SC == 0x81 (this means, SB has data that needs to be transferred)
		if (pGBc_peripherals->SC.scFields.CLOCK_SELECT == SERIAL_MASTER && pGBc_peripherals->SC.scFields.TRANSFER_ENABLE == ONE)
		{
			pGBc_instance->GBc_state.emulatorStatus.serialSlaveByteShiftCount = ZERO;

			// retrieve the data that we are supposed to transfer
			BYTE dataUnderTransfer = pGBc_peripherals->SB;

			// extract the "bitToBeSent" over serial interface
			BIT bitToBeSent = GETBIT(SEVEN, dataUnderTransfer);

			// shift the bits to left by one
			dataUnderTransfer <<= ONE;

			// send the "bitToBeSent" over serial interface
			while (!sendOverSerialLink(bitToBeSent))
			{
				;
			}

			// receive back "bitReceived" over serial interface
			FLAG rxStatus = FALSE;
			BIT bitReceived = ZERO;
			while (!receiveOverSerialLink(&bitReceived, &rxStatus, YES, _NETWORK_TIMEOUT_LIMIT))
			{
				;
			}

			if (GETBIT(ZERO, bitReceived) == ONE)
			{
				SETBIT(dataUnderTransfer, ZERO);
			}
			else
			{
				UNSETBIT(dataUnderTransfer, ZERO);
			}

			// update the SB (after the MSB was transferred out and LSB was transferred in)
			pGBc_peripherals->SB = dataUnderTransfer;

			// increment the shift counter
			pGBc_instance->GBc_state.emulatorStatus.serialMasterByteShiftCount++;

			// if all eight bits are shifted out...
			if (pGBc_instance->GBc_state.emulatorStatus.serialMasterByteShiftCount == EIGHT)
			{
				// reset the shift counter
				pGBc_instance->GBc_state.emulatorStatus.serialMasterByteShiftCount = RESET;

				// transfer is complete, so set bit 7 of SC to zero
				pGBc_peripherals->SC.scFields.TRANSFER_ENABLE = RESET;

				// request for serial interrupt
				requestInterrupts(INTERRUPTS::SERIAL_INTERRUPT);

				// go back to serial slave mode (let game request again to be in master mode if necessary)
				// pGBc_peripherals->SC.scFields.CLOCK_SELECT = SERIAL_SLAVE;
			}
		}
	}

	// Case where we are the Serial Slave
	// SC.CLOCK_SELECT == 0 (When we are in slave mode, state of SC.TRANSFER_ENABLE doesn't matter...)
	if (pGBc_peripherals->SC.scFields.CLOCK_SELECT == SERIAL_SLAVE)
	{
		pGBc_instance->GBc_state.emulatorStatus.serialMasterByteShiftCount = ZERO;

		// check if we received any data over serial link
		FLAG rxStatus = NO;
		BIT bitReceived = ZERO;
		receiveOverSerialLink(&bitReceived, &rxStatus, NO);

		// if we received data... set the LSB of SB register to the value present in received data
		if (rxStatus == YES)
		{
			BYTE dataUnderTransfer = pGBc_peripherals->SB;

			// extract the "bitToBeSent" over serial interface
			BIT bitToBeSent = GETBIT(SEVEN, dataUnderTransfer);

			// send the "bitToBeSent" over serial interface
			while (!sendOverSerialLink(bitToBeSent))
			{
				;
			}

			// shift the bits to left by one
			dataUnderTransfer <<= ONE;

			if (GETBIT(ZERO, bitReceived) == ONE)
			{
				SETBIT(dataUnderTransfer, ZERO);
			}
			else
			{
				UNSETBIT(dataUnderTransfer, ZERO);
			}

			// update the SB (after the MSB was transferred out and LSB was transferred in)
			pGBc_peripherals->SB = dataUnderTransfer;

			// increment the shift counter
			pGBc_instance->GBc_state.emulatorStatus.serialSlaveByteShiftCount++;

			// if all eight bits are shifted out...
			if (pGBc_instance->GBc_state.emulatorStatus.serialSlaveByteShiftCount == EIGHT)
			{
				// reset the shift counter
				pGBc_instance->GBc_state.emulatorStatus.serialSlaveByteShiftCount = RESET;

				// transfer is complete, so set bit 7 of SC to zero
				pGBc_peripherals->SC.scFields.TRANSFER_ENABLE = RESET;

				// request for serial interrupt
				requestInterrupts(INTERRUPTS::SERIAL_INTERRUPT);

				// go back to serial slave mode (let game request again to be in master mode if necessary)
				// pGBc_peripherals->SC.scFields.CLOCK_SELECT = SERIAL_SLAVE;
			}
		}
	}
}

void GBc_t::rtcTick()
{
	// Refer : https://gbdev.io/pandocs/MBC3.html?highlight=rtc#mbc3

	// Check if RTC is halted
	if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_HALT == ONE)
	{
		RETURN;
	}

	// Ideally, RTC should tick at 32768 Hz, i.e. it should increment by 0.0078125 second for every 32768 clocks
	// But our minimum resolution of RTC field is 1 second, i.e. the smallest unit that we can increment is by 1 second (fractional seconds is not possible)
	// Hence, instead of counting till 32768 clocks, we count till 4194304 clocks
	// Hence when "rtcCounter" reaches 4194304, this indicates 1 second of emulated time

	pGBc_instance->GBc_state.emulatorStatus.ticks.rtcCounter++;

	if (pGBc_instance->GBc_state.emulatorStatus.ticks.rtcCounter >= GB_GBC_REFERENCE_CLOCK_HZ)
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.rtcCounter = ZERO;

		// fetch the current day counter value (needed incase we need to increment the day counter)

		pGBc_instance->GBc_state.emulatorStatus.ticks.rtcDayCounter = getRTCDayCounter();

		// Increment the rtc second

		pGBc_instance->GBc_state.rtc.rtcFields.rtc_S++;

		if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_S > 0x3F) // 0x3F instead of 60 because of Invalid Rollover Tests mentioned in https://github.com/aaaaaa123456789/rtc3test/blob/master/tests.md
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_S = ZERO;
			RETURN;
		}
		else if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_S == 60) // ==60 instead of >59 because of Invalid Tick Tests mentioned in https://github.com/aaaaaa123456789/rtc3test/blob/master/tests.md
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_S = ZERO;
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_M++;
		}

		if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_M > 0x3F) // 0x3F instead of 60 because of Invalid Rollover Tests mentioned in https://github.com/aaaaaa123456789/rtc3test/blob/master/tests.md
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_M = ZERO;
			RETURN;
		}
		else if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_M == 60) // ==60 instead of >59 because of Invalid Tick Tests mentioned in https://github.com/aaaaaa123456789/rtc3test/blob/master/tests.md
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_M = ZERO;
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_H++;
		}

		if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_H > 0x1F) // 0x1F instead of 60 because of Invalid Rollover Tests mentioned in https://github.com/aaaaaa123456789/rtc3test/blob/master/tests.md
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_H = ZERO;
			RETURN;
		}
		else if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_H == 24) // ==24 instead of >23 because of Invalid Tick Tests mentioned in https://github.com/aaaaaa123456789/rtc3test/blob/master/tests.md
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_H = ZERO;
			pGBc_instance->GBc_state.emulatorStatus.ticks.rtcDayCounter++;
		}

		pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL = (pGBc_instance->GBc_state.emulatorStatus.ticks.rtcDayCounter & 0xFF);

		if (GETBIT(EIGHT, pGBc_instance->GBc_state.emulatorStatus.ticks.rtcDayCounter) == ONE)
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_MSB = ONE;
		}
		else
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_MSB = ZERO;
		}

		if (pGBc_instance->GBc_state.emulatorStatus.ticks.rtcDayCounter > 511)
		{
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL = ZERO;
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_MSB = ZERO;
			pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_CARRY = ONE;
		}
	}
}

void GBc_t::requestVblankStatInterrupt()
{
	if (isPPULCDEnabled() == NO)
	{
		// Note that LY==LYC signal should not be made zero here as PPU disable means 0 == 0 condition is satisfied
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
		RETURN;
	}

	if (pGBc_peripherals->STAT.lcdStatusFields.MODE1_VBLANK_STAT_INT_SRC == ONE)
	{
		if (pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.aggregateSignal == LO)
		{
			pGBc_instance->GBc_state.emulatorStatus.STAT_src = STAT_INTR_SRC::VBLANK;
			requestInterrupts(INTERRUPTS::LCD_STAT_INTERRUPT);
		}

		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = HI;
	}

	pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
	pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
}

void GBc_t::requestOamStatInterrupt()
{
	if (isPPULCDEnabled() == NO)
	{
		// Note that LY==LYC signal should not be made zero here as PPU disable means 0 == 0 condition is satisfied
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
		RETURN;
	}

	if (pGBc_peripherals->STAT.lcdStatusFields.MODE2_OAM_STAT_INT_SRC == ONE)
	{
		if (pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.aggregateSignal == LO)
		{
			pGBc_instance->GBc_state.emulatorStatus.STAT_src = STAT_INTR_SRC::OAM;
			requestInterrupts(INTERRUPTS::LCD_STAT_INTERRUPT);
		}

		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = HI;
	}

	pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
	pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
}

void GBc_t::requestHblankStatInterrupt()
{
	if (isPPULCDEnabled() == NO)
	{
		// Note that LY==LYC signal should not be made zero here as PPU disable means 0 == 0 condition is satisfied
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
		RETURN;
	}

	if (pGBc_peripherals->STAT.lcdStatusFields.MODE0_HBLANK_STAT_INT_SRC == ONE)
	{
		if (pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.aggregateSignal == LO)
		{
			pGBc_instance->GBc_state.emulatorStatus.STAT_src = STAT_INTR_SRC::HBLANK;
			requestInterrupts(INTERRUPTS::LCD_STAT_INTERRUPT);
		}

		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = HI;
	}

	pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
	pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
}

void GBc_t::ppuTick()
{
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY++;
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode++;
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerFrame++;

#if _DEBUG
	++OAM_STAT_TO_MODE_2_T_CYCLES;
#endif

	FLAG effectivePPUState = isPPULCDEnabled();

	// Refer https://gbdev.io/pandocs/Reducing_Power_Consumption.html?highlight=STOP#using-the-stop-instruction
	if (pGBc_instance->GBc_state.emulatorStatus.isCPUStopped == YES) MASQ_UNLIKELY
	{
		if (effectivePPUState == ENABLED)
		{
			if (pGBc_instance->GBc_state.emulatorStatus.freezeLCD == NO) MASQ_UNLIKELY
			{
				Pixel DARK = { 0x00 };
				if ((ROM_TYPE == ROM::GAME_BOY_COLOR) && (pGBc_display->currentLCDMode != LCD_MODES::MODE_LCD_DISPLAY_PIXELS))
				{
					DARK = getColorFromColorIDForGBC(0x0000, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
					std::fill_n(pGBc_display->imGuiBuffer.imGuiBuffer1D, sizeof(pGBc_display->imGuiBuffer.imGuiBuffer1D), DARK);
				}
				else if (ROM_TYPE == ROM::GAME_BOY)
				{
					DARK = paletteIDToColor.at(pGBc_instance->GBc_state.gb_palette).COLOR_099P.COLOR;
					std::fill_n(pGBc_display->imGuiBuffer.imGuiBuffer1D, sizeof(pGBc_display->imGuiBuffer.imGuiBuffer1D), DARK);
				}
				pGBc_instance->GBc_state.emulatorStatus.freezeLCD = YES;
			}

			effectivePPUState = DISABLED;
		}
	}

	// Can refer https://discord.com/channels/465585922579103744/465586075830845475/1025910017754419290 for STAT timing
	// Also refer https://forums.nesdev.org/viewtopic.php?f=20&t=13727#p162444

	if (effectivePPUState == ENABLED)
	{
		// If not in STOP, PPU is not 'DARK' anymore...
		pGBc_instance->GBc_state.emulatorStatus.freezeLCD = NO;

		switch (pGBc_display->currentLCDMode)
		{
		case LCD_MODES::MODE_LCD_V_BLANK:
		{
			// Allow VRAM and OAM read access if blocked
			pGBc_display->blockVramR = NO;
			pGBc_display->blockOAMR = NO;

			// Allow VRAM and OAM write access if blocked
			pGBc_display->blockVramR = NO;
			pGBc_display->blockOAMR = NO;

			// Refer https://www.reddit.com/r/emulation/comments/68p6wt/comment/dh3me0b/?utm_source=reddit&utm_medium=web2x&context=3
			// When LY == 153, LY register should reads 0 after the initial 4 dots of the scanline is done, eventhough we are still in VBLANK
			if ((pGBc_peripherals->LY == (getScreenHeight() + VBLANK_SCANLINES - ONE)) || (pGBc_display->isTheLastVblankLine == YES))
			{
				pGBc_display->isTheLastVblankLine = YES;

				if (ROM_TYPE == ROM::GAME_BOY)
				{
					/*
					 * Keeping ppuCounterPerLY == 5 passes ly_lyc_153_write-GS.gb but fails ly_new_frame-GS.gb and ly_lyc_0-GS.gb
					 * Keeping ppuCounterPerLY == 4 passes ly_new_frame-GS.gb and ly_lyc_0-GS.gb but fails ly_lyc_153_write-GS.gb
					 */
					PPUTODO("Conflicting PPU test results b/w ly_new_frame-GS.gb, ly_lyc_0-GS.gb and ly_lyc_153_write-GS.gb");
					// > 4 cause issues
					if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == FOUR)
					{
						// Process LY == LYC for LY = 153
						compareLYToLYC(pGBc_peripherals->LY);

						// Reset LY to zero post LY=LYC check
						pGBc_display->currentScanline = ZERO;
						pGBc_peripherals->LY = ZERO;
					}
					else if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == SIX)
					{
						// Clear LY == LYC
						pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_FLAG = ZERO;
						pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.LY_LYC_SIGNAL = LO;
					}
					// > 12 causes issues
					else if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == TEN)
					{
						// Process LY == LYC for LY = 0
						compareLYToLYC(pGBc_peripherals->LY);
					}
				}
				else
				{
					if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == TWO)
					{
						// Process LY == LYC for LY = 153
						compareLYToLYC(pGBc_peripherals->LY);
					}
					// > 8 causes issues
					else if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == SIX)
					{
						// Process LY == LYC for LY = 153
						compareLYToLYC(pGBc_peripherals->LY);

						// Reset LY to zero post LY=LYC check
						pGBc_display->currentScanline = ZERO;
						pGBc_peripherals->LY = ZERO;
					}
					// > 12 causes issues
					else if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == TEN)
					{
						// Process LY == LYC for LY = 0
						compareLYToLYC(pGBc_peripherals->LY);
					}
				}
			}
			// When LY = 144 - 152
			else
			{
				if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == ONE)
				{
					if (pGBc_peripherals->LY == getScreenHeight())
					{
						// Request VBLANK interrupt
						requestInterrupts(INTERRUPTS::VBLANK_INTERRUPT);

						// Generate STAT interrupt if required
						requestVblankStatInterrupt();

						// Set PPU mode
						setPPULCDMode(LCD_MODES::MODE_LCD_V_BLANK);

						// Clear the PPU mode specific STAT IRQ lines
						pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
						pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
					}

					// Process LY == LYC
					compareLYToLYC(pGBc_peripherals->LY);
				}
			}

			// PPU ticks per line >= 456
			if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == LCD_MODE_CYCLES::LCD_TOTAL_CYCLES_PER_SCANLINE)
			{
				// 144 <= LY < 153
				if ((pGBc_peripherals->LY >= getScreenHeight()) && (pGBc_peripherals->LY < (getScreenHeight() + VBLANK_SCANLINES - ONE)))
				{
					// Increment LY
					pGBc_display->currentScanline++;
					pGBc_peripherals->LY = pGBc_display->currentScanline;

					// Needed by round 7 of "ly_lyc_144-GS.gb" and "ly_lyc_144-C.gb"
					if (ROM_TYPE == ROM::GAME_BOY)
					{
						pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_FLAG = ZERO;
						pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.LY_LYC_SIGNAL = LO;
					}
				}

				if (pGBc_display->isTheLastVblankLine == YES)
				{
					pGBc_display->isTheLastVblankLine = CLEAR;

					PPUINFO("LCD MODE : V-Blank; Dots : %d", pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode);
					pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode = RESET;

					// Set the next LCD Mode
					pGBc_display->currentLCDMode = LCD_MODES::MODE_LCD_SEARCHING_OAM;

					if (ROM_TYPE == ROM::GAME_BOY)
					{
						/*
						* Needed by "ly00_mode0_2-GS.gb", "ly_lyc_0-GS.gb" and "stat_irq_blocking.gb"
						* Make sure to run with bios enabled
						*/
						setPPULCDMode(LCD_MODES::MODE_LCD_BITS_CLEAR);
					}

					// Reset PPU mode counter
					pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerFrame = ZERO;
				}

				// PPU ticks per line
				pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY = ZERO;
			}

			BREAK;
		}
		case LCD_MODES::MODE_LCD_SEARCHING_OAM:
		{
			// Allow VRAM read access to CPU if blocked
			pGBc_display->blockVramR = NO;

			// Block OAM read access to CPU
			pGBc_display->blockOAMR = YES;

			// Allow VRAM write access to CPU if blocked
			pGBc_display->blockVramW = NO;

			// Block OAM write access to CPU
			pGBc_display->blockOAMW = YES;

			// Initialize the below variable at the very first tick of the MODE_LCD_SEARCHING_OAM
			if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == ONE)
			{
				// Clear the PPU mode specific STAT IRQ lines
				pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
				pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;

				/*
				* As per section 8.11 of https://raw.githubusercontent.com/AntonioND/giibiiadvance/master/docs/TCAGBD.pdf
				* STAT interrupt for line 0 happens in next M cycle when compared to lines 1 to 143
				* For T-cycle accuracy, we refered to sameboy cycles provided in 
				* https://discord.com/channels/465585922579103744/465586075830845475/1025910040395251732
				* which mentions that STAT interrupt happens at same cycle as mode set, not 1-T cycle before
				* Many mooneye and wilbert pol's tests depend on this
				*/
				if (pGBc_peripherals->LY == ZERO)
				{
					requestOamStatInterrupt();
				}

				// Set the LCD Mode bits
				setPPULCDMode(LCD_MODES::MODE_LCD_SEARCHING_OAM);

				// Reset the fetcher FSM
				pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;

				// Set the dummy fetch flag as we are in new scanline
				pGBc_display->fakeBgFetcherRuns = ZERO;

				// Reset the Pixel FIFOs
				pGBc_display->bgWinPixelFIFO.clearFIFO();
				pGBc_display->tempBgWinPixelFIFO.clearFIFO();
				pGBc_display->objPixelFIFO.clearFIFO();

				// Reset other flags
				pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = CLEAR;
				pGBc_display->isThereAnyObjectCurrentlyGettingRendered = CLEAR;
				pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray = INVALID;
				pGBc_display->visibleObjectsPerScanLine = NULL;

				pGBc_display->shouldIncrementWindowLineCounter = CLEAR;
				pGBc_display->waitForNextLineForWindSyncGlitch = NO;
				pGBc_display->performWindSyncGlitch = NO;
				pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
				pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = NO;

				pGBc_display->pixelFetcherCounterPerScanLine = RESET;
				pGBc_display->pixelRenderCounterPerScanLine = -EIGHT;

				pGBc_display->pixelFetcherDots = RESET;
				pGBc_display->pixelRendererDots = RESET;
				pGBc_display->pixelPipelineDots = RESET;

				pGBc_display->shouldSimulateBGScrollingPenaltyNow = YES;

				pGBc_display->oamSearchCount = ZERO;
				pGBc_display->spriteCountPerScanLine = ZERO;
				pGBc_display->nX159SpritesPresent = ZERO;

				pGBc_display->bgToObjectPenalty = NO;
				pGBc_display->discardedPixelCount = RESET;

				pGBc_display->wasFetchingOBJ = NO;
				pGBc_display->prevSpriteX = INVALID;
				pGBc_display->wasNotFirstSpriteInX = NO;
				pGBc_display->wasX0Object = NO;

				pGBc_display->x159SpritesPresent = NO;
				pGBc_display->x159SpritesDone = NO;

				// clear "visibleOamIndexPerLY" as we are in new frame
				visibleOamIndexPerLY.clear();

				// Check whether "Y" window layer is triggerred for current scanline
				// Refer : https://gbdev.io/pandocs/Scrolling.html#window
				// Refer : https://discord.com/channels/465585922579103744/465586075830845475/852208456491728897
				// Refer : https://discord.com/channels/465585922579103744/465586075830845475/1295044210654842980
				// Note that WINDOW_LAYER_ENABLE should not be checked here as mentioned in https://discord.com/channels/465585922579103744/465586075830845475/757342004052099072
				if (pGBc_peripherals->LY == pGBc_peripherals->WY)
				{
					pGBc_display->yConditionForWindowIsMetForCurrentFrame = YES;
				}
			}

			if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == ONE)
			{
				// Process LY == LYC
				compareLYToLYC(pGBc_peripherals->LY);
			}

			// The operation of searching OAM for valid sprites should be equally split among the 80 cycles of MODE_LCD_SEARCHING_OAM
			// In total, we need to search through 40 sprites, hence we take 2 dots per oam entry
			if (((pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY & 0x01) == ZERO) // every alternate cycle
				&& pGBc_display->oamSearchCount < FORTY) // itterate over all the 40 sprites in OAM memory
			{
				if (pGBc_display->spriteCountPerScanLine < TEN)
				{
					int16_t yPosition = pGBc_memory->GBcMemoryMap.mOAM.OAMFields.OAM[pGBc_display->oamSearchCount].yPosition - SIXTEEN;

					auto verticalSizeOfSprite = (pGBc_peripherals->LCDC.lcdControlFields.OBJ_SIZE == ONE) ? SIXTEEN : EIGHT;

					if ((yPosition <= pGBc_peripherals->LY) && (pGBc_peripherals->LY < yPosition + verticalSizeOfSprite))
					{
						visibleObjects_t* entry =
							&(pGBc_display->arrayOfVisibleObjectsPerScanLine[pGBc_display->spriteCountPerScanLine]);

						++pGBc_display->spriteCountPerScanLine;

						entry->oamEntry = pGBc_memory->GBcMemoryMap.mOAM.OAMFields.OAM[pGBc_display->oamSearchCount];
						entry->indexWithinOAMMemory = pGBc_display->oamSearchCount;
						entry->alreadyProcessed = NO;
						entry->next = NULL;

						PPUTODO("Kind of a dirty hack (part 1) for handling timing for sprites at X = 159 at %d in %s", __LINE__, __FILE__);
						const DIM8 xPosSLimit = getScreenWidth() + EIGHT - ONE;
						if (pGBc_memory->GBcMemoryMap.mOAM.OAMFields.OAM[pGBc_display->oamSearchCount].xPosition == xPosSLimit)
						{
							++pGBc_display->nX159SpritesPresent;
							pGBc_display->x159SpritesPresent = YES;
						}

						// for debugger
						visibleOamIndexPerLY.insert(TO_UINT8(pGBc_display->oamSearchCount));
						visibleOamIndexPerFrame.insert(TO_UINT8(pGBc_display->oamSearchCount));

						if (!pGBc_display->visibleObjectsPerScanLine)
						{
							entry->next = pGBc_display->visibleObjectsPerScanLine;
							pGBc_display->visibleObjectsPerScanLine = entry;
						}
						else
						{
							visibleObjects_t* le = pGBc_display->visibleObjectsPerScanLine;

							while (le->next)
							{
								le = le->next;
							}

							le->next = entry;
						}
					}
				}

				++pGBc_display->oamSearchCount;
			}

			// PPU ticks per line >= 80 AND we have searched through all possible OAM sprites
			if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == LCD_MODE_CYCLES::LCD_SEARCHING_OAM
				&& pGBc_display->oamSearchCount == FORTY)
			{
				pGBc_display->isNewM3Scanline = YES;

				PPUINFO("LCD MODE : OAM Search; Dots : %d", pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode);
				pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode = RESET;

				// Set the next LCD Mode
				pGBc_display->currentLCDMode = LCD_MODES::MODE_LCD_DISPLAY_PIXELS;

				// To handle double-halt-cancel.gb and double-halt-cancel-gbconly.gb
				if (ROM_TYPE == ROM::GAME_BOY)
				{
					// Block VRAM access to CPU
					pGBc_display->blockVramR = YES;
				}

				// Un-block OAM write access to CPU (just for this cycle, will get blocked again in next cycle)
				// This momentary unblocking is needed by lcdon_write_timing-GS.gb
				pGBc_display->blockOAMW = NO;
			}

			BREAK;
		}
		case LCD_MODES::MODE_LCD_DISPLAY_PIXELS:
		{
			// Block VRAM read access to CPU (Needed when we directly come to mode 3 in line 0 post lcd on)
			pGBc_display->blockVramR = YES;

			// Block OAM read access to CPU (Needed when we directly come to mode 3 in line 0 post lcd on)
			pGBc_display->blockOAMR = YES;

			// Block VRAM write access to CPU (Needed when we directly come to mode 3 in line 0 post lcd on)
			pGBc_display->blockVramW = YES;

			// Block OAM write access to CPU (Needed when we directly come to mode 3 in line 0 post lcd on)
			pGBc_display->blockOAMW = YES;

			// Block CGB palette access to CPU
			pGBc_display->blockCGBPalette = YES;

			// Set the LCD Mode bits
			processPixelPipelineAndRender(ONE);

			if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == EIGHTYONE)
			{
				pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
				pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
				pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
				setPPULCDMode(LCD_MODES::MODE_LCD_DISPLAY_PIXELS);
			}

#if (GB_GBC_ENABLE_PPU_BG_MODE3_RUN_FOR_174_DOTS == YES)
			/*
			* Since we can remain in pixelRenderCounterPerScanLine == 159 condition for multiple dots for example when multiple sprites are present in X == 159 location
			* Ideally, what I want is to set the HBLANK mode on the last dot where pixelRenderCounterPerScanLine == 159
			* To detemine this, we have a very dirty solution below
			*/
			PPUTODO("Kind of a dirty hack (part 3) for handling timing for sprites at X = 159 at %d in %s", __LINE__, __FILE__);
			PPUTODO("Ideally, how is the below messed up if condition at %d in %s handled by GB/GBC HW?", __LINE__, __FILE__);
			if ((getPPULCDMode() != LCD_MODES::MODE_LCD_H_BLANK)
				&& (pGBc_display->pixelRenderCounterPerScanLine >= (TO_UINT16(getScreenWidth()) - ONE))
				&& (pGBc_display->shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone == NO)
				&& (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == NO))
			{
				FLAG proceedX159Sprites = (pGBc_display->x159SpritesPresent == NO) || (pGBc_display->x159SpritesDone == YES);
				if (proceedX159Sprites)
				{
					// Set the LCD Mode bits
					setPPULCDMode(LCD_MODES::MODE_LCD_H_BLANK);
#if _DEBUG
					constexpr COUNTER32 T_CYCLE_THR = LCD_SEARCHING_OAM + TX_DATA_LCD_CTRL_MIN + ONE;
					if ((OAM_STAT_TO_MODE_2_T_CYCLES > T_CYCLE_THR) && (pGBc_peripherals->LY != ZERO))
					{
						DEBUG("OAM_STAT_TO_MODE_2_T_CYCLES : %d", OAM_STAT_TO_MODE_2_T_CYCLES);
					}
#endif
				}
			}
#endif

			if (pGBc_display->pixelRenderCounterPerScanLine == TO_UINT16(getScreenWidth()))
			{
				if (pGBc_display->shouldIncrementWindowLineCounter == YES)
				{
					pGBc_display->windowLineCounter++;
				}

				pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = CLEAR;
				pGBc_display->isThereAnyObjectCurrentlyGettingRendered = CLEAR;
				pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray = INVALID;
				pGBc_display->visibleObjectsPerScanLine = NULL;

				pGBc_display->shouldIncrementWindowLineCounter = CLEAR;
				pGBc_display->waitForNextLineForWindSyncGlitch = NO;
				pGBc_display->performWindSyncGlitch = NO;
				pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
				pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = NO;

				pGBc_display->pixelFetcherCounterPerScanLine = RESET;
				pGBc_display->pixelRenderCounterPerScanLine = -EIGHT;

				pGBc_display->pixelFetcherDots = RESET;
				pGBc_display->pixelRendererDots = RESET;
				pGBc_display->pixelPipelineDots = RESET;

				PPUINFO("LCD MODE : Display Pixels; Dots : %d", pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode);
				pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode = RESET;

				// Set the next LCD Mode
				pGBc_display->tickAtMode3ToMode0 = pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY;
				pGBc_display->currentLCDMode = LCD_MODES::MODE_LCD_H_BLANK;

#if (GB_GBC_ENABLE_PPU_BG_MODE3_RUN_FOR_174_DOTS == YES)
				// Generate STAT interrupt if required
				requestHblankStatInterrupt();
#endif

				// Allow OAM read access to CPU if blocked
				pGBc_display->blockOAMR = NO;

				// Keep OAM write access blocked to CPU
				pGBc_display->blockOAMW = YES;

				// We are about enter HBLANK, so allow HDMA to block cpu pipeline if proper conditions are met
				if (pGBc_instance->GBc_state.emulatorStatus.cgbDMAMode == CGB_DMA_MODE::HDMA
					&& pGBc_instance->GBc_state.emulatorStatus.isHDMAActive == YES)
				{
					pGBc_instance->GBc_state.emulatorStatus.isHDMAAllowedToBlockCPUPipeline = YES;
				}
			}
			BREAK;
		}
		case LCD_MODES::MODE_LCD_H_BLANK:
		{
			pGBc_display->fakeBgFetcherRuns = ZERO;

			// Allow VRAM and OAM read access to CPU if blocked
			pGBc_display->blockVramR = NO;
			pGBc_display->blockOAMR = NO;

			// Allow VRAM and OAM write access to CPU if blocked
			pGBc_display->blockVramW = NO;
			pGBc_display->blockOAMW = NO;

			// Allow CGB palette access to CPU
			pGBc_display->blockCGBPalette = NO;

#if (GB_GBC_ENABLE_PPU_BG_MODE3_RUN_FOR_174_DOTS == NO)
			if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == pGBc_display->tickAtMode3ToMode0 + ONE)
			{
				// Set the LCD Mode bits
				setPPULCDMode(LCD_MODES::MODE_LCD_H_BLANK);	
			}

			if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == pGBc_display->tickAtMode3ToMode0 + TWO)
			{
				// Generate STAT interrupt if required
				requestHblankStatInterrupt();
			}
#endif

			// PPU ticks per line >= 456
			if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY == LCD_MODE_CYCLES::LCD_TOTAL_CYCLES_PER_SCANLINE)
			{
				if (pGBc_display->skipMode2 == YES && pGBc_display->currentScanline == RESET)
				{
					pGBc_display->skipMode2 = NO;
					pGBc_display->lcdJustEn = CLEAR;

					if (ENABLED)
					{
						// Reset the fetcher FSM
						pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;

						// Set the dummy fetch flag as we are in new scanline
						pGBc_display->fakeBgFetcherRuns = ZERO;

						// Reset the Pixel FIFOs
						pGBc_display->bgWinPixelFIFO.clearFIFO();
						pGBc_display->tempBgWinPixelFIFO.clearFIFO();
						pGBc_display->objPixelFIFO.clearFIFO();

						// Reset other flags
						pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = CLEAR;
						pGBc_display->isThereAnyObjectCurrentlyGettingRendered = CLEAR;
						pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray = INVALID;
						pGBc_display->visibleObjectsPerScanLine = NULL;

						pGBc_display->shouldIncrementWindowLineCounter = CLEAR;
						pGBc_display->waitForNextLineForWindSyncGlitch = NO;
						pGBc_display->performWindSyncGlitch = NO;
						pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
						pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = NO;

						pGBc_display->pixelFetcherCounterPerScanLine = RESET;
						pGBc_display->pixelRenderCounterPerScanLine = -EIGHT;

						pGBc_display->pixelFetcherDots = RESET;
						pGBc_display->pixelRendererDots = RESET;
						pGBc_display->pixelPipelineDots = RESET;

						pGBc_display->shouldSimulateBGScrollingPenaltyNow = YES;

						pGBc_display->oamSearchCount = ZERO;
						pGBc_display->spriteCountPerScanLine = ZERO;

						pGBc_display->bgToObjectPenalty = NO;
						pGBc_display->discardedPixelCount = RESET;

						pGBc_display->wasFetchingOBJ = NO;
						pGBc_display->prevSpriteX = INVALID;
						pGBc_display->wasNotFirstSpriteInX = NO;
						pGBc_display->wasX0Object = NO;

						pGBc_display->x159SpritesPresent = NO;
						pGBc_display->nX159SpritesPresent = ZERO;
						pGBc_display->x159SpritesDone = NO;

						// clear "visibleOamIndexPerLY" as we are in new frame
						visibleOamIndexPerLY.clear();
					}

					pGBc_display->isNewM3Scanline = YES;

					PPUINFO("LCD MODE : H-Blank; Dots : %d", pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode);
					pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode = RESET;

					// Set the next LCD Mode
					pGBc_display->currentLCDMode = LCD_MODES::MODE_LCD_DISPLAY_PIXELS;

					// PPU ticks per line (Resetting to 80 to handle special case of skiping mode 2)
					pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY = EIGHTY;
				}
				else
				{
					PPUINFO("LCD MODE : H-Blank; Dots : %d", pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode);
					pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode = RESET;

					// Increment LY
					pGBc_display->currentScanline++;
					pGBc_peripherals->LY = pGBc_display->currentScanline;

					/*
					* As per section 8.11 of https://raw.githubusercontent.com/AntonioND/giibiiadvance/master/docs/TCAGBD.pdf
					* STAT interrupt for line 0 happens in next M cycle when compared to lines 1 to 143
					* For T-cycle accuracy, we refered to sameboy cycles provided in
					* https://discord.com/channels/465585922579103744/465586075830845475/1025910040395251732
					* which mentions that STAT interrupt happens at same cycle as mode set, not 1-T cycle before
					* Many mooneye and wilbert pol's tests depend on this
					*/
					if (pGBc_display->currentScanline != ZERO)
					{
						// Generate STAT interrupt if required
						// Note: OAM STAT interrupt will be set even if we are about to enter VBlank state
						// Refer : https://github.com/Gekkio/mooneye-test-suite/blob/main/acceptance/ppu/vblank_stat_intr-GS.s
						requestOamStatInterrupt();

#if _DEBUG
						OAM_STAT_TO_MODE_2_T_CYCLES = RESET;
#endif
					}

					// LY == 144
					if (pGBc_display->currentScanline == getScreenHeight())
					{
						// Set the next LCD Mode
						pGBc_display->currentLCDMode = LCD_MODES::MODE_LCD_V_BLANK;

						// Clear "visibleOamIndexPerFrame" as we are enterring Vblank (and new frame)
						visibleOamIndexPerFrame.clear();

						// Set the VBLANK flag
						pGBc_display->wasVblankJustTriggerred = YES;

						// Handle blanking of LCD if needed here as VBLANK was just triggered
						if (pGBc_emuStatus->freezeLCDOneFrame == YES) MASQ_UNLIKELY
						{
							Pixel FROZEN = { 0x00 };
							if (ROM_TYPE == ROM::GAME_BOY_COLOR)
							{
								FROZEN = getColorFromColorIDForGBC(0x7FFF, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
							}
							else
							{
								FROZEN = paletteIDToColor.at(pGBc_instance->GBc_state.gb_palette).COLOR_000P.COLOR;
							}

							std::fill_n(pGBc_display->imGuiBuffer.imGuiBuffer1D, sizeof(pGBc_display->imGuiBuffer.imGuiBuffer1D), FROZEN);
							pGBc_instance->GBc_state.emulatorStatus.freezeLCDOneFrame = CLEAR;
						}

						// Reset the window line counter since we are in Vblank
						pGBc_display->windowLineCounter = RESET;

						// Reset the y condition latch for window
						// Refer to https://discord.com/channels/465585922579103744/465586075830845475/852208456491728897
						pGBc_display->yConditionForWindowIsMetForCurrentFrame = NO;

						if (ROM_TYPE == ROM::GAME_BOY)
						{
							// Refer 8.9.1 of https://raw.githubusercontent.com/AntonioND/giibiiadvance/master/docs/TCAGBD.pdf
							pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_FLAG = ZERO;
							pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.LY_LYC_SIGNAL = LO;
						}
					}
					else
					{
						// Set the next LCD Mode
						pGBc_display->currentLCDMode = LCD_MODES::MODE_LCD_SEARCHING_OAM;

						// Block OAM read access to CPU
						pGBc_display->blockOAMR = YES;

						if (ROM_TYPE == ROM::GAME_BOY_COLOR)
						{
							// Keep OAM write access blocked to CPU
							pGBc_display->blockOAMW = YES;
						}
						else if (ROM_TYPE == ROM::GAME_BOY)
						{
							// Keep OAM write access unblocked to CPU
							pGBc_display->blockOAMW = NO;

							// Refer 8.9.1 of https://raw.githubusercontent.com/AntonioND/giibiiadvance/master/docs/TCAGBD.pdf
							// To handle ly_lyc_0-GS.gb and ly_lyc_0-C.gb
							pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_FLAG = ZERO;
							pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.LY_LYC_SIGNAL = LO;
						}
					}

					// PPU ticks per line (Resetting to 0 for next line)
					pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY = ZERO;
				}
			}

			BREAK;
		}
		default:
		{
			FATAL("Invalid PPU Mode");
		}
		}
	}
	else
	{
		if (pGBc_instance->GBc_state.emulatorStatus.freezeLCD == NO) MASQ_UNLIKELY
		{
			Pixel FROZEN = { 0x00 };
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				FROZEN = getColorFromColorIDForGBC(0x7FFF, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
			}
			else
			{
				FROZEN = paletteIDToColor.at(pGBc_instance->GBc_state.gb_palette).COLOR_000P.COLOR;
			}

			std::fill_n(pGBc_display->imGuiBuffer.imGuiBuffer1D, sizeof(pGBc_display->imGuiBuffer.imGuiBuffer1D), FROZEN);
			pGBc_instance->GBc_state.emulatorStatus.freezeLCD = YES;
		}

		pGBc_display->fakeBgFetcherRuns = ZERO;

		// Allow VRAM access if blocked
		pGBc_display->blockOAMR = NO;
		pGBc_display->blockVramR = NO;
		pGBc_display->blockOAMW = NO;
		pGBc_display->blockVramW = NO;
		pGBc_display->blockCGBPalette = NO;

		if (pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerFrame >= PPU_CYCLES_PER_FRAME)
		{
			pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY = ZERO;
			pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode = ZERO;
			pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerFrame = ZERO;

			// Set the VBLANK flag
			pGBc_display->wasVblankJustTriggerred = YES;
		}
	}

#if (GB_GBC_ENABLE_TILE_SEL_GLITCH == YES)
	// Refer : https://github.com/mattcurrie/mealybug-tearoom-tests/blob/master/the-comprehensive-game-boy-ppu-documentation.md#tile_sel-bit-4
	if (pGBc_display->tileSelGlitchTCycles > RESET)
	{
		--pGBc_display->tileSelGlitchTCycles;
	}
#endif

#if (GB_GBC_ENABLE_CGB_SCY_WRITE_DELAY == YES)
	// Refer : https://github.com/mattcurrie/mealybug-tearoom-tests/blob/master/the-comprehensive-game-boy-ppu-documentation.md#scy-ff42
	if (pGBc_display->cgbSCYDelayTCycles > RESET)
	{
		if (--pGBc_display->cgbSCYDelayTCycles == RESET)
		{
			pGBc_peripherals->SCY = pGBc_display->cgbLatchedSCY;
		}
	}
#endif
}

void GBc_t::apuTick()
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.apuCounter++;

		continousDACCheck();	// If DAC is disabled, then Channel needs to be disabled immediately

		tickChannel(AUDIO_CHANNELS::CHANNEL_1, ONE);
		tickChannel(AUDIO_CHANNELS::CHANNEL_2, ONE);
		tickChannel(AUDIO_CHANNELS::CHANNEL_3, ONE);
		tickChannel(AUDIO_CHANNELS::CHANNEL_4, ONE);

		if (pGBc_instance->GBc_state.audio.wasDivAPUUpdated == YES)	// 512 Hz
		{
			if (pGBc_instance->GBc_state.audio.wasPowerCycled == YES)
			{
				pGBc_instance->GBc_state.audio.div_apu = RESET;
				pGBc_instance->GBc_state.audio.wasPowerCycled = NO;
			}

			pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter = FALSE;

			// % 2
			if ((pGBc_instance->GBc_state.audio.div_apu & ONE) == ZERO) // 256 Hz
			{
				pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter = TRUE;
				processSoundLength();
			}
			// % 4
			if ((pGBc_instance->GBc_state.audio.div_apu & THREE) == TWO) // 128 Hz
			{
				processFrequencySweep();
			}
			// % 8
			if ((pGBc_instance->GBc_state.audio.div_apu & SEVEN) == SEVEN) // 64 Hz
			{
				processEnvelopeSweep();
			}

			pGBc_instance->GBc_state.audio.wasDivAPUUpdated = NO;
		}

		captureDownsampledAudioSamples();
	}

void GBc_t::checkAllSTATInterrupts(FLAG isFF)
{
	if (isPPULCDEnabled())
	{
		LCD_MODES effLcdMode = getPPULCDMode();
		// Interrupts should not be affected by DMA STAT glitch as per docboy tests
		if (pGBc_instance->GBc_state.emulatorStatus.DMASTATGlitchEn == YES)
		{
			effLcdMode = pGBc_display->currentLCDMode;
		}

		if (pGBc_peripherals->LY == pGBc_peripherals->LYC)
		{
			pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_FLAG = ONE;
			if ((pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_STAT_INT_SRC == ONE) || (isFF))
			{
				if (pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.aggregateSignal == LO)
				{
					pGBc_instance->GBc_state.emulatorStatus.STAT_src = STAT_INTR_SRC::LY_LYC;
					requestInterrupts(INTERRUPTS::LCD_STAT_INTERRUPT);
				}

				pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.LY_LYC_SIGNAL = HI;
			}
		}
		else
		{
			pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_FLAG = ZERO;
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.LY_LYC_SIGNAL = LO;
		}
		if (
			((pGBc_peripherals->STAT.lcdStatusFields.MODE0_HBLANK_STAT_INT_SRC == ONE) || (isFF))
			&&
			(effLcdMode == LCD_MODES::MODE_LCD_H_BLANK))
		{
			if (pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.aggregateSignal == LO)
			{
				pGBc_instance->GBc_state.emulatorStatus.STAT_src = STAT_INTR_SRC::HBLANK;
				requestInterrupts(INTERRUPTS::LCD_STAT_INTERRUPT);
			}
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = HI;
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
		}
		else
		{
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
		}
		/*
		* NOTE: Based on wilbert-pol's and docboy's test rom, looks like STAT write will never trigger an OAM STAT interrupt
		* OAM STAT interrupt is triggered only with a mode transition
		* This behaviour is needed to pass the following tests
		* 1) Wilbert-pol's stat_write_if-GS.gb
		* 2) Wilbert-pol's stat_write_if-C.gb
		* 3) Docboy's hblank_stat_during_dma_transfer_oam_scan.gb
		* AND ALSO SURPRISINGLY
		* 4) Docboy's dma_during_oam_scan_check_stat_immediate.gb 
		* Further investigation by looking at sameboy's code implies same behaviour...
		*/
#if (DISABLED)
		if (
			((pGBc_peripherals->STAT.lcdStatusFields.MODE2_OAM_STAT_INT_SRC == ONE) || (isFF))
			&&
			(effLcdMode == LCD_MODES::MODE_LCD_SEARCHING_OAM))
		{
			if (pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.aggregateSignal == LO)
			{
				pGBc_instance->GBc_state.emulatorStatus.STAT_src = STAT_INTR_SRC::OAM;
				requestInterrupts(INTERRUPTS::LCD_STAT_INTERRUPT);
			}
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = HI;
		}
		else
		{
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
		}
#endif
		if (
			((pGBc_peripherals->STAT.lcdStatusFields.MODE1_VBLANK_STAT_INT_SRC == ONE) || (isFF))
			&&
			(effLcdMode == LCD_MODES::MODE_LCD_V_BLANK))
		{
			if (pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.aggregateSignal == LO)
			{
				pGBc_instance->GBc_state.emulatorStatus.STAT_src = STAT_INTR_SRC::VBLANK;
				requestInterrupts(INTERRUPTS::LCD_STAT_INTERRUPT);
			}
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = HI;
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
		}
		else
		{
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
		}
	}
	else
	{
		pGBc_instance->GBc_state.emulatorStatus.STAT_src = STAT_INTR_SRC::NONE;
		// Note that LY==LYC signal should not be made zero here as PPU disable means 0 == 0 condition is satisfied
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
	}
}

FLAG GBc_t::isInterruptReadyToBeServed()
{
	if ((pGBc_peripherals->IF.interruptRequestMemory
		& pGBc_memory->GBcMemoryMap.mInterruptEnable.interruptEnableMemory
		& 0x1F) != ZERO)
	{
		RETURN YES;
	}
	else
	{
		RETURN NO;
	}
}

void GBc_t::requestInterrupts(INTERRUPTS interrupt)
{
	switch (interrupt)
	{
	case INTERRUPTS::VBLANK_INTERRUPT:
		pGBc_peripherals->IF.interruptRequestFields.VBLANK = ONE;	//0xFF0F
		BREAK;
	case INTERRUPTS::LCD_STAT_INTERRUPT:
		pGBc_peripherals->IF.interruptRequestFields.LCD_STAT = ONE;	//0xFF0F
		BREAK;
	case INTERRUPTS::TIMER_INTERRUPT:
		pGBc_peripherals->IF.interruptRequestFields.TIMER = ONE;	//0xFF0F
		BREAK;
	case INTERRUPTS::SERIAL_INTERRUPT:
		pGBc_peripherals->IF.interruptRequestFields.SERIAL = ONE;	//0xFF0F
		BREAK;
	case INTERRUPTS::JOYPAD_INTERRUPT:
		pGBc_peripherals->IF.interruptRequestFields.JOYPAD = ONE;	//0xFF0F
		BREAK;
	default:
		FATAL("unknown interrupt");
	}
}

FLAG GBc_t::handleInterruptsIfApplicable(FLAG effectiveIME, FLAG effectiveInterruptQ)
{
	// Refer : https://gbdev.io/pandocs/Interrupts.html
	// And https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#isr-and-nmi
	// NOTE:
	// Gekkio recently published a test that shows that the GB CPU reads IE twice when firing an interrupt –
	// 1) once when it checks if an interrupt occurred, and
	// 2) once when it checks which interrupt occured.
	// These reads are not in the same M-Cycle.
	// Turns out it is also true for the IF register, and this is exactly what happens in Pinball Deluxe:
	// A VBlank interrupt occurs while the CPU handles a STAT interrupt
	// The VBlank interrupt effectively "hijacks" the routine and is handled before the STAT interrupt (because it has higher priority)

	// Also refer : https://www.reddit.com/r/EmuDev/comments/7206vh/sameboy_now_correctly_emulates_pinball_deluxe/
	// To simulate the T cycle accuracy needed for reads of IME, IF and IE and its effect on HALT mentioned in above link,
	// we have gbCpuTick2T
	// Source : Sameboy
	// Another source of documentation for this is from NesDev wiki mentioned below
	// Refer : https://forums.nesdev.org/viewtopic.php?p=240034#p240034
	// Refer : https://discord.com/channels/465585922579103744/465586075830845475/529008578065989642
	CPUTODO("Find more documentation for the need of gbCpuTick2T?");

	FLAG interruptWasServiced = NO;

	if (effectiveIME == ENABLED && effectiveInterruptQ == YES)
	{
		// Exit Halt
		pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = NO;

		uint16_t pcToHandler = INVALID_INTERRUPT_HANLDER_ADDRESS;

		pGBc_instance->GBc_state.emulatorStatus.eiEnState = EI_ENABLE_STATE::NOTHING_TO_BE_DONE;

		// NOTE: PC would already be incremented while executing the opcodes and hence no need to increment here again

		INCREMENT_PC_BY_ONE();
		cpuTickM(); // IDU PC- as per https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#isr-and-nmi
		TODO("Cycle OAM BUG Here!"); // Source: Sameboy; Refer : https://github.com/LIJI32/SameBoy/blob/master/Core/sm83_cpu.c#L1666
		DECREMENT_PC_BY_ONE();
		TODO("Trigger OAM BUG Here!"); // Source: Sameboy; Refer : https://github.com/LIJI32/SameBoy/blob/master/Core/sm83_cpu.c#L1668
		cpuTickM(); // IDU SP- as per https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#isr-and-nmi
		stackPush((BYTE)(GET_PC() >> EIGHT));
		cpuTickM(); // [SP-] = PC.high as per https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#isr-and-nmi

		// Note : This needs to be done to pass https://github.com/Gekkio/mooneye-test-suite/blob/main/acceptance/interrupts/ie_push.s
		interruptEnable_t old_ie = pGBc_memory->GBcMemoryMap.mInterruptEnable; // latch the old ie

		stackPush((BYTE)(GET_PC()));
		cpuTickM(); // [SP] = PC.low as per https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#isr-and-nmi

		// Determine which interrupt to service
		BYTE activeInterrupts = pGBc_peripherals->IF.interruptRequestMemory & old_ie.interruptEnableMemory & 0x1F;
		FLAG interruptFound = NO;
		ID8 interruptIndex = RESET;
		if (activeInterrupts)
		{
			static const uint16_t vectorTable[5] = { 0x0040, 0x0048, 0x0050, 0x0058, 0x0060 };

			// Find the lowest set bit — highest priority interrupt
			interruptIndex = (ID8)ctz32_portable(activeInterrupts);
			pcToHandler = vectorTable[interruptIndex];
			interruptFound = YES;
		}

		gbCpuTick2T(NO); // Part 1 of generic fetch as per https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#isr-and-nmi

		// Clear the interrupt request bit AFTER the final tick
		if (interruptFound == YES)
		{
			pGBc_peripherals->IF.interruptRequestMemory &= (ID8)~(1 << interruptIndex);
		}

		SET_PC(pcToHandler);

		pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn = DISABLED;
		interruptWasServiced = YES;

		gbCpuTick2T(YES); // Part 2 of generic fetch as per https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#isr-and-nmi
	}

	RETURN interruptWasServiced;
}

// Below are the emulated GB divider apis

BYTE GBc_t::getGBDividerMSB()
{
	RETURN pGBc_peripherals->DIV.divBytes.DIV_MSB.divByte;
}

BYTE GBc_t::getGBDividerLSB()
{
	RETURN pGBc_peripherals->DIV.divBytes.DIV_LSB.divByte;
}

void GBc_t::setGBDividerMSB(BYTE value)
{
	pGBc_peripherals->DIV.divBytes.DIV_MSB.divByte = value;
}

void GBc_t::setGBDividerLSB(BYTE value)
{
	pGBc_peripherals->DIV.divBytes.DIV_LSB.divByte = value;
}

BIT GBc_t::getDIVSpecialBitStatus(TIMERS timer)
{
	switch (timer)
	{
	case TIMERS::TIMER_00:
		RETURN pGBc_peripherals->DIV.divBytes.DIV_MSB.divFields.DIV_ONE;
	case TIMERS::TIMER_01:
		RETURN pGBc_peripherals->DIV.divBytes.DIV_LSB.divFields.DIV_THREE;
	case TIMERS::TIMER_10:
		RETURN pGBc_peripherals->DIV.divBytes.DIV_LSB.divFields.DIV_FIVE;
	case TIMERS::TIMER_11:
		RETURN pGBc_peripherals->DIV.divBytes.DIV_LSB.divFields.DIV_SEVEN;
	default:
		FATAL("Unknown Timer");
		RETURN ZERO;
	}
}

// Below are the emulated GB timer apis

SIGNAL GBc_t::getTIMASignalForGB()
{
	// Figure out if timer getting disabled is the reason for falling edge
	// If yes, we should not delay the TIMER IF by 1-M cycle
	// Refer : https://www.reddit.com/r/EmuDev/comments/acsu62/comment/ejq7i7p/?utm_source=share&utm_medium=web2x&context=3
	if (pGBc_peripherals->TAC.timerControlFields.TIMER_ENABLE == ZERO
		&& getDIVSpecialBitStatus(getWhichGBTimerToUse()) == ONE)
	{
		pGBc_instance->GBc_state.emulatorStatus.instantTimerIF = YES;
	}
	else
	{
		pGBc_instance->GBc_state.emulatorStatus.instantTimerIF = NO;
	}

	RETURN (((FLAG)pGBc_peripherals->TAC.timerControlFields.TIMER_ENABLE) && ((FLAG)getDIVSpecialBitStatus(getWhichGBTimerToUse())));
}

BYTE GBc_t::getGBTimer()
{
	RETURN pGBc_peripherals->TIMA;
}

void GBc_t::setGBTimer(BYTE value)
{
	pGBc_peripherals->TIMA = value;
}

void GBc_t::resetGBTimerToZero()
{
	pGBc_peripherals->TIMA = ZERO;
}

void GBc_t::resetGBTimer(uint8_t resetVal)
{
	pGBc_peripherals->TIMA = resetVal;
}

GBc_t::TIMERS GBc_t::getWhichGBTimerToUse()
{
	switch (pGBc_peripherals->TAC.timerControlFields.CLOCK_SELECT)
	{
	case 0:
		RETURN TIMERS::TIMER_00;
	case 1:
		RETURN TIMERS::TIMER_01;
	case 2:
		RETURN TIMERS::TIMER_10;
	case 3:
		RETURN TIMERS::TIMER_11;
	default:
		FATAL("Invalid Timer %d", (BYTE)(pGBc_peripherals->TAC.timerControlFields.CLOCK_SELECT));
		RETURN TIMERS::TIMER_INVALID;
	}
}

void GBc_t::processDMA()
{
	if (pGBc_instance->GBc_state.emulatorStatus.DMARestarted == YES)
	{
		if (pGBc_instance->GBc_state.emulatorStatus.DMAEndDelayUponRestart != ZERO)
		{
			pGBc_instance->GBc_state.emulatorStatus.DMAEndDelayUponRestart--;
		}
		else
		{
			pGBc_instance->GBc_state.emulatorStatus.DMARestarted = CLEAR;
			pGBc_instance->GBc_state.emulatorStatus.DMASource = (((uint16_t)pGBc_peripherals->DMA) << EIGHT);
			// Refer : https://discord.com/channels/465585922579103744/465586075830845475/1331744415961845890
			if (ROM_TYPE == ROM::GAME_BOY && (pGBc_instance->GBc_state.emulatorStatus.DMASource & 0xFF00) >= 0xFE00)
			{
				UNSETBIT(pGBc_instance->GBc_state.emulatorStatus.DMASource, THIRTEEN);
			}
			pGBc_instance->GBc_state.emulatorStatus.isDMAActive = YES;
			pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred = RESET;
			pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay = RESET;
		}
	}

	if (pGBc_instance->GBc_state.emulatorStatus.isDMAActive == NO)
	{
		RETURN;
	}

	if (pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay != ZERO)
	{
		pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay--;
		RETURN;
	}

	auto currentDMAWord = readRawMemory(pGBc_instance->GBc_state.emulatorStatus.DMASource + pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred, MEMORY_ACCESS_SOURCE::OAMDMA);
	writeRawMemory(OAM_START_ADDRESS + pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred, currentDMAWord, MEMORY_ACCESS_SOURCE::OAMDMA);

	pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred++;

	if (pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred < sizeof(pGBc_memory->GBcMemoryMap.mOAM.OAMMemory))
	{
		pGBc_instance->GBc_state.emulatorStatus.isDMAActive = YES;
	}
	else
	{
		pGBc_instance->GBc_state.emulatorStatus.isDMAActive = NO;
		pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred = RESET;
		pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay = DMA_DELAY;

		// Undo the DMA STAT glitch if required as DMA is not active anymore!
		// This is needed by "dma_during_hblank_finish_at_oam_scan_check_stat_round2.gb"
		OAMDMASTATModeGlitch();
	}
}

void GBc_t::processGPDMA()
{
	if (pGBc_instance->GBc_state.emulatorStatus.isGPDMAActive == NO)
	{
		RETURN;
	}

	// Refer : https://gbdev.io/pandocs/CGB_Registers.html#transfer-timings
	// Basically, for given ticks, in double speed mode, we transfer the half of the number of bytes that we would have transfered in normal speed mode

	for (BYTE ii = ZERO; ii < (isCGBDoubleSpeedEnabled() == YES ? ONE : TWO); ii++)
	{
		writeRawMemory(pGBc_instance->GBc_state.emulatorStatus.hDMADestination,
			readRawMemory(pGBc_instance->GBc_state.emulatorStatus.hDMASource, MEMORY_ACCESS_SOURCE::GPDMA),
			MEMORY_ACCESS_SOURCE::GPDMA);

		pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred++;
		pGBc_instance->GBc_state.emulatorStatus.hDMADestination++;
		pGBc_instance->GBc_state.emulatorStatus.hDMASource++;
		pGBc_instance->GBc_state.emulatorStatus.hDMATXLength--;
	}

	if (pGBc_instance->GBc_state.emulatorStatus.hDMATXLength > ZERO) // still some bytes are pending to be transferred
	{
		pGBc_instance->GBc_state.emulatorStatus.isGPDMAActive = YES;
	}
	// all the bytes have been transferred
	else
	{
		pGBc_instance->GBc_state.emulatorStatus.isGPDMAActive = NO;
		pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred = RESET;
		pGBc_peripherals->HDMA1 = 0xFF;
		pGBc_peripherals->HDMA2 = 0xFF;
		pGBc_peripherals->HDMA3 = 0xFF;
		pGBc_peripherals->HDMA4 = 0xFF;
		// Refer to 9.6 of https://raw.githubusercontent.com/AntonioND/giibiiadvance/master/docs/TCAGBD.pdf
		// Refer https://gbdev.io/pandocs/CGB_Registers.html?highlight=HDMA#bit-7--0--general-purpose-dma
		pGBc_peripherals->HDMA5 = 0xFF; // GPDMA transfer has completed
		pGBc_instance->GBc_state.emulatorStatus.isCPUExecutionBlocked = NO;
	}
}

void GBc_t::processHDMA()
{
	if (pGBc_instance->GBc_state.emulatorStatus.isHDMAActive == NO
		|| pGBc_instance->GBc_state.emulatorStatus.isHDMAAllowedToBlockCPUPipeline == NO
		|| pGBc_instance->GBc_state.emulatorStatus.isCPUHalted == YES)
	{
		RETURN;
	}

	pGBc_instance->GBc_state.emulatorStatus.isCPUExecutionBlocked = YES;

	// Refer : https://gbdev.io/pandocs/CGB_Registers.html#transfer-timings
	// Basically, for given ticks, in double speed mode, we transfer the half of the number of bytes that we would have transfered in normal speed mode

	for (BYTE ii = ZERO; ii < (isCGBDoubleSpeedEnabled() == YES ? ONE : TWO); ii++)
	{
		writeRawMemory(pGBc_instance->GBc_state.emulatorStatus.hDMADestination,
			readRawMemory(pGBc_instance->GBc_state.emulatorStatus.hDMASource, MEMORY_ACCESS_SOURCE::HDMA),
			MEMORY_ACCESS_SOURCE::HDMA);

		pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred++;
		pGBc_instance->GBc_state.emulatorStatus.hDMADestination++;
		pGBc_instance->GBc_state.emulatorStatus.hDMASource++;
		pGBc_instance->GBc_state.emulatorStatus.hDMATXLength--;

		if (pGBc_instance->GBc_state.emulatorStatus.hDMADestination == 0xA000) // Limit HDMA Destination Address
		{
			pGBc_instance->GBc_state.emulatorStatus.hDMADestination = 0x8000;
		}

		if (pGBc_instance->GBc_state.emulatorStatus.hDMASource == 0x8000) // Limit HDMA Source Address
		{
			pGBc_instance->GBc_state.emulatorStatus.hDMASource = 0xA000;
		}
	}

	if (pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred < 0x10)
	{
		pGBc_instance->GBc_state.emulatorStatus.isCPUExecutionBlocked = YES;
	}

	if (pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred == 0x10)
	{
		pGBc_instance->GBc_state.emulatorStatus.isCPUExecutionBlocked = NO;
		pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred = RESET;

		pGBc_instance->GBc_state.emulatorStatus.isHDMAAllowedToBlockCPUPipeline = NO;

		// update the remaining data to be transfered in HDMA registers

		// assume a case where out of 0x800 data (0x7F in HDMA5), 0x10 was transfered
		// remaming would be 0x7F0, which translates to 0x7E (i.e. 0x7F - 1); hence a decrement by 1 is enough in HDMA5 for every 0x10 byte transfer
		// During last 0x10 bytes transfer, in HDMA5 (0x00 - 1 = 0xFF) which indicates transfer is complete, so taking advantage of underflow
		// Refer to 9.6.2 of https://raw.githubusercontent.com/AntonioND/giibiiadvance/master/docs/TCAGBD.pdf
		// Refer https://gbdev.io/pandocs/CGB_Registers.html?highlight=HDMA#bit-7--1--hblank-dma
		pGBc_peripherals->HDMA5--;
	}

	if (pGBc_instance->GBc_state.emulatorStatus.hDMATXLength > ZERO) // still some bytes are pending to be transferred
	{
		pGBc_instance->GBc_state.emulatorStatus.isHDMAActive = YES;
	}
	// all the bytes have been transferred
	else
	{
		// Refer to 9.6.2 of https://raw.githubusercontent.com/AntonioND/giibiiadvance/master/docs/TCAGBD.pdf
		// Refer https://gbdev.io/pandocs/CGB_Registers.html?highlight=HDMA#bit-7--1--hblank-dma
		if (pGBc_peripherals->HDMA5 != 0xFF)
		{
			FATAL("HDMA Transfer Issue!");
		}

		pGBc_instance->GBc_state.emulatorStatus.isHDMAActive = NO;
		pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred = RESET;
		pGBc_peripherals->HDMA1 = pGBc_instance->GBc_state.emulatorStatus.hDMASource & 0xFF;
		pGBc_peripherals->HDMA2 = pGBc_instance->GBc_state.emulatorStatus.hDMASource >> EIGHT;
		pGBc_peripherals->HDMA3 = pGBc_instance->GBc_state.emulatorStatus.hDMADestination & 0xFF;
		pGBc_peripherals->HDMA4 = pGBc_instance->GBc_state.emulatorStatus.hDMADestination >> EIGHT;
		// Refer to 9.6.2 of https://raw.githubusercontent.com/AntonioND/giibiiadvance/master/docs/TCAGBD.pdf
		// Refer https://gbdev.io/pandocs/CGB_Registers.html?highlight=HDMA#bit-7--1--hblank-dma
		pGBc_peripherals->HDMA5 = 0xFF; // HDMA transfer has completed
		pGBc_instance->GBc_state.emulatorStatus.isCPUExecutionBlocked = NO;
	}
}

void GBc_t::captureIO()
{
	pGBc_emuStatus->keyUP = ImGui::IsKeyDown(ImGuiKey_UpArrow);
	pGBc_emuStatus->keyDOWN = ImGui::IsKeyDown(ImGuiKey_DownArrow);
	pGBc_emuStatus->keyLEFT = ImGui::IsKeyDown(ImGuiKey_LeftArrow);
	pGBc_emuStatus->keyRIGHT = ImGui::IsKeyDown(ImGuiKey_RightArrow);

	pGBc_emuStatus->keySTART = ImGui::IsKeyDown(ImGuiKey_Enter);
	pGBc_emuStatus->keySELECT = ImGui::IsKeyDown(ImGuiKey_Space);
	pGBc_emuStatus->keyA = ImGui::IsKeyDown(ImGuiKey_Z);
	pGBc_emuStatus->keyB = ImGui::IsKeyDown(ImGuiKey_X);

	BYTE previousJoyPadState = pGBc_peripherals->P1_JOYP.joyPadMemory;

	if (pGBc_peripherals->P1_JOYP.joyPadFields.P15_SEL_ACTION_KEYS == ZERO
		&& pGBc_peripherals->P1_JOYP.joyPadFields.P14_SEL_DIRECTION_KEYS == ONE)
	{
		pGBc_peripherals->P1_JOYP.joyPadFields.P10_RIGHT_A = ((pGBc_emuStatus->keyA == YES ?
			JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED));

		pGBc_peripherals->P1_JOYP.joyPadFields.P11_LEFT_B = ((pGBc_emuStatus->keyB == YES ?
			JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED));

		pGBc_peripherals->P1_JOYP.joyPadFields.P12_UP_SELECT = ((pGBc_emuStatus->keySELECT == YES ?
			JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED));

		pGBc_peripherals->P1_JOYP.joyPadFields.P13_DOWN_START = ((pGBc_emuStatus->keySTART == YES ?
			JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED));

	}
	else if (pGBc_peripherals->P1_JOYP.joyPadFields.P14_SEL_DIRECTION_KEYS == ZERO
		&& pGBc_peripherals->P1_JOYP.joyPadFields.P15_SEL_ACTION_KEYS == ONE)
	{
		pGBc_peripherals->P1_JOYP.joyPadFields.P10_RIGHT_A = ((pGBc_emuStatus->keyRIGHT == YES ?
			JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED));

		pGBc_peripherals->P1_JOYP.joyPadFields.P11_LEFT_B = ((pGBc_emuStatus->keyLEFT == YES ?
			JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED));

		pGBc_peripherals->P1_JOYP.joyPadFields.P12_UP_SELECT = ((pGBc_emuStatus->keyUP == YES ?
			JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED));

		pGBc_peripherals->P1_JOYP.joyPadFields.P13_DOWN_START = ((pGBc_emuStatus->keyDOWN == YES ?
			JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED));
	}
	else if (pGBc_peripherals->P1_JOYP.joyPadFields.P14_SEL_DIRECTION_KEYS == ZERO
		&& pGBc_peripherals->P1_JOYP.joyPadFields.P15_SEL_ACTION_KEYS == ZERO)
	{
		// Source: Sameboy; Refer https://github.com/LIJI32/SameBoy/blob/master/Core/joypad.c#L126
		// OR of both groups: pressed = dir OR action, then invert (active low)
		TODO("Find source of JOYP behaviour when both bits 4 and 5 are set");
		pGBc_peripherals->P1_JOYP.joyPadFields.P12_UP_SELECT =
			((pGBc_emuStatus->keyUP == YES ||
				pGBc_emuStatus->keySELECT == YES) ?
				JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED);

		pGBc_peripherals->P1_JOYP.joyPadFields.P13_DOWN_START =
			((pGBc_emuStatus->keyDOWN == YES ||
				pGBc_emuStatus->keySTART == YES) ?
				JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED);

		pGBc_peripherals->P1_JOYP.joyPadFields.P11_LEFT_B =
			((pGBc_emuStatus->keyLEFT == YES ||
				pGBc_emuStatus->keyB == YES) ?
				JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED);

		pGBc_peripherals->P1_JOYP.joyPadFields.P10_RIGHT_A =
			((pGBc_emuStatus->keyRIGHT == YES ||
				pGBc_emuStatus->keyA == YES) ?
				JOYPAD_STATES::PRESSED : JOYPAD_STATES::NOT_PRESSED);
	}
	else
	{
		// Keep all 4 keys to deactivated state
		pGBc_peripherals->P1_JOYP.joyPadMemory |= 0x0F;
	}

	// Check if we need to request for interrupt
	if (((previousJoyPadState & (~(pGBc_peripherals->P1_JOYP.joyPadMemory & 0x0F))) & 0x0F) != ZERO)
	{
		requestInterrupts(INTERRUPTS::JOYPAD_INTERRUPT);
	}
}

void GBc_t::processSerialClockSpeedBit()
{
	pGBc_instance->GBc_state.emulatorStatus.serialMasterByteShiftCount = RESET;

	if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_peripherals->SC.scFields.CLOCK_SPEED == ONE)
	{
		pGBc_instance->GBc_state.emulatorStatus.serialMaxClockPerTransfer = TO_UINT16(GB_GBC_REFERENCE_CLOCK_HZ / (262144.0f));
	}
	else
	{
		pGBc_instance->GBc_state.emulatorStatus.serialMaxClockPerTransfer = TO_UINT16(GB_GBC_REFERENCE_CLOCK_HZ / (8192.0f));
	}
}

FLAG GBc_t::sendOverSerialLink(BIT bitToSend)
{
	FLAG status = FAILURE;

	if (_ENABLE_NETWORK == YES)
	{
#if DISABLED
		olc::net::message<serialMsg> msg;
		gameSerialData toBeSent;
		toBeSent.ID = nEmulationInstanceID;
		toBeSent.data = bitToSend;
		msg << toBeSent;
		msg.header.id = serialMsg::Game_SendBit;
		GBcNetworkEngine->Send(msg);
		LOG("[Client] Bit sent from Client ID %u is %u", nEmulationInstanceID, bitToSend);
#endif
		status = SUCCESS;
	}
	else
	{
		status = SUCCESS;
	}

	RETURN status;
}

FLAG GBc_t::receiveOverSerialLink(BIT* bitReceived, FLAG* rxStatus, FLAG isBlocking, INC32 timeoutInUs)
{
	*bitReceived = ONE;
	*rxStatus = NO;

	if (_ENABLE_NETWORK == NO)
	{
		RETURN SUCCESS;
	}

#if DISABLED
	// Check once without busy-waiting
	if (!GBcNetworkEngine->Incoming().empty())
	{
		auto msg = GBcNetworkEngine->Incoming().pop_front().msg;
		if (msg.header.id == serialMsg::Game_ReceiveBit)
		{
			msg >> *bitReceived;
			*rxStatus = YES;
		}
		RETURN SUCCESS;
	}

	// Non-blocking case: just return
	if (isBlocking == NO)
	{
		RETURN SUCCESS;
	}

	// Blocking case: use a condition variable or sleep briefly
	// instead of busy-waiting
	std::this_thread::sleep_for(std::chrono::microseconds(timeoutInUs));
	RETURN SUCCESS;
#else
	// Network code is disabled, just return immediately
	RETURN SUCCESS;
#endif
}

void GBc_t::resetDivAPU(uint32_t value)
{
	pGBc_instance->GBc_state.audio.div_apu = value;
	pGBc_instance->GBc_state.audio.wasDivAPUUpdated = YES;
}

void GBc_t::incrementDivAPU(uint32_t nCycles)
{
	pGBc_instance->GBc_state.audio.div_apu += nCycles;
	pGBc_instance->GBc_state.audio.div_apu &= SEVEN; // % 8
	pGBc_instance->GBc_state.audio.wasDivAPUUpdated = YES;
}

DIM16 GBc_t::getChannelPeriod(AUDIO_CHANNELS channel)
{
	uint16_t period = ZERO;
	switch (channel)
	{
	case AUDIO_CHANNELS::CHANNEL_1:
		period = pGBc_peripherals->NR13.lowerPeriodValue;
		period |= pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.higherPeriodValue << EIGHT;
		RETURN period;
	case AUDIO_CHANNELS::CHANNEL_2:
		period = pGBc_peripherals->NR23.lowerPeriodValue;
		period |= pGBc_peripherals->NR24.channelHigherPeriodAndControlFields.higherPeriodValue << EIGHT;
		RETURN period;
	case AUDIO_CHANNELS::CHANNEL_3:
		period = pGBc_peripherals->NR33.lowerPeriodValue;
		period |= pGBc_peripherals->NR34.channelHigherPeriodAndControlFields.higherPeriodValue << EIGHT;
		RETURN period;
	default:
		RETURN period;
	}
}

FLAG GBc_t::enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS channel)
{
	FLAG wasEnableSuccess = false;

	if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ONE)
	{
		switch (channel)
		{
		case AUDIO_CHANNELS::CHANNEL_1:
			if (isDACEnabled(channel) == YES) // DAC check
			{
				// Since we triggered the channel, "length timer expiring" or "frequency sweep overflow" checks are reset
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = ENABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel1ONFlag = ONE;
				wasEnableSuccess = YES;
			}
			else
			{
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel1ONFlag = ZERO;
				pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.trigger = ZERO;
				wasEnableSuccess = NO;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_2:
			if (isDACEnabled(channel) == YES) // DAC check
			{
				// Since we triggered the channel, "length timer expiring" or "frequency sweep overflow" checks are reset
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = ENABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel2ONFlag = ONE;
				wasEnableSuccess = YES;
			}
			else
			{
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel2ONFlag = ZERO;
				pGBc_peripherals->NR24.channelHigherPeriodAndControlFields.trigger = ZERO;
				wasEnableSuccess = NO;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_3:
			if (isDACEnabled(channel) == YES) // DAC check
			{
				// Since we triggered the channel, "length timer expiring" or "frequency sweep overflow" checks are reset
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = ENABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel3ONFlag = ONE;
				wasEnableSuccess = YES;
			}
			else
			{
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel3ONFlag = ZERO;
				pGBc_peripherals->NR34.channelHigherPeriodAndControlFields.trigger = ZERO;
				wasEnableSuccess = NO;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_4:
			if (isDACEnabled(channel) == YES) // DAC check
			{
				// Since we triggered the channel, "length timer expiring" or "frequency sweep overflow" checks are reset
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = ENABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel4ONFlag = ONE;
				wasEnableSuccess = YES;
			}
			else
			{
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel4ONFlag = ZERO;
				pGBc_peripherals->NR44.channelHigherPeriodAndControlFields.trigger = ZERO;
				wasEnableSuccess = NO;
			}
			BREAK;
		default :
			FATAL ("Unknown Audio Channel : %d", TO_UINT8(channel));
			BREAK;
		}
	}

	RETURN wasEnableSuccess;
}

void GBc_t::continousDACCheck()
{
	if ((pGBc_peripherals->NR12.channelVolumeAndEnvelopeMemory & 0xF8) == ZERO)
	{
		pGBc_peripherals->NR52.channelSoundONOFFFields.channel1ONFlag = ZERO;
		pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
		pGBc_instance->GBc_state.audio.dacEnMap &= ~0x01;
	}
	else
	{
		pGBc_instance->GBc_state.audio.dacEnMap |= 0x01;
	}

	if ((pGBc_peripherals->NR22.channelVolumeAndEnvelopeMemory & 0xF8) == ZERO)
	{
		pGBc_peripherals->NR52.channelSoundONOFFFields.channel2ONFlag = ZERO;
		pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
		pGBc_instance->GBc_state.audio.dacEnMap &= ~0x02;
	}
	else
	{
		pGBc_instance->GBc_state.audio.dacEnMap |= 0x02;
	}

	if (pGBc_peripherals->NR30.channelDACEnableFields.dacEnable == ZERO)
	{
		pGBc_peripherals->NR52.channelSoundONOFFFields.channel3ONFlag = ZERO;
		pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
		pGBc_instance->GBc_state.audio.dacEnMap &= ~0x04;
	}
	else
	{
		pGBc_instance->GBc_state.audio.dacEnMap |= 0x04;
	}

	if ((pGBc_peripherals->NR42.channelVolumeAndEnvelopeMemory & 0xF8) == ZERO)
	{
		pGBc_peripherals->NR52.channelSoundONOFFFields.channel4ONFlag = ZERO;
		pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
		pGBc_instance->GBc_state.audio.dacEnMap &= ~0x08;
	}
	else
	{
		pGBc_instance->GBc_state.audio.dacEnMap |= 0x08;
	}
}

FLAG GBc_t::isDACEnabled(AUDIO_CHANNELS channel)
{
	FLAG status = NO;

	if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ONE)
	{
		switch (channel)
		{
		case AUDIO_CHANNELS::CHANNEL_1:
			if ((pGBc_peripherals->NR12.channelVolumeAndEnvelopeMemory & 0xF8) != ZERO)
			{
				status = YES;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_2:
			if ((pGBc_peripherals->NR22.channelVolumeAndEnvelopeMemory & 0xF8) != ZERO)
			{
				status = YES;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_3:
			if (pGBc_peripherals->NR30.channelDACEnableFields.dacEnable == ONE)
			{
				status = YES;
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_4:
			if ((pGBc_peripherals->NR42.channelVolumeAndEnvelopeMemory & 0xF8) != ZERO)
			{
				status = YES;
			}
			BREAK;
		default:
			RETURN status;
		}
	}

	RETURN status;
}

FLAG GBc_t::isChannel3Active()
{
	RETURN (pGBc_peripherals->NR52.channelSoundONOFFFields.channel3ONFlag == ONE
		&& pGBc_peripherals->NR34.channelHigherPeriodAndControlFields.trigger == ONE
		&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled == ENABLED);
}

void GBc_t::tickChannel(AUDIO_CHANNELS channel, uint32_t tCycles)
{
	if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ONE)
	{
		switch (channel)
		{
		case AUDIO_CHANNELS::CHANNEL_1:
			pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].frequencyTimer -= tCycles;
			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].frequencyTimer <= ZERO)
			{
				uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_1)) * FOUR;

				// Refer https://gbdev.io/pandocs/Audio_Registers.html#ff13--nr13-channel-1-period-low-write-only
				// Xly by 4 in GB/GBC was because of 4194304 / 1048576

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].frequencyTimer += resetFrequencyTimer;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].waveDutyPosition++;
				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].waveDutyPosition >= EIGHT)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].waveDutyPosition = ZERO;
				}

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled == ENABLED)
				{
					pGBc_instance->GBc_state.audio.sampleReadByChannel1 = SQUARE_WAVE_AMPLITUDE[pGBc_peripherals->NR11.channelLengthAndDutyFields.waveDuty]
						[pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].waveDutyPosition];
				}
				else
				{
					pGBc_instance->GBc_state.audio.sampleReadByChannel1 = TO_UINT8(MUTE_AUDIO);
				}
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_2:
			pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].frequencyTimer -= tCycles;
			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].frequencyTimer <= ZERO)
			{
				uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_2)) * FOUR;

				// Refer https://gbdev.io/pandocs/Audio_Registers.html#ff13--nr13-channel-1-period-low-write-only
				// Xly by 4 in GB/GBC was because of 4194304 / 1048576

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].frequencyTimer += resetFrequencyTimer;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].waveDutyPosition++;
				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].waveDutyPosition >= EIGHT)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].waveDutyPosition = ZERO;
				}

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled == ENABLED)
				{
					pGBc_instance->GBc_state.audio.sampleReadByChannel2 = SQUARE_WAVE_AMPLITUDE[pGBc_peripherals->NR21.channelLengthAndDutyFields.waveDuty]
						[pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].waveDutyPosition];
				}
				else
				{
					pGBc_instance->GBc_state.audio.sampleReadByChannel2 = TO_UINT8(MUTE_AUDIO);
				}
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_3:
			pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].frequencyTimer -= tCycles;
			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].frequencyTimer <= ZERO)
			{
				pGBc_instance->GBc_state.audio.didChannel3ReadWaveRamPostTrigger = YES;

				uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_3)) * TWO;

				// Refer https://gbdev.io/pandocs/Audio_Registers.html#ff13--nr13-channel-1-period-low-write-only
				// Xly by 4 in GB/GBC was because of 4194304 / 2097152

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].frequencyTimer += resetFrequencyTimer;

				// Refer : https://gbdev.io/pandocs/Audio_Registers.html#ff1e--nr34-channel-3-period-high--control
				// NOTE : Below code can execute even when "isChannel3Active() == NO" (Based on experimenting with Prehistorik Man)
				pGBc_instance->GBc_state.audio.waveRamCurrentIndex++;
				if (pGBc_instance->GBc_state.audio.waveRamCurrentIndex >= THIRTYTWO)
				{
					pGBc_instance->GBc_state.audio.waveRamCurrentIndex = ZERO;
				}

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled == ENABLED)
				{
					if (GETBIT(ZERO, pGBc_instance->GBc_state.audio.waveRamCurrentIndex) == ZERO)
					{
						pGBc_instance->GBc_state.audio.sampleReadByChannel3
							= pGBc_peripherals->waveRam[(uint8_t)(pGBc_instance->GBc_state.audio.waveRamCurrentIndex / TWO)].samples.upperNibble;
					}
					else
					{
						pGBc_instance->GBc_state.audio.sampleReadByChannel3
							= pGBc_peripherals->waveRam[(uint8_t)(pGBc_instance->GBc_state.audio.waveRamCurrentIndex / TWO)].samples.lowerNibble;
					}
				}
				else
				{
					pGBc_instance->GBc_state.audio.sampleReadByChannel3 = TO_UINT8(MUTE_AUDIO);
				}
			}
			BREAK;
		case AUDIO_CHANNELS::CHANNEL_4:
			pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].frequencyTimer -= tCycles;
			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].frequencyTimer <= ZERO)
			{
				byte divisor = AUDIO_CHANNEL_4_DIVISOR[pGBc_peripherals->NR43.channelFrequencyAndRandomnessFields.clockDivider];
				byte shiftAmount = pGBc_peripherals->NR43.channelFrequencyAndRandomnessFields.clockShift;
				uint16_t resetFrequencyTimer = divisor << shiftAmount;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].frequencyTimer += resetFrequencyTimer;

				uint16_t LFSR = pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].LFSR;
				BYTE LFSRWidth = pGBc_peripherals->NR43.channelFrequencyAndRandomnessFields.LFSRwidth;

				BYTE xorResult = (BYTE)((GETBIT(ZERO, LFSR)) ^ (GETBIT(ONE, LFSR)));
				LFSR = (uint16_t)((LFSR >> ONE) | (xorResult << FOURTEEN));

				if (LFSRWidth == LFSR_WIDTH_IS_7_BITS)
				{
					LFSR &= ~(ONE << SIX);
					LFSR |= (xorResult << SIX);
				}

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].LFSR = LFSR;

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled == ENABLED)
				{
					uint16_t LFSR = pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].LFSR;
					pGBc_instance->GBc_state.audio.sampleReadByChannel4 = (BYTE)((GETBIT(ZERO, (~LFSR))));
				}
				else
				{
					pGBc_instance->GBc_state.audio.sampleReadByChannel4 = TO_UINT8(MUTE_AUDIO);
				}
			}
			BREAK;
		default:
			RETURN;
		}
	}
}

void GBc_t::processSoundLength()
{
	if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ONE)
	{
		if (pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
			&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer > ZERO)
		{
			--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer;

			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer == ZERO)
			{
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel1ONFlag = ZERO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
			}
		}

		if (pGBc_peripherals->NR24.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
			&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer > ZERO)
		{
			--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer;

			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer == ZERO)
			{
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel2ONFlag = ZERO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
			}
		}

		if (pGBc_peripherals->NR34.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
			&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer > ZERO)
		{
			--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer;

			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer == ZERO)
			{
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel3ONFlag = ZERO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
			}
		}

		if (pGBc_peripherals->NR44.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
			&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer > ZERO)
		{
			--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer;

			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer == ZERO)
			{
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel4ONFlag = ZERO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
			}
		}
	}
}

SDIM32 GBc_t::getUpdatedFrequency()
{
	int32_t newFrequency = ZERO;

	if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ONE)
	{
		newFrequency = pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].shadowFrequency
			>> pGBc_peripherals->NR10.channelSweepFields.sweepSlopeControl;

		if (pGBc_peripherals->NR10.channelSweepFields.sweepDirection == ZERO)
		{
			newFrequency = pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].shadowFrequency + newFrequency;
		}
		else
		{
			pGBc_instance->GBc_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger = YES;
			newFrequency = pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].shadowFrequency - newFrequency;
		}

		if (newFrequency > 2047)
		{
			pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.trigger = ZERO;
			pGBc_peripherals->NR52.channelSoundONOFFFields.channel1ONFlag = ZERO;
			pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
		}
	}

	RETURN newFrequency;
}

void GBc_t::processFrequencySweep()
{
	if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ONE)
	{
		if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepTimer > ZERO)
		{
			--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepTimer;
		}

		if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepTimer == ZERO)
		{
			// reload the sweep timer
			if (pGBc_peripherals->NR10.channelSweepFields.sweepPace > ZERO)
			{
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepTimer
					= pGBc_peripherals->NR10.channelSweepFields.sweepPace;
			}
			else
			{
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepTimer
					= EIGHT;
			}

			// since the sweep timer has expired (and reloaded), update the frequency sweep
			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepEnabled == ENABLED
				&& pGBc_peripherals->NR10.channelSweepFields.sweepPace > ZERO)
			{
				int32_t newFrequency = getUpdatedFrequency();

				if (newFrequency <= 2047 && pGBc_peripherals->NR10.channelSweepFields.sweepSlopeControl > ZERO)
				{
					pGBc_peripherals->NR13.lowerPeriodValue = newFrequency & 0xFF;
					pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.higherPeriodValue = ((newFrequency >> EIGHT) & 0x07);

					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].shadowFrequency = newFrequency;

					performOverFlowCheck();
				}
			}
		}
	}
}

void GBc_t::processEnvelopeSweep()
{
	if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ONE)
	{
		if (pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.envelopeSweepPace != ZERO)
		{
			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer > ZERO)
			{
				--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer;
			}

			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer == ZERO)
			{
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer
					= pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.envelopeSweepPace;

				// volume is greater than 0 and we are in decrementing mode
				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume > 0x00
					&& pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.envelopeDirection == ZERO)
				{
					--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume;
				}
				// volume is less than 15 and we are in incrementing mode
				else if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume < 0x0F
					&& pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.envelopeDirection == ONE)
				{
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume;
				}

				// volume is either 0 or 15
				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume == 0x0F
					|| pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume == 0x00)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isVolumeEnvelopeStillDoingAutomaticUpdates = NO;
				}
			}
		}

		if (pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.envelopeSweepPace != ZERO)
		{
			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer > ZERO)
			{
				--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer;
			}

			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer == ZERO)
			{
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer
					= pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.envelopeSweepPace;

				// volume is greater than 0 and we are in decrementing mode
				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume > 0x00
					&& pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.envelopeDirection == ZERO)
				{
					--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume;
				}
				// volume is less than 15 and we are in incrementing mode
				else if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume < 0x0F
					&& pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.envelopeDirection == ONE)
				{
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume;
				}

				// volume is either 0 or 15
				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume == 0x0F
					|| pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume == 0x00)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isVolumeEnvelopeStillDoingAutomaticUpdates = NO;
				}
			}
		}

		if (pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.envelopeSweepPace != ZERO)
		{
			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer > ZERO)
			{
				--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer;
			}

			if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer == ZERO)
			{
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer
					= pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.envelopeSweepPace;

				// volume is greater than 0 and we are in decrementing mode
				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume > 0x00
					&& pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.envelopeDirection == ZERO)
				{
					--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume;
				}
				// volume is less than 15 and we are in incrementing mode
				else if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume < 0x0F
					&& pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.envelopeDirection == ONE)
				{
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume;
				}

				// volume is either 0 or 15
				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume == 0x0F
					|| pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume == 0x00)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isVolumeEnvelopeStillDoingAutomaticUpdates = NO;
				}
			}
		}
	}
}

BYTE GBc_t::getLogicalAmplitude(AUDIO_CHANNELS channel)
{
	uint8_t amplitude = LO;

	switch (channel)
	{
	case AUDIO_CHANNELS::CHANNEL_1:
		amplitude = pGBc_instance->GBc_state.audio.sampleReadByChannel1;
		BREAK;
	case AUDIO_CHANNELS::CHANNEL_2:
		amplitude = pGBc_instance->GBc_state.audio.sampleReadByChannel2;
		BREAK;
	case AUDIO_CHANNELS::CHANNEL_3:
		amplitude = pGBc_instance->GBc_state.audio.sampleReadByChannel3;
		BREAK;
	case AUDIO_CHANNELS::CHANNEL_4:
		amplitude = pGBc_instance->GBc_state.audio.sampleReadByChannel4;
		BREAK;
	default:
		FATAL("Unknown Audio Channel : %d", TO_UINT8(channel));
		BREAK;
	}

	RETURN amplitude;
}

float GBc_t::getDACOutput(AUDIO_CHANNELS channel)
{
	// Making dacOutput static as when DAC is disabled, returning last analog value before it was disabled is a good approx
	// Refer https://x.com/LIJI32/status/964555034011815936/photo/1
	static float dacOutput[AUDIO_CHANNELS::TOTAL_CHANNELS] = { static_cast<float>(LO) };

	if (isDACEnabled(channel) == YES)
	{
		if (channel == AUDIO_CHANNELS::CHANNEL_3)
		{
			float dacInput = static_cast<float>(((getLogicalAmplitude(channel)) & 0x0F) >> pGBc_instance->GBc_state.audio.channel3OutputLevelAndShift);
			// Refer to https://x.com/LIJI32/status/964555034011815936/photo/1 for -ve sign
			dacOutput[channel] = -static_cast<float>((dacInput / 7.5f) - ((float)ONE));
		}
		else
		{
			float dacInput = static_cast<float>(getLogicalAmplitude(channel) * pGBc_instance->GBc_state.audio.audioChannelInstance[(uint8_t)(channel)].currentVolume);
			// Refer to https://x.com/LIJI32/status/964555034011815936/photo/1 for -ve sign
			dacOutput[channel] = -static_cast<float>((dacInput / 7.5f) - ((float)ONE));
		}
	}

	RETURN dacOutput[channel];
}

// NOTE: Refer https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware for information on the below filter used
// Used to remove the DC bias added by the DACs
float GBc_t::finHPF(float sampleIn)
{
	float sampleOut = MUTE_AUDIO;
	static float capacitor = MUTE_AUDIO;

	if (_ENABLE_AUDIO_HPF == YES)
	{
		sampleOut = sampleIn - capacitor;

		// NOTE: The charge factor can be calculated for any output sampling rate as 0.999958 ^ (4194304 / rate) for gb and 0.998943 ^ (4194304 / rate) for gbc
		// Refer : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware

		// capacitor slowly charges to 'in' via their difference
		if (ROM_TYPE == ROM::GAME_BOY)
		{
			capacitor = sampleIn - (sampleOut * 0.9963f);
		}
		else if (ROM_TYPE == ROM::GAME_BOY_COLOR)
		{
			capacitor = sampleIn - (sampleOut * 0.9117f);
		}
	}
	else
	{
		sampleOut = sampleIn;
	}

	RETURN sampleOut;
}

void GBc_t::captureDownsampledAudioSamples()
{
	pGBc_instance->GBc_state.audio.downSamplingRatioCounter += ONE;

	if (pGBc_instance->GBc_state.audio.downSamplingRatioCounter >= ((uint32_t)(GB_GBC_REFERENCE_CLOCK_HZ / EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC)))
	{
		pGBc_instance->GBc_state.audio.downSamplingRatioCounter -= ((uint32_t)(GB_GBC_REFERENCE_CLOCK_HZ / EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC));

		GBC_AUDIO_SAMPLE_TYPE leftSample = MUTE_AUDIO;
		GBC_AUDIO_SAMPLE_TYPE rightSample = MUTE_AUDIO;

		GBC_AUDIO_SAMPLE_TYPE channel1Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_1);
		GBC_AUDIO_SAMPLE_TYPE channel2Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_2);
		GBC_AUDIO_SAMPLE_TYPE channel3Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_3);
		GBC_AUDIO_SAMPLE_TYPE channel4Sample = getDACOutput(AUDIO_CHANNELS::CHANNEL_4);

		// process left samples
		if (DISABLE_FIRST_PULSE_CHANNEL == NO && pGBc_peripherals->NR51.channelSoundPanningFields.mixChannel1ToLeftOutput == ONE)
		{
			leftSample += channel1Sample;
		}
		if (DISABLE_SECOND_PULSE_CHANNEL == NO && pGBc_peripherals->NR51.channelSoundPanningFields.mixChannel2ToLeftOutput == ONE)
		{
			leftSample += channel2Sample;
		}
		if (DISABLE_WAVE_CHANNEL == NO && pGBc_peripherals->NR51.channelSoundPanningFields.mixChannel3ToLeftOutput == ONE)
		{
			leftSample += channel3Sample;
		}
		if (DISABLE_NOISE_CHANNEL == NO && pGBc_peripherals->NR51.channelSoundPanningFields.mixChannel4ToLeftOutput == ONE)
		{
			leftSample += channel4Sample;
		}

		leftSample /= FOUR;
		// Refer to https://x.com/LIJI32/status/964555034011815936/photo/1
		leftSample *= pGBc_peripherals->NR50.channelMasterVolumeAndVINPanningFields.leftOutputVolume + ONE;
		leftSample += DC_BIAS_FOR_AUDIO_SAMPLES;
		// Refer to https://x.com/LIJI32/status/964555034011815936/photo/1
		leftSample = finHPF(leftSample);

		// process right samples
		if (DISABLE_FIRST_PULSE_CHANNEL == NO && pGBc_peripherals->NR51.channelSoundPanningFields.mixChannel1ToRightOutput == ONE)
		{
			rightSample += channel1Sample;
		}
		if (DISABLE_SECOND_PULSE_CHANNEL == NO && pGBc_peripherals->NR51.channelSoundPanningFields.mixChannel2ToRightOutput == ONE)
		{
			rightSample += channel2Sample;
		}
		if (DISABLE_WAVE_CHANNEL == NO && pGBc_peripherals->NR51.channelSoundPanningFields.mixChannel3ToRightOutput == ONE)
		{
			rightSample += channel3Sample;
		}
		if (DISABLE_NOISE_CHANNEL == NO && pGBc_peripherals->NR51.channelSoundPanningFields.mixChannel4ToRightOutput == ONE)
		{
			rightSample += channel4Sample;
		}

		rightSample /= FOUR;
		// Refer to https://x.com/LIJI32/status/964555034011815936/photo/1
		rightSample *= pGBc_peripherals->NR50.channelMasterVolumeAndVINPanningFields.rightOutputVolume + ONE;
		rightSample += DC_BIAS_FOR_AUDIO_SAMPLES;
		// Refer to https://x.com/LIJI32/status/964555034011815936/photo/1
		rightSample = finHPF(rightSample);

		if (pGBc_instance->GBc_state.audio.accumulatedTone >= AUDIO_BUFFER_SIZE_FOR_GB_GBC)
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

			if (SDL_PutAudioStreamData(audioStream, pGBc_instance->GBc_state.audio.audioBuffer, sizeof(pGBc_instance->GBc_state.audio.audioBuffer)) == FAILURE)
			{
				SDL_Log("Could not put data on Audio stream, %s", SDL_GetError());
			}
			pGBc_instance->GBc_state.audio.accumulatedTone = RESET;
		}
		else
		{
			pGBc_instance->GBc_state.audio.audioBuffer[pGBc_instance->GBc_state.audio.accumulatedTone] = leftSample;
			++pGBc_instance->GBc_state.audio.accumulatedTone;
			if (pGBc_instance->GBc_state.audio.accumulatedTone < AUDIO_BUFFER_SIZE_FOR_GB_GBC)
			{
				pGBc_instance->GBc_state.audio.audioBuffer[pGBc_instance->GBc_state.audio.accumulatedTone] = rightSample;
				++pGBc_instance->GBc_state.audio.accumulatedTone;
			}
					
		}
	}

	RETURN;
}

void GBc_t::playTheAudioFrame()
{
	RETURN;
}

void GBc_t::OAMDMASTATModeGlitch()
{
#if (GB_GBC_ENABLE_DMA_STAT_OAM_BOUNDARY_GLITCH == YES)
	// Just verify the OAM DMA active state; the transfer may not have started yet
	if (pGBc_instance->GBc_state.emulatorStatus.isDMAActive) MASQ_UNLIKELY
	{
		// Note: Below STAT mode reset behaviour is needed by "dma_during_pixel_transfer_check_stat_oam_scan.gb"
		// This is confirmed from sameboy as well https://github.com/LIJI32/SameBoy/blob/master/Core/display.c#L526
		PPUTODO("Find additional source for \"DMA can glitch the STAT mode at the OAM boundary\"");
		if ((isPPULCDEnabled() == YES) && (pGBc_peripherals->STAT.lcdStatusFields.MODE == LCD_MODES::MODE_LCD_SEARCHING_OAM))
		{
			pGBc_instance->GBc_state.emulatorStatus.DMASTATGlitchEn = YES;
			PPUTODO("Find out whether only mode bits read HBLANK or actual PPU mode is changed to HBLANK?");
			pGBc_peripherals->STAT.lcdStatusFields.MODE = LCD_MODES::MODE_LCD_H_BLANK;
		}
		RETURN;
	}
	else if (pGBc_instance->GBc_state.emulatorStatus.DMASTATGlitchEn == YES) MASQ_UNLIKELY
	{
		pGBc_instance->GBc_state.emulatorStatus.DMASTATGlitchEn = NO;
		pGBc_peripherals->STAT.lcdStatusFields.MODE = pGBc_display->currentLCDMode;
		RETURN;
	}
	else MASQ_LIKELY
	{
		RETURN;
	}
#endif
}

void GBc_t::setPPULCDMode(LCD_MODES lcdMode)
{
	pGBc_display->currentSpecialLCDMode = lcdMode;
	pGBc_peripherals->STAT.lcdStatusFields.MODE = lcdMode;

	// Trigger the DMA STAT glitch if needed!
	// This is needed by "dma_during_pixel_transfer_check_stat_oam_scan.gb"
	OAMDMASTATModeGlitch();
}

GBc_t::LCD_MODES GBc_t::getPPULCDMode()
{
	RETURN (LCD_MODES)pGBc_peripherals->STAT.lcdStatusFields.MODE;
}

FLAG GBc_t::isPPULCDEnabled()
{
	RETURN (pGBc_peripherals->LCDC.lcdControlFields.LCD_PPU_ENABLE == ONE);
}

void GBc_t::compareLYToLYC(ID LY)
{
	if (isPPULCDEnabled() == YES)
	{
		if (LY == pGBc_peripherals->LYC)
		{
			pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_FLAG = ONE;
			if (pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_STAT_INT_SRC == ONE)
			{
				if (pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.aggregateSignal == LO)
				{
					pGBc_instance->GBc_state.emulatorStatus.STAT_src = STAT_INTR_SRC::LY_LYC;
					requestInterrupts(INTERRUPTS::LCD_STAT_INTERRUPT);
				}

				pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.LY_LYC_SIGNAL = HI;
			}
		}
		else
		{
			pGBc_peripherals->STAT.lcdStatusFields.LYC_EQL_LY_FLAG = ZERO;
			pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.LY_LYC_SIGNAL = LO;
		}
	}
	else
	{
		// Note that LY==LYC signal should not be made zero here as PPU disable means 0 == 0 condition is satisfied
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.HBLANK_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.VBLANK_SIGNAL = LO;
		pGBc_instance->GBc_state.emulatorStatus.STATInterruptSignal.STATInterruptSources.OAM_SIGNAL = LO;
	}
}

void GBc_t::processLCDEnable()
{
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY = ZERO;
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode = ZERO;
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerFrame = ZERO;

	// Reset the fetcher FSM
	pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;
	// Reset the Pixel FIFOs
	pGBc_display->bgWinPixelFIFO.clearFIFO();
	pGBc_display->tempBgWinPixelFIFO.clearFIFO();
	pGBc_display->objPixelFIFO.clearFIFO();
	// Reset other flags
	pGBc_display->fakeBgFetcherRuns = ZERO;
	pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = CLEAR;
	pGBc_display->isThereAnyObjectCurrentlyGettingRendered = CLEAR;
	pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray = INVALID;
	pGBc_display->visibleObjectsPerScanLine = NULL;
	pGBc_display->shouldIncrementWindowLineCounter = CLEAR;
	pGBc_display->waitForNextLineForWindSyncGlitch = NO;
	pGBc_display->performWindSyncGlitch = NO;
	pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
	pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = NO;
	pGBc_display->pixelFetcherCounterPerScanLine = RESET;
	pGBc_display->pixelRenderCounterPerScanLine = -EIGHT;
	pGBc_display->pixelFetcherDots = RESET;
	pGBc_display->pixelRendererDots = RESET;
	pGBc_display->pixelPipelineDots = RESET;
	pGBc_display->isNewM3Scanline = YES;
	pGBc_display->shouldSimulateBGScrollingPenaltyNow = YES;
	pGBc_display->oamSearchCount = ZERO;
	pGBc_display->spriteCountPerScanLine = ZERO;
	pGBc_display->isTheLastVblankLine = NO;
	pGBc_display->wasVblankJustTriggerred = CLEAR;
	pGBc_display->blockOAMR = NO;
	pGBc_display->blockVramR = NO;
	pGBc_display->blockOAMW = NO;
	pGBc_display->blockVramW = NO;
	pGBc_display->blockCGBPalette = NO;
	pGBc_display->wasFetchingOBJ = NO;
	pGBc_display->prevSpriteX = INVALID;
	pGBc_display->wasNotFirstSpriteInX = NO;
	pGBc_display->nX159SpritesPresent = ZERO;
	pGBc_display->wasX0Object = NO;

	pGBc_display->currentScanline = ZERO;
	pGBc_peripherals->LY = ZERO;
	compareLYToLYC(pGBc_peripherals->LY);

	pGBc_display->currentLCDMode = LCD_MODES::MODE_LCD_H_BLANK;
	setPPULCDMode(LCD_MODES::MODE_LCD_H_BLANK);

	// Wierd PPU behaviour on PPU OFF -> PPU ON
	pGBc_display->lcdJustEn = YES;
	pGBc_display->skipMode2 = YES;

	// Blank for 1 frame
	pGBc_emuStatus->freezeLCDOneFrame = YES;

	// The initial mode 0 takes 4 less cycles
	// Ideally, when we start in mode 0, ppuCounterPerLY should be 456 - x where x is the number of cycles we want to spend in this special mode!
	// As per https://www.reddit.com/r/EmuDev/comments/8uahbc/dmg_bgb_lcd_timings_and_cnt/
	// Eventhough PPU starts in mode 0, it takes only 80 cycles to next stage similar to mode 2
	// On top of this, as per documentation present in https://github.com/Gekkio/mooneye-test-suite/blob/main/acceptance/ppu/lcdon_timing-GS.s
	// PPU is 2T cycles late
	// But https://discord.com/channels/465585922579103744/465586075830845475/1357824330343645495 mentions 4T cycles shorter
	uint16_t reduceBy = FOUR;

	// Enabling the below deactivated code causes pokemon "video.gbc" to have graphical artifacts
#if (DEACTIVATED)
	if (isCGBDoubleSpeedEnabled() == YES)
	{
		reduceBy /= TWO;
	}
#endif

	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY = 
		LCD_MODE_CYCLES::LCD_TOTAL_CYCLES_PER_SCANLINE - (LCD_MODE_CYCLES::LCD_SEARCHING_OAM - reduceBy);

	pGBc_display->tickAtMode3ToMode0 = RESET;

	// We are about enter HBLANK, so allow HDMA to block cpu pipeline if proper conditions are met
	if (pGBc_instance->GBc_state.emulatorStatus.cgbDMAMode == CGB_DMA_MODE::HDMA
		&& pGBc_instance->GBc_state.emulatorStatus.isHDMAActive == YES)
	{
		pGBc_instance->GBc_state.emulatorStatus.isHDMAAllowedToBlockCPUPipeline = YES;
	}
}

void GBc_t::processLCDDisable()
{
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY = ZERO;
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode = ZERO;
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerFrame = ZERO;

	// Reset the fetcher FSM
	pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;
	// Reset the Pixel FIFOs
	pGBc_display->bgWinPixelFIFO.clearFIFO();
	pGBc_display->tempBgWinPixelFIFO.clearFIFO();
	pGBc_display->objPixelFIFO.clearFIFO();
	// Reset other flags
	pGBc_display->fakeBgFetcherRuns = ZERO;
	pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = CLEAR;
	pGBc_display->isThereAnyObjectCurrentlyGettingRendered = CLEAR;
	pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray = INVALID;
	pGBc_display->visibleObjectsPerScanLine = NULL;
	pGBc_display->shouldIncrementWindowLineCounter = CLEAR;
	pGBc_display->waitForNextLineForWindSyncGlitch = NO;
	pGBc_display->performWindSyncGlitch = NO;
	pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
	pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = NO;
	pGBc_display->pixelFetcherCounterPerScanLine = RESET;
	pGBc_display->pixelRenderCounterPerScanLine = -EIGHT;
	pGBc_display->pixelFetcherDots = RESET;
	pGBc_display->pixelRendererDots = RESET;
	pGBc_display->pixelPipelineDots = RESET;
	pGBc_display->isNewM3Scanline = YES;
	pGBc_display->shouldSimulateBGScrollingPenaltyNow = YES;
	pGBc_display->oamSearchCount = ZERO;
	pGBc_display->spriteCountPerScanLine = ZERO;
	pGBc_display->isTheLastVblankLine = NO;
	pGBc_display->wasVblankJustTriggerred = CLEAR;
	pGBc_display->blockOAMR = NO;
	pGBc_display->blockVramR = NO;
	pGBc_display->blockOAMW = NO;
	pGBc_display->blockVramW = NO;
	pGBc_display->blockCGBPalette = NO;
	pGBc_display->wasFetchingOBJ = NO;
	pGBc_display->prevSpriteX = INVALID;
	pGBc_display->wasNotFirstSpriteInX = NO;
	pGBc_display->nX159SpritesPresent = ZERO;
	pGBc_display->wasX0Object = NO;

	setPPULCDMode(LCD_MODES::MODE_LCD_H_BLANK);
}

// Needed for CGB's DMG compatibility mode
BYTE GBc_t::getColorNumberFromColorIDForGB(BYTE palette, BYTE colorID)
{
	BYTE uColor = ZERO;
	BYTE firstBit = ZERO;
	BYTE zerothBit = ZERO;

	switch (colorID)
	{
	case 0: firstBit = 1; zerothBit = 0; BREAK;
	case 1: firstBit = 3; zerothBit = 2; BREAK;
	case 2: firstBit = 5; zerothBit = 4; BREAK;
	case 3: firstBit = 7; zerothBit = 6; BREAK;
	}

	uColor = GETBIT(firstBit, palette) << 1;
	uColor |= GETBIT(zerothBit, palette);

	RETURN uColor;
};

GBc_t::COLOR_FORMAT GBc_t::getColorFromColorIDForGB(BYTE palette, BYTE colorID)
{
	COLOR_FORMAT color;
	BYTE uColor = ZERO;
	BYTE firstBit = ZERO;
	BYTE zerothBit = ZERO;

	switch (colorID)
	{
	case 0: firstBit = 1; zerothBit = 0; BREAK;
	case 1: firstBit = 3; zerothBit = 2; BREAK;
	case 2: firstBit = 5; zerothBit = 4; BREAK;
	case 3: firstBit = 7; zerothBit = 6; BREAK;
	}

	uColor = GETBIT(firstBit, palette) << 1;
	uColor |= GETBIT(zerothBit, palette);

	switch (uColor)
	{
	case 0: color = paletteIDToColor.at(pGBc_instance->GBc_state.gb_palette).COLOR_000P; BREAK;
	case 1: color = paletteIDToColor.at(pGBc_instance->GBc_state.gb_palette).COLOR_033P; BREAK;
	case 2: color = paletteIDToColor.at(pGBc_instance->GBc_state.gb_palette).COLOR_066P; BREAK;
	case 3: color = paletteIDToColor.at(pGBc_instance->GBc_state.gb_palette).COLOR_099P; BREAK;
	}

	RETURN color;
};

GBc_t::COLOR_FORMAT GBc_t::getColorFromColorIDForGBC(uint16_t colorID, FLAG isColorCorrectionEnabled)
{
	COLOR_FORMAT colorOfInterest;

	colorOfInterest.COLOR.r = (((((colorID >> 00) & 0x1F) * 527) + 23) >> 6);
	colorOfInterest.COLOR.g = (((((colorID >> 05) & 0x1F) * 527) + 23) >> 6);
	colorOfInterest.COLOR.b = (((((colorID >> 10) & 0x1F) * 527) + 23) >> 6);

	// Gearboy is the source of the expression used below of for cgb color correction

	if (isColorCorrectionEnabled == true)
	{

		uint8_t red = (uint8_t)(((colorOfInterest.COLOR.r * 0.8125f) + (colorOfInterest.COLOR.g * 0.125f) + (colorOfInterest.COLOR.b * 0.0625f)) * 0.95f);
		uint8_t green = (uint8_t)(((colorOfInterest.COLOR.g * 0.75f) + (colorOfInterest.COLOR.b * 0.25f)) * 0.95f);
		uint8_t blue = (uint8_t)((((colorOfInterest.COLOR.r * 0.1875f) + (colorOfInterest.COLOR.g * 0.125f) + (colorOfInterest.COLOR.b * 0.6875f))) * 0.95f);

		colorOfInterest.COLOR.r = red;
		colorOfInterest.COLOR.g = green;
		colorOfInterest.COLOR.b = blue;
	}

	RETURN colorOfInterest;
}

void GBc_t::setPaletteIndexForCGB(FLAG isThisForBackground, uint8_t value)
{
	// Encoding of incoming "value" and its corresponding xCPD meaning is as follows
	// 0	-> xGP0 byte 0 ->	lower byte color ID 0	    -> in xCPD -> R/G
	// 1	-> xGP0 byte 1 ->	higher byte color ID 0		-> in xCPD -> G/B
	// 2	-> xGP0 byte 2 ->	lower byte color ID 1		-> in xCPD -> R/G
	// 3	-> xGP0 byte 3 ->	higher byte color ID 1		-> in xCPD -> G/B
	// 4	-> xGP1 byte 0 ->	lower byte color ID 2		-> in xCPD -> R/G
	// :	   
	// 63	-> xGP7 byte 3 ->	higher byte color ID 31		-> in xCPD -> G/B

	uint16_t palette_ID = ((value >> 3) & 0x07);

	// perform the following to get the color ID from value
	// (value / total colors per palettes) = (value / 2) => is same as => ((value >> 1) & 0x03)

	uint16_t color_ID = ((value >> 1) & 0x03);

	// perform the following to determine if higher byte or lower byte of color
	// if bit 0 is 0, then lower, otherwise higher

	FLAG color_high_byte = (GETBIT(ZERO, value) == ONE ? true : false);

	if (isThisForBackground == true)
	{
		// Update BEFORE_PROCESSING value from color ram w.r.t address specified in BCPS to BCPD (for read BCPD operation)

		// NOTE: Below code snippet is for Read Color/Palette RAM Operation
		// If we had set the address via BCPS to READ from BG section of Color/Palette RAM via BCPD;
		// The below code snippet takes care of setting the appropriate data from Color/Palette RAM to BCPD

		if (color_high_byte == true)
		{
			pGBc_peripherals->BCPD_BGPD = pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM
				[palette_ID][color_ID].gbcColorByteFields.HIGHER_BYTE;
		}
		else
		{
			pGBc_peripherals->BCPD_BGPD = pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM
				[palette_ID][color_ID].gbcColorByteFields.LOWER_BYTE;
		}
	}
	else
	{
		// Update BEFORE_PROCESSING value from color ram w.r.t address specified in OCPS to OCPD (for read OCPD operation)

		// NOTE: Below code snippet is for Read Color/Palette RAM Operation
		// If we had set the address via OCPS to READ from OBJ section of Color/Palette RAM via OCPS;
		// The below code snippet takes care of setting the appropriate data from Color/Palette RAM to OCPD

		if (color_high_byte == true)
		{
			pGBc_peripherals->OCPD_OBPD = pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM
				[palette_ID][color_ID].gbcColorByteFields.HIGHER_BYTE;
		}
		else
		{
			pGBc_peripherals->OCPD_OBPD = pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM
				[palette_ID][color_ID].gbcColorByteFields.LOWER_BYTE;
		}
	}

}

void GBc_t::setPaletteColorForCGB(FLAG isThisForBackground, uint8_t value)
{
	uint8_t paletteSpecification = ZERO;
	uint16_t updated_BEFORE_PROCESSING = ZERO;

	FLAG color_high_byte = ZERO;
	uint16_t color_ID = ZERO;
	uint16_t palette_ID = ZERO;

	if (isThisForBackground == true)
	{
		paletteSpecification = pGBc_peripherals->BCPS_BGPI.BCPSMemory;

		// perform the following to get the palette from paletteSpecification
		// (paletteSpecification / total palettes) = (paletteSpecification / 8) => is same as => ((paletteSpecification >> 3) & 0x07)

		palette_ID = ((paletteSpecification >> 3) & 0x07);

		// perform the following to get the color ID from paletteSpecification
		// (paletteSpecification / total colors per palettes) = (paletteSpecification / 2) => is same as => ((paletteSpecification >> 1) & 0x03)

		color_ID = ((paletteSpecification >> 1) & 0x03);

		// perform the following to determine if higher byte or lower byte of color
		// if bit 0 is 0, then lower, otherwise higher

		color_high_byte = (GETBIT(ZERO, paletteSpecification) == ONE ? true : false);

		if (color_high_byte == true)
		{
			pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM
				[palette_ID][color_ID].gbcColorByteFields.HIGHER_BYTE = value;
		}
		else
		{
			pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM
				[palette_ID][color_ID].gbcColorByteFields.LOWER_BYTE = value;
		}

		updated_BEFORE_PROCESSING = pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM
			[palette_ID][color_ID].gbcColor;
	}
	else
	{
		paletteSpecification = pGBc_peripherals->OCPS_OBPI.OCPSMemory;

		// perform the following to get the palette from paletteSpecification
		// (paletteSpecification / total palettes) = (paletteSpecification / 8) => is same as => ((paletteSpecification >> 3) & 0x07)

		palette_ID = ((paletteSpecification >> 3) & 0x07);

		// perform the following to get the color ID from paletteSpecification
		// (paletteSpecification / total colors per palettes) = (paletteSpecification / 2) => is same as => ((paletteSpecification >> 1) & 0x03)

		color_ID = ((paletteSpecification >> 1) & 0x03);

		// perform the following to determine if higher byte or lower byte of color
		// if bit 0 is 0, then lower, otherwise higher

		color_high_byte = (GETBIT(ZERO, paletteSpecification) == ONE ? true : false);

		if (color_high_byte == true)
		{
			pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM
				[palette_ID][color_ID].gbcColorByteFields.HIGHER_BYTE = value;
		}
		else
		{
			pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM
				[palette_ID][color_ID].gbcColorByteFields.LOWER_BYTE = value;
		}

		updated_BEFORE_PROCESSING = pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM
			[palette_ID][color_ID].gbcColor;
	}
}

#pragma region GFX_SUMMARY
/*
*
* we have the baseTileDataAddress and baseTileMapAddress;
* decode the data in the baseTileMapAddress;
* within the tile map region, we have around 1024 bytes;
* each bytes gives the tile ID in the tile data region;
* the addressing from map -> tile can be signed or unsigned;
* each 16 byte data in tile data area represents a tile;
* this encodes the color ID;
*
* so now, we have the scan line number
* from the scan line number, get the y position
* after getting y position, start from x position 0
* read the corresponding 16 byte data from tile data region
* get the color ID
* based on the current palette, get the actual olc color
* save this in the gfx_BG_WINDOW_OBJ
*
*
*
*
*
* ORIGIN  -->
*	|  ________________________________________________
*   v  |                INVISIBLE                     |
*      |     ____________________________________     |________________ END of VBLANK
*      |    |                                    |    |Scan Line = 0
*      |    |                                    |    |
*      |    |                                    |    |
*      |    |                                    |    |
*      |    |            VISIBLE                 |    |
*      |    |                                    |    |
*      |    |+++++++++ Current ScanLine ---------|    |
*      |    |                                    |    |
*      |    |                                    |    |
*      |    |                                    |    |
*      |    |____________________________________|    |________________ Start of VBLANK
*      |                                              |
*      |______________________________________________|
*
*     + indicates pixels that have been updated as of now in current scan line
*     - indicates pixels that have to be updated yet in current scan line
*
*
*
*
*
*
* As mentioned earlier, the viewing area is w.r.t yBG and xBG
* the window layer is w.r.t to viewing area
* that means, window is within the background which is within the display
*
*
* ORIGIN  -->
*  |   ________________________________________________
*  v   |											  |
*      |     ____________________________________     |________________ Start of BACKGROUND AREA (yBG)
*      |    |      BACKGROUND LAYER              |    |
*      |    |                                    |    |
*      |    |                                    |    |
*      |    |                                    |    |
*      |    |					                 |    |
*      |    |                                    |    |
*      |    |+++++++++ Current ScanLine ---------|    |
*      |    |                                    |    |
*      |    |  *******************************   |    |________________ Start of WINDOW AREA (yWindow)
*      |    |  *   WINDOW LAYER              *   |    |
*      |    |  *******************************   |    |________________ END of WINDOW AREA
*      |    |____________________________________|    |________________ END of BACKGROUND AREA
*      |                                              |
*      |______________________________________________|
*
*
*
* if we are processing the WINDOW layer, that means current scan line number is definetly after yWindow
* so, yPixelCoordinate = scan line number - yWindow
* else if we are processing BACKGROUND layer, that mean current scan line number is definelty after yBG
* and scan line number 0 is at yBG
* hence, yPixelCoordinate = yBG + scan line number
*
*
*
*
*
*
* now, we have see how exactly in 2D is the tile map written
* tile map can be from 0x9800 to 0x9BFF or from 0x9C00 to 0x9FFF
* lets consider the 0x9800 to 0x9BFF case
*
*   <------------- 32 tiles, hence 32 bytes ------------>
*	0x9800 0x9801 0x9802 ...                      0x981F    ^
*   0x9820 0x9821 0x9822 ...                      0x983F	| 32 tiles, hence 32 columns
*	:														|
*   0x9BE0 0x9BE1 0x9B22 ...                      0x9BFF	v
*
* (1) now xPixelCoordinate and yPixelCoordinate is 0, 0
* this indicates tile map ID 0, which indicates address should be 0x9800
*
* (2) if xPixelCoordinate and yPixelCoordinate is 8, 0
* this indicates tile map ID 1, which indicates address should be 0x9801
*
* (3) if xPixelCoordinate and yPixelCoordinate is 17, 9
* this indicates tile map ID 33, which indicates address should be 0x9822
*
* for case (3) int(xPixelCoordinate / 8) and int(yPixelCoordinate / 8) is 2, 1
* this indicates xTileCoordinate = 2 and yTileCoordinate = 1
*
* to get the tile map address
*      = 0x9800 + (yTileCoordinate * 32 + xTileCoordinate)
*
*      = 0x9800 + (1 * 32 + 2)
*      = 0x9822
*
*
*
*
*
*
* once we have the address in tile map area, we read the content to get the tile ID
* from tile ID we need to get the address in tile data area
* its calculation is as mentioned below
*
* if signed addressing is false
*
*	Byte in the tile map				Address range of the tile in the tile set
*
*	0									0x8000-0x800F (16 bytes)
*	1									0x8010-0x801F (16 bytes)
*	2									0x8020-0x802F (16 bytes)
*	...									...
*	255									0x8FF0-0x8FFF (16 bytes)
*
*
* if signed addressing is true
*
*   Signed byte in the tile map			Address range of the tile in the tile set
*
*	-128								0x8800-0x880F (16 bytes)
*	-127								0x8810-0x881F (16 bytes)
*	-126								0x8820-0x882F (16 bytes)
*	...									...
*	0									0x9000-0x900F (16 bytes)
*	...									...
*	127									0x97F0-0x97FF (16 bytes)
*
*
*
*
*
* the 2 byte from tile data area is mapped to tile in gfx as follows
*
* consider the 16 bytes as follows
* 7C 7C 00 C6 C6 00 00 FE C6 C6 00 C6 C6 00 00 00
* how it gets mapped is depicted below
*
*	Tile:                                     Image:
*
*	.33333..                     .33333.. -> 01111100 -> 0x007C
*	22...22.                                 01111100 -> 0x007C
*	11...11.                     22...22. -> 00000000 -> 0x0000
*	2222222. <-- digits                      11000110 -> 0x00C6
*	33...33.     represent       11...11. -> 11000110 -> 0x00C6
*	22...22.     color                       00000000 -> 0x0000
*	11...11.     numbers         2222222. -> 00000000 -> 0x0000
*	........                                 11111110 -> 0x00FE
*							     33...33. -> 11000110 -> 0x00C6
*							  			     11000110 -> 0x00C6
*							     22...22. -> 00000000 -> 0x0000
*							  			     11000110 -> 0x00C6
*							     11...11. -> 11000110 -> 0x00C6
*							  			     00000000 -> 0x0000
*							     ........ -> 00000000 -> 0x0000
*							  			     00000000 -> 0x0000
*
* so, for a given scan line, we don't need to retrieve all 16 bytes,
* just 2 byte should be sufficient
* i.e. for line n, we need (n*2) and (n*2 + 1) byte
*
* so, we have xPixelCoordinate and yPixelCoordinate
* to get line number from yPixelCoordinate, we need to do the following
*
* there are 32 vertical rows of 32 tiles each
* each tile is of 8 pixel in height
* note that the address in tile data area represents which we got represents the the 0th row of a new tile
* now, yPixelCoordinate % 8 => vertical offset from 0th row of a new tile (one of the 32 x 32 tiles)
* now as mentioned above, from the row of interest
* we need to get (row*2) and (row*2 + 1) byte
*
* once we get the 2 bytes, we need to determine the relative column which we are working on
* so, xPixelCoordinate % 8 should give this
*
* note that 0th bit gives color of last column of a tile
* so, if it was the 0th column, then it represents 7th column
*
*
*
*
*
*  There are 40 sprites (8x8 or 8x16) located in memory region 0x8000 - 0x8FFF
*  At a time, only 10 sprites can be displayed per Scan Line
*  All these sprites have their attributes stored as 4 byte data in OAM region 0xFE00 - 0xFE9F
*  Go over all the sprites attributes and see whether they need to be rendered or not
*
*  Each of the 4 bytes of the attribute indicate the following
*  1) Y position
*  2) X position
*  3) Tile index
*  4) Attributes/Flags
*
*  While checking all the sprites, we basically check if the Scan Line is part of any of the Spite
*  If they are not part of current Scan Line, we ignore it
*
*
*
*
*
*
*  256 sprites are stored in location 0x8000 to 0x8FFF
*  Total bytes in region is 0x1000 = 4096 bytes
*  Each sprite is of 8x8 in size (8x16 are just 2 8x8 sprites)
*  So, each 8x8 sprite needs 16 bytes, hence 256 sprites
*  Therefore, tileIndex for sprite can go from 0x00 to 0xFF
*  But OAM has entry for only 40 sprites
*
*	tileIndex       Address range of the tile in the tile set
*
*	0				0x8000-0x800F (16 bytes)
*	1				0x8010-0x801F (16 bytes)
*	2				0x8020-0x802F (16 bytes)
*	...				...
*	255				0x8FF0-0x8FFF (16 bytes)
*
*	If tileIndex is 2, then
*		address of tile = 0x8000 + (2*16)
*
*
*   consider the 16 bytes as follows
*   7C 7C 00 C6 C6 00 00 FE C6 C6 00 C6 C6 00 00 00
*   how it gets mapped is depicted below
*
* 	 Tile:                                     Image:
*
* 	 .33333..                     .33333.. -> 01111100 -> 0x007C
* 	 22...22.                                 01111100 -> 0x007C
* 	 11...11.                     22...22. -> 00000000 -> 0x0000
* 	 2222222. <-- digits                      11000110 -> 0x00C6
* 	 33...33.     represent       11...11. -> 11000110 -> 0x00C6
* 	 22...22.     color                       00000000 -> 0x0000
* 	 11...11.     numbers         2222222. -> 00000000 -> 0x0000
* 	 ........                                 11111110 -> 0x00FE
* 	 						      33...33. -> 11000110 -> 0x00C6
* 	 						  		   	      11000110 -> 0x00C6
* 	 						      22...22. -> 00000000 -> 0x0000
* 	 						  			      11000110 -> 0x00C6
* 	 						      11...11. -> 11000110 -> 0x00C6
* 	 						  			      00000000 -> 0x0000
* 	 						      ........ -> 00000000 -> 0x0000
* 	 						  	   		      00000000 -> 0x0000
*
*   so, for a given scan line, we don't need to retrieve all 16 bytes,
*   just 2 byte should be sufficient
*   i.e. for line n, we need (n*2) and (n*2 + 1) byte
*
*   so, we have xPixelCoordinate and yPixelCoordinate
*   to get line number from yPixelCoordinate, we need to do the following
*
*   there are 32 vertical rows of 32 tiles each
*   each tile is of 8 pixel in height
*   note that the address in tile data area represents which we got represents the the 0th row of a new tile
*   now, yPixelCoordinate % 8 => vertical offset from 0th row of a new tile (one of the 32 x 32 tiles)
*   now as mentioned above, from the row of interest
*   we need to get (row*2) and (row*2 + 1) byte
*
*   once we get the 2 bytes, we need to determine the relative column which we are working on
*   so, xPixelCoordinate % 8 should give this
*
*   note that 0th bit gives color of last column of a tile
*   so, if it was the 0th column, then it represents 7th column
*
*/
#pragma endregion GFX_SUMMARY

void GBc_t::processPixelPipelineAndRender(int32_t dots)
{
	if (pGBc_display->pixelRenderCounterPerScanLine >= TO_UINT16(getScreenWidth()))
	{
		RETURN;
	}

	pGBc_display->pixelPipelineDots += dots;

	auto runPixelFetcher = [&]()
		{
			static const int16_t sizeOfEachTileData = SIXTEEN;
			static const int16_t tileIDOffsetForSignedAddressing = 128;
			uint8_t spriteCountPerScanLine = ZERO;
			BYTE whichTileDataAreaFor_BG_Window = pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_TILE_DATA_AREA;
			BYTE whichTileMapAreaForBG = pGBc_peripherals->LCDC.lcdControlFields.BG_TILE_MAP_AREA;
			BYTE whichTileMapAreaForWindow = pGBc_peripherals->LCDC.lcdControlFields.WINDOW_TILE_MAP_AREA;
			FLAG tileIDRequiresSignedAddressing = NO;
			uint16_t baseTileDataAddress = ZERO;
			uint16_t baseTileMapAddress = ZERO;
			int16_t xWindow = (int16_t)((int16_t)pGBc_peripherals->WX - SEVEN);
			int16_t tileID = ZERO;
			uint8_t yWithinPixelCoordinate = ZERO;
			uint16_t yTileCoordinate = ZERO;
			uint16_t xTileCoordinate = ZERO;
			BYTE rowOfTheSelectedTile = ZERO;
			BYTE columnOfTheSelectedTile = ZERO;
			BYTE firstTileDataBytePerRow = ZERO;
			BYTE secondTileDataBytePerRow = ZERO;
			BYTE colorBitOfCurrentPixel = ZERO;
			BYTE colorIDOfCurrentPixel = ZERO;
			int16_t xObjCoordinate = ZERO;
			int16_t yObjCoordinate = ZERO;
			FLAG is8x16 = pGBc_peripherals->LCDC.lcdControlFields.OBJ_SIZE == ONE;

			// Refer : https://gbdev.io/pandocs/Scrolling.html#scrolling
			// Alsp refer https://github.com/Ashiepaws/GBEDG/blob/master/ppu/index.md#scx-at-a-sub-tile-layer
			if (pGBc_display->isNewM3Scanline == YES)
			{
				pGBc_display->discardedPixelCount = RESET;
				pGBc_display->xBGPerPixel = (pGBc_peripherals->SCX & SEVEN); // Amount to discard for this scanline
				pGBc_display->isNewM3Scanline = CLEAR;
			}

			if (pGBc_display->shouldFetchAndRenderWindowInsteadOfBG == YES)
			{
				if (whichTileMapAreaForWindow == ZERO)
				{
					baseTileMapAddress = (uint16_t)TM0x9800_TM0x9BFF;
				}
				else
				{
					baseTileMapAddress = (uint16_t)TM0x9C00_TM0x9FFF;
				}
			}
			else
			{
				if (whichTileMapAreaForBG == ZERO)
				{
					baseTileMapAddress = (uint16_t)TM0x9800_TM0x9BFF;
				}
				else
				{
					baseTileMapAddress = (uint16_t)TM0x9C00_TM0x9FFF;
				}
			}

			if (whichTileDataAreaFor_BG_Window == ZERO)
			{
				baseTileDataAddress = (uint16_t)TD0x8800_TD0x97FF;
				tileIDRequiresSignedAddressing = YES;
			}
			else
			{
				baseTileDataAddress = (uint16_t)TD0x8000_TD0x8FFF;
			}

			if (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == YES)
			{
				if (pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray == INVALID)
				{
					FATAL("Trying to fetch invalid sprite");
				}
				xObjCoordinate = (BYTE)(pGBc_display->arrayOfVisibleObjectsPerScanLine
					[pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray].oamEntry.xPosition) - EIGHT;
				yObjCoordinate = (BYTE)(pGBc_display->arrayOfVisibleObjectsPerScanLine
					[pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray].oamEntry.yPosition) - SIXTEEN;
			}
			else
			{
				if (pGBc_display->shouldFetchAndRenderWindowInsteadOfBG == YES)
				{
					yWithinPixelCoordinate = pGBc_display->windowLineCounter & SEVEN;
					xTileCoordinate = (pGBc_display->pixelFetcherCounterPerScanLine - xWindow) / EIGHT;
					xTileCoordinate &= 0x1F;
					yTileCoordinate = pGBc_display->windowLineCounter / EIGHT;
					yTileCoordinate &= 0x1F;
				}
				else
				{
					yWithinPixelCoordinate = (pGBc_peripherals->LY + pGBc_peripherals->SCY) & SEVEN;

					// Low 3 bits of SCX should be read only during start of new scanline
					// Refer :  https://gbdev.io/pandocs/Scrolling.html#scrolling
					// But here, pGBc_peripherals->SCX divide by 8 masks our the last 3 bits and hence alls good!
					xTileCoordinate = (pGBc_display->pixelFetcherCounterPerScanLine + pGBc_peripherals->SCX) / EIGHT;
					xTileCoordinate &= 0x1F;
					yTileCoordinate = (pGBc_peripherals->LY + pGBc_peripherals->SCY) / EIGHT;
					if (ROM_TYPE == ROM::GAME_BOY_COLOR)
					{
						// Refer : https://gbdev.io/pandocs/Scrolling.html#scrolling (CGB-E)
						if (pGBc_display->pixelFetcherState == PIXEL_FETCHER_STATES::WAIT_FOR_TILE)
						{
							pGBc_display->latchedSCYForGBC = pGBc_peripherals->SCY; // SCY is latched and hence mid scanline changes wont take effect
						}
						else
						{
							yWithinPixelCoordinate = (pGBc_peripherals->LY + pGBc_display->latchedSCYForGBC) & SEVEN;
							yTileCoordinate = (pGBc_peripherals->LY + pGBc_display->latchedSCYForGBC) / EIGHT;
						}
					}
					yTileCoordinate &= 0x1F;
				}
			}

			switch (pGBc_display->pixelFetcherState)
			{
			case PIXEL_FETCHER_STATES::DUMMY:
			{
				FATAL("Unknown Pixel Fetcher State");
				BREAK;
			}
			case PIXEL_FETCHER_STATES::WAIT_FOR_TILE: // 1st cycle
			{
				if (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == YES)
				{
					pGBc_display->wasFetchingOBJ = YES;
				}
				else
				{
					pGBc_display->addressInTileMapArea = baseTileMapAddress + (xTileCoordinate + (yTileCoordinate * THIRTYTWO));
					pGBc_display->wasFetchingOBJ = NO;
				}
				pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::GET_TILE;
				BREAK;
			}
			case PIXEL_FETCHER_STATES::GET_TILE: // 2nd cycle
			{
				if (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == YES)
				{
					BYTE tileIndex = (BYTE)pGBc_display->arrayOfVisibleObjectsPerScanLine
						[pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray].oamEntry.tileIndex;

					if (is8x16 == YES)
					{
						tileIndex &= 0xFE;
					}

					pGBc_display->pixelFetcherContext.objTileID = tileIndex;

					pGBc_display->wasFetchingOBJ = YES;
				}
				else
				{
					pGBc_display->pixelFetcherContext.bgWinTileID = 
						readRawMemory(pGBc_display->addressInTileMapArea, MEMORY_ACCESS_SOURCE::PPU, false, true);

					pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesMemory
						= (ROM_TYPE == ROM::GAME_BOY_COLOR ? readRawMemory(pGBc_display->addressInTileMapArea, MEMORY_ACCESS_SOURCE::PPU, true) : ZERO);

					pGBc_display->wasFetchingOBJ = NO;
				}
				pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_DATA_LOW;
				BREAK;
			}
			case PIXEL_FETCHER_STATES::WAIT_FOR_DATA_LOW: // 3rd cycle
			{
				if (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == YES)
				{
					BYTE tileIndex = pGBc_display->pixelFetcherContext.objTileID;
					oamEntryByte_t attribute = pGBc_display->arrayOfVisibleObjectsPerScanLine
						[pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray].oamEntry.attributes;

					auto verticalSizeOfSprite = (is8x16 == YES) ? SIXTEEN : EIGHT;
					auto relativeRowOfSprite = pGBc_peripherals->LY - yObjCoordinate;
					if (attribute.oamEntryFields.OAM_Y_FLIP == ONE)
					{
						relativeRowOfSprite = verticalSizeOfSprite - ONE - relativeRowOfSprite;
					}

					pGBc_display->addressInTileDataArea = TD0x8000_TD0x8FFF + (sizeOfEachTileData * tileIndex) + (TWO * relativeRowOfSprite);

					pGBc_display->wasFetchingOBJ = YES;
				}
				else
				{
					BYTE tileID = pGBc_display->pixelFetcherContext.bgWinTileID;

					rowOfTheSelectedTile = yWithinPixelCoordinate;

					if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_YFLIP == ONE)
					{
						rowOfTheSelectedTile = SEVEN - rowOfTheSelectedTile;
					}

					if (tileIDRequiresSignedAddressing == NO)
					{
						pGBc_display->addressInTileDataArea =
							baseTileDataAddress
							+ (sizeOfEachTileData * (BYTE)tileID)
							+ (TWO * rowOfTheSelectedTile);
					}
					else
					{
						pGBc_display->addressInTileDataArea =
							baseTileDataAddress
							+ (sizeOfEachTileData * ((SBYTE)tileID + tileIDOffsetForSignedAddressing))
							+ (TWO * rowOfTheSelectedTile);
					}

					pGBc_display->cached_BG_WINDOW_TILE_DATA_AREA 
						= pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_TILE_DATA_AREA;

					pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER 
						= pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_TILE_VRAM_BANK_NUMBER;

					pGBc_display->wasFetchingOBJ = NO;
				}
				pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::GET_TILE_DATA_LOW;
				BREAK;
			}
			case PIXEL_FETCHER_STATES::GET_TILE_DATA_LOW: // 4th cycle
			{
				if (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == YES)
				{
					oamEntryByte_t attribute = pGBc_display->arrayOfVisibleObjectsPerScanLine
						[pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray].oamEntry.attributes;

					if (ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBCompatibilityModeEnabled() == YES)
					{
						// NOTE: This particular check was added based on https://gbdev.io/pandocs/OAM.html#byte-3--attributesflags 
						PPUTODO("Why the particular check involving \"isCGBCompatibilityModeEnabled\" is needed because why would any DMG game's sprite have its attribute set to bank 1 ?");
						pGBc_display->pixelFetcherContext.objTileDataLo
							= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU, false, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY_COLOR && attribute.oamEntryFields.OAM_TILE_VRAM_BANK == ONE)
					{
						pGBc_display->pixelFetcherContext.objTileDataLo
							= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY_COLOR && attribute.oamEntryFields.OAM_TILE_VRAM_BANK == ZERO)
					{
						pGBc_display->pixelFetcherContext.objTileDataLo
							= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU, false, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY)
					{
						pGBc_display->pixelFetcherContext.objTileDataLo
							= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU);
					}

					pGBc_display->wasFetchingOBJ = YES;
				}
				else
				{
#if (GB_GBC_ENABLE_TILE_SEL_GLITCH == YES)
					FLAG isGlitched = pGBc_display->tileSelGlitchTCycles > RESET;
					FLAG useGlitched = NO;

					if (isGlitched == YES)
					{
						if (pGBc_display->cached_BG_WINDOW_TILE_DATA_AREA == SET)
						{
							useGlitched = !((FLAG)(pGBc_display->pixelFetcherContext.bgWinTileID & 0x80));
							pGBc_display->pixelFetcherContext.bgWinTileDataLo = pGBc_display->pixelFetcherContext.bgWinTileID;
						}
						else
						{
							useGlitched = YES;
							pGBc_display->pixelFetcherContext.bgWinTileDataLo = pGBc_display->tileSelGlitchedData;
						}
					}

					if (useGlitched == NO)
					{
						if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ONE)
						{
							pGBc_display->pixelFetcherContext.bgWinTileDataLo
								= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU, true);
						}
						else if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ZERO)
						{
							pGBc_display->pixelFetcherContext.bgWinTileDataLo
								= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU, false, true);
						}
						else if (ROM_TYPE == ROM::GAME_BOY)
						{
							pGBc_display->pixelFetcherContext.bgWinTileDataLo
								= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU);
						}
					}
					
					if (pGBc_display->cached_BG_WINDOW_TILE_DATA_AREA == SET && isGlitched == YES)
					{
						if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ONE)
						{
							pGBc_display->tileSelGlitchedData
								= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU, true);
						}
						else if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ZERO)
						{
							pGBc_display->tileSelGlitchedData
								= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU, false, true);
						}
						else if (ROM_TYPE == ROM::GAME_BOY)
						{
							FATAL("TILE_SEL glitch not applicable in DMG");
						}
					}
#else
					if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ONE)
					{
						pGBc_display->pixelFetcherContext.bgWinTileDataLo
							= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ZERO)
					{
						pGBc_display->pixelFetcherContext.bgWinTileDataLo
							= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU, false, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY)
					{
						pGBc_display->pixelFetcherContext.bgWinTileDataLo
							= readRawMemory(pGBc_display->addressInTileDataArea, MEMORY_ACCESS_SOURCE::PPU);
					}
#endif
					pGBc_display->wasFetchingOBJ = NO;
				}
				pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_DATA_HIGH;
				BREAK;
			}
			case PIXEL_FETCHER_STATES::WAIT_FOR_DATA_HIGH: // 5th cycle
			{
				if (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == YES)
				{
					BYTE tileIndex = pGBc_display->pixelFetcherContext.objTileID;
					oamEntryByte_t attribute = pGBc_display->arrayOfVisibleObjectsPerScanLine
						[pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray].oamEntry.attributes;

					auto verticalSizeOfSprite = (is8x16 == YES) ? SIXTEEN : EIGHT;
					auto relativeRowOfSprite = pGBc_peripherals->LY - yObjCoordinate;
					if (attribute.oamEntryFields.OAM_Y_FLIP == ONE)
					{
						relativeRowOfSprite = verticalSizeOfSprite - ONE - relativeRowOfSprite;
					}

					pGBc_display->addressInTileDataArea = TD0x8000_TD0x8FFF + (sizeOfEachTileData * tileIndex) + (TWO * relativeRowOfSprite);

					pGBc_display->wasFetchingOBJ = YES;
				}
				else
				{
					BYTE tileID = pGBc_display->pixelFetcherContext.bgWinTileID;
					rowOfTheSelectedTile = yWithinPixelCoordinate;

					if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_YFLIP == ONE)
					{
						rowOfTheSelectedTile = SEVEN - rowOfTheSelectedTile;
					}

					if (tileIDRequiresSignedAddressing == NO)
					{
						pGBc_display->addressInTileDataArea =
							baseTileDataAddress
							+ (sizeOfEachTileData * (BYTE)tileID)
							+ (TWO * rowOfTheSelectedTile);
					}
					else
					{
						pGBc_display->addressInTileDataArea =
							baseTileDataAddress
							+ (sizeOfEachTileData * ((SBYTE)tileID + tileIDOffsetForSignedAddressing))
							+ (TWO * rowOfTheSelectedTile);
					}

					pGBc_display->cached_BG_WINDOW_TILE_DATA_AREA
						= pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_TILE_DATA_AREA;

					pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER 
						= pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_TILE_VRAM_BANK_NUMBER;

					pGBc_display->wasFetchingOBJ = NO;
				}
				pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::GET_TILE_DATA_HIGH;
				BREAK;
			}
			case PIXEL_FETCHER_STATES::GET_TILE_DATA_HIGH: // 6th cycle
			{
				if (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == YES)
				{
					oamEntryByte_t attribute = pGBc_display->arrayOfVisibleObjectsPerScanLine
						[pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray].oamEntry.attributes;

					byte tileDataLo = pGBc_display->pixelFetcherContext.objTileDataLo;

					if (ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBCompatibilityModeEnabled() == YES)
					{
						// NOTE: This particular check was added based on https://gbdev.io/pandocs/OAM.html#byte-3--attributesflags 
						PPUTODO("Why the particular check involving \"isCGBCompatibilityModeEnabled\" is needed because why would any DMG game's sprite have its attribute set to bank 1 ?");
						pGBc_display->pixelFetcherContext.objTileDataHi
							= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU, false, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY_COLOR && attribute.oamEntryFields.OAM_TILE_VRAM_BANK == ONE)
					{
						pGBc_display->pixelFetcherContext.objTileDataHi
							= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY_COLOR && attribute.oamEntryFields.OAM_TILE_VRAM_BANK == ZERO)
					{
						pGBc_display->pixelFetcherContext.objTileDataHi
							= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU, false, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY)
					{
						pGBc_display->pixelFetcherContext.objTileDataHi
							= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU);
					}

					byte tileDataHi = pGBc_display->pixelFetcherContext.objTileDataHi;

					auto pixelFifoEntriesCount = ZERO;
					pixelFIFOEntity_t newPixelFifoEntries[EIGHT];
					for (BYTE relativeColumnOfSprite = ZERO; relativeColumnOfSprite < EIGHT; relativeColumnOfSprite++)
					{
						colorBitOfCurrentPixel = SEVEN - relativeColumnOfSprite;

						if (attribute.oamEntryFields.OAM_X_FLIP == ONE)
						{
							colorBitOfCurrentPixel = SEVEN - colorBitOfCurrentPixel;
						}

						colorIDOfCurrentPixel = GETBIT(colorBitOfCurrentPixel, tileDataHi);
						colorIDOfCurrentPixel <<= ONE;
						colorIDOfCurrentPixel |= GETBIT(colorBitOfCurrentPixel, tileDataLo);

						// Sanity check (mostly for sprites with X = [-7, -1])
						// Note that xObjCoordinate is Sprite X - 8
						int16_t xPixel = relativeColumnOfSprite + xObjCoordinate;

						if (ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBCompatibilityModeEnabled() == NO)
						{
							// Creating a map of "tile data address" vs "palette ID" for the debugger
							mapPalette[pGBc_display->addressInTileDataArea - VRAM_START_ADDRESS] = attribute.oamEntryFields.OAM_PALETTE_NUMBER_CGB | TYPE_OBJ;
							mapPalette[pGBc_display->addressInTileDataArea + ONE - VRAM_START_ADDRESS] = attribute.oamEntryFields.OAM_PALETTE_NUMBER_CGB | TYPE_OBJ;
						}
						else
						{
							// Creating a map of "tile data address" vs "palette ID" for the debugger
							mapPalette[pGBc_display->addressInTileDataArea - VRAM_START_ADDRESS] = attribute.oamEntryFields.OAM_PALETTE_NUMBER_DMG | TYPE_OBJ;
							mapPalette[pGBc_display->addressInTileDataArea + ONE - VRAM_START_ADDRESS] = attribute.oamEntryFields.OAM_PALETTE_NUMBER_DMG | TYPE_OBJ;
						}

						// Note that even the xPixel < 0 pixel are pushed to the FIFO but they get rendered off-screen
						// Refer https://discord.com/channels/465585922579103744/465586075830845475/1281867913829158956

						newPixelFifoEntries[pixelFifoEntriesCount].color = colorIDOfCurrentPixel;
						if (ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBCompatibilityModeEnabled() == NO)
						{
							newPixelFifoEntries[pixelFifoEntriesCount].spritePriority = pGBc_display->arrayOfVisibleObjectsPerScanLine
								[pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray].indexWithinOAMMemory;
							newPixelFifoEntries[pixelFifoEntriesCount].palette = attribute.oamEntryFields.OAM_PALETTE_NUMBER_CGB;
							newPixelFifoEntries[pixelFifoEntriesCount].backgroundPriority = attribute.oamEntryFields.OAM_BG_WINDOW_OVER_OBJ;
						}
						else
						{
							newPixelFifoEntries[pixelFifoEntriesCount].spritePriority = pGBc_display->arrayOfVisibleObjectsPerScanLine
								[pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray].indexWithinOAMMemory;
							// When in DMG Compatibility Mode, OBJ palette 0 or OBJ palette 1 based on OBP0 or OBP1
							// https://gbdev.io/pandocs/Power_Up_Sequence.html#compatibility-palettes
							newPixelFifoEntries[pixelFifoEntriesCount].palette = attribute.oamEntryFields.OAM_PALETTE_NUMBER_DMG;
							newPixelFifoEntries[pixelFifoEntriesCount].backgroundPriority = attribute.oamEntryFields.OAM_BG_WINDOW_OVER_OBJ;
						}
						newPixelFifoEntries[pixelFifoEntriesCount].validity = VALID;

						pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[pixelFifoEntriesCount] = newPixelFifoEntries[pixelFifoEntriesCount];
						++pixelFifoEntriesCount;
					}

					pGBc_display->pixelFetcherContext.cachedFifo_obj.validEntries = pixelFifoEntriesCount;
					pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray = INVALID;

					pGBc_display->fetchDone = YES;
					pGBc_display->pushDone = NO;

					pGBc_display->wasFetchingOBJ = YES;
				}
				else
				{
					byte tileDataLo = pGBc_display->pixelFetcherContext.bgWinTileDataLo;

#if (GB_GBC_ENABLE_TILE_SEL_GLITCH == YES)
					FLAG isGlitched = pGBc_display->tileSelGlitchTCycles > RESET;
					FLAG useGlitched = NO;

					if (isGlitched == YES)
					{
						if (pGBc_display->cached_BG_WINDOW_TILE_DATA_AREA == SET)
						{
							useGlitched = !((FLAG)(pGBc_display->pixelFetcherContext.bgWinTileID & 0x80));
							pGBc_display->pixelFetcherContext.bgWinTileDataHi = pGBc_display->pixelFetcherContext.bgWinTileID;
						}
						else
						{
							useGlitched = YES;
							pGBc_display->pixelFetcherContext.bgWinTileDataHi = pGBc_display->tileSelGlitchedData;
						}
					}

					if (useGlitched == NO)
					{
						if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ONE)
						{
							pGBc_display->pixelFetcherContext.bgWinTileDataHi
								= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU, true);

							pGBc_display->tileSelGlitchedData = pGBc_display->pixelFetcherContext.bgWinTileDataHi;
						}
						else if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ZERO)
						{
							pGBc_display->pixelFetcherContext.bgWinTileDataHi
								= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU, false, true);

							pGBc_display->tileSelGlitchedData = pGBc_display->pixelFetcherContext.bgWinTileDataHi;
						}
						else if (ROM_TYPE == ROM::GAME_BOY)
						{
							pGBc_display->pixelFetcherContext.bgWinTileDataHi
								= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU);
						}
					}

					if (pGBc_display->cached_BG_WINDOW_TILE_DATA_AREA == SET && isGlitched == YES)
					{
						if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ONE)
						{
							pGBc_display->tileSelGlitchedData
								= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU, true);
						}
						else if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ZERO)
						{
							pGBc_display->tileSelGlitchedData
								= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU, false, true);
						}
						else if (ROM_TYPE == ROM::GAME_BOY)
						{
							FATAL("TILE_SEL glitch not applicable in DMG");
						}
					}
#else
					if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ONE)
					{
						pGBc_display->pixelFetcherContext.bgWinTileDataHi
							= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->cached_BG_TILE_VRAM_BANK_NUMBER == ZERO)
					{
						pGBc_display->pixelFetcherContext.bgWinTileDataHi
							= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU, false, true);
					}
					else if (ROM_TYPE == ROM::GAME_BOY)
					{
						pGBc_display->pixelFetcherContext.bgWinTileDataHi
							= readRawMemory(pGBc_display->addressInTileDataArea + ONE, MEMORY_ACCESS_SOURCE::PPU);
					}
#endif

					byte tileDataHi = pGBc_display->pixelFetcherContext.bgWinTileDataHi;

					auto pixelFifoEntriesCount = ZERO;
					pixelFIFOEntity_t newPixelFifoEntries[EIGHT];
					for (columnOfTheSelectedTile = ZERO; columnOfTheSelectedTile < EIGHT; columnOfTheSelectedTile++)
					{
						colorBitOfCurrentPixel = SEVEN - columnOfTheSelectedTile;
						if (ROM_TYPE == ROM::GAME_BOY_COLOR && pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_XFLIP == ONE)
						{
							colorBitOfCurrentPixel = SEVEN - colorBitOfCurrentPixel;
						}
						colorIDOfCurrentPixel = GETBIT(colorBitOfCurrentPixel, tileDataHi);
						colorIDOfCurrentPixel <<= ONE;
						colorIDOfCurrentPixel |= GETBIT(colorBitOfCurrentPixel, tileDataLo);

						newPixelFifoEntries[pixelFifoEntriesCount].spritePriority = INVALID;
						newPixelFifoEntries[pixelFifoEntriesCount].color = colorIDOfCurrentPixel;
						if (ROM_TYPE == ROM::GAME_BOY_COLOR)
						{
							if (isCGBCompatibilityModeEnabled() == YES)
							{
								// https://gbdev.io/pandocs/Power_Up_Sequence.html#compatibility-palettes
								// Use BG Palette 0
								newPixelFifoEntries[pixelFifoEntriesCount].palette = ZERO;

								// Creating a map of "tile data address" vs "palette ID" for the debugger
								mapPalette[pGBc_display->addressInTileDataArea - VRAM_START_ADDRESS] = ZERO | TYPE_BG_WIN;
								mapPalette[pGBc_display->addressInTileDataArea + ONE - VRAM_START_ADDRESS] = ZERO | TYPE_BG_WIN;
							}
							else
							{
								newPixelFifoEntries[pixelFifoEntriesCount].palette = pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_PALETTE_NUMBER;

								// Creating a map of "tile data address" vs "palette ID" for the debugger
								mapPalette[pGBc_display->addressInTileDataArea - VRAM_START_ADDRESS] = pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_PALETTE_NUMBER | TYPE_BG_WIN;
								mapPalette[pGBc_display->addressInTileDataArea + ONE - VRAM_START_ADDRESS] = pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_PALETTE_NUMBER | TYPE_BG_WIN;
							}

							newPixelFifoEntries[pixelFifoEntriesCount].backgroundPriority = pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_to_OAM_Priority;
						}
						else if (ROM_TYPE == ROM::GAME_BOY)
						{
							newPixelFifoEntries[pixelFifoEntriesCount].palette = ZERO;
							newPixelFifoEntries[pixelFifoEntriesCount].backgroundPriority = pGBc_display->pixelFetcherContext.bgAttribute.bgMapAttributesFields.BG_to_OAM_Priority;

							// Creating a map of "tile data address" vs "palette ID" for the debugger
							mapPalette[pGBc_display->addressInTileDataArea - VRAM_START_ADDRESS] = ZERO | TYPE_BG_WIN;
							mapPalette[pGBc_display->addressInTileDataArea + ONE - VRAM_START_ADDRESS] = ZERO | TYPE_BG_WIN;
						}
						newPixelFifoEntries[pixelFifoEntriesCount].validity = VALID;

						pGBc_display->pixelFetcherContext.cachedFifo_bg_win.cachedFifo[pixelFifoEntriesCount] = newPixelFifoEntries[pixelFifoEntriesCount];
						++pixelFifoEntriesCount;
					}

					pGBc_display->pixelFetcherContext.cachedFifo_bg_win.validEntries = pixelFifoEntriesCount;
					pGBc_display->pixelFetcherCounterPerScanLine += EIGHT;

					pGBc_display->fetchDone = YES;
					pGBc_display->pushDone = NO;

					pGBc_display->wasFetchingOBJ = NO;
				}
				pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::SLEEP_OR_PUSH;
				// NOTE: As per few documentation, fallthrough is needed as both tasks share T cycle
				// Also implemented in sameboy https://github.com/LIJI32/SameBoy/blob/961950044512fc159f3e79a4e725ffbf307b960d/Core/display.c#L939
				PPUTODO("Find the relevant sources which indicates tile data high fetch and first sleep cycle share T cycle");
			}
			[[fallthrough]];
			default:
			case PIXEL_FETCHER_STATES::SLEEP_OR_PUSH: // 6th and 7th cycle
			{
				// Refer to https://www.reddit.com/r/EmuDev/comments/s6cpis/gameboy_trying_to_understand_sprite_fifo_behavior/htlwkx9/?context=3
				// Also refer to https://github.com/Ashiepaws/GBEDG/blob/master/ppu/index.md#background-pixel-fetching
				// As mentioned above, there is a 6 cycle dummy fetch done at the start of every scanline
				if (pGBc_display->fakeBgFetcherRuns == ZERO && pGBc_display->wasFetchingOBJ == NO)
				{
					++pGBc_display->fakeBgFetcherRuns;
					pGBc_display->pixelFetcherCounterPerScanLine = RESET; // Reset so that we start from the first tile again post the dummy BG cycle
				}

				auto pushToPixelFifo = [&]()
					{
						FLAG pixelFIFOPushStatus = FAILURE;

						if (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == YES)
						{
							if (pGBc_display->fetchDone == YES && pGBc_display->pushDone == NO)
							{
								// If there are no other sprites in the FIFO (no overlapping)
								if (pGBc_display->objPixelFIFO.isEmpty())
								{
									pGBc_display->pushDone = pGBc_display->objPixelFIFO.push(
										pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo
										, pGBc_display->pixelFetcherContext.cachedFifo_obj.validEntries
										, EIGHT);
								}
								// If sprites were already there, then handle for drawing priority
								else
								{
									auto numberOfEntitiesBeforeCurrentOverlap = pGBc_display->objPixelFIFO.numberOfEntities;
									auto index = ZERO;

									if (ROM_TYPE == ROM::GAME_BOY_COLOR && (GETBIT(ZERO, pGBc_peripherals->OPRI) == ZERO))
									{
										while (index <= (numberOfEntitiesBeforeCurrentOverlap - ONE))
										{
											// If "this" element was invalid, then all the following elemented would be invalid as well, hence we skip the insertion in this run 
											if (pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index].validity == INVALID)
											{
												BREAK;
											}

											if ((pGBc_display->objPixelFIFO.referenceElement(index)->color == COLOR_ID_ZERO
												&& pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index].color == COLOR_ID_ZERO)
												||
												(pGBc_display->objPixelFIFO.referenceElement(index)->color != COLOR_ID_ZERO
													&& pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index].color == COLOR_ID_ZERO))
											{
												DO_NOTHING;
											}
											else if ((pGBc_display->objPixelFIFO.referenceElement(index)->color == COLOR_ID_ZERO
												&& pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index].color != COLOR_ID_ZERO)
												||
												(pGBc_display->objPixelFIFO.referenceElement(index)->spritePriority >
													pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index].spritePriority))
											{
												pGBc_display->objPixelFIFO.insertValidElementAt(index, pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index]);
											}
											++index;
										}

										while (index <= SEVEN)
										{
											// If "this" element was invalid, then all the following elemented would be invalid as well, hence we skip the insertion in this run 
											if (pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index].validity == INVALID)
											{
												BREAK;
											}

											pGBc_display->objPixelFIFO.insertValidElementAt(index, pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index]);
											++index;
										}
									}
									else
									{
										while (index <= (numberOfEntitiesBeforeCurrentOverlap - ONE))
										{
											// If "this" element was invalid, then all the following elemented would be invalid as well, hence we skip the insertion in this run 
											if (pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index].validity == INVALID)
											{
												BREAK;
											}

											if (pGBc_display->objPixelFIFO.referenceElement(index)->color == COLOR_ID_ZERO)
											{
												pGBc_display->objPixelFIFO.insertValidElementAt(index, pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index]);
											}
											++index;
										}

										while (index <= SEVEN)
										{
											// If "this" element was invalid, then all the following elements would be invalid as well, hence we skip the insertion in this run 
											if (pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index].validity == INVALID)
											{
												BREAK;
											}

											pGBc_display->objPixelFIFO.insertValidElementAt(index, pGBc_display->pixelFetcherContext.cachedFifo_obj.cachedFifo[index]);
											++index;
										}
									}

									pGBc_display->pushDone = YES;
								}

								if (pGBc_display->pushDone == YES)
								{
									// Reset the validity of the temporary pixel buffers
									pGBc_display->pixelFetcherContext.clear_cachedFifo_obj();
									pixelFIFOPushStatus = SUCCESS;
									pGBc_display->fetchDone = NO;
								}
							}
							else
							{
								FATAL("Pixel Fetcher is in an unexpected state at line %d in file %s", __LINE__, __FILE__);
							}

							pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = NO; // Object Fetch is done
							pGBc_display->isThereAnyObjectCurrentlyGettingRendered = YES; // Object Rendering needs to start...
						}
						else
						{
							// TEMP FIFO -> BG FIFO
							if (pGBc_display->tempBgWinPixelFIFO.isEmpty() == NO
								&& pGBc_display->bgWinPixelFIFO.isEmpty() == YES)
							{
								if (pGBc_display->fetchDone == YES && pGBc_display->pushDone == NO)
								{
									pGBc_display->pushDone = pGBc_display->bgWinPixelFIFO.push(
										pGBc_display->tempBgWinPixelFIFO.pEntities
										, pGBc_display->tempBgWinPixelFIFO.numberOfEntities
										, EIGHT);

									if (pGBc_display->pushDone == YES)
									{
										pGBc_display->tempBgWinPixelFIFO.clearFIFO();
										pixelFIFOPushStatus = SUCCESS;
										pGBc_display->fetchDone = NO;
									}
								}
							}
							// FETCHED -> BG FIFO
							else if (pGBc_display->bgWinPixelFIFO.isEmpty() == YES)
							{
								if (pGBc_display->fetchDone == YES && pGBc_display->pushDone == NO)
								{
									pGBc_display->pushDone = pGBc_display->bgWinPixelFIFO.push(
										pGBc_display->pixelFetcherContext.cachedFifo_bg_win.cachedFifo
										, pGBc_display->pixelFetcherContext.cachedFifo_bg_win.validEntries
										, EIGHT);

									if (pGBc_display->pushDone == YES)
									{
										// Reset the validity of the temporary pixel buffers
										pGBc_display->pixelFetcherContext.clear_cachedFifo_bg_win();
										pixelFIFOPushStatus = SUCCESS;
										pGBc_display->fetchDone = NO;
									}
								}
							}
							// If we came till here, this means that BG was not empty...
							// But since we now need to fetch the OBJ data,
							// We store the FETCHED data to TEMP FIFO and go ahead with OBJ fetch
							else if (pGBc_display->shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone == YES)
							{
								if (pGBc_display->tempBgWinPixelFIFO.isEmpty() == YES)
								{
									if (pGBc_display->fetchDone == YES && pGBc_display->pushDone == NO)
									{
										pGBc_display->pushDone = pGBc_display->tempBgWinPixelFIFO.push(
											pGBc_display->pixelFetcherContext.cachedFifo_bg_win.cachedFifo
											, pGBc_display->pixelFetcherContext.cachedFifo_bg_win.validEntries
											, EIGHT);

										if (pGBc_display->pushDone == YES)
										{
											// Reset the validity of the temporary pixel buffers
											pGBc_display->pixelFetcherContext.clear_cachedFifo_bg_win();
											pixelFIFOPushStatus = SUCCESS;
											pGBc_display->fetchDone = NO;
										}
									}
								}
							}
							// If we came till here, this means that BG FIFO is not empty and we dont have any OBJ to fetch at this point
							// So we can remain in the same state and keep attempting to push until we succed...
						}

						RETURN pixelFIFOPushStatus;
					};

				if (ENABLED)
				{
					/* BG/WINDOW HANDLING, but next would be SPRITE HANDLING */

					if (pGBc_display->shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone == YES)
					{
						pushToPixelFifo(); // At this point, we would have stored the BG/WIN data in BG FIFO or in TEMP FIFO
						pGBc_display->shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone = NO;
						pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = YES;
						pGBc_display->fetchDone = NO;
						pGBc_display->pushDone = YES;
						pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::GET_TILE; // Directly jump to GET_TILE so that MAX penalty is 5 and not 6 for interrupting a BG fetch
					}
					else
					{
						/* SPRITE HANDLING */

						// If we have to perform Object FIFO push
						if (pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == YES)
						{
							// Perform Object FIFO push
							if (pushToPixelFifo() == FAILURE)
							{
								FATAL("Object FIFO push should always succeed at line %d in file %s", __LINE__, __FILE__);
							}

							pGBc_display->wasFetchingOBJ = YES;
						}

						/* BG/WINDOW HANDLING */

						// If we just fetched a batch of new BG pixels
						if (pGBc_display->pixelFetcherContext.cachedFifo_bg_win.validEntries != RESET)
						{
							pGBc_display->fetchDone = YES;
							pGBc_display->pushDone = NO;

							// Attempt to push this new batch
							if (pushToPixelFifo())
							{
								// If push was to actual BG FIFO
								if (pGBc_display->tempBgWinPixelFIFO.isEmpty() == YES)
								{
									pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;
								}
								// If push was to temp BG FIFO
								else
								{
									pGBc_display->fetchDone = YES;
									pGBc_display->pushDone = NO;
								}
							}
							else
							{
								pGBc_display->fetchDone = YES;
								pGBc_display->pushDone = NO;
							}
						}
						// If no new BG pixels were fetched but temp BG FIFO still has data, attempt to push it
						else if (pGBc_display->tempBgWinPixelFIFO.isEmpty() == NO)
						{
							if (pushToPixelFifo())
							{
								if (pGBc_display->tempBgWinPixelFIFO.isEmpty() == YES)
								{
									pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;
								}
								else
								{
									FATAL("Temporary BG/WIN FIFO push should always succeed at line %d in file %s", __LINE__, __FILE__);
									pGBc_display->fetchDone = YES;
									pGBc_display->pushDone = NO;
								}
							}
							else
							{
								// Original BG FIFO is still not empty, so remain in same state
								pGBc_display->fetchDone = YES;
								pGBc_display->pushDone = NO;
							}
						}
						else
						{
							// Nothing else to be done here, go to next state
							pGBc_display->fetchDone = NO;
							pGBc_display->pushDone = YES;
							pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;
						}
					}
				}

				BREAK;
			}
			}
		};

	auto renderPixelsPerScanLine = [&]()
		{
			pixelFIFOEntity_t bgWinpixelToBePushed;
			pixelFIFOEntity_t objPixelToBePushed;

			// Refer to last sentence in https://www.reddit.com/r/EmuDev/comments/s6cpis/gameboy_trying_to_understand_sprite_fifo_behavior/htlwkx9/?context=3
			// Also refer to https://www.reddit.com/r/EmuDev/comments/1902c0e/comment/lkt5ae4/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
			// As per the above links, BG FIFO should have data before checking for sprite, hence (pGBc_display->bgWinPixelFIFO.isEmpty() == NO) check is valid
			if (
				// BG FIFO should have pixels to push out
				(pGBc_display->bgWinPixelFIFO.isEmpty() == NO)
				&&
				// There should not be any object that is getting fetched or needs to be fetched
				(pGBc_display->shouldFetchObjInsteadOfWinAndBgNow == NO && pGBc_display->shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone == NO)
				)
			{
				if (_DISABLE_WIN == NO)
				{
					// WY condition was triggered
					if (pGBc_display->yConditionForWindowIsMetForCurrentFrame == YES)
					{
						// If Window Layer is triggered for the pixel about to be rendered
						if ((ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBCompatibilityModeEnabled() == NO)
							||
							((ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBCompatibilityModeEnabled() == YES && pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE == SET))
							||
							(ROM_TYPE == ROM::GAME_BOY && pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE == SET))
						{
							// LCDC.WINDOW_LAYER_ENABLE is set
							if (pGBc_peripherals->LCDC.lcdControlFields.WINDOW_LAYER_ENABLE == SET)
							{
								/*
								* Check whether "X" condition for Window Layer is passing (X == WX and not X >= WX as this is just a one time trigger!)
								* Refer to "WX condition was triggered" of https://gbdev.io/pandocs/Scrolling.html#window
								* 
								* Range of pixelRenderCounterPerScanLine at this point is [-8, 159]
								* [-8, 159] + 7 = [-1, 166] is the range of WX that is allowed
								* and since WX is uint8, effective range is [0, 166]
								* 
								* Also, note that WX of 7 put the window at the left most visible pixel as per pandocs
								* this can happen only when pixelRenderCounterPerScanLine is 0 which is the first visible dot, so everything works out w.r.t this as well
								*/
								if (pGBc_display->pixelRenderCounterPerScanLine + SEVEN == pGBc_peripherals->WX)
								{
									if (pGBc_display->shouldFetchAndRenderWindowInsteadOfBG == NO)
									{
										pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = YES; //  All conditions for window is met; so we use this flag indirectly to increment the window line counter as well...
										pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;
										pGBc_display->bgWinPixelFIFO.clearFIFO();
										pGBc_display->tempBgWinPixelFIFO.clearFIFO();
										pGBc_display->fetchDone = NO;
										pGBc_display->pushDone = YES;
										pGBc_display->pixelFetcherCounterPerScanLine = pGBc_display->pixelRenderCounterPerScanLine;
										// We should increment WLY after the current scanline is renderred, 
										// hence we just set a flag here, and in the next PPU mode, we increment the WLY based on this flag
										pGBc_display->shouldIncrementWindowLineCounter = YES;
										RETURN;
									}
								}
							}
							else
							{
								// Refer https://github.com/mattcurrie/mealybug-tearoom-tests/blob/master/the-comprehensive-game-boy-ppu-documentation.md#win_en-bit-5
								// WINDOW LAYER is disabled while in middle of rendering the window
								if (pGBc_display->shouldFetchAndRenderWindowInsteadOfBG == YES || pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile == YES)
								{
									// Wait until tile boundary to reset the fetcher to fetch BG
									if ((pGBc_display->pixelRenderCounterPerScanLine & SEVEN) == ZERO)
									{
										pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
										pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = NO;
										pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;
										pGBc_display->bgWinPixelFIFO.clearFIFO();
										pGBc_display->tempBgWinPixelFIFO.clearFIFO();
										pGBc_display->fetchDone = NO;
										pGBc_display->pushDone = YES;
										pGBc_display->pixelFetcherCounterPerScanLine = pGBc_display->pixelRenderCounterPerScanLine;
										RETURN;
									}
									else
									{
										pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = YES;
									}
								}
							}
						}
						else
						{
							pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
							pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = NO;
						}
					}
					else
					{
						pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
						pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = NO;
					}
				}
				else
				{
					pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
					pGBc_display->shouldFetchAndRenderBGInsteadOfWindowAfterCurrentTile = NO;
				}

				if (_DISABLE_OBJ == NO)
				{
					// If OBJ Layer is triggered for the pixel about to be rendered, so we need to start fetching a new object
					if (
						// Object Layer is enabled
						(pGBc_peripherals->LCDC.lcdControlFields.OBJ_ENABLE == ONE)
						&&
						// There are objects that needs to be rendered in this scan line
						(pGBc_display->visibleObjectsPerScanLine != NULL)
						)
					{
						auto itt = ZERO;
						visibleObjects_t* le = pGBc_display->visibleObjectsPerScanLine;

						// Loop over the non-processed visible sprites (after selection priority is applied)

						while (le)
						{
							if (le->alreadyProcessed == YES)
							{
								le = le->next;
								++itt;
								CONTINUE;
							}

							int16_t xPosition = le->oamEntry.xPosition - EIGHT;

							// We fetch the sprite X = 0 as well, but during OBJ FIFO push, none of the pixels will get pushed to OBJ FIFO and hence, sprite remains invisible
							// Refer https://discord.com/channels/465585922579103744/465586075830845475/1440674946333278228
							if (pGBc_display->pixelRenderCounterPerScanLine == xPosition)
							{
								pGBc_display->wasX0Object = (le->oamEntry.xPosition == ZERO);
								pGBc_display->wasNotFirstSpriteInX = (pGBc_display->prevSpriteX != le->oamEntry.xPosition) ? NO : YES;
								pGBc_display->prevSpriteX = le->oamEntry.xPosition;

								// Set the "alreadyProcessed" flag for this sprite to YES

								pGBc_display->arrayOfVisibleObjectsPerScanLine[itt].alreadyProcessed = YES;
								pGBc_display->indexOfOBJToFetchFromVisibleSpritesArray = itt;

								// To handle subsequent sprites
								if (pGBc_display->pixelFetcherState == PIXEL_FETCHER_STATES::WAIT_FOR_TILE /* If pixelFetcherState is WAIT_FOR_TILE, that means FIFO push is successful */
									&& pGBc_display->wasNotFirstSpriteInX == YES) /* Not the first sprite in this X */
								{
									// 6 cycle penalty
									pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = YES;
								}
								// To handle the first sprite
								else
								{
									// 6 cycle penaltly for sprite fetch + MAX 5 cycle penatly for interrupting BG fetch
									// MAX 5 and not 6 because we jump directly to GET_TILE instead of WAIT_FOR_TILE when the below flag is set!
									pGBc_display->shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone = YES;
								}

								PPUTODO("Kind of a dirty hack (part 2) for handling timing for sprites at X = 159 at %d in %s", __LINE__, __FILE__);
								const DIM8 xPosLimit = getScreenWidth() - ONE;
								if (pGBc_display->x159SpritesPresent == YES && pGBc_display->pixelRenderCounterPerScanLine == xPosLimit) MASQ_UNLIKELY
								{
									if (--pGBc_display->nX159SpritesPresent == RESET) // If no more sprites to be processed at X=159, then we are done... we can switch to HBLANK
									{
										pGBc_display->x159SpritesDone = YES;
									}
								}

								RETURN;
							}

							le = le->next;
							++itt;
						}
					}
					else
					{
						pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = NO;
						pGBc_display->shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone = NO;
					}
				}
				else
				{
					pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = NO;
					pGBc_display->shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone = NO;
				}

				if (ENABLED)
				{
					if (pGBc_display->shouldFetchAndRenderWindowInsteadOfBG == YES)
					{
						if (pGBc_peripherals->WX < SEVEN)
						{
							if (pGBc_display->discardedPixelCount < (SEVEN - pGBc_peripherals->WX))
							{
								if (pGBc_display->bgWinPixelFIFO.pop() == SUCCESS)
								{
									++pGBc_display->discardedPixelCount;
									RETURN;
								}
								else
								{
									FATAL("bgWinPixelFIFO Window Pop Failure");
								}
							}
						}
					}
					else
					{
						if (pGBc_display->discardedPixelCount < (pGBc_display->xBGPerPixel)) // < SCX % 8
						{
							if (pGBc_display->bgWinPixelFIFO.pop() == SUCCESS)
							{
								++pGBc_display->discardedPixelCount;
								RETURN;
							}
							else
							{
								FATAL("bgWinPixelFIFO BG Pop Failure");
							}
						}
					}
				}

				bgWinpixelToBePushed.validity = INVALID;
				if (pGBc_display->bgWinPixelFIFO.pop(&bgWinpixelToBePushed) == SUCCESS || _DISABLE_BG == YES || _DISABLE_WIN == YES)
				{
					if (ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBCompatibilityModeEnabled() == NO)
					{
						FLAG didWeRenderObjectForThisPixel = NO;

						if (pGBc_display->isThereAnyObjectCurrentlyGettingRendered == YES
							&& pGBc_display->objPixelFIFO.isEmpty() == NO)
						{
							objPixelToBePushed.validity = INVALID;
							PPUTODO("Is \"pGBc_peripherals->LCDC.lcdControlFields.OBJ_ENABLE\" check needed here ?");
							if (pGBc_display->objPixelFIFO.pop(&objPixelToBePushed) == SUCCESS
								&& objPixelToBePushed.validity != INVALID
								// Is this check necessary? if yes, will the similar kind of check be applicable to window layer or bg layer
								// Checking OBJ enable again as there might be a possibility of OBJ being enabled when we detected OBJ
								// but before the rendering happens, CPU might have disabled...
								&& pGBc_peripherals->LCDC.lcdControlFields.OBJ_ENABLE == ONE)
							{
								if (
									(bgWinpixelToBePushed.color == COLOR_ID_ZERO)
									||
									(pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE == ZERO)
									||
									(bgWinpixelToBePushed.backgroundPriority == ZERO && objPixelToBePushed.backgroundPriority == ZERO)
									)
								{
									if (objPixelToBePushed.color != COLOR_ID_ZERO)
									{
										uint16_t gbcColor = pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM
											[objPixelToBePushed.palette][objPixelToBePushed.color].gbcColor;

										// update the appropriate gfx buffer

										if (pGBc_display->pixelRenderCounterPerScanLine >= ZERO)
										{
											pGBc_display->gfxVisibleColorMap_BG_WINDOW_OBJ
												[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
												= gbcColor;

											pGBc_display->gfxVisible_BG_WINDOW_OBJ
												[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
												= getColorFromColorIDForGBC(gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2);

											// update the imgui buffer

											pGBc_display->imGuiBuffer.imGuiBuffer2D
												[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
												= getColorFromColorIDForGBC(gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
										}

										didWeRenderObjectForThisPixel = YES;
									}
								}

								if (pGBc_display->objPixelFIFO.isEmpty() == YES)
								{
									pGBc_display->isThereAnyObjectCurrentlyGettingRendered = NO;
									pGBc_display->objPixelFIFO.clearFIFO();
								}
							}
						}

						if (didWeRenderObjectForThisPixel == NO && bgWinpixelToBePushed.validity != INVALID)
						{
							if (_DISABLE_BG == NO
								||
								_DISABLE_BG == YES && pGBc_display->shouldFetchAndRenderWindowInsteadOfBG == YES) // processing window layer
							{
								uint16_t gbcColor = pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM
									[bgWinpixelToBePushed.palette][bgWinpixelToBePushed.color].gbcColor;

								// update the appropriate gfx buffer

								if (pGBc_display->pixelRenderCounterPerScanLine >= ZERO)
								{
									pGBc_display->gfxVisibleColorMap_BG_WINDOW_OBJ
										[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
										= gbcColor;

									pGBc_display->gfxVisible_BG_WINDOW_OBJ
										[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
										= getColorFromColorIDForGBC(gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2);

									// update the imgui buffer

									pGBc_display->imGuiBuffer.imGuiBuffer2D
										[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
										= getColorFromColorIDForGBC(gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
								}
							}
						}
					}
					else if (ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBCompatibilityModeEnabled() == YES)
					{
						FLAG didWeRenderObjectForThisPixel = CLEAR;

						if (pGBc_display->isThereAnyObjectCurrentlyGettingRendered == YES
							&& pGBc_display->objPixelFIFO.isEmpty() == NO)
						{
							objPixelToBePushed.validity = INVALID;
							PPUTODO("Is \"pGBc_peripherals->LCDC.lcdControlFields.OBJ_ENABLE\" check needed here ?");
							if (pGBc_display->objPixelFIFO.pop(&objPixelToBePushed) == SUCCESS
								&& objPixelToBePushed.validity != INVALID
								// Is this check necessary? if yes, will the similar kind of check be applicable to window layer or bg layer
								// Checking OBJ enable again as there might be a possibility of OBJ being enabled when we detected OBJ
								// but before the rendering happens, CPU might have disabled...
								&& pGBc_peripherals->LCDC.lcdControlFields.OBJ_ENABLE == ONE)
							{
								if (objPixelToBePushed.backgroundPriority == ZERO
									||
									(objPixelToBePushed.backgroundPriority == ONE &&
										(pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE == ZERO || bgWinpixelToBePushed.color == COLOR_ID_ZERO)))
								{
									if (objPixelToBePushed.color != COLOR_ID_ZERO)
									{
										uint16_t paletteAddress = (objPixelToBePushed.palette == ONE) ? OBP1_ADDRESS : OBP0_ADDRESS;
										BYTE palette = readRawMemory(paletteAddress, MEMORY_ACCESS_SOURCE::PPU);

										BYTE colorNumber = getColorNumberFromColorIDForGB(palette, objPixelToBePushed.color);

										uint16_t gbcColor = pGBc_instance->GBc_state.entireObjectPaletteRAM.paletteRAM
											[objPixelToBePushed.palette][colorNumber].gbcColor;

										if (pGBc_display->pixelRenderCounterPerScanLine >= ZERO)
										{
											pGBc_display->gfxVisibleColorMap_BG_WINDOW_OBJ
												[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
												= gbcColor;

											pGBc_display->gfxVisible_BG_WINDOW_OBJ
												[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
												= getColorFromColorIDForGBC(gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2);

											// update the imgui buffer

											pGBc_display->imGuiBuffer.imGuiBuffer2D
												[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
												= getColorFromColorIDForGBC(gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
										}

										didWeRenderObjectForThisPixel = YES;
									}
								}

								if (pGBc_display->objPixelFIFO.isEmpty() == YES)
								{
									pGBc_display->isThereAnyObjectCurrentlyGettingRendered = NO;
									pGBc_display->objPixelFIFO.clearFIFO();
								}
							}
						}

						if (didWeRenderObjectForThisPixel == NO
							&& bgWinpixelToBePushed.validity != INVALID)
						{
							if (_DISABLE_BG == NO
								||
								_DISABLE_BG == YES && pGBc_display->shouldFetchAndRenderWindowInsteadOfBG == YES) // processing window layer
							{
								BYTE palette = readRawMemory(BGP_ADDRESS, MEMORY_ACCESS_SOURCE::PPU);
								BYTE colorNumber = getColorNumberFromColorIDForGB(palette, bgWinpixelToBePushed.color);

								uint16_t gbcColor = pGBc_instance->GBc_state.entireBackgroundPaletteRAM.paletteRAM
									[bgWinpixelToBePushed.palette][colorNumber].gbcColor;

								if (pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE == ONE)
								{
									if (pGBc_display->pixelRenderCounterPerScanLine >= ZERO)
									{
										pGBc_display->gfxVisibleColorMap_BG_WINDOW_OBJ[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
											= gbcColor;

										pGBc_display->gfxVisible_BG_WINDOW_OBJ[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
											= getColorFromColorIDForGBC(gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2);

										// update the imgui buffer

										pGBc_display->imGuiBuffer.imGuiBuffer2D[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
											= getColorFromColorIDForGBC(gbcColor, pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2).COLOR;
									}
								}
								else
								{
									// Refer : https://gbdev.io/pandocs/LCDC.html#non-cgb-mode-dmg-sgb-and-cgb-in-compatibility-mode-bg-and-window-display
								}
							}
						}
					}
					else
					{
						FLAG didWeRenderObjectForThisPixel = CLEAR;

						if (pGBc_display->isThereAnyObjectCurrentlyGettingRendered == YES
							&& pGBc_display->objPixelFIFO.isEmpty() == NO)
						{
							objPixelToBePushed.validity = INVALID;
							PPUTODO("Is \"pGBc_peripherals->LCDC.lcdControlFields.OBJ_ENABLE\" check needed here ?");
							if (pGBc_display->objPixelFIFO.pop(&objPixelToBePushed) == SUCCESS
								&& objPixelToBePushed.validity != INVALID
								// Is this check necessary? if yes, will the similar kind of check be applicable to window layer or bg layer
								// Checking OBJ enable again as there might be a possibility of OBJ being enabled when we detected OBJ
								// but before the rendering happens, CPU might have disabled...
								&& pGBc_peripherals->LCDC.lcdControlFields.OBJ_ENABLE == ONE)
							{
								if (objPixelToBePushed.backgroundPriority == ZERO
									||
									(objPixelToBePushed.backgroundPriority == ONE &&
										(pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE == ZERO || bgWinpixelToBePushed.color == COLOR_ID_ZERO)))
								{
									if (objPixelToBePushed.color != COLOR_ID_ZERO)
									{
										uint16_t paletteAddress = (objPixelToBePushed.palette == ONE) ? OBP1_ADDRESS : OBP0_ADDRESS;
										BYTE palette = readRawMemory(paletteAddress, MEMORY_ACCESS_SOURCE::PPU);

										if (pGBc_display->pixelRenderCounterPerScanLine >= ZERO)
										{
											pGBc_display->gfxVisible_BG_WINDOW_OBJ
												[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
												= getColorFromColorIDForGB(palette, objPixelToBePushed.color);

											// update the imgui buffer

											pGBc_display->imGuiBuffer.imGuiBuffer2D
												[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
												= getColorFromColorIDForGB(palette, objPixelToBePushed.color).COLOR;
										}

										didWeRenderObjectForThisPixel = YES;
									}
								}

								if (pGBc_display->objPixelFIFO.isEmpty() == YES)
								{
									pGBc_display->isThereAnyObjectCurrentlyGettingRendered = NO;
									pGBc_display->objPixelFIFO.clearFIFO();
								}
							}
						}

						if (didWeRenderObjectForThisPixel == NO
							&& bgWinpixelToBePushed.validity != INVALID)
						{
							if (_DISABLE_BG == NO
								||
								_DISABLE_BG == YES && pGBc_display->shouldFetchAndRenderWindowInsteadOfBG == YES) // processing window layer
							{
								BYTE palette = readRawMemory(BGP_ADDRESS, MEMORY_ACCESS_SOURCE::PPU);
								if (pGBc_peripherals->LCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE == ONE)
								{
									if (pGBc_display->pixelRenderCounterPerScanLine >= ZERO)
									{
										pGBc_display->gfxVisible_BG_WINDOW_OBJ[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
											= getColorFromColorIDForGB(palette, bgWinpixelToBePushed.color);

										// update the imgui buffer

										pGBc_display->imGuiBuffer.imGuiBuffer2D[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
											= getColorFromColorIDForGB(palette, bgWinpixelToBePushed.color).COLOR;
									}
								}
								else
								{
									if (pGBc_display->pixelRenderCounterPerScanLine >= ZERO)
									{
										pGBc_display->gfxVisible_BG_WINDOW_OBJ[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
											= getColorFromColorIDForGB(palette, COLOR_ID_ZERO);

										// update the imgui buffer

										pGBc_display->imGuiBuffer.imGuiBuffer2D[pGBc_peripherals->LY][pGBc_display->pixelRenderCounterPerScanLine]
											= getColorFromColorIDForGB(palette, COLOR_ID_ZERO).COLOR;
									}
								}
							}
						}
					}
				}
				else
				{
					FATAL("Pixel FIFO pop error!");
				}

				pGBc_display->pixelRenderCounterPerScanLine += ONE;
			}
		};

	while (pGBc_display->pixelPipelineDots >= ONE)
	{
		renderPixelsPerScanLine();

		runPixelFetcher();

		if (pGBc_display->pixelRenderCounterPerScanLine >= static_cast<int16_t>(getScreenWidth()))
		{
			RETURN;
		}
		else
		{
			pGBc_display->pixelPipelineDots -= ONE;
		}
	}
}

// NOTE: This function is used to dynamically change "themes"
void GBc_t::translateGFX(PALETTE_ID from, PALETTE_ID to, PALETTE_ID colorCorrectionBefore, PALETTE_ID colorCorrectionAfter)
{
	// Reset the flag so that 'DARKENING' can be done once again post theme change
	pGBc_instance->GBc_state.emulatorStatus.freezeLCD = NO;

	if (ROM_TYPE == ROM::GAME_BOY_COLOR)
	{
		if (colorCorrectionBefore != colorCorrectionAfter)
		{
			for (uint32_t y = 0; y < getScreenHeight(); y++)
			{
				for (uint32_t x = 0; x < getScreenWidth(); x++)
				{
					pGBc_display->gfxVisible_BG_WINDOW_OBJ[y][x] = getColorFromColorIDForGBC(pGBc_display->gfxVisibleColorMap_BG_WINDOW_OBJ[y][x], colorCorrectionAfter == PALETTE_ID::PALETTE_2); // Palette 2 has color correction enabled
					
					// update the imgui buffer

					pGBc_display->imGuiBuffer.imGuiBuffer2D[y][x] = getColorFromColorIDForGBC(pGBc_display->gfxVisibleColorMap_BG_WINDOW_OBJ[y][x], colorCorrectionAfter == PALETTE_ID::PALETTE_2).COLOR;
				}
			}
		}
	}
	else if (ROM_TYPE == ROM::GAME_BOY)
	{
		for (uint32_t y = 0; y < getScreenHeight(); y++)
		{
			for (uint32_t x = 0; x < getScreenWidth(); x++)
			{
				switch (pGBc_display->gfxVisible_BG_WINDOW_OBJ[y][x].COLOR_ID)
				{
				case colorID::COLOR_000P:
				{
					pGBc_display->gfxVisible_BG_WINDOW_OBJ[y][x] = paletteIDToColor.at(to).COLOR_000P;
					BREAK;
				}
				case colorID::COLOR_033P:
				{
					pGBc_display->gfxVisible_BG_WINDOW_OBJ[y][x] = paletteIDToColor.at(to).COLOR_033P;
					BREAK;
				}
				case colorID::COLOR_066P:
				{
					pGBc_display->gfxVisible_BG_WINDOW_OBJ[y][x] = paletteIDToColor.at(to).COLOR_066P;
					BREAK;
				}
				case colorID::COLOR_099P:
				{
					pGBc_display->gfxVisible_BG_WINDOW_OBJ[y][x] = paletteIDToColor.at(to).COLOR_099P;
					BREAK;
				}
				}

				// update the imgui buffer

				pGBc_display->imGuiBuffer.imGuiBuffer2D[y][x] = pGBc_display->gfxVisible_BG_WINDOW_OBJ[y][x].COLOR;
			}
		}
	}
}

void GBc_t::displayCompleteScreen()
{
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glDisable(GL_BLEND);

	// Handle for gameboy system's texture

	glBindTexture(GL_TEXTURE_2D, gameboy_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGBc_display->imGuiBuffer.imGuiBuffer1D);

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
	// 1. Upload emulator framebuffer to gameboy_texture
	GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboy_texture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
		(GLvoid*)pGBc_display->imGuiBuffer.imGuiBuffer1D));

	// Choose filtering mode (NEAREST or LINEAR)
	GLint filter = (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER) ? GL_LINEAR : GL_NEAREST;
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));

	// 2. Render gameboy_texture into framebuffer (masquerade_texture target)
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
	GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// Pass 1: Render base texture (Game Boy framebuffer)
	GL_CALL(glUseProgram(shaderProgramBasic));
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboy_texture));
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

void GBc_t::initializeGraphics()
{
	// Clear the screen
	for (uint32_t y = ZERO; y < getScreenHeight(); y++)
	{
		for (uint32_t x = ZERO; x < getScreenWidth(); x++)
		{
			pGBc_display->imGuiBuffer.imGuiBuffer2D[y][x] = BLANK;
		}
	}

	pGBc_display->windowLineCounter = RESET; // Window Line Counter = 0

	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerLY = ZERO;
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerMode = ZERO;
	pGBc_instance->GBc_state.emulatorStatus.ticks.ppuCounterPerFrame = ZERO;

	pGBc_display->wasFetchingOBJ = NO;
	pGBc_display->prevSpriteX = INVALID;
	pGBc_display->wasNotFirstSpriteInX = NO;
	pGBc_display->wasX0Object = NO;

	pGBc_display->fetchDone = NO;
	pGBc_display->pushDone = YES;

	if (debugConfig._DEBUG_PPU_VIEWER_GUI == YES)
	{
		if ((debugConfig._DEBUG_PPU_VIEWER_GUI_TRIGGER < ZERO) || (debugConfig._DEBUG_PPU_VIEWER_GUI_TRIGGER > (getScreenHeight() + VBLANK_SCANLINES - ONE)))
		{
			// By default, let debugger trigger on LY == 144 (Vblank)
			pGBc_instance->GBc_state.emulatorStatus.debugger.debuggerTriggerOnWhichLY = getScreenHeight();
		}
		else
		{
			pGBc_instance->GBc_state.emulatorStatus.debugger.debuggerTriggerOnWhichLY = debugConfig._DEBUG_PPU_VIEWER_GUI_TRIGGER;
		}
	}
}

float GBc_t::getEmulationVolume()
{
	pGBc_instance->GBc_state.audio.emulatorVolume = SDL_GetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream));
	RETURN pGBc_instance->GBc_state.audio.emulatorVolume;
}

void GBc_t::setEmulationVolume(float volume)
{
	pGBc_instance->GBc_state.audio.emulatorVolume = volume;
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), volume);
	pt.put("gb-gbc._volume", volume);
	boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, pt);
}

void GBc_t::initializeAudio()
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	const SDL_AudioSpec AudioSettings{ SDL_AUDIO_F32, TO_UINT8(AUDIO_STREAMS::TOTAL_AUDIO_STREAMS), TO_UINT(EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC) };
	audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &AudioSettings, NULL, NULL);
	SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));

	resetDivAPU(ZERO); // DIV APU = 0

	pGBc_instance->GBc_state.emulatorStatus.ticks.apuCounter = RESET; // APU ticks = 0
	pGBc_instance->GBc_state.audio.accumulatedTone = RESET;

	// From MGBA
	pGBc_peripherals->waveRam[0].waveRamByte = 0x00;
	pGBc_peripherals->waveRam[1].waveRamByte = 0xFF;
	pGBc_peripherals->waveRam[2].waveRamByte = 0x00;
	pGBc_peripherals->waveRam[3].waveRamByte = 0xFF;
	pGBc_peripherals->waveRam[4].waveRamByte = 0x00;
	pGBc_peripherals->waveRam[5].waveRamByte = 0xFF;
	pGBc_peripherals->waveRam[6].waveRamByte = 0x00;
	pGBc_peripherals->waveRam[7].waveRamByte = 0xFF;
	pGBc_peripherals->waveRam[8].waveRamByte = 0x00;
	pGBc_peripherals->waveRam[9].waveRamByte = 0xFF;
	pGBc_peripherals->waveRam[10].waveRamByte = 0x00;
	pGBc_peripherals->waveRam[11].waveRamByte = 0xFF;
	pGBc_peripherals->waveRam[12].waveRamByte = 0x00;
	pGBc_peripherals->waveRam[13].waveRamByte = 0xFF;
	pGBc_peripherals->waveRam[14].waveRamByte = 0x00;
	pGBc_peripherals->waveRam[15].waveRamByte = 0xFF;

	// Setup the volume for audio

	pGBc_instance->GBc_state.audio.emulatorVolume = pt.get<std::float_t>("gb-gbc._volume");
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), pGBc_instance->GBc_state.audio.emulatorVolume);
}

FLAG GBc_t::runEmulationAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

FLAG GBc_t::runEmulationLoopAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

FLAG GBc_t::runEmulationAtFixedRate(uint32_t currentFrame)
{
	FLAG status = true;

	pGBc_instance->GBc_state.emulatorStatus.debugger.wasDebuggerJustTriggerred = CLEAR;

	loadQuirks();

	captureIO();

	playTheAudioFrame();

	if ((pGBc_instance->GBc_state.gb_palette != currEnGbPalette)
		||
		(pGBc_instance->GBc_state.gbc_palette != currEnGbcPalette))
	{
		pGBc_instance->GBc_state.gb_palette = currEnGbPalette;
		pGBc_instance->GBc_state.gbc_palette = currEnGbcPalette;
		translateGFX(pGBc_instance->GBc_state.gb_palette, currEnGbPalette, pGBc_instance->GBc_state.gbc_palette, currEnGbcPalette);
	}

	displayCompleteScreen();

	RETURN status;
}

FLAG GBc_t::runEmulationLoopAtFixedRate(uint32_t currentFrame)
{
	pGBc_display->wasVblankJustTriggerred = CLEAR;

	if (ROM_TYPE == ROM::TEST_SST)
	{
		static FLAG SST_DEBUG_PRINT = NO;
		COUNTER32 opcode = ZERO;
		COUNTER32 subopcode = ZERO;
		FLAG cbMode = NO;
		while (FOREVER)
		{
			if (opcode > 0xFF)
			{
				INFO("Completed Running all Tom Harte sm83 (v1) tests");
				PAUSE;
			}

			if (opcode == 0xCB)
			{
				cbMode = YES;
			}

			if (cbMode == NO)
			{
				while (
					opcode == 0xD3
					|| opcode == 0xDB
					|| opcode == 0xDD
					|| opcode == 0xE3
					|| opcode == 0xE4
					|| opcode == 0xEB
					|| opcode == 0xEC
					|| opcode == 0xED
					|| opcode == 0xF4
					|| opcode == 0xFC
					|| opcode == 0xFD
					)
				{
					++opcode;
				}
			}

			// Get the input
			std::string testCaseName = std::format("{:02x}", opcode);
			if (cbMode == NO)
			{
				testCaseName = _JSON_LOCATION + "\\" + testCaseName + ".json";
				//std::string testCaseName = _JSON_LOCATION + "\\" +  "test.json";
			}
			else
			{
				std::string subtestCaseName = std::format("{:02x}", subopcode);
				testCaseName = _JSON_LOCATION + "\\" + testCaseName + " " + subtestCaseName + ".json";
				//std::string testCaseName = _JSON_LOCATION + "\\" +  "test.json";
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
				int initial_sp = initial.get<int>("sp");
				int initial_a = initial.get<int>("a");
				int initial_b = initial.get<int>("b");
				int initial_c = initial.get<int>("c");
				int initial_d = initial.get<int>("d");
				int initial_e = initial.get<int>("e");
				int initial_f = initial.get<int>("f");
				int initial_h = initial.get<int>("h");
				int initial_l = initial.get<int>("l");
				//int initial_ime = initial.get<int>("ime");
				//int initial_ie = initial.get<int>("ie");

				if (SST_DEBUG_PRINT)
				{
					std::cout << "Initial PC: " << initial_pc << ", SP: " << initial_sp
						<< ", A: " << initial_a << ", B: " << initial_b
						<< ", C: " << initial_c << ", D: " << initial_d
						<< ", E: " << initial_e << ", F: " << initial_f
						<< ", H: " << initial_h << ", L: " << initial_l
						//<< ", IME: " << initial_ime << ", IE: " << initial_ie 
						<< std::endl;
				}

				pGBc_registers->pc = initial_pc;
				pGBc_registers->sp = initial_sp;
				pGBc_registers->af.aAndFRegisters.a = initial_a;
				pGBc_registers->bc.bAndCRegisters.b = initial_b;
				pGBc_registers->bc.bAndCRegisters.c = initial_c;
				pGBc_registers->de.dAndERegisters.d = initial_d;
				pGBc_registers->de.dAndERegisters.e = initial_e;
				pGBc_registers->af.aAndFRegisters.f.flagMemory = initial_f;
				pGBc_registers->hl.hAndLRegisters.h = initial_h;
				pGBc_registers->hl.hAndLRegisters.l = initial_l;

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

					pGBc_memory->GBcRawMemory[address] = value;
				}

				// Run the CPU
				processSOC();

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
				//int final_ime = final.get<int>("ime");
				//int final_ie = final.get<int>("ie");

				if (SST_DEBUG_PRINT)
				{
					std::cout << "Final PC: " << final_pc << ", SP: " << final_sp
						<< ", A: " << final_a << ", B: " << final_b
						<< ", C: " << final_c << ", D: " << final_d
						<< ", E: " << final_e << ", F: " << final_f
						<< ", H: " << final_h << ", L: " << final_l
						//<< ", IME: " << final_ime << ", IE: " << final_ie 
						<< std::endl;
				}

				if (pGBc_registers->pc != final_pc)
				{
					WARN("PC Mismatch");
					quitThisRun = YES;
				}
				if (pGBc_registers->sp != final_sp)
				{
					WARN("SP Mismatch");
					quitThisRun = YES;
				}
				if (pGBc_registers->af.aAndFRegisters.a != final_a)
				{
					WARN("A Mismatch");
					quitThisRun = YES;
				}
				if (pGBc_registers->bc.bAndCRegisters.b != final_b)
				{
					WARN("B Mismatch");
					quitThisRun = YES;
				}
				if (pGBc_registers->bc.bAndCRegisters.c != final_c)
				{
					WARN("C Mismatch");
					quitThisRun = YES;
				}
				if (pGBc_registers->de.dAndERegisters.d != final_d)
				{
					WARN("D Mismatch");
					quitThisRun = YES;
				}
				if (pGBc_registers->de.dAndERegisters.e != final_e)
				{
					WARN("E Mismatch");
					quitThisRun = YES;
				}
				if (pGBc_registers->af.aAndFRegisters.f.flagMemory != final_f)
				{
					WARN("F Mismatch");
					quitThisRun = YES;
				}
				if (pGBc_registers->hl.hAndLRegisters.h != final_h)
				{
					WARN("H Mismatch");
					quitThisRun = YES;
				}
				if (pGBc_registers->hl.hAndLRegisters.l != final_l)
				{
					WARN("L Mismatch");
					quitThisRun = YES;
				}

				pGBc_registers->pc = RESET;
				pGBc_registers->sp = RESET;
				pGBc_registers->af.aAndFRegisters.a = RESET;
				pGBc_registers->bc.bAndCRegisters.b = RESET;
				pGBc_registers->bc.bAndCRegisters.c = RESET;
				pGBc_registers->de.dAndERegisters.d = RESET;
				pGBc_registers->de.dAndERegisters.e = RESET;
				pGBc_registers->af.aAndFRegisters.f.flagMemory = RESET;
				pGBc_registers->hl.hAndLRegisters.h = RESET;
				pGBc_registers->hl.hAndLRegisters.l = RESET;
				//pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn = CLEAR;
				//pGBc_memory->GBcMemoryMap.mInterruptEnable.interruptEnableMemory = RESET;
				pGBc_instance->GBc_state.emulatorStatus.isCPUExecutionBlocked = NO;
				pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = NO;

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

					if (pGBc_memory->GBcRawMemory[address] != value)
					{
						WARN("RAM Mismatch");
						quitThisRun = YES;
					}

					pGBc_memory->GBcRawMemory[address] = RESET;
				}

#if (ENABLED)
				// Accessing cycles
				if (SST_DEBUG_PRINT)
				{
					std::cout << "Cycles:" << std::endl;
				}
				pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.indexer = RESET;
				INC8 indexer = RESET;
				INC8 prevRWIndex = RESET;
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

					std::string temp = "---";
					if (pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].isWrite == YES)
					{
						temp = "-wm";
						prevRWIndex = indexer;
					}
					if (pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[indexer].isRead == YES)
					{
						temp = "r-m";
						prevRWIndex = indexer;
					}

					if (cycle_type.compare(temp))
					{
						WARN("Operation Cycle Mismatch");
						quitThisRun = YES;
					}

					if (cycle_address != pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[prevRWIndex].address)
					{
						WARN("Address Cycle Mismatch");
						quitThisRun = YES;
					}

					if (cycle_value != pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[prevRWIndex].data)
					{
						WARN("Data Cycle Mismatch");
						quitThisRun = YES;
					}

					++indexer;
				}

				for (INC8 ii = ZERO; ii < TWENTY; ii++)
				{
					pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[ii].reset();
				}
#else
				pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.indexer = RESET;
#endif

				if (quitThisRun == YES)
				{
					BREAK;
				}

				// Update Stats
				if (cbMode == NO)
				{
					++pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.testCount[opcode];
				}
				else
				{
					++pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cbtestCount[subopcode];
				}

#if _DEBUG
				if (pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.testCount[192] == 1)
				{
					volatile int breakpoint = 0;
				}
#endif
			}

			if (subopcode == 0xFF)
			{
				cbMode = NO;
			}

			if (cbMode == NO)
			{
				++opcode;
			}
			else
			{
				++subopcode;
			}
		}
	}
	else
	{
		processSOC();

#if DEACTIVATED
		blarggConsoleOutput();
#endif

		runDebugger();
	}

	RETURN pGBc_display->wasVblankJustTriggerred;
}

FLAG GBc_t::initializeEmulator()
{
	FLAG status = true;

	pAbsolute_GBc_instance = std::make_shared<absolute_GBc_instance_t>();

	// For readability
	pGBc_instance = (GBc_instance_t*)&(pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance);
	pGBc_registers = &(pGBc_instance->GBc_state.registers);
	pGBc_cpuInstance = &(pGBc_instance->GBc_state.cpuInstance);
	pGBc_memory = &(pGBc_instance->GBc_state.GBcMemory);
	pGBc_flags = &(pGBc_registers->af.aAndFRegisters.f.flagFields);
	pGBc_peripherals = &(pGBc_instance->GBc_state.GBcMemory.GBcMemoryMap.mIO.IOFields);
	pGBc_emuStatus = &(pAbsolute_GBc_instance->absolute_GBc_state.GBc_instance.GBc_state.emulatorStatus);
	pGBc_display = &(pGBc_instance->GBc_state.display);

	// Other initializations
	pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = NO;
	pGBc_instance->GBc_state.emulatorStatus.isHaltBugActivated = HALT_BUG_STATE::HALT_BUG_DISABLED;
	pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn = NO;
	pGBc_instance->GBc_state.emulatorStatus.eiEnState = EI_ENABLE_STATE::NOTHING_TO_BE_DONE;
	pGBc_instance->GBc_state.emulatorStatus.isDMAActive = NO;
	pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay = DMA_DELAY;
	pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred = RESET;
	pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow = INVALID;

	// Serial initialization
	initializeSerialClockSpeed();

	// Other weird initializations
	TODO("\"serialCounter = 8\" is needed by \"boot_sclk_align-dmgABCmgb.gb\"; This is a temporary fix untill we link the \"serialCounter\" to \"div\"");
	pGBc_emuStatus->ticks.serialCounter = 0x08;

	// Only the below 2 configurations are read here instead of in the constructor because "pGBc_instance" is not ready at that time...
	currEnGbPalette = configToGbPaletteID.at(pt.get<std::string>("gb-gbc._force_gb_palette"));
	pGBc_instance->GBc_state.gb_palette = currEnGbPalette;
	currEnGbcPalette = ((to_bool(pt.get<std::string>("gb-gbc._enable_cgb_color_correction")) == YES) ? PALETTE_ID::PALETTE_2 : PALETTE_ID::PALETTE_1);
	pGBc_instance->GBc_state.gbc_palette = currEnGbcPalette;

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

		glGenTextures(1, &gameboy_texture);
		glBindTexture(GL_TEXTURE_2D, gameboy_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGBc_display->imGuiBuffer.imGuiBuffer1D);
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

		// 3. Game Boy texture (used to upload emulated framebuffer)
		GL_CALL(glGenTextures(1, &gameboy_texture));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, gameboy_texture));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGBc_display->imGuiBuffer.imGuiBuffer1D));
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

void GBc_t::destroyEmulator()
{
	// save SRAM + RTC (if applicable)

	std::filesystem::path saveDirectory(_SAVE_LOCATION);
	if (!(std::filesystem::exists(saveDirectory)))
	{
		std::filesystem::create_directory(saveDirectory);
	}

	// Saving SRAM + RTC

	std::string saveFileNameForThisROM = getSaveFileName(
		pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer
		, sizeof(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer)
	);

	saveFileNameForThisROM = _SAVE_LOCATION + "\\" + saveFileNameForThisROM;

	std::cout << "\nSaving to " << saveFileNameForThisROM << std::endl;

	std::ofstream outSRAM(saveFileNameForThisROM.c_str(), std::ios_base::binary);
	uint32_t sizeOfRAMSlot = 0x2000;
	uint8_t numberOfRAMSlots = ZERO;

	if (outSRAM.fail() == false)
	{
		numberOfRAMSlots = getNumberOfRAMBanksUsed();

		for (uint32_t ii = ZERO; ii < numberOfRAMSlots; ii++)
		{
			for (uint32_t jj = ZERO; jj < sizeOfRAMSlot; jj++)
			{
				BYTE ramByte = pGBc_instance->GBc_state.entireRam.ramMemoryBanks.mRAMBanks[ii][jj];
				outSRAM.write(reinterpret_cast<const char*> (&ramByte), ONE);
			}
		}

		// Refer to https://bgb.bircd.org/rtcsave.html
		constexpr size_t RTC_DUMP_SIZE = 48;

		std::vector<uint8_t> buffer;
		buffer.reserve(RTC_DUMP_SIZE); // Optional, avoid reallocations

		// Helper to write a single byte
		auto writeByte = [&](uint8_t value) {
			buffer.push_back(value);
			};

		// Helper to write 8-byte little-endian value
		auto write64LE = [&](uint64_t value) {
			uint8_t bytes[8];
			std::memcpy(bytes, &value, 8);
			buffer.insert(buffer.end(), bytes, bytes + 8);
			};

		// --- Write RTC fields ---
		writeByte(pGBc_instance->GBc_state.rtc.rtcFields.rtc_S);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);

		writeByte(pGBc_instance->GBc_state.rtc.rtcFields.rtc_M);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);

		writeByte(pGBc_instance->GBc_state.rtc.rtcFields.rtc_H);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);

		writeByte(pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);

		writeByte(pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHMemory);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);

		// 20 bytes written

		// --- Write latched RTC fields ---
		writeByte(pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_S);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);

		writeByte(pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_M);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);

		writeByte(pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_H);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);

		writeByte(pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DL);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);

		writeByte(pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DH.rtcDHMemory);
		writeByte(RESET); writeByte(RESET); writeByte(RESET);
		
		// Now 40 bytes total

		// --- Write 8-byte Unix timestamp (little endian) ---
		uint64_t unixTS = static_cast<uint64_t>(
			std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::system_clock::now().time_since_epoch()
			).count()
			);
		write64LE(unixTS); // Now total is 48 bytes

		// Final write
		outSRAM.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());

		outSRAM.flush();
	}

	outSRAM.close();

	// reset globals

	memset(doubleInput, ZERO, static_cast<size_t>(EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC / CEIL(GB_GBC_FPS)));
	memset(doubleOutput, ZERO, static_cast<size_t>(EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC / CEIL(GB_GBC_FPS)));

	_DISABLE_BG = NO;
	_DISABLE_WIN = NO;
	_DISABLE_OBJ = NO;
	_ENABLE_DMG_BIOS = NO;
	_ENABLE_CGB_BIOS = NO;
	_ENABLE_AUDIO_HPF = NO;
	_FORCE_GB_FOR_GBC = NO;
	_FORCE_GB_GFX_FOR_GBC = NO;
	_FORCE_GBC_FOR_GB = NO;

	// deallocate memory

	pAbsolute_GBc_instance.reset();

#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glDeleteTextures(1, &gameboy_texture);
	glDeleteTextures(1, &matrix_texture);
#else
	glDeleteTextures(1, &gameboy_texture);
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

void GBc_t::randomizeRAM()
{
	// Refer for more information : https://discord.com/channels/465585922579103744/465586075830845475/1024092503340765204
	
	// mt19937 with random_device seed
	// (random_device on some platforms is low quality, but that's okay here)
	std::random_device rd;
	std::mt19937 gen(rd());

	// Base uniform distribution (0–255). We will transform it manually.
	std::uniform_int_distribution<int> dist(0, 255);

	// WRAM
	for (auto& b : pGBc_memory->GBcMemoryMap.mWorkRam.wRamMemory)
	{
		// Get a uniform 0–255
		int r = dist(gen);

		// Exponential bias AWAY from zero:
		//   Convert uniform -> exponential by squaring + scaling.
		//
		//   The formula (r*r)/255 makes small numbers extremely rare,
		//   and pushes the distribution toward high values (negative bias toward 0).
		//
		//   r = 0   -> 0
		//   r = 64  -> 16
		//   r = 128 -> 64
		//   r = 200 -> 156
		//   r = 255 -> 255
		//
		r = (r * r) / 255;

		b = static_cast<uint8_t>(r);
	}

	// VRAM
	for (auto& b : pGBc_memory->GBcMemoryMap.mVideoRam.videoRamMemory)
	{
		int r = dist(gen);
		r = (r * r) / 255;   // same exponential bias
		b = static_cast<uint8_t>(r);
	}

	// HRAM
	for (auto& b : pGBc_memory->GBcMemoryMap.mHighRam.highRamMemory)
	{
		int r = dist(gen);
		r = (r * r) / 255;   // same exponential bias
		b = static_cast<uint8_t>(r);
	}
}

FLAG GBc_t::loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom)
{
	// open the rom file

	FILE* fp = NULL;
	uint32_t totalRomSize = 0;
	uint32_t totalAuxilaryRomSize = 0;

	if (((ROM_TYPE == ROM::GAME_BOY) || (ROM_TYPE == ROM::GAME_BOY_COLOR)))
	{
		errno_t err = fopen_portable(&fp, rom[ZERO].c_str(), "rb");

		if (!err && (fp != NULL))
		{
			// get the size of the complete rom
			fseek(fp, 0, SEEK_END);
			pAbsolute_GBc_instance->absolute_GBc_state.aboutRom.codeRomSize = ftell(fp);

			// read the complete rom
			rewind(fp);
			fread(pGBc_instance->GBc_state.entireRom.entireRomMemory + 0x0000, pAbsolute_GBc_instance->absolute_GBc_state.aboutRom.codeRomSize, 1, fp);

			// load memory bank 0 (BIOS is ignored but Cartridge Header is stored)
			rewind(fp);
			fread(pGBc_memory->GBcRawMemory + 0x0000, sizeof(romBank00_t), 1, fp);

			// if bios was loaded, replace the first 256 bytes with actual bios
			if (dmg_cgb_bios.biosFound == true && dmg_cgb_bios.unMapBios == false)
			{
				// Initialize the CPU registers to zero
				cpuSetRegister(REGISTER_TYPE::RT_PC, 0x0000);
				cpuSetRegister(REGISTER_TYPE::RT_AF, 0x0000);
				cpuSetRegister(REGISTER_TYPE::RT_BC, 0x0000);
				cpuSetRegister(REGISTER_TYPE::RT_DE, 0x0000);
				cpuSetRegister(REGISTER_TYPE::RT_HL, 0x0000);
				cpuSetRegister(REGISTER_TYPE::RT_SP, 0x0000);

				// Randomize the RAM (needed by some games as this is source of entropy)
				randomizeRAM();
			}
			else
			{
				if (ROM_TYPE == ROM::GAME_BOY)
				{
					INFO("Initialize few registers and memory regions of GB-GBC before starting the emulation");

					cpuSetRegister(REGISTER_TYPE::RT_PC, 0x0100);
					cpuSetRegister(REGISTER_TYPE::RT_AF, 0x01B0);
					cpuSetRegister(REGISTER_TYPE::RT_BC, 0x0013);
					cpuSetRegister(REGISTER_TYPE::RT_DE, 0x00D8);
					cpuSetRegister(REGISTER_TYPE::RT_HL, 0x014D);
					cpuSetRegister(REGISTER_TYPE::RT_SP, 0xFFFE);

					// bypassing "writeRawMemory" and directly writing to memory as we don't want the usual memory logic to be applicable here

					// Source : GB Pandocs
#if ENABLED
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF00] = 0xCF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF01] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF02] = 0x7E;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF03] = 0xFF; // Not documented in pandocs, but mts test expect this register (DIV LSB) to be initialized to 0xFF			
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF04] = 0xAB;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF05] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF06] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF07] = 0xF8;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF08];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF09];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0E];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0F] = 0xE1;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF10] = 0x80;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF11] = 0xBF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF12] = 0xF3;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF13] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF14] = 0xBF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF15];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF16] = 0x3F;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF17] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF18] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF19] = 0xBF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1A] = 0x7F;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1B] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1C] = 0x9F;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1D] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1E] = 0xBF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1F];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF20] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF21] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF22] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF23] = 0xBF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF24] = 0x77;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF25] = 0xF3;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF26] = 0xF1;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF27];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF28];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF29];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2F];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF40] = 0x91;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF41] = 0x85;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF42] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF43] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF44] = 0x91; // Deviation from pandocs; this is needed otherwise PPU mode and LY goes out of sync
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF45] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF46] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF47] = 0xFC;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF48];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF49];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4A] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4B] = 0x00;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4C];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4D] = 0xFF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4E];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4F] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF50] = 0x01; // Deviation; this is needed as per BESS specification
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF51] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF52] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF53] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF54] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF55] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF56] = 0xFF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF57];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF58];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF59];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5F];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF60];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF61];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF62];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF63];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF64];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF65];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF66];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF67];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF68] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF69] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6A] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6B] = 0xFF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6F];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF70] = 0xFF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF71];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF78];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF79];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7F];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFFFF] = 0x00;

					// initialize wave ram
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF30];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF31];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF32];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF33];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF34];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF35];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF36];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF37];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF38];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF39];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3F];
#endif
				}
				else if (ROM_TYPE == ROM::GAME_BOY_COLOR)
				{
					INFO("Initialize few registers and memory regions of GB-GBC before starting the emulation");

					cpuSetRegister(REGISTER_TYPE::RT_PC, 0x0100);
					cpuSetRegister(REGISTER_TYPE::RT_AF, 0x1180);
					cpuSetRegister(REGISTER_TYPE::RT_BC, 0x0000);
					cpuSetRegister(REGISTER_TYPE::RT_DE, 0xFF56);
					cpuSetRegister(REGISTER_TYPE::RT_HL, 0x000D);
					cpuSetRegister(REGISTER_TYPE::RT_SP, 0xFFFE);

					// bypassing "writeRawMemory" and directly writing to memory as we don't want the usual memory logic to be applicable here

					// Source : GB Pandocs (and few from VBA-M)

#if ENABLED
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF00] = 0xC7;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF01] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF02] = 0x7F;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF03] = 0xFF; // Not documented in pandocs, but mts test expect this register (DIV LSB) to be initialized to 0xFF	
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF04];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF05] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF06] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF07] = 0xF8;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF08];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF09];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0E];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF0F] = 0xE1;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF10] = 0x80;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF11] = 0xBF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF12] = 0xF3;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF13] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF14] = 0xBF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF15];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF16] = 0x3F;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF17] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF18] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF19] = 0xBF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1A] = 0x7F;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1B] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1C] = 0x9F;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1D] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1E] = 0xBF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF1F];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF20] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF21] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF22] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF23] = 0xBF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF24] = 0x77;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF25] = 0xF3;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF26] = 0xF1;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF27];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF28];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF29];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF2F];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF40] = 0x91;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF41];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF42] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF43] = 0x00;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF44];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF45] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF46] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF47] = 0xFC;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF48];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF49];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4A] = 0x00;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4B] = 0x00;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4C];
					TODO("Enabling GBcRawMemory[0xFF4D] = 0xFF causes glitch in \"Denshe de Go!2\" (This register corresponds to KEY1; used for CGB double speed)");
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4D] = 0xFF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4E];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF4F] = 0xFF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF50];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF51] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF52] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF53] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF54] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF55] = 0xFF;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF56] = 0xFF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF57];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF58];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF59];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF5F];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF60];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF61];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF62];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF63];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF64];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF65];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF66];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF67];
					TODO("GBcRawMemory[0xFF68] = 0x80 is slight deviation from pandocs to enable auto increment of BCPD/OCPD in CGB to make \"mezase.gbc\" (Pokemon Jap Intro) work");
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF68] = 0x80;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF69];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF6F];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF70] = 0xFF;
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF71];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF78];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF79];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF7F];
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFFFF] = 0x00;

					// initialize wave ram
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF30];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF31];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF32];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF33];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF34];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF35];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF36];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF37];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF38];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF39];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3A];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3B];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3C];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3D];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3E];
					//pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0xFF3F];
#endif
				}
			}

			// Get current LCD mode and scanline
			pGBc_display->currentLCDMode = getPPULCDMode();
			pGBc_display->currentScanline = pGBc_peripherals->LY;

			// Display some of the Cartridge information
			LOG_NEW_LINE;
			LOG("Cartridge Loaded:");
			if (ROM_TYPE == ROM::GAME_BOY)
			{
				LOG(" Title    : %s", pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.title.title);
			}
			else if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				LOG(" Title    : %s", pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.title.tile_AND_cgbType_Fields.title);
			}
			LOG(" Type     : %s", cartridgeTypeName());
			uint32_t romSizeInHeader = 32 << pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.romSize;
			LOG(" ROM Size : %d KB", romSizeInHeader);
			uint32_t actualRomSize = pAbsolute_GBc_instance->absolute_GBc_state.aboutRom.codeRomSize >> 10; // divide by 1024
			if (actualRomSize > romSizeInHeader)
			{
				WARN("ROM size mentioned in header doesn't match the actual ROM size");
				pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.romSize = (BYTE)(ceil(log2(actualRomSize)) - FIVE);
				INFO(" Actual ROM Size : %d KB\n", 32 << pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.romSize);
			}
			LOG(" RAM Size : %2.2X", pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.ramSize);
			LOG(" LIC Code : %s", cartridgeLicName());
			LOG(" ROM Vers : %2.2X", pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.maskRomVersion);
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				if (pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.title.tile_AND_cgbType_Fields.cgbType == 0x80)
				{
					LOG(" CGB Type : DMG and CGB");
				}
				else if (pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.title.tile_AND_cgbType_Fields.cgbType == 0xC0)
				{
					LOG(" CGB Type : Only CGB");
				}
				else
				{
					LOG(" CGB Type : Unknown Type (Most probably a DMG rom)");
					LOG(" Try to run in DMG mode if it doesn't run properly");
				}
			}

			uint8_t checksum = 0;
			for (uint16_t address = 0x0134; address <= 0x014C; address++)
			{
				checksum = checksum - pGBc_instance->GBc_state.entireRom.entireRomMemory[address] - 1;
			}

			pGBc_instance->GBc_state.emulatorStatus.checksum = checksum;

			LOG(" Checksum : %2.2X (%s)", checksum, (checksum == pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.headerChecksum) ? "PASSED" : "FAILED");
			LOG_NEW_LINE;

			rewind(fp);

			// close the rom for now
			fclose(fp);

			// indicate the type MBC type to rest of the system
			setMBCType(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.cartridgeType);

			// indicate the ROM banking type if applicable
			setROMBankType(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.romSize);

			// indicate the RAM banking type if applicable
			setRAMBankType(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.ramSize);

			// set the GB to ROM Mode if MBC1
			if (pGBc_emuStatus->mbc1 == true)
			{
				setROMModeIfMBC1();
			}

			// disable ram banking for now
			disableRAMBank();

			// initialize the ROM Bank number to ONE (ROM BANK 0 for 0x4000 to 0x7FFF doesn't make sense) until game overrides it if necessary
			setROMBankNumber(ONE);

			// initialize the RAM Bank number to ZERO (2K external RAM) as this is basic for all MBCs
			setRAMBankNumber(ZERO);

			// initialize the VRAM Bank number to ZERO
			setVRAMBankNumber(ZERO);

			// initialize the WRAM Bank number to ONE
			setWRAMBankNumber(ONE);

			// initialize the JoyPad state
			pGBc_peripherals->P1_JOYP.joyPadMemory = 0x0F; // lower nibble set to all ones indicate, no selection and no keys being active

			pAbsolute_GBc_instance->absolute_GBc_state.aboutRom.isRomLoaded = true;

			// load SRAM + RTC (if applicable)

			std::string saveFileNameForThisROM = getSaveFileName(
				pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer
				, sizeof(pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_buffer)
			);

			saveFileNameForThisROM = _SAVE_LOCATION + "\\" + saveFileNameForThisROM;

			INFO("Attempting to load %s", saveFileNameForThisROM.c_str());

			std::ifstream inSRAM(saveFileNameForThisROM.c_str(), std::ios::in | std::ios_base::binary);

			uint32_t sizeOfRAMSlot = 0x2000;
			uint8_t numberOfRAMSlots = ZERO;

			if (inSRAM.fail() == false)
			{
				numberOfRAMSlots = getNumberOfRAMBanksUsed();

				for (uint32_t ii = ZERO; ii < numberOfRAMSlots; ii++)
				{
					for (uint32_t jj = ZERO; jj < sizeOfRAMSlot; jj++)
					{
						BYTE ramByte = ZERO;
						inSRAM.read(reinterpret_cast<char*> (&ramByte), ONE);
						pGBc_instance->GBc_state.entireRam.ramMemoryBanks.mRAMBanks[ii][jj] = ramByte;
					}
				}
			}

			BYTE cartridgeType = pGBc_memory->GBcMemoryMap.mCodeRom.codeRomFields.romBank_00.romBank00_Fields.cartridge_header.cartridge_header_fields.cartridgeType;
			if (cartridgeType == 0x0F /* MBC3+TIMER+BATTERY* / || cartridgeType == 0x10 /* MBC3+TIMER+RAM+BATTERY 2 */)
			{
				pGBc_emuStatus->isRTCAvailableInMBC3 = YES;
			}
			else
			{
				pGBc_emuStatus->isRTCAvailableInMBC3 = NO;
			}

			if (isRTCAvailableInMBC3() == YES)
			{
				// By this point, we would have read 8192 (0x2000) bytes, so the remaining bytes is RTC data

				constexpr size_t RTC_DUMP_SIZE = 48;
				std::vector<uint8_t> buffer(RTC_DUMP_SIZE);

				// Read entire RTC block from file/stream
				inSRAM.read(reinterpret_cast<char*>(buffer.data()), RTC_DUMP_SIZE);

				inSRAM.close();

				// Offset tracker
				size_t offset = 0;

				// Helper to read 1 byte from buffer
				auto readByte = [&]() -> uint8_t
					{
						RETURN buffer[offset++];
					};

				// Helper to skip 3 unused bytes
				auto skipPadding3 = [&]()
					{
						offset += 3;
					};

				// Helper to read 8-byte little-endian uint64_t
				auto read64LE = [&]() -> uint64_t
					{
						uint64_t result;
						std::memcpy(&result, &buffer[offset], 8);
						offset += 8;
						RETURN result;
					};

				// --- Restore Non-Latched RTC Fields ---
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_S = readByte(); skipPadding3();
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_M = readByte(); skipPadding3();
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_H = readByte(); skipPadding3();
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL = readByte(); skipPadding3();
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHMemory = readByte(); skipPadding3();

				// --- Restore Latched RTC Fields ---
				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_S = readByte(); skipPadding3();
				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_M = readByte(); skipPadding3();
				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_H = readByte(); skipPadding3();
				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DL = readByte(); skipPadding3();
				pGBc_instance->GBc_state.rtcLatched.rtcFields.rtc_DH.rtcDHMemory = readByte(); skipPadding3();

				// --- Read 8-byte timestamp ---
				uint64_t unixSTS = read64LE();

				// Get current time in seconds
				uint64_t unixCTS = static_cast<uint64_t>(
					std::chrono::duration_cast<std::chrono::seconds>(
						std::chrono::system_clock::now().time_since_epoch()
					).count()
					);

				// Get diff
				auto totalSeconds = unixCTS - unixSTS;

				pGBc_instance->GBc_state.rtc.rtcFields.rtc_S += totalSeconds % 60;
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_M += (totalSeconds / 60) % 60;
				pGBc_instance->GBc_state.rtc.rtcFields.rtc_H += (totalSeconds / 3600) % 24;
				auto days = totalSeconds / 86400;

				if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_S >= 60)
				{
					pGBc_instance->GBc_state.rtc.rtcFields.rtc_S -= 60;
					pGBc_instance->GBc_state.rtc.rtcFields.rtc_M++;
				}
				if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_M >= 60)
				{
					pGBc_instance->GBc_state.rtc.rtcFields.rtc_M -= 60;
					pGBc_instance->GBc_state.rtc.rtcFields.rtc_H++;
				}
				if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_H >= 24)
				{
					pGBc_instance->GBc_state.rtc.rtcFields.rtc_H -= 24;
					days++;
				}
				while (days)
				{
					++pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL;

					// If DL overflows
					if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_DL == RESET)
					{
						if (pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_MSB)
						{
							pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_MSB = RESET;
							pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_CARRY = SET;
						}
						else
						{
							pGBc_instance->GBc_state.rtc.rtcFields.rtc_DH.rtcDHFields.DAYCOUNTER_MSB = SET;
						}
					}
					--days;
				}
			}
		}
	}
	else
	{
		RETURN false;
	}

	RETURN true;
}

void GBc_t::dumpRom()
{
	uint32_t scanner = 0;
	uint32_t addressField = 0x10;

	LOG("Code ROM DUMP");
	LOG("Address\t\t");
	for (uint32_t ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}
	LOG_NEW_LINE;
	LOG_NEW_LINE;
	LOG("00000000\t");
	for (uint32_t ii = 0; ii < (int)pAbsolute_GBc_instance->absolute_GBc_state.aboutRom.codeRomSize; ii++)
	{
		LOG("0x%02x\t", pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0x0000 + ii]);
		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}
	}

	LOG_NEW_LINE;
	LOG_NEW_LINE;
	LOG_NEW_LINE;
}

FLAG GBc_t::getRomLoadedStatus()
{
	RETURN pAbsolute_GBc_instance->absolute_GBc_state.aboutRom.isRomLoaded;
}

void GBc_t::loadQuirks()
{
	if (ImGui::IsKeyPressed(ImGuiKey_C) == true)
	{
		if (ROM_TYPE == ROM::GAME_BOY_COLOR)
		{
			auto nextPaletteID = (pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_1) ? PALETTE_ID::PALETTE_2 : PALETTE_ID::PALETTE_1;
			translateGFX(pGBc_instance->GBc_state.gb_palette, pGBc_instance->GBc_state.gb_palette, pGBc_instance->GBc_state.gbc_palette, nextPaletteID);
			currEnGbcPalette = nextPaletteID;
			pGBc_instance->GBc_state.gbc_palette = nextPaletteID;
			pt.put("gb-gbc._enable_cgb_color_correction", pGBc_instance->GBc_state.gbc_palette == PALETTE_ID::PALETTE_2);
			boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, pt);
		}
	}

	if (ImGui::IsKeyPressed(ImGuiKey_T) == true)
	{
		if (ROM_TYPE == ROM::GAME_BOY)
		{
			auto nextPaletteID = pGBc_instance->GBc_state.gb_palette;
			nextPaletteID = (PALETTE_ID)((BYTE)nextPaletteID + ONE);
			if (nextPaletteID == PALETTE_ID::PALETTE_MAX)
			{
				nextPaletteID = PALETTE_ID::PALETTE_1;
			}

			translateGFX(pGBc_instance->GBc_state.gb_palette, nextPaletteID, pGBc_instance->GBc_state.gbc_palette, pGBc_instance->GBc_state.gbc_palette);
			currEnGbPalette = nextPaletteID;
			pGBc_instance->GBc_state.gb_palette = nextPaletteID;
			pt.put<std::string>("gb-gbc._force_gb_palette", gbPaletteIDToConfig.at(nextPaletteID));
			boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, pt);
		}
	}


	if (ImGui::IsKeyReleased(ImGuiKey_Q) == true)
	{
		// re-read CONFIG.ini
		try
		{
			boost::property_tree::ini_parser::read_ini(_CONFIG_LOCATION, pt);
		}
		catch (std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		}

		_ENABLE_AUDIO_HPF = to_bool(pt.get<std::string>("gb-gbc._enable_audio_hpf"));

		auto currentPaletteID = pGBc_instance->GBc_state.gb_palette;
		currEnGbPalette = configToGbPaletteID.at(pt.get<std::string>("gb-gbc._force_gb_palette"));
		pGBc_instance->GBc_state.gb_palette = currEnGbPalette;
		auto colorCorrectionCurrentStatus = pGBc_instance->GBc_state.gbc_palette;
		currEnGbcPalette = ((to_bool(pt.get<std::string>("gb-gbc._enable_cgb_color_correction")) == YES) ? PALETTE_ID::PALETTE_2 : PALETTE_ID::PALETTE_1); // Palette 2 is with color correction enabled
		pGBc_instance->GBc_state.gbc_palette = currEnGbcPalette;

		if ((pGBc_instance->GBc_state.gb_palette != currentPaletteID)
			||
			(pGBc_instance->GBc_state.gbc_palette != colorCorrectionCurrentStatus))
		{
			translateGFX(currentPaletteID, pGBc_instance->GBc_state.gb_palette, colorCorrectionCurrentStatus, pGBc_instance->GBc_state.gbc_palette);
		}

		INFO("CONFIG.ini was reloaded!");
	}
}

#if DEACTIVATED
// This is the disassembly function. Its workings are not required for emulation.
// It is merely a convenience function to turn the binary instruction code into human readable form. 
// Its included as part of the emulator because it can take advantage of many of the CPUs internal operations to do this.
std::map<uint16_t, std::string> GBc_t::disassemble(uint16_t nStart, uint16_t nStop)
{
	uint32_t addr = nStart;
	uint8_t value = 0x00, lo = 0x00, hi = 0x00;
	std::map<uint16_t, std::string> mapLines;
	uint16_t line_addr = 0;

	// A convenient utility to convert variables into
	// hex strings because "modern C++"'s method with 
	// streams is atrocious
	auto hex = [](uint32_t n, uint8_t d)
		{
			std::string s(d, '0');
			for (int i = d - 1; i >= 0; i--, n >>= 4)
				s[i] = "0123456789ABCDEF"[n & 0xF];
			RETURN s;
		};

	// Starting at the specified address we read an instruction byte, 
	// which in turn yields information from the lookup table as to how many additional bytes we need to read and what the addressing mode is. 
	// We need this info to assemble human readable syntax, which is different depending upon the addressing mode
	// As the instruction is decoded, a std::string is assembled
	// with the readable output

	while (addr <= (uint32_t)nStop)
	{
		line_addr = addr;

		// Prefix line with instruction address
		std::string sInst = "$" + hex(addr, 4) + ": ";

		// Read instruction, and get its readable name
		uint8_t opcode = readRawMemory(addr, MEMORY_ACCESS_SOURCE::DEBUG_PORT); addr++;

		// Add the formed string to a std::map, using the instruction's address as the key. 
		// This makes it convenient to look for later as the instructions are variable in length, so a straight up incremental index is not sufficient.
		mapLines[line_addr] = sInst;
	}
}
#endif

void GBc_t::dumpCPURegisters(int x, int y, FLAG dumpCPU)
{

}

void GBc_t::dumpCode(int x, int y, int nLines, FLAG dumpCode)
{

}

void GBc_t::dumpGFXData(int x1, int y1, FLAG dumpVRAM, int x2, int y2, FLAG dumpPalette, int x3, int y3, FLAG dumpOAM, int x4, int y4, FLAG dumpBG, int x5, int y5, FLAG dumpInfo, FLAG hoverCheck)
{

}

void GBc_t::dumpCartInfo(int x5, int y5, FLAG dumpCartInfo)
{

}

void GBc_t::optimizedClearScreen(FLAG shouldPerform)
{

}

void GBc_t::runDebugger()
{

}
#pragma endregion EMULATION_DEFINITIONS

#pragma region SM83_DEFINITIONS
void GBc_t::cpuSetRegister(REGISTER_TYPE rt, uint16_t u16parameter)
{
	switch (rt)
	{

		// Normal Register access
	case REGISTER_TYPE::RT_A:
	{
		pGBc_registers->af.aAndFRegisters.a = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_F:
	{
		pGBc_registers->af.aAndFRegisters.f.flagMemory = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_B:
	{
		pGBc_registers->bc.bAndCRegisters.b = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_C:
	{
		pGBc_registers->bc.bAndCRegisters.c = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_D:
	{
		pGBc_registers->de.dAndERegisters.d = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_E:
	{
		pGBc_registers->de.dAndERegisters.e = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_H:
	{
		pGBc_registers->hl.hAndLRegisters.h = u16parameter & 0x00FF; BREAK;
	}
	case REGISTER_TYPE::RT_L:
	{
		pGBc_registers->hl.hAndLRegisters.l = u16parameter & 0x00FF; BREAK;
	}

	case REGISTER_TYPE::RT_PC:
	{
		SET_PC(u16parameter & 0xFFFF); BREAK;
	}
	case REGISTER_TYPE::RT_SP:
	{
		pGBc_registers->sp = u16parameter & 0xFFFF; BREAK;
	}

	case REGISTER_TYPE::RT_AF:
	{
		pGBc_registers->af.af_u16memory = u16parameter & 0xFFFF; BREAK;
	}
	case REGISTER_TYPE::RT_BC:
	{
		pGBc_registers->bc.bc_u16memory = u16parameter & 0xFFFF; BREAK;
	}
	case REGISTER_TYPE::RT_DE:
	{
		pGBc_registers->de.de_u16memory = u16parameter & 0xFFFF; BREAK;
	}
	case REGISTER_TYPE::RT_HL:
	{
		pGBc_registers->hl.hl_u16memory = u16parameter & 0xFFFF; BREAK;
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

uint16_t GBc_t::cpuReadRegister(REGISTER_TYPE rt)
{
	switch (rt)
	{

		// Normal Register access
	case REGISTER_TYPE::RT_A:
	{
		RETURN (pGBc_registers->af.aAndFRegisters.a & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_F:
	{
		RETURN (pGBc_registers->af.aAndFRegisters.f.flagMemory & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_B:
	{
		RETURN (pGBc_registers->bc.bAndCRegisters.b & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_C:
	{
		RETURN (pGBc_registers->bc.bAndCRegisters.c & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_D:
	{
		RETURN (pGBc_registers->de.dAndERegisters.d & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_E:
	{
		RETURN (pGBc_registers->de.dAndERegisters.e & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_H:
	{
		RETURN (pGBc_registers->hl.hAndLRegisters.h & 0x00FF); BREAK;
	}
	case REGISTER_TYPE::RT_L:
	{
		RETURN (pGBc_registers->hl.hAndLRegisters.l & 0x00FF); BREAK;
	}

	case REGISTER_TYPE::RT_PC:
	{
		RETURN (GET_PC() & 0xFFFF); BREAK;
	}
	case REGISTER_TYPE::RT_SP:
	{
		RETURN (pGBc_registers->sp & 0xFFFF); BREAK;
	}

	case REGISTER_TYPE::RT_AF:
	{
		RETURN (pGBc_registers->af.af_u16memory & 0xFFFF); BREAK;
	}
	case REGISTER_TYPE::RT_BC:
	{
		RETURN (pGBc_registers->bc.bc_u16memory & 0xFFFF); BREAK;
	}
	case REGISTER_TYPE::RT_DE:
	{
		RETURN (pGBc_registers->de.de_u16memory & 0xFFFF); BREAK;
	}
	case REGISTER_TYPE::RT_HL:
	{
		RETURN (pGBc_registers->hl.hl_u16memory & 0xFFFF); BREAK;
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

void GBc_t::cpuWritePointer(POINTER_TYPE mrt, uint16_t u16parameter)
{
	switch (mrt)
	{

		// Memory pointed by Register access
	case POINTER_TYPE::RT_M_HL:
	{
		writeRawMemory(pGBc_registers->hl.hl_u16memory, u16parameter & 0x00FF, MEMORY_ACCESS_SOURCE::CPU); BREAK;
	}
	case POINTER_TYPE::RT_M_DE:
	{
		writeRawMemory(pGBc_registers->de.de_u16memory, u16parameter & 0x00FF, MEMORY_ACCESS_SOURCE::CPU); BREAK;
	}
	case POINTER_TYPE::RT_M_BC:
	{
		writeRawMemory(pGBc_registers->bc.bc_u16memory, u16parameter & 0x00FF, MEMORY_ACCESS_SOURCE::CPU); BREAK;
	}

	case POINTER_TYPE::RT_M_NONE:
	{
		BREAK;
	}
	default:
	{
		BREAK;
	}
	}
}

BYTE GBc_t::cpuReadPointer(POINTER_TYPE mrt)
{
	switch (mrt)
	{

		// Memory pointed by Register access
	case POINTER_TYPE::RT_M_HL:
	{
		RETURN (readRawMemory(pGBc_registers->hl.hl_u16memory, MEMORY_ACCESS_SOURCE::CPU) & 0x00FF); BREAK;
	}
	case POINTER_TYPE::RT_M_DE:
	{
		RETURN (readRawMemory(pGBc_registers->de.de_u16memory, MEMORY_ACCESS_SOURCE::CPU) & 0x00FF); BREAK;
	}
	case POINTER_TYPE::RT_M_BC:
	{
		RETURN (readRawMemory(pGBc_registers->bc.bc_u16memory, MEMORY_ACCESS_SOURCE::CPU) & 0x00FF); BREAK;
	}

	case POINTER_TYPE::RT_M_NONE:
	{
		RETURN (BYTE)NULL;  BREAK;
	}
	default:
	{
		RETURN (BYTE)NULL; BREAK;
	}
	}
}

byte GBc_t::readRawMemory(uint16_t address
	, MEMORY_ACCESS_SOURCE source
	, FLAG FirstPriority_readFromVRAMBank01ForCGB
	, FLAG SecondPriority_readFromVRAMBank00ForCGB)
{
	if (ROM_TYPE == ROM::TEST_SST)
	{
		auto data = pGBc_memory->GBcRawMemory[address];

		auto index = pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.indexer;
		pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].address = address;
		pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].data = data;
		pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].isRead = YES;
		pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].isWrite = NO;
		++pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.indexer;

		RETURN data;
	}
	else
	{

		// Refer : https://discord.com/channels/465585922579103744/465586075830845475/1063495906441302016
		if (ENABLED)
		{
			auto currentDMAAddr = (pGBc_instance->GBc_state.emulatorStatus.DMASource + pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred);

			// On DMG, during OAM DMA, the CPU can access only HRAM (memory at $FF80-$FFFE)
			// Refer : https://gbdev.io/pandocs/OAM_DMA_Transfer.html
			// There is a deviation in CGB behaviour w.r.t DMA conflicts, refer the below table for more information
			TODO("Find source for \"On CGB, WRAM access still conflicts if DMA source is NOT VRAM\"");
			/*
			 * DMA Bus Conflict Behavior - DMG vs CGB
			 * Reference: SameBoy memory.c is_addr_in_dma_use()
			 * https://github.com/LIJI32/SameBoy/blob/master/Core/memory.c#L134-L150
			 *
			 * +----------------------------------+-------------+-------------+
			 * | Scenario                         |     DMG     |     CGB     |
			 * +----------------------------------+-------------+-------------+
			 * | Access ROM while DMA from ROM    |  Conflict   |  Conflict   |
			 * | Access ROM while DMA from ERAM   |  Conflict   |  Conflict   |
			 * | Access ROM while DMA from WRAM   |  Conflict   | No Conflict |
			 * | Access ROM while DMA from VRAM   |  Conflict   | No Conflict |
			 * +----------------------------------+-------------+-------------+
			 * | Access ERAM while DMA from ROM   |  Conflict   |  Conflict   |
			 * | Access ERAM while DMA from ERAM  |  Conflict   |  Conflict   |
			 * | Access ERAM while DMA from WRAM  |  Conflict   | No Conflict |
			 * | Access ERAM while DMA from VRAM  |  Conflict   | No Conflict |
			 * +----------------------------------+-------------+-------------+
			 * | Access WRAM while DMA from ROM   |  Conflict   |  Conflict   |
			 * | Access WRAM while DMA from ERAM  |  Conflict   |  Conflict   |
			 * | Access WRAM while DMA from WRAM  |  Conflict   |  Conflict   |
			 * | Access WRAM while DMA from VRAM  |  Conflict   | No Conflict |
			 * +----------------------------------+-------------+-------------+
			 * | Access VRAM while DMA from ROM   |  Conflict   | No Conflict |
			 * | Access VRAM while DMA from ERAM  |  Conflict   | No Conflict |
			 * | Access VRAM while DMA from WRAM  |  Conflict   | No Conflict |
			 * | Access VRAM while DMA from VRAM  |  Conflict   |  Conflict   |
			 * +----------------------------------+-------------+-------------+
			 *
			 * DMG: Single bus - ROM/ERAM/WRAM/VRAM all share the same bus
			 *      Conflict occurs when accessing any memory while DMA uses same bus
			 *
			 * CGB: Separate buses - External (ROM/ERAM) and Internal (WRAM/VRAM)
			 *      - ROM/ERAM conflicts only with DMA from ROM/ERAM (external bus)
			 *      - WRAM conflicts with DMA from ROM/ERAM/WRAM (any non-VRAM source)
			 *      - VRAM conflicts only with DMA from VRAM (internal bus)
			 */

			if ((pGBc_instance->GBc_state.emulatorStatus.isDMAActive == YES && pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay == ZERO) && source == MEMORY_ACCESS_SOURCE::CPU)
			{
				if (ROM_TYPE == ROM::GAME_BOY)
				{
					// reading from HIRAM
					if (address >= HIRAM_START_ADDRESS && address <= HIRAM_END_ADDRESS)
					{
						RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
					}
					else if (((address >= ROM_00_START_ADDRESS && address <= ROM_NN_END_ADDRESS)
						|| (address >= EXTERNAL_RAM_START_ADDRESS && address <= EXTERNAL_RAM_END_ADDRESS)
						|| (address >= WORK_RAM_00_START_ADDRESS && address <= ECHO_RAM_END_ADDRESS))
						&& ((currentDMAAddr >= ROM_00_START_ADDRESS && currentDMAAddr <= ROM_NN_END_ADDRESS)
							|| (currentDMAAddr >= EXTERNAL_RAM_START_ADDRESS && currentDMAAddr <= EXTERNAL_RAM_END_ADDRESS)
							|| (currentDMAAddr >= WORK_RAM_00_START_ADDRESS && currentDMAAddr <= ECHO_RAM_END_ADDRESS)))
					{
						WARN("Invalid access by DMG CPU to ROM/WRAM/ERAM when DMA is active");
						RETURN readRawMemory(currentDMAAddr, MEMORY_ACCESS_SOURCE::OAMDMA);
					}
					else if ((address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
						&& (currentDMAAddr >= VRAM_START_ADDRESS && currentDMAAddr <= VRAM_END_ADDRESS))
					{
						WARN("Invalid access by DMG CPU to VRAM when DMA is active");
						RETURN readRawMemory(currentDMAAddr, MEMORY_ACCESS_SOURCE::OAMDMA);
					}
					else if (address >= OAM_START_ADDRESS && address <= OAM_END_ADDRESS)
					{
						RETURN 0xFF;
					}
				}
				else
				{
					// reading from HIRAM
					if (address >= HIRAM_START_ADDRESS && address <= HIRAM_END_ADDRESS)
					{
						RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
					}
					else if (((address >= ROM_00_START_ADDRESS && address <= ROM_NN_END_ADDRESS)
						|| (address >= EXTERNAL_RAM_START_ADDRESS && address <= EXTERNAL_RAM_END_ADDRESS))
						&& ((currentDMAAddr >= ROM_00_START_ADDRESS && currentDMAAddr <= ROM_NN_END_ADDRESS)
							|| (currentDMAAddr >= EXTERNAL_RAM_START_ADDRESS && currentDMAAddr <= EXTERNAL_RAM_END_ADDRESS)))
					{
						WARN("Invalid access by CGB CPU to ROM/ERAM when DMA is active");
						RETURN readRawMemory(currentDMAAddr, MEMORY_ACCESS_SOURCE::OAMDMA);
					}
					// On CGB, WRAM access conflicts if DMA source is NOT VRAM
					// Reference: SameBoy memory.c is_addr_in_dma_use() line 142-144
					// https://github.com/LIJI32/SameBoy/blob/master/Core/memory.c#L142-L144
					else if ((address >= WORK_RAM_00_START_ADDRESS && address <= ECHO_RAM_END_ADDRESS)
						&& ((currentDMAAddr >= ROM_00_START_ADDRESS && currentDMAAddr <= ROM_NN_END_ADDRESS)
							|| (currentDMAAddr >= EXTERNAL_RAM_START_ADDRESS && currentDMAAddr <= EXTERNAL_RAM_END_ADDRESS)
							|| (currentDMAAddr >= WORK_RAM_00_START_ADDRESS && currentDMAAddr <= ECHO_RAM_END_ADDRESS)))
					{
						WARN("Invalid access by CGB CPU to WRAM when DMA is active");
						RETURN readRawMemory(currentDMAAddr, MEMORY_ACCESS_SOURCE::OAMDMA);
					}
					else if ((address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
						&& (currentDMAAddr >= VRAM_START_ADDRESS && currentDMAAddr <= VRAM_END_ADDRESS))
					{
						WARN("Invalid access by CGB CPU to VRAM when DMA is active");
						RETURN readRawMemory(currentDMAAddr, MEMORY_ACCESS_SOURCE::OAMDMA);
					}
					else if (address >= OAM_START_ADDRESS && address <= OAM_END_ADDRESS)
					{
						RETURN 0xFF;
					}
				}
			}
		}

		// reading from dmg/cgb bios if it was loaded
		if (dmg_cgb_bios.biosFound == true && dmg_cgb_bios.unMapBios == false)
		{
			// https://gbdev.io/pandocs/Power_Up_Sequence.html
			if (
				((ROM_TYPE == ROM::GAME_BOY) && (address >= DMG_BIOS_START_ADDRESS && address < DMG_BIOS_END_ADDRESS))
				||
				((ROM_TYPE == ROM::GAME_BOY_COLOR)
					&& ((address >= CGB_BIOS_START_ADDRESS_PART1 && address < CGB_BIOS_END_ADDRESS_PART1)
						||
						(address >= CGB_BIOS_START_ADDRESS_PART2 && address < CGB_BIOS_END_ADDRESS_PART2)
						))
				)
			{
				RETURN dmg_cgb_bios.biosImage[address];
			}
		}

		int16_t modedData = RESET;
		int16_t other1 = RESET;
		uint32_t index = RESET;

		// reading from ROM
		if (address >= ROM_00_START_ADDRESS && address <= ROM_00_END_ADDRESS)
		{
			auto ROMBankNumber = ZERO;
			if (pGBc_emuStatus->mbc1 == YES
				&& getROMOrRAMModeInMBC1() == MBC1_RAM_MODE)
			{
				ROMBankNumber = (pGBc_emuStatus->currentROMBankNumber.mbc1Fields.mbcBank2Reg << FIVE);
				ROMBankNumber &= (getNumberOfROMBanksUsed() - ONE);
			}

			if ((ceGBGBC->interceptCPURead(address, &modedData, &other1))
				&&
				((BYTE)other1 == pGBc_instance->GBc_state.entireRom.romMemoryBanks.mROMBanks[ROMBankNumber][address]))
			{
				RETURN TO_UINT8(modedData);
			}
			else
			{
				RETURN pGBc_instance->GBc_state.entireRom.romMemoryBanks.mROMBanks[ROMBankNumber][address];
			}
		}

		// reading from banked ROM
		if (address >= ROM_NN_START_ADDRESS && address <= ROM_NN_END_ADDRESS)
		{
			auto originalAddress = address;
			address -= 0x4000;

			if ((ceGBGBC->interceptCPURead(originalAddress, &modedData, &other1))
				&&
				((BYTE)other1 == pGBc_instance->GBc_state.entireRom.romMemoryBanks.mROMBanks[getROMBankNumber()][address]))
			{
				RETURN TO_UINT8(modedData);
			}
			else
			{
				RETURN pGBc_instance->GBc_state.entireRom.romMemoryBanks.mROMBanks[getROMBankNumber()][address];
			}
		}

		// reading from VRAM
		if (address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
		{
			if (pGBc_display->blockVramR == YES && source == MEMORY_ACCESS_SOURCE::CPU)
			{
				RETURN 0xFF;
			}

			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				address -= 0x8000;
				if (address < 0x2000)
				{
					if (FirstPriority_readFromVRAMBank01ForCGB == true)
					{
						RETURN pGBc_instance->GBc_state.entireVram.vramMemoryBanks.mVRAMBanks[ONE][address];
					}
					else if (SecondPriority_readFromVRAMBank00ForCGB == true)
					{
						RETURN pGBc_instance->GBc_state.entireVram.vramMemoryBanks.mVRAMBanks[ZERO][address];
					}
					else
					{
						RETURN pGBc_instance->GBc_state.entireVram.vramMemoryBanks.mVRAMBanks[getVRAMBankNumber()][address];
					}
				}
				else
				{
					FATAL("VRAM buffer overflow");
					PAUSE;
				}
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
			}
		}

		// reading from banked external RAM
		if (address >= EXTERNAL_RAM_START_ADDRESS && address <= EXTERNAL_RAM_END_ADDRESS)
		{
			auto originalAddress = address;

			if (pGBc_emuStatus->mbc3 == YES)
			{
				if (isRTCMappedToExternalRAM() == YES)
				{
					if (readFromRTCRegisterIfApplicable() != INVALID)
					{
						RETURN (BYTE)readFromRTCRegisterIfApplicable();
					}
					else
					{
						FATAL("RTC Invalid Read");
					}
				}
			}

			address -= 0xA000;

			// Refer 13.3 of https://gekkio.fi/files/gb-docs/gbctr.pdf
			if (pGBc_emuStatus->mbc2 == YES)
			{
				address &= (0x200 - ONE);
			}

			if (isRAMBankEnabled() == YES)
			{
				if ((ceGBGBC->interceptCPURead(originalAddress, &modedData, &other1))
					// NOTE: Below check is not needed I guess as only option is "1" or "8" to differentiate bank; Below bank number check is sufficient
					//&&
					//((BYTE)(other1 >> FOUR) == EIGHT)
					&&
					((BYTE)(other1 & 0x0F) == (BYTE)(getRAMBankNumber())))
				{
					RETURN TO_UINT8(modedData);
				}
				else
				{
					RETURN pGBc_instance->GBc_state.entireRam.ramMemoryBanks.mRAMBanks[getRAMBankNumber()][address];
				}
			}
			else
			{
				WARN("RAM Banking is not enabled!");
				RETURN 0xFF;
			}
		}

		// reading from work RAM
		if (address >= WORK_RAM_00_START_ADDRESS && address <= WORK_RAM_01_END_ADDRESS)
		{
			auto originalAddress = address;

			// WRAM 00
			if (address <= WORK_RAM_00_END_ADDRESS)
			{
				if (ceGBGBC->interceptCPURead(originalAddress, &modedData, &other1))
				{
					RETURN TO_UINT8(modedData);
				}
				else
				{
					RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
				}

				RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
			}
			// WRAM 01
			else
			{
				if (ROM_TYPE == ROM::GAME_BOY_COLOR)
				{
					address -= 0xD000;
					if (address < 0x1000)
					{
						// NOTE: "mWRAM01Banks" goes from 0 - 6, whereas "getWRAMBankNumber()" goes from 1 - 7
						// "getWRAMBankNumber()" is set by the game rom and hence, no tweaking is allowed inside the function
						// Hence, tweaking is possible only at the interfacing between mWRAM01Banks and "getWRAMBankNumber()"
						// Therefore, getWRAMBankNumber() is decremented once when used within mWRAM01Banks

						if ((ceGBGBC->interceptCPURead(originalAddress, &modedData, &other1))
							// NOTE: Below check is not needed I guess as only option is "1" or "9" to differentiate bank; Below bank number check is sufficient
							//&&
							//((BYTE)(other1 >> FOUR) == NINE)
							&&
							((BYTE)(other1 & 0x0F) == (BYTE)(getWRAMBankNumber())))
						{
							RETURN TO_UINT8(modedData);
						}
						else
						{
							RETURN pGBc_instance->GBc_state.entireWram01.wram01MemoryBanks.mWRAM01Banks[getWRAMBankNumber() - ONE][address];
						}
					}
					else
					{
						FATAL("WRAM buffer overflow");
					}
				}
				else if (ROM_TYPE == ROM::GAME_BOY)
				{
					if (ceGBGBC->interceptCPURead(originalAddress, &modedData, &other1))
					{
						RETURN TO_UINT8(modedData);
					}
					else
					{
						RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
					}
				}
			}
		}

		// reading from echo RAM
		if (address >= ECHO_RAM_START_ADDRESS && address <= ECHO_RAM_END_ADDRESS)
		{
			RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address - 0x2000];
		}

		// reading from OAM memory
		if (address >= OAM_START_ADDRESS && address <= OAM_END_ADDRESS)
		{
			if (pGBc_display->blockOAMR == YES && source == MEMORY_ACCESS_SOURCE::CPU)
			{
				RETURN 0xFF;
			}

			// Refer : https://forums.nesdev.org/viewtopic.php?t=14293
			// Refer : https://github.com/Gekkio/mooneye-test-suite/blob/main/acceptance/oam_dma_start.s
			if (((getPPULCDMode() == LCD_MODES::MODE_LCD_DISPLAY_PIXELS || getPPULCDMode() == LCD_MODES::MODE_LCD_SEARCHING_OAM) && source == MEMORY_ACCESS_SOURCE::CPU)
				|| (pGBc_instance->GBc_state.emulatorStatus.isDMAActive == YES && pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay == ZERO))
			{
				RETURN 0xFF;
			}

			RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
		}

		// reading from restricted memory returns 0
		if (address >= RESTRICTED_MEMORY_START_ADDRESS && address <= RESTRICTED_MEMORY_END_ADDRESS)
		{
			++pGBc_instance->GBc_state.emulatorStatus.unusableMemoryReads;

			// Refer to https://discord.com/channels/465585922579103744/465586075830845475/1331318659213430895
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				// Note that which.gbc uses this to distinguish b/w rev C and rev E
				
				// Extract high nibble of lower byte and duplicate it
				BYTE highNibble = (address >> 4) & 0x0F;
				RETURN (highNibble << 4) | highNibble;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				// Needed by docboy's not_usable_read_* tests
				if (isPPULCDEnabled() == NO || getPPULCDMode() == LCD_MODES::MODE_LCD_V_BLANK || getPPULCDMode() == LCD_MODES::MODE_LCD_H_BLANK)
				{
					RETURN ZERO;
				}
				else
				{
					RETURN 0xFF;
				}
			}
		}

		// reading from Joypad register; bits 7 and 6 are unused
		if (address == P1_JOYP_ADDRESS)
		{
			pGBc_peripherals->P1_JOYP.joyPadFields.JP_SPARE_06 = ONE;
			pGBc_peripherals->P1_JOYP.joyPadFields.JP_SPARE_07 = ONE;

			auto joyp = pGBc_peripherals->P1_JOYP;

			if ((joyp.joyPadFields.P14_SEL_DIRECTION_KEYS == RESET)
				&& (joyp.joyPadFields.P15_SEL_ACTION_KEYS == RESET))
			{
				joyp.joyPadFields.P10_RIGHT_A = ONE;
				joyp.joyPadFields.P11_LEFT_B = ONE;
				joyp.joyPadFields.P12_UP_SELECT = ONE;
				joyp.joyPadFields.P13_DOWN_START = ONE;
			}

			RETURN joyp.joyPadMemory;
		}

		// reading from SC register
		if (address == SC_ADDRESS)
		{
			auto SC = pGBc_peripherals->SC;
			if (ROM_TYPE == ROM::GAME_BOY)
			{
				SC.scFields.CLOCK_SPEED = SET;
			}
			SC.scFields.RESERVED = 0x1F;

			RETURN SC.scMemory;
		}

		// reading DIV LSB returns 0xFF as its not mapped
		if (address == DIV_ADDRESS_LSB)
		{
			RETURN 0xFF;
		}

		// reading from TAC; only 0 - 2 bits are valid 
		if (address == TAC_ADDRESS)
		{
			pGBc_peripherals->TAC.timerControlFields.TAC_3 = ONE;
			pGBc_peripherals->TAC.timerControlFields.TAC_4 = ONE;
			pGBc_peripherals->TAC.timerControlFields.TAC_5 = ONE;
			pGBc_peripherals->TAC.timerControlFields.TAC_6 = ONE;
			pGBc_peripherals->TAC.timerControlFields.TAC_7 = ONE;

			RETURN pGBc_peripherals->TAC.timerControlMemory;
		}

		if ((address >= 0xFF08) && (address <= 0xFF0E))
		{
			RETURN 0xFF;
		}

		// reading from IF; only 0 - 4 bits are valid 
		if (address == IF_ADDRESS)
		{
			pGBc_peripherals->IF.interruptRequestFields.NO_INT05 = ONE;
			pGBc_peripherals->IF.interruptRequestFields.NO_INT06 = ONE;
			pGBc_peripherals->IF.interruptRequestFields.NO_INT07 = ONE;

			RETURN pGBc_peripherals->IF.interruptRequestMemory;
		}

		// reading from Sound channel 1 sweep
		if (address == NR10_ADDRESS)
		{
			RETURN pGBc_peripherals->NR10.channelSweepMemory | 0x80;
		}

		// reading from Sound channel 1 length timer & duty cycle
		if (address == NR11_ADDRESS)
		{
			RETURN pGBc_peripherals->NR11.channelLengthAndDutyMemory | 0x3F;
		}

		// reading from Sound channel 1 volume & envelope
		if (address == NR12_ADDRESS)
		{
			RETURN pGBc_peripherals->NR12.channelVolumeAndEnvelopeMemory;
		}

		// reading from Sound channel 1 period low
		if (address == NR13_ADDRESS)
		{
			RETURN 0xFF;
		}

		// reading from Sound channel 1 period high & control
		if (address == NR14_ADDRESS)
		{
			RETURN pGBc_peripherals->NR14.channelHigherPeriodAndControlMemory | 0xBF;
		}

		// reading from undefined sound register
		if (address == 0xFF15)
		{
			RETURN 0xFF;
		}

		// reading from Sound channel 2 length timer & duty cycle
		if (address == NR21_ADDRESS)
		{
			RETURN pGBc_peripherals->NR21.channelLengthAndDutyMemory | 0x3F;
		}

		// reading from Sound channel 2 volume & envelope
		if (address == NR22_ADDRESS)
		{
			RETURN pGBc_peripherals->NR22.channelVolumeAndEnvelopeMemory;
		}

		// reading from Sound channel 2 period low
		if (address == NR23_ADDRESS)
		{
			RETURN 0xFF;
		}

		// reading from Sound channel 2 period high & control
		if (address == NR24_ADDRESS)
		{
			RETURN pGBc_peripherals->NR24.channelHigherPeriodAndControlMemory | 0xBF;
		}

		// reading from Sound channel 3 DAC enable
		if (address == NR30_ADDRESS)
		{
			RETURN pGBc_peripherals->NR30.channelDACEnableMemory | 0x7F;
		}

		// reading from Sound channel 3 length timer
		if (address == NR31_ADDRESS)
		{
			RETURN 0xFF;
		}

		// Sound channel 3 output level
		if (address == NR32_ADDRESS)
		{
			RETURN pGBc_peripherals->NR32.channelOutputLevelMemory | 0x9F;
		}

		// Sound channel 3 period low
		if (address == NR33_ADDRESS)
		{
			RETURN 0xFF;
		}

		// reading from Sound channel 3 period high & control
		if (address == NR34_ADDRESS)
		{
			RETURN pGBc_peripherals->NR34.channelHigherPeriodAndControlMemory | 0xBF;
		}

		// reading from undefined sound register
		if (address == 0xFF1F)
		{
			RETURN 0xFF;
		}

		// reading from Sound channel 4 length timer
		if (address == NR41_ADDRESS)
		{
			RETURN 0xFF;
		}

		// reading from Sound channel 4 volume & envelope
		if (address == NR42_ADDRESS)
		{
			RETURN pGBc_peripherals->NR42.channelVolumeAndEnvelopeMemory;
		}

		// Sound channel 4 frequency & randomness
		if (address == NR43_ADDRESS)
		{
			RETURN pGBc_peripherals->NR43.channelFrequencyAndRandomnessMemory;
		}

		// reading from Sound channel 4 control
		if (address == NR44_ADDRESS)
		{
			RETURN pGBc_peripherals->NR44.channelHigherPeriodAndControlMemory | 0xBF;
		}

		// reading from Master volume & VIN panning
		if (address == NR50_ADDRESS)
		{
			RETURN pGBc_peripherals->NR50.channelMasterVolumeAndVINPanningMemory;
		}

		// reading from Sound panning
		if (address == NR51_ADDRESS)
		{
			RETURN pGBc_peripherals->NR51.channelSoundPanningMemory;
		}

		// reading from Sound ON/OFF
		if (address == NR52_ADDRESS)
		{
			RETURN pGBc_peripherals->NR52.channelSoundONOFFMemory | 0x70;
		}

		// reading from undefined sound registers
		if (address > NR52_ADDRESS && address < WAVE_RAM_START_ADDRESS)
		{
			RETURN 0xFF;
		}

		// reading from Wave RAM
		if (address >= WAVE_RAM_START_ADDRESS && address <= WAVE_RAM_END_ADDRESS)
		{
			if (isChannel3Active() == YES)
			{
				// Wave RAM buffer is structured as follows
				//	ByteIndex	Higher Nibble		Lower Nibble
				//	0			0					1
				//	1			2					3
				//	2			4					5
				//	3			6					7
				//	4			8					9
				//	5			10					11
				//	6			12					13
				//	7			14					15
				//	8			16					17	
				//	9			18					19		
				// 	10			20					21
				// 	11			22					23
				// 	12			24					25
				// 	13			26					27
				// 	14			28					30
				// 	15			31					32
				// now, lets assume that waveRamIndex is 14
				// this indicates a byteIndex of 7
				// from waveRamIndex, we can get byteIndex via (uint8_t)(waveRamIndex / 2)

				if (ROM_TYPE == ROM::GAME_BOY)
				{
					if ((pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].frequencyTimer - TWO) < TWO)
					{
						// Refer to last section of the comment in https://forums.nesdev.org/viewtopic.php?p=162635&sid=a3e7beafbdae24ac094cba6b39a3d8ee#p162635
						uint8_t waveRamIndex = (pGBc_instance->GBc_state.audio.waveRamCurrentIndex + ONE) & THIRTYONE; // This goes from 0 - 31 (as it access nibble rather than byte
						RETURN pGBc_peripherals->waveRam[(uint8_t)(waveRamIndex / TWO)].waveRamByte;
					}
					else
					{
						RETURN 0xFF;
					}
				}
				else if (ROM_TYPE == ROM::GAME_BOY_COLOR)
				{
					uint8_t waveRamIndex = pGBc_instance->GBc_state.audio.waveRamCurrentIndex; // This goes from 0 - 31 (as it access nibble rather than byte)
					RETURN pGBc_peripherals->waveRam[(uint8_t)(waveRamIndex / TWO)].waveRamByte;
				}
			}
			else
			{
				RETURN pGBc_peripherals->waveRam[(address - WAVE_RAM_START_ADDRESS)].waveRamByte;
			}
		}

		// reading from STAT; only 0 - 6 bits are valid 
		if (address == STAT_ADDRESS)
		{
			pGBc_peripherals->STAT.lcdStatusFields.UNUSED_07 = ONE;
			RETURN pGBc_peripherals->STAT.lcdStatusMemory;
		}

		// reading LY; only 0 - 6 bits are valid
		if (address == LY_ADDRESS)
		{
			RETURN (isPPULCDEnabled()) ? pGBc_peripherals->LY : ZERO;
		}

		// reading from KEY0
		if (address == KEY0_ADDRESS)
		{
			CPUTODO("Is the unmap bios logic needed for KEY0 read?");
#if (DEACTIVATED)
			// Required by "unused_hwio-C.gb"
			if (dmg_cgb_bios.unMapBios == YES) MASQ_LIKELY
			{
				RETURN 0xFF;
			}
#endif

			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				pGBc_peripherals->KEY0.KEY0Fields.Reserved0 = 0x03;
				pGBc_peripherals->KEY0.KEY0Fields.Reserved1 = 0x1F;
				RETURN pGBc_peripherals->KEY0.KEY0Memory;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		// reading from KEY1
		if (address == KEY1_ADDRESS)
		{
			CPUTODO("Is the unmap bios logic needed for KEY1 read?");
#if (DEACTIVATED)
			// Required by "unused_hwio-C.gb"
			if (dmg_cgb_bios.unMapBios == YES) MASQ_LIKELY
			{
				RETURN 0xFF;
			}
#endif

			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				pGBc_peripherals->KEY1.KEY1Fields.Reserved = 0x3F;
				RETURN pGBc_peripherals->KEY1.KEY1Memory;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if (address == 0xFF4E)
		{
			RETURN 0xFF;
		}

		// reading from VRAM BANK switch register; only bit 0 is valid
		if (address == VRAM_BANK_SWITCH)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				uint8_t data = pGBc_peripherals->VBK & 0x01;
				data |= 0xFE;
				RETURN data;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if (address == BANK_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				RETURN 0xFE | ((dmg_cgb_bios.unMapBios == YES) ? 0x01 : 0x00);
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		// reading the HDMA source "high" address
		if (address == HDMA1_SOURCE_HIGH_REGISTER_ADDRESS)
		{
			RETURN 0xFF;
		}

		// reading the HDMA source "low" address
		if (address == HDMA2_SOURCE_LOW_REGISTER_ADDRESS)
		{
			RETURN 0xFF;
		}

		// reading the HDMA destination "high" address
		if (address == HDMA3_DEST_HIGH_REGISTER_ADDRESS)
		{
			RETURN 0xFF;
		}

		// reading the HDMA destination "low" address
		if (address == HDMA4_DEST_LOW_REGISTER_ADDRESS)
		{
			RETURN 0xFF;
		}

		// reading the HDMA destination "low" address
		if (address == HDMA5_CONFIG_REGISTER_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				auto hdma5 = pGBc_peripherals->HDMA5;
				if (pGBc_instance->GBc_state.emulatorStatus.isGPDMAActive == YES
					|| pGBc_instance->GBc_state.emulatorStatus.isHDMAActive == YES)
				{
					hdma5 &= ~0x80;
				}
				else
				{
					hdma5 |= 0x80;
				}
				RETURN hdma5;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		// reading from IR Port
		if (address == IR_PORT_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				RETURN pGBc_peripherals->RP;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if ((address >= 0xFF57) && (address <= 0xFF67))
		{
			RETURN 0xFF;
		}

		// reading from BCPS
		if (address == BCPS_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				pGBc_peripherals->BCPS_BGPI.BCPSFields.SPARE_06 = SET;
				RETURN pGBc_peripherals->BCPS_BGPI.BCPSMemory;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		// RETURN from BCPD
		if (address == BCPD_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				if (pGBc_display->blockCGBPalette == YES
					&& source == MEMORY_ACCESS_SOURCE::CPU)
				{
					RETURN 0xFF;
				}

				RETURN pGBc_peripherals->BCPD_BGPD;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		// reading from OCPS
		if (address == OCPS_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				pGBc_peripherals->OCPS_OBPI.OCPSFields.SPARE_06 = SET;
				RETURN pGBc_peripherals->OCPS_OBPI.OCPSMemory;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		// reading from OCPD
		if (address == OCPD_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				if (pGBc_display->blockCGBPalette == YES
					&& source == MEMORY_ACCESS_SOURCE::CPU)
				{
					RETURN 0xFF;
				}

				RETURN pGBc_peripherals->OCPD_OBPD;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		// reading from object priority mode register
		if (address == OBJECT_PRIORITY_MODE)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				RETURN pGBc_peripherals->OPRI | 0xFE;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if ((address >= 0xFF6D) && (address <= 0xFF6F))
		{
			RETURN 0xFF;
		}

		// reading from WRAM BANK switch register; only bits 0 - 3 are valid
		if (address == WRAM_BANK_SWITCH)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				BYTE data = pGBc_peripherals->SVBK & 0x07;
				RETURN data;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if (address == 0xFF71)
		{
			RETURN 0xFF;
		}

		if (address == 0xFF72)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if (address == 0xFF73)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if (address == 0xFF74)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if (address == 0xFF75)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] | 0x8F;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if (address == PCM12_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				BYTE ch1_pcm = RESET;
				BYTE ch2_pcm = RESET;

				// Channel 1 PCM (square wave)
				if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == SET 
				    && pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled == YES
					&& isDACEnabled(AUDIO_CHANNELS::CHANNEL_1))
				{
					ch1_pcm = getLogicalAmplitude(AUDIO_CHANNELS::CHANNEL_1)
						* pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume;
				}

				// Channel 2 PCM (square wave)
				if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == SET 
				    && pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled == YES
					&& isDACEnabled(AUDIO_CHANNELS::CHANNEL_2))
				{
					ch2_pcm = getLogicalAmplitude(AUDIO_CHANNELS::CHANNEL_2)
						* pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume;
				}

				RETURN (ch2_pcm << FOUR) | (ch1_pcm & 0x0F);
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if (address == PCM34_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				BYTE ch3_pcm = 0;
				BYTE ch4_pcm = 0;

				// Channel 3 PCM (wave channel)
				APUTODO("For checking is channel 3 is enabled, should we also check NR52.channelSoundONOFFFields.channel3ONFlag bit as well ?");
				if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == SET
				    && pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled == YES
					&& isDACEnabled(AUDIO_CHANNELS::CHANNEL_3))
				{
					ch3_pcm = (getLogicalAmplitude(AUDIO_CHANNELS::CHANNEL_3) & 0x0F)
						>> pGBc_instance->GBc_state.audio.channel3OutputLevelAndShift;
				}

				// Channel 4 PCM (noise channel)
				if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == SET 
				    && pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled == YES
					&& isDACEnabled(AUDIO_CHANNELS::CHANNEL_4))
				{
					ch4_pcm = getLogicalAmplitude(AUDIO_CHANNELS::CHANNEL_4)
						* pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume;
				}

				RETURN (ch4_pcm << 4) | (ch3_pcm & 0x0F);
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				RETURN 0xFF;
			}
		}

		if ((address >= 0xFF78) && (address <= 0xFF7F))
		{
			RETURN 0xFF;
		}

		// reading from HIRAM
		if (address >= HIRAM_START_ADDRESS && address <= HIRAM_END_ADDRESS)
		{
			RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
		}

		// reading from IE
		if (address == IE_ADDRESS)
		{
			RETURN pGBc_memory->GBcMemoryMap.mInterruptEnable.interruptEnableMemory;
		}

		// Any other valid address
		if (address >= ROM_00_START_ADDRESS && address <= IE_ADDRESS)
		{
			RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address];
		}
		else
		{
			FATAL("GB-GBC memory buffer overflow");
			RETURN (byte)INVALID;
		}
	}
}

void GBc_t::writeRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source)
{
	if (ROM_TYPE == ROM::TEST_SST)
	{
		auto index = pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.indexer;
		pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].address = address;
		pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].data = data;
		pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].isRead = NO;
		pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.cycles[index].isWrite = YES;
		++pGBc_instance->GBc_state.emulatorStatus.debugger.tomHarte.cycles.indexer;

		pGBc_memory->GBcRawMemory[address] = data;
	}
	else
	{
		// Refer : https://discord.com/channels/465585922579103744/465586075830845475/1063495906441302016
		if (ENABLED)
		{
			auto currentDMAAddr = (pGBc_instance->GBc_state.emulatorStatus.DMASource + pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred);

			// On DMG, during OAM DMA, the CPU can access only HRAM (memory at $FF80-$FFFE)
			// Refer : https://gbdev.io/pandocs/OAM_DMA_Transfer.html
			// There is a deviation in CGB behaviour w.r.t DMA conflicts, refer the below table for more information
			TODO("Find source for \"On CGB, WRAM access still conflicts if DMA source is NOT VRAM\"");
			/*
			 * DMA Bus Conflict Behavior - DMG vs CGB (applies to both reads and writes)
			 * Reference: SameBoy memory.c is_addr_in_dma_use()
			 * https://github.com/LIJI32/SameBoy/blob/master/Core/memory.c#L134-L150
			 *
			 * +----------------------------------+-------------+-------------+
			 * | Scenario                         |     DMG     |     CGB     |
			 * +----------------------------------+-------------+-------------+
			 * | Access ROM while DMA from ROM    |  Conflict   |  Conflict   |
			 * | Access ROM while DMA from ERAM   |  Conflict   |  Conflict   |
			 * | Access ROM while DMA from WRAM   |  Conflict   | No Conflict |
			 * | Access ROM while DMA from VRAM   |  Conflict   | No Conflict |
			 * +----------------------------------+-------------+-------------+
			 * | Access ERAM while DMA from ROM   |  Conflict   |  Conflict   |
			 * | Access ERAM while DMA from ERAM  |  Conflict   |  Conflict   |
			 * | Access ERAM while DMA from WRAM  |  Conflict   | No Conflict |
			 * | Access ERAM while DMA from VRAM  |  Conflict   | No Conflict |
			 * +----------------------------------+-------------+-------------+
			 * | Access WRAM while DMA from ROM   |  Conflict   |  Conflict   |
			 * | Access WRAM while DMA from ERAM  |  Conflict   |  Conflict   |
			 * | Access WRAM while DMA from WRAM  |  Conflict   |  Conflict   |
			 * | Access WRAM while DMA from VRAM  |  Conflict   | No Conflict |
			 * +----------------------------------+-------------+-------------+
			 * | Access VRAM while DMA from ROM   |  Conflict   | No Conflict |
			 * | Access VRAM while DMA from ERAM  |  Conflict   | No Conflict |
			 * | Access VRAM while DMA from WRAM  |  Conflict   | No Conflict |
			 * | Access VRAM while DMA from VRAM  |  Conflict   |  Conflict   |
			 * +----------------------------------+-------------+-------------+
			 *
			 * DMG: Single bus - ROM/ERAM/WRAM/VRAM all share the same bus
			 *      Conflict occurs when accessing any memory while DMA uses same bus
			 *
			 * CGB: Separate buses - External (ROM/ERAM) and Internal (WRAM/VRAM)
			 *      - ROM/ERAM conflicts only with DMA from ROM/ERAM (external bus)
			 *      - WRAM conflicts with DMA from ROM/ERAM/WRAM (any non-VRAM source)
			 *      - VRAM conflicts only with DMA from VRAM (internal bus)
			 */

			if ((pGBc_instance->GBc_state.emulatorStatus.isDMAActive == YES && pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay == ZERO)
				&& source == MEMORY_ACCESS_SOURCE::CPU)
			{
				if (ROM_TYPE == ROM::GAME_BOY)
				{
					// writing from HIRAM
					if (address >= HIRAM_START_ADDRESS && address <= HIRAM_END_ADDRESS)
					{
						pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
						RETURN;
					}
					else if (((address >= ROM_00_START_ADDRESS && address <= ROM_NN_END_ADDRESS)
							|| (address >= EXTERNAL_RAM_START_ADDRESS && address <= EXTERNAL_RAM_END_ADDRESS)
							|| (address >= WORK_RAM_00_START_ADDRESS && address <= ECHO_RAM_END_ADDRESS))
						&& ((currentDMAAddr >= ROM_00_START_ADDRESS && currentDMAAddr <= ROM_NN_END_ADDRESS)
							|| (currentDMAAddr >= EXTERNAL_RAM_START_ADDRESS && currentDMAAddr <= EXTERNAL_RAM_END_ADDRESS)
							|| (currentDMAAddr >= WORK_RAM_00_START_ADDRESS && currentDMAAddr <= ECHO_RAM_END_ADDRESS)))
					{
						WARN("Invalid access by DMG CPU to ROM/WRAM/ERAM when DMA is active");
						RETURN;
					}
					else if ((address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
						&& (currentDMAAddr >= VRAM_START_ADDRESS && currentDMAAddr <= VRAM_END_ADDRESS))
					{
						WARN("Invalid access by DMG CPU to VRAM when DMA is active");
						RETURN;
					}
					else if (address >= OAM_START_ADDRESS && address <= OAM_END_ADDRESS)
					{
						RETURN;
					}
				}
				else
				{
					// writing to HIRAM
					if (address >= HIRAM_START_ADDRESS && address <= HIRAM_END_ADDRESS)
					{
						pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
						RETURN;
					}
					else if (((address >= ROM_00_START_ADDRESS && address <= ROM_NN_END_ADDRESS)
						|| (address >= EXTERNAL_RAM_START_ADDRESS && address <= EXTERNAL_RAM_END_ADDRESS))
						&& ((currentDMAAddr >= ROM_00_START_ADDRESS && currentDMAAddr <= ROM_NN_END_ADDRESS)
							|| (currentDMAAddr >= EXTERNAL_RAM_START_ADDRESS && currentDMAAddr <= EXTERNAL_RAM_END_ADDRESS)))
					{
						WARN("Invalid access by CGB CPU to ROM/ERAM when DMA is active");
						RETURN;
					}
					// On CGB, WRAM write conflicts if DMA source is NOT VRAM
					// Reference: SameBoy memory.c is_addr_in_dma_use() line 142-144
					// https://github.com/LIJI32/SameBoy/blob/master/Core/memory.c#L142-L144
					else if ((address >= WORK_RAM_00_START_ADDRESS && address <= ECHO_RAM_END_ADDRESS)
						&& ((currentDMAAddr >= ROM_00_START_ADDRESS && currentDMAAddr <= ROM_NN_END_ADDRESS)
							|| (currentDMAAddr >= EXTERNAL_RAM_START_ADDRESS && currentDMAAddr <= EXTERNAL_RAM_END_ADDRESS)
							|| (currentDMAAddr >= WORK_RAM_00_START_ADDRESS && currentDMAAddr <= ECHO_RAM_END_ADDRESS)))
					{
						WARN("Invalid access by CGB CPU to WRAM when DMA is active");
						RETURN;
					}
					else if ((address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
						&& (currentDMAAddr >= VRAM_START_ADDRESS && currentDMAAddr <= VRAM_END_ADDRESS))
					{
						WARN("Invalid access by CGB CPU to VRAM when DMA is active");
						RETURN;
					}
					else if (address >= OAM_START_ADDRESS && address <= OAM_END_ADDRESS)
					{
						RETURN;
					}
				}
			}
		}

		// check for memory bank processing
		if (address <= ROM_NN_END_ADDRESS)
		{
			// Below if block needed for MBC block in BESS specifications
			if (ENABLED)
			{
				if (address <= 0x1FFF)
				{
					if (pGBc_emuStatus->mbc1 == YES
						||
						pGBc_emuStatus->mbc2 == YES
						||
						pGBc_emuStatus->mbc3 == YES
						||
						pGBc_emuStatus->mbc5 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg0 = data;
					}
				}
				else if (address <= 0x2FFF)
				{
					if (pGBc_emuStatus->mbc1 == YES
						||
						pGBc_emuStatus->mbc3 == YES
						||
						pGBc_emuStatus->mbc5 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg1 = data;
					}
					else if (pGBc_emuStatus->mbc2 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg0 = data;
					}
				}
				else if (address <= 0x3FFF)
				{
					if (pGBc_emuStatus->mbc1 == YES
						||
						pGBc_emuStatus->mbc3 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg1 = data;
					}
					else if (pGBc_emuStatus->mbc2 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg0 = data;
					}
					else if (pGBc_emuStatus->mbc5 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg2 = data;
					}
				}
				else if (address <= 0x5FFF)
				{
					if (pGBc_emuStatus->mbc1 == YES
						||
						pGBc_emuStatus->mbc3 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg2 = data;
					}
					if (pGBc_emuStatus->mbc5 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg3 = data;
					}
				}
				else if (address <= 0x7FFF)
				{
					if (pGBc_emuStatus->mbc1 == YES
						||
						pGBc_emuStatus->mbc3 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg3 = data;
					}
				}
				else if ((address >= 0xA000) && (address <= 0xBFFF))
				{
					if (pGBc_emuStatus->mbc3 == YES)
					{
						pGBc_emuStatus->dataWrittenToMBCReg4 = data;
					}
				}
			}

			// --- 0x0000 - 0x1FFF : RAM/RTC enable ---
			if (address <= 0x1FFF)
			{
				if (pGBc_emuStatus->mbc1 == YES || pGBc_emuStatus->mbc2 == YES || pGBc_emuStatus->mbc3 == YES || pGBc_emuStatus->mbc5 == YES)
				{
					// MBC2 special handling
					if (pGBc_emuStatus->mbc2 == YES)
					{
						// For MBC2, the least significant bit of the upper address byte must be zero to enable/disable cart RAM
						if (GETBIT(8, address) == ZERO)
						{
							pGBc_emuStatus->is_mbc2_rom_mode = NO;
							((data & 0x0F) == 0x0A) ? enableRAMBank() : disableRAMBank();
						}
						// For MBC2, the least significant bit of the upper address byte must be one to set the rom bank number
						else
						{
							pGBc_emuStatus->is_mbc2_rom_mode = YES;
							auto ROMBankNumber = (data & 0x0F);
							if (ROMBankNumber == ZERO) ROMBankNumber = ONE;
							ROMBankNumber &= (getNumberOfROMBanksUsed() - ONE);
							setROMBankNumber(ROMBankNumber);
						}
						RETURN;
					}

					// MBC1/3/5 RAM or RTC enable
					if ((data & 0x0F) == 0x0A)
					{
						enableRAMBank();
						if (pGBc_emuStatus->mbc3 == YES) enableRTCAccess();
					}
					else
					{
						disableRAMBank();
						if (pGBc_emuStatus->mbc3 == YES) disableRTCAccess();
					}
					RETURN;
				}
			}

			// --- 0x2000 - 0x3FFF : ROM bank lower bits ---
			else if (address <= 0x3FFF)
			{
				if (pGBc_emuStatus->mbc1 == YES || pGBc_emuStatus->mbc2 == YES || pGBc_emuStatus->mbc3 == YES || pGBc_emuStatus->mbc5 == YES)
				{
					auto ROMBankNumber = getROMBankNumber();

					if (pGBc_emuStatus->mbc1 == YES)
					{
						// Refer : https://gekkio.fi/files/gb-docs/gbctr.pdf
						pGBc_emuStatus->currentROMBankNumber.mbc1Fields.mbcBank1Reg = data & 0x1F;
						if (pGBc_emuStatus->currentROMBankNumber.mbc1Fields.mbcBank1Reg == ZERO)
							pGBc_emuStatus->currentROMBankNumber.mbc1Fields.mbcBank1Reg = ONE;
						RETURN;
					}

					if (pGBc_emuStatus->mbc2 == YES)
					{
						// For MBC2, the least significant bit of the upper address byte must be zero to enable/disable cart RAM
						if (GETBIT(8, address) == ZERO)
						{
							pGBc_emuStatus->is_mbc2_rom_mode = NO;
							((data & 0x0F) == 0x0A) ? enableRAMBank() : disableRAMBank();
						}
						else
						{
							pGBc_emuStatus->is_mbc2_rom_mode = YES;
							ROMBankNumber = data & 0x0F;
							if (ROMBankNumber == ZERO) ROMBankNumber = ONE;
							ROMBankNumber &= (getNumberOfROMBanksUsed() - ONE);
							setROMBankNumber(ROMBankNumber);
						}
						RETURN;
					}

					if (pGBc_emuStatus->mbc3 == YES)
					{
						ROMBankNumber = data & 0xFF; // MBC30 support
						if (ROMBankNumber == ZERO) ROMBankNumber = ONE;
						ROMBankNumber &= (getNumberOfROMBanksUsed() - ONE); // Ensure that rom bank number is within the maximum supported for the game
						setROMBankNumber(ROMBankNumber);
						RETURN;
					}

					if (pGBc_emuStatus->mbc5 == YES)
					{
						if (address <= 0x2FFF)
						{
							ROMBankNumber &= 0x100; // clear the lower 8 bits of the current ROM bank number
							ROMBankNumber |= (data & 0xFF); // set the lower 8 bits of incoming data to lower 8 bits of the current ROM bank number
						}
						else
						{
							ROMBankNumber &= 0xFF; // clear the 9th bit of the current ROM bank number
							ROMBankNumber |= ((data & 0x01) << EIGHT); // set the 0th bit of incoming data to the 9th bit of the current ROM bank number
						}
						ROMBankNumber &= (getNumberOfROMBanksUsed() - ONE); // Ensure that rom bank number is within the maximum supported for the game
						setROMBankNumber(ROMBankNumber);
						RETURN;
					}
				}
			}

			// --- 0x4000 - 0x5FFF : RAM bank or upper ROM bits ---
			else if (address <= 0x5FFF)
			{
				if (pGBc_emuStatus->mbc1 == YES)
				{
					// Refer : https://gekkio.fi/files/gb-docs/gbctr.pdf
					pGBc_emuStatus->currentROMBankNumber.mbc1Fields.mbcBank2Reg = data & 0x03;

					// change higher bits of ROM bank number
					if (getROMOrRAMModeInMBC1() == MBC1_ROM_MODE)
					{
						// Since we came back to ROM mode, fix the RAM bank number to 0
						setRAMBankNumber(ZERO);
						RETURN;
					}
					// set the RAM bank number
					else
					{
						auto ramBankNumber = (data & 0x03) & (getNumberOfRAMBanksUsed() - ONE);
						setRAMBankNumber(ramBankNumber);
						RETURN;
					}
				}
				// change RAM bank number or RTC register number for MBC3
				else if (pGBc_emuStatus->mbc3 == YES)
				{
					if (data <= 0x07)
					{
						auto ramBankNumber = data & (getNumberOfRAMBanksUsed() - ONE);
						setRAMBankNumber(ramBankNumber);
						shouldMapRTCToExternalRAM(NO);
					}
					else if (data >= 0x08 && data <= 0x0C)
					{
						setRTCRegisterNumber(TO_UINT8(data));
						shouldMapRTCToExternalRAM(YES);
					}
				}
				// set the RAM bank number for MBC5
				else if (pGBc_emuStatus->mbc5 == YES)
				{
					auto ramBankNumber = (data & 0x0F) & (getNumberOfRAMBanksUsed() - ONE);
					setRAMBankNumber(ramBankNumber);
					RETURN;
				}
			}

			// --- 0x6000 - 0x7FFF : MBC1 mode select / MBC3 latch ---
			else if (address <= 0x7FFF)
			{
				if (pGBc_emuStatus->mbc1 == YES)
				{
					(data & 0x01) ? setRAMModeInMBC1() : setROMModeIfMBC1();
					RETURN;
				}
				// latch clock data for mbc3
				else if (pGBc_emuStatus->mbc3 == YES)
				{
					if (isRTCAccessEnabled())
					{
						if (getRTCFSM() == ZERO && ((data & 0x01) == 0x00))
						{
							setRTCFSM(ONE);
						}
						else if (getRTCFSM() == ONE && ((data & 0x01) == 0x01))
						{
							latchRTCRegisters();
							setRTCFSM(ZERO);
						}
					}
				}
			}

			RETURN;
		}

		// writing to ROM is invalid
		if (address <= ROM_NN_START_ADDRESS)
		{
			WARN("Attempting to write to Read Only Memory 0x%X", address);
			RETURN;
		}

		// writing to VRAM
		if (address >= VRAM_START_ADDRESS && address <= VRAM_END_ADDRESS)
		{
			if (pGBc_display->blockVramW == YES && source == MEMORY_ACCESS_SOURCE::CPU)
			{
				RETURN;
			}

			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				address -= 0x8000;
				if (address < 0x2000)
				{
					pGBc_instance->GBc_state.entireVram.vramMemoryBanks.mVRAMBanks[getVRAMBankNumber()][address] = data;
				}
				else
				{
					WARN("VRAM buffer overflow");
				}
				RETURN;
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
				RETURN;
			}
		}

		// writing to banked RAM
		if (address >= EXTERNAL_RAM_START_ADDRESS && address <= EXTERNAL_RAM_END_ADDRESS)
		{
			if (pGBc_emuStatus->mbc3 == YES)
			{
				if (isRTCMappedToExternalRAM() == YES)
				{
					writeToRTCRegisterIfApplicable(TO_UINT8(data));
					RETURN;
				}
			}

			address -= 0xA000;

			// Refer 13.3 of https://gekkio.fi/files/gb-docs/gbctr.pdf
			if (pGBc_emuStatus->mbc2 == YES)
			{
				address &= (0x200 - ONE);
				data |= 0xF0; // Mask obtained after analysing the test code
			}

			if (isRAMBankEnabled() == YES)
			{
				pGBc_instance->GBc_state.entireRam.ramMemoryBanks.mRAMBanks[getRAMBankNumber()][address] = data;
			}
			else
			{
				WARN("RAM Banking is not enabled!");
			}

			RETURN;
		}

		// writing to Work RAM should update the Echo RAM
		if (address >= WORK_RAM_00_START_ADDRESS && address <= WORK_RAM_01_END_ADDRESS)
		{
			if (address <= 0xDDFF)
			{
				if (ROM_TYPE == ROM::GAME_BOY_COLOR)
				{
					// WRAM 00
					if (address <= WORK_RAM_00_END_ADDRESS)
					{
						pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
						pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address + 0x2000] = data;
						RETURN;
					}
					// WRAM 01
					else
					{
						if ((address - 0xD000) < 0x1000)
						{
							// NOTE: "mWRAM01Banks" goes from 0 - 6, whereas "getWRAMBankNumber()" goes from 1 - 7
							// "getWRAMBankNumber()" is set by the game rom and hence, no tweaking is allowed inside the function
							// Hence, tweaking is possible only at the interfacing between mWRAM01Banks and "getWRAMBankNumber()"
							// Therefore, getWRAMBankNumber() is decremented once when used within mWRAM01Banks
							pGBc_instance->GBc_state.entireWram01.wram01MemoryBanks.mWRAM01Banks[getWRAMBankNumber() - ONE][address - 0xD000] = data;
						}
						else
						{
							WARN("WRAM buffer overflow");
						}

						pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address + 0x2000] = data;
						RETURN;
					}
				}
				else if (ROM_TYPE == ROM::GAME_BOY)
				{
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address + 0x2000] = data;
					RETURN;
				}
			}
			// address >= 0xDE00
			else
			{
				if (ROM_TYPE == ROM::GAME_BOY_COLOR)
				{
					// WRAM 01
					address -= 0xD000;
					if (address < 0x1000)
					{
						// NOTE: "mWRAM01Banks" goes from 0 - 6, whereas "getWRAMBankNumber()" goes from 1 - 7
						// "getWRAMBankNumber()" is set by the game rom and hence, no tweaking is allowed inside the function
						// Hence, tweaking is possible only at the interfacing between mWRAM01Banks and "getWRAMBankNumber()"
						// Therefore, getWRAMBankNumber() is decremented once when used within mWRAM01Banks
						pGBc_instance->GBc_state.entireWram01.wram01MemoryBanks.mWRAM01Banks[(getWRAMBankNumber() - ONE)][address] = data;
					}
					else
					{
						FATAL("WRAM buffer overflow");
					}

					RETURN;
				}
				else if (ROM_TYPE == ROM::GAME_BOY)
				{
					pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
					RETURN;
				}
			}
		}

		// writing to Echo RAM should update the Work RAM
		if (address >= ECHO_RAM_START_ADDRESS && address <= ECHO_RAM_END_ADDRESS)
		{
			pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
			pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address - 0x2000] = data;
			RETURN;
		}

		// writing to OAM memory
		if (address >= OAM_START_ADDRESS && address <= OAM_END_ADDRESS)
		{
			if (pGBc_display->blockOAMW == YES && source == MEMORY_ACCESS_SOURCE::CPU)
			{
				RETURN;
			}

			pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
			RETURN;
		}

		// writing to restricted memory is ignored
		if (address >= RESTRICTED_MEMORY_START_ADDRESS && address <= RESTRICTED_MEMORY_END_ADDRESS)
		{
			++pGBc_instance->GBc_state.emulatorStatus.unusableMemoryWrites;
			RETURN;
		}

		// writing to Joypad register; bits 7 and 6 are unused
		if (address == P1_JOYP_ADDRESS)
		{
			const uint8_t selDir = GETBIT(FOUR, data); // P14
			const uint8_t selAct = GETBIT(FIVE, data); // P15

			pGBc_peripherals->P1_JOYP.joyPadFields.P14_SEL_DIRECTION_KEYS = selDir;
			pGBc_peripherals->P1_JOYP.joyPadFields.P15_SEL_ACTION_KEYS = selAct;

			captureIO();
			RETURN;
		}

		// writing to sc register 
		if (address == SC_ADDRESS)
		{
			BIT currentClockSpeed = pGBc_peripherals->SC.scFields.CLOCK_SPEED;

			// NOTE: update the SC register before calling "processSerialClockSpeedBit" as the function assumes latest value is present in the register

			pGBc_peripherals->SC.scMemory = (data & 0x83);

			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				if (currentClockSpeed != (GETBIT(ONE, data)))
				{
					processSerialClockSpeedBit();
				}
			}

			RETURN;
		}

		if (address == DIV_ADDRESS_LSB)
		{
			if (source == MEMORY_ACCESS_SOURCE::BESS)
			{
				pGBc_peripherals->DIV.divBytes.DIV_LSB.divByte = data;
			}

			RETURN;
		}

		// writing to divider register resets it
		if (address == DIV_ADDRESS_MSB)
		{
			if (source == MEMORY_ACCESS_SOURCE::BESS)
			{
				pGBc_peripherals->DIV.divBytes.DIV_MSB.divByte = data;
				RETURN;
			}

			// NOTE: For emulation purpose, we maintain an internal DIV counter
			// Now, when we reset the DIV register, this will ensure the incrmenting of DIV from 0 again
			// During this reset, if we didn't reset the internal DIV counter, then the time taken by DIV register
			// to reach its threshold post reset will be slightly quicker and not accurate, the reason being
			// internal DIV counter didnot start from 0; Hence we reset the internal DIV counter as well

			pGBc_instance->GBc_state.emulatorStatus.ticks.dividerCounter = ZERO;

			// NOTE: For rapid_toggle.gb to pass, we need to reset both DIV LSB and DIV MSB when written to DIV MSB
			pGBc_peripherals->DIV.divMemory = ZERO;
			RETURN;
		}

		// writing to TIMA
		if (address == TIMA_ADDRESS)
		{
			if (pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow != INVALID
				&& pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow < FOUR) // Strange cycle A
			{
				// Incase in the next timer M-cycle after TIMA overflow, TIMER interrupt was about to be raised, reset that!
				// Refer : https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html
				pGBc_instance->GBc_state.emulatorStatus.waitingToRequestTimerInterrupt = NO;
			}
			else if (pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow >= FOUR
				&& pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow < EIGHT) // Strange cycle B
			{
				RETURN; // Ignore the write to TIMA
			}

			pGBc_peripherals->TIMA = data;

			RETURN;
		}

		// writing to TMA
		if (address == TMA_ADDRESS)
		{
			pGBc_peripherals->TMA = data;

			TODO("This if block is needed to pass MTS's timer reload tests, but I am not yet completely convinced about the logic");
			if (pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow >= FOUR
				&& pGBc_instance->GBc_state.emulatorStatus.clocksAfterTIMAOverflow < EIGHT) // Strange cycle B
			{
				pGBc_peripherals->TIMA = data;
			}

			RETURN;
		}

		// writing to TAC register resets TIMA to TMA
		if (address == TAC_ADDRESS)
		{
			data &= 0x07; // zero out bits 7 - 3
			pGBc_peripherals->TAC.timerControlMemory = data;
			pGBc_peripherals->TAC.timerControlFields.TAC_3 = ONE;
			pGBc_peripherals->TAC.timerControlFields.TAC_4 = ONE;
			pGBc_peripherals->TAC.timerControlFields.TAC_5 = ONE;
			pGBc_peripherals->TAC.timerControlFields.TAC_6 = ONE;
			pGBc_peripherals->TAC.timerControlFields.TAC_7 = ONE;

			RETURN;
		}

		// writing to IF will only set bits 0 - 4; others are always zero
		if (address == IF_ADDRESS)
		{
			pGBc_peripherals->IF.interruptRequestMemory = data;

			pGBc_peripherals->IF.interruptRequestFields.NO_INT05 = ONE;
			pGBc_peripherals->IF.interruptRequestFields.NO_INT06 = ONE;
			pGBc_peripherals->IF.interruptRequestFields.NO_INT07 = ONE;

			RETURN;
		}

		// writing to Sound channel 1 sweep
		if (address == NR10_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			pGBc_peripherals->NR10.channelSweepMemory = data;

			// NOTE: One of the weird quirks of frequency sweep
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			if (pGBc_peripherals->NR10.channelSweepFields.sweepDirection == ZERO
				&& pGBc_instance->GBc_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger == YES)
			{
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel1ONFlag = ZERO;
				pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.trigger = ZERO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
			}

			RETURN;
		}

		// writing to Sound channel 1 length timer & duty cycle
		if (address == NR11_ADDRESS)
		{
			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				if (ROM_TYPE == ROM::GAME_BOY)
				{
					pGBc_peripherals->NR11.channelLengthAndDutyFields.initialLengthTimer = (data & 0x3F);
				}
				else
				{
					RETURN;
				}
			}
			else
			{
				pGBc_peripherals->NR11.channelLengthAndDutyMemory = data;
			}

			pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer
				= 64 - pGBc_peripherals->NR11.channelLengthAndDutyFields.initialLengthTimer;

			RETURN;
		}

		// writing to Sound channel 1 volume & envelope
		if (address == NR12_ADDRESS)
		{
			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			FLAG wasVolumePeriodZero = pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.envelopeSweepPace == ZERO;
			FLAG wasEnvelopeInSubtractMode = pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.envelopeDirection == ZERO;

			pGBc_peripherals->NR12.channelVolumeAndEnvelopeMemory = data;

			// NOTE: One of the weird quirks of APU called the "Zombie Mode"
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == SET 
				&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled == YES)
			{
				if (wasVolumePeriodZero == YES
					&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isVolumeEnvelopeStillDoingAutomaticUpdates == YES)
				{
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume;
				}
				else if (wasEnvelopeInSubtractMode == YES)
				{
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume;
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume;
				}

				if (
					(wasEnvelopeInSubtractMode == YES
						&& pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.envelopeDirection == ONE)
					||
					(wasEnvelopeInSubtractMode == NO
						&& pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.envelopeDirection == ZERO))
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume
						= SIXTEEN - pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume;
				}

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume
					&= 0x0F;
			}

			continousDACCheck();

			RETURN;
		}

		// writing to Sound channel 1 period low
		if (address == NR13_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			pGBc_peripherals->NR13.lowerPeriodValue = data;
			RETURN;
		}

		// writing to Sound channel 1 period high & control
		if (address == NR14_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			FLAG wasLengthEnableBitZero = pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.soundLengthEnable == ZERO;

			pGBc_peripherals->NR14.channelHigherPeriodAndControlMemory = data;

			// NOTE: One of the weird quirks of length counter
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			if (wasLengthEnableBitZero == YES
				&& pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
				&& pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter == TRUE
				&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer > ZERO)
			{
				--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer;

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer == ZERO)
				{
					pGBc_peripherals->NR52.channelSoundONOFFFields.channel1ONFlag = ZERO;
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
				}
			}

			if (pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.trigger == ONE)
			{
				enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_1);

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer == ZERO)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer = 64;

					// NOTE: One of the weird quirks of length counter
					// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
					if (pGBc_peripherals->NR14.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
						&& pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
					{
						--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer;
					}
				}

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].envelopePeriodTimer
					= pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.envelopeSweepPace;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].currentVolume
					= pGBc_peripherals->NR12.channelVolumeAndEnvelopeFields.initialVolumeOfEnvelope;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isVolumeEnvelopeStillDoingAutomaticUpdates = YES;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].shadowFrequency
					= getChannelPeriod(AUDIO_CHANNELS::CHANNEL_1);

				if (pGBc_peripherals->NR10.channelSweepFields.sweepPace > ZERO)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepTimer
						= pGBc_peripherals->NR10.channelSweepFields.sweepPace;
				}
				else
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepTimer = EIGHT;
				}

				if (pGBc_peripherals->NR10.channelSweepFields.sweepPace > ZERO
					|| pGBc_peripherals->NR10.channelSweepFields.sweepSlopeControl > ZERO)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepEnabled = ENABLED;
				}
				else
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].sweepEnabled = DISABLED;
				}

				pGBc_instance->GBc_state.audio.wasSweepDirectionNegativeAtleastOnceSinceLastTrigger = NO;

				if (pGBc_peripherals->NR10.channelSweepFields.sweepSlopeControl > ZERO)
				{
					performOverFlowCheck();
				}

				// Source : https://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu
				uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_1)) * FOUR;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].frequencyTimer
					= (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].frequencyTimer & THREE)
					+ resetFrequencyTimer;
			}

			RETURN;
		}

		// writing to Sound channel 2 length timer & duty cycle
		if (address == NR21_ADDRESS)
		{
			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				if (ROM_TYPE == ROM::GAME_BOY)
				{
					pGBc_peripherals->NR21.channelLengthAndDutyFields.initialLengthTimer = (data & 0x3F);
				}
				else
				{
					RETURN;
				}
			}
			else
			{
				pGBc_peripherals->NR21.channelLengthAndDutyMemory = data;
			}

			pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer
				= 64 - pGBc_peripherals->NR21.channelLengthAndDutyFields.initialLengthTimer;

			RETURN;
		}

		// writing to Sound channel 2 volume & envelope
		if (address == NR22_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			FLAG wasVolumePeriodZero = pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.envelopeSweepPace == ZERO;
			FLAG wasEnvelopeInSubtractMode = pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.envelopeDirection == ZERO;

			pGBc_peripherals->NR22.channelVolumeAndEnvelopeMemory = data;

			// NOTE: One of the weird quirks of APU called the "Zombie Mode"
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == SET 
				&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled == YES)
			{
				if (wasVolumePeriodZero == YES
					&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isVolumeEnvelopeStillDoingAutomaticUpdates == YES)
				{
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume;
				}
				else if (wasEnvelopeInSubtractMode == YES)
				{
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume;
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume;
				}

				if (
					(wasEnvelopeInSubtractMode == YES
						&& pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.envelopeDirection == ONE)
					||
					(wasEnvelopeInSubtractMode == NO
						&& pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.envelopeDirection == ZERO))
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume
						= SIXTEEN - pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume;
				}

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume
					&= 0x0F;
			}

			continousDACCheck();

			RETURN;
		}

		// writing to Sound channel 2 period low
		if (address == NR23_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			pGBc_peripherals->NR23.lowerPeriodValue = data;
			RETURN;
		}

		// writing to Sound channel 2 period high & control
		if (address == NR24_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			FLAG wasLengthEnableBitZero = pGBc_peripherals->NR24.channelHigherPeriodAndControlFields.soundLengthEnable == ZERO;

			pGBc_peripherals->NR24.channelHigherPeriodAndControlMemory = data;

			// NOTE: One of the weird quirks of length counter
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			if (wasLengthEnableBitZero == YES
				&& pGBc_peripherals->NR24.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
				&& pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter == TRUE
				&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer > ZERO)
			{
				--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer;

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer == ZERO)
				{
					pGBc_peripherals->NR52.channelSoundONOFFFields.channel2ONFlag = ZERO;
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
				}
			}

			if (pGBc_peripherals->NR24.channelHigherPeriodAndControlFields.trigger == ONE)
			{
				enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_2);

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer == ZERO)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer = 64;

					// NOTE: One of the weird quirks of length counter
					// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
					if (pGBc_peripherals->NR24.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
						&& pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
					{
						--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer;
					}
				}

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].envelopePeriodTimer
					= pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.envelopeSweepPace;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].currentVolume
					= pGBc_peripherals->NR22.channelVolumeAndEnvelopeFields.initialVolumeOfEnvelope;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isVolumeEnvelopeStillDoingAutomaticUpdates = YES;
		
				// Source : https://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu
				uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_2)) * FOUR;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].frequencyTimer
					= (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].frequencyTimer & THREE)
					+ resetFrequencyTimer;
			}

			RETURN;
		}

		// writing to Sound channel 3 DAC enable
		if (address == NR30_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			pGBc_peripherals->NR30.channelDACEnableMemory = data;

			continousDACCheck();

			RETURN;
		}

		// writing to Sound channel 3 length timer
		if (address == NR31_ADDRESS)
		{
			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO
				&& ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				RETURN;
			}

			pGBc_peripherals->NR31 = data;
			pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer
				= 256 - pGBc_peripherals->NR31;

			RETURN;
		}

		// Sound channel 3 output level
		if (address == NR32_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			pGBc_peripherals->NR32.channelOutputLevelMemory = data;

			switch (pGBc_peripherals->NR32.channelOutputLevelFields.outputLevelSelection)
			{
			case ZERO:
				pGBc_instance->GBc_state.audio.channel3OutputLevelAndShift = FOUR;
				BREAK;
			case ONE:
				pGBc_instance->GBc_state.audio.channel3OutputLevelAndShift = ZERO;
				BREAK;
			case TWO:
				pGBc_instance->GBc_state.audio.channel3OutputLevelAndShift = ONE;
				BREAK;
			case THREE:
				pGBc_instance->GBc_state.audio.channel3OutputLevelAndShift = TWO;
				BREAK;
			}

			RETURN;
		}

		// Sound channel 3 period low
		if (address == NR33_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			pGBc_peripherals->NR33.lowerPeriodValue = data;
			RETURN;
		}

		// writing to Sound channel 3 period high & control
		if (address == NR34_ADDRESS)
		{
			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			FLAG wasLengthEnableBitZero = pGBc_peripherals->NR34.channelHigherPeriodAndControlFields.soundLengthEnable == ZERO;

			pGBc_peripherals->NR34.channelHigherPeriodAndControlMemory = data;

			// NOTE: One of the weird quirks of length counter
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			if (wasLengthEnableBitZero == YES
				&& pGBc_peripherals->NR34.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
				&& pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter == TRUE
				&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer > ZERO)
			{
				--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer;

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer == ZERO)
				{
					pGBc_peripherals->NR52.channelSoundONOFFFields.channel3ONFlag = ZERO;
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
				}
			}

			if (pGBc_peripherals->NR34.channelHigherPeriodAndControlFields.trigger == ONE)
			{
				// As per obscure behaviour section of https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
				// "Triggering the wave channel on the DMG while it reads a sample byte will alter the first four bytes of wave RAM"
				if ((isDACEnabled(AUDIO_CHANNELS::CHANNEL_3) == ENABLED) && (ROM_TYPE == ROM::GAME_BOY)
					// Below check is to handle "DMG while it reads a sample byte"
					// So, Wave Channel reads a sample when its frequency timer is about to expire as mentioned in https://gbdev.io/pandocs/Audio_details.html#wave-channel-ch3
					// So, below condition is simulating when we are just about to increment 'waveRamCurrentIndex' and read a sample
					&& ((pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].frequencyTimer == (TWO + WAVE_TRIGGER_CORRUPTION_OFFSET_TICKS))))
				{
					// If we enterred this condition, that means by next 2 APU ticks, Wave Channel would have already 'waveRamCurrentIndex' and read a sample
					// we get the future index...
					auto indexInNext2ApuTicks = ((pGBc_instance->GBc_state.audio.waveRamCurrentIndex + ONE) & THIRTYONE) / TWO;
					if (indexInNext2ApuTicks < FOUR)
					{
						pGBc_peripherals->waveRam[ZERO].waveRamByte = pGBc_peripherals->waveRam[indexInNext2ApuTicks].waveRamByte;
					}
					else
					{
						for (int ii = FOUR; --ii >= ZERO; )
						{
							pGBc_peripherals->waveRam[ii].waveRamByte = pGBc_peripherals->waveRam[(indexInNext2ApuTicks & ~THREE) + ii].waveRamByte;
						}
					}
				}

				enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_3);

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer == ZERO)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer = 256;

					// NOTE: One of the weird quirks of length counter
					// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
					if (pGBc_peripherals->NR34.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
						&& pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
					{
						--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer;
					}
				}

				// Period needs to be reloaded as per https://gbdev.io/pandocs/Audio_Registers.html#ff1e--nr34-channel-3-period-high--control
				uint16_t resetFrequencyTimer = (2048 - getChannelPeriod(AUDIO_CHANNELS::CHANNEL_3)) * TWO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].frequencyTimer = resetFrequencyTimer;
				// As per https://forums.nesdev.org/viewtopic.php?p=188035#p188035, another 6 cycle delay is needed
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].frequencyTimer += SIX;

				if (ROM_TYPE == ROM::GAME_BOY)
				{
					APUTODO("For some reason, to pass test 9 and 12 of \"dmg_sound.gb\", we need to add additional %d cycles to frequencyTimer when trigger is set on a write to NR34", WAVE_TRIGGER_CORRUPTION_OFFSET_TICKS);
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].frequencyTimer += WAVE_TRIGGER_CORRUPTION_OFFSET_TICKS;
				}

				pGBc_instance->GBc_state.audio.waveRamCurrentIndex = RESET;
				pGBc_instance->GBc_state.audio.didChannel3ReadWaveRamPostTrigger = NO;
			}

			RETURN;
		}

		// writing to Sound channel 4 length timer
		if (address == NR41_ADDRESS)
		{
			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO
				&& ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				RETURN;
			}

			pGBc_peripherals->NR41.lengthTimerMemory = data;
			pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer
				= 64 - pGBc_peripherals->NR41.channelLengthTimerFields.lengthTimer;

			RETURN;
		}

		// writing to Sound channel 4 volume & envelope
		if (address == NR42_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			FLAG wasVolumePeriodZero = pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.envelopeSweepPace == ZERO;
			FLAG wasEnvelopeInSubtractMode = pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.envelopeDirection == ZERO;

			pGBc_peripherals->NR42.channelVolumeAndEnvelopeMemory = data;

			// NOTE: One of the weird quirks of APU called the "Zombie Mode"
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == SET 
				&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled == YES)
			{
				if (wasVolumePeriodZero == YES
					&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isVolumeEnvelopeStillDoingAutomaticUpdates == YES)
				{
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume;
				}
				else if (wasEnvelopeInSubtractMode == YES)
				{
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume;
					++pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume;
				}

				if (
					(wasEnvelopeInSubtractMode == YES
						&& pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.envelopeDirection == ONE)
					||
					(wasEnvelopeInSubtractMode == NO
						&& pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.envelopeDirection == ZERO))
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume
						= SIXTEEN - pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume;
				}

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume
					&= 0x0F;
			}

			continousDACCheck();

			RETURN;
		}

		// Sound channel 4 frequency & randomness
		if (address == NR43_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			pGBc_peripherals->NR43.channelFrequencyAndRandomnessMemory = data;
			RETURN;
		}

		// writing to Sound channel 4 control
		if (address == NR44_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			FLAG wasLengthEnableBitZero = pGBc_peripherals->NR44.channelHigherPeriodAndControlFields.soundLengthEnable == ZERO;

			pGBc_peripherals->NR44.channelHigherPeriodAndControlMemory = data;

			// NOTE: One of the weird quirks of length counter
			// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
			if (wasLengthEnableBitZero == YES
				&& pGBc_peripherals->NR44.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
				&& pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter == TRUE
				&& pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer > ZERO)
			{
				--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer;

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer == ZERO)
				{
					pGBc_peripherals->NR52.channelSoundONOFFFields.channel4ONFlag = ZERO;
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;
				}
			}

			if (pGBc_peripherals->NR44.channelHigherPeriodAndControlFields.trigger == ONE)
			{
				enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS::CHANNEL_4);

				if (pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer == ZERO)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer = 64;

					// NOTE: One of the weird quirks of length counter
					// Refer to "Obscure Behaviour" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
					if (pGBc_peripherals->NR44.channelHigherPeriodAndControlFields.soundLengthEnable == ONE
						&& pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter == TRUE)
					{
						--pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer;
					}

				}

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].envelopePeriodTimer
					= pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.envelopeSweepPace;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].currentVolume
					= pGBc_peripherals->NR42.channelVolumeAndEnvelopeFields.initialVolumeOfEnvelope;

				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isVolumeEnvelopeStillDoingAutomaticUpdates = YES;

				// Refer "trigger event" section of this link : https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].LFSR = 0x7FFF;

				// Source : https://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].frequencyTimer += EIGHT;
			}

			RETURN;
		}

		// writing to Master volume & VIN panning
		if (address == NR50_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			pGBc_peripherals->NR50.channelMasterVolumeAndVINPanningMemory = data;
			RETURN;
		}

		// writing to Sound panning
		if (address == NR51_ADDRESS)
		{

			if (pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				RETURN;
			}

			pGBc_peripherals->NR51.channelSoundPanningMemory = data;
			RETURN;
		}

		// writing to Sound ON/OFF
		if (address == NR52_ADDRESS)
		{
			BYTE APU_POWER_WAS = pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle;

			pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle = GETBIT(7, data);

			// Channel ON -> Channel OFF
			if (APU_POWER_WAS == ONE && pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ZERO)
			{
				for (uint16_t address = 0xFF10; address <= 0xFF25; address++)
				{
					pGBc_memory->GBcRawMemory[address] = ZERO;
				}

				pGBc_peripherals->NR52.channelSoundONOFFFields.channel1ONFlag = ZERO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].isChannelActuallyEnabled = DISABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel2ONFlag = ZERO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].isChannelActuallyEnabled = DISABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel3ONFlag = ZERO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].isChannelActuallyEnabled = DISABLED;
				pGBc_peripherals->NR52.channelSoundONOFFFields.channel4ONFlag = ZERO;
				pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].isChannelActuallyEnabled = DISABLED;

			}
			// Channel OFF -> Channel ON
			else if (APU_POWER_WAS == ZERO && pGBc_peripherals->NR52.channelSoundONOFFFields.allChannelONOFFToggle == ONE)
			{
				// Reset the length counters in CGB mode during power up
				if (ROM_TYPE == ROM::GAME_BOY_COLOR)
				{
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_1].lengthTimer = ZERO;
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_2].lengthTimer = ZERO;
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].lengthTimer = ZERO;
					pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_4].lengthTimer = ZERO;
				}

				pGBc_instance->GBc_state.audio.wasPowerCycled = YES;

				// NOTE: As we are resetting the frame sequencer, next half period WILL clock the length counter
				pGBc_instance->GBc_state.audio.nextHalfWillNotClockLengthCounter = FALSE;
			}

			RETURN;
		}

		// writing to Wave RAM
		if (address >= WAVE_RAM_START_ADDRESS && address <= WAVE_RAM_END_ADDRESS)
		{
			if (isChannel3Active() == YES)
			{
				if (ROM_TYPE == ROM::GAME_BOY && pGBc_instance->GBc_state.audio.audioChannelInstance[AUDIO_CHANNELS::CHANNEL_3].frequencyTimer - TWO < TWO)
				{
					uint8_t waveRamIndex = ((pGBc_instance->GBc_state.audio.waveRamCurrentIndex + ONE) & THIRTYONE); // This goes from 0 - 31 (as it access nibble rather than byte)
					pGBc_peripherals->waveRam[(uint8_t)(waveRamIndex / TWO)].waveRamByte = data;
				}
				else if (ROM_TYPE == ROM::GAME_BOY_COLOR)
				{
					uint8_t waveRamIndex = pGBc_instance->GBc_state.audio.waveRamCurrentIndex; // This goes from 0 - 31 (as it access nibble rather than byte)
					pGBc_peripherals->waveRam[(uint8_t)(waveRamIndex / TWO)].waveRamByte = data;
				}
			}
			else
			{
				pGBc_peripherals->waveRam[(address - WAVE_RAM_START_ADDRESS)].waveRamByte = data;
			}

			RETURN;
		}

		// writing to LCDC
		if (address == LCDC_ADDRESS)
		{
			auto oldLCDC = pGBc_peripherals->LCDC;

#if (GB_GBC_ENABLE_TILE_SEL_GLITCH == YES)
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				// Refer // Refer : https://github.com/mattcurrie/mealybug-tearoom-tests/blob/master/the-comprehensive-game-boy-ppu-documentation.md#tile_sel-bit-4

				BYTE oldTileSel = GETBIT(FOUR, oldLCDC.lcdControlMemory);
				BYTE newTileSel = GETBIT(FOUR, data);

				// Different behaviours for double speed and normal speed is based on 
				// Refer : https://github.com/LIJI32/SameBoy/blob/master/Core/sm83_cpu.c#L271
				// Refer : https://github.com/LIJI32/SameBoy/blob/master/Core/sm83_cpu.c#L288
				if (isCGBDoubleSpeedEnabled() == YES)
				{
					// Double speed: glitch on any change (0->1 or 1->0)
					pGBc_display->tileSelGlitchTCycles = (oldTileSel != newTileSel) ? TWO : RESET;
				}
				else
				{
					// Single speed: glitch only when resetting (1->0)
					pGBc_display->tileSelGlitchTCycles = ((oldTileSel == ONE && newTileSel == ZERO)) ? ONE : RESET;
				}
			}
#endif

			pGBc_peripherals->LCDC.lcdControlMemory = data;

			// LCD/PPU enabled
			// https://www.reddit.com/r/Gameboy/comments/a1c8h0/what_happens_when_a_gameboy_screen_is_disabled/
			// https://forums.nesdev.org/viewtopic.php?t=12990
			if ((GETBIT(SEVEN, oldLCDC.lcdControlMemory) == ZERO) && (GETBIT(SEVEN, data) == ONE))
			{
				// LCD cannot be enabled instantaneously
				processLCDEnable();
			}
			// LCD/PPU disabled
			// https://forums.nesdev.org/viewtopic.php?f=20&t=16434#p203762
			// https://www.reddit.com/r/Gameboy/comments/a1c8h0/what_happens_when_a_gameboy_screen_is_disabled/
			// https://forums.nesdev.org/viewtopic.php?t=12990
			else if ((GETBIT(SEVEN, oldLCDC.lcdControlMemory) == ONE) && (GETBIT(SEVEN, data) == ZERO))
			{
				processLCDDisable();
			}

			RETURN;
		}

		// writing to STAT address modifies only bit 3 to 7 (0 - 2 are read only)
		if (address == STAT_ADDRESS)
		{
			pGBc_instance->GBc_state.emulatorStatus.prevSTAT = pGBc_peripherals->STAT.lcdStatusMemory;
			pGBc_instance->GBc_state.emulatorStatus.newSTAT = data;
			// https://forums.nesdev.org/viewtopic.php?f=20&t=16434#p203762
			// Now if the interrupt source status bits are modified, then we need to process the interrupt request accordingly			
			if (ROM_TYPE == ROM::GAME_BOY)
			{
				checkAllSTATInterrupts(YES);
				pGBc_instance->GBc_state.emulatorStatus.checkSTATTPlusOneCycle = YES;

				RETURN;
			}

			BYTE readOnly = pGBc_peripherals->STAT.lcdStatusMemory & 0x07;
			data &= 0x78;
			data |= readOnly;
			pGBc_peripherals->STAT.lcdStatusMemory = data;
			pGBc_peripherals->STAT.lcdStatusFields.UNUSED_07 = ONE;
			checkAllSTATInterrupts(NO);

			RETURN;
		}

		// writing to SCY
		if (address == SCY_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
#if (GB_GBC_ENABLE_CGB_SCY_WRITE_DELAY == YES)
				// Refer https://github.com/mattcurrie/mealybug-tearoom-tests/blob/master/the-comprehensive-game-boy-ppu-documentation.md#scy-ff42
				pGBc_display->cgbSCYDelayTCycles = TWO;
				pGBc_display->cgbLatchedSCY = data;
#else
				pGBc_peripherals->SCY = data;
#endif
			}
			else if (ROM_TYPE == ROM::GAME_BOY)
			{
				pGBc_peripherals->SCY = data;
			}

			RETURN;
		}

		// writing to LY register resets it
		if (address == LY_ADDRESS)
		{
			// https://forums.nesdev.org/viewtopic.php?f=20&t=16434#p203762
			// https://www.reddit.com/r/Gameboy/comments/a1c8h0/what_happens_when_a_gameboy_screen_is_disabled/

			RETURN;
		}

		// We need to check for LY=LYC as frequenctly as possible
		// https://forums.nesdev.org/viewtopic.php?f=20&t=16434#p203762
		if (address == LYC_ADDRESS)
		{
			// If old and new values are same, this doesn't give rise to rising edge on STAT line and hence no interrupt
			// Hence a check is added for the transistion
			// Refer : https://discord.com/channels/465585922579103744/465586075830845475/1331788603516522516
			if (pGBc_peripherals->LYC != data)
			{
				pGBc_peripherals->LYC = data;

				// Refer : https://forums.nesdev.org/viewtopic.php?t=16434
				compareLYToLYC(pGBc_peripherals->LY);
			}
			RETURN;
		}

		// writing to DMA register initiates DMA transfer
		if (address == DMA_ADDRESS)
		{
			// Refer : https://github.com/Gekkio/mooneye-test-suite/blob/main/acceptance/oam_dma_start.s

			pGBc_peripherals->DMA = data;
			//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_DISASSEMBLY);

			if (source == MEMORY_ACCESS_SOURCE::BESS)
			{
				RETURN;
			}

			if (pGBc_instance->GBc_state.emulatorStatus.isDMAActive && pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay == ZERO)
			{
				pGBc_instance->GBc_state.emulatorStatus.DMARestarted = YES;
				pGBc_instance->GBc_state.emulatorStatus.DMAEndDelayUponRestart = DMA_DELAY;
			}
			else
			{
				pGBc_instance->GBc_state.emulatorStatus.DMASource = (((uint16_t)pGBc_peripherals->DMA) << 8);
				// Refer : https://discord.com/channels/465585922579103744/465586075830845475/1331744415961845890
				if (ROM_TYPE == ROM::GAME_BOY && (pGBc_instance->GBc_state.emulatorStatus.DMASource & 0xFF00) >= 0xFE00)
				{
					UNSETBIT(pGBc_instance->GBc_state.emulatorStatus.DMASource, THIRTEEN);
				}
				pGBc_instance->GBc_state.emulatorStatus.isDMAActive = YES;
				pGBc_instance->GBc_state.emulatorStatus.DMABytesTransferred = RESET;
				pGBc_instance->GBc_state.emulatorStatus.DMAStartDelay = DMA_DELAY;
			}

			// Trigger the DMA STAT glitch if needed!
			// This is needed by "dma_during_pixel_transfer_check_stat_oam_scan.gb"
			OAMDMASTATModeGlitch();
			checkAllSTATInterrupts(NO);

			RETURN;
		}

		// writing to BGP register
		if (address == BGP_ADDRESS)
		{
			pGBc_peripherals->BGP = data;
		}

		// writing to KEY0
		if (address == KEY0_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				// Refer https://github.com/alloncm/MagenTests?tab=readme-ov-file#key0-cpu-mode-register-lock-after-boot
				if (pGBc_peripherals->BANK == RESET)
				{
					pGBc_peripherals->KEY0.KEY0Memory = data;
				}
			}

			RETURN;
		}

		// writing to KEY1 to change the CGB speed mode
		if (address == KEY1_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				pGBc_peripherals->KEY1.KEY1Fields.PrepareSpeedSwitch = (data & 0x01);
			}

			RETURN;
		}

		// writing to VRAM BANK switch register to change VRAM banking in CGB mode
		if (address == VRAM_BANK_SWITCH)
		{
			pGBc_peripherals->VBK = data;

			uint8_t vramBankNumber = (data & 0x01);
			setVRAMBankNumber(vramBankNumber);

			RETURN;
		}

		// writing to BANK register to unmap the BIOS from 0x0000 to 0x01000 location
		if (address == BANK_ADDRESS)
		{
			pGBc_peripherals->BANK = data;

			if (dmg_cgb_bios.biosFound == true)
			{
				if ((data & 0x01) > ZERO)
				{
					dmg_cgb_bios.unMapBios = true;

					//SETBIT(ENABLE_LOGS, LOG_VERBOSITY_DISASSEMBLY);
				}
			}

			RETURN;
		}

		// updating the HDMA source "high" address
		if (address == HDMA1_SOURCE_HIGH_REGISTER_ADDRESS)
		{
			pGBc_peripherals->HDMA1 = data;

			auto currentHDMAAddress = pGBc_instance->GBc_state.emulatorStatus.hDMASource;
			currentHDMAAddress &= 0x00FF; // zero out the upper 8 bits
			currentHDMAAddress = (((uint16_t)(data)) << 8) | currentHDMAAddress; // update the upper 8 bits
			currentHDMAAddress &= 0xFFF0; // zero out the lower 4 bits
			pGBc_instance->GBc_state.emulatorStatus.hDMASource = currentHDMAAddress;
			RETURN;
		}

		// updating the HDMA source "low" address
		if (address == HDMA2_SOURCE_LOW_REGISTER_ADDRESS)
		{
			pGBc_peripherals->HDMA2 = data;

			auto currentHDMAAddress = pGBc_instance->GBc_state.emulatorStatus.hDMASource;
			currentHDMAAddress &= 0xFF00; // zero out the lower 8 bits
			currentHDMAAddress = data | currentHDMAAddress; // update the lower 8 bits
			currentHDMAAddress &= 0xFFF0; // zero out the lower 4 bits
			pGBc_instance->GBc_state.emulatorStatus.hDMASource = currentHDMAAddress;
			RETURN;
		}

		// updating the HDMA destination "high" address
		if (address == HDMA3_DEST_HIGH_REGISTER_ADDRESS)
		{
			pGBc_peripherals->HDMA3 = data;

			auto currentHDMAAddress = pGBc_instance->GBc_state.emulatorStatus.hDMADestination;
			currentHDMAAddress &= 0x00FF; // zero out the upper 8 bits
			currentHDMAAddress = (((uint16_t)(data)) << 8) | currentHDMAAddress; // update the upper 8 bits
			currentHDMAAddress &= 0x1FF0; // zero out the everything other than bits 4 - 12
			currentHDMAAddress |= 0x8000; // add offset of 0x8000
			pGBc_instance->GBc_state.emulatorStatus.hDMADestination = currentHDMAAddress;
			RETURN;
		}

		// updating the HDMA destination "low" address
		if (address == HDMA4_DEST_LOW_REGISTER_ADDRESS)
		{
			pGBc_peripherals->HDMA4 = data;

			auto currentHDMAAddress = pGBc_instance->GBc_state.emulatorStatus.hDMADestination;
			currentHDMAAddress &= 0xFF00; // zero out the lower 8 bits
			currentHDMAAddress = data | currentHDMAAddress; // update the lower 8 bits
			currentHDMAAddress &= 0x1FF0; // zero out the everything other than bits 4 - 12
			currentHDMAAddress |= 0x8000; // add offset of 0x8000
			pGBc_instance->GBc_state.emulatorStatus.hDMADestination = currentHDMAAddress;
			RETURN;
		}

		// updating the HDMA config address
		if (address == HDMA5_CONFIG_REGISTER_ADDRESS)
		{
			if (source == MEMORY_ACCESS_SOURCE::BESS)
			{
				pGBc_peripherals->HDMA5 = data;
				RETURN;
			}

			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				pGBc_instance->GBc_state.emulatorStatus.hDMATXLength = (((data & 0x7F) + 0x01) * 0x10);

				// Refer https://gbdev.io/pandocs/CGB_Registers.html?highlight=HDMA#confirming-if-the-dma-transfer-is-active
				// Note that we dont store bit 7 in the HDMA5 register 
				// because in read operation, this mode indicates the status of the GPDMA or HDMA transfer
				// Also refer to https://discord.com/channels/465585922579103744/465586075830845475/1336632187906297876
				// i.e. bottom 7 bits are always writable!
				pGBc_peripherals->HDMA5 = (data & 0x7F);

				// NOTE: "If GPDMA is already enabled" case will not arise,
				// because if GPDMA is enabled, CPU execution is blocked and hence we will never be able to perform memory write to HDMA5 register

				// If HDMA is already enabled
				if (pGBc_instance->GBc_state.emulatorStatus.cgbDMAMode == CGB_DMA_MODE::HDMA && pGBc_instance->GBc_state.emulatorStatus.isHDMAActive == YES)
				{
					// If 7th bit is 0, need to disable the currently running HDMA transaction
					if (GETBIT(7, data) == ZERO)
					{
						pGBc_instance->GBc_state.emulatorStatus.isHDMAActive = NO;
						pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred = RESET;

						// Refer https://gbdev.io/pandocs/CGB_Registers.html?highlight=HDMA#bit-7--1--hblank-dma
						// NOTE: Stoping transfer doesn't affect HDMA 0 - 4 registers
					}
				}
				// If HDMA is not enabled
				else
				{
					// Since HDMA was currently disabled and bit 7 is again 0, enable GPDMA
					if (GETBIT(7, data) == ZERO)
					{
						pGBc_instance->GBc_state.emulatorStatus.cgbDMAMode = CGB_DMA_MODE::GPDMA;
						pGBc_instance->GBc_state.emulatorStatus.isGPDMAActive = YES;
						pGBc_instance->GBc_state.emulatorStatus.isHDMAActive = NO;
						pGBc_instance->GBc_state.emulatorStatus.isCPUExecutionBlocked = YES;
						// As mentioned in top NOTE, if GPDMA is already enabled, we can never come here
						// So, this is a new GPDMA transaction, so we reset hDMABytesTransferred as well
						pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred = RESET;
					}
					// Since HDMA was currently disabled and bit 7 is 1, enable HDMA
					else
					{
						pGBc_instance->GBc_state.emulatorStatus.cgbDMAMode = CGB_DMA_MODE::HDMA;
						pGBc_instance->GBc_state.emulatorStatus.isGPDMAActive = NO;
						pGBc_instance->GBc_state.emulatorStatus.isHDMAActive = YES;
						// New HDMA transaction as currently its disabled, so resetting hDMABytesTransferred as well 
						pGBc_instance->GBc_state.emulatorStatus.hDMABytesTransferred = RESET;

						// If LCD is disabled or currenlty in HBLANK mode, then we can block the cpu pipeline
						if (isPPULCDEnabled() == NO || getPPULCDMode() == LCD_MODES::MODE_LCD_H_BLANK)
						{
							pGBc_instance->GBc_state.emulatorStatus.isHDMAAllowedToBlockCPUPipeline = YES;
						}
					}
				}
			}

			RETURN;
		}

		// writing to BCPS; i.e. updating background palette specification
		if (address == BCPS_ADDRESS)
		{
			pGBc_peripherals->BCPS_BGPI.BCPSMemory = data;
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				setPaletteIndexForCGB(true, data);
			}
			RETURN;
		}

		// writing to BCPD; i.e. setting background palette
		if (address == BCPD_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR
				&& pGBc_display->blockCGBPalette == YES
				&& source == MEMORY_ACCESS_SOURCE::CPU)
			{
				DO_NOTHING;
			}
			else
			{
				pGBc_peripherals->BCPD_BGPD = data;
				setPaletteColorForCGB(true, data);
			}

			if (pGBc_peripherals->BCPS_BGPI.BCPSFields.AutoIncrement == SET)
			{
				uint8_t newAddress = pGBc_peripherals->BCPS_BGPI.BCPSFields.Address;
				newAddress++;
				newAddress &= 0x3F;
				pGBc_peripherals->BCPS_BGPI.BCPSFields.Address = newAddress;
				setPaletteIndexForCGB(true, pGBc_peripherals->BCPS_BGPI.BCPSMemory);
			}

			RETURN;
		}

		// writing to OCPS; i.e. updating object palette specification
		if (address == OCPS_ADDRESS)
		{
			pGBc_peripherals->OCPS_OBPI.OCPSMemory = data;
			if (ROM_TYPE == ROM::GAME_BOY_COLOR)
			{
				setPaletteIndexForCGB(false, data);
			}
			RETURN;
		}

		// writing to OCPD; i.e. setting object palette
		if (address == OCPD_ADDRESS)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR
				&& pGBc_display->blockCGBPalette == YES
				&& source == MEMORY_ACCESS_SOURCE::CPU)
			{
				DO_NOTHING;
			}
			else
			{
				pGBc_peripherals->OCPD_OBPD = data;
				setPaletteColorForCGB(false, data);
			}

			if (pGBc_peripherals->OCPS_OBPI.OCPSFields.AutoIncrement == SET)
			{
				uint8_t newAddress = pGBc_peripherals->OCPS_OBPI.OCPSFields.Address;
				newAddress++;
				newAddress &= 0x3F;
				pGBc_peripherals->OCPS_OBPI.OCPSFields.Address = newAddress;
				setPaletteIndexForCGB(false, pGBc_peripherals->OCPS_OBPI.OCPSMemory);
			}

			RETURN;
		}

		// writing to object priority mode register
		if (address == OBJECT_PRIORITY_MODE)
		{
			pGBc_peripherals->OPRI = data;
			RETURN;
		}

		// writing to WRAM BANK switch register to change WRAM banking in CGB mode
		if (address == WRAM_BANK_SWITCH)
		{
			pGBc_peripherals->SVBK = data;

			uint8_t wramBankNumber = (data & 0x07);
			if (wramBankNumber == ZERO)
			{
				wramBankNumber = ONE;
			}
			setWRAMBankNumber(wramBankNumber);

			RETURN;
		}

		// reading from HIRAM
		if (address >= HIRAM_START_ADDRESS && address <= HIRAM_END_ADDRESS)
		{
			pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
			RETURN;
		}

		// writing to IE will only set bits 0 - 4; others are always zero
		if (address == IE_ADDRESS)
		{
			pGBc_memory->GBcMemoryMap.mInterruptEnable.interruptEnableMemory = data;
			RETURN;
		}

		// Any other valid address
		if (address >= ROM_00_START_ADDRESS && address <= IE_ADDRESS)
		{
			pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[address] = data;
		}
		else
		{
			FATAL("GB-GBC memory buffer overflow");
		}
	}
}

void GBc_t::bessIoSeq(uint8_t* mmr, uint8_t size)
{
	writeRawMemory(WRAM_BANK_SWITCH, mmr[WRAM_BANK_SWITCH - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(KEY0_ADDRESS, mmr[KEY0_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(KEY1_ADDRESS, mmr[KEY1_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(P1_JOYP_ADDRESS, mmr[P1_JOYP_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(SB_ADDRESS, mmr[SB_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(SC_ADDRESS, mmr[SC_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(DIV_ADDRESS_MSB, mmr[DIV_ADDRESS_MSB - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(DIV_ADDRESS_LSB, mmr[DIV_ADDRESS_LSB - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(TIMA_ADDRESS, mmr[TIMA_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(TMA_ADDRESS, mmr[TMA_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(TAC_ADDRESS, mmr[TAC_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(NR52_ADDRESS, mmr[NR52_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	for (unsigned ii = NR10_ADDRESS; ii < NR52_ADDRESS; ii++)
	{
		BYTE data = mmr[ii - GB_GBC_IO_MEMORY_START_ADDRESS];
		if (ii == NR14_ADDRESS
			|| ii == NR24_ADDRESS
			|| ii == NR34_ADDRESS
			|| ii == NR44_ADDRESS)
		{
			data &= ~0x80; // Should not trigger a channel;
		}
		writeRawMemory(ii, data, MEMORY_ACCESS_SOURCE::BESS);
	}
	for (unsigned ii = WAVE_RAM_START_ADDRESS; ii <= WAVE_RAM_END_ADDRESS; ii++)
	{
		writeRawMemory(ii, mmr[ii - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	}
	writeRawMemory(LCDC_ADDRESS, mmr[LCDC_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(STAT_ADDRESS, mmr[STAT_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(SCY_ADDRESS, mmr[SCY_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(SCX_ADDRESS, mmr[SCX_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	//pGBc_peripherals->LY = mmr[LY_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS];
	writeRawMemory(LYC_ADDRESS, mmr[LYC_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(DMA_ADDRESS, mmr[DMA_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(BGP_ADDRESS, mmr[BGP_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(OBP0_ADDRESS, mmr[OBP0_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(OBP1_ADDRESS, mmr[OBP1_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(WX_ADDRESS, mmr[WX_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(WY_ADDRESS, mmr[WY_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(VRAM_BANK_SWITCH, mmr[VRAM_BANK_SWITCH - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(HDMA1_SOURCE_HIGH_REGISTER_ADDRESS, mmr[HDMA1_SOURCE_HIGH_REGISTER_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(HDMA2_SOURCE_LOW_REGISTER_ADDRESS, mmr[HDMA2_SOURCE_LOW_REGISTER_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(HDMA3_DEST_HIGH_REGISTER_ADDRESS, mmr[HDMA3_DEST_HIGH_REGISTER_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(HDMA4_DEST_LOW_REGISTER_ADDRESS, mmr[HDMA4_DEST_LOW_REGISTER_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	//writeRawMemory(HDMA5_CONFIG_REGISTER_ADDRESS, mmr[HDMA4_DEST_LOW_REGISTER_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	//writeRawMemory(BCPD_ADDRESS, mmr[BCPD_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	//writeRawMemory(OCPD_ADDRESS, mmr[OCPD_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(BCPS_ADDRESS, mmr[BCPS_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(OCPS_ADDRESS, mmr[BCPS_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(OBJECT_PRIORITY_MODE, mmr[OBJECT_PRIORITY_MODE - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
	writeRawMemory(IF_ADDRESS, mmr[IF_ADDRESS - GB_GBC_IO_MEMORY_START_ADDRESS], MEMORY_ACCESS_SOURCE::BESS);
}

void GBc_t::stackPush(BYTE data)
{
	(pGBc_registers->sp)--;
	writeRawMemory(pGBc_registers->sp, data, MEMORY_ACCESS_SOURCE::CPU);
}

BYTE GBc_t::stackPop()
{
	BYTE popedData = readRawMemory(pGBc_registers->sp, MEMORY_ACCESS_SOURCE::CPU);
	(pGBc_registers->sp)++;
	RETURN popedData;
}

void GBc_t::processZeroFlag
(
	byte value
)
{
	if ((value & 0xFF) == 0x00)
	{
		pGBc_flags->FZERO = ONE;
	}
	else
	{
		pGBc_flags->FZERO = ZERO;
	}
}

void GBc_t::processFlagsForLogicalOperation
(
	byte value,
	FLAG isOperationAND
)
{
	pGBc_flags->FCARRY = ZERO;
	pGBc_flags->FSUB = ZERO;

	if (isOperationAND == true)
	{
		pGBc_flags->FHALFCARRY = ONE;
	}
	else
	{
		pGBc_flags->FHALFCARRY = ZERO;
	}

	processZeroFlag(value);
}

void GBc_t::processFlagsFor8BitAdditionOperation
(
	byte value1,
	byte value2,
	FLAG includeCarryInOperation,
	FLAG affectsCarryFlag
)
{
	auto unsignedResult1 = 0;
	auto unsignedResult2 = 0;

	// 8 bit unsigned addition
	if (includeCarryInOperation == true)
	{
		unsignedResult1 =
			(uint8_t)value1
			+ (uint8_t)value2
			+ (uint8_t)((pGBc_flags->FCARRY == 0) ? ZERO : ONE);
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
			+ (uint8_t)((pGBc_flags->FCARRY == 0) ? ZERO : ONE);

	}
	else
	{
		unsignedResult2 =
			((uint8_t)value1 & 0x0F)
			+ ((uint8_t)value2 & 0x0F);
	}

	// ARITHMETIC OPERATION TYPE Flag

	pGBc_flags->FSUB = ZERO;

	// ZERO flag

	if ((unsignedResult1 & 0xFF) == ZERO)
	{
		pGBc_flags->FZERO = ONE;
	}
	else
	{
		pGBc_flags->FZERO = ZERO;
	}

	// CARRY Flag

	if (affectsCarryFlag == true)
	{
		if (unsignedResult1 > 255)
		{
			pGBc_flags->FCARRY = ONE;
		}
		else
		{
			pGBc_flags->FCARRY = ZERO;
		}
	}

	// HALF CARRY Flag

	if (unsignedResult2 & 0x10)
	{
		pGBc_flags->FHALFCARRY = ONE;
	}
	else
	{
		pGBc_flags->FHALFCARRY = ZERO;
	}
}

void GBc_t::processFlagsFor16BitAdditionOperation
(
	uint16_t value1,
	uint16_t value2,
	FLAG includeCarryInOperation,
	FLAG setZero
)
{
	auto unsignedResult1 = 0;
	auto unsignedResult2 = 0;

	// 16 bit unsigned addition
	if (includeCarryInOperation == true)
	{
		unsignedResult1 =
			(uint16_t)value1
			+ (uint16_t)value2
			+ (uint16_t)((pGBc_flags->FCARRY == 0) ? ZERO : ONE);
	}
	else
	{
		unsignedResult1 =
			(uint16_t)value1
			+ (uint16_t)value2;
	}

	// 12 bit unsigned addition
	if (includeCarryInOperation == true)
	{
		unsignedResult2 =
			((uint16_t)value1 & 0x0FFF)
			+ ((uint16_t)value2 & 0x0FFF)
			+ (uint16_t)((pGBc_flags->FCARRY == 0) ? ZERO : ONE);

	}
	else
	{
		unsignedResult2 =
			((uint16_t)value1 & 0x0FFF)
			+ ((uint16_t)value2 & 0x0FFF);
	}

	// ARITHMETIC OPERATION TYPE Flag

	pGBc_flags->FSUB = ZERO;

	if (setZero == true)
	{
		// ZERO flag

		if ((unsignedResult1 & 0xFFFF) == ZERO)
		{
			pGBc_flags->FZERO = ONE;
		}
		else
		{
			pGBc_flags->FZERO = ZERO;
		}
	}

	// CARRY Flag

	if (unsignedResult1 > 65535)
	{
		pGBc_flags->FCARRY = ONE;
	}
	else
	{
		pGBc_flags->FCARRY = ZERO;
	}

	// HALF CARRY Flag

	if (unsignedResult2 & 0x1000)
	{
		pGBc_flags->FHALFCARRY = ONE;
	}
	else
	{
		pGBc_flags->FHALFCARRY = ZERO;
	}
}

void GBc_t::processFlagsFor8BitSubtractionOperation
(
	byte value1,
	byte value2,
	FLAG includeCarryInOperation,
	FLAG affectsCarryFlag
)
{
	auto result1 = 0;
	auto result2 = 0;

	// 8 bit subtraction
	if (includeCarryInOperation == true)
	{
		result1 =
			value1
			- value2
			- (uint8_t)((pGBc_flags->FCARRY == 0) ? ZERO : ONE);
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
			- (uint8_t)((pGBc_flags->FCARRY == 0) ? ZERO : ONE);

	}
	else
	{
		result2 =
			(value1 & 0x0F)
			- (value2 & 0x0F);
	}

	// ARITHMETIC OPERATION TYPE Flag

	pGBc_flags->FSUB = ONE;

	// ZERO flag

	if ((result1 & 0xFF) == ZERO)
	{
		pGBc_flags->FZERO = ONE;
	}
	else
	{
		pGBc_flags->FZERO = ZERO;
	}

	// CARRY Flag

	if (affectsCarryFlag == true)
	{
		if ((includeCarryInOperation == true) && pGBc_flags->FCARRY)
		{
			if (value2 >= value1)
			{
				pGBc_flags->FCARRY = ONE;
			}
			else
			{
				pGBc_flags->FCARRY = ZERO;
			}
		}
		else
		{
			if (value2 > value1)
			{
				pGBc_flags->FCARRY = ONE;
			}
			else
			{
				pGBc_flags->FCARRY = ZERO;
			}
		}
	}

	// HALF CARRY Flag

	if (result2 & 0x10)
	{
		pGBc_flags->FHALFCARRY = ONE;
	}
	else
	{
		pGBc_flags->FHALFCARRY = ZERO;
	}
}

void GBc_t::processFlagsFor16BitSubtractionOperation
(
	uint16_t value1,
	uint16_t value2,
	FLAG includeCarryInOperation,
	FLAG affectsCarryFlag
)
{
	auto result1 = 0;
	auto result2 = 0;

	// 16 bit subtraction
	if (includeCarryInOperation == true)
	{
		result1 =
			value1
			- value2
			- (uint16_t)((pGBc_flags->FCARRY == 0) ? ZERO : ONE);
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
			- (uint16_t)((pGBc_flags->FCARRY == 0) ? ZERO : ONE);

	}
	else
	{
		result2 =
			(value1 & 0x0FFF)
			- (value2 & 0x0FFF);
	}

	// ARITHMETIC OPERATION TYPE Flag

	pGBc_flags->FSUB = ONE;

	// ZERO flag

	if ((result1 & 0xFFFF) == ZERO)
	{
		pGBc_flags->FZERO = ONE;
	}
	else
	{
		pGBc_flags->FZERO = ZERO;
	}

	// CARRY Flag

	if (affectsCarryFlag == true)
	{
		if ((includeCarryInOperation == true) && pGBc_flags->FCARRY)
		{
			if (value2 >= value1)
			{
				pGBc_flags->FCARRY = ONE;
			}
			else
			{
				pGBc_flags->FCARRY = ZERO;
			}
		}
		else
		{
			if (value2 > value1)
			{
				pGBc_flags->FCARRY = ONE;
			}
			else
			{
				pGBc_flags->FCARRY = ZERO;
			}
		}
	}

	// HALF CARRY Flag

	if (result2 & 0x1000)
	{
		pGBc_flags->FHALFCARRY = ONE;
	}
	else
	{
		pGBc_flags->FHALFCARRY = ZERO;
	}
}

void GBc_t::processFlagFor0xE8And0xF8
(
	byte value1,
	byte value2
)
{
	auto Result1 = 0;
	auto Result2 = 0;

	// 8 bit unsigned addition

	Result1 =
		value1
		+ value2;

	// 4 bit signed addition

	Result2 =
		(value1 & 0x0F)
		+ (value2 & 0x0F);

	// ARITHMETIC OPERATION TYPE Flag

	pGBc_flags->FSUB = ZERO;

	// ZERO flag

	pGBc_flags->FZERO = ZERO;

	// CARRY Flag

	if (Result1 > 255)
	{
		pGBc_flags->FCARRY = ONE;
	}
	else
	{
		pGBc_flags->FCARRY = ZERO;
	}

	// HALF CARRY Flag

	if (Result2 & 0x10)
	{
		pGBc_flags->FHALFCARRY = ONE;
	}
	else
	{
		pGBc_flags->FHALFCARRY = ZERO;
	}
}

void GBc_t::processUnusedFlags(BYTE result)
{
	pGBc_flags->ZEROTH = result;
	pGBc_flags->FIRST = result;
	pGBc_flags->SECOND = result;
	pGBc_flags->THIRD = result;
}

void GBc_t::processUnusedJoyPadBits(BYTE value)
{
	pGBc_peripherals->P1_JOYP.joyPadFields.JP_SPARE_06 = value;
	pGBc_peripherals->P1_JOYP.joyPadFields.JP_SPARE_07 = value;
}

void GBc_t::processUnusedIFBits(BYTE value)
{
	pGBc_peripherals->IF.interruptRequestFields.NO_INT05 = value;
	pGBc_peripherals->IF.interruptRequestFields.NO_INT06 = value;
	pGBc_peripherals->IF.interruptRequestFields.NO_INT07 = value;
}

FLAG GBc_t::processSOC()
{
	FLAG status = true;

	// In Stop Mode!
	if (pGBc_instance->GBc_state.emulatorStatus.isCPUStopped == YES) MASQ_UNLIKELY
	{
		cpuTickM();
		if ((pGBc_peripherals->P1_JOYP.joyPadMemory & 0x0F) != 0x0F)
		{
			pGBc_instance->GBc_state.emulatorStatus.isCPUStopped = NO;
			TODO("Tick 4-T DMA cycles when existing stop mode (source : Sameboy) ");
			pGBc_display->blockOAMR = NO;
			pGBc_display->blockOAMW = NO;
			pGBc_display->blockVramR = NO;
			pGBc_display->blockVramW = NO;
			pGBc_display->blockCGBPalette = NO;
			TODO("Tick 8-T cycles when existing stop mode (source : Sameboy)");
		}
	}
	// Not In Stop Mode!
	else
	{
		// Refer : https://www.reddit.com/r/EmuDev/comments/7206vh/sameboy_now_correctly_emulates_pinball_deluxe/
		// To simulate the T cycle accuracy needed for reads of IME, IF and IE and its effect on HALT mentioned in above link,
		// we have gbCpuTick2T
		// Source : Sameboy
		// Another source of documentation for this is from NesDev wiki mentioned below
		// Refer : https://forums.nesdev.org/viewtopic.php?p=240034#p240034
		// Refer : https://discord.com/channels/465585922579103744/465586075830845475/529008578065989642
		CPUTODO("Find more documentation for the need of gbCpuTick2T?");

		if (pGBc_instance->GBc_state.emulatorStatus.isCPUHalted == YES
			&& ROM_TYPE == ROM::GAME_BOY
			&& pGBc_instance->GBc_state.emulatorStatus.isCPUJustHalted == NO)
		{
			gbCpuTick2T(NO);
		}

		auto effectiveInterruptQ = isInterruptReadyToBeServed();

		if (pGBc_instance->GBc_state.emulatorStatus.isCPUHalted == YES)
		{
			if (ROM_TYPE == ROM::GAME_BOY_COLOR
				|| pGBc_instance->GBc_state.emulatorStatus.isCPUJustHalted == YES)
			{
				cpuTickM();
			}
			else
			{
				gbCpuTick2T(YES);
			}
		}

		pGBc_instance->GBc_state.emulatorStatus.isCPUJustHalted = NO;

		auto effectiveIME = pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn;
		if (pGBc_instance->GBc_state.emulatorStatus.eiEnState == EI_ENABLE_STATE::EI_TO_BE_ENABLED)
		{
			pGBc_instance->GBc_state.emulatorStatus.interruptMasterEn = ENABLED;
			pGBc_instance->GBc_state.emulatorStatus.eiEnState = EI_ENABLE_STATE::NOTHING_TO_BE_DONE;
		}

		if (effectiveInterruptQ == YES)
		{
			pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = NO;
		}

		// Interrupts are checked when we are about to fetch next instruction
		// Refer : https://www.reddit.com/r/EmuDev/comments/1bm3mbc/looking_for_some_highlevel_guidance_regarding_gbc/
		handleInterruptsIfApplicable(effectiveIME, effectiveInterruptQ);

		if (pGBc_instance->GBc_state.emulatorStatus.isCPUHalted == NO)
		{
			// If CPU is blocked, we just have to tick the clock (don't run the cpu pipeline)
			// Currently, "isCPUExecutionBlocked" can only be set because of GPDMA or HDMA in GAME_BOY_COLOR mode
			// Even interrupt cannot override the GPDMA or HDMA transaction, hence the while loop!
			while (pGBc_instance->GBc_state.emulatorStatus.isCPUExecutionBlocked == YES)
			{
				cpuTickM();
			}

			runCPUPipeline();
		}
	}

	RETURN status;
}

void GBc_t::dumpCpuStateToConsole()
{
	LOG("--------------------------------------------");
	LOG("a register: \t\t%02x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.a);
	LOG("f register: \t\t%02x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.f.flagMemory);
	LOG("FZERO: \t\t\t%01x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.f.flagFields.FZERO);
	LOG("FSUB: \t\t\t%01x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.f.flagFields.FSUB);
	LOG("FHALFCARRY: \t\t%01x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.f.flagFields.FHALFCARRY);
	LOG("FCARRY: \t\t%01x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.f.flagFields.FCARRY);
	LOG("THIRD: \t\t\t%01x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.f.flagFields.THIRD);
	LOG("SECOND: \t\t%01x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.f.flagFields.SECOND);
	LOG("FIRST: \t\t\t%01x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.f.flagFields.FIRST);
	LOG("ZEROTH: \t\t%01x", pGBc_instance->GBc_state.registers.af.aAndFRegisters.f.flagFields.ZEROTH);
	LOG("b register: \t\t%02x", pGBc_instance->GBc_state.registers.bc.bAndCRegisters.b);
	LOG("c register: \t\t%02x", pGBc_instance->GBc_state.registers.bc.bAndCRegisters.c);
	LOG("d register: \t\t%02x", pGBc_instance->GBc_state.registers.de.dAndERegisters.d);
	LOG("e register: \t\t%02x", pGBc_instance->GBc_state.registers.de.dAndERegisters.e);
	LOG("h register: \t\t%02x", pGBc_instance->GBc_state.registers.hl.hAndLRegisters.h);
	LOG("l register: \t\t%02x", pGBc_instance->GBc_state.registers.hl.hAndLRegisters.l);
	LOG("(BC): \t\t\t%02x", readRawMemory(cpuReadRegister(REGISTER_TYPE::RT_BC), MEMORY_ACCESS_SOURCE::DEBUG_PORT));
	LOG("(DE): \t\t\t%02x", readRawMemory(cpuReadRegister(REGISTER_TYPE::RT_DE), MEMORY_ACCESS_SOURCE::DEBUG_PORT));
	LOG("(HL): \t\t\t%02x", readRawMemory(cpuReadRegister(REGISTER_TYPE::RT_HL), MEMORY_ACCESS_SOURCE::DEBUG_PORT));
	LOG("(SP): \t\t\t%02x", readRawMemory(cpuReadRegister(REGISTER_TYPE::RT_SP), MEMORY_ACCESS_SOURCE::DEBUG_PORT));
	LOG("program counter: \t%04x", pGBc_instance->GBc_state.registers.pc);
	LOG("stack pointer: \t\t%04x", pGBc_instance->GBc_state.registers.sp);
	LOG("--------------------------------------------");
}

void GBc_t::unimplementedInstruction()
{
	dumpCpuStateToConsole();
	CPUWARN("CPU Panic; unknown opcode! %02X\n", pGBc_cpuInstance->opcode);
}
#pragma endregion SM83_DEFINITIONS
