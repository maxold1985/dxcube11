#include <windows.h>

#include "renderer.h"
#include "imgui.h"
#include "imgui_impl_win32.h"

static Renderer g_renderer;

LRESULT CALLBACK WindowProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    if (ImGui::GetCurrentContext() != NULL)
    {
        if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam))
        {
            return 1;
        }
    }

    switch (message)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE)
            {
                DestroyWindow(window);
            }
            return 0;
        }
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE previousInstance,
    PWSTR commandLine,
    int showCommand
)
{
    UNREFERENCED_PARAMETER(previousInstance);
    UNREFERENCED_PARAMETER(commandLine);

    const wchar_t* className = L"DX11CubeWindow";

    WNDCLASSW windowClass;
    ZeroMemory(&windowClass, sizeof(windowClass));
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassW(&windowClass))
    {
        return 1;
    }

    RECT windowRect = { 0, 0, 800, 600 };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND window = CreateWindowExW(
        0,
        className,
        L"DirectX 11 - MSVC 2013",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        instance,
        NULL
    );

    if (window == NULL)
    {
        return 1;
    }

    ShowWindow(window, showCommand);

    if (!g_renderer.Initialize(window, 800, 600))
    {
        MessageBoxW(window, L"Falha ao inicializar DirectX 11.", L"Erro", MB_ICONERROR);
        return 1;
    }

    LARGE_INTEGER frequency;
    LARGE_INTEGER previousTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&previousTime);

    MSG message;
    ZeroMemory(&message, sizeof(message));

    bool running = true;

    while (running)
    {
        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                running = false;
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        if (!running)
        {
            break;
        }

        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);

        double elapsed = static_cast<double>(
            currentTime.QuadPart - previousTime.QuadPart
        ) / static_cast<double>(frequency.QuadPart);

        previousTime = currentTime;
        g_renderer.Render(static_cast<float>(elapsed));
    }

    g_renderer.Shutdown();
    return 0;
}
