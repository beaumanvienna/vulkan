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

#include <vulkan/vulkan.h>

#include "renderer/camera.h"
#include "scene/components.h"
#include "pointlights.h"

namespace GfxRenderEngine
{

    struct PointLight
    {
        glm::vec4 m_Position{}; // ignore w
        glm::vec4 m_Color{};    // w is intensity
    };

    struct DirectionalLight
    {
        glm::vec4 m_Direction{}; // ignore w
        glm::vec4 m_Color{};     // w is intensity
    };

    // remember alignment requirements!
    // https://www.oreilly.com/library/view/opengl-programming-guide/9780132748445/app09lev1sec2.html
    struct GlobalUniformBuffer
    {
        glm::mat4 m_Projection{1.0f};
        glm::mat4 m_View{1.0f};

        // point light
        glm::vec4 m_AmbientLightColor{0.0f, 0.0f, 0.0f, 0.0f};
        PointLight m_PointLights[MAX_LIGHTS];
        DirectionalLight m_DirectionalLight;
        int m_NumberOfActivePointLights;
        int m_NumberOfActiveDirectionalLights;
    };

    struct ShadowUniformBuffer
    {
        glm::mat4 m_Projection{1.0f};
        glm::mat4 m_View{1.0f};
    };

    struct VK_FrameInfo
    {
        int m_FrameIndex{0};
        uint m_ImageIndex{0};
        float m_FrameTime{0.0f};
        VkCommandBuffer m_CommandBuffer{nullptr};
        Camera* m_Camera{nullptr};
        VkDescriptorSet m_GlobalDescriptorSet{nullptr};
        VkDescriptorSet m_DiffuseDescriptorSet{nullptr};
    };

} // namespace GfxRenderEngine
