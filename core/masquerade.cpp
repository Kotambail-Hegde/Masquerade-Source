#pragma region TRICKS
// masquerade.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// Shortcuts:
// CTRL + M + O will collapse all.
// CTRL + M + L will expand all. (in VS 2013 - Toggle All outlining)
// CTRL + M + P will expand all and disable outlining.
// CTRL + M + M will collapse / expand the current section.
// CTRL + M + A will collapse all even in Html files
//
#pragma endregion TRICKS

#pragma region REFERENCES
#pragma endregion REFERENCES

// =========================================================
// INCLUDES — emulator modules are always compiled
// =========================================================
#pragma region INCLUDES
#include "helpers.h"
#include "abstractEmulation.h"
#include "defaults.h"
#if MASQ_ENABLE_GOL
#include "gameOfLife.h"
#endif
#if MASQ_ENABLE_CHIP8
#include "chip8.h"
#endif
#if MASQ_ENABLE_SI
#include "spaceInvaders.h"
#endif
#if MASQ_ENABLE_PACMAN
#include "pacMan.h"
#endif
#if MASQ_ENABLE_NES
#include "nes.h"
#endif
#if MASQ_ENABLE_GBC
#include "gbc.h"
#endif
#if MASQ_ENABLE_GBA
#include "gba.h"
#endif
#pragma endregion INCLUDES

// =========================================================
// DESKTOP-ONLY INCLUDES  (SDL / ImGui / STB / DWM)
// None of these are available or needed on RPI Pico.
// =========================================================
#ifndef __RPI_PICO__

#pragma region STB_INCLUDES
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma endregion STB_INCLUDES

#pragma region WINDOWS_INCLUDES
#if defined(_WIN32) && (ENABLED_IMGUI_DEFAULT_THEME == NO)
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif
#pragma endregion WINDOWS_INCLUDES

#endif // !__RPI_PICO__

// =========================================================
// GLOBAL INFRASTRUCTURE DECLARATIONS
// =========================================================
#pragma region GLOBAL_INFRASTRUCTURE_DECLARATIONS

// --- Args
int   gArgc = 0;
char** gArgv = nullptr;

// --- Emscripten / desktop mode flag
#ifdef __EMSCRIPTEN__
FLAG inEnscriptenMode = YES;
#else
FLAG inEnscriptenMode = NO;
#endif

// --- Logging
MAP64 ENABLE_LOGS = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000;
FLAG  isAppLoggingEnabled = YES;

#ifndef __RPI_PICO__
ImGuiLogBuffer appLog;
FLAG           isImGuiInitialized = NO;

std::vector<std::string> preImGuiLogBuffer;
std::mutex               preImGuiLogMutex;
#endif

// --- Master config instance
//     Desktop : boost::property_tree::ptree (populated from CONFIG.ini)
//     Pico    : PicoConfig_t stub (defaults come from pico_config.h)
static MasqConfig_t config;

// --- Desktop-only UI / path / OpenGL state
#ifndef __RPI_PICO__

// Theme
int     currentEmuTheme = SE_THEME_LIGHT;
int     previousEmuTheme = SE_THEME_LIGHT;
uint8_t customSEpalettes[FIVE * FOUR];

// Init screen
#ifdef __EMSCRIPTEN__
FLAG initScreen = YES;
#else
FLAG initScreen = NO;
#endif

// Drag-n-drop / menu-triggered ROM selection
std::vector<std::string> dynamicDragNDropAndMenuSelect;

// Recently opened items
const DIM8           _MAX_RECENTLY_USED_LIST_SIZE = EIGHT;
std::string          recentlyOpenedListPath;
std::deque<std::string> recentlyOpenedList;

// Menu-click control flags
FLAG quitOnMenuClick = NO;
FLAG rebootNeededOnMenuClick = NO;
FLAG saveContextOnReboot = NO;
FLAG startFromBoot = NO;

// Path strings (populated in postPrimaryBootLoader)
std::string exeName = "masquerade.exe";
std::string _BIOS_LOCATION;
std::string _CONFIG_LOCATION;
std::string _EXE_LOCATION;
std::string _IMGUI_LOCATION;
std::string _SAVE_LOCATION;
std::string _CHEAT_SAVE_LOCATION;
std::string _UI_INTERNAL_LOCATION;
std::string _FONT_LOCATION;

// OpenGL framebuffer / texture / shader handles
float    emuWindowX = 0.0f;
float    emuWindowY = 0.0f;
float    emuWindowMaxX = 0.0f;
float    emuWindowMaxY = 0.0f;
uint32_t frame_buffer = 0;
uint32_t masquerade_texture = 0;
uint32_t shaderProgramBasic = 0;
uint32_t shaderProgramBlend = 0;
uint32_t fullscreenVAO = 0;
uint32_t fullscreenVBO = 0;
uint32_t FRAME_BUFFER_SCALE = 4;

#pragma region IMGUI_SPECIFIC_DECLARATIONS
// IMGUI demo
const FLAG RUN_IMGUI_DEMO = NO;

// IMGUI default window settings
const std::string imguiDefaultIni = R"(
[Window][Debug##Default]
ViewportPos=2541,1373
ViewportId=0x16723995
Size=400,32
Collapsed=0

[Window][Emulation Window (Chip8)]
Pos=0,19
Size=585,391
Collapsed=0
DockId=0x00000001,0

[Window][Emulation Window (NES)]
Pos=0,19
Size=528,493
Collapsed=0
DockId=0x00000001,0

[Window][Emulation Window (GB-GBC)]
Pos=0,19
Size=336,301
Collapsed=0
DockId=0x00000001,0

[Window][Emulation Window (Game Of Life)]
Pos=0,19
Size=1552,781
Collapsed=0
DockId=0x00000001,0

[Window][Emulation Window (Space Invaders)]
Pos=0,19
Size=464,525
Collapsed=0
DockId=0x00000001,0

[Window][Emulation Window (GBA)]
Pos=0,19
Size=2560,1350
Collapsed=0
DockId=0x00000001,0

[Window][WindowOverViewport_11111111]
Pos=0,19
Size=336,301
Collapsed=0

[Window][Emulation Window (Masquerade)]
Pos=0,19
Size=236,125
Collapsed=0
DockId=0x00000001,0

[Window][Open GB/GBC]
Pos=61,140
Size=608,400
Collapsed=0

[Window][Emulation Window (PacMan)]
Pos=0,19
Size=528,589
Collapsed=0
DockId=0x00000001,0

[Window][Updater]
Pos=0,19
Size=236,125
Collapsed=0
DockId=0x00000001,1

[Window][Credits]
Pos=-1016,-398
Size=528,493
Collapsed=0
DockId=0x00000001,1

[Window][Logger Console]
Pos=0,19
Size=2560,1350
Collapsed=0
DockId=0x00000001,1

[Window][Cheats]
Pos=2186,19
Size=374,1350
Collapsed=0
DockId=0x00000002,0

[Docking][Data]
DockSpace   ID=0x08BD597D Window=0x1BBC0F80 Pos=1112,555 Size=336,301 Split=X Selected=0x5F0147C1
  DockNode  ID=0x00000001 Parent=0x08BD597D SizeRef=497,413 CentralNode=1 HiddenTabBar=1 Selected=0x5F0147C1
  DockNode  ID=0x00000002 Parent=0x08BD597D SizeRef=374,413 Selected=0x97A6199F
)";
#pragma endregion IMGUI_SPECIFIC_DECLARATIONS

#endif // !__RPI_PICO__

// --- Emulation state (always present)
INC8  numberOfRomsSelected = RESET;
std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> romsToRun;

FLAG          isBiosEnabled = NO;
unsigned long crcTable[256] = { 0 };
ROM           ROM_TYPE = ROM::NO_ROM;

debugConfig_t debugConfig;
FLAG          HitSkipPoint = NO;
FLAG          HitBreakPoint = NO;

FLAG     _ENABLE_AUDIO = NO;
FLAG     _MUTE_AUDIO = NO;
FLAG     _ENABLE_DISASSEMBLER = NO;
uint32_t _XSCALE = ONE;
FLAG     _ENABLE_FRAME_LIMIT = NO;
uint32_t _XFPS = ONE;
FLAG     _ENABLE_NETWORK = NO;
int32_t  _NETWORK_TIMEOUT_LIMIT = ONE;
FLAG     _ENABLE_QUICK_SAVE = YES;
FLAG     _ENABLE_BESS_FORMAT = NO;
FLAG     _ENABLE_REWIND = NO;
uint32_t _REWIND_BUFFER_SIZE = 5000;
int32_t  _TEST_NUMBER = INVALID;
FLAG     _ENABLE_ACCURATE_INPUT_SAMPLING = NO;

// Indicates that an absolute save-state output is loaded instead of a valid ROM
FLAG isAbsoluteLoad = NO;

// FIR filter sample buffer
double bufferForFIR[2048];

// Current actual FPS (written by desktop render loop, informational on Pico)
float _ACTUAL_FPS = 0.0f;

// Network support
uint32_t nEmulationInstanceID = ZERO;
FLAG     bWaitingForConnection = YES;

// Profiler
static uint32_t profilerFrameRate = ZERO;

// Video / palette state (palette IDs still relevant on Pico Waveshark display)
VIDEO_FILTERS currEnVFilter = VIDEO_FILTERS::NEAREST_FILTER;
PALETTE_ID    currEnGbPalette = PALETTE_ID::PALETTE_1;
PALETTE_ID    currEnGbcPalette = PALETTE_ID::PALETTE_1;

// NES Zapper Support
FLAG enableZapper = NO;

#pragma endregion GLOBAL_INFRASTRUCTURE_DECLARATIONS

// =========================================================
// GLOBAL INFRASTRUCTURE DEFINITION
// =========================================================
#pragma region GLOBAL_INFRASTRUCTURE_DEFINITION

// ---- Profiler ------------------------------------------
// timerStart uses std::thread — not available on bare-metal Pico
#pragma region PROFILER
#ifndef __RPI_PICO__

void timerStart(std::function<void(void)> func, unsigned int interval)
{
	std::thread([func, interval]()
		{
			while (true)
			{
				auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
				func();
				std::this_thread::sleep_until(x);
			}
		}).detach();
}

void displayNonPGEBasedFPS()
{
	if (debugConfig._DEBUG_PROFILER == true)
	{
		INFO("Non-PGE FPS: %d", profilerFrameRate);
		profilerFrameRate = ZERO;
	}
}

#endif // !__RPI_PICO__

void runProfiler()
{
	if (debugConfig._DEBUG_PROFILER == true)
	{
		++profilerFrameRate;
	}
}
#pragma endregion PROFILER

// ---- Emscripten persistent-FS helpers ------------------
#pragma region EMSCRIPTEN
#ifdef __EMSCRIPTEN__
FLAG SavePersistentFSComplete = NO;
FLAG ClearPersistentFSComplete = NO;

extern "C" void onSavePersistentFSComplete() {
	SavePersistentFSComplete = YES;
}
extern "C" void onClearPersistentFSComplete() {
	ClearPersistentFSComplete = YES;
}

extern "C" {

	EMSCRIPTEN_KEEPALIVE
		void listEmFilesRecursive(const std::string& path = "/")
	{
		DIR* dir = opendir(path.c_str());
		if (!dir)
		{
			INFO("Cannot open directory: %s", path.c_str()); RETURN;
		}

		dirent* entry;
		while ((entry = readdir(dir)) != nullptr)
		{
			std::string name = entry->d_name;
			if (name == "." || name == "..") continue;

			std::string fullPath = path + (path.back() == '/' ? "" : "/") + name;
			INFO("%s", fullPath.c_str());

			struct stat st;
			if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
				listEmFilesRecursive(fullPath);
		}
		closedir(dir);
	}

	EMSCRIPTEN_KEEPALIVE
		void listEmFiles()
	{
		INFO("____________________________________");
		listEmFilesRecursive("/");
		INFO("____________________________________");
	}

	using Callback = void(*)();

	EMSCRIPTEN_KEEPALIVE
		EM_JS(void, mountPersistentFS, (Callback callback), {
			FS.mkdir('/persistent');
			FS.mount(IDBFS, {}, '/persistent');
			FS.syncfs(true, function(err) {
				if (err)
		 {
console.error("syncfs error", err);
}
else
{
console.log("syncfs complete"); dynCall('v', callback);
}
});
			});

	EMSCRIPTEN_KEEPALIVE
		EM_JS(void, savePersistentFS, (Callback callback), {
			FS.syncfs(false, function(err) {
				if (err)
		 {
console.error("Error saving persistent FS:", err);
}
else
{
console.log("Saved persistent FS to IndexedDB"); dynCall('v', callback);
}
});
			});

	EMSCRIPTEN_KEEPALIVE
		EM_JS(void, clearPersistentFS, (Callback callback), {
			FS.readdir('/persistent').forEach(function(file) {
				if (file !== '.' && file !== '..')
		 {
try
{
FS.unlink('/persistent/' + file);
}
catch (e)
{
console.warn("Couldn't delete", file, e);
}
}
});
FS.syncfs(false, function(err) {
	if (err)
{
console.error("Error clearing persistent FS:", err);
}
else
{
console.log("Cleared persistent FS from IndexedDB"); dynCall('v', callback);
}
});
			});

} // extern "C"
#endif // __EMSCRIPTEN__
#pragma endregion EMSCRIPTEN

// ---- STB image helpers (desktop only) ------------------
#ifndef __RPI_PICO__
#pragma region STB

// Simple helper function to load an image into an OpenGL texture with common settings
FLAG LoadImageTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
{
	int image_width = 0;
	int image_height = 0;

	unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	GLuint image_texture;
	GL_CALL(glGenTextures(1, &image_texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, image_texture));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data));
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;
	return true;
}

// Open and read a file, then forward to LoadImageTextureFromMemory()
FLAG LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
{
	FILE* f = fopen(file_name, "rb");
	if (f == NULL) RETURN false;

	fseek(f, 0, SEEK_END);
	size_t file_size = (size_t)ftell(f);
	if (file_size == -1) RETURN false;

	fseek(f, 0, SEEK_SET);
	void* file_data = IM_ALLOC(file_size);
	fread(file_data, 1, file_size, f);
	fclose(f);

	FLAG ret = LoadImageTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
	IM_FREE(file_data);
	return ret;
}

#pragma endregion STB
#endif // !__RPI_PICO__

#pragma endregion GLOBAL_INFRASTRUCTURE_DEFINITION

// =========================================================
// CORE — Emulation_t class
// =========================================================
#pragma region CORE

class Emulation_t
{
	// ---- ROM / file-dialog filter declarations ---------------
	// Emscripten uses plain string constants; desktop uses NFD
	// filter structs; Pico needs neither (ROM is compiled-in).
public:

#ifdef __EMSCRIPTEN__
	char const* golfilters = { ".gol,.GOL" };
	char const* chip8filters = { ".ch8,.CH8,.c8,.C8,.sc8,.SC8,.xo8,.XO8" };
	char const* spaceInvadersFilter = { ".e,.f,.g,.h,.zip" };
	char const* pacmanFilter = { ".1m,.3m,.4a,.5e,.5f,.6e,.6f,.6h,.6j,.7f,.zip" };
	char const* nesFilter = { ".nes,.NES" };
	char const* gbGbcFilter = { ".gb,.gbc,.GB,.GBC" };
	char const* gbaFilter = { ".gba,.GBA" };

#elif !defined(__RPI_PICO__) // native desktop (NFD)
	nfdu8filteritem_t bootromfilters[1] = { { "Boot ROM",                               "bin,BIN"                         } };
	nfdu8filteritem_t golfilters[1] = { { "Game Of Life States",                    "gol,GOL"                         } };
	nfdu8filteritem_t chip8filters[1] = { { "Chip8/S-Chip/XO-Chip/Modern-Chip8 ROMs", "ch8,CH8,c8,C8,sc8,SC8,xo8,XO8"  } };
	nfdu8filteritem_t spaceInvadersFilter[1] = { { "SpaceInvaders ROMs",                     "e,f,g,h"                         } };
	nfdu8filteritem_t spaceInvadersAudioFilter[1] = { { "SpaceInvaders Audio",               "wav,WAV"                         } };
	nfdu8filteritem_t pacmanFilter[1] = { { "Pacman/MsPacman ROMs",                   "1m,3m,4a,5e,5f,6e,6f,6h,6j,7f"  } };
	nfdu8filteritem_t nesFilter[1] = { { "NES ROMs",                               "nes,NES"                         } };
	nfdu8filteritem_t gbGbcFilter[1] = { { "GB/GBC ROMs",                            "gb,gbc,GB,GBC"                   } };
	nfdu8filteritem_t gbaFilter[1] = { { "GBA ROMs",                               "gba,GBA"                         } };
#endif // ROM filter declarations

private:

	std::string appName = "Masquerade Emulator";

	abstractEmulation_t* current_instance = nullptr;

	float    myFPS = (float)DEFAULT_FPS;
	FLAG     bPostComplete = NO;
	FLAG     bEmulationRun = NO;
	uint32_t currentFrame = ZERO;
	uint64_t current1hzFrame = ZERO;

private:

	CheatEngine_t* ceMAS;

private:

	KeyBindings keyBindings;

	// ---- Constructor / Destructor ---------------------------
public:

