#pragma once

#include <GLTFSDK\IStreamReader.h>
#include <iostream>
#include <functional>
#include <memory>

using namespace std;

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GLTFPARSER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GLTFPARSER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GLTFPARSER_EXPORTS
#define GLTFPARSER_API __declspec(dllexport)
#else
#define GLTFPARSER_API __declspec(dllimport)
#endif

typedef long HRESULT;

class GLTFPARSER_API MaterialData
{
public:
	MaterialData() {}

	const char *MaterialName;
	float emmissiveFactor[3] = { 0.0f, 0.0f, 0.0f };
	float baseColourFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float metallicFactor = 1.0f;
	float roughnessFactor = 1.0f;

	int Pbrmetallicroughness_Basecolortexture;
	int Pbrmetallicroughness_Metallicroughnesstexture;
	int Normaltexture;
	int Occlusiontexture;
	int Emissivetexture;
};

class GLTFPARSER_API TextureData
{
public:
	TextureData() {}

	const void *pSysMem;
	unsigned int dataSize;
	unsigned int imgWidth;
	unsigned int imgHeight;
	unsigned int idx;
	unsigned int type;
};

class GLTFPARSER_API TransformData
{
public:
	TransformData() 
	{
		rotation[0] = rotation[1] = rotation[2] = rotation[3] = 0.0;
		translation[0] = translation[1] = translation[2] = 0.0;
		scale[0] = scale[1] = scale[2] = 1.0;
	}

	double rotation[4];
	double translation[3];
	double scale[3];
	double matrix[16];

	bool hasMatrix;
};

class GLTFPARSER_API SceneNodeData
{
public:
	SceneNodeData(){}

	const char *Name;
	bool isMesh;
	int nodeIndex;
	int parentIndex;
};

struct GLTFPARSER_API SubresourceData
{
	const void *pSysMem;
	unsigned int SysMemPitch;
	unsigned int SysMemSlicePitch;
	unsigned int accessorIdx;
};

struct GLTFPARSER_API BufferDesc
{
	unsigned int ByteWidth;
	unsigned int BindFlags;
	unsigned int CPUAccessFlags;
	unsigned int MiscFlags;
	unsigned int StructureByteStride;
	const char *BufferContentType;
	unsigned int Count;
};

class GLTFPARSER_API BufferData
{
public:
	BufferData();
	SubresourceData subresource;
	BufferDesc desc;
};

HRESULT GLTFPARSER_API ParseFile(shared_ptr<istream> inStr, 
	const string& baseUri,
	Microsoft::glTF::IStreamReader& gltfStreamReader,
	function<void(const BufferData&)> bufferCallback,
	function<void(const MaterialData&)> materialCallback,
	function<void(const TextureData&)> textureCallback,
	function<void(const TransformData&)> transformCallback,
	function<void(const SceneNodeData&)> sceneNodeCallback);
