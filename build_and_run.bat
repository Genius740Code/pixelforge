@echo off
echo Building PixelForge...

REM Check if MinGW or Visual Studio is available
where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using MinGW GCC compiler...
    g++ -std=c++17 -Wall src/main.cpp -o PixelForge.exe -lopengl32 -lgdi32 -luser32
    goto :run
)

where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Using Visual Studio compiler...
    cl /EHsc src/main.cpp /link opengl32.lib user32.lib gdi32.lib
    goto :run
)

echo Error: Neither Visual Studio nor MinGW compiler found.
echo Please install Visual Studio or MinGW and add to PATH, or compile manually.
goto :end

:run
if exist PixelForge.exe (
    echo Build successful! Running PixelForge...
    start PixelForge.exe
) else (
    echo Build failed. Please check the errors above.
)

:end
pause 