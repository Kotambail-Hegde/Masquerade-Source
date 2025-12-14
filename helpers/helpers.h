#pragma once

#pragma region ADDITIONAL_FUNCTIONALITIES_PRE
#define UNICODE										1
#define VERSION										static_cast<float>(0.7007)
#define WINDOW_PADDING								16
#define NOMINMAX
#define GL_FIXED_FUNCTION_PIPELINE					NO
#pragma endregion ADDITIONAL_FUNCTIONALITIES_PRE

// ----------------------
// Windows platform fix
// ----------------------
#ifdef _WIN32
// Target Windows 7 or later for Winsock2 / Boost.Asio
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>   // Must be included BEFORE Windows.h
#include <ws2tcpip.h>

#include <Windows.h>    // Only after winsock2.h
#else
#include <dlfcn.h>
#endif

// ----------------------
// C++ Standard Library
// ----------------------
#pragma region INCLUDES
#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <codecvt>
#include <cstdint>       // for SHA1
#include <cstring>       // for SHA1
#include <deque>
#include <filesystem>    // C++17 std::filesystem
#include <fstream>
#include <future>
#include <iomanip>       // for SHA1
#include <iostream>
#include <inttypes.h>    // For PRIu64
#include <locale>
#include <memory>
#include <math.h>
#include <mutex>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <sstream>       // for SHA1
#include <stdio.h>
#include <string>
#include <sys/stat.h>    // Specifically for GCC
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#pragma endregion INCLUDES

// ----------------------
// Boost Libraries
// ----------------------
#pragma region BOOST_INCLUDES
#ifndef BOOST_ALL_NO_LIB
#define BOOST_ALL_NO_LIB
#endif
#ifdef ENABLE_OTA_EXECUTABLE
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>
#endif
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/throw_exception.hpp>
#pragma endregion BOOST_INCLUDES

#ifndef ENABLE_OTA_EXECUTABLE
#pragma region EMSCRIPTEN_INCLUDES
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/html5.h>
#pragma region EMSCRIPTEN_FILE_BROWSER_INCLUDES
#include <dirent.h>
// Refer to https://github.com/Armchair-Software/emscripten-browser-file
#include "emscripten_browser_file.h"
#pragma endregion EMSCRIPTEN_FILE_BROWSER_INCLUDES
#endif
#pragma endregion EMSCRIPTEN_INCLUDES

#pragma region IMGUI_INCLUDES
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#ifdef __EMSCRIPTEN__
#include "emscripten/emscripten_mainloop_stub.h"
#endif
#pragma endregion IMGUI_INCLUDES

#pragma region GLAD_INCLUDES
#if !defined(__EMSCRIPTEN__) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
#include <glad/glad.h>
#endif
#pragma endregion GLAD_INCLUDES

#pragma region SDL_INCLUDES
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <SDL3/SDL_version.h> 
#pragma endregion SDL_INCLUDES

#pragma region NFD_INCLUDES
#ifndef __EMSCRIPTEN__
#include <nfd.hpp>
//#include <nfd_sdl2.h>
#endif
#pragma endregion NFD_INCLUDES
#endif

#pragma region MINIZ_INCLUDES
#include <miniz.h>
#pragma endregion MINIZ_INCLUDES

#pragma region MACROS
#ifndef FALSE
#define FALSE										0
#endif
#ifndef TRUE
#define TRUE										1
#endif
#ifndef ERROR
#define ERROR										0
#endif
#ifndef MAX_PATH
#define MAX_PATH									260
#endif
#define ZERO										0
#define ONE											1
#define TWO											2
#define THREE										3
#define FOUR										4
#define FIVE										5
#define SIX											6
#define SEVEN										7
#define EIGHT										8
#define NINE										9
#define TEN											10
#define ELEVEN										11
#define TWELVE										12
#define THIRTEEN									13
#define FOURTEEN									14
#define FIFTEEN										15
#define SIXTEEN										16
#define SEVENTEEN									17
#define EIGHTEEN									18
#define NINETEEN									19
#define TWENTY										20
#define TWENTYONE									21
#define TWENTYTWO									22
#define TWENTYTHREE									23
#define TWENTYFOUR									24
#define TWENTYFIVE									25
#define TWENTYSIX									26
#define TWENTYSEVEN									27
#define TWENTYEIGHT									28
#define TWENTYNINE									29
#define THIRTY										30
#define THIRTYONE									31
#define THIRTYTWO									32
#define THIRTYTHREE									33
#define THIRTYFOUR									34
#define THIRTYFIVE									35
#define THIRTYSIX									36
#define THIRTYSEVEN									37
#define THIRTYEIGHT									38
#define THIRTYNINE									39
#define FORTY										40
#define FORTYONE									41
#define FORTYTWO									42
#define FORTYTHREE									43
#define FORTYFOUR									44
#define FORTYFIVE									45
#define FORTYSIX									46
#define FIFTY										50
#define FIFTYSIX									56
#define FIFTYSEVEN									57
#define FIFTYEIGHT									58
#define FIFTYNINE									59
#define SIXTY										60
#define SIXTY										60
#define SIXTYONE									61
#define SIXTYTWO									62
#define SIXTYTHREE									63
#define SIXTYFOUR									64
#define SIXTYFIVE									65
#define SEVENTYSIX									76
#define EIGHTY										80
#define EIGHTYONE									81
#define EIGHTTWO									82
#define EIGHTYTHREE									83
#define EIGHTYFOUR									84
#define EIGHTYFIVE									85
#define ONEHUNDRED									100
#define ONETWENTYEIGHT								128
#define	ONEHUNDREDSIXTY								160
#define ONEHUNDREDSIXTYSIX							166
#define ONEEIGHTY									180
#define TWOTHIRTYSIX								236
#define TWOTHIRTYNINE								239
#define TWOFIFTYFIVE								255
#define TWOFIFTYSIX									256
#define TWOFIFTYSEVEN								257
#define TWOHUNDREDEIGHTY							280
#define TWONINTYEIGHT								298
#define THREEHUNDREDFOUR							304
#define THREETWENTY									320
#define THREETWENTYONE								321
#define THREETWENTYEIGHT							328
#define THREETHIRTYSIX								336
#define THREETHIRTYSEVEN							337
#define THREETHIRTYEIGHT							338
#define THREETHIRTYNINE								339
#define THREEFORTY									340
#define THREEFIFTY									350
#define FIVEHUNDREDTWELVE							512
#define FIVEHUNDREDTHIRTEEN							513
#define FIVEHUNDREDFOURTEEN							514
#define SIXFIFTYEIGHT								658
#define ONETHOUSAND									1000
#define ONETHOUSANDFIFTYEIGHT						1058

#define BOOT										ZERO
#define REBOOT										ONE

#define NOT_SUPPORTED_YET							ZERO
#define ENABLED										true
#define ACTIVE										ENABLED
#define ACTIVATED									ENABLED
#define INTERRUPT_ENABLED							ONE
#define DISABLED									false
#define DEACTIVATED									DISABLED
#define DOUBT										DISABLED
#define INTERRUPT_DISABLED							ZERO
#define YES											true
#define NO											false
#define DONT_CLOSE									true
#define CLOSE										false
#define NOT_USED									false
#define USED										true
#define NOT_READY									false
#define READY										true
#define SUCCESS										true
#define FAILURE										false
#define QUIT										false
#define ON											true
#define OFF											false
#define LO											ZERO
#define HI											ONE
#define RESET										ZERO
#define SET											ONE
#define CLEAR										false
#define INVALID										-ONE
#define AUTO										-ONE
#define RETRY										-ONE
#define VALID										ONE
#define ALWAYS										YES
#define COMPLETE									true
#define NOT_COMPLETE								false
#define PREV										ZERO
#define CURR										ONE
#define NEXT										TWO

#define REWIND_A									FALSE
#define REWIND_B									TRUE
#define TOTAL_REWINDS								TWO

#define BIT0										ZERO
#define BIT1										ONE
#define BIT2										TWO
#define BIT3										THREE
#define BIT4										FOUR
#define BIT5										FIVE
#define BIT6										SIX
#define BIT7										SEVEN

#define BIT0_SET									0b00000001
#define BIT1_SET									0b00000010
#define BIT2_SET									0b00000100
#define BIT3_SET									0b00001000
#define BIT4_SET									0b00010000
#define BIT5_SET									0b00100000
#define BIT6_SET									0b01000000
#define BIT7_SET									0b10000000

#define SIGNAL										bool
#define BIT											byte
#define BYTE										uint8_t
#define SBYTE										int8_t
#define SETBIT(val, n)								val|=(1ULL<<(n))
#define UNSETBIT(val, n)							val&=(~(1ULL<<(n)))
#define GETBIT(n, val)								(((val) >> (n)) & 1)
#define	CEIL(fValue)								((uint64_t)((float)fValue + (float)0.9f))
#define IS_ODD(val)									((val & ONE) == ONE)
#define IS_EVEN(val)								((val & ONE) == ZERO)

#define UWORD_FROM_UBYTES(HI, LO)					(uint16_t)((HI << EIGHT) | LO)

#define FPS_SLOTS									SIXTY
#define DEFAULT_FPS									ONEHUNDRED*ONETHOUSAND
#define MUTE_AUDIO									static_cast<float>(ZERO)
#define INVALID_INTERRUPT_HANLDER_ADDRESS			ZERO
#define SINGLE_ROM_FILE								ONE
#define TEST_ROM_FILE								TWO
#define REPLAY_ROM_FILE								THREE
#define MAX_NUMBER_ROMS_FOR_SI						FOUR
#define MAX_NUMBER_ROMS_FOR_PM						TEN
#define MAX_NUMBER_ROMS_FOR_MSPM					THIRTEEN
#define MAX_NUMBER_ROMS_PER_PLATFORM				THIRTEEN

