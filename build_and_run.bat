@echo off
echo Building PixelForge...

REM Create necessary directories if they don't exist
if not exist build mkdir build

REM Check if MinGW or Visual Studio is available
where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using MinGW GCC compiler...
    g++ -std=c++17 -Wall ^
        src/main.cpp ^
        src/core/application.cpp ^
        src/ui/main_window.cpp ^
        -o build/PixelForge.exe ^
        -lopengl32 -lgdi32 -luser32 -lcomdlg32 ^
        -mwindows
    goto :run
)

where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using Visual Studio compiler...
    cl /EHsc /std:c++17 ^
        src/main.cpp ^
        src/core/application.cpp ^
        src/ui/main_window.cpp ^
        /Fe:build\PixelForge.exe ^
        /link opengl32.lib user32.lib gdi32.lib comdlg32.lib ^
        /SUBSYSTEM:WINDOWS
    goto :run
)

echo Error: Neither Visual Studio nor MinGW compiler found.
echo Please install Visual Studio or MinGW and add to PATH, or compile manually.
goto :end

:run
if exist build\PixelForge.exe (
    echo Build successful! Running PixelForge...
    cd build
    start "" PixelForge.exe
    cd ..
) else (
    echo Build failed. Please check the errors above.
)

:end 