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

#include <algorithm>
#include <cstring>
#include <chrono>
#include <set>

#include "core.h"
#include "engine.h"
#include "auxiliary/file.h"
#include "gui/Common/File/pathBrowser.h"
#include "gui/Common/stringUtils.h"
#include "gui/Common/Thread/threadUtil.h"
#include "gui/Common/File/dirListing.h"

namespace GfxRenderEngine
{
    SCREEN_PathBrowser::~SCREEN_PathBrowser()
    {
        std::unique_lock<std::mutex> guard(pendingLock_);
        pendingCancel_ = true;
        pendingStop_ = true;
        pendingCond_.notify_all();
        guard.unlock();

        if (pendingThread_.joinable())
        {
            pendingThread_.join();
        }
    }

    // Normalize slashes.
    void SCREEN_PathBrowser::SetPath(const std::string& path)
    {
        if (!EngineCore::IsDirectory(path))
        {
            LOG_APP_ERROR("SCREEN_PathBrowser::SetPath: invalid path '{0}', falling back to home directory", path);
            m_Path = Engine::m_Engine->GetHomeDirectory();
            HandlePath();
            return;
        }
        if (path[0] == '!')
        {
            m_Path = path;
            HandlePath();
            return;
        }
        m_Path = path;
        for (size_t i = 0; i < m_Path.size(); i++)
        {
            if (m_Path[i] == '\\')
                m_Path[i] = '/';
        }
        if (!m_Path.size() || (m_Path[m_Path.size() - 1] != '/'))
        {
            m_Path += "/";
        }
        HandlePath();
    }

    void SCREEN_PathBrowser::HandlePath()
    {
        std::lock_guard<std::mutex> guard(pendingLock_);

        if (!m_Path.empty() && m_Path[0] == '!')
        {
            ready_ = true;
            pendingCancel_ = true;
            pendingPath_.clear();
            return;
        }
        if (!startsWith(m_Path, "http://") && !startsWith(m_Path, "https://"))
        {
            ready_ = true;
            pendingCancel_ = true;
            pendingPath_.clear();
            return;
        }

        ready_ = false;
        pendingCancel_ = false;
        pendingFiles_.clear();
        pendingPath_ = m_Path;
        pendingCond_.notify_all();

        if (pendingThread_.joinable())
            return;

        pendingThread_ = std::thread(
            [&]
            {
                setCurrentThreadName("PathBrowser");

                std::unique_lock<std::mutex> guard2(pendingLock_);
                std::vector<File::FileInfo> results;
                std::string lastPath;
                while (!pendingStop_)
                {
                    while (lastPath == pendingPath_ && !pendingCancel_)
                    {
                        pendingCond_.wait(guard2);
                    }
                    lastPath = pendingPath_;
                    bool success = false;
                    if (!lastPath.empty())
                    {
                        guard2.unlock();
                        results.clear();
                        success = false;
                        guard2.lock();
                    }

                    if (pendingPath_ == lastPath)
                    {
                        if (success && !pendingCancel_)
                        {
                            pendingFiles_ = results;
                        }
                        pendingPath_.clear();
                        lastPath.clear();
                        ready_ = true;
                    }
                }
            });
    }

    bool SCREEN_PathBrowser::IsListingReady() { return ready_; }

    bool SCREEN_PathBrowser::GetListing(std::vector<File::FileInfo>& fileInfo, const char* filter, bool* cancel)
    {
        std::unique_lock<std::mutex> guard(pendingLock_);
        while (!IsListingReady() && (!cancel || !*cancel))
        {
            guard.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            guard.lock();
        }
        Path path(m_Path);
        File::GetFilesInDir(path, &fileInfo, filter);

        return true;
    }

    void SCREEN_PathBrowser::Navigate(const std::string& path)
    {
        if (path == ".")
        {
            return;
        }
        if (path == "..")
        {
            // Upwards.
            // Check for windows drives.
            if (m_Path.size() == 3 && m_Path[1] == ':')
            {
                m_Path = "/";
            }
            else
            {
                size_t slash = m_Path.rfind('/', m_Path.size() - 2);
                if (slash != std::string::npos)
                {
                    m_Path = m_Path.substr(0, slash + 1);
                }
            }
        }
        else
        {
            if (path.size() > 2 && path[1] == ':' && m_Path == "/")
            {
                m_Path = path;
            }
            else
            {
                m_Path = m_Path + path;
            }
            if (m_Path[m_Path.size() - 1] != '/')
            {
                m_Path += "/";
            }
        }
        HandlePath();
    }
} // namespace GfxRenderEngine
