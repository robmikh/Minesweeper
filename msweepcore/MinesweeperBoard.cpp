#include "pch.h"
#include "MinesweeperBoard.h"

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

MinesweeperBoard::MinesweeperBoard(int boardWidth, int boardHeight, int mines)
    : m_gameBoardWidth(boardWidth)
    , m_gameBoardHeight(boardHeight)
    , m_numMines(mines)
{
    NewGame(nullptr);
}

void MinesweeperBoard::NewGame(std::function<void(int x, int y)> revealedSpotCallback)
{
    m_revealedSpotCallback = revealedSpotCallback;

    m_mineStates.clear();
    for (int x = 0; x < m_gameBoardWidth; x++)
    {
        for (int y = 0; y < m_gameBoardHeight; y++)
        {
            m_mineStates.push_back(MineState::Empty);
        }
    }

    m_gameOver = false;
    m_mineGenerationState = MineGenerationState::Deferred;
}

void MinesweeperBoard::CycleMineState(int x, int y)
{
    m_mineStates[ComputeIndex(x, y)] = ::CycleMineState(m_mineStates[ComputeIndex(x, y)]);
}

bool MinesweeperBoard::Sweep(int x, int y)
{
    if (m_mineGenerationState == MineGenerationState::Deferred)
    {
        // We don't want the first thing that the user clicks to be a mine. 
        // Generate mines but avoid putting it where the user clicked.
        GenerateMines(m_numMines, x, y);
        m_mineGenerationState = MineGenerationState::Generated;
    }

    bool hitMine = false;
    std::queue<std::pair<int, int>> sweeps;
    sweeps.push({ x, y });
    Reveal(sweeps.front().first, sweeps.front().second);

    while (!sweeps.empty())
    {
        auto [currentx, currenty] = sweeps.front();

        if (m_mines[ComputeIndex(currentx, currenty)])
        {
            // We hit a mine, game over
            hitMine = true;
            m_gameOver = true;
            break;
        }

        if (m_neighborCounts[ComputeIndex(currentx, currenty)] == 0)
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

void MinesweeperBoard::Reveal(int x, int y)
{
    m_mineStates[ComputeIndex(x, y)] = MineState::Revealed;
    m_revealedSpotCallback(x, y);
}

bool MinesweeperBoard::IsInBoundsAndUnmarked(int x, int y)
{
    int index = ComputeIndex(x, y);
    return IsInBounds(x, y) && m_mineStates[index] == MineState::Empty;
}

void MinesweeperBoard::PushIfUnmarked(std::queue<std::pair<int, int>>& sweeps, int x, int y)
{
    if (IsInBoundsAndUnmarked(x, y))
    {
        Reveal(x, y);
        sweeps.push({ x, y });
    }
}

void MinesweeperBoard::GenerateMines(int numMines, int excludeX, int excludeY)
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

int MinesweeperBoard::GenerateIndex(int min, int max)
{
    return min + (rand() % static_cast<int>(max - min + 1));
}

int MinesweeperBoard::ComputeIndex(int x, int y)
{
    return x * m_gameBoardHeight + y;
}

int MinesweeperBoard::ComputeXFromIndex(int index)
{
    return index / m_gameBoardHeight;
}

int MinesweeperBoard::ComputeYFromIndex(int index)
{
    return index % m_gameBoardHeight;
}

bool MinesweeperBoard::IsInBounds(int x, int y)
{
    return ((x >= 0 && x < m_gameBoardWidth) &&
             (y >= 0 && y < m_gameBoardHeight));
}

bool MinesweeperBoard::TestSpot(int x, int y)
{
    return IsInBounds(x, y) && m_mines[ComputeIndex(x, y)];
}

int MinesweeperBoard::GetSurroundingMineCount(int x, int y)
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

bool MinesweeperBoard::IsGameOver()
{
    return m_gameOver;
}

int MinesweeperBoard::GetBoardWidth()
{
    return m_gameBoardWidth;
}

int MinesweeperBoard::GetBoardHeight()
{
    return m_gameBoardHeight;
}

MineState MinesweeperBoard::GetMineState(int x, int y)
{
    return m_mineStates[ComputeIndex(x, y)];
}

bool MinesweeperBoard::IsMine(int x, int y)
{
    assert(m_gameOver || GetMineState(x, y) == MineState::Revealed);//Did you want to reveal the spot first?

    return m_mines[ComputeIndex(x, y)];
}

