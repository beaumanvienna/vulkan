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
#include "VKrenderPass.h"
#include "VKbloomRenderPass.h"
#include "bloom.h"

namespace GfxRenderEngine
{
    struct VK_PushConstantDataBloom
    {
        glm::vec2 m_SrcResolution;
        float m_FilterRadius;
        int m_ImageViewID;
    };

    class VK_RenderSystemBloom
    {

    public:

        static constexpr int NUMBER_OF_MIPMAPS = BLOOM_MIP_LEVELS; // number of down-sampled images

    public:

        VK_RenderSystemBloom
        (
            const VK_RenderPass& renderPass3D,
            VkDescriptorSetLayout& globalDescriptorSetLayout,
            VK_DescriptorPool& descriptorPool
        );
        ~VK_RenderSystemBloom();

        VK_RenderSystemBloom(const VK_RenderSystemBloom&) = delete;
        VK_RenderSystemBloom& operator=(const VK_RenderSystemBloom&) = delete;

        void RenderBloom(const VK_FrameInfo& frameInfo);
        void SetFilterRadius(float radius) {m_FilterRadius = radius;}

    private:

        void CreateRenderPassesDown();
        void CreateRenderPassesUp();
        void CreateBloomPipelinesLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayout);
        void CreateBloomPipelines();
        void CreateBloomDescriptorSetLayout();
        void CreateImageViews();
        void CreateDescriptorSets();

    private:

        VK_DescriptorPool& m_DescriptorPool;
        const VK_RenderPass& m_RenderPass3D;
        VkPipelineLayout m_BloomPipelineLayout;

        VkExtent2D m_Resolution;
        float m_FilterRadius;

        std::unique_ptr<VK_DescriptorSetLayout> m_BloomDescriptorSetLayout;
        std::vector<VkDescriptorSet> m_BloomDescriptorSets{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};

        VkImageView m_EmissionMipmapViews[VK_SwapChain::MAX_FRAMES_IN_FLIGHT][VK_RenderSystemBloom::NUMBER_OF_MIPMAPS];

        std::unique_ptr<VK_BloomRenderPass> m_RenderPassesDown[VK_RenderSystemBloom::NUMBER_OF_MIPMAPS];
        std::unique_ptr<VK_BloomRenderPass> m_RenderPassesUp[VK_RenderSystemBloom::NUMBER_OF_MIPMAPS];
        std::unique_ptr<VK_Pipeline> m_BloomPipelineDown[VK_RenderSystemBloom::NUMBER_OF_MIPMAPS];
        std::unique_ptr<VK_Pipeline> m_BloomPipelineUp[VK_RenderSystemBloom::NUMBER_OF_MIPMAPS];

    };
}
