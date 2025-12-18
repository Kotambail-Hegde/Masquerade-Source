cmake_minimum_required(VERSION 3.24.0)

project(masquerade)  # MUST come before checking CMAKE_SYSTEM_NAME

# --- Git commit hash with dirty state (CI-safe)
if (DEFINED GIT_COMMIT_HASH)
    # CI build (exact commit)
    set(MASQ_GIT_HASH "${GIT_COMMIT_HASH}")
    message(STATUS "MASQ_GIT_HASH = ${MASQ_GIT_HASH}")
else()
    # Local build
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
        # Not a git repo at all (zip/tarball/manual copy)
        set(MASQ_GIT_HASH "nogit")
    endif()
endif()

add_compile_definitions(MASQ_GIT_HASH="${MASQ_GIT_HASH}")

# Detect if we are running inside GitHub Actions
set(IS_GITHUB_ACTIONS FALSE)
if(DEFINED ENV{GITHUB_ACTIONS})
    if("$ENV{GITHUB_ACTIONS}" STREQUAL "true")
        set(IS_GITHUB_ACTIONS TRUE)
    endif()
endif()

# --- Automatically setup the required libraries path from root path
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND NOT IS_GITHUB_ACTIONS)
    message(STATUS "Building on Windows")
    include(${CMAKE_SOURCE_DIR}/Paths_win.cmake)
    # Boost
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}")
    # SDL3
    set(SDL3_CMAKE_PATH "${SDL3_ROOT}/cmake")
    set(SDL3_LIB_PATH "${SDL3_ROOT}/lib/libSDL3.a") # For Emscripten
    set(SDL3_INCLUDE_DIR "${SDL3_ROOT}/include")
    set(CMAKE_PREFIX_PATH "${SDL3_CMAKE_PATH}")
    # Nativefiledialog-extended
    set(NFD_CMAKE_PATH "${NFD_ROOT}/build/lib/cmake/nfd")
    set(nfd_DIR "${NFD_ROOT}/build/lib/cmake/nfd")
    set(NFD_INCLUDE_DIR "${NFD_ROOT}")
    set(NFD_SRC_INCLUDE_DIR "${NFD_ROOT}/src/include")
    set(CMAKE_PREFIX_PATH "${NFD_CMAKE_PATH}")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND NOT IS_GITHUB_ACTIONS)
    message(STATUS "Building on Linux")
    include(${CMAKE_SOURCE_DIR}/Paths_lin.cmake)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND IS_GITHUB_ACTIONS)
    message(STATUS "CI build (Windows) – skipping local Paths_*.cmake")
    # Normalize paths to forward slashes
    file(TO_CMAKE_PATH "${BOOST_ROOT}" BOOST_ROOT)
    file(TO_CMAKE_PATH "${IMGUI_ROOT}" IMGUI_ROOT)
    file(TO_CMAKE_PATH "${MINIZ_ROOT}" MINIZ_ROOT)
    file(TO_CMAKE_PATH "${STB_ROOT}" STB_ROOT)
    file(TO_CMAKE_PATH "${SDL3_ROOT}" SDL3_ROOT)
    file(TO_CMAKE_PATH "${NFD_ROOT}" NFD_ROOT)
    # Boost
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}")
    # SDL3
    set(SDL3_DIR "${SDL3_ROOT}/build/lib/cmake/SDL3")
    set(SDL3_LIB_PATH "${SDL3_ROOT}/build/lib/libSDL3.a") # For Emscripten
    set(SDL3_INCLUDE_DIR "${SDL3_ROOT}/build/include")
    list(APPEND CMAKE_PREFIX_PATH "${SDL3_DIR}")
    # Nativefiledialog-extended
    set(NFD_CMAKE_PATH "${NFD_ROOT}/build/lib/cmake/nfd")
    set(nfd_DIR "${NFD_ROOT}/build/lib/cmake/nfd")
    set(NFD_INCLUDE_DIR "${NFD_ROOT}")
    set(NFD_SRC_INCLUDE_DIR "${NFD_ROOT}/src/include")
    list(APPEND CMAKE_PREFIX_PATH "${nfd_DIR}")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND IS_GITHUB_ACTIONS)
    message(STATUS "CI build (Linux) – skipping local Paths_*.cmake")
    # Boost
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}")
    # SDL3
    set(SDL3_DIR "${SDL3_ROOT}/build/lib/cmake/SDL3")
    set(SDL3_LIB_PATH "${SDL3_ROOT}/build/lib/libSDL3.a") # For Emscripten
    set(SDL3_INCLUDE_DIR "${SDL3_ROOT}/build/include")
    list(APPEND CMAKE_PREFIX_PATH "${SDL3_DIR}")
    # Nativefiledialog-extended
    set(NFD_CMAKE_PATH "${NFD_ROOT}/build/lib/cmake/nfd")
    set(nfd_DIR "${NFD_ROOT}/build/lib/cmake/nfd")
    set(NFD_INCLUDE_DIR "${NFD_ROOT}")
    set(NFD_SRC_INCLUDE_DIR "${NFD_ROOT}/src/include")
    list(APPEND CMAKE_PREFIX_PATH "${nfd_DIR}")
