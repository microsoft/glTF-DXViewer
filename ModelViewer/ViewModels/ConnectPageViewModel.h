#pragma once

#include "Common/ViewModelBase.h"

namespace ViewModels
{
	using namespace Common;
	using namespace std;
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

		bool _loading = false;
		String^ _ipAddress;
	};
}

