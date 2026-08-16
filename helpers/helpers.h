#pragma once

// =========================================================
// PRE-DEFINES
// =========================================================
#pragma region ADDITIONAL_FUNCTIONALITIES_PRE
#define UNICODE                                     1
#define VERSION                                     static_cast<float>(0.7014)
#ifndef __RPI_PICO__
#define WINDOW_PADDING                              16          // pixels added around SDL window
#else
#define WINDOW_PADDING                              0           // no padding on Pico
#endif
#define NOMINMAX
#define GL_FIXED_FUNCTION_PIPELINE                  NO
#pragma endregion ADDITIONAL_FUNCTIONALITIES_PRE

// =========================================================
// PLATFORM INCLUDES
//
// __RPI_PICO__ : bare-metal Pico SDK headers only.
//                stdio_init_all() must be called in main()
//                to enable UART printf output.
// desktop      : Windows compat, full C++ STL, Boost, SDL…
// =========================================================
#pragma region PLATFORM_INCLUDES

#ifdef __RPI_PICO__

// ---- Pico SDK -----------------------------------------
#include "pico/stdlib.h"        // stdio_init_all, sleep_ms
#include "pico/time.h"          // sleep_ms
#include "pico/multicore.h"     // multicore support (optional — only if you use core1 for offloading tasks)
#include "pico/sync.h"          // mutexes, critical sections, etc. for multicore synchronization

// ---- Waveshare -----------------------------------------
extern "C" {
    #include "LCD_2in.h"
}

// ---- Minimal C / C++ headers for bare-metal ----------
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <memory>
#include <array>
#include <string>               // lightweight use only
#include <algorithm>            // std::transform, std::min/max
#include <bitset>
#include <inttypes.h>           // PRIu64
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// ---- Others ------------------------------------------

#include "pico_config.h"
#include "pico_rom.h"

#else // !__RPI_PICO__ — full desktop / Emscripten build

// ---- Windows platform fix ----------------------------
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>           // MUST come before Windows.h
#include <ws2tcpip.h>
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

// ---- C++ Standard Library ----------------------------
#pragma region INCLUDES
#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <chrono>
#include <codecvt>
#include <cstdint>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <inttypes.h>
#include <locale>
#include <memory>
#include <math.h>
#include <mutex>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#pragma endregion INCLUDES

// ---- Boost -------------------------------------------
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
#include <boost/process.hpp>
#include <boost/asio/io_context.hpp>
#endif
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/throw_exception.hpp>
#pragma endregion BOOST_INCLUDES

// ---- Emscripten / ImGui / GLAD / SDL / NFD -----------
#ifndef ENABLE_OTA_EXECUTABLE
#pragma region EMSCRIPTEN_INCLUDES
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/html5.h>
#include <dirent.h>
#include "emscripten_browser_file.h"
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
#endif
#pragma endregion NFD_INCLUDES
#endif // !ENABLE_OTA_EXECUTABLE

// ---- Miniz / RapidJSON (desktop only) ----------------
#pragma region MINIZ_INCLUDES
#include <miniz.h>
#pragma endregion MINIZ_INCLUDES

#pragma region RAPIDJSON_INCLUDES
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#pragma endregion RAPIDJSON_INCLUDES

#endif // __RPI_PICO__
#pragma endregion PLATFORM_INCLUDES

// =========================================================
// MACROS — platform-independent unless noted
// =========================================================
#pragma region MACROS
#ifndef FALSE
#define FALSE                                       0
#endif
#ifndef TRUE
#define TRUE                                        1
#endif
#ifndef ERROR
#define ERROR                                       0
#endif
#ifndef MAX_PATH
#define MAX_PATH                                    260
#endif

// --- Numeric constants --------------------------------
#define ZERO                                        0
#define ONE                                         1
#define TWO                                         2
#define THREE                                       3
#define FOUR                                        4
#define FIVE                                        5
#define SIX                                         6
#define SEVEN                                       7
#define EIGHT                                       8
#define NINE                                        9
#define TEN                                         10
#define ELEVEN                                      11
#define TWELVE                                      12
#define THIRTEEN                                    13
#define FOURTEEN                                    14
#define FIFTEEN                                     15
#define SIXTEEN                                     16
#define SEVENTEEN                                   17
#define EIGHTEEN                                    18
#define NINETEEN                                    19
#define TWENTY                                      20
#define TWENTYONE                                   21
#define TWENTYTWO                                   22
#define TWENTYTHREE                                 23
#define TWENTYFOUR                                  24
#define TWENTYFIVE                                  25
#define TWENTYSIX                                   26
#define TWENTYSEVEN                                 27
#define TWENTYEIGHT                                 28
#define TWENTYNINE                                  29
#define THIRTY                                      30
#define THIRTYONE                                   31
#define THIRTYTWO                                   32
#define THIRTYTHREE                                 33
#define THIRTYFOUR                                  34
#define THIRTYFIVE                                  35
#define THIRTYSIX                                   36
#define THIRTYSEVEN                                 37
#define THIRTYEIGHT                                 38
#define THIRTYNINE                                  39
#define FORTY                                       40
#define FORTYONE                                    41
#define FORTYTWO                                    42
#define FORTYTHREE                                  43
#define FORTYFOUR                                   44
#define FORTYFIVE                                   45
#define FORTYSIX                                    46
#define FORTYSEVEN                                  47
#define FORTYEIGHT                                  48
#define FORTYNINE                                   49
#define FIFTY                                       50
#define FIFTYONE                                    51
#define FIFTYTWO                                    52
#define FIFTYTHREE                                  53
#define FIFTYFOUR                                   54
#define FIFTYFIVE                                   55
#define FIFTYSIX                                    56
#define FIFTYSEVEN                                  57
#define FIFTYEIGHT                                  58
#define FIFTYNINE                                   59
#define SIXTY                                       60
#define SIXTYONE                                    61
#define SIXTYTWO                                    62
#define SIXTYTHREE                                  63
#define SIXTYFOUR                                   64
#define SIXTYFIVE                                   65
#define SIXTYSIX                                    66
#define SIXTYSEVEN                                  67
#define SIXTYEIGHT                                  68
#define SIXTYNINE                                   69
#define SEVENTY                                     70
#define SEVENTYONE                                  71
#define SEVENTYTWO                                  72
#define SEVENTYTHREE                                73
#define SEVENTYFOUR                                 74
#define SEVENTYFIVE                                 75
#define SEVENTYSIX                                  76
#define SEVENTYSEVEN                                77
#define SEVENTYEIGHT                                78
#define SEVENTYNINE                                 79
#define EIGHTY                                      80
#define EIGHTYONE                                   81
#define EIGHTYTWO                                   82
#define EIGHTYTHREE                                 83
#define EIGHTYFOUR                                  84
#define EIGHTYFIVE                                  85
#define EIGHTYSIX                                   86
#define EIGHTYSEVEN                                 87
#define EIGHTYEIGHT                                 88
#define EIGHTYNINE                                  89
#define NINETY                                      90
#define NINETYONE                                   91
#define NINETYTWO                                   92
#define NINETYTHREE                                 93
#define NINETYFOUR                                  94
#define NINETYFIVE                                  95
#define NINETYSIX                                   96
#define NINETYSEVEN                                 97
#define NINETYEIGHT                                 98
#define NINETYNINE                                  99
#define ONEHUNDRED                                  100
#define ONEHUNDREDONE                               101
#define ONEHUNDREDTWO                               102
#define ONEHUNDREDTHREE                             103
#define ONEHUNDREDFOUR                              104
#define ONEHUNDREDFIVE                              105
#define ONEHUNDREDSIX                               106
#define ONEHUNDREDSEVEN                             107
#define ONEHUNDREDEIGHT                             108
#define ONEHUNDREDNINE                              109
#define ONEHUNDREDTEN                               110
#define ONEHUNDREDELEVEN                            111
#define ONEHUNDREDTWELVE                            112
#define ONEHUNDREDTHIRTEEN                          113
#define ONEHUNDREDFOURTEEN                          114
#define ONEHUNDREDFIFTEEN                           115
#define ONEHUNDREDSIXTEEN                           116
#define ONEHUNDREDSEVENTEEN                         117
#define ONEHUNDREDEIGHTEEN                          118
#define ONEHUNDREDNINETEEN                          119
#define ONEHUNDREDTWENTY                            120
#define ONEHUNDREDTWENTYONE                         121
#define ONEHUNDREDTWENTYTWO                         122
#define ONEHUNDREDTWENTYTHREE                       123
#define ONEHUNDREDTWENTYFOUR                        124
#define ONEHUNDREDTWENTYFIVE                        125
#define ONEHUNDREDTWENTYSIX                         126
#define ONEHUNDREDTWENTYSEVEN                       127
#define ONETWENTYEIGHT                              128
#define ONEHUNDREDTWENTYNINE                        129
#define ONEHUNDREDTHIRTY                            130
#define ONEHUNDREDTHIRTYONE                         131
#define ONEHUNDREDTHIRTYTWO                         132
#define ONEHUNDREDTHIRTYTHREE                       133
#define ONEHUNDREDTHIRTYFOUR                        134
#define ONEHUNDREDTHIRTYFIVE                        135
#define ONEHUNDREDTHIRTYSIX                         136
#define ONEHUNDREDTHIRTYSEVEN                       137
#define ONEHUNDREDTHIRTYEIGHT                       138
#define ONEHUNDREDTHIRTYNINE                        139
#define ONEHUNDREDFORTY                             140
#define ONEHUNDREDFORTYONE                          141
#define ONEHUNDREDFORTYTWO                          142
#define ONEHUNDREDFORTYTHREE                        143
#define ONEHUNDREDFORTYFOUR                         144
#define ONEHUNDREDFORTYFIVE                         145
#define ONEHUNDREDFORTYSIX                          146
#define ONEHUNDREDFORTYSEVEN                        147
#define ONEHUNDREDFORTYEIGHT                        148
#define ONEHUNDREDFORTYNINE                         149
#define ONEHUNDREDFIFTY                             150
#define ONEHUNDREDFIFTYONE                          151
#define ONEHUNDREDFIFTYTWO                          152
#define ONEHUNDREDFIFTYTHREE                        153
#define ONEHUNDREDFIFTYFOUR                         154
#define ONEHUNDREDFIFTYFIVE                         155
#define ONEHUNDREDFIFTYSIX                          156
#define ONEHUNDREDFIFTYSEVEN                        157
#define ONEHUNDREDFIFTYEIGHT                        158
#define ONEHUNDREDFIFTYNINE                         159
#define ONEHUNDREDSIXTY                             160
#define ONEHUNDREDSIXTYONE                          161
#define ONEHUNDREDSIXTYTWO                          162
#define ONEHUNDREDSIXTYTHREE                        163
#define ONEHUNDREDSIXTYFOUR                         164
#define ONEHUNDREDSIXTYFIVE                         165
#define ONEHUNDREDSIXTYSIX                          166
#define ONEHUNDREDSIXTYSEVEN                        167
#define ONEHUNDREDSIXTYEIGHT                        168
#define ONEHUNDREDSIXTYNINE                         169
#define ONEHUNDREDSEVENTY                           170
#define ONEHUNDREDSEVENTYONE                        171
#define ONEHUNDREDSEVENTYTWO                        172
#define ONEHUNDREDSEVENTYTHREE                      173
#define ONEHUNDREDSEVENTYFOUR                       174
#define ONEHUNDREDSEVENTYFIVE                       175
#define ONEHUNDREDSEVENTYSIX                        176
#define ONEHUNDREDSEVENTYSEVEN                      177
#define ONEHUNDREDSEVENTYEIGHT                      178
#define ONEHUNDREDSEVENTYNINE                       179
#define ONEEIGHTY                                   180
#define ONEHUNDREDEIGHTYONE                         181
#define ONEHUNDREDEIGHTYTWO                         182
#define ONEHUNDREDEIGHTYTHREE                       183
#define ONEHUNDREDEIGHTYFOUR                        184
#define ONEHUNDREDEIGHTYFIVE                        185
#define ONEHUNDREDEIGHTYSIX                         186
#define ONEHUNDREDEIGHTYSEVEN                       187
#define ONEHUNDREDEIGHTYEIGHT                       188
#define ONEHUNDREDEIGHTYNINE                        189
#define ONEHUNDREDNINETY                            190
#define ONEHUNDREDNINETYONE                         191
#define ONEHUNDREDNINETYTWO                         192
#define ONEHUNDREDNINETYTHREE                       193
#define ONEHUNDREDNINETYFOUR                        194
#define ONEHUNDREDNINETYFIVE                        195
#define ONEHUNDREDNINETYSIX                         196
#define ONEHUNDREDNINETYSEVEN                       197
#define ONEHUNDREDNINETYEIGHT                       198
#define ONEHUNDREDNINETYNINE                        199
#define TWOHUNDRED                                  200
#define TWOHUNDREDONE                               201
#define TWOHUNDREDTWO                               202
#define TWOHUNDREDTHREE                             203
#define TWOHUNDREDFOUR                              204
#define TWOHUNDREDFIVE                              205
#define TWOHUNDREDSIX                               206
#define TWOHUNDREDSEVEN                             207
#define TWOHUNDREDEIGHT                             208
#define TWOHUNDREDNINE                              209
#define TWOHUNDREDTEN                               210
#define TWOHUNDREDELEVEN                            211
#define TWOHUNDREDTWELVE                            212
#define TWOHUNDREDTHIRTEEN                          213
#define TWOHUNDREDFOURTEEN                          214
#define TWOHUNDREDFIFTEEN                           215
#define TWOHUNDREDSIXTEEN                           216
#define TWOHUNDREDSEVENTEEN                         217
#define TWOHUNDREDEIGHTEEN                          218
#define TWOHUNDREDNINETEEN                          219
#define TWOHUNDREDTWENTY                            220
#define TWOHUNDREDTWENTYONE                         221
#define TWOHUNDREDTWENTYTWO                         222
#define TWOHUNDREDTWENTYTHREE                       223
#define TWOHUNDREDTWENTYFOUR                        224
#define TWOHUNDREDTWENTYFIVE                        225
#define TWOHUNDREDTWENTYSIX                         226
#define TWOHUNDREDTWENTYSEVEN                       227
#define TWOHUNDREDTWENTYEIGHT                       228
#define TWOHUNDREDTWENTYNINE                        229
#define TWOHUNDREDTHIRTY                            230
#define TWOHUNDREDTHIRTYONE                         231
#define TWOHUNDREDTHIRTYTWO                         232
#define TWOHUNDREDTHIRTYTHREE                       233
#define TWOHUNDREDTHIRTYFOUR                        234
#define TWOHUNDREDTHIRTYFIVE                        235
#define TWOTHIRTYSIX                                236
#define TWOHUNDREDTHIRTYSEVEN                       237
#define TWOHUNDREDTHIRTYEIGHT                       238
#define TWOTHIRTYNINE                               239
#define TWOHUNDREDFORTY                             240
#define TWOHUNDREDFORTYONE                          241
#define TWOHUNDREDFORTYTWO                          242
#define TWOHUNDREDFORTYTHREE                        243
#define TWOHUNDREDFORTYFOUR                         244
#define TWOHUNDREDFORTYFIVE                         245
#define TWOHUNDREDFORTYSIX                          246
#define TWOHUNDREDFORTYSEVEN                        247
#define TWOHUNDREDFORTYEIGHT                        248
#define TWOHUNDREDFORTYNINE                         249
#define TWOFIFTY                                    250
#define TWOFIFTYONE                                 251
#define TWOFIFTYTWO                                 252
#define TWOFIFTYTHREE                               253
#define TWOFIFTYFOUR                                254
#define TWOFIFTYFIVE                                255
#define TWOFIFTYSIX                                 256
#define TWOFIFTYSEVEN                               257
#define TWOHUNDREDSIXTYEIGHT                        268
#define TWOHUNDREDEIGHTY                            280
#define TWONINTYEIGHT                               298
#define THREEHUNDREDFOUR                            304
#define THREETWENTY                                 320
#define THREETWENTYONE                              321
#define THREETWENTYEIGHT                            328
#define THREETHIRTYSIX                              336
#define THREETHIRTYSEVEN                            337
#define THREETHIRTYEIGHT                            338
#define THREETHIRTYNINE                             339
#define THREEFORTY                                  340
#define THREEFIFTY                                  350
#define FIVEHUNDREDTWELVE                           512
#define FIVEHUNDREDTHIRTEEN                         513
#define FIVEHUNDREDFOURTEEN                         514
#define SIXFIFTYEIGHT                               658
#define ONETHOUSAND                                 1000
#define ONETHOUSANDFIFTYEIGHT                       1058

