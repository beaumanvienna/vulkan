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

#include "engine.h"
#include "renderer/cursor.h"

#include "VKwindow.h"

namespace GfxRenderEngine
{
    class VK_Cursor : public Cursor
    {
    public:
        VK_Cursor();
        virtual ~VK_Cursor();

        virtual bool SetCursor(const unsigned char* data, int length, uint xHot, uint yHot) override;
        virtual bool SetCursor(const std::string& fileName, uint xHot, uint yHot) override;
        virtual void DisallowCursor() override;
        virtual void RestoreCursor() override;
        virtual void AllowCursor() override;

    private:
        bool SetCursor();

    private:
        int m_Width, m_Height, m_BitsPerPixel;
        uint m_HotX, m_HotY;
        uchar* m_Pixels;
        GLFWcursor* m_Cursor;
        GLFWwindow* m_Window;
    };
} // namespace GfxRenderEngine
