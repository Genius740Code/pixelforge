#include "main_window.h"
#include <commdlg.h>

namespace PixelForge {

static const ResolutionPreset RESOLUTION_PRESETS[] = {
    { 1280, 720, L"1280 x 720 (HD)" },
    { 1920, 1080, L"1920 x 1080 (Full HD)" },
    { 2560, 1440, L"2560 x 1440 (QHD)" },
    { 3840, 2160, L"3840 x 2160 (4K)" },
    { 1280, 750, L"1280 x 750 (Custom)" }
};

MainWindow::MainWindow(HINSTANCE hInstance, const std::wstring& title, int width, int height)
    : m_hInstance(hInstance)
    , m_hwnd(nullptr)
    , m_title(title)
    , m_width(width)
    , m_height(height)
    , m_hasImage(false) {
    
    for (const auto& preset : RESOLUTION_PRESETS) {
        m_resolutions.push_back(preset);
    }
}

MainWindow::~MainWindow() {
    // Cleanup handled by Windows
}

bool MainWindow::Initialize() {
    // Register the window class
    const wchar_t CLASS_NAME[] = L"PixelForgeMainWindow";
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    if (!RegisterClassW(&wc)) {
        return false;
    }
    
    // Calculate window size to account for client area
    RECT rect = { 0, 0, m_width, m_height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    // Create the window
    m_hwnd = CreateWindowExW(
        0,                          // Optional window styles
        CLASS_NAME,                 // Window class
        m_title.c_str(),            // Window text
        WS_OVERLAPPEDWINDOW,        // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, // Position
        rect.right - rect.left,     // Width
        rect.bottom - rect.top,     // Height
        NULL,                       // Parent window
        NULL,                       // Menu
        m_hInstance,                // Instance handle
        this                        // Additional application data
    );
    
    if (m_hwnd == NULL) {
        return false;
    }
    
    // Create UI controls
    CreateControls();
    
    // Calculate canvas area
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    m_canvasRect = { 
        SIDEBAR_WIDTH, 
        0, 
        clientRect.right, 
        clientRect.bottom 
    };
    
    return true;
}

void MainWindow::Show() {
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;
    
    if (msg == WM_NCCREATE) {
        // Get the pointer to the window instance
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    
    // Call instance-specific handler
    if (pThis) {
        return pThis->HandleMessage(msg, wParam, lParam);
    }
    
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            HandleCommand(wParam, lParam);
            return 0;
        
        case WM_CLOSE:
            DestroyWindow(m_hwnd);
            return 0;
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        
        case WM_SIZE: {
            // Update canvas rectangle when window size changes
            RECT clientRect;
            GetClientRect(m_hwnd, &clientRect);
            m_canvasRect = { 
                SIDEBAR_WIDTH, 
                0, 
                clientRect.right, 
                clientRect.bottom 
            };
            InvalidateRect(m_hwnd, NULL, TRUE);
            return 0;
        }
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(m_hwnd, &clientRect);
            
            // Draw sidebar background
            RECT sidebarRect = { 0, 0, SIDEBAR_WIDTH, clientRect.bottom };
            HBRUSH sidebarBrush = CreateSolidBrush(RGB(240, 240, 245));
            FillRect(hdc, &sidebarRect, sidebarBrush);
            DeleteObject(sidebarBrush);
            
            // Draw separator line
            HPEN separatorPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
            HPEN oldPen = (HPEN)SelectObject(hdc, separatorPen);
            MoveToEx(hdc, SIDEBAR_WIDTH, 0, NULL);
            LineTo(hdc, SIDEBAR_WIDTH, clientRect.bottom);
            SelectObject(hdc, oldPen);
            DeleteObject(separatorPen);
            
            // Draw app title
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(50, 50, 50));
            
            RECT titleRect = { 20, 20, SIDEBAR_WIDTH - 20, 60 };
            DrawTextW(hdc, L"PixelForge", -1, &titleRect, DT_LEFT | DT_SINGLELINE);
            
            // Draw canvas area
            DrawCanvas(hdc);
            
            EndPaint(m_hwnd, &ps);
            return 0;
        }
        
        default:
            return DefWindowProcW(m_hwnd, msg, wParam, lParam);
    }
}

