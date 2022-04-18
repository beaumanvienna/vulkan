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

#include "gtc/quaternion.hpp"
#include "gtx/quaternion.hpp"

#include "scene/scene.h"

namespace GfxRenderEngine
{
    Scene::Scene()
        : m_IsRunning{false}
    {
        auto entity = m_Registry.create();
        // The root node gets a transform so that each and every node
        // has a transform, however, it should never be used
        TransformComponent transform{};
        m_Registry.emplace<TransformComponent>(entity, transform);
        m_SceneHierarchy.SetGameObject(entity);

        m_Dictionary.InsertShort("root", entity);
    }

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

    void TransformComponent::SetScale(const glm::vec3& scale)
    {
        m_Scale = scale;
        m_Dirty = true;
    }

    void TransformComponent::SetScaleX(const float scaleX)
    {
        m_Scale.x = scaleX;
        m_Dirty = true;
    }

    void TransformComponent::SetScaleY(const float scaleY)
    {
        m_Scale.y = scaleY;
        m_Dirty = true;
    }

    void TransformComponent::SetScaleZ(const float scaleZ)
    {
        m_Scale.z = scaleZ;
        m_Dirty = true;
    }

    void TransformComponent::AddScale(const glm::vec3& deltaScale)
    {
        SetScale(m_Scale + deltaScale);
    }

    void TransformComponent::SetRotation(const glm::vec3& rotation)
    {
        m_Rotation = rotation;
        m_Dirty = true;
    }

    void TransformComponent::SetRotation(const glm::quat& quaternion)
    {
        glm::vec3 convert = glm::eulerAngles(quaternion);
        // ZYX - model in Blender
        SetRotation(glm::vec3{convert.x, convert.y, convert.z});
    }

    void TransformComponent::SetRotationX(const float rotationX)
    {
        m_Rotation.x = rotationX;
        m_Dirty = true;
    }

    void TransformComponent::SetRotationY(const float rotationY)
    {
        m_Rotation.y = rotationY;
        m_Dirty = true;
    }

    void TransformComponent::SetRotationZ(const float rotationZ)
    {
        m_Rotation.z = rotationZ;
        m_Dirty = true;
    }

    void TransformComponent::AddRotation(const glm::vec3& deltaRotation)
    {
        SetRotation(m_Rotation + deltaRotation);
    }

    void TransformComponent::SetTranslation(const glm::vec3& translation)
    {
        m_Translation = translation;
        m_Dirty = true;
    }

    void TransformComponent::SetTranslationX(const float translationX)
    {
        m_Translation.x = translationX;
        m_Dirty = true;
    }

    void TransformComponent::SetTranslationY(const float translationY)
    {
        m_Translation.y = translationY;
        m_Dirty = true;
    }

    void TransformComponent::SetTranslationZ(const float translationZ)
    {
        m_Translation.z = translationZ;
        m_Dirty = true;
    }

    void TransformComponent::AddTranslation(const glm::vec3& deltaTranslation)
    {
        SetTranslation(m_Translation + deltaTranslation);
    }

    void TransformComponent::RecalculateMatrices()
    {
        auto scale = glm::scale(glm::mat4(1.0f), m_Scale);
        auto rotation = glm::toMat4(glm::quat(m_Rotation));
        auto translation = glm::translate(glm::mat4(1.0f), glm::vec3{m_Translation.x, m_Translation.y, m_Translation.z});

        m_Mat4 = translation * rotation * scale;
        m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_Mat4)));
    }

    const glm::mat4& TransformComponent::GetMat4()
    {
        if (m_Dirty)
        {
            m_Dirty = false;
            RecalculateMatrices();
        }
        return m_Mat4;
    }

    const glm::mat3& TransformComponent::GetNormalMatrix()
    {
        if (m_Dirty)
        {
            m_Dirty = false;
            RecalculateMatrices();
        }
        return m_NormalMatrix;
    }

}
