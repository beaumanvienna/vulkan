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
#include <fstream>
#include <filesystem>

namespace GfxRenderEngine
{
    namespace EngineCore
    {
        bool FileExists(const char* filename);
        bool FileExists(const std::string& filename);
        bool FileExists(const std::filesystem::directory_entry& filename);

        bool IsDirectory(const char* filename);
        bool IsDirectory(const std::string& filename);

        std::string GetFilenameWithoutPath(const std::filesystem::path& path);
        std::string GetPathWithoutFilename(const std::filesystem::path& path);
        std::string GetFilenameWithoutExtension(const std::filesystem::path& path);
        std::string GetFilenameWithoutPathAndExtension(const std::filesystem::path& path);
        std::string GetFileExtension(const std::filesystem::path& path);
        std::string GetCurrentWorkingDirectory();
        void SetCurrentWorkingDirectory(const std::filesystem::path& path);

        bool CreateDirectory(const std::string& filename);
        bool CopyFile(const std::string& src, const std::string& dest);
        std::ifstream::pos_type FileSize(const std::string& filename);
        std::string& AddSlash(std::string& filename);
    } // namespace EngineCore
} // namespace GfxRenderEngine
