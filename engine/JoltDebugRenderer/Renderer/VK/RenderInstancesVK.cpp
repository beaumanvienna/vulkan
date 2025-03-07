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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <TestFramework.h>

#include <Renderer/VK/RenderInstancesVK.h>
#include <Renderer/VK/RenderPrimitiveVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>
namespace JPH
{
    void RenderInstancesVK::Clear() { mRenderer->FreeBuffer(mInstancesBuffer); }

    void RenderInstancesVK::CreateBuffer(int inNumInstances, int inInstanceSize)
    {
        Clear();

        mRenderer->CreateBuffer(VkDeviceSize(inNumInstances) * inInstanceSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                mInstancesBuffer);
    }

    void* RenderInstancesVK::Lock()
    {
        void* data;
        FatalErrorIfFailed(vkMapMemory(mRenderer->GetDevice(), mInstancesBuffer.mMemory, mInstancesBuffer.mOffset,
                                       mInstancesBuffer.mSize, 0, &data));
        return data;
    }

    void RenderInstancesVK::Unlock() { vkUnmapMemory(mRenderer->GetDevice(), mInstancesBuffer.mMemory); }

    void RenderInstancesVK::Draw(RenderPrimitive* inPrimitive, int inStartInstance, int inNumInstances) const
    {
        if (inNumInstances <= 0)
            return;

        VkCommandBuffer command_buffer = mRenderer->GetCommandBuffer();
        RenderPrimitiveVK* primitive = static_cast<RenderPrimitiveVK*>(inPrimitive);

        VkBuffer buffers[] = {primitive->mVertexBuffer.mBuffer, mInstancesBuffer.mBuffer};
        VkDeviceSize offsets[] = {0, 0};
        vkCmdBindVertexBuffers(command_buffer, 0, 2, buffers, offsets);

        if (primitive->mIndexBuffer.mBuffer == VK_NULL_HANDLE)
        {
            vkCmdDraw(command_buffer, primitive->mNumVtxToDraw, inNumInstances, 0, inStartInstance);
        }
        else
        {
            vkCmdBindIndexBuffer(command_buffer, primitive->mIndexBuffer.mBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(command_buffer, primitive->mNumIdxToDraw, inNumInstances, 0, 0, inStartInstance);
        }
    }
} // namespace JPH