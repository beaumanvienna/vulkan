/* Engine Copyright (c) 2023 Engine Development Team
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

#pragma once

#include <vector>

#include "engine.h"
#include "entt.hpp"
#include "scene/dictionary.h"

namespace GfxRenderEngine
{
    class SceneGraph
    {
    public:
        class TreeNode
        {

        public:
            TreeNode(entt::entity gameObject, const std::string& name);
            TreeNode(TreeNode const& other);
            ~TreeNode();

            const std::string& GetName() const;
            uint GetChild(uint const childIndex);
            std::vector<uint>& GetChildren() { return m_Children; }

            uint Children();
            entt::entity GetGameObject() const;

        private:
            uint AddChild(uint const nodeIndex);

        private:
            entt::entity m_GameObject;
            std::string m_Name;
            std::vector<uint> m_Children;

            friend SceneGraph;
        };

    public:
        static constexpr uint ROOT_NODE = 0;
        static constexpr uint NODE_INVALID = -1;

    public:
        uint CreateNode(uint parentNode, entt::entity const gameObject, std::string const& name, Dictionary& dictionary);
        uint CreateRootNode(entt::entity const gameObject, std::string const& name, Dictionary& dictionary);
        TreeNode& GetNode(uint const nodeIndex);
        TreeNode& GetNodeByGameObject(entt::entity const gameObject);
        TreeNode& GetRoot();

        uint GetTreeNodeIndex(entt::entity const gameObject);
        void TraverseLog(uint nodeIndex, uint indent = 0);

    private:
        std::mutex m_MutexSceneGraph;
        std::vector<TreeNode> m_Nodes;
        std::map<entt::entity, uint> m_MapFromGameObjectToNode;
    };
} // namespace GfxRenderEngine
