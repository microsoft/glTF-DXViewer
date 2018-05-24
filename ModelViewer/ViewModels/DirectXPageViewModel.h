#pragma once

#include "Common/ViewModelBase.h"
#include "DirectXPageViewModelData.h"
#include "Content\Sample3DSceneRenderer.h"
#include <memory>
#include "Container.h"
#include "TransformViewModel.h"
#include "../SceneManager.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace ViewModels
{
	using namespace Common;
	using namespace Windows::UI;
	using namespace Windows::UI::Xaml::Data;
	using namespace ModelViewer;

	[Bindable]
	public ref class DirectXPageViewModel sealed : public ViewModelBase
	{
	public:
		DirectXPageViewModel();

		property float LightScale { float get(); void set(float val); }
		property float LightRotation { float get(); void set(float val); }
		property float LightPitch { float get(); void set(float val); }
		property float Ibl { float get(); void set(float val); }
		property Color LightColour { Color get(); void set(Color val); }
		property bool BaseColour { bool get(); void set(bool val); }
		property bool Metallic { bool get(); void set(bool val); }
		property bool Roughness { bool get(); void set(bool val); }
		property bool Diffuse { bool get(); void set(bool val); }
		property bool Specular { bool get(); void set(bool val); }
		property bool F { bool get(); void set(bool val); }
		property bool G { bool get(); void set(bool val); }
		property bool D { bool get(); void set(bool val); }
		property TransformViewModel^ SelectedTransform { TransformViewModel^ get(); void set(TransformViewModel^ val); }

	internal:
		void NotifySelectionChanged(shared_ptr<GraphNode> node);

		class SelectionChangedProxy
		{
		public:
			SelectionChangedProxy(DirectXPageViewModel^ owner) :
				_owner(owner)
			{
				SceneManager::Instance().RegisterForSelectionChanged(bind(&SelectionChangedProxy::NotifySelectionChanged, this, _1));
			}

			void NotifySelectionChanged(shared_ptr<GraphNode> node)
			{
				_owner->NotifySelectionChanged(node);
			}

		private:
			DirectXPageViewModel ^ _owner;
		};

	private:
		Color ConvertColor(const unsigned char col[3]);
		float *ConvertDirection(float rotation, float pitch, float *data);

		shared_ptr<DirectXPageViewModelData> _data;
		TransformViewModel^ _selectedTransform;
		SelectionChangedProxy _proxy;
	};

}

