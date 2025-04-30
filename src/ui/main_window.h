#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <gdiplus.h>

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
    void DrawCanvas(HDC hdc);
    void OpenImage();
    
    HWND CreateButton(const wchar_t* text, int x, int y, int width, int height, int id);
    
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    std::wstring m_title;
    int m_width;
    int m_height;
    
    RECT m_canvasRect;
    
    // UI Elements
    std::vector<HWND> m_buttons;
    std::vector<ResolutionPreset> m_resolutions;
    HWND m_customLabel;
    HWND m_widthInput;
    HWND m_heightInput;
    HWND m_applyButton;
    HWND m_openButton;
    
    // Custom resolution storage
    int m_customWidth;
    int m_customHeight;
    
    // Image handling
    bool m_hasImage;
    Gdiplus::Image* m_pImage = nullptr;
    ULONG_PTR m_gdiplusToken;
    
    // Constants
    static constexpr int BUTTON_HEIGHT = 30;
    static constexpr int BUTTON_WIDTH = 150;
    static constexpr int BUTTON_MARGIN = 10;
    static constexpr int SIDEBAR_WIDTH = 190;
    
    // Control IDs
    enum ControlIDs {
        ID_BUTTON_BASE = 100,
        ID_CUSTOM_WIDTH = 200,
        ID_CUSTOM_HEIGHT = 201,
        ID_APPLY_CUSTOM = 202,
        ID_OPEN_IMAGE = 203
    };
};

} // namespace PixelForge 