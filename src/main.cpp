#include <windows.h>
#include <commctrl.h>   // Windows common controls
#include <stdio.h>
#include "core/application.h"

// Initialize common controls - MinGW compatible approach
#if defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__)
// MinGW doesn't support pragma comment
#else
#pragma comment(lib, "comctl32.lib")
#endif

#ifdef DEBUG
// Debug console for logging
void EnableDebugConsole() {
    AllocConsole();
    FILE* pConsole;
    freopen_s(&pConsole, "CONOUT$", "w", stdout);
    printf("PixelForge Debug Console\n");
    printf("------------------------\n");
}
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    #ifdef DEBUG
    EnableDebugConsole();
    printf("Starting application in debug mode\n");
    printf("HINSTANCE: %p\n", hInstance);
    #endif
    
    // Initialize common controls - simpler method
    InitCommonControls();
    
    #ifdef DEBUG
    printf("Common controls initialized\n");
    #endif
    
    // Create the application instance
    PixelForge::Application app(hInstance);
    
    #ifdef DEBUG
    printf("Created application instance\n");
    #endif
    
    // Initialize the application
    if (!app.Initialize()) {
        #ifdef DEBUG
        printf("ERROR: Application initialization failed!\n");
        #endif
        MessageBoxW(NULL, L"Application initialization failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }
    
    #ifdef DEBUG
    printf("Application initialized successfully\n");
    printf("Starting main loop\n");
    #endif
    
    // Run the application
    return app.Run();
} 