#include "pch.h"
#include "ConnectPageViewModel.h"
#include "DelegateCommand.h"
#include "LoadingWrapper.h"
#include <pplawait.h>
#include <chrono>
#include <HolographicAppRemoting\Streamer.h>

using namespace ViewModels;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Microsoft::Holographic::AppRemoting;
using namespace winrt;

ConnectPageViewModel::ConnectPageViewModel()
{
	ConnectCommand = ref new DelegateCommand(ref new ExecuteDelegate(this, &ConnectPageViewModel::ExecuteConnectCommand), nullptr);
}

void ConnectPageViewModel::ExecuteConnectCommand(Object^ param)
{
}

void ConnectPageViewModel::OnRemoteConnected(RemoteContext context)
{

}

future<void> ConnectPageViewModel::Connect()
{
	Utility::Out(L"At Start of Connect");

	co_await 5s;

	// RAII-style for ensuring that the progress gets cleared robustly
	auto loader = make_unique<LoadingWrapper>([this]() { Loading = true; }, [this]() { Loading = false; });

	// Create on a thread pool thread?
	try
	{
		CreateRemoteContext(_remoteContext, 20000, false, PreferredVideoCodec::Default);
		_holographicSpace = HolographicSpace::CreateForCoreWindow(nullptr);
		
		winrt::weak_ref<winrt::Microsoft::Holographic::AppRemoting::IRemoteContext> remoteContextWeakRef = _remoteContext;

		_onConnectedEventRevoker = _remoteContext.OnConnected(winrt::auto_revoke, [this, remoteContextWeakRef]() {
				if (auto remoteContext = remoteContextWeakRef.get())
				{
					// Update UI state
					//this->OnRemoteConnected(remoteContext.as<RemoteContext>());
				}
			});
		_onDisconnectedEventRevoker =
			_remoteContext.OnDisconnected(winrt::auto_revoke, [this, remoteContextWeakRef](ConnectionFailureReason failureReason) {
				if (auto remoteContext = remoteContextWeakRef.get())
				{
					Utility::Out(L"Disconnected with reason %d", failureReason);

					// Update UI

					// Reconnect if this is a transient failure.
					if (failureReason == ConnectionFailureReason::HandshakeUnreachable ||
						failureReason == ConnectionFailureReason::TransportUnreachable ||
						failureReason == ConnectionFailureReason::ConnectionLost)
					{
						Utility::Out(L"Reconnecting...");

						//ConnectOrListen();
					}
					// Failure reason None indicates a normal disconnect.
					else if (failureReason != ConnectionFailureReason::None)
					{
						Utility::Out(L"Disconnected with unrecoverable error, not attempting to reconnect.");
					}
				}
			});

		_onListeningEventRevoker = _remoteContext.OnListening(winrt::auto_revoke, [this, remoteContextWeakRef](uint16_t port) {
				if (auto remoteContext = remoteContextWeakRef.get())
				{
					// Update UI state
				}
			});

		_remoteContext.Connect(_ipAddress->Data(), 8265);
	}
	catch (winrt::hresult_error & e)
	{
		Utility::Out(L"Connect failed with hr = 0x%08X", e.code());
	}

	Utility::Out(L"Connected");

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
