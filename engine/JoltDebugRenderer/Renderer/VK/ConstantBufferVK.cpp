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

#include <TestFramework.h>

#include <Renderer/VK/ConstantBufferVK.h>
#include <Renderer/VK/RendererVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>
namespace JPH
{
    ConstantBufferVK::ConstantBufferVK(RendererVK* inRenderer, VkDeviceSize inBufferSize) : mRenderer(inRenderer)
    {
        mRenderer->CreateBuffer(inBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mBuffer);
    }

    ConstantBufferVK::~ConstantBufferVK() { mRenderer->FreeBuffer(mBuffer); }

    void* ConstantBufferVK::MapInternal()
    {
        void* data = nullptr;
        FatalErrorIfFailed(vkMapMemory(mRenderer->GetDevice(), mBuffer.mMemory, mBuffer.mOffset, mBuffer.mSize, 0, &data));
        return data;
    }

    void ConstantBufferVK::Unmap() { vkUnmapMemory(mRenderer->GetDevice(), mBuffer.mMemory); }
} // namespace JPH