else()
    message(STATUS "Unsupported OS: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

# Normalize all root paths for consistent path handling across platforms
if(DEFINED IMGUI_ROOT)
    file(TO_CMAKE_PATH "${IMGUI_ROOT}" IMGUI_ROOT)
endif()
if(DEFINED MINIZ_ROOT)
    file(TO_CMAKE_PATH "${MINIZ_ROOT}" MINIZ_ROOT)
endif()
if(DEFINED STB_ROOT)
    file(TO_CMAKE_PATH "${STB_ROOT}" STB_ROOT)
endif()
if(DEFINED EFB_ROOT)
    file(TO_CMAKE_PATH "${EFB_ROOT}" EFB_ROOT)
endif()

# ImGui
set(IMGUI_DIR "${IMGUI_ROOT}")
set(IMGUI_BACKENDS_DIR "${IMGUI_ROOT}/backends")
set(IMGUI_MISC_CPP_DIR "${IMGUI_ROOT}/misc/cpp")
set(IMGUI_EXAMPLES_LIBS_DIR "${IMGUI_ROOT}/examples/libs")
# Miniz
set(MINIZ_DIR "${MINIZ_ROOT}")
# STB
set(STB_INCLUDE_DIR "${STB_ROOT}/stb")
if (EMSCRIPTEN)
# Emscripten Browser File Library
set(EFB_INCLUDE_DIR "${EFB_ROOT}")
endif()

# --- Set language standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies
if(NOT EMSCRIPTEN)
    find_package(nfd REQUIRED)
    find_package(SDL3 REQUIRED)
endif()

# --- OpenGL ES for Raspberry Pi / ARM Linux
if(CMAKE_SYSTEM_NAME  STREQUAL "Linux")
    # Check if building for ARM architecture (Raspberry Pi)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        message(STATUS "Detected ARM architecture - using OpenGL ES 3")
        add_compile_definitions(IMGUI_IMPL_OPENGL_ES3)
        
        # Link against OpenGL ES instead of desktop OpenGL
        find_package(OpenGL REQUIRED COMPONENTS GLES3)
    else()
        message(STATUS "Detected x86/x64 architecture - using desktop OpenGL")
        find_package(OpenGL REQUIRED COMPONENTS OpenGL)
    endif()
endif()

# --- Define ImGui user config for ALL platforms with absolute path
add_compile_definitions("IMGUI_USER_CONFIG=\"${CMAKE_SOURCE_DIR}/ui/imgui_custom_config.h\"")

# --- Define format macros and OpenGL ES for Emscripten
if (EMSCRIPTEN)
    add_compile_definitions(__STDC_FORMAT_MACROS=1)
    add_compile_definitions(IMGUI_IMPL_OPENGL_ES3)
endif()

# --- Common headers just for IDE visibility
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
    helpers/helpers.h
    cheats/cheats.h
    ui/imgui_custom_config.h
    ui/resource.h
)

