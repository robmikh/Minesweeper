#include "pch.h"
#include <random>
#include <queue>

#include "winrt/Windows.Foundation.Metadata.h"
#include "winrt/Windows.UI.Input.h"

using namespace winrt;
#include "ImageLoader.h"


using namespace std;

using namespace Windows::Graphics::Display;

enum MineState
{
    Empty = 0,
    Flag = 1,
    Question = 2,

    Last = 3
};

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const & view)
    {
        m_view = view;
    }

    void Load(hstring const&)
    {
    }

    void Uninitialize()
    {
        m_sizeChanged.revoke();
        m_pointerMoved.revoke();
        m_pointerPressed.revoke();

        if (m_loader != nullptr)
        {
            delete m_loader;
        }
    }

    void Run()
    {
        m_compositor = Compositor();
        m_loader = new ImageLoader(m_compositor);

        m_backgroundVisual = m_compositor.CreateSpriteVisual();
        m_backgroundVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
        m_backgroundVisual.Brush(m_compositor.CreateColorBrush(Colors::White()));
        m_target = m_compositor.CreateTargetForCurrentView();
        m_target.Root(m_backgroundVisual);

        m_tileSize = { 25, 25 };
        m_margin = { 2.5f, 2.5f };
        m_gameBoardMargin = { 100.0f, 100.0f };

        m_gameBoard = m_compositor.CreateContainerVisual();
        m_gameBoard.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0.0f });
        m_gameBoard.AnchorPoint({ 0.5f, 0.5f });
        m_backgroundVisual.Children().InsertAtTop(m_gameBoard);

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
        m_backgroundVisual.Children().InsertAtTop(m_selectionVisual);
        m_currentSelectionX = -1;
        m_currentSelectionY = -1;

        auto visual = m_compositor.CreateSpriteVisual();
        visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
        visual.Brush(m_compositor.CreateSurfaceBrush(m_loader->LoadFromUri(Uri(L"ms-appx:///Assets/StoreLogo.png"))));
        m_backgroundVisual.Children().InsertAtTop(visual);

        NewGame(16, 16, 40);

        m_sizeChanged = m_window.SizeChanged(auto_revoke, { this, &App::OnSizeChanged });
        UpdateBoardScale(GetWindowSize());

        m_pointerMoved = m_window.PointerMoved(auto_revoke, { this, &App::OnPointerMoved });
        m_pointerPressed = m_window.PointerPressed(auto_revoke, { this, &App::OnPointerPressed });

        m_window.Activate();

        CoreDispatcher dispatcher = m_window.Dispatcher();
        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    }

    void NewGame(int boardWidth, int boardHeight, int mines)
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

        UpdateBoardScale(GetWindowSize());
    }

    void SetWindow(CoreWindow const & window)
    {
        m_window = window;
    }

    void OnSizeChanged(CoreWindow const & window, WindowSizeChangedEventArgs const & args)
    {
        auto windowSize = float2(args.Size());
        UpdateBoardScale(windowSize);
    }

    void UpdateBoardScale(float2 windowSize)
    {
        float scaleFactor = ComputeScaleFactor(windowSize);

        m_gameBoard.Scale({ scaleFactor, scaleFactor, 1.0f });
    }

    float ComputeScaleFactor()
    {
        return ComputeScaleFactor(GetWindowSize());
    }

    float ComputeScaleFactor(float2 windowSize)
    {
        auto boardSize = m_gameBoard.Size() + m_gameBoardMargin;
        float scaleFactor = windowSize.y / boardSize.y;

        if (boardSize.x > windowSize.x)
        {
            scaleFactor = windowSize.x / boardSize.x;
        }

        return scaleFactor;
    }

    float2 GetWindowSize()
    {
        return { m_window.Bounds().Width, m_window.Bounds().Height };
    }

    void OnPointerMoved(IInspectable const & window, PointerEventArgs const & args)
    {
        float2 point = args.CurrentPoint().Position();

        auto windowSize = GetWindowSize();
        auto scale = ComputeScaleFactor();
        auto realBoardSize = m_gameBoard.Size() *scale;
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

    void OnPointerPressed(IInspectable const & window, PointerEventArgs const & args)
    {
        if (m_currentSelectionX >= 0.0f ||
            m_currentSelectionY >= 0.0f)
        {
            int index = ComputeIndex(m_currentSelectionX, m_currentSelectionY);
            auto visual = m_tiles[index];

            if (m_mineStates[index] != MineState::Last)
            {
                if (args.CurrentPoint().Properties().IsRightButtonPressed() ||
                    args.CurrentPoint().Properties().IsEraser())
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

    bool Sweep(int x, int y)
    {
        bool hitMine = false;
        queue<int> sweeps;
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
    
    void Reveal(int index)
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

    bool IsInBoundsAndUnmarked(int x, int y)
    {
        int index = ComputeIndex(x, y);
        return IsInBounds(x, y) && m_mineStates[index] == MineState::Empty;
    }

    void PushIfUnmarked(queue<int> &sweeps, int x, int y)
    {
        if (IsInBoundsAndUnmarked(x, y))
        {
            int index = ComputeIndex(x, y);
            Reveal(index);
            sweeps.push(index);
        }
    }

    CompositionColorBrush GetColorBrushFromMineState(MineState state)
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

    CompositionColorBrush GetColorBrushFromMineCount(int count)
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

    void GenerateMines(int numMines)
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

    int GenerateIndex(int min, int max)
    {
        return min + (rand() % static_cast<int>(max - min + 1));
    }

    int ComputeIndex(int x, int y)
    {
        return x * m_gameBoardHeight + y;
    }

    int ComputeXFromIndex(int index)
    {
        return index / m_gameBoardHeight;
    }

    int ComputeYFromIndex(int index)
    {
        return index % m_gameBoardHeight;
    }

    bool IsInBounds(int x, int y)
    {
        return ((x >= 0 && x < m_gameBoardWidth) &&
            (y >= 0 && y < m_gameBoardHeight));
    }

    bool TestSpot(int x, int y)
    {
        return IsInBounds(x, y) && m_mines[ComputeIndex(x, y)];
    }

    int GetSurroundingMineCount(int x, int y)
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

    CoreApplicationView m_view{ nullptr };
    CoreWindow m_window{ nullptr };

    Compositor m_compositor{ nullptr };
    ImageLoader* m_loader{ nullptr };

    SpriteVisual m_backgroundVisual{ nullptr };
    CompositionTarget m_target{ nullptr };

    ContainerVisual m_gameBoard{ nullptr };
    vector<SpriteVisual> m_tiles;
    SpriteVisual m_selectionVisual{ nullptr };

    int m_gameBoardWidth;
    int m_gameBoardHeight;
    float2 m_tileSize;
    float2 m_margin;
    float2 m_gameBoardMargin;
    int m_currentSelectionX;
    int m_currentSelectionY;
    vector<MineState> m_mineStates;
    vector<bool> m_mines;
    vector<int> m_neighborCounts;

    CoreWindow::SizeChanged_revoker m_sizeChanged;
    CoreWindow::PointerMoved_revoker m_pointerMoved;
    CoreWindow::PointerPressed_revoker m_pointerPressed;
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(App());
}