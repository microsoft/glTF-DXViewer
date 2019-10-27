#pragma once

#include "Common/ViewModelBase.h"
#include <winrt/Microsoft.Holographic.AppRemoting.h>

namespace ViewModels
{
	using namespace Common;
	using namespace Windows::UI::Xaml::Input;
	using namespace Platform;

	[Windows::UI::Xaml::Data::Bindable]
	public ref class ConnectPageViewModel sealed : public ViewModelBase
	{
	public:
		ConnectPageViewModel();

		property bool Loading { bool get(); void set(bool val); }
		property ICommand^ ConnectCommand;
		property String^ IPAddress { String^ get(); void set(String^ val); }

	private:
		void ExecuteConnectCommand(Object^ param);
		std::future<void> Connect();

		bool _loading = false;
		String^ _ipAddress;

		// RemoteContext used to connect with a Holographic Remoting player and display rendered frames
		winrt::Microsoft::Holographic::AppRemoting::RemoteContext _remoteContext = nullptr;
	};
}

