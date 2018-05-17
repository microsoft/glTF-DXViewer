#pragma once

#include "Content\ShaderStructures.h"
#include "Common\DirectXHelper.h"
#include "Common\DeviceResources.h"

using namespace ModelViewer;
using namespace Microsoft::WRL;
using namespace DX;

template<class T>
class ConstantBufferData
{
public:
	ConstantBufferData(unsigned int slot) :
		_slot(slot)
	{
	}

	~ConstantBufferData()
	{
		m_constantBuffer.Reset();
	}

	void Initialise(const DeviceResources& devResources)
	{
		CD3D11_BUFFER_DESC desc(sizeof(T), D3D11_BIND_CONSTANT_BUFFER);
		auto device = devResources.GetD3DDevice();
		DX::ThrowIfFailed(device->CreateBuffer(&desc, nullptr, &m_constantBuffer));
	}
	void Release() { m_constantBuffer.Reset(); }
	void Update(const DeviceResources& devResources)
	{
		if (m_constantBuffer == nullptr)
			return;

		assert(ConstantBuffer().Get());
		auto context = devResources.GetD3DDeviceContext();
		context->UpdateSubresource1(ConstantBuffer().Get(), 0, NULL, &(BufferData()), 0, 0, 0);
	}

	T& BufferData() { return _bufferData; }
	ComPtr<ID3D11Buffer> ConstantBuffer() { return m_constantBuffer; }

private:
	unsigned int _slot;
	T _bufferData;
	ComPtr<ID3D11Buffer> m_constantBuffer;
};

class BufferManager : public Singleton<BufferManager>
{
	friend class Singleton<BufferManager>;

public:
	ConstantBufferData<ModelViewProjectionConstantBuffer>& MVPBuffer()
	{
		return _mvpBuffer;
	}
	ConstantBufferData<cbPerObject>& PerObjBuffer()
	{
		return _cbPerObject;
	}
	ConstantBufferData<cbPerFrame>& PerFrameBuffer()
	{
		return _cbPerFrame;
	}

protected:
	BufferManager();

private:
	ConstantBufferData<ModelViewProjectionConstantBuffer> _mvpBuffer;
	ConstantBufferData<cbPerObject> _cbPerObject;
	ConstantBufferData<cbPerFrame> _cbPerFrame;
};