#define COMMENT										(ignored)

#define TO_UINT8									static_cast<uint8_t>
#define TO_UINT16									static_cast<uint16_t>
#define TO_UINT32									static_cast<uint32_t>
#define TO_UINT										static_cast<unsigned>

#define CONTINUE									continue
#define BREAK										break

#ifdef __EMSCRIPTEN__
#define GL_ASSERT(x)								if (!(x));
#else
#define GL_ASSERT(x)								if (!(x)) PAUSE;
#endif
#define GL_CALL(x)									GLClearError();\
	x;\
	GL_ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#ifdef __EMSCRIPTEN__
// Use JavaScript alert as pause on Emscripten (simplest no-SDL way)
#define PAUSE do { \
    emscripten_run_script("alert('Press OK to continue...');"); \
} while(0)

#elif defined(_WIN32) || defined(_WIN64)
#define PAUSE system("pause")

#elif defined(__GNUC__)
#define PAUSE do { \
    printf("Press Enter to continue...\n"); \
    fflush(stdout); \
    getchar(); \
} while(0)
#else
#define PAUSE do { \
    printf("Press any key to continue...\n"); \
} while(0)
#endif
#define RETURN										return
#define FOREVER										true
#define DO_NOTHING									/* Do Nothing */
#define STUB()										/* Do Nothing */
#define ASSERT										assert

#define IF_ADDRESS_WITHIN(CURRENT, START, END)		((CURRENT >= START) && (CURRENT <= END))
#define IF_ADDRESS_IS(CURRENT, ADDRESS)				(CURRENT == ADDRESS)

#define BLOCK(...)									if (ENABLED) {__VA_ARGS__;}
#define DEAD(...)									if (DISABLED) {__VA_ARGS__;}
#define CONDITIONAL(condition, ...)					if ((condition) == YES) {__VA_ARGS__;}
#define SEQUENCE(...)								do {__VA_ARGS__} while(ZERO);
#define RUN_FOR_(COUNT, ...)						for (unsigned counter = 0; counter < COUNT; counter++) {__VA_ARGS__;}

#define EMULATION_VOLUME							static_cast<float>(0.1)

#define WINDOW_POS_X								THIRTY
#define WINDOW_POS_Y								THIRTY

#define RETRO_MODE_X								ONETHOUSANDFIFTYEIGHT
#define RETRO_MODE_Y								TWONINTYEIGHT
#define RETRO_MODE_PX								ONE
#define RETRO_MODE_PY								ONE
#if (DISABLED)
#define WIN32_MODE_X								THREEFIFTY
#define WIN32_MODE_Y								ONEEIGHTY
#define WIN32_MODE_PX								ONE
#define WIN32_MODE_PY								ONE
#else
#define WIN32_MODE_X								TWOTHIRTYSIX
#define WIN32_MODE_Y								ONETWENTYEIGHT
#define WIN32_MODE_PX								ONE
#define WIN32_MODE_PY								ONE
#endif

#define ONE_MILLISECOND								ONE
#define ONE_SECOND									ONETHOUSAND

#define LOG_COLOR_NO_COLOR							""
#define LOG_COLOR_BLACK								"\033[0;30m"
#define LOG_COLOR_RED								"\033[0;31m"
#define LOG_COLOR_GREEN								"\033[0;32m"
#define LOG_COLOR_YELLOW							"\033[0;33m"
#define LOG_COLOR_BLUE								"\033[0;34m"
#define LOG_COLOR_MAGENTA							"\033[0;35m"
#define LOG_COLOR_CYAN								"\033[0;36m"
#define LOG_COLOR_WHITE								"\033[0;37m"
#define LOG_COLOR_END								"\033[0;m"

#define LOG_COLOR_BOLD_BLACK						"\033[1;30m"
#define LOG_COLOR_BOLD_RED							"\033[1;31m"
#define LOG_COLOR_BOLD_GREEN						"\033[1;32m"
#define LOG_COLOR_BOLD_YELLOW						"\033[1;33m"
#define LOG_COLOR_BOLD_BLUE							"\033[1;34m"
#define LOG_COLOR_BOLD_MAGENTA						"\033[1;35m"
#define LOG_COLOR_BOLD_CYAN							"\033[1;36m"
#define LOG_COLOR_BOLD_WHITE						"\033[1;37m"

#define LOG_COLOR_UND_BLACK							"\033[4;30m"
#define LOG_COLOR_UND_RED							"\033[4;31m"
#define LOG_COLOR_UND_GREEN							"\033[4;32m"
#define LOG_COLOR_UND_YELLOW						"\033[4;33m"
#define LOG_COLOR_UND_BLUE							"\033[4;34m"
#define LOG_COLOR_UND_MAGENTA						"\033[4;35m"
#define LOG_COLOR_UND_CYAN							"\033[4;36m"
#define LOG_COLOR_UND_WHITE							"\033[4;37m"

#define LOG_COLOR_BCK_BLACK							"\033[40m"
#define LOG_COLOR_BCK_RED							"\033[41m"
#define LOG_COLOR_BCK_GREEN							"\033[42m"
#define LOG_COLOR_BCK_YELLOW						"\033[43m"
#define LOG_COLOR_BCK_BLUE							"\033[44m"
#define LOG_COLOR_BCK_MAGENTA						"\033[45m"
#define LOG_COLOR_BCK_CYAN							"\033[46m"
#define LOG_COLOR_BCK_WHITE							"\033[47m"

#define LOG_COLOR_INT_BLACK							"\033[0;90m"
#define LOG_COLOR_INT_RED							"\033[0;91m"
#define LOG_COLOR_INT_GREEN							"\033[0;92m"
#define LOG_COLOR_INT_YELLOW						"\033[0;93m"
#define LOG_COLOR_INT_BLUE							"\033[0;94m"
#define LOG_COLOR_INT_MAGENTA						"\033[0;95m"
#define LOG_COLOR_INT_CYAN							"\033[0;96m"
#define LOG_COLOR_INT_WHITE							"\033[0;97m"

#define LOG_COLOR_BOLD_INT_BLACK					"\033[1;90m"
#define LOG_COLOR_BOLD_INT_RED						"\033[1;91m"
#define LOG_COLOR_BOLD_INT_GREEN					"\033[1;92m"
#define LOG_COLOR_BOLD_INT_YELLOW					"\033[1;93m"
#define LOG_COLOR_BOLD_INT_BLUE						"\033[1;94m"
#define LOG_COLOR_BOLD_INT_MAGENTA					"\033[1;95m"
#define LOG_COLOR_BOLD_INT_CYAN						"\033[1;96m"
#define LOG_COLOR_BOLD_INT_WHITE					"\033[1;97m"

#define LOG_COLOR_INT_BCK_BLACK						"\033[0;100m"
#define LOG_COLOR_INT_BCK_RED						"\033[0;101m"
#define LOG_COLOR_INT_BCK_GREEN						"\033[0;102m"
#define LOG_COLOR_INT_BCK_YELLOW					"\033[0;103m"
#define LOG_COLOR_INT_BCK_BLUE						"\033[0;104m"
#define LOG_COLOR_INT_BCK_MAGENTA					"\033[0;105m"
#define LOG_COLOR_INT_BCK_CYAN						"\033[0;106m"
#define LOG_COLOR_INT_BCK_WHITE						"\033[0;107m"

#define LOG_VERBOSITY								ZERO
#define LOG_VERBOSITY_CPUWARN						ONE	
#define LOG_VERBOSITY_APUWARN						TWO	
#define LOG_VERBOSITY_PPUWARN						THREE	
#define LOG_VERBOSITY_CPUTODO						FOUR	
#define LOG_VERBOSITY_APUTODO						FIVE	
#define LOG_VERBOSITY_PPUTODO						SIX
#define LOG_VERBOSITY_CPUINFO						SEVEN	
#define LOG_VERBOSITY_APUINFO						EIGHT	
#define LOG_VERBOSITY_PPUINFO						NINE
#define LOG_VERBOSITY_CPUEVENT						TEN	
#define LOG_VERBOSITY_APUEVENT						ELEVEN	
#define LOG_VERBOSITY_PPUEVENT						TWELVE
#define LOG_VERBOSITY_CPUMOREINFO					THIRTEEN	
#define LOG_VERBOSITY_APUMOREINFO					FOURTEEN
#define LOG_VERBOSITY_PPUMOREINFO					FIFTEEN
#define LOG_VERBOSITY_DISASSEMBLY					SIXTEEN
#define LOG_VERBOSITY_CPUINFRA						SEVENTEEN
#define LOG_VERBOSITY_APUINFRA						EIGHTEEN
#define LOG_VERBOSITY_PPUINFRA						NINETEEN
#define LOG_VERBOSITY_CPUDEBUG						TWENTY
#define LOG_VERBOSITY_APUDEBUG						TWENTYONE
#define LOG_VERBOSITY_PPUDEBUG						TWENTYTWO
#define LOG_VERBOSITY_WARN							FIFTYSEVEN
#define LOG_VERBOSITY_TODO							FIFTYEIGHT
#define LOG_VERBOSITY_INFO							FIFTYNINE
#define LOG_VERBOSITY_EVENT							SIXTY
#define LOG_VERBOSITY_MOREINFO						SIXTYONE	
#define LOG_VERBOSITY_INFRA							SIXTYTWO	
#define LOG_VERBOSITY_DEBUG							SIXTYTHREE		


