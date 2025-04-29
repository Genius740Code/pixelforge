#include "application.h"

namespace PixelForge {

Application::Application(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_mainWindow(nullptr) {
}

Application::~Application() {
    // Resources are cleaned up automatically by unique_ptr
}

bool Application::Initialize() {
    // Create the main window
    m_mainWindow = std::make_unique<MainWindow>(m_hInstance, L"PixelForge", 1280, 750);
    
    if (!m_mainWindow->Initialize()) {
        return false;
    }
    
    return true;
}

int Application::Run() {
    m_mainWindow->Show();
    
    // Main message loop
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

} // namespace PixelForge 