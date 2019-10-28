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
}

void ConnectPageViewModel::ExecuteConnectCommand(Object^ param)
{
	Connect();
}

void ConnectPageViewModel::OnRemoteConnected(RemoteContext context)
{

}

String^ PlatformFailureToString(ConnectionFailureReason reason)
{
	switch (reason)
	{
	case ConnectionFailureReason::AuthenticationFailed:
		return L"Authentication Failed";
	case ConnectionFailureReason::Canceled:
		return L"Canceled";
	case ConnectionFailureReason::ConnectionLost:
		return L"Connection Lost";
	case ConnectionFailureReason::DeviceLost:
		return L"Device Lost";
	case ConnectionFailureReason::DisconnectRequest:
		return L"Disconnect Request";
	case ConnectionFailureReason::HandshakeConnectionFailed:
		return L"Handshake Connection Failed";
	case ConnectionFailureReason::HandshakeFailed:
		return L"Handshake Failed";
	case ConnectionFailureReason::HandshakePortBusy:
		return L"Handshake Port Busy";
	case ConnectionFailureReason::HandshakeUnreachable:
		return L"Handshake Unreachable";
	case ConnectionFailureReason::IncompatibleTransportProtocols:
		return L"Incompatible Transport Protocols";
	case ConnectionFailureReason::NoServerCertificate:
		return L"No Server Certificate";
	case ConnectionFailureReason::ProtocolError:
		return L"Protocol Error";
	case ConnectionFailureReason::ProtocolVersionMismatch:
		return L"Protocol Version Mismatch";
	case ConnectionFailureReason::RemotingVersionMismatch:
		return L"Remoting Version Mismatch";
	case ConnectionFailureReason::TransportConnectionFailed:
		return L"Transport Connection Failed";
	case ConnectionFailureReason::TransportPortBusy:
		return L"Transport Port Busy";
	case ConnectionFailureReason::TransportUnreachable:
		return L"Transport Unreachable";
	default:
		return L"Unknown Failure";
	}
}

#include <future>

future<void> ConnectPageViewModel::SetStatusText(String^ text)
{
	auto disp = ::Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;
	co_await disp->RunAsync(::Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new ::Windows::UI::Core::DispatchedHandler([text, this]() 
			{
				if (StatusText->Length() > 0) StatusText += ref new String(L"\n");
				StatusText += text; 
			}));
}

future<void> ConnectPageViewModel::Connect()
{
	if (Loading == true)
		return;

	Utility::Out(L"At Start of Connect");
	StatusText = L"";

	// RAII-style for ensuring that the progress gets cleared robustly
	auto loader = make_unique<LoadingWrapper>([this]() { Loading = true; }, [this]() { Loading = false; });

	co_await 5s;

	// Create on a thread pool thread?
	try
	{
		CreateRemoteContext(_remoteContext, 20000, false, PreferredVideoCodec::Default);
		_holographicSpace = HolographicSpace::CreateForCoreWindow(nullptr);

		_deviceResources = std::make_shared<DXHelper::HolographicDeviceResources>();
		_deviceResources->SetHolographicSpace(_holographicSpace);

		// TODO: need to wire up the device lost and device restored events here...

		winrt::weak_ref<winrt::Microsoft::Holographic::AppRemoting::IRemoteContext> remoteContextWeakRef = _remoteContext;

		_onConnectedEventRevoker = _remoteContext.OnConnected(winrt::auto_revoke, [this, remoteContextWeakRef]() {
				if (auto remoteContext = remoteContextWeakRef.get())
				{
					auto state = remoteContext.ConnectionState();
					if (state == ConnectionState::Connected)
					{
						SetStatusText("Connected");
					}
					else if (state == ConnectionState::Connecting)
					{
						SetStatusText("Connecting");
					}
					else if (state == ConnectionState::Disconnected)
					{
						SetStatusText("Disconnected");
					}

					// Update UI state
					//this->OnRemoteConnected(remoteContext.as<RemoteContext>());
				}
			});
		_onDisconnectedEventRevoker =
			_remoteContext.OnDisconnected(winrt::auto_revoke, [this, remoteContextWeakRef](ConnectionFailureReason failureReason) {
				if (auto remoteContext = remoteContextWeakRef.get())
				{
					Utility::Out(L"Disconnected with reason %d", failureReason);
					SetStatusText(PlatformFailureToString(failureReason));

					// Update UI

					// Reconnect if this is a transient failure.
					if (failureReason == ConnectionFailureReason::HandshakeUnreachable ||
						failureReason == ConnectionFailureReason::TransportUnreachable ||
						failureReason == ConnectionFailureReason::ConnectionLost)
					{
						Utility::Out(L"Reconnecting...");
						SetStatusText(L"Reconnecting...");

						_remoteContext.Disconnect();
						_remoteContext.Connect(_ipAddress->Data(), 8265);
					}
					// Failure reason None indicates a normal disconnect.
					else if (failureReason != ConnectionFailureReason::None)
					{
						Utility::Out(L"Disconnected with unrecoverable error, not attempting to reconnect.");
						SetStatusText(L"Disconnected with unrecoverable error, not attempting to reconnect.");
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
		SetStatusText(L"Connect failed");
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
