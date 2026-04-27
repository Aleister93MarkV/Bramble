@echo off
REM ========================================
REM Crystal Audio Player - Build Script for Windows
REM ========================================

echo Building Crystal Audio Player for Windows...
echo.

REM Check if Qt6 is available
where qmake6 >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Qt6 not found. Please install Qt6 for Windows.
    echo Download from: https://www.qt.io/download
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

REM Run qmake
echo Running qmake...
qmake6 ..\CrystalAudioPlayer.pro -o Makefile

REM Build
echo Building...
mingw32-make -j4

if %ERRORLEVEL% EQU 0 (
    echo.
    echo BUILD SUCCESSFUL!
    echo Executable: build\CrystalAudioPlayer.exe
) else (
    echo.
    echo BUILD FAILED!
    exit /b 1
)

pause
