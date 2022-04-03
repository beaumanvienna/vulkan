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

#include <iostream>
#include <chrono>
#include <thread>

#include "scene/scene.h"

namespace GfxRenderEngine
{
    Scene::~Scene()
    {
        #ifdef DEBUG
            std::cout << "Scene::~Scene()" << std::endl;
        #endif
    }
    uint MeshComponent::m_DefaultNameTagCounter = 0;

    entt::entity Scene::CreateEntity()
    {
        Entity entity = Entity::CreateEntity(m_Registry);
        return entity.GetID();
    }

    void Scene::DestroyEntity(entt::entity entity)
    {
        // destroys an entity and all its components
        m_Registry.destroy(entity);
    }

    entt::entity Scene::CreatePointLight(const float intensity, const float radius,
                                         const glm::vec3& color)
    {
        entt::entity pointLight = CreateEntity();

        PointLightComponent pointLightComponent{intensity, radius, color};
        m_Registry.emplace<PointLightComponent>(pointLight, pointLightComponent);
        return pointLight;
    }

    MeshComponent::MeshComponent(std::string name, std::shared_ptr<Model> model, bool enabled)
        : m_Name{name}, m_Model{model}, m_Enabled{enabled}
    {
    }

    MeshComponent::MeshComponent(std::shared_ptr<Model> model, bool enabled)
        : m_Model{model}, m_Enabled{enabled}
    {
        m_Name = "mesh component " + std::to_string(m_DefaultNameTagCounter++);
    }

    glm::mat4 TransformComponent::Mat4()
    {
        const float c3 = glm::cos(m_Rotation.z);
        const float s3 = glm::sin(m_Rotation.z);
        const float c2 = glm::cos(m_Rotation.x);
        const float s2 = glm::sin(m_Rotation.x);
        const float c1 = glm::cos(m_Rotation.y);
        const float s1 = glm::sin(m_Rotation.y);
        return glm::mat4
        {
            {
                m_Scale.x * (c1 * c3 + s1 * s2 * s3),
                m_Scale.x * (c2 * s3),
                m_Scale.x * (c1 * s2 * s3 - c3 * s1),
                0.0f,
            },
            {
                m_Scale.y * (c3 * s1 * s2 - c1 * s3),
                m_Scale.y * (c2 * c3),
                m_Scale.y * (c1 * c3 * s2 + s1 * s3),
                0.0f,
            },
            {
                m_Scale.z * (c2 * s1),
                m_Scale.z * (-s2),
                m_Scale.z * (c1 * c2),
                0.0f,
            },
            {m_Translation.x, m_Translation.y, m_Translation.z, 1.0f}
        };
    }

    glm::mat3 TransformComponent::NormalMatrix()
    {
        const float c3 = glm::cos(m_Rotation.z);
        const float s3 = glm::sin(m_Rotation.z);
        const float c2 = glm::cos(m_Rotation.x);
        const float s2 = glm::sin(m_Rotation.x);
        const float c1 = glm::cos(m_Rotation.y);
        const float s1 = glm::sin(m_Rotation.y);

        const glm::vec3 inverseScale = 1.0f / m_Scale;

        return glm::mat3
        {
            {
                inverseScale.x * (c1 * c3 + s1 * s2 * s3),
                inverseScale.x * (c2 * s3),
                inverseScale.x * (c1 * s2 * s3 - c3 * s1),
            },
            {
                inverseScale.y * (c3 * s1 * s2 - c1 * s3),
                inverseScale.y * (c2 * c3),
                inverseScale.y * (c1 * c3 * s2 + s1 * s3),
            },
            {
                inverseScale.z * (c2 * s1),
                inverseScale.z * (-s2),
                inverseScale.z * (c1 * c2),
            }
        };
    }

}
