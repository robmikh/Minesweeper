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
{
    m_compositor = parentVisual.Compositor();
    m_root = m_compositor.CreateSpriteVisual();

    m_root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    m_root.Brush(m_compositor.CreateColorBrush(Colors::White()));
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

    NewGame(16, 16, 40);
    OnParentSizeChanged(parentSize);
}

void Minesweeper::OnPointerMoved(float2 point)
{
    if (m_gameOver || m_mineAnimationPlaying)
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
    int index = x * m_gameBoardHeight + y;

    if (IsInBounds(x, y) &&
        m_mineStates[index] != MineState::Revealed)
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
    if (m_gameOver && !m_mineAnimationPlaying)
    {
        NewGame(16, 16, 40);
    }

    if (m_currentSelectionX >= 0.0f ||
        m_currentSelectionY >= 0.0f)
    {
        int index = ComputeIndex(m_currentSelectionX, m_currentSelectionY);
        auto visual = m_tiles[index];

        if (m_mineStates[index] != MineState::Revealed)
        {
            if (isRightButton || isEraser)
            {
                auto state = m_mineStates[index];
                state = (MineState)((state + 1) % MineState::Revealed);
                m_mineStates[index] = state;
                visual.Brush(GetColorBrushFromMineState(state));
            }
            else if (m_mineStates[index] == MineState::Empty)
            {
                if (Sweep(m_currentSelectionX, m_currentSelectionY))
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
                    m_gameOver = true;
                }
                // TODO: Detect that the player has won
            }
        }
    }
}

