#include "pch.h"
#include "StreamHelper.h"

#include <shcore.h>
#include <windows.storage.streams.h>

namespace interop
{
    using namespace ABI::Windows::Storage::Streams;
}

winrt::com_ptr<IStream>
StreamHelper::CreateStreamOverRandomAccessStream(winrt::IRandomAccessStream const& stream)
{
    winrt::com_ptr<IStream> result;
    winrt::check_hresult(::CreateStreamOverRandomAccessStream(winrt::get_abi(stream), winrt::guid_of<IStream>(), result.put_void()));
    return result;
}