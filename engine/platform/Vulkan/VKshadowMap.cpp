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
        m_ShadowMapExtent.width = width;
        m_ShadowMapExtent.height = width;
        m_Device = VK_Core::m_Device;
        m_DepthFormat = m_Device->FindDepthFormat();

        CreateShadowRenderPass();
        CreateShadowDepthResources();
        CreateShadowFramebuffer();
    }

    VK_ShadowMap::~VK_ShadowMap()
    {
        vkDestroyImageView(m_Device->Device(), m_ShadowDepthImageView, nullptr);
        vkDestroyImage(m_Device->Device(), m_ShadowDepthImage, nullptr);
        vkFreeMemory(m_Device->Device(), m_ShadowDepthImageMemory, nullptr);
        vkDestroySampler(m_Device->Device(), m_ShadowDepthSampler, nullptr);
        vkDestroyRenderPass(m_Device->Device(), m_ShadowRenderPass, nullptr);
        vkDestroyFramebuffer(m_Device->Device(), m_ShadowFramebuffer, nullptr);
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

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // Index of the render pass being depended upon by dstSubpass
        dependencies[0].dstSubpass =
            static_cast<uint>(SubPassesShadow::SUBPASS_SHADOW); // The index of the render pass depending on srcSubpass
        dependencies[0].srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // What pipeline stage must have completed for the dependency
        dependencies[0].dstStageMask =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;            // What pipeline stage is waiting on the dependency
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // What access scopes influence the dependency
        dependencies[0].dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;              // What access scopes are waiting on the dependency
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // Other configuration about the dependency

        dependencies[1].srcSubpass = static_cast<uint>(SubPassesShadow::SUBPASS_SHADOW);
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
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

        auto result = vkCreateRenderPass(m_Device->Device(), &renderPassInfo, nullptr, &m_ShadowRenderPass);
        if (result != VK_SUCCESS)
        {
            m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create render pass!");
        }
    }

    void VK_ShadowMap::CreateShadowDepthResources()
    {
        // image
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_ShadowMapExtent.width;
        imageInfo.extent.height = m_ShadowMapExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = m_DepthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        m_Device->CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_ShadowDepthImage,
                                      m_ShadowDepthImageMemory);

        // image view
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_ShadowDepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_DepthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        {
            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_ShadowDepthImageView);
            if (result != VK_SUCCESS)
            {
                m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create texture image view! (CreateShadowDepthResources)");
            }
        }

        // sampler
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.compareEnable = VK_TRUE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_LESS;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1;
        samplerCreateInfo.maxAnisotropy = 1.0;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        {
            auto result = vkCreateSampler(m_Device->Device(), &samplerCreateInfo, nullptr, &m_ShadowDepthSampler);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create sampler!");
            }
        }

        m_DescriptorImageInfo.sampler = m_ShadowDepthSampler;
        m_DescriptorImageInfo.imageView = m_ShadowDepthImageView;
        m_DescriptorImageInfo.imageLayout = m_ImageLayout;
    }

    void VK_ShadowMap::CreateShadowFramebuffer()
    {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_ShadowRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint>(ShadowRenderTargets::NUMBER_OF_ATTACHMENTS);
        framebufferInfo.pAttachments = &m_ShadowDepthImageView;
        framebufferInfo.width = m_ShadowMapExtent.width;
        framebufferInfo.height = m_ShadowMapExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_Device->Device(), &framebufferInfo, nullptr, &m_ShadowFramebuffer) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create shadow framebuffer!");
        }
    }
} // namespace GfxRenderEngine
