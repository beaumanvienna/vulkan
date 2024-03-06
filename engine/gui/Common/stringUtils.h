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

#include <iostream>
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

namespace GfxRenderEngine
{
    inline bool startsWith(const std::string& str, const std::string& what)
    {
        if (str.size() < what.size())
        {
            return false;
        }
        return str.substr(0, what.size()) == what;
    }

    // inline bool endsWith(const std::string &str, const std::string &what)
    //{
    //     if (str.size() < what.size())
    //         return false;
    //     return str.substr(str.size() - what.size()) == what;
    // }
    //
    //// Only use on strings where you're only concerned about ASCII.
    // inline bool startsWithNoCase(const std::string &str, const std::string &what)
    //{
    //     if (str.size() < what.size())
    //         return false;
    //     return strncasecmp(str.c_str(), what.c_str(), what.size()) == 0;
    // }
    //
    inline bool endsWithNoCase(const std::string& str, const std::string& what)
    {
        if (str.size() < what.size())
        {
            return false;
        }
        const size_t offset = str.size() - what.size();
        return strncasecmp(str.c_str() + offset, what.c_str(), what.size()) == 0;
    }

    // void SCREEN_DataToHexString(const uint8_t *data, size_t size, std::string *output);
    // void SCREEN_DataToHexString(const char* prefix, uint32_t startAddr, const uint8_t* data, size_t size, std::string*
    // output);
    //
    std::string SCREEN_PStringFromFormat(const char* format, ...);
    std::string SCREEN_StringFromInt(int value);
    //
    // std::string SCREEN_StripSpaces(const std::string &s);
    // std::string SCREEN_StripQuotes(const std::string &s);
    //
    void SCREEN_PSplitString(const std::string& str, const char delim, std::vector<std::string>& output);
    //
    // void SCREEN_GetQuotedStrings(const std::string& str, std::vector<std::string>& output);
    //
    std::string SCREEN_ReplaceAll(std::string input, const std::string& src, const std::string& dest);
    //
    // void SCREEN_SkipSpace(const char **ptr);
    //
    // void SCREEN_truncate_cpy(char *dest, size_t destSize, const char *src);
    // template<size_t Count>
    // inline void SCREEN_truncate_cpy(char(&out)[Count], const char *src)
    //{
    //    SCREEN_truncate_cpy(out, Count, src);
    //}
    //
    // long SCREEN_parseHexLong(std::string s);
    // long SCREEN_parseLong(std::string s);
    // std::string SCREEN_PStringFromFormat(const char* format, ...);
    //// Cheap!
    // bool SCREEN_PCharArrayFromFormatV(char* out, int outsize, const char* format, va_list args);
    //
    // template<size_t Count>
    // inline void CharArrayFromFormat(char (& out)[Count], const char* format, ...)
    //{
    //     va_list args;
    //     va_start(args, format);
    //     SCREEN_PCharArrayFromFormatV(out, Count, format, args);
    //     va_end(args);
    // }
    //
    //// "C:/Windows/winhelp.exe" to "C:/Windows/", "winhelp", ".exe"
    // bool SCREEN_PSplitPath(const std::string& full_path, std::string* _pPath, std::string* _pFilename, std::string*
    // _pExtension);
    //
    // std::string SCREEN_GetFilenameFromPath(std::string full_path);
    //
} // namespace GfxRenderEngine