#define LOG_NEW_LINE								logger("\n");
#define LOG(message, ...)							if (GETBIT(LOG_VERBOSITY, ENABLE_LOGS)) MASQ_UNLIKELY {logger(message "\n", ##__VA_ARGS__);}
#define FATAL(message,...)							if (ONE) {logger(LOG_COLOR_RED "[FATAL] " message "\n" LOG_COLOR_END, ##__VA_ARGS__); PAUSE;}
#define CPUWARN(message,...)						if (GETBIT(LOG_VERBOSITY_CPUWARN, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_YELLOW "[WARN]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define CPUTODO(message,...)						if (GETBIT(LOG_VERBOSITY_CPUTODO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BOLD_RED "[TODO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define CPUINFO(message,...)						if (GETBIT(LOG_VERBOSITY_CPUINFO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_CYAN "[INFO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define CPUEVENT(message,...)						if (GETBIT(LOG_VERBOSITY_CPUEVENT, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_GREEN "[EVENT] " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define CPUMOREINFO(message,...)					if (GETBIT(LOG_VERBOSITY_CPUMOREINFO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BLUE "[INFO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define CPUDEBUG(message,...)						if (GETBIT(LOG_VERBOSITY_CPUDEBUG, ENABLE_LOGS)) MASQ_UNLIKELY {logger("[DEBUG] " message "\n", ##__VA_ARGS__);}
#define CPUINFRA(message,...)						if (GETBIT(LOG_VERBOSITY_CPUINFRA, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_MAGENTA "[INFRA] " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define APUWARN(message,...)						if (GETBIT(LOG_VERBOSITY_APUWARN, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_YELLOW "[WARN]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define APUTODO(message,...)						if (GETBIT(LOG_VERBOSITY_APUTODO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BOLD_RED "[TODO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define APUINFO(message,...)						if (GETBIT(LOG_VERBOSITY_APUINFO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_CYAN "[INFO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define APUEVENT(message,...)						if (GETBIT(LOG_VERBOSITY_APUEVENT, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_GREEN "[EVENT] " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define APUMOREINFO(message,...)					if (GETBIT(LOG_VERBOSITY_APUMOREINFO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BLUE "[INFO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define APUDEBUG(message,...)						if (GETBIT(LOG_VERBOSITY_APUDEBUG, ENABLE_LOGS)) MASQ_UNLIKELY {logger("[DEBUG] " message "\n", ##__VA_ARGS__);}
#define APUINFRA(message,...)						if (GETBIT(LOG_VERBOSITY_APUINFRA, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_MAGENTA "[INFRA] " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define PPUWARN(message,...)						if (GETBIT(LOG_VERBOSITY_PPUWARN, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_YELLOW "[WARN]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define PPUTODO(message,...)						if (GETBIT(LOG_VERBOSITY_PPUTODO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BOLD_RED "[TODO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define PPUINFO(message,...)						if (GETBIT(LOG_VERBOSITY_PPUINFO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_CYAN "[INFO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define PPUEVENT(message,...)						if (GETBIT(LOG_VERBOSITY_PPUEVENT, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_GREEN "[EVENT] " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define PPUMOREINFO(message,...)					if (GETBIT(LOG_VERBOSITY_PPUMOREINFO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BLUE "[INFO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define PPUDEBUG(message,...)						if (GETBIT(LOG_VERBOSITY_PPUDEBUG, ENABLE_LOGS)) MASQ_UNLIKELY {logger("[DEBUG] " message "\n", ##__VA_ARGS__);}
#define PPUINFRA(message,...)						if (GETBIT(LOG_VERBOSITY_PPUINFRA, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_MAGENTA "[INFRA] " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define DISASSEMBLY(message,...)					if (GETBIT(LOG_VERBOSITY_DISASSEMBLY, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_CYAN "[INSTR] " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define WARN(message,...)							if (GETBIT(LOG_VERBOSITY_WARN, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BOLD_YELLOW "[WARN]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define TODO(message,...)							if (GETBIT(LOG_VERBOSITY_TODO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BOLD_RED "[TODO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define INFO(message,...)							if (GETBIT(LOG_VERBOSITY_INFO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BOLD_CYAN "[INFO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define EVENT(message,...)							if (GETBIT(LOG_VERBOSITY_EVENT, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BOLD_GREEN "[EVENT] " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define MOREINFO(message,...)						if (GETBIT(LOG_VERBOSITY_MOREINFO, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BOLD_BLUE "[INFO]  " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}
#define DEBUG(message,...)							if (GETBIT(LOG_VERBOSITY_DEBUG, ENABLE_LOGS)) MASQ_UNLIKELY {logger("[DEBUG] " message "\n", ##__VA_ARGS__);}
#define INFRA(message,...)							if (GETBIT(LOG_VERBOSITY_INFRA, ENABLE_LOGS)) MASQ_UNLIKELY {logger(LOG_COLOR_BOLD_MAGENTA "[INFRA] " message "\n" LOG_COLOR_END, ##__VA_ARGS__);}

#define MASQ_UINT8									0
#define MASQ_UINT16									1
#define MASQ_UINT32									2
#define MASQ_UINT64									3
#define MASQ_SINT8									4
#define MASQ_SINT16									5
#define MASQ_SINT32									6
#define MASQ_SINT64									7
#define MASQ_FLOAT32								8
#define MASQ_FLOAT64								9

#define MASQ_UNUSED(x)								TODO("TODO: Unused variable \"" #x "\"\n")
#define MASQ_INLINE									inline

#define MASQ_UNLIKELY								[[unlikely]]
#define MASQ_LIKELY									[[likely]]

// Themes
#define ENABLED_IMGUI_DEFAULT_THEME					NO
// Below 3 themes are ported from SkyEmu!
#define SE_THEME_DARK								0
#define SE_THEME_LIGHT								1
#define SE_THEME_BLACK								2
//
#define THEME_CUSTOM								3
#pragma endregion MACROS

#ifndef byte
typedef unsigned char								byte;
#endif

#ifndef errno_t
typedef int											errno_t;
#endif

typedef float										CHIP8_AUDIO_SAMPLE_TYPE;
typedef float										SPACEINVADERS_AUDIO_SAMPLE_TYPE;
typedef float										PACMAN_AUDIO_SAMPLE_TYPE;
typedef float										NES_AUDIO_SAMPLE_TYPE;
typedef float										GBC_AUDIO_SAMPLE_TYPE;
#if (GBA_AUDIO_SAMPLE_FORMAT == MASQ_FLOAT32)
typedef float										GBA_AUDIO_SAMPLE_TYPE;
#else
typedef int16_t										GBA_AUDIO_SAMPLE_TYPE;
#endif

typedef bool										FLAG;
typedef byte										BIT;
typedef uint8_t										BYTE;
typedef int8_t										SBYTE;

typedef uint8_t										MAP8;
typedef int8_t										INC8;
typedef uint8_t										STATE8;
typedef int8_t										SSTATE8;
typedef uint16_t									MAP16;
typedef int16_t										INC16;
typedef uint16_t									STATE16;
typedef uint32_t									MAP32;
typedef int32_t										INC32;
typedef uint32_t									STATE32;
typedef int32_t										SSTATE32;
typedef uint64_t									MAP64;
typedef int64_t										INC64;
typedef uint64_t									STATE64;

typedef uint8_t										ID8;
typedef uint32_t									ID;
typedef uint64_t									ID64;

typedef uint8_t									    DIM8;
typedef uint16_t									DIM16;
typedef uint32_t									DIM32;
typedef int16_t										SDIM16;
typedef int32_t										SDIM32;

typedef uint8_t										COUNTER8;
typedef uint16_t									COUNTER16;
typedef uint32_t									COUNTER32;
typedef uint64_t									COUNTER64;
typedef int8_t										SCOUNTER8;
typedef int16_t										SCOUNTER16;
typedef int32_t										SCOUNTER32;
typedef int64_t										SCOUNTER64;

extern MAP64 ENABLE_LOGS;

#ifndef ENABLE_OTA_EXECUTABLE
extern float emuWindowX;
extern float emuWindowY;
extern float emuWindowMaxX;
extern float emuWindowMaxY;

inline std::string StripAnsiColors(const std::string & input)
{
	std::string output;
	output.reserve(input.size());

	for (size_t i = 0; i < input.size(); ++i)
	{
		// Look for ESC character (ASCII 27 or '\033')
		if (input[i] == '\033' && i + 1 < input.size() && input[i + 1] == '[')
		{
			// Skip until 'm' or end of string
			i += 2;
			while (i < input.size() && input[i] != 'm')
			{
				++i;
			}
			// Skip the 'm' too
			continue;
		}
		output += input[i];
	}

	RETURN output;
}

struct ImGuiLogBuffer
{
	std::vector<std::string> lines;
	std::mutex mutex;

	void Add(const char* str)
	{
		std::lock_guard<std::mutex> lock(mutex);
		lines.emplace_back(str);
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(mutex);
		lines.clear();
	}

	void Draw()
	{
		ImVec2 start_pos = ImGui::GetCursorScreenPos();
		ImVec2 avail = ImGui::GetContentRegionAvail();

		// Draw black background rectangle for the log area
		ImGui::GetWindowDrawList()->AddRectFilled(
			start_pos,
			ImVec2(start_pos.x + avail.x, start_pos.y + avail.y),
			IM_COL32(0, 0, 0, 255) // black
		);

		// Set green text color
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));

		{
			std::lock_guard<std::mutex> lock(mutex);
			for (const auto& line : lines)
			{
#if (DISABLED)
				// Calculate text size
				ImVec2 text_size = ImGui::CalcTextSize(line.c_str());

				// Compute X so text is centered in available width
				float text_x = start_pos.x + (avail.x - text_size.x) * 0.5f;

				// Move cursor to the X position (keep Y the same)
				ImGui::SetCursorScreenPos(ImVec2(text_x, ImGui::GetCursorScreenPos().y));
#endif

				// Draw text
				ImGui::TextUnformatted(line.c_str());
			}
		}

		ImGui::PopStyleColor();
	}
};

