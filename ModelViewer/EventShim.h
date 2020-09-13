#pragma once

#include <functional>

using namespace WinRTGLTFParser;
using namespace Platform;

ref class EventShim sealed
{
internal:
	EventShim(std::function<void(GLTF_BufferData^)> bcallback,
		std::function<void(GLTF_MaterialData^)> mcallback,
		std::function<void(GLTF_TextureData^)> tcallback,
		std::function<void(GLTF_TransformData^)> tmcallback,
		std::function<void(GLTF_SceneNodeData^)> sncallback) :
		bufferCallback(move(bcallback)),
		materialCallback(move(mcallback)),
		textureCallback(move(tcallback)),
		transformCallback(move(tmcallback)),
		sceneNodeCallback(move(sncallback))
	{

	}

public:
	void OnBuffer(Object^ sender, GLTF_BufferData^ data)
	{
		bufferCallback(data);
	}
	void OnTexture(Object^ sender, GLTF_TextureData^ data)
	{
		textureCallback(data);
	}
	void OnMaterial(Object^ sender, GLTF_MaterialData^ data)
	{
		materialCallback(data);
	}
	void OnTransform(Object^ sender, GLTF_TransformData^ data)
	{
		transformCallback(data);
	}
	void OnSceneNode(Object^ sender, GLTF_SceneNodeData^ data)
	{
		sceneNodeCallback(data);
	}

private:
	std::function<void(GLTF_BufferData^)> bufferCallback;
	std::function<void(GLTF_TextureData^)> textureCallback;
	std::function<void(GLTF_MaterialData^)> materialCallback;
	std::function<void(GLTF_TransformData^)> transformCallback;
	std::function<void(GLTF_SceneNodeData^)> sceneNodeCallback;
};