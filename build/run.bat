@echo off
echo ========================================
echo Building Duality: Apocalypse
echo ========================================

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Navigate to build directory
cd build

REM Configure with CMake
echo Configuring...
cmake .. -G "Visual Studio 17 2022" -A x64

REM Check if configuration succeeded
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b %errorlevel%
)

REM Build the project
echo Building...
cmake --build . --config Release --parallel

REM Check if build succeeded
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo.
echo ========================================
echo Build successful!
echo ========================================
echo.
echo Running game...

REM Run the game
.\Release\DualityApocalypse.exe

cd ..