// === Global Log Buffers ===
extern FLAG isAppLoggingEnabled;
extern ImGuiLogBuffer appLog;

// ImGui init tracking
extern bool isImGuiInitialized;

// Pre-init buffer
extern std::vector<std::string> preImGuiLogBuffer;
extern std::mutex preImGuiLogMutex;

// === Logging Function ===
inline void LogToImGui(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	std::string clean = StripAnsiColors(buffer);
	if (!clean.empty() && clean.back() == '\n')
		clean.pop_back();

	if (!isImGuiInitialized)
	{
		std::lock_guard<std::mutex> lock(preImGuiLogMutex);
		preImGuiLogBuffer.emplace_back(std::move(clean));
		RETURN;
	}

	appLog.Add(clean.c_str());
}

// === Call this right after ImGui is initialized ===
inline void FlushEarlyLogsToImGui()
{
	std::lock_guard<std::mutex> lock(preImGuiLogMutex);
	for (const auto& line : preImGuiLogBuffer)
	{
		appLog.Add(line.c_str());
	}
	preImGuiLogBuffer.clear();
	isImGuiInitialized = true;
}

enum class UI_MODE
{
	UI_OLC_RETRO = 0,
	UI_WIN32API,
};

enum class UI_MODE_STATUS
{
	UI_CLOSE = 0,
	UI_DONT_CLOSE,
	UI_REBOOT
};
#endif

// === Logger Entry Point ===
inline void logger(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

#ifndef ENABLE_OTA_EXECUTABLE
	if (isAppLoggingEnabled == YES)
	{
		va_list args_copy;
		va_copy(args_copy, args);

		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), fmt, args_copy);
		va_end(args_copy);


		LogToImGui("%s", buffer);
	}
#endif

	vprintf(fmt, args);

	va_end(args);
}

#ifndef __EMSCRIPTEN__
inline std::wstring to_wstring(const std::string& stringToConvert)
{
	std::wstring wideString =
		std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(stringToConvert);
	RETURN wideString;
}

inline bool doesFileExist(const std::string& name)
{
	struct stat buffer;
	RETURN(stat(name.c_str(), &buffer) == 0);
}
#endif

inline void blocking_delay_ms(int ms)
{
	auto start = std::chrono::steady_clock::now();
	while (std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - start)
		.count() < ms)
	{
		// Busy wait (CPU intensive!)
	}
}

inline void ifNoDirectoryThenCreate(std::string directory)
{
	// check if directory mentioned by "directory" exists, if not we need to explicitly create it
	if (!std::filesystem::is_directory(directory) || !std::filesystem::exists(directory))
	{
		std::filesystem::create_directories(directory); // create all parent directories recursively
	}
}

inline bool to_bool(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	std::istringstream is(str);
	bool b;
	is >> std::boolalpha >> b;
	RETURN b;
}

inline uint8_t countSetBits(int32_t number)
{
	const uint8_t nibble_to_bits[16]
		= { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

	int nibble = 0;
	if (0 == number)
		RETURN nibble_to_bits[0];

	// Find last nibble
	nibble = number & 0xf;

	// Use pre-stored values to find count
	// in last nibble plus recursively add
	// remaining nibbles.
	RETURN nibble_to_bits[nibble] + countSetBits(number >> 4);
}

inline bool isOddParity(int32_t number)
{
	RETURN(bool)(countSetBits(number) % 2);
}

// bit position starts from 0 to n - 1....

inline uint8_t getBit(uint32_t byte, uint8_t position)
{
	RETURN(byte >> position) & 1;
}

inline std::bitset<8> toBinary(int n)
{
	RETURN std::bitset<8>(n);
}

inline std::string hex(uint32_t n, uint8_t d)
{
	std::string s(d, '0');
	for (int i = d - 1; i >= 0; i--, n >>= 4)
		s[i] = "0123456789ABCDEF"[n & 0xF];
	RETURN s;
};

inline std::string toUpper(const std::string& str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { RETURN std::toupper(c); });
	RETURN result;
}

inline std::filesystem::path getexepath()
{
#ifdef __EMSCRIPTEN__
	// No executable path in WASM; return empty or fixed string
	return {};
#elif defined(_WIN32) || defined(_WIN64)
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, path, MAX_PATH);
	RETURN path;
#else
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	if (count > 0)
		RETURN std::string(result, count);
	else
		RETURN{};
#endif
}

inline int fast_positive_mod(const int input, const int ceil)
{
	// apply the modulo operator only when needed
	// (i.e. when the input is greater than the ceiling)
	RETURN input >= ceil ? input % ceil : input;
	// NB: the assumption here is that the numbers are positive
}

// For frequency of substring

inline void computeLPSArray(std::string pattern, size_t M, int lps[])
{

	// Length of the previous longest
	// prefix suffix
	int len = 0;
	int i = 1;
	lps[0] = 0; // lps[0] is always 0

	// The loop calculates lps[i] for
	// i = 1 to M-1
	while (i < M)
	{
		if (pattern[i] == pattern[len])
		{
			len++;
			lps[i] = len;
			i++;
		}
		else // (pat[i] != pat[len])
		{

			// This is tricky. Consider the example.
			// AAACAAAA and i = 7. The idea is similar
			// to search step.
			if (len != 0)
			{
				len = lps[len - 1];

				// Also, note that we do not
				// increment i here
			}
			else // if (len == 0)
			{
				lps[i] = len;
				i++;
			}
		}
	}
}

inline int KMPSearch(std::string pattern, std::string original)
{
	size_t M = pattern.length();
	size_t N = original.length();

	// Create lps[] that will hold the longest
	// prefix suffix values for pattern
	int lps[MAX_PATH];
	int j = 0; // index for pat[]

	// Preprocess the pattern (calculate lps[]
	// array)
	computeLPSArray(pattern, M, lps);

	int i = 0; // index for txt[]
	int res = 0;
	int next_i = 0;

	while (i < N)
	{
		if (pattern[j] == original[i])
		{
			j++;
			i++;
		}
		if (j == M)
		{

			// When we find pattern first time,
			// we iterate again to check if there
			// exists more pattern
			if ((j - 1) < (sizeof(lps) / sizeof(int)))
			{
				j = lps[j - 1];
			}
			res++;
		}

		// Mismatch after j matches
		else if (i < N && pattern[j] != original[i])
		{

			// Do not match lps[0..lps[j-1]]
			// characters, they will match anyway
			if (j != 0)
			{
				if ((j - 1) < (sizeof(lps) / sizeof(int)))
				{
					j = lps[j - 1];
				}
			}
			else
			{
				i = i + 1;
			}
		}
	}
	RETURN res;
}

inline void int8ToDouble(int8_t* input, double* output, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		output[i] = (double)input[i];
	}
}

inline void doubleToInt8(double* input, int8_t* output, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		if (input[i] > 127.0)
		{
			input[i] = 127.0;
		}
		else if (input[i] < -128.0)
		{
			input[i] = -128.0;
		}

		// convert
		output[i] = (int8_t)input[i];
	}
}

inline void int16ToDouble(int16_t* input, double* output, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		output[i] = (double)input[i];
	}
}

inline void doubleToInt16(double* input, int16_t* output, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		if (input[i] > 32767.0)
		{
			input[i] = 32767.0;
		}
		else if (input[i] < -32768.0)
		{
			input[i] = -32768.0;
		}

		// convert
		output[i] = (int16_t)input[i];
	}
}

inline void int32ToDouble(int32_t* input, double* output, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		output[i] = (double)input[i];
	}
}

inline void doubleToInt32(double* input, int32_t* output, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		if (input[i] > 2147483647.0)
		{
			input[i] = 2147483647.0;
		}
		else if (input[i] < -2147483648.0)
		{
			input[i] = -2147483648.0;
		}

		// convert
		output[i] = (int32_t)input[i];
	}
}

inline void int64ToDouble(int64_t* input, double* output, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		output[i] = (double)input[i];
	}
}

inline void doubleToInt64(double* input, int64_t* output, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		// convert
		output[i] = (int64_t)input[i];
	}
}

inline uint64_t signExtend64(uint64_t v, int currentNumberOfBits)
{
	if (v & (1ull << (currentNumberOfBits - 1)))
	{
		// Sign bit is set then fill upper bits with 1s
		return v | (~0ull << currentNumberOfBits);
	}
	else
	{
		// Sign bit not set then already correct
		return v;
	}
}

inline uint32_t signExtend32(uint32_t v, int currentNumberOfBits)
{
	if (v & (1u << (currentNumberOfBits - 1))) // if sign bit is set
	{
		return v | (~0u << currentNumberOfBits); // extend with 1s
	}
	else
	{
		return v; // positive, no change
	}
}

template <typename T>
inline std::string to_string_with_precision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	RETURN std::move(out).str();
}

inline int fopen_portable(FILE** file, const char* filename, const char* mode)
{
#ifdef _MSC_VER
	RETURN fopen_s(file, filename, mode);
#else
	* file = fopen(filename, mode);
	RETURN(*file != nullptr) ? 0 : errno;
#endif
}

