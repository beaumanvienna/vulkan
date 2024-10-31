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

#include <vulkan/vulkan.h>

#include "VKcore.h"
#include "VKpool.h"
#include "VKswapChain.h"

namespace GfxRenderEngine
{
    VK_Pool::VK_Pool(VkDevice& device, QueueFamilyIndices& queueFamilyIndices, ThreadPool& threadPoolPrimary,
                     ThreadPool& threadPoolSecondary)
        : m_Device{device}, m_QueueFamilyIndices{queueFamilyIndices}, m_PoolPrimary(threadPoolPrimary),
          m_PoolSecondary(threadPoolSecondary)
    {
        // helper lambdas
        auto createCommandPool = [this]()
        {
            VkCommandPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = m_QueueFamilyIndices.m_GraphicsFamily;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            VkCommandPool commandPool;
            {
                auto result = vkCreateCommandPool(m_Device, &poolInfo, nullptr, &commandPool);
                if (result != VK_SUCCESS)
                {
                    VK_Core::m_Device->PrintError(result);
                    LOG_CORE_CRITICAL("failed to create graphics command pool!");
                }
            }
            return commandPool;
        };

        auto createDescriptorPool = [device]()
        {
            static constexpr uint POOL_SIZE = 10000;
            std::unique_ptr<VK_DescriptorPool> descriptorPool =
                VK_DescriptorPool::Builder(device)
                    .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, POOL_SIZE)
                    .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, POOL_SIZE)
                    .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, POOL_SIZE)
                    .AddPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, POOL_SIZE)
                    .Build();
            return descriptorPool;
        };

        auto createUploadSemaphore = [this]()
        {
            VkSemaphore uploadSemaphore;
            VkSemaphoreTypeCreateInfo timelineCreateInfo{};
            timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            timelineCreateInfo.initialValue = 0;

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreInfo.pNext = &timelineCreateInfo;
            {
                auto result = vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &uploadSemaphore);
                if (result != VK_SUCCESS)
                {
                    VK_Core::m_Device->PrintError(result);
                    CORE_HARD_STOP("failed to create synchronization objects in VK_Pool::CreateSyncObjects()!");
                }
            }
            return uploadSemaphore;
        };

        auto emplacePoolObjects = [this, createCommandPool, createDescriptorPool, createUploadSemaphore](uint64 hash)
        {
            // create command pool
            m_CommandPools.emplace(hash, createCommandPool());

            // create descriptor pool
            m_DescriptorPools.emplace(hash, createDescriptorPool());

            // create upload semaphore pool
            m_UploadSemaphores.emplace(hash, createUploadSemaphore());

            // create signal value pool
            m_SignalValues.emplace(hash, 0);
        };

        auto loopOverWorkerThreads = [emplacePoolObjects](ThreadPool& pool)
        {
            // loop over all worker threads and fill the unordered maps for
            // thread id -> command pool and
            // thread id -> descriptor pool
            for (auto& threadID : pool.GetThreadIDs())
            {
                // Hash for thread id (created from a hasher class object and the thread id).
                uint64 hash = std::hash<std::thread::id>()(threadID);
                emplacePoolObjects(hash);
            }
        };

        loopOverWorkerThreads(m_PoolPrimary);
        loopOverWorkerThreads(m_PoolSecondary);

        // fill the unordered maps for also for main thread
        {
            // Calling get() on the future blocks until the worker thread from above terminates.
            std::thread::id threadID = std::this_thread::get_id();

            // Hash for thread id (created from a hasher class object and the thread id).
            uint64 hash = std::hash<std::thread::id>()(threadID);
            emplacePoolObjects(hash);
        }
    }

    VK_Pool::~VK_Pool()
    {
        for (auto& uploadSemaphore : m_UploadSemaphores)
        {
            vkDestroySemaphore(m_Device, uploadSemaphore.second, nullptr);
        }

        for (auto& commandPool : m_CommandPools)
        {
            vkDestroyCommandPool(m_Device, commandPool.second, nullptr);
        }
    }

    VkCommandPool& VK_Pool::GetCommandPool()
    {
        std::thread::id threadID = std::this_thread::get_id();
        uint64 hash = std::hash<std::thread::id>()(threadID);
        VkCommandPool& pool = m_CommandPools[hash];
        CORE_ASSERT(pool != nullptr, "no command pool found!");
        if (!pool)
        {
            std::cout << "thread id:" << threadID << std::endl;
            CORE_HARD_STOP("thread id");
        }
        return pool;
    }

    VK_DescriptorPool& VK_Pool::GetDescriptorPool()
    {
        std::thread::id threadID = std::this_thread::get_id();
        uint64 hash = std::hash<std::thread::id>()(threadID);
        CORE_ASSERT(m_DescriptorPools[hash] != nullptr, "no descriptor pool found!");
        if (!m_DescriptorPools[hash])
        {
            std::cout << "thread id:" << threadID << std::endl;
            CORE_HARD_STOP("thread id");
        }
        return *m_DescriptorPools[hash];
    }

    VkSemaphore& VK_Pool::GetUploadSemaphore()
    {
        std::thread::id threadID = std::this_thread::get_id();
        uint64 hash = std::hash<std::thread::id>()(threadID);
        CORE_ASSERT(m_UploadSemaphores[hash] != nullptr, "no upload semaphore found!");
        if (!m_UploadSemaphores[hash])
        {
            std::cout << "thread id:" << threadID << std::endl;
            CORE_HARD_STOP("thread id");
        }
        return m_UploadSemaphores[hash];
    }

    uint64& VK_Pool::GetSignalValue()
    {
        std::thread::id threadID = std::this_thread::get_id();
        uint64 hash = std::hash<std::thread::id>()(threadID);
        CORE_ASSERT(m_SignalValues[hash].Get() != SignalValue::DEFAULT_CONSTRUCTED, "no signal value found!");
        if (m_SignalValues[hash].Get() == SignalValue::DEFAULT_CONSTRUCTED)
        {
            std::cout << "thread id:" << threadID << std::endl;
            CORE_HARD_STOP("thread id");
        }
        return m_SignalValues[hash].Get();
    }

    void VK_Pool::ResetCommandPool()
    {
        VkCommandPoolResetFlags flags{VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT};
        vkResetCommandPool(m_Device, GetCommandPool(), flags);
    }

    void VK_Pool::ResetCommandPools(ThreadPool& threadpool)
    {
        VkCommandPoolResetFlags flags{VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT};
        for (auto& threadID : threadpool.GetThreadIDs())
        {
            uint64 hash = std::hash<std::thread::id>()(threadID);
            vkResetCommandPool(m_Device, m_CommandPools[hash], flags);
        }
    }

    void VK_Pool::ResetDescriptorPool() { GetDescriptorPool().ResetPool(); }

    void VK_Pool::ResetDescriptorPools(ThreadPool& threadpool)
    {
        for (auto& threadID : threadpool.GetThreadIDs())
        {
            uint64 hash = std::hash<std::thread::id>()(threadID);
            m_DescriptorPools[hash]->ResetPool();
        }
    }
} // namespace GfxRenderEngine
