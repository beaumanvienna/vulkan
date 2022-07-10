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

#include "engine.h"

#include "VKswapChain.h"

namespace GfxRenderEngine
{

    VK_SwapChain::VK_SwapChain(std::shared_ptr<VK_Device> device, VkExtent2D extent)
        : m_Device{device}, m_WindowExtent{extent}
    {
        Init();
    }

    VK_SwapChain::VK_SwapChain(std::shared_ptr<VK_Device> device, VkExtent2D extent, std::shared_ptr<VK_SwapChain> previous)
        : m_Device{device}, m_WindowExtent{extent}, m_OldSwapChain{previous}
    {
        Init();
        m_OldSwapChain.reset();
    }

    void VK_SwapChain::Init()
    {
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateGUIRenderPass();
        CreateDepthResources();
        CreateGBufferImages();
        CreateGBufferViews();
        CreateFramebuffers();
        CreateGUIFramebuffers();
        CreateSyncObjects();
    }

    VK_SwapChain::~VK_SwapChain()
    {
        for (auto imageView : m_SwapChainImageViews)
        {
            vkDestroyImageView(m_Device->Device(), imageView, nullptr);
        }
        m_SwapChainImageViews.clear();

        if (m_SwapChain != nullptr)
        {
            vkDestroySwapchainKHR(m_Device->Device(), m_SwapChain, nullptr);
            m_SwapChain = nullptr;
        }

        for (int i = 0; i < m_DepthImages.size(); i++)
        {
            vkDestroyImageView(m_Device->Device(), m_DepthImageViews[i], nullptr);
            vkDestroyImage(m_Device->Device(), m_DepthImages[i], nullptr);
            vkFreeMemory(m_Device->Device(), m_DepthImageMemorys[i], nullptr);
        }

        for (auto framebuffer : m_SwapChainFramebuffers)
        {
            vkDestroyFramebuffer(m_Device->Device(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(m_Device->Device(), m_RenderPass, nullptr);
        vkDestroyRenderPass(m_Device->Device(), m_GUIRenderPass, nullptr);

        // cleanup synchronization objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_Device->Device(), m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_Device->Device(), m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_Device->Device(), m_InFlightFences[i], nullptr);
        }

        DestroyGBuffers();
    }

    void VK_SwapChain::DestroyGBuffers()
    {
        for (int i = 0; i < m_GBufferPositionImages.size(); i++)
        {
            vkDestroyImageView(m_Device->Device(), m_GBufferPositionViews[i], nullptr);
            vkDestroyImage(m_Device->Device(), m_GBufferPositionImages[i], nullptr);
            vkFreeMemory(m_Device->Device(), m_GBufferPositionImageMemorys[i], nullptr);
        }
        for (int i = 0; i < m_GBufferNormalImages.size(); i++)
        {
            vkDestroyImageView(m_Device->Device(), m_GBufferNormalViews[i], nullptr);
            vkDestroyImage(m_Device->Device(), m_GBufferNormalImages[i], nullptr);
            vkFreeMemory(m_Device->Device(), m_GBufferNormalImageMemorys[i], nullptr);
        }
        for (int i = 0; i < m_GBufferColorImages.size(); i++)
        {
            vkDestroyImageView(m_Device->Device(), m_GBufferColorViews[i], nullptr);
            vkDestroyImage(m_Device->Device(), m_GBufferColorImages[i], nullptr);
            vkFreeMemory(m_Device->Device(), m_GBufferColorImageMemorys[i], nullptr);
        }
        for (int i = 0; i < m_GBufferMaterialImages.size(); i++)
        {
            vkDestroyImageView(m_Device->Device(), m_GBufferMaterialViews[i], nullptr);
            vkDestroyImage(m_Device->Device(), m_GBufferMaterialImages[i], nullptr);
            vkFreeMemory(m_Device->Device(), m_GBufferMaterialImageMemorys[i], nullptr);
        }
    }

    VkResult VK_SwapChain::AcquireNextImage(uint *imageIndex)
    {
        vkWaitForFences(
            m_Device->Device(),
            1,
            &m_InFlightFences[m_CurrentFrame],
            VK_TRUE,
            std::numeric_limits<uint64>::max());

        VkResult result = vkAcquireNextImageKHR(
            m_Device->Device(),
            m_SwapChain,
            std::numeric_limits<uint64>::max(),
            m_ImageAvailableSemaphores[m_CurrentFrame],  // must be a not signaled semaphore
            VK_NULL_HANDLE,
            imageIndex);

    return result;
    }

    VkResult VK_SwapChain::SubmitCommandBuffers(
        const VkCommandBuffer *buffers, uint *imageIndex)
    {
        if (m_ImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(m_Device->Device(), 1, &m_ImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
        }
        m_ImagesInFlight[*imageIndex] = m_InFlightFences[m_CurrentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_Device->Device(), 1, &m_InFlightFences[m_CurrentFrame]);
        if (vkQueueSubmit(m_Device->GraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR m_SwapChains[] = {m_SwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = m_SwapChains;

        presentInfo.pImageIndices = imageIndex;

        auto result = vkQueuePresentKHR(m_Device->PresentQueue(), &presentInfo);

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    void VK_SwapChain::CreateSwapChain()
    {
        SwapChainSupportDetails m_SwapChainSupport = m_Device->GetSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(m_SwapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(m_SwapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(m_SwapChainSupport.capabilities);

        uint ImageCount = m_SwapChainSupport.capabilities.minImageCount + 1;
        if (m_SwapChainSupport.capabilities.maxImageCount > 0 &&
            ImageCount > m_SwapChainSupport.capabilities.maxImageCount)
        {
            ImageCount = m_SwapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Device->Surface();

        createInfo.minImageCount = ImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = m_Device->FindPhysicalQueueFamilies();
        uint queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;      // Optional
            createInfo.pQueueFamilyIndices = nullptr;  // Optional
        }

        createInfo.preTransform = m_SwapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = (m_OldSwapChain == nullptr ? VK_NULL_HANDLE : m_OldSwapChain->m_SwapChain);

        auto result = vkCreateSwapchainKHR(m_Device->Device(), &createInfo, nullptr, &m_SwapChain);
        if (result != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create swap chain!");
        }

        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(m_Device->Device(), m_SwapChain, &ImageCount, nullptr);
        m_SwapChainImages.resize(ImageCount);
        vkGetSwapchainImagesKHR(m_Device->Device(), m_SwapChain, &ImageCount, m_SwapChainImages.data());

        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;
    }

    void VK_SwapChain::CreateImageViews()
    {
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        for (size_t i = 0; i < m_SwapChainImages.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_SwapChainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_SwapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_SwapChainImageViews[i]);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }
    }

    void VK_SwapChain::CreateGBufferImages()
    {
        VkExtent2D m_SwapChainExtent = GetSwapChainExtent();

        VkFormat gBufferPositionFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        VkFormat gBufferNormalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        VkFormat gBufferColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
        VkFormat gBufferMaterialFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        m_GBufferPositionImages.resize(ImageCount());
        m_GBufferPositionImageMemorys.resize(ImageCount());
        m_GBufferPositionViews.resize(ImageCount());

        for (int i = 0; i < m_GBufferPositionImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_SwapChainExtent.width;
            imageInfo.extent.height = m_SwapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = gBufferPositionFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_Device->CreateImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_GBufferPositionImages[i],
                m_GBufferPositionImageMemorys[i]);
        }

        m_GBufferNormalImages.resize(ImageCount());
        m_GBufferNormalImageMemorys.resize(ImageCount());
        m_GBufferNormalViews.resize(ImageCount());

        for (int i = 0; i < m_GBufferNormalImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_SwapChainExtent.width;
            imageInfo.extent.height = m_SwapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = gBufferNormalFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_Device->CreateImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_GBufferNormalImages[i],
                m_GBufferNormalImageMemorys[i]);
        }

        m_GBufferColorImages.resize(ImageCount());
        m_GBufferColorImageMemorys.resize(ImageCount());
        m_GBufferColorViews.resize(ImageCount());

        for (int i = 0; i < m_GBufferColorImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_SwapChainExtent.width;
            imageInfo.extent.height = m_SwapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = gBufferColorFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_Device->CreateImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_GBufferColorImages[i],
                m_GBufferColorImageMemorys[i]);
        }

        m_GBufferMaterialImages.resize(ImageCount());
        m_GBufferMaterialImageMemorys.resize(ImageCount());
        m_GBufferMaterialViews.resize(ImageCount());

        for (int i = 0; i < m_GBufferMaterialImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_SwapChainExtent.width;
            imageInfo.extent.height = m_SwapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = gBufferMaterialFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_Device->CreateImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_GBufferMaterialImages[i],
                m_GBufferMaterialImageMemorys[i]);
        }
    }

    void VK_SwapChain::CreateGBufferViews()
    {
        m_GBufferPositionViews.resize(m_GBufferPositionImages.size());
        for (size_t i = 0; i < m_GBufferPositionImages.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_GBufferPositionImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_GBufferPositionViews[i]);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }

        m_GBufferNormalViews.resize(m_GBufferNormalImages.size());
        for (size_t i = 0; i < m_GBufferNormalImages.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_GBufferNormalImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_GBufferNormalViews[i]);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }

        m_GBufferColorViews.resize(m_GBufferColorImages.size());
        for (size_t i = 0; i < m_GBufferColorImages.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_GBufferColorImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_GBufferColorViews[i]);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }

        m_GBufferMaterialViews.resize(m_GBufferMaterialImages.size());
        for (size_t i = 0; i < m_GBufferMaterialImages.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_GBufferMaterialImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_GBufferMaterialViews[i]);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }
    }

