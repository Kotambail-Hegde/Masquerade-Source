cmake_minimum_required(VERSION 3.24.0)

# =========================================================
# RPI PICO — PRE-PROJECT SETUP
#
# The Pico SDK MUST be bootstrapped before project() is
# called.  It sets CMAKE_SYSTEM_NAME=PICO and injects the
# arm-none-eabi toolchain.  We detect -DRPI_PICO=ON here
# and include pico_sdk_import.cmake early so that the
# toolchain is in place when project() runs.
#
# Usage:
#   cmake -DRPI_PICO=ON -DPICO_BOARD=pico_w ..
#   (omit PICO_BOARD to default to pico_w)
# =========================================================
option(RPI_PICO "Build for Raspberry Pi Pico (bare-metal)" OFF)

if (RPI_PICO)
    message(STATUS "RPI_PICO requested — bootstrapping Pico SDK before project()")

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND NOT IS_GITHUB_ACTIONS)
        message(STATUS "Building on Windows")
        include(${CMAKE_SOURCE_DIR}/Paths_win.cmake)
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND NOT IS_GITHUB_ACTIONS)
        message(STATUS "Building on Linux")
        include(${CMAKE_SOURCE_DIR}/Paths_lin.cmake)
    endif()

    # ---- Locate pico-vscode.cmake if the VS Code Pico extension is installed
    # This sets PICO_SDK_PATH, toolchain version, picotool path etc.
    if(WIN32)
        set(_PICO_USERHOME $ENV{USERPROFILE})
    else()
        set(_PICO_USERHOME $ENV{HOME})
    endif()

    # Use explicit SDK path (from Paths.cmake or -D)
    if (NOT PICO_SDK_PATH AND DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
    endif()

    # ---- Board selection (default: pico_w; override with -DPICO_BOARD=pico)
    if (NOT DEFINED PICO_BOARD)
        set(PICO_BOARD pico_w CACHE STRING "Pico board type (e.g. pico, pico_w, pico2)")
    endif()
    message(STATUS "  PICO_BOARD = ${PICO_BOARD}")

    # ---- Pull in the Pico SDK import helper.
    # Look for pico_sdk_import.cmake next to this CMakeLists.txt first,
    # then fall back to the SDK copy via PICO_SDK_PATH.
    if (NOT PICO_SDK_PATH)
        message(FATAL_ERROR "PICO_SDK_PATH not set")
    endif()

    include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

    # ---- project() must declare ASM for Pico crt0 / boot_stage2
    project(masquerade C CXX ASM)

    # ---- Initialise the SDK (must happen after project())
    pico_sdk_init()

else()
    # Standard desktop / Emscripten / RPI5 project — C+CXX only
    project(masquerade C CXX)
endif()

# =========================================================
# GCC SAFE UTF-8 + CHARSET FIX
# =========================================================
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# =========================================================
# FORCE SAFE RC COMPILER
# =========================================================
if (WIN32 AND NOT RPI_PICO)
    find_program(CMAKE_RC_COMPILER NAMES rc.exe)
    if (NOT CMAKE_RC_COMPILER)
        find_program(CMAKE_RC_COMPILER NAMES llvm-rc windres)
    endif()
    message(STATUS "Using RC compiler: ${CMAKE_RC_COMPILER}")
endif()

# =========================================================
# MODULE SELECTION
# =========================================================
set(MASQ_ONLY "" CACHE STRING "Build ONLY these modules (semicolon separated, e.g. CHIP8;NES)")
set(_MASQ_ALL GBA GBC NES PACMAN SI CHIP8 GOL)

string(REPLACE " " "" MASQ_ONLY "${MASQ_ONLY}")

if(MASQ_ONLY STREQUAL "")
    message(STATUS "MASQ_ONLY empty -> enabling ALL modules")
    foreach(_mod ${_MASQ_ALL})
        set(MASQ_ENABLE_${_mod} ON CACHE BOOL "" FORCE)
    endforeach()
else()
    message(STATUS "MASQ_ONLY = ${MASQ_ONLY}")
    foreach(_mod ${_MASQ_ALL})
        set(MASQ_ENABLE_${_mod} OFF CACHE BOOL "" FORCE)
    endforeach()
    foreach(_mod ${MASQ_ONLY})
        list(FIND _MASQ_ALL ${_mod} _idx)
        if(_idx EQUAL -1)
            message(FATAL_ERROR "Unknown module: ${_mod}")
        endif()
        set(MASQ_ENABLE_${_mod} ON CACHE BOOL "" FORCE)
    endforeach()
endif()

# =========================================================
# TARGET PLATFORM SELECTION
# =========================================================
# RPI_PICO is already declared above (must be pre-project)
option(EMSCRIPTEN "Build for WebAssembly (Emscripten)" OFF)
option(RPI_5      "Build for Raspberry Pi 5 (GLES)"    OFF)

# Enforce exclusivity
if(RPI_PICO)
    set(EMSCRIPTEN OFF CACHE BOOL "" FORCE)
    set(RPI_5      OFF CACHE BOOL "" FORCE)
elseif(EMSCRIPTEN)
    set(RPI_5      OFF CACHE BOOL "" FORCE)
    set(RPI_PICO   OFF CACHE BOOL "" FORCE)
elseif(RPI_5)
    set(EMSCRIPTEN OFF CACHE BOOL "" FORCE)
    set(RPI_PICO   OFF CACHE BOOL "" FORCE)
endif()

# Compile-time platform defines
if (EMSCRIPTEN)
    # toolchain already defines __EMSCRIPTEN__
elseif (RPI_5)
    add_compile_definitions(__RPI_5__)
elseif (RPI_PICO)
    add_compile_definitions(__RPI_PICO__)
endif()

# =========================================================
# PANEL BACKEND SELECTION (Pico only)
# Usage: cmake -DRPI_PICO=ON -DPANEL_BACKEND=PICOLCD2 ..
#        cmake -DRPI_PICO=ON -DPANEL_BACKEND=ILI ..
# =========================================================
if (RPI_PICO)
    set(PANEL_BACKEND "PICOLCD2" CACHE STRING "Panel backend: PICOLCD2 | ILI9341 ")
    message(STATUS "Panel backend: ${PANEL_BACKEND}")

    set(PANEL_ABSTRACTION_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/panel/backends)
    if (PANEL_BACKEND STREQUAL "PICOLCD2")
        add_compile_definitions(PANEL_BACKEND_PICOLCD2)
        set(PANEL_SOURCES
            panel/backends/picolcd2/LCD_2in.c
            panel/backends/picolcd2/DEV_Config.c
            panel/backends/picolcd2/font8.c
            panel/backends/picolcd2/font12.c
            panel/backends/picolcd2/font16.c
            panel/backends/picolcd2/font20.c
            panel/backends/picolcd2/font24.c
        )
        set(PANEL_DRIVER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/panel/backends/picolcd2)
        set(PANEL_ABSTRACTION_HEADERS
            panel/backends/picolcd2_backend.h
        )
        set(PANEL_DRIVER_HEADERS
            panel/backends/picolcd2/LCD_2in.h
            panel/backends/picolcd2/DEV_Config.h
            panel/backends/picolcd2/Debug.h
            panel/backends/picolcd2/fonts.h
            panel/backends/picolcd2_backend.h
        )
    elseif (PANEL_BACKEND STREQUAL "ILI9341")
        add_compile_definitions(PANEL_BACKEND_ILI9341)
        set(PANEL_SOURCES   "")   # TODO: add ILI sources
        set(PANEL_DRIVER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/panel/backends/ili9341)
        set(PANEL_ABSTRACTION_HEADERS   panel/backends/ili9341_backend.h)
        set(PANEL_DRIVER_HEADERS   "")   # TODO: add ILI driver headers
    else()
        message(FATAL_ERROR "Unknown PANEL_BACKEND: ${PANEL_BACKEND}")
    endif()

    set(PICO_PANEL_HEADERS
        panel/panel.h
        panel/panel_interface.h
        panel/panel_utils.h
        ${PANEL_ABSTRACTION_HEADERS}
        ${PANEL_DRIVER_HEADERS}
    )
endif()

# =========================================================
# GIT COMMIT HASH
# =========================================================
if (DEFINED GIT_COMMIT_HASH)
    set(MASQ_GIT_HASH "${GIT_COMMIT_HASH}")
    message(STATUS "MASQ_GIT_HASH = ${MASQ_GIT_HASH}")
else()
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE _GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if (_GIT_HASH)
        execute_process(
            COMMAND git diff --quiet
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE _GIT_DIRTY
        )
        if (NOT _GIT_DIRTY EQUAL 0)
            set(MASQ_GIT_HASH "${_GIT_HASH}-dirty")
        else()
            set(MASQ_GIT_HASH "${_GIT_HASH}")
        endif()
    else()
        set(MASQ_GIT_HASH "nogit")
    endif()
endif()
add_compile_definitions(MASQ_GIT_HASH="${MASQ_GIT_HASH}")

# =========================================================
# GITHUB ACTIONS DETECTION
# =========================================================
set(IS_GITHUB_ACTIONS FALSE)
if(DEFINED ENV{GITHUB_ACTIONS})
    if("$ENV{GITHUB_ACTIONS}" STREQUAL "true")
        set(IS_GITHUB_ACTIONS TRUE)
    endif()
endif()

# =========================================================
# LANGUAGE STANDARDS
# =========================================================
set(CMAKE_C_STANDARD   11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (RPI_PICO)
    # Pico SDK arm-none-eabi-g++ supports C++17 reliably.
    # C++23 support is partial on GCC 14 for ARM; use 17
    # unless your emulator cores strictly need 23 features.
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================================================
# EXTERNAL LIBRARY PATHS
#
# RPI_PICO: no external libs — Pico SDK provides everything
# All other platforms: unchanged from original
# =========================================================
if (RPI_PICO)
    message(STATUS "RPI_PICO -> FULL STRIP MODE — no SDL/ImGui/Boost/STB/NFD/miniz")
    # All set to empty so no accidental linkage
    set(SDL3_INCLUDE_DIR            "")
    set(SDL3_NET_INCLUDE_DIR        "")
    set(NFD_INCLUDE_DIR             "")
    set(BOOST_INCLUDE_DIR           "")
    set(STB_INCLUDE_DIR             "")
    set(RAPID_JSON_INCLUDE_DIR      "")
    set(IMGUI_DIR                   "")
    set(IMGUI_BACKENDS_DIR          "")
    set(IMGUI_MISC_CPP_DIR          "")
    set(IMGUI_EXAMPLES_LIBS_DIR     "")

elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND NOT IS_GITHUB_ACTIONS)
    message(STATUS "Building on Windows")
    include(${CMAKE_SOURCE_DIR}/Paths_win.cmake)
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}")
    set(SDL3_INCLUDE_DIR "${SDL3_ROOT}/include")
    set(SDL3_NET_INCLUDE_DIR "${SDL3_NET_ROOT}/include")
    set(NFD_INCLUDE_DIR "${NFD_ROOT}")
    set(NFD_SRC_INCLUDE_DIR "${NFD_ROOT}/src/include")

    list(APPEND CMAKE_PREFIX_PATH
        "${SDL3_ROOT}"
        "${SDL3_NET_ROOT}"
        "${BOOST_ROOT}"
        "${NFD_ROOT}"
        "${NFD_ROOT}/build/lib/cmake/nfd"
    )
    list(APPEND CMAKE_INCLUDE_PATH "${SDL3_ROOT}/include" "${SDL3_NET_ROOT}/include")
    list(APPEND CMAKE_LIBRARY_PATH "${SDL3_ROOT}/lib" "${SDL3_NET_ROOT}/lib")

elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND NOT IS_GITHUB_ACTIONS)
    message(STATUS "Building on Linux")
    include(${CMAKE_SOURCE_DIR}/Paths_lin.cmake)

elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND IS_GITHUB_ACTIONS)
    message(STATUS "CI build (Windows)")
    file(TO_CMAKE_PATH "${BOOST_ROOT}"      BOOST_ROOT)
    file(TO_CMAKE_PATH "${IMGUI_ROOT}"      IMGUI_ROOT)
    file(TO_CMAKE_PATH "${MINIZ_ROOT}"      MINIZ_ROOT)
    file(TO_CMAKE_PATH "${STB_ROOT}"        STB_ROOT)
    file(TO_CMAKE_PATH "${RAPID_JSON_ROOT}" RAPID_JSON_ROOT)
    file(TO_CMAKE_PATH "${SDL3_ROOT}"       SDL3_ROOT)
    file(TO_CMAKE_PATH "${SDL3_NET_ROOT}"   SDL3_NET_ROOT)
    file(TO_CMAKE_PATH "${NFD_ROOT}"        NFD_ROOT)
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}")
    set(SDL3_DIR "${SDL3_ROOT}/build/lib/cmake/SDL3")
    set(SDL3_LIB_PATH "${SDL3_ROOT}/build/lib/libSDL3.a")
    set(SDL3_INCLUDE_DIR "${SDL3_ROOT}/build/include")
    set(SDL3_net_DIR "${SDL3_NET_ROOT}/build/lib/cmake/SDL3_net")
    set(SDL3_NET_LIB_PATH "${SDL3_NET_ROOT}/build/lib/libSDL3_net.a")
    set(SDL3_NET_INCLUDE_DIR "${SDL3_NET_ROOT}/build/include")
    list(APPEND CMAKE_PREFIX_PATH "${SDL3_DIR}" "${SDL3_net_DIR}")
    set(NFD_CMAKE_PATH "${NFD_ROOT}/build/lib/cmake/nfd")
    set(nfd_DIR "${NFD_ROOT}/build/lib/cmake/nfd")
    set(NFD_INCLUDE_DIR "${NFD_ROOT}")
    set(NFD_SRC_INCLUDE_DIR "${NFD_ROOT}/src/include")
    list(APPEND CMAKE_PREFIX_PATH "${nfd_DIR}")

elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND IS_GITHUB_ACTIONS)
    message(STATUS "CI build (Linux)")
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}")
    set(SDL3_DIR "${SDL3_ROOT}/build/lib/cmake/SDL3")
    set(SDL3_LIB_PATH "${SDL3_ROOT}/build/lib/libSDL3.a")
    set(SDL3_INCLUDE_DIR "${SDL3_ROOT}/build/include")
    set(SDL3_net_DIR "${SDL3_NET_ROOT}/build/lib/cmake/SDL3_net")
    set(SDL3_NET_LIB_PATH "${SDL3_NET_ROOT}/build/lib/libSDL3_net.a")
    set(SDL3_NET_INCLUDE_DIR "${SDL3_NET_ROOT}/build/include")
    list(APPEND CMAKE_PREFIX_PATH "${SDL3_DIR}" "${SDL3_net_DIR}")
    set(NFD_CMAKE_PATH "${NFD_ROOT}/build/lib/cmake/nfd")
    set(nfd_DIR "${NFD_ROOT}/build/lib/cmake/nfd")
    set(NFD_INCLUDE_DIR "${NFD_ROOT}")
    set(NFD_SRC_INCLUDE_DIR "${NFD_ROOT}/src/include")
    list(APPEND CMAKE_PREFIX_PATH "${nfd_DIR}")

