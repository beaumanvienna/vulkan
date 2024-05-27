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

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace GfxRenderEngine
{
    class SCREEN_I18NRepo;
    class SCREEN_IniFile;
    class SCREEN_Section;

    struct I18NEntry
    {
        I18NEntry(const std::string& t) : text(t), readFlag(false) {}
        I18NEntry() : readFlag(false) {}
        std::string text;
        bool readFlag;
    };

    struct I18NCandidate
    {
        I18NCandidate() : key(0), defVal(0) {}
        I18NCandidate(const char* k, const char* d) : key(k), defVal(d) {}
        const char* key;
        const char* defVal;
    };

    class SCREEN_I18NCategory
    {
    public:
        SCREEN_I18NCategory(const char* name) : name_(name) {}
        const char* T(const char* key, const char* def = 0);
        const char* T(const std::string& key) { return T(key.c_str(), nullptr); }

        const std::map<std::string, std::string>& Missed() const
        {
            std::lock_guard<std::mutex> guard(missedKeyLock_);
            return missedKeyLog_;
        }

        const std::map<std::string, I18NEntry>& GetMap() { return map_; }
        void ClearMissed() { missedKeyLog_.clear(); }
        const char* GetName() const { return name_.c_str(); }

    private:
        SCREEN_I18NCategory(SCREEN_I18NRepo* repo, const char* name) : name_(name) {}
        void SetMap(const std::map<std::string, std::string>& m);

        std::string name_;

        std::map<std::string, I18NEntry> map_;
        mutable std::mutex missedKeyLock_;
        std::map<std::string, std::string> missedKeyLog_;

        friend class SCREEN_I18NRepo;
    };

    class SCREEN_I18NRepo
    {
    public:
        SCREEN_I18NRepo() {}
        ~SCREEN_I18NRepo();

        bool IniExists(const std::string& languageID) const;
        bool LoadIni(const std::string& languageID, const std::string& overridePath = "");
        void SaveIni(const std::string& languageID);

        std::string LanguageID();

        std::shared_ptr<SCREEN_I18NCategory> GetCategory(const char* categoryName);
        bool HasCategory(const char* categoryName) const
        {
            std::lock_guard<std::mutex> guard(catsLock_);
            return cats_.find(categoryName) != cats_.end();
        }
        const char* T(const char* category, const char* key, const char* def = 0);

    private:
        std::string GetIniPath(const std::string& languageID) const;
        void Clear();
        SCREEN_I18NCategory* LoadSection(const SCREEN_Section* section, const char* name);
        void SaveSection(SCREEN_IniFile& ini, SCREEN_Section* section, std::shared_ptr<SCREEN_I18NCategory> cat);

        mutable std::mutex catsLock_;
        std::map<std::string, std::shared_ptr<SCREEN_I18NCategory>> cats_;
        std::string languageID_;
    };

    extern SCREEN_I18NRepo SCREEN_i18nrepo;

    inline std::shared_ptr<SCREEN_I18NCategory> GetI18NCategory(const char* categoryName)
    {
        if (!categoryName)
            return nullptr;
        return SCREEN_i18nrepo.GetCategory(categoryName);
    }

    inline bool I18NCategoryLoaded(const char* categoryName) { return SCREEN_i18nrepo.HasCategory(categoryName); }

    inline const char* T(const char* category, const char* key, const char* def = 0)
    {
        return SCREEN_i18nrepo.T(category, key, def);
    }
} // namespace GfxRenderEngine