	Emulation_t(abstractEmulation_t* toEmulate, MasqConfig_t& config, CheatEngine_t* ce = nullptr)
	{
		bPostComplete = false;

		if (toEmulate == nullptr)
		{
			INFO("unsupported rom");
#ifndef __RPI_PICO__
			throw std::runtime_error("unsupported rom");
#else	// !__RPI_PICO__
			panic("unsupported rom");
#endif
		}
#ifndef __RPI_PICO__
		else
		{
			current_instance = toEmulate;

			std::stringstream stream;
			stream << std::fixed << std::setprecision(4) << current_instance->getVersion();
			std::string version = stream.str();

			appName = "Masquerade Emulator | v" + version
				+ std::string(" | ") + std::string(current_instance->getEmulatorName())
				+ std::string(" | ");
		}

		this->ceMAS = ce;
#else
		current_instance = toEmulate;

		MASQ_UNUSED(config);
		MASQ_UNUSED(ce);
#endif	// !__RPI_PICO__
	}

	~Emulation_t() 
	{
		;
	}

	// ---- Emulation helpers ----------------------------------
private:

	// loadConfig reads settings from the config store.
	// On Pico every call-site returns its baked-in default,
	// so we skip the whole function body.
	void loadConfig()
	{
#ifndef __RPI_PICO__
		if (bPostComplete == false || (isCLI() == YES || ImGui::IsKeyPressed(ImGuiKey_Home)))
		{
			debugConfig._DEBUG_FPS = to_bool(config.get<std::string>("debug._DEBUG_FPS", debugConfig._DEBUG_FPS ? "true" : "false"));
			debugConfig._DEBUG_MEMORY = to_bool(config.get<std::string>("debug._DEBUG_MEMORY", debugConfig._DEBUG_MEMORY ? "true" : "false"));
			debugConfig._DEBUG_REGISTERS = to_bool(config.get<std::string>("debug._DEBUG_REGISTERS", debugConfig._DEBUG_REGISTERS ? "true" : "false"));

			if (bPostComplete == false)
			{
				debugConfig._DEBUG_PROFILER = to_bool(config.get<std::string>("debug._DEBUG_PROFILER", debugConfig._DEBUG_PROFILER ? "true" : "false"));
				_ENABLE_AUDIO = to_bool(config.get<std::string>("mods._ENABLE_AUDIO", _ENABLE_AUDIO ? "true" : "false"));
				_MUTE_AUDIO = to_bool(config.get<std::string>("mods._MUTE_AUDIO", _MUTE_AUDIO ? "true" : "false"));

				if (_MUTE_AUDIO == YES)
				{
					INFO("AUDIO is by default MUTED");
					INFO("Press M to toggle between MUTE/UNMUTE");
				}

				_ENABLE_FRAME_LIMIT = to_bool(config.get<std::string>("mods._ENABLE_FRAME_LIMIT", _ENABLE_FRAME_LIMIT ? "true" : "false"));
				_ENABLE_QUICK_SAVE = to_bool(config.get<std::string>("mods._ENABLE_QUICK_SAVE", _ENABLE_QUICK_SAVE ? "true" : "false"));
				_ENABLE_BESS_FORMAT = to_bool(config.get<std::string>("mods._ENABLE_BESS_FORMAT", _ENABLE_BESS_FORMAT ? "true" : "false"));
				_ENABLE_NETWORK = to_bool(config.get<std::string>("mods._ENABLE_NETWORK", _ENABLE_NETWORK ? "true" : "false"));

				if (_ENABLE_NETWORK == YES)
					_NETWORK_TIMEOUT_LIMIT = config.get<std::uint32_t>("mods._NETWORK_TIMEOUT_LIMIT", _NETWORK_TIMEOUT_LIMIT);
				else
					_NETWORK_TIMEOUT_LIMIT = ONE;

				_ENABLE_REWIND = to_bool(config.get<std::string>("mods._ENABLE_REWIND", _ENABLE_REWIND ? "true" : "false"));
				if (_ENABLE_REWIND == YES)
					_REWIND_BUFFER_SIZE = config.get<std::uint32_t>("mods._REWIND_BUFFER_SIZE", _REWIND_BUFFER_SIZE);

				_ENABLE_ACCURATE_INPUT_SAMPLING = to_bool(config.get<std::string>("mods._ENABLE_ACCURATE_INPUT_SAMPLING", _ENABLE_ACCURATE_INPUT_SAMPLING ? "true" : "false"));
			}
		}
#endif // !__RPI_PICO__
	}

	void displayPGEBasedFPS(float fElapsedTime, uint8_t level)
	{
		FLAG     initializationState[FPS_SLOTS] = { CLEAR };
		uint32_t frames[FPS_SLOTS] = { RESET };
		float    accumulator[FPS_SLOTS] = { RESET };

		if (debugConfig._DEBUG_FPS == true)
		{
			if (initializationState[level] == false)
			{
				accumulator[level] = fElapsedTime;
				initializationState[level] = true;
			}
			++frames[level];
			accumulator[level] += fElapsedTime;
			if (accumulator[level] > 1.0f)
			{
				INFO("PGE FPS level %u : %u", level, frames[level]);
				frames[level] = 0;
				accumulator[level] = 0.0f;
			}
		}
	}

	// ---- Core run loop --------------------------------------
private:

	FLAG run()
	{
		FLAG status = SUCCESS;
		status = onEveryMasqueradeFrame();
		RETURN status;
	}

	FLAG onEveryMasqueradeFrame()
	{
		FLAG status = SUCCESS;
		status &= runEmulationSequence();

		// ---- Rewind --------------------------------------------
		if (_ENABLE_REWIND == YES)
		{
			current_instance->fillGamePlayStack();

#ifndef __RPI_PICO__
			if (ImGui::IsKeyDown(ImGuiKey_R))
				current_instance->rewindGamePlay();
#else
			// TODO: wire Pico physical button → rewindGamePlay()
#endif
		}

		// ---- Quick save / load (F1-F12) ------------------------
		if (_ENABLE_QUICK_SAVE == YES)
		{
#ifndef __RPI_PICO__
			if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyDown(ImGuiKey_Tab) == false)
			{
				for (uint16_t ii = ((uint16_t)ImGuiKey_F1); ii <= ((uint16_t)ImGuiKey_F12); ii++)
					if (ImGui::IsKeyPressed((ImGuiKey)ii))
						current_instance->saveState((ii - ((uint16_t)ImGuiKey_F1)));
			}

			if (ImGui::IsKeyDown(ImGuiKey_LeftShift) == false && ImGui::IsKeyDown(ImGuiKey_Tab) == false)
			{
				for (uint16_t ii = ((uint16_t)ImGuiKey_F1); ii <= ((uint16_t)ImGuiKey_F12); ii++)
					if (ImGui::IsKeyPressed((ImGuiKey)ii))
						current_instance->loadState((ii - ((uint16_t)ImGuiKey_F1)));
			}
#else
			// TODO: wire Pico physical buttons → saveState / loadState
#endif
		}

		RETURN status;
	}

	FLAG runEmulationSequence()
	{
		FLAG status = SUCCESS;
		status &= atFixedFPS();
		RETURN status;
	}

	FLAG atFixedFPS()
	{
		FLAG status = SUCCESS;
		switch (current_instance->getEmulationID())
		{
		case EMULATION_ID::DEFAULT_ID:
		case EMULATION_ID::GAME_OF_LIFE_ID:
		case EMULATION_ID::CHIP8_ID:
		case EMULATION_ID::SPACE_INVADERS_ID:
		case EMULATION_ID::PACMAN_ID:
		case EMULATION_ID::NES_ID:
		case EMULATION_ID::GB_GBC_ID:
		case EMULATION_ID::GBA_ID:
		case EMULATION_ID::DS_ID:
		{
			atFPSHz();
			at1Hz();
			BREAK;
		}
		default:
		{
			status = false;
			BREAK;
		}
		}
		RETURN status;
	}

	FLAG atFPSHz()
	{
		FLAG status = SUCCESS;
		++currentFrame;
		status &= runEmulationCore();
		RETURN status;
	}

	FLAG at1Hz()
	{
		FLAG status = SUCCESS;
		++current1hzFrame;

		if (current1hzFrame >= current_instance->getEmulationFPS())
		{
			current1hzFrame = RESET;

			if (_ENABLE_NETWORK == YES)
			{
#if DISABLED
				// Network heartbeat (not yet implemented)
#endif
			}
		}
		RETURN status;
	}

private:

	FLAG runEmulationCore()
	{
		FLAG status = SUCCESS;
		FLAG stopLoopingForThisFrame = NO;

		while (stopLoopingForThisFrame == NO)
			stopLoopingForThisFrame = current_instance->runEmulationLoopAtFixedRate(currentFrame);

		status = current_instance->runEmulationAtFixedRate(currentFrame);
		RETURN status;
	}

	// ---- Lifecycle ------------------------------------------
private:

	FLAG OnUserCreate()
	{
		FLAG status = SUCCESS;

#ifndef __RPI_PICO__
		loadConfig();

		if (debugConfig._DEBUG_PROFILER == true)
			timerStart(displayNonPGEBasedFPS, 1000);
#endif

#ifndef __RPI_PICO__
		IInputBackend* iBackend = new ImGuiInputBackend();
#else // !__RPI_PICO__
		IInputBackend* iBackend = new PicoInputBackend();
#endif

		current_instance->setupTheCoreOfEmulation(nullptr, nullptr, iBackend, nullptr);
		keyBindings.setDefault(current_instance->getEmulationID());

		currentFrame = ZERO;
		myFPS = current_instance->getEmulationFPS() * _XFPS;

		INFO("Launching the %s Emulator", current_instance->getEmulatorName());

#ifndef __RPI_PICO__
		if (debugConfig._DEBUG_MEMORY == true)
			current_instance->dumpRom();
#endif

		bEmulationRun = YES;
		bPostComplete = YES;

		RETURN status;
	}

	// ---- OnUserUpdate: two complete versions for clarity ----

#ifdef __RPI_PICO__

	// Pico version: bare-metal run loop with no OS, no SDL,
	// no ImGui.  Add your Waveshark display flush and physical
	// button polling here.
	FLAG OnUserUpdate()
	{
		FLAG status = SUCCESS;

		// TODO: poll Pico physical buttons and call
		//       current_instance->onKeyEvent(key, act) here.

		if (bEmulationRun == YES)
			status = run();

		// Update FPS multiplier (Chip8 variants can change it at runtime)
		myFPS = current_instance->getEmulationFPS() * _XFPS;

		RETURN status;
	}

#else // !__RPI_PICO__ — full desktop / Emscripten version

	FLAG OnUserUpdate()
	{
		FLAG status = SUCCESS;

		if (isCLI() == NO)
		{
			if (quitOnMenuClick == YES)
			{
				quitOnMenuClick = NO;
				RETURN CLOSE;
			}

			if (dynamicDragNDropAndMenuSelect.size() != ZERO)
				RETURN CLOSE;

			if (rebootNeededOnMenuClick == YES)
			{
				if (saveContextOnReboot == YES)
					current_instance->saveState(((0xFF + (uint8_t)ImGuiKey_F1) - ((uint8_t)ImGuiKey_F1)));

#ifdef __EMSCRIPTEN__
				savePersistentFS(onSavePersistentFSComplete);
				blocking_delay_ms(ONEHUNDRED);
#endif
				RETURN CLOSE;
			}

			loadConfig();
			PAUSE_OR_RESUME(ImGuiKey_P);
			MUTE_OR_UNMUTE(ImGuiKey_M);
		}

		if (bEmulationRun == YES)
		{
#if DISABLED
			// Network receive loop (not yet implemented)
#endif
			status = run();
		}

		myFPS = current_instance->getEmulationFPS() * _XFPS;
		RETURN status;
	}

#endif // __RPI_PICO__

	// ---- OnUserDestroy: two complete versions for clarity ---

#ifdef __RPI_PICO__

	// Pico version: no file system, no SDL window, no Boost.
	FLAG OnUserDestroy()
	{
		FLAG status = SUCCESS;

		current_instance->destroyEmulator();
		// TODO: Pico-specific teardown (Waveshark driver shutdown, etc.)

		RETURN status;
	}

#else // !__RPI_PICO__ — full desktop / Emscripten version

	FLAG OnUserDestroy(SDL_Window* window = nullptr)
	{
		FLAG status = SUCCESS;

		writeDequeToFile(recentlyOpenedList, recentlyOpenedListPath);

		if (window != nullptr && current_instance->getEmulationID() == EMULATION_ID::DEFAULT_ID)
		{
			int x = RESET, y = RESET;
			SDL_GetWindowSize(window, &x, &y);
			config.put("mods._X", std::uint16_t(x));
			config.put("mods._Y", std::uint16_t(y));
			boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
		}

#ifdef __EMSCRIPTEN__
		savePersistentFS(onSavePersistentFSComplete);
#endif

		current_instance->destroyEmulator();

#if defined(_WIN32)
		Sleep(ONE_SECOND);
#elif defined(__EMSCRIPTEN__)
		blocking_delay_ms(ONE_SECOND);
#else
		usleep(ONE_SECOND * 1000);
#endif

		RETURN status;
	}

#endif // __RPI_PICO__

	// ---- Emscripten file upload handler ---------------------
#ifdef __EMSCRIPTEN__
private:
	static void handle_upload_file(std::string const& filename, std::string const& mime_type, std::string_view buffer, void*)
	{
		if (buffer.empty())
		{
			INFO("Empty Buffer"); RETURN;
		}

		// EMSCRIPTEN FIX: Convert to lowercase and replace spaces with underscores.
		// Emscripten's IDBFS has issues with uppercase letters and spaces in filenames.
		std::string sanitized_filename = filename;
		std::transform(sanitized_filename.begin(), sanitized_filename.end(), sanitized_filename.begin(),
			[](unsigned char c) { return std::tolower(c); });
		std::replace(sanitized_filename.begin(), sanitized_filename.end(), ' ', '_');

		if (sanitized_filename != filename)
		{
			INFO("Filename sanitized (lowercase + spaces -> underscores):");
			INFO("  Original:  %s", filename.c_str());
			INFO("  Sanitized: %s", sanitized_filename.c_str());
		}

		std::vector<std::string> extracted_files;
		INC32 count = ONE;

		std::string persistent_path = "/persistent/" + sanitized_filename;
		INFO("File uploaded: %s (type: %s), size: %zu bytes", persistent_path.c_str(), mime_type.c_str(), buffer.size());
		INFO("Filename length: %zu", sanitized_filename.length());
		INFO("Has spaces: %s", (sanitized_filename.find(' ') != std::string::npos) ? "YES" : "NO");
		INFO("Has uppercase: %s", (std::any_of(sanitized_filename.begin(), sanitized_filename.end(),
			[](unsigned char c) { return std::isupper(c); })) ? "YES" : "NO");

		INFO("=== Contents of /persistent BEFORE write ===");
		try
		{
			for (const auto& entry : std::filesystem::directory_iterator("/persistent"))
				INFO("  - %s", entry.path().filename().c_str());
		}
		catch (const std::exception& e)
		{
			INFO("Error listing directory: %s", e.what());
		}

		std::ofstream ofs(persistent_path, std::ios::binary);
		if (!ofs)
		{
			INFO("CRITICAL: Failed to open file for writing: %s", persistent_path.c_str()); INFO("Errno: %d", errno); RETURN;
		}

		INFO("File opened successfully for writing");
		ofs.write(buffer.data(), buffer.size());
		if (!ofs)
		{
			INFO("CRITICAL: Failed to write data to file: %s", persistent_path.c_str()); INFO("Errno: %d", errno);
		}
		else
		{
			INFO("Data written successfully: %zu bytes", buffer.size());
		}
		ofs.close();
		INFO("File closed");

		INFO("=== Contents of /persistent AFTER write ===");
		try
		{
			for (const auto& entry : std::filesystem::directory_iterator("/persistent"))
				INFO("  - %s (size: %zu bytes)", entry.path().filename().c_str(), std::filesystem::file_size(entry.path()));
		}
		catch (const std::exception& e)
		{
			INFO("Error listing directory: %s", e.what());
		}

		std::ifstream test(persistent_path, std::ios::binary);
		if (test.good())
		{
			test.seekg(0, std::ios::end);
			size_t file_size = test.tellg();
			INFO("File verified on disk: %s (size: %zu bytes)", persistent_path.c_str(), file_size);
		}
		else
		{
			INFO("CRITICAL: File NOT readable after save: %s", persistent_path.c_str()); INFO("Errno: %d", errno);
		}
		test.close();

		INFO("=== Attempting alternative file access ===");
		struct stat file_stat;
		if (stat(persistent_path.c_str(), &file_stat) == 0)
			INFO("stat() succeeded: file size = %ld bytes", file_stat.st_size);
		else
			INFO("stat() failed: %d", errno);

		std::string ext = get_extension(persistent_path.c_str());
		if (strcmp(ext.c_str(), "zip") == 0)
		{
			INFO("It's a ZIP file!");
			count = extract_all_to_persistent_dir(persistent_path.c_str(), extracted_files);
		}
		else
		{
			extracted_files.emplace_back(sanitized_filename);
		}

		INFO("Calling savePersistentFS...");
		savePersistentFS(onSavePersistentFSComplete);

		for (const auto& path : extracted_files)
		{
			auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), path);
			if (it != recentlyOpenedList.end()) recentlyOpenedList.erase(it);
			recentlyOpenedList.push_front(path);
			if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE) recentlyOpenedList.pop_back();
			dynamicDragNDropAndMenuSelect.push_back(path);
		}

		INFO("=== handle_upload_file complete ===");
	}
#endif // __EMSCRIPTEN__

	// ---- ROM / BIOS / audio file selection (desktop only) ---
