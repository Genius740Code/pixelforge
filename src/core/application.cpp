#include "application.h"
#ifdef DEBUG
#include <stdio.h>
#endif

namespace PixelForge {

Application::Application(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_mainWindow(nullptr) {
    #ifdef DEBUG
    printf("Application::Constructor called\n");
    #endif
    
    // Verify hInstance is valid
    if (hInstance == NULL) {
        #ifdef DEBUG
        printf("WARNING: Received NULL hInstance\n");
        #endif
    }
}

Application::~Application() {
    #ifdef DEBUG
    printf("Application::Destructor called\n");
    #endif
    // Resources are cleaned up automatically by unique_ptr
}

bool Application::Initialize() {
    #ifdef DEBUG
    printf("Application::Initialize called\n");
    #endif
    
    // Verify we have a valid instance handle
    if (m_hInstance == NULL) {
        #ifdef DEBUG
        printf("ERROR: Cannot initialize with NULL hInstance\n");
        #endif
        return false;
    }
    
    try {
        // Create the main window
        m_mainWindow = std::make_unique<MainWindow>(m_hInstance, L"PixelForge", 1280, 750);
        
        #ifdef DEBUG
        printf("MainWindow created\n");
        #endif
        
        if (!m_mainWindow->Initialize()) {
            #ifdef DEBUG
            printf("ERROR: MainWindow initialization failed\n");
            #endif
            return false;
        }
        
        #ifdef DEBUG
        printf("MainWindow initialized successfully\n");
        #endif
        
        return true;
    } catch (const std::exception& e) {
        #ifdef DEBUG
        printf("ERROR: Exception during initialization: %s\n", e.what());
        #endif
        return false;
    } catch (...) {
        #ifdef DEBUG
        printf("ERROR: Unknown exception during initialization\n");
        #endif
        return false;
    }
}

int Application::Run() {
    #ifdef DEBUG
    printf("Application::Run called\n");
    #endif
    
    if (!m_mainWindow) {
        #ifdef DEBUG
        printf("ERROR: Cannot run with NULL mainWindow\n");
        #endif
        return 1;
    }
    
    m_mainWindow->Show();
    
    #ifdef DEBUG
    printf("MainWindow::Show called\n");
    printf("Entering message loop\n");
    #endif
    
    // Main message loop
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    #ifdef DEBUG
    printf("Exiting message loop with wParam: %lld\n", (long long)msg.wParam);
    #endif
    
    return (int)msg.wParam;
}

} // namespace PixelForge 