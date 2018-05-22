#pragma once
#include "Scene\GraphNode.h"
#include "Scene\MeshNode.h"
#include "ppltasks.h"
#include <future>
#include <experimental/resumable>
#include "Singleton.h"

using namespace Windows::Foundation;
using namespace ModelViewer;
using namespace std;
using namespace Platform;
using namespace concurrency;
using namespace WinRTGLTFParser;
using namespace Windows::Storage;

class ModelFactory : public Singleton<ModelFactory>
{
	friend class Singleton<ModelFactory>;
	
public:
	future<shared_ptr<GraphNode>> CreateFromFileAsync(StorageFile^ file);
	void CreateSceneNode(GLTF_SceneNodeData^ data);
	GraphNode *InitialiseMesh(GLTF_SceneNodeData^ data);
	void CreateBuffer(GLTF_BufferData^ data);
	void CreateTexture(GLTF_TextureData^ data);
	void CreateMaterial(GLTF_MaterialData^ data);
	void CreateTransform(GLTF_TransformData^ data);

protected:
	ModelFactory();

private:
	GLTF_Parser^ _parser;
	GraphNode *_root;
	GraphNode *_currentNode;
};

