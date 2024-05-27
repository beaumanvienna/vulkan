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

#include <iostream>
#include <streambuf>

#include "resources/resources.h"

namespace GfxRenderEngine
{

    class memoryBuffer : public std::basic_streambuf<char>
    {

    public:
        memoryBuffer(const uint8_t* dataPointer, size_t length)
        {
            setg((char*)dataPointer, (char*)dataPointer, (char*)dataPointer + length);
        }

        memoryBuffer(const char* path /* GNU */, int resourceID /* MSVC */, const std::string& resourceClass /* MSVC */)
        {
            size_t length;
            char* dataPointer = (char*)ResourceSystem::GetDataPointer(length, path, resourceID, resourceClass);
            setg((char*)dataPointer, (char*)dataPointer, (char*)dataPointer + length);
        }
    };

    class memoryStream : public std::istream
    {

    public:
        memoryStream(const uint8_t* dataPointer, size_t length) : std::istream(&m_Buffer), m_Buffer(dataPointer, length)
        {
            rdbuf(&m_Buffer);
        }

        memoryStream(const char* path /* GNU */, int resourceID /* MSVC */, const std::string& resourceClass /* MSVC */)
            : std::istream(&m_Buffer), m_Buffer(path, resourceID, resourceClass)
        {
            rdbuf(&m_Buffer);
        }

    private:
        memoryBuffer m_Buffer;
    };
} // namespace GfxRenderEngine
