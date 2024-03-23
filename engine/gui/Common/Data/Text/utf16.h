/*
   Copyright (c) 2013-2020 PPSSPP project
   https://github.com/hrydgard/ppsspp/blob/master/LICENSE.TXT

   Engine Copyright (c) 2021-2022 Engine Development Team
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

#include "gui/bitSet.h"

namespace GfxRenderEngine
{

#define UTF16_IS_LITTLE_ENDIAN (*(const uint16_t*)"\0\xff" >= 0x100)

    template <bool is_little> uint16_t UTF16_Swap(uint16_t u)
    {
        if (is_little)
        {
            return UTF16_IS_LITTLE_ENDIAN ? u : swap16(u);
        }
        else
        {
            return UTF16_IS_LITTLE_ENDIAN ? swap16(u) : u;
        }
    }

    template <bool is_little> struct UTF16_Type
    {
    public:
        static const char32_t INVALID = (char32_t)-1;

        UTF16_Type(const char16_t* c) : c_(c), index_(0) {}

        char32_t next()
        {
            const char32_t u = UTF16_Swap<is_little>(c_[index_++]);

            if ((u & 0xF800) == 0xD800)
            {
                return 0x10000 + (((u & 0x3FF) << 10) | (UTF16_Swap<is_little>(c_[index_++]) & 0x3FF));
            }
            return u;
        }

        bool end() const { return c_[index_] == 0; }

        int length() const
        {
            int len = 0;
            for (UTF16_Type<is_little> dec(c_); !dec.end(); dec.next())
            {
                ++len;
            }

            return len;
        }

        int shortIndex() const { return index_; }

        static int encode(char16_t* dest, char32_t u)
        {
            if (u >= 0x10000)
            {
                u -= 0x10000;
                *dest++ = UTF16_Swap<is_little>(0xD800 + ((u >> 10) & 0x3FF));
                *dest = UTF16_Swap<is_little>(0xDC00 + ((u >> 0) & 0x3FF));
                return 2;
            }
            else
            {
                *dest = UTF16_Swap<is_little>((char16_t)u);
                return 1;
            }
        }

        //    static int encodeUCS2(char16_t *dest, char32_t u)
        //    {
        //        if (u >= 0x10000 || (u >= 0xD800 && u <= 0xDFFF))
        //        {
        //            return 0;
        //        }
        //        else
        //        {
        //            *dest = UTF16_Swap<is_little>((char16_t)u);
        //            return 1;
        //        }
        //    }

        static int encodeUnits(char32_t u)
        {
            if (u >= 0x10000)
            {
                return 2;
            }
            else
            {
                return 1;
            }
        }
        //
        //    static int encodeUnitsUCS2(char32_t u)
        //    {
        //        if (u >= 0x10000 || (u >= 0xD800 && u <= 0xDFFF))
        //        {
        //            return 0;
        //        }
        //        else
        //        {
        //            return 1;
        //        }
        //    }
    private:
        const char16_t* c_;
        int index_;
    };

    typedef UTF16_Type<true> UTF16LE;
    typedef UTF16_Type<false> UTF16BE;
} // namespace GfxRenderEngine
