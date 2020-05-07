#include "pch.h"
#include "Minesweeper.h"

using namespace winrt;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

Minesweeper::Minesweeper(
    ContainerVisual const& parentVisual,
    float2 parentSize)
   : m_board(16, 16, 40)
{
    m_compositor = parentVisual.Compositor();
    m_root = m_compositor.CreateSpriteVisual();

    m_root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    m_root.Brush(m_compositor.CreateColorBrush(Colors::White()));
    m_root.BorderMode(CompositionBorderMode::Hard);
    parentVisual.Children().InsertAtTop(m_root);

    m_tileSize = { 25, 25 };
    m_margin = { 2.5f, 2.5f };
    m_gameBoardMargin = { 100.0f, 100.0f };

    m_gameBoard = m_compositor.CreateContainerVisual();
    m_gameBoard.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0.0f });
    m_gameBoard.AnchorPoint({ 0.5f, 0.5f });
    m_root.Children().InsertAtTop(m_gameBoard);

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
    m_root.Children().InsertAtTop(m_selectionVisual);
    m_currentSelectionX = -1;
    m_currentSelectionY = -1;

    GenerateAssets();
    
    NewGame();
    OnParentSizeChanged(parentSize);
}

void Minesweeper::OnPointerMoved(float2 point)
{
    if (m_board.IsGameOver() || m_mineAnimationPlaying)
    {
        return;
    }

    auto windowSize = m_parentSize;
    auto scale = ComputeScaleFactor();
    auto realBoardSize = m_gameBoard.Size() * scale;
    auto realOffset = (windowSize - realBoardSize) / 2.0f;

    point -= realOffset;
    point /= scale;

    int x = point.x / (m_tileSize.x + m_margin.x);
    int y = point.y / (m_tileSize.y + m_margin.y);
    int index = x * m_board.GetBoardHeight() + y;

    if (m_board.IsInBounds(x, y) &&
        m_board.GetMineState(x, y) != MineState::Revealed)
    {
        auto visual = m_tiles[index];

        m_selectionVisual.ParentForTransform(visual);
        m_currentSelectionX = x;
        m_currentSelectionY = y;
        m_selectionVisual.IsVisible(true);
    }
    else
    {
        m_currentSelectionX = -1;
        m_currentSelectionY = -1;
        m_selectionVisual.IsVisible(false);
    }
}

void Minesweeper::OnParentSizeChanged(float2 newSize)
{
    m_parentSize = newSize;
    UpdateBoardScale(newSize);
}

void Minesweeper::OnPointerPressed(
    bool isRightButton,
    bool isEraser)
{
    if (m_board.IsGameOver() && !m_mineAnimationPlaying)
    {
       NewGame();
    }

    if (m_currentSelectionX >= 0.0f ||
        m_currentSelectionY >= 0.0f)
    {
        int index = m_board.ComputeIndex(m_currentSelectionX, m_currentSelectionY);
        auto visual = m_tiles[index];

        if (m_board.GetMineState(m_currentSelectionX, m_currentSelectionY) != MineState::Revealed)
        {
            if (isRightButton || isEraser)
            {
                m_board.CycleMineState(m_currentSelectionX, m_currentSelectionY);
                visual.Brush(GetColorBrushFromMineState(m_board.GetMineState(m_currentSelectionX, m_currentSelectionY)));
            }
            else if (m_board.GetMineState(m_currentSelectionX, m_currentSelectionY) == MineState::Empty)
            {
                if (m_board.Sweep(m_currentSelectionX, m_currentSelectionY))
                {
                    // We hit a mine! Setup and play an animation while locking any input.
                    auto hitX = m_currentSelectionX;
                    auto hitY = m_currentSelectionY;

                    // First, hide the selection visual and reset the selection
                    m_selectionVisual.IsVisible(false);
                    m_currentSelectionX = -1;
                    m_currentSelectionY = -1;

                    // Create an animation batch so that we can know when the animations complete.
                    auto batch = m_compositor.CreateScopedBatch(CompositionBatchTypes::Animation);

                    PlayAnimationOnAllMines(hitX, hitY);

                    // Subscribe to the completion event and complete the batch
                    batch.Completed([=](auto&, auto&)
                    {
                        m_mineAnimationPlaying = false;
                    });
                    batch.End();

                    m_mineAnimationPlaying = true;
                }
                // TODO: Detect that the player has won
            }
        }
    }
}

