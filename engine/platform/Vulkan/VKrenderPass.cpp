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

#include "engine.h"

#include "auxiliary/instrumentation.h"

#include "shadowMapping.h"
#include "VKswapChain.h"
#include "VKshadowMap.h"
#include "VKrenderPass.h"
#include "systems/bloom/VKbloomRenderSystem.h"

namespace GfxRenderEngine
{

    VK_RenderPass::VK_RenderPass(VK_SwapChain* swapChain)
        : m_RenderPassExtent{swapChain->GetSwapChainExtent()}, m_SwapChain{swapChain}
    {
        m_Device = VK_Core::m_Device;

        m_DepthFormat = m_Device->FindDepthFormat();
        m_BufferPositionFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        m_BufferNormalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        m_BufferColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
        m_BufferMaterialFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        m_BufferEmissionFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        Create3DRenderPass();
        CreatePostProcessingRenderPass();
        CreateGUIRenderPass();

        CreateColorAttachmentResources();
        CreateDepthResources();

        CreateGBufferImages();
        CreateGBufferImageViews();

        Create3DFramebuffers();
        CreatePostProcessingFramebuffers();
        CreateGUIFramebuffers();
    }

    VK_RenderPass::~VK_RenderPass()
    {
        vkDestroyImageView(m_Device->Device(), m_DepthImageView, nullptr);
        vkDestroyImage(m_Device->Device(), m_DepthImage, nullptr);
        vkFreeMemory(m_Device->Device(), m_DepthImageMemory, nullptr);

        vkDestroyImageView(m_Device->Device(), m_ColorAttachmentView, nullptr);
        vkDestroyImage(m_Device->Device(), m_ColorAttachmentImage, nullptr);
        vkFreeMemory(m_Device->Device(), m_ColorAttachmentImageMemory, nullptr);

        for (auto framebuffer : m_3DFramebuffers)
        {
            vkDestroyFramebuffer(m_Device->Device(), framebuffer, nullptr);
        }
        for (auto framebuffer : m_PostProcessingFramebuffers)
        {
            vkDestroyFramebuffer(m_Device->Device(), framebuffer, nullptr);
        }
        for (auto framebuffer : m_GUIFramebuffers)
        {
            vkDestroyFramebuffer(m_Device->Device(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(m_Device->Device(), m_3DRenderPass, nullptr);
        vkDestroyRenderPass(m_Device->Device(), m_PostProcessingRenderPass, nullptr);
        vkDestroyRenderPass(m_Device->Device(), m_GUIRenderPass, nullptr);

        DestroyGBuffers();
    }

    void VK_RenderPass::CreateColorAttachmentResources()
    {
        VkFormat format = m_SwapChain->GetSwapChainImageFormat();

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_RenderPassExtent.width;
        imageInfo.extent.height = m_RenderPassExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        m_Device->CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_ColorAttachmentImage,
                                      m_ColorAttachmentImageMemory);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_ColorAttachmentImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_ColorAttachmentView);
        if (result != VK_SUCCESS)
        {
            m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create texture image view!");
        }
    }

    void VK_RenderPass::CreateDepthResources()
    {
        VkFormat depthFormat = m_Device->FindDepthFormat();
        m_DepthFormat = depthFormat;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_RenderPassExtent.width;
        imageInfo.extent.height = m_RenderPassExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        m_Device->CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_DepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_DepthImageView);
        if (result != VK_SUCCESS)
        {
            m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create texture image view!");
        }
    }

    void VK_RenderPass::Create3DFramebuffers()
    {
        m_3DFramebuffers.resize(m_SwapChain->ImageCount());
        for (size_t i = 0; i < m_SwapChain->ImageCount(); i++)
        {
            std::array<VkImageView, static_cast<uint>(RenderTargets3D::NUMBER_OF_ATTACHMENTS)> attachments = {
                m_ColorAttachmentView, m_DepthImageView,      m_GBufferPositionView, m_GBufferNormalView,
                m_GBufferColorView,    m_GBufferMaterialView, m_GBufferEmissionView};

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_3DRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint>(RenderTargets3D::NUMBER_OF_ATTACHMENTS);
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_RenderPassExtent.width;
            framebufferInfo.height = m_RenderPassExtent.height;
            framebufferInfo.layers = 1;

            auto result = vkCreateFramebuffer(m_Device->Device(), &framebufferInfo, nullptr, &m_3DFramebuffers[i]);
            if (result != VK_SUCCESS)
            {
                m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create framebuffer! Create3DFramebuffers()");
            }
        }
    }

