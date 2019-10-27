//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "CameraResources.h"
#include "IDeviceNotify.h"

#include <d2d1_2.h>
#include <d3d11_4.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <wrl/client.h>

namespace DXHelper
{
    // Creates and manages a Direct3D device and immediate context, Direct2D device and context (for debug), and the holographic swap chain.
    class HolographicDeviceResources
    {
    public:
        HolographicDeviceResources();
        ~HolographicDeviceResources();

        // Public methods related to Direct3D devices.
        void HandleDeviceLost();
        void RegisterDeviceNotify(IDeviceNotify* deviceNotify);
        void Trim();
        void Present(winrt::Windows::Graphics::Holographic::HolographicFrame frame);

        // Public methods related to holographic devices.
        void SetHolographicSpace(winrt::Windows::Graphics::Holographic::HolographicSpace space);
        void EnsureCameraResources(
            winrt::Windows::Graphics::Holographic::HolographicFrame frame,
            winrt::Windows::Graphics::Holographic::HolographicFramePrediction prediction);

        void AddHolographicCamera(winrt::Windows::Graphics::Holographic::HolographicCamera camera);
        void RemoveHolographicCamera(winrt::Windows::Graphics::Holographic::HolographicCamera camera);

        // Holographic accessors.
        template <typename LCallback>
        void UseHolographicCameraResources(LCallback const& callback);

        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice GetD3DInteropDevice() const
        {
            return m_d3dInteropDevice;
        }

        // D3D accessors.
        ID3D11Device4* GetD3DDevice() const
        {
            return m_d3dDevice.get();
        }
        template <typename F>
        auto UseD3DDeviceContext(F func) const
        {
            std::scoped_lock lock(m_d3dContextMutex);
            return func(m_d3dContext.get());
        }
        D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const
        {
            return m_d3dFeatureLevel;
        }
        bool GetDeviceSupportsVprt() const
        {
            return m_supportsVprt;
        }

        // DXGI acessors.
        IDXGIAdapter3* GetDXGIAdapter() const
        {
            return m_dxgiAdapter.get();
        }

        // D2D accessors.
        ID2D1Factory2* GetD2DFactory() const
        {
            return m_d2dFactory.get();
        }
        IDWriteFactory2* GetDWriteFactory() const
        {
            return m_dwriteFactory.get();
        }
        IWICImagingFactory2* GetWicImagingFactory() const
        {
            return m_wicFactory.get();
        }

    protected:
        void CreateDeviceResources();

    private:
        // Private methods related to the Direct3D device, and resources based on that device.
        void CreateDeviceIndependentResources();
        void InitializeUsingHolographicSpace();

    protected:
        // Direct3D objects.
        winrt::com_ptr<ID3D11Device4> m_d3dDevice;
        mutable std::recursive_mutex m_d3dContextMutex;
        winrt::com_ptr<ID3D11DeviceContext3> m_d3dContext;
        winrt::com_ptr<IDXGIAdapter3> m_dxgiAdapter;

        // Direct3D interop objects.
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_d3dInteropDevice;

        // Direct2D factories.
        winrt::com_ptr<ID2D1Factory2> m_d2dFactory;
        winrt::com_ptr<IDWriteFactory2> m_dwriteFactory;
        winrt::com_ptr<IWICImagingFactory2> m_wicFactory;

        // The IDeviceNotify can be held directly as it owns the DeviceResources.
        IDeviceNotify* m_deviceNotify = nullptr;

        // Whether or not the current Direct3D device supports the optional feature
        // for setting the render target array index from the vertex shader stage.
        bool m_supportsVprt = false;

    private:
        // The holographic space provides a preferred DXGI adapter ID.
        winrt::Windows::Graphics::Holographic::HolographicSpace m_holographicSpace = nullptr;

        // Properties of the Direct3D device currently in use.
        D3D_FEATURE_LEVEL m_d3dFeatureLevel = D3D_FEATURE_LEVEL_10_0;

        // Back buffer resources, etc. for attached holographic cameras.
        std::map<UINT32, std::unique_ptr<CameraResources>> m_cameraResources;
        std::mutex m_cameraResourcesLock;
    };
} // namespace DXHelper

// Device-based resources for holographic cameras are stored in a std::map. Access this list by providing a
// callback to this function, and the std::map will be guarded from add and remove
// events until the callback returns. The callback is processed immediately and must
// not contain any nested calls to UseHolographicCameraResources.
// The callback takes a parameter of type std::map<UINT32, std::unique_ptr<DXHelper::CameraResources>>&
// through which the list of cameras will be accessed.
template <typename LCallback>
void DXHelper::HolographicDeviceResources::UseHolographicCameraResources(LCallback const& callback)
{
    std::lock_guard<std::mutex> guard(m_cameraResourcesLock);
    callback(m_cameraResources);
}
