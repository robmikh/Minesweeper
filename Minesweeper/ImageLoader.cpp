#include "pch.h"

using namespace winrt;

#include "ImageLoader.h"

ImageLoader::ImageLoader(Compositor const & compositor)
{
    m_factory = CreateD2DFactory();
    m_d3dDevice = CreateD3DDevice();
    m_d2dDevice = CreateD2DDevice(m_factory, m_d3dDevice);
    m_wicFactory = CreateWICFactory();

    m_graphics = CompositionInteropHelper::CreateCompositionGraphicsDevice(compositor, m_d2dDevice);
}

ImageLoader::~ImageLoader()
{
    m_factory = nullptr;
    m_d3dDevice = nullptr;
    m_d2dDevice = nullptr;
}

ICompositionSurface
ImageLoader::LoadFromUri(Uri const& uri)
{
    auto surface = m_graphics.CreateDrawingSurface(
        { 0, 0 }, 
        Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 
        Windows::Graphics::DirectX::DirectXAlphaMode::Premultiplied);

    DrawUriIntoSurface(surface, uri);

    return surface;
}

fire_and_forget
ImageLoader::DrawUriIntoSurface(
    CompositionDrawingSurface surface,
    Uri uri)
{
    auto file = await StorageFile::GetFileFromApplicationUriAsync(uri);
    auto randomAccessStream = await file.OpenReadAsync();
    auto stream = StreamHelper::CreateStreamOverRandomAccessStream(randomAccessStream);

    com_ptr<IWICBitmapDecoder> wicBitmapDecoder;
    check_hresult(m_wicFactory->CreateDecoderFromStream(
        stream.get(),
        nullptr,
        WICDecodeMetadataCacheOnDemand,
        wicBitmapDecoder.put()));

    com_ptr<IWICBitmapFrameDecode> wicFrameBitmapDecode;
    check_hresult(wicBitmapDecoder->GetFrame(0, wicFrameBitmapDecode.put()));

    com_ptr<IWICFormatConverter> wicFormatConverter;
    check_hresult(m_wicFactory->CreateFormatConverter(wicFormatConverter.put()));

    auto targetPixelFormat = GUID_WICPixelFormat32bppPBGRA;

    check_hresult(wicFormatConverter->Initialize(
        wicFrameBitmapDecode.get(),
        targetPixelFormat,
        WICBitmapDitherTypeNone,
        NULL,
        0,
        WICBitmapPaletteTypeMedianCut));

    uint32_t width, height;
    check_hresult(wicFormatConverter->GetSize(&width, &height));

    com_ptr<ID2D1DeviceContext> context;
    check_hresult(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, context.put()));

    com_ptr<ID2D1Bitmap> bitmap;
    check_hresult(context->CreateBitmapFromWicBitmap(wicFormatConverter.get(), bitmap.put()));

    CompositionInteropHelper::ResizeSurface(surface, { (float)width, (float)height });
    {
        SurfaceContext surfaceContext(surface);
        auto deviceContext = surfaceContext.GetDeviceContext();

        deviceContext->DrawBitmap(bitmap.get());
    }
}

com_ptr<IWICImagingFactory2>
ImageLoader::CreateWICFactory()
{
    com_ptr<IWICImagingFactory2> wicFactory;
    check_hresult(
        ::CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            guid_of<IWICImagingFactory>(),
            wicFactory.put_void()));

    return wicFactory;
}

com_ptr<ID2D1Device>
ImageLoader::CreateD2DDevice(
    com_ptr<ID2D1Factory1> const& factory,
    com_ptr<ID3D11Device> const& device)
{
    com_ptr<ID2D1Device> result;
    check_hresult(factory->CreateDevice(device.as<IDXGIDevice>().get(), result.put()));
    return result;
}

com_ptr<ID3D11Device>
ImageLoader::CreateD3DDevice()
{
    com_ptr<ID3D11Device> device;
    HRESULT hr = CreateD3DDevice(D3D_DRIVER_TYPE_HARDWARE, device);

    if (DXGI_ERROR_UNSUPPORTED == hr)
    {
        hr = CreateD3DDevice(D3D_DRIVER_TYPE_WARP, device);
    }

    check_hresult(hr);
    return device;
}

com_ptr<ID2D1Factory1>
ImageLoader::CreateD2DFactory()
{
    D2D1_FACTORY_OPTIONS options{};

#ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    com_ptr<ID2D1Factory1> factory;

    check_hresult(D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        options,
        factory.put()));

    return factory;
}

HRESULT 
ImageLoader::CreateD3DDevice(
    D3D_DRIVER_TYPE const type, 
    com_ptr<ID3D11Device>& device)
{
    WINRT_ASSERT(!device);

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    return D3D11CreateDevice(
        nullptr,
        type,
        nullptr,
        flags,
        nullptr, 0,
        D3D11_SDK_VERSION,
        device.put(),
        nullptr,
        nullptr);
}
