#include "main_window.h"
#include <commdlg.h>
#ifdef DEBUG
#include <stdio.h>
#endif

namespace PixelForge {

// Initialize static map
std::map<HWND, void*> WindowMap::s_windowMap;

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
    // Unregister from map
    if (m_hwnd) {
        WindowMap::Unregister(m_hwnd);
    }
}

bool MainWindow::Initialize() {
    #ifdef DEBUG
    printf("MainWindow::Initialize called\n");
    #endif
    
    // Generate a unique class name to avoid conflicts
    static int classCounter = 0;
    wchar_t CLASS_NAME[64];
    swprintf(CLASS_NAME, 64, L"PixelForgeMainWindow_%d", classCounter++);
    
    #ifdef DEBUG
    printf("Using window class name: %ls\n", CLASS_NAME);
    #endif
    
    // Register the window class
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    
    if (!RegisterClassW(&wc)) {
        #ifdef DEBUG
        DWORD error = GetLastError();
        
        // Class already exists - try to use it
        if (error == ERROR_CLASS_ALREADY_EXISTS) {
            printf("Window class already exists, will use existing registration\n");
        } else {
            printf("ERROR: RegisterClass failed with error code: %lu\n", error);
            return false;
        }
        #else
        // In release mode, just check if it's already registered
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
        #endif
    } else {
        #ifdef DEBUG
        printf("Window class registered successfully\n");
        #endif
    }
    
    // Calculate window size to account for client area
    RECT rect = { 0, 0, m_width, m_height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    #ifdef DEBUG
    printf("Creating window with dimensions: %dx%d (adjusted: %ldx%ld)\n", 
           m_width, m_height, 
           rect.right - rect.left, rect.bottom - rect.top);
    #endif
    
    // Create the window with centered position
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    int windowX = (screenWidth - windowWidth) / 2;
    int windowY = (screenHeight - windowHeight) / 2;
    
    #ifdef DEBUG
    printf("About to create window, this=%p\n", this);
    #endif
    
    // Try creating the window without the this pointer in CreateWindowEx
    m_hwnd = CreateWindowExW(
        0,                          // Optional window styles
        CLASS_NAME,                 // Window class
        m_title.c_str(),            // Window text
        WS_OVERLAPPEDWINDOW,        // Window style
        windowX, windowY,           // Position (centered)
        windowWidth, windowHeight,  // Width and height
        NULL,                       // Parent window
        NULL,                       // Menu
        m_hInstance,                // Instance handle
        NULL                        // Additional application data - We'll register manually
    );
    
    if (m_hwnd == NULL) {
        #ifdef DEBUG
        printf("ERROR: CreateWindowEx failed with error code: %lu\n", GetLastError());
        #endif
        return false;
    }
    
    // Register the window with our map
    WindowMap::Register(m_hwnd, this);
    
    #ifdef DEBUG
    printf("Window created successfully: handle=%p\n", m_hwnd);
    printf("Registered window in WindowMap\n");
    #endif
    
    // Create UI controls
    CreateControls();
    
    #ifdef DEBUG
    printf("UI controls created\n");
    #endif
    
    // Calculate canvas area
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    m_canvasRect = { 
        SIDEBAR_WIDTH, 
        0, 
        clientRect.right, 
        clientRect.bottom 
    };
    
    #ifdef DEBUG
    printf("Canvas rect set: left=%ld, top=%ld, right=%ld, bottom=%ld\n",
           m_canvasRect.left, m_canvasRect.top, m_canvasRect.right, m_canvasRect.bottom);
    printf("MainWindow::Initialize completed successfully\n");
    #endif
    
    return true;
}

void MainWindow::Show() {
    #ifdef DEBUG
    printf("MainWindow::Show called\n");
    printf("Showing window with handle: %p\n", m_hwnd);
    #endif
    
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    
    #ifdef DEBUG
    printf("Window should now be visible\n");
    #endif
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Get the window instance from our map
    MainWindow* pThis = reinterpret_cast<MainWindow*>(WindowMap::GetInstance(hwnd));
    
    #ifdef DEBUG
    if (msg == WM_CREATE) {
        printf("WM_CREATE received for hwnd=%p\n", hwnd);
    }
    #endif
    
    // Call instance-specific handler if we have an instance
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
    #ifdef DEBUG
    printf("MainWindow::CreateControls called\n");
    #endif
    
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
        
        if (button == NULL) {
            #ifdef DEBUG
            printf("WARNING: Failed to create button for preset: %ls\n", preset.label);
            #endif
        } else {
            #ifdef DEBUG
            printf("Created button for preset: %ls\n", preset.label);
            #endif
            m_buttons.push_back(button);
        }
        
        y += BUTTON_HEIGHT + BUTTON_MARGIN;
    }
    
    // Create "Open Image" button
    HWND openButton = CreateButton(
        L"Open Image...",
        20, y + 20,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        ID_OPEN_IMAGE
    );
    
    if (openButton == NULL) {
        #ifdef DEBUG
        printf("WARNING: Failed to create 'Open Image' button\n");
        #endif
    } else {
        #ifdef DEBUG
        printf("Created 'Open Image' button\n");
        #endif
        m_buttons.push_back(openButton);
    }
    
    #ifdef DEBUG
    printf("MainWindow::CreateControls completed\n");
    #endif
}

HWND MainWindow::CreateButton(const wchar_t* text, int x, int y, int width, int height, int id) {
    #ifdef DEBUG
    printf("Creating button: '%ls', id=%d\n", text, id);
    #endif
    
    // Make sure parent window handle is valid
    if (m_hwnd == NULL) {
        #ifdef DEBUG
        printf("ERROR: Cannot create button - parent window handle is NULL\n");
        #endif
        return NULL;
    }
    
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
    
    if (button == NULL) {
        #ifdef DEBUG
        DWORD error = GetLastError();
        printf("ERROR: CreateWindow for button failed with error code: %lu\n", error);
        #endif
    }
    
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