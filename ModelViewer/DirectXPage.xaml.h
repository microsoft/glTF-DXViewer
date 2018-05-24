//
// DirectXPage.xaml.h
// Declaration of the DirectXPage class.
//

#pragma once

#include "DirectXPage.g.h"

#include "Common\DeviceResources.h"
#include "ModelViewerMain.h"
#include "SceneManager.h"

namespace ModelViewer
{
	using namespace Windows::UI::Xaml::Input;
	using namespace Windows::UI::Xaml::Controls;
	using namespace Windows::UI::Core;
	using namespace Platform;
	using namespace ViewModels;
	using namespace Windows::UI::ViewManagement;

	/// <summary>
	/// A page that hosts a DirectX SwapChainPanel.
	/// </summary>
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();
		virtual ~DirectXPage();

		void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
		void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

		property DirectXPageViewModel ^ ViewModel;

	private:
		class SceneUpdateProxy
		{
		public:
			SceneUpdateProxy(DirectXPage ^page) :
				owner(page)
			{
				SceneManager::Instance().RegisterForUpdates(bind(&SceneUpdateProxy::NotifySceneChanges, this, _1));
			}

			void NotifySceneChanges(SceneManager const& scene)
			{
				owner->NotifySceneChanges(scene);
			}

		private:
			DirectXPage ^ owner;
		};

		SceneUpdateProxy updates;
		UISettings^ _uiSettings;

		TreeViewNode^ CreateMeshNode(shared_ptr<GraphNode> node);
		TreeViewNode^ CreateContainerNode(shared_ptr<GraphNode> node);

		void NotifySceneChanges(SceneManager const& scene);
		
		TreeViewNode^ AddTreeItemsRecursive(shared_ptr<GraphNode> node, TreeViewNode^ parent);

		// Window event handlers.
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

		// DisplayInformation event handlers.
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnThemeColorChanged(UISettings^ settings, Object^ sender);

		// Other event handlers.
		void AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

		// Track our independent input on a background worker thread.
		Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
		Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;

		// Independent input handling functions.
		void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

		// Resources used to render the DirectX content in the XAML page background.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<ModelViewerMain> m_main; 
		bool m_windowVisible;
		bool m_controlPressed;
		void confirmColor_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void cancelColor_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void TreeView_ItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
	};
}

