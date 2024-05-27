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
#include <vector>
#include <stdio.h>
#include <inttypes.h>
#include "gui/Common/File/path.h"

namespace GfxRenderEngine
{
    namespace File
    {

        struct FileInfo
        {
            std::string name;
            Path fullName;
            bool exists = false;
            bool isDirectory = false;
            bool isWritable = false;
            uint64_t size = 0;

            uint64_t atime;
            uint64_t mtime;
            uint64_t ctime;
            uint32_t access; // st_mode & 0x1ff

            // Currently only supported for Android storage files.
            // Other places use different methods to get this.
            uint64_t lastModified = 0;

            bool operator<(const FileInfo& other) const;
        };

        bool GetFileInfo(const Path& path, FileInfo* fileInfo);

        enum
        {
            GETFILES_GETHIDDEN = 1,
        };

        size_t GetFilesInDir(const Path& directory, std::vector<FileInfo>* files, const char* filter = nullptr,
                             int flags = 0);
        int64_t GetDirectoryRecursiveSize(const Path& path, const char* filter = nullptr, int flags = 0);
        std::vector<File::FileInfo> ApplyFilter(std::vector<File::FileInfo> files, const char* filter);

#ifdef _WIN32
        std::vector<std::string> GetWindowsDrives();
#endif

    } // namespace File
} // namespace GfxRenderEngine
