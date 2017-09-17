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

	void Initialize(CoreApplicationView const &)
	{
	}

	void Load(hstring const&)
	{
	}

	void Uninitialize()
	{
	}

	void Run()
	{
		CoreWindow window = CoreWindow::GetForCurrentThread();
		window.Activate();

		CoreDispatcher dispatcher = window.Dispatcher();
		dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	}

	void SetWindow(CoreWindow const & window)
	{
		m_activated = window.Activated(auto_revoke, { this, &App::OnActivated });
	}

	void OnActivated(CoreWindow window, WindowActivatedEventArgs)
	{
		m_activated.revoke();

		Compositor compositor;
		SpriteVisual visual = compositor.CreateSpriteVisual();
		visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
		visual.Brush(compositor.CreateColorBrush(Colors::Red()));
		m_target = compositor.CreateTargetForCurrentView();
		m_target.Root(visual);
	}

	CoreWindow::Activated_revoker m_activated;
	CompositionTarget m_target{ nullptr };
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	CoreApplication::Run(App());
}