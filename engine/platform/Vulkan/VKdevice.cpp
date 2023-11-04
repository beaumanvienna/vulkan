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

#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

#include "engine.h"
#include "coreSettings.h"

#include "VKdevice.h"
#include "VKwindow.h"

#ifdef MACOSX
    #include <vulkan/vulkan_beta.h>
    #define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR static_cast<VkStructureType>(1000163000)
#endif

namespace GfxRenderEngine
{

    // local callback functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    VK_Device::VK_Device(VK_Window* window) : m_Window{window}
    {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateCommandPool();
    }

    VK_Device::~VK_Device()
    {
        vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
        vkDestroyCommandPool(m_Device, m_LoadCommandPool, nullptr);
        vkDestroyDevice(m_Device, nullptr);

        if (m_EnableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    }
    
    void VK_Device::Shutdown()
    {
        vkQueueWaitIdle(m_GraphicsQueue);
        vkQueueWaitIdle(m_PresentQueue);
        vkQueueWaitIdle(m_TransfertQueue);
    }

    void VK_Device::CreateInstance()
    {
        if (m_EnableValidationLayers && !CheckValidationLayerSupport())
        {
            LOG_CORE_CRITICAL("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "gfxRenderEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        #ifdef MACOSX
          createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (m_EnableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint>(m_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create instance!");
            exit(1);
        }

        HasGflwRequiredInstanceExtensions();
    }

    void VK_Device::PickPhysicalDevice()
    {
        uint deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            LOG_CORE_CRITICAL("failed to find GPUs with Vulkan support!");
        }
        //std::cout << "Device count: " << deviceCount << std::endl;
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        for (auto& device : devices)
        {
            if (IsPreferredDevice(device))
            {
                m_PhysicalDevice = device;
                vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
                LOG_CORE_INFO("Found a dedicated graphics card: {0}", m_Properties.deviceName);
                SetMaxUsableSampleCount();
                return;
            }
        }

        for (auto &device : devices)
        {
            if (IsSuitableDevice(device))
            {
                m_PhysicalDevice = device;
                vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
                LOG_CORE_INFO("found an onboard graphics card: {0}", m_Properties.deviceName);
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE)
        {
            LOG_CORE_CRITICAL("failed to find a suitable GPU!");
            exit(1);
        }
    }

    VkDeviceQueueCreateInfo VK_Device::CreateQueue(const QueueSpec& spec)
    {
        VkDeviceQueueCreateInfo queueCreateInfos = {};

        // graphics queue
        queueCreateInfos.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos.queueFamilyIndex = spec.m_QueueFamilyIndex;
        queueCreateInfos.queueCount = spec.m_QueueCount;
        queueCreateInfos.pQueuePriorities = &spec.m_QueuePriority;
        return queueCreateInfos;
    }

    void VK_Device::CreateLogicalDevice()
    {
        auto& indices = m_QueueFamilyIndices;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        for (int familyIndex : indices.m_UniqueFamilyIndices)
        {
            int queuesPerFamily = 0;
            if (familyIndex == indices.m_GraphicsFamily)
            {  // graphics
                ++queuesPerFamily;
            }
            else if (familyIndex == indices.m_PresentFamily) // present: only create a queue when in different family than graphics
            { 
                ++queuesPerFamily;
            }
            if (familyIndex == indices.m_TransferFamily)
            {  // transfer
               ++queuesPerFamily;
            }
            if (queuesPerFamily)
            {  
                QueueSpec spec =
                {
                    familyIndex,    // int     m_QueueFamilyIndex;
                    1.0f,           // float   m_QeuePriority;
                    queuesPerFamily // int     m_QueueCount;
                };
                queueCreateInfos.push_back(CreateQueue(spec));
            }
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint>(m_RequiredDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_RequiredDeviceExtensions.data();
        
        // might not really be necessary anymore because device specific validation layers
        // have been deprecated
        if (m_EnableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint>(m_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        #ifdef MACOSX
            VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilityFeatures = {};
            {
                portabilityFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR;

                VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
                physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                physicalDeviceFeatures2.pNext = &portabilityFeatures;
                vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &physicalDeviceFeatures2);
            }
            createInfo.pNext=&portabilityFeatures;
        #endif
        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create logical device!");
        }

        vkGetDeviceQueue(m_Device, indices.m_GraphicsFamily, indices.m_QueueIndices[QueueTypes::GRAPHICS], &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, indices.m_PresentFamily,  indices.m_QueueIndices[QueueTypes::PRESENT], &m_PresentQueue);
        vkGetDeviceQueue(m_Device, indices.m_TransferFamily, indices.m_QueueIndices[QueueTypes::TRANSFER], &m_TransfertQueue);
        m_TransferQueueSupportsGraphics = indices.m_GraphicsFamily == indices.m_TransferFamily;
    }

    void VK_Device::CreateCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_QueueFamilyIndices.m_GraphicsFamily;
        poolInfo.flags =
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_GraphicsCommandPool) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create graphics command pool!");
        }

        poolInfo.queueFamilyIndex = m_QueueFamilyIndices.m_TransferFamily;

        if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_LoadCommandPool) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create load command pool!");
        }
    }

    void VK_Device::CreateSurface()
    {
        m_Window->CreateWindowSurface(m_Instance, &m_Surface);
    }

    bool VK_Device::IsPreferredDevice(VkPhysicalDevice& device)
    {
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            LOG_CORE_INFO("checking if '{0}' is a preferred device ...", properties.deviceName);
        }
        if (!IsSuitableDevice(device))
        {
            return false;
        }

        auto props = VkPhysicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(device, &props);
        bool isPreferred = (props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        if (isPreferred)
        {
            LOG_CORE_INFO(" ... preferred device found!");
        }
        else
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            LOG_CORE_INFO(" ... no luck, ('{0}' is not a prefereed device)", properties.deviceName);
        }
        return isPreferred;
    }

