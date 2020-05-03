#pragma once

enum class MineState
{
    Empty = 0,
    Flag = 1,
    Question = 2,
    Revealed = 3
};

enum class MineGenerationState
{
    Deferred,
    Generated,
};

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
    void NewGame(int boardWidth, int boardHeight, int mines);
    float ComputeScaleFactor(winrt::Windows::Foundation::Numerics::float2 windowSize);
    float ComputeScaleFactor();
    void UpdateBoardScale(winrt::Windows::Foundation::Numerics::float2 windowSize);
    bool Sweep(int x, int y);
    void Reveal(int index);
    bool IsInBoundsAndUnmarked(int x, int y);
    void PushIfUnmarked(std::queue<int>& sweeps, int x, int y);
    winrt::Windows::UI::Composition::CompositionColorBrush GetColorBrushFromMineState(MineState state);
    winrt::Windows::UI::Composition::CompositionColorBrush GetColorBrushFromMineCount(int count);
    void GenerateMines(int numMines, int excludeX, int excludeY);
    int GenerateIndex(int min, int max);
    int ComputeIndex(int x, int y);
    int ComputeXFromIndex(int index);
    int ComputeYFromIndex(int index);
    bool IsInBounds(int x, int y);
    bool TestSpot(int x, int y);
    int GetSurroundingMineCount(int x, int y);
    void PlayMineAnimation(int index, winrt::Windows::Foundation::TimeSpan const& delay);
    void CheckTileForMineForAnimation(int x, int y, std::queue<int>& mineIndices, int& visitedTiles, int& minesInRing);
    void PlayAnimationOnAllMines(int centerX, int centerY);
    winrt::Windows::UI::Composition::CompositionShape GetShapeFromMineCount(int count);
    winrt::Windows::UI::Composition::CompositionSpriteShape GetDotShape(
        winrt::Windows::UI::Composition::CompositionGeometry const& geometry,
        winrt::Windows::UI::Composition::CompositionColorBrush const& brush,
        winrt::Windows::Foundation::Numerics::float2 offset);

private:
    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_root{ nullptr };

    winrt::Windows::UI::Composition::ContainerVisual m_gameBoard{ nullptr };
    std::vector<winrt::Windows::UI::Composition::SpriteVisual> m_tiles;
    winrt::Windows::UI::Composition::SpriteVisual m_selectionVisual{ nullptr };

    int m_gameBoardWidth;
    int m_gameBoardHeight;
    winrt::Windows::Foundation::Numerics::float2 m_tileSize;
    winrt::Windows::Foundation::Numerics::float2 m_margin;
    winrt::Windows::Foundation::Numerics::float2 m_gameBoardMargin;
    int m_currentSelectionX;
    int m_currentSelectionY;
    std::vector<MineState> m_mineStates;
    std::vector<bool> m_mines;
    std::vector<int> m_neighborCounts;
    winrt::Windows::Foundation::Numerics::float2 m_parentSize;
    MineGenerationState m_mineGenerationState = MineGenerationState::Deferred;
    int m_numMines = 0;

    bool m_mineAnimationPlaying = false;
    bool m_gameOver = false;
};