inline int memcpy_portable(void* dest, size_t destSize, const void* src, size_t count)
{
#if defined(_MSC_VER)
	// Use secure version on MSVC
	RETURN memcpy_s(dest, destSize, src, count);
#else
	// Safe fallback for GCC/Clang/Emscripten
	if (!dest || !src)
	{
		RETURN 1; // EINVAL-like error
	}
	if (count > destSize)
	{
		RETURN 1; // Would overflow destination buffer
	}
	std::memcpy(dest, src, count);
	RETURN 0;
#endif
}

inline uint32_t ctz32_portable(uint32_t x)
{
#if defined(_MSC_VER)
	unsigned long idx;
	_BitScanForward(&idx, x);
	return idx;
#else
	return __builtin_ctz(x);
#endif
}

// -----------------------
// Extract ZIP using miniz
// -----------------------
#ifndef __EMSCRIPTEN__
inline void extract_all_to_current_dir(const char* zip_path)
{
	mz_zip_archive zip;
	memset(&zip, 0, sizeof(zip));

	if (!mz_zip_reader_init_file(&zip, zip_path, 0))
	{
		LOG("Failed to open zip archive: %s", zip_path);
		RETURN;
	}

	mz_uint file_count = mz_zip_reader_get_num_files(&zip);

	for (mz_uint i = 0; i < file_count; ++i)
	{
		if (mz_zip_reader_is_file_a_directory(&zip, i))
		{
			CONTINUE; // skip dirs (shouldn't be any)
		}

		char filename[512] = { 0 };
		if (!mz_zip_reader_get_filename(&zip, i, filename, sizeof(filename)))
		{
			LOG("Failed to get filename for index %u", i);
			CONTINUE;
		}

		if (!mz_zip_reader_extract_to_file(&zip, i, filename, 0))
		{
			LOG("Failed to extract: %s", filename);
		}
		else
		{
			LOG("Extracted: %s", filename);
		}
	}

	mz_zip_reader_end(&zip);
}

inline bool extract_zip(const std::filesystem::path& zip_path, const std::filesystem::path& dest_dir)
{
	mz_zip_archive zip_archive;
	memset(&zip_archive, 0, sizeof(zip_archive));

	if (!mz_zip_reader_init_file(&zip_archive, zip_path.string().c_str(), 0))
	{
		FATAL("Failed to open ZIP archive: %s", zip_path.string().c_str());
		return false;
	}

	int num_files = static_cast<int>(mz_zip_reader_get_num_files(&zip_archive));
	for (int i = 0; i < num_files; ++i)
	{
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
			continue;

		std::filesystem::path out_file = dest_dir / file_stat.m_filename;
		if (file_stat.m_is_directory)
		{
			std::filesystem::create_directories(out_file);
		}
		else
		{
			std::filesystem::create_directories(out_file.parent_path());
			if (!mz_zip_reader_extract_to_file(&zip_archive, i, out_file.string().c_str(), 0))
			{
				FATAL("Failed to extract file: %s", out_file.string().c_str());
			}
		}
	}

	mz_zip_reader_end(&zip_archive);
	return true;
}
#else
// NOTE: Sync to persistant FS needs to be done outside this function
inline int extract_all_to_persistent_dir(const char* zip_path, std::vector<std::string>& out_paths)
{
	mz_zip_archive zip;
	memset(&zip, 0, sizeof(zip));

	if (!mz_zip_reader_init_file(&zip, zip_path, 0))
	{
		LOG("Failed to open zip archive: %s", zip_path);
		RETURN 0;
	}

	mz_uint file_count = mz_zip_reader_get_num_files(&zip);
	int extracted_count = 0;

	for (mz_uint i = 0; i < file_count; ++i)
	{
		if (mz_zip_reader_is_file_a_directory(&zip, i))
		{
			CONTINUE;
		}

		char filename[512] = { 0 };
		if (!mz_zip_reader_get_filename(&zip, i, filename, sizeof(filename)))
		{
			LOG("Failed to get filename for index %u", i);
			CONTINUE;
		}

		char full_path[1024];
		snprintf(full_path, sizeof(full_path), "/persistent/%s", filename);

		if (!mz_zip_reader_extract_to_file(&zip, i, full_path, 0))
		{
			LOG("Failed to extract: %s", full_path);
		}
		else
		{
			LOG("Extracted: %s", full_path);
			out_paths.emplace_back(filename);
			++extracted_count;
		}
	}

	mz_zip_reader_end(&zip);

	RETURN extracted_count;
}
#endif

inline std::string get_extension(const std::string& filename)
{
	size_t dot_pos = filename.find_last_of('.');
	if (dot_pos == std::string::npos || dot_pos == 0)
		RETURN ""; // No extension or filename starts with dot
	RETURN filename.substr(dot_pos + 1);
}

#ifndef ENABLE_OTA_EXECUTABLE
enum class EMULATION_ID : uint8_t
{
	DEFAULT_ID = 0,
	CHIP8_ID,
	SPACE_INVADERS_ID,
	PACMAN_ID,
	NES_ID,
	GB_GBC_ID,
	SNES_ID,
	N64_ID,
	GBA_ID,
	GAMECUBE_ID,
	DS_ID,
	WII_ID,
	DS3_ID,	// 3DS
	WIIU_ID,
	SWITCH_ID,
	GAME_OF_LIFE_ID,
	TEST_CPU_ID,
	TOTAL_ID,
	ANY_ID,
	INVALID_ID
};

typedef struct
{
	FLAG _DEBUG_BP_V1;
	FLAG _DEBUG_CALLSTACK;
	FLAG _DEBUG_PPU_VIEWER_GUI;
	INC64 _DEBUG_PPU_VIEWER_GUI_TRIGGER;
	FLAG _DEBUG_LOGGER_CLI;
	MAP64 _DEBUG_LOGGER_CLI_MASK;
	FLAG _DEBUG_FPS;
	FLAG _DEBUG_GRAPHICS;
	FLAG _DEBUG_KEYPAD;
	FLAG _DEBUG_LUT;
	FLAG _DEBUG_MEMORY;
	FLAG _DEBUG_PROFILER;
	FLAG _DEBUG_REGISTERS;
	FLAG _DEBUG_SOUND;
	FLAG _DEBUG_STEP;
	INC64 _DEBUG_SKIP_CYCLE;
} debugConfig_t;

enum class ROM
{
	GAME_OF_LIFE = ZERO,
	CHIP8,
	SPACE_INVADERS,
	PAC_MAN,
	MS_PAC_MAN,
	NES,
	GAME_BOY,
	GAME_BOY_COLOR,
	GAME_BOY_ADVANCE,
	TEST_SST,
	TEST_ROM_COM,
	TEST_ROM_CIM,
	TEST_ROM_TAP,
	TEST_ROM_BIN,
	COMPARE,
	REPLAY,
	NO_ROM
};

enum class EMULATOR_THEME
{
	DARK = SE_THEME_DARK,
	LIGHT = SE_THEME_LIGHT,
	BLACK = SE_THEME_BLACK
};

extern int currentEmuTheme;

std::unordered_map<std::string, EMULATOR_THEME> const configToEmuThemes =
{
	{"DARK", EMULATOR_THEME::DARK}
	,{"LIGHT", EMULATOR_THEME::LIGHT}
	,{"BLACK", EMULATOR_THEME::BLACK}
};

std::unordered_map<EMULATOR_THEME, std::string> const emuThemesToConfig =
{
	{EMULATOR_THEME::DARK, "DARK"}
	,{EMULATOR_THEME::LIGHT, "LIGHT"}
	,{EMULATOR_THEME::BLACK, "BLACK"}
};

enum class VIDEO_FILTERS
{
	NO_FILTER = ZERO,
	BILINEAR_FILTER,
	LCD_FILTER,
	CRT_FILTER,
	MAX_FILTERS
};

extern VIDEO_FILTERS currEnVFilter;

std::unordered_map<std::string, VIDEO_FILTERS> const configToVFilters =
{
	{"NO_FILTER", VIDEO_FILTERS::NO_FILTER}
	,{"BILINEAR_FILTER", VIDEO_FILTERS::BILINEAR_FILTER}
	,{"LCD_FILTER", VIDEO_FILTERS::LCD_FILTER}
	,{"CRT_FILTER", VIDEO_FILTERS::CRT_FILTER}
};

std::unordered_map<VIDEO_FILTERS, std::string> const vFiltersToConfig =
{
	{VIDEO_FILTERS::NO_FILTER, "NO_FILTER"}
	,{VIDEO_FILTERS::BILINEAR_FILTER, "BILINEAR_FILTER"}
	,{VIDEO_FILTERS::LCD_FILTER, "LCD_FILTER"}
	,{VIDEO_FILTERS::CRT_FILTER, "CRT_FILTER"}
};

enum class PALETTE_ID
{
	PALETTE_1,
	PALETTE_2,
	PALETTE_3,
	PALETTE_4,
	PALETTE_MAX,
	NO_PALETTE
};

extern PALETTE_ID currEnGbPalette;
extern PALETTE_ID currEnGbcPalette;

std::unordered_map<std::string, PALETTE_ID> const configToGbPaletteID =
{
	{"GearBoy", PALETTE_ID::PALETTE_1}
	,{"Black/White", PALETTE_ID::PALETTE_2}
	,{"SameBoy", PALETTE_ID::PALETTE_3}
	,{"BGB", PALETTE_ID::PALETTE_4}
};

std::unordered_map<PALETTE_ID, std::string> const gbPaletteIDToConfig =
{
	{PALETTE_ID::PALETTE_1, "GearBoy"}
	,{PALETTE_ID::PALETTE_2, "Black/White"}
	,{PALETTE_ID::PALETTE_3, "SameBoy"}
	,{PALETTE_ID::PALETTE_4, "BGB"}
};

