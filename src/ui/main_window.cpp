#include "main_window.h"
#include <commdlg.h>
#include <gdiplus.h>
#ifdef DEBUG
#include <stdio.h>
#endif

namespace PixelForge {

// Initialize static map
std::map<HWND, void*> WindowMap::s_windowMap;

static const ResolutionPreset RESOLUTION_PRESETS[] = {
    { 1280, 720, L"1280 × 720 (16:9)" },
    { 1920, 1080, L"1920 × 1080 (16:9)" },
    { 1280, 1024, L"1280 × 1024 (5:4)" },
    { 1280, 2400, L"1280 × 2400 (Phone)" },
    { 800, 1200, L"800 × 1200 (ebook)" },
    { 1200, 1200, L"1200 × 1200 (Square)" }
};

MainWindow::MainWindow(HINSTANCE hInstance, const std::wstring& title, int width, int height)
    : m_hInstance(hInstance)
    , m_hwnd(nullptr)
    , m_title(title)
    , m_width(width)
    , m_height(height)
    , m_hasImage(false)
    , m_customWidth(0)
    , m_customHeight(0) {
    
    for (const auto& preset : RESOLUTION_PRESETS) {
        m_resolutions.push_back(preset);
    }
    
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

MainWindow::~MainWindow() {
    // Unregister from map
    if (m_hwnd) {
        WindowMap::Unregister(m_hwnd);
    }
    
    // Clean up image if loaded
    if (m_pImage) {
        delete m_pImage;
        m_pImage = nullptr;
    }
    
    // Shutdown GDI+
    Gdiplus::GdiplusShutdown(m_gdiplusToken);
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
    
    // First add resolution presets
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
    
    // Add custom resolution input
    y += BUTTON_MARGIN;
    m_customLabel = CreateWindowW(
        L"STATIC", L"Custom Resolution:",
        WS_VISIBLE | WS_CHILD,
        20, y, BUTTON_WIDTH, 20,
        m_hwnd,
        NULL,
        m_hInstance,
        NULL
    );
    y += 25;
    
    // Width input
    m_widthInput = CreateWindowW(
        L"EDIT", L"1000",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
        20, y, 60, 25,
        m_hwnd,
        (HMENU)ID_CUSTOM_WIDTH,
        m_hInstance,
        NULL
    );
    
    // × symbol
    CreateWindowW(
        L"STATIC", L"×",
        WS_VISIBLE | WS_CHILD,
        85, y+5, 10, 20,
        m_hwnd,
        NULL,
        m_hInstance,
        NULL
    );
    
    // Height input
    m_heightInput = CreateWindowW(
        L"EDIT", L"1000",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
        100, y, 60, 25,
        m_hwnd,
        (HMENU)ID_CUSTOM_HEIGHT,
        m_hInstance,
        NULL
    );
    
    y += 35;
    
    // Apply custom resolution button
    m_applyButton = CreateButton(
        L"Apply Custom Size",
        20, y,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        ID_APPLY_CUSTOM
    );
    y += BUTTON_HEIGHT + BUTTON_MARGIN * 2;
    
    // Add open image button
    m_openButton = CreateButton(
        L"Open Image",
        20, y,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        ID_OPEN_IMAGE
    );
    
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
    int notificationCode = HIWORD(wParam);
    
    // Check if it's a resolution preset button
    if (controlId >= ID_BUTTON_BASE && controlId < ID_BUTTON_BASE + static_cast<int>(m_resolutions.size())) {
        int presetIndex = controlId - ID_BUTTON_BASE;
        const auto& preset = m_resolutions[presetIndex];
        
        // Resize the window
        ResizeWindow(preset.width, preset.height);
    }
    else if (controlId == ID_APPLY_CUSTOM && notificationCode == BN_CLICKED) {
        // Get values from edit controls
        wchar_t widthText[10] = {0};
        wchar_t heightText[10] = {0};
        
        GetWindowTextW(m_widthInput, widthText, 10);
        GetWindowTextW(m_heightInput, heightText, 10);
        
        int width = _wtoi(widthText);
        int height = _wtoi(heightText);
        
        // Validate input
        if (width >= 100 && width <= 10000 && height >= 100 && height <= 10000) {
            m_customWidth = width;
            m_customHeight = height;
            ResizeWindow(width, height);
        }
        else {
            MessageBoxW(m_hwnd, L"Please enter valid dimensions (100-10000 pixels)", L"Invalid Dimensions", MB_OK | MB_ICONWARNING);
        }
    }
    else if (controlId == ID_OPEN_IMAGE && notificationCode == BN_CLICKED) {
        OpenImage();
    }
}

void MainWindow::ResizeWindow(int width, int height) {
    // Update internal size
    m_width = width;
    m_height = height;
    
    // Calculate window size to account for client area and sidebar
    RECT rect = { 0, 0, width + SIDEBAR_WIDTH, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    // Get current window position
    RECT windowRect;
    GetWindowRect(m_hwnd, &windowRect);
    
    // Resize the window while maintaining position
    SetWindowPos(
        m_hwnd,
        NULL,
        windowRect.left, windowRect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        SWP_NOZORDER
    );
    
    // Update window title with current resolution
    std::wstring newTitle = m_title + L" (" + std::to_wstring(width) + L" × " + std::to_wstring(height) + L")";
    SetWindowTextW(m_hwnd, newTitle.c_str());
    
    // Force redraw
    InvalidateRect(m_hwnd, NULL, TRUE);
}

void MainWindow::OpenImage() {
    wchar_t fileName[MAX_PATH] = {0};
    
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFilter = L"Image Files\0*.jpg;*.jpeg;*.png;*.bmp;*.gif\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    
    if (GetOpenFileNameW(&ofn)) {
        // Clean up previous image if any
        if (m_pImage) {
            delete m_pImage;
            m_pImage = nullptr;
        }
        
        // Load the new image
        m_pImage = new Gdiplus::Image(fileName);
        
        if (m_pImage && m_pImage->GetLastStatus() == Gdiplus::Ok) {
            m_hasImage = true;
            
            // Get image dimensions
            int imageWidth = m_pImage->GetWidth();
            int imageHeight = m_pImage->GetHeight();
            
            // Update window to match image aspect ratio while maintaining canvas area
            ResizeWindow(imageWidth, imageHeight);
            
            // Update window title
            std::wstring fileNameOnly = fileName;
            size_t lastSlash = fileNameOnly.find_last_of(L'\\');
            if (lastSlash != std::wstring::npos) {
                fileNameOnly = fileNameOnly.substr(lastSlash + 1);
            }
            
            std::wstring newTitle = m_title + L" - " + fileNameOnly + 
                L" (" + std::to_wstring(imageWidth) + L" × " + std::to_wstring(imageHeight) + L")";
            SetWindowTextW(m_hwnd, newTitle.c_str());
        }
        else {
            MessageBoxW(m_hwnd, L"Failed to load the image.", L"Error", MB_OK | MB_ICONERROR);
            
            if (m_pImage) {
                delete m_pImage;
                m_pImage = nullptr;
            }
            
            m_hasImage = false;
        }
        
        // Force redraw
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void MainWindow::DrawCanvas(HDC hdc) {
    // Fill canvas background with white
    HBRUSH canvasBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &m_canvasRect, canvasBrush);
    DeleteObject(canvasBrush);
    
    // Calculate aspect ratio display area
    RECT aspectRect = m_canvasRect;
    
    if (m_width > 0 && m_height > 0) {
        int canvasWidth = m_canvasRect.right - m_canvasRect.left;
        int canvasHeight = m_canvasRect.bottom - m_canvasRect.top;
        
        // Calculate the maximum size that fits the canvas while maintaining aspect ratio
        float canvasRatio = (float)canvasWidth / canvasHeight;
        float aspectRatio = (float)m_width / m_height;
        
        int displayWidth, displayHeight;
        
        if (aspectRatio > canvasRatio) {
            // Width limited by canvas width
            displayWidth = canvasWidth - 40; // 20px margin on each side
            displayHeight = (int)(displayWidth / aspectRatio);
        } else {
            // Height limited by canvas height
            displayHeight = canvasHeight - 40; // 20px margin on each side
            displayWidth = (int)(displayHeight * aspectRatio);
        }
        
        // Center the aspect ratio rectangle
        int left = m_canvasRect.left + (canvasWidth - displayWidth) / 2;
        int top = m_canvasRect.top + (canvasHeight - displayHeight) / 2;
        
        aspectRect.left = left;
        aspectRect.top = top;
        aspectRect.right = left + displayWidth;
        aspectRect.bottom = top + displayHeight;
        
        // Draw checkerboard pattern for transparency
        for (int y = aspectRect.top; y < aspectRect.bottom; y += 10) {
            for (int x = aspectRect.left; x < aspectRect.right; x += 10) {
                RECT checkerRect = { x, y, x + 10, y + 10 };
                bool isLight = ((x / 10) + (y / 10)) % 2 == 0;
                HBRUSH checkerBrush = CreateSolidBrush(isLight ? RGB(240, 240, 240) : RGB(220, 220, 220));
                FillRect(hdc, &checkerRect, checkerBrush);
                DeleteObject(checkerBrush);
            }
        }
        
        // If we have an image, draw it
        if (m_hasImage && m_pImage) {
            Gdiplus::Graphics graphics(hdc);
            graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            graphics.DrawImage(m_pImage, 
                Gdiplus::Rect(aspectRect.left, aspectRect.top, 
                              aspectRect.right - aspectRect.left, 
                              aspectRect.bottom - aspectRect.top));
        }
        
        // Draw border around the aspect ratio rectangle
        HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
        HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
        
        Rectangle(hdc, 
            aspectRect.left, 
            aspectRect.top, 
            aspectRect.right, 
            aspectRect.bottom
        );
        
        SelectObject(hdc, oldPen);
        DeleteObject(borderPen);
        
        // Display resolution in the corner
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(50, 50, 50));
        
        std::wstring sizeText = std::to_wstring(m_width) + L" × " + std::to_wstring(m_height);
        RECT textRect = { aspectRect.left + 5, aspectRect.top + 5, aspectRect.right - 5, aspectRect.top + 25 };
        
        // Draw semi-transparent background for text
        RECT textBgRect = textRect;
        textBgRect.bottom = textBgRect.top + 20;
        HBRUSH textBgBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &textBgRect, textBgBrush);
        DeleteObject(textBgBrush);
        
        DrawTextW(hdc, sizeText.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE);
    } 
    else {
        // If no canvas size set, display a message
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(120, 120, 120));
        
        RECT textRect = m_canvasRect;
        DrawTextW(hdc, L"Select a resolution to begin", -1, &textRect, 
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

} // namespace PixelForge 