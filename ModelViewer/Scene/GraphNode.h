#pragma once

#include "Common\DeviceResources.h"
#include "Common\StepTimer.h"
#include "BoundingBox.h"
#include <Rpc.h>
#include "SceneContext.h"
#include <DirectXMath.h>

using namespace DX;
using namespace DirectX;

class GraphNode
{
public:
	GraphNode();
	virtual ~GraphNode();

	virtual void Update(StepTimer const& timer) = 0;
	virtual XMMATRIX PreDraw(SceneContext& context, XMMATRIX model) = 0;
	virtual void Draw(SceneContext& context, XMMATRIX model) = 0;
	virtual void CreateDeviceDependentResources() = 0;
	virtual void Initialise(const std::shared_ptr<DX::DeviceResources>& deviceResources) = 0;
	virtual void AddChild(std::shared_ptr<GraphNode> child) = 0;
	virtual void AfterLoad() = 0;
	virtual BoundingBox<float> GetBoundingBox() = 0;
	virtual void ForAllChildrenRecursive(std::function<void(GraphNode&)> func) = 0;
	virtual void ForAllChildrenRecursiveUntil(std::function<bool(GraphNode&)> func) = 0;
	virtual GraphNode *FindChildByIndex(int index) = 0;
	virtual GraphNode *FindChildById(GUID id) = 0;
	virtual size_t NumChildren() = 0;
	virtual std::shared_ptr<GraphNode> GetChild(int i) = 0;
	virtual const std::wstring& Name() const = 0;
	virtual void SetName(const std::wstring& name) = 0;
	virtual int Index() = 0;
	virtual GUID GetId() = 0;
	virtual bool IsSelected() = 0;
	virtual void SetSelected(bool sel) = 0;
};

