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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <direct.h>
#else
#include <strings.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#endif

#include <cstring>
#include <string>
#include <set>
#include <algorithm>
#include <cstdio>
#include <sys/stat.h>
#include <ctype.h>

#include "engine.h"
#include "gui/Common/Data/Text/utf8.h"
#include "gui/Common/stringUtils.h"
#include "gui/Common/File/dirListing.h"
#include "gui/Common/File/path.h"

#ifdef _WIN32
#include "time.h"
#endif

namespace GfxRenderEngine
{
#ifdef _WIN32
    inline struct tm* localtime_r(const time_t* clock, struct tm* result)
    {
        if (localtime_s(result, clock) == 0)
        {
            return result;
        }
        return NULL;
    }
#endif

    // Returns true if filename exists and is a directory
    bool IsDirectory(const Path& filename)
    {
        switch (filename.Type())
        {
            case PathType::NATIVE:
                break;
            default:
                return false;
        }

#if defined(_WIN32)
        WIN32_FILE_ATTRIBUTE_DATA data{};

        if (!GetFileAttributesExW(filename.ToWString().c_str(), GetFileExInfoStandard, &data) ||
            data.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
        {
            auto err = GetLastError();

            return false;
        }
        DWORD result = data.dwFileAttributes;
        return (result & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
#else
        std::string copy = filename.ToString();
        struct stat file_info;
        int result = stat(copy.c_str(), &file_info);
        if (result < 0)
        {
            return false;
        }
        return S_ISDIR(file_info.st_mode);
#endif
    }

    namespace File
    {

        bool GetFileInfo(const Path& path, FileInfo* fileInfo)
        {
            switch (path.Type())
            {
                case PathType::NATIVE:
                    break; // OK
                default:
                    return false;
            }

            // TODO: Expand relative paths?
            fileInfo->fullName = path;

#ifdef _WIN32
            auto FiletimeToStatTime = [](FILETIME ft)
            {
                const int windowsTickResolution = 10000000;
                const int64_t secToUnixEpoch = 11644473600LL;
                int64_t ticks = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
                return (int64_t)(ticks / windowsTickResolution - secToUnixEpoch);
            };

            WIN32_FILE_ATTRIBUTE_DATA attrs;
            if (!GetFileAttributesExW(path.ToWString().c_str(), GetFileExInfoStandard, &attrs))
            {
                fileInfo->size = 0;
                fileInfo->isDirectory = false;
                fileInfo->exists = false;
                return false;
            }
            fileInfo->size = (uint64_t)attrs.nFileSizeLow | ((uint64_t)attrs.nFileSizeHigh << 32);
            fileInfo->isDirectory = (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            fileInfo->isWritable = (attrs.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0;
            fileInfo->exists = true;
            fileInfo->atime = FiletimeToStatTime(attrs.ftLastAccessTime);
            fileInfo->mtime = FiletimeToStatTime(attrs.ftLastWriteTime);
            fileInfo->ctime = FiletimeToStatTime(attrs.ftCreationTime);
            if (attrs.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            {
                fileInfo->access = 0444; // Read
            }
            else
            {
                fileInfo->access = 0666; // Read/Write
            }
            if (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                fileInfo->access |= 0111; // Execute
            }
#else

#if (defined __ANDROID__) && (__ANDROID_API__ < 21)
            struct stat file_info;
            int result = stat(path.c_str(), &file_info);
#elif (MACOSX)
            struct stat file_info;
            int result = stat(path.c_str(), &file_info);
#else
            struct stat64 file_info;
            int result = stat64(path.c_str(), &file_info);
#endif
            if (result < 0)
            {
                fileInfo->exists = false;
                return false;
            }

            fileInfo->isDirectory = S_ISDIR(file_info.st_mode);
            fileInfo->isWritable = false;
            fileInfo->size = file_info.st_size;
            fileInfo->exists = true;
            fileInfo->atime = file_info.st_atime;
            fileInfo->mtime = file_info.st_mtime;
            fileInfo->ctime = file_info.st_ctime;
            fileInfo->access = file_info.st_mode & 0x1ff;

            if (file_info.st_mode & 0200)
            {
                fileInfo->isWritable = true;
            }
#endif
            return true;
        }

        bool GetModifTime(const Path& filename, tm& return_time)
        {
            memset(&return_time, 0, sizeof(return_time));
            FileInfo info;
            if (GetFileInfo(filename, &info))
            {
                time_t t = info.mtime;
                localtime_r((time_t*)&t, &return_time);
                return true;
            }
            else
            {
                return false;
            }
        }

        bool FileInfo::operator<(const FileInfo& other) const
        {
            if (isDirectory && !other.isDirectory)
            {
                return true;
            }
            else if (!isDirectory && other.isDirectory)
            {
                return false;
            }
            if (strcasecmp(name.c_str(), other.name.c_str()) < 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        std::vector<File::FileInfo> ApplyFilter(std::vector<File::FileInfo> files, const char* filter)
        {
            std::set<std::string> filters;
            if (filter)
            {
                std::string tmp;
                while (*filter)
                {
                    if (*filter == ':')
                    {
                        filters.insert("." + tmp);
                        tmp.clear();
                    }
                    else
                    {
                        tmp.push_back(*filter);
                    }
                    filter++;
                }
                if (!tmp.empty())
                {
                    filters.insert("." + tmp);
                }
            }

            auto pred = [&](const File::FileInfo& info)
            {
                if (info.isDirectory || !filter)
                {
                    return false;
                }
                std::string ext = info.fullName.GetFileExtension();
                return filters.find(ext) == filters.end();
            };
            files.erase(std::remove_if(files.begin(), files.end(), pred), files.end());
            return files;
        }

        size_t GetFilesInDir(const Path& directory, std::vector<FileInfo>* files, const char* filter, int flags)
        {

#ifdef _WIN32
            if (directory.IsRoot())
            {
                std::vector<std::string> drives = File::GetWindowsDrives();
                for (auto drive = drives.begin(); drive != drives.end(); ++drive)
                {
                    if (*drive == "A:/" || *drive == "B:/")
                    {
                        continue;
                    }
                    File::FileInfo fake;
                    fake.fullName = Path(*drive);
                    fake.name = *drive;
                    fake.isDirectory = true;
                    fake.exists = true;
                    fake.size = 0;
                    fake.isWritable = false;
                    files->push_back(fake);
                }
                return files->size();
            }
#endif

            size_t foundEntries = 0;
            std::set<std::string> filters;
            if (filter)
            {
                std::string tmp;
                while (*filter)
                {
                    if (*filter == ':')
                    {
                        filters.insert(std::move(tmp));
                        tmp.clear();
                    }
                    else
                    {
                        tmp.push_back(*filter);
                    }
                    filter++;
                }
                if (!tmp.empty())
                {
                    filters.insert(std::move(tmp));
                }
            }
#ifdef _WIN32
            // Find the first file in the directory.
            WIN32_FIND_DATA ffd;
            HANDLE hFind = FindFirstFileExW((directory.ToWString() + L"\\*").c_str(), FindExInfoStandard, &ffd,
                                            FindExSearchNameMatch, NULL, 0);
            if (hFind == INVALID_HANDLE_VALUE)
            {
                return 0;
            }

            do
            {
#ifdef _MSC_VER
                const std::string virtualName = ConvertWStringToUTF8(ffd.cFileName);
#else
                const std::string virtualName = ffd.cFileName;
#endif
#else
            struct dirent* result = NULL;

            // std::string directoryWithSlash = directory;
            // if (directoryWithSlash.back() != '/')
            //     directoryWithSlash += "/";

            DIR* dirp = opendir(directory.c_str());
            if (!dirp)
                return 0;

            while ((result = readdir(dirp)))
            {
                const std::string virtualName(result->d_name);
#endif
                // check for "." and ".."
                if (virtualName == "." || virtualName == "..")
                {
                    continue;
                }

                if (!(flags & GETFILES_GETHIDDEN))
                {
#ifdef _WIN32
                    if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0)
                        continue;
#else
                    if (virtualName[0] == '.')
                        continue;
#endif
                }

                FileInfo info;
                info.name = virtualName;
                if (directory.IsRoot())
                {
                    std::string str = "/" + std::string(virtualName.c_str());
                    info.fullName = Path(str);
                }
                else
                {
                    info.fullName = directory / virtualName;
                }

                info.isDirectory = IsDirectory(info.fullName);
                info.exists = true;
                info.size = 0;
                info.isWritable = false; // TODO - implement some kind of check
                if (!info.isDirectory)
                {
                    std::string ext = info.fullName.GetFileExtension();
                    if (!ext.empty())
                    {
                        ext = ext.substr(1); // Remove the dot.
                        if (filter && filters.find(ext) == filters.end())
                        {
                            continue;
                        }
                    }
                }

                if (files)
                {
                    files->push_back(std::move(info));
                }
                foundEntries++;
#ifdef _WIN32
            } while (FindNextFile(hFind, &ffd) != 0);
            FindClose(hFind);
#else
            }
            closedir(dirp);
#endif
            if (files)
            {
                std::sort(files->begin(), files->end());
            }
            return foundEntries;
        }

        int64_t GetDirectoryRecursiveSize(const Path& path, const char* filter, int flags)
        {
            std::vector<FileInfo> fileInfo;
            GetFilesInDir(path, &fileInfo, filter, flags);
            int64_t sizeSum = 0;

            for (size_t i = 0; i < fileInfo.size(); i++)
            {
                FileInfo finfo;
                GetFileInfo(fileInfo[i].fullName, &finfo);
                if (!finfo.isDirectory)
                {
                    sizeSum += finfo.size;
                }
                else
                {
                    sizeSum += GetDirectoryRecursiveSize(finfo.fullName, filter, flags);
                }
            }
            return sizeSum;
        }

#ifdef _WIN32

        std::vector<std::string> GetWindowsDrives()
        {
            std::vector<std::string> drives;

            const DWORD buffsize = GetLogicalDriveStrings(0, NULL);
            std::vector<wchar_t> buff(buffsize);
            if (GetLogicalDriveStringsW(buffsize, buff.data()) == buffsize - 1)
            {
                auto drive = buff.data();
                while (*drive)
                {
                    std::string str(ConvertWStringToUTF8(drive));
                    str.pop_back();
                    str += "/";
                    drives.push_back(str);

                    while (*drive++)
                    {
                    }
                }
            }
            return drives;
        }
#endif

    } // namespace File
} // namespace GfxRenderEngine
