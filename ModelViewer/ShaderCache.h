#pragma once
#include <map>
#include <memory>
#include "DXUtils.h"
#include "Common\DirectXHelper.h"
#include "Utility.h" 

using namespace std;
using namespace Microsoft::WRL;
using namespace DX;
using namespace Platform;

static const char *one = "1";

class ShaderDescriptor
{
public:
	ShaderDescriptor(string ShaderName, shared_ptr<DeviceResources> deviceResources) :
		_shaderName(ShaderName),
		_deviceResources(deviceResources)
	{
	}
	void AddDefine(const char *str)
	{
		_defines.push_back(str);
	}
	const vector<const char *>& Defines() const { return _defines; }
	const string& Name() const { return _shaderName; }
	shared_ptr<DeviceResources> DevResources() { return _deviceResources; }
	const shared_ptr<DeviceResources> DevResources() const { return _deviceResources; }

	int Hash();

private:
	bool _hashCalculated = false;
	vector<const char *> _defines;
	string _shaderName;
	shared_ptr<DeviceResources> _deviceResources;
	int _hash;
};

template <>
struct hash<ShaderDescriptor>
{
	size_t operator()(const ShaderDescriptor& descriptor) const
	{
		size_t res = 0;
		hash<string> myHash;
		for (auto str : descriptor.Defines())
		{
			res ^= myHash(str);
		}
		res ^= myHash(descriptor.Name());
		return res;
	}
};

template<class TShaderWrapper>
class ShaderCache
{
public:
	static ShaderCache& Instance()
	{
		static ShaderCache instance;
		return instance;
	}
	ShaderCache(ShaderCache const&) = delete;
	void operator=(ShaderCache const&) = delete;

	shared_ptr<TShaderWrapper> FindOrCreateShader(ShaderDescriptor descriptor)
	{
		// get the hash value...
		int hash = descriptor.Hash();
		map<int, shared_ptr<TShaderWrapper>>::iterator res = _shaders.find(hash);
		if (res != _shaders.end())
			return (*res).second;

		auto ret = TShaderWrapper::Create(descriptor);
		_shaders[hash] = ret;
		return ret;
	}

	static shared_ptr<D3D_SHADER_MACRO[]> GetShaderMacros(ShaderDescriptor descriptor)
	{
		// Note, remember to supply an array deleter otherwise the array pointer will not get 
		// deleted correctly
		shared_ptr<D3D_SHADER_MACRO[]> defines(new D3D_SHADER_MACRO[descriptor.Defines().size() + 1],
			default_delete<D3D_SHADER_MACRO[]>());

		int idx = 0;
		for (auto& def : descriptor.Defines())
		{
			Utility::Out(L"define %S", def);
			(defines.get())[idx].Name = def;
			(defines.get())[idx].Definition = one;
			idx++;
		}
		(defines.get())[idx].Name = nullptr;
		(defines.get())[idx].Definition = nullptr;

		return defines;
	}

private:
	ShaderCache() {}
	map<int, shared_ptr<TShaderWrapper>> _shaders;
};

class PixelShaderWrapper
{
public:
	static shared_ptr<PixelShaderWrapper> Create(ShaderDescriptor descriptor)
	{
		// Privately scoped struct which enables make_shared to be able to construct
		// a PixelShaderWrapper without making its constructor public
		struct make_shared_enabler : public PixelShaderWrapper 
		{
			make_shared_enabler(ShaderDescriptor desc) :
				PixelShaderWrapper(desc){}
		};

		auto ret = make_shared<make_shared_enabler>(descriptor);
		auto defines = ShaderCache<PixelShaderWrapper>::GetShaderMacros(descriptor);

		// Compile pixel shader shader
		ID3DBlob *psBlob = nullptr;

		// Work out the path to the shader...
		auto sf = Windows::ApplicationModel::Package::Current->InstalledLocation;
		String^ path(L"\\Assets\\Shaders\\");
		String^ filePath = sf->Path + path + "pbrpixel.hlsl";

		ThrowIfFailed(DXUtils::CompileShader(filePath->Data(), defines.get(), "main", "ps_5_0", &psBlob));
		ThrowIfFailed(descriptor.DevResources()->GetD3DDevice()->CreatePixelShader(psBlob->GetBufferPointer(), 
				psBlob->GetBufferSize(), nullptr, ret->PixelShaderAddressOf()));

		Utility::Out(L"Loaded Pixel Shader");
		return ret;
	}

