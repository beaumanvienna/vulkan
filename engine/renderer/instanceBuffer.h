/* Engine Copyright (c) 2023 Engine Development Team
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

#include <memory>

#include "engine.h"
#include "buffer.h"

namespace GfxRenderEngine
{

    class InstanceBuffer
    {

    public:
        virtual ~InstanceBuffer() = default;

        virtual void SetInstanceData(uint index, glm::mat4 const& mat4Global, glm::mat4 const& normalMatrix) = 0;
        virtual const glm::mat4& GetModelMatrix(uint index) = 0;
        virtual const glm::mat4& GetNormalMatrix(uint index) = 0;
        virtual std::shared_ptr<Buffer> GetBuffer() = 0;

        static std::shared_ptr<InstanceBuffer> Create(uint numInstances);
    };
} // namespace GfxRenderEngine
