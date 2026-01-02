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

#pragma region INCLUDES
#include "helpers.h"
#include "abstractEmulation.h"
#include "chip8.h"
#include "defaults.h"
#include "gameOfLife.h"
#include "gba.h"
#include "gbc.h"
#include "nes.h"
#include "pacMan.h"
#include "resource.h"
#include "spaceInvaders.h"
#pragma endregion INCLUDES

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

#pragma region GLOBAL_INFRASTRUCTURE_DECLARATIONS
// Args
int gArgc = 0;
char** gArgv = nullptr;
// Below flag will be used for run-time emscripten/desktop checks
#ifdef __EMSCRIPTEN__
FLAG inEnscriptenMode = YES;
#else
FLAG inEnscriptenMode = NO;
#endif

// Theme
int currentEmuTheme = SE_THEME_LIGHT;
int previousEmuTheme = SE_THEME_LIGHT;
uint8_t customSEpalettes[FIVE * FOUR]; // Needed only for custom SE theme

// Init Screen
#ifdef __EMSCRIPTEN__
FLAG initScreen = YES;
#else
FLAG initScreen = NO;
#endif

// Logging
MAP64 ENABLE_LOGS = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000;
#ifdef __EMSCRIPTEN__
FLAG isAppLoggingEnabled = YES;
#else
FLAG isAppLoggingEnabled = YES;
#endif
ImGuiLogBuffer appLog;
FLAG isImGuiInitialized = NO;
std::vector<std::string> preImGuiLogBuffer;
std::mutex preImGuiLogMutex;

// Pre-init buffer
extern std::vector<std::string> preImGuiLogBuffer;
extern std::mutex preImGuiLogMutex;

// Configuration
static boost::property_tree::ptree config;

// For drag n drop support
std::vector<std::string> dynamicDragNDropAndMenuSelect;

// Recently opened items
const DIM8 _MAX_RECENTLY_USED_LIST_SIZE = EIGHT;
std::string recentlyOpenedListPath;
std::deque<std::string> recentlyOpenedList;

// Other UI support variables
FLAG quitOnMenuClick = NO;
FLAG rebootNeededOnMenuClick = NO;
FLAG saveContextOnReboot = NO;
FLAG startFromBoot = NO;

// For network support
uint32_t nEmulationInstanceID = ZERO;
FLAG bWaitingForConnection = YES;

// For profiler
static uint32_t profilerFrameRate = ZERO;	// ticks for profiler
static uint64_t functionID = ZERO;

// For emulator
std::string exeName = "masquerade.exe";

// For emulator support
std::string _BIOS_LOCATION;
std::string _CONFIG_LOCATION;
std::string _EXE_LOCATION;
std::string _IMGUI_LOCATION;
std::string _SAVE_LOCATION;
std::string _CHEAT_SAVE_LOCATION;
std::string _UI_INTERNAL_LOCATION;
std::string _FONT_LOCATION;

INC8 numberOfRomsSelected = RESET;
std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> romsToRun;

FLAG isBiosEnabled = NO;

unsigned long crcTable[256] = { 0 };

ROM ROM_TYPE = ROM::NO_ROM;

debugConfig_t debugConfig;
FLAG HitSkipPoint = NO;
FLAG HitBreakPoint = NO;

FLAG _ENABLE_AUDIO = NO;
FLAG _MUTE_AUDIO = NO;
FLAG _ENABLE_DISASSEMBLER = NO;
uint32_t _XSCALE = ONE;
FLAG _ENABLE_FRAME_LIMIT = NO;
uint32_t _XFPS = ONE;
FLAG _ENABLE_NETWORK = NO;
int32_t _NETWORK_TIMEOUT_LIMIT = ONE;
FLAG _ENABLE_QUICK_SAVE = YES;
FLAG _ENABLE_BESS_FORMAT = NO;
FLAG _ENABLE_REWIND = NO;
uint32_t _REWIND_BUFFER_SIZE = 5000;
int32_t _TEST_NUMBER = INVALID;

// Indicates that a absolute save state's output is loaded instead of a valid ROM
FLAG isAbsoluteLoad = NO;

// array to hold/process input samples within the FIR filer
double bufferForFIR[2048];

// OpenGL specific
float emuWindowX = 0.0f;
float emuWindowY = 0.0f;
float emuWindowMaxX = 0.0f;
float emuWindowMaxY = 0.0f;
uint32_t frame_buffer;
uint32_t masquerade_texture;
uint32_t shaderProgramBasic = 0;
uint32_t shaderProgramBlend = 0;
uint32_t fullscreenVAO;
uint32_t fullscreenVBO;
uint32_t FRAME_BUFFER_SCALE = 4;

float _ACTUAL_FPS = 0.0f;

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
#pragma endregion GLOBAL_INFRASTRUCTURE_DECLARATIONS

#pragma region GLOBAL_INFRASTRUCTURE_DEFINITION
#pragma region PROFILER
// periodic thread
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

// display profiled data
void displayNonPGEBasedFPS()
{
	if (debugConfig._DEBUG_PROFILER == true)
	{
		INFO("Non-PGE FPS: %d", profilerFrameRate);
		profilerFrameRate = ZERO;
	}
}

// profiler
void runProfiler()
{
	if (debugConfig._DEBUG_PROFILER == true)
	{
		++profilerFrameRate;
	}
}
#pragma endregion PROFILER
#pragma region SPECIAL_EFFECTS

VIDEO_FILTERS currEnVFilter = VIDEO_FILTERS::NEAREST_FILTER;
PALETTE_ID currEnGbPalette = PALETTE_ID::PALETTE_1;
PALETTE_ID currEnGbcPalette = PALETTE_ID::PALETTE_1;

#pragma endregion SPECIAL_EFFECTS
#pragma region IMGUI
#pragma endregion IMGUI
#pragma region EMSCRIPTEN
#ifdef __EMSCRIPTEN__
FLAG SavePersistentFSComplete = NO;
extern "C" void onSavePersistentFSComplete()
{
	SavePersistentFSComplete = YES;
}
FLAG ClearPersistentFSComplete = NO;
extern "C" void onClearPersistentFSComplete()
{
	ClearPersistentFSComplete = YES;
}
extern "C" {
EMSCRIPTEN_KEEPALIVE
void listEmFilesRecursive(const std::string& path = "/") {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        INFO("Cannot open directory: %s", path.c_str());
        RETURN;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;

        if (name == "." || name == "..") continue;

        std::string fullPath = path + (path.back() == '/' ? "" : "/") + name;
        INFO("%s", fullPath.c_str());

        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            listEmFilesRecursive(fullPath);
        }
    }

    closedir(dir);
}

EMSCRIPTEN_KEEPALIVE
void listEmFiles() {
    INFO("____________________________________");
    listEmFilesRecursive("/");
    INFO("____________________________________");
}

using Callback = void(*)();

// Define the JS function that accepts a function pointer and calls it after syncfs
EMSCRIPTEN_KEEPALIVE
EM_JS(void, mountPersistentFS, (Callback callback), {
    FS.mkdir('/persistent');
    FS.mount(IDBFS, {}, '/persistent');
    FS.syncfs(true, function (err) {
        if (err) {
            console.error("syncfs error", err);
        } else {
            console.log("syncfs complete");
            // Call the C function pointer passed as argument
            dynCall('v', callback);
        }
    });
});

EMSCRIPTEN_KEEPALIVE
EM_JS(void, savePersistentFS, (Callback callback), {
    FS.syncfs(false, function(err) {
        if (err) {
            console.error("Error saving persistent FS:", err);
        } else {
            console.log("Saved persistent FS to IndexedDB");
            dynCall('v', callback); // Call the C++ callback
        }
    });
});

EMSCRIPTEN_KEEPALIVE
EM_JS(void, clearPersistentFS, (Callback callback), {
    FS.readdir('/persistent').forEach(function (file) {
        if (file !== '.' && file !== '..') {
            try {
                FS.unlink('/persistent/' + file);
            } catch (e) {
                console.warn("Couldn't delete", file, e);
            }
        }
    });

    FS.syncfs(false, function(err) {
        if (err) {
            console.error("Error clearing persistent FS:", err);
        } else {
            console.log("Cleared persistent FS from IndexedDB");
            dynCall('v', callback); // Call the C++ callback
        }
    });
});


} // extern "C"
#endif
#pragma endregion EMSCRIPTEN
#pragma region STB
// Simple helper function to load an image into a OpenGL texture with common settings
FLAG LoadImageTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;

	unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	GL_CALL(glGenTextures(1, &image_texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, image_texture));

	// Setup filtering parameters for display
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	// Upload pixels into texture
	GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data));
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
FLAG LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
{
	FILE* f = fopen(file_name, "rb");
	if (f == NULL)
		return false;
	fseek(f, 0, SEEK_END);
	size_t file_size = (size_t)ftell(f);
	if (file_size == -1)
		return false;
	fseek(f, 0, SEEK_SET);
	void* file_data = IM_ALLOC(file_size);
	fread(file_data, 1, file_size, f);
	fclose(f);
	FLAG ret = LoadImageTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
	IM_FREE(file_data);
	return ret;
}
#pragma endregion STB
#pragma endregion GLOBAL_INFRASTRUCTURE_DEFINITION

#pragma region CORE

class Emulation_t
{
#pragma region INFRASTRUCTURE_DECLARATION
public:

#ifdef __EMSCRIPTEN__
	char const* golfilters = { ".gol,.GOL" };
	char const* chip8filters = { ".ch8,.CH8,.c8,.C8,.sc8,.SC8,.xo8,.XO8" };
	char const* spaceInvadersFilter = { ".e,.f,.g,.h,.zip" };
	char const* pacmanFilter = { ".1m,.3m,.4a,.5e,.5f,.6e,.6f,.6h,.6j,.7f,.zip" };
	char const* nesFilter = { ".nes,.NES" };
	char const* gbGbcFilter = { ".gb,.gbc,.GB,.GBC" };
	char const* gbaFilter = { ".gba,.GBA" };
#else
	nfdu8filteritem_t bootromfilters[1] = { "Boot ROM", "bin,BIN" };
	nfdu8filteritem_t golfilters[1] = { "Game Of Life States", "gol,GOL" };
	nfdu8filteritem_t chip8filters[1] = { "Chip8/S-Chip/XO-Chip/Modern-Chip8 ROMs", "ch8,CH8,c8,C8,sc8,SC8,xo8,XO8" };
	nfdu8filteritem_t spaceInvadersFilter[1] = { "SpaceInvaders ROMs", "e,f,g,h" };
	nfdu8filteritem_t spaceInvadersAudioFilter[1] = { "SpaceInvaders Audio", "wav,WAV" };
	nfdu8filteritem_t pacmanFilter[1] = { "Pacman/MsPacman ROMs", "1m,3m,4a,5e,5f,6e,6f,6h,6j,7f" };
	nfdu8filteritem_t nesFilter[1] = { "NES ROMs", "nes,NES" };
	nfdu8filteritem_t gbGbcFilter[1] = { "GB/GBC ROMs", "gb,gbc,GB,GBC" };
	nfdu8filteritem_t gbaFilter[1] = { "GBA ROMs", "gba,GBA" };
#endif

private:

	std::string appName = "Masquerade Emulator";

	abstractEmulation_t* current_instance = nullptr;

	float myFPS = (float)DEFAULT_FPS;

	FLAG bPostComplete = NO;
	FLAG bEmulationRun = NO;

	uint32_t currentFrame = ZERO;
	uint64_t current1hzFrame = ZERO;

private:

	CheatEngine_t* ceMAS;
#pragma endregion INFRASTRUCTURE_DECLARATION

#pragma region INFRASTRUCTURE_DEFINITION
public:

	Emulation_t(abstractEmulation_t* toEmulate, boost::property_tree::ptree& config, CheatEngine_t* ce)
	{
		bPostComplete = false;

		if (toEmulate == nullptr)
		{
			INFO("unsupported rom");
			throw std::runtime_error("unsupported rom");
		}
		else
		{
			current_instance = toEmulate;

			std::stringstream stream;
			stream << std::fixed << std::setprecision(4) << current_instance->getVersion();
			std::string version = stream.str();

			appName = "Masquerade Emulator | v" + version + std::string(" | ") + std::string(current_instance->getEmulatorName()) + std::string(" | ");
		}

		this->ceMAS = ce;
	}

	~Emulation_t()
	{
		;
	}
#pragma endregion INFRASTRUCTURE_DEFINITION

#pragma region EMULATION_DEFINITION
private:

	void loadConfig()
	{
		if (bPostComplete == false || (isCLI() == YES || ImGui::IsKeyPressed(ImGuiKey_Home)))
		{
			debugConfig._DEBUG_FPS = to_bool(config.get<std::string>("debug._DEBUG_FPS"));
			debugConfig._DEBUG_MEMORY = to_bool(config.get<std::string>("debug._DEBUG_MEMORY"));
			debugConfig._DEBUG_REGISTERS = to_bool(config.get<std::string>("debug._DEBUG_REGISTERS"));

			if (bPostComplete == false)
			{
				debugConfig._DEBUG_PROFILER = to_bool(config.get<std::string>("debug._DEBUG_PROFILER"));

				_ENABLE_AUDIO = to_bool(config.get<std::string>("mods._ENABLE_AUDIO"));
				_MUTE_AUDIO = to_bool(config.get<std::string>("mods._MUTE_AUDIO"));

				if (_MUTE_AUDIO == YES)
				{
					INFO("AUDIO is by default MUTED");
					INFO("Press M to toggle between MUTE/UNMUTE");
				}

				_ENABLE_FRAME_LIMIT = to_bool(config.get<std::string>("mods._ENABLE_FRAME_LIMIT"));
				_ENABLE_QUICK_SAVE = to_bool(config.get<std::string>("mods._ENABLE_QUICK_SAVE"));
				_ENABLE_BESS_FORMAT = to_bool(config.get<std::string>("mods._ENABLE_BESS_FORMAT"));
				_ENABLE_NETWORK = to_bool(config.get<std::string>("mods._ENABLE_NETWORK"));
				if (_ENABLE_NETWORK == YES)
				{
					_NETWORK_TIMEOUT_LIMIT = config.get<std::uint32_t>("mods._NETWORK_TIMEOUT_LIMIT");
				}
				else
				{
					_NETWORK_TIMEOUT_LIMIT = ONE;
				}
				_ENABLE_REWIND = to_bool(config.get<std::string>("mods._ENABLE_REWIND"));
				if (_ENABLE_REWIND == YES)
				{
					_REWIND_BUFFER_SIZE = config.get<std::uint32_t>("mods._REWIND_BUFFER_SIZE");
				}
			}
		}
	}

