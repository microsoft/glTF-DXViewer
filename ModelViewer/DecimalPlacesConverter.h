#pragma once
namespace ModelViewer
{
	[Windows::UI::Xaml::Data::Bindable]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class DecimalPlacesConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		DecimalPlacesConverter();

		// Inherited via IValueConverter
		virtual Platform::Object ^ Convert(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language);
		virtual Platform::Object ^ ConvertBack(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language);

		property int DecimalPlaces
		{
			int get() { return _dps;; }
			void set(int value) { _dps = value; }
		}
	private:
		int _dps = 2;
	};
}
