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

#include "VKcore.h"
#include "VKstorageBuffer.h"

namespace GfxRenderEngine
{
    VK_StorageBuffer::StorageBufferID VK_StorageBuffer::m_GlobalStorageBufferIDCounter = 0;
    std::mutex VK_StorageBuffer::m_Mutex;

    VK_StorageBuffer::VK_StorageBuffer()
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_StorageBufferID = m_GlobalStorageBufferIDCounter;
            ++m_GlobalStorageBufferIDCounter;
        }
    }

    VK_StorageBuffer::~VK_StorageBuffer()
    {
        auto device = VK_Core::m_Device->Device();

        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
    }
} // namespace GfxRenderEngine
