/* Engine Copyright (c) 2022 Engine Development Team 
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

#include "scene/treeNode.h"

namespace GfxRenderEngine
{
    TreeNode::TreeNode(entt::entity gameObject,
                       const std::string& name,
                       const std::string& longName)
        : m_GameObject(gameObject),
          m_LongName(longName),
          m_Name(name)
    {}

    TreeNode::~TreeNode()
    {}

    entt::entity TreeNode::GetGameObject() const
    {
        return m_GameObject;
    }

    const std::string& TreeNode::GetName() const
    {
        return m_Name;
    }

    const std::string& TreeNode::GetLongName() const
    {
        return m_LongName;
    }

    uint TreeNode::Children() const
    {
        return m_Children.size();
    }

    TreeNode& TreeNode::GetChild(uint index)
    {
        return m_Children[index];
    }

    TreeNode* TreeNode::AddChild(const TreeNode& node, Dictionary& dictionary)
    {
        dictionary.InsertShort(node.GetName(), node.GetGameObject());
        dictionary.InsertLong(node.GetLongName(), node.GetGameObject());
        m_Children.push_back(node);
        return &m_Children.back();
    }

    void TreeNode::SetGameObject(entt::entity gameObject)
    {
        m_GameObject = gameObject;
    }

    void TreeNode::TraverseInfo(TreeNode& node, uint indent)
    {
        std::string indentStr(indent, ' ');
        LOG_CORE_INFO("{0}game object `{1}`, name: `{2}`", indentStr, static_cast<uint>(node.GetGameObject()), node.GetName());
        for (uint index = 0; index < node.Children(); ++index)
        {
            TraverseInfo(node.GetChild(index), indent + 4);
        }
    }

    void TreeNode::CreateLinearMap(std::map<entt::entity,TreeNode*>& sceneHierarchyLinear, TreeNode& node)
    {
        sceneHierarchyLinear[node.GetGameObject()] = &node;
        for (uint index = 0; index < node.Children(); ++index)
        {
            CreateLinearMap(sceneHierarchyLinear, node.GetChild(index));
        }
    }
}
