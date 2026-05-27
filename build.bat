@echo off
setlocal enabledelayedexpansion

set CXX=g++
set CXXFLAGS=-I./include -g -Wall -mconsole -fopenmp
set LDFLAGS=-LC:/msys64/mingw64/lib -fopenmp
set LIBS=-lcurl
set BUILD_DIR=build
set TARGET=%BUILD_DIR%\earnings_impact_study.exe

set SRCS=src\ui_visualization\Menu.cpp src\common\utils.cpp src\common\Matrix.cpp src\core\Stock.cpp src\core\Sector.cpp src\data\DataLoader.cpp src\data\StockHistory.cpp src\analysis\Calculation.cpp src\ui_visualization\Gnuplot.cpp

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
%CXX% %CXXFLAGS% -o %TARGET% %SRCS% %LDFLAGS% %LIBS%
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)
echo Build successful: %TARGET%
exit /b 0
