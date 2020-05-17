#pragma once

class CompAssets;
class VisualGrid;
struct TileCoordinate;

class CompUI
{
public:
    CompUI(
        winrt::Windows::UI::Composition::ContainerVisual const& parentVisual,
        winrt::Windows::Foundation::Numerics::float2 const& parentSize,
        winrt::Windows::Graphics::SizeInt32 const& gridSizeInTiles);
    ~CompUI() {}

    void Resize(winrt::Windows::Foundation::Numerics::float2 const& newSize);
    std::optional<TileCoordinate> HitTest(winrt::Windows::Foundation::Numerics::float2 const& point);
    void SelectTile(std::optional<TileCoordinate> tileCoordinate);
    std::optional<TileCoordinate> CurrentSelectedTile();
    void UpdateTileWithState(TileCoordinate const& tileCoordinate, MineState mineState);
    void Reset(winrt::Windows::Graphics::SizeInt32 const& gridSizeInTiles);
    void UpdateTileAsMine(TileCoordinate const& tileCoordinate);
    void UpdateTileWithMineCount(TileCoordinate const& tileCoordinate, int numMines);
    void PlayMineAnimations(std::queue<int> mineIndices, std::queue<int> minesPerRing);
    bool IsAnimationPlaying() { return m_mineAnimationPlaying; }

private:
    float ComputeScaleFactor(winrt::Windows::Foundation::Numerics::float2 windowSize);
    float ComputeScaleFactor();
    void UpdateBoardScale(winrt::Windows::Foundation::Numerics::float2 windowSize);
    void PlayMineAnimation(int index, winrt::Windows::Foundation::TimeSpan const& delay);

private:
    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_root{ nullptr };

    winrt::Windows::Foundation::Numerics::float2 m_parentSize;
    winrt::Windows::Foundation::Numerics::float2 m_gameBoardMargin;
    std::unique_ptr<IndexHelper> m_indexHelper;

    std::unique_ptr<VisualGrid> m_gameBoard;
    std::unique_ptr<CompAssets> m_assets;

    bool m_mineAnimationPlaying = false;
};