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

#include "engine.h"

namespace GfxRenderEngine
{

    class Texture
    {

    public:
        static constexpr bool USE_SRGB = true;
        static constexpr bool USE_UNORM = false;

    public:
        virtual ~Texture() = default;

        virtual bool Init(const uint width, const uint height, bool sRGB, const void* data, int minFilter,
                          int magFilter) = 0;
        virtual bool Init(const uint width, const uint height, float* data, const uint numberOfChannels,
                          bool linearFilter = true) = 0;
        virtual bool Init(const std::string& fileName, bool sRGB, bool flip = true) = 0;
        virtual bool Init(const unsigned char* data, int length, bool sRGB) = 0;
        virtual int GetWidth() const = 0;
        virtual int GetHeight() const = 0;
        virtual void Resize(uint width, uint height) = 0;
        virtual void Blit(uint x, uint y, uint width, uint height, uint bpp, const void* data) = 0;
        virtual void Blit(uint x, uint y, uint width, uint height, int dataFormat, int type, const void* data) = 0;
        virtual void SetFilename(const std::string& filename) = 0;

        static std::shared_ptr<Texture> Create();
    };
} // namespace GfxRenderEngine
