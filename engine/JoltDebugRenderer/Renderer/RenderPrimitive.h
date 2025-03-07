// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
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

#include <Jolt/Core/Reference.h>
namespace JPH
{
    /// Simple wrapper around vertex and index buffers
    class RenderPrimitive : public RefTarget<RenderPrimitive>, public RefTargetVirtual
    {
    public:
        /// Destructor
        virtual ~RenderPrimitive() override = default;

        /// Erase all primitive data
        void Clear();

        /// Check if this primitive contains any data
        bool IsEmpty() const { return mNumVtx == 0 && mNumIdx == 0; }

        /// Vertex buffer management functions
        virtual void CreateVertexBuffer(int inNumVtx, int inVtxSize, const void* inData = nullptr) = 0;
        virtual void ReleaseVertexBuffer();
        virtual void* LockVertexBuffer() = 0;
        virtual void UnlockVertexBuffer() = 0;
        int GetNumVtx() const { return mNumVtx; }
        int GetNumVtxToDraw() const { return mNumVtxToDraw; }
        void SetNumVtxToDraw(int inUsed) { mNumVtxToDraw = inUsed; }

        /// Index buffer management functions
        virtual void CreateIndexBuffer(int inNumIdx, const uint32* inData = nullptr) = 0;
        virtual void ReleaseIndexBuffer();
        virtual uint32* LockIndexBuffer() = 0;
        virtual void UnlockIndexBuffer() = 0;
        int GetNumIdx() const { return mNumIdx; }
        int GetNumIdxToDraw() const { return mNumIdxToDraw; }
        void SetNumIdxToDraw(int inUsed) { mNumIdxToDraw = inUsed; }

        /// Draw the primitive
        virtual void Draw() const = 0;

        /// Implement RefTargetVirtual, so we can conveniently use this class as DebugRenderer::Batch
        virtual void AddRef() override { RefTarget<RenderPrimitive>::AddRef(); }
        virtual void Release() override { RefTarget<RenderPrimitive>::Release(); }

    protected:
        int mNumVtx = 0;
        int mNumVtxToDraw = 0;
        int mVtxSize = 0;

        int mNumIdx = 0;
        int mNumIdxToDraw = 0;
    };
} // namespace JPH