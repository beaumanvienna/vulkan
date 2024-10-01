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
#include <iostream>
#include <mutex>
#include "BS_thread_pool/BS_thread_pool.hpp"

namespace GfxRenderEngine
{

    class ThreadPool
    {

    public:
        ThreadPool();
        ThreadPool(const BS::concurrency_t numThreads);

        void Wait();
        [[nodiscard]] BS::concurrency_t Size() const;

        template <typename FunctionType, typename ReturnType = std::invoke_result_t<std::decay_t<FunctionType>>>
        [[nodiscard]] std::future<ReturnType> SubmitTask(FunctionType&& task)
        {
            std::lock_guard<std::mutex> guard(m_Mutex);
            return m_Pool.submit_task(task);
        }
        [[nodiscard]] std::vector<std::thread::id> GetThreadIDs() const { return m_Pool.get_thread_ids(); }

    private:
        BS::thread_pool m_Pool;
        std::mutex m_Mutex;
    };
} // namespace GfxRenderEngine
