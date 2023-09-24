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

#include "VKcore.h"
#include "VKswapChain.h"
#include "VKrenderPass.h"
#include "VKmodel.h"

#include "systems/bloom/VKbloomRenderSystem.h"

namespace GfxRenderEngine
{
    VK_RenderSystemBloom::VK_RenderSystemBloom
    (
        VK_RenderPass const& renderPass3D,
        VkDescriptorSetLayout& globalDescriptorSetLayout,
        VK_DescriptorPool& descriptorPool
    ) :
        m_RenderPass3D{renderPass3D},
        m_FilterRadius{0.05},
        m_DescriptorPool{descriptorPool}
    {
        m_ExtendMipLevel0 = m_RenderPass3D.GetExtent();

        // render pass and frame buffers
        CreateImageViews();
        CreateAttachments();
        CreateRenderPass();     // use one render pass
        CreateFrameBuffers();   // use multiple frame buffers

        // pipelines
        CreateBloomDescriptorSetLayout();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts =
        {
            globalDescriptorSetLayout,
            m_BloomDescriptorSetLayout->GetDescriptorSetLayout(),
        };

        CreateDescriptorSet();  // use one descriptor set
        CreateBloomPipelinesLayout(descriptorSetLayouts);
        CreateBloomPipelines(); // use two pipelines (up pipeline and down pipeline)
    }

