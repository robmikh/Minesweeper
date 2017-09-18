#include "pch.h"
#include "CompositionInteropHelper.h"

#include <windows.ui.composition.interop.h>

namespace interop
{
    using namespace ABI::Windows::UI::Composition;
}

template <typename T>
T from_abi(::IUnknown* from)
{
    T to{ nullptr };

    winrt::check_hresult(from->QueryInterface(winrt::guid_of<T>(),
        reinterpret_cast<void**>(winrt::put_abi(to))));

    return to;
}

winrt::CompositionGraphicsDevice
CompositionInteropHelper::CreateCompositionGraphicsDevice(
    winrt::Compositor const& compositor,
    winrt::com_ptr<ID3D11Device> const& device)
{
    winrt::com_ptr<interop::ICompositorInterop> compositorInterop;
    winrt::com_ptr<interop::ICompositionGraphicsDevice> graphicsInterop;
    winrt::check_hresult(winrt::get_abi(compositor)->QueryInterface(winrt::guid_of<interop::ICompositorInterop>(), compositorInterop.put_void()));
    winrt::check_hresult(compositorInterop->CreateGraphicsDevice(device.get(), graphicsInterop.put()));

    auto graphics = from_abi<winrt::CompositionGraphicsDevice>(graphicsInterop.get());
    return graphics;
}

void
CompositionInteropHelper::ResizeSurface(
    winrt::CompositionDrawingSurface const& surface,
    winrt::Size const& size)
{
    winrt::com_ptr<interop::ICompositionDrawingSurfaceInterop> surfaceInterop;
    winrt::check_hresult(winrt::get_abi(surface)->QueryInterface(winrt::guid_of<interop::ICompositionDrawingSurfaceInterop>(), surfaceInterop.put_void()));

    SIZE newSize = {};
    newSize.cx = static_cast<LONG>(std::round(size.Width));
    newSize.cy = static_cast<LONG>(std::round(size.Height));
    winrt::check_hresult(surfaceInterop->Resize(newSize));
}

SurfaceContext::SurfaceContext(
    winrt::CompositionDrawingSurface const& surface)
{
    winrt::com_ptr<ID2D1DeviceContext> context;

    winrt::com_ptr<interop::ICompositionDrawingSurfaceInterop> surfaceInterop;
    winrt::get_abi(surface)->QueryInterface(winrt::guid_of<interop::ICompositionDrawingSurfaceInterop>(), surfaceInterop.put_void());

    POINT offset = {};
    winrt::check_hresult(surfaceInterop->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), context.put_void(), &offset));

    context->SetTransform(D2D1::Matrix3x2F::Translation(offset.x, offset.y));

    m_context = context;
    m_surface = surface;
}

SurfaceContext::~SurfaceContext()
{
    winrt::com_ptr<interop::ICompositionDrawingSurfaceInterop> surfaceInterop;
    winrt::get_abi(m_surface)->QueryInterface(winrt::guid_of<interop::ICompositionDrawingSurfaceInterop>(), surfaceInterop.put_void());

    winrt::check_hresult(surfaceInterop->EndDraw());
    m_context = nullptr;
    m_surface = nullptr;
}