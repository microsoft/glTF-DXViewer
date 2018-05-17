#pragma once

namespace Common
{
	namespace WUX = Windows::UI::Xaml;
	namespace WUXD = Windows::UI::Xaml::Data;

	public ref class BindableBase : public WUXD::INotifyPropertyChanged, WUX::DependencyObject
	{
	public:
		virtual event WUXD::PropertyChangedEventHandler^ PropertyChanged;

	internal:
		Platform::String^ getCallerName(const char* funName);

	protected:
		virtual void OnPropertyChanged(Platform::String^ value);
	};
}

