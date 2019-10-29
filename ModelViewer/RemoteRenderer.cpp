#include "pch.h"
#include "RemoteRenderer.h"
#include <HolographicAppRemoting\Streamer.h>

using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Microsoft::Holographic::AppRemoting;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Foundation::Numerics;

RemoteRenderer::RemoteRenderer()
{
}

HolographicFrame RemoteRenderer::Update(DX::StepTimer const& timer)
{
	_holographicSpace.WaitForNextFrameReady();

	HolographicFrame holographicFrame = _holographicSpace.CreateNextFrame();
	HolographicFramePrediction prediction = holographicFrame.CurrentPrediction();

	// Back buffers can change from frame to frame. Validate each buffer, and recreate resource 
	// views and depth buffers as needed.
	_deviceResources->EnsureCameraResources(holographicFrame, prediction);

	SpatialCoordinateSystem coordinateSystem = _referenceFrame.CoordinateSystem();

	// Use the coordinate system in input calculations...

	return holographicFrame;
}

void RemoteRenderer::Render(HolographicFrame holographicFrame)
{
	bool atLeastOneCameraRendered = false;

	_deviceResources->UseHolographicCameraResources([this, holographicFrame, &atLeastOneCameraRendered](
		std::map<UINT32, std::unique_ptr<DXHelper::CameraResources>>& cameraResourceMap) {
			holographicFrame.UpdateCurrentPrediction();
			HolographicFramePrediction prediction = holographicFrame.CurrentPrediction();

			SpatialCoordinateSystem coordinateSystem = _referenceFrame.CoordinateSystem();

			for (auto cameraPose : prediction.CameraPoses())
			{
				try
				{
					DXHelper::CameraResources* pCameraResources = cameraResourceMap[cameraPose.HolographicCamera().Id()].get();

					if (pCameraResources == nullptr)
					{
						continue;
					}

					_deviceResources->UseD3DDeviceContext([&](ID3D11DeviceContext3* context) 
						{
							// Clear the back buffer view.
							context->ClearRenderTargetView(pCameraResources->GetBackBufferRenderTargetView(), DirectX::Colors::Transparent);
							context->ClearDepthStencilView(
								pCameraResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

							// The view and projection matrices for each holographic camera will change
							// every frame. This function refreshes the data in the constant buffer for
							// the holographic camera indicated by cameraPose.
							pCameraResources->UpdateViewProjectionBuffer(_deviceResources, cameraPose, coordinateSystem);

							// Set up the camera buffer.
							bool cameraActive = pCameraResources->AttachViewProjectionBuffer(_deviceResources);

							// Only render world-locked content when positional tracking is active.
							if (cameraActive)
							{
								// Set the render target, and set the depth target drawing buffer.
								ID3D11RenderTargetView* const targets[1] = { pCameraResources->GetBackBufferRenderTargetView() };
								context->OMSetRenderTargets(1, targets, pCameraResources->GetDepthStencilView());

								// Render the scene objects.
								// TODO: render the scene here...
								SceneManager::Instance().Current()->Draw(*(_context.get()), XMMatrixIdentity());

								//m_spinningCubeRenderer->Render(pCameraResources->IsRenderingStereoscopic());
								//if (m_spatialSurfaceMeshRenderer != nullptr)
								//{
								//	m_spatialSurfaceMeshRenderer->Render(pCameraResources->IsRenderingStereoscopic());
								//}
								//m_spatialInputRenderer->Render(pCameraResources->IsRenderingStereoscopic());
								//m_qrCodeRenderer->Render(pCameraResources->IsRenderingStereoscopic());
							}
						});

					atLeastOneCameraRendered = true;
				}
				catch (const winrt::hresult_error&)
				{
				}
			}
		});

	if (atLeastOneCameraRendered)
	{
		m_deviceResources->Present(holographicFrame);
	}

	if (m_swapChain == nullptr && m_isInitialized)
	{
		// A device lost event has occurred.
		// Reconnection is necessary because the holographic streamer uses the D3D device.
		// The following resources depend on the D3D device:
		//   * Holographic streamer
		//   * Renderer
		//   * Holographic space
		// The InitializeRemoteContext() function will call the functions necessary to recreate these resources.
		ShutdownRemoteContext();
		InitializeRemoteContextAndConnectOrListen();
	}

	// Determine whether or not to copy to the preview buffer.
	bool copyPreview = m_remoteContext == nullptr || m_remoteContext.ConnectionState() != ConnectionState::Connected;
	if (copyPreview && m_isInitialized)
	{
		winrt::com_ptr<ID3D11Device1> spDevice;
		spDevice.copy_from(GetDeviceResources()->GetD3DDevice());

		winrt::com_ptr<ID3D11Texture2D> spBackBuffer;
		winrt::check_hresult(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), spBackBuffer.put_void()));

		// Create a render target view of the back buffer.
		// Creating this resource is inexpensive, and is better than keeping track of
		// the back buffers in order to pre-allocate render target views for each one.
		winrt::com_ptr<ID3D11RenderTargetView> spRenderTargetView;
		winrt::check_hresult(spDevice->CreateRenderTargetView(spBackBuffer.get(), nullptr, spRenderTargetView.put()));

		GetDeviceResources()->UseD3DDeviceContext(
			[&](auto context) { context->ClearRenderTargetView(spRenderTargetView.get(), DirectX::Colors::CornflowerBlue); });

		WindowPresentSwapChain();
	}

	m_framesPerSecond++;

}

