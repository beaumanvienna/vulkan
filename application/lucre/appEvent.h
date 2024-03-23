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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#pragma once

#include "events/event.h"

namespace LucreApp
{
    class AppEvent;
    enum class AppEventType
    {
        None = 0,
        SceneChanged,
        SceneFinished
    };

    enum AppEventCategory
    {
        None = 0,
        EventCategoryGameState = BIT(0)
    };

#define EVENT_CLASS_APP_CATEGORY(x) \
    int GetAppCategoryFlags() const override { return x; }
#define EVENT_CLASS_APP_TYPE(x)                                        \
    static AppEventType GetStaticAppType() { return AppEventType::x; } \
    AppEventType GetAppEventType() const override { return GetStaticAppType(); }

    class AppEvent : public Event
    {
        friend class AppEventDispatcher;
        virtual AppEventType GetAppEventType() const = 0;
        virtual int GetAppCategoryFlags() const = 0;
    };

    class AppEventDispatcher
    {
        template <typename T> using EventFn = std::function<bool(T&)>;

    public:
        AppEventDispatcher(AppEvent& event) : m_Event(event) {}

        template <typename T> bool Dispatch(EventFn<T> func)
        {
            if (m_Event.GetAppEventType() == T::GetStaticAppType())
            {
                m_Event.m_Handled |= func(*(T*)&m_Event);
                return true;
            }
            return false;
        }

    private:
        AppEvent& m_Event;
    };

    class SceneChangedEvent : public AppEvent
    {

    public:
        SceneChangedEvent(GameState::State newScene) : m_NewScene(newScene) {}

        EVENT_CLASS_CATEGORY(EventCategoryApplication);
        EVENT_CLASS_TYPE(ApplicationEvent);
        EVENT_CLASS_APP_CATEGORY(EventCategoryGameState);
        EVENT_CLASS_APP_TYPE(SceneChanged);

        GameState::State GetScene() const { return m_NewScene; }
        std::string ToString() const override
        {
            std::string strNewScene;
            switch (m_NewScene)
            {
                case GameState::State::SPLASH:
                    strNewScene = "splash";
                    break;
                case GameState::State::MAIN:
                    strNewScene = "main scene";
                    break;
                case GameState::State::SETTINGS:
                    strNewScene = "settings screen";
                    break;
                default:
                    strNewScene = "unkown scene";
            }
            std::stringstream str;
            str << "SceneChangedEvent, new scene is " + strNewScene;
            return str.str();
        }

    private:
        GameState::State m_NewScene;
    };

    class SceneFinishedEvent : public AppEvent
    {

    public:
        SceneFinishedEvent() {}

        EVENT_CLASS_CATEGORY(EventCategoryApplication);
        EVENT_CLASS_TYPE(ApplicationEvent);
        EVENT_CLASS_APP_CATEGORY(EventCategoryGameState);
        EVENT_CLASS_APP_TYPE(SceneFinished);

        std::string ToString() const override
        {
            std::stringstream str;
            str << "SceneFinishedEvent event";
            return str.str();
        }
    };
} // namespace LucreApp