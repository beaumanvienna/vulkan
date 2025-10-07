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

#include <TestFramework.h>

#include <JoltDebugRenderer/Renderer/RenderPrimitive.h>
namespace JPH
{
    void RenderPrimitive::ReleaseVertexBuffer()
    {
        mNumVtx = 0;
        mNumVtxToDraw = 0;
        mVtxSize = 0;
    }

    void RenderPrimitive::ReleaseIndexBuffer()
    {
        mNumIdx = 0;
        mNumIdxToDraw = 0;
    }

    void RenderPrimitive::Clear()
    {
        ReleaseVertexBuffer();
        ReleaseIndexBuffer();
    }

    void RenderPrimitive::CreateVertexBuffer(int inNumVtx, int inVtxSize, const void* inData)
    {
        ReleaseVertexBuffer();

        mNumVtx = inNumVtx;
        mNumVtxToDraw = inNumVtx;
        mVtxSize = inVtxSize;
    }

    void RenderPrimitive::CreateIndexBuffer(int inNumIdx, const uint32* inData)
    {
        ReleaseIndexBuffer();

        mNumIdx = inNumIdx;
        mNumIdxToDraw = inNumIdx;
    }
} // namespace JPH