else()
    message(STATUS "Unsupported OS: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

# ---- Desktop-only third-party path normalisation --------
if(NOT RPI_PICO)
    if(DEFINED IMGUI_ROOT)
        file(TO_CMAKE_PATH "${IMGUI_ROOT}" IMGUI_ROOT)
    endif()
    if(DEFINED MINIZ_ROOT)
        file(TO_CMAKE_PATH "${MINIZ_ROOT}" MINIZ_ROOT)
    endif()
    if(DEFINED STB_ROOT)
        file(TO_CMAKE_PATH "${STB_ROOT}" STB_ROOT)
    endif()
    if(DEFINED RAPID_JSON_ROOT)
        file(TO_CMAKE_PATH "${RAPID_JSON_ROOT}" RAPID_JSON_ROOT)
    endif()
    if(DEFINED EFB_ROOT)
        file(TO_CMAKE_PATH "${EFB_ROOT}" EFB_ROOT)
    endif()

    set(IMGUI_DIR              "${IMGUI_ROOT}")
    set(IMGUI_BACKENDS_DIR     "${IMGUI_ROOT}/backends")
    set(IMGUI_MISC_CPP_DIR     "${IMGUI_ROOT}/misc/cpp")
    set(IMGUI_EXAMPLES_LIBS_DIR "${IMGUI_ROOT}/examples/libs")
    set(MINIZ_DIR              "${MINIZ_ROOT}")
    set(STB_INCLUDE_DIR        "${STB_ROOT}/stb")
    set(RAPID_JSON_INCLUDE_DIR "${RAPID_JSON_ROOT}/include")
    if (EMSCRIPTEN)
        set(EFB_INCLUDE_DIR    "${EFB_ROOT}")
    endif()

    if(NOT EMSCRIPTEN)
        find_package(nfd   REQUIRED)
        find_package(SDL3  REQUIRED)
        find_package(SDL3_net  REQUIRED)
    endif()

    add_compile_definitions("IMGUI_USER_CONFIG=\"${CMAKE_SOURCE_DIR}/ui/imgui_custom_config.h\"")

    if (EMSCRIPTEN)
        add_compile_definitions(__STDC_FORMAT_MACROS=1)
        add_compile_definitions(IMGUI_IMPL_OPENGL_ES3)
    endif()
endif() # !RPI_PICO

# =========================================================
# HEADER LISTS  (IDE visibility)
# =========================================================
set(PROJECT_HEADERS
    bios/bios.h
    core/abstractEmulation.h
    core/defaults.h
    chip8/chip8.h
    spaceInvaders/spaceInvaders.h
    pacMan/pacMan.h
    nes/nes.h
    gb-gbc/gbc.h
    gba/gba.h
    gba/gba.inl
    gba/gba_opcodes.inl
    helpers/helpers.h
    cheats/cheats.h
    ui/imgui_custom_config.h
    ui/resource.h
)

set(THIRD_PARTY_HEADERS "")
set(PICO_HEADERS "")

if (NOT RPI_PICO)
    if(NOT EMSCRIPTEN)
        set(THIRD_PARTY_HEADERS_GLAD
            ui/glad/glad.h
            ui/KHR/khrplatform.h
        )
    endif()
    set(THIRD_PARTY_HEADERS_IMGUI
        ${IMGUI_DIR}/imconfig.h
        ${IMGUI_DIR}/imgui.h
        ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.h
        ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3_loader.h
        ${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.h
        ${IMGUI_MISC_CPP_DIR}/imgui_stdlib.h
        ${IMGUI_DIR}/imgui_internal.h
        ${IMGUI_DIR}/imstb_rectpack.h
        ${IMGUI_DIR}/imstb_textedit.h
        ${IMGUI_DIR}/imstb_truetype.h
    )
    set(THIRD_PARTY_HEADERS_MINIZ     ${MINIZ_DIR}/miniz.h)
    set(THIRD_PARTY_HEADERS_STB       "${STB_INCLUDE_DIR}/stb_image.h")
    set(THIRD_PARTY_HEADERS_RAPID_JSON
        "${RAPID_JSON_INCLUDE_DIR}/rapidjson/document.h"
        "${RAPID_JSON_INCLUDE_DIR}/rapidjson/filereadstream.h"
    )
    if (EMSCRIPTEN)
        set(THIRD_PARTY_HEADERS_EFB "${EFB_INCLUDE_DIR}/emscripten_browser_file.h")
    endif()
    list(APPEND THIRD_PARTY_HEADERS
        ${THIRD_PARTY_HEADERS_GLAD}
        ${THIRD_PARTY_HEADERS_IMGUI}
        ${THIRD_PARTY_HEADERS_MINIZ}
        ${THIRD_PARTY_HEADERS_NFD}
        ${THIRD_PARTY_HEADERS_STB}
        ${THIRD_PARTY_HEADERS_RAPID_JSON}
    )
    if (EMSCRIPTEN)
        list(APPEND THIRD_PARTY_HEADERS ${THIRD_PARTY_HEADERS_EFB})
    endif()
else()
    list(APPEND PICO_HEADERS
        pico/pico_config.h
        pico/pico_rom.h
        ${PICO_PANEL_HEADERS}          # ← panel headers for IDE visibility
    )
endif()

# =========================================================
# SOURCE FILE LISTS
# =========================================================
set(CORE_SOURCES core/masquerade.cpp)

# --- Emulator modules (feature-controlled) ---------------
set(GOL_SOURCES "")
if(MASQ_ENABLE_GOL)
    list(APPEND GOL_SOURCES gameOfLife/gameOfLife.cpp)
endif()

set(CHIP8_SOURCES "")
if(MASQ_ENABLE_CHIP8)
    list(APPEND CHIP8_SOURCES chip8/chip8.cpp chip8/chip8_mods.cpp)
endif()

set(SI_SOURCES "")
if(MASQ_ENABLE_SI)
    list(APPEND SI_SOURCES
        spaceInvaders/spaceInvaders.cpp
        spaceInvaders/spaceInvaders_mods.cpp
        spaceInvaders/spaceInvaders_opcodes.cpp
    )
endif()

set(PACMAN_SOURCES "")
if(MASQ_ENABLE_PACMAN)
    list(APPEND PACMAN_SOURCES
        pacMan/pacMan.cpp
        pacMan/pacMan_mods.cpp
        pacMan/pacMan_opcodes.cpp
    )
endif()

set(NES_SOURCES "")
if(MASQ_ENABLE_NES)
    list(APPEND NES_SOURCES
        nes/nes.cpp
        nes/nes_mods.cpp
        nes/nes_opcodes.cpp
    )
endif()

set(GBC_SOURCES "")
if(MASQ_ENABLE_GBC)
    list(APPEND GBC_SOURCES
        gb-gbc/gbc.cpp
        gb-gbc/gbc_mods.cpp
        gb-gbc/gbc_opcodes.cpp
    )
endif()

set(GBA_SOURCES "")
if(MASQ_ENABLE_GBA)
    list(APPEND GBA_SOURCES
        gba/gba.cpp
        gba/gba_mods.cpp
    )
endif()

# --- Desktop-only source files ---------------------------
set(CHEATS_SOURCES  "")
set(MINIZ_SOURCES   "")
set(IMGUI_SOURCES   "")
set(GLAD_SOURCES    "")
set(PLATFORM_SOURCES "")

if (NOT RPI_PICO)
    list(APPEND CHEATS_SOURCES cheats/cheats.cpp)
    list(APPEND MINIZ_SOURCES  ${MINIZ_DIR}/miniz.c)
    list(APPEND IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
        ${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.cpp
        ${IMGUI_MISC_CPP_DIR}/imgui_stdlib.cpp
    )
    if (NOT EMSCRIPTEN)
        if(NOT (CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64"))
            list(APPEND GLAD_SOURCES ui/glad.c)
            if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
                set_source_files_properties(ui/glad.c PROPERTIES COMPILE_FLAGS "-Wno-pedantic")
            endif()
        endif()
    endif()

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        include(CheckLanguage)
        check_language(RC)
        if (CMAKE_RC_COMPILER)
            enable_language(RC)
            message(STATUS "RC compiler found: ${CMAKE_RC_COMPILER}")
            set(PLATFORM_SOURCES masquerade.rc)
        else()
            message(WARNING "RC compiler not found — building without icon")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        configure_file(
            ${CMAKE_SOURCE_DIR}/masquerade.desktop.in
            ${CMAKE_BINARY_DIR}/masquerade.desktop
            @ONLY
        )
        install(FILES ${CMAKE_SOURCE_DIR}/assets/ui/icons/masquerade.png
                DESTINATION share/icons/hicolor/256x256/apps)
        install(FILES ${CMAKE_BINARY_DIR}/masquerade.desktop
                DESTINATION share/applications)
    endif()
endif()

# =========================================================
# FINAL SOURCE LIST + EXECUTABLE
# =========================================================
set(SOURCES
    ${PROJECT_HEADERS}
    ${THIRD_PARTY_HEADERS}
    ${PICO_HEADERS}
    ${CORE_SOURCES}
    ${GOL_SOURCES}
    ${CHIP8_SOURCES}
    ${SI_SOURCES}
    ${PACMAN_SOURCES}
    ${NES_SOURCES}
    ${GBC_SOURCES}
    ${GBA_SOURCES}
    ${CHEATS_SOURCES}
    ${MINIZ_SOURCES}
    ${IMGUI_SOURCES}
    ${GLAD_SOURCES}
    ${PLATFORM_SOURCES}
)

# Panel sources added separately (only on Pico)
if (RPI_PICO)
    list(APPEND SOURCES ${PANEL_SOURCES})
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

# =========================================================
# EMULATOR MODULE DEFINES
# =========================================================
foreach(_mod ${_MASQ_ALL})
    if(MASQ_ENABLE_${_mod})
        target_compile_definitions(${PROJECT_NAME} PRIVATE MASQ_ENABLE_${_mod}=1)
    else()
        target_compile_definitions(${PROJECT_NAME} PRIVATE MASQ_ENABLE_${_mod}=0)
    endif()
endforeach()

# =========================================================
# IDE SOURCE GROUPS
# =========================================================
source_group("Headers"                                      FILES ${PROJECT_HEADERS})
if (NOT EMSCRIPTEN)         
source_group("Headers\\Third-party\\GLAD"                   FILES ${THIRD_PARTY_HEADERS_GLAD})
endif()         
source_group("Headers\\Third-party\\IMGUI"                  FILES ${THIRD_PARTY_HEADERS_IMGUI})
source_group("Headers\\Third-party\\MINIZ"                  FILES ${THIRD_PARTY_HEADERS_MINIZ})
source_group("Headers\\Third-party\\NFD"                    FILES ${THIRD_PARTY_HEADERS_NFD})
source_group("Headers\\Third-party\\STB"                    FILES ${THIRD_PARTY_HEADERS_STB})
source_group("Headers\\Third-party\\RAPID_JSON"             FILES ${THIRD_PARTY_HEADERS_RAPID_JSON})
if (EMSCRIPTEN)         
source_group("Headers\\Third-party\\EFB"                    FILES ${THIRD_PARTY_HEADERS_EFB})
endif()         
if (RPI_PICO)           
source_group("Headers\\Pico"                                FILES ${PICO_HEADERS})
source_group("Headers\\Panel"                               FILES ${PANEL_HEADERS})
source_group("Headers\\Panel\\Backend"                      FILES ${PANEL_ABSTRACTION_HEADERS})
source_group("Headers\\Panel\\Backend\\${PANEL_BACKEND}"    FILES ${PANEL_DRIVER_HEADERS})
endif()         
source_group("Sources\\Core"                                FILES ${CORE_SOURCES})
source_group("Sources\\Game Of Life"                        FILES ${GOL_SOURCES})
source_group("Sources\\Chip8"                               FILES ${CHIP8_SOURCES})
source_group("Sources\\Space Invaders"                      FILES ${SI_SOURCES})
source_group("Sources\\PacMan"                              FILES ${PACMAN_SOURCES})
source_group("Sources\\NES"                                 FILES ${NES_SOURCES})
source_group("Sources\\GBC"                                 FILES ${GBC_SOURCES})
source_group("Sources\\GBA"                                 FILES ${GBA_SOURCES})
source_group("Sources\\Cheats"                              FILES ${CHEATS_SOURCES})
if (NOT EMSCRIPTEN)         
source_group("Sources\\Third-party\\Glad"                   FILES ${GLAD_SOURCES})
endif()         
source_group("Sources\\Third-party\\Miniz"                  FILES ${MINIZ_SOURCES})
source_group("Sources\\Third-party\\ImGui"                  FILES ${IMGUI_SOURCES})
if (RPI_PICO)           
source_group("Sources\\Panel\\Backend\\${PANEL_BACKEND}"    FILES ${PANEL_DRIVER_SOURCES})
endif()

# =========================================================
# OUTPUT DIRECTORIES
# =========================================================
if (NOT RPI_PICO)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# =========================================================
# COMPILER FLAGS
# =========================================================
if (RPI_PICO)
    # ---- Pico uses arm-none-eabi-g++ (GCC) ---------------
    # Pico SDK already sets -mcpu=cortex-m0plus -mthumb etc.
    # We add our standard safety flags on top.
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fsigned-char           # char is signed (matches desktop behaviour)
        -fno-strict-aliasing    # allow type punning
        -fwrapv                 # signed overflow wraps
        -fno-strict-overflow    # don't assume overflow is UB
        -Wall
        -Wextra
    )
    # Pico uses newlib which doesn't have full C++ exceptions/RTTI
    # Disable them to save flash+RAM
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "CLANG/LLVM")
    if (EMSCRIPTEN)
        message(STATUS "Using Emscripten (Clang-based)")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -mms-bitfields -fsigned-char -fno-strict-aliasing -fwrapv)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE
            -mms-bitfields -fsigned-char -fno-strict-aliasing
            -fwrapv -fno-strict-overflow
            -Wall -Wextra -Wpedantic
            -Wconversion -Wsign-conversion -Wcast-align)
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GNU compiler detected")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -mms-bitfields -fsigned-char -fno-strict-aliasing
        -fwrapv -fno-strict-overflow
        -Wall -Wextra -Wpedantic
        -Wconversion -Wsign-conversion -Wcast-align)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    message(STATUS "Intel compiler detected")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/W4 /RTC1>
        $<$<CONFIG:Release>:
            /GL /O2 /Ot /Oi /Oy /Ob2 /GS- /GT /GF
            /fp:fast /arch:AVX2 /Qpar /W3 /WX /sdl-
            /Zc:__cplusplus /Zc:inline /favor:INTEL64
            /DNDEBUG /fp:except- /MP
        >
    )
