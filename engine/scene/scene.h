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

#pragma once

#include <iostream>
#include <unordered_map>

#include "engine.h"
#include "entt.hpp"
#include "events/event.h"
#include "scene/registry.h"
#include "scene/sceneGraph.h"
#include "scene/dictionary.h"
#include "auxiliary/timestep.h"

namespace GfxRenderEngine
{
    class Camera;
    class Scene
    {

    public:
        Scene() = delete;
        Scene(const std::string& filepath, const std::string& alternativeFilepath);
        virtual ~Scene();

        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual void OnUpdate(const Timestep& timestep) = 0;
        virtual void OnEvent(Event& event) = 0;
        virtual Camera& GetCamera() = 0;
        virtual void OnResize() = 0;

        virtual void Load() = 0;
        virtual void Save() = 0;
        virtual void LoadScripts() = 0;
        virtual void StartScripts() = 0;
        virtual void ResetTimer() {}

        entt::entity CreatePointLight(const float intensity = 1.0f, const float radius = 0.1f,
                                      const glm::vec3& color = glm::vec3{1.0f, 1.0f, 1.0f});
        entt::entity CreateDirectionalLight(const float intensity = 1.0f,
                                            const glm::vec3& color = glm::vec3{1.0f, 1.0f, 1.0f});

        bool IsFinished() const { return !m_IsRunning; }
        void SetRunning() { m_IsRunning = true; }
        Registry& GetRegistry() { return m_Registry; };
        Dictionary& GetDictionary() { return m_Dictionary; };
        SceneGraph& GetSceneGraph() { return m_SceneGraph; }
        SceneGraph::TreeNode* GetTreeNode(entt::entity entity) { return &m_SceneGraph.GetNodeByGameObject(entity); }
        SceneGraph::TreeNode& GetTreeNode(uint nodeIndex) { return m_SceneGraph.GetNode(nodeIndex); }
        uint GetTreeNodeIndex(entt::entity entity) { return m_SceneGraph.GetTreeNodeIndex(entity); }

    protected:
        std::string m_Name;
        std::string m_Filepath;
        std::string m_AlternativeFilepath;
        Registry m_Registry;
        Dictionary m_Dictionary;
        SceneGraph m_SceneGraph;
        bool m_IsRunning;

        // scene lights
        uint m_SceneLightsGroupNode;
        uint m_LightCounter;

        friend class SceneLoader;
        friend class SceneLoaderJSON;
    };
} // namespace GfxRenderEngine
