#include "pch.h"
#include "GraphContainerNode.h"
#include "BufferManager.h"

GraphContainerNode::GraphContainerNode(int index) : 
	_index(index)
{
	CoCreateGuid(&_guid);
}

GraphContainerNode::~GraphContainerNode()
{
}

void GraphContainerNode::Update(StepTimer const& timer)
{
	for (auto child : _children)
	{
		child->Update(timer);
	}
}

XMMATRIX GraphContainerNode::PreDraw(SceneContext& context, XMMATRIX model)
{
	// Don't execute this until loaded..

	XMMATRIX mat;
	if (_hasMatrix)
	{
		mat = XMLoadFloat4x4(&_matrix);
	}
	else
	{
		mat = 
			XMMatrixTranspose(
				XMMatrixAffineTransformation(
					XMLoadFloat3(&_scale), 
					XMLoadFloat3(&emptyVector), 
					XMLoadFloat4(&_rotation), 
					XMLoadFloat3(&_translation)));
	}
	if (!XMMatrixIsIdentity(model))
	{
		mat = XMMatrixMultiply(model, mat);
	}
	model = mat;

	// Prepare to pass the updated model matrix to the shader 
	XMStoreFloat4x4(&BufferManager::Instance().MVPBuffer().BufferData().model, mat);
	BufferManager::Instance().MVPBuffer().Update(*(DevResources()));
	return mat;
}

void GraphContainerNode::Draw(SceneContext& context, XMMATRIX model)
{
	for (auto child : _children)
	{
		auto modelMatrix = child->PreDraw(context, model);
		child->Draw(context, modelMatrix);
	}
}

GraphNode *GraphContainerNode::FindChildByIndex(int index)
{
	if (Index() == index)
		return this;
	for (auto child : _children)
	{
		auto ret = child->FindChildByIndex(index);
		if (ret != nullptr)
			return ret;
	}
	return nullptr;
}

GraphNode *GraphContainerNode::FindChildById(GUID id)
{
	if (_guid == id)
		return this;
	for (auto child : _children)
	{
		return child->FindChildById(id);
	}
	return nullptr;
}

void GraphContainerNode::ForAllChildrenRecursive(function<void(GraphNode&)> func)
{
	//func(*this);
	for (auto child : _children)
	{
		func(*child);
		child->ForAllChildrenRecursive(func);
	}
}

void GraphContainerNode::ForAllChildrenRecursiveUntil(function<bool(GraphNode&)> func)
{
	auto ret = func(*this);
	if (!ret)
		return;

	for (auto child : _children)
	{
		ret = func(*child);
		if (!ret)
			return;
		child->ForAllChildrenRecursive(func);
	}
}

BoundingBox<float> GraphContainerNode::GetBoundingBox()
{
	BoundingBox<float> ret;
	for (auto child : _children)
	{
		ret.Grow(child->GetBoundingBox());
	}
	return ret;
}

void GraphContainerNode::CreateDeviceDependentResources()
{
	for (auto child : _children)
	{
		child->CreateDeviceDependentResources();
	}
}

void GraphContainerNode::Initialise(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	_deviceResources = deviceResources;
}

void GraphContainerNode::AddChild(shared_ptr<GraphNode> child)
{
	_children.push_back(child);
}

size_t GraphContainerNode::NumChildren()
{
	return _children.size();
}

shared_ptr<GraphNode> GraphContainerNode::GetChild(int i)
{
	return _children[i];
}

const wstring& GraphContainerNode::Name() const
{
	return _name;
}

void GraphContainerNode::SetName(const wstring& name)
{
	if (_name != name)
		_name = name;
}

bool GraphContainerNode::IsSelected()
{
	return _selected;
}

void GraphContainerNode::SetSelected(bool sel)
{
	if (sel == _selected)
		return;

	// Set all child nodes accordingly..
	ForAllChildrenRecursive([sel](GraphNode& node)
	{
		node.SetSelected(sel);
	});
}

void GraphContainerNode::CreateTransform(GLTF_TransformData^ data)
{
	// If we are handed a matrix, just apply that, otherwise break down into scale, rotate, translate
	// and generate the matrix from those..
	if (data->hasMatrix)
	{
		XMFLOAT4X4 mat =
		{
			data->matrix[0],
			data->matrix[1],
			data->matrix[2],
			data->matrix[3],
			data->matrix[4],
			data->matrix[5],
			data->matrix[6],
			data->matrix[7],
			data->matrix[8],
			data->matrix[9],
			data->matrix[10],
			data->matrix[11],
			data->matrix[12],
			data->matrix[13],
			data->matrix[14],
			data->matrix[15]
		};

		XMStoreFloat4x4(&BufferManager::Instance().MVPBuffer().BufferData().model, XMLoadFloat4x4(&mat));
	}
	else
	{
		_scale = { data->scale[0], data->scale[1], data->scale[2] };
		_translation = { data->translation[0], data->translation[1], data->translation[2] };

		// Using the conversion from right-handed coordinate system of OpenGL to left-handed coordinate
		// system of DirectX
		// q.x, q.y, -q.z, -q.w
		//
		_rotation = { data->rotation[0], data->rotation[1], -data->rotation[2], -data->rotation[3] };

		//XMVECTOR scale = { 1.0, 1.0, 1.0 };
		//XMVECTOR translation = { 0.0, 0.0, 0.0 };

		XMVECTOR ypr = { 0.0, 180.0, 0.0 };
		// generate a quaternion from angle for testing...
		XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(ypr);

		//auto matrix = XMMatrixRotationQuaternion(quat);

		// Create matrix from scale
		auto matrix = XMMatrixAffineTransformation(XMLoadFloat3(&_scale), XMLoadFloat3(&emptyVector), XMLoadFloat4(&_rotation), XMLoadFloat3(&_translation));

		// Prepare to pass the updated model matrix to the shader 
		XMStoreFloat4x4(&BufferManager::Instance().MVPBuffer().BufferData().model, matrix);
	}
}
