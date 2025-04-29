#include <windows.h>
#include "core/application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Create the application instance
    PixelForge::Application app(hInstance);
    
    // Initialize the application
    if (!app.Initialize()) {
        MessageBoxW(NULL, L"Application initialization failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }
    
    // Run the application
    return app.Run();
} 