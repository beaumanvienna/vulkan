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

    VK_DescriptorSetLayout::Builder& VK_DescriptorSetLayout::Builder::AddBinding
    (
        uint binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint count
    )
    {
        ASSERT(m_Bindings.count(binding) == 0); // binding already in use
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

    VK_DescriptorSetLayout::VK_DescriptorSetLayout
    (
        std::unordered_map<uint,
        VkDescriptorSetLayoutBinding> bindings)
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

        auto result = vkCreateDescriptorSetLayout(VK_Core::m_Device->Device(), &descriptorSetLayoutInfo,
                        nullptr, &m_DescriptorSetLayout);
        if (result != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create descriptor set layout!");
        }
    }

    VK_DescriptorSetLayout::~VK_DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(VK_Core::m_Device->Device(), m_DescriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    VK_DescriptorPool::Builder& VK_DescriptorPool::Builder::AddPoolSize
        (
            VkDescriptorType descriptorType, 
            uint count
        )
    {
        m_PoolSizes.push_back({descriptorType, count});
        return *this;
    }

    VK_DescriptorPool::Builder& VK_DescriptorPool::Builder::SetPoolFlags
        (
            VkDescriptorPoolCreateFlags flags
        )
    {
        m_PoolFlags = flags;
        return *this;
    }

    VK_DescriptorPool::Builder& VK_DescriptorPool::Builder::SetMaxSets(uint count)
    {
        m_MaxSets = count;
        return *this;
    }

    std::unique_ptr<VK_DescriptorPool> VK_DescriptorPool::Builder::Build() const
    {
        return std::make_unique<VK_DescriptorPool>(m_MaxSets, m_PoolFlags, m_PoolSizes);
    }

    // *************** Descriptor Pool *********************

    VK_DescriptorPool::VK_DescriptorPool
    (
        uint maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes
    )
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        auto result = vkCreateDescriptorPool
                      (
                        VK_Core::m_Device->Device(),
                        &descriptorPoolInfo,
                        nullptr,
                        &m_DescriptorPool
                      );
        if (result != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create descriptor pool!");
        }
    }

    VK_DescriptorPool::~VK_DescriptorPool()
    {
        vkDeviceWaitIdle(VK_Core::m_Device->Device());
        vkDestroyDescriptorPool(VK_Core::m_Device->Device(), m_DescriptorPool, nullptr);
    }

    bool VK_DescriptorPool::AllocateDescriptorSet
        (
            const VkDescriptorSetLayout descriptorSetLayout,
            VkDescriptorSet& descriptor
        ) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        auto result = vkAllocateDescriptorSets(VK_Core::m_Device->Device(), &allocInfo, &descriptor);
        if (result != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }

    void VK_DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
    {
        vkFreeDescriptorSets
        (
            VK_Core::m_Device->Device(),
            m_DescriptorPool,
            static_cast<uint>(descriptors.size()),
            descriptors.data()
        );
    }

    void VK_DescriptorPool::ResetPool()
    {
        vkResetDescriptorPool(VK_Core::m_Device->Device(), m_DescriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    VK_DescriptorWriter::VK_DescriptorWriter(VK_DescriptorSetLayout& setLayout, VK_DescriptorPool& pool)
        : m_SetLayout{setLayout}, m_Pool{pool} {}

    VK_DescriptorWriter& VK_DescriptorWriter::WriteBuffer
        (
            uint binding,
            VkDescriptorBufferInfo *bufferInfo
        )
    {
        ASSERT(m_SetLayout.m_Bindings.count(binding) == 1); // layout does not contain specified binding

        auto& bindingDescription = m_SetLayout.m_Bindings[binding];

        ASSERT(bindingDescription.descriptorCount == 1); // binding single descriptor info, but binding expects multiple

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        m_Writes.push_back(write);
        return *this;
    }

    VK_DescriptorWriter& VK_DescriptorWriter::WriteImage
        (
            uint binding,
            VkDescriptorImageInfo *imageInfo
        )
    {
        ASSERT(m_SetLayout.m_Bindings.count(binding) == 1) // layout does not contain specified binding

        auto& bindingDescription = m_SetLayout.m_Bindings[binding];

        ASSERT(bindingDescription.descriptorCount == 1); // binding single descriptor info, but binding expects multiple

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        m_Writes.push_back(write);
        return *this;
    }

    bool VK_DescriptorWriter::Build(VkDescriptorSet& set)
    {
        bool success = m_Pool.AllocateDescriptorSet(m_SetLayout.GetDescriptorSetLayout(), set);
        if (!success)
        {
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
        vkUpdateDescriptorSets(VK_Core::m_Device->Device(), m_Writes.size(), m_Writes.data(), 0, nullptr);
    }
}
