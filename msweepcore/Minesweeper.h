#pragma once
class CompUI;

class Minesweeper : public IMinesweeper
{
public:
    Minesweeper(
        winrt::Windows::UI::Composition::ContainerVisual const& parentVisual,
        winrt::Windows::Foundation::Numerics::float2 parentSize);
    ~Minesweeper() override {}

    void OnPointerMoved(winrt::Windows::Foundation::Numerics::float2 point) override;
    void OnParentSizeChanged(winrt::Windows::Foundation::Numerics::float2 newSize) override;
    void OnPointerPressed(
        bool isRightButton,
        bool isEraser) override;

private:
    void NewGame(int boardWidth, int boardHeight, int mines);
    bool Sweep(int x, int y);
    void Reveal(int index);
    bool IsInBoundsAndUnmarked(int x, int y);
    void PushIfUnmarked(std::queue<int>& sweeps, int x, int y);
    void GenerateMines(int numMines, int excludeX, int excludeY);
    int GenerateIndex(int min, int max);
    bool TestSpot(int x, int y);
    int GetSurroundingMineCount(int x, int y);
    void CheckTileForMineForAnimation(int x, int y, std::queue<int>& mineIndices, int& visitedTiles, int& minesInRing);
    void PlayAnimationOnAllMines(int centerX, int centerY);
    winrt::Windows::UI::Composition::CompositionShape GetShapeFromMineCount(int count);
    winrt::Windows::UI::Composition::CompositionSpriteShape GetDotShape(
        winrt::Windows::UI::Composition::CompositionGeometry const& geometry,
        winrt::Windows::UI::Composition::CompositionColorBrush const& brush,
        winrt::Windows::Foundation::Numerics::float2 offset);
    bool CheckIfWon();

private:
    std::unique_ptr<CompUI> m_ui;

    int m_gameBoardWidth;
    int m_gameBoardHeight;
    std::unique_ptr<IndexHelper> m_indexHelper;

    std::vector<MineState> m_mineStates;
    std::vector<bool> m_mines;
    std::vector<int> m_neighborCounts;
    MineGenerationState m_mineGenerationState = MineGenerationState::Deferred;
    int m_numMines = 0;

    bool m_gameOver = false;
};