// --- Boot types ---------------------------------------
#define BOOT                                        ZERO
#define REBOOT                                      ONE

// --- Boolean / state aliases -------------------------
#define NOT_SUPPORTED_YET                           ZERO
#define ENABLED                                     true
#define ACTIVE                                      ENABLED
#define ACTIVATED                                   ENABLED
#define INTERRUPT_ENABLED                           ONE
#define DISABLED                                    false
#define DEACTIVATED                                 DISABLED
#define DOUBT                                       DISABLED
#define INTERRUPT_DISABLED                          ZERO
#define YES                                         true
#define NO                                          false
#define DONT_CLOSE                                  true
#define CLOSE                                       false
#define NOT_USED                                    false
#define USED                                        true
#define NOT_READY                                   false
#define READY                                       true
#define SUCCESS                                     true
#define FAILURE                                     false
#define QUIT                                        false
#define ON                                          true
#define OFF                                         false
#define LO                                          ZERO
#define HI                                          ONE
#define RESET                                       ZERO
#define SET                                         ONE
#define CLEAR                                       false
#define INVALID                                     -ONE
#define AUTO                                        -ONE
#define RETRY                                       -ONE
#define VALID                                       ONE
#define ALWAYS                                      YES
#define COMPLETE                                    true
#define NOT_COMPLETE                                false
#define PREV                                        ZERO
#define CURR                                        ONE
#define NEXT                                        TWO

// --- Rewind -------------------------------------------
#define REWIND_A                                    FALSE
#define REWIND_B                                    TRUE
#define TOTAL_REWINDS                               TWO

// --- Bit position aliases -----------------------------
#define BIT0                                        ZERO
#define BIT1                                        ONE
#define BIT2                                        TWO
#define BIT3                                        THREE
#define BIT4                                        FOUR
#define BIT5                                        FIVE
#define BIT6                                        SIX
#define BIT7                                        SEVEN

#define BIT0_SET                                    0b00000001
#define BIT1_SET                                    0b00000010
#define BIT2_SET                                    0b00000100
#define BIT3_SET                                    0b00001000
#define BIT4_SET                                    0b00010000
#define BIT5_SET                                    0b00100000
#define BIT6_SET                                    0b01000000
#define BIT7_SET                                    0b10000000

// --- Bit manipulation ---------------------------------
#define SIGNAL                                      bool
#define BIT                                         byte
#define BYTE                                        uint8_t
#define SBYTE                                       int8_t
#define SETBIT(val, n)                              val|=(1ULL<<(n))
#define UNSETBIT(val, n)                            val&=(~(1ULL<<(n)))
#define GETBIT(n, val)                              (((val) >> (n)) & 1)
#define CEIL(fValue)                                ((uint64_t)((float)fValue + (float)0.9f))
#define IS_ODD(val)                                 ((val & ONE) == ONE)
#define IS_EVEN(val)                                ((val & ONE) == ZERO)
#define UWORD_FROM_UBYTES(HI, LO)                  (uint16_t)((HI << EIGHT) | LO)

// --- Emulation timing ---------------------------------
#define FPS_SLOTS                                   SIXTY
#define DEFAULT_FPS                                 ONEHUNDRED*ONETHOUSAND
#define MUTE_AUDIO                                  static_cast<float>(ZERO)
#define INVALID_INTERRUPT_HANLDER_ADDRESS           ZERO
#define SINGLE_ROM_FILE                             ONE
#define TEST_ROM_FILE                               TWO
#define COMPARE_OR_REPLAY_ROM_FILE                  THREE
#define MAX_NUMBER_ROMS_FOR_SI                      FOUR
#define MAX_NUMBER_ROMS_FOR_PM                      TEN
#define MAX_NUMBER_ROMS_FOR_MSPM                    THIRTEEN
#define MAX_NUMBER_ROMS_PER_PLATFORM                THIRTEEN

#define COMMENT                                     (ignored)

// --- Cast shorthands ---------------------------------
#define TO_UINT8                                    static_cast<uint8_t>
#define TO_UINT16                                   static_cast<uint16_t>
#define TO_UINT32                                   static_cast<uint32_t>
#define TO_UINT                                     static_cast<unsigned>
#define TO_INT                                      static_cast<int>

// --- Flow control ------------------------------------
#define CONTINUE                                    continue
#define BREAK                                       break
#define FALLTHROUGH                                 ((void)0)

// --- OpenGL error helpers (desktop only) -------------
#ifndef __RPI_PICO__
#ifdef __EMSCRIPTEN__
#define GL_ASSERT(x)                                if (!(x));
#else
#define GL_ASSERT(x)                                if (!(x)) PAUSE;
#endif
#define GL_CALL(x)                                  GLClearError();\
    x;\
    GL_ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#endif // !__RPI_PICO__

// --- PAUSE: platform-specific -------------------------
#ifdef __RPI_PICO__
// On Pico: busy-wait (breakpoint via UART or debugger)
#define PAUSE                                       do { for(;;) tight_loop_contents(); } while(0)
#elif defined(__EMSCRIPTEN__)
#define PAUSE                                       do { emscripten_run_script("alert('Press OK to CONTINUE...');"); } while(0)
#elif defined(_WIN32) || defined(_WIN64)
#define PAUSE                                       system("pause")
#elif defined(__GNUC__)
#define PAUSE                                       do { printf("Press Enter to CONTINUE...\n"); fflush(stdout); getchar(); } while(0)
#else
#define PAUSE                                       do { printf("Press any key to CONTINUE...\n"); } while(0)
#endif

// --- Misc flow / utility macros ----------------------
#define RETURN                                      return
#define FOREVER                                     true
#define DO_NOTHING                                  /* Do Nothing */
#define STUB()                                      /* Do Nothing */
#define ASSERT                                      assert

#define IF_ADDRESS_WITHIN(CURRENT, START, END)      ((CURRENT >= START) && (CURRENT <= END))
#define IF_ADDRESS_IS(CURRENT, ADDRESS)             (CURRENT == ADDRESS)

#define BLOCK(...)                                  if (ENABLED) {__VA_ARGS__;}
#define DEAD(...)                                   if (DISABLED) {__VA_ARGS__;}
#define CONDITIONAL(condition, ...)                 if ((condition) == YES) {__VA_ARGS__;}
#define SEQUENCE(...)                               do {__VA_ARGS__} while(ZERO);
#define RUN_FOR_(COUNT, ...)                        for (unsigned counter = 0; counter < COUNT; counter++) {__VA_ARGS__;}

#define EMULATION_VOLUME                            static_cast<float>(0.1)

// --- Window defaults (desktop only) ------------------
#ifndef __RPI_PICO__
#define WINDOW_POS_X                                THIRTY
#define WINDOW_POS_Y                                THIRTY
#define RETRO_MODE_X                                ONETHOUSANDFIFTYEIGHT
#define RETRO_MODE_Y                                TWONINTYEIGHT
#define RETRO_MODE_PX                               ONE
#define RETRO_MODE_PY                               ONE
#define WIN32_MODE_X                                TWOTHIRTYSIX
#define WIN32_MODE_Y                                ONETWENTYEIGHT
#define WIN32_MODE_PX                               ONE
#define WIN32_MODE_PY                               ONE
#endif // !__RPI_PICO__

#define ONE_MILLISECOND                             ONE
#define ONE_SECOND                                  ONETHOUSAND

