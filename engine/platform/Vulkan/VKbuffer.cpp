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
   
   Encapsulates a vulkan buffer
   Based on https://github.com/blurrypiano/littleVulkanEngine/blob/main/src/lve_buffer.cpp
   Initially based off VulkanBuffer by Sascha Willems -
   https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h */

#include "VKbuffer.h"

namespace GfxRenderEngine
{
    /**
     * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
     *
     * @param instanceSize The size of an instance
     * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
     * minUniformBufferOffsetAlignment)
     *
     * @return VkResult of the buffer mapping call
     */
    VkDeviceSize VK_Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
    {
      if (minOffsetAlignment > 0)
      {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
      }
      return instanceSize;
    }

    VK_Buffer::VK_Buffer(VkDeviceSize instanceSize, uint instanceCount,
                VkBufferUsageFlags usageFlags, MemoryFlags memoryPropertyFlags,
                VkDeviceSize minOffsetAlignment)
        : m_Device(VK_Core::m_Device.get()), m_InstanceSize{instanceSize}, m_InstanceCount{instanceCount},
          m_UsageFlags{usageFlags}, m_MemoryPropertyFlags{memoryPropertyFlags}
    {
        m_AlignmentSize = GetAlignment(m_InstanceSize, minOffsetAlignment);
        m_BufferSize = m_AlignmentSize * m_InstanceCount;
    
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = m_BufferSize;
        bufferInfo.usage = m_UsageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        bool hostAccessible = false;
        VmaAllocationInfo vmaAllocationInfo = {};
        auto vmaAllocationFlags = static_cast<VmaAllocationCreateFlags>(memoryPropertyFlags);
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

        if (vmaCreateBuffer(m_Device->GetVmaAllocator(), &bufferInfo, &vmaAllocationCreateInfo, &m_Buffer, &m_VmaAllocation, &vmaAllocationInfo) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create buffer!");
        }

        m_Mapped = hostAccessible ? vmaAllocationInfo.pMappedData : nullptr;
    }


    VK_Buffer::VK_Buffer(uint size, Buffer::BufferUsage bufferUsage)
            : m_Device(VK_Core::m_Device.get())
        {
            if (bufferUsage == Buffer::BufferUsage::SMALL_SHADER_DATA_BUFFER_VISIBLE_TO_CPU)
            {
                m_InstanceSize          = size;
                m_InstanceCount         = 1;
                m_UsageFlags            = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                m_MemoryPropertyFlags   = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                
                VkDeviceSize minOffsetAlignment = m_Device->m_Properties.limits.minUniformBufferOffsetAlignment;
                
                m_AlignmentSize = GetAlignment(m_InstanceSize, minOffsetAlignment);
                m_BufferSize = m_AlignmentSize * m_InstanceCount;
               
                VkBufferCreateInfo bufferInfo{};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = size;
                bufferInfo.usage = m_UsageFlags;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                bool hostAccessible = false;
                VmaAllocationInfo vmaAllocationInfo = {};
                auto vmaAllocationFlags = static_cast<VmaAllocationCreateFlags>(MemoryFlagBits::HOST_ACCESS_RANDOM);
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

                if (vmaCreateBuffer(m_Device->GetVmaAllocator(), &bufferInfo, &vmaAllocationCreateInfo, &m_Buffer, &m_VmaAllocation, &vmaAllocationInfo) != VK_SUCCESS)
                {
                    LOG_CORE_CRITICAL("failed to create buffer!");
                }

                m_Mapped = hostAccessible ? vmaAllocationInfo.pMappedData : nullptr;
            }
        }


    VK_Buffer::~VK_Buffer()
    {
        vmaDestroyBuffer(m_Device->GetVmaAllocator(), m_Buffer, m_VmaAllocation);
    }

    /**
     * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
     *
     * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
     * buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the buffer mapping call
     */

    void VK_Buffer::MapBuffer()
    {
        //Map();
    }

    /**
     * Copies the specified data to the m_Mapped buffer. Default value writes whole buffer range
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
     * range.
     * @param offset (Optional) Byte offset from beginning of m_Mapped region
     *
     */
    void VK_Buffer::WriteToBuffer(const void *data, VkDeviceSize size, VkDeviceSize offset)
    {
        if (!(m_Mapped)) LOG_CORE_CRITICAL("void VK_Buffer::WriteToBuffer(...): cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE)
        {
            memcpy(m_Mapped, data, m_BufferSize);
        }
        else
        {
            char *memOffset = (char *)m_Mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */
    VkDescriptorBufferInfo VK_Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
    {
        return VkDescriptorBufferInfo
        {
            m_Buffer,
            offset,
            size,
        };
    }

    /**
     * Copies "m_InstanceSize" bytes of data to the mapped buffer at an offset of index * m_AlignmentSize
     *
     * @param data Pointer to the data to copy
     * @param index Used in offset calculation
     *
     */
    void VK_Buffer::WriteToIndex(void *data, int index)
    {
        WriteToBuffer(data, m_InstanceSize, index * m_AlignmentSize);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * m_AlignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    VkDescriptorBufferInfo VK_Buffer::DescriptorInfoForIndex(int index)
    {
        return DescriptorInfo(m_AlignmentSize, index * m_AlignmentSize);
    }
}
