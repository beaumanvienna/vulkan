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

#include <iostream>
#include <chrono>
#include <thread>

#include "gtc/quaternion.hpp"
#include "gtx/quaternion.hpp"

#include "auxiliary/file.h"
#include "scene/components.h"
#include "scene/scene.h"
#include "renderer/camera.h"

namespace GfxRenderEngine
{
    Scene::Scene(const std::string& filepath, const std::string& alternativeFilepath)
        : m_IsRunning(false), m_Filepath(filepath), m_AlternativeFilepath{alternativeFilepath}, m_SceneLightsGroupNode{0},
          m_LightCounter{0}
    {
        {
            m_Name = EngineCore::GetFilenameWithoutExtension(filepath);
            auto entity = m_Registry.Create();
            // The root node gets a transform so that each and every node
            // has a transform, however, it should never be used
            TransformComponent transform{};
            m_Registry.emplace<TransformComponent>(entity, transform);

            m_SceneGraph.CreateRootNode(entity, m_Name + "::sceneRoot", m_Dictionary);
        }
        {
            // create lights group
            auto name = "SceneLights";
            auto entity = m_Registry.Create();

            TransformComponent transform{};
            m_Registry.emplace<TransformComponent>(entity, transform);

            m_SceneLightsGroupNode = m_SceneGraph.CreateNode(SceneGraph::ROOT_NODE, entity, name, m_Dictionary);
        }
    }

    Scene::~Scene() {}

    entt::entity Scene::CreatePointLight(const float intensity, const float radius, const glm::vec3& color)
    {
        entt::entity pointLight = m_Registry.Create();

        // transform
        TransformComponent lightTransform{};
        m_Registry.emplace<TransformComponent>(pointLight, lightTransform);

        // point light component
        PointLightComponent pointLightComponent{intensity, radius, color};
        m_Registry.emplace<PointLightComponent>(pointLight, pointLightComponent);

        // add to scene graph
        std::string name = "light" + std::to_string(m_LightCounter);
        m_SceneGraph.CreateNode(m_SceneLightsGroupNode, pointLight, name, m_Dictionary);
        ++m_LightCounter;

        return pointLight;
    }

    entt::entity Scene::CreateDirectionalLight(const float intensity, const glm::vec3& color)
    {
        entt::entity directionlLight = m_Registry.Create();

        DirectionalLightComponent directionlLightComponent{intensity, color};
        m_Registry.emplace<DirectionalLightComponent>(directionlLight, directionlLightComponent);
        return directionlLight;
    }
} // namespace GfxRenderEngine