#ifndef __RPI_PICO__
private:

	void romSelect(ROM type)
	{
#ifdef __EMSCRIPTEN__
		switch (type)
		{
		case ROM::CHIP8:              INFO("Chip8");                    emscripten_browser_file::upload(chip8filters, handle_upload_file); BREAK;
		case ROM::SPACE_INVADERS:     INFO("Space Invaders");           emscripten_browser_file::upload(spaceInvadersFilter, handle_upload_file); BREAK;
		case ROM::PAC_MAN:
		case ROM::MS_PAC_MAN:         INFO("PacMan / Ms PacMan");       emscripten_browser_file::upload(pacmanFilter, handle_upload_file); BREAK;
		case ROM::NES:                INFO("Nintendo Entertainment System"); emscripten_browser_file::upload(nesFilter, handle_upload_file); BREAK;
		case ROM::GAME_BOY:
		case ROM::GAME_BOY_COLOR:     INFO("Game Boy / Game Boy Color"); emscripten_browser_file::upload(gbGbcFilter, handle_upload_file); BREAK;
		case ROM::GAME_BOY_ADVANCE:   INFO("Game Boy Advance");         emscripten_browser_file::upload(gbaFilter, handle_upload_file); BREAK;
		default: FATAL("Unsupported ROM type : %u", TO_UINT(type)); RETURN;
		}
#else // native desktop (NFD)
		nfdu8char_t* outPath;
		const nfdpathset_t* outPaths;
		nfdu8filteritem_t     filters[1];
		nfdopendialogu8args_t args = { 0 };

		switch (type)
		{
		case ROM::GAME_OF_LIFE:   filters->name = golfilters->name;           filters->spec = golfilters->spec;           BREAK;
		case ROM::CHIP8:          filters->name = chip8filters->name;         filters->spec = chip8filters->spec;         BREAK;
		case ROM::SPACE_INVADERS: filters->name = spaceInvadersFilter->name;  filters->spec = spaceInvadersFilter->spec;  BREAK;
		case ROM::PAC_MAN:
		case ROM::MS_PAC_MAN:     filters->name = pacmanFilter->name;         filters->spec = pacmanFilter->spec;         BREAK;
		case ROM::NES:            filters->name = nesFilter->name;            filters->spec = nesFilter->spec;            BREAK;
		case ROM::GAME_BOY:
		case ROM::GAME_BOY_COLOR: filters->name = gbGbcFilter->name;         filters->spec = gbGbcFilter->spec;          BREAK;
		case ROM::GAME_BOY_ADVANCE: filters->name = gbaFilter->name;         filters->spec = gbaFilter->spec;            BREAK;
		default: FATAL("Unsupported ROM type : %u", TO_UINT(type)); RETURN;
		}

		args.filterList = filters;
		args.filterCount = ONE;
		nfdresult_t result;

		if (type == ROM::SPACE_INVADERS || type == ROM::PAC_MAN || type == ROM::MS_PAC_MAN)
		{
			result = NFD_OpenDialogMultiple(&outPaths, filters, ONE, NULL);
			if (result == NFD_OKAY)
			{
				nfdpathsetsize_t numPaths;
				NFD_PathSet_GetCount(outPaths, &numPaths);
				for (nfdpathsetsize_t i = 0; i < numPaths; ++i)
				{
					nfdchar_t* path;
					NFD_PathSet_GetPath(outPaths, i, &path);
					INFO("Opening %i: %s", (int)i, path);
					dynamicDragNDropAndMenuSelect.push_back(std::string(path));
					NFD_PathSet_FreePath(path);
				}
				NFD_PathSet_Free(outPaths);
			}
			else if (result == NFD_CANCEL)
			{
				;
			}
			else
			{
				FATAL("Error: %s", NFD_GetError());
			}
		}
		else
		{
			result = NFD_OpenDialogU8_With(&outPath, &args);
			if (result == NFD_OKAY)
			{
				INFO("Opening : %s", outPath);
				auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), std::string(outPath));
				if (it != recentlyOpenedList.end()) recentlyOpenedList.erase(it);
				recentlyOpenedList.push_front(std::string(outPath));
				if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE) recentlyOpenedList.pop_back();
				dynamicDragNDropAndMenuSelect.push_back(std::string(outPath));
				NFD_FreePathU8(outPath);
			}
			else if (result == NFD_CANCEL)
			{
				;
			}
			else
			{
				FATAL("Error: %s", NFD_GetError());
			}
		}
#endif // __EMSCRIPTEN__
	}

	void bootRomSelect(ROM type)
	{
#ifndef __EMSCRIPTEN__
		nfdu8char_t* outPath = nullptr;
		const nfdpathset_t* outPaths = nullptr;
		nfdu8filteritem_t     filters[1];
		nfdopendialogu8args_t args = { 0 };

		filters->name = bootromfilters->name;
		filters->spec = bootromfilters->spec;
		args.filterList = filters;
		args.filterCount = ONE;

		nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
		if (result == NFD_OKAY)
		{
			INFO("Load : %s", outPath);

			if (type == ROM::GAME_BOY)         config.put("gb_gbc._dmg_bios_location", std::string(outPath));
			else if (type == ROM::GAME_BOY_COLOR)   config.put("gb_gbc._cgb_bios_location", std::string(outPath));
			else if (type == ROM::GAME_BOY_ADVANCE) config.put("gba._gba_bios_location", std::string(outPath));

			boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL)
		{
			;
		}
		else
		{
			FATAL("Error: %s", NFD_GetError());
		}
#endif // !__EMSCRIPTEN__
	}

	void loadSIAudioWAV(std::string type)
	{
#ifndef __EMSCRIPTEN__
		nfdu8char_t* outPath = nullptr;
		const nfdpathset_t* outPaths = nullptr;
		nfdu8filteritem_t     filters[1];
		nfdopendialogu8args_t args = { 0 };

		filters->name = spaceInvadersAudioFilter->name;
		filters->spec = spaceInvadersAudioFilter->spec;
		args.filterList = filters;
		args.filterCount = ONE;

		nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
		if (result == NFD_OKAY)
		{
			INFO("Load : %s", outPath);
			std::string option = "spaceinvaders._" + type;
			config.put(option, std::string(outPath));
			boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL)
		{
			;
		}
		else
		{
			FATAL("Error: %s", NFD_GetError());
		}
#endif // !__EMSCRIPTEN__
	}

#endif // !__RPI_PICO__ (romSelect / bootRomSelect / loadSIAudioWAV)

	// ---- Windows DWM title-bar colouring (desktop only) -----
#if defined(_WIN32) && !defined(__RPI_PICO__) && (ENABLED_IMGUI_DEFAULT_THEME == NO)
private:

#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR  34
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

	void SetDWMTitlebarColor(SDL_Window* window, ImVec4 color)
	{
		auto ToCOLORREF = [](const ImVec4& c) -> COLORREF {
			RETURN RGB(static_cast<int>(c.x * 255.0f + 0.5f),
				static_cast<int>(c.y * 255.0f + 0.5f),
				static_cast<int>(c.z * 255.0f + 0.5f));
			};

		const char* windowTitle = SDL_GetWindowTitle(window);
		HWND hwnd = FindWindowA(NULL, windowTitle);
		if (!hwnd)
		{
			SDL_Log("Failed to find HWND for window title: %s", windowTitle); RETURN;
		}

		COLORREF WinColor = ToCOLORREF(color);
		DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &WinColor, sizeof(WinColor));
		DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &WinColor, sizeof(WinColor));
	}
#endif // _WIN32 && !__RPI_PICO__

	// ---- ImGui theme setup (desktop only) -------------------
#ifndef __RPI_PICO__
public:

	int setupThemes()
	{
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 0.5f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.40f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.22f, 0.22f, 0.22f, 0.92f);
		colors[ImGuiCol_Border] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.8f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.8f);
		colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
		colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.9f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

		// ---- Custom palette override --------------------------------
		if (currentEmuTheme == THEME_CUSTOM)
		{
			uint8_t* palette = customSEpalettes;

			auto applyPaletteEntry = [&](int idx, auto&&... targets) {
				if (!palette[idx * 4 + 3]) return;
				float r = palette[idx * 4 + 0] / 255.0f, g = palette[idx * 4 + 1] / 255.0f,
					b = palette[idx * 4 + 2] / 255.0f, a = palette[idx * 4 + 3] / 255.0f;
				(void)std::initializer_list<int>{ ((targets = ImVec4(r, g, b, a)), 0)... };
				};

			// Base color
			if (palette[0 * 4 + 3])
			{
				float r = palette[0 * 4 + 0] / 255.0f, g = palette[0 * 4 + 1] / 255.0f, b = palette[0 * 4 + 2] / 255.0f, a = palette[0 * 4 + 3] / 255.0f;
				colors[ImGuiCol_WindowBg] = colors[ImGuiCol_ChildBg] = colors[ImGuiCol_PopupBg] = colors[ImGuiCol_MenuBarBg] = ImVec4(r, g, b, a);
			}
			// Text color
			if (palette[1 * 4 + 3])
			{
				float r = palette[1 * 4 + 0] / 255.0f, g = palette[1 * 4 + 1] / 255.0f, b = palette[1 * 4 + 2] / 255.0f, a = palette[1 * 4 + 3] / 255.0f;
				colors[ImGuiCol_PlotLinesHovered] = colors[ImGuiCol_PlotHistogramHovered] = colors[ImGuiCol_Text] = ImVec4(r, g, b, a);
				colors[ImGuiCol_TextDisabled] = ImVec4(r, g, b, a * 0.4f);
				colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(r, g, b, a * 0.6f);
				colors[ImGuiCol_SliderGrabActive] = colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(r, g, b, a * 0.8f);
			}
			// Second color
			if (palette[2 * 4 + 3])
			{
				float r = palette[2 * 4 + 0] / 255.0f, g = palette[2 * 4 + 1] / 255.0f, b = palette[2 * 4 + 2] / 255.0f, a = palette[2 * 4 + 3] / 255.0f;
				colors[ImGuiCol_FrameBg] = ImVec4(r, g, b, a * 0.5f);
				colors[ImGuiCol_ScrollbarBg] = ImVec4(r, g, b, a);
				colors[ImGuiCol_Button] = ImVec4(r, g, b, a);
				colors[ImGuiCol_ButtonHovered] = ImVec4(r, g, b, a * 0.54f);
				colors[ImGuiCol_ButtonActive] = ImVec4(r * 2, g * 2, b * 2, a);
			}
			// Tab/Header color
			if (palette[3 * 4 + 3])
			{
				float r = palette[3 * 4 + 0] / 255.0f, g = palette[3 * 4 + 1] / 255.0f, b = palette[3 * 4 + 2] / 255.0f, a = palette[3 * 4 + 3] / 255.0f;
				colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive] = colors[ImGuiCol_TitleBgCollapsed] =
					colors[ImGuiCol_TableHeaderBg] = colors[ImGuiCol_TableBorderStrong] = ImVec4(r, g, b, a);
				colors[ImGuiCol_SliderGrab] = colors[ImGuiCol_ScrollbarGrab] = ImVec4(r, g, b, a);
				colors[ImGuiCol_FrameBgHovered] = ImVec4(r, g, b, a * 0.75f);
				colors[ImGuiCol_FrameBgActive] = ImVec4(r, g, b, a);
				colors[ImGuiCol_Tab] = colors[ImGuiCol_Header] = ImVec4(r, g, b, a * 0.5f);
				colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered] = ImVec4(r, g, b, a * 0.75f);
				colors[ImGuiCol_TabActive] = colors[ImGuiCol_HeaderActive] = ImVec4(r, g, b, a);
			}
			// Accent color
			if (palette[4 * 4 + 3])
			{
				float r = palette[4 * 4 + 0] / 255.0f, g = palette[4 * 4 + 1] / 255.0f, b = palette[4 * 4 + 2] / 255.0f, a = palette[4 * 4 + 3] / 255.0f;
				colors[ImGuiCol_PlotLines] = colors[ImGuiCol_PlotHistogram] = colors[ImGuiCol_CheckMark] = ImVec4(r, g, b, a);
			}
		}

		// ---- Light theme: invert luminance -------------------------
		if (currentEmuTheme == SE_THEME_LIGHT)
		{
			int invert_list[] = {
				ImGuiCol_Text, ImGuiCol_TextDisabled, ImGuiCol_WindowBg, ImGuiCol_ChildBg,
				ImGuiCol_PopupBg, ImGuiCol_Border, ImGuiCol_BorderShadow, ImGuiCol_FrameBg,
				ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive, ImGuiCol_TitleBg,
				ImGuiCol_TitleBgActive, ImGuiCol_TitleBgCollapsed, ImGuiCol_MenuBarBg,
				ImGuiCol_ScrollbarBg, ImGuiCol_ScrollbarGrab, ImGuiCol_ScrollbarGrabHovered,
				ImGuiCol_ScrollbarGrabActive, ImGuiCol_SliderGrab, ImGuiCol_SliderGrabActive,
				ImGuiCol_Button, ImGuiCol_ButtonHovered, ImGuiCol_ButtonActive,
				ImGuiCol_Header, ImGuiCol_HeaderHovered, ImGuiCol_HeaderActive,
				ImGuiCol_Separator, ImGuiCol_SeparatorHovered, ImGuiCol_SeparatorActive,
				ImGuiCol_ResizeGrip, ImGuiCol_ResizeGripHovered, ImGuiCol_ResizeGripActive,
				ImGuiCol_Tab, ImGuiCol_TabHovered, ImGuiCol_TabActive,
				ImGuiCol_TabUnfocused, ImGuiCol_TabUnfocusedActive,
				ImGuiCol_TableHeaderBg, ImGuiCol_TableBorderStrong, ImGuiCol_TableBorderLight,
				ImGuiCol_TableRowBg, ImGuiCol_TableRowBgAlt, ImGuiCol_TextSelectedBg,
				ImGuiCol_DragDropTarget, ImGuiCol_NavHighlight,
				ImGuiCol_NavWindowingHighlight, ImGuiCol_NavWindowingDimBg, ImGuiCol_ModalWindowDimBg,
			};
			for (int i = 0; i < (int)(sizeof(invert_list) / sizeof(invert_list[0])); ++i)
			{
				colors[invert_list[i]].x = 1.0f - colors[invert_list[i]].x;
				colors[invert_list[i]].y = 1.0f - colors[invert_list[i]].y;
				colors[invert_list[i]].z = 1.0f - colors[invert_list[i]].z;
			}
		}

		// ---- Style metrics -----------------------------------------
		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowPadding = ImVec2(8.00f, 8.00f);
		style->FramePadding = ImVec2(5.00f, 2.00f);
		style->ItemSpacing = ImVec2(6.00f, 6.00f);
		style->TouchExtraPadding = ImVec2(2.00f, 4.00f);
		style->IndentSpacing = 25;
		style->ScrollbarSize = 15;
		style->GrabMinSize = 10;
		style->WindowBorderSize = 0;
		style->ChildBorderSize = 0;
		style->PopupBorderSize = 0;
		style->FrameBorderSize = 0;
		style->TabBorderSize = 0;
		style->WindowRounding = 0;
		style->ChildRounding = 4;
		style->FrameRounding = 0;
		style->PopupRounding = 0;
		style->ScrollbarRounding = 9;
		style->GrabRounding = 100;
		style->LogSliderDeadzone = 4;
		style->TabRounding = 4;
		style->ButtonTextAlign = ImVec2(0.5f, 0.5f);

		// ---- Black theme: zero out backgrounds ---------------------
		if (currentEmuTheme == SE_THEME_BLACK)
		{
			int black_list[] = {
				ImGuiCol_WindowBg, ImGuiCol_ChildBg, ImGuiCol_PopupBg,
				ImGuiCol_TitleBg,  ImGuiCol_MenuBarBg,
			};
			colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.9f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.6f);
			for (int i = 0; i < (int)(sizeof(black_list) / sizeof(black_list[0])); ++i)
			{
				colors[black_list[i]].x = 0; colors[black_list[i]].y = 0; colors[black_list[i]].z = 0;
			}
		}

		RETURN SUCCESS;
	}

#endif // !__RPI_PICO__ (setupThemes)

	// ---- Start: two complete versions for clarity -----------
public:

#ifdef __RPI_PICO__

	// Pico version: bare-metal infinite loop.
	// Initialise your Waveshark display driver BEFORE calling
	// Start(). Drive the display flush from inside the emulator
	// (abstractEmulation_t::runEmulationAtFixedRate) or add it
	// here after OnUserUpdate() returns.
	int Start()
	{
		OnUserCreate();
		for (;;)
		{
			OnUserUpdate();
		}
		// Never reached on Pico; OnUserDestroy() must be called
		// explicitly if you ever add a shutdown path.
		RETURN ZERO;
	}

