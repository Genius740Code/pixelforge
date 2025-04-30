@echo off
echo Building PixelForge in DEBUG mode...

REM Create necessary directories if they don't exist
if not exist build mkdir build

REM Check for source files
if not exist src\main.cpp (
    echo Error: Source files missing. Please ensure you have the correct source code structure.
    goto :end
)

REM Copy any necessary resources to build directory
if exist src\resources (
    echo Copying resources...
    if not exist build\resources mkdir build\resources
    xcopy /E /Y src\resources build\resources > nul
)

REM Set include path for better compilation
set INCLUDE_PATH=-I./src

REM Make sure previous build is not running
taskkill /F /IM PixelForge_debug.exe >nul 2>&1

REM Check if MinGW or Visual Studio is available
where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using MinGW GCC compiler...
    g++ -std=c++17 -Wall -g -DDEBUG %INCLUDE_PATH% ^
        src/main.cpp ^
        src/core/application.cpp ^
        src/ui/main_window.cpp ^
        -o build/PixelForge_debug.exe ^
        -lopengl32 -lgdi32 -luser32 -lcomdlg32 -lgdiplus -lcomctl32
    set BUILD_RESULT=%ERRORLEVEL%
    goto :check_build
)

where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using Visual Studio compiler...
    cl /EHsc /std:c++17 /Zi /DDEBUG /I"./src" ^
        src/main.cpp ^
        src/core/application.cpp ^
        src/ui/main_window.cpp ^
        /Fe:build\PixelForge_debug.exe ^
        /link opengl32.lib user32.lib gdi32.lib comdlg32.lib gdiplus.lib comctl32.lib ^
        /DEBUG
    set BUILD_RESULT=%ERRORLEVEL%
    goto :check_build
)

echo Error: Neither Visual Studio nor MinGW compiler found.
echo Please install Visual Studio or MinGW and add to PATH, or compile manually.
goto :end

:check_build
if %BUILD_RESULT% EQU 0 (
    echo Build successful!
    goto :run
) else (
    echo Build failed with error code %BUILD_RESULT%. Please check the errors above.
    goto :end
)

:run
if exist build\PixelForge_debug.exe (
    echo Running PixelForge in DEBUG mode...
    cd build
    echo Debug output will appear in this console window.
    PixelForge_debug.exe
    cd ..
) else (
    echo Error: Debug executable not found after build.
    echo Build may have failed. Please check the errors above.
)

:end
pause 