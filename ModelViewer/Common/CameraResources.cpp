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

#include "pch.h"

#include "CameraResources.h"
#include "holographicDeviceResources.h"
#include "DirectXHelper.h"

#include <windows.graphics.directx.direct3d11.interop.h>

using namespace DirectX;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Perception::Spatial;

DXHelper::CameraResources::CameraResources(const HolographicCamera& camera)
    : m_holographicCamera(camera)
    , m_isStereo(camera.IsStereo())
    , m_d3dRenderTargetSize(camera.RenderTargetSize())
{
    m_d3dViewport = CD3D11_VIEWPORT(0.f, 0.f, m_d3dRenderTargetSize.Width, m_d3dRenderTargetSize.Height);
};

// Updates resources associated with a holographic camera's swap chain.
// The app does not access the swap chain directly, but it does create
// resource views for the back buffer.
void DXHelper::CameraResources::CreateResourcesForBackBuffer(
    DXHelper::HolographicDeviceResources* pDeviceResources, const HolographicCameraRenderingParameters& cameraParameters)
{
    const auto device = pDeviceResources->GetD3DDevice();

    // Get a DXGI interface for the holographic camera's back buffer.
    // Holographic cameras do not provide the DXGI swap chain, which is owned
    // by the system. The Direct3D back buffer resource is provided using WinRT
    // interop APIs.
    winrt::com_ptr<ID3D11Resource> resource;
    {
        winrt::com_ptr<::IInspectable> inspectable = cameraParameters.Direct3D11BackBuffer().as<::IInspectable>();
        winrt::com_ptr<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess> dxgiInterfaceAccess;

        HRESULT hr = inspectable->QueryInterface(__uuidof(dxgiInterfaceAccess), dxgiInterfaceAccess.put_void());
        if (FAILED(hr))
        {
            winrt::throw_hresult(hr);
        }

        hr = dxgiInterfaceAccess->GetInterface(__uuidof(resource), resource.put_void());
        if (FAILED(hr))
        {
            winrt::throw_hresult(hr);
        }
    }

    // Get a Direct3D interface for the holographic camera's back buffer.
    winrt::com_ptr<ID3D11Texture2D> cameraBackBuffer;
    resource.as(cameraBackBuffer);

    // Determine if the back buffer has changed. If so, ensure that the render target view
    // is for the current back buffer.
    if (m_d3dBackBuffer != cameraBackBuffer)
    {
        // This can change every frame as the system moves to the next buffer in the
        // swap chain. This mode of operation will occur when certain rendering modes
        // are activated.
        m_d3dBackBuffer = cameraBackBuffer;

        // Get the DXGI format for the back buffer.
        // This information can be accessed by the app using CameraResources::GetBackBufferDXGIFormat().
        D3D11_TEXTURE2D_DESC backBufferDesc;
        m_d3dBackBuffer->GetDesc(&backBufferDesc);
        m_dxgiFormat = backBufferDesc.Format;

        D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {};
        viewDesc.ViewDimension = backBufferDesc.ArraySize > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2D;
        viewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        if (backBufferDesc.ArraySize > 1)
        {
            viewDesc.Texture2DArray.ArraySize = backBufferDesc.ArraySize;
        }

        // Create a render target view of the back buffer.
        // Creating this resource is inexpensive, and is better than keeping track of
        // the back buffers in order to pre-allocate render target views for each one.
        m_d3dRenderTargetView = nullptr;
        winrt::check_hresult(device->CreateRenderTargetView(m_d3dBackBuffer.get(), &viewDesc, m_d3dRenderTargetView.put()));

        // Check for render target size changes.
        winrt::Windows::Foundation::Size currentSize = m_holographicCamera.RenderTargetSize();
        if (m_d3dRenderTargetSize != currentSize)
        {
            // Set render target size.
            m_d3dRenderTargetSize = currentSize;

            // A new depth stencil view is also needed.
            m_d3dDepthStencilView = nullptr;
        }
    }

    // Refresh depth stencil resources, if needed.
    if (m_d3dDepthStencilView == nullptr)
    {
        // Create a depth stencil view for use with 3D rendering if needed.
        CD3D11_TEXTURE2D_DESC depthStencilDesc(
            DXGI_FORMAT_D16_UNORM,
            static_cast<UINT>(m_d3dRenderTargetSize.Width),
            static_cast<UINT>(m_d3dRenderTargetSize.Height),
            m_isStereo ? 2 : 1, // Create two textures when rendering in stereo.
            1,                  // Use a single mipmap level.
            D3D11_BIND_DEPTH_STENCIL);

        winrt::com_ptr<ID3D11Texture2D> depthStencil;
        winrt::check_hresult(device->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.put()));

        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(
            m_isStereo ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D);

        winrt::check_hresult(device->CreateDepthStencilView(depthStencil.get(), &depthStencilViewDesc, m_d3dDepthStencilView.put()));
    }

    // Create the constant buffer, if needed.
    if (m_viewProjectionConstantBuffer == nullptr)
    {
        // Create a constant buffer to store view and projection matrices for the camera.
        CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
        winrt::check_hresult(device->CreateBuffer(&constantBufferDesc, nullptr, m_viewProjectionConstantBuffer.put()));
    }
}

