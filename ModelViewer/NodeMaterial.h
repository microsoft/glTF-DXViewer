#pragma once

#include <map>

using namespace Microsoft::WRL;
using namespace std;
using namespace WinRTGLTFParser;

class TextureWrapper
{
public:
	TextureWrapper(unsigned int idx,
		unsigned int type,
		ComPtr<ID3D11Texture2D> tex,
		ComPtr<ID3D11ShaderResourceView> textureResourceView,
		ComPtr<ID3D11SamplerState> texSampler) :
		_idx(idx),
		_tex(tex),
		_textureResourceView(textureResourceView),
		_textureSampler(texSampler)
	{
		_type = type;
	}

	ComPtr<ID3D11SamplerState> GetSampler() { return _textureSampler; }
	ComPtr<ID3D11ShaderResourceView> GetShaderResourceView() { return _textureResourceView; }
	ComPtr<ID3D11Texture2D> GetTexture() { return _tex; }
	unsigned int GetIndex() { return _idx; }
	unsigned int Type() { return _type; }

private:
	ComPtr<ID3D11SamplerState> _textureSampler;
	ComPtr<ID3D11ShaderResourceView> _textureResourceView;
	ComPtr<ID3D11Texture2D> _tex;
	unsigned int _idx;
	unsigned int _type;
};

class NodeMaterial
{
public:
	NodeMaterial();
	~NodeMaterial();

	void Initialise(GLTF_MaterialData^ data);
	void AddTexture(unsigned int idx, 
					unsigned int type,
					ComPtr<ID3D11Texture2D> tex, 
					ComPtr<ID3D11ShaderResourceView> textureResourceView, 
					ComPtr<ID3D11SamplerState> texSampler);
	void AddTexture(shared_ptr<TextureWrapper> tex);

	shared_ptr<TextureWrapper> GetTexture(unsigned int idx) { return _textures[idx]; }

	shared_ptr<TextureWrapper> HasTextureId(unsigned int idx);
	bool HasTexture(unsigned int idx) { return _textures.find(idx) != _textures.end(); }
	unsigned int GetNumTextures() { return _textures.size(); }

	map<unsigned int, shared_ptr<TextureWrapper>>& Textures() { return _textures; }

	XMFLOAT4& BaseColourFactor() { return _baseColorFactor; }
	void SetBaseColourFactor(XMFLOAT4 bcf) { _baseColorFactor = bcf; }

	XMFLOAT3& EmissiveFactor() { return _emmissiveFactor; }

	float MetallicFactor() { return _metallicFactor; }
	float RoughnessFactor() { return _roughnessFactor; }
	
private:
	map<unsigned int, shared_ptr<TextureWrapper>> _textures;
	wstring _name;

	XMFLOAT3 _emmissiveFactor = { 0.0f, 0.0f, 0.0f };

	unsigned int _Pbrmetallicroughness_Basecolortexture = 0;
	unsigned int _Pbrmetallicroughness_Metallicroughnesstexture = 0;
	unsigned int _Normaltexture = 0;
	unsigned int _Occlusiontexture = 0;
	unsigned int _Emissivetexture = 0;

	XMFLOAT4 _baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float _metallicFactor = 1.0f;
	float _roughnessFactor = 1.0f;
};

