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

#include "scene/dictionary.h"

namespace GfxRenderEngine
{
    void Dictionary::Insert(const std::string& key, entt::entity value)
    {
        m_DictStr2GameObject[key] = value;
    }

    void Dictionary::InsertLong(const std::string& key, entt::entity value)
    {
        m_GameObject2LongStr[value] = key;
        Insert(key, value);
    }

    void Dictionary::InsertShort(const std::string& key, entt::entity value)
    {
        m_GameObject2ShortStr[value] = key;
        Insert(key, value);
    }

    entt::entity Dictionary::Retrieve(const std::string& key)
    {
        if (m_DictStr2GameObject.find(key) != m_DictStr2GameObject.end())
        {
            return m_DictStr2GameObject[key];
        }
        else
        {
            LOG_CORE_CRITICAL("Dictionary::Retrieve, game object '{0}' not found", key);
            return entt::null;
        }
    }

    void Dictionary::List() const
    {
        LOG_CORE_WARN("listing dictionary:");
        for (auto& it: m_DictStr2GameObject)
        {
            LOG_CORE_INFO("key: `{0}`, value: `{1}`", it.first, it.second);
        }
    }

    const std::string& Dictionary::GetShortName(entt::entity gameObject)
    {
        ASSERT(m_GameObject2ShortStr.find(gameObject) != m_GameObject2ShortStr.end());
        return m_GameObject2ShortStr[gameObject];
    }

    const std::string& Dictionary::GetLongName(entt::entity gameObject)
    {
        ASSERT(m_GameObject2LongStr.find(gameObject) != m_GameObject2LongStr.end());
        return m_GameObject2LongStr[gameObject];
    }
}
