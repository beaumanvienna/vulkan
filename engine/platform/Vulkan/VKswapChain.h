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

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "VKdevice.h"
#include <vulkan/vulkan.h>

class VK_SwapChain
{

public:

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    VK_SwapChain(std::shared_ptr<VK_Device> device, VkExtent2D m_WindowExtent);
    ~VK_SwapChain();

    VK_SwapChain(const VK_SwapChain &) = delete;
    VK_SwapChain& operator=(const VK_SwapChain &) = delete;

    VkFramebuffer GetFrameBuffer(int index) { return m_SwapChainFramebuffers[index]; }
    VkRenderPass GetRenderPass() { return m_RenderPass; }
    VkImageView GetImageView(int index) { return m_SwapChainImageViews[index]; }
    size_t ImageCount() { return m_SwapChainImages.size(); }
    VkFormat GetSwapChainImageFormat() { return m_SwapChainImageFormat; }
    VkExtent2D GetSwapChainExtent() { return m_SwapChainExtent; }
    uint Width() { return m_SwapChainExtent.width; }
    uint Height() { return m_SwapChainExtent.height; }
    
    float ExtentAspectRatio()
    {
        return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
    }
    VkFormat FindDepthFormat();
    
    VkResult AcquireNextImage(uint *imageIndex);
    VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint *imageIndex);
    
private:

    void CreateSwapChain();
    void CreateImageViews();
    void CreateDepthResources();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;

    std::vector<VkFramebuffer> m_SwapChainFramebuffers;
    VkRenderPass m_RenderPass;

    std::vector<VkImage> m_DepthImages;
    std::vector<VkDeviceMemory> m_DepthImageMemorys;
    std::vector<VkImageView> m_DepthImageViews;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkImageView> m_SwapChainImageViews;

    std::shared_ptr<VK_Device> m_Device;
    VkExtent2D m_WindowExtent;

    VkSwapchainKHR m_SwapChain;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;
    std::vector<VkFence> m_ImagesInFlight;
    size_t m_CurrentFrame = 0;
};