	void displayPGEBasedFPS(float fElapsedTime, uint8_t level)
	{
		FLAG initializationState[FPS_SLOTS] = {CLEAR};
		uint32_t frames[FPS_SLOTS] = { RESET };
		float accumulator[FPS_SLOTS] = { RESET };

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

		if (_ENABLE_REWIND == YES /*&& isBiosEnabled == NO*/)
		{
			current_instance->fillGamePlayStack();

			if (ImGui::IsKeyDown(ImGuiKey_R))
			{
				current_instance->rewindGamePlay();
			}
		}

		if (_ENABLE_QUICK_SAVE == YES /*&& isBiosEnabled == NO*/)
		{
			if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyDown(ImGuiKey_Tab) == false)
			{
				for (uint16_t ii = ((uint16_t)ImGuiKey_F1); ii <= ((uint16_t)ImGuiKey_F12); ii++)
				{
					if (ImGui::IsKeyPressed((ImGuiKey)ii))
					{
						current_instance->saveState((ii - ((uint16_t)ImGuiKey_F1)));
					}
				}
			}

			if (ImGui::IsKeyDown(ImGuiKey_LeftShift) == false && ImGui::IsKeyDown(ImGuiKey_Tab) == false)
			{
				for (uint16_t ii = ((uint16_t)ImGuiKey_F1); ii <= ((uint16_t)ImGuiKey_F12); ii++)
				{
					if (ImGui::IsKeyPressed((ImGuiKey)ii))
					{
						current_instance->loadState((ii - ((uint16_t)ImGuiKey_F1)));
					}
				}
			}
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
				olc::net::message<serialMsg> msg;
				gameSerialData heartBeat;
				heartBeat.ID = nEmulationInstanceID;
				heartBeat.heartBeat = YES;
				msg << heartBeat;
				msg.header.id = serialMsg::Game_HeartBeat;
				Send(msg);
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
		{
			stopLoopingForThisFrame = current_instance->runEmulationLoopAtFixedRate(currentFrame);
		}

		status = current_instance->runEmulationAtFixedRate(currentFrame);

		RETURN status;
	}

private:

	FLAG OnUserCreate()
	{
		// Called once at the start, so create things here

		FLAG status = SUCCESS;

		// Read the configuration.ini

		loadConfig();

		// Spawn the profiling thread if profiler was enabled

		if (debugConfig._DEBUG_PROFILER == true)
		{
			timerStart(displayNonPGEBasedFPS, 1000); // 1 second rate
		}

		// Provide the graphics and audio engine to emulator and initialize the emulator
		current_instance->setupTheCoreOfEmulation(nullptr, nullptr, nullptr);

		currentFrame = ZERO;

		myFPS = current_instance->getEmulationFPS() * _XFPS;

		INFO("Launching the %s Emulator", current_instance->getEmulatorName());

		if (debugConfig._DEBUG_MEMORY == true)
		{
			current_instance->dumpRom();
		}

		bEmulationRun = YES;
		bPostComplete = YES;

		RETURN status;
	}

	FLAG OnUserUpdate()
	{
		// called once per frame

		FLAG status = SUCCESS;
		
		if (isCLI() == NO)
		{

			if (quitOnMenuClick == YES)
			{
				quitOnMenuClick = NO;
				RETURN CLOSE;
			}

			if (dynamicDragNDropAndMenuSelect.size() != ZERO)
			{
				RETURN CLOSE;
			}

			if (rebootNeededOnMenuClick == YES)
			{
				if (saveContextOnReboot == YES)
				{
					// save current state here so that it can be loaded back again...
					current_instance->saveState(((0xFF + (uint8_t)ImGuiKey_F1) - ((uint8_t)ImGuiKey_F1)));
				}

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
			if (_ENABLE_NETWORK == YES)
			{
				if (IsConnected())
				{
					while (!Incoming().empty())
					{
						auto msg = Incoming().pop_front().msg;

						switch (msg.header.id)
						{

						case (serialMsg::Client_Accepted):
						{
							INFO("[Client] masquerade-Server Accepted Client - You're In!");
							olc::net::message<serialMsg> msg;
							msg.header.id = serialMsg::Client_RegisterWithServer;
							gameSerialData registration;
							registration.ID = nEmulationInstanceID;
							msg << registration;
							Send(msg);
							BREAK;
						}

						case (serialMsg::Client_AssignID):
						{
							gameSerialData getID;
							msg >> nEmulationInstanceID;
							INFO("[Client] Assigned Client ID = %u", nEmulationInstanceID);
							bWaitingForConnection = NO;
							BREAK;
						}

						}
					}
				}

				if (bWaitingForConnection)
				{
					Clear(olc::DARK_BLUE);
					DrawString({ 10, 10 }, "Waiting To", olc::WHITE);
					DrawString({ 10, 20 }, "Connect...", olc::WHITE);
					RETURN status;
				}
			}
#endif
			status = run();
		}

		// Update frame rate if it needs to change for next run
		// Currently needed for supporting Chip8 variants

		myFPS = current_instance->getEmulationFPS() * _XFPS;
		
		RETURN status;
	}

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

			// Write to CONFIG.ini
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
		usleep(ONE_SECOND * 1000);     // Convert ms to �s
#endif

		RETURN status;
	}

private:

#ifdef __EMSCRIPTEN__
	static void handle_upload_file(std::string const& filename, std::string const& mime_type, std::string_view buffer, void*)
	{
		if (buffer.empty())
		{
			INFO("Empty Buffer");
			RETURN;
		}
		
		std::vector<std::string> extracted_files;
		INC32 count = ONE;
		
		// Use original filename AS-IS to debug
		std::string persistent_path = "/persistent/" + filename;
		INFO("File uploaded: %s (type: %s), size: %zu bytes", persistent_path.c_str(), mime_type.c_str(), buffer.size());
		INFO("Filename length: %zu", filename.length());
		INFO("Has spaces: %s", (filename.find(' ') != std::string::npos) ? "YES" : "NO");
		
		// Debug: list what's in /persistent before write
		INFO("=== Contents of /persistent BEFORE write ===");
		try {
			for (const auto& entry : std::filesystem::directory_iterator("/persistent"))
			{
				INFO("  - %s", entry.path().filename().c_str());
			}
		} catch (const std::exception& e) {
			INFO("Error listing directory: %s", e.what());
		}
		
		std::ofstream ofs(persistent_path, std::ios::binary);
		if (!ofs)
		{
			INFO("CRITICAL: Failed to open file for writing: %s", persistent_path.c_str());
			INFO("Errno: %d", errno);
			RETURN;
		}
		
		INFO("File opened successfully for writing");
		ofs.write(buffer.data(), buffer.size());
		
		if (!ofs)
		{
			INFO("CRITICAL: Failed to write data to file: %s", persistent_path.c_str());
			INFO("Errno: %d", errno);
		}
		else
		{
			INFO("Data written successfully: %zu bytes", buffer.size());
		}
		
		ofs.close();
		INFO("File closed");
		
		// Debug: list what's in /persistent after write
		INFO("=== Contents of /persistent AFTER write ===");
		try {
			for (const auto& entry : std::filesystem::directory_iterator("/persistent"))
			{
				INFO("  - %s (size: %zu bytes)", entry.path().filename().c_str(), std::filesystem::file_size(entry.path()));
			}
		} catch (const std::exception& e) {
			INFO("Error listing directory: %s", e.what());
		}
		
		// Verify file exists
		std::ifstream test(persistent_path, std::ios::binary);
		if (test.good())
		{
			test.seekg(0, std::ios::end);
			size_t file_size = test.tellg();
			INFO("File verified on disk: %s (size: %zu bytes)", persistent_path.c_str(), file_size);
		}
		else
		{
			INFO("CRITICAL: File NOT readable after save: %s", persistent_path.c_str());
			INFO("Errno: %d", errno);
		}
		test.close();
		
		// Try to open it a different way
		INFO("=== Attempting alternative file access ===");
		struct stat file_stat;
		if (stat(persistent_path.c_str(), &file_stat) == 0)
		{
			INFO("stat() succeeded: file size = %ld bytes", file_stat.st_size);
		}
		else
		{
			INFO("stat() failed: %d", errno);
		}
		
		std::string ext = get_extension(persistent_path.c_str());
		if (strcmp(ext.c_str(), "zip") == 0)
		{
			INFO("It's a ZIP file!");
			count = extract_all_to_persistent_dir(persistent_path.c_str(), extracted_files);
		}
		else
		{
			extracted_files.emplace_back(filename);
		}
		
		INFO("Calling savePersistentFS...");
		savePersistentFS(onSavePersistentFSComplete);
		
		for (const auto& path : extracted_files)
		{
			auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), path);
			if (it != recentlyOpenedList.end())
			{
				recentlyOpenedList.erase(it);
			}
			recentlyOpenedList.push_front(path);
			if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE)
			{
				recentlyOpenedList.pop_back();
			}
			dynamicDragNDropAndMenuSelect.push_back(path);
		}
		
		INFO("=== handle_upload_file complete ===");
	}
#endif

	void romSelect(ROM type)
	{
#ifdef __EMSCRIPTEN__
		switch (type)
		{
		case ROM::CHIP8:
		{
			INFO("Chip8");
			emscripten_browser_file::upload(chip8filters, handle_upload_file);
			BREAK;
		}
		case ROM::SPACE_INVADERS:
		{
			INFO("Space Invaders");
			emscripten_browser_file::upload(spaceInvadersFilter, handle_upload_file);
			BREAK;
		}
		case ROM::PAC_MAN:
		case ROM::MS_PAC_MAN:
		{
			INFO("PacMan / Ms PacMan");
			emscripten_browser_file::upload(pacmanFilter, handle_upload_file);
			BREAK;
		}
		case ROM::NES:
		{
			INFO("Nintendo Entertainment System");
			emscripten_browser_file::upload(nesFilter, handle_upload_file);
			BREAK;
		}
		case ROM::GAME_BOY:
		case ROM::GAME_BOY_COLOR:
		{
			INFO("Game Boy / Game Boy Color");
			emscripten_browser_file::upload(gbGbcFilter, handle_upload_file);
			BREAK;
		}
		case ROM::GAME_BOY_ADVANCE:
		{
			INFO("Game Boy Advance");
			emscripten_browser_file::upload(gbaFilter, handle_upload_file);
			BREAK;
		}
		default:
		{
			FATAL("Unsupported ROM type : %u", TO_UINT(type));
			RETURN;
		}
		}
#else
		nfdu8char_t* outPath;
		const nfdpathset_t* outPaths;
		nfdu8filteritem_t filters[1];
		nfdopendialogu8args_t args = { 0 };

		switch (type)
		{
		case ROM::GAME_OF_LIFE:
		{
			filters->name = golfilters->name;
			filters->spec = golfilters->spec;
			BREAK;
		}
		case ROM::CHIP8:
		{
			filters->name = chip8filters->name;
			filters->spec = chip8filters->spec;
			BREAK;
		}
		case ROM::SPACE_INVADERS:
		{
			filters->name = spaceInvadersFilter->name;
			filters->spec = spaceInvadersFilter->spec;
			BREAK;
		}
		case ROM::PAC_MAN:
		case ROM::MS_PAC_MAN:
		{
			filters->name = pacmanFilter->name;
			filters->spec = pacmanFilter->spec;
			BREAK;
		}
		case ROM::NES:
		{
			filters->name = nesFilter->name;
			filters->spec = nesFilter->spec;
			BREAK;
		}
		case ROM::GAME_BOY:
		case ROM::GAME_BOY_COLOR:
		{
			filters->name = gbGbcFilter->name;
			filters->spec = gbGbcFilter->spec;
			BREAK;
		}
		case ROM::GAME_BOY_ADVANCE:
		{
			filters->name = gbaFilter->name;
			filters->spec = gbaFilter->spec;
			BREAK;
		}
		default:
		{
			FATAL("Unsupported ROM type : %u", TO_UINT(type));
			RETURN;
		}
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

				nfdpathsetsize_t i;
				for (i = 0; i < numPaths; ++i)
				{
					nfdchar_t* path;
					NFD_PathSet_GetPath(outPaths, i, &path);
					INFO("Opening %i: %s", (int)i, path);
					dynamicDragNDropAndMenuSelect.push_back(std::string(path));
					// remember to free the pathset path with NFD_PathSet_FreePath (not NFD_FreePath!)
					NFD_PathSet_FreePath(path);
				}

				// remember to free the pathset memory (since NFD_OKAY is returned)
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
				// Check if the element was found
				if (it != recentlyOpenedList.end())
				{
					// Element found, delete it
					recentlyOpenedList.erase(it);
				}
				recentlyOpenedList.push_front(std::string(outPath));
				if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE)
				{
					recentlyOpenedList.pop_back();
				}
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
#endif
	}

	void bootRomSelect(ROM type)
	{
#ifdef __EMSCRIPTEN__
#else
		nfdu8char_t* outPath = nullptr;
		const nfdpathset_t* outPaths = nullptr;
		nfdu8filteritem_t filters[1];
		nfdopendialogu8args_t args = { 0 };

		filters->name = bootromfilters->name;
		filters->spec = bootromfilters->spec;

		args.filterList = filters;
		args.filterCount = ONE;
		nfdresult_t result;

		result = NFD_OpenDialogU8_With(&outPath, &args);

		if (result == NFD_OKAY)
		{
			INFO("Load : %s", outPath);
			auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), std::string(outPath));

			if (type == ROM::GAME_BOY)
			{
				config.put("gb-gbc._dmg_bios_location", std::string(outPath));
			}
			else if (type == ROM::GAME_BOY_COLOR)
			{
				config.put("gb-gbc._cgb_bios_location", std::string(outPath));
			}
			else if (type == ROM::GAME_BOY_ADVANCE)
			{
				config.put("gba._gba_bios_location", std::string(outPath));
			}

			// Write to CONFIG.ini
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
#endif
	}

	void loadSIAudioWAV(std::string type)
	{
#ifdef __EMSCRIPTEN__
#else
		nfdu8char_t* outPath = nullptr;
		const nfdpathset_t* outPaths = nullptr;
		nfdu8filteritem_t filters[1];
		nfdopendialogu8args_t args = { 0 };

		filters->name = spaceInvadersAudioFilter->name;
		filters->spec = spaceInvadersAudioFilter->spec;

		args.filterList = filters;
		args.filterCount = ONE;
		nfdresult_t result;

		result = NFD_OpenDialogU8_With(&outPath, &args);

		if (result == NFD_OKAY)
		{
			INFO("Load : %s", outPath);
			auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), std::string(outPath));

			std::string option = "spaceinvaders._" + type;

			config.put(option, std::string(outPath));

			// Write to CONFIG.ini
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
#endif
	}

private:

#if defined(_WIN32) && (ENABLED_IMGUI_DEFAULT_THEME == NO)

// ---- MinGW / older Windows SDK compatibility ----
#ifndef DWMWA_BORDER_COLOR
    #define DWMWA_BORDER_COLOR   34
#endif

#ifndef DWMWA_CAPTION_COLOR
    #define DWMWA_CAPTION_COLOR  35
#endif

	void SetDWMTitlebarColor(SDL_Window* window, ImVec4 color)
	{
		struct Vec3
		{
			float x, y, z;
		};

		auto ToCOLORREF = [](const ImVec4& color) -> COLORREF
			{
				const auto r = static_cast<int>(color.x * 255.0f + 0.5f);
				const auto g = static_cast<int>(color.y * 255.0f + 0.5f);
				const auto b = static_cast<int>(color.z * 255.0f + 0.5f);
				RETURN RGB(r, g, b); // RGB macro gives BGR COLORREF
			};

		const char* windowTitle = SDL_GetWindowTitle(window);
		HWND hwnd = FindWindowA(NULL, windowTitle);
		if (!hwnd)
		{
			SDL_Log("Failed to find HWND for window title: %s", windowTitle);
			RETURN;
		}

		COLORREF WinColor = ToCOLORREF(color);
		DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &WinColor, sizeof(WinColor));
		DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &WinColor, sizeof(WinColor));
	}