void Minesweeper::NewGame()
{
   m_board.NewGame([this](int x, int y)
   {
      int indexRevealed = m_board.ComputeIndex(x, y);
      auto visual = m_tiles[indexRevealed];

      if (m_board.IsMine(x, y))
      {
         visual.Brush(m_mineBrush);
      }
      else
      {
         int count = m_board.GetSurroundingMineCount(x, y);
         visual.Brush(GetColorBrushFromMineCount(count));
      
         if (count > 0)
         {
            auto shape = GetShapeFromMineCount(count);
            auto shapeVisual = m_compositor.CreateShapeVisual();
            shapeVisual.RelativeSizeAdjustment({ 1, 1 });
            shapeVisual.Shapes().Append(shape);
            shapeVisual.BorderMode(CompositionBorderMode::Soft);
            visual.Children().InsertAtTop(shapeVisual);
         }
      }
   });

    m_gameBoard.Children().RemoveAll();
    m_tiles.clear();

    m_gameBoard.Size((m_tileSize + m_margin) * float2(m_board.GetBoardWidth(), m_board.GetBoardHeight()));

    for (int x = 0; x < m_board.GetBoardWidth(); x++)
    {
        for (int y = 0; y < m_board.GetBoardHeight(); y++)
        {
            SpriteVisual visual = m_compositor.CreateSpriteVisual();
            visual.Size(m_tileSize);
            visual.CenterPoint({ m_tileSize / 2.0f, 0.0f });
            visual.Offset(float3((m_margin / 2.0f) + (float2(m_tileSize + m_margin) * float2(x, y)), 0.0f));
            visual.Brush(GetColorBrushFromMineState(MineState::Empty));

            m_gameBoard.Children().InsertAtTop(visual);
            m_tiles.push_back(visual);
        }
    }

    m_mineAnimationPlaying = false;

    m_selectionVisual.IsVisible(false);
    m_currentSelectionX = -1;
    m_currentSelectionY = -1;

    UpdateBoardScale(m_parentSize);
}

void Minesweeper::UpdateBoardScale(float2 windowSize)
{
    float scaleFactor = ComputeScaleFactor(windowSize);

    m_gameBoard.Scale({ scaleFactor, scaleFactor, 1.0f });
}

float Minesweeper::ComputeScaleFactor()
{
    return ComputeScaleFactor(m_parentSize);
}

float Minesweeper::ComputeScaleFactor(float2 windowSize)
{
    auto boardSize = m_gameBoard.Size() + m_gameBoardMargin;

    auto windowRatio = windowSize.x / windowSize.y;
    auto boardRatio = boardSize.x / boardSize.y;

    auto scaleFactor = windowSize.x / boardSize.x;
    if (windowRatio > boardRatio)
    {
        scaleFactor = windowSize.y / boardSize.y;
    }
    return scaleFactor;
}

CompositionColorBrush Minesweeper::GetColorBrushFromMineState(MineState state)
{
    return m_mineStateBrushes.at(state);
}

CompositionColorBrush Minesweeper::GetColorBrushFromMineCount(int count)
{
    return m_mineCountBackgroundBrushes.at(count);
}

