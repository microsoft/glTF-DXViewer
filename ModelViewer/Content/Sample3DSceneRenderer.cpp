#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"
#include "Utility.h"

#include "..\EventShim.h"
#include "SceneManager.h"
#include "BufferManager.h"

#include "DirectXPageViewModelData.h"

using namespace Windows::Storage::Streams;
using namespace Windows::Storage;
using namespace Microsoft::WRL;

using namespace ModelViewer;

using namespace DirectX;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;

void updateMathScales(string selected)
{
	float mathDiff = (selected == "mathDiff") ? 1.0f : 0.0f;
	float baseColor = (selected == "baseColor") ? 1.0f : 0.0f;
	float metallic = (selected == "metallic") ? 1.0f : 0.0f;
	float roughness = (selected == "roughness") ? 1.0f : 0.0f;

	BufferManager::Instance().PerObjBuffer().BufferData().scaleDiffBaseMR.x = mathDiff;
	BufferManager::Instance().PerObjBuffer().BufferData().scaleDiffBaseMR.y = baseColor;
	BufferManager::Instance().PerObjBuffer().BufferData().scaleDiffBaseMR.z = metallic;
	BufferManager::Instance().PerObjBuffer().BufferData().scaleDiffBaseMR.w = roughness;

	float mathF = (selected == "mathF") ? 1.0f : 0.0f;
	float mathG = (selected == "mathG") ? 1.0f : 0.0f;
	float mathD = (selected == "mathD") ? 1.0f : 0.0f;
	float mathSpec = (selected == "mathSpec") ? 1.0f : 0.0f;

	BufferManager::Instance().PerObjBuffer().BufferData().scaleFGDSpec.x = mathF;
	BufferManager::Instance().PerObjBuffer().BufferData().scaleFGDSpec.y = mathG;
	BufferManager::Instance().PerObjBuffer().BufferData().scaleFGDSpec.z = mathD;
	BufferManager::Instance().PerObjBuffer().BufferData().scaleFGDSpec.w = mathSpec;
};

future<IBuffer^> GetBuffer(StorageFolder^ imgfolder, String^ imgType, String^ side, int mipmapLevel)
{
	String^ fileName(imgType + "_" + side + "_" + mipmapLevel + ".jpg");
	Utility::Out(L"Loading file [%s\\%s]", imgfolder->Path->Data(), fileName->Data());
	auto file = co_await imgfolder->GetFileAsync(fileName);
	auto buffer = co_await FileIO::ReadBufferAsync(file);
	return buffer;
}

