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
#include "bindless/VKbindlessImage.h"

namespace GfxRenderEngine
{
    VK_BindlessImage::VK_BindlessImage()
        : m_NextBindlessIndex{0}, m_BindlessImageSetLayout{VK_NULL_HANDLE}, m_DescriptorPoolImages{VK_NULL_HANDLE},
          m_BindlessSetImages{VK_NULL_HANDLE}
    {
        // Reserve container capacities to avoid rehash/multiple allocations
        m_ImageID2BindlessImageID.reserve(TEXTURE_ID_2_BINDLESS_ID_PREALLOC);
        m_PendingUpdates.reserve(PENDING_UPDATES_PREALLOC);

        CreateDescriptorSetLayout();
        CreateDescriptorPool();
        CreateDescriptorSet();
    }

    VK_BindlessImage::~VK_BindlessImage()
    {
        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);

        vkDestroyDescriptorSetLayout(VK_Core::m_Device->Device(), m_BindlessImageSetLayout, nullptr);
        vkDestroyDescriptorPool(VK_Core::m_Device->Device(), m_DescriptorPoolImages, nullptr);
        m_BindlessImageSetLayout = VK_NULL_HANDLE;
        m_DescriptorPoolImages = VK_NULL_HANDLE;
        m_BindlessSetImages = VK_NULL_HANDLE;
    }

    void VK_BindlessImage::CreateDescriptorSetLayout()
    { // bindless array of storage images
        VkDescriptorSetLayoutBinding bindlessImageBinding{};
        bindlessImageBinding.binding = 0;
        bindlessImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        bindlessImageBinding.descriptorCount = static_cast<uint>(MAX_DESCRIPTOR); // upper bound, large enough for Lucre
        bindlessImageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

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
        layoutInfo.pBindings = &bindlessImageBinding;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result =
                vkCreateDescriptorSetLayout(VK_Core::m_Device->Device(), &layoutInfo, nullptr, &m_BindlessImageSetLayout);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("Failed to create descriptor set layout for bindless textures");
                m_BindlessImageSetLayout = VK_NULL_HANDLE;
            }
        }
    }
    void VK_BindlessImage::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        poolSize.descriptorCount = static_cast<uint>(MAX_DESCRIPTOR);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT; // required for bindless

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result = vkCreateDescriptorPool(VK_Core::m_Device->Device(), &poolInfo, nullptr, &m_DescriptorPoolImages);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("Failed to create descriptor pool for bindless textures");
                m_DescriptorPoolImages = VK_NULL_HANDLE;
            }
        }
    }

    void VK_BindlessImage::CreateDescriptorSet()
    {
        uint descriptorCount = static_cast<uint>(MAX_DESCRIPTOR);

        VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
        countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        countInfo.descriptorSetCount = 1;
        countInfo.pDescriptorCounts = &descriptorCount;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPoolImages;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_BindlessImageSetLayout;
        allocInfo.pNext = &countInfo;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            auto result = vkAllocateDescriptorSets(VK_Core::m_Device->Device(), &allocInfo, &m_BindlessSetImages);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("Failed to allocate bindless descriptor set!");
                m_BindlessSetImages = VK_NULL_HANDLE;
            }
        }
    }

    VK_BindlessImage::BindlessImageID VK_BindlessImage::AddImage(StorageImage* storageImage)
    {
        StorageImage::StorageImageID storageImageID = storageImage->GetStorageImageID();

        // guard map + pending vector
        std::lock_guard<std::mutex> guard(m_Mutex);

        if (m_NextBindlessIndex >= MAX_DESCRIPTOR)
        {
            LOG_CORE_CRITICAL("Bindless descriptor array overflow: exceeded {0}", MAX_DESCRIPTOR);
            return BINDLESS_ID_TEXTURE_ATLAS; // use texture atlas instead
        }

        // check if the texture is already registered (single lookup via find() plus iterator)
        auto it = m_ImageID2BindlessImageID.find(storageImageID);
        if (it != m_ImageID2BindlessImageID.end())
        {
            return it->second; // already registered
        }

        uint bindlessIndex = m_NextBindlessIndex;
        m_ImageID2BindlessImageID.emplace(storageImageID, bindlessIndex);
        m_PendingUpdates.push_back(storageImage);
        ++m_NextBindlessIndex;

        return bindlessIndex;
    }

    void VK_BindlessImage::UpdateBindlessDescriptorSets()
    {
        // Lock the mutex for a short period to move pending items
        std::vector<StorageImage*> pendingUpdates;
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
            StorageImage* storageImage = pendingUpdates[index];
            StorageImage::StorageImageID storageImageID = storageImage->GetStorageImageID();

            // find index; if missing, skip this texture
            auto it = m_ImageID2BindlessImageID.find(storageImageID);
            if (it == m_ImageID2BindlessImageID.end())
            {
                // should not happen, but skip defensively
                LOG_CORE_WARN("Image ID {} not found in bindless map while updating descriptors", storageImageID);
                continue;
            }

            const uint bindlessIndex = it->second;

            const VkDescriptorImageInfo& imageInfo = static_cast<VK_StorageImage*>(storageImage)->GetDescriptorImageInfo();

            descriptorWrites.emplace_back(VkWriteDescriptorSet{
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, // sType
                nullptr,                                // pNext
                m_BindlessSetImages,                    // dstSet
                0,                                      // dstBinding (Assuming binding 0 for texture array)
                bindlessIndex,                          // dstArrayElement
                1,                                      // descriptorCount
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,       // descriptorType
                &imageInfo,                             // pImageInfo
                nullptr,                                // pBufferInfo
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
