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

    VK_Buffer::VK_Buffer(VkDeviceSize instanceSize, uint instanceCount, VkBufferUsageFlags usageFlags,
                         VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment)
        : m_Device(VK_Core::m_Device), m_InstanceSize{instanceSize}, m_InstanceCount{instanceCount},
          m_UsageFlags{usageFlags}, m_MemoryPropertyFlags{memoryPropertyFlags}
    {
        m_AlignmentSize = GetAlignment(m_InstanceSize, minOffsetAlignment);
        m_BufferSize = m_AlignmentSize * m_InstanceCount;
        m_Device->CreateBuffer(m_BufferSize, m_UsageFlags, m_MemoryPropertyFlags, m_Buffer, m_Memory);
    }

    VK_Buffer::VK_Buffer(uint size, Buffer::BufferUsage bufferUsage) : m_Device(VK_Core::m_Device)
    {
        switch (bufferUsage)
        {
            case Buffer::BufferUsage::UNIFORM_BUFFER_VISIBLE_TO_CPU:
            {
                m_InstanceSize = size;
                m_InstanceCount = 1;
                m_UsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                m_MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

                VkDeviceSize minOffsetAlignment = m_Device->m_Properties.limits.minUniformBufferOffsetAlignment;

                m_AlignmentSize = GetAlignment(m_InstanceSize, minOffsetAlignment);
                m_BufferSize = m_AlignmentSize * m_InstanceCount;
                m_Device->CreateBuffer(m_BufferSize, m_UsageFlags, m_MemoryPropertyFlags, m_Buffer, m_Memory);
#ifdef DEBUG
                auto maxUniformBufferRange = m_Device->m_Properties.limits.maxUniformBufferRange;
                if (m_BufferSize > maxUniformBufferRange)
                {
                    std::string str =
                        std::string("VK_Buffer::VK_Buffer, usage BufferUsage::UNIFORM_BUFFER_VISIBLE_TO_CPU, buffer size ") +
                        std::to_string(m_BufferSize) + std::string(" is larger than ") +
                        std::to_string(maxUniformBufferRange);
                    CORE_HARD_STOP(str);
                }
#endif
                break;
            }
            case Buffer::BufferUsage::STORAGE_BUFFER_VISIBLE_TO_CPU:
            {
                m_InstanceSize = size;
                m_InstanceCount = 1;
                m_UsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                m_MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

                VkDeviceSize minOffsetAlignment = m_Device->m_Properties.limits.minUniformBufferOffsetAlignment;

                m_AlignmentSize = GetAlignment(m_InstanceSize, minOffsetAlignment);
                m_BufferSize = m_AlignmentSize * m_InstanceCount;
                m_Device->CreateBuffer(m_BufferSize, m_UsageFlags, m_MemoryPropertyFlags, m_Buffer, m_Memory);
                break;
            }
            default:
            {
                LOG_CORE_CRITICAL("unrecognized buffer usage");
            }
        }
    }

    VK_Buffer::~VK_Buffer()
    {
        Unmap();
        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkDestroyBuffer(m_Device->Device(), m_Buffer, nullptr);
            vkFreeMemory(m_Device->Device(), m_Memory, nullptr);
        }
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
    VkResult VK_Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
    {
        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
        if (!(m_Buffer && m_Memory))
        {
            LOG_CORE_CRITICAL("VkResult VK_Buffer::Map(...): Called map on buffer before create");
        }
        auto result = vkMapMemory(m_Device->Device(), m_Memory, offset, size, 0, &m_Mapped);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            CORE_HARD_STOP("VK_Buffer::Map");
        }
        return result;
    }

    void VK_Buffer::MapBuffer() { Map(); }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void VK_Buffer::Unmap()
    {
        if (m_Mapped)
        {
            {
                std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
                vkUnmapMemory(m_Device->Device(), m_Memory);
            }
            m_Mapped = nullptr;
        }
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
    void VK_Buffer::WriteToBuffer(const void* data, VkDeviceSize size, VkDeviceSize offset)
    {
        if (!(m_Mapped))
            LOG_CORE_CRITICAL("void VK_Buffer::WriteToBuffer(...): cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE)
        {
            memcpy(m_Mapped, data, m_BufferSize);
        }
        else
        {
            char* memOffset = (char*)m_Mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
     * complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the flush call
     */
    VkResult VK_Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_Memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        VkResult result;
        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            result = vkFlushMappedMemoryRanges(m_Device->Device(), 1, &mappedRange);
        }
        return result;
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
     * the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the invalidate call
     */
    VkResult VK_Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_Memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(m_Device->Device(), 1, &mappedRange);
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
        return VkDescriptorBufferInfo{
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
    void VK_Buffer::WriteToIndex(void* data, int index) { WriteToBuffer(data, m_InstanceSize, index * m_AlignmentSize); }

    /**
     *  Flush the memory range at index * m_AlignmentSize of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    VkResult VK_Buffer::FlushIndex(int index) { return Flush(m_AlignmentSize, index * m_AlignmentSize); }

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

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * m_AlignmentSize
     *
     * @return VkResult of the invalidate call
     */
    VkResult VK_Buffer::InvalidateIndex(int index) { return Invalidate(m_AlignmentSize, index * m_AlignmentSize); }
} // namespace GfxRenderEngine
