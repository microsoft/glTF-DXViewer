#include "pch.h"
#include "DXGrid.h"
#include "Content\ShaderStructures.h"
#include "Common\DirectXHelper.h"

using namespace ModelViewer;
using namespace DirectX;

DXGrid::DXGrid()
{
}

void DXGrid::Initialise(ID3D11Device *device)
{
	int gridWidth = 10;
	float cellWidth = 1.0f;
	float cellHeight = 1.0f;

	unique_ptr<VertexPositionColor[]> vertices;
	unique_ptr<unsigned long[]> indices;
	int index;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	int num = (gridWidth + 1) / 2;
	
	int numInRow = (num * 2 + 1)-1;

	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = numInRow * numInRow;

	// Set the index count to the same as the vertex count.
	m_indexCount = numInRow * 4;

	// Create the vertex array.
	vertices = make_unique<VertexPositionColor[]>(m_vertexCount);

	// Create the index array.
	indices = make_unique<unsigned long[]>(m_indexCount);
	index = 0;

	for (int i = -num; i <= num; i++)
	{
		for (int j = -num; j <= num; j++)
		{
			if (i != 0 && j != 0)
			{
				vertices[index].pos = XMFLOAT3(cellWidth * i, 0.0f, cellHeight * j);
				vertices[index].color = XMFLOAT3(0.4f, 0.4f, 0.4f);
				index++;
			}
		}
	}

	index = 0;

	// first one direction...
	for (int k = 0; k < numInRow; k++)
	{
		indices[index] = k;
		index++;
		indices[index] = k + (numInRow * (numInRow - 1));
		index++;
	}

	// then the other...
	for (int k = 0; k < numInRow * numInRow; k+=numInRow)
	{
		indices[index] = k;
		index++;
		indices[index] = k + numInRow - 1;
		index++;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexPositionColor) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices.get();
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
	indexData.pSysMem = indices.get();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	DX::ThrowIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf()));

	D3D11_RASTERIZER_DESC rasterizerState;
	rasterizerState.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.DepthBias = 1;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = -1.0;
	rasterizerState.DepthClipEnable = false;
	rasterizerState.ScissorEnable = false;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = false;
	device->CreateRasterizerState(&rasterizerState, &_pRasterState);
}

void DXGrid::RenderBuffers(ID3D11DeviceContext* deviceContext)
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

