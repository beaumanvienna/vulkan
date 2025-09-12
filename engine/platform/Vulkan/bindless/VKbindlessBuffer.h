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

#pragma once

#include "engine.h"

#include "VKstorageBuffer.h"

namespace GfxRenderEngine
{
    class VK_BindlessBuffer
    {
    public:
        using BindlessBufferID = uint;

    public:
        VK_BindlessBuffer();
        ~VK_BindlessBuffer();

        // Not copyable or movable
        VK_BindlessBuffer(const VK_BindlessBuffer&) = delete;
        VK_BindlessBuffer& operator=(const VK_BindlessBuffer&) = delete;
        VK_BindlessBuffer(VK_BindlessBuffer&&) = delete;
        VK_BindlessBuffer& operator=(VK_BindlessBuffer&&) = delete;

        BindlessBufferID AddBuffer(StorageBuffer* storageBuffer);
        void UpdateBindlessDescriptorSets();

        [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_BindlessBufferSetLayout; }
        [[nodiscard]] VkDescriptorSet GetDescriptorSet() const { return m_BindlessSetBuffers; }
        [[nodiscard]] BindlessBufferID GetBufferCount() const { return m_NextBindlessIndex; }
        [[nodiscard]] BindlessBufferID GetMaxDescriptors() const { return MAX_DESCRIPTOR; }

    private:
        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSet();

    private:
        constexpr static BindlessBufferID MAX_DESCRIPTOR = 16384u;
        constexpr static BindlessBufferID BINDLESS_ID_TEXTURE_ATLAS = 0u;
        BindlessBufferID m_NextBindlessIndex;
        VkDescriptorSetLayout m_BindlessBufferSetLayout;
        VkDescriptorPool m_DescriptorPoolBuffers;
        VkDescriptorSet m_BindlessSetBuffers;
        std::mutex m_Mutex; // protect shared data

        // Map texture ID to bindless index
        constexpr static size_t TEXTURE_ID_2_BINDLESS_ID_PREALLOC = 4096u;
        std::unordered_map<StorageBuffer::StorageBufferID, BindlessBufferID> m_BufferID2BindlessBufferID;
        constexpr static size_t PENDING_UPDATES_PREALLOC = 256u;
        std::vector<StorageBuffer*> m_PendingUpdates;
    };
} // namespace GfxRenderEngine
