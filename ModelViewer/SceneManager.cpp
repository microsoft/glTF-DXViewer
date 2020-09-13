#include "pch.h"
#include "SceneManager.h"

using namespace std;

SceneManager::SceneManager()
{
	_sceneNode = make_shared<RootNode>(-1);
}

shared_ptr<RootNode> SceneManager::Current()
{
	return _sceneNode;
}

const shared_ptr<RootNode> SceneManager::Current() const
{
	return _sceneNode;
}

void SceneManager::AddNode(shared_ptr<GraphNode> newNode)
{
	Current()->AddChild(newNode);
	SceneChanged.notify(*this);
}

void SceneManager::SetDevResources(const shared_ptr<DX::DeviceResources>& deviceResources)
{
	_deviceResources = deviceResources;
}

void SceneManager::SetSelected(shared_ptr<GraphNode> node)
{
	// De-select all other nodes in the scene..
	_sceneNode->ForAllChildrenRecursive([](GraphNode& nd)
	{
		nd.SetSelected(false);
	});

	// Finally, set our node selected.
	node->SetSelected(true);
	SelectionChanged.notify(node);
}

shared_ptr<GraphNode> SceneManager::GetSelected()
{
	// Loop through the scene to find the one and only selected node (only supporting 
	// single-selection for now)
	shared_ptr<GraphNode> result;
	_sceneNode->ForAllChildrenRecursiveUntil([&result](GraphNode& nd)
	{
		if (nd.IsSelected())
		{
			result.reset(&nd);
		}
		return true;
	});

	return result;
}
