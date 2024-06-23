/* Engine Copyright (c) 2022 Engine Development Team
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
#include "renderer/texture.h"
#include "renderer/buffer.h"
#include "engine/platform/Vulkan/resource.h"
#include "engine.h"

namespace GfxRenderEngine
{
    class ResourceDescriptor;
    class Resources
    {
    public:
        enum BufferIndices
        {
            INSTANCE_BUFFER_INDEX = 0,
            SKELETAL_ANIMATION_BUFFER_INDEX,
            HEIGHTMAP,
            MULTI_PURPOSE_BUFFER,
            NUM_BUFFERS
        };

        // fixed-size array for resource buffers
        typedef std::array<std::shared_ptr<Buffer>, Resources::NUM_BUFFERS> ResourceBuffers;

        enum ResourceFeatures // bitset
        {
            HAS_INSTANCING = GLSL_HAS_INSTANCING,
            HAS_SKELETAL_ANIMATION = GLSL_HAS_SKELETAL_ANIMATION,
            HAS_HEIGHTMAP = GLSL_HAS_HEIGHTMAP
        };

    public:
        std::shared_ptr<ResourceDescriptor> m_ResourceDescriptor;
        ResourceBuffers m_ResourceBuffers;
    };

} // namespace GfxRenderEngine
