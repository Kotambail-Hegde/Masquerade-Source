#pragma once

#pragma region REFERENCES
#pragma endregion REFERENCES

#pragma region INCLUDES
//
#include "helpers.h"
//
#include "abstractEmulation.h"
#pragma endregion INCLUDES

#pragma region MACROS
#pragma region WIP
#pragma endregion WIP
#define GB_GBC_FPS										59.73f
#define RESET_TICK										FALSE
#define INVALID_TICK									NO
#define VALID_TICK										YES
#define EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC			48000.0f
#ifdef __EMSCRIPTEN__
#define AUDIO_BUFFER_SIZE_FOR_GB_GBC					(CEIL((EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC / GB_GBC_FPS)))  // 32
#else
#define AUDIO_BUFFER_SIZE_FOR_GB_GBC					(CEIL((EMULATED_AUDIO_SAMPLING_RATE_FOR_GB_GBC / GB_GBC_FPS)))
#endif
#define PIXEL_FIFO_SIZE_FOR_GB_GBC						(SIXTEEN)
// Gameboy Camera
#define GBCAM_SENSOR_EXTRA_LINES						(8)
#define GBCAM_SENSOR_W									(128)
#define GBCAM_SENSOR_H									(112 + GBCAM_SENSOR_EXTRA_LINES)
#define GBCAM_W											(128)
#define GBCAM_H											(112)
#pragma endregion MACROS

#pragma region CORE
class GBcPrinterEngine_t
{
public:

	void reset();
	void startPacket();
	FLAG receiveBitFromGB(BIT bitReceived);
	FLAG sendBitToGB(BIT* bitToSend);
	void tick();

private:

	BYTE txByte = ZERO;
	BYTE rxByte = ZERO;

	uint8_t txBitCount = ZERO;
	uint8_t rxBitCount = ZERO;

	uint8_t compression = ZERO;

	uint16_t packetLength = ZERO;
	uint16_t packetIndex = ZERO;

	uint16_t checksum = ZERO;
	uint16_t receivedChecksum = ZERO;

	BYTE status = ZERO;

	std::vector<BYTE> imageBuffer;

	// PRINT's own packet carries 4 argument bytes (sheets, margins, palette,
	// exposure) through the same GB_PRINTER_DATA state as tile data -- these
	// must NOT end up mixed into imageBuffer.
	struct
	{
		BYTE numSheets = ZERO;
		BYTE margins = ZERO;
		BYTE palette = ZERO;
		BYTE exposure = ZERO;
	} printArgs;

	// One entry per printed image this session, so multiple prints can be
	// reviewed side by side instead of each replacing the last.

	// A roll's content, in order. Gap blocks store no baked size -- their
	// pixel height is always computed from the CURRENT cosmeticGapPx at
	// composite time, so every gap in every roll stays uniform even if the
	// slider changes mid-session (recompositeRoll() rebuilds everything).
	struct RollBlock
	{
		bool isGap = false;
		std::vector<BYTE> imageRgba; // only populated when isGap == false
		int imageHeightPx = 0;       // only populated when isGap == false
	};

	struct PrintedImageWindow
	{
		uint64_t id = ZERO; // stable identity independent of vector index (index shifts on erase)
		GLuint textureId = 0;
		int width = 0;
		int height = 0; // derived: recomputed by recompositeRoll(), not authoritative
		std::string title;
		bool open = true; // user can close the window == "tear off the paper"; entry gets pruned next frame
		bool savedToDisk = false; // tracks whether "Save PNG" has been clicked, for button label/state
		std::string savedPath; // absolute path, shown in the UI once saved
		float displayScale = 1.0f; // per-window zoom; only ever changed uniformly via +/- buttons
		std::vector<RollBlock> blocks; // source of truth for this roll's content
		std::vector<BYTE> rgbaPixels; // composited cache, rebuilt by recompositeRoll()
		uint32_t printCountInRoll = ZERO;
		bool isJamSource = false; // true if THIS roll hitting its cap is what caused the current jam
	};

	// The roll currently "loaded in the printer," receiving new prints.
	// -1 means no active roll -- the next PRINT starts a fresh one. Tracked
	// by id (not vector index), since erase-on-close can shift indices.
	int64_t activeRollId = -1;
	uint64_t nextRollId = ZERO;

	static constexpr int ROLL_WIDTH_PX = 160; // fixed paper width: 20 tiles * 8px, matches the printer's 20-tile buffer

	// Nintendo's official paper spec: "up to 180 prints per roll." Real
	// paper physically runs out; we mimic that by auto tearing off once hit
	// so the next print starts a new roll instead of growing forever.
	static constexpr uint32_t MAX_PRINTS_PER_ROLL = 180;

	// NOT an authoritative hardware constant -- Pan Docs describes the
	// margin byte's nibbles as "feed before/after" but doesn't document an
	// exact pixel/line conversion I could confirm. Runtime-tunable (see the
	// "GB Printer Settings" panel in drawImGuiWindows) so it can be dialed
	// in against a real printer or a known-accurate emulator, rather than
	// a compiled-in guess.
	int marginPixelsPerUnit = 8;

	// True once the current roll has hit MAX_PRINTS_PER_ROLL. Persists
	// across INIT (real hardware can't materialize new paper just because
	// the game re-initializes the link) -- only clears when the person
	// closes the jammed roll's window, i.e. "changes the paper."
	bool isPaperJammed = false;

	std::vector<PrintedImageWindow> printedImageWindows;

	// how many tick() calls remain before the current print job reports "done"
	uint32_t printTicksRemaining = ZERO;
	static constexpr uint32_t PRINT_DURATION_TICKS = 60; // ~1 second at 60 ticks/sec, tune to taste

	// No documented mm/px-per-unit value exists for the margin nibbles, and
	// real ROMs use them inconsistently (per shonumi's GBE+ writeup and
	// Raphael-Boichot's ~110-game hardware-capture decoder project) -- so
	// rather than fake pixel-accurate spacing from an unconfirmed number,
	// margin-after != 0 is just treated as "this print ends a logical
	// image." Gap SIZE below is a display preference, not a hardware value
	// -- tunable purely for your own readability, same category as zoom.
	// Read by recompositeRoll() every time a roll is rebuilt, so changing
	// this always applies uniformly across the whole roll (see RollBlock).
	int cosmeticGapPx = 8;

	// Member variables for tracking RLE decompression state across incoming bytes
	uint8_t runLength = 0;      // Remaining bytes to process in the active run
	bool isCompressedRun = false; // true = repeated single byte; false = raw uncompressed stream

	enum class GB_PRINTER_STATE
	{
		GB_PRINTER_NONE,
		GB_PRINTER_MAGIC_88,
		GB_PRINTER_MAGIC_33,
		GB_PRINTER_COMMAND,
		GB_PRINTER_COMPRESSION,
		GB_PRINTER_LENGTH_LOW,
		GB_PRINTER_LENGTH_HIGH,
		GB_PRINTER_DATA,
		GB_PRINTER_CHECKSUM_LOW,
		GB_PRINTER_CHECKSUM_HIGH
	};

	enum GB_PRINTER_COMMAND : BYTE
	{
		INIT = 0x01,
		PRINT = 0x02,
		DATA = 0x04,
		STATUS = 0x0F
	};

	enum GB_PRINTER_STATUS : BYTE
	{
		STATUS_CHECKSUM_ERROR = 0x01,
		STATUS_PRINTING = 0x02,
		STATUS_IMAGE_FULL = 0x04,
		STATUS_UNPROCESSED = 0x08,
		STATUS_PACKET_ERROR = 0x10,
		STATUS_PAPER_JAM = 0x20,
		STATUS_OTHER_ERROR = 0x40,
		STATUS_LOW_BATTERY = 0x80
	};

	enum GB_PRINTER_RESPONSE : BYTE
	{
		STATUS_RESPONSE = 0x81
	};

	enum class GB_PRINTER_TX_STATE
	{
		GB_PRINTER_TX_NONE,
		GB_PRINTER_TX_RESPONSE,
		GB_PRINTER_TX_STATUS
	};

	GB_PRINTER_STATE state = GB_PRINTER_STATE::GB_PRINTER_NONE;
	GB_PRINTER_COMMAND command = GB_PRINTER_COMMAND::INIT;
	GB_PRINTER_TX_STATE txState = GB_PRINTER_TX_STATE::GB_PRINTER_TX_NONE;

	void processReceivedByte(BYTE dataReceived);
	void dispatchCommand();
	std::vector<BYTE> decodeTilesToRgba(const std::vector<BYTE>& pixelData, uint32_t& outHeightPx);
	void appendPrintToRoll(const std::vector<BYTE>& pixelData);
	void appendBlankFeedToRoll();
	void regenerateRollTexture(PrintedImageWindow& window);
	void recompositeRoll(PrintedImageWindow& window);

public:

	// Call once per frame from your existing ImGui render pass (wherever
	// your other debug/tool windows get drawn).
	void drawImGuiWindows();

private:

	bool savePrintedImageAsPng(PrintedImageWindow& window);
	PrintedImageWindow* findWindowById(uint64_t id);
};

class GBc4PlayerAdapterEngine_t
{
public:
	GBc4PlayerAdapterEngine_t();
	~GBc4PlayerAdapterEngine_t();
	void reset();
	void tick();
};

class GBcCameraEngine_t
{
public:
	~GBcCameraEngine_t()
	{
		deinitialize();
	}

	void reset(void* camera)
	{
		gbCamera = static_cast<SDL_Camera*>(camera);
		initialized = (gbCamera != nullptr) ? YES : NO;
	}

	void deinitialize()
	{
		if (scaledFrame != nullptr)
		{
			SDL_DestroySurface(scaledFrame);
			scaledFrame = nullptr;
		}
		gbCamera = nullptr;
		initialized = NO;
	}

	// Returns YES if webcamOutput was refreshed this call, NO if no new
	// frame was available (normal — SDL_AcquireCameraFrame is non-blocking)
	// or on failure. Prior contents of webcamOutput are left untouched
	// on a NO.
	FLAG captureFrame();

	MASQ_INLINE FLAG isCameraInitialized()
	{
		RETURN initialized;
	}
	MASQ_INLINE SDL_Camera* getGbCamera()
	{
		RETURN gbCamera;
	}

private:
	FLAG initialized = NO;

	SDL_Camera* gbCamera = nullptr;

	// Owned by us. Created once, reused every frame, freed in deinitialize().
	SDL_Surface* scaledFrame = nullptr;

	FLAG ensureScaledFrame()
	{
		if (scaledFrame != nullptr)
		{
			RETURN YES;
		}
		scaledFrame = SDL_CreateSurface(GBCAM_SENSOR_W, GBCAM_SENSOR_H, SDL_PIXELFORMAT_RGBA32);
		if (scaledFrame == nullptr)
		{
			DEBUG("GBcCameraEngine_t: SDL_CreateSurface failed: %s", SDL_GetError());
			RETURN NO;
		}
		RETURN YES;
	}

public:
	// Webcam image (raw, luminance)
	int preprocessed[GBCAM_SENSOR_W][GBCAM_SENSOR_H];
	// Image processed by sensor chip
	int postprocessed[GBCAM_SENSOR_W][GBCAM_SENSOR_H];
};

class GBcBarcodeEngine_t
{
public:

	void reset();

	// Called by GBc_t::detectSerialDevice() the instant it identifies this
	// peripheral (the GB has just sent the handshake's first two bytes,
	// 0x10 and 0x07 -- that's the 0x1007 magic check). Those two bytes never
	// reached sendBitToGB/receiveBitFromGB below -- this engine isn't
	// invoked until serialDevice == GB_BARCODE_BOY, which only becomes true
	// AFTER detection completes -- so this fast-forwards RX/TX state to
	// line up with byte 3 of the real handshake instead of restarting from
	// byte 1. Mirrors GBcPrinterEngine_t::startPacket() doing the same for
	// the printer's 0x88/0x33 magic bytes.
	void startHandshake();

	FLAG receiveBitFromGB(BIT bitReceived);
	FLAG sendBitToGB(BIT* bitToSend);
	void tick();
	void barcodeScan(const BYTE* barcode);
	FLAG isClocking()
	{
		RETURN clocking;
	}

private:

	BYTE rxByte = ZERO;

	uint8_t txBitCount = ZERO;
	uint8_t rxBitCount = ZERO;

	BYTE status = ZERO;

	BYTE barcode[13];
	BYTE txData[30];
	BYTE txDataIndex = ZERO;

	FLAG clocking = NO;

	// Fixed 4-byte handshake exchange -- reverse-engineered by Shonumi
	// (shonumi.github.io/articles/art7.html) and verified against coffee-gb's
	// tested implementation (github.com/trekawek/coffee-gb, commit 4789306):
	// the GB, as clock master, always sends 0x10 0x07 0x10 0x07; the Barcode
	// Boy always replies 0xFF 0xFF 0x10 0x07. The game only actually checks
	// the reply's last two bytes, but real hardware (and coffee-gb) sends
	// the same 4 bytes every single time -- every handshake, not just the
	// first -- so this does too.
	static constexpr BYTE HANDSHAKE_EXPECT_FROM_GB[4] = { 0x10, 0x07, 0x10, 0x07 };
	static constexpr BYTE HANDSHAKE_REPLY_TO_GB[4] = { 0xFF, 0xFF, 0x10, 0x07 };
	uint8_t handshakeIndex = ZERO;

	// RX-side self-detection of the GB's own handshake bytes -- mirrors
	// GBcPrinterEngine_t's GB_PRINTER_NONE/MAGIC_33 pattern exactly.
	// GBc_t::detectSerialDevice() only ever runs ONCE per session (it stops
	// dead as soon as serialDevice != GB_LINK_CABLE), so every handshake
	// after the first -- the re-handshake this device requires after every
	// scan -- has to be recognised by THIS engine from the raw bytes the GB
	// sends, the same way the printer re-detects 0x88/0x33 itself for its
	// 2nd+ print job.
	enum class GB_BARCODE_STATE
	{
		GB_BARCODE_NONE,         // expecting byte 1 (0x10)
		GB_BARCODE_MAGIC_DUMMY1, // byte 1 confirmed, expecting byte 2 (0x07)
		GB_BARCODE_MAGIC_DUMMY2, // byte 2 confirmed, expecting byte 3 (0x10)
		GB_BARCODE_MAGIC_10,     // byte 3 confirmed, expecting byte 4 (0x07)
		GB_BARCODE_MAGIC_07      // full handshake confirmed
	};

	enum class GB_BARCODE_TX_STATE
	{
		GB_BARCODE_TX_HANDSHAKE, // driving HANDSHAKE_REPLY_TO_GB[handshakeIndex]
		GB_BARCODE_TX_SEND       // driving txData[txDataIndex]
	};

	GB_BARCODE_STATE state = GB_BARCODE_STATE::GB_BARCODE_NONE;
	GB_BARCODE_TX_STATE txState = GB_BARCODE_TX_STATE::GB_BARCODE_TX_HANDSHAKE;

	void processReceivedByte(BYTE dataReceived);
};

class GBc_t : public abstractEmulation_t
{
#pragma region INFRASTRUCTURE_DECLARATIONS

public:

	std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom;
	const float myFPS = (float)GB_GBC_FPS;

public:

	uint32_t y_offset = 0;
	uint32_t x_offset = 0;
	static const uint32_t screen_height = 144;
	static const uint32_t screen_width = 160;
	static const uint32_t pixel_height = 2;
	static const uint32_t pixel_width = 2;
	static const uint32_t debugger_screen_height = 560;
	static const uint32_t debugger_screen_width = 456; // 880
	static const uint32_t debugger_pixel_height = 1;
	static const uint32_t debugger_pixel_width = 1;
	const char* NAME = "GB-GBC";

private:

	MasqConfig_t pt;

#ifndef __RPI_PICO__
private:

	uint8_t const SST_ROMS = TWO;

private:

	uint32_t profiler_FrameRate;
	uint64_t functionID;

private:

	CheatEngine_t* ceGBGBC;
#endif // !__RPI_PICO__
#pragma endregion INFRASTRUCTURE_DECLARATIONS

#pragma region SM83_DECLARATION
private:

	enum class REGISTER_TYPE	// register_op_type
	{
		RT_A,				// 0
		RT_F,				// 1
		RT_B,				// 2
		RT_C,				// 3
		RT_D,				// 4
		RT_E,				// 5
		RT_H,				// 6
		RT_L,				// 7
		RT_PC,				// 8
		RT_SP,				// 9
		RT_AF,				// 10
		RT_BC,				// 11
		RT_DE,				// 12
		RT_HL,				// 13
		RT_IE,				// 14
		RT_TOTAL,			// TOTAL = 15
		RT_NONE
	};

	enum class POINTER_TYPE
	{
		RT_M_HL,			// 0
		RT_M_DE,			// 1
		RT_M_BC,			// 2
		RT_M_TOTAL,			// TOTAL = 3
		RT_M_NONE
	};

	enum class CPU_TICK_TYPE
	{
		READ_WRITE,
		DUMMY
	};

	enum class OAM_ACCESS_TYPE
	{
		READ,
		WRITE
	};

	enum class SPECULATION_ORDER
	{
		NONE,
		FIRST,
		SECOND
	};

private:

PACK_BEGIN

	typedef struct
	{
		uint8_t opcode;
		uint8_t previousOpcode;
	} cpu_t;

	typedef struct
	{
		uint8_t ZEROTH : 1; // bit  0
		uint8_t FIRST : 1; // bit  1
		uint8_t SECOND : 1; // bit  2
		uint8_t THIRD : 1; // bit  3
		uint8_t FCARRY : 1; // bit  4
		uint8_t FHALFCARRY : 1; // bit  5	
		uint8_t FSUB : 1; // bit  6
		uint8_t FZERO : 1; // bit  7
	} flagFields_t;

	typedef union
	{
		flagFields_t flagFields;
		uint8_t flagMemory;
	} flag_t;

	typedef struct
	{
		flag_t f;				// 0 - 7
		uint8_t a;				// 8 - 15
	} aAndFRegisters_t;

	typedef union
	{
		aAndFRegisters_t aAndFRegisters;
		uint16_t af_u16memory;	// <-a->|<-f->
	} af_t;

