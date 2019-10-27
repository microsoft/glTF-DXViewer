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
#include <memory>
#include "SceneManager.h"

namespace ModelViewer
{
    [Windows::UI::Xaml::Data::Bindable]
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class GraphNodeData sealed
    {
	internal:
		GraphNodeData(std::shared_ptr<GraphNode> node) :
			_node(node)
		{
			this->Name = ref new String(node->Name().c_str());
		}

    public:
 
        property Platform::String^ Name
        {
            Platform::String^ get() { return name; }
            void set(Platform::String^ value) { name = value; }
        }

        property bool IsFolder
        {
            bool get() { return isFolder; }
            void set(bool value) { isFolder = value; }
        }

		property bool IsSelected
		{
			bool get() { return _node->IsSelected(); }
			void set(bool value) 
			{
				SceneManager::Instance().SetSelected(_node);
			}
		}

    private:
        Platform::String^ name = nullptr;
        bool isFolder = false;
		std::shared_ptr<GraphNode> _node;
    };
}

