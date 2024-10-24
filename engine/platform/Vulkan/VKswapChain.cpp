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

#include "engine.h"

#include "auxiliary/instrumentation.h"

#include "VKswapChain.h"
#include "VKcore.h"

namespace GfxRenderEngine
{

    VK_SwapChain::VK_SwapChain(VkExtent2D extent) : m_WindowExtent{extent} { Init(); }

    VK_SwapChain::VK_SwapChain(VkExtent2D extent, std::shared_ptr<VK_SwapChain> previous)
        : m_WindowExtent{extent}, m_OldSwapChain{previous}
    {
        Init();
        m_OldSwapChain.reset();
    }

    void VK_SwapChain::Init()
    {
        m_Device = VK_Core::m_Device;

        CreateSwapChain();
        CreateImageViews();

        CreateSyncObjects();
    }

    VK_SwapChain::~VK_SwapChain()
    {
        for (auto imageView : m_SwapChainImageViews)
        {
            vkDestroyImageView(m_Device->Device(), imageView, nullptr);
        }

        if (m_SwapChain != nullptr)
        {
            vkDestroySwapchainKHR(m_Device->Device(), m_SwapChain, nullptr);
            m_SwapChain = nullptr;
        }

        // cleanup synchronization objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_Device->Device(), m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_Device->Device(), m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_Device->Device(), m_InFlightFences[i], nullptr);
        }
    }

    VkResult VK_SwapChain::AcquireNextImage(uint* imageIndex)
    {
        PROFILE_SCOPE("waitFor InFlightFences");
        vkWaitForFences(m_Device->Device(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE,
                        std::numeric_limits<uint64>::max());

        auto result = vkAcquireNextImageKHR(m_Device->Device(), m_SwapChain, std::numeric_limits<uint64>::max(),
                                            m_ImageAvailableSemaphores[m_CurrentFrame], // must be a not signaled semaphore
                                            VK_NULL_HANDLE, imageIndex);

        return result;
    }

    VkResult VK_SwapChain::SubmitCommandBuffers(const VkCommandBuffer* buffers, uint* imageIndex)
    {
        if (m_ImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        {
            ZoneScopedN("SCB waitFence"); // SCB: submit command buffers
            PROFILE_SCOPE("waitFor ImagesInFlight");
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

        {
            vkResetFences(m_Device->Device(), 1, &m_InFlightFences[m_CurrentFrame]);
            {
                ZoneScopedN("SCB queueSubmit");
                std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
                auto result = vkQueueSubmit(m_Device->GraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
                if (result != VK_SUCCESS)
                {
                    VK_Core::m_Device->PrintError(result);
                    LOG_CORE_CRITICAL("failed to submit draw command buffer!");
                }
            }
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR m_SwapChains[] = {m_SwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = m_SwapChains;

        presentInfo.pImageIndices = imageIndex;

        VkResult result{};
        {
            ZoneScopedN("vkQueuePresentKHR");
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            result = vkQueuePresentKHR(m_Device->PresentQueue(), &presentInfo);
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    void VK_SwapChain::CreateSwapChain()
    {
        SwapChainSupportDetails m_SwapChainSupport = m_Device->GetSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(m_SwapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(m_SwapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(m_SwapChainSupport.capabilities);

        uint imageCount = m_SwapChainSupport.capabilities.minImageCount + 1;
        if (m_SwapChainSupport.capabilities.maxImageCount > 0 && imageCount > m_SwapChainSupport.capabilities.maxImageCount)
        {
            imageCount = m_SwapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Device->Surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices& indices = m_Device->PhysicalQueueFamilies();
        uint queueFamilyIndices[] = {static_cast<uint>(indices.m_GraphicsFamily),
                                     static_cast<uint>(indices.m_PresentFamily)};

        if (indices.m_GraphicsFamily != indices.m_PresentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;     // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = m_SwapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = (m_OldSwapChain == nullptr ? VK_NULL_HANDLE : m_OldSwapChain->m_SwapChain);

        auto result = vkCreateSwapchainKHR(m_Device->Device(), &createInfo, nullptr, &m_SwapChain);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create swap chain!");
        }

        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(m_Device->Device(), m_SwapChain, &imageCount, nullptr);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device->Device(), m_SwapChain, &imageCount, m_SwapChainImages.data());

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
                VK_Core::m_Device->PrintError(result);
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
            if (vkCreateSemaphore(m_Device->Device(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) !=
                    VK_SUCCESS ||
                vkCreateSemaphore(m_Device->Device(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) !=
                    VK_SUCCESS ||
                vkCreateFence(m_Device->Device(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create synchronization objects for a frame!");
            }
        }
    }

    VkSurfaceFormatKHR VK_SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR VK_SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (auto& presentMode : availablePresentModes)
        {
            if (presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
            {
                return presentMode;
            }
        }
        // guaranteed to be supported
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VK_SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = m_WindowExtent;
            actualExtent.width =
                std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height,
                                           std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    bool VK_SwapChain::CompareSwapFormats(const VK_SwapChain& swapChain) const
    {
        bool imageFormatEqual = (swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat);
        return (imageFormatEqual);
    }
} // namespace GfxRenderEngine
