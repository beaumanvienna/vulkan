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
        static constexpr int MAX_FRAMES_IN_FLIGHT =
            2; // refers to calculated frames in between vsync (not at the same time!)

        VK_SwapChain(VkExtent2D windowExtent);
        VK_SwapChain(VkExtent2D windowExtent, std::shared_ptr<VK_SwapChain> previous);
        ~VK_SwapChain();

        VK_SwapChain(const VK_SwapChain&) = delete;
        VK_SwapChain& operator=(const VK_SwapChain&) = delete;

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

        VkResult AcquireNextImage(uint* imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint* imageIndex);
        bool CompareSwapFormats(const VK_SwapChain& swapChain) const;

    private:
        void Init();
        void CreateSwapChain();
        void CreateImageViews();

        void CreateSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat m_SwapChainImageFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VkExtent2D m_SwapChainExtent{};

        std::vector<VkImage> m_SwapChainImages{};
        std::vector<VkImageView> m_SwapChainImageViews{};

        VK_Device* m_Device;
        std::shared_ptr<VK_SwapChain> m_OldSwapChain;
        VkExtent2D m_WindowExtent{};

        VkSwapchainKHR m_SwapChain{nullptr};

        std::vector<VkSemaphore> m_ImageAvailableSemaphores{};
        std::vector<VkSemaphore> m_RenderFinishedSemaphores{};
        std::vector<VkFence> m_InFlightFences{};
        std::vector<VkFence> m_ImagesInFlight{};
        size_t m_CurrentFrame{0};
    };
} // namespace GfxRenderEngine
