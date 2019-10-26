#include "pch.h"
#include "LoadingWrapper.h"

using namespace std;

LoadingWrapper::LoadingWrapper(function<void()> ctor, function<void()> dtor) :
	_dtor(dtor)
{
	Schedule(ctor);
}

future<void> LoadingWrapper::Schedule(function<void()> fn)
{
	auto disp = Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;
	co_await disp->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([fn]() { fn(); }));
}

LoadingWrapper::~LoadingWrapper()
{
	Schedule(_dtor);
}