// Releases resources associated with a back buffer.
void DXHelper::CameraResources::ReleaseResourcesForBackBuffer(DXHelper::HolographicDeviceResources* pDeviceResources)
{
    // Release camera-specific resources.
    m_d3dBackBuffer = nullptr;
    m_d3dRenderTargetView = nullptr;
    m_d3dDepthStencilView = nullptr;
    m_viewProjectionConstantBuffer = nullptr;

    // Ensure system references to the back buffer are released by clearing the render
    // target from the graphics pipeline state, and then flushing the Direct3D context.
    pDeviceResources->UseD3DDeviceContext([](auto context) {
        ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {nullptr};
        context->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
        context->Flush();
    });
}

// Updates the view/projection constant buffer for a holographic camera.
void DXHelper::CameraResources::UpdateViewProjectionBuffer(
    std::shared_ptr<DXHelper::HolographicDeviceResources> deviceResources,
    const HolographicCameraPose& cameraPose,
    const SpatialCoordinateSystem& coordinateSystem)
{
    // The system changes the viewport on a per-frame basis for system optimizations.
    m_d3dViewport =
        CD3D11_VIEWPORT(cameraPose.Viewport().X, cameraPose.Viewport().Y, cameraPose.Viewport().Width, cameraPose.Viewport().Height);

    // The projection transform for each frame is provided by the HolographicCameraPose.
    auto cameraProjectionTransform = cameraPose.ProjectionTransform();

    // Get a container object with the view and projection matrices for the given
    // pose in the given coordinate system.
    auto viewTransformContainer = cameraPose.TryGetViewTransform(coordinateSystem);

    // If TryGetViewTransform returns a null pointer, that means the pose and coordinate
    // system cannot be understood relative to one another; content cannot be rendered
    // in this coordinate system for the duration of the current frame.
    // This usually means that positional tracking is not active for the current frame, in
    // which case it is possible to use a SpatialLocatorAttachedFrameOfReference to render
    // content that is not world-locked instead.
    DXHelper::ViewProjectionConstantBuffer viewProjectionConstantBufferData = {};
    bool viewTransformAcquired = viewTransformContainer != nullptr;
    if (viewTransformAcquired)
    {
        // Otherwise, the set of view transforms can be retrieved.
        auto viewCoordinateSystemTransform = viewTransformContainer.Value();

        // Update the view matrices. Holographic cameras (such as Microsoft HoloLens) are
        // constantly moving relative to the world. The view matrices need to be updated
        // every frame.
        XMStoreFloat4x4(
            &viewProjectionConstantBufferData.viewProjection[0],
            XMMatrixTranspose(XMLoadFloat4x4(&viewCoordinateSystemTransform.Left) * XMLoadFloat4x4(&cameraProjectionTransform.Left)));
        XMStoreFloat4x4(
            &viewProjectionConstantBufferData.viewProjection[1],
            XMMatrixTranspose(XMLoadFloat4x4(&viewCoordinateSystemTransform.Right) * XMLoadFloat4x4(&cameraProjectionTransform.Right)));
    }

    // Use the D3D device context to update Direct3D device-based resources.
    deviceResources->UseD3DDeviceContext([&](auto context) {
        // Loading is asynchronous. Resources must be created before they can be updated.
        if (context == nullptr || m_viewProjectionConstantBuffer == nullptr || !viewTransformAcquired)
        {
            m_framePending = false;
        }
        else
        {
            // Update the view and projection matrices.
            context->UpdateSubresource(m_viewProjectionConstantBuffer.get(), 0, nullptr, &viewProjectionConstantBufferData, 0, 0);

            m_framePending = true;
        }
    });
}

// Gets the view-projection constant buffer for the HolographicCamera and attaches it
// to the shader pipeline.
bool DXHelper::CameraResources::AttachViewProjectionBuffer(std::shared_ptr<DXHelper::HolographicDeviceResources> deviceResources)
{
    // This method uses Direct3D device-based resources.
    return deviceResources->UseD3DDeviceContext([&](auto context) {
        // Loading is asynchronous. Resources must be created before they can be updated.
        // Cameras can also be added asynchronously, in which case they must be initialized
        // before they can be used.
        if (context == nullptr || m_viewProjectionConstantBuffer == nullptr || m_framePending == false)
        {
            return false;
        }

        // Set the viewport for this camera.
        context->RSSetViewports(1, &m_d3dViewport);

        // Send the constant buffer to the vertex shader.
        ID3D11Buffer* pBuffer = m_viewProjectionConstantBuffer.get();
        context->VSSetConstantBuffers(1, 1, &pBuffer);

        // The template includes a pass-through geometry shader that is used by
        // default on systems that don't support the D3D11_FEATURE_D3D11_OPTIONS3::
        // VPAndRTArrayIndexFromAnyShaderFeedingRasterizer extension. The shader
        // will be enabled at run-time on systems that require it.
        // If your app will also use the geometry shader for other tasks and those
        // tasks require the view/projection matrix, uncomment the following line
        // of code to send the constant buffer to the geometry shader as well.
        /*context->GSSetConstantBuffers(
            1,
            1,
            m_viewProjectionConstantBuffer.GetAddressOf()
            );*/

        m_framePending = false;

        return true;
    });
}
