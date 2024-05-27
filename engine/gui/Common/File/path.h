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

namespace GfxRenderEngine
{
    enum class PathType
    {
        UNDEFINED = 0,
        NATIVE = 1,
        CONTENT_URI = 2,
        HTTP = 3,
    };

    class Path
    {
    private:
        void Init(const std::string& str);

    public:
        Path() : type_(PathType::UNDEFINED) {}
        explicit Path(const std::string& str);

#ifdef _WIN32
        explicit Path(const std::wstring& str);
#endif

        PathType Type() const { return type_; }

        bool Valid() const { return !m_Path.empty(); }
        bool IsRoot() const { return m_Path == "/"; }

        bool empty() const { return !Valid(); }
        void clear()
        {
            type_ = PathType::UNDEFINED;
            m_Path.clear();
        }
        size_t size() const { return m_Path.size(); }

        const char* c_str() const { return m_Path.c_str(); }

        bool IsAbsolute() const;

        // returns a path extended with a subdirectory
        Path operator/(const std::string& subdir) const;

        // navigates down into a subdir
        void operator/=(const std::string& subdir);

        // file extension manipulation
        Path WithExtraExtension(const std::string& ext) const;
        Path WithReplacedExtension(const std::string& oldExtension, const std::string& newExtension) const;
        Path WithReplacedExtension(const std::string& newExtension) const;

        // removes the last component
        std::string GetFilename() const;
        std::string GetFileExtension() const;
        std::string GetDirectory() const;

        const std::string& ToString() const;

#ifdef _WIN32
        std::wstring ToWString() const;
#endif

        std::string ToVisualString() const;

        bool CanNavigateUp() const;
        Path NavigateUp() const;

        // navigates as far up as possible from this path
        Path GetRootVolume() const;

        std::string PathTo(const Path& child);

        bool operator==(const Path& other) const { return m_Path == other.m_Path && type_ == other.type_; }
        bool operator!=(const Path& other) const { return m_Path != other.m_Path || type_ != other.type_; }

        bool FilePathContains(const std::string& needle) const;

        bool StartsWith(const Path& other) const;

        bool operator<(const Path& other) const { return m_Path < other.m_Path; }

    private:
        std::string m_Path;

        PathType type_;
    };
} // namespace GfxRenderEngine
