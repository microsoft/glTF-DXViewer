#ifndef _GLTF_PARSER_H
#define _GLTF_PARSER_H

#include <istream>

namespace WinRTGLTFParser
{
	using namespace Windows::Storage::Streams;
	using namespace Windows::Storage;
	using namespace Platform;
	using namespace std;

	String^ ToStringHat(char* ch);

	public ref class GLTF_MaterialData sealed
	{
	internal:
		GLTF_MaterialData(const MaterialData& data)
		{
			MaterialName = ToStringHat(const_cast<char *>(data.MaterialName));
			emmissiveFactor = ref new Array<float>(3);
			emmissiveFactor[0] = data.emmissiveFactor[0];
			emmissiveFactor[1] = data.emmissiveFactor[1];
			emmissiveFactor[2] = data.emmissiveFactor[2];

			baseColourFactor = ref new Array<float>(4);
			baseColourFactor[0] = data.baseColourFactor[0];
			baseColourFactor[1] = data.baseColourFactor[1];
			baseColourFactor[2] = data.baseColourFactor[2];
			baseColourFactor[3] = data.baseColourFactor[3];

			metallicFactor = data.metallicFactor;
			roughnessFactor = data.roughnessFactor;

			Pbrmetallicroughness_Basecolortexture = data.Pbrmetallicroughness_Basecolortexture;
			Pbrmetallicroughness_Metallicroughnesstexture = data.Pbrmetallicroughness_Metallicroughnesstexture;
			Normaltexture = data.Normaltexture;
			Occlusiontexture = data.Occlusiontexture;
			Emissivetexture = data.Emissivetexture;
		}

	public:
		property String^ MaterialName;
		property Array<float>^ emmissiveFactor;
		property Array<float>^ baseColourFactor;
		
		property float metallicFactor;
		property float roughnessFactor;

		property unsigned int Pbrmetallicroughness_Basecolortexture;
		property unsigned int Pbrmetallicroughness_Metallicroughnesstexture;
		property unsigned int Normaltexture;
		property unsigned int Occlusiontexture;
		property unsigned int Emissivetexture;
	};

	public ref class GLTF_TransformData sealed
	{
	internal:
		GLTF_TransformData(const TransformData& data)
		{
			hasMatrix = data.hasMatrix;

			if (hasMatrix == false)
			{
				rotation = ref new Array<float>(4);

				rotation[0] = data.rotation[0];
				rotation[1] = data.rotation[1];
				rotation[2] = data.rotation[2];
				rotation[3] = data.rotation[3];

				scale = ref new Array<float>(3);

				scale[0] = data.scale[0];
				scale[1] = data.scale[1];
				scale[2] = data.scale[2];

				translation = ref new Array<float>(3);

				translation[0] = data.translation[0];
				translation[1] = data.translation[1];
				translation[2] = data.translation[2];
			}
			else
			{
				matrix = ref new Array<float>(16);

				//matrix[0] = data.matrix[0];
				//matrix[1] = data.matrix[1];
				//matrix[2] = data.matrix[2];
				//matrix[3] = data.matrix[3];
				//matrix[4] = data.matrix[4];
				//matrix[5] = data.matrix[5];
				//matrix[6] = data.matrix[6];
				//matrix[7] = data.matrix[7];
				//matrix[8] = data.matrix[8];
				//matrix[9] = data.matrix[9];
				//matrix[10] = data.matrix[10];
				//matrix[11] = data.matrix[11];
				//matrix[12] = data.matrix[12];
				//matrix[13] = data.matrix[13];
				//matrix[14] = data.matrix[14];
				//matrix[15] = data.matrix[15];

				matrix[0] = data.matrix[0];
				matrix[1] = data.matrix[4];
				matrix[2] = data.matrix[8];
				matrix[3] = data.matrix[12];
				matrix[4] = data.matrix[1];
				matrix[5] = data.matrix[5];
				matrix[6] = data.matrix[9];
				matrix[7] = data.matrix[13];
				matrix[8] = data.matrix[2];
				matrix[9] = data.matrix[6];
				matrix[10] = data.matrix[10];
				matrix[11] = data.matrix[14];
				matrix[12] = data.matrix[3];
				matrix[13] = data.matrix[7];
				matrix[14] = data.matrix[11];
				matrix[15] = data.matrix[15];
			}
		}
	public:
		property Array<float>^ rotation;
		property Array<float>^ translation;
		property Array<float>^ scale;
		property Array<float>^ matrix;
		property bool hasMatrix;
	};