    void VK_SwapChain::CreateRenderPass()
    {
        // ATTACHMENT_BACKBUFFER
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = GetSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_BACKBUFFER;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // ATTACHMENT_DEPTH
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_DEPTH;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // ATTACHMENT_GBUFFER_POSITION
        VkAttachmentDescription gBufferPositionAttachment = {};
        gBufferPositionAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        gBufferPositionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        gBufferPositionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gBufferPositionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gBufferPositionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gBufferPositionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gBufferPositionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gBufferPositionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference gBufferPositionAttachmentRef = {};
        gBufferPositionAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_GBUFFER_POSITION;
        gBufferPositionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference gBufferPositionInputAttachmentRef = {};
        gBufferPositionInputAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_GBUFFER_POSITION;
        gBufferPositionInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // ATTACHMENT_GBUFFER_NORMAL
        VkAttachmentDescription gBufferNormalAttachment = {};
        gBufferNormalAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        gBufferNormalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        gBufferNormalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gBufferNormalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gBufferNormalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gBufferNormalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gBufferNormalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gBufferNormalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference gBufferNormalAttachmentRef = {};
        gBufferNormalAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_GBUFFER_NORMAL;
        gBufferNormalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference gBufferNormalInputAttachmentRef = {};
        gBufferNormalInputAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_GBUFFER_NORMAL;
        gBufferNormalInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // ATTACHMENT_GBUFFER_COLOR
        VkAttachmentDescription gBufferColorAttachment = {};
        gBufferColorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
        gBufferColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        gBufferColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gBufferColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gBufferColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gBufferColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gBufferColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gBufferColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference gBufferColorAttachmentRef = {};
        gBufferColorAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_GBUFFER_COLOR;
        gBufferColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference gBufferColorInputAttachmentRef = {};
        gBufferColorInputAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_GBUFFER_COLOR;
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
        gBufferMaterialAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_GBUFFER_MATERIAL;
        gBufferMaterialAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference gBufferMaterialInputAttachmentRef = {};
        gBufferMaterialInputAttachmentRef.attachment = (uint)RenderTargets::ATTACHMENT_GBUFFER_MATERIAL;
        gBufferMaterialInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // geometry pass
        std::array<VkAttachmentReference, NUMBER_OF_GBUFFER_ATTACHMENTS> gBufferAttachments =
        {
            gBufferPositionAttachmentRef,
            gBufferNormalAttachmentRef,
            gBufferColorAttachmentRef,
            gBufferMaterialAttachmentRef
        };
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
        std::array<VkAttachmentReference, NUMBER_OF_GBUFFER_ATTACHMENTS> inputAttachments = 
        {
            gBufferPositionInputAttachmentRef,
            gBufferNormalInputAttachmentRef,
            gBufferColorInputAttachmentRef,
            gBufferMaterialInputAttachmentRef
        };

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

        // ligthing depends on geometry
        dependencies[0].srcSubpass      = (uint)SubPasses::SUBPASS_GEOMETRY; // Index of the render pass being depended upon by dstSubpass
        dependencies[0].dstSubpass      = (uint)SubPasses::SUBPASS_LIGHTING; // The index of the render pass depending on srcSubpass
        dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // What pipeline stage must have completed for the dependency
        dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // What pipeline stage is waiting on the dependency
        dependencies[0].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // What access scopes influence the dependency
        dependencies[0].dstAccessMask   = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; // What access scopes are waiting on the dependency
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // Other configuration about the dependency

        // transparency depends on lighting
        dependencies[1].srcSubpass      = (uint)SubPasses::SUBPASS_LIGHTING;
        dependencies[1].dstSubpass      = (uint)SubPasses::SUBPASS_TRANSPARENCY;
        dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        
        dependencies[2].srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[2].dstSubpass      = (uint)SubPasses::SUBPASS_GEOMETRY;
        dependencies[2].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[2].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[2].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[2].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[3].srcSubpass      = (uint)SubPasses::SUBPASS_GEOMETRY;
        dependencies[3].dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[3].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[3].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[3].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[3].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // render pass
        std::array<VkAttachmentDescription, (uint)RenderTargets::NUMBER_OF_ATTACHMENTS> attachments = 
        {
            colorAttachment,
            depthAttachment,
            gBufferPositionAttachment,
            gBufferNormalAttachment,
            gBufferColorAttachment,
            gBufferMaterialAttachment
        };
        std::array<VkSubpassDescription, (uint)SubPasses::NUMBER_OF_SUBPASSES> subpasses = 
        {
            subpassGeometry,
            subpassLighting,
            subpassTransparency
        };

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = (uint)RenderTargets::NUMBER_OF_ATTACHMENTS;
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = (uint)SubPasses::NUMBER_OF_SUBPASSES;
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = NUMBER_OF_DEPENDENCIES;
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(m_Device->Device(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create render pass!");
        }
    }

    void VK_SwapChain::CreateGUIRenderPass()
    {
        // ATTACHMENT_BACKBUFFER
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = GetSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = (uint)RenderTargetsGUI::ATTACHMENT_BACKBUFFER;
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

        dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass      = (uint)SubPassesGUI::SUBPASS_GUI;
        dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass      = (uint)SubPassesGUI::SUBPASS_GUI;
        dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // render pass
        std::array<VkAttachmentDescription, (uint)RenderTargetsGUI::NUMBER_OF_ATTACHMENTS> attachments = 
        {
            colorAttachment
        };
        std::array<VkSubpassDescription, (uint)SubPassesGUI::NUMBER_OF_SUBPASSES> subpasses = 
        {
            subpassGUI
        };

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = (uint)RenderTargetsGUI::NUMBER_OF_ATTACHMENTS;
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = (uint)SubPassesGUI::NUMBER_OF_SUBPASSES;
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = NUMBER_OF_DEPENDENCIES;
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(m_Device->Device(), &renderPassInfo, nullptr, &m_GUIRenderPass) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create render pass!");
        }
    }

