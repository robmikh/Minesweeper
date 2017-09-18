#pragma once

class StreamHelper
{
public:
    static winrt::com_ptr<IStream> CreateStreamOverRandomAccessStream(winrt::IRandomAccessStream const& stream);

private:
    StreamHelper() {}
};