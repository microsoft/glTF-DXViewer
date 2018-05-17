#pragma once
#include "Scene\GraphContainerNode.h"

class RootNode :
	public GraphContainerNode
{
public:
	RootNode(int index);
	~RootNode();

	shared_ptr<GraphNode> SelectedNode();

private:
	shared_ptr<GraphNode> _selectedNode;
};

