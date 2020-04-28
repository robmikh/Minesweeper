#pragma once
#include <robmikh.common/DesktopWindow.h>

class Minesweeper;

struct MainWindow : robmikh::common::desktop::DesktopWindow<MainWindow>
{
    static const std::wstring ClassName;
    static void RegisterWindowClass();
    MainWindow(
        std::wstring const& titleString, 
        std::shared_ptr<Minesweeper> const& game,
        winrt::Windows::Graphics::SizeInt32 const& windowSize);
    LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam);

private:
    winrt::Windows::Graphics::SizeInt32 GetWindowSize();

private:
    std::shared_ptr<Minesweeper> m_game;
};