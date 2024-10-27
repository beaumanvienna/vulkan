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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The code in this file is based on and inspired by the project
   https://github.com/TheCherno/Hazel. The license of this prject can
   be found under https://github.com/TheCherno/Hazel/blob/master/LICENSE
   */

#include "VKcore.h"
#include "VKdescriptor.h"

namespace GfxRenderEngine
{
    // *************** Descriptor Set Layout Builder *********************

    VK_DescriptorSetLayout::Builder& VK_DescriptorSetLayout::Builder::AddBinding(uint binding,
                                                                                 VkDescriptorType descriptorType,
                                                                                 VkShaderStageFlags stageFlags, uint count)
    {
        CORE_ASSERT(m_Bindings.count(binding) == 0, "binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        m_Bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<VK_DescriptorSetLayout> VK_DescriptorSetLayout::Builder::Build() const
    {
        return std::make_unique<VK_DescriptorSetLayout>(m_Bindings);
    }

    // *************** Descriptor Set Layout *********************

    VK_DescriptorSetLayout::VK_DescriptorSetLayout(std::unordered_map<uint, VkDescriptorSetLayoutBinding> bindings)
        : m_Bindings{bindings}
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings)
        {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result = vkCreateDescriptorSetLayout(VK_Core::m_Device->Device(), &descriptorSetLayoutInfo, nullptr,
                                                      &m_DescriptorSetLayout);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create descriptor set layout!");
            }
        }
    }

    VK_DescriptorSetLayout::~VK_DescriptorSetLayout()
    {
        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
        vkDestroyDescriptorSetLayout(VK_Core::m_Device->Device(), m_DescriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************
    VK_DescriptorPool::Builder::Builder(VkDevice device) : m_Device{device} {}

    VK_DescriptorPool::Builder& VK_DescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, uint count)
    {
        CORE_ASSERT(!m_MaxSetsCalled, "SetMaxSets() is optionally. It must be the final call before build().");
        m_MaxSets += count;
        m_PoolSizes.push_back({descriptorType, count});
        return *this;
    }

    VK_DescriptorPool::Builder& VK_DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
    {
        m_PoolFlags = flags;
        return *this;
    }

    VK_DescriptorPool::Builder& VK_DescriptorPool::Builder::SetMaxSets(uint count)
    {
        m_MaxSetsCalled = true;
        m_MaxSets = count;
        return *this;
    }

    std::unique_ptr<VK_DescriptorPool> VK_DescriptorPool::Builder::Build() const
    {
        return std::make_unique<VK_DescriptorPool>(m_Device, m_MaxSets, m_PoolFlags, m_PoolSizes);
    }

    // *************** Descriptor Pool *********************

    VK_DescriptorPool::VK_DescriptorPool(VkDevice device, uint maxSets, VkDescriptorPoolCreateFlags poolFlags,
                                         const std::vector<VkDescriptorPoolSize>& poolSizes)
        : m_Device{device}
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &m_DescriptorPool);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create descriptor pool!");
            }
        }
    }

    VK_DescriptorPool::~VK_DescriptorPool()
    {
        VK_Core::m_Device->WaitIdle();
        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        }
    }

    bool VK_DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout,
                                                  VkDescriptorSet& descriptor) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result = vkAllocateDescriptorSets(VK_Core::m_Device->Device(), &allocInfo, &descriptor);
            CORE_ASSERT(result == VK_SUCCESS, "vkAllocateDescriptorSets failed");
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                CORE_HARD_STOP("AllocateDescriptorSet failed");
                return false;
            }
        }
        return true;
    }

    void VK_DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
    {
        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
        vkFreeDescriptorSets(VK_Core::m_Device->Device(), m_DescriptorPool, static_cast<uint>(descriptors.size()),
                             descriptors.data());
    }

    void VK_DescriptorPool::ResetPool()
    {
        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
        vkResetDescriptorPool(VK_Core::m_Device->Device(), m_DescriptorPool, 0);
    }

    // *************** Descriptor Writer *********************
    VK_DescriptorWriter::VK_DescriptorWriter(VK_DescriptorSetLayout& setLayout)
        : m_SetLayout{setLayout}, m_DescriptorPool{VK_Core::m_Device->GetLoadPool()->GetDescriptorPool()}
    {
    }

    VK_DescriptorWriter& VK_DescriptorWriter::WriteBuffer(uint binding, const VkDescriptorBufferInfo& bufferInfo)
    {
        CORE_ASSERT(m_SetLayout.m_Bindings.count(binding) == 1, "layout does not contain specified binding");

        auto& bindingDescription = m_SetLayout.m_Bindings[binding];

        CORE_ASSERT(bindingDescription.descriptorCount == 1, "binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = &bufferInfo;
        write.descriptorCount = 1;

        m_Writes.push_back(write);
        return *this;
    }

    VK_DescriptorWriter& VK_DescriptorWriter::WriteImage(uint binding, const VkDescriptorImageInfo& imageInfo)
    {
        if (!(m_SetLayout.m_Bindings.count(binding) == 1)) // layout does not contain specified binding
        {
            LOG_CORE_CRITICAL("VK_DescriptorWriter::WriteImage: layout does not contain specified binding");
        }

        auto& bindingDescription = m_SetLayout.m_Bindings[binding];

        if (!(bindingDescription.descriptorCount == 1)) // binding single descriptor info, but binding expects multiple
        {
            LOG_CORE_CRITICAL(
                "VK_DescriptorWriter::WriteImage: binding single descriptor info, but binding expects multiple");
        }

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = &imageInfo;
        write.descriptorCount = 1;

        m_Writes.push_back(write);
        return *this;
    }

    VK_DescriptorWriter& VK_DescriptorWriter::WriteImage(uint binding,
                                                         const std::vector<VkDescriptorImageInfo>& imageInfoAll)
    {
        if (!(m_SetLayout.m_Bindings.count(binding) == 1)) // layout does not contain specified binding
        {
            LOG_CORE_CRITICAL("VK_DescriptorWriter::WriteImage: layout does not contain specified binding");
        }

        auto& bindingDescription = m_SetLayout.m_Bindings[binding];

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfoAll.data();
        write.descriptorCount = imageInfoAll.size();

        m_Writes.push_back(write);
        return *this;
    }

    bool VK_DescriptorWriter::Build(VkDescriptorSet& set)
    {
        bool success = m_DescriptorPool.AllocateDescriptorSet(m_SetLayout.GetDescriptorSetLayout(), set);
        CORE_ASSERT(success, "AllocateDescriptorSet failed");
        if (!success)
        {
            CORE_HARD_STOP("VK_DescriptorWriter::Build");
            return false;
        }
        Overwrite(set);
        return true;
    }

    void VK_DescriptorWriter::Overwrite(VkDescriptorSet& set)
    {
        for (auto& write : m_Writes)
        {
            write.dstSet = set;
        }
        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkUpdateDescriptorSets(VK_Core::m_Device->Device(), m_Writes.size(), m_Writes.data(), 0, nullptr);
        }
    }
} // namespace GfxRenderEngine
