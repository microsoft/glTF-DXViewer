#include "pch.h"
#include "ModelFactory.h"
#include "EventShim.h"
#include "SceneManager.h"

using namespace std;

ModelFactory::ModelFactory()
{
}

void ModelFactory::CreateBuffer(GLTF_BufferData^ data)
{
	auto node = dynamic_cast<MeshNode *>(_currentNode);
	if (node)
	{
		node->CreateBuffer(data);
	}
}
void ModelFactory::CreateTexture(GLTF_TextureData^ data)
{
	auto node = dynamic_cast<MeshNode *>(_currentNode);
	if (node)
	{
		node->CreateTexture(data);
	}
}
void ModelFactory::CreateMaterial(GLTF_MaterialData^ data)
{
	auto node = dynamic_cast<MeshNode *>(_currentNode);
	if (node)
	{
		node->CreateMaterial(data);
	}
}
void ModelFactory::CreateTransform(GLTF_TransformData^ data)
{
	auto node = dynamic_cast<GraphContainerNode *>(_currentNode);
	if (node)
	{
		node->CreateTransform(data);
	}
}

GraphNode * ModelFactory::InitialiseMesh(GLTF_SceneNodeData^ data)
{
	auto mesh = new MeshNode(data->NodeIndex);
	_currentNode = mesh;

	auto devResources = SceneManager::Instance().DevResources();
	mesh->Initialise(devResources);

	return mesh;
}

void ModelFactory::CreateSceneNode(GLTF_SceneNodeData^ data)
{
	GraphNode *parent = nullptr;
	if (_root && data->ParentIndex != -1)
	{
		parent = _root->FindChildByIndex(data->ParentIndex);
	}

	if (data->IsMesh)
	{
		_currentNode = InitialiseMesh(data);
	}
	else
	{
		_currentNode = new GraphContainerNode(data->NodeIndex);
		auto devResources = SceneManager::Instance().DevResources();
		_currentNode->Initialise(devResources);
	}
	_currentNode->SetName(data->Name->Data());

	if (_root == nullptr)
	{
		_root = _currentNode;
	}
	else if (parent && _currentNode)
	{
		shared_ptr<GraphNode> sp;
		sp.reset(_currentNode);
		parent->AddChild(sp);
	}
}

future<shared_ptr<GraphNode>> ModelFactory::CreateFromFileAsync(StorageFile^ file)
{
	_parser = ref new GLTF_Parser();
	_root = _currentNode = nullptr;

	function<void(GLTF_SceneNodeData^)> snmmemfun = bind(&ModelFactory::CreateSceneNode, &(ModelFactory::Instance()), placeholders::_1);
	function<void(GLTF_BufferData^)> memfun = bind(&ModelFactory::CreateBuffer, &(ModelFactory::Instance()), placeholders::_1);
	function<void(GLTF_TextureData^)> tmemfun = bind(&ModelFactory::CreateTexture, &(ModelFactory::Instance()), placeholders::_1);
	function<void(GLTF_MaterialData^)> mmemfun = bind(&ModelFactory::CreateMaterial, &(ModelFactory::Instance()), placeholders::_1);
	function<void(GLTF_TransformData^)> tmmemfun = bind(&ModelFactory::CreateTransform, &(ModelFactory::Instance()), placeholders::_1);

	auto es = ref new EventShim(memfun, mmemfun, tmemfun, tmmemfun, snmmemfun);

	_parser->OnBufferEvent += ref new BufferEventHandler(es, &EventShim::OnBuffer);
	_parser->OnTextureEvent += ref new TextureEventHandler(es, &EventShim::OnTexture);
	_parser->OnMaterialEvent += ref new MaterialEventHandler(es, &EventShim::OnMaterial);
	_parser->OnTransformEvent += ref new TransformEventHandler(es, &EventShim::OnTransform);
	_parser->OnSceneNodeEvent += ref new SceneNodeEventHandler(es, &EventShim::OnSceneNode);

	co_await async([this, file]() { _parser->ParseFile(file); });

	_root->AfterLoad();

	// call afterLoad on all children...
	_root->ForAllChildrenRecursive([](GraphNode& node) 
	{
		node.AfterLoad();
	});

	shared_ptr<GraphNode> sp;
	sp.reset(_root);
	co_return sp;
}