#endif

public:

	int setupThemes()
	{
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
		colors[ImGuiCol_TextDisabled] = ImVec4( 0.6f, 0.6f, 0.6f, 0.5f );
		colors[ImGuiCol_WindowBg] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
		colors[ImGuiCol_ChildBg] = ImVec4( 0.19f, 0.19f, 0.19f, 0.40f );
		colors[ImGuiCol_PopupBg] = ImVec4( 0.22f, 0.22f, 0.22f, 0.92f );
		colors[ImGuiCol_Border] = ImVec4( 0.1f, 0.1f, 0.1f, 1.0f );
		colors[ImGuiCol_BorderShadow] = ImVec4( 0.00f, 0.00f, 0.00f, 0.24f );
		colors[ImGuiCol_FrameBg] = ImVec4( 0.2f, 0.2f, 0.2f, 0.9f );
		colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.1f, 0.1f, 0.1f, 1.0f );
		colors[ImGuiCol_FrameBgActive] = ImVec4( 0.29f, 0.29f, 0.29f, 1.00f );
		colors[ImGuiCol_TitleBg] = ImVec4( 0.00f, 0.00f, 0.00f, 1.00f );
		colors[ImGuiCol_TitleBgActive] = ImVec4( 0.06f, 0.06f, 0.06f, 1.00f );
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 0.00f, 0.00f, 0.00f, 1.00f );
		colors[ImGuiCol_MenuBarBg] = ImVec4( 0.10f, 0.10f, 0.10f, 1.00f );
		colors[ImGuiCol_ScrollbarBg] = ImVec4( 0.05f, 0.05f, 0.05f, 0.54f );
		colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.34f, 0.34f, 0.34f, 0.54f );
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.40f, 0.40f, 0.40f, 0.54f );
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 0.56f, 0.56f, 0.56f, 0.54f );
		colors[ImGuiCol_CheckMark] = ImVec4( 0.33f, 0.67f, 0.86f, 1.00f );
		colors[ImGuiCol_SliderGrab] = ImVec4( 0.34f, 0.34f, 0.34f, 0.8f );
		colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.56f, 0.56f, 0.56f, 0.8f );
		colors[ImGuiCol_Button] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
		colors[ImGuiCol_ButtonHovered] = ImVec4( 0.19f, 0.19f, 0.19f, 0.54f );
		colors[ImGuiCol_ButtonActive] = ImVec4( 0.4f, 0.4f, 0.4f, 1.00f );
		colors[ImGuiCol_Header] = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
		colors[ImGuiCol_HeaderHovered] = ImVec4( 0.00f, 0.00f, 0.00f, 0.36f );
		colors[ImGuiCol_HeaderActive] = ImVec4( 0.20f, 0.22f, 0.23f, 0.33f );
		colors[ImGuiCol_Separator] = ImVec4( 0.28f, 0.28f, 0.28f, 0.9f );
		colors[ImGuiCol_SeparatorHovered] = ImVec4( 0.44f, 0.44f, 0.44f, 0.29f );
		colors[ImGuiCol_SeparatorActive] = ImVec4( 0.40f, 0.44f, 0.47f, 1.00f );
		colors[ImGuiCol_ResizeGrip] = ImVec4( 0.28f, 0.28f, 0.28f, 0.29f );
		colors[ImGuiCol_ResizeGripHovered] = ImVec4( 0.44f, 0.44f, 0.44f, 0.29f );
		colors[ImGuiCol_ResizeGripActive] = ImVec4( 0.40f, 0.44f, 0.47f, 1.00f );
		colors[ImGuiCol_Tab] = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
		colors[ImGuiCol_TabHovered] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
		colors[ImGuiCol_TabActive] = ImVec4( 0.20f, 0.20f, 0.20f, 0.36f );
		colors[ImGuiCol_TabUnfocused] = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
		colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4( 0.33f, 0.67f, 0.86f, 1.00f );
		colors[ImGuiCol_PlotLinesHovered] = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
		colors[ImGuiCol_PlotHistogram] = ImVec4( 0.33f, 0.67f, 0.86f, 1.00f );
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
		colors[ImGuiCol_TableHeaderBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
		colors[ImGuiCol_TableBorderStrong] = ImVec4( 0.00f, 0.00f, 0.00f, 0.52f );
		colors[ImGuiCol_TableBorderLight] = ImVec4( 0.28f, 0.28f, 0.28f, 0.29f );
		colors[ImGuiCol_TableRowBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
		colors[ImGuiCol_TableRowBgAlt] = ImVec4( 1.00f, 1.00f, 1.00f, 0.06f );
		colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.20f, 0.22f, 0.23f, 1.00f );
		colors[ImGuiCol_DragDropTarget] = ImVec4( 0.33f, 0.67f, 0.86f, 1.00f );
		colors[ImGuiCol_NavHighlight] = ImVec4( 1.00f, 0.00f, 0.00f, 1.00f );
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4( 1.00f, 0.00f, 0.00f, 0.70f );
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4( 1.00f, 0.00f, 0.00f, 0.20f );
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4( 1.00f, 0.00f, 0.00f, 0.35f );

		if (currentEmuTheme == THEME_CUSTOM)
		{
			uint8_t* palette = customSEpalettes;
			//Base color
			if (palette[0 * 4 + 3])
			{
				float r = static_cast<float>(palette[0 * 4 + 0]) / 255.0f;
				float g = static_cast<float>(palette[0 * 4 + 1]) / 255.0f;
				float b = static_cast<float>(palette[0 * 4 + 2]) / 255.0f;
				float a = static_cast<float>(palette[0 * 4 + 3]) / 255.0f;
				colors[ImGuiCol_WindowBg] = ImVec4( r, g, b, a );
				colors[ImGuiCol_ChildBg] = ImVec4( r, g, b, a );
				colors[ImGuiCol_PopupBg] = ImVec4( r, g, b, a );
				colors[ImGuiCol_MenuBarBg] = ImVec4( r, g, b, a );
			}
			//Text Color
			if (palette[1 * 4 + 3])
			{
				float r = static_cast<float>(palette[1 * 4 + 0]) / 255.0f;
				float g = static_cast<float>(palette[1 * 4 + 1]) / 255.0f;
				float b = static_cast<float>(palette[1 * 4 + 2]) / 255.0f;
				float a = static_cast<float>(palette[1 * 4 + 3]) / 255.0f;
				colors[ImGuiCol_PlotLinesHovered] =
					colors[ImGuiCol_PlotHistogramHovered] =
					colors[ImGuiCol_Text] = ImVec4( r,g,b,a );
				colors[ImGuiCol_TextDisabled] = ImVec4( r,g,b,a * 0.4f );
				colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( r,g,b,a * 0.6f );
				colors[ImGuiCol_SliderGrabActive] = colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( r,g,b,a * 0.8f );
			}
			//Second Color
			if (palette[2 * 4 + 3])
			{
				float r = static_cast<float>(palette[2 * 4 + 0]) / 255.0f;
				float g = static_cast<float>(palette[2 * 4 + 1]) / 255.0f;
				float b = static_cast<float>(palette[2 * 4 + 2]) / 255.0f;
				float a = static_cast<float>(palette[2 * 4 + 3]) / 255.0f;
				colors[ImGuiCol_FrameBg] = ImVec4( r,g,b,a * 0.5f );
				colors[ImGuiCol_ScrollbarBg] = ImVec4( r,g,b,a );
				colors[ImGuiCol_Button] = ImVec4( r, g, b, a );
				colors[ImGuiCol_ButtonHovered] = ImVec4( r,g,b, a * 0.54f );
				colors[ImGuiCol_ButtonActive] = ImVec4( r * 2,g * 2,b * 2, a * 1.00f );
			}
			//Tab/Header
			if (palette[3 * 4 + 3])
			{
				float r = static_cast<float>(palette[3 * 4 + 0]) / 255.0f;
				float g = static_cast<float>(palette[3 * 4 + 1]) / 255.0f;
				float b = static_cast<float>(palette[3 * 4 + 2]) / 255.0f;
				float a = static_cast<float>(palette[3 * 4 + 3]) / 255.0f;
				colors[ImGuiCol_TitleBg] =
					colors[ImGuiCol_TitleBgActive] =
					colors[ImGuiCol_TitleBgCollapsed] =
					colors[ImGuiCol_TableHeaderBg] =
					colors[ImGuiCol_TableBorderStrong] = ImVec4( r,g,b,a );

				colors[ImGuiCol_SliderGrab] = colors[ImGuiCol_ScrollbarGrab] = ImVec4( r,g,b,a );

				colors[ImGuiCol_FrameBgHovered] = ImVec4( r,g,b,a * 0.75f );
				colors[ImGuiCol_FrameBgActive] = ImVec4( r,g,b,a );

				colors[ImGuiCol_Tab] =
					colors[ImGuiCol_Header] = ImVec4( r,g,b,a * 0.5f );
				colors[ImGuiCol_TabHovered] =
					colors[ImGuiCol_HeaderHovered] = ImVec4( r,g,b,a * 0.75f );
				colors[ImGuiCol_TabActive] =
					colors[ImGuiCol_HeaderActive] = ImVec4( r,g,b,a );

			}
			//Accent color (checkmark, bar/line graph)
			if (palette[4 * 4 + 3])
			{
				float r = static_cast<float>(palette[4 * 4 + 0]) / 255.0f;
				float g = static_cast<float>(palette[4 * 4 + 1]) / 255.0f;
				float b = static_cast<float>(palette[4 * 4 + 2]) / 255.0f;
				float a = static_cast<float>(palette[4 * 4 + 3]) / 255.0f;
				colors[ImGuiCol_PlotLines] =
					colors[ImGuiCol_PlotHistogram] =
					colors[ImGuiCol_CheckMark] = ImVec4( r,g,b,a );
			}
		}

		if (currentEmuTheme == SE_THEME_LIGHT)
		{
			int invert_list[] = {
			  ImGuiCol_Text,
			  ImGuiCol_TextDisabled,
			  ImGuiCol_WindowBg,
			  ImGuiCol_ChildBg,
			  ImGuiCol_PopupBg,
			  ImGuiCol_Border,
			  ImGuiCol_BorderShadow,
			  ImGuiCol_FrameBg,
			  ImGuiCol_FrameBgHovered,
			  ImGuiCol_FrameBgActive,
			  ImGuiCol_TitleBg,
			  ImGuiCol_TitleBgActive,
			  ImGuiCol_TitleBgCollapsed,
			  ImGuiCol_MenuBarBg,
			  ImGuiCol_ScrollbarBg,
			  ImGuiCol_ScrollbarGrab,
			  ImGuiCol_ScrollbarGrabHovered,
			  ImGuiCol_ScrollbarGrabActive,
			  ImGuiCol_SliderGrab,
			  ImGuiCol_SliderGrabActive,
			  ImGuiCol_Button,
			  ImGuiCol_ButtonHovered,
			  ImGuiCol_ButtonActive,
			  ImGuiCol_Header,
			  ImGuiCol_HeaderHovered,
			  ImGuiCol_HeaderActive,
			  ImGuiCol_Separator,
			  ImGuiCol_SeparatorHovered,
			  ImGuiCol_SeparatorActive,
			  ImGuiCol_ResizeGrip,
			  ImGuiCol_ResizeGripHovered,
			  ImGuiCol_ResizeGripActive,
			  ImGuiCol_Tab,
			  ImGuiCol_TabHovered,
			  ImGuiCol_TabActive,
			  ImGuiCol_TabUnfocused,
			  ImGuiCol_TabUnfocusedActive,
			  ImGuiCol_TableHeaderBg,
			  ImGuiCol_TableBorderStrong,
			  ImGuiCol_TableBorderLight,
			  ImGuiCol_TableRowBg,
			  ImGuiCol_TableRowBgAlt,
			  ImGuiCol_TextSelectedBg,
			  ImGuiCol_DragDropTarget,
			  ImGuiCol_NavHighlight,
			  ImGuiCol_NavWindowingHighlight,
			  ImGuiCol_NavWindowingDimBg,
			  ImGuiCol_ModalWindowDimBg,
			};
			for (int i = 0; i < sizeof(invert_list) / sizeof(invert_list[0]); ++i)
			{
				colors[invert_list[i]].x = 1.0f - colors[invert_list[i]].x;
				colors[invert_list[i]].y = 1.0f - colors[invert_list[i]].y;
				colors[invert_list[i]].z = 1.0f - colors[invert_list[i]].z;
			}
		}

		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowPadding = ImVec2( 8.00f, 8.00f );
		style->FramePadding = ImVec2( 5.00f, 2.00f );
		//style->CellPadding                       = ImVec2(6.00f, 6.00f);
		style->ItemSpacing = ImVec2( 6.00f, 6.00f );
		//style->ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
		style->TouchExtraPadding = ImVec2( 2.00f, 4.00f );
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
		style->ButtonTextAlign = ImVec2( 0.5,0.5 );

		if (currentEmuTheme == SE_THEME_BLACK)
		{
			int black_list[] = {
			  ImGuiCol_WindowBg,
			  ImGuiCol_ChildBg,
			  ImGuiCol_PopupBg,
			  //ImGuiCol_FrameBg,
			  ImGuiCol_TitleBg,
			  ImGuiCol_MenuBarBg,
			  //ImGuiCol_ScrollbarBg,
			};
			colors[ImGuiCol_Button] = ImVec4( 0.18f, 0.18f, 0.18f, 1.00f );
			colors[ImGuiCol_FrameBg] = ImVec4( 0.15f, 0.15f, 0.15f, 0.9f );
			colors[ImGuiCol_ScrollbarBg] = ImVec4( 0.1f, 0.1f, 0.1f, 0.6f );

			for (int i = 0; i < sizeof(black_list) / sizeof(black_list[0]); ++i)
			{
				colors[black_list[i]].x = 0;
				colors[black_list[i]].y = 0;
				colors[black_list[i]].z = 0;
			}

		}

		RETURN SUCCESS;
	}

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
			// Setup SDL
			if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
			{
				FATAL("Error: SDL_Init(): %s", SDL_GetError());
				RETURN - ONE;
			}

			// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
		// GL ES 2.0 + GLSL 100 (WebGL 1.0)
			const char* glsl_version = "#version 100";
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
		// GL ES 3.0 + GLSL 300 es (WebGL 2.0)
			const char* glsl_version = "#version 300 es";
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
		// GL 3.2 Core + GLSL 150
			const char* glsl_version = "#version 150";
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
		// GL 3.0 + GLSL 130
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

			auto newX = config.get<std::int16_t>("mods._X") - WINDOW_PADDING;
			auto newY = config.get<std::int16_t>("mods._Y") - WINDOW_PADDING - WINDOW_PADDING;
			if (newX > 0 && newY > 0)
			{
				current_instance->setScreenWidth(newX);
				current_instance->setScreenHeight(newY);
			}

			// Create window with graphics context
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
			Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
			SDL_Window* window = SDL_CreateWindow(
				"Masquerade Emulator"
				, (current_instance->getScreenWidth() * current_instance->getPixelWidth()) + WINDOW_PADDING							// WINDOW_PADDING added to account for vertical borders
				, (current_instance->getScreenHeight() * current_instance->getPixelHeight()) + WINDOW_PADDING + WINDOW_PADDING		// WINDOW_PADDING + WINDOW_PADDING added to account for horizontal borders
				, window_flags);
			if (window == nullptr)
			{
				FATAL("Error: SDL_CreateWindow(): %s", SDL_GetError());
				RETURN - ONE;
			}