void RemoteRenderer::Initialise()
{
	CreateRemoteContext(_remoteContext, 20000, false, PreferredVideoCodec::Default);
	_holographicSpace = HolographicSpace::CreateForCoreWindow(nullptr);

	_deviceResources = std::make_shared<DXHelper::HolographicDeviceResources>();
	_deviceResources->SetHolographicSpace(_holographicSpace);
	_context = std::make_unique<SceneContext>(_deviceResources->GetD3DDeviceContext());

	_spatialLocator = SpatialLocator::GetDefault();

	_cameraAddedToken = _holographicSpace.CameraAdded({ this, &RemoteRenderer::OnCameraAdded });
	_cameraRemovedToken = _holographicSpace.CameraRemoved({ this, &RemoteRenderer::OnCameraRemoved });

	{
		_referenceFrame = _spatialLocator.CreateStationaryFrameOfReferenceAtCurrentLocation(float3::zero(), quaternion(0, 0, 0, 1), 0.0);
	}
}

void RemoteRenderer::OnCameraAdded(const HolographicSpace& sender, const HolographicSpaceCameraAddedEventArgs& args)
{
	winrt::Windows::Foundation::Deferral deferral = args.GetDeferral();
	auto holographicCamera = args.Camera();

	std::async(std::launch::async, [this, deferral, holographicCamera]() 
		{
			_deviceResources->AddHolographicCamera(holographicCamera);
			deferral.Complete();
		});
}

void RemoteRenderer::OnCameraRemoved(const HolographicSpace& sender, const HolographicSpaceCameraRemovedEventArgs& args)
{
	_deviceResources->RemoveHolographicCamera(args.Camera());
}

wchar_t *PlatformFailureToString(ConnectionFailureReason reason)
{
	switch (reason)
	{
	case ConnectionFailureReason::AuthenticationFailed:
		return L"Authentication Failed";
	case ConnectionFailureReason::Canceled:
		return L"Canceled";
	case ConnectionFailureReason::ConnectionLost:
		return L"Connection Lost";
	case ConnectionFailureReason::DeviceLost:
		return L"Device Lost";
	case ConnectionFailureReason::DisconnectRequest:
		return L"Disconnect Request";
	case ConnectionFailureReason::HandshakeConnectionFailed:
		return L"Handshake Connection Failed";
	case ConnectionFailureReason::HandshakeFailed:
		return L"Handshake Failed";
	case ConnectionFailureReason::HandshakePortBusy:
		return L"Handshake Port Busy";
	case ConnectionFailureReason::HandshakeUnreachable:
		return L"Handshake Unreachable";
	case ConnectionFailureReason::IncompatibleTransportProtocols:
		return L"Incompatible Transport Protocols";
	case ConnectionFailureReason::NoServerCertificate:
		return L"No Server Certificate";
	case ConnectionFailureReason::ProtocolError:
		return L"Protocol Error";
	case ConnectionFailureReason::ProtocolVersionMismatch:
		return L"Protocol Version Mismatch";
	case ConnectionFailureReason::RemotingVersionMismatch:
		return L"Remoting Version Mismatch";
	case ConnectionFailureReason::TransportConnectionFailed:
		return L"Transport Connection Failed";
	case ConnectionFailureReason::TransportPortBusy:
		return L"Transport Port Busy";
	case ConnectionFailureReason::TransportUnreachable:
		return L"Transport Unreachable";
	default:
		return L"Unknown Failure";
	}
}

wchar_t *ConnectionStateToString(ConnectionState state)
{
	switch (state)
	{
	case ConnectionState::Connected:
		return L"Connected";
	case ConnectionState::Disconnected:
		return L"Disconnected";
	case ConnectionState::Connecting:
		return L"Connecting";
	default:
		return L"Unknown State";
	}
}

future<void> RemoteRenderer::ConnectAsync(const wchar_t* ipAddress)
{
	// TODO: need to wire up the device lost and device restored events here...

	winrt::weak_ref<winrt::Microsoft::Holographic::AppRemoting::IRemoteContext> remoteContextWeakRef = _remoteContext;

	_onConnectedEventRevoker = _remoteContext.OnConnected(winrt::auto_revoke, [this, remoteContextWeakRef]() 
		{
			if (auto remoteContext = remoteContextWeakRef.get())
			{
				auto state = remoteContext.ConnectionState();
				_statusUpdated(ConnectionStateToString(state));
			}
		});

	_onDisconnectedEventRevoker =
	_remoteContext.OnDisconnected(winrt::auto_revoke, [this, remoteContextWeakRef, ipAddress](ConnectionFailureReason failureReason) 
		{
			if (auto remoteContext = remoteContextWeakRef.get())
			{
				Utility::Out(L"Disconnected with reason %d", failureReason);
				_statusUpdated(PlatformFailureToString(failureReason));

				// Reconnect if this is a transient failure.
				if (failureReason == ConnectionFailureReason::HandshakeUnreachable ||
					failureReason == ConnectionFailureReason::TransportUnreachable ||
					failureReason == ConnectionFailureReason::ConnectionLost)
				{
					Utility::Out(L"Reconnecting...");
					_statusUpdated(L"Reconnecting...");

					_remoteContext.Disconnect();
					_remoteContext.Connect(ipAddress, 8265);
				}
				// Failure reason None indicates a normal disconnect.
				else if (failureReason != ConnectionFailureReason::None)
				{
					Utility::Out(L"Disconnected with unrecoverable error, not attempting to reconnect.");
					_statusUpdated(L"Disconnected with unrecoverable error, not attempting to reconnect.");
				}
			}
		});

	_onListeningEventRevoker = _remoteContext.OnListening(winrt::auto_revoke, [this, remoteContextWeakRef](uint16_t port) 
		{
			if (auto remoteContext = remoteContextWeakRef.get())
			{
			}
		});

	co_await std::async(std::launch::async, [this, ipAddress]()
		{
			_remoteContext.Connect(ipAddress, 8265);
		});
}