#include "pch.h"
#include "Axis.h"
#include "Content\ShaderStructures.h"
#include "Common\DirectXHelper.h"

using namespace ModelViewer;
using namespace DirectX;

Axis::Axis(float axisLength) : _axisLength(axisLength)
{
}

void Axis::Initialise(ID3D11Device *device)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	XMFLOAT3 colour = { 1.0f, 1.0f, 1.0f };
	static const VertexPositionColor vertices [] =
	{
		{ XMFLOAT3(0.0f, _axisLength, 0.0f), colour },
		{ XMFLOAT3(0.0f, -_axisLength, 0.0f), colour },
		{ XMFLOAT3(-_axisLength, 0.0f, 0.0f), colour },
		{ XMFLOAT3(_axisLength, 0.0f, 0.0f), colour },
		{ XMFLOAT3(0.0f, 0.0f, -_axisLength), colour },
		{ XMFLOAT3(0.0f, 0.0f, _axisLength), colour },
	};
	m_vertexCount = 6;

	static const unsigned long axisIndices [] =
	{
		0, 1, 2, 3, 4, 5
	};
	m_indexCount = 6;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexPositionColor) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	DX::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.GetAddressOf()));

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = axisIndices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	DX::ThrowIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf()));

	D3D11_RASTERIZER_DESC rasterizerState;	
	rasterizerState.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.DepthBias = 10;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0.0;
	rasterizerState.DepthClipEnable = false;
	rasterizerState.ScissorEnable = false;
	rasterizerState.MultisampleEnable = true;
	rasterizerState.AntialiasedLineEnable = true;
	DX::ThrowIfFailed(device->CreateRasterizerState(&rasterizerState, &_pRasterState));
}

void Axis::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexPositionColor);
	offset = 0;

	auto vb = m_vertexBuffer.Get();

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//Set the render format to line list.
	// Set the type of primitive that should be rendered from this vertex buffer, in this case a line list.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	deviceContext->RSSetState(_pRasterState.Get());
}

