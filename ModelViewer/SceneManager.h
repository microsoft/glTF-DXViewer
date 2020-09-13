#pragma once

#include "Scene\RootNode.h"
#include "Singleton.h"

// Just using this singleton for the time-being until it becomes clearer on how
// to best structure..
//
using namespace DX;
using namespace ModelViewer;

class SceneManager : public Singleton<SceneManager>
{
	friend class Singleton<SceneManager>;

public:
	std::shared_ptr<RootNode> Current();
	const std::shared_ptr<RootNode> Current() const;
	void AddNode(std::shared_ptr<GraphNode> newNode);

	void SetDevResources(const std::shared_ptr<DeviceResources>& deviceResources);
	std::shared_ptr<DeviceResources> DevResources() { return _deviceResources; }

	sub_token RegisterForUpdates(std::function<void(SceneManager const&)> slot)
	{
		return SceneChanged.subscribe(slot);
	}

	void UnregisterForUpdates(sub_token conn)
	{
		conn.disconnect();
	}

	sub_token RegisterForSelectionChanged(std::function<void(std::shared_ptr<GraphNode>)> slot)
	{
		return SelectionChanged.subscribe(slot);
	}

	void SetSelected(std::shared_ptr<GraphNode> node);
	std::shared_ptr<GraphNode> GetSelected();

protected:
	SceneManager();

private:

	// Just have once scene for now..
	std::shared_ptr<RootNode> _sceneNode;
	std::shared_ptr<DeviceResources> _deviceResources;

	subject<SceneManager const&> SceneChanged;
	subject<std::shared_ptr<GraphNode>> SelectionChanged;
};
