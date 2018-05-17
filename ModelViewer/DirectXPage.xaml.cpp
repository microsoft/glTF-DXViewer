//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include "SceneManager.h"
#include "FileSystemData.h"
#include <memory>
#include "Scene\MeshNode.h"
#include <numeric>

#undef max
#undef min

using namespace ModelViewer;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;
using namespace std;

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr),
	updates(this)
{
	ViewModel = ref new DirectXPageViewModel();

	InitializeComponent();

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;
	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	// Register our SwapChainPanel to get independent input pointer events
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// Register for pointer events, which will be raised on the background thread.
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

		// Begin processing input messages as they're delivered.
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// Run task on a dedicated high priority background thread.
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	m_main = std::unique_ptr<ModelViewerMain>(new ModelViewerMain(m_deviceResources));

	auto color = (Windows::UI::Color)Application::Current->Resources->Lookup("SystemAccentColor");
	float a = color.A / (float)(numeric_limits<unsigned char>::max());
	float r = color.R / (float)(numeric_limits<unsigned char>::max());
	float g = color.G / (float)(numeric_limits<unsigned char>::max());
	float b = color.B / (float)(numeric_limits<unsigned char>::max());

	XMVECTORF32 col = { r, g, b, a };
	m_main->SetBackgroundColour(col);
	
	_uiSettings = ref new UISettings();
	_uiSettings->ColorValuesChanged += ref new TypedEventHandler<UISettings^, Object^>(this, &DirectXPage::OnThemeColorChanged);

	//ViewModel->SetRenderer(m_main->Renderer());
	m_main->StartRenderLoop();
}

DirectXPage::~DirectXPage()
{
	// Stop rendering and processing events on destruction.
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

void DirectXPage::OnThemeColorChanged(UISettings^ settings, Object^ sender)
{
	Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
	{
		auto color = (Windows::UI::Color)Application::Current->Resources->Lookup("SystemAccentColor");
		float a = color.A / (float)(numeric_limits<unsigned char>::max());
		float r = color.R / (float)(numeric_limits<unsigned char>::max());
		float g = color.G / (float)(numeric_limits<unsigned char>::max());
		float b = color.B / (float)(numeric_limits<unsigned char>::max());

		XMVECTORF32 col = { r, g, b, a };
		m_main->SetBackgroundColour(col);
	}));
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// Stop rendering when the app is suspended.
	m_main->StopRenderLoop();

	// Put code to save app state here.
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Put code to load app state here.

	// Start rendering when the app is resumed.
	m_main->StartRenderLoop();
}

TreeNode^ DirectXPage::AddTreeItemsRecursive(shared_ptr<GraphNode> node, TreeNode^ parent)
{
	if (parent == nullptr)
		parent = CreateContainerNode(node);
	parent->IsExpanded = true;
	for (int i = 0; i < node->NumChildren(); i++)
	{
		auto child = node->GetChild(i);
		TreeNode^ treeNode;
		if (dynamic_cast<MeshNode *>(child.get()))
		{
			treeNode = CreateMeshNode(child);
		}
		else
		{
			treeNode = CreateContainerNode(child);
		}
		parent->Append(treeNode);
		AddTreeItemsRecursive(child, treeNode);
	}
	return parent;
}

void DirectXPage::NotifySceneChanges(SceneManager const& scene)
{
	auto scn = &scene;
	if (scn == nullptr)
		return;

	// Marshal the rest onto the ui thread...
	Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([scn, this]()
	{
		sampleTreeView->RootNode->Clear();

		auto root = scn->Current();
		auto parent = AddTreeItemsRecursive(root, nullptr);
		sampleTreeView->RootNode->Append(parent);
	}));
}

TreeNode^ DirectXPage::CreateMeshNode(shared_ptr<GraphNode> node)
{
	auto data = ref new GraphNodeData(node);
	auto treeNode = ref new TreeNode();
	treeNode->Data = data;
	return treeNode;
}

TreeNode^ DirectXPage::CreateContainerNode(shared_ptr<GraphNode> node)
{
	auto data = ref new GraphNodeData(node);
	data->IsFolder = true;
	auto treeNode = ref new TreeNode();
	treeNode->Data = data;
	return treeNode;
}
// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{ 
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

// Called when the app bar button is clicked.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// Use the app bar if it is appropriate for your app. Design the app bar, 
	// then fill in event handlers (like this one).
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	m_main->StartTracking(e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y, e->KeyModifiers);
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
	// Update the pointer tracking code.
	if (m_main->IsTracking())
	{
		m_main->TrackingUpdate(e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y, e->KeyModifiers);
	}
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	// Stop tracking pointer movement when the pointer is released.
	m_main->StopTracking(e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y, e->KeyModifiers);
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}

void ModelViewer::DirectXPage::confirmColor_Click(Object^ sender, RoutedEventArgs^ e)
{
	// Assign the selected color to a variable to use outside the popup.
	ViewModel->LightColour = myColorPicker->Color;

	// Close the Flyout.
	colorPickerButton->Flyout->Hide();
}

void ModelViewer::DirectXPage::cancelColor_Click(Object^ sender, RoutedEventArgs^ e)
{
	colorPickerButton->Flyout->Hide();
}

void ModelViewer::DirectXPage::TreeView_ItemClick(Object^ sender, ItemClickEventArgs^ e)
{
	auto item = dynamic_cast<TreeNode^>(e->ClickedItem);
	if (item == nullptr)
		return;
	auto nodeData = dynamic_cast<GraphNodeData^>(item->Data);
	if (nodeData == nullptr)
		return;
	nodeData->IsSelected = true;
}
