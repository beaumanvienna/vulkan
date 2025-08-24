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
#include "engine.h"
namespace GfxRenderEngine
{

    // HiResImage: class to load an EXR and HDR from disk, to provide the data buffer, and free it
    class HiResImage
    {
    public:
        enum ImageType
        {
            HDR = 1,
            EXR,
            UNDEFINED
        };

    public:
        HiResImage() = delete;
        HiResImage(std::string const& filename);
        ~HiResImage();

        float* GetBuffer() const { return m_Buffer; }
        bool IsInitialized() const { return m_Initialized; };
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        ImageType GetImageType() const { return m_ImageType; }
        std::string const& GetFilename() const { return m_Filename; }

    private:
        std::string m_Filename;
        int m_Width;
        int m_Height;
        float* m_Buffer; // will hold RGBA float data
        ImageType m_ImageType;
        bool m_Initialized;
    };
} // namespace GfxRenderEngine
