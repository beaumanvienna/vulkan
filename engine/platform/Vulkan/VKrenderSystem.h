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

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "engine.h"
#include "scene/entity.h"

#include "VKdevice.h"
#include "VKpipeline.h"

namespace GfxRenderEngine
{
    struct VK_SimplePushConstantData
    {
        glm::mat2 m_Transform{1.0f};
        glm::vec2 m_Offset{0.0f};
        alignas(16) glm::vec3 m_Color{1.0f};
    };

    class VK_RenderSystem
    {

    public:

        VK_RenderSystem(std::shared_ptr<VK_Device> device, VkRenderPass renderPass);
        ~VK_RenderSystem();

        VK_RenderSystem(const VK_RenderSystem&) = delete;
        VK_RenderSystem& operator=(const VK_RenderSystem&) = delete;

        void RenderEntities(VkCommandBuffer commandBuffer, std::vector<Entity>& entities);

    private:

        void CreatePipelineLayout();
        void CreatePipeline(VkRenderPass renderPass);

    private:

        std::shared_ptr<VK_Device> m_Device;
        std::unique_ptr<VK_Pipeline> m_Pipeline;
        VkPipelineLayout m_PipelineLayout;

    };
}