#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace ModelViewer
{
	using namespace std;
	using namespace DX;
	using namespace Concurrency;
	using namespace Windows::Foundation;

	class ModelViewerMain : public IDeviceNotify
	{
	public:
		ModelViewerMain(const shared_ptr<DeviceResources>& deviceResources);
		~ModelViewerMain();
		void CreateWindowSizeDependentResources();
		void StartTracking(float positionX, float positionY, VirtualKeyModifiers mod) 
		{ 
			m_sceneRenderer->StartTracking(positionX, positionY, mod);
		}
		void TrackingUpdate(float positionX, float positionY, VirtualKeyModifiers mod)
		{ 
			m_sceneRenderer->TrackingUpdate(positionX, positionY, mod);
		}
		void StopTracking(float positionX, float positionY, VirtualKeyModifiers mod) 
		{ 
			m_sceneRenderer->StopTracking(positionX, positionY, mod);
		}
		bool IsTracking() { return m_sceneRenderer->IsTracking(); }
		void StartRenderLoop();
		void StopRenderLoop();
		critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		void SetBackgroundColour(XMVECTORF32 colour) { XMStoreFloat4(&_backgroundColour, colour); }

	private:
		void ProcessInput();
		void Update();
		bool Render();

		// Cached pointer to device resources.
		shared_ptr<DeviceResources> m_deviceResources;
		shared_ptr<RemoteRenderer> m_remoteRenderer;

		// TODO: Replace with your own content renderers.
		unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		IAsyncAction^ m_renderLoopWorker;
		critical_section m_criticalSection;

		// Rendering loop timer.
		StepTimer m_timer;

		// Track current input pointer position.
		float m_pointerLocationX;
		float m_pointerLocationY;
		VirtualKeyModifiers m_mod;

		XMFLOAT4 _backgroundColour;
	};
}