struct Pixel
{
	union
	{
		uint32_t n = 0xFF << TWENTYFOUR;
		struct {
			uint8_t r; uint8_t g; uint8_t b; uint8_t a;
		};
	};

	enum Mode {
		NORMAL, MASK, ALPHA, CUSTOM
	};

	Pixel();
	Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 0xFF);
	Pixel(uint32_t p);
	Pixel& operator = (const Pixel& v) = default;
	bool   operator ==(const Pixel& p) const;
	bool   operator !=(const Pixel& p) const;
	Pixel  operator * (const float i) const;
	Pixel  operator / (const float i) const;
	Pixel& operator *=(const float i);
	Pixel& operator /=(const float i);
	Pixel  operator + (const Pixel& p) const;
	Pixel  operator - (const Pixel& p) const;
	Pixel& operator +=(const Pixel& p);
	Pixel& operator -=(const Pixel& p);
	Pixel  operator * (const Pixel& p) const;
	Pixel& operator *=(const Pixel& p);
	Pixel  inv() const;
};

static const Pixel
GREY(192, 192, 192), DARK_GREY(128, 128, 128), VERY_DARK_GREY(64, 64, 64),
RED(255, 0, 0), DARK_RED(128, 0, 0), VERY_DARK_RED(64, 0, 0),
YELLOW(255, 255, 0), DARK_YELLOW(128, 128, 0), VERY_DARK_YELLOW(64, 64, 0),
GREEN(0, 255, 0), DARK_GREEN(0, 128, 0), VERY_DARK_GREEN(0, 64, 0),
CYAN(0, 255, 255), DARK_CYAN(0, 128, 128), VERY_DARK_CYAN(0, 64, 64),
BLUE(0, 0, 255), DARK_BLUE(0, 0, 128), VERY_DARK_BLUE(0, 0, 64),
MAGENTA(255, 0, 255), DARK_MAGENTA(128, 0, 128), VERY_DARK_MAGENTA(64, 0, 64),
WHITE(255, 255, 255), BLACK(0, 0, 0), BLANK(0, 0, 0, 0);

inline Pixel::Pixel()
{
	r = 0; g = 0; b = 0; a = 0xFF;
}

inline Pixel::Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	n = red | (green << 8) | (blue << 16) | (alpha << 24);
}

inline Pixel::Pixel(uint32_t p)
{
	n = p;
}

inline bool Pixel::operator==(const Pixel& p) const
{
	RETURN n == p.n;
}

inline bool Pixel::operator!=(const Pixel& p) const
{
	RETURN n != p.n;
}

inline Pixel Pixel::operator * (const float i) const
{
	float fR = std::min(255.0f, std::max(0.0f, float(r) * i));
	float fG = std::min(255.0f, std::max(0.0f, float(g) * i));
	float fB = std::min(255.0f, std::max(0.0f, float(b) * i));
	RETURN Pixel(uint8_t(fR), uint8_t(fG), uint8_t(fB), a);
}

inline Pixel Pixel::operator / (const float i) const
{
	float fR = std::min(255.0f, std::max(0.0f, float(r) / i));
	float fG = std::min(255.0f, std::max(0.0f, float(g) / i));
	float fB = std::min(255.0f, std::max(0.0f, float(b) / i));
	RETURN Pixel(uint8_t(fR), uint8_t(fG), uint8_t(fB), a);
}

inline Pixel  Pixel::operator + (const Pixel& p) const
{
	uint8_t nR = uint8_t(std::min(255, std::max(0, int(r) + int(p.r))));
	uint8_t nG = uint8_t(std::min(255, std::max(0, int(g) + int(p.g))));
	uint8_t nB = uint8_t(std::min(255, std::max(0, int(b) + int(p.b))));
	RETURN Pixel(nR, nG, nB, a);
}

inline Pixel  Pixel::operator - (const Pixel& p) const
{
	uint8_t nR = uint8_t(std::min(255, std::max(0, int(r) - int(p.r))));
	uint8_t nG = uint8_t(std::min(255, std::max(0, int(g) - int(p.g))));
	uint8_t nB = uint8_t(std::min(255, std::max(0, int(b) - int(p.b))));
	RETURN Pixel(nR, nG, nB, a);
}

inline Pixel Pixel::operator * (const Pixel& p) const
{
	uint8_t nR = uint8_t(std::min(255.0f, std::max(0.0f, float(r) * float(p.r) / 255.0f)));
	uint8_t nG = uint8_t(std::min(255.0f, std::max(0.0f, float(g) * float(p.g) / 255.0f)));
	uint8_t nB = uint8_t(std::min(255.0f, std::max(0.0f, float(b) * float(p.b) / 255.0f)));
	uint8_t nA = uint8_t(std::min(255.0f, std::max(0.0f, float(a) * float(p.a) / 255.0f)));
	RETURN Pixel(nR, nG, nB, nA);
}

inline Pixel Pixel::inv() const
{
	uint8_t nR = uint8_t(std::min(255, std::max(0, 255 - int(r))));
	uint8_t nG = uint8_t(std::min(255, std::max(0, 255 - int(g))));
	uint8_t nB = uint8_t(std::min(255, std::max(0, 255 - int(b))));
	RETURN Pixel(nR, nG, nB, a);
}

enum class SHADER_TYPE
{
	NONE = -ONE,
	VERTEX = ZERO,
	FRAGMENT = ONE
};

typedef struct
{
	std::string vertexSource;
	std::string fragmentSource;
} shaderProgramSource_t;

std::unordered_map<std::string, EMULATION_ID> const _fileExtentionToEmulationPlatform =
{
	{".gol", EMULATION_ID::GAME_OF_LIFE_ID}
	,{".ch8", EMULATION_ID::CHIP8_ID}
	,{".CH8", EMULATION_ID::CHIP8_ID}
	,{".c8", EMULATION_ID::CHIP8_ID}
	,{".C8", EMULATION_ID::CHIP8_ID}
	,{".sc8", EMULATION_ID::CHIP8_ID}
	,{".SC8", EMULATION_ID::CHIP8_ID}
	,{".xo8", EMULATION_ID::CHIP8_ID}
	,{".XO8", EMULATION_ID::CHIP8_ID}
	,{".e", EMULATION_ID::SPACE_INVADERS_ID}
	,{".f", EMULATION_ID::SPACE_INVADERS_ID}
	,{".g", EMULATION_ID::SPACE_INVADERS_ID}
	,{".h", EMULATION_ID::SPACE_INVADERS_ID}
	,{".E", EMULATION_ID::SPACE_INVADERS_ID}
	,{".F", EMULATION_ID::SPACE_INVADERS_ID}
	,{".G", EMULATION_ID::SPACE_INVADERS_ID}
	,{".H", EMULATION_ID::SPACE_INVADERS_ID}
	,{".1m", EMULATION_ID::PACMAN_ID}
	,{".3m", EMULATION_ID::PACMAN_ID}
	,{".4a", EMULATION_ID::PACMAN_ID}
	,{".5e", EMULATION_ID::PACMAN_ID}
	,{".5f", EMULATION_ID::PACMAN_ID}
	,{".6e", EMULATION_ID::PACMAN_ID}
	,{".6f", EMULATION_ID::PACMAN_ID}
	,{".6h", EMULATION_ID::PACMAN_ID}
	,{".6j", EMULATION_ID::PACMAN_ID}
	,{".7f", EMULATION_ID::PACMAN_ID}
	,{".1M", EMULATION_ID::PACMAN_ID}
	,{".3M", EMULATION_ID::PACMAN_ID}
	,{".4A", EMULATION_ID::PACMAN_ID}
	,{".5E", EMULATION_ID::PACMAN_ID}
	,{".5F", EMULATION_ID::PACMAN_ID}
	,{".6E", EMULATION_ID::PACMAN_ID}
	,{".6H", EMULATION_ID::PACMAN_ID}
	,{".6F", EMULATION_ID::PACMAN_ID}
	,{".6J", EMULATION_ID::PACMAN_ID}
	,{".7F", EMULATION_ID::PACMAN_ID}
	,{"", EMULATION_ID::PACMAN_ID}
	,{".nes", EMULATION_ID::NES_ID}
	,{".NES", EMULATION_ID::NES_ID}
	,{".gb", EMULATION_ID::GB_GBC_ID}
	,{".gbc", EMULATION_ID::GB_GBC_ID}
	,{".GB", EMULATION_ID::GB_GBC_ID}
	,{".GBC", EMULATION_ID::GB_GBC_ID}
	,{".gba", EMULATION_ID::GBA_ID}
	,{".GBA", EMULATION_ID::GBA_ID}
	,{".com", EMULATION_ID::TEST_CPU_ID}
	,{".COM", EMULATION_ID::TEST_CPU_ID}
	,{".cim", EMULATION_ID::TEST_CPU_ID}
	,{".CIM", EMULATION_ID::TEST_CPU_ID}
	,{".tap", EMULATION_ID::TEST_CPU_ID}
	,{".TAP", EMULATION_ID::TEST_CPU_ID}
	,{".bin", EMULATION_ID::TEST_CPU_ID}
	,{".BIN", EMULATION_ID::TEST_CPU_ID}
};

std::unordered_map<uint32_t, EMULATION_ID> const _numberOfRomsToEmulationPlatform =
{
	// Below mentioned pairs are never checked
	{1, EMULATION_ID::ANY_ID}	// Multiple platforms can have only 1 ROM input
	// Below mentioned pairs are checked in "getType"
	,{2, EMULATION_ID::TEST_CPU_ID}
	,{3, EMULATION_ID::ANY_ID} // Used for log based replay mode
	,{4, EMULATION_ID::SPACE_INVADERS_ID}
	,{10, EMULATION_ID::PACMAN_ID}
	,{13, EMULATION_ID::PACMAN_ID}
};