# --- Third-party headers for IDE visibility only
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

set(THIRD_PARTY_HEADERS_MINIZ
    ${MINIZ_DIR}/miniz.h
)

if(NOT EMSCRIPTEN)
# set(THIRD_PARTY_HEADERS_NFD
#     "${NFD_SRC_INCLUDE_DIR}/nfd.hpp"
# )
endif()

set(THIRD_PARTY_HEADERS_STB
    "${STB_INCLUDE_DIR}/stb_image.h"
)

if (EMSCRIPTEN)
set(THIRD_PARTY_HEADERS_EFB
    "${EFB_INCLUDE_DIR}/emscripten_browser_file.h"
)
endif()

set(THIRD_PARTY_HEADERS
    ${THIRD_PARTY_HEADERS_GLAD}
    ${THIRD_PARTY_HEADERS_IMGUI}
    ${THIRD_PARTY_HEADERS_MINIZ}
    ${THIRD_PARTY_HEADERS_NFD}
    ${THIRD_PARTY_HEADERS_STB}
)

if (EMSCRIPTEN)
    list(APPEND THIRD_PARTY_HEADERS
        ${THIRD_PARTY_HEADERS_EFB}
    )
endif()

# --- Core source files
set(CORE_SOURCES
    core/masquerade.cpp
)

# --- Emulator modules
set(GOL_SOURCES
    gameOfLife/gameOfLife.cpp
)

set(CHIP8_SOURCES
    chip8/chip8.cpp
    chip8/chip8_mods.cpp
)

set(SI_SOURCES
    spaceInvaders/spaceInvaders.cpp
    spaceInvaders/spaceInvaders_mods.cpp
    spaceInvaders/spaceInvaders_opcodes.cpp
)

set(PACMAN_SOURCES
    pacMan/pacMan.cpp
    pacMan/pacMan_mods.cpp
    pacMan/pacMan_opcodes.cpp
)

set(NES_SOURCES
    nes/nes.cpp
    nes/nes_mods.cpp
    nes/nes_opcodes.cpp
)

set(GBC_SOURCES
    gb-gbc/gbc.cpp
    gb-gbc/gbc_mods.cpp
    gb-gbc/gbc_opcodes.cpp
)

set(GBA_SOURCES
    gba/gba.cpp
    gba/gba_mods.cpp
    gba/gba_opcodes.cpp
)

# --- Misc modules
set(CHEATS_SOURCES
    cheats/cheats.cpp
)

# --- Third-party
set(MINIZ_SOURCES
    ${MINIZ_DIR}/miniz.c
)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.cpp
    ${IMGUI_MISC_CPP_DIR}/imgui_stdlib.cpp
)