#ifndef __EMSCRIPTEN__

			// Resize SDL window if requested
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
				FATAL("Error: SDL_GL_CreateContext(): %s", SDL_GetError());
				RETURN - ONE;
			}

			// This is needed (especially for emscripten builds)
			SDL_StartTextInput(window);

			SDL_GL_MakeCurrent(window, gl_context);
			SDL_GL_SetSwapInterval(1); // Enable vsync
#ifndef __EMSCRIPTEN__
			SDL_ShowWindow(window);
#endif

			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
			//io.ConfigViewportsNoAutoMerge = true;
			//io.ConfigViewportsNoTaskBarIcon = true;

			io.ConfigWindowsMoveFromTitleBarOnly = true;

			// Setup Dear ImGui style
#if (ENABLED_IMGUI_DEFAULT_THEME == YES)
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsLight();
#else
			setupThemes();
#endif

			// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			// Setup Platform/Renderer backends
			ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
			ImGui_ImplOpenGL3_Init(glsl_version);

			// Load Fonts
			// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
			// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
			// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
			// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
			// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
			// - Read 'docs/FONTS.md' for more instructions and details.
			// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
			// - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
			//io.Fonts->AddFontDefault();
			io.Fonts->AddFontFromFileTTF((std::filesystem::path(_FONT_LOCATION) / "segoeui.ttf").string().c_str(),16.0f);
			//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
			//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
			//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
			//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
			//IM_ASSERT(font != nullptr);

#ifndef __EMSCRIPTEN__
		// Native File System
			if (NFD_Init() != NFD_OKAY)
			{
				FATAL("Error: NFD_Init(): %s", NFD_GetError());
				RETURN - ONE;
			}

			TODO("NFD_GetNativeWindowFromSDLWindow needs to be called when NFD's support for SDL3 is available");
			// NFD_GetNativeWindowFromSDLWindow(sdlWindow /* SDL_Window* */, &args.parentWindow);
#endif

		// Our state
			FLAG show_demo_window = true;
			FLAG show_another_window = false;
			ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