	ID3D11PixelShader **PixelShaderAddressOf() { return m_pixelShader.GetAddressOf(); }
	ID3D11PixelShader *PixelShader() { return m_pixelShader.Get(); }

private:
	PixelShaderWrapper(const ShaderDescriptor& descriptor) :
		_descriptor(descriptor)
	{
	}
	const ShaderDescriptor& _descriptor;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	shared_ptr<DeviceResources> _deviceResources;
};

class VertexShaderWrapper
{
public:
	static shared_ptr<VertexShaderWrapper> Create(ShaderDescriptor descriptor)
	{
		// Privately scoped struct which enables make_shared to be able to construct
		// a PixelShaderWrapper without making its constructor public
		struct make_shared_enabler : public VertexShaderWrapper
		{
			make_shared_enabler(ShaderDescriptor desc) :
				VertexShaderWrapper(desc) {}
		};

		auto ret = make_shared<make_shared_enabler>(descriptor);
		auto defines = ShaderCache<VertexShaderWrapper>::GetShaderMacros(descriptor);

		// Compile vertex shader
		ID3DBlob *vsBlob = nullptr;

		// Work out the path to the shader...
		auto sf = Windows::ApplicationModel::Package::Current->InstalledLocation;
		String^ path(L"\\Assets\\Shaders\\");
		String^ filePath = sf->Path + path + "pbrvertex.hlsl";

		ThrowIfFailed(DXUtils::CompileShader(filePath->Data(), defines.get(), "main", "vs_5_0", &vsBlob));

		ThrowIfFailed(descriptor.DevResources()->GetD3DDevice()->CreateVertexShader(
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, ret->VertextShaderAddressOf()));

		Utility::Out(L"Loaded Vertex Shader");

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",		0,  DXGI_FORMAT_R32G32B32_FLOAT,	1,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",	0,  DXGI_FORMAT_R32G32_FLOAT,		2,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		auto uvs = std::find(descriptor.Defines().begin(), descriptor.Defines().end(), "UV");
		bool hasUVs = uvs != descriptor.Defines().end();

		auto dynamicVertexDesc = make_unique<D3D11_INPUT_ELEMENT_DESC[]>(hasUVs ? 3 : 2);
		(dynamicVertexDesc.get())[0] = vertexDesc[0];
		(dynamicVertexDesc.get())[1] = vertexDesc[1];
		if (hasUVs)
		{
			(dynamicVertexDesc.get())[2] = vertexDesc[2];
		}

		// Now create an Input layout that matches..
		ThrowIfFailed(descriptor.DevResources()->GetD3DDevice()->CreateInputLayout(dynamicVertexDesc.get(),
			hasUVs ? 3 : 2, vsBlob->GetBufferPointer(),vsBlob->GetBufferSize(), ret->InputLayoutAddressOf()));

		return ret;
	}

	ID3D11VertexShader **VertextShaderAddressOf() { return m_vertexShader.GetAddressOf(); }
	ID3D11VertexShader *VertexShader() { return m_vertexShader.Get(); }
	ID3D11InputLayout **InputLayoutAddressOf() { return m_inputLayout.GetAddressOf(); }
	ID3D11InputLayout *InputLayout() { return m_inputLayout.Get(); }

private:
	VertexShaderWrapper(const ShaderDescriptor& descriptor) :
		_descriptor(descriptor)
	{
	}
	const ShaderDescriptor& _descriptor;
	ComPtr<ID3D11InputLayout> m_inputLayout;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	unique_ptr<D3D_SHADER_MACRO[]> _defines = nullptr;
	shared_ptr<DeviceResources> _deviceResources;
};


