#include "pch.h"
#include "ConnectPageViewModel.h"
#include "DelegateCommand.h"
#include "LoadingWrapper.h"
#include <pplawait.h>
#include <chrono>

using namespace ViewModels;
using namespace std::chrono;

ConnectPageViewModel::ConnectPageViewModel()
{
	ConnectCommand = ref new DelegateCommand(ref new ExecuteDelegate(this, &ConnectPageViewModel::ExecuteConnectCommand), nullptr);
}

void ConnectPageViewModel::ExecuteConnectCommand(Object^ param)
{
}

future<void> ConnectPageViewModel::Connect()
{
	Utility::Out(L"At Start of Connect");

	// RAII-style for ensuring that the progress gets cleared robustly
	auto loader = make_unique<LoadingWrapper>([this]() { Loading = true; }, [this]() { Loading = false; });

	//co_await 5s;

	Utility::Out(L"Connected");
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
