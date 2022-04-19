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

#include <iostream>
#include <memory>

#include "engine.h"

#include "engine/platform/Vulkan/VKswapChain.h"

namespace GfxRenderEngine
{
    constexpr int MAX_LIGHTS = 10;
    class Model;

    class TransformComponent
    {
    public:

        void SetScale(const glm::vec3& scale);
        void SetScaleX(const float scaleX);
        void SetScaleY(const float scaleY);
        void SetScaleZ(const float scaleZ);
        void AddScale(const glm::vec3& deltaScale);
        void SetRotation(const glm::vec3& rotation);
        void SetRotation(const glm::quat& quaternion);
        void SetRotationX(const float rotationX);
        void SetRotationY(const float rotationY);
        void SetRotationZ(const float rotationZ);
        void AddRotation(const glm::vec3& deltaRotation);
        void SetTranslation(const glm::vec3& translation);
        void SetTranslationX(const float translationX);
        void SetTranslationY(const float translationY);
        void SetTranslationZ(const float translationZ);
        void AddTranslation(const glm::vec3& deltaTranslation);

        // the getters must be const; only the setters have write access
        const glm::vec3& GetScale() { return m_Scale; }
        const glm::vec3& GetRotation() { return m_Rotation; }
        const glm::vec3& GetTranslation() { return m_Translation; }

        const glm::mat4& GetMat4();
        const glm::mat3& GetNormalMatrix();

    private:

        void RecalculateMatrices();

    private:

        bool m_Dirty{true};
        glm::vec3 m_Scale{1.0f};
        glm::vec3 m_Rotation{};
        glm::vec3 m_Translation{};
        glm::mat4 m_Mat4{};
        glm::mat3 m_NormalMatrix{};

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

    struct ScriptComponent
    {
        std::string m_Filepath;
    };

    // models without a normal map
    struct DefaultDiffuseComponent // diffuse map aka albedo map aka color map
    {
        float m_Roughness;
        float m_Metallic;
    };

    struct PbrNoMapComponent
    {
        float m_Roughness;
        float m_Metallic;
        glm::vec3 m_Color;
    };

    struct PbrDiffuseComponent
    {
        VkDescriptorSet m_DescriptorSet[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        float m_Roughness;
        float m_Metallic;
    };

    struct PbrDiffuseNormalComponent
    {
        VkDescriptorSet m_DescriptorSet[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        float m_Roughness;
        float m_Metallic;
        float m_NormalMapIntensity;
    };

    struct PbrDiffuseNormalRoughnessMetallicComponent
    {
        VkDescriptorSet m_DescriptorSet[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        float m_NormalMapIntensity;
    };

    struct PbrDiffuseRoughnessMetallicComponent
    {
        VkDescriptorSet m_DescriptorSet[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
    };
}