// --- ANSI log color codes ----------------------------
// On Pico these are harmless (most serial terminals honour them)
#define LOG_COLOR_NO_COLOR                          ""
#define LOG_COLOR_BLACK                             "\033[0;30m"
#define LOG_COLOR_RED                               "\033[0;31m"
#define LOG_COLOR_GREEN                             "\033[0;32m"
#define LOG_COLOR_YELLOW                            "\033[0;33m"
#define LOG_COLOR_BLUE                              "\033[0;34m"
#define LOG_COLOR_MAGENTA                           "\033[0;35m"
#define LOG_COLOR_CYAN                              "\033[0;36m"
#define LOG_COLOR_WHITE                             "\033[0;37m"
#define LOG_COLOR_END                               "\033[0;m"
#define LOG_COLOR_BOLD_BLACK                        "\033[1;30m"
#define LOG_COLOR_BOLD_RED                          "\033[1;31m"
#define LOG_COLOR_BOLD_GREEN                        "\033[1;32m"
#define LOG_COLOR_BOLD_YELLOW                       "\033[1;33m"
#define LOG_COLOR_BOLD_BLUE                         "\033[1;34m"
#define LOG_COLOR_BOLD_MAGENTA                      "\033[1;35m"
#define LOG_COLOR_BOLD_CYAN                         "\033[1;36m"
#define LOG_COLOR_BOLD_WHITE                        "\033[1;37m"
#define LOG_COLOR_UND_RED                           "\033[4;31m"
#define LOG_COLOR_UND_GREEN                         "\033[4;32m"
#define LOG_COLOR_UND_YELLOW                        "\033[4;33m"
#define LOG_COLOR_UND_BLUE                          "\033[4;34m"
#define LOG_COLOR_UND_CYAN                          "\033[4;36m"
#define LOG_COLOR_UND_WHITE                         "\033[4;37m"
#define LOG_COLOR_BCK_BLACK							"\033[40m"
#define LOG_COLOR_BCK_RED							"\033[41m"
#define LOG_COLOR_BCK_GREEN							"\033[42m"
#define LOG_COLOR_BCK_YELLOW						"\033[43m"
#define LOG_COLOR_BCK_BLUE							"\033[44m"
#define LOG_COLOR_BCK_MAGENTA						"\033[45m"
#define LOG_COLOR_BCK_CYAN							"\033[46m"
#define LOG_COLOR_BCK_WHITE							"\033[47m"
#define LOG_COLOR_INT_BLACK                         "\033[0;90m"
#define LOG_COLOR_INT_RED                           "\033[0;91m"
#define LOG_COLOR_INT_GREEN                         "\033[0;92m"
#define LOG_COLOR_INT_YELLOW                        "\033[0;93m"
#define LOG_COLOR_INT_BLUE                          "\033[0;94m"
#define LOG_COLOR_INT_MAGENTA                       "\033[0;95m"
#define LOG_COLOR_INT_CYAN                          "\033[0;96m"
#define LOG_COLOR_INT_WHITE                         "\033[0;97m"
#define LOG_COLOR_BOLD_INT_BLACK                    "\033[1;90m"
#define LOG_COLOR_BOLD_INT_RED                      "\033[1;91m"
#define LOG_COLOR_BOLD_INT_GREEN                    "\033[1;92m"
#define LOG_COLOR_BOLD_INT_YELLOW                   "\033[1;93m"
#define LOG_COLOR_BOLD_INT_BLUE                     "\033[1;94m"
#define LOG_COLOR_BOLD_INT_MAGENTA                  "\033[1;95m"
#define LOG_COLOR_BOLD_INT_CYAN                     "\033[1;96m"
#define LOG_COLOR_BOLD_INT_WHITE                    "\033[1;97m"
#define LOG_COLOR_INT_BCK_BLACK                     "\033[0;100m"
#define LOG_COLOR_INT_BCK_RED                       "\033[0;101m"
#define LOG_COLOR_INT_BCK_GREEN                     "\033[0;102m"
#define LOG_COLOR_INT_BCK_YELLOW                    "\033[0;103m"
#define LOG_COLOR_INT_BCK_BLUE                      "\033[0;104m"
#define LOG_COLOR_INT_BCK_MAGENTA                   "\033[0;105m"
#define LOG_COLOR_INT_BCK_CYAN                      "\033[0;106m"
#define LOG_COLOR_INT_BCK_WHITE                     "\033[0;107m"

// --- Log verbosity levels ----------------------------
#define LOG_VERBOSITY                               ZERO
#define LOG_VERBOSITY_CPUWARN                       ONE
#define LOG_VERBOSITY_APUWARN                       TWO
#define LOG_VERBOSITY_PPUWARN                       THREE
#define LOG_VERBOSITY_CPUTODO                       FOUR
#define LOG_VERBOSITY_APUTODO                       FIVE
#define LOG_VERBOSITY_PPUTODO                       SIX
#define LOG_VERBOSITY_CPUINFO                       SEVEN
#define LOG_VERBOSITY_APUINFO                       EIGHT
#define LOG_VERBOSITY_PPUINFO                       NINE
#define LOG_VERBOSITY_CPUEVENT                      TEN
#define LOG_VERBOSITY_APUEVENT                      ELEVEN
#define LOG_VERBOSITY_PPUEVENT                      TWELVE
#define LOG_VERBOSITY_CPUMOREINFO                   THIRTEEN
#define LOG_VERBOSITY_APUMOREINFO                   FOURTEEN
#define LOG_VERBOSITY_PPUMOREINFO                   FIFTEEN
#define LOG_VERBOSITY_DISASSEMBLY                   SIXTEEN
#define LOG_VERBOSITY_CPUINFRA                      SEVENTEEN
#define LOG_VERBOSITY_APUINFRA                      EIGHTEEN
#define LOG_VERBOSITY_PPUINFRA                      NINETEEN
#define LOG_VERBOSITY_CPUDEBUG                      TWENTY
#define LOG_VERBOSITY_APUDEBUG                      TWENTYONE
#define LOG_VERBOSITY_PPUDEBUG                      TWENTYTWO
#define LOG_VERBOSITY_WARN                          FIFTYSEVEN
#define LOG_VERBOSITY_TODO                          FIFTYEIGHT
#define LOG_VERBOSITY_INFO                          FIFTYNINE
#define LOG_VERBOSITY_EVENT                         SIXTY
#define LOG_VERBOSITY_MOREINFO                      SIXTYONE
#define LOG_VERBOSITY_INFRA                         SIXTYTWO
#define LOG_VERBOSITY_DEBUG                         SIXTYTHREE

// --- Log macros — identical on Pico and desktop ------
// logger() is the only function that differs (see below)
#define LOG_NEW_LINE do{logger("\n");}while(0)
#define LOG(message, ...) do{if(GETBIT(LOG_VERBOSITY,ENABLE_LOGS))MASQ_UNLIKELY{logger(message "\n",##__VA_ARGS__);}}while(0)
#define FATAL(message, ...) do{if(ONE){logger(LOG_COLOR_RED "[FATAL] " message "\n" LOG_COLOR_END,##__VA_ARGS__);PAUSE;}}while(0)
#define CPUWARN(message, ...) do{if(GETBIT(LOG_VERBOSITY_CPUWARN,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_YELLOW "[WARN]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define CPUTODO(message, ...) do{if(GETBIT(LOG_VERBOSITY_CPUTODO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BOLD_RED "[TODO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define CPUINFO(message, ...) do{if(GETBIT(LOG_VERBOSITY_CPUINFO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_CYAN "[INFO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define CPUEVENT(message, ...) do{if(GETBIT(LOG_VERBOSITY_CPUEVENT,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_GREEN "[EVENT] " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define CPUMOREINFO(message, ...) do{if(GETBIT(LOG_VERBOSITY_CPUMOREINFO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BLUE "[INFO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define CPUDEBUG(message, ...) do{if(GETBIT(LOG_VERBOSITY_CPUDEBUG,ENABLE_LOGS))MASQ_UNLIKELY{logger("[DEBUG] " message "\n",##__VA_ARGS__);}}while(0)
#define CPUINFRA(message, ...) do{if(GETBIT(LOG_VERBOSITY_CPUINFRA,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_MAGENTA "[INFRA] " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define APUWARN(message, ...) do{if(GETBIT(LOG_VERBOSITY_APUWARN,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_YELLOW "[WARN]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define APUTODO(message, ...) do{if(GETBIT(LOG_VERBOSITY_APUTODO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BOLD_RED "[TODO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define APUINFO(message, ...) do{if(GETBIT(LOG_VERBOSITY_APUINFO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_CYAN "[INFO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define APUEVENT(message, ...) do{if(GETBIT(LOG_VERBOSITY_APUEVENT,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_GREEN "[EVENT] " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define APUMOREINFO(message, ...) do{if(GETBIT(LOG_VERBOSITY_APUMOREINFO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BLUE "[INFO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define APUDEBUG(message, ...) do{if(GETBIT(LOG_VERBOSITY_APUDEBUG,ENABLE_LOGS))MASQ_UNLIKELY{logger("[DEBUG] " message "\n",##__VA_ARGS__);}}while(0)
#define APUINFRA(message, ...) do{if(GETBIT(LOG_VERBOSITY_APUINFRA,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_MAGENTA "[INFRA] " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define PPUWARN(message, ...) do{if(GETBIT(LOG_VERBOSITY_PPUWARN,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_YELLOW "[WARN]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define PPUTODO(message, ...) do{if(GETBIT(LOG_VERBOSITY_PPUTODO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BOLD_RED "[TODO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define PPUINFO(message, ...) do{if(GETBIT(LOG_VERBOSITY_PPUINFO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_CYAN "[INFO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define PPUEVENT(message, ...) do{if(GETBIT(LOG_VERBOSITY_PPUEVENT,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_GREEN "[EVENT] " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define PPUMOREINFO(message, ...) do{if(GETBIT(LOG_VERBOSITY_PPUMOREINFO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BLUE "[INFO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define PPUDEBUG(message, ...) do{if(GETBIT(LOG_VERBOSITY_PPUDEBUG,ENABLE_LOGS))MASQ_UNLIKELY{logger("[DEBUG] " message "\n",##__VA_ARGS__);}}while(0)
#define PPUINFRA(message, ...) do{if(GETBIT(LOG_VERBOSITY_PPUINFRA,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_MAGENTA "[INFRA] " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define DISASSEMBLY(message, ...) do{if(GETBIT(LOG_VERBOSITY_DISASSEMBLY,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_CYAN "[INSTR] " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define WARN(message, ...) do{if(GETBIT(LOG_VERBOSITY_WARN,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BOLD_YELLOW "[WARN]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define TODO(message, ...) do{if(GETBIT(LOG_VERBOSITY_TODO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BOLD_RED "[TODO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define INFO(message, ...) do{if(GETBIT(LOG_VERBOSITY_INFO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BOLD_CYAN "[INFO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define EVENT(message, ...) do{if(GETBIT(LOG_VERBOSITY_EVENT,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BOLD_GREEN "[EVENT] " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define MOREINFO(message, ...) do{if(GETBIT(LOG_VERBOSITY_MOREINFO,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BOLD_BLUE "[INFO]  " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define DEBUG(message, ...) do{if(GETBIT(LOG_VERBOSITY_DEBUG,ENABLE_LOGS))MASQ_UNLIKELY{logger("[DEBUG] " message "\n",##__VA_ARGS__);}}while(0)
#define INFRA(message, ...) do{if(GETBIT(LOG_VERBOSITY_INFRA,ENABLE_LOGS))MASQ_UNLIKELY{logger(LOG_COLOR_BOLD_MAGENTA "[INFRA] " message "\n" LOG_COLOR_END,##__VA_ARGS__);}}while(0)
#define MASQ_LOG    LOG

// --- MASQ type-index constants -----------------------
#define MASQ_UINT8                                  0
#define MASQ_UINT16                                 1
#define MASQ_UINT32                                 2
#define MASQ_UINT64                                 3
#define MASQ_SINT8                                  4
#define MASQ_SINT16                                 5
#define MASQ_SINT32                                 6
#define MASQ_SINT64                                 7
#define MASQ_FLOAT32                                8
#define MASQ_FLOAT64                                9

#define MASQ_UNUSED(x)                              (void)(x);

// --- Inline / hint attributes ------------------------
#if defined(_MSC_VER)
#define MASQ_INLINE                                 __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define MASQ_INLINE __attribute__((always_inline))  inline
#else
#define MASQ_INLINE                                 inline
#endif

#ifdef DEBUG_NO_INLINE
#undef  MASQ_INLINE
#define MASQ_INLINE
#endif

// [[unlikely]] / [[likely]] — not supported by all Pico toolchains
#if defined(__EMSCRIPTEN__) || defined(__RPI_PICO__)
#define MASQ_UNLIKELY
#define MASQ_LIKELY
#else
#define MASQ_UNLIKELY                               [[unlikely]]
#define MASQ_LIKELY                                 [[likely]]
#endif

// --- Cross-platform struct packing -------------------
#if defined(_MSC_VER)
#define PACK_BEGIN __pragma(pack(push, 1))
#define PACK_END   __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
#if defined(__EMSCRIPTEN__) || defined(__RPI_PICO__)
#define PACK_BEGIN _Pragma("pack(push, 1)")
#define PACK_END   _Pragma("pack(pop)")
#else
#define PACK_BEGIN \
        _Pragma("pack(push, 1)") \
        _Pragma("ms_struct on")
#define PACK_END \
        _Pragma("ms_struct off") \
        _Pragma("pop")
#endif
#else
#define PACK_BEGIN
#define PACK_END
#endif

// --- Optimization (desktop only) ----------------
#if defined(_MSC_VER)
#define OPT_SPEED    __pragma(optimize("t", on))  // favor speed
#define OPT_SIZE     __pragma(optimize("s", on))  // favor size
#define OPT_DEFAULT  __pragma(optimize("", on))   // reset to defaults
#else
#define OPT_SPEED
#define OPT_SIZE
#define OPT_DEFAULT
#endif

// --- ImGui theme IDs (desktop only) ------------------
#ifndef __RPI_PICO__
#define ENABLED_IMGUI_DEFAULT_THEME                 NO
#define SE_THEME_DARK                               0
#define SE_THEME_LIGHT                              1
#define SE_THEME_BLACK                              2
#define THEME_CUSTOM                                3
#endif

// --- CPU SST toggles (all platforms) -----------------
#define ENABLE_I8080_SST                            NO
#define ENABLE_Z80_SST                              NO
#define ENABLE_SM83_SST                             NO
#define ENABLE_R2A03_SST                            NO
#define ENABLE_ARM7TDMI_SST                         NO
#pragma endregion MACROS