# --- GLAD
set(GLAD_SOURCES "")
if (NOT EMSCRIPTEN)
    # Don't include GLAD for ARM/OpenGL ES builds
    if(NOT (CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64"))
        set(GLAD_SOURCES
            ui/glad.c
        )
    endif()
endif()

set(PLATFORM_SOURCES "")
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(PLATFORM_SOURCES masquerade.rc masquerade.aps)
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

# --- Final unified source list
set(SOURCES
    ${PROJECT_HEADERS}
    ${THIRD_PARTY_HEADERS}
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

# --- Add the executable target
add_executable(${PROJECT_NAME} ${SOURCES})

# --- IDE groupings for Visual Studio / CLion
source_group("Headers"                      FILES ${PROJECT_HEADERS})
if (NOT EMSCRIPTEN)
source_group("Headers\\Third-party\\GLAD"   FILES ${THIRD_PARTY_HEADERS_GLAD})
endif()
source_group("Headers\\Third-party\\IMGUI"  FILES ${THIRD_PARTY_HEADERS_IMGUI})
source_group("Headers\\Third-party\\MINIZ"  FILES ${THIRD_PARTY_HEADERS_MINIZ})
source_group("Headers\\Third-party\\NFD"    FILES ${THIRD_PARTY_HEADERS_NFD})
source_group("Headers\\Third-party\\STB"    FILES ${THIRD_PARTY_HEADERS_STB})
if (EMSCRIPTEN)
source_group("Headers\\Third-party\\EFB"    FILES ${THIRD_PARTY_HEADERS_EFB})
endif()
source_group("Sources\\Core"                FILES ${CORE_SOURCES})
source_group("Sources\\Game Of Life"        FILES ${GOL_SOURCES})
source_group("Sources\\Chip8"               FILES ${CHIP8_SOURCES})
source_group("Sources\\Space Invaders"      FILES ${SI_SOURCES})
source_group("Sources\\PacMan"              FILES ${PACMAN_SOURCES})
source_group("Sources\\NES"                 FILES ${NES_SOURCES})
source_group("Sources\\GBC"                 FILES ${GBC_SOURCES})
source_group("Sources\\GBA"                 FILES ${GBA_SOURCES})
source_group("Sources\\Cheats"              FILES ${CHEATS_SOURCES})
if (NOT EMSCRIPTEN)
source_group("Sources\\Third-party\\Glad"   FILES ${GLAD_SOURCES})
endif()
source_group("Sources\\Third-party\\Miniz"  FILES ${MINIZ_SOURCES})
source_group("Sources\\Third-party\\ImGui"  FILES ${IMGUI_SOURCES})

# --- Setup target directories
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# --- Compiler-specific flags and options
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "CLANG/LLVM")

    if (EMSCRIPTEN)
        message(STATUS "Using Emscripten (Clang-based)")
        #target_compile_options(${PROJECT_NAME} PRIVATE
        #    "-std=c++20"
        #    #"-gsource-map"
        #    #"-O0"
        #)
    else()
        message(STATUS "Using native Clang")
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # target_compile_options(${PROJECT_NAME} PRIVATE /std:c++20) # optional if not set elsewhere
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/W4 /RTC1>
        $<$<CONFIG:Release>:/GL /O2 /Ot /Oi>
    )
endif()

if(NOT EMSCRIPTEN)
    get_target_property(SDL3_INCLUDE_DIR SDL3::Headers INTERFACE_INCLUDE_DIRECTORIES)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        get_target_property(SDL3_DLL SDL3::SDL3 IMPORTED_LOCATION_RELEASE)
        get_target_property(SDL3_IMPLIB SDL3::SDL3 IMPORTED_IMPLIB_RELEASE)
    endif()
endif()

# --- Project-specific include directories
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
)

# --- Third-party/common libraries
target_include_directories(${PROJECT_NAME} PUBLIC
    "${BOOST_INCLUDE_DIR}"
    "${IMGUI_DIR}"
    "${IMGUI_BACKENDS_DIR}"
    "${IMGUI_MISC_CPP_DIR}"
    "${IMGUI_EXAMPLES_LIBS_DIR}"
    "${STB_INCLUDE_DIR}"
    "${SDL3_INCLUDE_DIR}"
    "${MINIZ_DIR}"
)

# --- Platform-specific include directories
if(EMSCRIPTEN)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${EFB_INCLUDE_DIR}"
    )
endif()

