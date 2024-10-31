/* Engine Copyright (c) 2024 Engine Development Team
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
    void Dictionary::Insert(std::string const& key, entt::entity value)
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        m_DictStr2GameObject[key] = value;
        m_GameObject2Str[value] = key;
    }

    entt::entity Dictionary::Retrieve(const std::string& key)
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        if (m_DictStr2GameObject.find(key) != m_DictStr2GameObject.end())
        {
            return m_DictStr2GameObject[key];
        }
        else
        {
            LOG_CORE_WARN("Dictionary::Retrieve, game object '{0}' not found", key);
            return entt::null;
        }
    }

    void Dictionary::List()
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        LOG_CORE_INFO("listing dictionary:");
        for (auto& it : m_DictStr2GameObject)
        {
            LOG_CORE_INFO("key: `{0}`, value: `{1}`", it.first, it.second);
        }
    }

    const std::string& Dictionary::GetName(entt::entity gameObject)
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        CORE_ASSERT((m_GameObject2Str.find(gameObject) != m_GameObject2Str.end()), "Dictionary::GetName no entry found");
        return m_GameObject2Str[gameObject];
    }

    size_t Dictionary::Size()
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        return m_DictStr2GameObject.size();
    }
} // namespace GfxRenderEngine
