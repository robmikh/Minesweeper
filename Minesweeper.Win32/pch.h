#pragma once

#include <Windows.h>
#include <Unknwn.h>
#include <inspectable.h>

#include <wil/cppwinrt.h>

// WinRT
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.Graphics.h>

#include <windows.ui.composition.interop.h>
#include <DispatcherQueue.h>

// STL
#include <atomic>
#include <memory>
#include <vector>
#include <queue>
#include <math.h>
#include <string>

// WIL
#include <wil/resource.h>

// D3D
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d2d1_3.h>
#include <wincodec.h>

// Helpers
#include <robmikh.common/composition.desktop.interop.h>
#include <robmikh.common/composition.interop.h>
#include <robmikh.common/d3dHelpers.h>
#include <robmikh.common/d3dHelpers.desktop.h>
#include <robmikh.common/direct3d11.interop.h>
#include <robmikh.common/stream.interop.h>
#include <robmikh.common/hwnd.interop.h>
#include <robmikh.common/dispatcherqueue.desktop.interop.h>
#include <robmikh.common/capture.desktop.interop.h>