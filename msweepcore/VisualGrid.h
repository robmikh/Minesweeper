#pragma once
struct TileCoordinate
{
    int x;
    int y;
};

class VisualGrid
{
public:
    VisualGrid(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::Windows::Graphics::SizeInt32 const& gridSizeInTiles,
        winrt::Windows::Foundation::Numerics::float2 const& tileSize,
        winrt::Windows::Foundation::Numerics::float2 const& margin);
    ~VisualGrid() {}

    void Reset(winrt::Windows::Graphics::SizeInt32 const& gridSizeInTiles);

    winrt::Windows::UI::Composition::ContainerVisual Root() { return m_root; }
    winrt::Windows::UI::Composition::SpriteVisual SelectionVisual() { return m_selectionVisual; }
    winrt::Windows::Foundation::Numerics::float2 Size() { return m_root.Size(); }
    const std::vector<winrt::Windows::UI::Composition::SpriteVisual>& Tiles() const { return m_tiles; }
    std::optional<TileCoordinate> HitTest(winrt::Windows::Foundation::Numerics::float2 const& point);
    winrt::Windows::UI::Composition::SpriteVisual GetTile(int x, int y);
    void SelectTile(std::optional<TileCoordinate> tileCoordinate);
    std::optional<TileCoordinate> CurrentSelectedTile() { return m_currentSelection; }

private:
    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_root{ nullptr };

    std::vector<winrt::Windows::UI::Composition::SpriteVisual> m_tiles;
    winrt::Windows::UI::Composition::SpriteVisual m_selectionVisual{ nullptr };
    std::unique_ptr<IndexHelper> m_indexHelper;

    int m_gridWidthInTiles;
    int m_gridHeightInTiles;
    winrt::Windows::Foundation::Numerics::float2 m_tileSize;
    winrt::Windows::Foundation::Numerics::float2 m_margin;

    std::optional<TileCoordinate> m_currentSelection = std::nullopt;
};