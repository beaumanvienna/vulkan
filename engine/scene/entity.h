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

#pragma once

#include <string>
#include <memory>

#include "engine.h"
#include "entt.hpp"
#include "renderer/model.h"

namespace GfxRenderEngine
{

    struct TransformComponent
    {
        glm::vec3 m_Scale{1.0f};
        glm::vec3 m_Rotation{};
        glm::vec3 m_Translation{};

        glm::mat4 Mat4();
        glm::mat3 NormalMatrix();
    };

    class MeshComponent
    {
    public:
        std::string m_Name;
        std::shared_ptr<Model> m_Model;
        MeshComponent(std::string name, std::shared_ptr<Model> model)
            : m_Name{name}, m_Model{model} {}
        MeshComponent(std::shared_ptr<Model> model) 
            : m_Name{"mesh component " + std::to_string(m_DefaultNameTag++)},
              m_Model{model} {}
    private:
        static uint m_DefaultNameTag;
    };

    struct PointLightComponent
    {
        float m_LightIntensity{1.0f};
    };

    class Entity
    {

    public:

        using id_t = entt::entity;

    public:

        ~Entity();

        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;
        Entity(Entity&&) = default;
        Entity& operator=(Entity&&) = default;

        id_t GetID() const { return m_ID; }

        static Entity CreateEntity(entt::registry& registry);

    private:
        Entity(id_t id, entt::registry& registry): m_ID{id}, m_Registry{registry} {}

    private:

        id_t m_ID;
        entt::registry& m_Registry;

    };
}