// =========================================================
// TYPE DEFINITIONS — identical on all platforms
// =========================================================
#ifndef byte
typedef unsigned char byte;
#endif
#ifndef errno_t
typedef int errno_t;
#endif

typedef float CHIP8_AUDIO_SAMPLE_TYPE;
typedef float SPACEINVADERS_AUDIO_SAMPLE_TYPE;
typedef float PACMAN_AUDIO_SAMPLE_TYPE;
typedef float NES_AUDIO_SAMPLE_TYPE;
typedef float GBC_AUDIO_SAMPLE_TYPE;
#if (GBA_AUDIO_SAMPLE_FORMAT == MASQ_FLOAT32)
typedef float    GBA_AUDIO_SAMPLE_TYPE;
#else
typedef int16_t  GBA_AUDIO_SAMPLE_TYPE;
#endif

typedef bool     FLAG;
typedef byte     BIT;
typedef uint8_t  BYTE;
typedef int8_t   SBYTE;
typedef uint8_t  MAP8;
typedef int8_t   INC8;
typedef uint8_t  STATE8;
typedef int8_t   SSTATE8;
typedef uint16_t MAP16;
typedef int16_t  INC16;
typedef uint16_t STATE16;
typedef uint32_t MAP32;
typedef int32_t  INC32;
typedef uint32_t STATE32;
typedef int32_t  SSTATE32;
typedef uint64_t MAP64;
typedef int64_t  INC64;
typedef uint64_t STATE64;
typedef uint8_t  ID8;
typedef uint32_t ID;
typedef uint64_t ID64;
typedef uint8_t  DIM8;
typedef uint16_t DIM16;
typedef uint32_t DIM32;
typedef int16_t  SDIM16;
typedef int32_t  SDIM32;
typedef uint8_t  COUNTER8;
typedef uint16_t COUNTER16;
typedef uint32_t COUNTER32;
typedef uint64_t COUNTER64;
typedef int8_t   SCOUNTER8;
typedef int16_t  SCOUNTER16;
typedef int32_t  SCOUNTER32;
typedef int64_t  SCOUNTER64;

// =========================================================
// CONFIG ABSTRACTION
//
// Desktop  : MasqConfig_t == boost::property_tree::ptree
// RPI Pico : MasqConfig_t == PicoConfig_t  (thin stub)
//
// PicoConfig_t mirrors the .get<T>(key, default) /
// .put(key, value) interface so every call-site compiles
// without change.  All .get() calls simply return the
// supplied default; all .put() calls are no-ops.
//
// The ROM and settings are compiled-in via generated headers
// (pico_rom.h / pico_config.h) — populate those defaults
// there rather than in CONFIG.ini.
//
// NOTE: Emulator class constructors that currently declare
//       'boost::property_tree::ptree &' must be changed to
//       'MasqConfig_t &' for Pico builds (same edits as here).
// =========================================================
#ifdef __RPI_PICO__

struct PicoConfig_t
{
    template<typename T>
    T get(const std::string& key, T defaultVal) const
    {
        RETURN get_impl<T>(key.c_str(), defaultVal);
    }

    template<typename T>
    T get(const char* key, T defaultVal) const
    {
        RETURN get_impl<T>(key, defaultVal);
    }

    template<typename T>
    void put(const std::string& /*key*/, T /*val*/)
    {
        // no-op (ROM config is immutable on Pico)
    }

    template<typename T>
    void put(const char* /*key*/, T /*val*/)
    {
        // no-op
    }

private:
    static constexpr uint32_t fnv1a(const char* str)
    {
        uint32_t hash = 2166136261u;
        while (*str)
        {
            hash ^= (uint8_t)(*str++);
            hash *= 16777619u;
        }
        RETURN hash;
    }

    template<typename T>
    static T get_impl(const char* key, T defaultVal)
    {
        const uint32_t h = fnv1a(key);

        for (size_t i = 0; i < PICO_CONFIG_TABLE_SIZE; i++)
        {
            const auto& e = PICO_CONFIG_TABLE[i];

            if (e.hash == h)
            {
                if constexpr (std::is_same_v<T, bool>)
                {
                    if (e.type == PicoType::BOOL)   RETURN e.b;
                    if (e.type == PicoType::INT)    RETURN e.i != 0;
                }
                else if constexpr (std::is_same_v<T, int>)
                {
                    if (e.type == PicoType::INT)    RETURN e.i;
                    if (e.type == PicoType::FLOAT)  RETURN static_cast<int>(e.f);
                }
                else if constexpr (std::is_same_v<T, float>)
                {
                    if (e.type == PicoType::FLOAT)  RETURN e.f;
                    if (e.type == PicoType::INT)    RETURN static_cast<float>(e.i);
                }
                else if constexpr (std::is_same_v<T, const char*>)
                {
                    if (e.type == PicoType::STRING) RETURN e.s;
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    if (e.type == PicoType::STRING) return std::string(e.s);
                    if (e.type == PicoType::BOOL)   return e.b ? std::string("true") : std::string("false");
                    if (e.type == PicoType::INT)    return std::to_string(e.i);
                    if (e.type == PicoType::FLOAT)  return std::to_string(e.f);
                }

                RETURN defaultVal;
            }
        }

        RETURN defaultVal;
    }
};

using MasqConfig_t = PicoConfig_t;

#else // !__RPI_PICO__

using MasqConfig_t = boost::property_tree::ptree;

#endif // __RPI_PICO__

// =========================================================
// GLOBAL EXTERNS
// =========================================================
extern MAP64 ENABLE_LOGS;

#ifndef __RPI_PICO__
// Desktop-only OpenGL / emulation window externs
extern float    emuWindowX;
extern float    emuWindowY;
extern float    emuWindowMaxX;
extern float    emuWindowMaxY;
#endif

MASQ_INLINE uint32_t fnv1a_runtime(const std::string& str)
{
    uint32_t hash = 0x811C9DC5; // offset basis

    for (unsigned char c : str)
    {
        hash ^= c;
        hash *= 0x01000193; // FNV prime
    }

    RETURN hash;
}

MASQ_INLINE constexpr uint32_t fnv1a_constexpr(const char* str)
{
    uint32_t hash = 2166136261u;
    while (*str)
    {
        hash ^= static_cast<uint32_t>(*str++);
        hash *= 16777619u;
    }
    RETURN hash;
}

#ifdef __RPI_PICO__
constexpr bool getPicoConfigBool(uint32_t hash)
{
    for (const auto& entry : PICO_CONFIG_TABLE)
    {
        if (entry.hash == hash && entry.type == PicoType::BOOL)
            RETURN entry.b;  // anonymous union, access directly
    }
    RETURN false;
}

constexpr int getPicoConfigInt(uint32_t hash)
{
    for (const auto& entry : PICO_CONFIG_TABLE)
    {
        if (entry.hash == hash && entry.type == PicoType::INT)
            RETURN entry.i;
    }
    RETURN 0;
}
#endif // __RPI_PICO__

// =========================================================
// LOGGER
//
// Desktop : routes to ImGui log buffer + stdout.
// Pico    : plain printf to UART (call stdio_init_all()
//           in main() before any logging).
// =========================================================

#ifdef __RPI_PICO__

void PANEL_PrintStr(const char* buf);  // forward declare — avoids circular include

// =========================================================
// LOGGER
// =========================================================

MASQ_INLINE void StripAnsiColors(const char* input, char* output, size_t outSize)
{
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j < outSize - 1; ++i)
    {
        if (input[i] == '\033' && input[i + 1] == '[')
        {
            i += 2;
            while (input[i] != '\0' && input[i] != 'm') ++i;
            continue;
        }
        output[j++] = input[i];
    }
    output[j] = '\0';
}

inline void logger(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char    buf[1024];
    va_list args2;
    va_copy(args2, args);
    vsnprintf(buf, sizeof(buf), fmt, args2);
    va_end(args2);

    // UART gets raw (ANSI colors intact)
    vprintf(fmt, args);

    // LCD gets stripped
    char stripped[1024];
    StripAnsiColors(buf, stripped, sizeof(stripped));
    PANEL_PrintStr(stripped);

    va_end(args);
}

#else // !__RPI_PICO__ — full desktop logger with ImGui sink

// --- Desktop helpers ----------------------------------
#ifndef ENABLE_OTA_EXECUTABLE

template<typename... T>
MASQ_INLINE void dummy(T...) 
{
    ;
}

MASQ_INLINE std::string StripAnsiColors(const std::string& input)
{
    std::string output;
    output.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '\033' && i + 1 < input.size() && input[i + 1] == '[')
        {
            i += 2;
            while (i < input.size() && input[i] != 'm') ++i;
            CONTINUE;
        }
        output += input[i];
    }
    RETURN output;
}

struct ImGuiLogBuffer
{
    std::vector<std::string> lines;
    std::mutex               mutex;

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
        ImGui::GetWindowDrawList()->AddRectFilled(start_pos,
            ImVec2(start_pos.x + avail.x, start_pos.y + avail.y), IM_COL32(0, 0, 0, 255));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (const auto& line : lines) ImGui::TextUnformatted(line.c_str());
        }
        ImGui::PopStyleColor();
    }
};

extern FLAG isAppLoggingEnabled;
extern ImGuiLogBuffer appLog;
extern bool isImGuiInitialized;
extern std::vector<std::string> preImGuiLogBuffer;
extern std::mutex preImGuiLogMutex;

inline void LogToImGui(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    std::string clean = StripAnsiColors(buffer);
    if (!clean.empty() && clean.back() == '\n') clean.pop_back();

    if (!isImGuiInitialized)
    {
        std::lock_guard<std::mutex> lock(preImGuiLogMutex);
        preImGuiLogBuffer.emplace_back(std::move(clean));
        RETURN;
    }
    appLog.Add(clean.c_str());
}

MASQ_INLINE void FlushEarlyLogsToImGui()
{
    std::lock_guard<std::mutex> lock(preImGuiLogMutex);
    for (const auto& line : preImGuiLogBuffer) appLog.Add(line.c_str());
    preImGuiLogBuffer.clear();
    isImGuiInitialized = true;
}

enum class UI_MODE {
    UI_OLC_RETRO = 0, UI_WIN32API
};
enum class UI_MODE_STATUS {
    UI_CLOSE = 0, UI_DONT_CLOSE, UI_REBOOT
};

#endif // !ENABLE_OTA_EXECUTABLE

// Desktop logger: ImGui sink + stdout
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

#endif // __RPI_PICO__

// =========================================================
// UTILITY FUNCTIONS
// =========================================================

template<typename T>
MASQ_INLINE void saveStruct(BYTE* dst, const T& src)
{
    static_assert(std::is_trivially_copyable_v<T>);
    memcpy(dst, &src, sizeof(T));
}

template<typename T>
MASQ_INLINE void loadStruct(T& dst, const BYTE* src)
{
    static_assert(std::is_trivially_copyable_v<T>);
    memcpy(&dst, src, sizeof(T));
}

// --- Timing ------------------------------------------
// Desktop uses std::chrono; Pico uses Pico SDK sleep_ms()
MASQ_INLINE void blocking_delay_ms(int ms)
{
#ifdef __RPI_PICO__
    sleep_ms((uint32_t)ms);
#else
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count() < ms)
    {
    }
#endif
}

// --- Directory creation (desktop only) ---------------
MASQ_INLINE void ifNoDirectoryThenCreate(std::string directory)
{
#ifndef __RPI_PICO__
    if (!std::filesystem::is_directory(directory) || !std::filesystem::exists(directory))
        std::filesystem::create_directories(directory);
#else
    (void)directory; // no filesystem on Pico; ROM and config are compiled-in
#endif
}

// --- Config string to bool ---------------------------
MASQ_INLINE bool to_bool(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
#ifdef __RPI_PICO__
    RETURN(str == "true" || str == "1" || str == "yes");
#else
    std::istringstream is(str);
    bool b;
    is >> std::boolalpha >> b;
    RETURN b;
#endif
}

// Converts the given string to uppercase in-place.
// - Modifies the original string (no copy is made)
// - Uses std::toupper safely with unsigned char to avoid UB
// - Fastest option when mutation is acceptable
MASQ_INLINE std::string toUpperCopy(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { RETURN std::toupper(c); });
    RETURN s;
}

// Converts a given string to lowercase (ASCII-safe).
// Performs in-place transformation on a copy of the input string and returns it.
// Uses std::tolower with unsigned char cast to avoid undefined behavior for negative char values.
// Safe for both desktop and embedded (Pico) builds.
MASQ_INLINE std::string toLower(std::string s)
{
    for (char& c : s)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    RETURN s;
}

// --- Bit utilities -----------------------------------
MASQ_INLINE uint8_t countSetBits(int32_t number)
{
    const uint8_t nibble_to_bits[16] =
    { 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4 };
    if (0 == number) RETURN nibble_to_bits[0];
    int nibble = number & 0xf;
    RETURN nibble_to_bits[nibble] + countSetBits(number >> 4);
}