    void VK_SwapChain::CreateFramebuffers()
    {
        m_SwapChainFramebuffers.resize(ImageCount());
        for (size_t i = 0; i < ImageCount(); i++)
        {
            std::array<VkImageView, (uint)RenderTargets::NUMBER_OF_ATTACHMENTS> attachments = 
            {
                m_SwapChainImageViews[i],
                m_DepthImageViews[i],
                m_GBufferPositionViews[i],
                m_GBufferNormalViews[i],
                m_GBufferColorViews[i],
                m_GBufferMaterialViews[i],
            };

            VkExtent2D m_SwapChainExtent = GetSwapChainExtent();
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = (uint)RenderTargets::NUMBER_OF_ATTACHMENTS;
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_SwapChainExtent.width;
            framebufferInfo.height = m_SwapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(
                    m_Device->Device(),
                    &framebufferInfo,
                    nullptr,
                    &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create framebuffer!");
            }
        }
    }

    void VK_SwapChain::CreateGUIFramebuffers()
    {
        m_GUIFramebuffers.resize(ImageCount());
        for (size_t i = 0; i < ImageCount(); i++)
        {
            std::array<VkImageView, (uint)RenderTargetsGUI::NUMBER_OF_ATTACHMENTS> attachments = 
            {
                m_SwapChainImageViews[i]
            };

            VkExtent2D m_SwapChainExtent = GetSwapChainExtent();
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_GUIRenderPass;
            framebufferInfo.attachmentCount = (uint)RenderTargetsGUI::NUMBER_OF_ATTACHMENTS;
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_SwapChainExtent.width;
            framebufferInfo.height = m_SwapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(
                    m_Device->Device(),
                    &framebufferInfo,
                    nullptr,
                    &m_GUIFramebuffers[i]) != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create framebuffer!");
            }
        }
    }

    void VK_SwapChain::CreateDepthResources()
    {
        VkFormat depthFormat = FindDepthFormat();
        m_SwapChainDepthFormat = depthFormat;
        VkExtent2D m_SwapChainExtent = GetSwapChainExtent();

        m_DepthImages.resize(ImageCount());
        m_DepthImageMemorys.resize(ImageCount());
        m_DepthImageViews.resize(ImageCount());

        for (int i = 0; i < m_DepthImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = m_SwapChainExtent.width;
            imageInfo.extent.height = m_SwapChainExtent.height;
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

            m_Device->CreateImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_DepthImages[i],
                m_DepthImageMemorys[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_DepthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_Device->Device(), &viewInfo, nullptr, &m_DepthImageViews[i]) != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create texture image view!");
            }
        }
    }

    void VK_SwapChain::CreateSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(ImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (
                vkCreateSemaphore(m_Device->Device(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) !=
                    VK_SUCCESS ||
                vkCreateSemaphore(m_Device->Device(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) !=
                    VK_SUCCESS ||
                vkCreateFence(m_Device->Device(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create synchronization objects for a frame!");
            }
        }
    }

    VkSurfaceFormatKHR VK_SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR VK_SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        //std::cout << "Present mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VK_SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = m_WindowExtent;
            actualExtent.width = std::max(
            capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(
            capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    VkFormat VK_SwapChain::FindDepthFormat()
    {
        return m_Device->FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    bool VK_SwapChain::CompareSwapFormats(const VK_SwapChain& swapChain) const
    {
        bool depthFormatEqual = (swapChain.m_SwapChainDepthFormat == m_SwapChainDepthFormat);
        bool imageFormatEqual = (swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat);
        return (depthFormatEqual && imageFormatEqual);
    }
}
