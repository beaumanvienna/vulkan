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

#include <JoltDebugRenderer/Renderer/VK/RendererVK.h>
#include <JoltDebugRenderer/Renderer/RenderInstances.h>
namespace JPH
{
    class RenderPrimitive;

    /// Vulkan implementation of a render instances object
    class RenderInstancesVK : public RenderInstances
    {
    public:
        /// Constructor
        RenderInstancesVK(RendererVK* inRenderer) : mRenderer(inRenderer) {}
        virtual ~RenderInstancesVK() override { Clear(); }

        /// Erase all instance data
        virtual void Clear() override;

        /// Instance buffer management functions
        virtual void CreateBuffer(int inNumInstances, int inInstanceSize) override;
        virtual void* Lock() override;
        virtual void Unlock() override;

        /// Draw the instances when context has been set by Renderer::BindShader
        virtual void Draw(RenderPrimitive* inPrimitive, int inStartInstance, int inNumInstances) const override;

    private:
        RendererVK* mRenderer;

        BufferVK mInstancesBuffer;
    };
} // namespace JPH