const uint8_t parityLUT[0x100] = {
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

enum class serialMsg : uint32_t
{
	Server_GetStatus,
	Server_GetPing,

	Client_Accepted,
	Client_AssignID,
	Client_RegisterWithServer,
	Client_UnregisterWithServer,

	Game_SendBit,
	Game_ReceiveBit,
	Game_HeartBeat,
};

typedef struct
{
	uint32_t ID;
	BIT data;
	SIGNAL heartBeat;
	uint16_t pad0;
} gameSerialData;

extern unsigned long crcTable[256];
extern double bufferForFIR[2048];

//

inline void createLUTForCRC()
{
	unsigned long POLYNOMIAL = 0xEDB88320;
	unsigned long remainder;
	uint8_t b = 0;
	do
	{
		// Start with the data byte
		remainder = b;
		for (unsigned long bit = 8; bit > 0; --bit)
		{
			if (remainder & 1)
				remainder = (remainder >> 1) ^ POLYNOMIAL;
			else
				remainder = (remainder >> 1);
		}
		crcTable[(size_t)b] = remainder;
	}
	while (0 != ++b);
}

inline unsigned long genCRC(uint8_t* p, size_t n)
{
	unsigned long crc = 0xfffffffful;
	size_t i;
	for (i = 0; i < n; i++)
		crc = crcTable[*p++ ^ (crc & 0xff)] ^ (crc >> 8);
	RETURN(~crc);
}

inline std::string getUniqueGameID(uint8_t* in, size_t length)
{
	unsigned long crc = ZERO;

	TODO("Use HASH instead of CRC");
	crc = genCRC(in, length);
	//LOG("CRC: 0x%X", crc);
	std::stringstream stream;
	stream << std::hex << crc;
	std::string gameID(stream.str());

	RETURN gameID;
}

inline std::string getSaveFileName(uint8_t* in, uint64_t length)
{
	std::string saveFile = getUniqueGameID(in, length);
	saveFile += ".battery.sram";
	RETURN saveFile;
}

inline std::string getRTCSaveName(uint8_t* in, uint64_t length)
{
	std::string rtcSave = getUniqueGameID(in, length);
	rtcSave += ".battery.rtc";
	RETURN rtcSave;
}

inline std::string getSaveStateName(uint8_t* in, uint64_t length)
{
	std::string saveFile = getUniqueGameID(in, length);
	saveFile += ".state";
	RETURN saveFile;
}

//

inline void firFilterInit(void)
{
	memset(bufferForFIR, ZERO, sizeof(bufferForFIR));
}

inline void firFilter(const double* coeffs, double* input, double* output, int length, int filterLength)
{
	double acc;     // accumulator for MACs
	const double* coeffp; // pointer to coefficients
	double* inputp; // pointer to input samples
	int n;
	int k;

	// put the new samples at the high end of the buffer
	memcpy(&bufferForFIR[filterLength - 1], input, length * sizeof(double));

	// apply the filter to each input sample
	for (n = 0; n < length; n++)
	{
		// calculate output n
		coeffp = coeffs;
		inputp = &bufferForFIR[filterLength - 1 + n];
		acc = 0;
		for (k = 0; k < filterLength; k++)
		{
			acc += (*coeffp++) * (*inputp--);
		}
		output[n] = acc;
	}

	// shift input samples back in time for next time
	memmove(&bufferForFIR[0], &bufferForFIR[length], (filterLength - 1) * sizeof(double));

}

//

inline void getMouseRelPosIfDocked(float* xpos, float* ypos, uint32_t emuScreenWidth, uint32_t emuScreenHeight)
{
	static const float upperborder = 8;
	static const float otherborder = 8;
	float maxX = emuWindowMaxX - otherborder;
	float maxY = emuWindowMaxY - otherborder;
	*xpos = ImGui::GetMousePos().x - emuWindowX;
	*ypos = ImGui::GetMousePos().y - emuWindowY;
	*xpos -= otherborder;
	*ypos -= upperborder;
	if ((*xpos + otherborder > maxX)
		||
		(*ypos + upperborder > maxY)
		||
		(*xpos < 0 || *ypos < 0))
	{
		*xpos = *ypos = 0;
	}
	*xpos = *xpos * emuScreenWidth / maxX;
	*ypos = *ypos * emuScreenHeight / maxY;
}

template <typename T>
inline void printQ(std::queue<T> q) {
	std::cout << "Queue: ";
	while (!q.empty())
	{
		std::cout << q.front() << " ";
		q.pop();
	}
	std::cout << std::endl;
}

// Function to write a deque to a file
inline void writeDequeToFile(const std::deque<std::string>& myDeque, const std::string& filename) {
	std::ofstream outputFile(filename);
	if (outputFile.is_open())
	{
		for (const std::string& element : myDeque)
		{
			outputFile << element << std::endl;
		}
		outputFile.close();
	}
	else
	{
		std::cerr << "Unable to open file for writing: " << filename << std::endl;
	}
}

// Function to read a deque from a file
inline std::deque<std::string> readDequeFromFile(const std::string& filename) {
	std::deque<std::string> myDeque;
	std::ifstream inputFile(filename);
	if (inputFile.is_open())
	{
		std::string line;
		while (std::getline(inputFile, line))
		{
			myDeque.push_back(line);
		}
		inputFile.close();
	}
	else
	{
		std::cerr << "Unable to open file for reading: " << filename << std::endl;
	}
	RETURN myDeque;
}

#if (GL_FIXED_FUNCTION_PIPELINE == NO)
const std::string defaultPassthroughVertexShaderSrc =
"#if defined(WEBGL)\n"
"#version 300 es\n"
"precision mediump float;\n"
"#else\n"
"#version 330 core\n"
"#endif\n"
"\n"
"layout(location = 0) in vec2 pos;\n"
"layout(location = 1) in vec2 uv;\n"
"\n"
"out vec2 TexCoord;\n"
"\n"
"void main() \n"
"{\n"
"    TexCoord = uv;\n"
"    gl_Position = vec4(pos, 0.0, 1.0);\n"
"}\n";

const std::string defaultPassthroughFragmentShaderSrc =
"#if defined(WEBGL)\n"
"#version 300 es\n"
"precision mediump float;\n"
"#else\n"
"#version 330 core\n"
"#endif\n"
"\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"\n"
"uniform sampler2D u_Texture;\n"
"\n"
"void main() \n"
"{\n"
"    FragColor = texture(u_Texture, TexCoord);\n"
"}\n";

const std::string defaultBlendVertexShaderSrc =
"#if defined(WEBGL)\n"
"#version 300 es\n"
"precision mediump float;\n"
"#else\n"
"#version 330 core\n"
"#endif\n"
"\n"
"layout(location = 0) in vec2 pos;\n"
"layout(location = 1) in vec2 uv;\n"
"\n"
"out vec2 TexCoord;\n"
"\n"
"void main()\n"
"{\n"
"    TexCoord = uv;\n"
"    gl_Position = vec4(pos, 0.0, 1.0);\n"
"}\n";

const std::string defaultBlendFragmentShaderSrc =
"#if defined(WEBGL)\n"
"#version 300 es\n"
"precision mediump float;\n"
"#else\n"
"#version 330 core\n"
"#endif\n"
"\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"\n"
"uniform sampler2D u_Texture;\n"
"uniform float u_Alpha;\n"
"uniform vec2 u_TexelSize;\n"
"\n"
"void main()\n"
"{\n"
"    // Tile dot matrix pattern once per pixel\n"
"    vec2 scaledUV = gl_FragCoord.xy * u_TexelSize;\n"
"    vec4 texColor = texture(u_Texture, scaledUV);\n"
"    FragColor = vec4(texColor.rgb, texColor.a * u_Alpha);\n"
"}\n";

inline void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

inline FLAG GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		LOG("[OpenGL Error] (%d) : in function %s (file %s) at line %u", error, function, file, line);
		RETURN FAILURE;
	}
	RETURN SUCCESS;
}

inline void stripBlock(std::string& source, const std::string& startTag, const std::string& endTag)
{
	size_t start = source.find(startTag);
	while (start != std::string::npos)
	{
		size_t end = source.find(endTag, start);
		if (end == std::string::npos) break;

		// Erase from start of #if to end of #else line
		source.erase(start, end + endTag.length() - start);

		// Find next block
		start = source.find(startTag);
	}
}

