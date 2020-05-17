#include "pch.h"
#include "VisualGrid.h"

using namespace winrt;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Graphics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

VisualGrid::VisualGrid(
    Compositor const& compositor,
    SizeInt32 const& gridSizeInTiles,
    float2 const& tileSize,
    float2 const& margin)
{
    m_compositor = compositor;
    m_root = m_compositor.CreateSpriteVisual();

    m_tileSize = tileSize;
    m_margin = margin;

    m_selectionVisual = m_compositor.CreateSpriteVisual();
    auto colorBrush = m_compositor.CreateColorBrush(Colors::Red());
    auto nineGridBrush = m_compositor.CreateNineGridBrush();
    nineGridBrush.SetInsets(m_margin.x, m_margin.y, m_margin.x, m_margin.y);
    nineGridBrush.IsCenterHollow(true);
    nineGridBrush.Source(colorBrush);
    m_selectionVisual.Brush(nineGridBrush);
    m_selectionVisual.Offset(float3(m_margin * -1.0f, 0));
    m_selectionVisual.IsVisible(false);
    m_selectionVisual.Size(m_tileSize + m_margin * 2.0f);

    Reset(gridSizeInTiles);
}

void VisualGrid::Reset(
    SizeInt32 const& gridSizeInTiles)
{
    m_gridWidthInTiles = gridSizeInTiles.Width;
    m_gridHeightInTiles = gridSizeInTiles.Height;

    m_root.Children().RemoveAll();
    m_tiles.clear();

    m_root.Size((m_tileSize + m_margin) * float2(m_gridWidthInTiles, m_gridHeightInTiles));
    m_indexHelper = std::make_unique<IndexHelper>(m_gridWidthInTiles, m_gridHeightInTiles);

    for (int x = 0; x < m_gridWidthInTiles; x++)
    {
        for (int y = 0; y < m_gridHeightInTiles; y++)
        {
            auto visual = m_compositor.CreateSpriteVisual();
            visual.Size(m_tileSize);
            visual.CenterPoint({ m_tileSize / 2.0f, 0.0f });
            visual.Offset(float3((m_margin / 2.0f) + (float2(m_tileSize + m_margin) * float2(x, y)), 0.0f));

            m_root.Children().InsertAtTop(visual);
            m_tiles.push_back(visual);
        }
    }
}

std::optional<TileCoordinate> VisualGrid::HitTest(float2 const& point)
{
    int x = point.x / (m_tileSize.x + m_margin.x);
    int y = point.y / (m_tileSize.y + m_margin.y);
    
    if (m_indexHelper->IsInBounds(x, y))
    {
        return std::optional<TileCoordinate>{ { x, y } };
    }
    else
    {
        return std::nullopt;
    }
}

SpriteVisual VisualGrid::GetTile(int x, int y)
{
    return m_tiles[m_indexHelper->ComputeIndex(x, y)];
}

void VisualGrid::SelectTile(std::optional<TileCoordinate> tileCoordinate)
{
    m_currentSelection = tileCoordinate;
    if (auto selectedTileCoordinate = tileCoordinate)
    {
        auto visual = m_tiles[m_indexHelper->ComputeIndex(selectedTileCoordinate->x, selectedTileCoordinate->y)];
        m_selectionVisual.ParentForTransform(visual);
        m_selectionVisual.IsVisible(true);
    }
    else 
    {
        m_selectionVisual.IsVisible(false);
    }
}