future<void *> LoadFileDataAsync(IBuffer^ buffer, int& fileSize)
{
	fileSize = buffer->Length;

	// Query the IBufferByteAccess interface.  
	ComPtr<IBufferByteAccess> bufferByteAccess;
	reinterpret_cast<IInspectable*>(buffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

	::byte* data = nullptr;
	bufferByteAccess->Buffer(&data);

	co_return static_cast<void *>(data);
}

#include "ImgUtils.h"

future<vector<::byte>> LoadCubeImagesAsync(StorageFolder^ imgFolder, String^ imgType, String^ side, int mipLevel, uint32_t& width, uint32_t& height)
{
	int dataSize = 0;
	auto buffer = co_await GetBuffer(imgFolder, imgType, side, mipLevel);

	auto fileData = co_await LoadFileDataAsync(buffer, dataSize);
	auto pth = imgFolder->Path + "\\" + imgType + "_" + side + "_" + mipLevel + ".jpg";

	auto bytes = ImgUtils::LoadRGBAImage(fileData, dataSize, width, height, true, pth->Data());
	co_return bytes;
}

const wchar_t *sides[] = 
{
	L"back",
	L"front",
	L"top",
	L"bottom",
	L"left",
	L"right"
};


future<Sample3DSceneRenderer::TexWrapper> Sample3DSceneRenderer::CreateCubeMapAsync(ID3D11Device3 *device, 
	StorageFolder^ imgFolder, String^ imgType, int mipLevels)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.MipLevels = mipLevels;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = texDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	vector<D3D11_SUBRESOURCE_DATA> pData(6 * mipLevels);
	uint32_t twidth = 0;
	uint32_t theight = 0;
	vector<vector<::byte>> bytes(6 * mipLevels);
	for (int j = 0; j < mipLevels; j++)
	{
		for (int i = 0; i < 6; i++)
		{
			int idx = j * 6 + i;
			Utility::Out(L"Loading cube image [%d]", idx);
			uint32_t width = 0;
			uint32_t height = 0;
			auto imgData = co_await LoadCubeImagesAsync(imgFolder, imgType, ref new String(sides[i]), j, width, height);
			bytes[idx] = imgData;
			Utility::Out(L"cube image size [%d, %d, %d]", width, height, bytes[idx].size());
			if (width > twidth)
				twidth = width;
			if (height > theight)
				theight = height;
			Utility::Out(L"Loaded cube image [%d]", idx);
			pData[idx].pSysMem = bytes[idx].data();
			pData[idx].SysMemPitch = width * 4;
			pData[idx].SysMemSlicePitch = 0;
		}
	}

	texDesc.Width = twidth;
	texDesc.Height = theight;

	ComPtr<ID3D11Texture2D> tex;
	HRESULT hr = device->CreateTexture2D(&texDesc, &pData[0], tex.GetAddressOf());
	assert(hr == S_OK);

	ComPtr<ID3D11ShaderResourceView> resourceView;
	hr = device->CreateShaderResourceView(tex.Get(), &SMViewDesc, resourceView.ReleaseAndGetAddressOf());

	assert(hr == S_OK);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MAXIMUM_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> samplerState;
	DX::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, samplerState.ReleaseAndGetAddressOf()));

	TexWrapper ret;
	ret.texResourceView = resourceView;
	ret.texSampler = samplerState;

	co_return ret;
}

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	_context = make_unique<SceneContext>(m_deviceResources->GetD3DDeviceContext());

	auto data = Container::Instance().ResolveDirectXPageViewModelData();
	data->RegisterForUpdates(bind(&Sample3DSceneRenderer::NotifyDataChanged, this, _1));

	BufferManager::Instance().MVPBuffer().BufferData().light_direction = XMFLOAT4(1.7f, 11.0f, 5.7f, 1.0f);
	BufferManager::Instance().PerFrameBuffer().BufferData().light.dir = XMFLOAT3(0.5f, 0.5f, -0.5f);
	BufferManager::Instance().PerFrameBuffer().BufferData().light.colour = XMFLOAT3(10.0f, 10.0f, 10.0f);

	// Just testing by initialising these here...
	BufferManager::Instance().PerObjBuffer().BufferData().normalScale = 1.0f;
	BufferManager::Instance().PerObjBuffer().BufferData().emissiveFactor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	BufferManager::Instance().PerObjBuffer().BufferData().occlusionStrength = 1.0f;
	BufferManager::Instance().PerObjBuffer().BufferData().metallicRoughnessValues = XMFLOAT2(1.0f, 1.0f);

	BufferManager::Instance().PerObjBuffer().BufferData().baseColorFactor.x = 1.0f;
	BufferManager::Instance().PerObjBuffer().BufferData().baseColorFactor.y = 1.0f;
	BufferManager::Instance().PerObjBuffer().BufferData().baseColorFactor.z = 1.0f;
	BufferManager::Instance().PerObjBuffer().BufferData().baseColorFactor.w = 1.0f;

	//updateMathScales("baseColor");

	BufferManager::Instance().PerObjBuffer().BufferData().scaleIBLAmbient.x = 1.0f;
	BufferManager::Instance().PerObjBuffer().BufferData().scaleIBLAmbient.y = 1.0f;
	BufferManager::Instance().PerObjBuffer().BufferData().scaleIBLAmbient.z = 1.0f;
	BufferManager::Instance().PerObjBuffer().BufferData().scaleIBLAmbient.w = 1.0f;

	// Need to find out how to set these...
	//XMFLOAT3 camera;

	SceneManager::Instance().SetDevResources(deviceResources);

	_grid = make_unique<DXGrid>();
	_grid->Initialise(deviceResources->GetD3DDevice());
	_mainAxes = make_unique<Axis>(1000.0f);
	_mainAxes->Initialise(deviceResources->GetD3DDevice());
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		1000.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&BufferManager::Instance().MVPBuffer().BufferData().projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	auto camMatrix = XMMatrixRotationRollPitchYaw(_pitch, _yaw, _roll);

	XMVECTORF32 alongZ = { _panx, _pany, _zoom };
	XMVECTORF32 at = { _panx, _pany, 0.0f, 0.0f };

	auto eye = XMVector3TransformCoord(alongZ, camMatrix);
	XMStoreFloat4x4(&BufferManager::Instance().MVPBuffer().BufferData().view, 
		XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	XMFLOAT4 eyevec;
	XMStoreFloat4(&eyevec, eye);
	BufferManager::Instance().PerObjBuffer().BufferData().camera.x = eyevec.x;
	BufferManager::Instance().PerObjBuffer().BufferData().camera.y = eyevec.y;
	BufferManager::Instance().PerObjBuffer().BufferData().camera.z = eyevec.z;
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		//float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		//double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		//float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(0.0f);
	}

	SceneManager::Instance().Current()->Update(timer);
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	//XMStoreFloat4x4(&BufferManager::Instance().MVPBuffer().BufferData().model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::StartTracking(float positionX, float positionY, VirtualKeyModifiers mod)
{
	Utility::Out(L"StartTracking [%f %f]", positionX, positionY);

	m_tracking = true;
	_lastPosY = positionY;
	_lastPosX = positionX;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX, float positionY, VirtualKeyModifiers mod)
{
	if (m_tracking)
	{
		//Utility::Out(L"TrackingUpdate [%f %f] %d", positionX, positionY, mod);

		if ((int)(mod & VirtualKeyModifiers::Control) != 0)
		{
			_zoom += (positionY - _lastPosY) / 120.0f;
		}
		else if ((int)(mod & VirtualKeyModifiers::Shift) != 0)
		{
			_panx += (positionX - _lastPosX) / 200.0f;
			_pany += (positionY - _lastPosY) / 200.0f;
		}
		else
		{
			_pitch += (positionY - _lastPosY) / 100.0f;
			_yaw += (positionX - _lastPosX) / 100.0f;
		}


		_lastPosY = positionY;
		_lastPosX = positionX;

		CreateWindowSizeDependentResources();
	}
}

