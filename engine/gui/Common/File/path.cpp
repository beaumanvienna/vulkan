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

#include <cstring>

#include "gui/Common/File/path.h"
#include "gui/Common/Data/Text/utf8.h"
#include "gui/Common/stringUtils.h"

namespace GfxRenderEngine
{
    Path::Path(const std::string& str)
    {
        if (str.empty())
        {
            type_ = PathType::UNDEFINED;
        }
        else if (startsWith(str, "http://") || startsWith(str, "https://"))
        {
            type_ = PathType::HTTP;
        }
        else
        {
            type_ = PathType::NATIVE;
        }

        Init(str);
    }

#ifdef _WIN32
    Path::Path(const std::wstring& str)
    {
        type_ = PathType::NATIVE;
        Init(ConvertWStringToUTF8(str));
    }
#endif

    void Path::Init(const std::string& str)
    {
        m_Path = str;

#ifdef _WIN32
        for (size_t i = 0; i < m_Path.size(); i++)
        {
            if (m_Path[i] == '\\')
            {
                m_Path[i] = '/';
            }
        }
#endif

        if (type_ == PathType::NATIVE && m_Path.size() > 1 && m_Path.back() == '/')
        {
            m_Path.pop_back();
        }
    }

    Path Path::operator/(const std::string& subdir) const
    {

        if (subdir.empty())
        {
            return Path(m_Path);
        }
        std::string fullPath = m_Path;
        if (subdir.front() != '/')
        {
            fullPath += "/";
        }
        fullPath += subdir;

        if (fullPath.back() == '/')
        {
            fullPath.pop_back();
        }
        return Path(fullPath);
    }

    void Path::operator/=(const std::string& subdir) { *this = *this / subdir; }

    Path Path::WithExtraExtension(const std::string& ext) const { return Path(m_Path + ext); }

    Path Path::WithReplacedExtension(const std::string& oldExtension, const std::string& newExtension) const
    {
        if (endsWithNoCase(m_Path, oldExtension))
        {
            std::string newPath = m_Path.substr(0, m_Path.size() - oldExtension.size());
            return Path(newPath + newExtension);
        }
        else
        {
            return Path(*this);
        }
    }

    Path Path::WithReplacedExtension(const std::string& newExtension) const
    {

        if (m_Path.empty())
        {
            return Path(*this);
        }
        std::string extension = GetFileExtension();
        std::string newPath = m_Path.substr(0, m_Path.size() - extension.size()) + newExtension;
        return Path(newPath);
    }

    std::string Path::GetFilename() const
    {

        size_t pos = m_Path.rfind('/');
        if (pos != std::string::npos)
        {
            return m_Path.substr(pos + 1);
        }
        return m_Path;
    }

    static std::string GetExtFromString(const std::string& str)
    {
        size_t pos = str.rfind(".");
        if (pos == std::string::npos)
        {
            return "";
        }
        size_t slash_pos = str.rfind("/");
        if (slash_pos != std::string::npos && slash_pos > pos)
        {
            return "";
        }
        std::string ext = str.substr(pos);
        for (size_t i = 0; i < ext.size(); i++)
        {
            ext[i] = tolower(ext[i]);
        }
        return ext;
    }

    std::string Path::GetFileExtension() const { return GetExtFromString(m_Path); }

    std::string Path::GetDirectory() const
    {
        size_t pos = m_Path.rfind('/');
        if (type_ == PathType::HTTP)
        {
            if (pos + 1 == m_Path.size())
            {
                pos = m_Path.rfind('/', pos - 1);
                if (pos != m_Path.npos && pos > 8)
                {
                    return m_Path.substr(0, pos + 1);
                }
            }
        }

        if (pos != std::string::npos)
        {
            if (pos == 0)
            {
                return "/";
            }
            return m_Path.substr(0, pos);
#ifdef _WIN32
        }
        else if (m_Path.size() == 2 && m_Path[1] == ':')
        {
            return "/";
#endif
        }
        else
        {
            size_t c_pos = m_Path.rfind(':');
            if (c_pos != std::string::npos)
            {
                return m_Path.substr(0, c_pos + 1);
            }
        }
        return m_Path;
    }

    bool Path::FilePathContains(const std::string& needle) const
    {
        std::string haystack;
        if (type_ == PathType::CONTENT_URI)
        {
        }
        else
        {
            haystack = m_Path;
        }
        return haystack.find(needle) != std::string::npos;
    }

    bool Path::StartsWith(const Path& other) const
    {
        if (type_ != other.type_)
        {
            return false;
        }
        return startsWith(m_Path, other.m_Path);
    }

    const std::string& Path::ToString() const { return m_Path; }

#ifdef _WIN32
    std::wstring Path::ToWString() const
    {
        std::wstring w = ConvertUTF8ToWString(m_Path);
        for (size_t i = 0; i < w.size(); i++)
        {
            if (w[i] == '/')
            {
                w[i] = '\\';
            }
        }
        return w;
    }
#endif

    std::string Path::ToVisualString() const { return m_Path; }

    bool Path::CanNavigateUp() const
    {
        if (m_Path == "/" || m_Path == "")
        {
            return false;
        }
        if (type_ == PathType::HTTP)
        {
            size_t rootSlash = m_Path.find_first_of('/', strlen("https://"));
            if (rootSlash == m_Path.npos || m_Path.size() < rootSlash + 1)
            {
                return false;
            }
        }
        return true;
    }

    Path Path::NavigateUp() const
    {

        std::string dir = GetDirectory();
        return Path(dir);
    }

    Path Path::GetRootVolume() const
    {
        if (!IsAbsolute())
        {

            return Path(m_Path);
        }

#ifdef _WIN32
        if (m_Path[1] == ':')
        {
            std::string path = m_Path.substr(0, 2);
            return Path(path);
        }
#endif

        return Path("/");
    }

    bool Path::IsAbsolute() const
    {
        if (type_ == PathType::CONTENT_URI)
        {
            return true;
        }

        if (m_Path.empty())
        {
            return true;
        }
        else if (m_Path.front() == '/')
        {
            return true;
        }
#ifdef _WIN32
        else if (m_Path.size() > 3 && m_Path[1] == ':')
        {
            return true;
        }
#endif
        else
        {
            return false;
        }
    }

    std::string Path::PathTo(const Path& other)
    {
        if (!other.StartsWith(*this))
        {
            return std::string();
        }

        std::string diff;

        if (m_Path == "/")
        {
            diff = other.m_Path.substr(1);
        }
        else
        {
            diff = other.m_Path.substr(m_Path.size() + 1);
        }

        return diff;
    }
} // namespace GfxRenderEngine
