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
    VK_RenderSystemBloom::VK_RenderSystemBloom(VK_RenderPass const& renderPass3D)
        : m_RenderPass3D{renderPass3D}, m_FilterRadius{0.001}
    {
        m_ExtentMipLevel0 = m_RenderPass3D.GetExtent();

        // render pass and frame buffers
        CreateImageViews();
        CreateAttachments();
        CreateRenderPasses();     // up and down renderpass
        CreateFrameBuffersDown(); // use 'NUMBER_OF_MIPMAPS-1' frame buffers for downsampling
        CreateFrameBuffersUp();   // use 'NUMBER_OF_MIPMAPS-1' frame buffers for upsampling

        // pipelines
        CreateBloomDescriptorSetLayout();
        CreateDescriptorSet(); // use one descriptor set
        CreateBloomPipelinesLayout();
        CreateBloomPipelines(); // use two pipelines (up pipeline and down pipeline)
    }

    VK_RenderSystemBloom::~VK_RenderSystemBloom()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_BloomPipelineLayout, nullptr);

        for (uint mipLevel = 0; mipLevel < VK_RenderSystemBloom::NUMBER_OF_MIPMAPS; ++mipLevel)
        {
            vkDestroyImageView(VK_Core::m_Device->Device(), m_EmissionMipmapViews[mipLevel], nullptr);
        }

        vkDestroySampler(VK_Core::m_Device->Device(), m_Sampler, nullptr);
    }

    void VK_RenderSystemBloom::CreateImageViews()
    {
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

            auto result =
                vkCreateImageView(VK_Core::m_Device->Device(), &viewInfo, nullptr, &m_EmissionMipmapViews[mipLevel]);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }
    }

    void VK_RenderSystemBloom::CreateAttachments()
    {

        // attachments: targets to render into

        VkFormat format = m_RenderPass3D.GetFormatEmission();
        VkExtent2D extent = m_RenderPass3D.GetExtent();

        { // down
            // iterate from mip 1 (first image to down sample into) to the last mip;
            // the level 1 mip image and following mip levels have to be cleared
            //
            //  --> VK_ATTACHMENT_LOAD_OP_CLEAR
            //
            // e.g. if BLOOM_MIP_LEVELS == 4, then use mip level 1, 2, 3
            // so that we downsample mip 0 into mip 1 (== render target), etc.
            // (the g-buffer level zero image must not be cleared)
            // before the pass: VK_IMAGE_LAYOUT_UNDEFINED
            // after the pass VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            // during the pass: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            uint mipLevel = 1;
            for (uint index = 0; index < VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES; ++index)
            {
                VkExtent2D extentMipLevel{extent.width >> mipLevel, extent.height >> mipLevel};

                VK_Attachments::Attachment attachment{
                    m_EmissionMipmapViews[mipLevel],          // VkImageView         m_ImageView;
                    format,                                   // VkFormat            m_Format;
                    extentMipLevel,                           // VkExtent2D          m_Extent;
                    VK_ATTACHMENT_LOAD_OP_CLEAR,              // VkAttachmentLoadOp  m_LoadOp;
                    VK_ATTACHMENT_STORE_OP_STORE,             // VkAttachmentStoreOp m_StoreOp;
                    VK_IMAGE_LAYOUT_UNDEFINED,                // VkImageLayout       m_InitialLayout;
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // VkImageLayout       m_FinalLayout;
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // VkImageLayout       m_SubpassLayout;
                };
                m_AttachmentsDown.Add(attachment);
                ++mipLevel;
            }
        }

        { // up
            // iterate from second last mip to mip 0;
            // do not clear the images
            //
            //  --> VK_ATTACHMENT_LOAD_OP_LOAD
            //
            // e.g. if BLOOM_MIP_LEVELS == 4, then use mip level 2, 1, 0
            // so that we upsample the last mip (mip BLOOM_MIP_LEVELS-1) into (mip BLOOM_MIP_LEVELS-2)
            // before the pass: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            // after the pass VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            // during the pass: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            uint mipLevel = VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES - 1;
            for (uint index = 0; index < VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES; ++index)
            {
                VkExtent2D extentMipLevel{extent.width >> mipLevel, extent.height >> mipLevel};

                VK_Attachments::Attachment attachment{
                    m_EmissionMipmapViews[mipLevel],          // VkImageView         m_ImageView;
                    format,                                   // VkFormat            m_Format;
                    extentMipLevel,                           // VkExtent2D          m_Extent;
                    VK_ATTACHMENT_LOAD_OP_LOAD,               // VkAttachmentLoadOp  m_LoadOp;
                    VK_ATTACHMENT_STORE_OP_STORE,             // VkAttachmentStoreOp m_StoreOp;
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // VkImageLayout       m_InitialLayout;
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // VkImageLayout       m_FinalLayout;
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // VkImageLayout       m_SubpassLayout;
                };
                m_AttachmentsUp.Add(attachment);
                --mipLevel;
            }
        }
    }

    void VK_RenderSystemBloom::CreateRenderPasses()
    {
        { // down
            // use any image from m_AttachmentsDown since they all have VK_ATTACHMENT_LOAD_OP_CLEAR,
            // e.g., m_attachmentsDown[0] -> mip level 1
            VK_Attachments::Attachment& attachment = m_AttachmentsDown[0];
            m_RenderPassDown = std::make_unique<VK_BloomRenderPass>(attachment);
        }
        { // up
            // use any image from m_AttachmentsUp since they all have VK_ATTACHMENT_LOAD_OP_CLEAR,
            // m_attachmentsUp[0] -> mip level 'NUMBER_OF_MIPMAPS - 2'
            VK_Attachments::Attachment& attachment = m_AttachmentsUp[0];
            m_RenderPassUp = std::make_unique<VK_BloomRenderPass>(attachment);
        }
    }

    // this function creates a frame buffer for each downsampled image
    // for example if BLOOM_MIP_LEVELS == 4, then it creates 3 frame buffers
    void VK_RenderSystemBloom::CreateFrameBuffersDown()
    {
        auto renderPass = m_RenderPassDown->Get();
        for (uint index = 0; index < VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES; ++index)
        {
            auto& attachment = m_AttachmentsDown[index]; // m_attachmentsDown[0] -> mip level 1
            m_FramebuffersDown[index] = std::make_unique<VK_BloomFrameBuffer>(attachment, renderPass);
        }
    }

    // this function creates a frame buffer for each upsampled image
    // for example if BLOOM_MIP_LEVELS == 4, then it creates 3 frame buffers
    void VK_RenderSystemBloom::CreateFrameBuffersUp()
    {
        auto renderPass = m_RenderPassUp->Get();
        for (uint index = 0; index < VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES; ++index)
        {
            auto& attachment = m_AttachmentsUp[index]; // m_attachmentsUp[0] -> mip level [NUMBER_OF_MIPMAPS-2]
            m_FramebuffersUp[index] = std::make_unique<VK_BloomFrameBuffer>(attachment, renderPass);
        }
    }

    void VK_RenderSystemBloom::CreateBloomDescriptorSetLayout()
    {
        // sampler for mipmaps of g-buffer emission image
        // individual mip level will be accessed with 'textureLOD()'
        VK_DescriptorSetLayout::Builder descriptorSetLayoutBuilder;
        descriptorSetLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_BloomDescriptorSetsLayout = descriptorSetLayoutBuilder.Build();
    }

    void VK_RenderSystemBloom::CreateDescriptorSet()
    {
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext = nullptr;
        samplerCreateInfo.flags = 0;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.anisotropyEnable = VK_TRUE;
        samplerCreateInfo.maxAnisotropy = 16.0;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

        {
            auto result = vkCreateSampler(VK_Core::m_Device->Device(), &samplerCreateInfo, nullptr, &m_Sampler);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create sampler!");
            }
        }

        VK_DescriptorWriter descriptorWriter(*m_BloomDescriptorSetsLayout);

        for (uint mipLevel = 0; mipLevel < VK_RenderSystemBloom::NUMBER_OF_MIPMAPS; ++mipLevel)
        {
            VkDescriptorImageInfo descriptorImageInfo{};
            descriptorImageInfo.sampler = m_Sampler;
            descriptorImageInfo.imageView = m_EmissionMipmapViews[mipLevel];
            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            descriptorWriter.WriteImage(0, descriptorImageInfo);
            descriptorWriter.Build(m_BloomDescriptorSets[mipLevel]);
        }
    }

    // up & down share the same layout
    void VK_RenderSystemBloom::CreateBloomPipelinesLayout()
    {
        VkDescriptorSetLayout const& descriptorSetLayout = m_BloomDescriptorSetsLayout->GetDescriptorSetLayout();
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataBloom);

        VkPipelineLayoutCreateInfo bloomPipelineLayoutInfo{};
        bloomPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        bloomPipelineLayoutInfo.setLayoutCount = 1;
        bloomPipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        bloomPipelineLayoutInfo.pushConstantRangeCount = 1;
        bloomPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        auto result =
            vkCreatePipelineLayout(VK_Core::m_Device->Device(), &bloomPipelineLayoutInfo, nullptr, &m_BloomPipelineLayout);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemBloom::CreateBloomPipelines()
    {
        CORE_ASSERT(m_BloomPipelineLayout != nullptr, "m_BloomPipelineLayout is null");

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = m_RenderPassDown->Get();
        pipelineConfig.pipelineLayout = m_BloomPipelineLayout;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.colorBlendAttachment.blendEnable = VK_FALSE;
        pipelineConfig.subpass = 0;
        pipelineConfig.m_BindingDescriptions.clear(); // this pipeline is not using vertices
        pipelineConfig.m_AttributeDescriptions.clear();

        // down
        m_BloomPipelineDown = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/bloomDown.vert.spv",
                                                            "bin-int/bloomDown.frag.spv", pipelineConfig);

        // up
        pipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
        pipelineConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        pipelineConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        pipelineConfig.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        m_BloomPipelineUp = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/bloomUp.vert.spv",
                                                          "bin-int/bloomUp.frag.spv", pipelineConfig);
    }

    void VK_RenderSystemBloom::SetViewPort(const VK_FrameInfo& frameInfo, VkExtent2D const& extent)
    {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, extent};
        vkCmdSetViewport(frameInfo.m_CommandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(frameInfo.m_CommandBuffer, 0, 1, &scissor);
    }

    void VK_RenderSystemBloom::BeginRenderPass(VK_FrameInfo const& frameInfo, VK_BloomRenderPass* renderpass,
                                               VK_BloomFrameBuffer* framebuffer)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderpass->Get();
        renderPassInfo.framebuffer = framebuffer->Get();

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = framebuffer->GetExtent();

        VkClearValue clearValue{};
        clearValue.color = {{0.50f, 0.30f, 0.70f, 1.0f}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(frameInfo.m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VK_RenderSystemBloom::RenderBloom(VK_FrameInfo const& frameInfo)
    {
        // down -------------------------------------------------------------------------------------------------------------
        m_BloomPipelineDown->Bind(frameInfo.m_CommandBuffer);

        // sample from mip level 0 to mip level NUMBER_OF_MIPMAPS - 2
        // render into mip level 1 to mip level NUMBER_OF_MIPMAPS - 1
        // e.g. if NUMBER_OF_MIPMAPS == 4, then sample from 0, 1, 2 and render into 1, 2, 3
        uint mipLevel = 0;
        for (uint index = 0; index < VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES; ++index)
        {
            BeginRenderPass(frameInfo, m_RenderPassDown.get(), m_FramebuffersDown[index].get());
            VkExtent2D extent{m_ExtentMipLevel0.width >> (mipLevel + 1), m_ExtentMipLevel0.height >> (mipLevel + 1)};
            SetViewPort(frameInfo, extent);
            VK_PushConstantDataBloom push{};

            push.m_SrcResolution = glm::vec2(extent.width, extent.height);
            push.m_FilterRadius = m_FilterRadius;

            vkCmdPushConstants(frameInfo.m_CommandBuffer, m_BloomPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(VK_PushConstantDataBloom), &push);

            vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_BloomPipelineLayout,            // VkPipelineLayout layout
                                    0,                                // uint32_t         firstSet
                                    1,                                // uint32_t         descriptorSetCount
                                    &m_BloomDescriptorSets[mipLevel], // VkDescriptorSet* pDescriptorSets
                                    0,                                // uint32_t         dynamicOffsetCount
                                    nullptr                           // const uint32_t*  pDynamicOffsets
            );

            vkCmdDraw(frameInfo.m_CommandBuffer,
                      3, // vertexCount
                      1, // instanceCount
                      0, // firstVertex
                      0  // firstInstance
            );
            vkCmdEndRenderPass(frameInfo.m_CommandBuffer);
            ++mipLevel;
        }

        // up ---------------------------------------------------------------------------------------------------------------
        m_BloomPipelineUp->Bind(frameInfo.m_CommandBuffer);

        // sample from mip level mip level NUMBER_OF_MIPMAPS - 1 to 1
        // render into mip level NUMBER_OF_MIPMAPS - 2 to mip level 0
        // e.g. if NUMBER_OF_MIPMAPS == 4, then sample from 3, 2, 1 and render into 2, 1, 0
        mipLevel = VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES;
        for (uint index = 0; index < VK_RenderSystemBloom::NUMBER_OF_DOWNSAMPLED_IMAGES; ++index)
        {
            BeginRenderPass(frameInfo, m_RenderPassUp.get(), m_FramebuffersUp[index].get());
            VkExtent2D extent{m_ExtentMipLevel0.width >> (mipLevel - 1), m_ExtentMipLevel0.height >> (mipLevel - 1)};
            SetViewPort(frameInfo, extent);
            VK_PushConstantDataBloom push{};

            push.m_SrcResolution = glm::vec2(extent.width, extent.height);
            push.m_FilterRadius = m_FilterRadius;

            vkCmdPushConstants(frameInfo.m_CommandBuffer, m_BloomPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(VK_PushConstantDataBloom), &push);

            vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_BloomPipelineLayout,            // VkPipelineLayout layout
                                    0,                                // uint32_t         firstSet
                                    1,                                // uint32_t         descriptorSetCount
                                    &m_BloomDescriptorSets[mipLevel], // VkDescriptorSet* pDescriptorSets
                                    0,                                // uint32_t         dynamicOffsetCount
                                    nullptr                           // const uint32_t*  pDynamicOffsets
            );

            vkCmdDraw(frameInfo.m_CommandBuffer,
                      3, // vertexCount
                      1, // instanceCount
                      0, // firstVertex
                      0  // firstInstance
            );
            vkCmdEndRenderPass(frameInfo.m_CommandBuffer);
            --mipLevel;
        }
    }
} // namespace GfxRenderEngine