void MainWindow::CreateControls() {
    // Create resolution preset buttons
    int y = 70;
    int buttonId = ID_BUTTON_BASE;
    
    for (const auto& preset : m_resolutions) {
        HWND button = CreateButton(
            preset.label,
            20, y,
            BUTTON_WIDTH, BUTTON_HEIGHT,
            buttonId++
        );
        
        m_buttons.push_back(button);
        y += BUTTON_HEIGHT + BUTTON_MARGIN;
    }
    
    // Create "Open Image" button
    HWND openButton = CreateButton(
        L"Open Image...",
        20, y + 20,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        ID_OPEN_IMAGE
    );
    
    m_buttons.push_back(openButton);
}

HWND MainWindow::CreateButton(const wchar_t* text, int x, int y, int width, int height, int id) {
    HWND button = CreateWindowW(
        L"BUTTON",                  // Button class
        text,                       // Button text
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // Styles
        x, y, width, height,        // Position and size
        m_hwnd,                     // Parent window
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), // Menu handle (used for control ID)
        m_hInstance,                // Instance handle
        NULL                        // Additional data
    );
    
    return button;
}

void MainWindow::HandleCommand(WPARAM wParam, LPARAM lParam) {
    int controlId = LOWORD(wParam);
    
    // Check if it's a resolution preset button
    if (controlId >= ID_BUTTON_BASE && controlId < ID_BUTTON_BASE + static_cast<int>(m_resolutions.size())) {
        int presetIndex = controlId - ID_BUTTON_BASE;
        const auto& preset = m_resolutions[presetIndex];
        
        // Resize the window
        ResizeWindow(preset.width, preset.height);
    } else if (controlId == ID_OPEN_IMAGE) {
        OpenImage();
    }
}

void MainWindow::ResizeWindow(int width, int height) {
    // Update internal size
    m_width = width;
    m_height = height;
    
    // Calculate window size to account for client area
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    // Resize the window
    SetWindowPos(
        m_hwnd,
        NULL,
        0, 0,
        rect.right - rect.left,
        rect.bottom - rect.top,
        SWP_NOMOVE | SWP_NOZORDER
    );
    
    // Update window title with current resolution
    std::wstring newTitle = m_title + L" (" + std::to_wstring(width) + L" x " + std::to_wstring(height) + L")";
    SetWindowTextW(m_hwnd, newTitle.c_str());
}

void MainWindow::OpenImage() {
    wchar_t fileName[MAX_PATH] = { 0 };
    
    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFilter = L"Image Files\0*.bmp;*.jpg;*.jpeg;*.png;*.gif\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"png";
    
    if (GetOpenFileNameW(&ofn)) {
        m_hasImage = true;
        InvalidateRect(m_hwnd, &m_canvasRect, TRUE);
    }
}

void MainWindow::DrawCanvas(HDC hdc) {
    // Fill canvas background with white
    HBRUSH canvasBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &m_canvasRect, canvasBrush);
    DeleteObject(canvasBrush);
    
    // Draw canvas border
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    
    Rectangle(hdc, 
        m_canvasRect.left, 
        m_canvasRect.top, 
        m_canvasRect.right, 
        m_canvasRect.bottom
    );
    
    SelectObject(hdc, oldPen);
    DeleteObject(borderPen);
    
    // If no image is loaded, display a message
    if (!m_hasImage) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(120, 120, 120));
        
        RECT textRect = m_canvasRect;
        DrawTextW(hdc, L"Select a resolution or open an image to begin", -1, &textRect, 
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

} // namespace PixelForge 