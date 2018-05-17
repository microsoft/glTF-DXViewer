#include "pch.h"
#include "RootPageViewModel.h"
#include "DelegateCommand.h"
#include "SceneManager.h"
#include "Utility.h"
#include <experimental/resumable>
#include <pplawait.h>

using namespace ViewModels;
using namespace Windows::Storage::Pickers;

RootPageViewModel::RootPageViewModel()
{
	LoadFileCommand = ref new DelegateCommand(ref new ExecuteDelegate(this, &RootPageViewModel::ExecuteLoadCommand), nullptr);
}

class LoadingWrapper
{
public:
	LoadingWrapper(function<void()> ctor, function<void()> dtor) :
		_dtor(dtor)
	{
		Schedule(ctor);
	}

	future<void> Schedule(function<void()> fn)
	{
		auto disp = Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;
		co_await disp->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
			ref new Windows::UI::Core::DispatchedHandler([fn]() { fn(); }));
	}

	~LoadingWrapper()
	{
		Schedule(_dtor);
	}

private:
	function<void()> _dtor;
};

future<shared_ptr<GraphNode>> RootPageViewModel::LoadFileAsync()
{
	auto fop = ref new FileOpenPicker();
	fop->FileTypeFilter->Append(".glb");
	
	// The code flow supports gltf files but unless we copy all of the loose files over 
	// to a location that the native C++ environment can open them from then we will just
	// get access denied.
	//fop->FileTypeFilter->Append(".gltf");

	auto file = co_await fop->PickSingleFileAsync();
	if (file == nullptr)
		co_return nullptr;

	// RAII-style for ensuring that the progress gets cleared robustly
	auto loader = make_unique<LoadingWrapper>([this]() { Loading = true; }, [this]() { Loading = false; });

	Utility::Out(L"filename = %s", file->Path->Data());
	Filename = file->Path;

	// Since we don't have access to open a file in native code I'll take a copy of the file here
	// and access it from the application's temp folder. Another option might be to implement a streambuf
	// which streams data from a Winrt stream but since this is just a sample that seems quite high effort.
	// A knock-on effect from this is that GLTF files won't load (only GLB) since the files referenced by the
	// GLTF file i.e. .bin, .jpg, etc. won't have also been copied across..
	//
	auto tempFolder = Windows::Storage::ApplicationData::Current->TemporaryFolder;
	auto tempFile = co_await file->CopyAsync(tempFolder, file->Name, NameCollisionOption::GenerateUniqueName);

	Utility::Out(L"temp file path = %s", tempFile->Path->Data());
	auto ret = co_await ModelFactory::Instance().CreateFromFileAsync(tempFile->Path);
	co_return ret;
}

future<void> RootPageViewModel::Load()
{
	Utility::Out(L"At Start of Load");
	auto node = co_await LoadFileAsync();
	if (node == nullptr)
		co_return;

	Utility::Out(L"Loaded");

	// Add the GraphNode to the scene	
	auto current = SceneManager::Instance().Current();
	SceneManager::Instance().AddNode(node);
}

void RootPageViewModel::ExecuteLoadCommand(Object^ param)
{
	Load();
}

bool RootPageViewModel::Loading::get()
{
	return _loading;
}

void RootPageViewModel::Loading::set(bool val)
{
	if (_loading == val)
		return;
	_loading = val;
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

String^ RootPageViewModel::Filename::get()
{
	return _filename;
}

void RootPageViewModel::Filename::set(String^ val)
{
	if (_filename == val)
		return;
	_filename = val;
	OnPropertyChanged(getCallerName(__FUNCTION__));
}
