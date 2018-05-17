#pragma once

#include <functional>

using namespace WinRTGLTFParser;
using namespace std;
using namespace Platform;

ref class EventShim sealed
{
internal:
	EventShim(function<void(GLTF_BufferData^)> bcallback,
			  function<void(GLTF_MaterialData^)> mcallback,
			  function<void(GLTF_TextureData^)> tcallback,
			  function<void(GLTF_TransformData^)> tmcallback,
			  function<void(GLTF_SceneNodeData^)> sncallback) :
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
	function<void(GLTF_BufferData^)> bufferCallback;
	function<void(GLTF_TextureData^)> textureCallback;
	function<void(GLTF_MaterialData^)> materialCallback;
	function<void(GLTF_TransformData^)> transformCallback;
	function<void(GLTF_SceneNodeData^)> sceneNodeCallback;
};