#else // !__RPI_PICO__ — full SDL3 / ImGui / OpenGL desktop version

	int Start()
	{
		if (isCLI() == YES)
		{
			OnUserCreate();
			OnUserUpdate();
			OnUserDestroy();
		}
		else
		{
			// ---- SDL init -------------------------------------------
			if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
			{
				FATAL("Error: SDL_Init(): %s", SDL_GetError());
				RETURN - ONE;
			}

			// ---- GLSL / context version selection -------------------
#if defined(IMGUI_IMPL_OPENGL_ES2)
			const char* glsl_version = "#version 100";
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
			const char* glsl_version = "#version 300 es";
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
			const char* glsl_version = "#version 150";
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
			const char* glsl_version = "#version 130";
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
			const char* glsl_version = "#version 330";
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
#endif

			// ---- Window creation ------------------------------------
			auto newX = config.get<std::int16_t>("mods._X", 345) - WINDOW_PADDING;
			auto newY = config.get<std::int16_t>("mods._Y", 200) - WINDOW_PADDING - WINDOW_PADDING;
			if (newX > 0 && newY > 0)
			{
				current_instance->setScreenWidth(newX);
				current_instance->setScreenHeight(newY);
			}

			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
			Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
			SDL_Window* window = SDL_CreateWindow(
				"Masquerade Emulator",
				(current_instance->getScreenWidth() * current_instance->getPixelWidth()) + WINDOW_PADDING,
				(current_instance->getScreenHeight() * current_instance->getPixelHeight()) + WINDOW_PADDING + WINDOW_PADDING,
				window_flags);
			if (window == nullptr)
			{
				FATAL("Error: SDL_CreateWindow(): %s", SDL_GetError()); RETURN -ONE;
			}

#ifndef __EMSCRIPTEN__
			if (_XSCALE > ONE)
			{
				int w, h;
				SDL_GetWindowSize(window, &w, &h);
				SDL_SetWindowSize(window, w * _XSCALE, h * _XSCALE);
			}
			SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#endif

			SDL_GLContext gl_context = SDL_GL_CreateContext(window);
			if (gl_context == nullptr)
			{
				FATAL("Error: SDL_GL_CreateContext(): %s", SDL_GetError()); RETURN -ONE;
			}

			SDL_StartTextInput(window);
			SDL_GL_MakeCurrent(window, gl_context);
			SDL_GL_SetSwapInterval(1); // Enable vsync

#ifndef __EMSCRIPTEN__
			SDL_ShowWindow(window);
#endif

			// ---- Dear ImGui setup -----------------------------------
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			io.ConfigWindowsMoveFromTitleBarOnly = true;

#if (ENABLED_IMGUI_DEFAULT_THEME == YES)
			ImGui::StyleColorsDark();
#else
			setupThemes();
#endif

			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
			ImGui_ImplOpenGL3_Init(glsl_version);

			io.Fonts->AddFontFromFileTTF((std::filesystem::path(_FONT_LOCATION) / "segoeui.ttf").string().c_str(), 16.0f);

#ifndef __EMSCRIPTEN__
			if (NFD_Init() != NFD_OKAY)
			{
				FATAL("Error: NFD_Init(): %s", NFD_GetError()); RETURN -ONE;
			}
			TODO("NFD_GetNativeWindowFromSDLWindow needs to be called when NFD's support for SDL3 is available");
#endif

			// ---- Load splash / click-to-start image -----------------
			ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

#if defined(_WIN32) && (ENABLED_IMGUI_DEFAULT_THEME == NO)
			SetDWMTitlebarColor(window, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
#endif

#if !defined(__EMSCRIPTEN__) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
			gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
#endif

			ImGui::SetNextWindowSize(ImVec2((float)current_instance->getScreenWidth(), (float)current_instance->getScreenHeight()));
			std::string emuWindow = "Emulation Window (" + std::string(current_instance->getEmulatorName()) + ")";

			FlushEarlyLogsToImGui();

			if (RUN_IMGUI_DEMO == NO)
				OnUserCreate();

#ifndef __EMSCRIPTEN__
			auto uiSpritesDir = config.get<std::string>("internal._ui_sprites_directory", "");
			if (uiSpritesDir.empty())
			{
				FATAL("Could not locate the UI sprites directory"); RETURN - ONE;
			}
			std::string imLoc = uiSpritesDir + "\\BG1.png";
#else
			std::string imLoc = "assets/ui/sprites/BG1.png";
#endif
			int    clickWinWidth = 0, clickWinHeight = 0;
			uint32_t clickWinTexture = 0;
			FLAG clickWinStatus = LoadTextureFromFile(imLoc.c_str(), &clickWinTexture, &clickWinWidth, &clickWinHeight);
			IM_ASSERT(clickWinStatus);

			// ---- Main loop ------------------------------------------
			ID64 tickAtStart = RESET;
			FLAG done = NO;
			io.IniFilename = _IMGUI_LOCATION.c_str();

			// Shared SDL event handler
			auto handleSDLEvent = [&](SDL_Event& e)
				{
					ImGui_ImplSDL3_ProcessEvent(&e);
					if (e.type == SDL_EVENT_QUIT)
						done = true;
					if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && e.window.windowID == SDL_GetWindowID(window))
						done = true;
					if (e.type == SDL_EVENT_DROP_FILE)
					{
						auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), std::string(e.drop.data));
						if (it != recentlyOpenedList.end()) recentlyOpenedList.erase(it);
						recentlyOpenedList.push_front(e.drop.data);
						if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE) recentlyOpenedList.pop_back();
						dynamicDragNDropAndMenuSelect.push_back(e.drop.data);
					}
					if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP)
					{
						EmuKey       key = keyBindings.resolve(current_instance->getEmulationID(), (int)e.key.scancode);
						EmuKeyAction act = (e.type == SDL_EVENT_KEY_DOWN) ? EmuKeyAction::PRESSED : EmuKeyAction::RELEASED;
						if (key != EmuKey::UNKNOWN)
							current_instance->onKeyEvent(key, act);
					}
				};

			// Input-hint callback (called mid-frame from emulator)
			current_instance->setInputHintCallback([&]()
				{
					SDL_Event e;
					if (SDL_PollEvent(&e)) handleSDLEvent(e);
				});

#ifdef __EMSCRIPTEN__
			auto denominator = myFPS <= 60 ? myFPS : 60;
			const double timestep = 1.0 / denominator;
			EMSCRIPTEN_MAINLOOP_BEGIN
#else
			while (!done)