void Sample3DSceneRenderer::StopTracking(float positionX, float positionY, VirtualKeyModifiers mod)
{
	Utility::Out(L"StopTracking [%f %f]", positionX, positionY);
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMStoreFloat4x4(&BufferManager::Instance().MVPBuffer().BufferData().model, XMMatrixTranspose(XMMatrixRotationY(0.0)));

	DrawAxis(context, _mainAxes.get());
	DrawGrid(context);

	context->RSSetState(_pRasterState1.Get());

	BufferManager::Instance().PerObjBuffer().Update(*m_deviceResources);
	BufferManager::Instance().PerFrameBuffer().Update(*m_deviceResources);

	// Not sure which start slot to use here - maybe we need to keep a count..
	context->PSSetShaderResources(8, 1, _envTexResourceView.GetAddressOf());
	context->PSSetSamplers(8, 1, _envTexSampler.GetAddressOf());
	context->PSSetShaderResources(9, 1, _brdfLutResourceView.GetAddressOf());
	context->PSSetSamplers(9, 1, _brdfLutSampler.GetAddressOf());
	context->PSSetShaderResources(10, 1, _envSpecularTexResourceView.GetAddressOf());
	context->PSSetSamplers(10, 1, _envSpecularTexSampler.GetAddressOf());
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, XMMatrixIdentity());
	_context->SetModel(matrix);
	SceneManager::Instance().Current()->Draw(*(_context.get()), XMMatrixIdentity());
	return;
}

