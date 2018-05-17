#pragma once
namespace ModelViewer
{
	using namespace std;

	class RenderTexture
	{
	public:
		RenderTexture(const shared_ptr<DeviceResources>& deviceResources);
		~RenderTexture();

		void Initialize(int width, int height);
		void Shutdown();

		void SetRenderTarget(ID3D11DepthStencilView *depthStencilView);
		void ClearRenderTarget(ID3D11DepthStencilView *depthStencilView, XMFLOAT4 colour);
		ID3D11ShaderResourceView *GetShaderResourceView() { return m_shaderResourceView.Get(); }
		ID3D11Texture2D *GetTexture() { return m_renderTargetTexture.Get(); }

	private:
		ComPtr<ID3D11Texture2D> m_renderTargetTexture;
		ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
		shared_ptr<DeviceResources> m_deviceResources;
	};
}