#endif
			{
				auto LOOP = [&]()
					{
						if (ENABLED)
						{
							static FLAG showEmuWin = YES;
							static FLAG showUpdWin = NO;
							static FLAG showAboutWin = NO;
							static FLAG showLoggerWin = NO;
							static FLAG showCheatWin = NO;
							static FLAG maintainAspectRatio = config.get<FLAG>("mods._MAINTAIN_ASPECT_RATIO", true);
							static FLAG accurateInputSampling = config.get<FLAG>("mods._ENABLE_ACCURATE_INPUT_SAMPLING", false);

							tickAtStart = SDL_GetTicksNS();

#if (ENABLED_IMGUI_DEFAULT_THEME == NO)
							if (currentEmuTheme != previousEmuTheme)
							{
								setupThemes();
								previousEmuTheme = currentEmuTheme;
#if _WIN32
								SetDWMTitlebarColor(window, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
#endif
							}
#endif
							SDL_SetWindowTitle(window, (appName + to_string_with_precision(io.Framerate, ONE)).c_str());
							_ACTUAL_FPS = io.Framerate;

							SDL_Event event;
							while (SDL_PollEvent(&event)) handleSDLEvent(event);

							if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
							{
								SDL_Delay(10); RETURN;
							}

							// ---- Dear ImGui frame ---------------------------
							ImGui_ImplOpenGL3_NewFrame();
							ImGui_ImplSDL3_NewFrame();
							ImGui::NewFrame();

							ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_AutoHideTabBar;
							if (!showUpdWin && !showAboutWin && !showLoggerWin)
								dockSpaceFlags |= ImGuiDockNodeFlags_NoTabBar;
							ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockSpaceFlags);

							if (initScreen == NO)
							{
								if (ImGui::BeginMainMenuBar())
								{
									// ---- File menu ----------------------------------
									if (ImGui::BeginMenu("File"))
									{
										if (ImGui::BeginMenu("Open Game Of Life##OpenGameOfLife", MASQ_ENABLE_GOL))
										{
											if (ImGui::MenuItem("Create New##CreateNew"))
											{
												dynamicDragNDropAndMenuSelect.push_back("dummy.gol");
#ifdef __EMSCRIPTEN__
												SavePersistentFSComplete = YES;
#endif
											}
											if (ImGui::MenuItem("Open GOL##OpenGOL")) romSelect(ROM::GAME_OF_LIFE);
											ImGui::EndMenu();
										}
										ImGui::Separator();
										if (ImGui::BeginMenu("Open Chip-8/S-Chip/XO-Chip/Modern-Chip8##OpenChip-8/S-Chip/XO-Chip/Modern-Chip8", MASQ_ENABLE_CHIP8))
										{
											auto menuOption = [&](const char* label, const char* key)
												{
													FLAG isTicked = to_bool(config.get<std::string>(key, "false")) == YES;
													if (ImGui::MenuItem(label, nullptr, isTicked))
													{
														config.put("chip8._chip8", CLEAR); config.put("chip8._schip_modern", CLEAR);
														config.put("chip8._schip_legacy", CLEAR); config.put("chip8._xo_chip", CLEAR);
														config.put("chip8._modern_chip8", CLEAR);
														config.put(key, YES);
														boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
														romSelect(ROM::CHIP8);
													}
												};
											menuOption("Open Chip-8", "chip8._chip8");
											menuOption("Open S-Chip (Modern)", "chip8._schip_modern");
											menuOption("Open S-Chip (Legacy)", "chip8._schip_legacy");
											menuOption("Open XO-Chip", "chip8._xo_chip");
											menuOption("Open Modern-Chip8", "chip8._modern_chip8");
											ImGui::EndMenu();
										}
										ImGui::Separator();
										if (ImGui::MenuItem("Open Space Invaders##OpenSI", nullptr, false, MASQ_ENABLE_SI))
											romSelect(ROM::SPACE_INVADERS);
										if (ImGui::BeginMenu("Open Pacman/Ms Pacman##OpenPacman", MASQ_ENABLE_PACMAN))
										{
											if (ImGui::MenuItem("Open Midway/Namco Pacman##OpenPacmanVar")) romSelect(ROM::PAC_MAN);
											if (ImGui::MenuItem("Open Ms Pacman##OpenMsPacman"))            romSelect(ROM::MS_PAC_MAN);
											ImGui::EndMenu();
										}
										ImGui::Separator();
										if (ImGui::MenuItem("Open NES##OpenNES", nullptr, false, MASQ_ENABLE_NES))
											romSelect(ROM::NES);
										if (ImGui::BeginMenu("Open GB/GBC##OpenGB/GBC", MASQ_ENABLE_GBC))
										{
											if (ImGui::MenuItem("Open GB##OpenGB"))   romSelect(ROM::GAME_BOY);
											if (ImGui::MenuItem("Open GBC##OpenGBC")) romSelect(ROM::GAME_BOY_COLOR);
											ImGui::EndMenu();
										}
										if (ImGui::MenuItem("Open GBA##OpenGBA", nullptr, false, MASQ_ENABLE_GBA))
											romSelect(ROM::GAME_BOY_ADVANCE);
										ImGui::Separator();
										if (ImGui::BeginMenu("Open Recent##OpenRecent"))
										{
											if (recentlyOpenedList.empty())
											{
												ImGui::MenuItem("Nothing to display##Nothingtodisplay");
											}
											else
											{
												for (const auto& element : recentlyOpenedList)
												{
													if (ImGui::MenuItem(element.c_str()))
													{
														INFO("Opening : %s", element.c_str());
														std::string copyOfElement = element;
														auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), element);
														if (it != recentlyOpenedList.end()) recentlyOpenedList.erase(it);
														recentlyOpenedList.push_front(copyOfElement);
														if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE) recentlyOpenedList.pop_back();

														std::string prefix = "/persistent/";
														if (copyOfElement.rfind(prefix, 0) == 0) copyOfElement.erase(0, prefix.length());
														dynamicDragNDropAndMenuSelect.push_back(copyOfElement);
#ifdef __EMSCRIPTEN__
														SavePersistentFSComplete = YES;
#endif
													}
												}
												ImGui::Separator();
												if (ImGui::MenuItem("Clear the recently opened item history##ClearHistory"))
													recentlyOpenedList.clear();
											}
											ImGui::EndMenu();
										}
										if (ImGui::MenuItem("Reset")) rebootNeededOnMenuClick = YES;
										if (ImGui::MenuItem("Quit"))
										{
#ifdef __EMSCRIPTEN__
											clearPersistentFS(onClearPersistentFSComplete);
											blocking_delay_ms(ONEHUNDRED);
#endif
											done = YES;
										}
										ImGui::EndMenu();
									}

									// ---- Emulation menu -----------------------------
									if (ImGui::BeginMenu("Emulation"))
									{
										if (ImGui::BeginMenu("Bios"))
										{
											if (ImGui::BeginMenu("GB Bios##GBBios", MASQ_ENABLE_GBC))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gb_gbc._use_dmg_bios", "true"));
												if (ImGui::MenuItem("Load##GB Bios", NULL, NO, inEnscriptenMode == NO)) bootRomSelect(ROM::GAME_BOY);
												if (ImGui::MenuItem("Enable##GB Bios", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("gb_gbc._use_dmg_bios", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("GBC Bios##GBCBios", MASQ_ENABLE_GBC))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gb_gbc._use_cgb_bios", "true"));
												if (ImGui::MenuItem("Load##GBC Bios", NULL, NO, inEnscriptenMode == NO)) bootRomSelect(ROM::GAME_BOY_COLOR);
												if (ImGui::MenuItem("Enable##GBC Bios", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("gb_gbc._use_cgb_bios", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("GBA Bios##GBABios", MASQ_ENABLE_GBA))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gba._use_gba_bios", "true"));
												if (ImGui::MenuItem("Load##GBA Bios", NULL, NO, inEnscriptenMode == NO)) bootRomSelect(ROM::GAME_BOY_ADVANCE);
												if (ImGui::MenuItem("Enable##GBA Bios", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("gba._use_gba_bios", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											ImGui::EndMenu();
										}
										if (ImGui::BeginMenu("Audio"))
										{
											float volume = current_instance->getEmulationVolume();
											if (current_instance->getEmulationID() == EMULATION_ID::DEFAULT_ID)
											{
												ImGui::BeginDisabled();
												ImGui::SliderFloat("Volume", &volume, 0, 1);
												if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
													ImGui::SetTooltip("This setting is available only post game selection");
												ImGui::EndDisabled();
											}
											else
											{
												ImGui::SliderFloat("Volume", &volume, 0, 1);
											}
											current_instance->setEmulationVolume((float)volume);
											ImGui::Separator();
											if (ImGui::BeginMenu("Space Invaders##SpaceInvaders", MASQ_ENABLE_SI))
											{
												if (ImGui::BeginMenu("Load Space Invaders WAV##LoadSpaceInvadersWAV", inEnscriptenMode == NO))
												{
													const char* items[][2] = {
														{"UFO",         "UFO"},           {"Shot",         "Shot"},
														{"Player Dies", "PlayerDies"},    {"Invader Dies", "InvaderDies"},
														{"Fleet Move 1","FleetMovement1"},{"Fleet Move 2", "FleetMovement2"},
														{"Fleet Move 3","FleetMovement3"},{"Fleet Move 4", "FleetMovement4"},
														{"UFO Hit",     "UFOHit"}
													};
													size_t maxLabelLen = 0;
													for (auto& item : items) maxLabelLen = std::max(maxLabelLen, strlen(item[0]));
													for (auto& item : items)
													{
														char label[64];
														snprintf(label, sizeof(label), "%-*s", (int)maxLabelLen, item[0]);
														if (ImGui::MenuItem(label)) loadSIAudioWAV(item[1]);
													}
													ImGui::EndMenu();
												}
												ImGui::EndMenu();
											}
											ImGui::EndMenu();
										}
										if (ImGui::BeginMenu("Video"))
										{
											if (ImGui::MenuItem("Aspect Ratio##AspectRatio", NULL, maintainAspectRatio))
											{
												maintainAspectRatio = (maintainAspectRatio == YES ? NO : YES);
												config.put<FLAG>("mods._MAINTAIN_ASPECT_RATIO", maintainAspectRatio);
												boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
											}
											if (ImGui::BeginMenu("Shaders"))
											{
												static int selection = TO_INT(currEnVFilter);
												ImGui::RadioButton("Nearest", &selection, TO_INT(VIDEO_FILTERS::NEAREST_FILTER));
#ifdef __EMSCRIPTEN__
												ImGui::BeginDisabled();
#endif
												ImGui::RadioButton("Bilinear", &selection, TO_INT(VIDEO_FILTERS::BILINEAR_FILTER));
												ImGui::RadioButton("LCD", &selection, TO_INT(VIDEO_FILTERS::LCD_FILTER));
#ifdef __EMSCRIPTEN__
												ImGui::EndDisabled();
#endif
												ImGui::BeginDisabled();
												ImGui::RadioButton("CRT", &selection, TO_INT(VIDEO_FILTERS::CRT_FILTER));
												ImGui::EndDisabled();
												if (currEnVFilter != (VIDEO_FILTERS)selection)
												{
													config.put<std::string>("mods._VIDEO_EFFECTS", vFiltersToConfig.at((VIDEO_FILTERS)selection));
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												currEnVFilter = (VIDEO_FILTERS)selection;
												ImGui::EndMenu();
											}
											ImGui::Separator();
											if (ImGui::BeginMenu("GB", MASQ_ENABLE_GBC))
											{
												if (ImGui::BeginMenu("GB Color Palette##gb_palette_menu"))
												{
													static int selection = TO_INT(currEnGbPalette);
													ImGui::RadioButton("GearBoy", &selection, TO_INT(PALETTE_ID::PALETTE_1));
													ImGui::RadioButton("Black/White", &selection, TO_INT(PALETTE_ID::PALETTE_2));
													ImGui::RadioButton("SameBoy", &selection, TO_INT(PALETTE_ID::PALETTE_3));
													ImGui::RadioButton("BGB", &selection, TO_INT(PALETTE_ID::PALETTE_4));
													if (currEnGbPalette != (PALETTE_ID)selection)
													{
														config.put<std::string>("gb_gbc._force_gb_palette", gbPaletteIDToConfig.at((PALETTE_ID)selection));
														boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
													}
													currEnGbPalette = (PALETTE_ID)selection;
													ImGui::EndMenu();
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("GBC", MASQ_ENABLE_GBC))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gb_gbc._enable_cgb_color_correction", "true"));
												if (ImGui::MenuItem("GBC Color Correction", "C", isTicked))
												{
													isTicked = !isTicked;
													config.put("gb_gbc._enable_cgb_color_correction", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
													currEnGbcPalette = ((currEnGbcPalette == PALETTE_ID::PALETTE_1) ? PALETTE_ID::PALETTE_2 : PALETTE_ID::PALETTE_1);
												}
												ImGui::EndMenu();
											}
											ImGui::EndMenu();
										}
										if (ImGui::BeginMenu("Input"))
										{
											if (ImGui::MenuItem("Accurate Input Sampling", NULL, accurateInputSampling))
											{
												accurateInputSampling = (accurateInputSampling == YES ? NO : YES);
												_ENABLE_ACCURATE_INPUT_SAMPLING = accurateInputSampling;
												config.put<FLAG>("mods._ENABLE_ACCURATE_INPUT_SAMPLING", accurateInputSampling);
												boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
											}
											if (ImGui::IsItemHovered()) ImGui::SetTooltip("Some ROMs like \"tellinglys\" needs this to be enabled");
											ImGui::Separator();
											if (ImGui::BeginMenu("NES", MASQ_ENABLE_NES))
											{
												if (ImGui::MenuItem("Enable Zapper##EnableZapper", NULL, enableZapper))
												{
													enableZapper = (enableZapper == YES ? NO : YES);
												}
												ImGui::EndMenu();
											}
											ImGui::EndMenu();
										}
										if (ImGui::BeginMenu("Other Settings"))
										{
											if (ImGui::BeginMenu("Game Of Life", MASQ_ENABLE_GOL))
											{
												if (ImGui::BeginMenu("Boundary Condition"))
												{
													static FLAG isTicked = to_bool(config.get<std::string>("gameoflife._is_torroidal"));
													if (ImGui::MenuItem("Torroidal", NULL, isTicked))
													{
														isTicked = !isTicked;
														config.put("gameoflife._is_torroidal", isTicked);
														boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
													}
													ImGui::EndMenu();
												}
												if (ImGui::IsItemHovered()) ImGui::SetTooltip("Needs restart to take effect");
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("Chip8 Family##Chip8Family", MASQ_ENABLE_CHIP8))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("chip8._enable_c8_db", "false"));
												if (ImGui::MenuItem("Enable ROM Database##EnableROMDatabase", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("chip8._enable_c8_db", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("Space Invaders##SpaceInvaders", MASQ_ENABLE_SI))
											{
												static FLAG DIP[FOUR] = {
													to_bool(config.get<std::string>("spaceinvaders._DIP3", "true")),
													to_bool(config.get<std::string>("spaceinvaders._DIP5", "true")),
													to_bool(config.get<std::string>("spaceinvaders._DIP6", "true")),
													to_bool(config.get<std::string>("spaceinvaders._DIP7", "true"))
												};
												static const STATE8 DIPLUT[4] = { 3, 5, 6, 7 };
												FLAG wasClicked = NO;
												for (INC8 ii = RESET; ii < (INC8)sizeof(DIP); ii++)
													if (ImGui::Checkbox(("DIP" + std::to_string(DIPLUT[ii])).c_str(), &DIP[ii])) wasClicked = YES;
												if (wasClicked)
												{
													config.put("spaceinvaders._DIP3", DIP[ZERO]); config.put("spaceinvaders._DIP5", DIP[ONE]);
													config.put("spaceinvaders._DIP6", DIP[TWO]);  config.put("spaceinvaders._DIP7", DIP[THREE]);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("NES##NESFamily", MASQ_ENABLE_NES))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("nes._enable_nes_db", "false"));
												if (ImGui::MenuItem("Enable ROM Database##EnableROMDatabase2", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("nes._enable_nes_db", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("GB##GBFamily", MASQ_ENABLE_GBC))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gb_gbc._force_gbc_for_gb", "false"));
												if (ImGui::MenuItem("CGB Mode", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("gb_gbc._force_gbc_for_gb", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											ImGui::EndMenu();
										}
										ImGui::EndMenu();
									}

									// ---- Option menu --------------------------------
									if (ImGui::BeginMenu("Option"))
									{
										if (current_instance->getEmulationID() == EMULATION_ID::DEFAULT_ID)
										{
											ImGui::MenuItem("Debugger", NULL, NO, DISABLED);
											if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
												ImGui::SetTooltip("This setting is available only post game selection");
										}
										else
										{
											if (ImGui::BeginMenu("Debugger"))
											{
												if (ImGui::BeginMenu("CPU", NO)) ImGui::EndMenu();
												if (ImGui::BeginMenu("PPU", NO)) ImGui::EndMenu();
												if (ImGui::BeginMenu("APU", NO)) ImGui::EndMenu();
												ImGui::EndMenu();
											}
										}
										ImGui::Separator();
										if (ImGui::BeginMenu("Cheats"))
										{
											if (ImGui::MenuItem(showCheatWin == NO ? "Open Cheat Hub" : "Close Cheat Hub"))
												showCheatWin = (showCheatWin == NO) ? YES : NO;
											ImGui::EndMenu();
										}
										ImGui::Separator();
										if (ImGui::BeginMenu("Load/Save"))
										{
											static FLAG isQLSTicked = to_bool(config.get<std::string>("mods._ENABLE_QUICK_SAVE", "true"));
											static FLAG isBESSTicked = to_bool(config.get<std::string>("mods._ENABLE_BESS_FORMAT", "false"));
											if (ImGui::MenuItem("Enable Save States", NULL, isQLSTicked))
											{
												isQLSTicked = !isQLSTicked; _ENABLE_QUICK_SAVE = isQLSTicked;
												config.put("mods._ENABLE_QUICK_SAVE", isQLSTicked);
												boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
											}
											if (ImGui::MenuItem("Enable BESS format", NULL, isBESSTicked && _ENABLE_QUICK_SAVE))
											{
												isBESSTicked = !isBESSTicked; _ENABLE_BESS_FORMAT = isBESSTicked && _ENABLE_QUICK_SAVE;
												config.put("mods._ENABLE_BESS_FORMAT", isBESSTicked);
												boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
											}
											if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
												ImGui::SetTooltip("This setting is available only if Save States are enabled");
											ImGui::EndMenu();
										}
										ImGui::Separator();
										ImGui::MenuItem("Logger Configuration##LoggerConfiguration", NULL, NO, NO);
										if (ImGui::MenuItem("Logger Console##LoggerConsole", NULL, showLoggerWin))
											showLoggerWin = (showLoggerWin == NO) ? YES : NO;
										ImGui::Separator();
										if (ImGui::BeginMenu("Theme"))
										{
											const char* themes[] = { "Dark", "Light", "Black" };
											for (int i = 0; i < IM_ARRAYSIZE(themes); i++)
											{
												if (ImGui::MenuItem(themes[i], NULL, currentEmuTheme == i))
												{
													currentEmuTheme = i;
													config.put<std::string>("mods._EMULATOR_THEME", emuThemesToConfig.at((EMULATOR_THEME)i));
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
											}
											ImGui::EndMenu();
										}
										ImGui::EndMenu();
									}

									// ---- Help menu ----------------------------------
									if (ImGui::BeginMenu("Help"))
									{
										if (ImGui::MenuItem("Masquerade-OTA for updates!##OTA")) showUpdWin = YES;
										ImGui::Separator();
										if (ImGui::MenuItem("About")) showAboutWin = YES;
										ImGui::EndMenu();
									}

									ImGui::EndMainMenuBar();
								} // BeginMainMenuBar

								// ---- Emulation window ----------------------------
								ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.5f * ImGui::GetStyle().ScrollbarSize);
								ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
								ImGui::Begin(emuWindow.c_str(), &showEmuWin,
									ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
									ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
									ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);

								if (maintainAspectRatio == NO || current_instance->getEmulationID() == EMULATION_ID::DEFAULT_ID)
								{
									if (current_instance->getEmulationID() == EMULATION_ID::DEFAULT_ID)
									{
										// --- Recently-played home screen ---------
										{
											const char* header = "Load Recently Played Game";
											ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
											float headerTopSpace = 10.0f;
											if (showUpdWin || showAboutWin || showLoggerWin) headerTopSpace *= 2.0f;
											else ImGui::Dummy(ImVec2(0.0f, headerTopSpace));

											ImDrawList* drawList = ImGui::GetWindowDrawList();
											ImVec2      winPos = ImGui::GetWindowPos();
											ImVec2      shiftedPos = { winPos.x, winPos.y + headerTopSpace };
											ImVec2      winSize = ImGui::GetWindowSize();
											ImVec2      winMax = { shiftedPos.x + winSize.x, shiftedPos.y + winSize.y };

											drawList->AddRectFilled(winPos, winMax, ImGui::GetColorU32(ImGuiCol_ChildBg));

											ImVec2 textSize = ImGui::CalcTextSize(header);
											float  headerHeight = textSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
											ImVec2 headerMax = { shiftedPos.x + winSize.x, shiftedPos.y + headerHeight };
											drawList->AddRectFilled(shiftedPos, headerMax, ImGui::GetColorU32(ImGuiCol_TitleBgActive));
											drawList->AddText({ shiftedPos.x + ImGui::GetStyle().FramePadding.x * 2.0f, shiftedPos.y + ImGui::GetStyle().FramePadding.y },
												ImGui::GetColorU32(ImGuiCol_Text), header);
											ImGui::Dummy(ImVec2(0.0f, headerHeight));
											ImGui::PopStyleVar();
										}

										ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_ChildBg]);
										ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
										ImGui::BeginChild("##Child1", ImGui::GetContentRegionAvail());

										if (recentlyOpenedList.empty())
										{
											ImGui::SetCursorPosX(ImGui::GetStyle().FramePadding.x * 2.0f);
											ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
											ImGui::TextDisabled("Nothing to display...");
										}

										int ii = RESET;
										static FLAG isHoveringOverPath[_MAX_RECENTLY_USED_LIST_SIZE] = { NO };
										const FLAG  displayX = NO;

										for (auto it = recentlyOpenedList.begin(); it != recentlyOpenedList.end(); )
										{
											FLAG itemSelected = NO;
											const std::filesystem::path filePath = it->c_str();

											const char* romType = "Unknown";
											std::string ext = filePath.extension().string();
											if (ext == ".gba") romType = "GBA";
											else if (ext == ".gbc") romType = "GBC";
											else if (ext == ".gb")  romType = "GB";
											else if (ext == ".nes") romType = "NES";
											else if (ext == ".ch8") romType = "CH8";

											if (strcmp(romType, "Unknown") == 0)
											{
												++it; CONTINUE;
											}

											ImGui::PushID(ii);
											ImVec2 rowStartPos = ImGui::GetCursorPos();
											FLAG   hoverOverGameType = NO;
											float  leftOffset = 40.0f;

											// Game-type label
											{
												ImVec2 romTypeSize = ImGui::CalcTextSize(romType);
												ImVec2 gameTypeBtn = { leftOffset, romTypeSize.y * 2.5f };
												ImGui::InvisibleButton(("##gameType_hover_zone_" + std::to_string(ii)).c_str(), gameTypeBtn);
												if (ImGui::IsItemHovered())
												{
													hoverOverGameType = YES;
													if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
													{
														dynamicDragNDropAndMenuSelect.push_back(filePath.string()); itemSelected = YES;
													}
													else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
													{
														it = recentlyOpenedList.erase(it); ImGui::PopID(); CONTINUE;
													}
												}
												ImGui::SetCursorPosX(rowStartPos.x + ImGui::GetStyle().FramePadding.x * 2.0f);
												ImGui::SetCursorPosY(rowStartPos.y + romTypeSize.y * 0.5f + ImGui::GetStyle().FramePadding.y);
												ImGui::TextDisabled("%s", romType);
											}

											// File-name selectable
											ImGui::SetCursorPos(rowStartPos);
											ImVec2 xSize = ImGui::CalcTextSize("X");
											float  deleteButtonW = xSize.x;
											ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x + leftOffset);
											float selectableWidth = displayX == NO
												? ImGui::GetContentRegionAvail().x
												: ImGui::GetContentRegionAvail().x - deleteButtonW - 10.0f;

											{
												ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
												ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
												if (ImGui::Selectable((filePath.stem().string() + "##file").c_str(), isHoveringOverPath[ii], 0, ImVec2(selectableWidth, 0)))
												{
													dynamicDragNDropAndMenuSelect.push_back(filePath.string()); itemSelected = YES;
												}
												ImGui::PopStyleColor(2);
												if (ImGui::IsItemHovered() || hoverOverGameType == YES)
												{
													isHoveringOverPath[ii] = YES;
													if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
													{
														it = recentlyOpenedList.erase(it); ImGui::PopID(); CONTINUE;
													}
												}
												else
												{
													isHoveringOverPath[ii] = NO;
												}
											}

											ImVec2 posSelectable = ImGui::GetItemRectMin();
											ImVec2 sizeSelectable = ImGui::GetItemRectSize();
											ImVec4 colorSelectable = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);

											// Path label
											{
												ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x + leftOffset);
												ImVec2 pathInvbuttonPos = ImGui::GetCursorScreenPos();
												ImVec2 pathInvButton = ImGui::CalcTextSize(filePath.string().c_str());
												pathInvButton.x = ImGui::GetContentRegionAvail().x;
												ImGui::InvisibleButton(("##path_hover_zone_" + std::to_string(ii)).c_str(), pathInvButton);
												if (ImGui::IsItemHovered())
												{
													isHoveringOverPath[ii] = YES;
													if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
													{
														dynamicDragNDropAndMenuSelect.push_back(filePath.string()); itemSelected = YES;
													}
													else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
													{
														it = recentlyOpenedList.erase(it); ImGui::PopID(); CONTINUE;
													}
												}
												ImGui::SetCursorScreenPos(pathInvbuttonPos);
												ImGui::TextDisabled("%s", filePath.string().c_str());
											}

											// Hover highlight for game-type + path strips
											if (isHoveringOverPath[ii] == YES)
											{
												float px = ImGui::GetStyle().FramePadding.x, py = ImGui::GetStyle().FramePadding.y;
												ImVec2 pos = ImGui::GetItemRectMin();
												ImVec2 bgMin = { pos.x - px * 0.5f, pos.y - py * 1.5f };
												ImVec2 bgMax = { bgMin.x + sizeSelectable.x, bgMin.y + sizeSelectable.y + py };
												ImGui::GetWindowDrawList()->AddRectFilled(bgMin, bgMax, ImGui::GetColorU32(colorSelectable));
												bgMin = { posSelectable.x - leftOffset, posSelectable.y };
												bgMax = { bgMin.x + leftOffset, bgMin.y + sizeSelectable.y * 2.0f + py };
												ImGui::GetWindowDrawList()->AddRectFilled(bgMin, bgMax, ImGui::GetColorU32(colorSelectable));
											}

											ImGui::Separator();
											ImGui::PopID();
											++ii; ++it;
											if (itemSelected) BREAK;
										}

										ImGui::EndChild();
										ImGui::PopStyleColor();
										ImGui::PopStyleVar();
									}
									else
									{
										ImGui::Image((ImTextureID)(uintptr_t)masquerade_texture, (ImVec2)ImGui::GetContentRegionAvail());
									}
								}
								else
								{
									// Aspect-ratio preserving display
									ImVec2 avail_size = ImGui::GetContentRegionAvail();
									float  fbWidth = (float)(current_instance->getScreenWidth() * FRAME_BUFFER_SCALE);
									float  fbHeight = (float)(current_instance->getScreenHeight() * FRAME_BUFFER_SCALE);
									float  fbAspect = fbWidth / fbHeight;

									ImVec2 imageSize;
									if (avail_size.x / avail_size.y > fbAspect)
									{
										imageSize.y = avail_size.y; imageSize.x = imageSize.y * fbAspect;
									}
									else
									{
										imageSize.x = avail_size.x; imageSize.y = imageSize.x / fbAspect;
									}

									ImVec2 cursor_pos = ImGui::GetCursorPos();
									ImVec2 offset = { (avail_size.x - imageSize.x) * 0.5f, (avail_size.y - imageSize.y) * 0.5f };
									ImGui::SetCursorPos(ImVec2(cursor_pos.x + offset.x, cursor_pos.y + offset.y));
									ImGui::Image((ImTextureID)(uintptr_t)masquerade_texture, imageSize);
								}

								emuWindowX = ImGui::GetWindowPos().x;
								emuWindowY = ImGui::GetWindowPos().y;
								emuWindowMaxX = ImGui::GetWindowWidth();
								emuWindowMaxY = ImGui::GetWindowHeight();

								ImGui::End();
								ImGui::PopStyleVar(); // WindowPadding

								// ---- Updater window -------------------------
								if (showUpdWin == YES)
								{
#ifndef __EMSCRIPTEN__
									ImGui::Begin("Updater", &showUpdWin, ImGuiWindowFlags_AlwaysAutoResize);
									ImGui::Text("Current Version : v%.4f", VERSION);
									ImGui::Text("Build Type : P0052");
									ImGui::Separator();
									ImGui::End();
#endif
								}

								// ---- About window ---------------------------
								if (showAboutWin == YES)
								{
									const float columnWidth = 150.0f;
									ImGui::Begin("About", &showAboutWin, ImGuiWindowFlags_AlwaysAutoResize);

									if (ImGui::CollapsingHeader("Build Info"))
									{
										ImGui::BeginGroup();
										ImGui::Columns(2, nullptr, false);
										ImGui::SetColumnWidth(0, columnWidth);

										ImGui::Text("Build");       ImGui::NextColumn(); ImGui::Text(": P0052");               ImGui::NextColumn();
										ImGui::Text("Version");     ImGui::NextColumn(); ImGui::Text(": v%.4f", VERSION);      ImGui::NextColumn();
										ImGui::Text("Built On");    ImGui::NextColumn(); ImGui::Text(": %s at %s IST", __DATE__, __TIME__); ImGui::NextColumn();
#if defined(NDEBUG)
										ImGui::Text("Build Type");  ImGui::NextColumn(); ImGui::Text(": Release");             ImGui::NextColumn();
#else
										ImGui::Text("Build Type");  ImGui::NextColumn(); ImGui::Text(": Debug");               ImGui::NextColumn();
#endif
										ImGui::Text("Commit");      ImGui::NextColumn(); ImGui::Text(": %s", MASQ_GIT_HASH);  ImGui::NextColumn();

										ImGui::Text("C++ Standard"); ImGui::NextColumn();
#if   __cplusplus == 199711L 
										ImGui::Text(": C++98/03");
#elif __cplusplus == 201103L 
										ImGui::Text(": C++11");
#elif __cplusplus == 201402L 
										ImGui::Text(": C++14");
#elif __cplusplus == 201703L 
										ImGui::Text(": C++17");
#elif __cplusplus == 202002L 
										ImGui::Text(": C++20");
#elif __cplusplus >  202002L 
										ImGui::Text(": C++23 or newer");
#else                        
										ImGui::Text(": Unknown (%ld)", __cplusplus);
#endif
										ImGui::NextColumn();

										ImGui::Text("Compiler"); ImGui::NextColumn();
#if   defined(_MSC_VER)  
										ImGui::Text(": MSVC (%d)", _MSC_VER);
#elif defined(__clang__) 
										ImGui::Text(": Clang %d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUG__)  
										ImGui::Text(": G++ %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else                    
										ImGui::Text(": Unknown");
#endif
										ImGui::NextColumn();

										ImGui::Text("Architecture"); ImGui::NextColumn();
#if   defined(_M_X64) || defined(__x86_64__)   
										ImGui::Text(": x86_64");
#elif defined(_M_IX86) || defined(__i386__)    
										ImGui::Text(": x86 (32-bit)");
#elif defined(__aarch64__) || defined(_M_ARM64) 
										ImGui::Text(": ARM64");
#elif defined(__arm__) || defined(_M_ARM)      
										ImGui::Text(": ARM (32-bit)");
#else                                          
										ImGui::Text(": Unknown");
#endif
										ImGui::NextColumn();

										ImGui::Text("Platform"); ImGui::NextColumn();
#if   defined(_WIN32)   
										ImGui::Text(": Windows");
#elif defined(__linux__) 
										ImGui::Text(": Linux");
#elif defined(__APPLE__) 
										ImGui::Text(": macOS");
#else                   
										ImGui::Text(": Unknown");
#endif
										ImGui::Columns(1);
										ImGui::EndGroup();
									}

									if (ImGui::CollapsingHeader("Library Info"))
									{
										ImGui::BeginGroup();
										ImGui::Columns(2, nullptr, false);
										ImGui::SetColumnWidth(0, columnWidth);
#ifdef IMGUI_VERSION
										ImGui::Text("Dear ImGui"); ImGui::NextColumn(); ImGui::Text(": v%s", IMGUI_VERSION); ImGui::NextColumn();
#endif
#ifdef SDL_MAJOR_VERSION
										ImGui::Text("SDL Header"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION); ImGui::NextColumn();
#endif
#if defined(SDL_VERSIONNUM_MAJOR)
										{
											int sdlVer = SDL_GetVersion();
											ImGui::Text("SDL Linked"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", SDL_VERSIONNUM_MAJOR(sdlVer), SDL_VERSIONNUM_MINOR(sdlVer), SDL_VERSIONNUM_MICRO(sdlVer)); ImGui::NextColumn();
										}
#endif
#ifdef GLAD_GL_VERSION_1_0
										if (GLAD_GL_VERSION_1_0)
										{
											ImGui::Text("Glad"); ImGui::NextColumn(); ImGui::Text(": %d.%d", GLVersion.major, GLVersion.minor); ImGui::NextColumn();
										}
#endif
#ifdef BOOST_VERSION
										ImGui::Text("Boost"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", BOOST_VERSION / 100000, (BOOST_VERSION / 100) % 1000, BOOST_VERSION % 100); ImGui::NextColumn();
#endif
#ifdef MZ_VERSION
										ImGui::Text("Miniz"); ImGui::NextColumn(); ImGui::Text(": v%s", mz_version()); ImGui::NextColumn();
#endif
										ImGui::Text("Nativefiledialog-extended"); ImGui::NextColumn(); ImGui::Text(": 1.2.1"); ImGui::NextColumn();
#ifdef __EMSCRIPTEN__
										ImGui::Text("Emscripten-browser-file"); ImGui::NextColumn(); ImGui::Text(": 0.1.0"); ImGui::NextColumn();
#endif
#ifdef STB_IMAGE_IMPLEMENTATION
										ImGui::Text("STB (stb_image)"); ImGui::NextColumn(); ImGui::Text(": %.2f", 2.28); ImGui::NextColumn();
#endif
										ImGui::Text("Chip-8-database"); ImGui::NextColumn(); ImGui::Text(": 0.1.0"); ImGui::NextColumn();
										ImGui::Text("NES-database"); ImGui::NextColumn(); ImGui::Text(": 0.1.0"); ImGui::NextColumn();
										ImGui::Columns(1);
										ImGui::EndGroup();
									}

									if (ImGui::CollapsingHeader("SDL Info"))
									{
										ImGui::BeginGroup();
										ImGui::Columns(2, nullptr, false);
										ImGui::SetColumnWidth(0, columnWidth);
#ifdef SDL_VERSION
										ImGui::Text("SDL Header"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION); ImGui::NextColumn();
#endif
#if defined(SDL_VERSIONNUM_MAJOR)
										{
											int sdlVer = SDL_GetVersion();
											ImGui::Text("SDL Linked"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", SDL_VERSIONNUM_MAJOR(sdlVer), SDL_VERSIONNUM_MINOR(sdlVer), SDL_VERSIONNUM_MICRO(sdlVer)); ImGui::NextColumn();
										}
#endif
										{
											const char* rev = SDL_GetRevision();
											if (rev && *rev)
											{
												ImGui::Text("SDL Revision"); ImGui::NextColumn(); ImGui::Text(": %s", rev); ImGui::NextColumn();
											}
										}
										ImGui::Columns(1);
										ImGui::EndGroup();
									}

									if (ImGui::CollapsingHeader("CPU Info"))
									{
										ImGui::BeginGroup();
										ImGui::Columns(2, nullptr, false);
										ImGui::SetColumnWidth(0, columnWidth);
										ImGui::Text("Logical Cores"); ImGui::NextColumn(); ImGui::Text(": %d", SDL_GetNumLogicalCPUCores()); ImGui::NextColumn();
										ImGui::Text("L1 Cache");      ImGui::NextColumn(); ImGui::Text(": %d KB", SDL_GetCPUCacheLineSize());    ImGui::NextColumn();
										ImGui::Text("System RAM");    ImGui::NextColumn(); ImGui::Text(": %d MB", SDL_GetSystemRAM());           ImGui::NextColumn();
										ImGui::Dummy(ImVec2(0.0f, 6.0f));
										ImGui::Text("Features:"); ImGui::NextColumn(); ImGui::Text(""); ImGui::NextColumn();
										ImGui::Text("SSE");    ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("SSE2");   ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE2() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("SSE3");   ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE3() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("SSE4.1"); ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE41() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("SSE4.2"); ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE42() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("AVX");    ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasAVX() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("AVX2");   ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasAVX2() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("NEON");   ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasNEON() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Columns(1);
										ImGui::EndGroup();
									}

									if (ImGui::CollapsingHeader("GPU Info"))
									{
										ImGui::BeginGroup();
										ImGui::Columns(2, nullptr, false);
										ImGui::SetColumnWidth(0, columnWidth);
										const GLubyte* vendor = glGetString(GL_VENDOR);
										const GLubyte* renderer = glGetString(GL_RENDERER);
										const GLubyte* version = glGetString(GL_VERSION);
										const GLubyte* shading = glGetString(GL_SHADING_LANGUAGE_VERSION);
										if (vendor)
										{
											ImGui::Text("GL Vendor");    ImGui::NextColumn(); ImGui::Text(": %s", vendor);   ImGui::NextColumn();
										}
										if (renderer)
										{
											ImGui::Text("GL Renderer");  ImGui::NextColumn(); ImGui::Text(": %s", renderer); ImGui::NextColumn();
										}
										if (version)
										{
											ImGui::Text("GL Version");   ImGui::NextColumn(); ImGui::Text(": %s", version);  ImGui::NextColumn();
										}
										if (shading)
										{
											ImGui::Text("GLSL Version"); ImGui::NextColumn(); ImGui::Text(": %s", shading);  ImGui::NextColumn();
										}
										ImGui::Columns(1);
										ImGui::EndGroup();
									}

									if (ImGui::CollapsingHeader("Developer Info"))
									{
										ImGui::Text("By Kotambail-Hegde");
										ImGui::TextLinkOpenURL("Homepage", "https://kotambail-hegde.github.io/Masquerade-Emulator/");
										ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();
										ImGui::TextLinkOpenURL("Releases", "https://github.com/Kotambail-Hegde/Masquerade-Emulator/releases");
										ImGui::Separator();
										ImGui::Text("(c) 2017-2026 Saurabh S Hegde");
										ImGui::Text("For licensing, refer to");
										ImGui::SameLine();
										ImGui::TextLinkOpenURL("License", "https://github.com/Kotambail-Hegde/Masquerade-Emulator/blob/main/LICENSE.md");
									}
									ImGui::End();
								}

								// ---- Logger console -------------------------
								if (showLoggerWin == YES)
								{
									ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 255));
									ImGui::Begin("Logger Console", &showLoggerWin, ImGuiWindowFlags_AlwaysAutoResize);
									appLog.Draw();
									ImGui::End();
									ImGui::PopStyleColor();
								}

								// ---- Cheat hub ------------------------------
								if (showCheatWin == YES
									&& ((current_instance->getEmulationID() == EMULATION_ID::NES_ID)
										|| (current_instance->getEmulationID() == EMULATION_ID::GB_GBC_ID)
										|| (current_instance->getEmulationID() == EMULATION_ID::GBA_ID)))
								{
									ImGui::Begin("Cheats", &showCheatWin, ImGuiWindowFlags_AlwaysAutoResize);
									ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
									FLAG atleastOneCE = NO;
									static std::string dgg, dgs, dgar, dcb, gamegenie, gameshark, actionreplay, codebreaker;

									static int32_t selectedCEMode = CheatEngine_t::CHEATING_ENGINE::GAMEGENIE;

									// Reset all per-game cheat UI state when the loaded game changes
									static EMULATION_ID lastEmuID = EMULATION_ID::DEFAULT_ID;
									if (current_instance->getEmulationID() != lastEmuID)
									{
										lastEmuID = current_instance->getEmulationID();
										dgg.clear(); dgs.clear(); dgar.clear(); dcb.clear();
										gamegenie.clear(); gameshark.clear(); actionreplay.clear(); codebreaker.clear();
										selectedCEMode = CheatEngine_t::CHEATING_ENGINE::GAMEGENIE;
									}

									bool isGBA = (current_instance->getEmulationID() == EMULATION_ID::GBA_ID);
									bool isGBGBC = (current_instance->getEmulationID() == EMULATION_ID::GB_GBC_ID);

									// Radio buttons -- AR v3 only shown for GBA
									ImGui::RadioButton("GameGenie", &selectedCEMode, CheatEngine_t::CHEATING_ENGINE::GAMEGENIE);
									if (isGBA || isGBGBC)
										ImGui::RadioButton("GameShark", &selectedCEMode, CheatEngine_t::CHEATING_ENGINE::GAMESHARK);
									if (isGBA)
									{
										ImGui::RadioButton("Action Replay V3", &selectedCEMode, CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3);
										ImGui::RadioButton("CodeBreaker", &selectedCEMode, CheatEngine_t::CHEATING_ENGINE::CODEBREAKER);
									}

									ceMAS->setCheatEngineMode((CheatEngine_t::CHEATING_ENGINE)selectedCEMode, current_instance->getEmulationID());

									// GameGenie input
									{
										atleastOneCE = YES;
										ImGui::Text("GameGenie");
										ImGui::InputText("GG Name", &dgg, input_text_flags);
										if (ImGui::InputText("GG Code", &gamegenie, input_text_flags))
										{
											if (dgg.empty()) dgg = gamegenie;
											selectedCEMode = CheatEngine_t::CHEATING_ENGINE::GAMEGENIE;
											ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, current_instance->getEmulationID());
											ceMAS->applyNewCheat(dgg, gamegenie);
											dgg.clear(); gamegenie.clear();
										}
										ImGui::Separator();
									}

									// GameShark input -- GB/GBC and GBA only
									if (isGBA || isGBGBC)
									{
										atleastOneCE = YES;
										ImGui::Text("GameShark");
										ImGui::InputText("GS Name", &dgs, input_text_flags);
										if (ImGui::InputText("GS Code", &gameshark, input_text_flags))
										{
											if (dgs.empty()) dgs = gameshark;
											selectedCEMode = CheatEngine_t::CHEATING_ENGINE::GAMESHARK;
											ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMESHARK, current_instance->getEmulationID());
											ceMAS->applyNewCheat(dgs, gameshark);
											dgs.clear(); gameshark.clear();
										}
										ImGui::Separator();
									}

									// Action Replay V3 and Codebreaker input -- GBA only
									if (isGBA)
									{
										atleastOneCE = YES;
										ImGui::Text("Action Replay V3");
										ImGui::InputText("AR Name", &dgar, input_text_flags);
										if (ImGui::InputText("AR Code", &actionreplay, input_text_flags))
										{
											if (dgar.empty()) dgar = actionreplay;
											selectedCEMode = CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3;
											ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3, current_instance->getEmulationID());
											ceMAS->applyNewCheat(dgar, actionreplay);
											dgar.clear(); actionreplay.clear();
										}
										ImGui::Separator();
										ImGui::Text("CodeBreaker");
										ImGui::InputText("CB Name", &dcb, input_text_flags);
										if (ImGui::InputText("CB Code", &codebreaker, input_text_flags))
										{
											if (dcb.empty()) dcb = codebreaker;
											selectedCEMode = CheatEngine_t::CHEATING_ENGINE::CODEBREAKER;
											ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::CODEBREAKER, current_instance->getEmulationID());
											ceMAS->applyNewCheat(dcb, codebreaker);
											dcb.clear(); codebreaker.clear();
										}
										ImGui::Separator();
									}

									// Cheat list tabs
									if (atleastOneCE == YES)
									{
										if (ImGui::BeginTabBar("Cheat List", ImGuiTabBarFlags_None))
										{
											// GameGenie tab
											if (ImGui::BeginTabItem("GameGenie"))
											{
												INC8 ii = RESET;
												std::string ggKeyToDelete;
												auto ggState = ceMAS->getCheatEnDisList(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE);
												for (auto& [key, value] : ceMAS->getCheatList(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE))
												{
													if (ii < MAX_CHEAT_COUNT_PER_ENGINE)
													{
														FLAG enabledFlag = NO;
														if (ggState.find(key) != ggState.end())
															enabledFlag = ggState[key];
														bool enabledBool = (enabledFlag == YES);

														uint32_t codeCount = ceMAS->getSubCodeCount(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, key);
														if (codeCount == 0) codeCount = 1;

														std::string displayLabel = value;
														if (codeCount > 1)
															displayLabel += " (" + std::to_string(codeCount) + " codes)";

														if (ImGui::Checkbox(displayLabel.c_str(), &enabledBool))
														{
															if (enabledBool == true)
															{
																selectedCEMode = TO_UINT(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE);
																ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, current_instance->getEmulationID());
																ceMAS->enableCheat(key);
															}
															else
															{
																ceMAS->disableCheat(key);
															}
														}
														ImGui::SameLine();
														float btnW = ImGui::CalcTextSize("Delete##gg##").x + ImGui::GetStyle().FramePadding.x * 2.0f;
														ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - btnW);
														if (ImGui::Button(std::string("Delete##gg##" + std::to_string(ii)).c_str()))
														{
															if (ceMAS->getCheatEngineMode() == CheatEngine_t::CHEATING_ENGINE::GAMEGENIE)
																ggKeyToDelete = key;
														}
														ii++;
													}
												}
												if (!ggKeyToDelete.empty())
													ceMAS->deleteCheat(ggKeyToDelete);
												ImGui::EndTabItem();
											}

											// GameShark tab -- GB/GBC and GBA only
											if ((isGBA || isGBGBC) && ImGui::BeginTabItem("GameShark"))
											{
												INC8 ii = RESET;
												std::string gsKeyToDelete;
												auto gsState = ceMAS->getCheatEnDisList(CheatEngine_t::CHEATING_ENGINE::GAMESHARK);
												for (auto& [key, value] : ceMAS->getCheatList(CheatEngine_t::CHEATING_ENGINE::GAMESHARK))
												{
													if (ii < MAX_CHEAT_COUNT_PER_ENGINE)
													{
														FLAG enabledFlag = NO;
														if (gsState.find(key) != gsState.end())
															enabledFlag = gsState[key];
														bool enabledBool = (enabledFlag == YES);

														uint32_t codeCount = ceMAS->getSubCodeCount(CheatEngine_t::CHEATING_ENGINE::GAMESHARK, key);
														if (codeCount == 0) codeCount = 1;

														std::string displayLabel = value;
														if (codeCount > 1)
															displayLabel += " (" + std::to_string(codeCount) + " codes)";

														if (ImGui::Checkbox(displayLabel.c_str(), &enabledBool))
														{
															if (enabledBool == true)
															{
																selectedCEMode = TO_UINT(CheatEngine_t::CHEATING_ENGINE::GAMESHARK);
																ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMESHARK, current_instance->getEmulationID());
																ceMAS->enableCheat(key);
															}
															else
															{
																ceMAS->disableCheat(key);
															}
														}
														ImGui::SameLine();
														float btnW = ImGui::CalcTextSize("Delete##gs##").x + ImGui::GetStyle().FramePadding.x * 2.0f;
														ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - btnW);
														if (ImGui::Button(std::string("Delete##gs##" + std::to_string(ii)).c_str()))
														{
															if (ceMAS->getCheatEngineMode() == CheatEngine_t::CHEATING_ENGINE::GAMESHARK)
																gsKeyToDelete = key;
														}
														ii++;
													}
												}
												if (!gsKeyToDelete.empty())
													ceMAS->deleteCheat(gsKeyToDelete);
												ImGui::EndTabItem();
											}

											// Action Replay V3 tab -- GBA only
											if (isGBA && ImGui::BeginTabItem("Action Replay V3"))
											{
												INC8 ii = RESET;
												std::string arKeyToDelete;
												auto arState = ceMAS->getCheatEnDisList(CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3);
												for (auto& [key, value] : ceMAS->getCheatList(CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3))
												{
													if (ii < MAX_CHEAT_COUNT_PER_ENGINE)
													{
														FLAG enabledFlag = NO;
														if (arState.find(key) != arState.end())
															enabledFlag = arState[key];
														bool enabledBool = (enabledFlag == YES);

														uint32_t codeCount = ceMAS->getSubCodeCount(CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3, key);
														if (codeCount == 0) codeCount = 1;

														std::string displayLabel = value;
														if (codeCount > 1)
															displayLabel += " (" + std::to_string(codeCount) + " codes)";

														if (ImGui::Checkbox(displayLabel.c_str(), &enabledBool))
														{
															if (enabledBool == true)
															{
																selectedCEMode = TO_UINT(CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3);
																ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3, current_instance->getEmulationID());
																ceMAS->enableCheat(key);
															}
															else
															{
																ceMAS->disableCheat(key);
															}
														}
														ImGui::SameLine();
														float btnW = ImGui::CalcTextSize("Delete##ar##").x + ImGui::GetStyle().FramePadding.x * 2.0f;
														ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - btnW);
														if (ImGui::Button(std::string("Delete##ar##" + std::to_string(ii)).c_str()))
														{
															if (ceMAS->getCheatEngineMode() == CheatEngine_t::CHEATING_ENGINE::ACTION_REPLAY_V3)
																arKeyToDelete = key;
														}
														ii++;
													}
												}
												if (!arKeyToDelete.empty())
													ceMAS->deleteCheat(arKeyToDelete);
												ImGui::EndTabItem();
											}

											// CodeBreaker tab -- GBA only
											if (isGBA && ImGui::BeginTabItem("CodeBreaker"))
											{
												INC8 ii = RESET;
												std::string cbKeyToDelete;
												auto cbState = ceMAS->getCheatEnDisList(CheatEngine_t::CHEATING_ENGINE::CODEBREAKER);
												for (auto& [key, value] : ceMAS->getCheatList(CheatEngine_t::CHEATING_ENGINE::CODEBREAKER))
												{
													if (ii < MAX_CHEAT_COUNT_PER_ENGINE)
													{
														FLAG enabledFlag = NO;
														if (cbState.find(key) != cbState.end())
															enabledFlag = cbState[key];
														bool enabledBool = (enabledFlag == YES);

														uint32_t codeCount = ceMAS->getSubCodeCount(CheatEngine_t::CHEATING_ENGINE::CODEBREAKER, key);
														if (codeCount == 0) codeCount = 1;

														std::string displayLabel = value;
														if (codeCount > 1)
															displayLabel += " (" + std::to_string(codeCount) + " codes)";

														if (ImGui::Checkbox(displayLabel.c_str(), &enabledBool))
														{
															if (enabledBool == true)
															{
																selectedCEMode = TO_UINT(CheatEngine_t::CHEATING_ENGINE::CODEBREAKER);
																ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::CODEBREAKER, current_instance->getEmulationID());
																ceMAS->enableCheat(key);
															}
															else
															{
																ceMAS->disableCheat(key);
															}
														}
														ImGui::SameLine();
														float btnW = ImGui::CalcTextSize("Delete##cb##").x + ImGui::GetStyle().FramePadding.x * 2.0f;
														ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - btnW);
														if (ImGui::Button(std::string("Delete##cb##" + std::to_string(ii)).c_str()))
														{
															if (ceMAS->getCheatEngineMode() == CheatEngine_t::CHEATING_ENGINE::CODEBREAKER)
																cbKeyToDelete = key;
														}
														ii++;
													}
												}
												if (!cbKeyToDelete.empty())
													ceMAS->deleteCheat(cbKeyToDelete);
												ImGui::EndTabItem();
											}

											ImGui::EndTabBar();
										}
									}

									ImGui::End();
								}

								ImGui::PopStyleVar(); // ScrollbarSize

								// ---- Per-frame emulation update --------------
								if (OnUserUpdate() != SUCCESS)
								{
#ifndef __EMSCRIPTEN__
									done = YES;
#else
									if (SavePersistentFSComplete == YES || ClearPersistentFSComplete == YES)
									{
										SavePersistentFSComplete = NO; ClearPersistentFSComplete = NO;
										done = YES;
									}
#endif
								}
							}
							else // initScreen == YES
							{
								if (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))
								{
									initScreen = NO;
								}
								else
								{
									ImGui::Begin(emuWindow.c_str(), &showEmuWin,
										ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
										ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
									ImVec2 image_size = ImVec2((float)clickWinWidth, (float)clickWinHeight);
									ImGui::SetCursorPosX((ImGui::GetWindowSize().x - image_size.x) * 0.5f);
									ImGui::SetCursorPosY((ImGui::GetWindowSize().y - image_size.y) * 0.5f);
									ImGui::Image((ImTextureID)(uintptr_t)clickWinTexture, image_size);
									ImGui::End();
								}
							}

							// ---- Render ----------------------------------
							ImGui::Render();
							glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
							glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
								clear_color.z * clear_color.w, clear_color.w);
							glClear(GL_COLOR_BUFFER_BIT);
							ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

							if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
							{
								SDL_Window* backup_win = SDL_GL_GetCurrentWindow();
								SDL_GLContext backup_ctx = SDL_GL_GetCurrentContext();
								ImGui::UpdatePlatformWindows();
								ImGui::RenderPlatformWindowsDefault();
								SDL_GL_MakeCurrent(backup_win, backup_ctx);
							}
							SDL_GL_SwapWindow(window);

