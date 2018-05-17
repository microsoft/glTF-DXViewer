#include "pch.h"
#include "DecimalPlacesConverter.h"
#include <iomanip> 
#include <sstream>  

using namespace ModelViewer;
using namespace Platform;
using namespace Windows::UI::Xaml::Interop;

DecimalPlacesConverter::DecimalPlacesConverter()
{
}

Object ^ DecimalPlacesConverter::Convert(Object ^value, TypeName targetType, Object ^parameter, String ^language)
{
	if (value != nullptr)
	{
		wchar_t number[256];
		swprintf_s(number, 256, L"%.2f", (float)value);
		return ref new Platform::String(number);
	}
	return nullptr;
}

Object ^ DecimalPlacesConverter::ConvertBack(Object ^value, TypeName targetType, Object ^parameter, String ^language)
{
	if (value != nullptr)
	{
		auto str = value->ToString();
		auto ret = _wtof(str->Data());
		return (float)ret;
	}
	return nullptr;
}
