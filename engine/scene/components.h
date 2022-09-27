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

#pragma once

#include <string>
#include <memory>

#include "engine.h"

#include "engine/platform/Vulkan/VKswapChain.h"

namespace GfxRenderEngine
{
    constexpr int MAX_LIGHTS = 128;
    class Model;

    class TransformComponent
    {
    public:

        TransformComponent();
        TransformComponent(const glm::mat4& transform);

        void SetScale(const glm::vec3& scale);
        void SetScale(const float scale);
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

        void SetMat4(const glm::mat4& mat4) { m_Mat4 = mat4; }

        // the getters must be const; only the setters have write access
        const glm::vec3& GetScale() { return m_Scale; }
        const glm::vec3& GetRotation() { return m_Rotation; }
        const glm::vec3& GetTranslation() { return m_Translation; }

        const glm::mat4& GetMat4();
        const glm::mat3& GetNormalMatrix();
        void  SetDirtyFlag() { m_Dirty = true; }
        const bool GetDirtyFlag() const { return m_Dirty; }

    private:

        void RecalculateMatrices();

    private:

        bool m_Dirty{true};
        glm::vec3 m_Scale;
        glm::vec3 m_Rotation;
        glm::vec3 m_Translation;
        glm::mat4 m_Mat4;
        glm::mat3 m_NormalMatrix;

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

    struct DirectionalLightComponent
    {
        float m_LightIntensity{1.0f};
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

    class NativeScript;
    struct ScriptComponent
    {
        std::string m_Filepath;
        std::shared_ptr<NativeScript> m_Script;

        ScriptComponent(const std::string& filepath);
    };

    struct SpriteRendererComponent
    {
        float m_Roughness;
        float m_Metallic;
    };

    struct SpriteRendererComponent2D
    {
        uint m_Tag;
    };

    struct PbrNoMapTag
    {
        uint m_Tag;
    };

    struct PbrDiffuseTag
    {
        uint m_Tag;
    };

    struct PbrDiffuseNormalTag
    {
        uint m_Tag;
    };

    struct PbrDiffuseNormalRoughnessMetallicTag
    {
        uint m_Tag;
    };

    struct CubemapComponent
    {
        uint m_Tag;
    };
}