    void VK_RenderPass::CreatePostProcessingFramebuffers()
    {
        m_PostProcessingFramebuffers.resize(m_SwapChain->ImageCount());
        for (size_t i = 0; i < m_SwapChain->ImageCount(); i++)
        {
            std::array<VkImageView, static_cast<uint>(RenderTargetsPostProcessing::NUMBER_OF_ATTACHMENTS)> attachments = {
                m_SwapChain->GetImageView(i), m_ColorAttachmentView, m_GBufferEmissionView};

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_PostProcessingRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint>(RenderTargetsPostProcessing::NUMBER_OF_ATTACHMENTS);
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_RenderPassExtent.width;
            framebufferInfo.height = m_RenderPassExtent.height;
            framebufferInfo.layers = 1;

            auto result =
                vkCreateFramebuffer(m_Device->Device(), &framebufferInfo, nullptr, &m_PostProcessingFramebuffers[i]);
            if (result != VK_SUCCESS)
            {
                m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create framebuffer! CreatePostProcessingFramebuffers()");
            }
        }
    }

    void VK_RenderPass::CreateGUIFramebuffers()
    {
        m_GUIFramebuffers.resize(m_SwapChain->ImageCount());
        for (size_t i = 0; i < m_SwapChain->ImageCount(); i++)
        {
            std::array<VkImageView, static_cast<uint>(RenderTargetsGUI::NUMBER_OF_ATTACHMENTS)> attachments = {
                m_SwapChain->GetImageView(i)};

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_GUIRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint>(RenderTargetsGUI::NUMBER_OF_ATTACHMENTS);
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_RenderPassExtent.width;
            framebufferInfo.height = m_RenderPassExtent.height;
            framebufferInfo.layers = 1;

            auto result = vkCreateFramebuffer(m_Device->Device(), &framebufferInfo, nullptr, &m_GUIFramebuffers[i]);
            if (result != VK_SUCCESS)
            {
                m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create framebuffer! CreateGUIFramebuffers()");
            }
        }
    }

