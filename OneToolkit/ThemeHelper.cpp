﻿#include "pch.h"
#include "ThemeHelper.h"

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::ViewManagement;

void* winrt_make_OneToolkit_UI_ThemeHelper()
{
	return detach_abi(make<OneToolkit::UI::factory_implementation::ThemeHelper>());
}

namespace winrt::OneToolkit::UI::factory_implementation
{
	ApplicationTheme ThemeHelper::SystemAppsTheme()
	{
		return UISettingsHelper::Instance().GetColorValue(UIColorType::Background) == Colors::Black() ? ApplicationTheme::Dark : ApplicationTheme::Light;
	}

	ApplicationTheme ThemeHelper::SystemShellTheme()
	{
		throw hresult_not_implemented();
	}
}