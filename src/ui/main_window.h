#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace PixelForge {

struct ResolutionPreset {
    int width;
    int height;
    const wchar_t* label;
};

// Static map to store window references
class WindowMap {
public:
    static void Register(HWND hwnd, void* instance) {
        s_windowMap[hwnd] = instance;
    }
    
    static void* GetInstance(HWND hwnd) {
        auto it = s_windowMap.find(hwnd);
        if (it != s_windowMap.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    static void Unregister(HWND hwnd) {
        s_windowMap.erase(hwnd);
    }
    
private:
    static std::map<HWND, void*> s_windowMap;
};

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance, const std::wstring& title, int width, int height);
    ~MainWindow();

    bool Initialize();
    void Show();
    
    HWND GetHandle() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    
    void CreateControls();
    void HandleCommand(WPARAM wParam, LPARAM lParam);
    void ResizeWindow(int width, int height);
    void OpenImage();
    void DrawCanvas(HDC hdc);
    
    HWND CreateButton(const wchar_t* text, int x, int y, int width, int height, int id);
    
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    std::wstring m_title;
    int m_width;
    int m_height;
    
    RECT m_canvasRect;
    bool m_hasImage;
    
    // UI Elements
    std::vector<HWND> m_buttons;
    std::vector<ResolutionPreset> m_resolutions;
    
    // Constants
    static constexpr int BUTTON_HEIGHT = 30;
    static constexpr int BUTTON_WIDTH = 150;
    static constexpr int BUTTON_MARGIN = 10;
    static constexpr int SIDEBAR_WIDTH = 190;
    
    // Control IDs
    enum ControlIDs {
        ID_BUTTON_BASE = 100,
        ID_CUSTOM_RESOLUTION = 200,
        ID_OPEN_IMAGE = 201
    };
};

} // namespace PixelForge 