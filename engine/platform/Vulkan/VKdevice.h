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

#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace GfxRenderEngine
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        uint graphicsFamily;
        uint presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class VK_Window;

    class VK_Device
    {

    public:

        #ifdef NDEBUG
        const bool m_EnableValidationLayers = false;
        #else
        const bool m_EnableValidationLayers = true;
        #endif

        VK_Device(VK_Window* window);
        ~VK_Device();

        // Not copyable or movable
        VK_Device(const VK_Device &) = delete;
        VK_Device& operator=(const VK_Device &) = delete;
        VK_Device(VK_Device &&) = delete;
        VK_Device& operator=(VK_Device &&) = delete;

        void Shutdown();

        VkDevice Device() { return m_Device; }
        VkCommandPool GetCommandPool() { return m_GraphicsCommandPool; }
        VkPhysicalDevice PhysicalDevice() { return m_PhysicalDevice; }
        VkSurfaceKHR Surface() { return m_Surface; }
        VkQueue GraphicsQueue() { return m_DeviceQueues[DeviceQueues::GRAPHICS_QUEUE]; }
        VkQueue PresentQueue() { return m_PresentQueue; }

        SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(m_PhysicalDevice); }
        uint FindMemoryType(uint typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(m_PhysicalDevice); }
        void SetMaxUsableSampleCount();
        VkFormat FindSupportedFormat
        (
            const std::vector<VkFormat> &candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features
        );
        VkFormat FindDepthFormat();

        // Buffer Helper Functions
        void CreateBuffer
        (
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &bufferMemory
        );

        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBufferToImage
        (
            VkBuffer buffer,
            VkImage image,
            uint width,
            uint height,
            uint layerCount
        );

        void CreateImageWithInfo
        (
            const VkImageCreateInfo &imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage &image,
            VkDeviceMemory &imageMemory
        );

        VkPhysicalDeviceProperties m_Properties;
        VkSampleCountFlagBits m_SampleCountFlagBits;
        
        VkInstance GetInstance() const { return m_Instance; }
        uint32_t GetGraphicsQueueFamily() { return FindPhysicalQueueFamilies().graphicsFamily; }

    private:

        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool();

        // helper functions
        bool IsSuitableDevice(VkPhysicalDevice device);
        bool IsPreferredDevice(VkPhysicalDevice device);
        std::vector<const char*> GetRequiredExtensions();
        bool CheckValidationLayerSupport();
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void HasGflwRequiredInstanceExtensions();
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

        VkInstance m_Instance;
        VkDebugUtilsMessengerEXT m_DebugMessenger;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VK_Window* m_Window;
        VkCommandPool m_GraphicsCommandPool;
        VkCommandPool m_LoadCommandPool;

        VkDevice m_Device;
        VkSurfaceKHR m_Surface;

        enum DeviceQueues
        {
            GRAPHICS_QUEUE = 0,
            LOAD_QUEUE,
            NUMBER_OF_QUEUES
        };
        VkQueue m_DeviceQueues[DeviceQueues::NUMBER_OF_QUEUES];
        VkQueue m_PresentQueue;

        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    };
}
