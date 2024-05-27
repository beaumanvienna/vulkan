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

#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <cstring>
#include <cstdlib>

#include "gui/Common/File/dirListing.h"

namespace GfxRenderEngine
{
    // Abstraction above path that lets you navigate easily.
    // "/" is a special path that means the root of the file system. On Windows,
    // listing this will yield drives.
    class SCREEN_PathBrowser
    {
    public:
        SCREEN_PathBrowser() {}
        SCREEN_PathBrowser(std::string path) { SetPath(path); }
        ~SCREEN_PathBrowser();

        void SetPath(const std::string& path);
        bool IsListingReady();
        bool GetListing(std::vector<File::FileInfo>& fileInfo, const char* filter = nullptr, bool* cancel = nullptr);
        void Navigate(const std::string& path);

        std::string GetPath() const { return m_Path; }
        std::string GetFriendlyPath() const
        {
            std::string str = GetPath();
#ifndef _MSC_VER
            char* home = getenv("HOME");
            if (home != nullptr && !strncmp(str.c_str(), home, strlen(home)))
            {
                str = str.substr(strlen(home));
                str.insert(0, 1, '~');
            }
#endif
            return str;
        }

    private:
        void HandlePath();

        std::string m_Path;
        std::string pendingPath_;
        std::vector<File::FileInfo> pendingFiles_;
        std::condition_variable pendingCond_;
        std::mutex pendingLock_;
        std::thread pendingThread_;
        bool pendingCancel_ = false;
        bool pendingStop_ = false;
        bool ready_ = false;
    };
} // namespace GfxRenderEngine
