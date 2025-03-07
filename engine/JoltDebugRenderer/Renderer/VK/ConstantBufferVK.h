// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#pragma once

#include <Renderer/VK/BufferVK.h>
namespace JPH
{
    class RendererVK;

    /// A binary blob that can be used to pass constants to a shader
    class ConstantBufferVK
    {
    public:
        /// Constructor
        ConstantBufferVK(RendererVK* inRenderer, VkDeviceSize inBufferSize);
        ~ConstantBufferVK();

        /// Map / unmap buffer (get pointer to data). This will discard all data in the buffer.
        template <typename T> T* Map() { return reinterpret_cast<T*>(MapInternal()); }
        void Unmap();

        VkBuffer GetBuffer() const { return mBuffer.mBuffer; }

    private:
        void* MapInternal();

        RendererVK* mRenderer;
        BufferVK mBuffer;
    };
} // namespace JPH