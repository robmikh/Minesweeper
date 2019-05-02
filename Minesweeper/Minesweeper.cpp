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
	auto windowSize = m_parentSize;
	auto scale = ComputeScaleFactor();
	auto realBoardSize = m_gameBoard.Size() * scale;
	auto realOffset = (windowSize - realBoardSize) / 2.0f;

	point -= realOffset;
	point /= scale;

	int x = point.x / (m_tileSize.x + m_margin.x);
	int y = point.y / (m_tileSize.y + m_margin.y);
	int index = x * m_gameBoardHeight + y;

	if ((x >= 0 && x < m_gameBoardWidth) &&
		(y >= 0 && y < m_gameBoardHeight) &&
		m_mineStates[index] != MineState::Last)
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
	if (m_currentSelectionX >= 0.0f ||
		m_currentSelectionY >= 0.0f)
	{
		int index = ComputeIndex(m_currentSelectionX, m_currentSelectionY);
		auto visual = m_tiles[index];

		if (m_mineStates[index] != MineState::Last)
		{
			if (isRightButton ||
				isEraser)
			{
				auto state = m_mineStates[index];
				state = (MineState)((state + 1) % MineState::Last);
				m_mineStates[index] = state;
				visual.Brush(GetColorBrushFromMineState(state));
			}
			else if (m_mineStates[index] == MineState::Empty)
			{
				Sweep(m_currentSelectionX, m_currentSelectionY);
			}
		}
	}
}

void Minesweeper::NewGame(int boardWidth, int boardHeight, int mines)
{
	m_gameBoardWidth = boardWidth;
	m_gameBoardHeight = boardHeight;

	m_gameBoard.Children().RemoveAll();

	m_gameBoard.Size((m_tileSize + m_margin) * float2(m_gameBoardWidth, m_gameBoardHeight));

	for (int x = 0; x < m_gameBoardWidth; x++)
	{
		for (int y = 0; y < m_gameBoardHeight; y++)
		{
			SpriteVisual visual = m_compositor.CreateSpriteVisual();
			visual.Size(m_tileSize);
			visual.Offset(float3((m_margin / 2.0f) + (float2(m_tileSize + m_margin) * float2(x, y)), 0.0f));
			visual.Brush(m_compositor.CreateColorBrush(Colors::Blue()));

			m_gameBoard.Children().InsertAtTop(visual);
			m_tiles.push_back(visual);
			m_mineStates.push_back(MineState::Empty);
		}
	}

	GenerateMines(mines);

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
	float scaleFactor = windowSize.y / boardSize.y;

	if (boardSize.x > windowSize.x)
	{
		scaleFactor = windowSize.x / boardSize.x;
	}

	return scaleFactor;
}

bool Minesweeper::Sweep(int x, int y)
{
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

	m_mineStates[index] = MineState::Last;
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

void Minesweeper::GenerateMines(int numMines)
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
		do
		{
			index = GenerateIndex(0, m_gameBoardWidth * m_gameBoardHeight - 1);
		} while (m_mines[index]);

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
	int index = ComputeIndex(x, y);
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