    VK_RenderSystemBloom::~VK_RenderSystemBloom()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_BloomPipelineLayout, nullptr);

        vkDestroyImageView(VK_Core::m_Device->Device(), m_EmissionView, nullptr);
        for (uint mipLevel = 0; mipLevel < VK_RenderSystemBloom::NUMBER_OF_MIPMAPS; ++mipLevel)
        {
            vkDestroyImageView(VK_Core::m_Device->Device(), m_EmissionMipmapViews[mipLevel], nullptr);
        }

        vkDestroySampler(VK_Core::m_Device->Device(), m_Sampler, nullptr);
    }

    void VK_RenderSystemBloom::CreateImageViews()
    {
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_RenderPass3D.GetImageEmission();
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_RenderPass3D.GetFormatEmission();
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = VK_RenderSystemBloom::NUMBER_OF_MIPMAPS;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(VK_Core::m_Device->Device(), &viewInfo, nullptr, &m_EmissionView);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }

        for (uint mipLevel = 0; mipLevel < VK_RenderSystemBloom::NUMBER_OF_MIPMAPS; ++mipLevel)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_RenderPass3D.GetImageEmission();
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_RenderPass3D.GetFormatEmission();
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = mipLevel;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(VK_Core::m_Device->Device(), &viewInfo, nullptr, &m_EmissionMipmapViews[mipLevel]);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }
    }

    void VK_RenderSystemBloom::CreateAttachments()
    {
        VkFormat format = m_RenderPass3D.GetFormatEmission();
        VkExtent2D extent = m_RenderPass3D.GetExtent();


        for (uint mipLevel = 0; mipLevel < BLOOM_MIP_LEVELS; ++mipLevel)
        {

            // the g-buffer level zero image must not be cleared,
            // the level 1 mip image and following mip levels do have to be cleared
            VkAttachmentLoadOp loadOp = 
                ( (mipLevel == 0) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR);


            VkExtent2D extentMipLevel{extent.width>>mipLevel, extent.height>>mipLevel};

            VK_Attachments::Attachment attachment
            {
                m_EmissionMipmapViews[mipLevel],            // VkImageView
                format,                                     // VkFormat
                extentMipLevel,                             // VkExtent2D
                loadOp,                                     // VkAttachmentLoadOp
                VK_ATTACHMENT_STORE_OP_STORE,               // VkAttachmentStoreOp
                VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   // VkImageLayout
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL    // VkImageLayout
            };
            m_Attachments.Add(attachment);
        }
    }

    void VK_RenderSystemBloom::CreateRenderPass()
    {
        // use miplevel 1 (rather than miplevel 0) as it has VK_ATTACHMENT_LOAD_OP_CLEAR
        const uint mipLevelForAttachmentDescription = 1;
        VK_Attachments::Attachment attachment = m_Attachments.Get(mipLevelForAttachmentDescription); 
        m_RenderPass = std::make_unique<VK_BloomRenderPass>(attachment);
    }


    // this function creates a frame buffer for each downsampled image
    // for example if BLOOM_MIP_LEVELS == 4, then it creates 3 frame buffers
    void VK_RenderSystemBloom::CreateFrameBuffers()
    {
        auto renderPass = m_RenderPass->Get();
        for (uint index = 0; index < VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES; ++index)
        {
            auto attachment = m_Attachments[index + 1];  // start with mip level 1 (rather than mip level 0)
            m_Framebuffers[index] = std::make_unique<VK_BloomFrameBuffer>(attachment, renderPass);
        }
    }

    void VK_RenderSystemBloom::CreateBloomDescriptorSetLayout()
    {
        // sampler for mipmaps of g-buffer emission image
        // individual mip level will be accessed with 'textureLOD()'
        VK_DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT); 
        m_BloomDescriptorSetLayout = descriptorSetLayoutBuilder.Build();
    }

    void VK_RenderSystemBloom::CreateDescriptorSet()
    {
        VK_DescriptorWriter descriptorWriter(*m_BloomDescriptorSetLayout, m_DescriptorPool);

        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = static_cast<float>(VK_RenderSystemBloom::NUMBER_OF_MIPMAPS);
        samplerCreateInfo.maxAnisotropy = 4.0;
        samplerCreateInfo.anisotropyEnable = VK_TRUE;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        {
            auto result = vkCreateSampler(VK_Core::m_Device->Device(), &samplerCreateInfo, nullptr, &m_Sampler);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create sampler!");
            }
        }
        
        VkDescriptorImageInfo descriptorImageInfo {};
        descriptorImageInfo.sampler     = m_Sampler;
        descriptorImageInfo.imageView   = m_EmissionView;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        descriptorWriter.WriteImage(0, descriptorImageInfo);
        descriptorWriter.Build(m_BloomDescriptorSet);
    }

    // up & down share the same layout
    void VK_RenderSystemBloom::CreateBloomPipelinesLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataBloom);

        VkPipelineLayoutCreateInfo bloomPipelineLayoutInfo{};
        bloomPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        bloomPipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayout.size());
        bloomPipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
        bloomPipelineLayoutInfo.pushConstantRangeCount = 1;
        bloomPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(VK_Core::m_Device->Device(), &bloomPipelineLayoutInfo, nullptr, &m_BloomPipelineLayout) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemBloom::CreateBloomPipelines()
    {
        ASSERT(m_BloomPipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = m_RenderPass->Get();
        pipelineConfig.pipelineLayout = m_BloomPipelineLayout;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.subpass = 0;
        pipelineConfig.m_BindingDescriptions.clear();   // this pipeline is not using vertices
        pipelineConfig.m_AttributeDescriptions.clear();

        // down
        m_BloomPipelineDown = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin-int/bloomDown.vert.spv",
            "bin-int/bloomDown.frag.spv",
            pipelineConfig
        );

        // up
        m_BloomPipelineUp = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin-int/bloomUp.vert.spv",
            "bin-int/bloomUp.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystemBloom::RenderBloom(const VK_FrameInfo& frameInfo)
    {
        { // common
            std::vector<VkDescriptorSet> descriptorSets =
            {
                frameInfo.m_GlobalDescriptorSet,
                m_BloomDescriptorSet
            };

            vkCmdBindDescriptorSets
            (
                frameInfo.m_CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_BloomPipelineLayout,      // VkPipelineLayout layout
                0,                          // uint32_t         firstSet
                descriptorSets.size(),      // uint32_t         descriptorSetCount
                descriptorSets.data(),      // VkDescriptorSet* pDescriptorSets
                0,                          // uint32_t         dynamicOffsetCount
                nullptr                     // const uint32_t*  pDynamicOffsets
            );
        }

        // down -------------------------------------------------------------------------------------------------------------
        m_BloomPipelineDown->Bind(frameInfo.m_CommandBuffer);

        // sample from mip level 0 to mip level NUMBER_OF_MIPMAPS - 2
        // e.g. if NUMBER_OF_MIPMAPS == 4, then sample from 0, 1, and 2
        for (uint mipLevel = 0; mipLevel < VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES; ++mipLevel)
        {
            
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_RenderPass->Get();
            renderPassInfo.framebuffer = m_Framebuffers[mipLevel]->Get();

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_Framebuffers[mipLevel]->GetExtent();

            VkClearValue clearValue{};
            clearValue.color = {0.50f, 0.30f, 0.70f, 1.0f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearValue;

            vkCmdBeginRenderPass(frameInfo.m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            VK_PushConstantDataBloom push{};

            push.m_SrcResolution = glm::vec2(m_ExtendMipLevel0.width << mipLevel, m_ExtendMipLevel0.height << mipLevel);
            push.m_FilterRadius  = m_FilterRadius;
            push.m_ImageViewID   = mipLevel;

            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                m_BloomPipelineLayout,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataBloom),
                &push);

            vkCmdDraw
            (
                frameInfo.m_CommandBuffer,
                3,      // vertexCount
                1,      // instanceCount
                0,      // firstVertex
                0       // firstInstance
            );
            vkCmdEndRenderPass(frameInfo.m_CommandBuffer);
        }

        // up ---------------------------------------------------------------------------------------------------------------
        m_BloomPipelineUp->Bind(frameInfo.m_CommandBuffer);

        // sample from mip level NUMBER_OF_MIPMAPS-1 to mip level 1
        // e.g. if NUMBER_OF_MIPMAPS == 4, then sample from 3, 2, and 1
        for (uint mipLevel = VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES; mipLevel > 0; --mipLevel)
        {
            
        }
    }
}
