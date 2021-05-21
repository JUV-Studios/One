#include "pch.h"
#include "PageViewLinkItem.h"

using namespace Platform;
using namespace Concurrency;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml::Data;
using namespace OneToolkit::UI::Controls;

PageViewLinkItem::PageViewLinkItem()
{
	SelectsOnInvoked = false;
	Options = ref new LauncherOptions();
	Options->TreatAsUntrusted = true;
}

String^ PageViewLinkItem::Label::get()
{
	return Content->ToString();
}

void PageViewLinkItem::Label::set(String^ value)
{
	if (Content != value)
	{
		Content = value;
		PropertyChanged(this, ref new PropertyChangedEventArgs("Label"));
	}
}

IAsyncAction^ PageViewLinkItem::InvokeAsync()
{
	return create_async([this]() -> task<void>
		{
			auto result = co_await Launcher::LaunchUriAsync(NavigateUri, Options);
			AfterLinkLaunched(result);
		});
}