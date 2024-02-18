#include "VKgpuResources.h"

namespace GfxRenderEngine {
    auto GPUResourceId::isEmpty() const -> bool {
        return version == 0;
    }

    auto ImageId::defaultView() const -> ImageViewId {
        return ImageViewId{ {.index = index, .version = version} };
    }

    GPUShaderResourceTable::GPUShaderResourceTable(uint maxBuffers, uint maxImages, uint maxSamplers, VkDevice device, bool enableDebugNames) : m_VkDevice{device} {
        m_BufferSlots.m_MaxResources = maxBuffers;
        m_ImageSlots.m_MaxResources = maxImages;
        m_SamplerSlots.m_MaxResources = maxSamplers;

        const VkDescriptorPoolSize storageImageDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = static_cast<uint>(m_ImageSlots.m_MaxResources),
        };

        const VkDescriptorPoolSize sampledImageDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint>(m_ImageSlots.m_MaxResources),
        };

        const VkDescriptorPoolSize samplerDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<uint>(m_SamplerSlots.m_MaxResources),
        };

        const std::array<VkDescriptorPoolSize, 3> poolSizes = {
            storageImageDescriptorPoolSize,
            sampledImageDescriptorPoolSize,
            samplerDescriptorPoolSize,
        };

        const VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = static_cast<uint>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
        };

        vkCreateDescriptorPool(device, &vkDescriptorPoolCreateInfo, nullptr, &this->m_VkDescriptorPool);
        if (enableDebugNames) {
            auto descriptor_pool_name = "mega descriptor pool";
            VkDebugUtilsObjectNameInfoEXT descriptor_pool_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL,
                .objectHandle = reinterpret_cast<uint64_t>(m_VkDescriptorPool),
                .pObjectName = descriptor_pool_name,
            };
            //vkSetDebugUtilsObjectNameEXT(device, &descriptor_pool_name_info);
        }

        const VkDescriptorSetLayoutBinding storageImageDescriptorSetLayoutBinding{
            .binding = STORAGE_IMAGE_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = static_cast<uint>(m_ImageSlots.m_MaxResources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        const VkDescriptorSetLayoutBinding sampledImageDescriptorSetLayoutBinding{
            .binding = SAMPLED_IMAGE_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint>(m_ImageSlots.m_MaxResources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        const VkDescriptorSetLayoutBinding samplerDescriptorSetLayoutBinding{
            .binding = SAMPLER_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<uint>(m_SamplerSlots.m_MaxResources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        const std::array<VkDescriptorSetLayoutBinding, 3> descriptorSetLayoutBindings = {
            storageImageDescriptorSetLayoutBinding,
            sampledImageDescriptorSetLayoutBinding,
            samplerDescriptorSetLayoutBinding,
        };

        const std::array<VkDescriptorBindingFlags, 3> vkDescriptorBindingFlags = {
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        };

        VkDescriptorSetLayoutBindingFlagsCreateInfo vkDescriptorSetLayoutBindingFlagsCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = static_cast<uint>(vkDescriptorBindingFlags.size()),
            .pBindingFlags = vkDescriptorBindingFlags.data(),
        };

        const VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &vkDescriptorSetLayoutBindingFlagsCreateInfo,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = static_cast<uint>(descriptorSetLayoutBindings.size()),
            .pBindings = descriptorSetLayoutBindings.data(),
        };

        vkCreateDescriptorSetLayout(device, &vkDescriptorSetLayoutCreateInfo, nullptr, &this->m_VkDescriptorSetLayout);
        if (enableDebugNames) {
            auto name = "mega descriptor set layout";
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                .objectHandle = reinterpret_cast<uint64_t>(m_VkDescriptorSetLayout),
                .pObjectName = name,
            };
            //vkSetDebugUtilsObjectNameEXT(device, &name_info);
        }

        const VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = this->m_VkDescriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &this->m_VkDescriptorSetLayout,
        };

        vkAllocateDescriptorSets(device, &vkDescriptorSetAllocateInfo, &this->m_VkDescriptorSet);
        if (enableDebugNames) {
            auto name = "mega descriptor set";
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET,
                .objectHandle = reinterpret_cast<uint64_t>(m_VkDescriptorSet),
                .pObjectName = name,
            };
            //vkSetDebugUtilsObjectNameEXT(device, &name_info);
        }

        // Constant buffer set:

        std::array<VkDescriptorSetLayoutBinding, CONSTANT_BUFFER_BINDING_COUNT> constantBufferLayoutBindings = {};
        for (uint binding = 0; binding < CONSTANT_BUFFER_BINDING_COUNT; ++binding) {
            constantBufferLayoutBindings[binding] = VkDescriptorSetLayoutBinding{
                .binding = binding,
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = {},
            };
        }

        const std::array<VkDescriptorBindingFlags, CONSTANT_BUFFER_BINDING_COUNT> constantBufferSetBindingFlags = {
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
        };

        VkDescriptorSetLayoutBindingFlagsCreateInfo constantBufferSetBindingFlagsInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = static_cast<uint>(constantBufferSetBindingFlags.size()),
            .pBindingFlags = constantBufferSetBindingFlags.data(),
        };

        VkDescriptorSetLayoutCreateInfo constantBufferBindingsSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &constantBufferSetBindingFlagsInfo,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
            .bindingCount = CONSTANT_BUFFER_BINDING_COUNT,
            .pBindings = constantBufferLayoutBindings.data(),
        };

        vkCreateDescriptorSetLayout(device, &constantBufferBindingsSetLayoutCreateInfo, nullptr, &this->m_UniformBufferDescriptorSetLayout);
        if (enableDebugNames) {
            auto name = "uniform buffer set layout";
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                .objectHandle = reinterpret_cast<uint64_t>(m_UniformBufferDescriptorSetLayout),
                .pObjectName = name,
            };
            //vkSetDebugUtilsObjectNameEXT(device, &name_info);
        }

        /*std::array<VkDescriptorSetLayout, 2> vk_descriptor_set_layouts = { this->m_VkDescriptorSetLayout, this->uniform_buffer_descriptor_set_layout };
        VkPipelineLayoutCreateInfo vk_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .setLayoutCount = static_cast<uint>(vk_descriptor_set_layouts.size()),
            .pSetLayouts = vk_descriptor_set_layouts.data(),
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };

        vkCreatePipelineLayout(device, &vk_pipeline_create_info, nullptr, pipeline_layouts.data());
        if (enableDebugNames) {
            auto name = "pipeline layout (push constant size 0)";
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                .objectHandle = reinterpret_cast<uint64_t>(*pipeline_layouts.data()),
                .pObjectName = name,
            };
            //vkSetDebugUtilsObjectNameEXT(device, &name_info);
        }

        for (usize i = 1; i < PIPELINE_LAYOUT_COUNT; ++i) {
            const VkPushConstantRange vk_push_constant_range{
                .stageFlags = VK_SHADER_STAGE_ALL,
                .offset = 0,
                .size = static_cast<uint>(i * 4),
            };
            vk_pipeline_create_info.pushConstantRangeCount = 1;
            vk_pipeline_create_info.pPushConstantRanges = &vk_push_constant_range;
            vkCreatePipelineLayout(device, &vk_pipeline_create_info, nullptr, &pipeline_layouts.at(i));
            if (enableDebugNames) {
                auto name = fmt::format("pipeline layout (push constant size {})", i * 4);
                VkDebugUtilsObjectNameInfoEXT name_info{
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    .objectHandle = reinterpret_cast<uint64_t>(pipeline_layouts.at(i)),
                    .pObjectName = name.c_str(),
                };
                //vkSetDebugUtilsObjectNameEXT(device, &name_info);
            }
        }*/
    }

    GPUShaderResourceTable::~GPUShaderResourceTable() {
        auto printRemaining = [&](std::string prefix, auto& pages) {
            std::string ret{ prefix + "\nthis can happen due to not waiting for the gpu to finish executing, as gfx defers destruction. List of survivors:\n" };
            for (auto& page : pages) {
                if (page) {
                    for (auto& slot : *page) {
                        bool handleInvalid = {};
                        if constexpr (std::is_same_v<decltype(slot.first), ImplBufferSlot>) {
                            handleInvalid = slot.first.vkBuffer == VK_NULL_HANDLE;
                        }
                        if constexpr (std::is_same_v<decltype(slot.first), ImplImageSlot>) {
                            handleInvalid = slot.first.vkImage == VK_NULL_HANDLE;
                        }
                        if constexpr (std::is_same_v<decltype(slot.first), ImplSamplerSlot>) {
                            handleInvalid = slot.first.vkSampler == VK_NULL_HANDLE;
                        }
                        if (!handleInvalid) {
                            ret += std::string("debug name: \"") + std::string(slot.first.info.name) + '\"';
                            if (slot.first.zombie) {
                                ret += " (destroy was already called)";
                            }
                            ret += "\n";
                        }
                    }
                }
            }
            return ret;
        };

        printRemaining("detected leaked buffers; not all buffers have been destroyed before destroying the device;", m_BufferSlots.m_Pages);
        printRemaining("detected leaked images; not all images have been destroyed before destroying the device;", m_ImageSlots.m_Pages);
        printRemaining("detected leaked samplers; not all samplers have been destroyed before destroying the device;", m_SamplerSlots.m_Pages);

        vkDestroyDescriptorSetLayout(m_VkDevice, this->m_VkDescriptorSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(m_VkDevice, this->m_UniformBufferDescriptorSetLayout, nullptr);
        vkResetDescriptorPool(m_VkDevice, this->m_VkDescriptorPool, {});
        vkDestroyDescriptorPool(m_VkDevice, this->m_VkDescriptorPool, nullptr);
    }

    void GPUShaderResourceTable::WriteDescriptorSetSampler(VkSampler vkSampler, uint index) {
        const VkDescriptorImageInfo vkDescriptorImageInfo{
            .sampler = vkSampler,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        const VkWriteDescriptorSet vkWriteDescriptorSetStorage{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_VkDescriptorSet,
            .dstBinding = SAMPLER_BINDING,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &vkDescriptorImageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        vkUpdateDescriptorSets(m_VkDevice, 1, &vkWriteDescriptorSetStorage, 0, nullptr);
    }

    void GPUShaderResourceTable::WriteDescriptorSetImage(VkImageView vkImageView, ImageUsageFlags usage, uint index) {
        uint descriptorSetWriteCount = 0;
        std::array<VkWriteDescriptorSet, 2> descriptorSetWrites = {};

        const VkDescriptorImageInfo vkDescriptorImageInfo{
            .sampler = VK_NULL_HANDLE,
            .imageView = vkImageView,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        const VkWriteDescriptorSet vkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_VkDescriptorSet,
            .dstBinding = STORAGE_IMAGE_BINDING,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &vkDescriptorImageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        if ((usage & ImageUsageFlagBits::SHADER_STORAGE) != ImageUsageFlagBits::NONE) {
            descriptorSetWrites.at(descriptorSetWriteCount++) = vkWriteDescriptorSet;
        }

        const VkDescriptorImageInfo vkDescriptorImageInfoSampled{
            .sampler = VK_NULL_HANDLE,
            .imageView = vkImageView,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };

        const VkWriteDescriptorSet vkWriteDescriptorSetSampled{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_VkDescriptorSet,
            .dstBinding = SAMPLED_IMAGE_BINDING,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &vkDescriptorImageInfoSampled,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        if ((usage & ImageUsageFlagBits::SHADER_SAMPLED) != ImageUsageFlagBits::NONE) {
            descriptorSetWrites.at(descriptorSetWriteCount++) = vkWriteDescriptorSetSampled;
        }

        vkUpdateDescriptorSets(m_VkDevice, descriptorSetWriteCount, descriptorSetWrites.data(), 0, nullptr);
    }
}