	typedef struct
	{
		uint8_t c;				// 0 - 7
		uint8_t b;				// 8 - 15
	} bAndCRegisters_t;

	typedef union
	{
		bAndCRegisters_t bAndCRegisters;
		uint16_t bc_u16memory;	// <-b->|<-c->
	} bc_t;

	typedef struct
	{
		uint8_t e;				// 0 - 7
		uint8_t d;				// 8 - 15
	} dAndERegisters_t;

	typedef union
	{
		dAndERegisters_t dAndERegisters;
		uint16_t de_u16memory;	// <-d->|<-e->
	} de_t;

	typedef struct
	{
		uint8_t l;				// 0 - 7
		uint8_t h;				// 8 - 15
	} hAndLRegisters_t;

	typedef union
	{
		hAndLRegisters_t hAndLRegisters;
		uint16_t hl_u16memory;	// <-h->|<-l->
	} hl_t;

	typedef struct
	{
		af_t af;
		bc_t bc;
		de_t de;
		hl_t hl;
		uint16_t pc;
		uint16_t sp;
	} registers_t;

#pragma endregion SM83_DECLARATION

#pragma region EMULATION_DECLARATIONS
private:

	bios_t dmg_cgb_bios;

private:

	enum GBC_MODE : BYTE
	{
		CGB,
		DMG_MGB,
		PGB0,
		PGB1
	};

	enum class MEMORY_ACCESS_SOURCE
	{
		DEBUG_PORT,
		CPU,
		PPU,
		APU,
		OAMDMA,
		GPDMA,
		HDMA,
		BESS
	};

	enum HALT_BUG_STATE : bool
	{
		HALT_BUG_DISABLED,
		HALT_BUG_ENABLED
	};

	enum class INTERRUPTS
	{
		INTERRUPT_INVALID = -1,
		NO_INTERRUPT = 0x00,
		VBLANK_INTERRUPT = 0x01,
		LCD_STAT_INTERRUPT = 0x02,
		TIMER_INTERRUPT = 0x04,
		SERIAL_INTERRUPT = 0x08,
		JOYPAD_INTERRUPT = 0x10
	};

	enum EI_ENABLE_STATE : bool
	{
		NOTHING_TO_BE_DONE,
		EI_TO_BE_ENABLED
	};

	enum class CGB_DMA_MODE
	{
		GPDMA = 0,
		HDMA
	};

	enum DIVIDERS : int16_t
	{
		DIVIDER_INVALID = -1,
		DIVIDER_TOTAL = 1,

		// DIVIDER_01 is 16384 Hz -> 16384 increments in 1 second (runs w.r.t to Clock like eveything else in GB)
		// Clock is 4.194304 MHz -> 4194304 increments or ticks in 1 second
		// We have the clock ticks in the form of emulated CPU cycles 
		// Display Rate / VBlank interrupt for GB is @ 60 Hz, so @ 60 Hz, we need to re-process and push the COMPLETE HUGE GFX to display
		// So, our master loop is set to run @ 60 Hz, which inturn will house the while loop for core; after every 60 Hz loop, we process the VBlank
		// We run a continous while loop for simulating the GB core, but the host speed is too much, so to maintain it similar to GB
		// At max, we need to run 70221 core loops per 60 Hz function
		// In one frame @ 60 Hz, we run for 70221 clock ticks (i.e emulated CPU cycles)
		// In one frame @ 60 Hz, we run for (4.194304 MHx / 16384 Hz) divider ticks
		// In one frame @ 60 Hz, we run for 256 divider ticks

		DIVIDER_01 = 256
	};

	enum TIMERS : int16_t
	{
		TIMER_INVALID = -1,
		// Refer DIVIDERS enum for the explaination on how we got the value for TIMER_01
		TIMER_01 = 16,		// 4.194304 MHz / 262144 Hz
		// Refer DIVIDERS enum for the explaination on how we got the value for TIMER_02
		TIMER_10 = 64,		// 4.194304 MHz / 65536 Hz
		// Refer DIVIDERS enum for the explaination on how we got the value for TIMER_03
		TIMER_11 = 256,		// 4.194304 MHz / 16384 Hz
		// Refer DIVIDERS enum for the explaination on how we got the value for TIMER_04
		TIMER_00 = 1024		// 4.194304 MHz / 4096 Hz 
	};

	enum JOYPAD_STATES : uint8_t
	{
		PRESSED = ZERO,
		NOT_PRESSED = ONE
	};

	enum AUDIO_CHANNELS : uint8_t
	{
		CHANNEL_1 = ZERO,
		CHANNEL_2 = ONE,
		CHANNEL_3 = TWO,
		CHANNEL_4 = THREE,
		TOTAL_CHANNELS
	};

	enum class AUDIO_STREAMS
	{
		L = ZERO,
		R = ONE,
		TOTAL_AUDIO_STREAMS
	};

	const uint8_t AUDIO_CHANNEL_4_DIVISOR[EIGHT]
	{
		/* 0 */ 8,
		/* 1 */ 16,
		/* 2 */ 32,
		/* 3 */ 48,
		/* 4 */ 64,
		/* 5 */ 80,
		/* 6 */ 96,
		/* 7 */ 112
	};

	const uint8_t SQUARE_WAVE_AMPLITUDE[FOUR][EIGHT] =
	{
		{LO, LO, LO, LO, LO, LO, LO, HI},
		{LO, LO, LO, LO, LO, LO, HI, HI},
		{LO, LO, LO, LO, HI, HI, HI, HI},
		{HI, HI, HI, HI, HI, HI, LO, LO}
	};

	enum class PIXEL_FETCHER_STATES
	{
		WAIT_FOR_TILE = ONE,
		GET_TILE = TWO,
		WAIT_FOR_DATA_LOW = THREE,
		GET_TILE_DATA_LOW = FOUR,
		WAIT_FOR_DATA_HIGH = FIVE,
		GET_TILE_DATA_HIGH = SIX,
		SLEEP_OR_PUSH = SEVEN
	};

	enum LCD_MODES : uint8_t
	{
		MODE_LCD_H_BLANK = 0,
		MODE_LCD_V_BLANK = 1,
		MODE_LCD_SEARCHING_OAM = 2,
		MODE_LCD_DISPLAY_PIXELS = 3,
		// This is set to 4 as bits 0-1 is still 0 and we can differentiate b/w MODE_LCD_H_BLANK and MOCE_LCD_BITS_CLEAR
		MODE_LCD_BITS_CLEAR = 4,
	};

public:

	enum LCD_MODE_CYCLES : uint16_t
	{
		LCD_SEARCHING_OAM = 80,
		TX_DATA_LCD_CTRL_MIN = 172,
		TX_DATA_LCD_CTRL_MAX = 289,
		LCD_H_BLANK_MIN = 87,
		LCD_H_BLANK_MAX = 204,
		LCD_TOTAL_CYCLES_PER_SCANLINE = 456,
		LCD_V_BLANK = 4560,
	};

	enum class GB_SERIAL_DEVICE
	{
		GB_PRINTER,
		GB_LINK_CABLE,
		GB_4_PLAYER_ADAPTER,
		GB_BARCODE_BOY,
		GB_POCKET_SONAR,
		GB_MOBILE_ADAPTER,
	};

private:

	typedef struct
	{
		uint32_t placeholder;
	} quirks_t;

private:

	static const uint8_t ALPHA = 255;

	enum class colorID
	{
		COLOR_099P,
		COLOR_066P,
		COLOR_033P,
		COLOR_000P
	};

	typedef struct
	{
		Pixel COLOR;
		colorID COLOR_ID;
	} COLOR_FORMAT;

	typedef struct
	{
		COLOR_FORMAT COLOR_099P;
		COLOR_FORMAT COLOR_066P;
		COLOR_FORMAT COLOR_033P;
		COLOR_FORMAT COLOR_000P;
	} FORMAT_2BPP;

	std::unordered_map<PALETTE_ID, FORMAT_2BPP> const paletteIDToColor =
	{
		{PALETTE_ID::PALETTE_1,
		{{Pixel(0x14, 0x44, 0x03, ALPHA), colorID::COLOR_099P}, {Pixel(0x2B, 0x55, 0x03, ALPHA), colorID::COLOR_066P}, {Pixel(0x4D, 0x6B, 0x03, ALPHA), colorID::COLOR_033P}, {Pixel(0x87, 0x96, 0x03, ALPHA), colorID::COLOR_000P}}}
		,{PALETTE_ID::PALETTE_2,
		{{Pixel(0x00, 0x00, 0x00, ALPHA), colorID::COLOR_099P}, {Pixel(0x55, 0x55, 0x55, ALPHA), colorID::COLOR_066P}, {Pixel(0xAA, 0xAA, 0xAA, ALPHA), colorID::COLOR_033P}, {Pixel(0xFF, 0xFF, 0xFF, ALPHA), colorID::COLOR_000P}}}
		,{PALETTE_ID::PALETTE_3,
		{{Pixel(0x08, 0x18, 0x10, ALPHA), colorID::COLOR_099P}, {Pixel(0x39, 0x61, 0x39, ALPHA), colorID::COLOR_066P}, {Pixel(0x84, 0xA5, 0x63, ALPHA), colorID::COLOR_033P}, {Pixel(0xC6, 0xDE, 0x8C, ALPHA), colorID::COLOR_000P}}}
		,{PALETTE_ID::PALETTE_4,
		{{Pixel(0x08, 0x18, 0x20, ALPHA), colorID::COLOR_099P}, {Pixel(0x34, 0x68, 0x56, ALPHA), colorID::COLOR_066P}, {Pixel(0x88, 0xC0, 0x70, ALPHA), colorID::COLOR_033P}, {Pixel(0xE0, 0xF8, 0xD0, ALPHA), colorID::COLOR_000P}}}
		,{PALETTE_ID::PALETTE_5,
		{{Pixel(0x0F, 0x38, 0x0F, ALPHA), colorID::COLOR_099P}, {Pixel(0x30, 0x62, 0x30, ALPHA), colorID::COLOR_066P}, {Pixel(0x7B, 0x9C, 0x0F, ALPHA), colorID::COLOR_033P}, {Pixel(0x9B, 0xBC, 0x0F, ALPHA), colorID::COLOR_000P}}}
		,{PALETTE_ID::PALETTE_6,
		{{Pixel(0x22, 0x24, 0x21, ALPHA), colorID::COLOR_099P}, {Pixel(0x50, 0x54, 0x45, ALPHA), colorID::COLOR_066P}, {Pixel(0x7E, 0x8E, 0x67, ALPHA), colorID::COLOR_033P}, {Pixel(0xAC, 0xBE, 0x8C, ALPHA), colorID::COLOR_000P}}}
	};

private:

	enum class MBCType  : uint16_t
	{
		NONE, 
		MBC1, 
		MBC1M, 
		MBC2, 
		MBC3, 
		MBC5, 
		MBC6,
		MBC7,
		MMM01,
		M161,
		HUC1,
		HUC3,
		WISDOM_TREE,
		POCKET_CAMERA,
		POKE_2IN1,
		INVALID_MBC
	};

	const std::unordered_map<uint16_t, MBCType> kMBCTypeMap = 
	{
		{0x00, MBCType::NONE},
		{0x01, MBCType::MBC1}, {0x02, MBCType::MBC1}, {0x03, MBCType::MBC1},
		{0x05, MBCType::MBC2}, {0x06, MBCType::MBC2},
		{0x0F, MBCType::MBC3}, {0x10, MBCType::MBC3}, {0x11, MBCType::MBC3},{0x12, MBCType::MBC3}, {0x13, MBCType::MBC3},
		{0x19, MBCType::MBC5}, {0x1A, MBCType::MBC5}, {0x1B, MBCType::MBC5},{0x1C, MBCType::MBC5}, {0x1D, MBCType::MBC5}, {0x1E, MBCType::MBC5},
		{0x20, MBCType::MBC6},
		{0x22, MBCType::MBC7},
		{0xFC, MBCType::POCKET_CAMERA},
		{0xFE, MBCType::HUC3},
		{0xFF, MBCType::HUC1},
	};

	enum class ROMBankType : uint16_t
	{
		ROM_32K,
		ROM_64K,
		ROM_128K,
		ROM_256K,
		ROM_512K,
		ROM_1M,
		ROM_2M,
		ROM_4M,
		ROM_8M,
		ROM_1_1M = 34,
		ROM_1_2M = 35,
		ROM_1_5M = 36,
		ROM_UNKNOWN = 0xFFFF
	};

	enum class RAMBankType : uint16_t
	{
		NO_BANK = 0,
		RAM_2K = 1,
		RAM_8K = 2,
		RAM_32K = 3,
		RAM_128K = 4,
		RAM_64K = 5,
		RAM_UNKNOWN = 0xFFFF
	};

	typedef struct
	{
		uint8_t CLOCK_SELECT : 1; // bit  0
		uint8_t CLOCK_SPEED : 1; // bit  1
		uint8_t RESERVED : 5; // bit  2 - 6
		uint8_t TRANSFER_ENABLE : 1; // bit  7
	} scFields_t;

	typedef union
	{
		uint8_t scMemory;
		scFields_t scFields;
	} sc_t;

	typedef struct
	{
		uint8_t title[0x000F];
		uint8_t cgbType;
	} tile_AND_cgbType_Fields_t;

	typedef union
	{
		tile_AND_cgbType_Fields_t tile_AND_cgbType_Fields;
		uint8_t title[0x0010];
	} title_AND_cgbType_t;

	typedef struct
	{
		uint8_t entryPoint[0x0004];
		uint8_t nintendologo[0x0030];
		title_AND_cgbType_t title;
		char newLicCode[2];
		uint8_t sgbFlag;
		uint8_t cartridgeType;
		uint8_t romSize;
		uint8_t ramSize;
		uint8_t destinationCode;
		uint8_t oldLicCode;
		uint8_t maskRomVersion;
		uint8_t headerChecksum;
		uint16_t globalChecksum;
	} cartridge_header_fields_t;

	typedef union
	{
		cartridge_header_fields_t cartridge_header_fields;
		uint8_t cartridge_header_buffer[sizeof(cartridge_header_fields_t)];
	} cartridge_header_t;

	typedef struct
	{
		uint8_t romBank00_Field1[0x0100];		// 0x0000 - 0x00FF
		cartridge_header_t cartridge_header;	// 0x0100 - 0x014F
		uint8_t romBank00_Field2[0x3EB0];		// 0x0150 - 0x3FFF
	} romBank00_Fields_t;

	typedef union
	{
		romBank00_Fields_t romBank00_Fields;
		uint8_t romBank00_Memory[0x4000];
	} romBank00_t;

	typedef struct
	{
		romBank00_t romBank_00;
		BYTE romBank_NN[0x4000];
	} codeRomFields_t;

	typedef union
	{
		codeRomFields_t codeRomFields;
		BYTE codeRomMemory[sizeof(codeRomFields_t)];
	} codeRomMemory_t;

	typedef struct
	{
		uint8_t tileData[0x0800];
	} videoRamTileDataField_t;

	typedef struct
	{
		videoRamTileDataField_t tileDataBlock[3];
	} videoRamTileDataFields_t;

	typedef struct
	{
		uint8_t tileMap[0x0400];
	} videoRamTileMapField_t;

	typedef struct
	{
		videoRamTileMapField_t tileMapBlock[2];
	} videoRamTileMapFields_t;

	typedef struct
	{
		videoRamTileDataFields_t videoRamTileDataFields;
		videoRamTileMapFields_t videoRamTileMapField;
	} videoRamFields_t;

	typedef union
	{
		videoRamFields_t videoRamFields;
		uint8_t videoRamMemory[sizeof(videoRamFields_t)];
	} videoRamMemory_t;

	typedef struct
	{
		uint8_t externalRam[0x2000];
	} externalRamFields_t;

	typedef union
	{
		externalRamFields_t externalRamFields;
		uint8_t externalRamMemory[sizeof(externalRamFields_t)];
	} externalRamMemory_t;

	typedef struct
	{
		uint8_t workRam_00[0x1000];
		uint8_t workRam_NN[0x1000];
	} workRamFields_t;

	typedef union
	{
		workRamFields_t workRamFields;
		uint8_t wRamMemory[sizeof(workRamFields_t)];
	} workRamMemory_t;

	typedef struct
	{
		uint8_t echoRam[0x1E00];
	} echoRamFields_t;

	typedef union
	{
		echoRamFields_t echoRamFields;
		uint8_t echoRamMemory[sizeof(echoRamFields_t)];
	} echoRamMemory_t;

	typedef struct
	{
		uint8_t OAM_PALETTE_NUMBER_CGB : 3; // bit  0 - 2
		uint8_t OAM_TILE_VRAM_BANK : 1; // bit  3
		uint8_t OAM_PALETTE_NUMBER_DMG : 1; // bit  4
		uint8_t OAM_X_FLIP : 1; // bit  5	
		uint8_t OAM_Y_FLIP : 1; // bit  6
		uint8_t OAM_BG_WINDOW_OVER_OBJ : 1; // bit  7
	} oamEntryFields_t;

	typedef union
	{
		oamEntryFields_t oamEntryFields;
		uint8_t oamEntryByte;
	} oamEntryByte_t;

	typedef struct
	{
		BYTE yPosition;
		BYTE xPosition;
		BYTE tileIndex;
		oamEntryByte_t attributes;
	} OAMEntry_t;

	typedef struct
	{
		OAMEntry_t OAM[(0x00A0 / FOUR)];
	} OAMFields_t;

	typedef union
	{
		OAMFields_t OAMFields;
		uint8_t OAMMemory[sizeof(OAMFields_t)];
		uint8_t OAMRows[20][8];
	} OAMMemory_t;

	typedef struct
	{
		uint8_t CLOCK_SELECT : 2; // bit  0 - 1
		uint8_t TIMER_ENABLE : 1; // bit  2
		uint8_t TAC_3 : 1; // bit  3
		uint8_t TAC_4 : 1; // bit  4
		uint8_t TAC_5 : 1; // bit  5	
		uint8_t TAC_6 : 1; // bit  6
		uint8_t TAC_7 : 1; // bit  7
	} timerControlFields_t;

