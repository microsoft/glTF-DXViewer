#include "pch.h"
#include "SceneContext.h"
#include <D3D11_2.h>

SceneContext::SceneContext(ID3D11DeviceContext3 *context) :
	_context(context)
{
	XMStoreFloat4x4(&_model, XMMatrixIdentity());
}

SceneContext::~SceneContext()
{
}
