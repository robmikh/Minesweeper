#include "pch.h"
#include "CompAssets.h"

using namespace winrt;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Graphics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

CompositionSpriteShape GetDotShape(
    Compositor const& compositor,
    CompositionGeometry const& geometry,
    CompositionColorBrush const& brush,
    float2 offset)
{
    auto shape = compositor.CreateSpriteShape(geometry);
    shape.FillBrush(brush);
    shape.Offset(offset);
    return shape;
}

CompAssets::CompAssets(
    Compositor const& compositor,
    float2 const& tileSize)
{
    GenerateAssets(compositor, tileSize);
}

CompositionColorBrush CompAssets::GetColorBrushFromMineState(MineState state)
{
    return m_mineStateBrushes.at(state);
}

CompositionColorBrush CompAssets::GetColorBrushFromMineCount(int count)
{
    return m_mineCountBackgroundBrushes.at(count);
}

CompositionShape CompAssets::GetShapeFromMineCount(int count)
{
    return m_mineCountShapes.at(count);
}

void CompAssets::GenerateAssets(
    Compositor const& compositor,
    float2 const& tileSize)
{
    m_mineBrush = compositor.CreateColorBrush(Colors::Red());

    m_mineStateBrushes.clear();
    m_mineStateBrushes.insert({ MineState::Empty, compositor.CreateColorBrush(Colors::Blue()) });
    m_mineStateBrushes.insert({ MineState::Flag, compositor.CreateColorBrush(Colors::Orange()) });
    m_mineStateBrushes.insert({ MineState::Question, compositor.CreateColorBrush(Colors::LimeGreen()) });

    m_mineCountBackgroundBrushes.clear();
    m_mineCountBackgroundBrushes.insert({ 1, compositor.CreateColorBrush(Colors::LightBlue()) });
    m_mineCountBackgroundBrushes.insert({ 2, compositor.CreateColorBrush(Colors::LightGreen()) });
    m_mineCountBackgroundBrushes.insert({ 3, compositor.CreateColorBrush(Colors::LightSalmon()) });
    m_mineCountBackgroundBrushes.insert({ 4, compositor.CreateColorBrush(Colors::LightSteelBlue()) });
    m_mineCountBackgroundBrushes.insert({ 5, compositor.CreateColorBrush(Colors::MediumPurple()) });
    m_mineCountBackgroundBrushes.insert({ 6, compositor.CreateColorBrush(Colors::LightCyan()) });
    m_mineCountBackgroundBrushes.insert({ 7, compositor.CreateColorBrush(Colors::Maroon()) });
    m_mineCountBackgroundBrushes.insert({ 8, compositor.CreateColorBrush(Colors::DarkSeaGreen()) });
    m_mineCountBackgroundBrushes.insert({ 0, compositor.CreateColorBrush(Colors::WhiteSmoke()) });

    m_mineCountShapes.clear();
    {
        auto circleGeometry = compositor.CreateEllipseGeometry();
        circleGeometry.Radius(tileSize / 12.0f);
        auto dotBrush = compositor.CreateColorBrush(Colors::Black());

        // 1
        {
            auto containerShape = compositor.CreateContainerShape();
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, tileSize / 2.0f));
            m_mineCountShapes.insert({ 1, containerShape });
        }

        // 2
        {
            auto containerShape = compositor.CreateContainerShape();
            auto thirdX = tileSize.x / 3.0f;
            auto halfY = tileSize.y / 2.0f;
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { thirdX , halfY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { thirdX * 2.0f , halfY }));
            m_mineCountShapes.insert({ 2, containerShape });
        }

        // 3
        {
            auto containerShape = compositor.CreateContainerShape();
            auto fourthX = tileSize.x / 4.0f;
            auto fourthY = tileSize.y / 4.0f;
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, tileSize / 2.0f));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            m_mineCountShapes.insert({ 3, containerShape });
        }


        // 4
        {
            auto containerShape = compositor.CreateContainerShape();
            auto thirdX = tileSize.x / 3.0f;
            auto thirdY = tileSize.y / 3.0f;
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { thirdX , thirdY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { thirdX * 2.0f , thirdY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { thirdX , thirdY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { thirdX * 2.0f , thirdY * 2.0f }));
            m_mineCountShapes.insert({ 4, containerShape });
        }

        // 5
        {
            auto containerShape = compositor.CreateContainerShape();
            auto fourthX = tileSize.x / 4.0f;
            auto fourthY = tileSize.y / 4.0f;
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, tileSize / 2.0f));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 3.0f }));
            m_mineCountShapes.insert({ 5, containerShape });
        }

        // 6
        {
            auto containerShape = compositor.CreateContainerShape();
            auto fourthX = tileSize.x / 4.0f;
            auto fourthY = tileSize.y / 4.0f;
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 2.0f }));
            m_mineCountShapes.insert({ 6, containerShape });
        }

        // 7
        {
            auto containerShape = compositor.CreateContainerShape();
            auto fourthX = tileSize.x / 4.0f;
            auto fourthY = tileSize.y / 4.0f;
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, tileSize / 2.0f));
            m_mineCountShapes.insert({ 7, containerShape });
        }

        // 8
        {
            auto containerShape = compositor.CreateContainerShape();
            auto fourthX = tileSize.x / 4.0f;
            auto fourthY = tileSize.y / 4.0f;
            auto halfX = tileSize.y / 2.0f;
            auto thirdY = tileSize.y / 3.0f;
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX, fourthY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { halfX, thirdY }));
            containerShape.Shapes().Append(GetDotShape(compositor, circleGeometry, dotBrush, { halfX, thirdY * 2.0f }));
            m_mineCountShapes.insert({ 8, containerShape });
        }
    }
}