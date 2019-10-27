#pragma once

#include "Common/ViewModelBase.h"
#include <future>
#include "ModelFactory.h"

namespace ViewModels
{
	using namespace Common;
	using namespace Windows::UI::Xaml::Input;
	using namespace Platform;

	[Windows::UI::Xaml::Data::Bindable]
	public ref class RootPageViewModel sealed : public ViewModelBase
	{
	public:
		RootPageViewModel();

		property bool Loading { bool get(); void set(bool val); }
		property ICommand^ LoadFileCommand;
		property String^ Filename { String^ get(); void set(String^ val); }

	private:
		void ExecuteLoadCommand(Object^ param);
		std::future<std::shared_ptr<GraphNode>> RootPageViewModel::LoadFileAsync();
		std::future<void> RootPageViewModel::Load();

		bool _loading = false;
		String^ _filename;
	};
}
