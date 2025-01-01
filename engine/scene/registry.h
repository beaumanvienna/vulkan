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

#pragma once

#include "entt.hpp"

#include "engine.h"

namespace GfxRenderEngine
{

    class Registry
    {

    public:
        Registry();
        ~Registry();
        entt::registry& Get() { return m_Registry; }

        [[nodiscard]] entt::entity Create();

        template <typename Component, typename... Args> decltype(auto) emplace(const entt::entity entity, Args&&... args)
        {
            std::lock_guard<std::mutex> guard(m_Mutex);
            return m_Registry.emplace<Component>(entity, std::forward<Args>(args)...);
        }

        template <typename Component> decltype(auto) remove(const entt::entity entity)
        {
            std::lock_guard<std::mutex> guard(m_Mutex);
            return m_Registry.remove<Component>(entity);
        }

        template <typename Component> [[nodiscard]] decltype(auto) get([[maybe_unused]] const entt::entity entity)
        {
            std::lock_guard<std::mutex> guard(m_Mutex);
            return m_Registry.get<Component>(entity);
        }

        template <typename Component, typename... Other, typename... Exclude>
        [[nodiscard]] auto view(entt::exclude_t<Exclude...> = {})
        {
            std::lock_guard<std::mutex> guard(m_Mutex);
            return m_Registry.view<Component, Other...>();
        }

        template <typename... Component> [[nodiscard]] bool all_of(const entt::entity entity)
        {
            std::lock_guard<std::mutex> guard(m_Mutex);
            return m_Registry.all_of<Component...>(entity);
        }

    private:
        std::mutex m_Mutex;
        entt::registry m_Registry;
    };
} // namespace GfxRenderEngine