MASQ_INLINE bool isOddParity(int32_t number)
{
    RETURN(bool)(countSetBits(number) % 2);
}

MASQ_INLINE uint8_t getBit(uint32_t byte, uint8_t position)
{
    RETURN(byte >> position) & 1;
}

MASQ_INLINE std::bitset<8> toBinary(int n)
{
    RETURN std::bitset<8>(n);
}

static constexpr MASQ_INLINE uint32_t next_pow2(uint32_t x)
{
    if (x <= 1) RETURN 1;
    --x;
#if defined(__GNUC__) || defined(__clang__) || defined(__EMSCRIPTEN__) || defined(__RPI_PICO__)
    RETURN 1u << (32 - __builtin_clz(x));
#elif defined(_MSC_VER)
    if (std::is_constant_evaluated())
    {
        x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
        RETURN x + 1;
    }
    else
    {
        unsigned long index;
        _BitScanReverse(&index, x);
        RETURN 1u << (index + 1);
    }
#else
    x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
    RETURN x + 1;
#endif
}

MASQ_INLINE std::string hex(uint32_t n, uint8_t d)
{
    std::string s(d, '0');
    for (int i = d - 1; i >= 0; i--, n >>= 4)
        s[i] = "0123456789ABCDEF"[n & 0xF];
    RETURN s;
}

MASQ_INLINE std::string toUpper(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { RETURN std::toupper(c); });
    RETURN result;
}

MASQ_INLINE int fast_positive_mod(const int input, const int ceil)
{
    RETURN input >= ceil ? input % ceil : input;
}

// For frequency of substring
MASQ_INLINE void computeLPSArray(std::string pattern, size_t M, int lps[])
{
    // Length of the previous longest
    // prefix suffix
    int len = 0;
    unsigned int i = 1;
    lps[0] = 0; // lps[0] is always 0
    // The loop calculates lps[i] for
    // i = 1 to M-1
    while (i < M)
    {
        if (pattern[i] == pattern[len])
        {
            len++; lps[i] = len; i++;
        }
        else // (pat[i] != pat[len])
        {
            // This is tricky. Consider the example.
            // AAACAAAA and i = 7. The idea is similar
            // to search step.
            if (len != 0)
                len = lps[len - 1]; // Also, note that we do not increment i here
            else // if (len == 0)
            {
                lps[i] = len; i++;
            }
        }
    }
}

MASQ_INLINE int KMPSearch(std::string pattern, std::string original)
{
    size_t M = pattern.length();
    size_t N = original.length();
    // Create lps[] that will hold the longest
    // prefix suffix values for pattern
    int lps[MAX_PATH];
    unsigned int j = 0; // index for pat[]
    // Preprocess the pattern (calculate lps[]
    // array)
    computeLPSArray(pattern, M, lps);
    unsigned int i = 0; // index for txt[]
    unsigned int res = 0;
    while (i < N)
    {
        if (pattern[j] == original[i])
        {
            j++; i++;
        }
        if (j == M)
        {
            // When we find pattern first time,
            // we iterate again to check if there
            // exists more pattern
            if ((j - 1U) < (sizeof(lps) / sizeof(int)))
                j = lps[j - 1U];
            res++;
        }
        // Mismatch after j matches
        else if (i < N && pattern[j] != original[i])
        {
            // Do not match lps[0..lps[j-1]]
            // characters, they will match anyway
            if (j != 0U)
            {
                if ((j - 1U) < (sizeof(lps) / sizeof(int)))
                    j = lps[j - 1U];
                else
                    i++;
            }
        }
    }
    RETURN res;
}

MASQ_INLINE uint64_t signExtend64(uint64_t v, int currentNumberOfBits)
{
    if (v & (1ull << (currentNumberOfBits - 1)))
        RETURN v | (~0ull << currentNumberOfBits);
    RETURN v;
}

MASQ_INLINE uint32_t signExtend32(uint32_t v, int currentNumberOfBits)
{
    if (v & (1u << (currentNumberOfBits - 1)))
        RETURN v | (~0u << currentNumberOfBits);
    RETURN v;
}

// --- String helpers (desktop only, use sstream) ------
#ifndef __RPI_PICO__
template <typename T>
MASQ_INLINE std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    RETURN std::move(out).str();
}

MASQ_INLINE std::wstring to_wstring(const std::string& stringToConvert)
{
    RETURN std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(stringToConvert);
}

MASQ_INLINE bool doesFileExist(const std::string& name)
{
    struct stat buffer;
    RETURN(stat(name.c_str(), &buffer) == 0);
}

MASQ_INLINE std::filesystem::path getexepath()
{
#ifdef __EMSCRIPTEN__
	// No executable path in WASM; RETURN empty or fixed string
	RETURN {};
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
#endif // !__RPI_PICO__

// --- Portable file / memory helpers ------------------
MASQ_INLINE int fopen_portable(FILE** file, const char* filename, const char* mode)
{
#ifdef _MSC_VER
    RETURN fopen_s(file, filename, mode);
#else
    * file = fopen(filename, mode);
    RETURN(*file != nullptr) ? 0 : errno;
#endif
}

MASQ_INLINE int memcpy_portable(void* dest, size_t destSize, const void* src, size_t count)
{
#if defined(_MSC_VER)
    RETURN memcpy_s(dest, destSize, src, count);
#else
    if (!dest || !src)      RETURN 1;
    if (count > destSize)   RETURN 1;
    std::memcpy(dest, src, count);
    RETURN 0;
#endif
}

MASQ_INLINE uint32_t ctz32_portable(uint32_t x)
{
#if defined(_MSC_VER)
    unsigned long idx;
    _BitScanForward(&idx, x);
    RETURN idx;
#else
    RETURN __builtin_ctz(x);
#endif
}

// --- Extension helper (used by ROM detection) --------
MASQ_INLINE std::string get_extension(const std::string& filename)
{
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == std::string::npos || dot_pos == 0) RETURN "";
    RETURN filename.substr(dot_pos + 1);
}

// --- FIR filter conversion helpers -------------------
MASQ_INLINE void int8ToDouble(int8_t* input, double* output, int length)
{
    for (int i = 0; i < length; i++) output[i] = (double)input[i];
}
MASQ_INLINE void doubleToInt8(double* input, int8_t* output, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (input[i] > 127.0)  input[i] = 127.0;
        if (input[i] < -128.0) input[i] = -128.0;
        output[i] = (int8_t)input[i];
    }
}
MASQ_INLINE void int16ToDouble(int16_t* input, double* output, int length)
{
    for (int i = 0; i < length; i++) output[i] = (double)input[i];
}
MASQ_INLINE void doubleToInt16(double* input, int16_t* output, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (input[i] > 32767.0)  input[i] = 32767.0;
        if (input[i] < -32768.0) input[i] = -32768.0;
        output[i] = (int16_t)input[i];
    }
}
MASQ_INLINE void int32ToDouble(int32_t* input, double* output, int length)
{
    for (int i = 0; i < length; i++) output[i] = (double)input[i];
}
MASQ_INLINE void doubleToInt32(double* input, int32_t* output, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (input[i] > 2147483647.0)  input[i] = 2147483647.0;
        if (input[i] < -2147483648.0) input[i] = -2147483648.0;
        output[i] = (int32_t)input[i];
    }
}
MASQ_INLINE void int64ToDouble(int64_t* input, double* output, int length)
{
    for (int i = 0; i < length; i++) output[i] = (double)input[i];
}
MASQ_INLINE void doubleToInt64(double* input, int64_t* output, int length)
{
    for (int i = 0; i < length; i++) output[i] = (int64_t)input[i];
}

// Used by NES database
#ifndef __RPI_PICO__
MASQ_INLINE uint32_t hexStr32(const char* s)
{
    if (!s || s[0] == '\0') return 0u;
    uint32_t v = 0;
    while (*s)
    {
        v <<= 4;
        char c = *s++;
        if (c >= '0' && c <= '9') v |= static_cast<uint32_t>(c - '0');
        else if (c >= 'A' && c <= 'F') v |= static_cast<uint32_t>(c - 'A' + 10);
        else if (c >= 'a' && c <= 'f') v |= static_cast<uint32_t>(c - 'a' + 10);
    }
    return v;
}

// "_size":"16384" stored as string in the converted JSON
MASQ_INLINE uint32_t u32str(const rapidjson::Value& obj, const char* key)
{
    if (!obj.HasMember(key)) return 0u;
    const auto& v = obj[key];
    if (v.IsUint())   return v.GetUint();
    if (v.IsString()) return static_cast<uint32_t>(std::stoul(v.GetString()));
    return 0u;
}

MASQ_INLINE const char* strval(const rapidjson::Value& obj, const char* key)
{
    if (!obj.HasMember(key)) return "";
    const auto& v = obj[key];
    return v.IsString() ? v.GetString() : "";
}
#endif

// --- ZIP extraction (desktop + Emscripten only) ------
#ifndef __RPI_PICO__
#ifndef __EMSCRIPTEN__
MASQ_INLINE void extract_all_to_current_dir(const char* zip_path)
{
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, zip_path, 0))
    {
        LOG("Failed to open zip archive: %s", zip_path); RETURN;
    }

    mz_uint file_count = mz_zip_reader_get_num_files(&zip);
    for (mz_uint i = 0; i < file_count; ++i)
    {
        if (mz_zip_reader_is_file_a_directory(&zip, i)) CONTINUE;
        char filename[512] = { 0 };
        if (!mz_zip_reader_get_filename(&zip, i, filename, sizeof(filename)))
        {
            LOG("Failed to get filename for index %u", i); CONTINUE;
        }
        if (!mz_zip_reader_extract_to_file(&zip, i, filename, 0))
            LOG("Failed to extract: %s", filename);
        else
            LOG("Extracted: %s", filename);
    }
    mz_zip_reader_end(&zip);
}

MASQ_INLINE bool extract_zip(const std::filesystem::path& zip_path,
    const std::filesystem::path& dest_dir)
{
    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));
    if (!mz_zip_reader_init_file(&zip_archive, zip_path.string().c_str(), 0))
    {
        FATAL("Failed to open ZIP archive: %s", zip_path.string().c_str()); RETURN false;
    }

    int num_files = static_cast<int>(mz_zip_reader_get_num_files(&zip_archive));
    for (int i = 0; i < num_files; ++i)
    {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) CONTINUE;
        std::filesystem::path out_file = dest_dir / file_stat.m_filename;
        if (file_stat.m_is_directory)
            std::filesystem::create_directories(out_file);
        else
        {
            std::filesystem::create_directories(out_file.parent_path());
            if (!mz_zip_reader_extract_to_file(&zip_archive, i, out_file.string().c_str(), 0))
                FATAL("Failed to extract file: %s", out_file.string().c_str());
        }
    }
    mz_zip_reader_end(&zip_archive);
    RETURN true;
}
#else // __EMSCRIPTEN__
// NOTE: Sync to persistent FS must be done outside this function
MASQ_INLINE int extract_all_to_persistent_dir(const char* zip_path,
    std::vector<std::string>& out_paths)
{
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, zip_path, 0))
    {
        LOG("Failed to open zip archive: %s", zip_path); RETURN 0;
    }

    mz_uint file_count = mz_zip_reader_get_num_files(&zip);
    int extracted_count = 0;
    for (mz_uint i = 0; i < file_count; ++i)
    {
        if (mz_zip_reader_is_file_a_directory(&zip, i)) CONTINUE;
        char filename[512] = { 0 };
        if (!mz_zip_reader_get_filename(&zip, i, filename, sizeof(filename)))
        {
            LOG("Failed to get filename for index %u", i); CONTINUE;
        }
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "/persistent/%s", filename);
        if (!mz_zip_reader_extract_to_file(&zip, i, full_path, 0))
            LOG("Failed to extract: %s", full_path);
        else
        {
            LOG("Extracted: %s", full_path); out_paths.emplace_back(filename); ++extracted_count;
        }
    }
    mz_zip_reader_end(&zip);
    RETURN extracted_count;
}
#endif // __EMSCRIPTEN__
#endif // !__RPI_PICO__

#ifndef ENABLE_OTA_EXECUTABLE
// --- File list helpers (desktop only) ----------------
#ifndef __RPI_PICO__
MASQ_INLINE void writeDequeToFile(const std::deque<std::string>& myDeque,
    const std::string& filename)
{
    std::ofstream outputFile(filename);
    if (outputFile.is_open())
    {
        for (const std::string& element : myDeque) outputFile << element << std::endl;
        outputFile.close();
    }
    else std::cerr << "Unable to open file for writing: " << filename << std::endl;
}

MASQ_INLINE std::deque<std::string> readDequeFromFile(const std::string& filename)
{
    std::deque<std::string> myDeque;
    std::ifstream inputFile(filename);
    if (inputFile.is_open())
    {
        std::string line;
        while (std::getline(inputFile, line)) myDeque.push_back(line);
        inputFile.close();
    }
    else std::cerr << "Unable to open file for reading: " << filename << std::endl;
    RETURN myDeque;
}
#endif // !__RPI_PICO__