void Sample3DSceneRenderer::DrawGrid(ID3D11DeviceContext2 *context)
{
	BufferManager::Instance().MVPBuffer().BufferData().color.x = 0.65f;
	BufferManager::Instance().MVPBuffer().BufferData().color.y = 0.65f;
	BufferManager::Instance().MVPBuffer().BufferData().color.z = 0.65f;

	BufferManager::Instance().MVPBuffer().Update(*m_deviceResources);

	_grid->RenderBuffers(context);

	context->IASetInputLayout(_lineDrawingInputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(_simpleVertexShader.Get(), nullptr, 0);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers(0, 1, BufferManager::Instance().MVPBuffer().ConstantBuffer().GetAddressOf());

	// Attach our pixel shader.
	context->PSSetShader(_simplePixelShader.Get(), nullptr, 0);

	// Draw the objects.
	context->DrawIndexed(_grid->IndexCount(), 0, 0);
}

void Sample3DSceneRenderer::DrawAxis(ID3D11DeviceContext2 *context, Axis *axis)
{
	BufferManager::Instance().MVPBuffer().BufferData().color.x = 0.15f;
	BufferManager::Instance().MVPBuffer().BufferData().color.y = 0.15f;
	BufferManager::Instance().MVPBuffer().BufferData().color.z = 0.15f;

	BufferManager::Instance().MVPBuffer().Update(*m_deviceResources);

	axis->RenderBuffers(context);

	context->IASetInputLayout(_lineDrawingInputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(_simpleVertexShader.Get(), nullptr, 0);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers(0, 1, BufferManager::Instance().MVPBuffer().ConstantBuffer().GetAddressOf());

	// Attach our pixel shader.
	context->PSSetShader(_simplePixelShader.Get(), nullptr, 0);

	// Draw the objects.
	context->DrawIndexed(axis->IndexCount(), 0, 0);
}

void Sample3DSceneRenderer::NotifyDataChanged(DirectXPageViewModelData const& payload)
{
	auto col1 = payload.LightScale() * payload.LightColour()[0] / 255;
	auto col2 = payload.LightScale() * payload.LightColour()[1] / 255;
	auto col3 = payload.LightScale() * payload.LightColour()[2] / 255;

	BufferManager::Instance().PerFrameBuffer().BufferData().light.colour = XMFLOAT3(col1, col2, col3);

	auto dir1 = payload.LightDirection()[0];
	auto dir2 = payload.LightDirection()[1];
	auto dir3 = payload.LightDirection()[2];
	BufferManager::Instance().PerFrameBuffer().BufferData().light.dir = XMFLOAT3(dir1, dir2, dir3);

	auto scaleF = payload.F() ? 1.0f : 0.0f;
	auto scaleG = payload.G() ? 1.0f : 0.0f;
	auto scaleD = payload.D() ? 1.0f : 0.0f;
	auto specular = payload.Specular() ? 1.0f : 0.0f;

	BufferManager::Instance().PerObjBuffer().BufferData().scaleFGDSpec = XMFLOAT4(scaleF, scaleG, scaleD, specular);

	auto scaleDiffuse = payload.Diffuse() ? 1.0f : 0.0f;
	auto scaleBasecolour = payload.BaseColour() ? 1.0f : 0.0f;
	auto scaleMetallic = payload.Metallic() ? 1.0f : 0.0f;
	auto scaleRoughness = payload.Roughness() ? 1.0f : 0.0f;

	BufferManager::Instance().PerObjBuffer().BufferData().scaleDiffBaseMR =
		XMFLOAT4(scaleDiffuse, scaleBasecolour, scaleMetallic, scaleRoughness);

	float ibl = payload.Ibl();
	BufferManager::Instance().PerObjBuffer().BufferData().scaleIBLAmbient = XMFLOAT4(ibl, ibl, 0.0f, 0.0f);
}

future<Sample3DSceneRenderer::TexWrapper> Sample3DSceneRenderer::CreateBdrfLutAsync(StorageFolder^ imgFolder)
{
	// First load the file...
	auto file = co_await imgFolder->GetFileAsync(ref new String(L"brdfLUT.png"));
	auto buffer = co_await FileIO::ReadBufferAsync(file);
	auto fileSize = buffer->Length;

	// Query the IBufferByteAccess interface.  
	ComPtr<IBufferByteAccess> bufferByteAccess;
	reinterpret_cast<IInspectable*>(buffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

	::byte* data = nullptr;
	bufferByteAccess->Buffer(&data);

	uint32_t width;
	uint32_t height;

	auto image = ImgUtils::LoadRGBAImage((void *)data, fileSize, width, height);

	D3D11_TEXTURE2D_DESC txtDesc = {};
	txtDesc.MipLevels = txtDesc.ArraySize = 1;
	txtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	txtDesc.SampleDesc.Count = 1;
	txtDesc.Usage = D3D11_USAGE_IMMUTABLE;
	txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	txtDesc.Width = width;
	txtDesc.Height = height;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = image.data();
	initialData.SysMemPitch = txtDesc.Width * sizeof(uint32_t);

	ComPtr<ID3D11Texture2D> tex;
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateTexture2D(&txtDesc, &initialData,
			tex.GetAddressOf()));

	ComPtr<ID3D11ShaderResourceView> resourceView;
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateShaderResourceView(tex.Get(),
			nullptr, resourceView.GetAddressOf()));

	// Create sampler.
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> samplerState;
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf()));

	TexWrapper ret;
	ret.texResourceView = resourceView;
	ret.texSampler = samplerState;
	co_return ret;
}

