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

#pragma once

#include <vulkan/vulkan.h>

#include "engine.h"
#include "renderer/buffer.h"

#include "VKcore.h"

namespace GfxRenderEngine
{
    class VK_Buffer : public Buffer
    {

    public:
        VK_Buffer(VkDeviceSize instanceSize, uint instanceCount, VkBufferUsageFlags usageFlags,
                  VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);

        VK_Buffer(uint size, Buffer::BufferUsage bufferUsage = Buffer::BufferUsage::UNIFORM_BUFFER_VISIBLE_TO_CPU);

        virtual ~VK_Buffer() override;

        VK_Buffer(const VK_Buffer&) = delete;
        VK_Buffer& operator=(const VK_Buffer&) = delete;

        VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        virtual void MapBuffer() override;
        void Unmap();

        void WriteToBuffer(const void* data, VkDeviceSize size, VkDeviceSize offset);
        virtual void WriteToBuffer(const void* data) override
        {
            WriteToBuffer(data, VK_WHOLE_SIZE /*VkDeviceSize size*/, 0 /*VkDeviceSize offset*/);
        }
        VkResult Flush(VkDeviceSize size, VkDeviceSize offset);
        virtual bool Flush() override
        {
            auto result = Flush(VK_WHOLE_SIZE /*VkDeviceSize size*/, 0 /*VkDeviceSize offset*/);
            return result == VkResult::VK_SUCCESS;
        }
        VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void WriteToIndex(void* data, int index);
        VkResult FlushIndex(int index);
        VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
        VkResult InvalidateIndex(int index);

        VkBuffer GetBuffer() const { return m_Buffer; }
        void* GetMappedMemory() const { return m_Mapped; }
        uint GetInstanceCount() const { return m_InstanceCount; }
        VkDeviceSize GetInstanceSize() const { return m_InstanceSize; }
        VkDeviceSize GetAlignmentSize() const { return m_AlignmentSize; }
        VkBufferUsageFlags GetUsageFlags() const { return m_UsageFlags; }
        VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return m_MemoryPropertyFlags; }
        VkDeviceSize GetBufferSize() const { return m_BufferSize; }

    private:
        static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

    private:
        VK_Device* m_Device;
        void* m_Mapped = nullptr;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;

        VkDeviceSize m_BufferSize;
        uint m_InstanceCount;
        VkDeviceSize m_InstanceSize;
        VkDeviceSize m_AlignmentSize;
        VkBufferUsageFlags m_UsageFlags;
        VkMemoryPropertyFlags m_MemoryPropertyFlags;
    };
} // namespace GfxRenderEngine
