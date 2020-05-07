#pragma once

#include <functional>
#include <utility>

enum class MineState
{
    Empty = 0,
    Flag = 1,
    Question = 2,
    Revealed = 3
};

enum class MineGenerationState
{
    Deferred,
    Generated,
};

class MinesweeperBoard
{
public:
    MinesweeperBoard(int boardWidth, int boardHeight, int mines);
    void NewGame(std::function<void(int x, int y)> revealedSpotCallback );
    bool IsGameOver();
    int GetBoardWidth();
    int GetBoardHeight();

    int ComputeIndex(int x, int y);

    void CycleMineState(int x, int y);
    bool Sweep(int x, int y);
    bool IsInBounds(int x, int y);
    int GetSurroundingMineCount(int x, int y);
    MineState GetMineState(int x, int y);
    bool IsMine(int x, int y);

private:
    void PushIfUnmarked(std::queue<std::pair<int, int>>& sweeps, int x, int y);
    void GenerateMines(int numMines, int excludeX, int excludeY);
    int GenerateIndex(int min, int max);
    int ComputeXFromIndex(int index);
    int ComputeYFromIndex(int index);
    void Reveal(int x, int y);
    bool IsInBoundsAndUnmarked(int x, int y);
    bool TestSpot(int x, int y);

private:
    int m_gameBoardWidth;
    int m_gameBoardHeight;

    MineGenerationState m_mineGenerationState = MineGenerationState::Deferred;
    int m_numMines = 0;

    std::vector<MineState> m_mineStates;
    std::vector<bool> m_mines;
    std::vector<int> m_neighborCounts;

    bool m_gameOver = false;

    std::function<void( int x, int y )> m_revealedSpotCallback;
};
