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

#pragma once

#include <Renderer/RenderPrimitive.h>
#include <Renderer/VK/RendererVK.h>
#include <Renderer/VK/BufferVK.h>
namespace JPH
{
    /// Vulkan implementation of a render primitive
    class RenderPrimitiveVK : public RenderPrimitive
    {
    public:
        /// Constructor
        RenderPrimitiveVK(RendererVK* inRenderer) : mRenderer(inRenderer) {}
        virtual ~RenderPrimitiveVK() override { Clear(); }

        /// Vertex buffer management functions
        virtual void CreateVertexBuffer(int inNumVtx, int inVtxSize, const void* inData = nullptr) override;
        virtual void ReleaseVertexBuffer() override;
        virtual void* LockVertexBuffer() override;
        virtual void UnlockVertexBuffer() override;

        /// Index buffer management functions
        virtual void CreateIndexBuffer(int inNumIdx, const uint32* inData = nullptr) override;
        virtual void ReleaseIndexBuffer() override;
        virtual uint32* LockIndexBuffer() override;
        virtual void UnlockIndexBuffer() override;

        /// Draw the primitive
        virtual void Draw() const override;

    private:
        friend class RenderInstancesVK;

        RendererVK* mRenderer;

        BufferVK mVertexBuffer;
        bool mVertexBufferDeviceLocal = false;

        BufferVK mIndexBuffer;
        bool mIndexBufferDeviceLocal = false;
    };
} // namespace JPH