// --- Mouse helper (desktop / ImGui only) -------------
#ifndef __RPI_PICO__
MASQ_INLINE FLAG getMouseRelPosIfDocked(float* xpos, float* ypos,
    uint32_t emuScreenWidth,
    uint32_t emuScreenHeight)
{
    FLAG inside = YES;
    static const float upperborder = 8, otherborder = 8;
    float maxX = emuWindowMaxX - otherborder;
    float maxY = emuWindowMaxY - otherborder;
    *xpos = ImGui::GetMousePos().x - emuWindowX - otherborder;
    *ypos = ImGui::GetMousePos().y - emuWindowY - upperborder;
    if ((*xpos + otherborder > maxX) || (*ypos + upperborder > maxY) ||
        (*xpos < 0 || *ypos < 0))
    {
        *xpos = *ypos = 0; inside = NO;
    }
    *xpos = *xpos * emuScreenWidth / maxX;
    *ypos = *ypos * emuScreenHeight / maxY;
    RETURN inside;
}
#endif // !__RPI_PICO__

// =========================================================
// ENUMERATIONS AND STRUCTS
// =========================================================

enum EMULATION_ID : uint8_t
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
    DS3_ID,
    WIIU_ID,
    SWITCH_ID,
    GAME_OF_LIFE_ID,
    TEST_CPU_ID,
    TOTAL_ID,
    ANY_ID,
    ANY_ID_FOR_COMPARE_OR_REPLAY,
    INVALID_ID
};

