#include "pch.h"

using namespace winrt;

using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace Windows::System;
using namespace Windows::UI;

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
	}

	void Run()
	{
		m_compositor = Compositor();
		m_backgroundVisual = m_compositor.CreateSpriteVisual();
		m_backgroundVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
		m_backgroundVisual.Brush(m_compositor.CreateColorBrush(Colors::Red()));
		m_target = m_compositor.CreateTargetForCurrentView();
		m_target.Root(m_backgroundVisual);

		m_window.Activate();

		CoreDispatcher dispatcher = m_window.Dispatcher();
		dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	}

	void SetWindow(CoreWindow const & window)
	{
		m_window = window;
	}

	CoreApplicationView m_view{ nullptr };
	CoreWindow m_window{ nullptr };

	Compositor m_compositor{ nullptr };
	SpriteVisual m_backgroundVisual{ nullptr };
	CompositionTarget m_target{ nullptr };
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	CoreApplication::Run(App());
}