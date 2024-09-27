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

#include "systems/bloom/bloom.h"
#include "systems/bloom/VKbloomRenderPass.h"
#include "systems/bloom/VKbloomFrameBuffer.h"

namespace GfxRenderEngine
{
    struct VK_PushConstantDataBloom
    {
        glm::vec2 m_SrcResolution;
        float m_FilterRadius;
        float m_Padding;
    };

    class VK_RenderSystemBloom
    {

    public:
        static constexpr int NUMBER_OF_MIPMAPS = BLOOM_MIP_LEVELS; // number of down-sampled images plus level 0
        static constexpr int NUMBER_OF_DOWNSAMPLED_IMAGES = NUMBER_OF_MIPMAPS - 1; // number of down-sampled images

    public:
        VK_RenderSystemBloom(VK_RenderPass const& renderPass3D);
        ~VK_RenderSystemBloom();

        VK_RenderSystemBloom(const VK_RenderSystemBloom&) = delete;
        VK_RenderSystemBloom& operator=(const VK_RenderSystemBloom&) = delete;

        void RenderBloom(VK_FrameInfo const& frameInfo);
        void SetFilterRadius(float radius) { m_FilterRadius = radius; }

    private:
        void CreateImageViews();
        void CreateAttachments();
        void CreateRenderPasses();
        void CreateFrameBuffersDown();
        void CreateFrameBuffersUp();

        void CreateDescriptorSet();
        void CreateBloomPipelinesLayout();
        void CreateBloomPipelines();
        void CreateBloomDescriptorSetLayout();

        void BeginRenderPass(VK_FrameInfo const& frameInfo, VK_BloomRenderPass* renderpass,
                             VK_BloomFrameBuffer* framebuffer);
        void SetViewPort(const VK_FrameInfo& frameInfo, VkExtent2D const& extent);

    private:
        VK_RenderPass const& m_RenderPass3D; // external 3D pass
        VkPipelineLayout m_BloomPipelineLayout;

        VkExtent2D m_ExtentMipLevel0;
        float m_FilterRadius;

        VkSampler m_Sampler;
        std::unique_ptr<VK_DescriptorSetLayout> m_BloomDescriptorSetsLayout;
        VkDescriptorSet m_BloomDescriptorSets[VK_RenderSystemBloom::NUMBER_OF_MIPMAPS];

        VkImageView m_EmissionMipmapViews[VK_RenderSystemBloom::NUMBER_OF_MIPMAPS];
        VK_Attachments m_AttachmentsDown;
        VK_Attachments m_AttachmentsUp;

        std::unique_ptr<VK_BloomRenderPass> m_RenderPassDown;
        std::unique_ptr<VK_BloomRenderPass> m_RenderPassUp;
        std::unique_ptr<VK_BloomFrameBuffer> m_FramebuffersDown[VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES];
        std::unique_ptr<VK_BloomFrameBuffer> m_FramebuffersUp[VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES];
        std::unique_ptr<VK_Pipeline> m_BloomPipelineDown;
        std::unique_ptr<VK_Pipeline> m_BloomPipelineUp;
    };
} // namespace GfxRenderEngine
