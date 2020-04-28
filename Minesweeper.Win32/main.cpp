#include "pch.h"
#include "Minesweeper.h"
#include "MainWindow.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Windows::UI::Composition::Desktop;
    using namespace Windows::Graphics;
}

namespace util
{
    using namespace robmikh::common::desktop;
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    winrt::init_apartment();

    MainWindow::RegisterWindowClass();

    // Create the DispatcherQueue that the compositor needs to run
    auto controller = util::CreateDispatcherQueueControllerForCurrentThread();

    // Initialize Composition
    auto compositor = winrt::Compositor();
    auto root = compositor.CreateContainerVisual();
    root.RelativeSizeAdjustment({ 1.0f, 1.0f });

    // Create our game
    winrt::SizeInt32 windowSize = { 800, 600 };
    auto game = std::make_shared<Minesweeper>(root, winrt::float2{ (float)windowSize.Width, (float)windowSize.Height });

    // Create our main window
    auto window = MainWindow(L"Minesweeper.Win32", game, windowSize);

    // Hookup our visual tree to the window
    auto target = window.CreateWindowTarget(compositor);
    target.Root(root);

    // Message pump
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}