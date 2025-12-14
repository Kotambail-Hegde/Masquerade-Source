@echo off
setlocal ENABLEDELAYEDEXPANSION

set BUILD_DIR=build

:: === CLEAN ===
if "%1"=="clean" (
    echo Cleaning build directory...
    rmdir /s /q "%BUILD_DIR%" 2>nul
    goto :eof
)

:: === ENV SETUP ONLY ===
if "%1"=="env" (
    echo Setting up Visual Studio and Emscripten environments...
    call vcvars64.bat
    call emsdk_env.bat
    echo Environment setup complete.
    goto :eof
)

:: === ENVIRONMENT SETUP (ONCE) ===
echo Setting up Visual Studio and Emscripten environments...
call vcvars64.bat
call emsdk_env.bat
echo Environment setup complete.
echo.

:: === INDIVIDUAL SELECTION ===
if "%1"=="" goto all
if "%1"=="1" goto vs
if "%1"=="2" goto ninja_msvc
if "%1"=="3" goto nmake_msvc
if "%1"=="4" goto ninja_gcc
if "%1"=="5" goto mingw_gcc
if "%1"=="6" goto ninja_ems
if "%1"=="7" goto nmake_ems
if "%1"=="8" goto ninja_clang
if "%1"=="9" goto nmake_clang

goto :eof

:: === BUILD TARGETS ===

:vs
    echo === 1) Visual Studio Solution 2022 + MSVC ===
    cmake -G "Visual Studio 17 2022" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -S . -B "%BUILD_DIR%/x86_64/msvc/solution (IDE)/"
    goto :eof

:ninja_msvc
    echo === 2) Ninja + MSVC ===
    cmake -G "Ninja Multi-Config" -DCMAKE_DEFAULT_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -S . -B "%BUILD_DIR%/x86_64/msvc/ninja/"
    goto :eof

:nmake_msvc
    echo === 3) NMake + MSVC ===
    cmake -G "NMake Makefiles" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -S . -B "%BUILD_DIR%/x86_64/msvc/nmake/"
    goto :eof

:ninja_gcc
    echo === 4) Ninja + GCC ===
    cmake -G "Ninja Multi-Config" -DCMAKE_DEFAULT_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -S . -B "%BUILD_DIR%/x86_64/gcc/ninja/"
    goto :eof

:mingw_gcc
    echo === 5) MinGW Makefiles + GCC ===
    cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -S . -B "%BUILD_DIR%/x86_64/gcc/mingw-make/"
    goto :eof

:ninja_ems
    echo === 6) Ninja + Emscripten ===
    call emcmake cmake -G "Ninja Multi-Config" -DCMAKE_DEFAULT_BUILD_TYPE=Release -S . -B "%BUILD_DIR%/emscripten/ninja/"
    goto :eof

:nmake_ems
    echo === 7) NMake + Emscripten ===
    call emcmake cmake -G "NMake Makefiles" -S . -B "%BUILD_DIR%/emscripten/nmake/"
    goto :eof

:ninja_clang
    echo === 8) Ninja + Clang ===
    cmake -G "Ninja Multi-Config" -DCMAKE_DEFAULT_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S . -B "%BUILD_DIR%/x86_64/clang/ninja/"
    goto :eof

:nmake_clang
    echo === 9) NMake + Clang ===
    cmake -G "NMake Makefiles" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S . -B "%BUILD_DIR%/x86_64/clang/nmake/"
    goto :eof

:all
    echo ==== Running all configurations... ====
    echo.

    echo [1/9] Visual Studio Solution...
    call :vs

    echo [2/9] Ninja + MSVC...
    call :ninja_msvc

    echo [3/9] NMake + MSVC...
    call :nmake_msvc

    echo [4/9] Ninja + GCC...
    call :ninja_gcc

    echo [5/9] MinGW Makefiles + GCC...
    call :mingw_gcc

    echo [6/9] Ninja + Emscripten...
    call :ninja_ems

    echo [7/9] NMake + Emscripten...
    call :nmake_ems

    echo [8/9] Ninja + Clang...
    call :ninja_clang

    echo [9/9] NMake + Clang...
    call :nmake_clang

    echo.
    echo ==== All configurations generated ====
    pause
    goto :eof
