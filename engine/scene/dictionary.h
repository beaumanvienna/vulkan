/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

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
#include <unordered_map>

#include "engine.h"
#include "entt.hpp"

namespace GfxRenderEngine
{

    class Dictionary
    {

    public:

        void InsertShort(const std::string& key, entt::entity value);
        void InsertLong(const std::string& key, entt::entity value);
        entt::entity Retrieve(const std::string& key);
        size_t Size() const { return m_DictStr2GameObject.size(); }
        void List() const;

        const std::string& GetShortName(entt::entity gameObject);
        const std::string& GetLongName(entt::entity gameObject);

    private:

        void Insert(const std::string& key, entt::entity value);

    private:

        std::unordered_map<std::string, entt::entity> m_DictStr2GameObject;
        std::unordered_map<entt::entity, std::string> m_GameObject2ShortStr;
        std::unordered_map<entt::entity, std::string> m_GameObject2LongStr;

    };

}
