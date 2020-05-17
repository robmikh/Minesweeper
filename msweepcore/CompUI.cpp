#include "pch.h"
#include "VisualGrid.h"
#include "CompAssets.h"
#include "CompUI.h"

using namespace winrt;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Graphics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

CompUI::CompUI(
    ContainerVisual const& parentVisual,
    float2 const& parentSize,
    SizeInt32 const& gridSizeInTiles)
{
    m_compositor = parentVisual.Compositor();
    m_root = m_compositor.CreateSpriteVisual();
    m_parentSize = parentSize;

    m_root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    m_root.Brush(m_compositor.CreateColorBrush(Colors::White()));
    m_root.BorderMode(CompositionBorderMode::Hard);
    parentVisual.Children().InsertAtTop(m_root);

    auto tileSize = float2{ 25, 25 };
    m_gameBoard = std::make_unique<VisualGrid>(
        m_compositor,
        gridSizeInTiles,
        tileSize,
        float2{ 2.5f, 2.5f });
    m_gameBoardMargin = { 100.0f, 100.0f };

    auto gameBoardVisual = m_gameBoard->Root();
    gameBoardVisual.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0 });
    gameBoardVisual.AnchorPoint({ 0.5f, 0.5f });
    m_root.Children().InsertAtTop(gameBoardVisual);

    auto selectionVisual = m_gameBoard->SelectionVisual();
    m_root.Children().InsertAtTop(selectionVisual);

    m_assets = std::make_unique<CompAssets>(m_compositor, tileSize);
}

void CompUI::SelectTile(std::optional<TileCoordinate> tileCoordinate) { m_gameBoard->SelectTile(tileCoordinate); }
std::optional<TileCoordinate> CompUI::CurrentSelectedTile() { return m_gameBoard->CurrentSelectedTile(); }

void CompUI::Resize(float2 const& newSize)
{
    m_parentSize = newSize;
    UpdateBoardScale(newSize);
}

std::optional<TileCoordinate> CompUI::HitTest(float2 const& point)
{
    auto windowSize = m_parentSize;
    auto scale = ComputeScaleFactor();
    auto realBoardSize = m_gameBoard->Size() * scale;
    auto realOffset = (windowSize - realBoardSize) / 2.0f;

    return m_gameBoard->HitTest((point - realOffset) / scale);
}

void CompUI::UpdateBoardScale(float2 windowSize)
{
    float scaleFactor = ComputeScaleFactor(windowSize);
    m_gameBoard->Root().Scale({ scaleFactor, scaleFactor, 1.0f });
}

float CompUI::ComputeScaleFactor()
{
    return ComputeScaleFactor(m_parentSize);
}

float CompUI::ComputeScaleFactor(float2 windowSize)
{
    auto boardSize = m_gameBoard->Size() + m_gameBoardMargin;

    auto windowRatio = windowSize.x / windowSize.y;
    auto boardRatio = boardSize.x / boardSize.y;

    auto scaleFactor = windowSize.x / boardSize.x;
    if (windowRatio > boardRatio)
    {
        scaleFactor = windowSize.y / boardSize.y;
    }
    return scaleFactor;
}

void CompUI::UpdateTileWithState(TileCoordinate const& tileCoordinate, MineState mineState)
{
    auto visual = m_gameBoard->GetTile(tileCoordinate.x, tileCoordinate.y);
    visual.Brush(m_assets->GetColorBrushFromMineState(mineState));
}

void CompUI::Reset(SizeInt32 const& gridSizeInTiles)
{
    m_gameBoard->Reset(gridSizeInTiles);
    m_indexHelper = std::make_unique<IndexHelper>(gridSizeInTiles.Width, gridSizeInTiles.Height);

    for (auto& visual : m_gameBoard->Tiles())
    {
        visual.Brush(m_assets->GetColorBrushFromMineState(MineState::Empty));
    }

    UpdateBoardScale(m_parentSize);
    m_mineAnimationPlaying = false;
}

void CompUI::UpdateTileAsMine(TileCoordinate const& tileCoordinate)
{
    auto visual = m_gameBoard->GetTile(tileCoordinate.x, tileCoordinate.y);
    visual.Brush(m_assets->GetMineBrush());
}

void CompUI::UpdateTileWithMineCount(TileCoordinate const& tileCoordinate, int numMines)
{
    auto visual = m_gameBoard->GetTile(tileCoordinate.x, tileCoordinate.y);
    visual.Brush(m_assets->GetColorBrushFromMineCount(numMines));

    if (numMines > 0)
    {
        auto shape = m_assets->GetShapeFromMineCount(numMines);
        auto shapeVisual = m_compositor.CreateShapeVisual();
        shapeVisual.RelativeSizeAdjustment({ 1, 1 });
        shapeVisual.Shapes().Append(shape);
        shapeVisual.BorderMode(CompositionBorderMode::Soft);
        visual.Children().InsertAtTop(shapeVisual);
    }
}

void CompUI::PlayMineAnimations(std::queue<int> mineIndices, std::queue<int> minesPerRing)
{
    // Create an animation batch so that we can know when the animations complete.
    auto batch = m_compositor.CreateScopedBatch(CompositionBatchTypes::Animation);

    // Iterate and animate each mine
    auto animationDelayStep = std::chrono::milliseconds(100);
    auto currentDelay = std::chrono::milliseconds(0);
    auto currentMinesCount = 0;
    while (!mineIndices.empty())
    {
        auto mineIndex = mineIndices.front();
        PlayMineAnimation(mineIndex, currentDelay);
        currentMinesCount++;

        auto minesOnCurrentLevel = minesPerRing.front();
        if (currentMinesCount == minesOnCurrentLevel)
        {
            currentMinesCount = 0;
            minesPerRing.pop();
            currentDelay += animationDelayStep;
        }
        mineIndices.pop();
    }

    // Subscribe to the completion event and complete the batch
    batch.Completed([=](auto&, auto&)
    {
        m_mineAnimationPlaying = false;
    });
    batch.End();

    m_mineAnimationPlaying = true;
}

void CompUI::PlayMineAnimation(int index, TimeSpan const& delay)
{
    auto visual = m_gameBoard->GetTile(m_indexHelper->ComputeXFromIndex(index), m_indexHelper->ComputeYFromIndex(index));
    // First, we need to promote the visual to the top.
    auto parentChildren = visual.Parent().Children();
    parentChildren.Remove(visual);
    parentChildren.InsertAtTop(visual);
    // Make sure the visual has the mine brush
    visual.Brush(m_assets->GetMineBrush());
    // Play the animation
    auto animation = m_compositor.CreateVector3KeyFrameAnimation();
    animation.InsertKeyFrame(0.0f, { 1.0f, 1.0f, 1.0f });
    animation.InsertKeyFrame(0.7f, { 2.0f, 2.0f, 1.0f });
    animation.InsertKeyFrame(1.0f, { 1.0f, 1.0f, 1.0f });
    animation.Duration(std::chrono::milliseconds(600));
    animation.DelayTime(delay);
    animation.IterationBehavior(AnimationIterationBehavior::Count);
    animation.IterationCount(1);
    visual.StartAnimation(L"Scale", animation);
}