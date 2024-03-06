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

#include "gui/Common/Data/Text/wrapText.h"
#include "gui/Common/Render/drawBuffer.h"
#include "gui/Common/Data/Text/utf8.h"

namespace GfxRenderEngine
{
    bool SCREEN_WordWrapper::IsCJK(uint32_t c)
    {
        if (c < 0x1000)
        {
            return false;
        }
        bool result = (c >= 0x1100 && c <= 0x11FF);
        result = result || (c >= 0x2E80 && c <= 0x2FFF);
        result = result || (c >= 0x3040 && c <= 0x4DB5);
        result = result || (c >= 0x4E00 && c <= 0x9FBB);
        result = result || (c >= 0xAC00 && c <= 0xD7AF);
        result = result || (c >= 0xF900 && c <= 0xFAD9);
        result = result || (c >= 0x20000 && c <= 0x2A6D6);
        result = result || (c >= 0x2F800 && c <= 0x2FA1D);
        return result;
    }

    bool SCREEN_WordWrapper::IsPunctuation(uint32_t c)
    {
        switch (c)
        {
            case ',':
            case '.':
            case ':':
            case '!':
            case ')':
            case '?':
            case 0x00AD:
            case 0x3001:
            case 0x3002:
            case 0x06D4:
            case 0xFF01:
            case 0xFF09:
            case 0xFF1F:
                return true;

            default:
                return false;
        }
    }

    bool SCREEN_WordWrapper::IsSpace(uint32_t c)
    {
        switch (c)
        {
            case '\t':
            case ' ':
            case 0x2002:
            case 0x2003:
            case 0x3000:
                return true;

            default:
                return false;
        }
    }

    bool SCREEN_WordWrapper::IsShy(uint32_t c) { return c == 0x00AD; }

    std::string SCREEN_WordWrapper::Wrapped()
    {
        if (out_.empty())
        {
            Wrap();
        }
        return out_;
    }

    bool SCREEN_WordWrapper::WrapBeforeWord()
    {
        if (flags_ & FLAG_WRAP_TEXT)
        {
            if (x_ + wordWidth_ > maxW_ && !out_.empty())
            {
                if (IsShy(out_[out_.size() - 1]))
                {
                    out_[out_.size() - 1] = '-';
                }
                out_ += "\n";
                lastLineStart_ = out_.size();
                x_ = 0.0f;
                forceEarlyWrap_ = false;
                return true;
            }
        }
        if (flags_ & FLAG_ELLIPSIZE_TEXT)
        {
            if (x_ + wordWidth_ > maxW_)
            {
                if (!out_.empty() && IsSpace(out_[out_.size() - 1]))
                {
                    out_[out_.size() - 1] = '.';
                    out_ += "..";
                }
                else
                {
                    out_ += "...";
                }
                x_ = maxW_;
            }
        }
        return false;
    }

    void SCREEN_WordWrapper::AppendWord(int endIndex, bool addNewline)
    {
        int lastWordStartIndex = lastIndex_;
        if (WrapBeforeWord())
        {
            SCREEN_UTF8 utf8Word(str_, lastWordStartIndex);
            while (lastWordStartIndex < endIndex)
            {
                const uint32_t c = utf8Word.next();
                if (!IsSpace(c))
                {
                    break;
                }
                lastWordStartIndex = utf8Word.byteIndex();
            }
        }

        if (x_ < maxW_)
        {
            out_.append(str_ + lastWordStartIndex, str_ + endIndex);
        }
        else
        {
            scanForNewline_ = true;
        }
        if (addNewline && (flags_ & FLAG_WRAP_TEXT))
        {
            out_ += "\n";
            lastLineStart_ = out_.size();
            scanForNewline_ = false;
        }
        else
        {
            size_t pos = out_.substr(lastLineStart_).find_last_of("\n");
            if (pos != out_.npos)
            {
                lastLineStart_ += pos;
            }
        }
        lastIndex_ = endIndex;
    }

    void SCREEN_WordWrapper::Wrap()
    {
        out_.clear();

        size_t len = strlen(str_);

        out_.reserve(len + len / 16);

        if (MeasureWidth(str_, len) <= maxW_)
        {
            out_ = str_;
            return;
        }

        if (flags_ & FLAG_ELLIPSIZE_TEXT)
        {
            ellipsisWidth_ = MeasureWidth("...", 3);
        }

        for (SCREEN_UTF8 utf(str_); !utf.end();)
        {
            int beforeIndex = utf.byteIndex();
            uint32_t c = utf.next();
            int afterIndex = utf.byteIndex();

            if (c == '\n')
            {
                AppendWord(afterIndex, false);
                x_ = 0.0f;
                wordWidth_ = 0.0f;

                forceEarlyWrap_ = false;
                scanForNewline_ = false;
                continue;
            }

            if (scanForNewline_)
            {
                lastIndex_ = afterIndex;
                continue;
            }

            float newWordWidth = MeasureWidth(str_ + lastIndex_, afterIndex - lastIndex_);

            if (wordWidth_ > 0.0f && IsSpace(c))
            {
                AppendWord(afterIndex, false);

                x_ = MeasureWidth(out_.c_str() + lastLineStart_, out_.size() - lastLineStart_);
                wordWidth_ = 0.0f;
                continue;
            }

            if (wordWidth_ > 0.0f && newWordWidth > maxW_)
            {
                if (x_ > 0.0f && x_ + wordWidth_ > maxW_ && beforeIndex > lastIndex_)
                {
                    forceEarlyWrap_ = true;

                    wordWidth_ = 0.0f;
                    while (utf.byteIndex() > lastIndex_)
                    {
                        utf.bwd();
                    }
                    continue;
                }

                AppendWord(beforeIndex, true);
                if (lastLineStart_ != out_.size())
                {
                    x_ = MeasureWidth(out_.c_str() + lastLineStart_, out_.size() - lastLineStart_);
                }
                else
                {
                    x_ = 0.0f;
                }
                wordWidth_ = 0.0f;
                forceEarlyWrap_ = false;

                continue;
            }

            if ((flags_ & FLAG_ELLIPSIZE_TEXT) && wordWidth_ > 0.0f && x_ + newWordWidth + ellipsisWidth_ > maxW_)
            {
                if ((flags_ & FLAG_WRAP_TEXT) == 0)
                {
                    AppendWord(beforeIndex, true);
                    if (lastLineStart_ != out_.size())
                    {
                        x_ = MeasureWidth(out_.c_str() + lastLineStart_, out_.size() - lastLineStart_);
                    }
                    else
                    {
                        x_ = 0.0f;
                    }
                    wordWidth_ = 0.0f;
                    forceEarlyWrap_ = false;
                    continue;
                }
            }

            wordWidth_ = newWordWidth;

            if (wordWidth_ > 0.0f && (IsCJK(c) || IsPunctuation(c) || forceEarlyWrap_))
            {
                AppendWord(afterIndex, false);
                x_ += wordWidth_;
                wordWidth_ = 0.0f;
            }
        }

        AppendWord((int)len, false);
    }
} // namespace GfxRenderEngine
