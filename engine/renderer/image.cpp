/* Engine Copyright (c) 2024 Engine Development Team
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

#include "stb_image.h"

#include "renderer/image.h"

namespace GfxRenderEngine
{
    Image::Image(std::string const& filename) : m_DataBuffer{nullptr}, m_Width{0}, m_Height{0}, m_BytesPerPixel{0}
    {
        stbi_set_flip_vertically_on_load(false);
        m_DataBuffer = stbi_load(filename.c_str(), &m_Width, &m_Height, &m_BytesPerPixel, 0);
    }

    Image::~Image() { stbi_image_free(m_DataBuffer); }

    uchar* Image::Get() { return m_DataBuffer; }

    int Image::Width() const { return m_Width; }
    int Image::Height() const { return m_Height; }
    int Image::BytesPerPixel() const { return m_BytesPerPixel; }
    int Image::Size() const { return m_Width * m_Height; }
    bool Image::IsValid() const { return m_DataBuffer != nullptr; }

    uchar Image::operator[](uint index) const { return m_DataBuffer[index]; }

} // namespace GfxRenderEngine