#if defined(_WIN32) && (ENABLED_IMGUI_DEFAULT_THEME == NO)
			SetDWMTitlebarColor(window, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
#endif

#if !defined(__EMSCRIPTEN__) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
            // Setup function pointers
            gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
#endif

			// Intention is to set the necessary params for "Emulation Window"
			ImGui::SetNextWindowSize(ImVec2((float)current_instance->getScreenWidth(), (float)current_instance->getScreenHeight()));

			std::string emuWindow = "Emulation Window (" + std::string(current_instance->getEmulatorName()) + ")";

			FlushEarlyLogsToImGui();

			if (RUN_IMGUI_DEMO == NO)
			{
				// Other Masquerade specific initializations
				OnUserCreate();
			}

			// Load Image or Gif if any
#ifndef __EMSCRIPTEN__
			std::string imLoc = config.get<std::string>("internal._ui_sprites_directory") + "\\BG1.png";
#else
			std::string imLoc = "assets/ui/sprites/BG1.png";
#endif
			int clickWinWidth = 0;
			int clickWinHeight = 0;
			uint32_t clickWinTexture = 0;
			FLAG clickWinStatus = LoadTextureFromFile(imLoc.c_str(), &clickWinTexture, &clickWinWidth, &clickWinHeight);
			IM_ASSERT(clickWinStatus);

			// Main loop
			ID64 tickAtStart = RESET;
			FLAG done = NO;
			io.IniFilename = _IMGUI_LOCATION.c_str();
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
						if (RUN_IMGUI_DEMO == YES)
						{
							// Poll and handle events (inputs, window resize, etc.)
							// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
							// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
							// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
							// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
							SDL_Event event;
							while (SDL_PollEvent(&event))
							{
								ImGui_ImplSDL3_ProcessEvent(&event);
								if (event.type == SDL_EVENT_QUIT)
									done = true;
								if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
									done = true;
							}
							if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
							{
								SDL_Delay(10);
								RETURN;
							}

							// Start the Dear ImGui frame
							ImGui_ImplOpenGL3_NewFrame();
							ImGui_ImplSDL3_NewFrame();
							ImGui::NewFrame();

							// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
							if (show_demo_window)
								ImGui::ShowDemoWindow(&show_demo_window);

							// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
							{
								static float f = 0.0f;
								static int counter = 0;

								ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

								ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
								ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
								ImGui::Checkbox("Another Window", &show_another_window);

								ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
								ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

								if (ImGui::Button("Button"))                            // Buttons RETURN true when clicked (most widgets RETURN true when edited/activated)
									counter++;
								ImGui::SameLine();
								ImGui::Text("counter = %d", counter);

								ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
								ImGui::End();
							}

							// 3. Show another simple window.
							if (show_another_window)
							{
								ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our FLAG variable (the window will have a closing button that will clear the FLAG when clicked)
								ImGui::Text("Hello from another window!");
								if (ImGui::Button("Close Me"))
									show_another_window = false;
								ImGui::End();
							}

							// Rendering
							ImGui::Render();
							glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
							glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
							glClear(GL_COLOR_BUFFER_BIT);
							ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

							// Update and Render additional Platform Windows
							// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
							//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
							if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
							{
								SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
								SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
								ImGui::UpdatePlatformWindows();
								ImGui::RenderPlatformWindowsDefault();
								SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
							}

							SDL_GL_SwapWindow(window);
						}
						else
						{
							// Windows to be displayed
							static FLAG showEmuWin = YES;
							static FLAG showUpdWin = NO;
							static FLAG showAboutWin = NO;
							static FLAG showLoggerWin = NO;
							static FLAG showCheatWin = NO;
							static FLAG maintainAspectRatio = config.get<FLAG>("mods._MAINTAIN_ASPECT_RATIO");

							// Get tick
							tickAtStart = SDL_GetTicksNS();

#if (ENABLED_IMGUI_DEFAULT_THEME == NO)
							// Update theme if changed!
							if (currentEmuTheme != previousEmuTheme)
							{
								setupThemes();
								previousEmuTheme = currentEmuTheme;
#if _WIN32
								SetDWMTitlebarColor(window, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
#endif
							}
#endif

							// Update window title
							SDL_SetWindowTitle(window, (appName + to_string_with_precision(io.Framerate, ONE)).c_str());

							// Save the actual current FPS
							_ACTUAL_FPS = io.Framerate;

							// Poll and handle events (inputs, window resize, etc.)
							// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
							// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
							// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
							// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
							SDL_Event event;
							while (SDL_PollEvent(&event))
							{
								ImGui_ImplSDL3_ProcessEvent(&event);
								if (event.type == SDL_EVENT_QUIT)
									done = true;
								if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
									done = true;
								if (event.type == SDL_EVENT_DROP_FILE)
								{
									auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), std::string(event.drop.data));
									// Check if the element was found
									if (it != recentlyOpenedList.end())
									{
										// Element found, delete it
										recentlyOpenedList.erase(it);
									}
									recentlyOpenedList.push_front(event.drop.data);
									if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE)
									{
										recentlyOpenedList.pop_back();
									}
									dynamicDragNDropAndMenuSelect.push_back(event.drop.data);
								}
							}
							if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
							{
								SDL_Delay(10);
								RETURN;
							}

							// Start the Dear ImGui frame
							ImGui_ImplOpenGL3_NewFrame();
							ImGui_ImplSDL3_NewFrame();
							ImGui::NewFrame();

							ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_AutoHideTabBar; // Doesn't hide the triangle in the top
							if (!showUpdWin && !showAboutWin && !showLoggerWin)
							{
								dockSpaceFlags |= ImGuiDockNodeFlags_NoTabBar; // Hides the triangle in the top but causes other artifacts when we have combination of windows which needs hiding and which doesnt need hiding
							}
							ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockSpaceFlags);

							if (initScreen == NO)
							{
								if (ImGui::BeginMainMenuBar())
								{
									if (ImGui::BeginMenu("File"))
									{
										//ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(184, 134, 11, 255));
										if (ImGui::BeginMenu("Open Game Of Life"))
										{
											//ImGui::PopStyleColor();
											if (ImGui::MenuItem("Create New"))
											{
												dynamicDragNDropAndMenuSelect.push_back("dummy.gol");
#ifdef __EMSCRIPTEN__
												SavePersistentFSComplete = YES;
#endif
											}
											if (ImGui::MenuItem("Open GOL"))
											{
												romSelect(ROM::GAME_OF_LIFE);
											}
											ImGui::EndMenu();
										}
										else
										{
											//ImGui::PopStyleColor();
										}
										ImGui::Separator();
										if (ImGui::BeginMenu("Open Chip-8/S-Chip/XO-Chip/Modern-Chip8"))
										{
											auto menuOption = [&](const char* label, const char* key)
												{
													FLAG isTicked = to_bool(config.get<std::string>(key)) == YES;
													if (ImGui::MenuItem(label, nullptr, isTicked))
													{
														// Clear all
														config.put("chip8._chip8", CLEAR);
														config.put("chip8._schip_modern", CLEAR);
														config.put("chip8._schip_legacy", CLEAR);
														config.put("chip8._xo_chip", CLEAR);
														config.put("chip8._modern_chip8", CLEAR);

														// Set selected
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
										if (ImGui::MenuItem("Open Space Invaders"))
										{
											romSelect(ROM::SPACE_INVADERS);
										}
										if (ImGui::BeginMenu("Open Pacman/Ms Pacman"))
										{
											if (ImGui::MenuItem("Open Midway/Namco Pacman"))
											{
												romSelect(ROM::PAC_MAN);
											}
											if (ImGui::MenuItem("Open Ms Pacman"))
											{
												romSelect(ROM::MS_PAC_MAN);
											}
											ImGui::EndMenu();
										}
										ImGui::Separator();
										if (ImGui::MenuItem("Open NES"))
										{
											romSelect(ROM::NES);
										}
										if (ImGui::BeginMenu("Open GB/GBC"))
										{
											if (ImGui::MenuItem("Open GB"))
											{
												romSelect(ROM::GAME_BOY);
											}
											if (ImGui::MenuItem("Open GBC"))
											{
												romSelect(ROM::GAME_BOY_COLOR);
											}
											ImGui::EndMenu();
										}
										if (ImGui::MenuItem("Open GBA"))
										{
											romSelect(ROM::GAME_BOY_ADVANCE);
										}
										ImGui::Separator();
										if (ImGui::BeginMenu("Open Recent"))
										{
											if (recentlyOpenedList.empty())
											{
												ImGui::MenuItem("Nothing to display");			
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
														// Check if the element was found
														if (it != recentlyOpenedList.end())
														{
															// Element found, delete it
															recentlyOpenedList.erase(it);
														}

														recentlyOpenedList.push_front(copyOfElement);
														if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE)
														{
															recentlyOpenedList.pop_back();
														}

														std::string prefix = "/persistent/";
														if (copyOfElement.rfind(prefix, 0) == 0) // rfind == 0 means it starts with it
														{
															copyOfElement.erase(0, prefix.length());
														}
														dynamicDragNDropAndMenuSelect.push_back(copyOfElement);
#ifdef __EMSCRIPTEN__
														SavePersistentFSComplete = YES;
#endif
													}
												}

												ImGui::Separator();
												//ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(184, 134, 11, 255));
												if (ImGui::MenuItem("Clear the recently opened item history"))
												{
													recentlyOpenedList.clear();
												}
												//ImGui::PopStyleColor();
											}
											ImGui::EndMenu();
										}
										if (ImGui::MenuItem("Reset"))
										{
											rebootNeededOnMenuClick = YES;
										}
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
									if (ImGui::BeginMenu("Emulation"))
									{
										if (ImGui::BeginMenu("Bios"))
										{
											if (ImGui::BeginMenu("GB Bios"))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gb-gbc._use_dmg_bios"));
												if (ImGui::MenuItem("Load##GB Bios", NULL, NO, inEnscriptenMode == NO))
												{
													bootRomSelect(ROM::GAME_BOY);
												}
												if (ImGui::MenuItem("Enable##GB Bios", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("gb-gbc._use_dmg_bios", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("GBC Bios"))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gb-gbc._use_cgb_bios"));
												if (ImGui::MenuItem("Load##GBC Bios", NULL, NO, inEnscriptenMode == NO))
												{
													bootRomSelect(ROM::GAME_BOY_COLOR);
												}
												if (ImGui::MenuItem("Enable##GBC Bios", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("gb-gbc._use_cgb_bios", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("GBA Bios"))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gba._use_gba_bios"));
												if (ImGui::MenuItem("Load##GBA Bios", NULL, NO, inEnscriptenMode == NO))
												{
													bootRomSelect(ROM::GAME_BOY_ADVANCE);
												}
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
											float volume = (current_instance->getEmulationVolume());
											if (current_instance->getEmulationID() == EMULATION_ID::DEFAULT_ID)
											{
												ImGui::BeginDisabled();
												ImGui::SliderFloat("Volume", &volume, 0, 1);
												if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
												{
													ImGui::SetTooltip("This setting is available only post game selection");
												}
												ImGui::EndDisabled();
											}
											else
											{
												ImGui::SliderFloat("Volume", &volume, 0, 1);
											}
											current_instance->setEmulationVolume((float)volume);
											ImGui::Separator();
											if (ImGui::BeginMenu("Space Invaders"))
											{
												if (ImGui::BeginMenu("Load Space Invaders WAV", inEnscriptenMode == NO))
												{
													const char* items[][2] = {
														{ "UFO",              "UFO" },
														{ "Shot",             "Shot" },
														{ "Player Dies",      "PlayerDies" },
														{ "Invader Dies",     "InvaderDies" },
														{ "Fleet Move 1",     "FleetMovement1" },
														{ "Fleet Move 2",     "FleetMovement2" },
														{ "Fleet Move 3",     "FleetMovement3" },
														{ "Fleet Move 4",     "FleetMovement4" },
														{ "UFO Hit",          "UFOHit" }
													};

													// Find max label length
													size_t maxLabelLen = 0;
													for (auto& item : items)
													{
														maxLabelLen = std::max(maxLabelLen, strlen(item[0]));
													}

													for (auto& item : items)
													{
														char label[64];
														snprintf(label, sizeof(label), "%-*s", (int)maxLabelLen, item[0]);

														if (ImGui::MenuItem(label))
														{
															loadSIAudioWAV(item[1]);
														}
													}

													ImGui::EndMenu();
												}
												ImGui::EndMenu();
											}
											ImGui::EndMenu();
										}
										if (ImGui::BeginMenu("Video"))
										{
											if (ImGui::MenuItem("Aspect Ratio", NULL, maintainAspectRatio))
											{
												maintainAspectRatio = (maintainAspectRatio == YES ? NO : YES);
												config.put<FLAG>("mods._MAINTAIN_ASPECT_RATIO", maintainAspectRatio);
												boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
											}
											if (ImGui::BeginMenu("Scaling / Shaders"))
											{
												int selection = TO_UINT(currEnVFilter);
												ImGui::RadioButton("Nearest", &selection, TO_UINT(VIDEO_FILTERS::NEAREST_FILTER));
#ifdef __EMSCRIPTEN__
												ImGui::BeginDisabled();
#endif
												ImGui::RadioButton("Bilinear", &selection, TO_UINT(VIDEO_FILTERS::BILINEAR_FILTER));
												ImGui::RadioButton("LCD", &selection, TO_UINT(VIDEO_FILTERS::LCD_FILTER));
#ifdef __EMSCRIPTEN__
												ImGui::EndDisabled();
#endif
												ImGui::BeginDisabled();
												ImGui::RadioButton("CRT", &selection, TO_UINT(VIDEO_FILTERS::CRT_FILTER));
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
											if (ImGui::BeginMenu("GB"))
											{
												if (ImGui::BeginMenu("GB Color Palette"))
												{
													int selection = TO_UINT(currEnGbPalette);
													ImGui::RadioButton("GearBoy", &selection, TO_UINT(PALETTE_ID::PALETTE_1));
													ImGui::RadioButton("Black/White", &selection, TO_UINT(PALETTE_ID::PALETTE_2));
													ImGui::RadioButton("SameBoy", &selection, TO_UINT(PALETTE_ID::PALETTE_3));
													ImGui::RadioButton("BGB", &selection, TO_UINT(PALETTE_ID::PALETTE_4));
													if (currEnGbPalette != (PALETTE_ID)selection)
													{
														config.put<std::string>("gb-gbc._force_gb_palette", gbPaletteIDToConfig.at((PALETTE_ID)selection));
														boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
													}
													currEnGbPalette = (PALETTE_ID)selection;
													ImGui::EndMenu();
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("GBC"))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gb-gbc._enable_cgb_color_correction"));
												if (ImGui::MenuItem("GBC Color Correction", "C", isTicked))
												{
													isTicked = !isTicked;
													config.put("gb-gbc._enable_cgb_color_correction", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
													currEnGbcPalette = ((currEnGbcPalette == PALETTE_ID::PALETTE_1) ? PALETTE_ID::PALETTE_2 : PALETTE_ID::PALETTE_1);
												}
												ImGui::EndMenu();
											}
											ImGui::EndMenu();
										}
										if (ImGui::BeginMenu("Other Settings"))
										{
											//ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(184, 134, 11, 255));
											if (ImGui::BeginMenu("Game Of Life"))
											{
												//ImGui::PopStyleColor();
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
												if (ImGui::IsItemHovered())
												{
													ImGui::SetTooltip("Needs restart to take effect");
												}
												ImGui::EndMenu();
											}
											else
											{
												//ImGui::PopStyleColor();
											}
											if (ImGui::BeginMenu("Chip8 Family"))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("chip8._enable_c8_db"));
												if (ImGui::MenuItem("Enable ROM Database", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("chip8._enable_c8_db", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("Space Invaders"))
											{
												static FLAG DIP[FOUR] =
												{
													to_bool(config.get<std::string>("spaceinvaders._DIP3"))
													, to_bool(config.get<std::string>("spaceinvaders._DIP5"))
													, to_bool(config.get<std::string>("spaceinvaders._DIP6"))
													, to_bool(config.get<std::string>("spaceinvaders._DIP7"))
												};
												static const STATE8 DIPLUT[4] = { 3, 5, 6, 7 };
												FLAG wasClicked = NO;
												for (INC8 ii = RESET; ii < sizeof(DIP); ii++)
												{
													if (ImGui::Checkbox(("DIP" + std::to_string(DIPLUT[ii])).c_str(), &DIP[ii]))
													{
														wasClicked = YES;
													}
												}
												if (wasClicked)
												{
													config.put("spaceinvaders._DIP3", DIP[ZERO]);
													config.put("spaceinvaders._DIP5", DIP[ONE]);
													config.put("spaceinvaders._DIP6", DIP[TWO]);
													config.put("spaceinvaders._DIP7", DIP[THREE]);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											if (ImGui::BeginMenu("GB"))
											{
												static FLAG isTicked = to_bool(config.get<std::string>("gb-gbc._force_gbc_for_gb"));
												if (ImGui::MenuItem("CGB Mode", NULL, isTicked))
												{
													isTicked = !isTicked;
													config.put("gb-gbc._force_gbc_for_gb", isTicked);
													boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
												}
												ImGui::EndMenu();
											}
											ImGui::EndMenu();
										}
										ImGui::EndMenu();
									}
									if (ImGui::BeginMenu("Option"))
									{
										if (current_instance->getEmulationID() == EMULATION_ID::DEFAULT_ID)
										{
											ImGui::MenuItem("Debugger", NULL, NO, DISABLED);
											if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
											{
												ImGui::SetTooltip("This setting is available only post game selection");
											}
										}
										else
										{
											if (ImGui::BeginMenu("Debugger"))
											{
												if (ImGui::BeginMenu("CPU", NO))
												{
													ImGui::EndMenu();
												}
												if (ImGui::BeginMenu("PPU", NO))
												{
													ImGui::EndMenu();
												}
												if (ImGui::BeginMenu("APU", NO))
												{
													ImGui::EndMenu();
												}
												ImGui::EndMenu();
											}
										}
										ImGui::Separator();
										if (ImGui::BeginMenu("Cheats"))
										{
											if (ImGui::MenuItem(showCheatWin == NO ? ("Open Cheat Hub") : ("Close Cheat Hub")))
											{
												showCheatWin = (showCheatWin == NO) ? YES : NO;
											}
											ImGui::EndMenu();
										}
										ImGui::Separator();
										if (ImGui::BeginMenu("Load/Save"))
										{
											static FLAG isQLSTicked = to_bool(config.get<std::string>("mods._ENABLE_QUICK_SAVE"));
											if (ImGui::MenuItem("Enable Save States", NULL, isQLSTicked))
											{
												isQLSTicked = !isQLSTicked;
												_ENABLE_QUICK_SAVE = isQLSTicked;
												config.put("mods._ENABLE_QUICK_SAVE", isQLSTicked);
												boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
											}
											static FLAG isBESSTicked = to_bool(config.get<std::string>("mods._ENABLE_BESS_FORMAT"));
											if (ImGui::MenuItem("Enable BESS format", NULL, isBESSTicked && _ENABLE_QUICK_SAVE))
											{
												isBESSTicked = !isBESSTicked;
												_ENABLE_BESS_FORMAT = isBESSTicked && _ENABLE_QUICK_SAVE;
												config.put("mods._ENABLE_BESS_FORMAT", isBESSTicked);
												boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
											}
											if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
											{
												ImGui::SetTooltip("This setting is available only if Save States are enabled");
											}
											ImGui::EndMenu();
										}
										ImGui::Separator();
										ImGui::MenuItem("Logger Configuration", NULL, NO, NO);
										if (ImGui::MenuItem("Logger Console", NULL, showLoggerWin))
										{
											showLoggerWin = (showLoggerWin == NO) ? YES : NO;
										}
										ImGui::Separator();
										if (ImGui::BeginMenu("Theme"))
										{
											ImGui::Combo("##Theme", &currentEmuTheme, "Dark\0Light\0Black\0", 0);
											if (currentEmuTheme != previousEmuTheme)
											{
												config.put<std::string>("mods._EMULATOR_THEME", emuThemesToConfig.at((EMULATOR_THEME)currentEmuTheme));
												boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, config);
											}
											ImGui::EndMenu();
										}
										ImGui::EndMenu();
									}
									if (ImGui::BeginMenu("Help"))
									{
										if (ImGui::MenuItem("Masquerade-OTA for updates!"))
										{
#ifndef __EMSCRIPTEN__
#if DEACTIVATED
											ota.checkForUpdates(config);
#endif
#endif
											showUpdWin = YES;
										}
										ImGui::Separator();
										if (ImGui::MenuItem("About"))
										{
											showAboutWin = YES;
										}
										ImGui::EndMenu();
									}
									ImGui::EndMainMenuBar();
								}

								ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.5f * ImGui::GetStyle().ScrollbarSize);
								ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
								ImGui::Begin(emuWindow.c_str(), &showEmuWin,
									ImGuiWindowFlags_NoTitleBar
									| ImGuiWindowFlags_NoMove
									| ImGuiWindowFlags_NoCollapse
									| ImGuiWindowFlags_NoScrollbar
									| ImGuiWindowFlags_NoBackground
									| ImGuiWindowFlags_AlwaysAutoResize);

								if (maintainAspectRatio == NO || current_instance->getEmulationID() == EMULATION_ID::DEFAULT_ID)
								{
									if (current_instance->getEmulationID() == EMULATION_ID::DEFAULT_ID)
									{
										{
											const char* header = "Load Recently Played Game";

											// To remove spacing between this title bar and next item
											ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));

											// Some vertical space before this title bar starts
											float headerTopSpace = 10.0f;

											// Vertical adjustment when title bar is forced because of other windows
											if (showUpdWin || showAboutWin || showLoggerWin)
											{
												headerTopSpace *= 2.0f;
											}
											else
											{
												ImGui::Dummy(ImVec2(0.0f, headerTopSpace));
											}

											// Get window draw list and positions
											ImDrawList* drawList = ImGui::GetWindowDrawList();
											ImVec2 winPos = ImGui::GetWindowPos();
											ImVec2 shiftedWinPos = winPos;
											shiftedWinPos.y += headerTopSpace;
											ImVec2 winSize = ImGui::GetWindowSize();
											ImVec2 winMax = ImVec2(shiftedWinPos.x + winSize.x, shiftedWinPos.y + winSize.y);

											// Draw window background (optional � usually already drawn by ImGui)
											ImU32 bgColor = ImGui::GetColorU32(ImGuiCol_ChildBg);
											// We use winPos instead of shiftedWinPos as we want this to color the new space created after the shift happened
											drawList->AddRectFilled(winPos, winMax, bgColor);

											// Draw header bar background
											ImVec2 textSize = ImGui::CalcTextSize(header);
											float headerHeight = textSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
											ImVec2 headerMax = ImVec2(shiftedWinPos.x + winSize.x, shiftedWinPos.y + headerHeight);

											ImU32 headerBg = ImGui::GetColorU32(ImGuiCol_TitleBgActive);
											drawList->AddRectFilled(shiftedWinPos, headerMax, headerBg);

											// Draw text (centered vertically in the header)
											ImVec2 textPos = ImVec2(shiftedWinPos.x + ImGui::GetStyle().FramePadding.x * 2.0f, shiftedWinPos.y + ImGui::GetStyle().FramePadding.y);
											drawList->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), header);

											// Space below the header so content doesn't overlap
											ImGui::Dummy(ImVec2(0.0f, headerHeight));

											// To remove spacing between this title bar and next item
											ImGui::PopStyleVar();
										}

										ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_ChildBg]);
										ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
										ImVec2 childSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
										ImGui::BeginChild("##Child1", childSize);

										std::filesystem::path filePath;

										if (recentlyOpenedList.empty())
										{
											ImGui::SetCursorPosX(ImGui::GetStyle().FramePadding.x * 2.0f);
											ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
											ImGui::TextDisabled("Nothing to display...");
										}

										int ii = RESET;
										static FLAG isHoveringOverPath[_MAX_RECENTLY_USED_LIST_SIZE] = { NO };
										const FLAG displayX = NO;
										for (auto it = recentlyOpenedList.begin(); it != recentlyOpenedList.end(); )
										{
											FLAG done = NO;

											const std::filesystem::path filePath = it->c_str();

											// Detect ROM type
											const char* romType = "Unknown";
											std::string ext = filePath.extension().string();
											if (ext == ".gba")      romType = "GBA";
											else if (ext == ".gbc") romType = "GBC";
											else if (ext == ".gb")  romType = "GB";
											else if (ext == ".nes") romType = "NES";
											else if (ext == ".ch8") romType = "CH8";

											if (strcmp(romType, "Unknown") == 0)
											{
												it++;
												CONTINUE;
											}

											ImGui::PushID(ii);

											// Start row
											ImVec2 rowStartPos = ImGui::GetCursorPos();

											// Handle hovering for game type
											FLAG hoverOverGameType = NO;

											float leftOffset = 40.0f;

											// Implement game type
											{
												// RomType label (offset downward for visual alignment)
												ImVec2 romTypeSize = ImGui::CalcTextSize(romType);
												// Setup Invisible button where the game type will be displayed
												ImVec2 gameTypeInvButton = { 0.0f, 0.0f };
												gameTypeInvButton.x = leftOffset;
												gameTypeInvButton.y = romTypeSize.y * 2.5f;
												ImGui::InvisibleButton(("##gameType_hover_zone_" + std::to_string(ii)).c_str(), gameTypeInvButton);
												// If hovering over game type block
												if (ImGui::IsItemHovered())
												{
													hoverOverGameType = YES;
													if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
													{
														INFO("%s", filePath.string().c_str());
														dynamicDragNDropAndMenuSelect.push_back(filePath.string());
														done = YES;
													}
													else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
													{
														it = recentlyOpenedList.erase(it);
														ImGui::PopID();
														CONTINUE;
													}
												}
												// Reset the position
												float romTypeOffsetY = romTypeSize.y * 0.5f + ImGui::GetStyle().FramePadding.y;
												ImGui::SetCursorPosX(rowStartPos.x + ImGui::GetStyle().FramePadding.x * 2.0f);
												ImGui::SetCursorPosY(rowStartPos.y + romTypeOffsetY);
												// Display rom type
												ImGui::TextDisabled("%s", romType);
											}

											// File name selectable
											ImGui::SetCursorPos(rowStartPos);
											ImVec2 xSize = ImGui::CalcTextSize("X");
											float deleteButtonWidth = xSize.x /*+ ImGui::GetStyle().FramePadding.x * 2.0f*/;
											ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x + leftOffset);

											float selectableWidth = ImGui::GetContentRegionAvail().x - deleteButtonWidth - 10.0f;

											// NOTE: Below line completely hides the delete button 'X'
											if (displayX == NO)
											{
												selectableWidth = ImGui::GetContentRegionAvail().x;
											}

											// Implement game name
											{
												ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
												ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
												// We will remove 'seleted/hovered color' here for this selectable and manually implement below
												if (ImGui::Selectable((filePath.stem().string() + "##file").c_str(), isHoveringOverPath[ii], 0, ImVec2(selectableWidth, 0)))
												{
													INFO("%s", filePath.string().c_str());
													dynamicDragNDropAndMenuSelect.push_back(filePath.string());
													done = YES;
												}
												ImGui::PopStyleColor(2);

												if (ImGui::IsItemHovered() || hoverOverGameType == YES)
												{
													isHoveringOverPath[ii] = YES;
													if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
													{
														it = recentlyOpenedList.erase(it);
														ImGui::PopID();
														CONTINUE;
													}
												}
												else
												{
													isHoveringOverPath[ii] = NO;
												}
											}

											ImVec2 posSelectable = ImGui::GetItemRectMin();
											ImVec2 sizeSelectable = ImGui::GetItemRectSize(); // (width, height)
											ImVec4 colorSelectable = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);

											// Implement delete button 'X'
											if (displayX == YES)
											{
												// Right-aligned "X" delete (appears on hover)
												ImGui::SameLine();
												ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x + leftOffset + selectableWidth + 5.0f);

												// Reserve the space for the button
												ImVec2 buttonPos = ImGui::GetCursorScreenPos();
												if (ImGui::InvisibleButton(("##hover_zone_" + std::to_string(ii)).c_str(), xSize))
												{
													it = recentlyOpenedList.erase(it);
													ImGui::PopID();
													CONTINUE;
												}

												// Draw the red "X" only if hovered
												if (ImGui::IsItemHovered())
												{
													ImGui::SetCursorScreenPos(buttonPos);
													ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
													ImGui::TextUnformatted("X");
													ImGui::PopStyleColor();
												}
												else
												{
													ImGui::SetCursorScreenPos(buttonPos);
													ImGui::TextDisabled("X");
												}
											}

											// Implement the path 
											{
												// Full file path label
												ImGui::SetCursorPosX(ImGui::GetCursorStartPos().x + leftOffset);
												// Setup Invisible button where the path will be displayed
												ImVec2 pathInvbuttonPos = ImGui::GetCursorScreenPos();
												ImVec2 pathInvButton = ImGui::CalcTextSize(filePath.string().c_str());
												pathInvButton.x = ImGui::GetContentRegionAvail().x;
												ImGui::InvisibleButton(("##path_hover_zone_" + std::to_string(ii)).c_str(), pathInvButton);
												// If hovering over the path, highlight
												if (ImGui::IsItemHovered())
												{
													isHoveringOverPath[ii] = YES;
													// If the path is selected, this also should be considered
													if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
													{
														INFO("%s", filePath.string().c_str());
														dynamicDragNDropAndMenuSelect.push_back(filePath.string());
														done = YES;
													}
													else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
													{
														it = recentlyOpenedList.erase(it);
														ImGui::PopID();
														CONTINUE;
													}
												}
												// Reset position for path
												ImGui::SetCursorScreenPos(pathInvbuttonPos);
												// Display path
												ImGui::TextDisabled("%s", filePath.string().c_str());
											}

											// TODO: Display size at the horizontal end of path

											// Implement selected/hovered color for game type and path
											if (isHoveringOverPath[ii] == YES)
											{
												ImVec2 pos = ImGui::GetItemRectMin();
												float padding_x = ImGui::GetStyle().FramePadding.x;
												float padding_y = ImGui::GetStyle().FramePadding.y;
												ImVec2 bgMin = ImVec2(pos.x - (padding_x * 0.5f), pos.y - (padding_y * 1.5f));
												ImVec2 bgMax = ImVec2(bgMin.x + sizeSelectable.x, bgMin.y + sizeSelectable.y + padding_y);
												// Draw background using selectable highlight color
												ImGui::GetWindowDrawList()->AddRectFilled(bgMin, bgMax, ImGui::GetColorU32(colorSelectable));

												// Add highlighting for game type as well
												bgMin = ImVec2(posSelectable.x - leftOffset, posSelectable.y);
												bgMax = ImVec2(bgMin.x + leftOffset, bgMin.y + sizeSelectable.y * 2.0f + padding_y);
												// Draw background using selectable highlight color
												ImGui::GetWindowDrawList()->AddRectFilled(bgMin, bgMax, ImGui::GetColorU32(colorSelectable));
											}

											ImGui::Separator();
											ImGui::PopID();
											++ii;
											++it;

											if (done)
											{
												BREAK;
											}
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
									ImVec2 avail_size = ImGui::GetContentRegionAvail();

									// Emulator framebuffer dimensions
									float fbWidth = static_cast<float>(current_instance->getScreenWidth() * FRAME_BUFFER_SCALE);
									float fbHeight = static_cast<float>(current_instance->getScreenHeight() * FRAME_BUFFER_SCALE);
									float fbAspect = static_cast<float>(fbWidth / fbHeight);	// ratio = x / y

									ImVec2 imageSize;

									// Compute size that maintains aspect ratio
									if (avail_size.x / avail_size.y > fbAspect)
									{
										// Too wide, fit by height
										imageSize.y = avail_size.y;
										imageSize.x = imageSize.y * fbAspect; // x = (y * ratio)
									}
									else
									{
										// Too tall, fit by width
										imageSize.x = avail_size.x;
										imageSize.y = imageSize.x / fbAspect;	// y = (x * ratio)
									}

									// Center the image inside the window
									ImVec2 cursor_pos = ImGui::GetCursorPos();
									ImVec2 offset = {
										(avail_size.x - imageSize.x) * 0.5f,
										(avail_size.y - imageSize.y) * 0.5f
									};
									ImGui::SetCursorPos(ImVec2(cursor_pos.x + offset.x, cursor_pos.y + offset.y));

									// Draw the image
									ImGui::Image((ImTextureID)(uintptr_t)masquerade_texture, imageSize);
								}

								// Save window position/size (unchanged)
								emuWindowX = ImGui::GetWindowPos().x;
								emuWindowY = ImGui::GetWindowPos().y;
								emuWindowMaxX = ImGui::GetWindowWidth();
								emuWindowMaxY = ImGui::GetWindowHeight();

								ImGui::End();
								ImGui::PopStyleVar(); // For window padding

								if (showUpdWin == YES)
								{
#ifndef __EMSCRIPTEN__
									ImGui::Begin("Updater", &showUpdWin, ImGuiWindowFlags_AlwaysAutoResize);
#if DEACTIVATED
									if (isOtaPossible == YES)
									{
										ImGui::Text("Update Available!");
										if (ImGui::Button("Upgrade!"))
										{
											ota.upgrade(config);
											showUpdWin = CLOSE;
										}
									}
									else
									{
										ImGui::Text("Current Version : v%.4f", VERSION);
										ImGui::Text("Build Type : P0052");
										ImGui::Separator();
										ImGui::Text("Already using the latest version!");
										ImGui::Separator();
									}
#else
									ImGui::Text("Current Version : v%.4f", VERSION);
									ImGui::Text("Build Type : P0052");
									ImGui::Separator();
#endif
									ImGui::End();
#endif
								}

								if (showAboutWin == YES)
								{
									const float columnWidth = 150.0f;

									ImGui::Begin("About", &showAboutWin, ImGuiWindowFlags_AlwaysAutoResize);

									if (ImGui::CollapsingHeader("Build Info"))
									{
										ImGui::BeginGroup();
										ImGui::Columns(2, nullptr, false);
										ImGui::SetColumnWidth(0, columnWidth);

										ImGui::Text("Build");        ImGui::NextColumn(); ImGui::Text(": P0052");               ImGui::NextColumn();
										ImGui::Text("Version");        ImGui::NextColumn(); ImGui::Text(": v%.4f", VERSION);      ImGui::NextColumn();
										ImGui::Text("Built On"); ImGui::NextColumn(); ImGui::Text(": %s at %s IST", __DATE__, __TIME__); ImGui::NextColumn();

#if defined(NDEBUG)
										ImGui::Text("Build Type");     ImGui::NextColumn(); ImGui::Text(": Release");             ImGui::NextColumn();
#else
										ImGui::Text("Build Type");     ImGui::NextColumn(); ImGui::Text(": Debug");               ImGui::NextColumn();
#endif
										ImGui::Text("Commit");       ImGui::NextColumn(); ImGui::Text(": %s", MASQ_GIT_HASH); ImGui::NextColumn();

										ImGui::Text("C++ Standard");   ImGui::NextColumn();
#if __cplusplus == 199711L
										ImGui::Text(": C++98/03");
#elif __cplusplus == 201103L
										ImGui::Text(": C++11");
#elif __cplusplus == 201402L
										ImGui::Text(": C++14");
#elif __cplusplus == 201703L
										ImGui::Text(": C++17");
#elif __cplusplus == 202002L
										ImGui::Text(": C++20");
#elif __cplusplus > 202002L
										ImGui::Text(": C++23 or newer");
#else
										ImGui::Text(": Unknown (%ld)", __cplusplus);
#endif
										ImGui::NextColumn();

										ImGui::Text("Compiler");       ImGui::NextColumn();
#if defined(_MSC_VER)
										ImGui::Text(": MSVC (%d)", _MSC_VER);
#elif defined(__clang__)
										ImGui::Text(": Clang %d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUG__)
										ImGui::Text(": G++ %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
										ImGui::Text(": Unknown");
#endif
										ImGui::NextColumn();

										ImGui::Text("Architecture");   ImGui::NextColumn();
#if defined(_M_X64) || defined(__x86_64__)
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

										ImGui::Text("Platform");       ImGui::NextColumn();
#if defined(_WIN32)
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
										int sdlVer = SDL_GetVersion();
										ImGui::Text("SDL Linked"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", SDL_VERSIONNUM_MAJOR(sdlVer), SDL_VERSIONNUM_MINOR(sdlVer), SDL_VERSIONNUM_MICRO(sdlVer)); ImGui::NextColumn();
#endif

#ifdef GLAD_GL_VERSION_1_0
										if (GLAD_GL_VERSION_1_0)
										{
											ImGui::Text("Glad"); ImGui::NextColumn();
											ImGui::Text(": %d.%d", GLVersion.major, GLVersion.minor); ImGui::NextColumn();
										}
#endif

#ifdef BOOST_VERSION
										ImGui::Text("Boost"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", BOOST_VERSION / 100000, (BOOST_VERSION / 100) % 1000, BOOST_VERSION % 100); ImGui::NextColumn();
#endif

										// Static Library Info (manually add versions if needed)
#ifdef MZ_VERSION
										ImGui::Text("Miniz"); ImGui::NextColumn(); ImGui::Text(": v%s", mz_version()); ImGui::NextColumn();
#endif

										int major = 1, minor = 2, patch = 1;
										ImGui::Text("Nativefiledialog-extended"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", major, minor, patch); ImGui::NextColumn();

#ifdef __EMSCRIPTEN__
										major = 0;
										minor = 1;
										patch = 0;
										ImGui::Text("Emscripten-browser-file"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", major, minor, patch); ImGui::NextColumn();
#endif


#ifdef STB_IMAGE_IMPLEMENTATION
										ImGui::Text("STB (stb_image)"); ImGui::NextColumn(); ImGui::Text(": %.2f", 2.28); ImGui::NextColumn();
#endif

										major = 0;
										minor = 1;
										patch = 0;
										ImGui::Text("Chip-8-database"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d", major, minor, patch); ImGui::NextColumn();

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
										int sdlVer = SDL_GetVersion();
										ImGui::Text("SDL Linked"); ImGui::NextColumn(); ImGui::Text(": %d.%d.%d",
											SDL_VERSIONNUM_MAJOR(sdlVer),
											SDL_VERSIONNUM_MINOR(sdlVer),
											SDL_VERSIONNUM_MICRO(sdlVer)); ImGui::NextColumn();
#endif

										const char* rev = SDL_GetRevision();
										if (rev && *rev)
										{
											ImGui::Text("SDL Revision"); ImGui::NextColumn(); ImGui::Text(": %s", rev); ImGui::NextColumn();
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
										ImGui::Text("L1 Cache");      ImGui::NextColumn(); ImGui::Text(": %d KB", SDL_GetCPUCacheLineSize()); ImGui::NextColumn();
										ImGui::Text("System RAM");    ImGui::NextColumn(); ImGui::Text(": %d MB", SDL_GetSystemRAM()); ImGui::NextColumn();

										ImGui::Dummy(ImVec2(0.0f, 6.0f));  // Small vertical spacing
										ImGui::Text("Features:");         // Section label
										ImGui::NextColumn(); ImGui::Text(""); ImGui::NextColumn();

										ImGui::Text("SSE");     ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("SSE2");    ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE2() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("SSE3");    ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE3() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("SSE4.1");  ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE41() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("SSE4.2");  ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasSSE42() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("AVX");     ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasAVX() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("AVX2");    ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasAVX2() ? "Yes" : "No"); ImGui::NextColumn();
										ImGui::Text("NEON");    ImGui::NextColumn(); ImGui::Text(": %s", SDL_HasNEON() ? "Yes" : "No"); ImGui::NextColumn();

										ImGui::Columns(1); // Reset back to single column after done
										ImGui::EndGroup();
									}

									if (ImGui::CollapsingHeader("GPU Info"))
									{
										const GLubyte* vendor = glGetString(GL_VENDOR);
										const GLubyte* renderer = glGetString(GL_RENDERER);
										const GLubyte* version = glGetString(GL_VERSION);
										const GLubyte* shading = glGetString(GL_SHADING_LANGUAGE_VERSION);

										ImGui::BeginGroup();
										ImGui::Columns(2, nullptr, false);
										ImGui::SetColumnWidth(0, columnWidth);

										if (vendor)
										{
											ImGui::Text("GL Vendor");    ImGui::NextColumn(); ImGui::Text(": %s", vendor);    ImGui::NextColumn();
										}
										if (renderer)
										{
											ImGui::Text("GL Renderer");  ImGui::NextColumn(); ImGui::Text(": %s", renderer);  ImGui::NextColumn();
										}
										if (version)
										{
											ImGui::Text("GL Version");   ImGui::NextColumn(); ImGui::Text(": %s", version);   ImGui::NextColumn();
										}
										if (shading)
										{
											ImGui::Text("GLSL Version"); ImGui::NextColumn(); ImGui::Text(": %s", shading);   ImGui::NextColumn();
										}

										ImGui::Columns(1);
										ImGui::EndGroup();
									}

									if (ImGui::CollapsingHeader("Developer Info"))
									{
										ImGui::Text("By Kotambail-Hegde");
										ImGui::TextLinkOpenURL("Homepage", "https://kotambail-hegde.github.io/Masquerade-Emulator/");
										ImGui::SameLine();
										ImGui::Text("|");
										ImGui::SameLine();
										ImGui::TextLinkOpenURL("Releases", "https://github.com/Kotambail-Hegde/Masquerade-Emulator/releases");
										ImGui::Separator();
										ImGui::Text("(c) 2017-2025 Saurabh S Hegde");
										ImGui::Text("For licensing, refer to");
										ImGui::SameLine();
										ImGui::TextLinkOpenURL("License", "https://github.com/Kotambail-Hegde/Masquerade-Emulator/blob/main/LICENSE.md");
									}
									ImGui::End();
								}

								if (showLoggerWin == YES)
								{
									ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 255));
									ImGui::Begin("Logger Console", &showLoggerWin, ImGuiWindowFlags_AlwaysAutoResize);
									appLog.Draw();
									ImGui::End();
									ImGui::PopStyleColor();
								}

								if (showCheatWin == YES
									&&
									((current_instance->getEmulationID() == EMULATION_ID::NES_ID)
										||
										(current_instance->getEmulationID() == EMULATION_ID::GB_GBC_ID)
										||
										(current_instance->getEmulationID() == EMULATION_ID::GBA_ID)))
								{
									ImGui::Begin("Cheats", &showCheatWin, ImGuiWindowFlags_AlwaysAutoResize);
									ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
									FLAG atleastOneCE = NO;
									static std::string dgg, dgs, gamegenie, gameshark;
									static FLAG gg[2][MAX_CHEAT_COUNT_PER_ENGINE] = { NO };
									static FLAG gs[2][MAX_CHEAT_COUNT_PER_ENGINE] = { NO };
									static int32_t selectedCEMode;
									ImGui::RadioButton("GameGenie", &selectedCEMode, TO_UINT(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE));
									ImGui::RadioButton("GameShark", &selectedCEMode, TO_UINT(CheatEngine_t::CHEATING_ENGINE::GAMESHARK));
									ceMAS->setCheatEngineMode((CheatEngine_t::CHEATING_ENGINE)selectedCEMode, current_instance->getEmulationID());
									FLAG enable = YES;
									if (enable)
									{
										atleastOneCE = YES;
										ImGui::Text("GameGenie");
										ImGui::InputText("GG Name", &dgg, input_text_flags);
										if (ImGui::InputText("GG Code", &gamegenie, input_text_flags))
										{
											if (dgg.empty())
											{
												dgg = gamegenie;
											}
											selectedCEMode = TO_UINT(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE);
											ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, current_instance->getEmulationID());
											ceMAS->applyNewCheat(dgg, gamegenie);

											dgg.clear();
											gamegenie.clear();
										}
										ImGui::Separator();
									}
									enable = (current_instance->getEmulationID() == EMULATION_ID::GB_GBC_ID) || (current_instance->getEmulationID() == EMULATION_ID::GBA_ID);
									if (enable)
									{
										atleastOneCE = YES;
										ImGui::Text("GameShark");
										ImGui::InputText("GS Name", &dgs, input_text_flags);
										if (ImGui::InputText("GS Code", &gameshark, input_text_flags))
										{
											if (dgs.empty())
											{
												dgs = gameshark;
											}
											selectedCEMode = TO_UINT(CheatEngine_t::CHEATING_ENGINE::GAMESHARK);
											ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMESHARK, current_instance->getEmulationID());
											ceMAS->applyNewCheat(dgs, gameshark);

											dgs.clear();
											gameshark.clear();
										}
										ImGui::Separator();
									}
									if (atleastOneCE == YES)
									{
										ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
										if (ImGui::BeginTabBar("Cheat List", tab_bar_flags))
										{
											enable = YES;
											if (enable)
											{
												if (ImGui::BeginTabItem("GameGenie"))
												{
													//ceMAS->listAllTheCheats(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, gg);

													INC8 ii = RESET;
													for (auto& [key, value] : ceMAS->getCheatList(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE))
													{
														if (ii < MAX_CHEAT_COUNT_PER_ENGINE)
														{
															if (ceMAS->getCheatEngineMode() != CheatEngine_t::CHEATING_ENGINE::GAMEGENIE)
															{
																gg[PREV][ii] = NO;
																gg[CURR][ii] = NO;
															}
															else
															{
																if (gg[PREV][ii] != gg[1][ii])
																{
																	gg[PREV][ii] = gg[CURR][ii];
																	if (gg[CURR][ii] == NO)
																	{
																		ceMAS->disableCheat(key);
																	}
																	else
																	{
																		selectedCEMode = TO_UINT(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE);
																		ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE, current_instance->getEmulationID());
																		ceMAS->enableCheat(key);
																	}
																}
															}

															ImGui::Checkbox(value.c_str(), &(gg[CURR][ii]));
															ImGui::SameLine();
															float contentRegionRight = ImGui::GetWindowContentRegionMax().x;
															float buttonWidth = ImGui::CalcTextSize("Delete##gg##").x + ImGui::GetStyle().FramePadding.x * 2.0f;
															float rightAlignedX = contentRegionRight - buttonWidth;
															ImGui::SetCursorPosX(rightAlignedX);
															if (ImGui::Button(std::string("Delete##gg##" + std::to_string(ii)).c_str()))
															{
																gg[PREV][ii] = NO;
																gg[CURR][ii] = NO;
																if (ceMAS->getCheatEngineMode() == CheatEngine_t::CHEATING_ENGINE::GAMEGENIE)
																{
																	ceMAS->deleteCheat(key);
																	ceMAS->getCheatList(CheatEngine_t::CHEATING_ENGINE::GAMEGENIE).erase(key);
																}
															}
															ii++;
														}
													}

													ImGui::EndTabItem();
												}
											}
											enable = (current_instance->getEmulationID() == EMULATION_ID::GB_GBC_ID) || (current_instance->getEmulationID() == EMULATION_ID::GBA_ID);
											if (enable)
											{
												if (ImGui::BeginTabItem("GameShark"))
												{
													//ceMAS->listAllTheCheats(CheatEngine_t::CHEATING_ENGINE::GAMESHARK, gg);

													INC8 ii = RESET;
													for (auto& [key, value] : ceMAS->getCheatList(CheatEngine_t::CHEATING_ENGINE::GAMESHARK))
													{
														if (ii < MAX_CHEAT_COUNT_PER_ENGINE)
														{
															if (ceMAS->getCheatEngineMode() != CheatEngine_t::CHEATING_ENGINE::GAMESHARK)
															{
																gs[PREV][ii] = NO;
																gs[CURR][ii] = NO;
															}
															else
															{
																if (gs[PREV][ii] != gs[1][ii])
																{
																	gs[PREV][ii] = gs[CURR][ii];
																	if (gs[CURR][ii] == NO)
																	{
																		ceMAS->disableCheat(key);
																	}
																	else
																	{
																		selectedCEMode = TO_UINT(CheatEngine_t::CHEATING_ENGINE::GAMESHARK);
																		ceMAS->setCheatEngineMode(CheatEngine_t::CHEATING_ENGINE::GAMESHARK, current_instance->getEmulationID());
																		ceMAS->enableCheat(key);
																	}
																}
															}

															ImGui::Checkbox(value.c_str(), &(gs[CURR][ii]));
															ImGui::SameLine();
															float contentRegionRight = ImGui::GetWindowContentRegionMax().x;
															float buttonWidth = ImGui::CalcTextSize("Delete##gg##").x + ImGui::GetStyle().FramePadding.x * 2.0f;
															float rightAlignedX = contentRegionRight - buttonWidth;
															ImGui::SetCursorPosX(rightAlignedX);
															if (ImGui::Button(std::string("Delete##gs##" + std::to_string(ii)).c_str()))
															{
																gs[PREV][ii] = NO;
																gs[CURR][ii] = NO;
																if (ceMAS->getCheatEngineMode() == CheatEngine_t::CHEATING_ENGINE::GAMESHARK)
																{
																	ceMAS->deleteCheat(key);
																	ceMAS->getCheatList(CheatEngine_t::CHEATING_ENGINE::GAMESHARK).erase(key);
																}
															}

															ii++;
														}
													}

													ImGui::EndTabItem();
												}
											}
											ImGui::EndTabBar();
										}
									}
									ImGui::End();
								}

								ImGui::PopStyleVar(); // For scroll bar size

								// Other Masquerade specific continous updates
								if (OnUserUpdate() != SUCCESS)
								{
#ifndef __EMSCRIPTEN__
									done = YES;
#else
									if (SavePersistentFSComplete == YES || ClearPersistentFSComplete == YES)
									{
										SavePersistentFSComplete = NO;
										ClearPersistentFSComplete = NO;
										done = YES;
									}
#endif
								}
							}
							else
							{
								if (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))
								{
									initScreen = NO;
								}
								else
								{
									ImGui::Begin(emuWindow.c_str(), &showEmuWin,
										ImGuiWindowFlags_NoTitleBar
										| ImGuiWindowFlags_NoCollapse
										| ImGuiWindowFlags_NoScrollbar
										| ImGuiWindowFlags_AlwaysAutoResize);
									ImVec2 avail_size = ImGui::GetContentRegionAvail();
									ImVec2 image_size = ImVec2(static_cast<float>(clickWinWidth), static_cast<float>(clickWinHeight));
									ImGui::SetCursorPosX((ImGui::GetWindowSize().x - image_size.x) * 0.5f);
									ImGui::SetCursorPosY((ImGui::GetWindowSize().y - image_size.y) * 0.5f);
									ImGui::Image((ImTextureID)(uintptr_t)clickWinTexture, image_size);
									ImGui::End();
								}
							}

							// Rendering
							ImGui::Render();

							glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
							glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
							glClear(GL_COLOR_BUFFER_BIT);
							ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

							// Update and Render additional Platform Windows
							// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
							// For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
							if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
							{
								SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
								SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
								ImGui::UpdatePlatformWindows();
								ImGui::RenderPlatformWindowsDefault();
								SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
							}

							SDL_GL_SwapWindow(window);

#ifndef __EMSCRIPTEN__
							// Maintain Precise FPS
							if (1000000000 / myFPS > SDL_GetTicksNS() - tickAtStart)
							{
								SDL_DelayPrecise(static_cast<uint64_t>((1000000000.0f / myFPS)) - (SDL_GetTicksNS() - tickAtStart));
							}
#endif
							if (done == YES)
							{
#if __EMSCRIPTEN__
								TODO("No cleanup of Masquerade, IMGUI, GLES3 or SDL3 is done here as doing so is causing issue in reloading the .js and .wasm. Hoping that Browser takes care of the necessary cleanup");
								INFO("Restarting Masquerade");
								emscripten_cancel_main_loop();

								if (rebootNeededOnMenuClick == YES)
								{
									emscripten_run_script("location.reload();"); // Reload URL as is without any stripping
								}
								else
								{
									BYTE numberOfInputs = RESET;

									if (dynamicDragNDropAndMenuSelect.size() != ZERO)
									{
										numberOfInputs = dynamicDragNDropAndMenuSelect.size();

										for (int ii = ZERO; ii < numberOfInputs; ii++)
										{
											romsToRun[ii] = dynamicDragNDropAndMenuSelect[ii].c_str();
										}

										if (numberOfInputs > ONE)
										{
											for (uint32_t ii = ZERO; ii < numberOfInputs; ii++)
											{
												// Hack!
												if (numberOfInputs == FOUR) // Then this is most probably space invaders
												{
													std::string SI_ROMS_EXT_LIST[FOUR] =
													{
														".e"
														,".f"
														,".g"
														,".h"
													};

													for (uint32_t jj = ZERO; jj < MAX_NUMBER_ROMS_FOR_SI; jj++)
													{
														if (dynamicDragNDropAndMenuSelect[ii].find(SI_ROMS_EXT_LIST[jj]) != std::string::npos)
														{
															romsToRun[jj] = dynamicDragNDropAndMenuSelect[ii].c_str();
														}
													}
												}
												else if (numberOfInputs == TEN) // Then this is most probably pacman
												{
													std::string PM_ROMS_EXT_LIST[TEN] =
													{
														"6e"
														,"6f"
														,"6h"
														,"6j"
														,"7f"
														,"4a"
														,"5e"
														,"5f"
														,"1m"
														,"3m"
													};

													for (uint32_t jj = ZERO; jj < MAX_NUMBER_ROMS_FOR_PM; jj++)
													{
														if (dynamicDragNDropAndMenuSelect[ii].find(PM_ROMS_EXT_LIST[jj]) != std::string::npos)
														{
															romsToRun[jj] = dynamicDragNDropAndMenuSelect[ii].c_str();
														}
													}
												}
												else if (numberOfInputs == THIRTEEN) // Then this is most probably ms pacman
												{
													std::string MSPM_ROMS_EXT_LIST[THIRTEEN] =
													{
														".6e"
														,".6f"
														,".6h"
														,".6j"
														,".7f"
														,".4a"
														,"5e"
														,"5f"
														,".1m"
														,".3m"
														,"u5"
														,"u6"
														,"u7"
													};

													for (uint32_t jj = ZERO; jj < MAX_NUMBER_ROMS_FOR_MSPM; jj++)
													{
														if (dynamicDragNDropAndMenuSelect[ii].find(MSPM_ROMS_EXT_LIST[jj]) != std::string::npos)
														{
															romsToRun[jj] = dynamicDragNDropAndMenuSelect[ii].c_str();
														}
													}
												}
											}
										}

										dynamicDragNDropAndMenuSelect.clear();

										// Encode the ROM paths
										std::string js = "setTimeout(() => { let roms = [";

										for (int i = 0; i < numberOfInputs; ++i)
										{
											std::string romPath = romsToRun[i];

											// Ensure it begins with "/persistent/"
											if (romPath.rfind("/persistent/", 0) != 0)
												romPath = "/persistent/" + romPath;

											// Remove leading slash for URL encoding (GH Pages fix)
											std::string urlRomPath = romPath.substr(1);  // "persistent/xxx.gb"

											js += "'" + urlRomPath + "'";
											if (i < numberOfInputs - 1) js += ",";
										}

										js += "]; let url = window.location.pathname.replace(/[^/]+$/, '') + 'masquerade.html?roms=' + roms.length;";

										// Add ROM parameters
										js += "roms.forEach((rom, i) => url += '&rom' + i + '=' + encodeURIComponent(rom));";

										// Perform redirect
										js += "location.href = url; }, 0);";

										emscripten_run_script(js.c_str());
									}
									else
									{
										// Restart -> Reset the URL such that rom information is stripped out and only bare minimum remains
										emscripten_run_script(
											"location.href = window.location.pathname.replace(/[^/]+$/, '') + 'masquerade.html';"
										);
									}
								}
#endif
							}
						}
					};

