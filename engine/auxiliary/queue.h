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

#include <queue>
#include <mutex>
#include <functional>

namespace GfxRenderEngine
{
    namespace Atomic
    {
        template <typename T> class Queue
        {
        public:
            Queue() = default;                  // default
            Queue(Queue const& other) = delete; // copy
            Queue(Queue&& other) = delete;      // move
            ~Queue() {}

            void Emplace(T&& value) // constructs element in-place at the end via move
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_Queue.emplace(std::move(value));
            }

            void EmplaceBack(T&& value) // constructs element in-place at the end via move
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_Queue.emplace(std::move(value));
            }

            void Pop() // removes the first element
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_Queue.pop();
            }

            auto& Front()
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                return m_Queue.front();
            }

            auto& Back()
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                return m_Queue.back();
            }

            auto Empty()
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                return m_Queue.empty();
            }

            auto Size()
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                return m_Queue.size();
            }

            void Clear()
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                for (; !m_Queue.empty(); m_Queue.pop())
                {
                }
            }

            void Reset() { Clear(); }

            void DoAll(std::function<void(T&&)> function)
            {
                for (; !Empty(); Pop())
                {
                    function(std::move(Front()));
                }
            }

        private:
            std::queue<T> m_Queue;
            std::mutex m_Mutex;
        };

    } // namespace Atomic
} // namespace GfxRenderEngine
