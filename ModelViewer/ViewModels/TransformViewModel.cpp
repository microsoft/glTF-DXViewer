#include "pch.h"
#include "TransformViewModel.h"

using namespace ViewModels;

float TransformViewModel::PositionX::get()
{
	if (_selectedNode)
	{
		shared_ptr<GraphContainerNode> derived =
			dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
		return derived->GetTranslation().x;
	}
	return 0.0f;
}

void TransformViewModel::PositionX::set(float val)
{
	shared_ptr<GraphContainerNode> derived =
		dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
	derived->SetTranslation(val, derived->GetTranslation().y, derived->GetTranslation().z);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

float TransformViewModel::PositionY::get()
{
	if (_selectedNode)
	{
		shared_ptr<GraphContainerNode> derived =
			dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
		return derived->GetTranslation().y;
	}
	return 0.0f;
}

void TransformViewModel::PositionY::set(float val)
{
	shared_ptr<GraphContainerNode> derived =
		dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
	derived->SetTranslation(derived->GetTranslation().x, val, derived->GetTranslation().z);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

float TransformViewModel::PositionZ::get()
{
	if (_selectedNode)
	{
		shared_ptr<GraphContainerNode> derived =
			dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
		return derived->GetTranslation().z;
	}
	return 0.0f;
}

void TransformViewModel::PositionZ::set(float val)
{
	shared_ptr<GraphContainerNode> derived =
		dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
	derived->SetTranslation(derived->GetTranslation().x, derived->GetTranslation().y, val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

float TransformViewModel::RotationX::get()
{
	if (_selectedNode)
	{
		shared_ptr<GraphContainerNode> derived =
			dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
		//return derived->GetRotationX();
		return derived->GetRotation().x;
	}
	return 0.0f;
}

void TransformViewModel::RotationX::set(float val)
{
	shared_ptr<GraphContainerNode> derived =
		dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
	derived->SetRotationRoll(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

float TransformViewModel::RotationY::get()
{
	if (_selectedNode)
	{
		shared_ptr<GraphContainerNode> derived =
			dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
		//return derived->GetRotationY();
		return derived->GetRotation().y;
	}
	return 0.0f;
}

void TransformViewModel::RotationY::set(float val)
{
	shared_ptr<GraphContainerNode> derived =
		dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
	derived->SetRotationPitch(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

float TransformViewModel::RotationZ::get()
{
	if (_selectedNode)
	{
		shared_ptr<GraphContainerNode> derived =
			dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
		//return derived->GetRotationZ();
		return derived->GetRotation().z;
	}
	return 0.0f;
}

void TransformViewModel::RotationZ::set(float val)
{
	shared_ptr<GraphContainerNode> derived =
		dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
	derived->SetRotationYaw(val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

float TransformViewModel::ScaleX::get()
{
	if (_selectedNode)
	{
		shared_ptr<GraphContainerNode> derived =
			dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
		return derived->GetScale().x;
	}
	return 0.0f;
}

void TransformViewModel::ScaleX::set(float val)
{
	shared_ptr<GraphContainerNode> derived =
		dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
	derived->SetScale(val, derived->GetScale().y, derived->GetScale().z);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

float TransformViewModel::ScaleY::get()
{
	if (_selectedNode)
	{
		shared_ptr<GraphContainerNode> derived =
			dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
		return derived->GetScale().y;
	}
	return 0.0f;
}

void TransformViewModel::ScaleY::set(float val)
{
	shared_ptr<GraphContainerNode> derived =
		dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
	derived->SetScale(derived->GetScale().x, val, derived->GetScale().z);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}

float TransformViewModel::ScaleZ::get()
{
	if (_selectedNode)
	{
		shared_ptr<GraphContainerNode> derived =
			dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
		return derived->GetScale().z;
	}
	return 0.0f;
}

void TransformViewModel::ScaleZ::set(float val)
{
	shared_ptr<GraphContainerNode> derived =
		dynamic_pointer_cast<GraphContainerNode>(_selectedNode);
	derived->SetScale(derived->GetScale().x, derived->GetScale().y, val);
	OnPropertyChanged(getCallerName(__FUNCTION__));
}
