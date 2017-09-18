#pragma once

class ImageLoader
{
public:
    ImageLoader(Compositor const & compositor);
    ~ImageLoader();

    ICompositionSurface LoadFromUri(Uri const& uri);

private:
    fire_and_forget DrawUriIntoSurface(
        CompositionDrawingSurface surface, 
        Uri uri);

private:
    static HRESULT CreateD3DDevice(
        D3D_DRIVER_TYPE const type, 
        com_ptr<ID3D11Device>& device);
    static com_ptr<ID3D11Device> CreateD3DDevice();
    static com_ptr<ID2D1Factory1> CreateD2DFactory();
    static com_ptr<ID2D1Device> CreateD2DDevice(
        com_ptr<ID2D1Factory1> const& factory,
        com_ptr<ID3D11Device> const& device);
    static com_ptr<IWICImagingFactory2> CreateWICFactory();

private:
    com_ptr<ID3D11Device> m_d3dDevice;
    com_ptr<ID2D1Factory1> m_factory;
    com_ptr<ID2D1Device> m_d2dDevice;
    com_ptr<IWICImagingFactory2> m_wicFactory;

    CompositionGraphicsDevice m_graphics{ nullptr };
};