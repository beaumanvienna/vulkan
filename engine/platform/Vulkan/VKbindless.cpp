/* Engine Copyright (c) 2025 Engine Development Team
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

#include <vulkan/vulkan.h>

#include "VKcore.h"
#include "VKbindless.h"

namespace GfxRenderEngine
{
    VK_Bindless::VK_Bindless() : m_Counter{0}
    {
        CreateDescriptorSetLayout();
        CreateDescriptorPool();
        CreateDescriptorSet();
    }

    VK_Bindless::~VK_Bindless()
    {
        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
        vkDestroyDescriptorSetLayout(VK_Core::m_Device->Device(), m_BindlessTextureSetLayout, nullptr);
        vkDestroyDescriptorPool(VK_Core::m_Device->Device(), m_DescriptorPoolTextures, nullptr);
    }

    void VK_Bindless::CreateDescriptorSetLayout()
    { // bindless array of sampled images (textures)
        VkDescriptorSetLayoutBinding bindlessTextureBinding{};
        bindlessTextureBinding.binding = 0;
        bindlessTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        bindlessTextureBinding.descriptorCount = MAX_DESCRIPTOR; // upper bound, large enough for Lucre
        bindlessTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
        bindlessTextureBinding.pImmutableSamplers = nullptr;

        VkDescriptorBindingFlags bindingFlags =
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.bindingCount = 1;
        bindingFlagsInfo.pBindingFlags = &bindingFlags;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = &bindingFlagsInfo;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &bindlessTextureBinding;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result =
                vkCreateDescriptorSetLayout(VK_Core::m_Device->Device(), &layoutInfo, nullptr, &m_BindlessTextureSetLayout);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("Failed to create descriptor set layout for bindless textures");
            }
        }
    }
    void VK_Bindless::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        poolSize.descriptorCount = MAX_DESCRIPTOR;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT; // required for bindless

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result = vkCreateDescriptorPool(VK_Core::m_Device->Device(), &poolInfo, nullptr, &m_DescriptorPoolTextures);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("Failed to create descriptor pool for bindless textures");
            }
        }
    }

    void VK_Bindless::CreateDescriptorSet()
    {
        // Say you want exactly N textures in this set
        uint descriptorCount = MAX_DESCRIPTOR;

        VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
        countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        countInfo.descriptorSetCount = 1;
        countInfo.pDescriptorCounts = &descriptorCount;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPoolTextures;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_BindlessTextureSetLayout;
        allocInfo.pNext = &countInfo;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result = vkAllocateDescriptorSets(VK_Core::m_Device->Device(), &allocInfo, &m_BindlessSetTextures);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("Failed to allocate bindless descriptor set!");
            }
        }
    }

    uint VK_Bindless::AddTexture(std::shared_ptr<Texture> const& texture)
    {
        auto tex = static_cast<VK_Texture*>(texture.get());
        return AddTexture(*tex);
    }

    uint VK_Bindless::AddTexture(VK_Texture const& texture)
    {
        uint returnValue = m_Counter;

        VkDescriptorImageInfo imageInfo;
        imageInfo.imageView = texture.GetDescriptorImageInfo().imageView;
        imageInfo.imageLayout = texture.GetDescriptorImageInfo().imageLayout;
        imageInfo.sampler = texture.GetDescriptorImageInfo().sampler;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = m_BindlessSetTextures;
        write.dstBinding = 0;              // binding we declared in the layout
        write.dstArrayElement = m_Counter; // start index in the array
        write.descriptorCount = 1;         // number of textures to write
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        write.pImageInfo = &imageInfo;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkUpdateDescriptorSets(VK_Core::m_Device->Device(), 1, &write, 0, nullptr);
        }
        ++m_Counter;
        return returnValue;
    }
} // namespace GfxRenderEngine
