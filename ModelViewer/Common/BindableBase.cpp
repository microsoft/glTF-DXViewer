#include "pch.h"
#include "BindableBase.h"

#include <sstream>
#include <regex>

using namespace Common;

using namespace Platform;
using namespace Windows::UI::Xaml::Data;

String^ BindableBase::getCallerName(const char* funName)
{
	std::string str(funName);
	std::regex regex("(\\w+)(?=\\:\\:)|(?!\\:\\:)(\\w+)");
	std::sregex_iterator it(str.begin(), str.end(), regex);
	std::sregex_iterator it_end;

	std::vector<std::string> vec;

	while (it != it_end)
	{
		vec.push_back((*it).str());
		++it;
	}

	std::string last = *(std::end(vec) - 1);
	std::wstring ws(last.begin(), last.end());

	//if property - get the one before last
	if (last.compare("set") || last.compare("get")) {
		std::string one_before_last = *(std::end(vec) - 2);
		ws.assign(one_before_last.begin(), one_before_last.end());
	}

	return ref new String(ws.c_str());
}

void BindableBase::OnPropertyChanged(String^ value)
{
	PropertyChanged(this, ref new PropertyChangedEventArgs(value));
}