	typedef union
	{
		timerControlFields_t timerControlFields;
		uint8_t timerControlMemory;
	} timerControl_t;

	typedef struct
	{
		uint8_t VBLANK : 1; // bit  0
		uint8_t LCD_STAT : 1; // bit  1
		uint8_t TIMER : 1; // bit  2
		uint8_t SERIAL : 1; // bit  3
		uint8_t JOYPAD : 1; // bit  4
		uint8_t NO_INT05 : 1; // bit  5	
		uint8_t NO_INT06 : 1; // bit  6
		uint8_t NO_INT07 : 1; // bit  7
	} interruptRequestFields_t;

	typedef union
	{
		interruptRequestFields_t interruptRequestFields;
		uint8_t interruptRequestMemory;
	} interruptRequest_t;

	typedef struct
	{
		uint8_t DIV_ZERO : 1; // bit  0
		uint8_t DIV_ONE : 1; // bit  1
		uint8_t DIV_TWO : 1; // bit  2
		uint8_t DIV_THREE : 1; // bit  3
		uint8_t DIV_FOUR : 1; // bit  4
		uint8_t DIV_FIVE : 1; // bit  5	
		uint8_t DIV_SIX : 1; // bit  6
		uint8_t DIV_SEVEN : 1; // bit  7
	} divFields_t;

	typedef union
	{
		divFields_t divFields;
		uint8_t divByte;
	} divByte_t;

	typedef struct
	{
		divByte_t DIV_LSB;			// FF03
		divByte_t DIV_MSB;			// FF04
	} divBytes_t;

	typedef union
	{
		divBytes_t divBytes;
		uint16_t divMemory;
	} div_t;

	typedef struct
	{
		uint8_t BG_WINDOW_LAYER_ENABLE : 1; // bit  0
		uint8_t OBJ_ENABLE : 1; // bit  1
		uint8_t OBJ_SIZE : 1; // bit  2
		uint8_t BG_TILE_MAP_AREA : 1; // bit  3
		uint8_t BG_WINDOW_TILE_DATA_AREA : 1; // bit  4
		uint8_t WINDOW_LAYER_ENABLE : 1; // bit  5	
		uint8_t WINDOW_TILE_MAP_AREA : 1; // bit  6
		uint8_t LCD_PPU_ENABLE : 1; // bit  7
	} lcdControlFields_t;

	typedef union
	{
		lcdControlFields_t lcdControlFields;
		uint8_t lcdControlMemory;
	} lcdControl_t;

	typedef struct
	{
		uint8_t MODE : 2; // bit  0 - 1
		uint8_t LYC_EQL_LY_FLAG : 1; // bit  2
		uint8_t MODE0_HBLANK_STAT_INT_SRC : 1; // bit  3
		uint8_t MODE1_VBLANK_STAT_INT_SRC : 1; // bit  4
		uint8_t MODE2_OAM_STAT_INT_SRC : 1; // bit  5	
		uint8_t LYC_EQL_LY_STAT_INT_SRC : 1; // bit  6
		uint8_t UNUSED_07 : 1; // bit  7
	} lcdStatusFields_t;

	typedef union
	{
		lcdStatusFields_t lcdStatusFields;
		uint8_t lcdStatusMemory;
	} lcdStatus_t;

	typedef struct
	{
		uint8_t P10_RIGHT_A : 1; // bit  0
		uint8_t P11_LEFT_B : 1; // bit  1
		uint8_t P12_UP_SELECT : 1; // bit  2	
		uint8_t P13_DOWN_START : 1; // bit  3
		uint8_t P14_SEL_DIRECTION_KEYS : 1; // bit  4
		uint8_t P15_SEL_ACTION_KEYS : 1; // bit  5	
		uint8_t JP_SPARE_06 : 1; // bit  6
		uint8_t JP_SPARE_07 : 1; // bit  7
	} joyPadFields_t;

	typedef union
	{
		joyPadFields_t joyPadFields;
		BYTE joyPadMemory;
	} joyPadMemory_t;

	typedef struct
	{
		uint8_t Address : 6; // bit  0 - 5
		uint8_t SPARE_06 : 1; // bit  6
		uint8_t AutoIncrement : 1; // bit  7
	} BCPSFields_t;

	typedef union
	{
		BCPSFields_t BCPSFields;
		BYTE BCPSMemory;
	} BCPSMemory_t;

	typedef struct
	{
		uint8_t Address : 6; // bit  0 - 5
		uint8_t SPARE_06 : 1; // bit  6
		uint8_t AutoIncrement : 1; // bit  7
	} OCPSFields_t;

	typedef union
	{
		BCPSFields_t OCPSFields;
		BYTE OCPSMemory;
	} OCPSMemory_t;

	typedef struct
	{
		uint8_t Reserved0 : 1; // Bit 0: Writable, function unknown
		uint8_t padding : 1; // Bit 1: Padding to align the next field correctly
		uint8_t mode : 2; // Bits 2-3: Core CPU Mode selection
		uint8_t Reserved1 : 4; // Bits 4-7: Remaining upper bits
	} KEY0Fields_t;

	typedef union
	{
		KEY0Fields_t KEY0Fields;
		BYTE KEY0Memory;
	} KEY0Memory_t;

	typedef struct
	{
		uint8_t PrepareSpeedSwitch : 1; // bit  0
		uint8_t Reserved : 6; // bit  1 - 6
		uint8_t CurrentSpeed : 1; // bit  7
	} KEY1Fields_t;

	typedef union
	{
		KEY1Fields_t KEY1Fields;
		BYTE KEY1Memory;
	} KEY1Memory_t;

	typedef struct
	{
		uint8_t sweepSlopeControl : 3; // bit  0 - 2
		uint8_t sweepDirection : 1; // bit  3
		uint8_t sweepPace : 3; // bit  4 - 6
		uint8_t reserved : 1; // bit  7 
	} channelSweepFields_t;

	typedef union
	{
		channelSweepFields_t channelSweepFields;
		BYTE channelSweepMemory;
	} channelSweepMemory_t;

	typedef struct
	{
		uint8_t initialLengthTimer : 6; // bit  0 - 5
		uint8_t waveDuty : 2; // bit  6 - 7
	} channelLengthAndDutyFields_t;

	typedef union
	{
		channelLengthAndDutyFields_t channelLengthAndDutyFields;
		BYTE channelLengthAndDutyMemory;
	} channelLengthAndDutyMemory_t;

	typedef struct
	{
		uint8_t envelopeSweepPace : 3; // bit  0 - 2
		uint8_t envelopeDirection : 1; // bit  3
		uint8_t initialVolumeOfEnvelope : 4; // bit  4 - 7
	} channelVolumeAndEnvelopeFields_t;

	typedef union
	{
		channelVolumeAndEnvelopeFields_t channelVolumeAndEnvelopeFields;
		BYTE channelVolumeAndEnvelopeMemory;
	} channelVolumeAndEnvelopeMemory_t;

	typedef struct
	{
		BYTE lowerPeriodValue;
	} channelLowerPeriodMemory_t;

	typedef struct
	{
		uint8_t higherPeriodValue : 3; // bit  0 - 2
		uint8_t reserved : 3; // bit  3 - 5
		uint8_t soundLengthEnable : 1; // bit  6
		uint8_t trigger : 1; // bit  7
	} channelHigherPeriodAndControlFields_t;

	typedef union
	{
		channelHigherPeriodAndControlFields_t channelHigherPeriodAndControlFields;
		BYTE channelHigherPeriodAndControlMemory;
	} channelHigherPeriodAndControlMemory_t;

	typedef struct
	{
		uint8_t reserved : 7; // bit  0 - 6
		uint8_t dacEnable : 1; // bit  7
	} channelDACEnableFields_t;

	typedef union
	{
		channelDACEnableFields_t channelDACEnableFields;
		BYTE channelDACEnableMemory;
	} channelDACEnableMemory_t;

	typedef struct
	{
		uint8_t reserved00 : 5; // bit  0 - 4
		uint8_t outputLevelSelection : 2; // bit  5 - 6
		uint8_t reserved01 : 1; // bit  7
	} channelOutputLevelFields_t;

	typedef union
	{
		channelOutputLevelFields_t channelOutputLevelFields;
		BYTE channelOutputLevelMemory;
	} channelOutputLevelMemory_t;

	typedef struct
	{
		uint8_t lengthTimer : 6; // bit  0 - 5
		uint8_t reserved : 1; // bit  6 - 7
	} channelLengthTimerFields_t;

	typedef union
	{
		channelLengthTimerFields_t channelLengthTimerFields;
		BYTE lengthTimerMemory;
	} channelLengthTimerMemory_t;

	typedef struct
	{
		uint8_t clockDivider : 3; // bit  0 - 2
		uint8_t LFSRwidth : 1; // bit  3
		uint8_t clockShift : 4; // bit  4 - 7
	} channelFrequencyAndRandomnessFields_t;

	typedef union
	{
		channelFrequencyAndRandomnessFields_t channelFrequencyAndRandomnessFields;
		BYTE channelFrequencyAndRandomnessMemory;
	} channelFrequencyAndRandomnessMemory_t;

	typedef struct
	{
		uint8_t rightOutputVolume : 3; // bit  0 - 2
		uint8_t mixVINToRightOutput : 1; // bit  3
		uint8_t leftOutputVolume : 3; // bit  4 - 6
		uint8_t mixVINToLeftOutput : 1; // bit  7
	} channelMasterVolumeAndVINPanningFields_t;

	typedef union
	{
		channelMasterVolumeAndVINPanningFields_t channelMasterVolumeAndVINPanningFields;
		BYTE channelMasterVolumeAndVINPanningMemory;
	} channelMasterVolumeAndVINPanningMemory_t;

	typedef struct
	{
		uint8_t mixChannel1ToRightOutput : 1; // bit  0
		uint8_t mixChannel2ToRightOutput : 1; // bit  1
		uint8_t mixChannel3ToRightOutput : 1; // bit  2
		uint8_t mixChannel4ToRightOutput : 1; // bit  3
		uint8_t mixChannel1ToLeftOutput : 1; // bit  4
		uint8_t mixChannel2ToLeftOutput : 1; // bit  5
		uint8_t mixChannel3ToLeftOutput : 1; // bit  6
		uint8_t mixChannel4ToLeftOutput : 1; // bit  7
	} channelSoundPanningFields_t;

	typedef union
	{
		channelSoundPanningFields_t channelSoundPanningFields;
		BYTE channelSoundPanningMemory;
	} channelSoundPanningMemory_t;

	typedef struct
	{
		uint8_t channel1ONFlag : 1; // bit  0
		uint8_t channel2ONFlag : 1; // bit  1
		uint8_t channel3ONFlag : 1; // bit  2
		uint8_t channel4ONFlag : 1; // bit  3
		uint8_t reserved : 3; // bit  4 - 6
		uint8_t allChannelONOFFToggle : 1; // bit  7
	} channelSoundONOFFFields_t;

	typedef union
	{
		channelSoundONOFFFields_t channelSoundONOFFFields;
		BYTE channelSoundONOFFMemory;
	} channelSoundONOFFMemory_t;

	typedef struct
	{
		uint8_t lowerNibble : 4; // bits 0 - 3
		uint8_t upperNibble : 4; // bits 4 - 7
	} samples_t;

	typedef union
	{
		samples_t samples;
		uint8_t waveRamByte;
	} waveRamByte_t;

	typedef struct
	{
		joyPadMemory_t P1_JOYP;							// FF00
		uint8_t SB;										// FF01
		sc_t SC;										// FF02
		div_t DIV;										// FF03 - FF04
		uint8_t TIMA;									// FF05
		uint8_t TMA;									// FF06
		timerControl_t TAC;								// FF07
		uint8_t SPARE_01;								// FF08
		uint8_t SPARE_02;								// FF09
		uint8_t SPARE_03;								// FF0A
		uint8_t SPARE_04;								// FF0B
		uint8_t SPARE_05;								// FF0C
		uint8_t SPARE_06;								// FF0D
		uint8_t SPARE_07;								// FF0E
		interruptRequest_t IF;							// FF0F
		channelSweepMemory_t NR10;						// FF10
		channelLengthAndDutyMemory_t NR11;				// FF11
		channelVolumeAndEnvelopeMemory_t NR12;			// FF12
		channelLowerPeriodMemory_t NR13;				// FF13
		channelHigherPeriodAndControlMemory_t NR14;		// FF14
		uint8_t SPARE_08;								// FF15
		channelLengthAndDutyMemory_t NR21;				// FF16
		channelVolumeAndEnvelopeMemory_t NR22;			// FF17
		channelLowerPeriodMemory_t NR23;				// FF18
		channelHigherPeriodAndControlMemory_t NR24;		// FF19
		channelDACEnableMemory_t NR30;					// FF1A
		uint8_t NR31;									// FF1B
		channelOutputLevelMemory_t NR32;				// FF1C
		channelLowerPeriodMemory_t NR33;				// FF1D
		channelHigherPeriodAndControlMemory_t NR34;		// FF1E
		uint8_t SPARE_09;								// FF1F
		channelLengthTimerMemory_t NR41;				// FF20
		channelVolumeAndEnvelopeMemory_t NR42;			// FF21
		channelFrequencyAndRandomnessMemory_t NR43;		// FF22
		channelHigherPeriodAndControlMemory_t NR44;		// FF23
		channelMasterVolumeAndVINPanningMemory_t NR50;	// FF24
		channelSoundPanningMemory_t NR51;				// FF25
		channelSoundONOFFMemory_t NR52;					// FF26
		uint8_t SPARE_10;								// FF27
		uint8_t SPARE_11;								// FF28
		uint8_t SPARE_12;								// FF29
		uint8_t SPARE_13;								// FF2A
		uint8_t SPARE_14;								// FF2B
		uint8_t SPARE_15;								// FF2C
		uint8_t SPARE_16;								// FF2D
		uint8_t SPARE_17;								// FF2E
		uint8_t SPARE_18;								// FF2F
		waveRamByte_t waveRam[0x0010];					// FF30 - FF3F
		lcdControl_t LCDC;								// FF40
		lcdStatus_t STAT;								// FF41
		uint8_t SCY;									// FF42
		uint8_t SCX;									// FF43
		uint8_t LY;										// FF44
		uint8_t LYC;									// FF45	
		uint8_t DMA;									// FF46	
		uint8_t BGP;									// FF47	
		uint8_t OBP0;									// FF48	
		uint8_t OBP1;									// FF49	
		uint8_t WY;										// FF4A	
		uint8_t WX;										// FF4B	
		KEY0Memory_t KEY0;								// FF4C
		KEY1Memory_t KEY1;								// FF4D
		uint8_t SPARE_20;								// FF4E	
		uint8_t VBK;									// FF4F
		uint8_t BANK;									// FF50	
		uint8_t HDMA1;									// FF51	
		uint8_t HDMA2;									// FF52	
		uint8_t HDMA3;									// FF53	
		uint8_t HDMA4;									// FF54	
		uint8_t HDMA5;									// FF55	
		uint8_t RP;										// FF56
		uint8_t SPARE_22;								// FF57	
		uint8_t SPARE_23;								// FF58	
		uint8_t SPARE_24;								// FF59	
		uint8_t SPARE_25;								// FF5A	
		uint8_t SPARE_26;								// FF5B	
		uint8_t SPARE_27;								// FF5C
		uint8_t SPARE_28;								// FF5D
		uint8_t SPARE_29;								// FF5E	
		uint8_t SPARE_30;								// FF5F
		uint8_t SPARE_31;								// FF60	
		uint8_t SPARE_32;								// FF61	
		uint8_t SPARE_33;								// FF62	
		uint8_t SPARE_34;								// FF63	
		uint8_t SPARE_35;								// FF64	
		uint8_t SPARE_36;								// FF65	
		uint8_t SPARE_37;								// FF66
		uint8_t SPARE_38;								// FF67
		BCPSMemory_t BCPS_BGPI;							// FF68	
		uint8_t BCPD_BGPD;								// FF69	
		OCPSMemory_t OCPS_OBPI;							// FF6A	
		uint8_t OCPD_OBPD;								// FF6B	
		uint8_t OPRI;									// FF6C	
		uint8_t SPARE_39;								// FF6D
		uint8_t SPARE_40;								// FF6E	
		uint8_t SPARE_41;								// FF6F	
		uint8_t SVBK;									// FF70	
		uint8_t SPARE_42;								// FF71
		uint8_t SPARE_43;   							// FF72
		uint8_t SPARE_44;   							// FF73
		uint8_t SPARE_45;   							// FF74
		uint8_t SPARE_46;								// FF75	
		uint8_t PCM12;									// FF76		
		uint8_t PCM34;									// FF77	
		uint8_t SPARE_47;								// FF78
		uint8_t SPARE_48;   							// FF79
		uint8_t SPARE_49;   							// FF7A
		uint8_t SPARE_50;   							// FF7B
		uint8_t SPARE_51;								// FF7C	
		uint8_t SPARE_52;   							// FF7D
		uint8_t SPARE_53;								// FF7E	
		uint8_t SPARE_54;   							// FF7F
	} IOFields_t;

	typedef union
	{
		IOFields_t IOFields;
		uint8_t IOMemory[sizeof(IOFields_t)];
	} IOMemory_t;

	typedef struct
	{
		uint8_t highRam[0x007F];
	} highRamFields_t;

	typedef union
	{
		highRamFields_t highRamFields;
		uint8_t highRamMemory[sizeof(highRamFields_t)];
	} highRamMemory_t;

	typedef struct
	{
		uint8_t VBLANK : 1; // bit  0
		uint8_t LCD_STAT : 1; // bit  1
		uint8_t TIMER : 1; // bit  2
		uint8_t SERIAL : 1; // bit  3
		uint8_t JOYPAD : 1; // bit  4
		uint8_t NO_INT05 : 1; // bit  5	
		uint8_t NO_INT06 : 1; // bit  6
		uint8_t NO_INT07 : 1; // bit  7
	} interruptEnableFields_t;

	typedef union
	{
		interruptEnableFields_t interruptEnableFields;
		uint8_t interruptEnableMemory;
	} interruptEnable_t;

