#include "pch.h"

#include "winrt/Windows.Foundation.Metadata.h"
#include "winrt/Windows.UI.Input.h"

using namespace Windows::Graphics::Display;
using namespace Windows::Foundation::Metadata;

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
	}

	void Run()
	{
		m_compositor = Compositor();
		m_backgroundVisual = m_compositor.CreateSpriteVisual();
		m_backgroundVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
		m_backgroundVisual.Brush(m_compositor.CreateColorBrush(Colors::White()));
		m_target = m_compositor.CreateTargetForCurrentView();
		m_target.Root(m_backgroundVisual);

		m_gameBoardWidth = 25;
		m_gameBoardHeight = 25;
		m_tileSize = { 25, 25 };
		m_margin = { 2.5f, 2.5f };
		m_gameBoardMargin = { 100.0f, 100.0f };

		m_gameBoard = m_compositor.CreateContainerVisual();
		m_gameBoard.Size((m_tileSize + m_margin) * float2(m_gameBoardWidth, m_gameBoardHeight));
		m_gameBoard.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0.0f });
		m_gameBoard.AnchorPoint({ 0.5f, 0.5f });
		m_backgroundVisual.Children().InsertAtTop(m_gameBoard);

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
			}
		}

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

		m_sizeChanged = m_window.SizeChanged(auto_revoke, { this, &App::OnSizeChanged });
		UpdateBoardScale(GetWindowSize());

		m_pointerMoved = m_window.PointerMoved(auto_revoke, { this, &App::OnPointerMoved });
		m_pointerPressed = m_window.PointerPressed(auto_revoke, { this, &App::OnPointerPressed });

		m_window.Activate();

		CoreDispatcher dispatcher = m_window.Dispatcher();
		dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
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

		if ((x >= 0 && x < m_gameBoardWidth) &&
			(y >= 0 && y < m_gameBoardHeight))
		{
			int index = x * m_gameBoardHeight + y;
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
			int index = m_currentSelectionX * m_gameBoardHeight + m_currentSelectionY;
			auto visual = m_tiles[index];

			if (args.CurrentPoint().Properties().IsRightButtonPressed() ||
				args.CurrentPoint().Properties().IsEraser())
			{
				visual.Brush(m_compositor.CreateColorBrush(Colors::Orange()));
			}
			else
			{
				visual.Brush(m_compositor.CreateColorBrush(Colors::Green()));
			}
		}
	}

	CoreApplicationView m_view{ nullptr };
	CoreWindow m_window{ nullptr };

	Compositor m_compositor{ nullptr };
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

	CoreWindow::SizeChanged_revoker m_sizeChanged;
	CoreWindow::PointerMoved_revoker m_pointerMoved;
	CoreWindow::PointerPressed_revoker m_pointerPressed;
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	CoreApplication::Run(App());
}