future<void> Sample3DSceneRenderer::CreateEnvironmentMapResourcesAsync(String^ envName)
{
	String^ imgType(L"diffuse");
	String^ path(L"\\Assets\\textures\\");
	String^ temp = Windows::ApplicationModel::Package::Current->InstalledLocation->Path + path + envName + "\\"  + imgType;
	auto imgFolder = co_await Windows::ApplicationModel::Package::Current->InstalledLocation->GetFolderFromPathAsync(temp);
	auto res = co_await CreateCubeMapAsync(m_deviceResources->GetD3DDevice(), imgFolder, imgType, 1);

	_envTexResourceView = res.texResourceView;
	_envTexSampler = res.texSampler;

	temp = Windows::ApplicationModel::Package::Current->InstalledLocation->Path + path;
	auto brdfFolder = co_await Windows::ApplicationModel::Package::Current->InstalledLocation->GetFolderFromPathAsync(temp);
	res = co_await CreateBdrfLutAsync(brdfFolder);

	_brdfLutResourceView = res.texResourceView;
	_brdfLutSampler = res.texSampler;

	imgType = L"specular";
	temp = Windows::ApplicationModel::Package::Current->InstalledLocation->Path + path + envName + "\\" + imgType;
	imgFolder = co_await Windows::ApplicationModel::Package::Current->InstalledLocation->GetFolderFromPathAsync(temp);

	ComPtr<ID3D11ShaderResourceView> rv;
	ComPtr<ID3D11SamplerState> ss;
	res = co_await CreateCubeMapAsync(m_deviceResources->GetD3DDevice(), imgFolder, imgType, 1);

	_envSpecularTexResourceView = res.texResourceView;
	_envSpecularTexSampler = res.texSampler;

	co_return;
}

future<void> Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// Create sampler.
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateSamplerState(&samplerDesc,
			_spSampler.ReleaseAndGetAddressOf()));

	D3D11_RASTERIZER_DESC rasterizerState;

	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.DepthBias = false;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = false;
	rasterizerState.ScissorEnable = false;
	rasterizerState.MultisampleEnable = true;
	rasterizerState.AntialiasedLineEnable = true;
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterizerState, &_pRasterState1));

	BufferManager::Instance().MVPBuffer().Initialise(*m_deviceResources);
	BufferManager::Instance().PerFrameBuffer().Initialise(*m_deviceResources);
	BufferManager::Instance().PerObjBuffer().Initialise(*m_deviceResources);

	// Load shaders asynchronously for line rendering...
	auto loadVSTask2 = DX::ReadDataAsync(L"SimpleVertexShader.cso");
	auto loadPSTask2 = DX::ReadDataAsync(L"SimplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask2 = loadVSTask2.then([this](const std::vector<::byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&_simpleVertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC SimpleVertexDesc[] =
		{
			{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",		0,  DXGI_FORMAT_R32G32B32_FLOAT,	1,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				SimpleVertexDesc,
				2/*ARRAYSIZE(SimpleVertexDesc)*/,
				&fileData[0],
				fileData.size(),
				&_lineDrawingInputLayout
			)
		);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask2 = loadPSTask2.then([this](const std::vector<::byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&_simplePixelShader
			)
		);

		CD3D11_BUFFER_DESC lineDrawingConstantBufferDesc(sizeof(LineDrawingConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&lineDrawingConstantBufferDesc,
				nullptr,
				&_lineDrawingConstantBuffer
			)
		);
	});

	co_await CreateEnvironmentMapResourcesAsync(ref new String(L"papermill"));

	(createPSTask2 && createVSTask2).then([this]()
	{
		m_loadingComplete = true;
	});

	co_return;
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();

	BufferManager::Instance().MVPBuffer().Release();
	BufferManager::Instance().PerFrameBuffer().Release();
	BufferManager::Instance().PerObjBuffer().Release();

	_lineDrawingConstantBuffer.Reset();

	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();

	for (auto& buffer : _buffers)
	{
		buffer.second.Buffer().Reset();
	}

	_buffers.clear();
	_spTexture.Reset();
}