	typedef struct
	{
		codeRomMemory_t mCodeRom;
		videoRamMemory_t mVideoRam;
		externalRamMemory_t mExternalRam;
		workRamMemory_t mWorkRam;
		echoRamMemory_t mEchoRam;
		OAMMemory_t mOAM;
		BYTE mForbidden[0x0060];
		IOMemory_t mIO;
		highRamMemory_t mHighRam;
		interruptEnable_t mInterruptEnable;
	} memoryMap_t;

	typedef union
	{
		memoryMap_t GBcMemoryMap;
		BYTE GBcRawMemory[sizeof(memoryMap_t)];
	} GBcMemory_t;

	typedef struct
	{
		uint8_t DAYCOUNTER_MSB : 1; // bit  0
		uint8_t SPARE_1 : 1; // bit  1
		uint8_t SPARE_2 : 1; // bit  2	
		uint8_t SPARE_3 : 1; // bit  3
		uint8_t SPARE_4 : 1; // bit  4
		uint8_t SPARE_5 : 1; // bit  5	
		uint8_t DAYCOUNTER_HALT : 1; // bit  6
		uint8_t DAYCOUNTER_CARRY : 1; // bit  7
	} rtcDHFields_t;

	typedef union
	{
		rtcDHFields_t rtcDHFields;
		BYTE rtcDHMemory;
	} rtcDHMemory_t;

	typedef struct
	{
		BYTE rtc_S;
		BYTE rtc_M;
		BYTE rtc_H;
		BYTE rtc_DL;
		rtcDHMemory_t rtc_DH;
	} rtcFields_t;

	typedef union
	{
		rtcFields_t rtcFields;
		uint8_t rtcBuffer[sizeof(rtcFields_t)];
	} rtc_t;

	typedef struct
	{
		FLAG isChannelActuallyEnabled;
		int32_t lengthTimer;
		int32_t frequencyTimer;
		int32_t waveDutyPosition;
		int32_t envelopePeriodTimer;
		uint8_t currentVolume;
		FLAG sweepEnabled;
		int32_t shadowFrequency;
		int32_t sweepTimer;
		uint16_t LFSR;
		FLAG isVolumeEnvelopeStillDoingAutomaticUpdates;
	} audioChannelInstance_t;

	typedef struct
	{
		FLAG nextHalfWillNotClockLengthCounter;
		FLAG wasSweepDirectionNegativeAtleastOnceSinceLastTrigger;
		FLAG didChannel3ReadWaveRamPostTrigger;
		BYTE waveRamCurrentIndex;
		BYTE channel3OutputLevelAndShift;
		FLAG wasDivAPUUpdated;
		FLAG wasPowerCycled;
		int32_t div_apu;
		MAP8 dacEnMap;
		BYTE sampleReadByChannel1;
		BYTE sampleReadByChannel2;
		BYTE sampleReadByChannel3;
		BYTE sampleReadByChannel4;
		int32_t downSamplingRatioCounter;
		uint32_t accumulatedTone;
		float emulatorVolume;
#ifndef __RPI_PICO__
		audioChannelInstance_t audioChannelInstance[(uint8_t)AUDIO_CHANNELS::TOTAL_CHANNELS];
		GBC_AUDIO_SAMPLE_TYPE audioBuffer[AUDIO_BUFFER_SIZE_FOR_GB_GBC];
#endif // !__RPI_PICO__
	} audio_t;

	// Since we are going to sort the OAM objects w.r.t "x", using linked list instead of array
	struct visibleObjects_t
	{
		FLAG alreadyProcessed;
		OAMEntry_t oamEntry;
		int32_t indexWithinOAMMemory;
		struct visibleObjects_t* next;
	};

	typedef struct
	{
		uint8_t BG_PALETTE_NUMBER : 3; // bit  0 - 2	
		uint8_t BG_TILE_VRAM_BANK_NUMBER : 1; // bit  3
		uint8_t BG_SPARE_04 : 1; // bit  4
		uint8_t BG_XFLIP : 1; // bit  5	
		uint8_t BG_YFLIP : 1; // bit  6
		uint8_t BG_to_OAM_Priority : 1; // bit  7
	} bgMapAttributesFields_t;

	struct pixelFIFOEntity_t
	{
		int8_t color;
		int8_t palette;
		int8_t spritePriority;
		int8_t backgroundPriority;
		int8_t validity;

		pixelFIFOEntity_t() : color(ZERO), palette(ZERO), spritePriority(ZERO), backgroundPriority(ZERO), validity(INVALID)
		{
			this->color = ZERO;
			this->palette = ZERO;
			this->spritePriority = ZERO;
			this->backgroundPriority = ZERO;
			this->validity = INVALID;
		}
	};

	struct pixelFIFO_t
	{
		struct pixelFIFOEntity_t pEntities[PIXEL_FIFO_SIZE_FOR_GB_GBC];
		BYTE numberOfEntities;

		pixelFIFO_t() : numberOfEntities(ZERO)
		{
			pixelFIFOEntity_t dummy;
			dummy.validity = INVALID;

			for (BYTE ii = ZERO; ii < PIXEL_FIFO_SIZE_FOR_GB_GBC; ii++)
			{
				pEntities[ii] = dummy;
			}
		}

		FLAG isEmpty()
		{
			RETURN (numberOfEntities <= ZERO);
		}

		FLAG isFull()
		{
			RETURN (numberOfEntities >= PIXEL_FIFO_SIZE_FOR_GB_GBC);
		}

		FLAG needsFilling()
		{
			RETURN (numberOfEntities <= (PIXEL_FIFO_SIZE_FOR_GB_GBC - EIGHT));
		}

		FLAG pop(struct pixelFIFOEntity_t* pEntity)
		{
			if (isEmpty() == YES)
			{
				RETURN FAILURE;
			}

			// Is not empty, but still first entry is invalid !
			if (pEntities[ZERO].validity == INVALID)
			{
				FATAL("Trying to pop an invalid pixel entry");
				RETURN FAILURE;
			}

			*pEntity = pEntities[ZERO];

			numberOfEntities -= ONE; // 1 old entry was popped out of fifo

			for (BYTE ii = ZERO; ii < (PIXEL_FIFO_SIZE_FOR_GB_GBC - ONE); ii++)
			{
				pEntities[ii] = pEntities[ii + ONE];
			}

			struct pixelFIFOEntity_t dummy;
			dummy.validity = INVALID;

			pEntities[PIXEL_FIFO_SIZE_FOR_GB_GBC - ONE] = dummy;

			RETURN SUCCESS;
		}

		FLAG pop(void)
		{
			struct pixelFIFOEntity_t dummy;
			RETURN pop(&dummy);
		}

		FLAG push(struct pixelFIFOEntity_t pEntity[EIGHT], uint8_t validEntryCount, DIM8 actualFIFOSize)
		{
			if (actualFIFOSize == EIGHT)
			{
				if (isEmpty() == NO)
				{
					RETURN FAILURE;
				}
			}
			else if (actualFIFOSize == SIXTEEN)
			{
				if (needsFilling() == NO)
				{
					RETURN FAILURE;
				}
			}

			BYTE jj = ZERO;
			auto startIdx = numberOfEntities;
			auto endIdx = numberOfEntities + validEntryCount;
			for (BYTE ii = startIdx; ii < endIdx; ii++)
			{
				if (pEntity[jj].validity != INVALID)
				{
					pEntities[ii] = pEntity[jj];
					++jj;
					++numberOfEntities;
				}
			}

			RETURN SUCCESS;
		}

		struct pixelFIFOEntity_t* referenceElement(BYTE index)
		{
			if (index > (numberOfEntities - ONE))
			{
				RETURN NULL;
			}

			RETURN &(pEntities[index]);
		}

		FLAG insertValidElementAt(BYTE index, struct pixelFIFOEntity_t pEntity)
		{
			if (isFull() == YES)
			{
				RETURN FAILURE;
			}

			if (pEntity.validity == INVALID)
			{
				RETURN FAILURE;
			}

			if (index > (numberOfEntities - ONE))
			{
				numberOfEntities++;
			}

			pEntities[index] = pEntity;

			RETURN SUCCESS;
		}

		void clearFIFO()
		{
			numberOfEntities = ZERO;

			struct pixelFIFOEntity_t dummy;
			dummy.backgroundPriority = 0;
			dummy.color = 0;
			dummy.palette = 0;
			dummy.spritePriority = 0;
			dummy.validity = INVALID;

			for (BYTE ii = ZERO; ii < PIXEL_FIFO_SIZE_FOR_GB_GBC; ii++)
			{
				pEntities[ii] = dummy;
			}
		}
	};

	typedef union
	{
		bgMapAttributesFields_t bgMapAttributesFields;
		BYTE bgMapAttributesMemory;
	} bgMapAttributes_t;

