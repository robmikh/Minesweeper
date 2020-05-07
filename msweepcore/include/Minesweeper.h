#pragma once

#include "MinesweeperBoard.h"

class Minesweeper
{
public:
    Minesweeper(
        winrt::Windows::UI::Composition::ContainerVisual const& parentVisual,
        winrt::Windows::Foundation::Numerics::float2 parentSize);
    ~Minesweeper() {}

    void OnPointerMoved(winrt::Windows::Foundation::Numerics::float2 point);
    void OnParentSizeChanged(winrt::Windows::Foundation::Numerics::float2 newSize);
    void OnPointerPressed(
        bool isRightButton,
        bool isEraser);

private:
    void NewGame();
    float ComputeScaleFactor(winrt::Windows::Foundation::Numerics::float2 windowSize);
    float ComputeScaleFactor();
    void UpdateBoardScale(winrt::Windows::Foundation::Numerics::float2 windowSize);

    winrt::Windows::UI::Composition::CompositionColorBrush GetColorBrushFromMineState(MineState state);
    winrt::Windows::UI::Composition::CompositionColorBrush GetColorBrushFromMineCount(int count);

    void PlayMineAnimation(int index, winrt::Windows::Foundation::TimeSpan const& delay);
    void CheckTileForMineForAnimation(int x, int y, std::queue<int>& mineIndices, int& visitedTiles, int& minesInRing);
    void PlayAnimationOnAllMines(int centerX, int centerY);
    winrt::Windows::UI::Composition::CompositionShape GetShapeFromMineCount(int count);
    winrt::Windows::UI::Composition::CompositionSpriteShape GetDotShape(
        winrt::Windows::UI::Composition::CompositionGeometry const& geometry,
        winrt::Windows::UI::Composition::CompositionColorBrush const& brush,
        winrt::Windows::Foundation::Numerics::float2 offset);
    void GenerateAssets();

private:
    MinesweeperBoard m_board;

    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_root{ nullptr };

    winrt::Windows::UI::Composition::ContainerVisual m_gameBoard{ nullptr };
    std::vector<winrt::Windows::UI::Composition::SpriteVisual> m_tiles;
    winrt::Windows::UI::Composition::SpriteVisual m_selectionVisual{ nullptr };

    winrt::Windows::Foundation::Numerics::float2 m_tileSize;
    winrt::Windows::Foundation::Numerics::float2 m_margin;
    winrt::Windows::Foundation::Numerics::float2 m_gameBoardMargin;
    int m_currentSelectionX;
    int m_currentSelectionY;

    winrt::Windows::Foundation::Numerics::float2 m_parentSize;
    bool m_mineAnimationPlaying = false;

    winrt::Windows::UI::Composition::CompositionColorBrush m_mineBrush{ nullptr };
    std::map<MineState, winrt::Windows::UI::Composition::CompositionColorBrush> m_mineStateBrushes;
    std::map<int, winrt::Windows::UI::Composition::CompositionColorBrush> m_mineCountBackgroundBrushes;
    std::map<int, winrt::Windows::UI::Composition::CompositionShape> m_mineCountShapes;
};