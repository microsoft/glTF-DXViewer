#include "pch.h"
#include "RenderTexture.h"
#include "Common\DirectXHelper.h"

using namespace ModelViewer;
using namespace DX;

RenderTexture::RenderTexture(const shared_ptr<DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
}

RenderTexture::~RenderTexture()
{
}

void RenderTexture::Shutdown()
{
	m_renderTargetTexture.Reset();
	m_renderTargetView.Reset();
	m_shaderResourceView.Reset();
}

void RenderTexture::Initialize(int width, int height)
{
	D3D11_TEXTURE2D_DESC textureDesc;

	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	auto dev = m_deviceResources->GetD3DDevice();

	// Create the render target texture.
	ThrowIfFailed(dev->CreateTexture2D(&textureDesc, NULL, m_renderTargetTexture.GetAddressOf()));

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	ThrowIfFailed(dev->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, &m_renderTargetView));

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	ThrowIfFailed(dev->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, &m_shaderResourceView));
}

void RenderTexture::SetRenderTarget(ID3D11DepthStencilView *depthStencilView)
{
	m_deviceResources->GetD3DDeviceContext()->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
}

void RenderTexture::ClearRenderTarget(ID3D11DepthStencilView *depthStencilView, XMFLOAT4 colour)
{
	auto dc = m_deviceResources->GetD3DDeviceContext();
	float col[] = { colour.x, colour.y, colour.z, colour.w };
	dc->ClearRenderTargetView(m_renderTargetView.Get(), &col[0]);

	// Clear the depth buffer.
	dc->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
