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
#include "gtx/matrix_decompose.hpp"

#include "scene/components.h"
#include "auxiliary/file.h"
#include "renderer/camera.h"
#include "renderer/instanceBuffer.h"

namespace GfxRenderEngine
{
    uint MeshComponent::m_DefaultNameTagCounter = 0;

    MeshComponent::MeshComponent(std::string const& name, std::shared_ptr<Model> model, bool enabled)
        : m_Name{name}, m_Model{model}, m_Enabled{enabled}
    {
    }

    MeshComponent::MeshComponent(std::shared_ptr<Model> model, bool enabled) : m_Model{model}, m_Enabled{enabled}
    {
        m_Name = "mesh component " + std::to_string(m_DefaultNameTagCounter++);
    }

    TransformComponent::TransformComponent()
        : m_Scale(glm::vec3(1.0)), m_Rotation(glm::vec3(0.0)), m_Translation(glm::vec3(0.0)), m_Dirty(true)
    {
    }

    TransformComponent::TransformComponent(glm::vec3& scale, glm::quat& rotation, glm::vec3& translation)
        : m_Scale(scale), m_Rotation(glm::eulerAngles(rotation)), m_Translation(translation), m_Dirty(true)
    {
    }

    TransformComponent::TransformComponent(const glm::mat4& mat4) { SetMat4Local(mat4); }

    void TransformComponent::SetMat4Local(const glm::mat4& mat4)
    {
        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(mat4, scale, rotation, translation, skew, perspective);
        glm::vec3 rotationEuler = glm::eulerAngles(rotation);

        SetTranslation(translation);
        SetRotation(rotationEuler);
        SetScale(scale);
    }

    void TransformComponent::SetDirtyFlag() { m_Dirty = true; }

    bool TransformComponent::GetDirtyFlag() const { return m_Dirty; }

    void TransformComponent::SetScale(const glm::vec3& scale)
    {
        m_Scale = scale;
        m_Dirty = true;
    }

    void TransformComponent::SetScale(const float scale)
    {
        m_Scale = glm::vec3{scale};
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

    void TransformComponent::AddScale(const glm::vec3& deltaScale) { SetScale(m_Scale + deltaScale); }

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
        m_Dirty = true;
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

    void TransformComponent::AddRotation(const glm::vec3& deltaRotation) { SetRotation(m_Rotation + deltaRotation); }

    void TransformComponent::AddRotationY(const float deltaRotation)
    {
        SetRotation(m_Rotation + glm::vec3(0.0f, deltaRotation, 0.0f));
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

    void TransformComponent::AddTranslationX(const float deltaTranslation)
    {
        SetTranslation(m_Translation + glm::vec3(deltaTranslation, 0.0f, 0.0f));
    }

    void TransformComponent::RecalculateMatrices()
    {
        auto scale = glm::scale(glm::mat4(1.0f), m_Scale);
        auto rotation = glm::toMat4(glm::quat(m_Rotation));
        auto translation = glm::translate(glm::mat4(1.0f), glm::vec3{m_Translation.x, m_Translation.y, m_Translation.z});

        m_Mat4Local = translation * rotation * scale;

        m_Dirty = false;
    }

    const glm::mat4& TransformComponent::GetMat4Local()
    {
        if (m_Dirty)
        {
            RecalculateMatrices();
        }
        return m_Mat4Local;
    }

    void TransformComponent::SetMat4Global(const glm::mat4& parent)
    {
        if (m_InstanceBuffer)
        {
            auto mat4Global = parent * GetMat4Local();
            auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mat4Global)));
            m_InstanceBuffer->SetInstanceData(m_InstanceIndex, mat4Global, normalMatrix);
        }
        else
        {
            m_Mat4Global = parent * GetMat4Local();
            m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_Mat4Global)));
        }
        m_Parent = parent;
    }

    // no scene graph
    void TransformComponent::SetMat4Global()
    {
        if (m_InstanceBuffer)
        {
            auto mat4Global = GetMat4Local();
            auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mat4Global)));
            m_InstanceBuffer->SetInstanceData(m_InstanceIndex, mat4Global, normalMatrix);
        }
        else
        {
            m_Mat4Global = GetMat4Local();
            m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_Mat4Global)));
        }
    }

    void TransformComponent::SetInstance(std::shared_ptr<InstanceBuffer>& instanceBuffer, uint instanceIndex)
    {
        m_InstanceIndex = instanceIndex;
        m_InstanceBuffer = instanceBuffer;
    }

    const glm::mat4& TransformComponent::GetMat4Global()
    {
        if (m_InstanceBuffer)
        {
            return m_InstanceBuffer->GetModelMatrix(m_InstanceIndex);
        }
        else
        {
            return m_Mat4Global;
        }
    }

    const glm::mat4& TransformComponent::GetNormalMatrix()
    {
        if (m_InstanceBuffer)
        {
            return m_InstanceBuffer->GetNormalMatrix(m_InstanceIndex);
        }
        else
        {
            return m_NormalMatrix;
        }
    }

    const glm::mat4& TransformComponent::GetParent() { return m_Parent; }

    ScriptComponent::ScriptComponent(const std::string& filepath) : m_Filepath(filepath) {}

    ScriptComponent::ScriptComponent(const std::string_view filepath) : m_Filepath(std::string(filepath)) {}
} // namespace GfxRenderEngine
