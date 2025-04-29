#pragma once

#include <windows.h>
#include <string>
#include <memory>
#include "../ui/main_window.h"

namespace PixelForge {

class Application {
public:
    Application(HINSTANCE hInstance);
    ~Application();

    bool Initialize();
    int Run();

private:
    HINSTANCE m_hInstance;
    std::unique_ptr<MainWindow> m_mainWindow;
};

} // namespace PixelForge 