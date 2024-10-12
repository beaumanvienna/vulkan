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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once
#include <unordered_map>

#include "engine.h"
#include "VKdescriptor.h"
#include "VKdeviceStructs.h"
#include "auxiliary/threadPool.h"

namespace GfxRenderEngine
{
    class VK_Pool
    {

    public:
        VK_Pool(VkDevice& device, QueueFamilyIndices& queueFamilyIndices, ThreadPool& threadPoolPrimary,
                ThreadPool& threadPoolSecondary);
        ~VK_Pool();

        VkCommandPool& GetCommandPool();
        VK_DescriptorPool& GetDescriptorPool();
        VkSemaphore& GetUploadSemaphore();
        uint64& GetSignalValue();

        void ResetCommandPool();
        void ResetCommandPools(ThreadPool& threadpool);

        void ResetDescriptorPool();
        void ResetDescriptorPools(ThreadPool& threadpool);

        // Not copyable or movable
        VK_Pool(const VK_Pool&) = delete;
        VK_Pool& operator=(const VK_Pool&) = delete;
        VK_Pool(VK_Pool&&) = delete;
        VK_Pool& operator=(VK_Pool&&) = delete;

    private:
        VkDevice& m_Device;
        QueueFamilyIndices& m_QueueFamilyIndices;
        ThreadPool& m_PoolPrimary;
        ThreadPool& m_PoolSecondary;

        // pool of command pools
        std::unordered_map<uint64, VkCommandPool> m_CommandPools;
        // pool of descriptor pools
        std::unordered_map<uint64, std::unique_ptr<VK_DescriptorPool>> m_DescriptorPools;
        // pool of upload semaphores
        std::unordered_map<uint64, VkSemaphore> m_UploadSemaphores;

        // pool of upload signal values
        struct SignalValue
        {
            static constexpr uint64 DEFAULT_CONSTRUCTED = -1;
            uint64 m_Value{DEFAULT_CONSTRUCTED};
            uint64& Get() { return m_Value; }
        };
        std::unordered_map<uint64, SignalValue> m_SignalValues;
    };
} // namespace GfxRenderEngine
