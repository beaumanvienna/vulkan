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
#include <vulkan/vulkan.h>

#include "VKdevice.h"

namespace GfxRenderEngine
{
    class VK_SwapChain
    {

    public:
        enum SubPasses
        {
            SUBPASS_GEOMETRY = 0,
            SUBPASS_LIGHTING,
            NUMBER_OF_SUBPASSES
        };

        enum RenderTargets
        {
            ATTACHMENT_BACKBUFFER = 0,
            ATTACHMENT_DEPTH,
            ATTACHMENT_GBUFFER_POSITION,
            ATTACHMENT_GBUFFER_NORMAL,
            ATTACHMENT_GBUFFER_COLOR,
            ATTACHMENT_GBUFFER_MATERIAL,
            NUMBER_OF_ATTACHMENTS
        };

        static constexpr int NUMBER_OF_GBUFFER_ATTACHMENTS = NUMBER_OF_ATTACHMENTS - ATTACHMENT_GBUFFER_POSITION;

    public:

        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        VK_SwapChain(std::shared_ptr<VK_Device> device, VkExtent2D m_WindowExtent);
        VK_SwapChain(std::shared_ptr<VK_Device> device, VkExtent2D m_WindowExtent, std::shared_ptr<VK_SwapChain> previous);
        ~VK_SwapChain();

        VK_SwapChain(const VK_SwapChain &) = delete;
        VK_SwapChain& operator=(const VK_SwapChain &) = delete;

        VkFramebuffer GetFrameBuffer(int index) { return m_SwapChainFramebuffers[index]; }
        VkRenderPass GetRenderPass() { return m_RenderPass; }
        VkImageView GetImageView(int index) { return m_SwapChainImageViews[index]; }
        VkImageView GetImageViewGBufferPosition(int index) { return m_GBufferPositionViews[index]; }
        VkImageView GetImageViewGBufferNormal(int index) { return m_GBufferNormalViews[index]; }
        VkImageView GetImageViewGBufferColor(int index) { return m_GBufferColorViews[index]; }
        VkImageView GetImageViewGBufferMaterial(int index) { return m_GBufferMaterialViews[index]; }
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
        bool CompareSwapFormats(const VK_SwapChain& swapChain) const;

    private:
        void Init();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        void CreateGBufferImages();
        void CreateGBufferViews();
        void DestroyGBuffers();

        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkFormat m_SwapChainImageFormat;
        VkFormat m_SwapChainDepthFormat;
        VkExtent2D m_SwapChainExtent;

        std::vector<VkFramebuffer> m_SwapChainFramebuffers;
        VkRenderPass m_RenderPass;

        std::vector<VkImage> m_DepthImages;
        std::vector<VkImageView> m_DepthImageViews;
        std::vector<VkDeviceMemory> m_DepthImageMemorys;

        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImage> m_GBufferPositionImages;
        std::vector<VkImage> m_GBufferNormalImages;
        std::vector<VkImage> m_GBufferColorImages;
        std::vector<VkImage> m_GBufferMaterialImages;

        std::vector<VkImageView> m_SwapChainImageViews;
        std::vector<VkImageView> m_GBufferPositionViews;
        std::vector<VkImageView> m_GBufferNormalViews;
        std::vector<VkImageView> m_GBufferColorViews;
        std::vector<VkImageView> m_GBufferMaterialViews;

        std::vector<VkDeviceMemory> m_GBufferPositionImageMemorys;
        std::vector<VkDeviceMemory> m_GBufferNormalImageMemorys;
        std::vector<VkDeviceMemory> m_GBufferColorImageMemorys;
        std::vector<VkDeviceMemory> m_GBufferMaterialImageMemorys;

        std::shared_ptr<VK_Device> m_Device;
        std::shared_ptr<VK_SwapChain> m_OldSwapChain;
        VkExtent2D m_WindowExtent;

        VkSwapchainKHR m_SwapChain;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        size_t m_CurrentFrame = 0;
    };
}
