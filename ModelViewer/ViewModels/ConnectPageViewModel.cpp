#include "pch.h"
#include "ConnectPageViewModel.h"
#include "DelegateCommand.h"

using namespace ViewModels;

ConnectPageViewModel::ConnectPageViewModel()
{
	ConnectCommand = ref new DelegateCommand(ref new ExecuteDelegate(this, &ConnectPageViewModel::ExecuteConnectCommand), nullptr);
}

void ConnectPageViewModel::ExecuteConnectCommand(Object^ param)
{
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
