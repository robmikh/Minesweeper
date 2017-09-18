#pragma once

#pragma comment(lib, "windowsapp") 
#pragma comment(lib, "d2d1")
#pragma comment(lib, "windowscodecs")

#include "winrt/Windows.ApplicationModel.Core.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.UI.Core.h"
#include "winrt/Windows.Graphics.Imaging.h"
#include "winrt/Windows.Graphics.Display.h"
#include "winrt/Windows.UI.Composition.h"
#include "winrt/Windows.Storage.h"
#include "winrt/Windows.Storage.Streams.h"

#include <d2d1_1.h>
#include <d3d11_1.h>
#include <wincodec.h>

#include <vector>

namespace winrt
{
    using namespace Windows::ApplicationModel::Core;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI::Core;
    using namespace Windows::System;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Windows::Storage;
    using namespace Windows::Storage::Streams;
}

#include "CompositionInteropHelper.h"
#include "StreamHelper.h"
