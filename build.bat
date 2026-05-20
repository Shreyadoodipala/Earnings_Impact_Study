@echo off
setlocal enabledelayedexpansion

set CXX=g++
set CXXFLAGS=-I./include -g -Wall
set BUILD_DIR=build
set TARGET=%BUILD_DIR%\test.exe

set SRCS=test.cpp src\common\utils.cpp src\common\Matrix.cpp src\core\Stock.cpp src\data\DataLoader.cpp

if "%1"=="clean" (
    if exist %BUILD_DIR% (
        del /q %BUILD_DIR%\* 2>nul
        echo Cleaned build directory.
    )
    exit /b 0
)

if "%1"=="run" (
    call :build
    if errorlevel 1 exit /b 1
    echo.
    echo Running %TARGET%...
    .\%TARGET%
    exit /b !errorlevel!
)

call :build
exit /b !errorlevel!

:build
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
echo Building %TARGET%...
%CXX% %CXXFLAGS% -o %TARGET% %SRCS%
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)
echo Build successful: %TARGET%
exit /b 0
