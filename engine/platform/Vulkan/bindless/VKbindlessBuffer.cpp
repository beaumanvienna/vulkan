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
#include "bindless/VKbindlessBuffer.h"

namespace GfxRenderEngine
{
    VK_BindlessBuffer::BufferDeviceAddress VK_BindlessBuffer::AddBuffer(Buffer* buffer)
    {
        if (buffer == nullptr)
        {
            return 0;
        }

        VkBufferDeviceAddressInfo addressInfo{};
        addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        addressInfo.buffer = static_cast<VK_Buffer*>(buffer)->GetBuffer();

        BufferDeviceAddress gpuAddress = vkGetBufferDeviceAddress(VK_Core::m_Device->Device(), &addressInfo);

        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_BufferAddresses[buffer->GetBufferID()] = gpuAddress;
        }

        return gpuAddress;
    }

    VK_BindlessBuffer::BufferDeviceAddress VK_BindlessBuffer::GetBufferAddress(Buffer::BufferID bufferId)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        auto it = m_BufferAddresses.find(bufferId);
        if (it != m_BufferAddresses.end())
        {
            return it->second;
        }

        return 0; // invalid
    }

    void VK_BindlessBuffer::RemoveBuffer(Buffer::BufferID bufferId)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_BufferAddresses.erase(bufferId);
    }
} // namespace GfxRenderEngine
