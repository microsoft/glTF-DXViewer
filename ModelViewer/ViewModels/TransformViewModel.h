#pragma once

#include "Common/ViewModelBase.h"

namespace ViewModels
{
	using namespace Windows::UI::Xaml::Data;

	[Bindable]
	public ref class TransformViewModel sealed : public ViewModelBase
	{
	internal:
		TransformViewModel(shared_ptr<GraphNode> node) :
			_selectedNode(node)
		{

		}

	public:
		property float PositionX { float get(); void set(float val); }
		property float PositionY { float get(); void set(float val); }
		property float PositionZ { float get(); void set(float val); }

		property float RotationX { float get(); void set(float val); }
		property float RotationY { float get(); void set(float val); }
		property float RotationZ { float get(); void set(float val); }

		property float ScaleX { float get(); void set(float val); }
		property float ScaleY { float get(); void set(float val); }
		property float ScaleZ { float get(); void set(float val); }

	private:
		shared_ptr<GraphNode> _selectedNode;
	};
}