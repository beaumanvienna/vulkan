/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

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
    TreeNode::TreeNode() : m_GameObject((entt::entity) -1)
    {}

    TreeNode::TreeNode(entt::entity gameObject) : m_GameObject(gameObject)
    {}

    TreeNode::~TreeNode()
    {}

    entt::entity TreeNode::GetGameObject() const
    {
        return m_GameObject;
    }

    uint TreeNode::Children() const
    {
        return m_Children.size();
    }

    const TreeNode& TreeNode::GetChild(uint index) const
    {
        return m_Children[index];
    }

    void TreeNode::Push(const TreeNode& node)
    {
        m_Children.push_back(node);
    }
}