# --- Link Libraries
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "CLANG/LLVM")
    if (EMSCRIPTEN)
        message(STATUS "Using Emscripten (Clang-based)")
        set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
        target_link_libraries(${PROJECT_NAME} PRIVATE "${SDL3_LIB_PATH}")
        target_link_options(${PROJECT_NAME} PRIVATE
            "-sUSE_SDL=3"
            "-sFORCE_FILESYSTEM=1"
            "-lidbfs.js"
            "-sWASM=1"
            "-sUSE_WEBGL2=1" 
            "-sMIN_WEBGL_VERSION=2" 
            "-sMAX_WEBGL_VERSION=2" 
            "-sFULL_ES3=1" 
            "-sWASM_BIGINT=1"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sDISABLE_EXCEPTION_CATCHING=0"
            "-sNO_EXIT_RUNTIME=1" 
            #"-sASYNCIFY"
            #"-sASYNCIFY=1"
            #"-sASYNCIFY_ONLY=['main','savePersistentFS','mountPersistentFS']"
            #"-sASYNCIFY_IMPORTS=['emscripten_asm_const_int']"
            "-sASSERTIONS=0"
            "-sSAFE_HEAP=0"
            "-sSTACK_SIZE=256MB"
            "-sSTACK_OVERFLOW_CHECK=0"
            "-sUSE_GLFW=0"  # If you're using SDL and want to avoid GLFW linkage
            "-sINITIAL_MEMORY=512MB"  # Helps avoid page faults up front
            "-sEXPORTED_RUNTIME_METHODS=[ccall]" # Needed for Emscripten Browser File Library
            "-sEXPORTED_FUNCTIONS=[_main,_malloc,_free]" # Needed for Emscripten Browser File Library
            "--shell-file=${CMAKE_SOURCE_DIR}/shell_minimal.html"  # Optional custom HTML shell
            "--preload-file=${CMAKE_SOURCE_DIR}/assets@/assets"
            #"-gsource-map"
            #"-Os"
            "-O3"
            "-flto"
            "-msimd128"
            "-fno-math-errno"
            "-fno-rtti"
            "-sSINGLE_FILE=1"
        )
    else()
        message(STATUS "Using native Clang")
        target_link_libraries(${PROJECT_NAME} PRIVATE
            nfd::nfd
            SDL3::SDL3
            # SDL3::SDL3-static
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GNU compiler detected")
    if(CMAKE_SYSTEM_NAME  STREQUAL "Windows")
        message(STATUS "Linking Windows libraries")
        target_link_libraries(${PROJECT_NAME} PRIVATE
            nfd::nfd
            ${SDL3_IMPLIB}
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
    elseif(CMAKE_SYSTEM_NAME  STREQUAL "Linux")
        message(STATUS "Linking Linux libraries")
        
        # Check if using OpenGL ES or desktop OpenGL
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
            message(STATUS "Linking OpenGL ES 3 libraries for ARM")
            target_link_libraries(${PROJECT_NAME} PRIVATE
                nfd::nfd
                SDL3::SDL3
                OpenGL::GLES3  # Use GLES3 instead of GL
                EGL           # Required for OpenGL ES
                pthread
                dl
                m
            )
        else()
            message(STATUS "Linking desktop OpenGL libraries for x86/x64")
            target_link_libraries(${PROJECT_NAME} PRIVATE
                nfd::nfd
                SDL3::SDL3
                GL            # Desktop OpenGL for x86/x64
                pthread
                dl
                m
            )
        endif()
    endif()
    target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED __STDC_FORMAT_MACROS)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    message(STATUS "INTEL")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${PYTHON_LIBRARIES})
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "MSVC")
        target_link_libraries(${PROJECT_NAME} PRIVATE
        nfd::nfd
        # SDL3::SDL3-static
        "${SDL3_IMPLIB}"
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/INCREMENTAL:NO>          # Disable incremental linking (required for PGO)
        $<$<CONFIG:Release>:/LTCG:PGOPTIMIZE>         # Optimize using profile
        $<$<CONFIG:Release>:/USEPROFILE>              # Enable profile-guided optimization
        $<$<CONFIG:Release>:/PGD:${CMAKE_SOURCE_DIR}/assets/internal/pgo/masquerade.pgd>
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED __STDC_FORMAT_MACROS)
endif()

# --- PRE/POST Build Scripts
if (NOT EMSCRIPTEN)
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
    # Always copy LICENSE.md
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/LICENSE.md"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/LICENSE.md"

    # Always copy assets/
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"

    COMMENT "Copying dependencies..."
)

# Windows-only copy for SDL3.dll
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${SDL3_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL3.dll"
    )
endif()
endif()
