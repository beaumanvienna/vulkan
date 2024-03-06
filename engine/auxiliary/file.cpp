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

#include "auxiliary/file.h"

namespace GfxRenderEngine
{
    namespace EngineCore
    {
        bool FileExists(const char* filename)
        {
            std::ifstream infile(filename);
            return infile.good();
        }

        bool FileExists(const std::string& filename)
        {
            std::ifstream infile(filename.c_str());
            return infile.good();
        }

        bool FileExists(const std::filesystem::directory_entry& filename) { return filename.exists(); }

        bool IsDirectory(const char* filename)
        {
            std::filesystem::path path(filename);
            return is_directory(path);
        }

        bool IsDirectory(const std::string& filename)
        {
            bool isDirectory = false;
            std::filesystem::path path(filename);

            try
            {
                isDirectory = is_directory(path);
            }
            catch (...)
            {
                isDirectory = false;
            }
            return isDirectory;
        }

        std::string GetFilenameWithoutPath(const std::filesystem::path& path)
        {
#ifndef _WIN32
            std::string filenameWithoutPath = path.filename();
#else
            std::filesystem::path withoutPath{std::filesystem::path(path.filename())};
            std::string filenameWithoutPath = withoutPath.string();
#endif
            return filenameWithoutPath;
        }

        std::string GetPathWithoutFilename(const std::filesystem::path& path)
        {
#ifndef _WIN32
            std::string pathWithoutFilename = path.parent_path();
#else
            std::filesystem::path withoutFilename{std::filesystem::path(path.parent_path())};
            std::string pathWithoutFilename = withoutFilename.string();
#endif
            if (!pathWithoutFilename.empty())
            {
                if (pathWithoutFilename.back() != '/')
                {
                    pathWithoutFilename += '/';
                }
            }
            return pathWithoutFilename;
        }

        std::string GetFilenameWithoutExtension(const std::filesystem::path& path)
        {
#ifndef _WIN32
            std::string filenameWithoutExtension = path.stem();
#else
            std::filesystem::path withoutExtension{std::filesystem::path(path.stem())};
            std::string filenameWithoutExtension = withoutExtension.string();
#endif

            return filenameWithoutExtension;
        }

        std::string GetFilenameWithoutPathAndExtension(const std::filesystem::path& path)
        {
            return GetFilenameWithoutPath(GetFilenameWithoutExtension(path));
        }

        std::string GetFileExtension(const std::filesystem::path& path)
        {
#ifndef _WIN32
            std::string ext = path.extension();
#else
            std::filesystem::path extension{std::filesystem::path(path.extension())};
            std::string ext = extension.string();
#endif
            return ext;
        }

        std::string GetCurrentWorkingDirectory()
        {
#ifdef _MSC_VER
            return std::filesystem::current_path().string();
#else
            return std::filesystem::current_path();
#endif
        }

        void SetCurrentWorkingDirectory(const std::filesystem::path& path) { std::filesystem::current_path(path); }

        bool CreateDirectory(const std::string& filename)
        {
#ifdef _MSC_VER
            std::filesystem::create_directories(filename);
            return IsDirectory(filename);
#else
            return std::filesystem::create_directories(filename);
#endif
        }

        bool CopyFile(const std::string& src, const std::string& dest)
        {
            std::ifstream source(src.c_str(), std::ios::binary);
            std::ofstream destination(dest.c_str(), std::ios::binary);
            destination << source.rdbuf();
            return source && destination;
        }

        std::ifstream::pos_type FileSize(const std::string& filename)
        {
            std::ifstream in(filename.c_str(), std::ifstream::ate | std::ifstream::binary);
            return in.tellg();
        }

        std::string& AddSlash(std::string& filename)
        {
#ifdef _MSC_VER
            const char* slash = "\\";
#else
            const char* slash = "/";
#endif

            if (filename.substr(filename.size() - 1) != slash)
            {
                filename += slash;
            }

            return filename;
        }
    } // namespace EngineCore
} // namespace GfxRenderEngine