void Minesweeper::NewGame(int boardWidth, int boardHeight, int mines)
{
    m_gameBoardWidth = boardWidth;
    m_gameBoardHeight = boardHeight;

    m_gameBoard.Children().RemoveAll();
    m_tiles.clear();
    m_mineStates.clear();

    m_gameBoard.Size((m_tileSize + m_margin) * float2(m_gameBoardWidth, m_gameBoardHeight));

    for (int x = 0; x < m_gameBoardWidth; x++)
    {
        for (int y = 0; y < m_gameBoardHeight; y++)
        {
            SpriteVisual visual = m_compositor.CreateSpriteVisual();
            visual.Size(m_tileSize);
            visual.CenterPoint({ m_tileSize / 2.0f, 0.0f });
            visual.Offset(float3((m_margin / 2.0f) + (float2(m_tileSize + m_margin) * float2(x, y)), 0.0f));
            visual.Brush(m_compositor.CreateColorBrush(Colors::Blue()));

            m_gameBoard.Children().InsertAtTop(visual);
            m_tiles.push_back(visual);
            m_mineStates.push_back(MineState::Empty);
        }
    }

    m_mineAnimationPlaying = false;
    m_gameOver = false;
    m_mineGenerationState = MineGenerationState::Deferred;
    m_numMines = mines;

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

bool Minesweeper::Sweep(int x, int y)
{
    if (m_mineGenerationState == MineGenerationState::Deferred)
    {
        // We don't want the first thing that the user clicks to be a mine. 
        // Generate mines but avoid putting it where the user clicked.
        GenerateMines(m_numMines, x, y);
        m_mineGenerationState = MineGenerationState::Generated;
    }

    bool hitMine = false;
    std::queue<int> sweeps;
    sweeps.push(ComputeIndex(x, y));
    Reveal(sweeps.front());

    while (!sweeps.empty())
    {
        int index = sweeps.front();
        int currentx = ComputeXFromIndex(index);
        int currenty = ComputeYFromIndex(index);

        if (m_mines[index])
        {
            // We hit a mine, game over
            hitMine = true;
            break;
        }

        if (m_neighborCounts[index] == 0)
        {
            PushIfUnmarked(sweeps, currentx - 1, currenty - 1);
            PushIfUnmarked(sweeps, currentx, currenty - 1);
            PushIfUnmarked(sweeps, currentx + 1, currenty - 1);
            PushIfUnmarked(sweeps, currentx + 1, currenty);
            PushIfUnmarked(sweeps, currentx + 1, currenty + 1);
            PushIfUnmarked(sweeps, currentx, currenty + 1);
            PushIfUnmarked(sweeps, currentx - 1, currenty + 1);
            PushIfUnmarked(sweeps, currentx - 1, currenty);
        }

        sweeps.pop();
    }

    return hitMine;
}

void Minesweeper::Reveal(int index)
{
    auto visual = m_tiles[index];

    if (m_mines[index])
    {
        visual.Brush(m_compositor.CreateColorBrush(Colors::Red()));
    }
    else
    {
        int count = m_neighborCounts[index];
        visual.Brush(GetColorBrushFromMineCount(count));
    }

    m_mineStates[index] = MineState::Revealed;
}

bool Minesweeper::IsInBoundsAndUnmarked(int x, int y)
{
    int index = ComputeIndex(x, y);
    return IsInBounds(x, y) && m_mineStates[index] == MineState::Empty;
}

void Minesweeper::PushIfUnmarked(std::queue<int> & sweeps, int x, int y)
{
    if (IsInBoundsAndUnmarked(x, y))
    {
        int index = ComputeIndex(x, y);
        Reveal(index);
        sweeps.push(index);
    }
}

CompositionColorBrush Minesweeper::GetColorBrushFromMineState(MineState state)
{
    switch (state)
    {
    case MineState::Empty:
        return m_compositor.CreateColorBrush(Colors::Blue());
    case MineState::Flag:
        return m_compositor.CreateColorBrush(Colors::Orange());
    case MineState::Question:
        return m_compositor.CreateColorBrush(Colors::LimeGreen());
    default:
        return m_compositor.CreateColorBrush(Colors::Black());
    }
}

CompositionColorBrush Minesweeper::GetColorBrushFromMineCount(int count)
{
    switch (count)
    {
    case 1:
        return m_compositor.CreateColorBrush(Colors::LightBlue());
    case 2:
        return m_compositor.CreateColorBrush(Colors::LightGreen());
    case 3:
        return m_compositor.CreateColorBrush(Colors::LightSalmon());
    case 4:
        return m_compositor.CreateColorBrush(Colors::LightSteelBlue());
    case 5:
        return m_compositor.CreateColorBrush(Colors::MediumPurple());
    case 6:
        return m_compositor.CreateColorBrush(Colors::LightCyan());
    case 7:
        return m_compositor.CreateColorBrush(Colors::Maroon());
    case 8:
        return m_compositor.CreateColorBrush(Colors::DarkSeaGreen());
    default:
        return m_compositor.CreateColorBrush(Colors::WhiteSmoke());
    }
}

void Minesweeper::GenerateMines(int numMines, int excludeX, int excludeY)
{
    m_mines.clear();
    for (int x = 0; x < m_gameBoardWidth; x++)
    {
        for (int y = 0; y < m_gameBoardHeight; y++)
        {
            m_mines.push_back(false);
        }
    }

    srand(time(0));
    for (int i = 0; i < numMines; i++)
    {
        int index = -1;
        auto excludeIndex = ComputeIndex(excludeX, excludeY);
        do
        {
            index = GenerateIndex(0, m_gameBoardWidth * m_gameBoardHeight - 1);
        } while (index == excludeIndex || m_mines[index]);

        m_mines[index] = true;
    }

    m_neighborCounts.clear();
    for (int i = 0; i < m_mines.size(); i++)
    {
        int x = ComputeXFromIndex(i);
        int y = ComputeYFromIndex(i);

        if (m_mines[i])
        {
            // -1 means a mine
            m_neighborCounts.push_back(-1);
        }
        else
        {
            int count = GetSurroundingMineCount(x, y);
            m_neighborCounts.push_back(count);
        }
    }
}

int Minesweeper::GenerateIndex(int min, int max)
{
    return min + (rand() % static_cast<int>(max - min + 1));
}

int Minesweeper::ComputeIndex(int x, int y)
{
    return x * m_gameBoardHeight + y;
}

int Minesweeper::ComputeXFromIndex(int index)
{
    return index / m_gameBoardHeight;
}

int Minesweeper::ComputeYFromIndex(int index)
{
    return index % m_gameBoardHeight;
}

bool Minesweeper::IsInBounds(int x, int y)
{
    return ((x >= 0 && x < m_gameBoardWidth) &&
        (y >= 0 && y < m_gameBoardHeight));
}

bool Minesweeper::TestSpot(int x, int y)
{
    return IsInBounds(x, y) && m_mines[ComputeIndex(x, y)];
}

int Minesweeper::GetSurroundingMineCount(int x, int y)
{
    int count = 0;

    if (TestSpot(x + 1, y))
    {
        count++;
    }

    if (TestSpot(x - 1, y))
    {
        count++;
    }

    if (TestSpot(x, y + 1))
    {
        count++;
    }

    if (TestSpot(x, y - 1))
    {
        count++;
    }

    if (TestSpot(x + 1, y + 1))
    {
        count++;
    }

    if (TestSpot(x - 1, y - 1))
    {
        count++;
    }

    if (TestSpot(x - 1, y + 1))
    {
        count++;
    }

    if (TestSpot(x + 1, y - 1))
    {
        count++;
    }

    return count;
}

void Minesweeper::PlayMineAnimation(int index, TimeSpan const& delay)
{
    auto visual = m_tiles[index];
    // First, we need to promote the visual to the top.
    auto parentChildren = visual.Parent().Children();
    parentChildren.Remove(visual);
    parentChildren.InsertAtTop(visual);
    // Make sure the visual has the mine brush
    visual.Brush(m_compositor.CreateColorBrush(Colors::Red()));
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
    if (IsInBounds(x, y))
    {
        auto tileIndex = ComputeIndex(x, y);
        if (m_mines[tileIndex])
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
            auto hitMineIndex = ComputeIndex(centerX, centerY);
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