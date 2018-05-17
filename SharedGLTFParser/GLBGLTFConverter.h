#pragma once

#include "stdafx.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Serialize.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <iostream>
#include <fstream>
#include <sstream>

namespace GLTFParser
{
	using namespace Microsoft::glTF;
	using namespace std;

	enum class GLTFTextureType
	{
		BaseColour = 0,
		Normal = 1,
		Emission = 2,
		Occlusion = 3,
		MetallicRoughness = 4
	};

	class GLTFFileData
	{
	public:

		class Callbacks
		{
		public:
			function<void(const BufferData&)> Buffers;
			function<void(const TextureData&)> Textures;
			function<void(const MaterialData&)> Materials;
			function<void(const TransformData&)> Transform;
			function<void(const SceneNodeData&)> SceneNode;
		};

		class ParserContext
		{
		public:
			ParserContext(const GLTFDocument& document,
				const Callbacks& callbacks,
				const GLTFResourceReader& resources) :
				_document(document),
				_callbacks(callbacks),
				_resources(resources)
			{
			}

			const GLTFDocument& document() const { return _document; }
			const Callbacks& callbacks() const { return _callbacks; }
			const GLTFResourceReader& resources() const { return _resources; }

		private:
			const GLTFDocument& _document;
			const Callbacks& _callbacks;
			const GLTFResourceReader& _resources;
		};

		const Callbacks& EventHandlers() const { return _callbacks; }
		Callbacks& EventHandlers() { return _callbacks; }

		void Read(shared_ptr<istream> file);
		void CheckExtensions(const GLTFDocument& document);
		void ParseDocument(const ParserContext& parser);

		void LoadScene(const ParserContext& parser, const Scene& scene);
		void LoadMeshNode(const ParserContext& parser, const Node& mNode);
		void LoadTransform(const ParserContext& parser, const Node& mNode);
		void LoadSceneNode(const ParserContext& parser, const Node& sceneNode, int nodeIndex, int parentIndex);
		void LoadMaterialNode(const ParserContext& parser, const Material& mNode);
		void LoadMaterialTextures(const ParserContext& parser, const Material& mNode);
		void LoadTexture(const ParserContext& parser, const Texture& texture, GLTFTextureType type);
		void LoadBufferFromAccessorId(const ParserContext& parser, const string& accessorId, 
									  const string& bufferType);
		void LoadBuffer(const ParserContext& parser, const BufferView& bufferView,
						const string& bufferType, const Accessor& accessor) const;

	private:
		GLTFFileData::Callbacks _callbacks;

		enum ComponentType
		{
			BYTE = 5120, // 1
			UNSIGNED_BYTE = 5121, // 1
			SHORT = 5122, // 2
			UNSIGNED_SHORT = 5123, // 2
			UNSIGNED_INT = 5125, // 4
			FLOAT = 5126 // 4
		};

		void PopulateDocument();
	};
};

