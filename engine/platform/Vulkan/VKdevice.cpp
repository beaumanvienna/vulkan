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
    static const VkBufferUsageFlags BUFFER_USE_FLAGS =
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT |
        VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT |
        VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
        VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;

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

        // I set every resource to have 10000 handles
        m_GPUShaderResourceTable = std::make_unique<GPUShaderResourceTable>(10000, 10000, 10000, m_Device, false);
    }

    VK_Device::~VK_Device()
    {
        vmaDestroyAllocator(m_VmaAllocator);
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

        const VkPhysicalDeviceFeatures REQUIRED_PHYSICAL_DEVICE_FEATURES{
            .robustBufferAccess = VK_FALSE,
            .fullDrawIndexUint32 = VK_FALSE,
            .imageCubeArray = VK_TRUE,
            .independentBlend = VK_TRUE,
            .geometryShader = VK_FALSE,
            .tessellationShader = VK_TRUE,
            .sampleRateShading = VK_FALSE,
            .dualSrcBlend = VK_FALSE,
            .logicOp = VK_FALSE,
            .multiDrawIndirect = VK_TRUE,
            .drawIndirectFirstInstance = VK_FALSE,
            .depthClamp = VK_TRUE,
            .depthBiasClamp = VK_FALSE,
            .fillModeNonSolid = VK_TRUE,
            .depthBounds = VK_FALSE,
            .wideLines = VK_TRUE,
            .largePoints = VK_FALSE,
            .alphaToOne = VK_FALSE,
            .multiViewport = VK_FALSE,
            .samplerAnisotropy = VK_TRUE,
            .textureCompressionETC2 = VK_FALSE,
            .textureCompressionASTC_LDR = VK_FALSE,
            .textureCompressionBC = VK_FALSE,
            .occlusionQueryPrecise = VK_FALSE,
            .pipelineStatisticsQuery = VK_FALSE,
            .vertexPipelineStoresAndAtomics = VK_FALSE,
            .fragmentStoresAndAtomics = VK_TRUE,
            .shaderTessellationAndGeometryPointSize = VK_FALSE,
            .shaderImageGatherExtended = VK_FALSE,
            .shaderStorageImageExtendedFormats = VK_FALSE,
            .shaderStorageImageMultisample = VK_TRUE,
            .shaderStorageImageReadWithoutFormat = VK_TRUE,
            .shaderStorageImageWriteWithoutFormat = VK_TRUE,
            .shaderUniformBufferArrayDynamicIndexing = VK_FALSE,
            .shaderSampledImageArrayDynamicIndexing = VK_FALSE,
            .shaderStorageBufferArrayDynamicIndexing = VK_FALSE,
            .shaderStorageImageArrayDynamicIndexing = VK_FALSE,
            .shaderClipDistance = VK_FALSE,
            .shaderCullDistance = VK_FALSE,
            .shaderFloat64 = VK_FALSE,
            .shaderInt64 = VK_TRUE,
            .shaderInt16 = VK_TRUE,
            .shaderResourceResidency = VK_FALSE,
            .shaderResourceMinLod = VK_FALSE,
            .sparseBinding = VK_FALSE,
            .sparseResidencyBuffer = VK_FALSE,
            .sparseResidencyImage2D = VK_FALSE,
            .sparseResidencyImage3D = VK_FALSE,
            .sparseResidency2Samples = VK_FALSE,
            .sparseResidency4Samples = VK_FALSE,
            .sparseResidency8Samples = VK_FALSE,
            .sparseResidency16Samples = VK_FALSE,
            .sparseResidencyAliased = VK_FALSE,
            .variableMultisampleRate = VK_FALSE,
            .inheritedQueries = VK_FALSE,
        };

        void* REQUIRED_DEVICE_FEATURE_P_CHAIN = nullptr;

        VkPhysicalDeviceDescriptorIndexingFeatures vkPhysicalDeviceDescriptorIndexingFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
            .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
            .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
            .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
            .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
            .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
            .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
            .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
            .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
            .descriptorBindingPartiallyBound = VK_TRUE,
            .descriptorBindingVariableDescriptorCount = VK_FALSE,
            .runtimeDescriptorArray = VK_TRUE,
        };

        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void*>(&vkPhysicalDeviceDescriptorIndexingFeatures);

        VkPhysicalDeviceBufferDeviceAddressFeatures vkPhysicalDeviceBufferDeviceAddressFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .bufferDeviceAddress = VK_TRUE,
            .bufferDeviceAddressCaptureReplay = VK_TRUE,
            .bufferDeviceAddressMultiDevice = VK_FALSE,
        };

        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void*>(&vkPhysicalDeviceBufferDeviceAddressFeatures);

        VkPhysicalDeviceFeatures2 vkPhysicalDeviceFeatures2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN,
            .features = REQUIRED_PHYSICAL_DEVICE_FEATURES,
        };

        REQUIRED_DEVICE_FEATURE_P_CHAIN = reinterpret_cast<void*>(&vkPhysicalDeviceFeatures2);
        
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = REQUIRED_DEVICE_FEATURE_P_CHAIN;
        
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        
        createInfo.pEnabledFeatures = nullptr;
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

        VmaVulkanFunctions vmaVulkanFunctions = {
            .vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = &vkGetDeviceProcAddr
        };

        VmaAllocatorCreateInfo vmaAllocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = m_PhysicalDevice,
            .device = m_Device,
            .preferredLargeHeapBlockSize = 0,
            .pAllocationCallbacks = nullptr,
            .pDeviceMemoryCallbacks = nullptr,
            .pHeapSizeLimit = nullptr,
            .pVulkanFunctions = &vmaVulkanFunctions,
            .instance = m_Instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
            .pTypeExternalMemoryHandleTypes = nullptr 
        };

        vmaCreateAllocator(&vmaAllocatorCreateInfo, &m_VmaAllocator);
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

    auto VK_Device::CreateBuffer(const BufferInfo& info) -> BufferId {
        auto [id, ret] = m_GPUShaderResourceTable->m_BufferSlots.NewSlot();

        if (!(info.size > 0)) {
            LOG_CORE_CRITICAL("can not create buffers with size zero");
        }

        ret.info = info;

        const VkBufferCreateInfo vkBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .size = static_cast<VkDeviceSize>(info.size),
            .usage = BUFFER_USE_FLAGS,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = {},
            .pQueueFamilyIndices = {},
        };

        bool hostAccessible = false;
        VmaAllocationInfo vmaAllocationInfo = {};
        auto vmaAllocationFlags = static_cast<VmaAllocationCreateFlags>(info.memoryFlags);
        if (((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT) != 0u) ||
            ((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0u) ||
            ((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0u)) {
            vmaAllocationFlags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            hostAccessible = true;
        }

        const VmaAllocationCreateInfo vmaAllocationCreateInfo{
            .flags = vmaAllocationFlags,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<uint>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        if (vmaCreateBuffer(m_VmaAllocator, &vkBufferCreateInfo, &vmaAllocationCreateInfo, &ret.vkBuffer, &ret.vmaAllocation, &vmaAllocationInfo) != VK_SUCCESS) {
            LOG_CORE_CRITICAL("failed to create buffer!");
        }

        const VkBufferDeviceAddressInfo vkBufferDeviceAddressInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .pNext = nullptr,
            .buffer = ret.vkBuffer,
        };

        ret.deviceAddress = vkGetBufferDeviceAddress(m_Device, &vkBufferDeviceAddressInfo);

        ret.hostAddress = hostAccessible ? vmaAllocationInfo.pMappedData : nullptr;
        ret.zombie = false;

        //if (this->impl_ctx.as<ImplInstance>()->info.enable_debug_names && !buffer_info.name.empty()) {
        //    const auto buffer_name = buffer_info.name;
        //    const VkDebugUtilsObjectNameInfoEXT buffer_name_info{
        //        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        //        .pNext = nullptr,
        //        .objectType = VK_OBJECT_TYPE_BUFFER,
        //        .objectHandle = reinterpret_cast<uint64_t>(ret.vk_buffer),
        //        .pObjectName = buffer_name.c_str(),
        //    };
        //    vkSetDebugUtilsObjectNameEXT(vk_device, &buffer_name_info);
        //}

        return BufferId{ id };
    }

    auto isDepthFormat(Format format) -> bool
    {
        switch (format)
        {
        case Format::D16_UNORM: return true;
        case Format::X8_D24_UNORM_PACK32: return true;
        case Format::D32_SFLOAT: return true;
        case Format::S8_UINT: return true;
        case Format::D16_UNORM_S8_UINT: return true;
        case Format::D24_UNORM_S8_UINT: return true;
        case Format::D32_SFLOAT_S8_UINT: return true;
        default: return false;
        }
    }

    auto isStencilFormat(Format format) -> bool
    {
        switch (format)
        {
        case Format::S8_UINT: return true;
        case Format::D16_UNORM_S8_UINT: return true;
        case Format::D24_UNORM_S8_UINT: return true;
        case Format::D32_SFLOAT_S8_UINT: return true;
        default: return false;
        }
    }

    auto inferAspectFromFormat(Format format) -> VkImageAspectFlags
    {
        if (isDepthFormat(format) || isStencilFormat(format))
        {
            return (isDepthFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) | (isStencilFormat(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
        }
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    auto makeSubressourceRange(ImageMipArraySlice const& slice, VkImageAspectFlags aspect) -> VkImageSubresourceRange
    {
        return VkImageSubresourceRange
        {
            .aspectMask = aspect,
            .baseMipLevel = slice.baseMipLevel,
            .levelCount = slice.levelCount,
            .baseArrayLayer = slice.baseArrayLayer,
            .layerCount = slice.layerCount,
        };
    }

    auto VK_Device::validateImageSlice(const ImageMipArraySlice& slice, ImageId id) -> ImageMipArraySlice {
        if (slice.levelCount == std::numeric_limits<uint>::max() || slice.levelCount == 0) {
            auto image_info = GetImageSlot(id).info;
            return ImageMipArraySlice{
                .baseMipLevel = 0,
                .levelCount = image_info.mipLevelCount,
                .baseArrayLayer = 0,
                .layerCount = image_info.arrayLayerCount,
            };
        }
        else {
            return slice;
        }
    }

    auto VK_Device::validateImageSlice(const ImageMipArraySlice& slice, ImageViewId id) -> ImageMipArraySlice {
        if (slice.levelCount == std::numeric_limits<uint>::max() || slice.levelCount == 0) {
            return GetImageViewSlot(id).info.slice;
        }
        else {
            return slice;
        }
    }

    auto initializeImageCreateInfoFromImageInfo(const ImageInfo& info) -> VkImageCreateInfo {
        if (!(std::popcount(info.sampleCount) == 1 && info.sampleCount <= 64)) {
            LOG_CORE_CRITICAL("image samples must be power of two and between 1 and 64(inclusive)");
        }
        if (!(info.size.x > 0 &&
            info.size.y > 0 &&
            info.size.z > 0)) {
            LOG_CORE_CRITICAL("image (x,y,z) dimensions must be greater then 0");
        }
        if (!(info.arrayLayerCount > 0)) {
            LOG_CORE_CRITICAL("image array layer count must be greater then 0");
        }
        if (!(info.mipLevelCount > 0)) {
            LOG_CORE_CRITICAL("image mip layer count must be greater then 0");
        }

        const auto vkImageType = static_cast<VkImageType>(info.dimensions - 1);

        auto vkImageCreateFlags = static_cast<VkImageCreateFlags>(info.flags);

        const VkImageCreateInfo vkImageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = vkImageCreateFlags,
            .imageType = vkImageType,
            .format = static_cast<VkFormat>(info.format),
            .extent = std::bit_cast<VkExtent3D>(info.size),
            .mipLevels = info.mipLevelCount,
            .arrayLayers = info.arrayLayerCount,
            .samples = static_cast<VkSampleCountFlagBits>(info.sampleCount),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = info.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = {},
            .pQueueFamilyIndices = {},
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        return vkImageCreateInfo;
    }

    auto VK_Device::CreateImage(const ImageInfo& info) -> ImageId { 
        auto [id, image_slot_variant] = m_GPUShaderResourceTable->m_ImageSlots.NewSlot();
        //GFX_DBG_ASSERT_TRUE_M(image_info.dimensions >= 1 && image_info.dimensions <= 3, "image dimensions must be a value between 1 to 3(inclusive)");
        ImplImageSlot ret = {};
        ret.zombie = false;
        ret.info = info;
        ret.viewSlot.info = ImageViewInfo{
            .type = static_cast<ImageViewType>(info.dimensions - 1),
            .format = info.format,
            .image = {id},
            .slice = ImageMipArraySlice{
                .baseMipLevel = 0,
                .levelCount = info.mipLevelCount,
                .baseArrayLayer = 0,
                .layerCount = info.arrayLayerCount,
            },
            .name = info.name,
        };
        ret.aspectFlags = inferAspectFromFormat(info.format);
        const VkImageCreateInfo vkImageCreateInfo = initializeImageCreateInfoFromImageInfo(info);
        const VmaAllocationCreateInfo vmaAllocationCreateInfo{
            .flags = static_cast<VmaAllocationCreateFlags>(info.memoryFlags),
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<uint>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };


        if (vmaCreateImage(m_VmaAllocator, &vkImageCreateInfo, &vmaAllocationCreateInfo, &ret.vkImage, &ret.vmaAllocation, nullptr) != VK_SUCCESS) {
            LOG_CORE_CRITICAL("failed to create image!");
        }

        VkImageViewType vkImageViewType = {};
        if (info.arrayLayerCount > 1) {
            if (!(info.dimensions >= 1 && info.dimensions <= 2)) {
                LOG_CORE_CRITICAL("image dimensions must be 1 or 2 if making an image array!");
            }

            vkImageViewType = static_cast<VkImageViewType>(info.dimensions + 3);
        }
        else {
            vkImageViewType = static_cast<VkImageViewType>(info.dimensions - 1);
        }

        const VkImageViewCreateInfo vkImageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .image = ret.vkImage,
            .viewType = vkImageViewType,
            .format = *reinterpret_cast<const VkFormat*>(&info.format),
            .components = VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = ret.aspectFlags,
                .baseMipLevel = 0,
                .levelCount = info.mipLevelCount,
                .baseArrayLayer = 0,
                .layerCount = info.arrayLayerCount,
            },
        };

        if (vkCreateImageView(m_Device, &vkImageViewCreateInfo, nullptr, &ret.viewSlot.vkImageView) != VK_SUCCESS) {
            LOG_CORE_CRITICAL("failed to create image view!");
        }

        //if (this->impl_ctx.as<ImplInstance>()->info.enable_debug_names && !info.name.empty()) {
        //    auto image_name = image_info.name;
        //    const VkDebugUtilsObjectNameInfoEXT swapchain_image_name_info{
        //        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        //        .pNext = nullptr,
        //        .objectType = VK_OBJECT_TYPE_IMAGE,
        //        .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image),
        //        .pObjectName = image_name.c_str(),
        //    };
        //    vkSetDebugUtilsObjectNameEXT(m_Device, &swapchain_image_name_info);

        //    auto image_view_name = image_info.name;
        //    const VkDebugUtilsObjectNameInfoEXT swapchain_image_view_name_info{
        //        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        //        .pNext = nullptr,
        //        .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
        //        .objectHandle = reinterpret_cast<uint64_t>(ret.view_slot.vk_image_view),
        //        .pObjectName = image_view_name.c_str(),
        //    };
        //    vkSetDebugUtilsObjectNameEXT(m_Device, &swapchain_image_view_name_info);
        //}
        m_GPUShaderResourceTable->WriteDescriptorSetImage(ret.viewSlot.vkImageView, info.usage, id.index);

        image_slot_variant = ret;

        return ImageId{ id };
    }

    auto VK_Device::CreateImageView(const ImageViewInfo& info) -> ImageViewId { 
        auto [id, imageSlot] = m_GPUShaderResourceTable->m_ImageSlots.NewSlot();
        imageSlot = {};
        const ImplImageSlot& parentImageSlot = GetImageSlot(info.image);
        ImplImageViewSlot ret = {};
        ret.info = info;
        ImageMipArraySlice slice = validateImageSlice(info.slice, info.image);
        ret.info.slice = slice;
        const VkImageViewCreateInfo vkImageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .image = parentImageSlot.vkImage,
            .viewType = static_cast<VkImageViewType>(info.type),
            .format = *reinterpret_cast<const VkFormat*>(&info.format),
            .components = VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = makeSubressourceRange(slice, parentImageSlot.aspectFlags),
        };

        if (vkCreateImageView(m_Device, &vkImageViewCreateInfo, nullptr, &ret.vkImageView) != VK_SUCCESS) {
            LOG_CORE_CRITICAL("failed to create image view!");
        }
 /*       if (this->impl_ctx.as<ImplInstance>()->info.enable_debug_names && !image_view_info.name.empty()) {
            auto image_view_name = image_view_info.name;
            const VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
                .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image_view),
                .pObjectName = image_view_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(this->vk_device, &name_info);
        }*/
        //write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, ret.vk_image_view, parentImageSlot.info.usage, id.index);
        m_GPUShaderResourceTable->WriteDescriptorSetImage(ret.vkImageView, parentImageSlot.info.usage, id.index);
        imageSlot.viewSlot = ret;
        return ImageViewId{ id };
    }

    auto VK_Device::CreateSampler(const SamplerInfo& info) -> SamplerId {
        auto [id, ret] = m_GPUShaderResourceTable->m_SamplerSlots.NewSlot();

        ret.info = info;
        ret.zombie = false;

        VkSamplerReductionModeCreateInfo vk_sampler_reduction_mode_create_info{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO,
            .pNext = nullptr,
            .reductionMode = static_cast<VkSamplerReductionMode>(info.reductionMode),
        };

        if (!(info.mipmapFilter != Filter::CUBIC_IMG)) {
            LOG_CORE_CRITICAL("can not use cube addressing for mipmap filtering!");
        }

        const VkSamplerCreateInfo vkSamplerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = reinterpret_cast<void*>(&vk_sampler_reduction_mode_create_info),
            .flags = {},
            .magFilter = static_cast<VkFilter>(info.magnificationFilter),
            .minFilter = static_cast<VkFilter>(info.minificationFilter),
            .mipmapMode = static_cast<VkSamplerMipmapMode>(info.mipmapFilter),
            .addressModeU = static_cast<VkSamplerAddressMode>(info.addressModeU),
            .addressModeV = static_cast<VkSamplerAddressMode>(info.addressModeV),
            .addressModeW = static_cast<VkSamplerAddressMode>(info.addressModeW),
            .mipLodBias = info.mipLodBias,
            .anisotropyEnable = static_cast<VkBool32>(info.enableAnisotropy),
            .maxAnisotropy = info.maxAnisotropy,
            .compareEnable = static_cast<VkBool32>(info.enableCompare),
            .compareOp = static_cast<VkCompareOp>(info.compareOp),
            .minLod = info.minLod,
            .maxLod = info.maxLod,
            .borderColor = static_cast<VkBorderColor>(info.borderColor),
            .unnormalizedCoordinates = static_cast<VkBool32>(info.enableUnnormalizedCoordinates),
        };

        if (vkCreateSampler(m_Device, &vkSamplerCreateInfo, nullptr, &ret.vkSampler) != VK_SUCCESS) {
            LOG_CORE_CRITICAL("failed to create sampler");
        }

        //if (this->impl_ctx.as<ImplInstance>()->info.enable_debug_names && !info.name.empty()) {
        //    auto sampler_name = info.name;
        //    const VkDebugUtilsObjectNameInfoEXT sampler_name_info{
        //        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        //        .pNext = nullptr,
        //        .objectType = VK_OBJECT_TYPE_SAMPLER,
        //        .objectHandle = reinterpret_cast<uint64_t>(ret.vk_sampler),
        //        .pObjectName = sampler_name.c_str(),
        //    };
        //    vkSetDebugUtilsObjectNameEXT(this->vk_device, &sampler_name_info);
        //}

        m_GPUShaderResourceTable->WriteDescriptorSetSampler(ret.vkSampler, id.index);

        return SamplerId{ id };
    }

    void VK_Device::DestroyBuffer(const BufferId& id) {
        ImplBufferSlot& bufferSlot = m_GPUShaderResourceTable->m_BufferSlots.DereferenceId(id);
        vmaDestroyBuffer(m_VmaAllocator, bufferSlot.vkBuffer, bufferSlot.vmaAllocation);
        bufferSlot = {};
        m_GPUShaderResourceTable->m_BufferSlots.ReturnSlot(id);
    }

    void VK_Device::DestroyImage(const ImageId& id) {
        ImplImageSlot& imageSlot = m_GPUShaderResourceTable->m_ImageSlots.DereferenceId(id);
        std::cout << "TODO: destroy image" << std::endl;
        //m_GPUShaderResourceTable->write_descriptor_set_image(this->vk_null_image_view, image_slot.info.usage, id.index);
        vkDestroyImageView(m_Device, imageSlot.viewSlot.vkImageView, nullptr);
        if (imageSlot.swapchainImageIndex == NOT_OWNED_BY_SWAPCHAIN) {
            vmaDestroyImage(m_VmaAllocator, imageSlot.vkImage, imageSlot.vmaAllocation);
        }
        imageSlot = {};
        m_GPUShaderResourceTable->m_ImageSlots.ReturnSlot(id);
    }

    void VK_Device::DestroyImageView(const ImageViewId& id) {
        if (m_GPUShaderResourceTable->m_ImageSlots.DereferenceId(id).vkImage != VK_NULL_HANDLE) {
            LOG_CORE_CRITICAL("can not destroy default image view of image");
        }

        ImplImageViewSlot& imageSlot = m_GPUShaderResourceTable->m_ImageSlots.DereferenceId(id).viewSlot;
        std::cout << "TODO: destroy image view" << std::endl;
        //m_GPUShaderResourceTable->write_descriptor_set_image(this->vk_null_image_view, ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::SHADER_SAMPLED, id.index);
        vkDestroyImageView(m_Device, imageSlot.vkImageView, nullptr);
        imageSlot = {};
        m_GPUShaderResourceTable->m_ImageSlots.ReturnSlot(id);
    }

    void VK_Device::DestroySampler(const SamplerId& id) {
        ImplSamplerSlot& samplerSlot = m_GPUShaderResourceTable->m_SamplerSlots.DereferenceId(id);
        std::cout << "TODO: destroy sampler" << std::endl;
        //m_GPUShaderResourceTable->write_descriptor_set_sampler(m_Device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_sampler, id.index);
        vkDestroySampler(m_Device, samplerSlot.vkSampler, nullptr);
        samplerSlot = {};
        m_GPUShaderResourceTable->m_SamplerSlots.ReturnSlot(id);
    }

    auto VK_Device::GetBufferSlot(const BufferId& id) -> ImplBufferSlot {
        return m_GPUShaderResourceTable->m_BufferSlots.DereferenceId(id);
    }

    auto VK_Device::GetImageSlot(const ImageId& id) -> ImplImageSlot {
        return m_GPUShaderResourceTable->m_ImageSlots.DereferenceId(id);
    }

    auto VK_Device::GetImageViewSlot(const ImageViewId& id) -> ImplImageViewSlot {
        return m_GPUShaderResourceTable->m_ImageSlots.DereferenceId(id).viewSlot;
    }

    auto VK_Device::GetSamplerSlot(const SamplerId& id) -> ImplSamplerSlot {
        return m_GPUShaderResourceTable->m_SamplerSlots.DereferenceId(id);
    }
}