endif()

# =========================================================
# INCLUDE DIRECTORIES
# =========================================================
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/cheats
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/helpers
    ${CMAKE_SOURCE_DIR}/bios
    ${CMAKE_SOURCE_DIR}/gameOfLife
    ${CMAKE_SOURCE_DIR}/chip8
    ${CMAKE_SOURCE_DIR}/spaceInvaders
    ${CMAKE_SOURCE_DIR}/pacMan
    ${CMAKE_SOURCE_DIR}/nes
    ${CMAKE_SOURCE_DIR}/gb-gbc
    ${CMAKE_SOURCE_DIR}/gba
    ${CMAKE_SOURCE_DIR}/pico
    ${CMAKE_SOURCE_DIR}/panel
    ${PANEL_ABSTRACTION_INCLUDE_DIR}
    ${PANEL_DRIVER_INCLUDE_DIR}
)

if (NOT RPI_PICO)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
        "${BOOST_INCLUDE_DIR}"
        "${IMGUI_DIR}"
        "${IMGUI_BACKENDS_DIR}"
        "${IMGUI_MISC_CPP_DIR}"
        "${IMGUI_EXAMPLES_LIBS_DIR}"
        "${STB_INCLUDE_DIR}"
        "${RAPID_JSON_INCLUDE_DIR}"
        "${SDL3_INCLUDE_DIR}"
        "${SDL3_NET_INCLUDE_DIR}"
        "${MINIZ_DIR}"
    )
    if(EMSCRIPTEN)
        target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "${EFB_INCLUDE_DIR}")
    endif()
