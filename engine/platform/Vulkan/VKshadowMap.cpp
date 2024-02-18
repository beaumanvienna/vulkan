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

#include "engine.h"

#include "auxiliary/instrumentation.h"
#include "platform/Vulkan/shadowMapping.h"

#include "VKshadowMap.h"

namespace GfxRenderEngine
{

    VK_ShadowMap::VK_ShadowMap(int width)
    {
        m_ShadowMapExtent.width  = width;
        m_ShadowMapExtent.height = width;
        m_Device = VK_Core::m_Device;
        m_DepthFormat = m_Device->FindDepthFormat();

        CreateShadowRenderPass();
        CreateShadowDepthResources();
        CreateShadowFramebuffer();
    }

    VK_ShadowMap::~VK_ShadowMap()
    {
        VK_Core::m_Device->DestroyImage(m_ShadowDepthImage);
        VK_Core::m_Device->DestroySampler(m_ShadowDepthSampler);
        vkDestroyRenderPass(m_Device->Device(), m_ShadowRenderPass, nullptr);
    }

    void VK_ShadowMap::CreateShadowRenderPass()
    {
        // ATTACHMENT_DEPTH
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = m_DepthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_ImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        depthAttachment.finalLayout = m_ImageLayout;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = static_cast<uint>(ShadowRenderTargets::ATTACHMENT_DEPTH);
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // subpass
        VkSubpassDescription subpassShadow = {};
        subpassShadow.flags = 0;
        subpassShadow.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassShadow.inputAttachmentCount = 0;
        subpassShadow.pInputAttachments = nullptr;
        subpassShadow.colorAttachmentCount = 0;
        subpassShadow.pColorAttachments = nullptr;
        subpassShadow.pResolveAttachments = nullptr;
        subpassShadow.pDepthStencilAttachment = &depthAttachmentRef;
        subpassShadow.preserveAttachmentCount = 0;
        subpassShadow.pPreserveAttachments = nullptr;

        // dependencies
        constexpr uint NUMBER_OF_DEPENDENCIES = 2;
        std::array<VkSubpassDependency, NUMBER_OF_DEPENDENCIES> dependencies;

        dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;                                // Index of the render pass being depended upon by dstSubpass
        dependencies[0].dstSubpass      = static_cast<uint>(SubPassesShadow::SUBPASS_SHADOW); // The index of the render pass depending on srcSubpass
        dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;              // What pipeline stage must have completed for the dependency
        dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;         // What pipeline stage is waiting on the dependency
        dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;                          // What access scopes influence the dependency
        dependencies[0].dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;       // What access scopes are waiting on the dependency
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;                        // Other configuration about the dependency

        dependencies[1].srcSubpass      = static_cast<uint>(SubPassesShadow::SUBPASS_SHADOW);
        dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // render pass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint>(ShadowRenderTargets::NUMBER_OF_ATTACHMENTS);
        renderPassInfo.pAttachments = &depthAttachment;
        renderPassInfo.subpassCount = static_cast<uint>(SubPassesShadow::NUMBER_OF_SUBPASSES);
        renderPassInfo.pSubpasses = &subpassShadow;
        renderPassInfo.dependencyCount = NUMBER_OF_DEPENDENCIES;
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(m_Device->Device(), &renderPassInfo, nullptr, &m_ShadowRenderPass) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create render pass!");
        }
    }

    void VK_ShadowMap::CreateShadowDepthResources()
    {
        // image
        m_ShadowDepthImage = VK_Core::m_Device->CreateImage({
            .format = static_cast<Format>(m_DepthFormat),
            .size = { static_cast<uint>(m_ShadowMapExtent.width), static_cast<uint>(m_ShadowMapExtent.height), 1 },
            .usage = ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | ImageUsageFlagBits::SHADER_SAMPLED
            });

        //// sampler
        m_ShadowDepthSampler = VK_Core::m_Device->CreateSampler({
            .enableCompare = true,
            .compareOp = CompareOp::LESS,
            .maxLod = 1.0f,
            .borderColor = BorderColor::FLOAT_OPAQUE_WHITE
            });

        m_DescriptorImageInfo.sampler     = VK_Core::m_Device->GetSamplerSlot(m_ShadowDepthSampler).vkSampler;
        m_DescriptorImageInfo.imageView   = VK_Core::m_Device->GetImageViewSlot(m_ShadowDepthImage.defaultView()).vkImageView;
        m_DescriptorImageInfo.imageLayout = m_ImageLayout;
    }

    void VK_ShadowMap::CreateShadowFramebuffer()
    {
        VkImageView imageView = VK_Core::m_Device->GetImageViewSlot(m_ShadowDepthImage.defaultView()).vkImageView;
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_ShadowRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint>(ShadowRenderTargets::NUMBER_OF_ATTACHMENTS);
        framebufferInfo.pAttachments = &imageView;
        framebufferInfo.width = m_ShadowMapExtent.width;
        framebufferInfo.height = m_ShadowMapExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
                m_Device->Device(),
                &framebufferInfo,
                nullptr,
                &m_ShadowFramebuffer) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create shadow framebuffer!");
        }
    }
}
