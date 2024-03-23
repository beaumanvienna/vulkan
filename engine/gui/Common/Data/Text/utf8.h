/*
   Basic UTF-8 manipulation routines
   by Jeff Bezanson
   placed in the public domain fall 2005

   This code is designed to provide the utilities you need to manipulate
   UTF-8 as an internal string encoding. These functions do not perform the
   error checking normally needed when handling UTF-8 data, so if you happen
   to be from the Unicode Consortium you will want to flay me alive.
   I do this because error checking can be performed at the boundaries (I/O),
   with these routines reserved for higher performance on data known to be
   valid.

   Further modified by hrydgard@gmail.com.
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

#include <cstdint>
#include <string>

namespace GfxRenderEngine
{
    uint32_t u8_nextchar(const char* s, int* i);
    int u8_wc_toutf8(char* dest, uint32_t ch);
    int u8_strlen(const char* s);
    void u8_inc(const char* s, int* i);
    void u8_dec(const char* s, int* i);

    class SCREEN_UTF8
    {
    public:
        static const uint32_t INVALID = (uint32_t)-1;
        SCREEN_UTF8(const char* c) : c_(c), index_(0) {}
        SCREEN_UTF8(const char* c, int index) : c_(c), index_(index) {}
        bool end() const { return c_[index_] == 0; }
        uint32_t next() { return u8_nextchar(c_, &index_); }
        uint32_t peek()
        {
            int tempIndex = index_;
            return u8_nextchar(c_, &tempIndex);
        }
        void fwd() { u8_inc(c_, &index_); }
        void bwd() { u8_dec(c_, &index_); }
        int length() const { return u8_strlen(c_); }
        int byteIndex() const { return index_; }
        static int encode(char* dest, uint32_t ch) { return u8_wc_toutf8(dest, ch); }
        static int encodeUnits(uint32_t ch)
        {
            if (ch < 0x80)
            {
                return 1;
            }
            else if (ch < 0x800)
            {
                return 2;
            }
            else if (ch < 0x10000)
            {
                return 3;
            }
            else if (ch < 0x110000)
            {
                return 4;
            }
            return 0;
        }

    private:
        const char* c_;
        int index_;
    };

    int UTF8StringNonASCIICount(const char* utf8string);
    bool UTF8StringHasNonASCII(const char* utf8string);

    // Used by SymbolMap/assembler
    std::wstring ConvertUTF8ToWString(const std::string& source);
    std::string ConvertWStringToUTF8(const std::wstring& wstr);

    std::string ConvertUCS2ToUTF8(const std::u16string& wstr);

    // Dest size in units, not bytes.
    void ConvertUTF8ToUCS2(char16_t* dest, size_t destSize, const std::string& source);
    std::u16string ConvertUTF8ToUCS2(const std::string& source);
} // namespace GfxRenderEngine