typedef struct
{
    FLAG  _DEBUG_BP_V1;
    FLAG  _DEBUG_CALLSTACK;
    FLAG  _DEBUG_PPU_VIEWER_GUI;
    INC64 _DEBUG_PPU_VIEWER_GUI_TRIGGER;
    FLAG  _DEBUG_LOGGER_CLI;
    MAP64 _DEBUG_LOGGER_CLI_MASK;
    FLAG  _DEBUG_FPS;
    FLAG  _DEBUG_GRAPHICS;
    FLAG  _DEBUG_KEYPAD;
    FLAG  _DEBUG_LUT;
    FLAG  _DEBUG_MEMORY;
    FLAG  _DEBUG_PROFILER;
    FLAG  _DEBUG_REGISTERS;
    FLAG  _DEBUG_SOUND;
    FLAG  _DEBUG_STEP;
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

// --- Theme / filter / palette enums ------------------
// Kept on Pico too: palette IDs affect display output
// even on a Waveshark display driver.
enum class EMULATOR_THEME
{
    DARK = 0,
    LIGHT = 1,
    BLACK = 2
};

enum class VIDEO_FILTERS
{
    NEAREST_FILTER = ZERO,
    BILINEAR_FILTER,
    LCD_FILTER,
    CRT_FILTER,
    MAX_FILTERS
};

enum class PALETTE_ID
{
    PALETTE_1,
    PALETTE_2,
    PALETTE_3,
    PALETTE_4,
    PALETTE_5,
    PALETTE_6,
    PALETTE_MAX,
    NO_PALETTE
};

// --- Config LUTs (desktop only — unordered_map is too
//     heavy for Pico SRAM and not needed when ROM/config
//     are compiled-in via pico_config.h / pico_rom.h)
#ifndef __RPI_PICO__

extern int          currentEmuTheme;
extern VIDEO_FILTERS currEnVFilter;
extern PALETTE_ID    currEnGbPalette;
extern PALETTE_ID    currEnGbcPalette;

std::unordered_map<std::string, EMULATOR_THEME> const configToEmuThemes =
{
    {"DARK",  EMULATOR_THEME::DARK },
    {"LIGHT", EMULATOR_THEME::LIGHT},
    {"BLACK", EMULATOR_THEME::BLACK}
};
std::unordered_map<EMULATOR_THEME, std::string> const emuThemesToConfig =
{
    {EMULATOR_THEME::DARK,  "DARK" },
    {EMULATOR_THEME::LIGHT, "LIGHT"},
    {EMULATOR_THEME::BLACK, "BLACK"}
};
std::unordered_map<std::string, VIDEO_FILTERS> const configToVFilters =
{
    {"NEAREST_FILTER",  VIDEO_FILTERS::NEAREST_FILTER },
    {"BILINEAR_FILTER", VIDEO_FILTERS::BILINEAR_FILTER},
    {"LCD_FILTER",      VIDEO_FILTERS::LCD_FILTER     },
    {"CRT_FILTER",      VIDEO_FILTERS::CRT_FILTER     }
};
std::unordered_map<VIDEO_FILTERS, std::string> const vFiltersToConfig =
{
    {VIDEO_FILTERS::NEAREST_FILTER,  "NEAREST_FILTER" },
    {VIDEO_FILTERS::BILINEAR_FILTER, "BILINEAR_FILTER"},
    {VIDEO_FILTERS::LCD_FILTER,      "LCD_FILTER"     },
    {VIDEO_FILTERS::CRT_FILTER,      "CRT_FILTER"     }
};
std::unordered_map<std::string, PALETTE_ID> const configToGbPaletteID =
{
    {"GearBoy",    PALETTE_ID::PALETTE_1},
    {"Black/White",PALETTE_ID::PALETTE_2},
    {"SameBoy",    PALETTE_ID::PALETTE_3},
    {"BGB",        PALETTE_ID::PALETTE_4},
    {"Gameboy",    PALETTE_ID::PALETTE_5},
    {"DMG",        PALETTE_ID::PALETTE_6}
};
std::unordered_map<PALETTE_ID, std::string> const gbPaletteIDToConfig =
{
    {PALETTE_ID::PALETTE_1, "GearBoy"    },
    {PALETTE_ID::PALETTE_2, "Black/White"},
    {PALETTE_ID::PALETTE_3, "SameBoy"    },
    {PALETTE_ID::PALETTE_4, "BGB"        },
    {PALETTE_ID::PALETTE_5, "Gameboy"    },
    {PALETTE_ID::PALETTE_6, "DMG"        }
};

// --- ROM extension / count lookup tables (desktop) ---
// On Pico the ROM is compiled-in; getType() is not called.
std::unordered_map<std::string, EMULATION_ID> const _fileExtentionToEmulationPlatform =
{
    {".gol", EMULATION_ID::GAME_OF_LIFE_ID},
    {".ch8", EMULATION_ID::CHIP8_ID}, {".CH8", EMULATION_ID::CHIP8_ID},
    {".c8",  EMULATION_ID::CHIP8_ID}, {".C8",  EMULATION_ID::CHIP8_ID},
    {".sc8", EMULATION_ID::CHIP8_ID}, {".SC8", EMULATION_ID::CHIP8_ID},
    {".xo8", EMULATION_ID::CHIP8_ID}, {".XO8", EMULATION_ID::CHIP8_ID},
    {".e",   EMULATION_ID::SPACE_INVADERS_ID}, {".E", EMULATION_ID::SPACE_INVADERS_ID},
    {".f",   EMULATION_ID::SPACE_INVADERS_ID}, {".F", EMULATION_ID::SPACE_INVADERS_ID},
    {".g",   EMULATION_ID::SPACE_INVADERS_ID}, {".G", EMULATION_ID::SPACE_INVADERS_ID},
    {".h",   EMULATION_ID::SPACE_INVADERS_ID}, {".H", EMULATION_ID::SPACE_INVADERS_ID},
    {".1m",  EMULATION_ID::PACMAN_ID}, {".1M", EMULATION_ID::PACMAN_ID},
    {".3m",  EMULATION_ID::PACMAN_ID}, {".3M", EMULATION_ID::PACMAN_ID},
    {".4a",  EMULATION_ID::PACMAN_ID}, {".4A", EMULATION_ID::PACMAN_ID},
    {".5e",  EMULATION_ID::PACMAN_ID}, {".5E", EMULATION_ID::PACMAN_ID},
    {".5f",  EMULATION_ID::PACMAN_ID}, {".5F", EMULATION_ID::PACMAN_ID},
    {".6e",  EMULATION_ID::PACMAN_ID}, {".6E", EMULATION_ID::PACMAN_ID},
    {".6f",  EMULATION_ID::PACMAN_ID}, {".6F", EMULATION_ID::PACMAN_ID},
    {".6h",  EMULATION_ID::PACMAN_ID}, {".6H", EMULATION_ID::PACMAN_ID},
    {".6j",  EMULATION_ID::PACMAN_ID}, {".6J", EMULATION_ID::PACMAN_ID},
    {".7f",  EMULATION_ID::PACMAN_ID}, {".7F", EMULATION_ID::PACMAN_ID},
    {"",     EMULATION_ID::PACMAN_ID},
    {".nes", EMULATION_ID::NES_ID}, {".NES", EMULATION_ID::NES_ID},
    {".gb",  EMULATION_ID::GB_GBC_ID}, {".GB",  EMULATION_ID::GB_GBC_ID},
    {".gbc", EMULATION_ID::GB_GBC_ID}, {".GBC", EMULATION_ID::GB_GBC_ID},
    {".gba", EMULATION_ID::GBA_ID},    {".GBA", EMULATION_ID::GBA_ID},
    {".com", EMULATION_ID::TEST_CPU_ID}, {".COM", EMULATION_ID::TEST_CPU_ID},
    {".cim", EMULATION_ID::TEST_CPU_ID}, {".CIM", EMULATION_ID::TEST_CPU_ID},
    {".tap", EMULATION_ID::TEST_CPU_ID}, {".TAP", EMULATION_ID::TEST_CPU_ID},
    {".bin", EMULATION_ID::TEST_CPU_ID}, {".BIN", EMULATION_ID::TEST_CPU_ID},
};

std::unordered_map<uint32_t, EMULATION_ID> const _numberOfRomsToEmulationPlatform =
{
    { 1,  EMULATION_ID::ANY_ID                                  },
    { 2,  EMULATION_ID::TEST_CPU_ID                             },
    { 3,  EMULATION_ID::ANY_ID_FOR_COMPARE_OR_REPLAY            },
    { 4,  EMULATION_ID::SPACE_INVADERS_ID                       },
    {10,  EMULATION_ID::PACMAN_ID                               },
    {13,  EMULATION_ID::PACMAN_ID                               }
};

MASQ_INLINE const char* getEmulationName(EMULATION_ID id)
{
    static const std::unordered_map<EMULATION_ID, const char*> map =
    {
        {EMULATION_ID::DEFAULT_ID, "Default"},
        {EMULATION_ID::CHIP8_ID, "CHIP-8"},
        {EMULATION_ID::SPACE_INVADERS_ID, "Space Invaders"},
        {EMULATION_ID::PACMAN_ID, "Pac-Man"},
        {EMULATION_ID::NES_ID, "NES"},
        {EMULATION_ID::GB_GBC_ID, "Game Boy / Game Boy Color"},
        {EMULATION_ID::SNES_ID, "SNES"},
        {EMULATION_ID::N64_ID, "Nintendo 64"},
        {EMULATION_ID::GBA_ID, "Game Boy Advance"},
        {EMULATION_ID::GAMECUBE_ID, "GameCube"},
        {EMULATION_ID::DS_ID, "Nintendo DS"},
        {EMULATION_ID::WII_ID, "Wii"},
        {EMULATION_ID::DS3_ID, "PlayStation 3"},
        {EMULATION_ID::WIIU_ID, "Wii U"},
        {EMULATION_ID::SWITCH_ID, "Nintendo Switch"},
        {EMULATION_ID::GAME_OF_LIFE_ID, "Game of Life"},
        {EMULATION_ID::TEST_CPU_ID, "Test CPU"},
        {EMULATION_ID::TOTAL_ID, "Total"},
        {EMULATION_ID::ANY_ID, "Any"},
        {EMULATION_ID::ANY_ID_FOR_COMPARE_OR_REPLAY, "Any (Compare/Replay)"},
        {EMULATION_ID::INVALID_ID, "Invalid"}
    };

    auto it = map.find(id);
    RETURN (it != map.end()) ? it->second : "Unknown";
}

#else

MASQ_INLINE EMULATION_ID getPlatformFromExtension(std::string ext)
{
    toLower(ext);

    switch (fnv1a_runtime(ext))
    {
        // GAME OF LIFE
    case fnv1a_constexpr(".gol"):
        RETURN EMULATION_ID::GAME_OF_LIFE_ID;

        // CHIP8
    case fnv1a_constexpr(".ch8"):
    case fnv1a_constexpr(".c8"):
    case fnv1a_constexpr(".sc8"):
    case fnv1a_constexpr(".xo8"):
        RETURN EMULATION_ID::CHIP8_ID;

        // SPACE INVADERS
    case fnv1a_constexpr(".e"):
    case fnv1a_constexpr(".f"):
    case fnv1a_constexpr(".g"):
    case fnv1a_constexpr(".h"):
        RETURN EMULATION_ID::SPACE_INVADERS_ID;

        // PACMAN
    case fnv1a_constexpr(".1m"):
    case fnv1a_constexpr(".3m"):
    case fnv1a_constexpr(".4a"):
    case fnv1a_constexpr(".5e"):
    case fnv1a_constexpr(".5f"):
    case fnv1a_constexpr(".6e"):
    case fnv1a_constexpr(".6f"):
    case fnv1a_constexpr(".6h"):
    case fnv1a_constexpr(".6j"):
    case fnv1a_constexpr(".7f"):
    case fnv1a_constexpr(""):
        RETURN EMULATION_ID::PACMAN_ID;

        // NES
    case fnv1a_constexpr(".nes"):
        RETURN EMULATION_ID::NES_ID;

        // GB/GBC
    case fnv1a_constexpr(".gb"):
    case fnv1a_constexpr(".gbc"):
        RETURN EMULATION_ID::GB_GBC_ID;

        // GBA
    case fnv1a_constexpr(".gba"):
        RETURN EMULATION_ID::GBA_ID;

        // TEST CPU
    case fnv1a_constexpr(".com"):
    case fnv1a_constexpr(".cim"):
    case fnv1a_constexpr(".tap"):
    case fnv1a_constexpr(".bin"):
        RETURN EMULATION_ID::TEST_CPU_ID;
    }

    RETURN EMULATION_ID::DEFAULT_ID;
}
#endif // !__RPI_PICO__

// --- Pixel struct (used by all display backends) ------
struct Pixel
{
    union {
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
    Pixel  operator * (const float i)  const;
    Pixel  operator / (const float i)  const;
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
    GREY              (192, 192, 192),
    DARK_GREY         (128, 128, 128),
    VERY_DARK_GREY    ( 64,  64,  64),

    RED               (255,   0,   0),
    DARK_RED          (128,   0,   0),
    VERY_DARK_RED     ( 64,   0,   0),

    YELLOW            (255, 255,   0),
    DARK_YELLOW       (128, 128,   0),
    VERY_DARK_YELLOW  ( 64,  64,   0),

    GREEN             (  0, 255,   0),
    DARK_GREEN        (  0, 128,   0),
    VERY_DARK_GREEN   (  0,  64,   0),

    CYAN              (  0, 255, 255),
    DARK_CYAN         (  0, 128, 128),
    VERY_DARK_CYAN    (  0,  64,  64),

    BLUE              (  0,   0, 255),
    DARK_BLUE         (  0,   0, 128),
    VERY_DARK_BLUE    (  0,   0,  64),

    MAGENTA           (255,   0, 255),
    DARK_MAGENTA      (128,   0, 128),
    VERY_DARK_MAGENTA ( 64,   0,  64),

    WHITE             (255, 255, 255),
    BLACK             (  0,   0,   0),
    BLANK             (  0,   0,   0,   0);

MASQ_INLINE Pixel::Pixel() {
    r = 0; g = 0; b = 0; a = 0xFF;
}
MASQ_INLINE Pixel::Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    n = red | (green << 8) | (blue << 16) | (alpha << 24);
}
MASQ_INLINE Pixel::Pixel(uint32_t p) {
    n = p;
}
MASQ_INLINE bool  Pixel::operator==(const Pixel& p) const {
    RETURN n == p.n;
}
MASQ_INLINE bool  Pixel::operator!=(const Pixel& p) const {
    RETURN n != p.n;
}
MASQ_INLINE Pixel Pixel::operator*(const float i) const {
    float fR = std::min(255.0f, std::max(0.0f, float(r) * i)), fG = std::min(255.0f, std::max(0.0f, float(g) * i)), fB = std::min(255.0f, std::max(0.0f, float(b) * i)); RETURN Pixel(uint8_t(fR), uint8_t(fG), uint8_t(fB), a);
}
MASQ_INLINE Pixel Pixel::operator/(const float i) const {
    float fR = std::min(255.0f, std::max(0.0f, float(r) / i)), fG = std::min(255.0f, std::max(0.0f, float(g) / i)), fB = std::min(255.0f, std::max(0.0f, float(b) / i)); RETURN Pixel(uint8_t(fR), uint8_t(fG), uint8_t(fB), a);
}
MASQ_INLINE Pixel Pixel::operator+(const Pixel& p) const {
    uint8_t nR = uint8_t(std::min(255, std::max(0, int(r) + int(p.r)))), nG = uint8_t(std::min(255, std::max(0, int(g) + int(p.g)))), nB = uint8_t(std::min(255, std::max(0, int(b) + int(p.b)))); RETURN Pixel(nR, nG, nB, a);
}
MASQ_INLINE Pixel Pixel::operator-(const Pixel& p) const {
    uint8_t nR = uint8_t(std::min(255, std::max(0, int(r) - int(p.r)))), nG = uint8_t(std::min(255, std::max(0, int(g) - int(p.g)))), nB = uint8_t(std::min(255, std::max(0, int(b) - int(p.b)))); RETURN Pixel(nR, nG, nB, a);
}
MASQ_INLINE Pixel Pixel::operator*(const Pixel& p) const {
    uint8_t nR = uint8_t(std::min(255.0f, std::max(0.0f, float(r) * float(p.r) / 255.0f))), nG = uint8_t(std::min(255.0f, std::max(0.0f, float(g) * float(p.g) / 255.0f))), nB = uint8_t(std::min(255.0f, std::max(0.0f, float(b) * float(p.b) / 255.0f))), nA = uint8_t(std::min(255.0f, std::max(0.0f, float(a) * float(p.a) / 255.0f))); RETURN Pixel(nR, nG, nB, nA);
}
MASQ_INLINE Pixel Pixel::inv() const {
    uint8_t nR = uint8_t(std::min(255, std::max(0, 255 - int(r)))), nG = uint8_t(std::min(255, std::max(0, 255 - int(g)))), nB = uint8_t(std::min(255, std::max(0, 255 - int(b)))); RETURN Pixel(nR, nG, nB, a);
}

MASQ_INLINE uint16_t PixelToRGB565(const Pixel& p) {
    uint8_t r = (p.n >> 0)  & 0xFF;
    uint8_t g = (p.n >> 8)  & 0xFF;
    uint8_t b = (p.n >> 16) & 0xFF;

    return (uint16_t)(
        ((r >> 3) << 11) |
        ((g >> 2) << 5)  |
        ((b >> 3) << 0)
    );
}

struct EmulationIDHash
{
    std::size_t operator()(EMULATION_ID id) const
    {
        RETURN std::hash<uint8_t>{}(static_cast<uint8_t>(id));
    }
};

// --- Parity LUT (used by emulators) ------------------
const uint8_t parityLUT[0x100] = {
1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
};

// --- Network serial types (desktop only) -------------
#ifndef __RPI_PICO__
enum class serialMsg : uint32_t
{
    Server_GetStatus, Server_GetPing,
    Client_Accepted, Client_AssignID, Client_RegisterWithServer, Client_UnregisterWithServer,
    Game_SendBit, Game_ReceiveBit, Game_HeartBeat,
};
typedef struct {
    uint32_t ID; BIT data; SIGNAL heartBeat; uint16_t pad0;
} gameSerialData;
#endif // !__RPI_PICO__

// =========================================================
// CRC-32 — used on all platforms for save-state IDs
// =========================================================
extern unsigned long crcTable[256];
extern double        bufferForFIR[2048];

static uint32_t crc32_table[256];
static int      crc32_table_initialized = 0;

MASQ_INLINE void crc32_init(void)
{
    for (uint32_t i = 0; i < 256; ++i)
    {
        uint32_t crc = i;
        for (uint32_t j = 0; j < 8; ++j)
            crc = (crc & 1) ? ((crc >> 1) ^ 0xEDB88320) : (crc >> 1);
        crc32_table[i] = crc;
    }
    crc32_table_initialized = 1;
}

MASQ_INLINE uint32_t crc32_compute(const uint8_t* data, size_t length)
{
    if (!crc32_table_initialized) crc32_init();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i)
        crc = (crc >> 8) ^ crc32_table[(uint8_t)((crc ^ data[i]) & 0xFF)];
    RETURN crc ^ 0xFFFFFFFF;
}

MASQ_INLINE void createLUTForCRC()
{
    unsigned long POLYNOMIAL = 0xEDB88320;
    unsigned long remainder;
    uint8_t b = 0;
    do
    {
        remainder = b;
        for (unsigned long bit = 8; bit > 0; --bit)
            remainder = (remainder & 1) ? ((remainder >> 1) ^ POLYNOMIAL) : (remainder >> 1);
        crcTable[(size_t)b] = remainder;
    }
    while (0 != ++b);
}

MASQ_INLINE unsigned long genCRC(uint8_t* p, size_t n)
{
    unsigned long crc = 0xfffffffful;
    for (size_t i = 0; i < n; i++)
        crc = crcTable[*p++ ^ (crc & 0xff)] ^ (crc >> 8);
    RETURN(~crc);
}

MASQ_INLINE std::string getUniqueGameID(uint8_t* in, size_t length)
{
    TODO("Use HASH instead of CRC");
    unsigned long crc = genCRC(in, length);
    // Convert to hex string manually (no sstream on Pico)
    char buf[16];
    snprintf(buf, sizeof(buf), "%lx", crc);
    RETURN std::string(buf);
}

MASQ_INLINE std::string getSaveFileName(uint8_t* in, uint64_t length)
{
    RETURN getUniqueGameID(in, (size_t)length) + ".battery.sram";
}

MASQ_INLINE std::string getRTCSaveName(uint8_t* in, uint64_t length)
{
    RETURN getUniqueGameID(in, (size_t)length) + ".battery.rtc";
}

MASQ_INLINE std::string getSaveStateName(uint8_t* in, uint64_t length)
{
    RETURN getUniqueGameID(in, (size_t)length) + ".state";
}

// =========================================================
// FIR FILTER — used by audio pipeline on all platforms
// =========================================================
MASQ_INLINE void firFilterInit(void)
{
    memset(bufferForFIR, ZERO, sizeof(bufferForFIR));
}

MASQ_INLINE void firFilter(const double* coeffs, double* input, double* output,
    int length, int filterLength)
{
    double acc;
    const double* coeffp;
    double* inputp;
    memcpy(&bufferForFIR[filterLength - 1], input, (size_t)length * sizeof(double));
    for (int n = 0; n < length; n++)
    {
        coeffp = coeffs;
        inputp = &bufferForFIR[filterLength - 1 + n];
        acc = 0;
        for (int k = 0; k < filterLength; k++) acc += (*coeffp++) * (*inputp--);
        output[n] = acc;
    }
    memmove(&bufferForFIR[0], &bufferForFIR[length],
        (size_t)(filterLength - 1) * sizeof(double));
}

// =========================================================
// isCLI — not meaningful on Pico (no CLI mode)
// =========================================================
extern ROM ROM_TYPE;

// Needed by NES
extern FLAG enableZapper;
extern FLAG nesReset;

MASQ_INLINE FLAG isCLI()
{
#ifdef __RPI_PICO__
    RETURN NO; // Pico always runs in emulation mode
#else
    RETURN ((ROM_TYPE == ROM::TEST_ROM_COM)
        || (ROM_TYPE == ROM::TEST_ROM_CIM)
        || (ROM_TYPE == ROM::TEST_ROM_TAP)
        || (ROM_TYPE == ROM::TEST_ROM_BIN)
        || (ROM_TYPE == ROM::TEST_SST));
#endif
}

// =========================================================
// INPUT — EmuKey / EmuKeyAction / IInputBackend / KeyBindings
// =========================================================

enum class EmuKey
{
    // D-pad / general
    UP, DOWN, LEFT, RIGHT,

    // Buttons
    A, B, X, Y,
    START, SELECT,
    L, R,

    // Keypad (0x0–0xF)
    K0, K1, K2, K3,
    K4, K5, K6, K7,
    K8, K9,
    KA, KB, KC, KD,
    KE, KF,

    // Others
    KT, KQ,

    // signs
    Kp, Kn,

    UNKNOWN
};

enum class EmuKeyAction 
{
    PRESSED, RELEASED
};

class IInputBackend
{
public:
    virtual bool isDown(EmuKey key) const = 0;
    virtual bool isPressed(EmuKey key) const = 0;
    virtual bool isReleased(EmuKey key) const = 0;

    virtual ~IInputBackend() = default;
};

#ifndef __RPI_PICO__
class ImGuiInputBackend : public IInputBackend
{
public:

    bool isDown(EmuKey key) const override {
        RETURN ImGui::IsKeyDown(toImGui(key));
    }
    bool isPressed(EmuKey key) const override {
        RETURN ImGui::IsKeyPressed(toImGui(key));
    }
    bool isReleased(EmuKey key) const override {
        RETURN ImGui::IsKeyReleased(toImGui(key));
    }

private:

    ImGuiKey toImGui(EmuKey key) const
    {
        switch (key)
        {
        case EmuKey::UP:    RETURN ImGuiKey_UpArrow;
        case EmuKey::DOWN:  RETURN ImGuiKey_DownArrow;
        case EmuKey::LEFT:  RETURN ImGuiKey_LeftArrow;
        case EmuKey::RIGHT: RETURN ImGuiKey_RightArrow;

        case EmuKey::A:     RETURN ImGuiKey_Z;
        case EmuKey::B:     RETURN ImGuiKey_X;

        case EmuKey::K0: RETURN ImGuiKey_Keypad0;
        case EmuKey::K1: RETURN ImGuiKey_Keypad1;
        case EmuKey::K2: RETURN ImGuiKey_Keypad2;
        case EmuKey::K3: RETURN ImGuiKey_Keypad3;
        case EmuKey::K4: RETURN ImGuiKey_Keypad4;
        case EmuKey::K5: RETURN ImGuiKey_Keypad5;
        case EmuKey::K6: RETURN ImGuiKey_Keypad6;
        case EmuKey::K7: RETURN ImGuiKey_Keypad7;
        case EmuKey::K8: RETURN ImGuiKey_Keypad8;
        case EmuKey::K9: RETURN ImGuiKey_Keypad9;

        case EmuKey::KA: RETURN ImGuiKey_A;
        case EmuKey::KB: RETURN ImGuiKey_B;
        case EmuKey::KC: RETURN ImGuiKey_C;
        case EmuKey::KD: RETURN ImGuiKey_D;
        case EmuKey::KE: RETURN ImGuiKey_E;
        case EmuKey::KF: RETURN ImGuiKey_F;

        case EmuKey::KQ: RETURN ImGuiKey_Q;
        case EmuKey::KT: RETURN ImGuiKey_T;

        case EmuKey::Kp: RETURN ImGuiKey_KeypadAdd;
        case EmuKey::Kn: RETURN ImGuiKey_KeypadSubtract;

        default: RETURN ImGuiKey_None;
        }
    }
};
#else
class PicoInputBackend : public IInputBackend
{
public:

    bool isDown(EmuKey key) const override {
        RETURN readGPIOState(key);
    }
    bool isPressed(EmuKey key) const override {
        RETURN readGPIOPressed(key);
    }
    bool isReleased(EmuKey key) const override {
        RETURN readGPIOReleased(key);
    }

private:

    bool readGPIOState(EmuKey) const {
        RETURN false;
    }
    bool readGPIOPressed(EmuKey) const {
        RETURN false;
    }
    bool readGPIOReleased(EmuKey) const {
        RETURN false;
    }
};
#endif

// ---- KeyBindings ----------------------------------------
// Desktop uses SDL scancodes as key codes.
// Pico    uses GPIO pin numbers as key codes.
// The rest of the interface is identical.

class KeyBindings
{
public:

    void setDefault(EMULATION_ID id)
    {
#ifdef __RPI_PICO__
        // ---- Default GPIO pin mapping for Pico ----
        // Adjust these to match your physical wiring.
        // Pin numbers are just uint32_t keys here.
        bindings[id] =
        {
            { 10, EmuKey::A      },   // GPIO 10 → A
            { 11, EmuKey::B      },   // GPIO 11 → B
            { 12, EmuKey::START  },   // GPIO 12 → START
            { 13, EmuKey::SELECT },   // GPIO 13 → SELECT
            { 14, EmuKey::UP     },   // GPIO 14 → UP
            { 15, EmuKey::DOWN   },   // GPIO 15 → DOWN
            { 16, EmuKey::LEFT   },   // GPIO 16 → LEFT
            { 17, EmuKey::RIGHT  },   // GPIO 17 → RIGHT
        };
#else
        // ---- Default SDL scancode mapping ----------
        bindings[id] =
        {
            { SDL_SCANCODE_Z,      EmuKey::A      },
            { SDL_SCANCODE_X,      EmuKey::B      },
            { SDL_SCANCODE_RETURN, EmuKey::START  },
            { SDL_SCANCODE_SPACE,  EmuKey::SELECT },
            { SDL_SCANCODE_UP,     EmuKey::UP     },
            { SDL_SCANCODE_DOWN,   EmuKey::DOWN   },
            { SDL_SCANCODE_LEFT,   EmuKey::LEFT   },
            { SDL_SCANCODE_RIGHT,  EmuKey::RIGHT  },
        };
#endif
    }

    EmuKey resolve(EMULATION_ID id, int keyCode) const
    {
        auto sysIt = bindings.find(id);
        if (sysIt == bindings.end()) RETURN EmuKey::UNKNOWN;
        auto keyIt = sysIt->second.find(keyCode);
        if (keyIt == sysIt->second.end()) RETURN EmuKey::UNKNOWN;
        RETURN keyIt->second;
    }

    void rebind(EMULATION_ID id, EmuKey key, int newKeyCode)
    {
        auto& map = bindings[id];
        for (auto it = map.begin(); it != map.end(); )
            it = (it->second == key) ? map.erase(it) : std::next(it);
        map[newKeyCode] = key;
    }

    int getCurrentCode(EMULATION_ID id, EmuKey key) const
    {
        auto sysIt = bindings.find(id);
        if (sysIt == bindings.end()) RETURN INVALID;
        for (auto& [code, k] : sysIt->second)
            if (k == key) RETURN code;
        RETURN INVALID;
    }

    // load() / save() use boost::property_tree on desktop.
    // On Pico settings come from pico_config.h — no-ops here.
#ifndef __RPI_PICO__
    void load(MasqConfig_t& config, EMULATION_ID id);
    void save(MasqConfig_t& config, EMULATION_ID id);
#endif

private:
    // Key code type is int on both platforms (GPIO pin or SDL scancode).
    std::unordered_map<EMULATION_ID,
        std::unordered_map<int, EmuKey>,
        EmulationIDHash> bindings;
};

// ---- OpenGL shader helpers (desktop only) ----------------
// Shader strings and GL utilities are entirely absent on Pico.
#ifndef __RPI_PICO__

#if (GL_FIXED_FUNCTION_PIPELINE == NO)

enum class SHADER_TYPE {
    NONE = -ONE, VERTEX = ZERO, FRAGMENT = ONE
};
typedef struct {
    std::string vertexSource; std::string fragmentSource;
} shaderProgramSource_t;

// Default passthrough shader sources (unchanged from original)
const std::string defaultPassthroughVertexShaderSrc =
"#if defined(WEBGL)\n#version 300 es\nprecision mediump float;\n#else\n#version 330 core\n#endif\n"
"layout(location=0) in vec2 pos;\nlayout(location=1) in vec2 uv;\nout vec2 TexCoord;\n"
"void main(){TexCoord=uv;gl_Position=vec4(pos,0.0,1.0);}\n";

const std::string defaultPassthroughFragmentShaderSrc =
"#if defined(WEBGL)\n#version 300 es\nprecision mediump float;\n#else\n#version 330 core\n#endif\n"
"in vec2 TexCoord;out vec4 FragColor;uniform sampler2D u_Texture;\n"
"void main(){FragColor=texture(u_Texture,TexCoord);}\n";

const std::string defaultBlendVertexShaderSrc =
"#if defined(WEBGL)\n#version 300 es\nprecision mediump float;\n#else\n#version 330 core\n#endif\n"
"layout(location=0) in vec2 pos;\nlayout(location=1) in vec2 uv;\nout vec2 TexCoord;\n"
"void main(){TexCoord=uv;gl_Position=vec4(pos,0.0,1.0);}\n";

const std::string defaultBlendFragmentShaderSrc =
"#if defined(WEBGL)\n#version 300 es\nprecision mediump float;\n#else\n#version 330 core\n#endif\n"
"in vec2 TexCoord;out vec4 FragColor;uniform sampler2D u_Texture;uniform float u_Alpha;uniform vec2 u_TexelSize;\n"
"void main(){vec2 scaledUV=gl_FragCoord.xy*u_TexelSize;vec4 texColor=texture(u_Texture,scaledUV);FragColor=vec4(texColor.rgb,texColor.a*u_Alpha);}\n";

MASQ_INLINE void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}
MASQ_INLINE FLAG GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        LOG("[OpenGL Error] (%d) : in function %s (file %s) at line %u", error, function, file, line); RETURN FAILURE;
    }
    RETURN SUCCESS;
}

