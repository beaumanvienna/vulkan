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

#pragma once

#include <unordered_map>

#include "engine.h"
#include "events/event.h"
#include "scene/scene.h"

namespace LucreApp
{

    class GameState
    {

    public:

        enum class State
        {
            SPLASH,
            MAIN,
            BEACH,
            CUTSCENE,
            SETTINGS
        };

    public:

        GameState();

        void Start();
        void Stop();
        Scene* OnUpdate();
        void OnEvent(Event& event);

        void EnableUserInput(bool enable);

        Scene& GetScene();
        void SetState(State state);
        State GetState() const { return m_State; }
        bool UserInputIsInabled() const { return m_UserInputEnabled;}

    private:

        void Load(State state);

    private:

        State m_State;
        std::unordered_map<State, std::unique_ptr<Scene>> m_Scenes;
        bool m_UserInputEnabled;
        bool m_InputIdle;
        bool m_MainSceneLoaded;

    };
}