#if defined(__EMSCRIPTEN__) && ENABLED
				static double accumulator = 0.0;
				static double previousTime = emscripten_get_now() / 1000.0;

				double currentTime = emscripten_get_now() / 1000.0;  // Convert to seconds
				double deltaTime = currentTime - previousTime;
				previousTime = currentTime;

				// 1) Avoid big spikes at startup or tab-switch
				const double MAX_DELTA_TIME = 0.1;
				deltaTime = std::clamp(deltaTime, 0.0, MAX_DELTA_TIME);

				accumulator += deltaTime;

				// 2) Prevent long catch-up bursts
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
			}

#ifdef __EMSCRIPTEN__
			EMSCRIPTEN_MAINLOOP_END;
#else

			if (RUN_IMGUI_DEMO == NO)
			{
				// Other Masquerade specific cleanup
				glDeleteTextures(1, &masquerade_texture);
				OnUserDestroy(window);
#if (GL_FIXED_FUNCTION_PIPELINE == NO)
				glDeleteBuffers(1, &fullscreenVBO);
				glDeleteVertexArrays(1, &fullscreenVAO);
#endif
#if (GL_FIXED_FUNCTION_PIPELINE == NO)
				glDeleteProgram(shaderProgramBasic);
				glDeleteProgram(shaderProgramBlend);
#endif
				glDeleteFramebuffers(1, &frame_buffer);
			}

			// Native File System
			NFD_Quit();

			// Cleanup
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			ImGui::DestroyContext();

			SDL_GL_DestroyContext(gl_context);
			SDL_StopTextInput(window);
			SDL_DestroyWindow(window);
			SDL_Quit();
