#pragma once

#include "Scene\RootNode.h"
#include "Singleton.h"

// Just using this singleton for the time-being until it becomes clearer on how
// to best structure..
//
using namespace std;
using namespace DX;
using namespace ModelViewer;

class SceneManager : public Singleton<SceneManager>
{
	friend class Singleton<SceneManager>;

public:
	shared_ptr<RootNode> Current();
	const shared_ptr<RootNode> Current() const;
	void AddNode(shared_ptr<GraphNode> newNode);

	void SetDevResources(const shared_ptr<DeviceResources>& deviceResources);
	shared_ptr<DeviceResources> DevResources() { return _deviceResources; }

	sub_token RegisterForUpdates(function<void(SceneManager const&)> slot)
	{
		return SceneChanged.subscribe(slot);
	}

	void UnregisterForUpdates(sub_token conn)
	{
		conn.disconnect();
	}

	sub_token RegisterForSelectionChanged(function<void(shared_ptr<GraphNode>)> slot)
	{
		return SelectionChanged.subscribe(slot);
	}

	void SetSelected(shared_ptr<GraphNode> node);
	shared_ptr<GraphNode> GetSelected();

protected:
	SceneManager();

private:

	// Just have once scene for now..
	shared_ptr<RootNode> _sceneNode;
	shared_ptr<DeviceResources> _deviceResources;

	subject<SceneManager const&> SceneChanged;
	subject<shared_ptr<GraphNode>> SelectionChanged;
};
