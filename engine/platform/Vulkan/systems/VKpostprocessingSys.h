/* Engine Copyright (c) 2023 Engine Development Team
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
#include "VKpipeline.h"
#include "VKframeInfo.h"
#include "VKdescriptor.h"

namespace GfxRenderEngine
{
    class VK_RenderSystemPostProcessing
    {

    public:
        VK_RenderSystemPostProcessing(VkRenderPass renderPass,
                                      std::vector<VkDescriptorSetLayout>& postProcessingDescriptorSetLayouts,
                                      const VkDescriptorSet* postProcessingDescriptorSet);
        ~VK_RenderSystemPostProcessing();

        VK_RenderSystemPostProcessing(const VK_RenderSystemPostProcessing&) = delete;
        VK_RenderSystemPostProcessing& operator=(const VK_RenderSystemPostProcessing&) = delete;

        void PostProcessingPass(const VK_FrameInfo& frameInfo);

    private:
        void CreatePostProcessingPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        void CreatePostProcessingPipeline(VkRenderPass renderPass);

    private:
        VkPipelineLayout m_PostProcessingPipelineLayout;
        std::unique_ptr<VK_Pipeline> m_PostProcessingPipeline;

        const VkDescriptorSet* m_PostProcessingDescriptorSets;
    };
} // namespace GfxRenderEngine
