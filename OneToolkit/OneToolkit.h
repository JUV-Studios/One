// (c) 2021 JUV Studios. All rights reserved. Included as part of OneToolkit for use in C++ projects targeting the Windows platform.

#pragma once
#include <juv.h>
#ifdef CPPWINRT_VERSION
#include <winrt/OneToolkit.System.h>
#include <winrt/OneToolkit.Storage.h>
#include <winrt/OneToolkit.Runtime.h>
#include <winrt/OneToolkit.Lifecycle.h>
#include <winrt/OneToolkit.UI.Input.h>
#include <winrt/OneToolkit.Media.Imaging.h>
#if defined(framework_winui3) && __has_include(<winrt/Microsoft.UI.h>)
#include <winrt/Microsoft.UI.Xaml.Data.h>

namespace winrt
{
	namespace AppFramework = Microsoft::UI::Xaml;
}

#else
#include <winrt/Windows.UI.Xaml.Data.h>

namespace winrt
{
	namespace AppFramework = Windows::UI::Xaml;
}

#endif
#if defined(__cpp_modules) && !defined(onetoolkit_ixx)
import OneToolkit;
#endif
#elif __cplusplus_winrt
#ifdef framework_winui3
namespace AppFramework = Microsoft::UI::Xaml;
#else
namespace AppFramework = Windows::UI::Xaml;
#endif
#endif