	struct pixelFetcherContext_t
	{
		BYTE bgWinTileID;
		BYTE bgWinTileDataLo;
		BYTE bgWinTileDataHi;
		BYTE objTileID;
		BYTE objTileDataLo;
		BYTE objTileDataHi;
		bgMapAttributes_t bgAttribute;
		struct
		{
			pixelFIFOEntity_t cachedFifo[EIGHT];
			BYTE validEntries;
		} cachedFifo_bg_win;
		struct
		{
			pixelFIFOEntity_t cachedFifo[EIGHT];
			BYTE validEntries;
		} cachedFifo_obj;
		void clear_cachedFifo_bg_win()
		{
			for (BYTE ii = 0; ii < EIGHT; ii++)
			{
				cachedFifo_bg_win.cachedFifo[ii].backgroundPriority = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].color = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].palette = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].spritePriority = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].validity = INVALID;
				cachedFifo_bg_win.validEntries = RESET;
			}
		}
		void clear_cachedFifo_obj()
		{
			for (BYTE ii = 0; ii < EIGHT; ii++)
			{
				cachedFifo_obj.cachedFifo[ii].backgroundPriority = ZERO;
				cachedFifo_obj.cachedFifo[ii].color = ZERO;
				cachedFifo_obj.cachedFifo[ii].palette = ZERO;
				cachedFifo_obj.cachedFifo[ii].spritePriority = ZERO;
				cachedFifo_obj.cachedFifo[ii].validity = INVALID;
				cachedFifo_obj.validEntries = RESET;
			}
		}
		void clearAllCachedFifos()
		{
			for (BYTE ii = 0; ii < EIGHT; ii++)
			{
				cachedFifo_bg_win.cachedFifo[ii].backgroundPriority = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].color = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].palette = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].spritePriority = ZERO;
				cachedFifo_bg_win.cachedFifo[ii].validity = INVALID;
				cachedFifo_bg_win.validEntries = RESET;
				cachedFifo_obj.cachedFifo[ii].backgroundPriority = ZERO;
				cachedFifo_obj.cachedFifo[ii].color = ZERO;
				cachedFifo_obj.cachedFifo[ii].palette = ZERO;
				cachedFifo_obj.cachedFifo[ii].spritePriority = ZERO;
				cachedFifo_obj.cachedFifo[ii].validity = INVALID;
				cachedFifo_obj.validEntries = RESET;
			}
		}
		void reset()
		{
			bgWinTileID = RESET;
			bgWinTileDataLo = RESET;
			bgWinTileDataHi = RESET;
			objTileID = RESET;
			objTileDataLo = RESET;
			objTileDataHi = RESET;
			bgAttribute.bgMapAttributesMemory = RESET;
			clearAllCachedFifos();
		}
	};

	typedef struct
	{
		uint16_t fakeBgFetcherRuns;
		LCD_MODES currentLCDMode;
		LCD_MODES currentSpecialLCDMode;
		uint8_t currentScanline;
		uint16_t tickAtMode3ToMode0;
		FLAG lcdJustEn;
		FLAG skipMode2;
		uint16_t latchedSCYForGBC;
		int16_t windowLineCounter;
		FLAG wasVblankJustTriggerred;
		FLAG yConditionForWindowIsMetForCurrentFrame;
		FLAG shouldFetchAndRenderWindowInsteadOfBG;
		FLAG delayedWindowActivationWX0;
		FLAG cachedWinEnablePerFrame;
		FLAG windowDisableGlitchPixel;
		FLAG ignoreSCXLowBitsAfterWindow;
		FLAG gfxOfCurrentScanLineUpdated;
		FLAG isNewM3Scanline;
		FLAG tileSelGlitch;       // 1-T-cycle pulse, set by CPU write, cleared after 1 PPU tick
		BYTE dataForSelGlitch;    // latched: updated after sprite render and end of mode 3
		BYTE latchedOldLCDC;
		BYTE latchedLCDC;
		COUNTER8 latchedLCDCForDelay;
		int16_t oamSearchCount;
		int16_t spriteCountPerScanLine;
		FLAG shouldSimulateBGScrollingPenaltyNow;
		PIXEL_FETCHER_STATES pixelFetcherState;
		struct pixelFetcherContext_t pixelFetcherContext;
		struct pixelFIFO_t bgWinPixelFIFO;
		struct pixelFIFO_t objPixelFIFO;
		struct pixelFIFO_t tempBgWinPixelFIFO;
		int16_t discardedPixelCount;
		int16_t discardedPixelCountForWin;
		BYTE xBGPerPixel;
		FLAG scxLatchedThisScanline;
		COUNTER8 cgbSCYDelayTCycles;
		BYTE cgbLatchedSCY;
		BYTE effectiveSCX;
		COUNTER8 cgbLYCDelayTCycles;
		BYTE cgbLatchedLYC;
		BYTE wxDelayTCycles;
		BYTE latchedWXForDelay;
		BYTE latchedWindowDiscardTarget;
		FLAG noPixelRenderedSinceWindowTrigger;
		FLAG prevDMGPixelIsBG;
		FLAG prevCGBPixelIsBG;
		FLAG prevDMGPixelIsOBJ;
		FLAG prevCGBPixelIsOBJ;
		SBYTE prevDMGPixelBGColor;
		SBYTE prevCGBPixelBGPalette;
		SBYTE prevCGBPixelBGColor;
		SBYTE prevDMGPixelOBJColor;
		SBYTE prevDMGPixelOBJPalette;
		SBYTE prevCGBPixelOBJColor;
		SBYTE prevCGBPixelOBJPalette;
		BYTE latchedWX;
		int16_t latchedXWindow;
		int16_t pixelFetcherDots;
		int16_t pixelRendererDots;
		int16_t pixelPipelineDots;
		int16_t pixelFetcherCounterPerScanLine;
		int16_t pixelRenderCounterPerScanLine;
		FLAG x159SpritesPresent;
		BYTE nX159SpritesPresent;
		FLAG x159SpritesDone;
		FLAG shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone;
		FLAG shouldFetchObjInsteadOfWinAndBgNow;
		FLAG isThereAnyObjectCurrentlyGettingRendered;
		int16_t indexOfOBJToFetchFromVisibleSpritesArray;
		visibleObjects_t* visibleObjectsPerScanLine;			// Linked List for the visible sprites per scanline
		visibleObjects_t arrayOfVisibleObjectsPerScanLine[TEN];	// Memory for the visibleObjectsPerScanLine
		FLAG wasFetchingOBJ;
		int16_t prevSpriteX;
		FLAG wasNotFirstSpriteInX;
		FLAG wasX0Object;
		FLAG abortObjectFetch;
		uint16_t addressInTileMapArea;
		uint16_t addressInTileDataArea;
		COUNTER8 tileSelGlitchTCycles;
		BYTE tileSelGlitchedData;
		BYTE cached_BG_WINDOW_TILE_DATA_AREA;
		BYTE cached_BG_TILE_VRAM_BANK_NUMBER;
		FLAG fetchDone;
		FLAG pushDone;
		FLAG bgToObjectPenalty;
		FLAG isTheLastVblankLine;
		FLAG forceLY153Compare;
		FLAG lycCompareSuppressed;
		FLAG blockVramR;
		FLAG blockOAMR;
		FLAG blockVramW;
		FLAG blockOAMW;
		FLAG blockCGBPalette;
		int16_t emulatedPPUCyclePerPPUMode;
		// ---- TODO : Need Memory Optmization ----------------------------
		uint16_t gfxVisibleColorMap_BG_WINDOW_OBJ[screen_height][screen_width];
		COLOR_FORMAT gfxVisible_BG_WINDOW_OBJ[screen_height][screen_width];
		union
		{
			Pixel imGuiBuffer1D[screen_width * screen_height];
			Pixel imGuiBuffer2D[screen_height][screen_width];
		} imGuiBuffer;
		COLOR_FORMAT gfx_BG_WINDOW[256][256];
		COLOR_FORMAT imGuiFullBuffer2D[256][256];
		// ----------------------------------------------------------------
		uint64_t filters;
		uint64_t debugVariable;
	} display_t;

	typedef struct
	{
		uint8_t bg_colorID : 2; // bit  0 - 1
		uint8_t bg_has_priority_for_cgb : 1; // bit  2
		uint8_t this_x_coordinate_is_already_populated_for_dmg : 1; // bit  3	
		uint8_t this_x_coordinate_is_already_populated_for_cgb : 1; // bit  4
		uint8_t spare05 : 1; // bit  5
		uint8_t spare06 : 1; // bit  6	
		uint8_t spare07 : 1; // bit  7
	} colorCacheFields_t;

	typedef union
	{
		colorCacheFields_t colorCacheFields;
		BYTE colorCacheMemory;
	} colorCacheMemory_t;

	typedef struct
	{
		uint64_t globalCounter;
		uint32_t keySamplingCounter;
		uint32_t lcdBlankCounter;
		uint64_t apuCounter;
		uint64_t cpuCounter;
		FLAG isDoubleSpeedHi;
		BYTE pad[3];
		uint16_t dividerCounter;
		uint16_t serialCounter;
		uint16_t ppuCounterPerLY;
		uint16_t ppuCounterPerMode;
		uint32_t ppuCounterPerFrame;
		uint16_t rtcDayCounter;
		uint16_t timerCounter;
		uint64_t rtcCounter;
		uint64_t stopCounter;
	} ticks_t;

	typedef struct
	{
		uint8_t HBLANK_SIGNAL : ONE; // bits 0
		uint8_t VBLANK_SIGNAL : ONE; // bits 1
		uint8_t OAM_SIGNAL : ONE; // bits 3
		uint8_t LY_LYC_SIGNAL : ONE; // bits 
		uint8_t UNUSED0 : ONE; // bits 5
		uint8_t UNUSED1 : ONE; // bits 6
		uint8_t UNUSED2 : ONE; // bits 7
		uint8_t UNUSED3 : ONE; // bits 8
	} STATInterruptSources_t;

	typedef union
	{
		STATInterruptSources_t STATInterruptSources;
		BYTE aggregateSignal;
	} STATInterruptSignal_t;

	struct debugger_t
	{
		FLAG wasDebuggerJustTriggerred;
		int64_t debuggerTriggerOnWhichLY;
		int64_t lyChangePersistance;
		struct
		{
			uint32_t testCount[TWOFIFTYSIX];
			uint32_t cbtestCount[TWOFIFTYSIX];
			struct
			{
				uint32_t indexer;
				struct
				{
					FLAG isRead;
					FLAG isWrite;
					uint8_t data;
					uint16_t address;

					void reset()
					{
						isRead = CLEAR;
						isWrite = CLEAR;
						data = RESET;
						address = RESET;
					};
				} cycles[TWENTY];
			} cycles;
		} tomHarte;
	};

	typedef enum
	{
		HBLANK,
		VBLANK,
		OAM,
		LY_LYC,
		NONE
	} STAT_INTR_SRC;

	enum class HUC3_MODES
	{
		RAM_RO	= 0x0,
		RAM_RW	= 0xA,
		RTC_W	= 0xB,
		RTC_R	= 0xC,
		RTC_RW	= 0xD,
		IR		= 0xE,
		UNKNOWN	= 0xF
	};

	enum class MMM01_MODES
	{
		UNMAPPED = 0x0,
		MAPPED = 0x1,
		UNKNOWN = 0xF
	};

	typedef struct
	{
		STAT_INTR_SRC STAT_src;
		STATInterruptSignal_t STATInterruptSignal;
		FLAG checkSTATTPlusOneCycle;
		byte prevSTAT;
		byte newSTAT;
		byte dataWrittenToMBCReg0;
		byte dataWrittenToMBCReg1;
		byte dataWrittenToMBCReg2;
		byte dataWrittenToMBCReg3;
		byte dataWrittenToMBCReg4;
		byte dataWrittenToMBCReg5;
		byte dataWrittenToMBCReg6;
		byte dataWrittenToMBCReg7;
		byte dataWrittenToMBCReg8;
		FLAG isMBC2ROMMode;
		FLAG isMBC1Mode1;
		MBCType activeMBC;
		ROMBankType romBank;
		union
		{
			struct
			{
				uint16_t romBankLo : FIVE; // bits 0 - 4
				uint16_t romBankHi_ramBank : TWO;	// bits 5 - 6
				uint16_t pad : NINE; // bits 7 - 16
			} mbc1Fields;
			struct
			{
				uint16_t romBankLo : FOUR; // bits 0 - 3
				uint16_t romBankHi_ramBank : TWO;	// bits 4 - 5
				uint16_t pad : TEN; // bits 76 - 16
			} mbc1mFields;
			struct
			{
				uint16_t romBankLo : FIVE; // bits 0 - 4
				uint16_t romBankMid_ramBankLo : TWO;	// bits 5 - 6
				uint16_t romBankHi : TWO;	// bits 7 - 8
				uint16_t pad : SEVEN; // bits 9 - 16
			} mmm01Fields;
			uint16_t raw;
		} currentROMBankNumber;
		uint16_t currentROMBankNumberB;
		struct
		{
			union
			{
				struct
				{
					BYTE ax2xAccXLo;
					BYTE ax3xAccXHi;
				} fields;
				uint16_t raw;
			} accX;
			union
			{
				struct
				{
					BYTE ax4xAccYLo;
					BYTE ax5xAccYHi;
				} fields;
				uint16_t raw;
			} accY;
			union
			{
				struct
				{
					BYTE ax6xAccZLo;
					BYTE ax7xAccZHi;
				} fields;
				uint16_t raw;
			} accZ;
			FLAG isErased;
			// EEPROM state
			FLAG     eepromCS;
			FLAG     eepromCLK;
			FLAG     eepromDI;
			FLAG     eepromDO;
			FLAG     eepromWriteEnabled;
			uint16_t eepromCommand;
			uint8_t  eepromArgBitsLeft;
			uint16_t eepromReadBits;
		} mbc7Regs;
		FLAG isHuc1IrMode;
		FLAG huc1EnableIrTx;
		FLAG huc1IrSignalRx;
		FLAG huc3EnableIrTx;
		FLAG huc3IrSignalRx;
		struct
		{
			BYTE command;
			BYTE argument;
			BYTE result;
			BYTE rtcSeconds; // internal seconds, not directly game-readable
			INC8 rtcMemIdx;
			BYTE rtcMem[0x80];
		} huc3Rtc;
		HUC3_MODES huc3Mode;
		RAMBankType ramBank;
		FLAG enableRAMBanking;
		FLAG isMBC7RamEn1;
		FLAG isMBC7RamEn2;
		uint8_t currentRAMBankNumber;
		uint8_t currentVRAMBankNumber;
		uint8_t currentWRAMBankNumber;
		uint8_t currentRAMBankNumberB;
		uint16_t serialMaxClockPerTransfer;
		uint16_t serialMasterByteShiftCount;
		uint16_t serialSlaveByteShiftCount;
		GB_SERIAL_DEVICE serialDevice;
		uint64_t serialDetectionShiftRegister;
		uint8_t serialDetectionBitCount;
#ifndef __RPI_PICO__
		struct
		{
			FLAG flashEnable;
			FLAG flashProtSec0; // for Protect/Unprotect Sector 0 flash commands
			FLAG flashEnSec0AndHidden;
			FLAG isFlashForA;
			FLAG isFlashForB;
			union
			{
				BYTE raw[0x100000];
				BYTE sector[8][0x20000];
				BYTE bank[0x80][0x2000]; // 0x2000 is same as the ROM bank size for MBC6!
			} flash;
			BYTE flashHidden[256];
			uint8_t flashCmdState; // 0=IDLE, 1=UNLOCK1, 2=UNLOCK2, 3=PROGRAM, 4=ERASE_SETUP, 5=ERASE_UNLOCK1, 6=ERASE_CMD
		} mbc6;
#endif // !__RPI_PICO__
		struct
		{
			FLAG isMMM01Mode1;
			BYTE ramBankMask;
			BYTE romBankMask;
			BYTE ramBankLo;
			BYTE ramBankHi;
			FLAG writeDisable;
			FLAG muxEnabled;
			BYTE mux0RomBankMid; // stores the romBankMid before mux was enabled
			MMM01_MODES mmm01Mode;
		} mmm01;
		struct
		{
			FLAG isCAMMode;
			FLAG startCapture;
			union
			{
				struct
				{
					// A000
					BYTE triggerStatus;
					// A001-A005
					BYTE configuration[5];
					union
					{
						// Raw register view (A006-A035)
						BYTE registers[48];
						// Decoded 4x4x3-byte matrix
						BYTE matrix[4][4][3];
					};
				};
				// Complete register space (A000-A035)
				BYTE allRegisters[54];
			};
			uint32_t captureTicksRemaining;
		} cameraUnit;
		struct
		{
			byte  MBChi;       // high ROM bank base (added to switchable bank number)
			FLAG  bcSelect;    // once set, game-select writes are locked out
			FLAG  bank0Change; // set when 0x0000 write has bits 7:6 == 0b11
		} poke2in1;
		FLAG m161OneBankSwitchDone;
		FLAG isBatteryAvailable;
		FLAG isCartRAMAvailable;
		FLAG isRTCAvailable;
		FLAG enableRTCAccessTimer;
		FLAG mapRTCRegisters;
		uint8_t currentRTCRegister;
		BYTE rtcFsm;
		FLAG keyUP;
		FLAG keyDOWN;
		FLAG keyLEFT;
		FLAG keyRIGHT;
		FLAG keySTART;
		FLAG keySELECT;
		FLAG keyA;
		FLAG keyB;
		FLAG isNewTimerCycle;
		SIGNAL timaIncSignal;
		SIGNAL fallingEdgeDetectorDelay;
		FLAG instantTimerIF;
		FLAG waitingToRequestTimerInterrupt;
		int16_t clocksAfterTIMAOverflow;
		FLAG isCPUHalted;
		FLAG isCPUJustHalted;
		FLAG isCPUStopped;
		FLAG stopLCD;
		FLAG stopLCDDone;
		FLAG stopKeepDrawingMode3;
		FLAG freezeLCDOneFrame;
		int32_t exitHaltInTCycles;
		enum HALT_BUG_STATE isHaltBugActivated;
		enum EI_ENABLE_STATE eiEnState;
		FLAG interruptMasterEn;	// IME
		FLAG gfxEn;
		FLAG isCPUExecutionBlocked;
		FLAG isDMAActive;
		uint16_t DMAStartDelay;
		uint16_t DMASource;
		uint16_t DMABytesTransferred;
		FLAG DMARestarted;
		uint16_t DMAEndDelayUponRestart;
		FLAG DMASTATGlitchEn;
		enum CGB_DMA_MODE cgbDMAMode;
		FLAG isHDMAActive;
		FLAG isHDMAAllowedToBlockCPUPipeline;
		uint16_t hDMASource;
		uint16_t hDMADestination;
		int16_t hDMATXLength;
		uint16_t hDMABytesTransferred;
		FLAG isGPDMAActive;
		FLAG isCGBDoubleSpeedMode;
		uint32_t checksum;
		uint64_t unusableMemoryReads;
		uint64_t unusableMemoryWrites;
		ticks_t ticks;
		debugger_t debugger;
	} emulatorStatus_t;

	// ---- TODO : Need Memory Optmization ----------------------------
	// Data stored in all the ROM memory banks of the cartridge
	typedef union
	{
		uint8_t mROMBanks[0x200][0x4000];
		uint8_t mROMBanks8KB[0x400][0x2000];
	} romMemoryBanks_t;

	typedef union
	{
		romMemoryBanks_t romMemoryBanks;
		uint8_t entireRomMemory[sizeof(romMemoryBanks_t)];
	} entireRom_t;

	// Data stored in all the RAM memory banks of the cartridge
	typedef union
	{
		uint8_t mRAMBanks[0x80][0x2000];
		uint8_t mRAMBanks4KB[0x100][0x1000];
	} ramMemoryBanks_t;

	typedef union
	{
		ramMemoryBanks_t ramMemoryBanks;
		uint8_t entireRamMemory[sizeof(ramMemoryBanks_t)];
	} entireRam_t;

	// Data stored in all the VRAM memory banks in CGB mode
	typedef struct
	{
		uint8_t mVRAMBanks[0x02][0x2000];
	} vramMemoryBanks_t;

	typedef union
	{
		vramMemoryBanks_t vramMemoryBanks;
#if DISABLED
		bgMapAttributes_t entireVramMemoryFields[sizeof(vramMemoryBanks_t)];
#endif
		uint8_t entireVramMemory[sizeof(vramMemoryBanks_t)];
	} entireVram_t;

	// Data stored in all the WRAM memory banks in CGB mode
	typedef struct
	{
		uint8_t mWRAM01Banks[0x07][0x1000];
	} wram01MemoryBanks_t;

	typedef union
	{
		wram01MemoryBanks_t wram01MemoryBanks;
		uint8_t entireWram01Memory[sizeof(wram01MemoryBanks_t)];
	} entireWram01_t;
	// ----------------------------------------------------------------

	// Palette ram of size 64 bytes for background and object 
	// Palette ram's data granularity is 16 bit; https://gbdev.io/pandocs/Palettes.html#ff69--bcpdbgpd-cgb-mode-only-background-color-palette-data--background-palette-data

	typedef struct
	{
		uint8_t LOWER_BYTE : 8; // bit 0 - 7	
		uint8_t HIGHER_BYTE : 8; // bit 7 - 15
	} gbcColorByteFields_t;

	typedef union
	{
		gbcColorByteFields_t gbcColorByteFields;
		uint16_t gbcColor;
	} gbcColor_t;

	typedef union
	{
		gbcColor_t paletteRAM[EIGHT][FOUR];
		uint8_t paletteRAMMemory[sizeof(gbcColor_t) * EIGHT * FOUR];
	} entireBackgroundPaletteRAM_t;

	typedef union
	{
		gbcColor_t paletteRAM[EIGHT][FOUR];
		uint8_t paletteRAMMemory[sizeof(gbcColor_t) * EIGHT * FOUR];
	} entireObjectPaletteRAM_t;

	struct GBcCameraDebugStages_t
	{
		FLAG hasData = NO;
		int rawWebcam[GBCAM_SENSOR_W][GBCAM_SENSOR_H];
		int postExposure[GBCAM_SENSOR_W][GBCAM_SENSOR_H];
		int postInvert[GBCAM_SENSOR_W][GBCAM_SENSOR_H];
		int postFilter[GBCAM_SENSOR_W][GBCAM_SENSOR_H];
		BYTE fourColor[GBCAM_W][GBCAM_H];
		BYTE finalTiles[14][16][16];
	};

	typedef struct
	{
		// core
		registers_t registers;
		cpu_t cpuInstance;
		//
		GBcMemory_t GBcMemory;
		//
		rtc_t rtcLatched;
		rtc_t rtc;
		//
		entireRom_t entireRom; // TODO : Need Memory Optmization
		entireRam_t entireRam; // TODO : Need Memory Optmization
		entireVram_t entireVram; // TODO : Need Memory Optmization
		entireWram01_t entireWram01; // TODO : Need Memory Optmization
		entireBackgroundPaletteRAM_t entireBackgroundPaletteRAM;
		entireObjectPaletteRAM_t entireObjectPaletteRAM;
		// semi - core
		display_t display; // TODO : Need Memory Optmization
		audio_t audio;
		// non - core
		quirks_t quirks;
		PALETTE_ID gb_palette;
		PALETTE_ID gbc_palette; // Used to handle GBC color correction
		GBcCameraDebugStages_t cameraDebugStages;
		emulatorStatus_t emulatorStatus;
	} GBc_state_t;

	typedef union
	{
		GBc_state_t GBc_state;
		uint8_t GBc_memoryState[sizeof(GBc_state_t)];
	} GBc_instance_t;

	typedef struct
	{
		FLAG isRomLoaded;
		uint32_t codeRomSize;
	} aboutRom_t;

	typedef struct
	{
		GBc_instance_t GBc_instance;
		aboutRom_t aboutRom;
	} absolute_GBc_state_t;

	union absolute_GBc_instance_t
	{
		absolute_GBc_state_t absolute_GBc_state;
		uint8_t GBc_absoluteMemoryState[sizeof(absolute_GBc_state_t)];
		absolute_GBc_instance_t()
		{
			memset(this, ZERO, sizeof(absolute_GBc_instance_t));
		}
	};

	std::shared_ptr <absolute_GBc_instance_t> pAbsolute_GBc_instance;
	GBc_instance_t* pGBc_instance = nullptr;			// for readability
	registers_t* pGBc_registers = nullptr;				// for readability
	cpu_t* pGBc_cpuInstance = nullptr;					// for readability
	GBcMemory_t* pGBc_memory = nullptr;					// for readability
	flagFields_t* pGBc_flags = nullptr;					// for readability
	IOFields_t* pGBc_peripherals = nullptr;				// for readability
	emulatorStatus_t* pGBc_emuStatus = nullptr;			// for readability
	display_t* pGBc_display = nullptr;			// for readability

	PACK_END

#pragma region GBC_DEBUGGER
		// Deliberately OUTSIDE the PACK_BEGIN/PACK_END (GBc_state_t) region above, so none
		// of this is ever save-stated / BESS-serialized. Pure UI + debug scratch state.
public:

#ifndef __RPI_PICO__

	enum class GBC_DEBUG_PIXEL_SAMPLE_MODE : uint8_t
	{
		PER_FRAME = 0,	// default -- identical cost to a non-debug run (existing once-per-vblank upload)
		PER_LY,			// screen texture refreshed once per scanline
		PER_DOT			// screen texture refreshed every dot (slow; foundation for the future per-dot pixel viewer)
	};

	enum class PIXEL_SOURCE_TAG : uint8_t {
		NONE = 0, BG, WINDOW, OBJ
	};

	// Extensible beyond PPU later -- CPU/APU registers can just be appended here.
	enum class GBC_DEBUG_TRACKED_REGISTER : uint8_t {
		LCDC = 0, STAT, SCX, SCY, LY, LYC, DMA, BGP, OBP0, OBP1, WX, WY, COUNT
	};

	struct PPUEvent_t
	{
		uint32_t frameNumber = ZERO;
		BYTE scanline = ZERO;
		uint16_t dot = ZERO;
		uint8_t registerIndex = ZERO;
		uint8_t oldValue = ZERO;
		uint8_t newValue = ZERO;
		uint16_t pc = ZERO;
	};

	enum class DEBUGGER_TAB
	{
		PPU,
		CPU,
		APU,
		EVENT_VIEWER
	};

	struct gbcDebugger_t
	{
		FLAG windowOpen = NO;					// Emulation -> Debug -> GBC

		struct ppu_t
		{
			FLAG enabled = NO;					// master switch. NO == zero extra cost, same as no debugger at all
			GBC_DEBUG_PIXEL_SAMPLE_MODE pixelOutputSampleMode = GBC_DEBUG_PIXEL_SAMPLE_MODE::PER_FRAME;

			FLAG showRegisters = YES;
			FLAG showTileViewer = YES;
			FLAG showBGMapViewer = YES;
			FLAG showWindowMapViewer = YES;
			FLAG showOAMViewer = YES;
			FLAG showPaletteViewer = YES;

			FLAG tileViewerUseBank1 = NO;			// CGB VRAM bank 0 or 1

			FLAG viewportShowBG = YES;
			FLAG viewportShowWindow = NO;
			FLAG viewportShowSprites = NO;
			FLAG viewportShowGrid = YES;
			FLAG viewportShowViewportRect = YES;	// Complete Viewport tab only -- overlay the SCX/SCY rectangle on the 256x256 map, wrapping
			FLAG tileViewerShowGrid = YES;
			FLAG bgMapViewerShowGrid = YES;		// BG Map panel only
			FLAG winMapViewerShowGrid = YES;		// Window Map panel only -- independent of the above

			int selectedOAMEntry = 0;
			FLAG oamUseGalleryView = YES;

			int selectedTileIndex = 0;
			int tileViewerPreviewPalette = ZERO;	// CGB only: which BG palette (0-7) to preview the selected tile through

			FLAG dockLayoutBuilt = NO;				// one-shot: default panel arrangement built?

			// ---- run / breakpoint state (see point 3 below) ----
			FLAG paused = NO;						// when YES, emulation is completely frozen
			FLAG stepRequested = NO;				// single-shot: advance exactly one processSOC() call, then re-pause
			FLAG runToBreakpointArmed = NO;		// running at full, undecorated speed toward (breakpointLY, breakpointDot)
			uint8_t breakpointLY = ZERO;
			uint16_t breakpointDot = ZERO;

			FLAG gridColorWhite = YES;	// applies to every grid overlay: Tiles, BG Map, Window Map, Complete Viewport
			FLAG fullscreen = NO;

			DEBUGGER_TAB activeTab = DEBUGGER_TAB::PPU;
		} ppu;

		struct eventViewer_t
		{
			static const int CAPACITY = 4096;

			FLAG enabled = NO;
			FLAG snapshotValid = NO;
			uint8_t lastValues[(int)GBC_DEBUG_TRACKED_REGISTER::COUNT] = { ZERO };
			FLAG showRegister[(int)GBC_DEBUG_TRACKED_REGISTER::COUNT] =
			{ YES, YES, YES, YES, YES, YES, YES, YES, YES, YES, YES, YES };

			// True ring buffer: once full, new writes overwrite the OLDEST entry (via head wrapping), rather than silently refusing to record anything further.
			PPUEvent_t ring[CAPACITY];
			int head = ZERO;	// next write slot (wraps)
			int count = ZERO;	// valid entries, caps at CAPACITY

			// STAT mode (0-3) captured every tick at (scanline, dot) -- independent of whether
			// any tracked register actually changed, so the mode bands are continuous.
			uint8_t modeTimeline[154][456] = { { ZERO } };

			uint32_t frameCounter = ZERO;
			int lastLY = -1;
		} eventViewer;

	} gbcDebugger;

	// Debug-only GL resources (created lazily, first time the debugger window opens)
	GLuint debugTileViewerTexture = ZERO;
	GLuint debugBGMapTexture = ZERO;
	GLuint debugWindowMapTexture = ZERO;
	GLuint debugOAMSpriteTexture = ZERO;
	GLuint debugMiniScreenTexture = ZERO;			// mirrors the live 160x144 screen, for the sprite-position preview
	GLuint debugTileDetailTexture = ZERO;			// the selected tile, decoded through whichever palette is chosen in the detail panel
	std::array<Pixel, 8 * 8> debugTileDetailPixels;
	GLuint debugLiveBGTexture = ZERO;
	GLuint debugLiveWindowTexture = ZERO;
	GLuint debugViewportTexture = ZERO;
	FLAG debugTexturesInitialized = NO;

	// Live per-pixel capture: exactly what the BG/Window fetcher actually produced this frame,
	// nothing more -- blank wherever that layer didn't contribute this frame.
	std::array<Pixel, 160 * 144> debugLiveBGPixels;
	std::array<Pixel, 160 * 144> debugLiveWindowPixels;

	// Complete Viewport: the real composited frame, tagged per-pixel with which layer
	// actually produced it -- captured in screen space (160x144), then translated into
	// map space (256x256) for display, since that's where Window/OBJ actually need to
	// land relative to a scrolling BG.
	std::array<Pixel, 160 * 144> debugViewportPixels;
	std::array<uint8_t, 160 * 144> debugViewportSource;	// holds PIXEL_SOURCE_TAG values
	std::array<Pixel, 256 * 256> debugViewportMapPixels;	// the actual 256x256 canvas rendered by the panel

	struct PixelDebugInfo_t
	{
		BYTE LY = ZERO;
		uint16_t pixelRenderCounterPerScanLine = ZERO;
		uint16_t ppuCounterPerLY = ZERO;
		uint16_t ppuCounterPerMode = ZERO;
		uint32_t ppuCounterPerFrame = ZERO;
		int pixelFetcherState = ZERO;
		BYTE capturedSCX = ZERO;	// SCX/SCY active AT THE MOMENT this pixel committed -- NOT
		BYTE capturedSCY = ZERO;	// the same as "current" SCX/SCY for raster-split ROMs that
		// change scroll mid-frame (see PPU_DEBUGGER.md for why this matters)
		FLAG captured = NO;	// was this screen-space pixel actually rendered this frame?
	};
	std::array<PixelDebugInfo_t, 160 * 144> debugViewportPixelInfo;

	// Click-selection, viewport tab only -- persists until clicked elsewhere or off-rect.
	FLAG viewportPixelSelected = NO;
	int viewportSelectedMapX = ZERO;
	int viewportSelectedMapY = ZERO;
	PixelDebugInfo_t viewportSelectedPixelInfo;

	int debugLastCapturedLY = -1;
	int debugLastPixelCounterCaptured = -1;
	uint8_t debugLastLYSeenByLoop = 0xFF;	// sentinel so the very first LY encountered still yields once

	// Debug-only CPU-side pixel scratch buffers -- kept completely separate from
	// gfxVisible*/imGuiBuffer so the hot PPU path never touches or grows because of these.
	std::array<Pixel, 128 * 192> debugTileViewerPixels;	// 16x24 tiles of 8x8px = all 384 tiles in one VRAM bank
	std::array<Pixel, 256 * 256> debugBGMapPixels;			// 32x32 tiles of 8x8px
	std::array<Pixel, 256 * 256> debugWindowMapPixels;
	std::array<Pixel, 8 * 16 * 40> debugOAMSpritePixels;	// 40 sprites, worst case 8x16

	GLuint debugBGMap9800Texture = ZERO;
	GLuint debugBGMap9C00Texture = ZERO;
	std::array<Pixel, 256 * 256> debugBGMap9800Pixels;
	std::array<Pixel, 256 * 256> debugBGMap9C00Pixels;
	int debugWindowPixelsCapturedThisFrame = ZERO;

	void renderGBCDebuggerUI();

	void debugSyncScreenIfNeeded();
	void debugEventViewerCheck();
	void renderGBCDebuggerEventViewerTab();

private:

	void renderGBCDebuggerPPUTab();
	void renderGBCDebuggerRegistersPanel();
	void renderGBCDebuggerTileViewerPanel();
	void renderGBCDebuggerBGMapPanel();
	void renderGBCDebuggerWindowMapPanel();
	void debugRebuildSpecificBGMap(uint16_t mapBaseOffset, std::array<Pixel, 256 * 256>& outBuffer);
	void renderGBCDebuggerViewportPanel();
	void debugRebuildViewportBGMapPixels();
	void renderGBCDebuggerOAMPanel();
	void renderGBCDebuggerPalettePanel();
	void debugEnsureTexturesCreated();
	void debugRebuildTileViewerPixels();
	MASQ_INLINE BYTE debugReadVRAM(uint8_t bank, uint16_t offsetWithinBank)
	{
		if (ROM_TYPE == ROM::GAME_BOY)
		{
			// DMG has no VRAM banking (bank is always conceptually 0) and its real storage is the
			// flat memory-map union at the raw $8000+offset address, NOT entireVram -- that array
			// is exclusively the CGB path and is simply never written to for a DMG ROM.
			RETURN pGBc_instance->GBc_state.GBcMemory.GBcRawMemory[0x8000 + offsetWithinBank];
		}
		RETURN pGBc_instance->GBc_state.entireVram.vramMemoryBanks.mVRAMBanks[bank][offsetWithinBank];
	}
	void debugRebuildTileDetailPixels(int tileIdx, uint8_t bank, int paletteIdx);
	void debugRebuildOAMSpritePixels();

#endif // !__RPI_PICO__

#pragma endregion GBC_DEBUGGER

#pragma region GBC_SERIAL_LINK
#ifndef __RPI_PICO__
public:

	FLAG isSerialLinkConnected();
	FLAG tickSerialLink(BYTE outgoingByte, BYTE* outReceivedByte);
	FLAG tickSerialLinkAsSlave(BYTE outgoingByte, BYTE* outReceivedByte);
#endif // !__RPI_PICO__
#pragma endregion GBC_SERIAL_LINK

private:

	IInputBackend* pInputBackend = nullptr;

private:

#ifndef __RPI_PICO__
	SDL_AudioStream* audioStream = nullptr;
#endif // !__RPI_PICO__

private:

#ifndef __RPI_PICO__
	// TODO : Placeholder to handle network
#endif // !__RPI_PICO__

private:

#ifndef __RPI_PICO__
	GBcPrinterEngine_t gbPrinterEngine;
#endif // !__RPI_PICO__

private:

#ifndef __RPI_PICO__
	std::deque<GBc_state_t> gamePlay;
#endif // !__RPI_PICO__

private:

#ifndef __RPI_PICO__
	GLuint gbcStageTex_rawWebcam = 0;
	GLuint gbcStageTex_postExposure = 0;
	GLuint gbcStageTex_postInvert = 0;
	GLuint gbcStageTex_postFilter = 0;
	GLuint gbcStageTex_fourColor = 0;
	GLuint gbcStageTex_finalOutput = 0;
	GBcCameraEngine_t gbCameraEngine;
	SDL_Camera* pCameraBackend = nullptr;
#endif // !__RPI_PICO__

private:

#ifndef __RPI_PICO__
	GBcBarcodeEngine_t gbBarcodeEngine;
#endif // !__RPI_PICO__

public:

#ifndef __RPI_PICO__
	FLAG show_gbc_capture_stages_window = NO;
	BYTE camera_capture_timing_percent = 1;
	uint16_t cam_exposure_divisor = 0x3D00;
#endif // !__RPI_PICO__

private:

	static const uint16_t TYPE_BG_WIN = (ZERO << FIFTEEN);
	static const uint16_t TYPE_OBJ = (ONE << FIFTEEN);
	uint16_t mapPalette[8192 /* TODO: Calculate this based on VRAM instead of using magic numbers */] = {ZERO}; // MSB == 1 (OBJ) ; MSB == 0 (BG)
	std::set<BYTE> visibleOamIndexPerLY;
	std::set<BYTE> visibleOamIndexPerFrame;
	//std::map<uint16_t, std::string> mapAsm;

#pragma endregion EMULATION_DECLARATIONS

#pragma region BESS
PACK_BEGIN
private:

#ifndef __RPI_PICO__
	// BESS specifications
	// Refer to https://github.com/LIJI32/SameBoy/blob/master/BESS.md

	enum class BESS_BLOCKS
	{
		BESS_NAME,
		BESS_INFO,
		BESS_CORE,
		BESS_XOAM,
		BESS_MBC,
		BESS_RTC,
		BESS_END,
		BESS_HEADER,
		BESS_FOOTER,
		BESS_TOTAL
	};

	struct BESS_FOOTER_t
	{
		uint32_t off_blk_0;								// 4 bytes
		char ascii_tag[0x04];							// 4 bytes
	};

	struct BESS_BLOCK_HEADER_t
	{
		char ascii_ident[0x04];							// 4 bytes
		uint32_t blk_len;								// 4 bytes
	};

	struct BESS_BLOCK_NAME_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		char name_ver[0x20];							// 32 bytes
	};

	struct BESS_BLOCK_INFO_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		char title[0x10];								// 16 bytes
		uint16_t chksum;								// 2 bytes
	};

	struct BESS_BLOCK_CORE_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		uint16_t maj_bess_ver;							// 2 bytes		-> offset 0x00
		uint16_t min_bess_ver;							// 2 bytes		-> offset 0x02
		char mdl_indent[0x04];							// 4 bytes		-> offset 0x04
		uint16_t pc;									// 2 bytes		-> offset 0x08
		uint16_t af;									// 2 bytes		-> offset 0x0A
		uint16_t bc;									// 2 bytes		-> offset 0x0C
		uint16_t de;									// 2 bytes		-> offset 0x0E
		uint16_t hl;									// 2 bytes		-> offset 0x10
		uint16_t sp;									// 2 bytes		-> offset 0x12
		uint8_t ime;									// 1 byte		-> offset 0x14
		uint8_t ie;										// 1 byte		-> offset 0x15
		uint8_t exec_state;								// 1 byte		-> offset 0x16
		uint8_t rsv;									// 1 byte		-> offset 0x17
		uint8_t mmr[0x80];								// 128 bytes	-> offset 0x18
		uint32_t size_ram;								// 4 bytes		-> offset 0x98
		uint32_t off_ram;								// 4 bytes		-> offset 0x9C
		uint32_t size_vram;								// 4 bytes		-> offset 0xA0
		uint32_t off_vram;								// 4 bytes		-> offset 0xA4
		uint32_t size_mbcram;							// 4 bytes		-> offset 0xA8
		uint32_t off_mbcram;							// 4 bytes		-> offset 0xAC
		uint32_t size_oam;								// 4 bytes		-> offset 0xB0
		uint32_t off_oam;								// 4 bytes		-> offset 0xB4
		uint32_t size_hram;								// 4 bytes		-> offset 0xB8
		uint32_t off_hram;								// 4 bytes		-> offset 0xBC
		uint32_t size_obj_pram;							// 4 bytes		-> offset 0xC0
		uint32_t off_bg_pram;							// 4 bytes		-> offset 0xC4
		uint32_t size_bg_pram;							// 4 bytes		-> offset 0xC8
		uint32_t off_obj_pram;							// 4 bytes		-> offset 0xCC
	};

	struct BESS_BLOCK_XOAM_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		uint8_t xoam[0x60];								// 96 bytes
	};

#pragma warning(disable: 4200)
	struct BESS_BLOCK_MBC_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		uint8_t mbc[];									// variable bytes
	};
#pragma warning(default: 4200)

	struct BESS_BLOCK_RTC_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
		uint8_t curr_sec;								// 1 byte
		uint8_t pad0[0x03];								// 3 bytes
		uint8_t curr_min;								// 1 byte
		uint8_t pad1[0x03];								// 3 bytes
		uint8_t curr_hr;								// 1 byte
		uint8_t pad2[0x03];								// 3 bytes
		uint8_t curr_day;								// 1 byte
		uint8_t pad3[0x03];								// 3 bytes
		uint8_t curr_ovf;								// 1 byte
		uint8_t pad4[0x03];								// 3 bytes
		uint8_t latched_sec;							// 1 byte
		uint8_t pad5[0x03];								// 3 bytes
		uint8_t latched_min;							// 1 byte
		uint8_t pad6[0x03];								// 3 bytes
		uint8_t latched_hr;								// 1 byte
		uint8_t pad7[0x03];								// 3 bytes
		uint8_t latched_day;							// 1 byte
		uint8_t pad8[0x03];								// 3 bytes
		uint8_t latched_ovf;							// 1 byte
		uint8_t pad9[0x03];								// 3 bytes
		uint64_t unix_time;								// 8 bytes
	};

	// TODO: BESS_BLOCK_HUC3_t
	// TODO: BESS_BLOCK_TPP1_t
	// TODO: BESS_BLOCK_MBC7_t
	// TODO: BESS_BLOCK_SGB_t

	struct BESS_BLOCK_END_t
	{
		BESS_BLOCK_HEADER_t BESS_BLOCK_HEADER;			// 8 bytes
	};

	// Internal BESS blocks

	struct BESS_BLOCKS_PRESENT_t
	{

	};
#endif // !__RPI_PICO__

PACK_END
#pragma endregion BESS

#pragma region INFRASTRUCTURE_METHOD_DECLARATION
public:

	GBc_t(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, MasqConfig_t& config, CheatEngine_t* ce = nullptr);
	void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* input = nullptr, void* network = nullptr, void* camera = nullptr) override;
	void sendBiosToEmulator(bios_t* bios = nullptr) override {};

	FLAG resetAudio(void* audio = nullptr) override { 
		RETURN SUCCESS; 
	};
	FLAG resetInput(void* input = nullptr) override {
		RETURN SUCCESS;
	};
	FLAG resetNetwork(void* network = nullptr) override {
		RETURN SUCCESS;
	};
	FLAG resetCamera(void* camera = nullptr);