#ifndef __EMSCRIPTEN__
							// Precise FPS limiter
							if (1000000000 / myFPS > SDL_GetTicksNS() - tickAtStart)
								SDL_DelayPrecise((uint64_t)(1000000000.0f / myFPS) - (SDL_GetTicksNS() - tickAtStart));
#endif

							// ---- Emscripten restart handling --------------
							if (done == YES)
							{
#ifdef __EMSCRIPTEN__
								TODO("No cleanup of Masquerade, IMGUI, GLES3 or SDL3 is done here as doing so causes issues reloading .js/.wasm");
								INFO("Restarting Masquerade");
								emscripten_cancel_main_loop();

								if (rebootNeededOnMenuClick == YES)
								{
									emscripten_run_script("location.reload();");
								}
								else
								{
									BYTE numberOfInputs = RESET;
									if (dynamicDragNDropAndMenuSelect.size() != ZERO)
									{
										numberOfInputs = dynamicDragNDropAndMenuSelect.size();
										for (int ii = ZERO; ii < numberOfInputs; ii++)
											romsToRun[ii] = dynamicDragNDropAndMenuSelect[ii].c_str();

										if (numberOfInputs > ONE)
										{
											for (uint32_t ii = ZERO; ii < numberOfInputs; ii++)
											{
												if (numberOfInputs == FOUR)
												{
													std::string SI_ROMS_EXT_LIST[FOUR] = { ".e",".f",".g",".h" };
													for (uint32_t jj = ZERO; jj < MAX_NUMBER_ROMS_FOR_SI; jj++)
														if (dynamicDragNDropAndMenuSelect[ii].find(SI_ROMS_EXT_LIST[jj]) != std::string::npos)
															romsToRun[jj] = dynamicDragNDropAndMenuSelect[ii].c_str();
												}
												else if (numberOfInputs == TEN)
												{
													std::string PM_ROMS_EXT_LIST[TEN] = { "6e","6f","6h","6j","7f","4a","5e","5f","1m","3m" };
													for (uint32_t jj = ZERO; jj < MAX_NUMBER_ROMS_FOR_PM; jj++)
														if (dynamicDragNDropAndMenuSelect[ii].find(PM_ROMS_EXT_LIST[jj]) != std::string::npos)
															romsToRun[jj] = dynamicDragNDropAndMenuSelect[ii].c_str();
												}
												else if (numberOfInputs == THIRTEEN)
												{
													std::string MSPM_ROMS_EXT_LIST[THIRTEEN] = { ".6e",".6f",".6h",".6j",".7f",".4a","5e","5f",".1m",".3m","u5","u6","u7" };
													for (uint32_t jj = ZERO; jj < MAX_NUMBER_ROMS_FOR_MSPM; jj++)
														if (dynamicDragNDropAndMenuSelect[ii].find(MSPM_ROMS_EXT_LIST[jj]) != std::string::npos)
															romsToRun[jj] = dynamicDragNDropAndMenuSelect[ii].c_str();
												}
											}
										}

										dynamicDragNDropAndMenuSelect.clear();

										std::string js = "setTimeout(() => { let roms = [";
										for (int i = 0; i < numberOfInputs; ++i)
										{
											std::string romPath = romsToRun[i];
											if (romPath.rfind("/persistent/", 0) != 0) romPath = "/persistent/" + romPath;
											std::string urlRomPath = romPath.substr(1);
											js += "'" + urlRomPath + "'";
											if (i < numberOfInputs - 1) js += ",";
										}
										js += "]; let url = window.location.pathname.replace(/[^/]+$/, '') + 'masquerade.html?roms=' + roms.length;";
										js += "roms.forEach((rom, i) => url += '&rom' + i + '=' + encodeURIComponent(rom));";
										js += "location.href = url; }, 0);";
										emscripten_run_script(js.c_str());
									}
									else
									{
										emscripten_run_script("location.href = window.location.pathname.replace(/[^/]+$/, '') + 'masquerade.html';");
									}
								}
#endif // __EMSCRIPTEN__
							}
						}
					}; // LOOP lambda

