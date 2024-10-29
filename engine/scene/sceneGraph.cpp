/* Engine Copyright (c) 2024 Engine Development Team
   https://github.com/beaumanvienna/vulkan

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "scene/sceneGraph.h"

namespace GfxRenderEngine
{
    SceneGraph::TreeNode::TreeNode(entt::entity gameObject, const std::string& name) : m_GameObject(gameObject), m_Name(name)
    {
    }

    SceneGraph::TreeNode::TreeNode(TreeNode const& other)
        : m_GameObject(other.m_GameObject), m_Name(other.m_Name), m_Children(other.m_Children)
    {
    }

    SceneGraph::TreeNode::~TreeNode() {}

    entt::entity SceneGraph::TreeNode::GetGameObject() const { return m_GameObject; }

    const std::string& SceneGraph::TreeNode::GetName() const { return m_Name; }

    uint SceneGraph::TreeNode::Children() { return m_Children.size(); }

    uint SceneGraph::TreeNode::GetChild(uint const childIndex) { return m_Children[childIndex]; }

    uint SceneGraph::TreeNode::AddChild(uint const nodeIndex)
    {
        uint childIndex = m_Children.size();
        m_Children.push_back(nodeIndex);
        return childIndex;
    }

    uint SceneGraph::CreateNode(uint parentNode, entt::entity const gameObject, std::string const& name,
                                Dictionary& dictionary)
    {
        std::lock_guard<std::mutex> guard(m_MutexSceneGraph);
        uint nodeIndex = m_Nodes.size();
        m_Nodes.push_back({gameObject, name});
        dictionary.Insert(name, gameObject);
        m_MapFromGameObjectToNode[gameObject] = nodeIndex;
        m_Nodes[parentNode].AddChild(nodeIndex);
        return nodeIndex;
    }

    uint SceneGraph::CreateRootNode(entt::entity const gameObject, std::string const& name, Dictionary& dictionary)
    {
        std::lock_guard<std::mutex> guard(m_MutexSceneGraph);
        uint nodeIndex = m_Nodes.size();
        m_Nodes.push_back({gameObject, name});
        dictionary.Insert(name, gameObject);
        m_MapFromGameObjectToNode[gameObject] = nodeIndex;
        return nodeIndex;
    }

    void SceneGraph::TraverseLog(uint nodeIndex, uint indent)
    {
        std::string indentStr(indent, ' ');
        TreeNode& treeNode = m_Nodes[nodeIndex];
        LOG_CORE_INFO("{0}game object `{1}`, name: `{2}`", indentStr, static_cast<uint>(treeNode.GetGameObject()),
                      treeNode.GetName());
        for (uint childIndex = 0; childIndex < treeNode.Children(); ++childIndex)
        {
            TraverseLog(treeNode.GetChild(childIndex), indent + 4);
        }
    }

    SceneGraph::TreeNode& SceneGraph::GetNode(uint const nodeIndex)
    {
        std::lock_guard<std::mutex> guard(m_MutexSceneGraph);
        return m_Nodes[nodeIndex];
    }

    SceneGraph::TreeNode& SceneGraph::GetNodeByGameObject(entt::entity const gameObject)
    {
        std::lock_guard<std::mutex> guard(m_MutexSceneGraph);
        uint nodeIndex = m_MapFromGameObjectToNode[gameObject];
        return m_Nodes[nodeIndex];
    }

    SceneGraph::TreeNode& SceneGraph::GetRoot()
    {
        std::lock_guard<std::mutex> guard(m_MutexSceneGraph);
        CORE_ASSERT(m_Nodes.size(), "SceneGraph::GetRoot(): scene graph is empty");
        return m_Nodes[SceneGraph::ROOT_NODE];
    }

    uint SceneGraph::GetTreeNodeIndex(entt::entity const gameObject)
    {
        std::lock_guard<std::mutex> guard(m_MutexSceneGraph);
        uint returnValue = NODE_INVALID;

        if (m_MapFromGameObjectToNode.find(gameObject) != m_MapFromGameObjectToNode.end())
        {
            returnValue = m_MapFromGameObjectToNode[gameObject];
        }
        return returnValue;
    }
} // namespace GfxRenderEngine