MASQ_INLINE void stripBlock(std::string& source,
    const std::string& startTag,
    const std::string& endTag)
{
    size_t start = source.find(startTag);
    while (start != std::string::npos)
    {
        size_t end = source.find(endTag, start);
        if (end == std::string::npos) BREAK;
        source.erase(start, end + endTag.length() - start);
        start = source.find(startTag);
    }
}

MASQ_INLINE shaderProgramSource_t parseShader(const std::string& filepath)
{
    std::string vertex, fragment;
    std::ifstream stream(filepath);
    if (!stream.is_open())
    {
        LOG("Failed to open shader file: %s — using defaults", filepath.c_str());
        if (filepath.find("blend") != std::string::npos)
        {
            vertex = defaultBlendVertexShaderSrc; fragment = defaultBlendFragmentShaderSrc;
        }
        else
        {
            vertex = defaultPassthroughVertexShaderSrc; fragment = defaultPassthroughFragmentShaderSrc;
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
                type = (line.find("vertex") != std::string::npos) ? SHADER_TYPE::VERTEX : SHADER_TYPE::FRAGMENT;
            else
                ss[TO_UINT(type)] << line << '\n';
        }
        vertex = ss[0].str(); fragment = ss[1].str();
    }

    auto removeLine = [](std::string& src, const std::string& marker) {
        size_t pos = 0;
        while ((pos = src.find(marker, pos)) != std::string::npos)
        {
            size_t end = src.find('\n', pos); src.erase(pos, (end == std::string::npos ? src.size() - pos : end - pos + 1));
        }
        };

#ifndef __EMSCRIPTEN__
    stripBlock(vertex, "#if defined(WEBGL)", "#else");
    stripBlock(fragment, "#if defined(WEBGL)", "#else");
#else
    stripBlock(vertex, "#else", "#endif");
    stripBlock(fragment, "#else", "#endif");
#endif
    for (auto* s : { &vertex, &fragment })
        for (auto& tag : { std::string("#if defined(WEBGL)"), std::string("#else"), std::string("#endif") })
            removeLine(*s, tag);

    RETURN{ vertex, fragment };
}

MASQ_INLINE uint32_t compileShader(uint32_t type, const std::string& source)
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
        char* message = (char*)alloca((size_t)length * sizeof(char));
        GL_CALL(glGetShaderInfoLog(id, length, &length, message));
        LOG("Failed to compile %s shader!\n%s", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), message);
        GL_CALL(glDeleteShader(id));
        RETURN ZERO;
    }
    RETURN id;
}

MASQ_INLINE uint32_t createShader(const std::string& vertexShader,
    const std::string& fragmentShader)
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
#endif // GL_FIXED_FUNCTION_PIPELINE == NO

#if DEACTIVATED
MASQ_INLINE int EstimateConvertedOutputBytes(
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
        RETURN - 1; // Unsupported input format
    }
    int inputFrameSize = inputSampleSize * inputChannels;

    if (inputFrameSize == 0) RETURN - 1;

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
        RETURN - 1; // Unsupported output format
    }

    int outputFrameSize = outputSampleSize * outputChannels;
    RETURN outputFrames* outputFrameSize;
}

MASQ_INLINE int CalculateMaxSafeBufferSize(SDL_AudioDeviceID device, float duration_seconds)
{
    SDL_AudioSpec outputSpec;
    if (SDL_GetAudioDeviceFormat(device, &outputSpec, NULL) < 0)
    {
        SDL_Log("Failed to get output device format: %s", SDL_GetError());
        RETURN - 1;
    }

    int bytes_per_sample = SDL_AUDIO_BITSIZE(outputSpec.format) / 8;

    // Validate format
    if (!SDL_AUDIO_ISFLOAT(outputSpec.format) && !SDL_AUDIO_ISSIGNED(outputSpec.format))
    {
        SDL_Log("Unsupported format: 0x%X", outputSpec.format);
        RETURN - 1;
    }

    int bytes_per_frame = bytes_per_sample * outputSpec.channels;
    int max_safe_bytes = (int)(outputSpec.freq * duration_seconds * bytes_per_frame);

    RETURN max_safe_bytes;
}
#endif

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

        RETURN hash;
    }

    static std::string toHexString(const std::array<uint8_t, 20>& digest)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (uint8_t b : digest)
            ss << std::setw(2) << static_cast<int>(b);
        RETURN ss.str();
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
        RETURN(value << bits) | (value >> (32 - bits));
    }

private:
    uint32_t m_h[5]{};
    std::array<uint8_t, 64> m_block{};
    size_t m_blockByteIndex{};
    size_t m_messageByteLength{};
};

#endif // !__RPI_PICO__ (shader section)

#endif // !ENABLE_OTA_EXECUTABLE