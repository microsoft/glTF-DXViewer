#include "pch.h"
#include "NodeMaterial.h"

NodeMaterial::NodeMaterial()
{
}

NodeMaterial::~NodeMaterial()
{
}

void NodeMaterial::Initialise(GLTF_MaterialData^ data)
{
	_name = data->MaterialName->Data();
	_Emissivetexture = data->Emissivetexture;
	_Normaltexture = data->Normaltexture;
	_Occlusiontexture = data->Occlusiontexture;
	_Pbrmetallicroughness_Basecolortexture = data->Pbrmetallicroughness_Basecolortexture;
	_Pbrmetallicroughness_Metallicroughnesstexture = data->Pbrmetallicroughness_Metallicroughnesstexture;
	_emmissiveFactor.x = data->emmissiveFactor[0];
	_emmissiveFactor.y = data->emmissiveFactor[1];
	_emmissiveFactor.z = data->emmissiveFactor[2];
	_baseColorFactor.x = data->baseColourFactor[0];
	_baseColorFactor.y = data->baseColourFactor[1];
	_baseColorFactor.z = data->baseColourFactor[2];
	_baseColorFactor.w = data->baseColourFactor[3];
	_metallicFactor = data->metallicFactor;
	_roughnessFactor = data->roughnessFactor;
}

shared_ptr<TextureWrapper> NodeMaterial::HasTextureId(unsigned int idx)
{
	auto res = std::find_if(_textures.begin(), _textures.end(), [idx](pair<unsigned int, shared_ptr<TextureWrapper>> tex)
	{
		return tex.second->GetIndex() == idx ? true : false;
	});
	return res != _textures.end() ? (*res).second : nullptr;
}

void NodeMaterial::AddTexture(shared_ptr<TextureWrapper> tex)
{
	_textures[tex->Type()] = tex;
}

void NodeMaterial::AddTexture(unsigned int idx, 
							  unsigned int type,	
							  ComPtr<ID3D11Texture2D> tex, 
							  ComPtr<ID3D11ShaderResourceView> textureResourceView, 
							  ComPtr<ID3D11SamplerState> texSampler)
{
	_textures[type] = make_shared<TextureWrapper>(TextureWrapper(idx, type, tex, textureResourceView, texSampler));
}
