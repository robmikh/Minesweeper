#include "pch.h"
#include "msweepcore.h"
#include "MainWindow.h"
#include <Windowsx.h>

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Graphics;
}

const std::wstring MainWindow::ClassName = L"Minesweeper.Win32.MainWindow";

void MainWindow::RegisterWindowClass()
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = instance;
    wcex.hIcon = LoadIconW(instance, IDI_APPLICATION);
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszClassName = ClassName.c_str();
    wcex.hIconSm = LoadIconW(instance, IDI_APPLICATION);
    winrt::check_bool(RegisterClassExW(&wcex));
}

MainWindow::MainWindow(
    std::wstring const& titleString,
    std::shared_ptr<IMinesweeper> const& game,
    winrt::Windows::Graphics::SizeInt32 const& windowSize)
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    m_game = game;

    winrt::check_bool(CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP, ClassName.c_str(), titleString.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, windowSize.Width, windowSize.Height, nullptr, nullptr, instance, this));
    WINRT_ASSERT(m_window);

    ShowWindow(m_window, SW_SHOWDEFAULT);
    UpdateWindow(m_window);
}

LRESULT MainWindow::MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam)
{
    if (WM_DESTROY == message)
    {
        PostQuitMessage(0);
        return 0;
    }

    switch (message)
    {
    case WM_MOUSEMOVE:
    {
        auto rawX = GET_X_LPARAM(lparam);
        auto rawY = GET_Y_LPARAM(lparam);
        winrt::float2 point = { (float)rawX, (float)rawY };
        m_game->OnPointerMoved(point);
    }
        break;
    case WM_SIZE:
    case WM_SIZING:
    {
        auto windowSize = GetWindowSize();
        m_game->OnParentSizeChanged({ (float)windowSize.Width, (float)windowSize.Height });
    }
        break;
    case WM_LBUTTONDOWN:
        m_game->OnPointerPressed(false, false);
        break;
    case WM_RBUTTONDOWN:
        m_game->OnPointerPressed(true, false);
        break;
    }

    return base_type::MessageHandler(message, wparam, lparam);
}

winrt::Windows::Graphics::SizeInt32 MainWindow::GetWindowSize()
{
    RECT rect = {};
    winrt::check_bool(GetClientRect(m_window, &rect));
    auto windowWidth = rect.right - rect.left;
    auto windowHeight = rect.bottom - rect.top;
    winrt::SizeInt32 windowSize = { windowWidth, windowHeight };
    return windowSize;
}