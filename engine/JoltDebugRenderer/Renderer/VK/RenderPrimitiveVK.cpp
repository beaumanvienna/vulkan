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

#include <Renderer/VK/RenderPrimitiveVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>
namespace JPH
{
    void RenderPrimitiveVK::ReleaseVertexBuffer()
    {
        mRenderer->FreeBuffer(mVertexBuffer);
        mVertexBufferDeviceLocal = false;

        RenderPrimitive::ReleaseVertexBuffer();
    }

    void RenderPrimitiveVK::ReleaseIndexBuffer()
    {
        mRenderer->FreeBuffer(mIndexBuffer);
        mIndexBufferDeviceLocal = false;

        RenderPrimitive::ReleaseIndexBuffer();
    }

    void RenderPrimitiveVK::CreateVertexBuffer(int inNumVtx, int inVtxSize, const void* inData)
    {
        RenderPrimitive::CreateVertexBuffer(inNumVtx, inVtxSize, inData);

        VkDeviceSize size = VkDeviceSize(inNumVtx) * inVtxSize;
        if (inData != nullptr)
        {
            mRenderer->CreateDeviceLocalBuffer(inData, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mVertexBuffer);
            mVertexBufferDeviceLocal = true;
        }
        else
            mRenderer->CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    mVertexBuffer);
    }

    void* RenderPrimitiveVK::LockVertexBuffer()
    {
        JPH_ASSERT(!mVertexBufferDeviceLocal);

        void* data;
        FatalErrorIfFailed(vkMapMemory(mRenderer->GetDevice(), mVertexBuffer.mMemory, mVertexBuffer.mOffset,
                                       VkDeviceSize(mNumVtx) * mVtxSize, 0, &data));
        return data;
    }

    void RenderPrimitiveVK::UnlockVertexBuffer() { vkUnmapMemory(mRenderer->GetDevice(), mVertexBuffer.mMemory); }

    void RenderPrimitiveVK::CreateIndexBuffer(int inNumIdx, const uint32* inData)
    {
        RenderPrimitive::CreateIndexBuffer(inNumIdx, inData);

        VkDeviceSize size = VkDeviceSize(inNumIdx) * sizeof(uint32);
        if (inData != nullptr)
        {
            mRenderer->CreateDeviceLocalBuffer(inData, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mIndexBuffer);
            mIndexBufferDeviceLocal = true;
        }
        else
            mRenderer->CreateBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    mIndexBuffer);
    }

    uint32* RenderPrimitiveVK::LockIndexBuffer()
    {
        JPH_ASSERT(!mIndexBufferDeviceLocal);

        void* data;
        vkMapMemory(mRenderer->GetDevice(), mIndexBuffer.mMemory, mIndexBuffer.mOffset,
                    VkDeviceSize(mNumIdx) * sizeof(uint32), 0, &data);
        return reinterpret_cast<uint32*>(data);
    }

    void RenderPrimitiveVK::UnlockIndexBuffer() { vkUnmapMemory(mRenderer->GetDevice(), mIndexBuffer.mMemory); }

    void RenderPrimitiveVK::Draw() const
    {
        VkCommandBuffer command_buffer = mRenderer->GetCommandBuffer();

        VkBuffer vertex_buffers[] = {mVertexBuffer.mBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        std::cout << "RenderPrimitiveVK::Draw() vkCmdBindVertexBuffers, vertex_buffers: " << vertex_buffers
                  << ", mVertexBuffer.mSize: " << mVertexBuffer.mSize << std::endl;
        { // push camera
            auto& cam0 = mRenderer->GetCam0();
            RendererVK::PushConstants pushConstants{};
            pushConstants.m_Projection = cam0.GetProjectionMatrix();
            pushConstants.m_View = cam0.GetViewMatrix();

            vkCmdPushConstants(command_buffer, mRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, 128,
                               &pushConstants);
        }
        if (mIndexBuffer.mBuffer == VK_NULL_HANDLE)
        {
            std::cout << " RenderPrimitiveVK::Draw() vkCmdDraw, vertex count: " << mNumVtxToDraw << std::endl;
            vkCmdDraw(command_buffer, mNumVtxToDraw, 1, 0, 0);
        }
        else
        {
            std::cout << "RenderPrimitiveVK::Draw() vkCmdBindIndexBuffer, mIndexBuffer.mBuffer: " << mIndexBuffer.mBuffer
                      << ", mIndexBuffer.Size: " << mIndexBuffer.mSize << std::endl;
            vkCmdBindIndexBuffer(command_buffer, mIndexBuffer.mBuffer, 0, VK_INDEX_TYPE_UINT32);
            std::cout << "RenderPrimitiveVK::Draw() vkCmdDrawIndex, index count: " << mNumIdxToDraw << std::endl;
            vkCmdDrawIndexed(command_buffer, mNumIdxToDraw, 1, 0, 0, 0);
        }
    }
} // namespace JPH