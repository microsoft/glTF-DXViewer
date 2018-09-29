#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
//#include "../SharedGLTFParser/gltfparser.h"
#include "DXGrid.h"
#include "Axis.h"
#include "./Scene/GraphContainerNode.h"

namespace ModelViewer
{
	using namespace WinRTGLTFParser;
	//using namespace std;
	using namespace Microsoft::WRL;
	using namespace Windows::System;
	using namespace Platform;
	using namespace Windows::Storage;
	using namespace DX;

	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DeviceResources>& deviceResources);

		class TexWrapper
		{
		public:
			ComPtr<ID3D11ShaderResourceView> texResourceView;
			ComPtr<ID3D11SamplerState> texSampler;
		};

		std::future<void> CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		std::future<void> CreateEnvironmentMapResourcesAsync(String^ envName);
		std::future<TexWrapper> CreateCubeMapAsync(ID3D11Device3 *device, StorageFolder^ imgFolder, String^ imgType, int mipLevel);
		std::future<TexWrapper> CreateBdrfLutAsync(StorageFolder^ imgFolder);
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking(float positionX, float positionY, VirtualKeyModifiers mod);
		void TrackingUpdate(float positionX, float positionY, VirtualKeyModifiers mod);
		void StopTracking(float positionX, float positionY, VirtualKeyModifiers mod);
		bool IsTracking() { return m_tracking; }
		void NotifyDataChanged(DirectXPageViewModelData const& data);

	private:
		void Rotate(float radians);

	private:
		class BufferWrapper
		{
		public:
			BufferWrapper(GLTF_BufferData^ data, ComPtr<ID3D11Buffer> buffer) :
				_data(data),
				_buffer(buffer)
			{
			}
			BufferWrapper() {}
			ComPtr<ID3D11Buffer>& Buffer() { return _buffer; }

			GLTF_BufferData^ Data() { return _data; }

		private:
			GLTF_BufferData^ _data;
			ComPtr<ID3D11Buffer> _buffer;
		};

		void DrawGrid(ID3D11DeviceContext2 *context);
		void DrawAxis(ID3D11DeviceContext2 *context, Axis *axis);

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		ComPtr<ID3D11InputLayout>	m_inputLayout;
		ComPtr<ID3D11Buffer>		m_vertexBuffer;
		ComPtr<ID3D11Buffer>		m_indexBuffer;
		ComPtr<ID3D11VertexShader>	m_vertexShader;
		ComPtr<ID3D11PixelShader>	m_pixelShader;

		ComPtr<ID3D11Buffer>		_lineDrawingConstantBuffer;
		ComPtr<ID3D11InputLayout>	_lineDrawingInputLayout;
		ComPtr<ID3D11VertexShader>	_simpleVertexShader;
		ComPtr<ID3D11PixelShader>	_simplePixelShader;

		ComPtr<ID3D11ShaderResourceView> _envTexResourceView;
		ComPtr<ID3D11SamplerState> _envTexSampler;

		ComPtr<ID3D11ShaderResourceView> _envSpecularTexResourceView;
		ComPtr<ID3D11SamplerState> _envSpecularTexSampler;

		ComPtr<ID3D11SamplerState> _brdfLutSampler;
		ComPtr<ID3D11ShaderResourceView> _brdfLutResourceView;

		ComPtr<ID3D11SamplerState>          _spSampler;
		ComPtr<ID3D11ShaderResourceView>    _spTexture;

		std::map<std::wstring, BufferWrapper> _buffers;

		// System resources for cube geometry.
		//LineDrawingConstantBuffer _lineDrawingConstantBufferData;

		uint32	m_indexCount;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		float _lastPosX = 0.0f;
		float _lastPosY = 0.0f;

		float _yaw = -0.05f;
		float _pitch = -0.2f;
		float _roll = 0.0f;
		float _panx = 0.0f;
		float _pany = 0.0f;
		float _zoom = 5.0f;

		std::unique_ptr<DXGrid> _grid;
		std::unique_ptr<Axis> _mainAxes;

		ComPtr<ID3D11RasterizerState> _pRasterState1;
		std::unique_ptr<SceneContext> _context;
	};
}

