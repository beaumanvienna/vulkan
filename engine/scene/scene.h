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

#include <vulkan/vulkan.h>

#include "engine.h"
#include "entt.hpp"
#include "scene/entity.h"
#include "events/event.h"
#include "auxiliary/timestep.h"

#include "engine/platform/Vulkan/VKswapChain.h"

namespace GfxRenderEngine
{
    constexpr int MAX_LIGHTS = 10;

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

        MeshComponent(std::string name, std::shared_ptr<Model> model, bool enabled = true);
        MeshComponent(std::shared_ptr<Model> model, bool enabled = true);

        std::string m_Name;
        std::shared_ptr<Model> m_Model;
        bool m_Enabled;

    private:

        static uint m_DefaultNameTagCounter;

    };

    struct PointLightComponent
    {
        float m_LightIntensity{1.0f};
        float m_Radius{1.0f};
        glm::vec3 m_Color{1.0f, 1.0f, 1.0f};
    };

    struct RigidbodyComponent
    {
        enum Type
        { 
            STATIC,
            DYNAMIC
        };

        Type m_Type = Type::STATIC;
        void* m_Body = nullptr;

    };

    // models without a normal map
    struct DiffuseMapComponent
    {
        uint m_TextureSlot; // diffuse map aka albedo map aka color map
    };

    struct GLTFComponent
    {
        VkDescriptorSet m_DescriptorSet[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
    };

    struct NormalMappingComponent
    {
        VkDescriptorSet m_DescriptorSet[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        float m_Roughness;
        float m_Metallic;
        float m_NormalMapIntensity;
    };

    struct PBRComponent
    {
        VkDescriptorSet m_DescriptorSet[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        float m_Roughness;
        float m_Metallic;
        float m_NormalMapIntensity;
    };

    class Scene
    {

    public:

        Scene() : m_IsRunning{false} {}
        virtual ~Scene();

        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual void OnUpdate(const Timestep& timestep) = 0;
        virtual void OnEvent(Event& event) = 0;
        virtual void OnResize() = 0;

        entt::entity CreateEntity();
        void DestroyEntity(entt::entity entity);

        entt::entity CreatePointLight(const float intensity = 1.0f, const float radius = 0.1f,
                                      const glm::vec3& color = glm::vec3{1.0f, 1.0f, 1.0f});

        bool IsFinished() const { return !m_IsRunning; }

    protected:

        entt::registry m_Registry;
        bool m_IsRunning;

    };
}