    bool VK_Device::IsSuitableDevice(VkPhysicalDevice& device)
    {
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            LOG_CORE_INFO("checking if '{0}' is a suitable device ...", properties.deviceName);
        }
        // check if blacklisted
        vkGetPhysicalDeviceProperties(device, &m_Properties);

        std::string name = m_Properties.deviceName;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });

        std::string blacklisted = CoreSettings::m_BlacklistedDevice;
        std::transform(blacklisted.begin(), blacklisted.end(), blacklisted.begin(), [](unsigned char c){ return std::tolower(c); });

        if (name.find(blacklisted) != std::string::npos)
        {
            LOG_CORE_INFO("ignoring blacklisted device: {0}", name);
            return false;
        }

        QueueFamilyIndices indices = FindQueueFamilies(device);

        // check extensions
        bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        bool suitable = indices.IsComplete() &&
                        extensionsSupported &&
                        swapChainAdequate &&
                        supportedFeatures.samplerAnisotropy;
        if (suitable)
        {
            m_QueueFamilyIndices = indices;
            LOG_CORE_INFO("suitable device found");
        }
        return suitable;
    }

    void VK_Device::PopulateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;  // Optional
    }

    void VK_Device::SetupDebugMessenger()
    {
        if (!m_EnableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);
        auto result = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
        if (result != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to set up debug messenger!");
        }
    }

    bool VK_Device::CheckValidationLayerSupport()
    {
        uint layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : m_ValidationLayers)
        {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> VK_Device::GetRequiredExtensions()
    {
        uint glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (m_EnableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        #ifdef MACOSX
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            //extensions.push_back("VK_KHR_portability_subset");
        #endif

        return extensions;
    }

    void VK_Device::HasGflwRequiredInstanceExtensions()
    {
        uint extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::unordered_set<std::string> available;
        for (const auto &extension : extensions) {
            available.insert(extension.extensionName);
        }

        auto requiredExtensions = GetRequiredExtensions();
        for (const auto &required : requiredExtensions)
        {
            if (available.find(required) == available.end())
            {
                LOG_CORE_CRITICAL("Missing required glfw extension");
            }
        }
    }

    bool VK_Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties
        (
            device,
            nullptr,
            &extensionCount,
            availableExtensions.data()
        );

        std::set<std::string> requiredExtensions(m_RequiredDeviceExtensions.begin(), m_RequiredDeviceExtensions.end());

        // check if all required extensions are in available extensions
        // if it finds each required extension, requiredExtensions will be empty
        for (const auto &extension : availableExtensions)
        {
            if (requiredExtensions.erase(extension.extensionName))
            {
                LOG_CORE_INFO("extension '{0}' is available", extension.extensionName);
            }
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices VK_Device::FindQueueFamilies(VkPhysicalDevice& device)
    {
        LOG_CORE_INFO("finding queue family indices");
        QueueFamilyIndices indices;

        uint queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        indices.m_UniqueFamilyIndices.resize(queueFamilyCount);
        for (size_t index = 0; index < queueFamilyCount; ++index)
        {
            indices.m_UniqueFamilyIndices[index] = NO_ASSIGNED;
        }
        int numberOfQueues = 0;
        int uniqueIndices = 0;
        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            int availableQueues = queueFamily.queueCount;
            // graphics queue
            if ((queueFamily.queueCount > 0) &&
                    (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
                    (indices.m_GraphicsFamily == NO_ASSIGNED) &&
                    (availableQueues>0))
            {
                if (indices.m_UniqueFamilyIndices[i] != i)
                {
                    indices.m_UniqueFamilyIndices[uniqueIndices] = i;
                    ++uniqueIndices;
                }
                indices.m_GraphicsFamily = i;
                ++numberOfQueues;
                --availableQueues;
            }
            // present queue (can be shared with graphics queue, no need to check availableQueues)
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
            if ((queueFamily.queueCount > 0) &&
                    presentSupport && 
                    (indices.m_PresentFamily == NO_ASSIGNED))
            {
                indices.m_PresentFamily = i;

                if (indices.m_GraphicsFamily != i) // if it needs a dedicated queue
                {
                    if (indices.m_UniqueFamilyIndices[i] != i)
                    {
                        indices.m_UniqueFamilyIndices[uniqueIndices] = i;
                        ++uniqueIndices;
                    }
                    ++numberOfQueues;
                    --availableQueues;
                }
            }
            // transfer queue
            if ((queueFamily.queueCount > 0) && 
                    (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && 
                    (indices.m_TransferFamily == NO_ASSIGNED) &&
                    (availableQueues>0))
            {
                if (indices.m_UniqueFamilyIndices[i] != i)
                {
                    indices.m_UniqueFamilyIndices[uniqueIndices] = i;
                    ++uniqueIndices;
                }
                ++numberOfQueues;
                indices.m_TransferFamily = i;
            }
            
            if (indices.IsComplete())
            {
                indices.m_NumberOfQueues = numberOfQueues;
                break;
            }

            i++;
        }
        if (!indices.IsComplete())
        {
            LOG_CORE_INFO("queue family indices incomplete!");
            QueueFamilyIndices empty;
            return empty;
        }
        LOG_CORE_INFO("all queue family indices found");

        indices.m_QueueIndices[QueueTypes::GRAPHICS] = 0;
        indices.m_QueueIndices[QueueTypes::PRESENT] = 0; // either shares the same queue with grapics or has a different queue family, in which it will also be queue 0
        if ( (indices.m_TransferFamily == indices.m_GraphicsFamily) || (indices.m_TransferFamily == indices.m_PresentFamily))
        {
            indices.m_QueueIndices[QueueTypes::TRANSFER] = 1;
        }
        else
        {   //transfer has a dedicated queue family 
            indices.m_QueueIndices[QueueTypes::TRANSFER] = 0;
        }

        return indices;
    }

    SwapChainSupportDetails VK_Device::QuerySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

        uint formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
        }

        uint presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                m_Surface,
                &presentModeCount,
                details.presentModes.data());
        }
        return details;
    }

    VkFormat VK_Device::FindSupportedFormat
    (
        const std::vector<VkFormat> &candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features
    )
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }
        LOG_CORE_CRITICAL("failed to find supported format!");
        return VK_FORMAT_UNDEFINED;
    }

    VkFormat VK_Device::FindDepthFormat()
    {
        return FindSupportedFormat
        (
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    uint VK_Device::FindMemoryType(uint typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);
        for (uint i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        LOG_CORE_CRITICAL("failed to find suitable memory type!");
        return 0;
    }

    void VK_Device::CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create vertex buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
    }

    VkCommandBuffer VK_Device::BeginSingleTimeCommands(QueueTypes type)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        // single time commands are transfer commands
        // however, commands like vkCmdBlitImage require graphics support on transfer queues
        // --> if the transfer queue is of the same queue family as the graphics queue,
        // the load command pool can be used, multithreading enabled, "type" be ignored
        // otherwise the graphics queue has to be used, if requested by "type"
        if (m_TransferQueueSupportsGraphics) 
        {
            allocInfo.commandPool = m_LoadCommandPool;
        }
        else
        {
            allocInfo.commandPool = type == QueueTypes::TRANSFER ? m_LoadCommandPool : m_GraphicsCommandPool;
        }
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void VK_Device::EndSingleTimeCommands(VkCommandBuffer commandBuffer, QueueTypes type)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // see BeginSingleTimeCommands
        if ((type == QueueTypes::TRANSFER) || m_TransferQueueSupportsGraphics)
        {
            vkQueueSubmit(TransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(TransferQueue());
            vkFreeCommandBuffers(m_Device, m_LoadCommandPool, 1, &commandBuffer);
        }
        else
        {
            vkQueueSubmit(GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(GraphicsQueue());
            vkFreeCommandBuffers(m_Device, m_GraphicsCommandPool, 1, &commandBuffer);
        }
        

    }

    void VK_Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands(QueueTypes::TRANSFER);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer, QueueTypes::TRANSFER);
    }

    void VK_Device::CopyBufferToImage(
        VkBuffer buffer, VkImage image, uint width, uint height, uint layerCount)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands(QueueTypes::TRANSFER);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
        EndSingleTimeCommands(commandBuffer, QueueTypes::TRANSFER);
    }

    void VK_Device::CreateImageWithInfo(
        const VkImageCreateInfo &imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory)
    {
        if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to allocate image memory! in 'void VK_Device::CreateImageWithInfo'");
        }

        if (vkBindImageMemory(m_Device, image, imageMemory, 0) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to bind image memory!");
        }
    }

    void VK_Device::SetMaxUsableSampleCount()
    {
        VkSampleCountFlags counts = m_Properties.limits.framebufferColorSampleCounts & m_Properties.limits.framebufferDepthSampleCounts;

        if (counts & VK_SAMPLE_COUNT_64_BIT)
        {
            LOG_CORE_INFO("sample count: VK_SAMPLE_COUNT_64_BIT");
            m_SampleCountFlagBits = VK_SAMPLE_COUNT_64_BIT;
        }
        else if (counts & VK_SAMPLE_COUNT_32_BIT)
        {
            LOG_CORE_INFO("sample count: VK_SAMPLE_COUNT_32_BIT");
            m_SampleCountFlagBits = VK_SAMPLE_COUNT_32_BIT;
        }
        else if (counts & VK_SAMPLE_COUNT_16_BIT)
        {
            LOG_CORE_INFO("sample count: VK_SAMPLE_COUNT_16_BIT");
            m_SampleCountFlagBits = VK_SAMPLE_COUNT_16_BIT;
        }
        else if (counts & VK_SAMPLE_COUNT_8_BIT)
        {
            LOG_CORE_INFO("sample count: VK_SAMPLE_COUNT_8_BIT");
            m_SampleCountFlagBits = VK_SAMPLE_COUNT_8_BIT;
        }
        else if (counts & VK_SAMPLE_COUNT_4_BIT)
        {
            LOG_CORE_INFO("sample count: VK_SAMPLE_COUNT_4_BIT");
            m_SampleCountFlagBits = VK_SAMPLE_COUNT_4_BIT;
        }
        else if (counts & VK_SAMPLE_COUNT_2_BIT)
        {
            LOG_CORE_INFO("sample count: VK_SAMPLE_COUNT_2_BIT");
            m_SampleCountFlagBits = VK_SAMPLE_COUNT_2_BIT;
        }
        else
        {
            LOG_CORE_INFO("sample count: VK_SAMPLE_COUNT_1_BIT");
            m_SampleCountFlagBits = VK_SAMPLE_COUNT_1_BIT;
        }
    }
}
