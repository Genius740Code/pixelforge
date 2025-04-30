@echo off
echo Building PixelForge...

REM Create build directory if needed
if not exist build mkdir build

REM Check for source files
if not exist src\main.cpp (
    echo Error: Source files missing. Please ensure you have the correct source code structure.
    exit /b 1
)

REM Copy resources if they exist
if exist src\resources (
    echo Copying resources...
    if not exist build\resources mkdir build\resources
    xcopy /E /Y src\resources build\resources > nul
)

REM Set include path
set INCLUDE_PATH=-I./src

REM Terminate any running instances
taskkill /F /IM PixelForge.exe >nul 2>&1

REM Check for available compilers
where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using MinGW GCC compiler...
    g++ -std=c++17 -Wall %INCLUDE_PATH% ^
        src/main.cpp ^
        src/core/application.cpp ^
        src/ui/main_window.cpp ^
        -o build/PixelForge.exe ^
        -lopengl32 -lgdi32 -luser32 -lcomdlg32 -lgdiplus -lcomctl32 ^
        -mwindows
    set BUILD_RESULT=%ERRORLEVEL%
    goto :check_build
)

where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using Visual Studio compiler...
    cl /EHsc /std:c++17 /I"./src" ^
        src/main.cpp ^
        src/core/application.cpp ^
        src/ui/main_window.cpp ^
        /Fe:build\PixelForge.exe ^
        /link opengl32.lib user32.lib gdi32.lib comdlg32.lib gdiplus.lib comctl32.lib ^
        /SUBSYSTEM:WINDOWS
    set BUILD_RESULT=%ERRORLEVEL%
    goto :check_build
)

echo Error: Neither Visual Studio nor MinGW compiler found.
echo Please install Visual Studio or MinGW and add to PATH, or compile manually.
exit /b 1

:check_build
if %BUILD_RESULT% EQU 0 (
    echo Build successful!
    echo Running PixelForge...
    cd build
    start "" PixelForge.exe
    cd ..
) else (
    echo Build failed with error code %BUILD_RESULT%. Please check the errors above.
    pause
    exit /b 1
) 