inline shaderProgramSource_t parseShader(const std::string& filepath)
{
	std::string vertex;
	std::string fragment;

	std::ifstream stream(filepath);
	if (!stream.is_open())
	{
		LOG("Failed to open shader file: %s", filepath.c_str());
		LOG("Switching to default shaders");

		if (filepath.find("blend") != std::string::npos)
		{
			vertex = defaultBlendVertexShaderSrc;
			fragment = defaultBlendFragmentShaderSrc;
		}
		else
		{
			vertex = defaultPassthroughVertexShaderSrc;
			fragment = defaultPassthroughFragmentShaderSrc;
		}
	}
	else
	{
		std::string line;
		std::stringstream ss[TWO];
		SHADER_TYPE type = SHADER_TYPE::NONE;
		while (getline(stream, line))
		{
			if (line.find("#shader") != std::string::npos)
			{
				if (line.find("vertex") != std::string::npos)
				{
					type = SHADER_TYPE::VERTEX;
				}
				else if (line.find("fragment") != std::string::npos)
				{
					type = SHADER_TYPE::FRAGMENT;
				}
			}
			else
			{
				ss[TO_UINT(type)] << line << '\n';
			}
		}

		vertex = ss[0].str();
		fragment = ss[1].str();
	}

	auto removeLine = [](std::string& src, const std::string& marker) 
		{
			size_t pos = 0;
			while ((pos = src.find(marker, pos)) != std::string::npos)
			{
				size_t end = src.find('\n', pos);
				src.erase(pos, (end == std::string::npos ? src.size() - pos : end - pos + 1));
			}
		};

#ifndef __EMSCRIPTEN__
	// Strip WebGL version if not running in WebGL
	stripBlock(vertex, "#if defined(WEBGL)", "#else");
	stripBlock(fragment, "#if defined(WEBGL)", "#else");
#else
	// Strip WebGL version if not running in WebGL
	stripBlock(vertex, "#else", "#endif");
	stripBlock(fragment, "#else", "#endif");
#endif

	removeLine(vertex, "#if defined(WEBGL)");
	removeLine(vertex, "#else");
	removeLine(vertex, "#endif");
	removeLine(fragment, "#if defined(WEBGL)");
	removeLine(fragment, "#else");
	removeLine(fragment, "#endif");

	RETURN{ vertex, fragment };
}

inline uint32_t compileShader(uint32_t type, const std::string& source)
{
	uint32_t id = glCreateShader(type);
	const char* src = source.c_str();
	GL_CALL(glShaderSource(id, 1, &src, nullptr));
	GL_CALL(glCompileShader(id));

	int32_t result;
	GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int32_t length;
		GL_CALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char)); // TODO: Can lead to stack overflow!
		GL_CALL(glGetShaderInfoLog(id, length, &length, message));
		LOG("Failed to compile %s shader!", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
		LOG("%s", message);
		GL_CALL(glDeleteShader(id));
		RETURN ZERO;
	}

	RETURN id;
}

inline uint32_t createShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	uint32_t program = glCreateProgram();
	uint32_t vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	uint32_t fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	if (!vs || !fs)
	{
		LOG("Shader creation failed. Program will not be linked.");
#ifdef __EMSCRIPTEN__
		emscripten_run_script("alert('Shader creation failed!')");
#endif
		RETURN TO_UINT32(NULL);
	}

	GL_CALL(glAttachShader(program, vs));
	GL_CALL(glAttachShader(program, fs));
	GL_CALL(glLinkProgram(program));
	GL_CALL(glValidateProgram(program));

	GL_CALL(glDeleteShader(vs));
	GL_CALL(glDeleteShader(fs));

	RETURN program;
}
#endif

#if DEACTIVATED
inline int EstimateConvertedOutputBytes(
	int inputBytes,
	int inputRate,
	SDL_AudioFormat inputFormat,
	int inputChannels,
	int outputRate,
	SDL_AudioFormat outputFormat,
	int outputChannels
) {
	// Bytes per input frame
	int inputSampleSize = SDL_AUDIO_BITSIZE(inputFormat) / 8;
	if (SDL_AUDIO_ISFLOAT(inputFormat) || SDL_AUDIO_ISSIGNED(inputFormat))
	{
		// valid
	}
	else
	{
		return -1; // Unsupported input format
	}
	int inputFrameSize = inputSampleSize * inputChannels;

	if (inputFrameSize == 0) return -1;

	int inputFrames = inputBytes / inputFrameSize;

	// Resample
	double resampleRatio = (double)outputRate / inputRate;
	int outputFrames = (int)(inputFrames * resampleRatio + 0.5);

	// Bytes per output frame
	int outputSampleSize = SDL_AUDIO_BITSIZE(outputFormat) / 8;
	if (SDL_AUDIO_ISFLOAT(outputFormat) || SDL_AUDIO_ISSIGNED(outputFormat))
	{
		// valid
	}
	else
	{
		return -1; // Unsupported output format
	}

	int outputFrameSize = outputSampleSize * outputChannels;
	return outputFrames * outputFrameSize;
}

inline int CalculateMaxSafeBufferSize(SDL_AudioDeviceID device, float duration_seconds) 
{
	SDL_AudioSpec outputSpec;
	if (SDL_GetAudioDeviceFormat(device, &outputSpec, NULL) < 0)
	{
		SDL_Log("Failed to get output device format: %s", SDL_GetError());
		return -1;
	}

	int bytes_per_sample = SDL_AUDIO_BITSIZE(outputSpec.format) / 8;

	// Validate format
	if (!SDL_AUDIO_ISFLOAT(outputSpec.format) && !SDL_AUDIO_ISSIGNED(outputSpec.format))
	{
		SDL_Log("Unsupported format: 0x%X", outputSpec.format);
		return -1;
	}

	int bytes_per_frame = bytes_per_sample * outputSpec.channels;
	int max_safe_bytes = (int)(outputSpec.freq * duration_seconds * bytes_per_frame);

	return max_safe_bytes;
}
#endif

extern ROM ROM_TYPE;
inline FLAG isCLI()
{
	FLAG modeCLI = ((ROM_TYPE == ROM::TEST_ROM_COM)
		|| (ROM_TYPE == ROM::TEST_ROM_CIM)
		|| (ROM_TYPE == ROM::TEST_ROM_TAP)
		|| (ROM_TYPE == ROM::TEST_ROM_BIN)
		|| (ROM_TYPE == ROM::TEST_SST));

	RETURN modeCLI;
}
#endif

//--------------------------------------------------------------------------------------------------------------//
class SHA1_CUSTOM
{
public:
	SHA1_CUSTOM() {
		reset();
	}

	void update(const uint8_t* data, size_t len)
	{
		for (size_t i = 0; i < len; ++i)
		{
			m_block[m_blockByteIndex++] = data[i];
			m_messageByteLength++;

			if (m_blockByteIndex == 64)
			{
				processBlock();
				m_blockByteIndex = 0;
			}
		}
	}

	std::array<uint8_t, 20> digest()
	{
		uint64_t totalBits = m_messageByteLength * 8;

		// Ensure we never write past the end
		if (m_blockByteIndex >= 64)
		{
			processBlock();
			m_blockByteIndex = 0;
		}

		// Append 0x80
		m_block[m_blockByteIndex++] = 0x80;

		// If not enough space for length, pad and process
		if (m_blockByteIndex > 56)
		{
			while (m_blockByteIndex < 64)
				m_block[m_blockByteIndex++] = 0;
			processBlock();
			m_blockByteIndex = 0;
		}

		// Pad with zeros until 56 bytes
		while (m_blockByteIndex < 56)
			m_block[m_blockByteIndex++] = 0;

		// Append total message length (8 bytes big-endian)
		for (int i = 7; i >= 0; --i)
			m_block[m_blockByteIndex++] = static_cast<uint8_t>((totalBits >> (i * 8)) & 0xFF);

		processBlock();

		// Convert m_h to bytes
		std::array<uint8_t, 20> hash{};
		for (int i = 0; i < 5; ++i)
		{
			hash[i * 4 + 0] = static_cast<uint8_t>((m_h[i] >> 24) & 0xFF);
			hash[i * 4 + 1] = static_cast<uint8_t>((m_h[i] >> 16) & 0xFF);
			hash[i * 4 + 2] = static_cast<uint8_t>((m_h[i] >> 8) & 0xFF);
			hash[i * 4 + 3] = static_cast<uint8_t>((m_h[i]) & 0xFF);
		}

		return hash;
	}

	static std::string toHexString(const std::array<uint8_t, 20>& digest)
	{
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		for (uint8_t b : digest)
			ss << std::setw(2) << static_cast<int>(b);
		return ss.str();
	}

private:
	void reset()
	{
		m_h[0] = 0x67452301;
		m_h[1] = 0xEFCDAB89;
		m_h[2] = 0x98BADCFE;
		m_h[3] = 0x10325476;
		m_h[4] = 0xC3D2E1F0;

		m_blockByteIndex = 0;
		m_messageByteLength = 0;
		std::memset(m_block.data(), 0, m_block.size());
	}

	void processBlock()
	{
		uint32_t w[80]{};
		for (int i = 0; i < 16; ++i)
		{
			w[i] = (m_block[i * 4 + 0] << 24) |
				(m_block[i * 4 + 1] << 16) |
				(m_block[i * 4 + 2] << 8) |
				(m_block[i * 4 + 3]);
		}
		for (int i = 16; i < 80; ++i)
			w[i] = leftrotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

		uint32_t a = m_h[0], b = m_h[1], c = m_h[2], d = m_h[3], e = m_h[4];

		for (int i = 0; i < 80; ++i)
		{
			uint32_t f, k;
			if (i < 20)
			{
				f = (b & c) | ((~b) & d); k = 0x5A827999;
			}
			else if (i < 40)
			{
				f = b ^ c ^ d; k = 0x6ED9EBA1;
			}
			else if (i < 60)
			{
				f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC;
			}
			else
			{
				f = b ^ c ^ d; k = 0xCA62C1D6;
			}

			uint32_t temp = leftrotate(a, 5) + f + e + k + w[i];
			e = d; d = c; c = leftrotate(b, 30); b = a; a = temp;
		}

		m_h[0] += a; m_h[1] += b; m_h[2] += c; m_h[3] += d; m_h[4] += e;
	}

	static uint32_t leftrotate(uint32_t value, uint32_t bits)
	{
		return (value << bits) | (value >> (32 - bits));
	}

private:
	uint32_t m_h[5]{};
	std::array<uint8_t, 64> m_block{};
	size_t m_blockByteIndex{};
	size_t m_messageByteLength{};
};
