#pragma once

#include "Common/ViewModelBase.h"
#include <winrt/Microsoft.Holographic.AppRemoting.h>
#include <winrt/Windows.Graphics.Holographic.h>

namespace ViewModels
{
	[Windows::UI::Xaml::Data::Bindable]
	public ref class ConnectPageViewModel sealed : public Common::ViewModelBase
	{
	public:
		ConnectPageViewModel();

		property bool Loading { bool get(); void set(bool val); }
		property Windows::UI::Xaml::Input::ICommand^ ConnectCommand;
		property Platform::String^ IPAddress { Platform::String^ get(); void set(Platform::String^ val); }

	private:
		void ExecuteConnectCommand(Object^ param);
		std::future<void> Connect();

		void ConnectPageViewModel::OnRemoteConnected(winrt::Microsoft::Holographic::AppRemoting::RemoteContext context);

		bool _loading = false;
		Platform::String^ _ipAddress = L"127.0.0.1";

		// RemoteContext used to connect with a Holographic Remoting player and display rendered frames
		winrt::Microsoft::Holographic::AppRemoting::RemoteContext _remoteContext = nullptr;
		winrt::Windows::Graphics::Holographic::HolographicSpace _holographicSpace = nullptr;

		winrt::Microsoft::Holographic::AppRemoting::IRemoteContext::OnConnected_revoker _onConnectedEventRevoker;
		winrt::Microsoft::Holographic::AppRemoting::IRemoteContext::OnDisconnected_revoker _onDisconnectedEventRevoker;
		winrt::Microsoft::Holographic::AppRemoting::IRemoteContext::OnListening_revoker _onListeningEventRevoker;
	};
}