    void VK_RenderPass::CreateGBufferImages()
    {
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_RenderPassExtent.width;
            imageInfo.extent.height = m_RenderPassExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = m_BufferPositionFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_Device->CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_GBufferPositionImage,
                                          m_GBufferPositionImageMemory);
        }

        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_RenderPassExtent.width;
            imageInfo.extent.height = m_RenderPassExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = m_BufferNormalFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_Device->CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_GBufferNormalImage,
                                          m_GBufferNormalImageMemory);
        }

        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_RenderPassExtent.width;
            imageInfo.extent.height = m_RenderPassExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = m_BufferColorFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_Device->CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_GBufferColorImage,
                                          m_GBufferColorImageMemory);
        }

        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_RenderPassExtent.width;
            imageInfo.extent.height = m_RenderPassExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = m_BufferMaterialFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_Device->CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_GBufferMaterialImage,
                                          m_GBufferMaterialImageMemory);
        }

        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_RenderPassExtent.width;
            imageInfo.extent.height = m_RenderPassExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = VK_RenderSystemBloom::NUMBER_OF_MIPMAPS;
            imageInfo.arrayLayers = 1;
            imageInfo.format = m_BufferEmissionFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage =
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_Device->CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_GBufferEmissionImage,
                                          m_GBufferEmissionImageMemory);
        }
    }

    void VK_RenderPass::CreateGBufferImageViews()
    {
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_GBufferPositionImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_BufferPositionFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_GBufferPositionView);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }

        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_GBufferNormalImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_BufferNormalFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_GBufferNormalView);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }

        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_GBufferColorImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_BufferColorFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_GBufferColorView);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }

        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_GBufferMaterialImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_BufferMaterialFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_GBufferMaterialView);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }

        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_GBufferEmissionImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_BufferEmissionFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_GBufferEmissionView);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }
    }

    void VK_RenderPass::Create3DRenderPass()
    {
        // ATTACHMENT_COLOR
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_SwapChain->GetSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_COLOR);
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // ATTACHMENT_DEPTH
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = m_Device->FindDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_DEPTH);
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // ATTACHMENT_GBUFFER_POSITION
        VkAttachmentDescription gBufferPositionAttachment = {};
        gBufferPositionAttachment.format = m_BufferPositionFormat;
        gBufferPositionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        gBufferPositionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gBufferPositionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gBufferPositionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gBufferPositionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gBufferPositionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gBufferPositionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference gBufferPositionAttachmentRef = {};
        gBufferPositionAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_POSITION);
        gBufferPositionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference gBufferPositionInputAttachmentRef = {};
        gBufferPositionInputAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_POSITION);
        gBufferPositionInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // ATTACHMENT_GBUFFER_NORMAL
        VkAttachmentDescription gBufferNormalAttachment = {};
        gBufferNormalAttachment.format = m_BufferNormalFormat;
        gBufferNormalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        gBufferNormalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gBufferNormalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gBufferNormalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gBufferNormalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gBufferNormalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gBufferNormalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference gBufferNormalAttachmentRef = {};
        gBufferNormalAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_NORMAL);
        gBufferNormalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference gBufferNormalInputAttachmentRef = {};
        gBufferNormalInputAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_NORMAL);
        gBufferNormalInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // ATTACHMENT_GBUFFER_COLOR
        VkAttachmentDescription gBufferColorAttachment = {};
        gBufferColorAttachment.format = m_BufferColorFormat;
        gBufferColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        gBufferColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gBufferColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gBufferColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gBufferColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gBufferColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gBufferColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference gBufferColorAttachmentRef = {};
        gBufferColorAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_COLOR);
        gBufferColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference gBufferColorInputAttachmentRef = {};
        gBufferColorInputAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_COLOR);
        gBufferColorInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // ATTACHMENT_GBUFFER_MATERIAL
        VkAttachmentDescription gBufferMaterialAttachment = {};
        gBufferMaterialAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        gBufferMaterialAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        gBufferMaterialAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gBufferMaterialAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gBufferMaterialAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gBufferMaterialAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gBufferMaterialAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gBufferMaterialAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference gBufferMaterialAttachmentRef = {};
        gBufferMaterialAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_MATERIAL);
        gBufferMaterialAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference gBufferMaterialInputAttachmentRef = {};
        gBufferMaterialInputAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_MATERIAL);
        gBufferMaterialInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // ATTACHMENT_GBUFFER_EMISSION
        VkAttachmentDescription gBufferEmissionAttachment = {};
        gBufferEmissionAttachment.format = m_BufferEmissionFormat;
        gBufferEmissionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        gBufferEmissionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gBufferEmissionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gBufferEmissionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gBufferEmissionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gBufferEmissionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gBufferEmissionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference gBufferEmissionAttachmentRef = {};
        gBufferEmissionAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_EMISSION);
        gBufferEmissionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference gBufferEmissionInputAttachmentRef = {};
        gBufferEmissionInputAttachmentRef.attachment = static_cast<uint>(RenderTargets3D::ATTACHMENT_GBUFFER_EMISSION);
        gBufferEmissionInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // geometry pass
        std::array<VkAttachmentReference, NUMBER_OF_GBUFFER_ATTACHMENTS> gBufferAttachments = {
            gBufferPositionAttachmentRef, gBufferNormalAttachmentRef, gBufferColorAttachmentRef,
            gBufferMaterialAttachmentRef, gBufferEmissionAttachmentRef};

        VkSubpassDescription subpassGeometry = {};
        subpassGeometry.flags = 0;
        subpassGeometry.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassGeometry.inputAttachmentCount = 0;
        subpassGeometry.pInputAttachments = nullptr;
        subpassGeometry.colorAttachmentCount = NUMBER_OF_GBUFFER_ATTACHMENTS;
        subpassGeometry.pColorAttachments = gBufferAttachments.data();
        subpassGeometry.pResolveAttachments = nullptr;
        subpassGeometry.pDepthStencilAttachment = &depthAttachmentRef;
        subpassGeometry.preserveAttachmentCount = 0;
        subpassGeometry.pPreserveAttachments = nullptr;

        // lighting pass
        std::array<VkAttachmentReference, NUMBER_OF_GBUFFER_ATTACHMENTS> inputAttachments = {
            gBufferPositionInputAttachmentRef, gBufferNormalInputAttachmentRef, gBufferColorInputAttachmentRef,
            gBufferMaterialInputAttachmentRef, gBufferEmissionInputAttachmentRef};

        VkSubpassDescription subpassLighting = {};
        subpassLighting.flags = 0;
        subpassLighting.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassLighting.inputAttachmentCount = NUMBER_OF_GBUFFER_ATTACHMENTS;
        subpassLighting.pInputAttachments = inputAttachments.data();
        subpassLighting.colorAttachmentCount = 1;
        subpassLighting.pColorAttachments = &colorAttachmentRef;
        subpassLighting.pResolveAttachments = nullptr;
        subpassLighting.pDepthStencilAttachment = &depthAttachmentRef;
        subpassLighting.preserveAttachmentCount = 0;
        subpassLighting.pPreserveAttachments = nullptr;

        // transparency pass

        VkSubpassDescription subpassTransparency = {};
        subpassTransparency.flags = 0;
        subpassTransparency.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassTransparency.inputAttachmentCount = 0;
        subpassTransparency.pInputAttachments = nullptr;
        subpassTransparency.colorAttachmentCount = 1;
        subpassTransparency.pColorAttachments = &colorAttachmentRef;
        subpassTransparency.pResolveAttachments = nullptr;
        subpassTransparency.pDepthStencilAttachment = &depthAttachmentRef;
        subpassTransparency.preserveAttachmentCount = 0;
        subpassTransparency.pPreserveAttachments = nullptr;

        constexpr uint NUMBER_OF_DEPENDENCIES = 4;
        std::array<VkSubpassDependency, NUMBER_OF_DEPENDENCIES> dependencies;

        // lighting depends on geometry
        dependencies[0].srcSubpass =
            static_cast<uint>(SubPasses3D::SUBPASS_GEOMETRY); // Index of the render pass being depended upon by dstSubpass
        dependencies[0].dstSubpass =
            static_cast<uint>(SubPasses3D::SUBPASS_LIGHTING); // The index of the render pass depending on srcSubpass
        dependencies[0].srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // What pipeline stage must have completed for the dependency
        dependencies[0].dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // What pipeline stage is waiting on the dependency
        dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // What access scopes influence the dependency
        dependencies[0].dstAccessMask =
            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;                       // What access scopes are waiting on the dependency
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // Other configuration about the dependency

        // transparency depends on lighting
        dependencies[1].srcSubpass = static_cast<uint>(SubPasses3D::SUBPASS_LIGHTING);
        dependencies[1].dstSubpass = static_cast<uint>(SubPasses3D::SUBPASS_TRANSPARENCY);
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[2].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[2].dstSubpass = static_cast<uint>(SubPasses3D::SUBPASS_GEOMETRY);
        dependencies[2].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[2].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[2].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[3].srcSubpass = static_cast<uint>(SubPasses3D::SUBPASS_GEOMETRY);
        dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[3].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[3].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // render pass
        std::array<VkAttachmentDescription, static_cast<uint>(RenderTargets3D::NUMBER_OF_ATTACHMENTS)> attachments = {
            colorAttachment,        depthAttachment,           gBufferPositionAttachment, gBufferNormalAttachment,
            gBufferColorAttachment, gBufferMaterialAttachment, gBufferEmissionAttachment};
        std::array<VkSubpassDescription, static_cast<uint>(SubPasses3D::NUMBER_OF_SUBPASSES)> subpasses = {
            subpassGeometry, subpassLighting, subpassTransparency};

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint>(RenderTargets3D::NUMBER_OF_ATTACHMENTS);
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = static_cast<uint>(SubPasses3D::NUMBER_OF_SUBPASSES);
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = NUMBER_OF_DEPENDENCIES;
        renderPassInfo.pDependencies = dependencies.data();

        auto result = vkCreateRenderPass(m_Device->Device(), &renderPassInfo, nullptr, &m_3DRenderPass);
        if (result != VK_SUCCESS)
        {
            m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create render pass!");
        }
    }

    void VK_RenderPass::CreatePostProcessingRenderPass()
    {
        // ATTACHMENT_COLOR
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_SwapChain->GetSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = static_cast<uint>(RenderTargetsPostProcessing::ATTACHMENT_COLOR);
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // INPUT_ATTACHMENT_3DPASS_COLOR
        VkAttachmentDescription inputAttachment3DPassColor = {};
        inputAttachment3DPassColor.format = m_SwapChain->GetSwapChainImageFormat();
        inputAttachment3DPassColor.samples = VK_SAMPLE_COUNT_1_BIT;
        inputAttachment3DPassColor.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        inputAttachment3DPassColor.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        inputAttachment3DPassColor.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        inputAttachment3DPassColor.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        inputAttachment3DPassColor.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        inputAttachment3DPassColor.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // INPUT_ATTACHMENT_GBUFFER_EMISSION
        VkAttachmentDescription inputAttachmentgBufferEmission = {};
        inputAttachmentgBufferEmission.format = m_BufferEmissionFormat;
        inputAttachmentgBufferEmission.samples = VK_SAMPLE_COUNT_1_BIT;
        inputAttachmentgBufferEmission.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        inputAttachmentgBufferEmission.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        inputAttachmentgBufferEmission.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        inputAttachmentgBufferEmission.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        inputAttachmentgBufferEmission.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        inputAttachmentgBufferEmission.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // input attachments
        VkAttachmentReference gColorInputAttachmentRef = {};
        gColorInputAttachmentRef.attachment = static_cast<uint>(RenderTargetsPostProcessing::INPUT_ATTACHMENT_3DPASS_COLOR);
        gColorInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference gBufferEmissionInputAttachmentRef = {};
        gBufferEmissionInputAttachmentRef.attachment =
            static_cast<uint>(RenderTargetsPostProcessing::INPUT_ATTACHMENT_GBUFFER_EMISSION);
        gBufferEmissionInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        std::array<VkAttachmentReference, NUMBER_OF_POSTPROCESSING_INPUT_ATTACHMENTS> inputAttachments = {
            gColorInputAttachmentRef, gBufferEmissionInputAttachmentRef};

        // subpass
        VkSubpassDescription subpassPostProcessing = {};
        subpassPostProcessing.flags = 0;
        subpassPostProcessing.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassPostProcessing.inputAttachmentCount = NUMBER_OF_POSTPROCESSING_INPUT_ATTACHMENTS;
        subpassPostProcessing.pInputAttachments = inputAttachments.data();
        subpassPostProcessing.colorAttachmentCount = NUMBER_OF_POSTPROCESSING_OUPUT_ATTACHMENTS;
        subpassPostProcessing.pColorAttachments = &colorAttachmentRef;
        subpassPostProcessing.pResolveAttachments = nullptr;
        subpassPostProcessing.pDepthStencilAttachment = nullptr;
        subpassPostProcessing.preserveAttachmentCount = 0;
        subpassPostProcessing.pPreserveAttachments = nullptr;

        constexpr uint NUMBER_OF_DEPENDENCIES = 2;
        std::array<VkSubpassDependency, NUMBER_OF_DEPENDENCIES> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = static_cast<uint>(SubPassesPostProcessing::SUBPASS_BLOOM);
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = static_cast<uint>(SubPassesPostProcessing::SUBPASS_BLOOM);
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // render pass
        std::array<VkAttachmentDescription, static_cast<uint>(RenderTargetsPostProcessing::NUMBER_OF_ATTACHMENTS)>
            attachments = {colorAttachment, inputAttachment3DPassColor, inputAttachmentgBufferEmission};
        std::array<VkSubpassDescription, static_cast<uint>(SubPassesPostProcessing::NUMBER_OF_SUBPASSES)> subpasses = {
            subpassPostProcessing};

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint>(RenderTargetsPostProcessing::NUMBER_OF_ATTACHMENTS);
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = static_cast<uint>(SubPassesPostProcessing::NUMBER_OF_SUBPASSES);
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = NUMBER_OF_DEPENDENCIES;
        renderPassInfo.pDependencies = dependencies.data();

        auto result = vkCreateRenderPass(m_Device->Device(), &renderPassInfo, nullptr, &m_PostProcessingRenderPass);
        if (result != VK_SUCCESS)
        {
            m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create render pass!");
        }
    }

    void VK_RenderPass::CreateGUIRenderPass()
    {
        // ATTACHMENT_COLOR
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_SwapChain->GetSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = static_cast<uint>(RenderTargetsGUI::ATTACHMENT_COLOR);
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // subpass
        VkSubpassDescription subpassGUI = {};
        subpassGUI.flags = 0;
        subpassGUI.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassGUI.inputAttachmentCount = 0;
        subpassGUI.pInputAttachments = nullptr;
        subpassGUI.colorAttachmentCount = 1;
        subpassGUI.pColorAttachments = &colorAttachmentRef;
        subpassGUI.pResolveAttachments = nullptr;
        subpassGUI.pDepthStencilAttachment = nullptr;
        subpassGUI.preserveAttachmentCount = 0;
        subpassGUI.pPreserveAttachments = nullptr;

        constexpr uint NUMBER_OF_DEPENDENCIES = 2;
        std::array<VkSubpassDependency, NUMBER_OF_DEPENDENCIES> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = static_cast<uint>(SubPassesGUI::SUBPASS_GUI);
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = static_cast<uint>(SubPassesGUI::SUBPASS_GUI);
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // render pass
        std::array<VkAttachmentDescription, static_cast<uint>(RenderTargetsGUI::NUMBER_OF_ATTACHMENTS)> attachments = {
            colorAttachment};
        std::array<VkSubpassDescription, static_cast<uint>(SubPassesGUI::NUMBER_OF_SUBPASSES)> subpasses = {subpassGUI};

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint>(RenderTargetsGUI::NUMBER_OF_ATTACHMENTS);
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = static_cast<uint>(SubPassesGUI::NUMBER_OF_SUBPASSES);
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = NUMBER_OF_DEPENDENCIES;
        renderPassInfo.pDependencies = dependencies.data();

        auto result = vkCreateRenderPass(m_Device->Device(), &renderPassInfo, nullptr, &m_GUIRenderPass);
        if (result != VK_SUCCESS)
        {
            m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create render pass! CreateGUIRenderPass()");
        }
    }

    void VK_RenderPass::DestroyGBuffers()
    {
        vkDestroyImageView(m_Device->Device(), m_GBufferPositionView, nullptr);
        vkDestroyImage(m_Device->Device(), m_GBufferPositionImage, nullptr);
        vkFreeMemory(m_Device->Device(), m_GBufferPositionImageMemory, nullptr);

        vkDestroyImageView(m_Device->Device(), m_GBufferNormalView, nullptr);
        vkDestroyImage(m_Device->Device(), m_GBufferNormalImage, nullptr);
        vkFreeMemory(m_Device->Device(), m_GBufferNormalImageMemory, nullptr);

        vkDestroyImageView(m_Device->Device(), m_GBufferColorView, nullptr);
        vkDestroyImage(m_Device->Device(), m_GBufferColorImage, nullptr);
        vkFreeMemory(m_Device->Device(), m_GBufferColorImageMemory, nullptr);

        vkDestroyImageView(m_Device->Device(), m_GBufferMaterialView, nullptr);
        vkDestroyImage(m_Device->Device(), m_GBufferMaterialImage, nullptr);
        vkFreeMemory(m_Device->Device(), m_GBufferMaterialImageMemory, nullptr);

        vkDestroyImageView(m_Device->Device(), m_GBufferEmissionView, nullptr);
        vkDestroyImage(m_Device->Device(), m_GBufferEmissionImage, nullptr);
        vkFreeMemory(m_Device->Device(), m_GBufferEmissionImageMemory, nullptr);
    }
} // namespace GfxRenderEngine
