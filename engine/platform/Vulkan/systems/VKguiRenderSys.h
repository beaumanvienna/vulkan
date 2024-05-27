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

#include <memory>
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "engine.h"
#include "renderer/camera.h"
#include "scene/scene.h"

#include "VKdevice.h"
#include "VKbuffer.h"
#include "VKpipeline.h"
#include "VKframeInfo.h"
#include "VKdescriptor.h"

namespace GfxRenderEngine
{
    struct VK_PushConstantDataGUIRenderer
    {
        glm::mat4 m_MVP{1.0f};
        glm::vec2 m_UV[2];
    };

    struct VK_PushConstantDataGUIRenderer2
    {
        glm::mat4 m_Mat4{1.0f};
        glm::vec2 m_UV[2];
    };

    class VK_RenderSystemGUIRenderer
    {

    public:
        VK_RenderSystemGUIRenderer(VkRenderPass renderPass, VK_DescriptorSetLayout& globalDescriptorSetLayout);
        ~VK_RenderSystemGUIRenderer();

        VK_RenderSystemGUIRenderer(const VK_RenderSystemGUIRenderer&) = delete;
        VK_RenderSystemGUIRenderer& operator=(const VK_RenderSystemGUIRenderer&) = delete;

        void RenderSprite(const VK_FrameInfo& frameInfo, const Sprite& sprite, const glm::mat4& modelViewProjectionMatrix);
        void RenderSprite(const VK_FrameInfo& frameInfo, const Sprite& sprite, const glm::mat4& position,
                          const glm::vec4& color, const float textureID = 1.0f);

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

    private:
        const uint m_VertexCount = 6;
        VkPipelineLayout m_PipelineLayout;
        std::unique_ptr<VK_Pipeline> m_Pipeline;
        std::unique_ptr<VK_Pipeline> m_Pipeline2;
    };
} // namespace GfxRenderEngine
