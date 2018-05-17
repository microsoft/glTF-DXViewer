#pragma once

namespace ModelViewer
{
	using namespace Windows::UI::Xaml::Input;
	using namespace Windows::Foundation;
	using namespace Platform;

	public delegate void ExecuteDelegate(Object^ parameter);
	public delegate bool CanExecuteDelegate(Object^ parameter);

	public ref class DelegateCommand sealed : public ICommand
	{
	public:
		DelegateCommand(ExecuteDelegate^ execute, CanExecuteDelegate^ canExecute);

		virtual event EventHandler<Object^>^ CanExecuteChanged;
		virtual void Execute(Object^ parameter);
		virtual bool CanExecute(Object^ parameter);

	private:
		ExecuteDelegate ^ executeDelegate;
		CanExecuteDelegate^ canExecuteDelegate;
		bool lastCanExecute;
	};
}