#endif
		}

		RETURN ZERO;
	}
#pragma endregion EMULATION_DEFINITION
};
#pragma endregion CORE

#pragma region BOOT_UP
void arrangeRoms(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM>& romsToRun)
{
	const size_t numberOfInputs =
		std::count_if(romsToRun.begin(), romsToRun.end(),
			[](const std::string& s) { return !s.empty(); });

	// Expected order lists
	static constexpr std::array siExts = { ".e", ".f", ".g", ".h" };
	static constexpr std::array pmExts = { "6e","6f","6h","6j","7f",
										   "4a","5e","5f","1m","3m" };
	static constexpr std::array mspmExts = { ".6e",".6f",".6h",".6j",".7f",
											 ".4a","5e","5f",".1m",".3m",
											 "u5","u6","u7" };

	std::array<std::string, 13> reordered{};
	reordered = romsToRun;

	if (numberOfInputs == siExts.size())
	{
		for (const auto& fname : romsToRun)
		{
			for (size_t jj = RESET; jj < siExts.size(); ++jj)
				if (fname.ends_with(siExts[jj]))
					reordered[jj] = fname;
		}
	}
	else if (numberOfInputs == pmExts.size())
	{
		for (const auto& fname : romsToRun)
		{
			for (size_t jj = RESET; jj < pmExts.size(); ++jj)
				if (fname.ends_with(pmExts[jj]))
					reordered[jj] = fname;
		}
	}
	else if (numberOfInputs == mspmExts.size())
	{
		for (const auto& fname : romsToRun)
		{
			for (size_t jj = RESET; jj < mspmExts.size(); ++jj)
				if (fname.ends_with(mspmExts[jj]))
					reordered[jj] = fname;
		}
	}

	romsToRun = reordered; // overwrite original with rearranged order
}

