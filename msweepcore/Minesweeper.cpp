#include "pch.h"
#include "CompUI.h"
#include "VisualGrid.h"
#include "CompAssets.h"
#include "Minesweeper.h"

using namespace winrt;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Graphics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

std::shared_ptr<IMinesweeper> CreateMinesweeper(
    ContainerVisual parentVisual,
    float2 parentSize)
{
    return std::make_shared<Minesweeper>(parentVisual, parentSize);
}

MineState CycleMineState(MineState const& mineState)
{
    switch (mineState)
    {
    case MineState::Empty:
        return MineState::Flag;
    case MineState::Flag:
        return MineState::Question;
    case MineState::Question:
        return MineState::Empty;
    case MineState::Revealed:
        throw std::runtime_error("We shouldn't be cycling a revealed tile!");
    }
}

Minesweeper::Minesweeper(
    ContainerVisual const& parentVisual,
    float2 parentSize)
{
    auto boardSizeInTiles = SizeInt32{ 16, 16 };
    m_ui = std::make_unique<CompUI>(parentVisual, parentSize, boardSizeInTiles);

    NewGame(boardSizeInTiles.Width, boardSizeInTiles.Height, 40);
    OnParentSizeChanged(parentSize);
}

void Minesweeper::OnPointerMoved(float2 point)
{
    if (m_gameOver || m_ui->IsAnimationPlaying())
    {
        return;
    }

    std::optional<TileCoordinate> selectedTile = std::nullopt;
    if (auto tile = m_ui->HitTest(point))
    {
        if (m_mineStates[m_indexHelper->ComputeIndex(tile->x, tile->y)] != MineState::Revealed)
        {
            TileCoordinate result = {};
            result.x = tile->x;
            result.y = tile->y;
            auto temp = std::optional<TileCoordinate>(result);
            selectedTile.swap(temp);
        }
    }
    m_ui->SelectTile(selectedTile);
}

void Minesweeper::OnParentSizeChanged(float2 newSize)
{
    m_ui->Resize(newSize);
}

void Minesweeper::OnPointerPressed(
    bool isRightButton,
    bool isEraser)
{
    if (m_gameOver && !m_ui->IsAnimationPlaying())
    {
        NewGame(m_gameBoardWidth, m_gameBoardWidth, m_numMines);
    }

    if (auto currentSelection = m_ui->CurrentSelectedTile())
    {
        int index = m_indexHelper->ComputeIndex(currentSelection->x, currentSelection->y);

        if (m_mineStates[index] != MineState::Revealed)
        {
            if (isRightButton || isEraser)
            {
                auto state = CycleMineState(m_mineStates[index]);
                m_mineStates[index] = state;
                m_ui->UpdateTileWithState(*currentSelection, state);
            }
            else if (m_mineStates[index] == MineState::Empty)
            {
                if (Sweep(currentSelection->x, currentSelection->y))
                {
                    // We hit a mine! Setup and play an animation while locking any input.
                    auto hitX = currentSelection->x;
                    auto hitY = currentSelection->y;

                    // First, hide the selection visual and reset the selection
                    m_ui->SelectTile(std::nullopt);

                    PlayAnimationOnAllMines(hitX, hitY);

                    m_gameOver = true;
                }
                else if (CheckIfWon())
                {
                    m_ui->SelectTile(std::nullopt);
                    // TODO: Play a win animation
                    m_gameOver = true;
                }
            }
        }
    }
}

void Minesweeper::NewGame(int boardWidth, int boardHeight, int mines)
{
    m_gameBoardWidth = boardWidth;
    m_gameBoardHeight = boardHeight;
    m_indexHelper = std::make_unique<IndexHelper>(boardWidth, boardHeight);

    m_ui->Reset({ boardWidth, boardHeight });
    m_mineStates.clear();

    for (auto i = 0; i < boardWidth * boardHeight; i++)
    {
        m_mineStates.push_back(MineState::Empty);
    }

    m_gameOver = false;
    m_mineGenerationState = MineGenerationState::Deferred;
    m_numMines = mines;
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
    sweeps.push(m_indexHelper->ComputeIndex(x, y));
    Reveal(sweeps.front());

    while (!sweeps.empty())
    {
        int index = sweeps.front();
        int currentx = m_indexHelper->ComputeXFromIndex(index);
        int currenty = m_indexHelper->ComputeYFromIndex(index);

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
    auto tileCoordinate = TileCoordinate{ m_indexHelper->ComputeXFromIndex(index), m_indexHelper->ComputeYFromIndex(index) };

    if (m_mines[index])
    {
        m_ui->UpdateTileAsMine(tileCoordinate);
    }
    else
    {
        int count = m_neighborCounts[index];
        m_ui->UpdateTileWithMineCount(tileCoordinate, count);
    }

    m_mineStates[index] = MineState::Revealed;
}

bool Minesweeper::IsInBoundsAndUnmarked(int x, int y)
{
    int index = m_indexHelper->ComputeIndex(x, y);
    return m_indexHelper->IsInBounds(x, y) && m_mineStates[index] == MineState::Empty;
}

void Minesweeper::PushIfUnmarked(std::queue<int> & sweeps, int x, int y)
{
    if (IsInBoundsAndUnmarked(x, y))
    {
        int index = m_indexHelper->ComputeIndex(x, y);
        Reveal(index);
        sweeps.push(index);
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
        auto excludeIndex = m_indexHelper->ComputeIndex(excludeX, excludeY);
        do
        {
            index = GenerateIndex(0, m_gameBoardWidth * m_gameBoardHeight - 1);
        } while (index == excludeIndex || m_mines[index]);

        m_mines[index] = true;
    }

    m_neighborCounts.clear();
    for (int i = 0; i < m_mines.size(); i++)
    {
        int x = m_indexHelper->ComputeXFromIndex(i);
        int y = m_indexHelper->ComputeYFromIndex(i);

        if (m_mines[i])
        {
            // -1 means a mine
            m_neighborCounts.push_back(-1);
            // DEBUG
#if SHOW_MINES
            m_ui->UpdateTileWithState({ x, y }, MineState::Question);
#endif
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

bool Minesweeper::TestSpot(int x, int y)
{
    return m_indexHelper->IsInBounds(x, y) && m_mines[m_indexHelper->ComputeIndex(x, y)];
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

void Minesweeper::CheckTileForMineForAnimation(int x, int y, std::queue<int>& mineIndices, int& visitedTiles, int& minesInRing)
{
    if (m_indexHelper->IsInBounds(x, y))
    {
        auto tileIndex = m_indexHelper->ComputeIndex(x, y);
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
    while (visitedTiles < (m_gameBoardWidth * m_gameBoardHeight))
    {
        if (ringLevel == 0)
        {
            auto hitMineIndex = m_indexHelper->ComputeIndex(centerX, centerY);
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
    m_ui->PlayMineAnimations(mineIndices, minesPerRing);
}

bool Minesweeper::CheckIfWon()
{
    // Get the number of non-revealed tiles
    auto nonRevealedTiles = 0;
    for (auto& state : m_mineStates)
    {
        if (state != MineState::Revealed)
        {
            nonRevealedTiles += 1;
        }
    }

    return nonRevealedTiles == m_numMines;
}
