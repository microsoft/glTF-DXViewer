#include "pch.h"
#include "ConnectPageViewModel.h"
#include "DelegateCommand.h"
#include "LoadingWrapper.h"
#include <pplawait.h>
#include <chrono>
#include <HolographicAppRemoting\Streamer.h>
#include <Common\HolographicDeviceResources.h>

using namespace ViewModels;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Microsoft::Holographic::AppRemoting;
using namespace winrt;

ConnectPageViewModel::ConnectPageViewModel()
{
	ConnectCommand = ref new DelegateCommand(ref new ExecuteDelegate(this, &ConnectPageViewModel::ExecuteConnectCommand), nullptr);
	_remoteRenderer = Container::Instance().ResolveRemoteRenderer();
}

void ConnectPageViewModel::ExecuteConnectCommand(Object^ param)
{
	ConnectAsync();
}

void ConnectPageViewModel::OnRemoteConnected(RemoteContext context)
{

}

#include <future>

future<void> ConnectPageViewModel::SetStatusTextAsync(wchar_t *text)
{
	auto disp = ::Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;
	co_await disp->RunAsync(::Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new ::Windows::UI::Core::DispatchedHandler([text, this]() 
			{
				if (StatusText->Length() > 0) StatusText += ref new String(L"\n");
				StatusText += ref new String(text); 
			}));
}

future<void> ConnectPageViewModel::ConnectAsync()
{
	if (Loading == true)
		return;

	Utility::Out(L"At Start of Connect");
	StatusText = L"";

	// RAII-style for ensuring that the progress gets cleared robustly
	auto loader = make_unique<LoadingWrapper>([this]() { Loading = true; }, [this]() { Loading = false; });

	// Just a dummy async wait while testing... 
	co_await 1s;

	// Create on a thread pool thread?
	_remoteRenderer->RegisterForStatusUpdates([this](wchar_t *text)
		{
			SetStatusTextAsync(text);
		});

	co_await _remoteRenderer->ConnectAsync(_ipAddress->Data());
	co_return;
}

bool ConnectPageViewModel::Loading::get()
{
	return _loading;
}

void ConnectPageViewModel::Loading::set(bool val)
{
	if (_loading == val)
		return;
	_loading = val;
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

String^ ConnectPageViewModel::StatusText::get()
{
	return _statusText;
}

void ConnectPageViewModel::StatusText::set(String^ val)
{
	if (_statusText == val)
		return;
	_statusText = val;
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

String^ ConnectPageViewModel::IPAddress::get()
{
	return _ipAddress;
}

void ConnectPageViewModel::IPAddress::set(String^ val)
{
	if (_ipAddress == val)
		return;
	_ipAddress = val;
	OnPropertyChanged(getCallerName(__FUNCTION__));
}
