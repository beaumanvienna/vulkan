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
#include "bindless/VKbindlessBuffer.h"

namespace GfxRenderEngine
{
    VK_BindlessBuffer::VK_BindlessBuffer()
        : m_NextBindlessIndex{0}, m_BindlessBufferSetLayout{VK_NULL_HANDLE}, m_DescriptorPoolBuffers{VK_NULL_HANDLE},
          m_BindlessSetBuffers{VK_NULL_HANDLE}
    {
        // Reserve container capacities to avoid rehash/multiple allocations
        m_BufferID2BindlessBufferID.reserve(TEXTURE_ID_2_BINDLESS_ID_PREALLOC);
        m_PendingUpdates.reserve(PENDING_UPDATES_PREALLOC);

        CreateDescriptorSetLayout();
        CreateDescriptorPool();
        CreateDescriptorSet();
    }

    VK_BindlessBuffer::~VK_BindlessBuffer()
    {
        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);

        vkDestroyDescriptorSetLayout(VK_Core::m_Device->Device(), m_BindlessBufferSetLayout, nullptr);
        vkDestroyDescriptorPool(VK_Core::m_Device->Device(), m_DescriptorPoolBuffers, nullptr);
        m_BindlessBufferSetLayout = VK_NULL_HANDLE;
        m_DescriptorPoolBuffers = VK_NULL_HANDLE;
        m_BindlessSetBuffers = VK_NULL_HANDLE;
    }

    void VK_BindlessBuffer::CreateDescriptorSetLayout()
    { // bindless array of storage buffers
        VkDescriptorSetLayoutBinding bindlessBufferBinding{};
        bindlessBufferBinding.binding = 0;
        bindlessBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindlessBufferBinding.descriptorCount = static_cast<uint>(MAX_DESCRIPTOR); // upper bound, large enough for Lucre
        bindlessBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

        auto bindingFlags = std::to_array<VkDescriptorBindingFlags>(
            {VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT});

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.bindingCount = static_cast<uint>(bindingFlags.size());
        bindingFlagsInfo.pBindingFlags = bindingFlags.data();

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = &bindingFlagsInfo;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &bindlessBufferBinding;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result =
                vkCreateDescriptorSetLayout(VK_Core::m_Device->Device(), &layoutInfo, nullptr, &m_BindlessBufferSetLayout);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("Failed to create descriptor set layout for bindless textures");
                m_BindlessBufferSetLayout = VK_NULL_HANDLE;
            }
        }
    }
    void VK_BindlessBuffer::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSize.descriptorCount = static_cast<uint>(MAX_DESCRIPTOR);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT; // required for bindless

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result = vkCreateDescriptorPool(VK_Core::m_Device->Device(), &poolInfo, nullptr, &m_DescriptorPoolBuffers);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("Failed to create descriptor pool for bindless textures");
                m_DescriptorPoolBuffers = VK_NULL_HANDLE;
            }
        }
    }

    void VK_BindlessBuffer::CreateDescriptorSet()
    {
        uint descriptorCount = static_cast<uint>(MAX_DESCRIPTOR);

        VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
        countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        countInfo.descriptorSetCount = 1;
        countInfo.pDescriptorCounts = &descriptorCount;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPoolBuffers;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_BindlessBufferSetLayout;
        allocInfo.pNext = &countInfo;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result = vkAllocateDescriptorSets(VK_Core::m_Device->Device(), &allocInfo, &m_BindlessSetBuffers);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("Failed to allocate bindless descriptor set!");
                m_BindlessSetBuffers = VK_NULL_HANDLE;
            }
        }
    }

    VK_BindlessBuffer::BindlessBufferID VK_BindlessBuffer::AddBuffer(StorageBuffer* storageBuffer)
    {
        StorageBuffer::StorageBufferID storageBufferID = storageBuffer->GetStorageBufferID();

        // guard map + pending vector
        std::lock_guard<std::mutex> guard(m_Mutex);

        if (m_NextBindlessIndex >= MAX_DESCRIPTOR)
        {
            LOG_CORE_CRITICAL("Bindless descriptor array overflow: exceeded {0}", MAX_DESCRIPTOR);
            return BINDLESS_ID_TEXTURE_ATLAS; // use texture atlas instead
        }

        // check if the texture is already registered (single lookup via find() plus iterator)
        auto it = m_BufferID2BindlessBufferID.find(storageBufferID);
        if (it != m_BufferID2BindlessBufferID.end())
        {
            return it->second; // already registered
        }

        uint bindlessIndex = m_NextBindlessIndex;
        m_BufferID2BindlessBufferID.emplace(storageBufferID, bindlessIndex);
        m_PendingUpdates.push_back(storageBuffer);
        ++m_NextBindlessIndex;

        return bindlessIndex;
    }

    void VK_BindlessBuffer::UpdateBindlessDescriptorSets()
    {
        // Lock the mutex for a short period to move pending items
        std::vector<StorageBuffer*> pendingUpdates;
        {
            std::lock_guard<std::mutex> guard(m_Mutex);
            if (m_PendingUpdates.empty())
            {
                return; // No updates are needed
            }
            pendingUpdates = std::move(m_PendingUpdates);
            m_PendingUpdates = {}; // as per discussion with Lukases and No Life
            m_PendingUpdates.reserve(PENDING_UPDATES_PREALLOC);
        }

        // Prepare the writes outside the lock
        const size_t numberOfUpdates = pendingUpdates.size();
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(numberOfUpdates);

        for (size_t index = 0; index < numberOfUpdates; ++index)
        {
            StorageBuffer* storageBuffer = pendingUpdates[index];
            StorageBuffer::StorageBufferID storageBufferID = storageBuffer->GetStorageBufferID();

            // find index; if missing, skip this texture
            auto it = m_BufferID2BindlessBufferID.find(storageBufferID);
            if (it == m_BufferID2BindlessBufferID.end())
            {
                // should not happen, but skip defensively
                LOG_CORE_WARN("Buffer ID {} not found in bindless map while updating descriptors", storageBufferID);
                continue;
            }

            const uint bindlessIndex = it->second;

            const VkDescriptorBufferInfo& bufferInfo =
                static_cast<VK_StorageBuffer*>(storageBuffer)->GetDescriptorBufferInfo();

            descriptorWrites.emplace_back(VkWriteDescriptorSet{
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, // sType
                nullptr,                                // pNext
                m_BindlessSetBuffers,                   // dstSet
                0,                                      // dstBinding (Assuming binding 0 for texture array)
                bindlessIndex,                          // dstArrayElement
                1,                                      // descriptorCount
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,      // descriptorType
                nullptr,                                // pImageInfo
                &bufferInfo,                            // pBufferInfo
                nullptr                                 // pTexelBufferView
            });
        }

        if (descriptorWrites.empty())
        {
            return;
        }

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            // Perform the batched update
            vkUpdateDescriptorSets(VK_Core::m_Device->Device(), static_cast<uint>(descriptorWrites.size()),
                                   descriptorWrites.data(), 0, nullptr);
        }
    }
} // namespace GfxRenderEngine
