/* Copyright (c) 2013-2020 PPSSPP project
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

#include <string>
#include <cstdint>


namespace GfxRenderEngine
{
    class SCREEN_WordWrapper
    {

    public:
        SCREEN_WordWrapper(const char* str, float maxW, int flags) : str_(str), maxW_(maxW), flags_(flags) {}

        std::string Wrapped();

    protected:
        virtual float MeasureWidth(const char* str, size_t bytes) = 0;
        void Wrap();
        bool WrapBeforeWord();
        void AppendWord(int endIndex, bool addNewline);

        static bool IsCJK(uint32_t c);
        static bool IsPunctuation(uint32_t c);
        static bool IsSpace(uint32_t c);
        static bool IsShy(uint32_t c);

        const char* const str_;
        const float maxW_;
        const int flags_;
        std::string out_;

        // Index of last output / start of current word.
        int lastIndex_ = 0;
        // Index of last line start.
        size_t lastLineStart_ = 0;
        // Position the current word starts at.
        float x_ = 0.0f;
        // Most recent width of word since last index.
        float wordWidth_ = 0.0f;
        // Width of "..." when flag is set, zero otherwise.
        float ellipsisWidth_ = 0.0f;
        // Force the next word to cut partially and wrap.
        bool forceEarlyWrap_ = false;
        // Skip all characters until the next newline.
        bool scanForNewline_ = false;
    };
} // namespace GfxRenderEngine