abstractEmulation_t* getType(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config, CheatEngine_t* ce)
{
	EMULATION_ID suspectedID = EMULATION_ID::DEFAULT_ID;

	// First check if the number of ROMs passed matches any of the expected emulation platform...

	if (_numberOfRomsToEmulationPlatform.count(nFiles) == ZERO)
	{
		RETURN new defaults_t;
	}

	if (nFiles == SINGLE_ROM_FILE)
	{
		// Lot of Emulation Platforms have "Number Of ROM" requirements set to 1
		// So, check based on "_numberOfRomsToEmulationPlatform" is not present inside this "if" block

		std::filesystem::path filepath;
		filepath = rom[ZERO];

		// Unknown ROM extention
		if (_fileExtentionToEmulationPlatform.count(filepath.extension().string()) == ZERO)
		{
			RETURN new defaults_t;
		}

		suspectedID = _fileExtentionToEmulationPlatform.find(filepath.extension().string())->second;

		if (suspectedID == EMULATION_ID::CHIP8_ID)
		{
			RETURN new chip8_t(rom, config);
		}
		if (suspectedID == EMULATION_ID::NES_ID)
		{
			RETURN new NES_t(ONE, rom, config, ce);
		}
		else if (suspectedID == EMULATION_ID::GB_GBC_ID)
		{
			RETURN new GBc_t(ONE, rom, config, ce);
		}
		else if (suspectedID == EMULATION_ID::GBA_ID)
		{
			RETURN new GBA_t(ONE, rom, config);
		}
		else if (suspectedID == EMULATION_ID::GAME_OF_LIFE_ID)
		{
			RETURN new gameOfLife_t(config);
		}
		else
		{
			RETURN new defaults_t;
		}
	}
	else if (nFiles == TEST_ROM_FILE)
	{
		auto emulationPlatforms = _numberOfRomsToEmulationPlatform.find(nFiles);
		suspectedID = emulationPlatforms->second;

		// TODO: Re-arrange the ROM list as needed by the individual emulators...

		std::filesystem::path filepath;
		filepath = rom[ONE];

		if ((rom[ZERO] == "-8080SST") || (toUpper(rom[ZERO]) == "-I8080SST"))
		{
			INFO("Setting up the 8080 SST environment");
			RETURN new spaceInvaders_t(nFiles, rom, config);
		}
		else if (toUpper(rom[ZERO]) == "-Z80SST")
		{
			INFO("Setting up the Z80 SST environment");
			RETURN new pacMan_t(nFiles, rom, config);
		}
		else if ((toUpper(rom[ZERO]) == "-R6502SST") || (toUpper(rom[ZERO]) == "-N6502SST"))
		{
			INFO("Setting up the Ricoh2A03 / NES6502 SST environment");
			RETURN new NES_t(nFiles, rom, config, ce);
		}
		else if (toUpper(rom[ZERO]) == "-SM83SST")
		{
			INFO("Setting up the SM83 SST environment");
			RETURN new GBc_t(nFiles, rom, config, ce);
		}
		else if (toUpper(rom[ZERO]) == "-ARM7TDMISST")
		{
			INFO("Setting up the ARM7TDMI SST environment");
			RETURN new GBA_t(nFiles, rom, config);
		}

		// Unknown ROM extention
		if (_fileExtentionToEmulationPlatform.count(filepath.extension().string()) == ZERO)
		{
			RETURN new defaults_t;
		}

		if (_fileExtentionToEmulationPlatform.find(filepath.extension().string())->second != suspectedID)
		{
			INFO("ROM file is corrupted");
			RETURN new defaults_t;
		}

		if ((rom[ZERO] == "-8080") || (toUpper(rom[ZERO]) == "-I8080"))
		{
			INFO("Setting up the 8080 test environment");
			RETURN new spaceInvaders_t(--nFiles, rom, config);
		}
		else if (toUpper(rom[ZERO]) == "-Z80")
		{
			INFO("Setting up the Z80 test environment");
			RETURN new pacMan_t(--nFiles, rom, config);
		}
		else if (toUpper(rom[ZERO]) == "-6502")
		{
			INFO("Setting up the 6502 test environment");
			RETURN new NES_t(--nFiles, rom, config, ce);
		}
		else
		{
			INFO("Undefined Core");
			RETURN new defaults_t;
		}
	}
	else if (nFiles == REPLAY_ROM_FILE)
	{
		if ((toUpper(rom[ZERO]) == "-R") || (toUpper(rom[ZERO]) == "-C"))
		{
			if (toUpper(rom[ONE].substr(rom[ONE].find_last_of(".") + ONE)) == "GBA")
			{
				if (toUpper(rom[ZERO]) == "-R")
				{
					FATAL("Replay Mode is not supported yet; Only Compare Mode is supported as of now")
				}
				else
				{
					INFO("Setting up the GBA environment in replay mode");
					RETURN new GBA_t(nFiles, rom, config);
				}
			}
		}

		FATAL("Not supported yet!");
		RETURN new defaults_t;
	}
	else
	{
		// Either of the following
		// 1) Space Invaders
		// 2) Pac Man (or Ms Pac Man)
		// All 2 mentioned above have unique "Number Of ROM" requirements!

		auto emulationPlatforms = _numberOfRomsToEmulationPlatform.find(nFiles);
		suspectedID = emulationPlatforms->second;

		std::filesystem::path* filepath = new std::filesystem::path[nFiles];

		if (filepath)
		{
			for (int count = 0; count < nFiles; count++)
			{
				filepath[count] = rom[count];

				if (_fileExtentionToEmulationPlatform.count(filepath[count].extension().string()) == ZERO)
				{
					delete[] filepath;
					RETURN new defaults_t;
				}

				if (_fileExtentionToEmulationPlatform.find(filepath[count].extension().string())->second != suspectedID)
				{
					INFO("Some of the ROM files are corrupted");
					delete[] filepath;
					RETURN new defaults_t;
				}

			}

			// TODO: Re-arrange the ROM list as needed by the individual emulators...

			delete[] filepath;

			if (suspectedID == EMULATION_ID::SPACE_INVADERS_ID)
			{
				arrangeRoms(rom);
				RETURN new spaceInvaders_t(nFiles, rom, config);
			}
			else if (suspectedID == EMULATION_ID::PACMAN_ID)
			{
				arrangeRoms(rom);
				RETURN new pacMan_t(nFiles, rom, config);
			}

		}
	}

	RETURN new defaults_t;
}

FLAG startMasquerade(int nFiles, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, ID bootType = BOOT)
{
	FLAG status = SUCCESS;

	INFO("Running Masquerade Emulator!");
	INFO("If you experience screen tearing -> Please ensure V-Sync is enabled");
	INFO("Nvidia Control Panel > 3D Settings > Global settings > V - Sync->ON");
	INFO("If you experience low/limitted FPS -> Please ensure V-Sync is disabled");
	INFO("Nvidia Control Panel > 3D Settings > Global settings > V - Sync->OFF");

	// check if scaling is needed
	_XSCALE = config.get<DIM32>("mods._XSCALE");

	// check if FPS boost is needed
	_XFPS = config.get<DIM32>("mods._XFPS");

	// check if debugger needs to be enabled
	debugConfig._DEBUG_PPU_VIEWER_GUI = to_bool(config.get<std::string>("debug._DEBUG_PPU_VIEWER_GUI"));
	debugConfig._DEBUG_PPU_VIEWER_GUI_TRIGGER = config.get<INC64>("debug._DEBUG_PPU_VIEWER_GUI_TRIGGER");
	debugConfig._DEBUG_LOGGER_CLI = to_bool(config.get<std::string>("debug._DEBUG_LOGGER_CLI"));
	debugConfig._DEBUG_LOGGER_CLI_MASK = config.get<MAP64>("debug._DEBUG_LOGGER_CLI_MASK");

	// get emulator theme
	currentEmuTheme = TO_UINT8(configToEmuThemes.at(config.get<std::string>("mods._EMULATOR_THEME")));
	previousEmuTheme = currentEmuTheme;

	// get video filter selected
	currEnVFilter = configToVFilters.at(config.get<std::string>("mods._VIDEO_EFFECTS"));

	numberOfRomsSelected = nFiles;

	CheatEngine_t* ce = new CheatEngine_t();
	ce->loadCheatNames( _CHEAT_SAVE_LOCATION);
	abstractEmulation_t* toEmulate = getType(nFiles, rom, config, ce);
	Emulation_t Emulation(toEmulate, config, ce);
	Emulation.Start();
	ce->saveCheatNames(_CHEAT_SAVE_LOCATION);
	delete toEmulate;
	toEmulate = nullptr;
	delete ce;
	ce = nullptr;
	RETURN status;
}

void secondaryBootLoader(int argc, std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> argv, ID bootType = BOOT)
{
	if (argc == SINGLE_ROM_FILE)
	{
		INFO("ROM loaded: %s", argv[ZERO].c_str());
		INFO("ROM length: %zu", argv[ZERO].length());
		INFO("ROM contains spaces: %s", (argv[ZERO].find(' ') != std::string::npos) ? "YES" : "NO");

		auto it = std::find(recentlyOpenedList.begin(), recentlyOpenedList.end(), argv[ZERO].c_str());
		// Check if the element was found
		if (it != recentlyOpenedList.end())
		{
			// Element found, delete it
			recentlyOpenedList.erase(it);
		}
		recentlyOpenedList.push_front(argv[ZERO].c_str());
		if (recentlyOpenedList.size() > _MAX_RECENTLY_USED_LIST_SIZE)
		{
			recentlyOpenedList.pop_back();
		}
	}
	else if (argc > SINGLE_ROM_FILE)
	{
		INFO("ROM file loaded:");
		for (int count = ZERO; count < argc; count++)
		{
			INFO("%s", argv[count].c_str());
		}
	}

	// Start Masquerade

	startMasquerade(argc, argv, bootType);
}

void postPrimaryBootLoader()
{
#ifdef __EMSCRIPTEN__
	listEmFiles();
#endif

	// --- Setup all the paths
#ifndef __EMSCRIPTEN__
	_EXE_LOCATION = getexepath().parent_path().string(); // path till masquerade.exe
	_CONFIG_LOCATION = (std::filesystem::path(_EXE_LOCATION) / "assets" / "CONFIG.ini").string();
#else
	_CONFIG_LOCATION = "assets/CONFIG.ini";
#endif

	INFO("Searching for %s", _CONFIG_LOCATION.c_str());

	// --- Read CONFIG.ini
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

	// ------------------------------------------------------------------
	// Normalize ALL paths coming from CONFIG.ini (Windows → POSIX safe)
	// ------------------------------------------------------------------
	auto normalizePath = [](std::string& p)
		{
			std::replace(p.begin(), p.end(), '\\', '/');
		};

#ifndef __EMSCRIPTEN__
	std::string uiWorkingDir = config.get<std::string>("internal._ui_working_directory");
	normalizePath(uiWorkingDir);

	_IMGUI_LOCATION = (std::filesystem::path(uiWorkingDir) / "IMGUI.ini").string();
#else
	_IMGUI_LOCATION = "assets/ui/config/IMGUI.ini";
#endif

	// --- Ensure IMGUI directory exists
	std::filesystem::create_directories(std::filesystem::path(_IMGUI_LOCATION).parent_path());

	// --- Check if IMGUI.ini exists
	struct stat buffer;
	if (stat(_IMGUI_LOCATION.c_str(), &buffer) != ZERO)
	{
		// IMGUI.ini doesn't exist, so create a default one....
		std::ofstream out(_IMGUI_LOCATION);
		if (!out)
		{
			throw std::runtime_error("Failed to open file: " + _IMGUI_LOCATION);
		}

		INFO("Not able to find IMGUI.ini");
		INFO("Creating a masquerade default for now!");
		out << imguiDefaultIni;
		out.close();
	}

	INFO("Searching for %s", _IMGUI_LOCATION.c_str());

	createLUTForCRC();

#ifndef __EMSCRIPTEN__
	recentlyOpenedListPath = config.get<std::string>("internal._working_directory");
	normalizePath(recentlyOpenedListPath);
#else
	recentlyOpenedListPath = "assets/internal";
#endif

	// --- Ensure working directory exists
	ifNoDirectoryThenCreate(recentlyOpenedListPath);

#ifndef __EMSCRIPTEN__
	recentlyOpenedListPath = (std::filesystem::path(recentlyOpenedListPath) / "recentlyOpenedListPath.dir").string();

	recentlyOpenedList = readDequeFromFile(recentlyOpenedListPath);
#else
	recentlyOpenedListPath += "/recentlyOpenedListPath.dir";
#endif

#ifndef __EMSCRIPTEN__
	_CHEAT_SAVE_LOCATION = (std::filesystem::path(config.get<std::string>("internal._working_directory")) /"cheats.txt").string();
	normalizePath(_CHEAT_SAVE_LOCATION);
#else
	_CHEAT_SAVE_LOCATION = "assets/internal/cheats.txt";
#endif

	BYTE bootType = BOOT;

BOOT_AGAIN:
	romsToRun.fill("");

	if (bootType == BOOT)
	{
		for (int ii = ZERO; ii < gArgc - ONE; ii++)
		{
			romsToRun[ii] = gArgv[ii + ONE];
		}
		secondaryBootLoader(gArgc - ONE, romsToRun);
	}
	else
	{
		if (dynamicDragNDropAndMenuSelect.size() != ZERO)
		{
			const DIM8 numberOfInputs = static_cast<DIM8>(dynamicDragNDropAndMenuSelect.size());

			for (DIM8 ii = ZERO; ii < numberOfInputs; ii++)
			{
				romsToRun[ii] = dynamicDragNDropAndMenuSelect[ii].c_str();
			}

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

	RETURN;
}

int main(int argc, char* argv[])
{
	gArgc = argc;
	gArgv = argv;

	SETBIT(ENABLE_LOGS, LOG_VERBOSITY);
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);

	// Handle commands if applicable
	if (gArgc > ONE)
	{
		if (strcmp(gArgv[ONE], "-h") == ZERO || strcmp(gArgv[ONE], "--help") == ZERO)
		{
			INFO("Figure it out yourself...");
			RETURN ZERO;
		}
	}

#ifdef __EMSCRIPTEN__
	INFO("Masquerade running in emscripten mode");
	mountPersistentFS(postPrimaryBootLoader);
#else
	postPrimaryBootLoader();
#endif

	RETURN ZERO;
}
#pragma endregion BOOT_UP