#if defined(__EMSCRIPTEN__) && ENABLED
				static double accumulator = 0.0;
				static double previousTime = emscripten_get_now() / 1000.0;

				double currentTime = emscripten_get_now() / 1000.0;
				double deltaTime = currentTime - previousTime;
				previousTime = currentTime;
				deltaTime = std::clamp(deltaTime, 0.0, 0.1);
				accumulator += deltaTime;

				constexpr int MAX_UPDATES = 2;
				int numUpdates = 0;
				while (accumulator >= timestep && numUpdates < MAX_UPDATES)
				{
					LOOP();
					accumulator -= timestep;
					++numUpdates;
				}
#else
				LOOP();
#endif
			} // while (!done)

#ifdef __EMSCRIPTEN__
			EMSCRIPTEN_MAINLOOP_END;
#else
			if (RUN_IMGUI_DEMO == NO)
			{
				glDeleteTextures(1, &masquerade_texture);
				masquerade_texture = 0; // Reset!

				OnUserDestroy(window);

#if (GL_FIXED_FUNCTION_PIPELINE == NO)
				glDeleteBuffers(1, &fullscreenVBO);
				fullscreenVBO = 0; // Reset!

				glDeleteVertexArrays(1, &fullscreenVAO);
				fullscreenVAO = 0; // Reset!

				glDeleteProgram(shaderProgramBasic);
				shaderProgramBasic = 0; // Reset!

				glDeleteProgram(shaderProgramBlend);
				shaderProgramBlend = 0; // Reset!
#endif
				glDeleteFramebuffers(1, &frame_buffer);
				frame_buffer = 0; // Reset!
			}

			NFD_Quit();
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			ImGui::DestroyContext();
			SDL_GL_DestroyContext(gl_context);
			SDL_StopTextInput(window);
			SDL_DestroyWindow(window);
			SDL_Quit();
#endif
		} // else (not CLI)

		RETURN ZERO;
	} // Start()

#endif // __RPI_PICO__ / !__RPI_PICO__ (Start)

}; // class Emulation_t

#pragma endregion CORE

// =========================================================
// BOOT-UP
// =========================================================
#pragma region BOOT_UP

#ifndef __RPI_PICO__
// Reorder multi-file ROM sets (SI / PacMan / Ms PacMan) into
// the canonical slot order expected by each emulator.
// Used on all platforms.
void arrangeRoms(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM>& romsToRun)
{
	const size_t numberOfInputs =
		std::count_if(romsToRun.begin(), romsToRun.end(),
			[](const std::string& s) { return !s.empty(); });

	static constexpr std::array siExts = { ".e",".f",".g",".h" };
	static constexpr std::array pmExts = { "6e","6f","6h","6j","7f","4a","5e","5f","1m","3m" };
	static constexpr std::array mspmExts = { ".6e",".6f",".6h",".6j",".7f",".4a","5e","5f",".1m",".3m","u5","u6","u7" };

	std::array<std::string, 13> reordered{};
	reordered = romsToRun;

	if (numberOfInputs == siExts.size())
	{
		for (const auto& fname : romsToRun)
			for (size_t jj = RESET; jj < siExts.size(); ++jj)
				if (fname.ends_with(siExts[jj])) reordered[jj] = fname;
	}
	else if (numberOfInputs == pmExts.size())
	{
		for (const auto& fname : romsToRun)
			for (size_t jj = RESET; jj < pmExts.size(); ++jj)
				if (fname.ends_with(pmExts[jj])) reordered[jj] = fname;
	}
	else if (numberOfInputs == mspmExts.size())
	{
		for (const auto& fname : romsToRun)
			for (size_t jj = RESET; jj < mspmExts.size(); ++jj)
				if (fname.ends_with(mspmExts[jj])) reordered[jj] = fname;
	}

	romsToRun = reordered;
}
#endif // !__RPI_PICO__

inline EMULATION_ID getPlatformFromPath(const std::string& path)
{
#ifndef __RPI_PICO__

	// Desktop (keep your current behavior)
	std::filesystem::path filepath = path;

	auto it = _fileExtentionToEmulationPlatform.find(filepath.extension().string());
	if (it != _fileExtentionToEmulationPlatform.end())
		RETURN it->second;

	RETURN EMULATION_ID::DEFAULT_ID;

#else

	// -----------------------------
	// PICO IMPLEMENTATION (NO STL)
	// -----------------------------

	// Get extension (no allocation)
	const char* ext = nullptr;
	const char* cpath = path.c_str();

	for (const char* p = cpath; *p; ++p)
	{
		if (*p == '.')
			ext = p;
		else if (*p == '/' || *p == '\\')
			ext = nullptr;
	}

	if (!ext)
		ext = "";

	// Case-insensitive compare
	auto ieq = [](const char* a, const char* b) -> bool
		{
			while (*a && *b)
			{
				char ca = *a;
				char cb = *b;

				if (ca >= 'A' && ca <= 'Z') ca += ('a' - 'A');
				if (cb >= 'A' && cb <= 'Z') cb += ('a' - 'A');

				if (ca != cb)
					RETURN false;

				++a; ++b;
			}
			RETURN*a == *b;
		};

	// ---- lookup ----

	// CHIP8
	if (ieq(ext, ".ch8") || ieq(ext, ".c8") || ieq(ext, ".sc8") || ieq(ext, ".xo8"))
		RETURN EMULATION_ID::CHIP8_ID;

	// GAME OF LIFE
	if (ieq(ext, ".gol"))
		RETURN EMULATION_ID::GAME_OF_LIFE_ID;

	// SPACE INVADERS
	if (ieq(ext, ".e") || ieq(ext, ".f") || ieq(ext, ".g") || ieq(ext, ".h"))
		RETURN EMULATION_ID::SPACE_INVADERS_ID;

	// PACMAN
	if (ieq(ext, ".1m") || ieq(ext, ".3m") || ieq(ext, ".4a") ||
		ieq(ext, ".5e") || ieq(ext, ".5f") ||
		ieq(ext, ".6e") || ieq(ext, ".6f") ||
		ieq(ext, ".6h") || ieq(ext, ".6j") ||
		ieq(ext, ".7f") || ext[0] == '\0')
		RETURN EMULATION_ID::PACMAN_ID;

	// NES
	if (ieq(ext, ".nes"))
		RETURN EMULATION_ID::NES_ID;

	// GB/GBC
	if (ieq(ext, ".gb") || ieq(ext, ".gbc"))
		RETURN EMULATION_ID::GB_GBC_ID;

	// GBA
	if (ieq(ext, ".gba"))
		RETURN EMULATION_ID::GBA_ID;

	// TEST CPU
	if (ieq(ext, ".com") || ieq(ext, ".cim") ||
		ieq(ext, ".tap") || ieq(ext, ".bin"))
		RETURN EMULATION_ID::TEST_CPU_ID;

	RETURN EMULATION_ID::DEFAULT_ID;

#endif
}