void Minesweeper::PlayMineAnimation(int index, TimeSpan const& delay)
{
    auto visual = m_tiles[index];
    // First, we need to promote the visual to the top.
    auto parentChildren = visual.Parent().Children();
    parentChildren.Remove(visual);
    parentChildren.InsertAtTop(visual);
    // Make sure the visual has the mine brush
    visual.Brush(m_mineBrush);
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

void Minesweeper::CheckTileForMineForAnimation(int x, int y, std::queue<int>& mineIndices, int& visitedTiles, int& minesInRing)
{
    if (m_board.IsInBounds(x, y))
    {
        auto tileIndex = m_board.ComputeIndex(x, y);
        if (m_board.IsMine(x, y))
        {
            mineIndices.push(tileIndex);
            minesInRing++;
        }
        visitedTiles++;
    }
}

void Minesweeper::PlayAnimationOnAllMines(int centerX, int centerY)
{
    // Build a queue that contains the indices of the mines in a spiral starting from the clicked mine.
    std::queue<int> mineIndices;
    std::queue<int> minesPerRing;
    int visitedTiles = 0;
    int ringLevel = 0;
    while (visitedTiles < m_tiles.size())
    {
        if (ringLevel == 0)
        {
            auto hitMineIndex = m_board.ComputeIndex(centerX, centerY);
            mineIndices.push(hitMineIndex);
            minesPerRing.push(1);
            visitedTiles++;
        }
        else
        {
            auto currentMinesInRing = 0;

            // Check the top side
            for (auto x = centerX - ringLevel; x <= (centerX + ringLevel); x++)
            {
                auto y = centerY - ringLevel;
                CheckTileForMineForAnimation(x, y, mineIndices, visitedTiles, currentMinesInRing);
            }

            // Check the right side
            for (auto y = centerY - ringLevel + 1; y <= (centerY + ringLevel); y++)
            {
                auto x = centerX + ringLevel;
                CheckTileForMineForAnimation(x, y, mineIndices, visitedTiles, currentMinesInRing);
            }

            // Check the bottom side
            for (auto x = centerX - ringLevel; x < (centerX + ringLevel); x++)
            {
                auto y = centerY + ringLevel;
                CheckTileForMineForAnimation(x, y, mineIndices, visitedTiles, currentMinesInRing);
            }

            // Check the left side
            for (auto y = centerY - ringLevel + 1; y < (centerY + ringLevel); y++)
            {
                auto x = centerX - ringLevel;
                CheckTileForMineForAnimation(x, y, mineIndices, visitedTiles, currentMinesInRing);
            }

            if (currentMinesInRing > 0)
            {
                minesPerRing.push(currentMinesInRing);
            }
        }
        ringLevel++;
    }

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
}

CompositionShape Minesweeper::GetShapeFromMineCount(int count)
{
    return m_mineCountShapes.at(count);
}

CompositionSpriteShape Minesweeper::GetDotShape(
    CompositionGeometry const& geometry,
    CompositionColorBrush const& brush,
    float2 offset)
{
    auto shape = m_compositor.CreateSpriteShape(geometry);
    shape.FillBrush(brush);
    shape.Offset(offset);
    return shape;
}

void Minesweeper::GenerateAssets()
{
    m_mineBrush = m_compositor.CreateColorBrush(Colors::Red());

    m_mineStateBrushes.clear();
    m_mineStateBrushes.insert({ MineState::Empty, m_compositor.CreateColorBrush(Colors::Blue()) });
    m_mineStateBrushes.insert({ MineState::Flag, m_compositor.CreateColorBrush(Colors::Orange()) });
    m_mineStateBrushes.insert({ MineState::Question, m_compositor.CreateColorBrush(Colors::LimeGreen()) });

    m_mineCountBackgroundBrushes.clear();
    m_mineCountBackgroundBrushes.insert({ 1, m_compositor.CreateColorBrush(Colors::LightBlue()) });
    m_mineCountBackgroundBrushes.insert({ 2, m_compositor.CreateColorBrush(Colors::LightGreen()) });
    m_mineCountBackgroundBrushes.insert({ 3, m_compositor.CreateColorBrush(Colors::LightSalmon()) });
    m_mineCountBackgroundBrushes.insert({ 4, m_compositor.CreateColorBrush(Colors::LightSteelBlue()) });
    m_mineCountBackgroundBrushes.insert({ 5, m_compositor.CreateColorBrush(Colors::MediumPurple()) });
    m_mineCountBackgroundBrushes.insert({ 6, m_compositor.CreateColorBrush(Colors::LightCyan()) });
    m_mineCountBackgroundBrushes.insert({ 7, m_compositor.CreateColorBrush(Colors::Maroon()) });
    m_mineCountBackgroundBrushes.insert({ 8, m_compositor.CreateColorBrush(Colors::DarkSeaGreen()) });
    m_mineCountBackgroundBrushes.insert({ 0, m_compositor.CreateColorBrush(Colors::WhiteSmoke()) });

    m_mineCountShapes.clear();
    {
        auto circleGeometry = m_compositor.CreateEllipseGeometry();
        circleGeometry.Radius(m_tileSize / 12.0f);
        auto dotBrush = m_compositor.CreateColorBrush(Colors::Black());

        // 1
        {
            auto containerShape = m_compositor.CreateContainerShape();
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, m_tileSize / 2.0f));
            m_mineCountShapes.insert({ 1, containerShape });
        }

        // 2
        {
            auto containerShape = m_compositor.CreateContainerShape();
            auto thirdX = m_tileSize.x / 3.0f;
            auto halfY = m_tileSize.y / 2.0f;
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { thirdX , halfY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { thirdX * 2.0f , halfY }));
            m_mineCountShapes.insert({ 2, containerShape });
        }
        
        // 3
        {
            auto containerShape = m_compositor.CreateContainerShape();
            auto fourthX = m_tileSize.x / 4.0f;
            auto fourthY = m_tileSize.y / 4.0f;
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, m_tileSize / 2.0f));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            m_mineCountShapes.insert({ 3, containerShape });
        }
        
        
        // 4
        {
            auto containerShape = m_compositor.CreateContainerShape();
            auto thirdX = m_tileSize.x / 3.0f;
            auto thirdY = m_tileSize.y / 3.0f;
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { thirdX , thirdY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { thirdX * 2.0f , thirdY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { thirdX , thirdY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { thirdX * 2.0f , thirdY * 2.0f }));
            m_mineCountShapes.insert({ 4, containerShape });
        }
        
        // 5
        {
            auto containerShape = m_compositor.CreateContainerShape();
            auto fourthX = m_tileSize.x / 4.0f;
            auto fourthY = m_tileSize.y / 4.0f;
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, m_tileSize / 2.0f));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 3.0f }));
            m_mineCountShapes.insert({ 5, containerShape });
        }
        
        // 6
        {
            auto containerShape = m_compositor.CreateContainerShape();
            auto fourthX = m_tileSize.x / 4.0f;
            auto fourthY = m_tileSize.y / 4.0f;
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 2.0f }));
            m_mineCountShapes.insert({ 6, containerShape });
        }
        
        // 7
        {
            auto containerShape = m_compositor.CreateContainerShape();
            auto fourthX = m_tileSize.x / 4.0f;
            auto fourthY = m_tileSize.y / 4.0f;
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, m_tileSize / 2.0f));
            m_mineCountShapes.insert({ 7, containerShape });
        }
        
        // 8
        {
            auto containerShape = m_compositor.CreateContainerShape();
            auto fourthX = m_tileSize.x / 4.0f;
            auto fourthY = m_tileSize.y / 4.0f;
            auto halfX = m_tileSize.y / 2.0f;
            auto thirdY = m_tileSize.y / 3.0f;
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX, fourthY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 3.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { fourthX * 3.0f, fourthY * 2.0f }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { halfX, thirdY }));
            containerShape.Shapes().Append(GetDotShape(circleGeometry, dotBrush, { halfX, thirdY * 2.0f }));
            m_mineCountShapes.insert({ 8, containerShape });
        }
    }
}