public:

	float getVersion();
	MASQ_INLINE uint32_t getScreenWidth() override
	{
		RETURN this->screen_width;
	}
	MASQ_INLINE uint32_t getScreenHeight() override
	{
		RETURN this->screen_height;
	}
	MASQ_INLINE uint32_t getPixelWidth() override
	{
		RETURN this->pixel_width;
	}
	MASQ_INLINE uint32_t getPixelHeight() override
	{
		RETURN this->pixel_height;
	}
	void setEmulationWindowOffsets(uint32_t x, uint32_t y, FLAG isEnabled);
	uint32_t getTotalScreenWidth() override;
	uint32_t getTotalScreenHeight() override;
	uint32_t getTotalPixelWidth() override;
	uint32_t getTotalPixelHeight() override;
	void setScreenWidth(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setScreenHeight(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setPixelWidth(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setPixelHeight(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setTotalScreenWidth(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setTotalScreenHeight(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setTotalPixelWidth(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	void setTotalPixelHeight(uint32_t size) override
	{
		MASQ_UNUSED(size);
	}
	const char* getEmulatorName() override;
	float getEmulationFPS() override;
	void setEmulationID(EMULATION_ID ID) override;
	EMULATION_ID getEmulationID() override;

public:

	FLAG getRomLoadedStatus() override;
	void randomizeRAM();
	FLAG loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) override;
	void dumpRom() override;

public:

	void blarggConsoleOutput();

public:

	//std::map<uint16_t, std::string> disassemble(uint16_t nStart, uint16_t nStop);

	void dumpCPURegisters(int x, int y, FLAG dumpCPU);
	void dumpCode(int x, int y, int nLines, FLAG dumpCode);
	void dumpGFXData(int x1, int y1, FLAG dumpVRAM, int x2, int y2, FLAG dumpPalette, int x3, int y3, FLAG dumpOAM, int x4, int y4, FLAG dumpBG, int x5, int y5, FLAG dumpInfo, FLAG hoverCheck);
	void dumpCartInfo(int x5, int y5, FLAG dumpCartInfo);
	void optimizedClearScreen(FLAG shouldPerform);
	void runDebugger();
#pragma endregion INFRASTRUCTURE_METHOD_DECLARATION

#pragma region EMULATION_METHOD_DECLARATION
private:

	const char* cartridgeLicName();
	const char* cartridgeTypeName();

private:

	FLAG isCGBDoubleSpeedEnabled();
	void toggleCGBSpeedMode();
	FLAG isCGBCompatibilityModeEnabled();

private:

	FLAG isBatteryAvailable();
	FLAG isCartRAMAvailable();

	FLAG isRTCAvailable();
	void enableRTCAccess();
	void disableRTCAccess();
	FLAG isRTCAccessEnabled();

	void setRTCFSM(uint8_t fsmState);
	uint8_t getRTCFSM();

	uint8_t getRTCRegisterNumber();
	void shouldMapRTCToExternalRAM(FLAG shouldMapRTC);
	FLAG isRTCMappedToExternalRAM();
	void setRTCRegisterNumber(uint8_t rtcRegisterNumber);
	int readFromRTCRegisterIfApplicable();
	uint16_t getRTCDayCounter();
	void setRTCDayCounter(uint16_t dayCounterValue);
	void writeToRTCRegisterIfApplicable(uint8_t data);

	void latchRTCRegisters();

private:

	MASQ_INLINE void initMBC() const
	{
		pGBc_emuStatus->activeMBC = MBCType::NONE;
		RETURN;
	}
	MASQ_INLINE FLAG isNoMBC() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::NONE;
	}
	MASQ_INLINE FLAG isMBC1() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::MBC1;
	}
	MASQ_INLINE FLAG isMBC1M() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::MBC1M;
	}
	MASQ_INLINE FLAG isMBC2() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::MBC2;
	}
	MASQ_INLINE FLAG isMBC3() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::MBC3;
	}
	MASQ_INLINE FLAG isMBC5() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::MBC5;
	}
	MASQ_INLINE FLAG isMBC6() const
	{
#ifdef __RPI_PICO__
		FATAL("MBC6 is not supported");
		RETURN NO;
#else
		RETURN pGBc_emuStatus->activeMBC == MBCType::MBC6;
#endif
	}
	MASQ_INLINE FLAG isMBC7() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::MBC7;
	}
	MASQ_INLINE FLAG isMMM01() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::MMM01;
	}
	MASQ_INLINE FLAG isM161() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::M161;
	}
	MASQ_INLINE FLAG isHUC1() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::HUC1;
	}
	MASQ_INLINE FLAG isHUC3() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::HUC3;
	}
	MASQ_INLINE FLAG isWT() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::WISDOM_TREE;
	}
	MASQ_INLINE FLAG isGameBoyCamera() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::POCKET_CAMERA;
	}
	MASQ_INLINE FLAG isPoke2in1() const
	{
		RETURN pGBc_emuStatus->activeMBC == MBCType::POKE_2IN1;
	}
	void setMBCType(uint16_t mbcType, MBCType force = MBCType::INVALID_MBC);
	void setROMBankType(uint16_t romBankType);
	void setROMBankNumber(uint16_t romBankNumber);
	uint16_t getROMBankNumber();
	uint16_t getNumberOfROMBanksUsed();
	void setSimpleModeInMBC1();
	void setAdvancedModeInMBC1();
	FLAG getMBCModeInMBC1();

	void setSimpleModeInMMM01();
	void setAdvancedModeInMMM01();
	FLAG getMBCModeInMMM01();

	void enableRAMBank();
	void disableRAMBank();
	FLAG isRAMBankEnabled();
	void setRAMBankType(uint16_t ramBankType);
	uint8_t getRAMBankNumber();
	void setRAMBankNumber(uint8_t ramBankNumber);

	uint8_t getNumberOfRAMBanksUsed();
	uint8_t getVRAMBankNumber();
	void setVRAMBankNumber(uint8_t vramBankNumber);
	uint8_t getWRAMBankNumber();
	void setWRAMBankNumber(uint8_t wramBankNumber);

	// For MBC6
	void setROMBankNumberB(uint16_t romBankNumber);
	uint16_t getROMBankNumberB();
	uint8_t getRAMBankNumberB();
	void setRAMBankNumberB(uint8_t ramBankNumber);

public:

	// For MBC6
#ifndef __RPI_PICO__
	void processMBC6FlashWrite(uint16_t cpuAddr, BYTE data);
#endif // !__RPI_PICO__

public:

	// For MMM01
	void updateMMM01RamBanking();

public:

	void doCameraCapture();
	void CreateGBCStageTextures();
	void UploadStageGrayscale(GLuint tex, const int* src, int w, int h, int srcColStride);
	void UploadStageFourColor(GLuint tex, const BYTE* src, int w, int h, int srcColStride);
	void UploadStageFinalOutput(GLuint tex, const BYTE finalTiles[14][16][16]);
	void RenderGBCCaptureStagesUI();

public:

	BYTE getGBDividerMSB();
	BYTE getGBDividerLSB();
	void setGBDividerMSB(BYTE value);
	void setGBDividerLSB(BYTE value);
	BIT getDIVSpecialBitStatus(TIMERS timer);

	SIGNAL getTIMASignalForGB();
	BYTE getGBTimer();
	void setGBTimer(BYTE value);
	void resetGBTimerToZero();
	void resetGBTimer(uint8_t resetVal);
	TIMERS getWhichGBTimerToUse();

public:

	void processDMA();
	void processGPDMA();
	void processHDMA();

public:

	void updateJOYP(STATE8 prevState);
	void captureIO();

public:

	void processSerialClockSpeedBit();
	FLAG sendOverSerialLink(BIT bitToSend);
	void detectSerialDevice(BIT bitToSend);
	void resetSerialDeviceDetection();
	GB_SERIAL_DEVICE getSerialDevice() const;

public:

	void resetDivAPU(uint32_t value);
	void incrementDivAPU(uint32_t nCycles);
	DIM16 getChannelPeriod(AUDIO_CHANNELS channel);
	FLAG enableChannelWhenTriggeredIfDACIsEnabled(AUDIO_CHANNELS channel);
	void continousDACCheck();
	FLAG isDACEnabled(AUDIO_CHANNELS channel);
	FLAG isChannel3Active();
	void tickChannel(AUDIO_CHANNELS channel, uint32_t tCycles);
	void processSoundLength();
	SDIM32 getUpdatedFrequency();
	void processFrequencySweep();
	void processEnvelopeSweep();
	BYTE getLogicalAmplitude(AUDIO_CHANNELS channel);
	float getDACOutput(AUDIO_CHANNELS channel);
	float finHPF(float sampleIn);
	void captureDownsampledAudioSamples();
	void playTheAudioFrame();

public:

	void freezeLCD();
	void setPPULCDMode(LCD_MODES lcdMode);
	LCD_MODES getPPULCDMode();
	FLAG isPPULCDEnabled();
	void compareLYToLYC(ID LY);
	void requestVblankStatInterrupt();
	void requestOamStatInterrupt();
	void requestHblankStatInterrupt();
	void processLCDEnable();
	void processLCDDisable();
	BYTE getColorNumberFromColorIDForGB(BYTE palette, BYTE colorID);
	COLOR_FORMAT getColorFromColorIDForGB(BYTE palette, BYTE colorID);
	COLOR_FORMAT getColorFromColorIDForGBC(uint16_t colorID, FLAG isColorCorrectionEnabled);
	void setPaletteIndexForCGB(FLAG isThisForBackground, uint8_t value);
	void setPaletteColorForCGB(FLAG isThisForBackground, uint8_t value);
	void processPixelPipelineAndRender(int32_t dots);
	void translateGFX(PALETTE_ID from, PALETTE_ID to, PALETTE_ID colorCorrectionBefore, PALETTE_ID colorCorrectionAfter);
	void displayCompleteScreen();
	void OAMDMASTATModeGlitch();

public:

	MASQ_INLINE void barcodeScan(const BYTE* barcode)
	{
		gbBarcodeEngine.barcodeScan(barcode);
	}

private:

	void loadQuirks();

public:

	FLAG saveState(uint8_t id = 0) override;
	FLAG loadState(uint8_t id = 0) override;

	FLAG absoluteSaveState(uint8_t id);
	FLAG absoluteLoadState(uint8_t id);

	FLAG fillGamePlayStack() override;
	FLAG rewindGamePlay() override;

	FLAG bessSaveState(uint8_t id = 0);
	void bessIoSeq(uint8_t* mmr, uint8_t size);
	FLAG bessLoadState(uint8_t id = 0);

public:

	FLAG runEmulationAtHostRate(uint32_t currentFrame) override;
	FLAG runEmulationLoopAtHostRate(uint32_t currentFrame) override;
	FLAG runEmulationAtFixedRate(uint32_t currentFrame) override;
	FLAG runEmulationLoopAtFixedRate(uint32_t currentFrame) override;
	FLAG onKeyEvent(EmuKey key, EmuKeyAction action) override;

public:

	float getEmulationVolume() override;
	void setEmulationVolume(float volume) override;

public:

	void initializeGraphics();
	void initializeAudio();
	void reInitializeAudio();
	FLAG initializeEmulator() override;
	void destroyEmulator() override;
#pragma endregion EMULATION_METHOD_DECLARATION

#pragma region SM83_METHOD_DECLARATION
private:
	
	void cpuSetRegister(REGISTER_TYPE rt, uint16_t u16parameter);
	uint16_t cpuReadRegister(REGISTER_TYPE rt);
	void cpuWritePointer(POINTER_TYPE mrt, uint16_t u16parameter);
	BYTE cpuReadPointer(POINTER_TYPE mrt);

	byte readRawMemory(uint16_t address
		, MEMORY_ACCESS_SOURCE source
		, FLAG FirstPriority_readFromVRAMBank01ForCGB = false
		, FLAG SecondPriority_readFromVRAMBank00ForCGB = false);
	void executeHUC3ExtendedCommand();
	void executeHUC3Command();
	void writeRawMemory(uint16_t address, byte data, MEMORY_ACCESS_SOURCE source);

	MASQ_INLINE void stackPush(BYTE data)
	{
		(pGBc_registers->sp)--;
		bool wasBlocked = handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		if (!wasBlocked) // TODO: Confirm this behaviour; Source : Sameboy
		{
			writeRawMemory(pGBc_registers->sp, data, MEMORY_ACCESS_SOURCE::CPU);
		}
	}
	MASQ_INLINE BYTE stackPop(FLAG triggerOAMCorruption = NO)
	{
		handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::READ);
		BYTE popedData = readRawMemory(pGBc_registers->sp, MEMORY_ACCESS_SOURCE::CPU);
		(pGBc_registers->sp)++;
		if (triggerOAMCorruption == YES)
		{
			handleOAMCorruption(pGBc_registers->sp, OAM_ACCESS_TYPE::WRITE);
		}
		RETURN popedData;
	}
	MASQ_INLINE void processZeroFlag(byte value)
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
	MASQ_INLINE void processUnusedFlags(BYTE result)
	{
		pGBc_flags->ZEROTH = result;
		pGBc_flags->FIRST = result;
		pGBc_flags->SECOND = result;
		pGBc_flags->THIRD = result;
	}
	MASQ_INLINE void processUnusedJoyPadBits(BYTE value)
	{
		pGBc_peripherals->P1_JOYP.joyPadFields.JP_SPARE_06 = value;
		pGBc_peripherals->P1_JOYP.joyPadFields.JP_SPARE_07 = value;
	}
	MASQ_INLINE void processUnusedIFBits(BYTE value)
	{
		pGBc_peripherals->IF.interruptRequestFields.NO_INT05 = value;
		pGBc_peripherals->IF.interruptRequestFields.NO_INT06 = value;
		pGBc_peripherals->IF.interruptRequestFields.NO_INT07 = value;
	}
	void processFlagsForLogicalOperation
	(
		byte value,
		FLAG isOperationAND
	);
	void processFlagsFor8BitAdditionOperation
	(
		byte value1,
		byte value2,
		FLAG includeCarryInOperation,
		FLAG affectsCarryFlag = true
	);
	void processFlagsFor16BitAdditionOperation
	(
		uint16_t value1,
		uint16_t value2,
		FLAG includeCarryInOperation,
		FLAG setSZPoF = true
	);
	void processFlagsFor8BitSubtractionOperation
	(
		byte value1,
		byte value2,
		FLAG includeCarryInOperation,
		FLAG affectsCarryFlag = true
	);
	void processFlagsFor16BitSubtractionOperation
	(
		uint16_t value1,
		uint16_t value2,
		FLAG includeCarryInOperation,
		FLAG affectsCarryFlag = true
	);
	void processFlagFor0xE8And0xF8
	(
		byte value1,
		byte value2
	);

private:

	MASQ_INLINE uint16_t GET_PC()
	{
		RETURN pGBc_registers->pc;
	}

	MASQ_INLINE void SET_PC(uint16_t pc)
	{
		pGBc_registers->pc = pc;
	}

	MASQ_INLINE void INCREMENT_BC_BY_ONE()
	{
		pGBc_registers->bc.bc_u16memory++;
	}

	MASQ_INLINE void INCREMENT_DE_BY_ONE()
	{
		pGBc_registers->de.de_u16memory++;
	}

	MASQ_INLINE void INCREMENT_HL_BY_ONE()
	{
		pGBc_registers->hl.hl_u16memory++;
	}

	MASQ_INLINE FLAG INCREMENT_PC_BY_ONE()
	{
		if (pGBc_instance->GBc_state.emulatorStatus.isHaltBugActivated == HALT_BUG_STATE::HALT_BUG_ENABLED)
		{
			pGBc_instance->GBc_state.emulatorStatus.isHaltBugActivated = HALT_BUG_STATE::HALT_BUG_DISABLED;
			RETURN YES;
		}
		else
		{
			pGBc_registers->pc++;
			RETURN NO;
		}
	}

	MASQ_INLINE void INCREMENT_SP_BY_ONE()
	{
		pGBc_registers->sp++;
	}

	MASQ_INLINE void DECREMENT_BC_BY_ONE()
	{
		pGBc_registers->bc.bc_u16memory--;
	}

	MASQ_INLINE void DECREMENT_DE_BY_ONE()
	{
		pGBc_registers->de.de_u16memory--;
	}

	MASQ_INLINE void DECREMENT_HL_BY_ONE()
	{
		pGBc_registers->hl.hl_u16memory--;
	}

	MASQ_INLINE void DECREMENT_PC_BY_ONE()
	{
		pGBc_registers->pc--;
	}

	MASQ_INLINE void DECREMENT_SP_BY_ONE()
	{
		pGBc_registers->sp--;
	}

private:

	FLAG processSOC();
	void runCPUPipeline();
	void dumpCpuStateToConsole();
	void unimplementedInstruction();

