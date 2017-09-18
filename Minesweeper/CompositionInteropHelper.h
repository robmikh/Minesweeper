#pragma once

class CompositionInteropHelper
{
public:
    static winrt::CompositionGraphicsDevice CreateCompositionGraphicsDevice(
        winrt::Compositor const& compositor,
        winrt::com_ptr<ID3D11Device> const& device);

    static void ResizeSurface(
        winrt::CompositionDrawingSurface const& surface,
        winrt::Size const& size);

private:
    CompositionInteropHelper() { }
};

class SurfaceContext
{
public:
    SurfaceContext(winrt::CompositionDrawingSurface const& surface);
    ~SurfaceContext();

    winrt::com_ptr<ID2D1DeviceContext> GetDeviceContext() { return m_context; }

private:
    winrt::CompositionDrawingSurface m_surface{ nullptr };
    winrt::com_ptr<ID2D1DeviceContext> m_context;
};