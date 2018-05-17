//
// RootPage.xaml.h
// Declaration of the RootPage class
//

#pragma once

#include "RootPage.g.h"
#include "ViewModels\RootPageViewModel.h"

namespace ModelViewer
{
	using namespace ViewModels;

	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class RootPage sealed
	{
	public:
		RootPage();

		void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
		void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

		Windows::UI::Xaml::Controls::Frame^ GetContentFrame() { return this->ContentFrame; }
		property RootPageViewModel ^ ViewModel;

	private:
		void NavView_ItemInvoked(Windows::UI::Xaml::Controls::NavigationView^ sender, Windows::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs^ args);
		void NavView_SelectionChanged(Windows::UI::Xaml::Controls::NavigationView^ sender, Windows::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs^ args);
		void MoreInfoBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void NavView_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ImportClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