// Detect ROM type from file count / extension and instantiate
// the correct emulator object.  Uses MasqConfig_t so it
// compiles with either boost::property_tree::ptree (desktop)
// or PicoConfig_t (Pico).
abstractEmulation_t* getType(int nFiles,
	std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom,
	MasqConfig_t& config,
	CheatEngine_t* ce = nullptr)
{
#ifndef __RPI_PICO__
	if (_numberOfRomsToEmulationPlatform.count(nFiles) == ZERO)
		RETURN new defaults_t;
#endif // !__RPI_PICO__

	if (nFiles == SINGLE_ROM_FILE) /* one file as input */
	{
		EMULATION_ID suspectedID = getPlatformFromPath(rom[ZERO]);

#if MASQ_ENABLE_CHIP8
		if (suspectedID == EMULATION_ID::CHIP8_ID)
			RETURN new chip8_t(rom, config);
#endif
#if MASQ_ENABLE_NES
		if (suspectedID == EMULATION_ID::NES_ID)
			RETURN new NES_t(ONE, rom, config, ce);
#endif
#if MASQ_ENABLE_GBC
		if (suspectedID == EMULATION_ID::GB_GBC_ID)
			RETURN new GBc_t(ONE, rom, config, ce);
#endif
#if MASQ_ENABLE_GBA
		if (suspectedID == EMULATION_ID::GBA_ID)
			RETURN new GBA_t(ONE, rom, config, ce);
#endif
#if MASQ_ENABLE_GOL
		if (suspectedID == EMULATION_ID::GAME_OF_LIFE_ID)
			RETURN new gameOfLife_t(config);
#endif
	}
#ifndef __RPI_PICO__
	else if (nFiles == TEST_ROM_FILE)  /* two files as input  */
	{
		EMULATION_ID suspectedID = getPlatformFromPath(rom[ONE]);

#if MASQ_ENABLE_SI
		if ((rom[ZERO] == "-8080SST") || (toUpper(rom[ZERO]) == "-I8080SST"))
		{
			INFO("Setting up the 8080 SST environment"); RETURN new spaceInvaders_t(nFiles, rom, config);
		}
		else if ((rom[ZERO] == "-8080") || (toUpper(rom[ZERO]) == "-I8080"))
		{
			INFO("Setting up the 8080 test environment"); RETURN new spaceInvaders_t(--nFiles, rom, config);
		}
#endif
#if MASQ_ENABLE_PACMAN
		if (toUpper(rom[ZERO]) == "-Z80SST")
		{
			INFO("Setting up the Z80 SST environment"); RETURN new pacMan_t(nFiles, rom, config);
		}
		else if (toUpper(rom[ZERO]) == "-Z80")
		{
			INFO("Setting up the Z80 test environment"); RETURN new pacMan_t(--nFiles, rom, config);
		}
#endif
#if MASQ_ENABLE_NES
		if ((toUpper(rom[ZERO]) == "-R6502SST") || (toUpper(rom[ZERO]) == "-N6502SST"))
		{
			INFO("Setting up the Ricoh2A03 / NES6502 SST environment"); RETURN new NES_t(nFiles, rom, config, ce);
		}
		else if (toUpper(rom[ZERO]) == "-6502")
		{
			INFO("Setting up the 6502 test environment"); RETURN new NES_t(--nFiles, rom, config, ce);
		}
#endif
#if MASQ_ENABLE_GBC
		if (toUpper(rom[ZERO]) == "-SM83SST")
		{
			INFO("Setting up the SM83 SST environment"); RETURN new GBc_t(nFiles, rom, config, ce);
		}
#endif
#if MASQ_ENABLE_GBA
		if (toUpper(rom[ZERO]) == "-ARM7TDMISST")
		{
			INFO("Setting up the ARM7TDMI SST environment"); RETURN new GBA_t(nFiles, rom, config);
		}
#endif
		INFO("ROM file is corrupted / Undefined Core");
	}
	else if (nFiles == COMPARE_OR_REPLAY_ROM_FILE) /* three files as input  */
	{
#if MASQ_ENABLE_GBA
		const std::string arg0 = toUpperCopy(rom[ZERO]);
		const std::string ext = toUpperCopy(std::filesystem::path(rom[ONE]).extension().string());

		const bool isReplay = (arg0 == "-R");
		const bool isCompare = (arg0 == "-C");

		if (!(isReplay || isCompare) || ext != ".GBA")
			RETURN new defaults_t;

		if (isReplay)
		{
			INFO("Replay Mode is not supported yet; Only Compare Mode is supported as of now");
			RETURN new defaults_t;
		}

		INFO("Setting up the GBA environment in compare mode");
		RETURN new GBA_t(nFiles, rom, config);
#else
		INFO("Not supported yet!");
#endif
	}
	else   /* four or more files as input  */
	{
		auto itPlatform = _numberOfRomsToEmulationPlatform.find(nFiles);
		if (itPlatform == _numberOfRomsToEmulationPlatform.end())
		{
			RETURN new defaults_t;
		}

		EMULATION_ID suspectedID = itPlatform->second;
		for (int count = 0; count < nFiles; ++count)
		{
			const std::filesystem::path filepath{ rom[count] };
			const std::string ext = filepath.extension().string();

			auto it = _fileExtentionToEmulationPlatform.find(ext);

			// Extension not supported
			if (it == _fileExtentionToEmulationPlatform.end())
			{
				RETURN new defaults_t;
			}

			// Mismatch in expected platform
			const EMULATION_ID fileID = it->second;
			if (fileID != suspectedID)
			{
				INFO("Some of the ROM files are corrupted");
				RETURN new defaults_t;
			}
		}
#if MASQ_ENABLE_SI
		if (suspectedID == EMULATION_ID::SPACE_INVADERS_ID)
		{
			arrangeRoms(rom);
			RETURN new spaceInvaders_t(nFiles, rom, config);
		}
#endif
#if MASQ_ENABLE_PACMAN
		if (suspectedID == EMULATION_ID::PACMAN_ID)
		{
			arrangeRoms(rom);
			RETURN new pacMan_t(nFiles, rom, config);
		}
#endif
	}

#endif // !__RPI_PICO__

	MASQ_UNUSED(ce);
	RETURN new defaults_t;
}

FLAG startMasquerade(int nFiles,
	std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom,
	ID bootType = BOOT)
{
	FLAG status = SUCCESS;

	INFO("Running Masquerade Emulator!");
#ifndef __RPI_PICO__
	INFO("If you experience screen tearing -> Please ensure V-Sync is enabled");
	INFO("Nvidia Control Panel > 3D Settings > Global settings > V-Sync -> ON");
	INFO("If you experience low/limited FPS -> Please ensure V-Sync is disabled");
	INFO("Nvidia Control Panel > 3D Settings > Global settings > V-Sync -> OFF");
#endif // !__RPI_PICO__

#ifndef __RPI_PICO__
	// Read runtime tunables from CONFIG.ini
	_XSCALE = config.get<DIM32>("mods._XSCALE", 1);
	_XFPS = config.get<DIM32>("mods._XFPS", 1);

	debugConfig._DEBUG_PPU_VIEWER_GUI = to_bool(config.get<std::string>("debug._DEBUG_PPU_VIEWER_GUI", "false"));
	debugConfig._DEBUG_PPU_VIEWER_GUI_TRIGGER = config.get<INC64>("debug._DEBUG_PPU_VIEWER_GUI_TRIGGER", 71);
	debugConfig._DEBUG_LOGGER_CLI = to_bool(config.get<std::string>("debug._DEBUG_LOGGER_CLI", "false"));
	debugConfig._DEBUG_LOGGER_CLI_MASK = config.get<MAP64>("debug._DEBUG_LOGGER_CLI_MASK", 0);

	currentEmuTheme = TO_UINT8(configToEmuThemes.at(config.get<std::string>("mods._EMULATOR_THEME", "DARK")));
	previousEmuTheme = currentEmuTheme;
	currEnVFilter = configToVFilters.at(config.get<std::string>("mods._VIDEO_EFFECTS", "LCD_FILTER"));
#endif // !__RPI_PICO__

	numberOfRomsSelected = nFiles;

#ifndef __RPI_PICO__
	CheatEngine_t* ce = new CheatEngine_t();
	ce->loadCheatNames(_CHEAT_SAVE_LOCATION);
#endif // !__RPI_PICO__

	abstractEmulation_t* toEmulate = getType(
		nFiles
		, rom
		, config
#ifndef __RPI_PICO__
		, ce
#endif // !__RPI_PICO__
	);
	Emulation_t Emulation(
		toEmulate
		, config
#ifndef __RPI_PICO__
		, ce
#endif // !__RPI_PICO__
	);
	Emulation.Start();

#ifndef __RPI_PICO__
	ce->saveCheatNames(_CHEAT_SAVE_LOCATION);
#endif // !__RPI_PICO__

	delete toEmulate; toEmulate = nullptr;

#ifndef __RPI_PICO__
	delete ce;        ce = nullptr;
#endif // !__RPI_PICO__

	MASQ_UNUSED(bootType);
	RETURN status;
}

void secondaryBootLoader(int argc,
	std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> argv,
	ID bootType = BOOT)
{
	if (argc == SINGLE_ROM_FILE)
	{
		INFO("ROM loaded: %s", argv[ZERO].c_str());
		INFO("ROM length: %zu", argv[ZERO].length());
		INFO("ROM contains spaces: %s", (argv[ZERO].find(' ') != std::string::npos) ? "YES" : "NO");

#ifndef __RPI_PICO__
		auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), argv[ZERO].c_str());
		if (it != recentlyOpenedList.end()) recentlyOpenedList.erase(it);
		recentlyOpenedList.push_front(argv[ZERO].c_str());
		if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE) recentlyOpenedList.pop_back();
#endif // !__RPI_PICO__
	}
	else if (argc > SINGLE_ROM_FILE)
	{
		INFO("ROM files loaded:");
		for (int count = ZERO; count < argc; count++)
			INFO("  %s", argv[count].c_str());
	}

	startMasquerade(argc, argv, bootType);
}

// =========================================================
// postPrimaryBootLoader: two complete versions
//
// Desktop  : reads CONFIG.ini, sets up all paths, runs the
//            BOOT / REBOOT loop.
// RPI Pico : ROM and settings are compiled-in; just call
//            secondaryBootLoader with the built-in ROM name.
//            Populate PICO_ROM_NAME via pico_rom.h.
// =========================================================

#ifdef __RPI_PICO__

void postPrimaryBootLoader()
{
	// --- Pico boot: ROM compiled-in via pico_rom.h --------
	// Include your generated header before masquerade.cpp:
	//   #include "pico_rom.h"     // defines PICO_ROM_NAME (the ROM label string)
	//   #include "pico_config.h"  // optional: #define overrides for emulator tunables

	SETBIT(ENABLE_LOGS, LOG_VERBOSITY);
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);

	createLUTForCRC();

	romsToRun.fill("");
	romsToRun[0] = PICO_ROM_NAME; // defined in pico_rom.h

	secondaryBootLoader(1, romsToRun);
}

#else // !__RPI_PICO__ — full desktop / Emscripten version

void postPrimaryBootLoader()
{
#ifdef __EMSCRIPTEN__
	listEmFiles();
#endif

	// --- Setup all the paths ---------------------------------
#ifndef __EMSCRIPTEN__
	_EXE_LOCATION = getexepath().parent_path().string();
	_CONFIG_LOCATION = (std::filesystem::path(_EXE_LOCATION) / "assets" / "CONFIG.ini").string();
#else
	_CONFIG_LOCATION = "assets/CONFIG.ini";
#endif

	INFO("Searching for %s", _CONFIG_LOCATION.c_str());

	// --- Read CONFIG.ini -------------------------------------
	try
	{
		boost::property_tree::ini_parser::read_ini(_CONFIG_LOCATION, config);
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		FATAL("Unable to read the CONFIG.ini");
	}

#ifndef __EMSCRIPTEN__
	_FONT_LOCATION = (std::filesystem::path(_EXE_LOCATION) / "assets" / "ui" / "fonts").string();
#else
	_FONT_LOCATION = "assets/ui/fonts/";
#endif

	// --- Normalize CONFIG.ini paths (Windows → POSIX safe) --
	auto normalizePath = [](std::string& p) { std::replace(p.begin(), p.end(), '\\', '/'); };

#ifndef __EMSCRIPTEN__
	std::string uiWorkingDir = config.get<std::string>("internal._ui_working_directory", "");
	if (uiWorkingDir.empty()) FATAL("Could not locate the UI working directory");
	normalizePath(uiWorkingDir);
	_IMGUI_LOCATION = (std::filesystem::path(uiWorkingDir) / "IMGUI.ini").string();
#else
	_IMGUI_LOCATION = "assets/ui/config/IMGUI.ini";
#endif

	// --- Ensure IMGUI directory exists -----------------------
	std::filesystem::create_directories(std::filesystem::path(_IMGUI_LOCATION).parent_path());

	// --- Create default IMGUI.ini if absent ------------------
	struct stat buffer;
	if (stat(_IMGUI_LOCATION.c_str(), &buffer) != ZERO)
	{
		std::ofstream out(_IMGUI_LOCATION);
		if (!out) throw std::runtime_error("Failed to open file: " + _IMGUI_LOCATION);
		INFO("Not able to find IMGUI.ini");
		INFO("Creating a masquerade default for now!");
		out << imguiDefaultIni;
		out.close();
	}

	INFO("Searching for %s", _IMGUI_LOCATION.c_str());
	createLUTForCRC();

#ifndef __EMSCRIPTEN__
	std::string workingDir = config.get<std::string>("internal._working_directory", "");
	if (workingDir.empty())
	{
		FATAL("Could not locate the working directory"); RETURN;
	}
	normalizePath(workingDir);
	recentlyOpenedListPath = workingDir;
#else
	recentlyOpenedListPath = "assets/internal";
#endif

	ifNoDirectoryThenCreate(recentlyOpenedListPath);

#ifndef __EMSCRIPTEN__
	recentlyOpenedListPath = (std::filesystem::path(recentlyOpenedListPath) / "recentlyOpenedListPath.dir").string();
	recentlyOpenedList = readDequeFromFile(recentlyOpenedListPath);
	_CHEAT_SAVE_LOCATION = (std::filesystem::path(workingDir) / "cheats.txt").string();
	normalizePath(_CHEAT_SAVE_LOCATION);
#else
	recentlyOpenedListPath += "/recentlyOpenedListPath.dir";
	_CHEAT_SAVE_LOCATION = "assets/internal/cheats.txt";
#endif

	// --- Boot / reboot loop ----------------------------------
	BYTE bootType = BOOT;

BOOT_AGAIN:
	romsToRun.fill("");

	if (bootType == BOOT)
	{
		for (int ii = ZERO; ii < gArgc - ONE; ii++)
			romsToRun[ii] = gArgv[ii + ONE];
		secondaryBootLoader(gArgc - ONE, romsToRun);
	}
	else
	{
		if (dynamicDragNDropAndMenuSelect.size() != ZERO)
		{
			const DIM8 numberOfInputs = static_cast<DIM8>(dynamicDragNDropAndMenuSelect.size());
			for (DIM8 ii = ZERO; ii < numberOfInputs; ii++)
				romsToRun[ii] = dynamicDragNDropAndMenuSelect[ii].c_str();
			arrangeRoms(romsToRun);
			dynamicDragNDropAndMenuSelect.clear();
			bootType = BOOT;
			secondaryBootLoader(numberOfInputs, romsToRun, bootType);
		}
		else if (rebootNeededOnMenuClick == YES)
		{
			rebootNeededOnMenuClick = NO;
			secondaryBootLoader(numberOfRomsSelected, romsToRun, bootType);
		}
	}

	if (dynamicDragNDropAndMenuSelect.size() != ZERO || rebootNeededOnMenuClick == YES)
	{
		bootType = REBOOT;
		goto BOOT_AGAIN;
	}
}

#endif // __RPI_PICO__ / !__RPI_PICO__ (postPrimaryBootLoader)

// =========================================================
// main
// =========================================================
int main(int argc, char* argv[])
{
	gArgc = argc;
	gArgv = argv;

	SETBIT(ENABLE_LOGS, LOG_VERBOSITY);
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);

#ifndef __RPI_PICO__
	// Handle --version / --help flags (desktop / CLI only)
	if (argc > ONE)
	{
		std::string arg = argv[1];
		if (arg == "--version" || arg == "-v")
		{
			LOG("%.4f\n", VERSION);
			fflush(stdout);
			RETURN 0;
		}
		else if (arg == "--help" || arg == "-h")
		{
			LOG("Masquerade Multi-System Emulator v%.4f\n", VERSION);
			LOG("\nUsage:\n");
			LOG("  masquerade [ROM_FILE...]       - Load and run ROM(s)\n");
			LOG("  masquerade --version           - Print version and exit\n");
			LOG("  masquerade --help              - Show this help message\n");
			RETURN ZERO;
		}
	}

#ifdef __EMSCRIPTEN__
	INFO("Masquerade running in emscripten mode");
	mountPersistentFS(postPrimaryBootLoader);
#else
	postPrimaryBootLoader();
#endif

#else // __RPI_PICO__
	if (PANEL_INIT() != SUCCESS)
	{
		FATAL("Failed to initialize panel"); RETURN -ONE;
	}
	PANEL_BACKEND_INIT();
	PANEL_BACKEND_CLEAR(PixelToRGB565_fast(WHITE));

	// Pico: no arg parsing, no OS, no filesystem.
	// postPrimaryBootLoader() boots directly from compiled-in ROM.
	postPrimaryBootLoader();

#endif // __RPI_PICO__

	RETURN ZERO;
}

#pragma endregion BOOT_UP