	public ref class GLTF_SceneNodeData sealed
	{
	internal:
		GLTF_SceneNodeData(const SceneNodeData& data)
		{
			Name = ToStringHat(const_cast<char *>(data.Name));
			IsMesh = data.isMesh;
			NodeIndex = data.nodeIndex;
			ParentIndex = data.parentIndex;
		}

	public:
		property String^ Name;
		property bool IsMesh;
		property int NodeIndex;
		property int ParentIndex;
	};
	
	public ref class GLTF_TextureData sealed
	{
	internal:
		GLTF_TextureData(const TextureData& data)
		{
			pSysMem = (IntPtr)(void *)data.pSysMem;
			DataSize = data.dataSize;
			ImgWidth = data.imgWidth;
			ImgHeight = data.imgHeight;
			Idx = data.idx;
			Type = data.type;
		}

	public:
		property IntPtr pSysMem;
		property size_t DataSize;
		property unsigned int ImgWidth;
		property unsigned int ImgHeight;
		property unsigned int Idx;
		property unsigned int Type;
	};

	public ref class GLTF_SubresourceData sealed
	{
	public:
		property size_t ByteWidth;
		property unsigned int BindFlags;
		property unsigned int CPUAccessFlags;
		property unsigned int MiscFlags;
		property size_t StructureByteStride;
	};

	public ref class GLTF_BufferDesc sealed
	{
	public:
		property IntPtr pSysMem;
		property unsigned int SysMemPitch;
		property unsigned int SysMemSlicePitch;
		property String^ BufferContentType;
		property size_t Count;
		property unsigned int accessorIdx;
	};

	public ref class GLTF_BufferData sealed
	{
	internal:
		GLTF_BufferData(const BufferData& data)
		{
			_subData = ref new GLTF_SubresourceData();

			_subData->ByteWidth = data.desc.ByteWidth;
			_subData->BindFlags = data.desc.BindFlags;
			_subData->CPUAccessFlags = data.desc.CPUAccessFlags;
			_subData->MiscFlags = data.desc.MiscFlags;
			_subData->StructureByteStride = data.desc.StructureByteStride;

			_bufDesc = ref new GLTF_BufferDesc();

			_bufDesc->pSysMem = (IntPtr)(void *)(data.subresource.pSysMem);
			_bufDesc->SysMemPitch = data.subresource.SysMemPitch;
			_bufDesc->SysMemSlicePitch = data.subresource.SysMemSlicePitch;
			_bufDesc->BufferContentType = ToStringHat(const_cast<char *>(data.desc.BufferContentType));
			_bufDesc->accessorIdx = data.subresource.accessorIdx;

			_bufDesc->Count = data.desc.Count;
		}

	public:
		property GLTF_SubresourceData^ SubResource 
		{
			GLTF_SubresourceData^ get() { return _subData; }
		}

		property GLTF_BufferDesc^ BufferDescription
		{
			GLTF_BufferDesc^ get() { return _bufDesc; }
		}

	private:
		GLTF_SubresourceData^ _subData;
		GLTF_BufferDesc^ _bufDesc;
	};

	public delegate void BufferEventHandler(Object^ sender, GLTF_BufferData^);
	public delegate void TextureEventHandler(Object^ sender, GLTF_TextureData^);
	public delegate void MaterialEventHandler(Object^ sender, GLTF_MaterialData^);
	public delegate void TransformEventHandler(Object^ sender, GLTF_TransformData^);
	public delegate void SceneNodeEventHandler(Object^ sender, GLTF_SceneNodeData^);

	public ref class GLTF_Parser sealed
    {
    public:
		GLTF_Parser();

		event BufferEventHandler^ OnBufferEvent;
		event TextureEventHandler^ OnTextureEvent;
		event MaterialEventHandler^ OnMaterialEvent;
		event TransformEventHandler^ OnTransformEvent;
		event SceneNodeEventHandler^ OnSceneNodeEvent;

		/// <summary>
		/// Will parse a .GLB file given the input filename
		/// Note. this would ideally be a stream interface passed in but since I would need
		/// to write the code to interface a WinRT stream and native C++ stream I am avoinding it
		/// for now. The native implementation uses std::istream. 
		/// </summary>
		/// <param name="Filename"></param>
		/// <returns></returns>
		void ParseFile(StorageFile^ storageFile);
    };
}

#endif //_GLTF_PARSER_H