endif()

# =========================================================
# LINK LIBRARIES
# =========================================================
if (RPI_PICO)
    # ---- Pico SDK libraries --------------------------------
    # pico_stdlib  : stdio (UART/USB), sleep_ms, time, GPIO
    # hardware_gpio: gpio_init, gpio_set_function etc.
    # pico_multicore: optional — add if you use core1
    #
    # Add hardware_* libs here as your emulator needs them.
    target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib          # stdio + sleep + time + GPIO primitives
        hardware_gpio        # GPIO control
        hardware_timer       # tight_loop_contents, time_us_64
        hardware_pwm         # PWM (audio output if needed)
        hardware_dma         # DMA (fast pixel transfer to display)
        hardware_spi         # SPI (display bus)
        hardware_i2c         # I2C (if any peripherals need it)
        hardware_irq         # IRQ support
        hardware_clocks      # Clock configuration (e.g. set_sys_clock_khz for overclocking)
        pico_multicore       # multicore support (optional — only if you use core1 for offloading tasks)
    )

    # ---- CYW43 (Wi-Fi / onboard LED on pico_w) -----------
    # Only link if the board has the CYW43 chip
    if (PICO_CYW43_SUPPORTED)
        target_link_libraries(${PROJECT_NAME} PRIVATE
            pico_cyw43_arch_none    # CYW43 without TCP/IP (saves RAM)
        )
        target_compile_definitions(${PROJECT_NAME} PRIVATE PICO_CYW43_SUPPORTED=1)
    endif()

    pico_enable_stdio_uart(${PROJECT_NAME} 1)
    pico_enable_stdio_usb(${PROJECT_NAME} 1)   # set to 1 for USB CDC serial instead

    # ---- Generate .uf2 / .bin / .hex / .map outputs ------
    # Drop the .uf2 onto the Pico's mass-storage drive to flash
    pico_add_extra_outputs(${PROJECT_NAME})

    message(STATUS "Pico build configured:")
    message(STATUS "  Board         : ${PICO_BOARD}")
    message(STATUS "  Panel backend : ${PANEL_BACKEND}")
    message(STATUS "  UART stdio    : enabled")
    message(STATUS "  USB stdio     : enabled")
    message(STATUS "  Extra outputs : .uf2 .bin .hex .map")

    # ---------------------------------------
    # Output directory (generator-safe)
    # ---------------------------------------
    if(CMAKE_CONFIGURATION_TYPES) # multi-config (VS)
        set(PICO_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
    else() # single-config (Ninja/Make)
        set(PICO_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
    endif()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.uf2"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.bin"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.hex"
        COMMENT "Move Pico outputs → target dir (Debug/Release)"
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "CLANG/LLVM")
    if (EMSCRIPTEN)
        message(STATUS "Using Emscripten (Clang-based)")
        set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

        # Prioritize local Wasm install paths, fallback to GitHub Actions runner paths
        if (EXISTS "${SDL3_WASM_LIB}")
            target_link_libraries(${PROJECT_NAME} PRIVATE "${SDL3_WASM_LIB}")
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE "${SDL3_LIB_PATH}")
        endif()

        if (EXISTS "${SDL3_NET_WASM_LIB}")
            target_link_libraries(${PROJECT_NAME} PRIVATE "${SDL3_NET_WASM_LIB}")
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE "${SDL3_NET_LIB_PATH}")
        endif()

        # Link Emscripten network backend
        target_link_libraries(${PROJECT_NAME} PRIVATE sockets)

        target_link_options(${PROJECT_NAME} PRIVATE
            "-std=c++23" "-sUSE_SDL=3" "-sFORCE_FILESYSTEM=1" "-lidbfs.js"
            "-sWASM=1" "-sUSE_WEBGL2=1" "-sMIN_WEBGL_VERSION=2"
            "-sMAX_WEBGL_VERSION=2" "-sFULL_ES3=1" "-sWASM_BIGINT=1"
            "-sALLOW_MEMORY_GROWTH=1" "-sDISABLE_EXCEPTION_CATCHING=0"
            "-sNO_EXIT_RUNTIME=1" "-sASSERTIONS=0" "-sSAFE_HEAP=0"
            "-sSTACK_SIZE=256MB" "-sSTACK_OVERFLOW_CHECK=0" "-sUSE_GLFW=0"
            "-sINITIAL_MEMORY=512MB"
            "-sEXPORTED_RUNTIME_METHODS=[ccall]"
            "-sEXPORTED_FUNCTIONS=[_main,_malloc,_free]"
            "--shell-file=${CMAKE_SOURCE_DIR}/shell_minimal.html"
            "--preload-file=${CMAKE_SOURCE_DIR}/assets@/assets"
            "-O3" "-flto" "-msimd128" "-fno-math-errno" "-fno-rtti"
        )
    else()
        message(STATUS "Using native Clang")
        target_link_libraries(${PROJECT_NAME} PRIVATE nfd::nfd SDL3::SDL3 SDL3_net::SDL3_net)
    endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GNU compiler detected")
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        message(STATUS "Linking Windows libraries")
        target_link_libraries(${PROJECT_NAME} PRIVATE
            nfd::nfd
            SDL3::SDL3
            SDL3_net::SDL3_net
            ws2_32 
            winmm 
            user32 
            gdi32 
            opengl32 
            gdiplus 
            Shlwapi 
            dwmapi 
            stdc++fs
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if (RPI_5)
            message(STATUS "RPI_5 -> GLES (EGL backend)")
            target_link_libraries(${PROJECT_NAME} PRIVATE
                nfd::nfd SDL3::SDL3 SDL3_net::SDL3_net OpenGL::GLES3 EGL pthread dl m)
        elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
            message(STATUS "ARM -> GLES fallback")
            target_link_libraries(${PROJECT_NAME} PRIVATE
                nfd::nfd SDL3::SDL3 SDL3_net::SDL3_net OpenGL::GLES3 EGL pthread dl m)
        else()
            message(STATUS "Desktop Linux -> OpenGL")
            target_link_libraries(${PROJECT_NAME} PRIVATE
                nfd::nfd SDL3::SDL3 SDL3_net::SDL3_net GL pthread dl m)
        endif()
    endif()
    target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED __STDC_FORMAT_MACROS)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    message(STATUS "INTEL")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${PYTHON_LIBRARIES})

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "MSVC")
    target_link_libraries(${PROJECT_NAME} PRIVATE nfd::nfd SDL3::SDL3 SDL3_net::SDL3_net)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/INCREMENTAL:NO>
        $<$<CONFIG:Release>:/LTCG:PGOPTIMIZE>
        $<$<CONFIG:Release>:/USEPROFILE>
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
        $<$<CONFIG:Release>:/PGD:${CMAKE_SOURCE_DIR}/assets/internal/pgo/masquerade.pgd>
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED __STDC_FORMAT_MACROS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS>
        $<$<CONFIG:Release>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS>
    )
endif()

# =========================================================
# PRE / POST BUILD SCRIPTS (desktop only)
# =========================================================
if (NOT EMSCRIPTEN AND NOT RPI_PICO)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_custom_command(
            TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/assets/internal/pgo"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMENT "Copying all PGO files (.pgc/.pgd) to output"
        )
    endif()

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/LICENSE.md"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/LICENSE.md"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Copying dependencies..."
    )

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3>
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3_net::SDL3_net>
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endif()
endif()
