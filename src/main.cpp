#include <windows.h>
#include <gl/GL.h>
#include <iostream>

// Link with OpenGL libraries
#pragma comment(lib, "opengl32.lib")

const wchar_t* WINDOW_TITLE = L"PixelForge";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

HWND hwnd;
HDC hdc;
HGLRC hrc;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool SetupOpenGL();
void CleanupOpenGL();
void Render();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the window class
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = L"PixelForgeWindow";
    wc.style = CS_OWNDC;
    
    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Create the window
    hwnd = CreateWindowW(
        L"PixelForgeWindow",
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    if (hwnd == NULL) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Initialize OpenGL
    if (!SetupOpenGL()) {
        MessageBoxW(NULL, L"OpenGL initialization failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Main message loop
    MSG msg = {0};
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            Render();
            SwapBuffers(hdc);
        }
    }
    
    // Cleanup OpenGL
    CleanupOpenGL();
    
    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool SetupOpenGL() {
    hdc = GetDC(hwnd);
    
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32, // Color depth
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, // Depth buffer
        8,  // Stencil buffer
        0, 0, 0, 0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0) {
        return false;
    }
    
    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        return false;
    }
    
    hrc = wglCreateContext(hdc);
    if (hrc == NULL) {
        return false;
    }
    
    if (!wglMakeCurrent(hdc, hrc)) {
        return false;
    }
    
    return true;
}

void CleanupOpenGL() {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hrc);
    ReleaseDC(hwnd, hdc);
}

void Render() {
    // Clear the window with a solid color (white)
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
} 