private:

	MASQ_INLINE void handleStopBasedHalt()
	{
		if (pGBc_instance->GBc_state.emulatorStatus.exitHaltInTCycles > RESET) MASQ_UNLIKELY
		{
			if (--pGBc_instance->GBc_state.emulatorStatus.exitHaltInTCycles == RESET)
			{
				pGBc_instance->GBc_state.emulatorStatus.isCPUHalted = NO;
				pGBc_instance->GBc_state.emulatorStatus.isCPUJustHalted = NO; // precaution
			}
		}
	}
	void cpuTickM(int32_t specAddress = INVALID, int32_t specData = INVALID, CPU_TICK_TYPE type = CPU_TICK_TYPE::READ_WRITE);
	void gbCpuTick2T(FLAG isT2orT3, int32_t specAddress = INVALID, int32_t specData = INVALID);
	void syncOtherGBModuleTicks(int32_t specAddress = INVALID, int32_t specData = INVALID);
	MASQ_INLINE FLAG isDoubleSpeedTickHi() const
	{
		RETURN (pGBc_instance->GBc_state.emulatorStatus.ticks.isDoubleSpeedHi == YES);
	}
	MASQ_INLINE void setNextTickForDoubleSpeed()
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.isDoubleSpeedHi = !pGBc_instance->GBc_state.emulatorStatus.ticks.isDoubleSpeedHi;
	}
	MASQ_INLINE void resetTickForDoubleSpeed()
	{
		pGBc_instance->GBc_state.emulatorStatus.ticks.isDoubleSpeedHi = RESET_TICK;
	}
	MASQ_INLINE void tickDotClockModules(FLAG onHI, int32_t specAddress = INVALID, int32_t specData = INVALID)
	{
		// Encapsulated gating logic
		if (isCGBDoubleSpeedEnabled() == NO || isDoubleSpeedTickHi() == onHI)
		{
#if (GB_GBC_ENABLE_HIGHER_ORDER_SPECULATION == YES)
			if (onHI == YES)
			{
				speculativeCpuMemWrite(specAddress, specData, SPECULATION_ORDER::SECOND);
			}
			else
#endif
			{
				speculativeCpuMemWrite(specAddress, specData, SPECULATION_ORDER::FIRST);
			}
			rtcTick();
			ppuTick();
			apuTick();
		}
	}
	void dmaTick();
	void joypadTick();
	void timerTick();
	void serialTick();
	void rtcTick();
	void cameraTick();
	static MASQ_INLINE uint16_t readOAMWord(const BYTE* OAM, int byteOffset)
	{
		RETURN (uint16_t)OAM[byteOffset] | ((uint16_t)OAM[byteOffset + 1] << 8);
	}
	static MASQ_INLINE void writeOAMWord(BYTE* OAM, int byteOffset, uint16_t value)
	{
		OAM[byteOffset + 0] = (BYTE)(value & 0xFF);
		OAM[byteOffset + 1] = (BYTE)(value >> 8);
	}
	MASQ_INLINE FLAG handleOAMCorruption(uint16_t value, OAM_ACCESS_TYPE type)
	{
		if ((value < 0xFE00 || value > 0xFEFF) || ROM_TYPE != ROM::GAME_BOY)
		{
			RETURN NO;
		}

		if (pGBc_display->currentLCDMode != LCD_MODES::MODE_LCD_SEARCHING_OAM)
		{
			RETURN NO;
		}

		auto* OAM = pGBc_memory->GBcMemoryMap.mOAM.OAMMemory;

		/*
		* 
		*   OAM (160 bytes)
		*
		*	Row 0   FE00-FE07
		*	+-------------------------------+
		*	| Sprite 0 | Sprite 1 |
		*	+-------------------------------+
		*
		*	Row 1   FE08-FE0F
		*	+-------------------------------+
		*	| Sprite 2 | Sprite 3 |
		*	+-------------------------------+
		*
		*	Row 2   FE10-FE17
		*	+-------------------------------+
		*	| Sprite 4 | Sprite 5 |
		*	+-------------------------------+
		*
		*	Row 3   FE18-FE1F
		*	+-------------------------------+
		*	| Sprite 6 | Sprite 7 |
		*	+-------------------------------+
		*
		*	Row 4   FE20-FE27
		*	+-------------------------------+
		*	| Sprite 8 | Sprite 9 |
		*	+-------------------------------+
		*
		*	...
		*
		*	Row 15  FE78-FE7F
		*	+-------------------------------+
		*	| Sprite 30 | Sprite 31 |
		*	+-------------------------------+
		*
		*	Row 16  FE80-FE87
		*	+-------------------------------+
		*	| Sprite 32 | Sprite 33 |
		*	+-------------------------------+
		*
		*	Row 17  FE88-FE8F
		*	+-------------------------------+
		*	| Sprite 34 | Sprite 35 |
		*	+-------------------------------+
		*
		*	Row 18  FE90-FE97
		*	+-------------------------------+
		*	| Sprite 36 | Sprite 37 |
		*	+-------------------------------+
		*
		*	Row 19  FE98-FE9F
		*	+-------------------------------+
		*	| Sprite 38 | Sprite 39 |
		*	+-------------------------------+
		* 
		*/

		const uint8_t currentRow = pGBc_display->oamSearchCount / 2;

		if (currentRow == ZERO || pGBc_display->oamSearchCount >= FORTY)
		{
			RETURN NO;
		}

		const int cur = currentRow * EIGHT;
		const int prev = cur - EIGHT;

		if (type == OAM_ACCESS_TYPE::WRITE)
		{
			uint16_t a = readOAMWord(OAM, cur);
			uint16_t b = readOAMWord(OAM, prev);
			uint16_t c = readOAMWord(OAM, prev + 4);

			uint16_t result = ((a ^ c) & (b ^ c)) ^ c;
			writeOAMWord(OAM, cur, result);

			for (uint8_t i = TWO; i < EIGHT; i++)
			{
				OAM[cur + i] = OAM[prev + i];
			}
			RETURN YES;
		}

		// ---- READ corruption: branches on which "quadrant" of 4 rows we're in ----
		switch (cur & 0x18)
		{
		case 0x10: // "secondary" - reaches back 2 rows
		{
			const int prev2 = cur - 16;

			uint16_t rPrev2 = readOAMWord(OAM, prev2);
			uint16_t rPrev = readOAMWord(OAM, prev);
			uint16_t rCur = readOAMWord(OAM, cur);
			uint16_t rPrev4 = readOAMWord(OAM, prev + 4);

			uint16_t result = (rPrev & (rPrev2 | rCur | rPrev4)) | (rPrev2 & rCur & rPrev4);
			writeOAMWord(OAM, prev, result);

			for (uint8_t i = 0; i < EIGHT; i++)
			{
				OAM[prev2 + i] = OAM[prev + i];
			}
			break;
		}

		case 0x00: // "tertiary"/"quaternary" - reaches back up to 4 rows
		{
			const int prev2 = cur - 16;
			const int prev3 = cur - 32;

			uint16_t rCur = readOAMWord(OAM, cur);
			uint16_t rPrev4 = readOAMWord(OAM, prev + 4);
			uint16_t rPrev = readOAMWord(OAM, prev);
			uint16_t rPrev2 = readOAMWord(OAM, prev2);
			uint16_t rPrev3 = readOAMWord(OAM, prev3);

			uint16_t result;

			if (cur == 0x40)
			{
				// Quaternary - DMG-revision formula.
				uint16_t rowZero = readOAMWord(OAM, 0);
				uint16_t rMid1 = readOAMWord(OAM, cur - 6);  // 2nd word of prev row
				uint16_t rMid2 = readOAMWord(OAM, cur - 14); // 2nd word of prev2 row

				result = (rPrev & (rPrev3 | rPrev2 | (~rMid1 & rMid2) | rPrev4 | rCur))
					| (rPrev4 & rPrev2 & rPrev3);
			}
			else if (cur == 0x20)
			{
				result = (rPrev & (rCur | rPrev4 | rPrev2 | rPrev3)) | (rCur & rPrev4 & rPrev2 & rPrev3);
			}
			else if (cur == 0x60)
			{
				result = (rPrev & (rCur | rPrev4 | rPrev2 | rPrev3)) | (rPrev4 & rPrev2 & rPrev3);
			}
			else
			{
				result = rPrev | (rCur & rPrev4 & rPrev2 & rPrev3);
			}

			writeOAMWord(OAM, prev, result);

			for (uint8_t i = 0; i < EIGHT; i++)
			{
				OAM[prev2 + i] = OAM[prev3 + i] = OAM[prev + i];
			}
			break;
		}

		default: // 0x08 / 0x18 - the plain case
		{
			uint16_t rCur = readOAMWord(OAM, cur);
			uint16_t rPrev = readOAMWord(OAM, prev);
			uint16_t rPrev4 = readOAMWord(OAM, prev + 4);

			uint16_t result = rPrev | (rCur & rPrev4);
			writeOAMWord(OAM, prev, result);
			writeOAMWord(OAM, cur, result);
			break;
		}
		}

		// Unconditional for every read branch above: the accessed row
		// fully becomes a copy of the (now-updated) previous row.
		for (uint8_t i = 0; i < EIGHT; i++)
		{
			OAM[cur + i] = OAM[prev + i];
		}

		// Edge case: row 0x80 additionally clones itself back onto row 0.
		if (cur == 0x80)
		{
			for (uint8_t i = 0; i < EIGHT; i++)
			{
				OAM[i] = OAM[cur + i];
			}
		}

		RETURN YES;
	}
	MASQ_INLINE void checkWindowYTrigger(uint8_t ly)
	{
		// Check whether "Y" window layer is triggerred for current scanline
		// Refer : https://gbdev.io/pandocs/Scrolling.html#window
		// Refer : https://discord.com/channels/465585922579103744/465586075830845475/852208456491728897
		// Refer : https://discord.com/channels/465585922579103744/465586075830845475/1295044210654842980

		// Note that WINDOW_LAYER_ENABLE should not be checked here as mentioned in https ://discord.com/channels/465585922579103744/465586075830845475/757342004052099072
		pGBc_display->yConditionForWindowIsMetForCurrentFrame |= (ly == pGBc_peripherals->WY);
	}
	MASQ_INLINE void activateWindow()
	{
#if (GB_GBC_ENABLE_WINDESYNC_GLITCH == YES)
		pGBc_display->cachedWinEnablePerFrame = YES;
#endif

		// All conditions for window are met; use this flag to increment the window line counter as well.
		pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = YES;
		pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;
		pGBc_display->bgWinPixelFIFO.clearFIFO();
		pGBc_display->tempBgWinPixelFIFO.clearFIFO();
		pGBc_display->fetchDone = NO;
		pGBc_display->pushDone = YES;

		// Latch the WX-derived window origin at the moment of trigger.
		pGBc_display->latchedWX = pGBc_peripherals->WX;
		pGBc_display->latchedXWindow = (int16_t)((int16_t)pGBc_peripherals->WX - SEVEN);
		pGBc_display->latchedWindowDiscardTarget = (pGBc_peripherals->WX < SEVEN) ? (BYTE)(SEVEN - pGBc_peripherals->WX) : ZERO;
		
		// Increment window line counter.
		pGBc_display->windowLineCounter++;

#if (GB_GBC_ENABLE_WIN_REACTIVATION_GLITCH == YES)
		pGBc_display->noPixelRenderedSinceWindowTrigger = YES;
#endif

		pGBc_display->pixelFetcherCounterPerScanLine = pGBc_display->pixelRenderCounterPerScanLine;
	}
	MASQ_INLINE void deactivateWindow()
	{
		pGBc_display->shouldFetchAndRenderWindowInsteadOfBG = NO;
		pGBc_display->pixelFetcherState = PIXEL_FETCHER_STATES::WAIT_FOR_TILE;
		pGBc_display->bgWinPixelFIFO.clearFIFO();
		pGBc_display->tempBgWinPixelFIFO.clearFIFO();
		pGBc_display->fetchDone = NO;
		pGBc_display->pushDone = YES;
		pGBc_display->pixelFetcherCounterPerScanLine = pGBc_display->pixelRenderCounterPerScanLine;
	}
	MASQ_INLINE void processLCDCTransition(BYTE oldLCDC, BYTE newLCDC)
	{
		// LCD/PPU enabled
		// https://www.reddit.com/r/Gameboy/comments/a1c8h0/what_happens_when_a_gameboy_screen_is_disabled/
		// https://forums.nesdev.org/viewtopic.php?t=12990
		if ((GETBIT(SEVEN, oldLCDC) == ZERO) && (GETBIT(SEVEN, newLCDC) == ONE))
		{
			// LCD cannot be enabled instantaneously
			processLCDEnable();
		}
		// LCD/PPU disabled
		// https://forums.nesdev.org/viewtopic.php?f=20&t=16434#p203762
		// https://www.reddit.com/r/Gameboy/comments/a1c8h0/what_happens_when_a_gameboy_screen_is_disabled/
		// https://forums.nesdev.org/viewtopic.php?t=12990
		else if ((GETBIT(SEVEN, oldLCDC) == ONE) && (GETBIT(SEVEN, newLCDC) == ZERO))
		{
			processLCDDisable();
		}
	}
	void ppuTick();
	void apuTick();
	MASQ_INLINE void abortObjectFetch()
	{
		PPUTODO("The abortObjectFetch logic is currently disabled as this DOESN'T WORK and messes up mealybug tests");
#if (DEACTIVATED)
		//if (pGBc_display->abortObjectFetch == YES) MASQ_UNLIKELY
		//{
		//	pGBc_display->wasFetchingOBJ = NO;
		//	pGBc_display->shouldFetchObjInsteadOfWinAndBgNow = NO;
		//	pGBc_display->shouldFetchObjInsteadOfWinAndBgPostBGFetchIsDone = NO;
		//	pGBc_display->isThereAnyObjectCurrentlyGettingRendered = NO;
		//	pGBc_display->abortObjectFetch = NO;
		//}
#endif
	}
	MASQ_INLINE void speculativeCpuMemWrite(uint16_t address, BYTE data, SPECULATION_ORDER order = SPECULATION_ORDER::NONE)
	{
		const FLAG isMode3 = (pGBc_display->currentLCDMode == LCD_MODES::MODE_LCD_DISPLAY_PIXELS);

#if (GB_GBC_ENABLE_BGP_OBP_MID_SCANLINE_GLITCH == YES)
		if (((uint16_t)(address - BGP_ADDRESS) <= (uint16_t)(OBP1_ADDRESS - BGP_ADDRESS)) && order == SPECULATION_ORDER::FIRST)
		{
			typedef typename std::remove_pointer<decltype(pGBc_peripherals)>::type PeripheralStructType;
			static const size_t PALETTE_LUT[] = 
			{
				offsetof(PeripheralStructType, BGP),  // Index 0 maps to 0xFF47
				offsetof(PeripheralStructType, OBP0), // Index 1 maps to 0xFF48
				offsetof(PeripheralStructType, OBP1)  // Index 2 maps to 0xFF49
			};

			// Check if we are in the pixel transfer mode (Mode 3)
			if (isMode3)
			{
				BYTE* const target = (BYTE*)((char*)pGBc_peripherals + PALETTE_LUT[address - BGP_ADDRESS]);

				if (ROM_TYPE == ROM::GAME_BOY_COLOR)
				{
					if (isCGBCompatibilityModeEnabled() == YES)
					{
						/*
						* CGB Mid-Scanline Update (Time Travel)
						*
						* Hardware Behavior (Model E):
						* Newer CGB revisions do not exhibit the DMG "OR-glitch".
						* However, the write still takes effect immediately, meaning the
						* previous pixel fetcher should retroactively use the new BGP/OBP0/OBP1 value.
						*/

						*target = data;
					}
				}
				else
				{
					/*
					* BGP/OBP0/OBP1 Mid-Scanline "OR-Glitch" (Time Travel)
					*
					* Hardware Behavior:
					* On DMG consoles, writing to the BGP/OBP0/OBP1 register during MODE_3 (Pixel Transfer)
					* causes a bus conflict. The PPU's pixel fetcher briefly reads the result of
					* (current_BGP/OBP0/OBP1 | new_value) due to the register write cycle timing.
					*
					* Implementation (Time Travel):
					* Since our PPU renders dots sequentially, a write at the current dot effectively
					* impacts the preceding pixel's palette. We "time travel" by patching the
					* previously rendered pixel in our frame buffer using the 'prevDMGPixelBGColor'
					* state, retroactively applying the bitwise OR effect before the new register
					* value is committed.
					*/

					*target |= data;
				}
			}
			RETURN;
		}
#endif
#if (GB_GBC_ENABLE_SCX_MID_SCANLINE_GLITCH == YES)
		if (address == SCX_ADDRESS && order == SPECULATION_ORDER::FIRST)
		{
			// Check if we are in the pixel transfer mode (Mode 3)
			if (isMode3)
			{
				if ((ROM_TYPE == ROM::GAME_BOY) || (ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBDoubleSpeedEnabled() == YES))
				{
					pGBc_peripherals->SCX = data;
				}
			}
			RETURN;
		}
#endif
#if (GB_GBC_ENABLE_SCY_MID_SCANLINE_GLITCH == YES)
		if (address == SCY_ADDRESS && order == SPECULATION_ORDER::FIRST)
		{
			// Check if we are in the pixel transfer mode (Mode 3)
			if (isMode3)
			{
				if (ROM_TYPE == ROM::GAME_BOY)
				{
					pGBc_peripherals->SCY = data;
				}
			}
			RETURN;
		}
#endif
#if (GB_GBC_ENABLE_LCDC_MID_SCANLINE_GLITCH == YES)
		if (address == LCDC_ADDRESS && order == SPECULATION_ORDER::FIRST)
		{
			// Check if we are in the pixel transfer mode (Mode 3)
			if (isMode3)
			{
				if (ROM_TYPE == ROM::GAME_BOY_COLOR && isCGBDoubleSpeedEnabled() == YES)
				{
					PPUTODO("For CGB doublespeed, the doublespeed HI and LO cycle handling for LCDC mode 3 glitch is not handled; timing is most likely not complete");
					lcdControl_t oldLCDC = { RESET };
					lcdControl_t newLCDC = { RESET };

					oldLCDC.lcdControlMemory = pGBc_peripherals->LCDC.lcdControlMemory;
					pGBc_display->latchedOldLCDC = oldLCDC.lcdControlMemory;

					newLCDC.lcdControlMemory = data;

#define LCDC_BG_EN_MASK      (1U << ZERO)
#define LCDC_ENABLE_MASK     (1U << SEVEN)
					lcdControl_t tempLCDC = newLCDC;
					tempLCDC.lcdControlMemory =
						(newLCDC.lcdControlMemory & ~(LCDC_BG_EN_MASK | LCDC_ENABLE_MASK)) |
						(oldLCDC.lcdControlMemory & (LCDC_BG_EN_MASK | LCDC_ENABLE_MASK));
#undef LCDC_BG_EN_MASK
#undef LCDC_ENABLE_MASK

#if (GB_GBC_ENABLE_TILE_SEL_GLITCH == YES)
					BIT oldTileSel = GETBIT(FOUR, oldLCDC.lcdControlMemory);
					BIT newTileSel = GETBIT(FOUR, newLCDC.lcdControlMemory);
					FLAG triggerGlitch = (oldTileSel != newTileSel);	// 0->1 or 1->0
					if (triggerGlitch == YES)
					{
						pGBc_display->tileSelGlitch = YES;
						pGBc_display->tileSelGlitchTCycles = ONE;
					}
#endif

					pGBc_display->latchedLCDC = newLCDC.lcdControlMemory;
					pGBc_display->latchedLCDCForDelay = ONE;
					pGBc_peripherals->LCDC.lcdControlMemory = tempLCDC.lcdControlMemory;
				}
				else if ((ROM_TYPE == ROM::GAME_BOY))
				{
					const auto display_counter = pGBc_display->pixelRenderCounterPerScanLine;
					const auto should_fetch_obj = pGBc_display->shouldFetchObjInsteadOfWinAndBgNow;

					lcdControl_t oldLCDC = { RESET };
					lcdControl_t newLCDC = { RESET };

					oldLCDC.lcdControlMemory = pGBc_peripherals->LCDC.lcdControlMemory;
					pGBc_display->latchedOldLCDC = oldLCDC.lcdControlMemory;

					newLCDC.lcdControlMemory = data;

					if (newLCDC.lcdControlFields.OBJ_ENABLE == RESET &&
						(display_counter == ZERO || should_fetch_obj == YES))
					{
						oldLCDC.lcdControlFields.OBJ_ENABLE = RESET;
						pGBc_display->abortObjectFetch = YES;
					}

					PPUTODO("Sameboy implements the below condition but for this seems to break the mealybug test which sameboy is also failing! so deactivating out for now...");
#if (DEACTIVATED)
					if (newLCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE)
					{
						oldLCDC.lcdControlFields.BG_WINDOW_LAYER_ENABLE = SET;
					}
#endif

					pGBc_peripherals->LCDC.lcdControlMemory = oldLCDC.lcdControlMemory;
				}
			}
			RETURN;
		}
#endif

		MASQ_UNUSED(order);
	}

public:

	void checkAllSTATInterrupts(FLAG isFF);
	FLAG isInterruptReadyToBeServed();
	void requestInterrupts(INTERRUPTS interrupt);
	FLAG handleInterruptsIfApplicable(FLAG effectiveIME, FLAG effectiveInterruptQ);
#pragma endregion SM83_METHOD_DECLARATION
};
#pragma endregion CORE