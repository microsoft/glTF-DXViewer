//
// ConnectPage.xaml.h
// Declaration of the ConnectPage class
//

#pragma once

#include "ConnectPage.g.h"

namespace ModelViewer
{
	using namespace ViewModels;

	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ConnectPage sealed
	{
	public:
		ConnectPage();

		property ConnectPageViewModel^ ViewModel;
	};
}
