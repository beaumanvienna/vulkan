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

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "engine.h"
#include "renderer/camera.h"
#include "renderer/shader.h"
#include "scene/scene.h"

#include "VKdevice.h"
#include "VKpipeline.h"
#include "VKframeInfo.h"
#include "VKdescriptor.h"
#include "bindless/VKbindlessTexture.h"
#include "bindless/VKbindlessImage.h"

namespace GfxRenderEngine
{
    class VK_RenderSystemPbrBindless
    {

    public:
        VK_RenderSystemPbrBindless(VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        ~VK_RenderSystemPbrBindless();

        VK_RenderSystemPbrBindless(const VK_RenderSystemPbrBindless&) = delete;
        VK_RenderSystemPbrBindless& operator=(const VK_RenderSystemPbrBindless&) = delete;

        void RenderEntities(const VK_FrameInfo& frameInfo, Registry& registry, VK_BindlessTexture* bindlessTexture,
                            VK_BindlessImage* bindlessImage);
        void SetVertexCtrl(VertexCtrl const& vertexCtrl);

    private:
        void CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        void CreatePipeline(VkRenderPass renderPass);

    private:
        VkPipelineLayout m_PipelineLayout;
        std::unique_ptr<VK_Pipeline> m_Pipeline;
        DrawCallInfo m_DrawCallInfo;
    };
} // namespace GfxRenderEngine
