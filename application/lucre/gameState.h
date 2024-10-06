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

#include <mutex>

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
            NULL_STATE = 0,
            SPLASH,
            SETTINGS,
            CUTSCENE, // do not change order
            // insert game levels here
            MAIN,
            BEACH,
            NIGHT,
            DESSERT,
            TERRAIN,
            ISLAND_2,
            VOLCANO,
            RESERVED0,
            MAX_STATES
        };

    public:
        GameState();
        ~GameState() {}

        void Start();
        void Stop();
        Scene* OnUpdate();

        void EnableUserInput(bool enable);

        void DeleteScene();
        void PrepareDeleteScene();
        void SetState(State state);
        bool IsLoaded(State state);
        void SetLoaded(State state, bool isLoaded = true);
        std::string StateToString(State state) const;
        void SetNextState(State state);
        void LoadNextState();
        State GetState() const { return m_State; }
        State GetNextState() const { return m_NextState; }
        bool UserInputIsInabled() const { return m_UserInputEnabled; }

        Scene* GetScene();
        Scene* GetScene(State state);
        Scene* GetNextScene();
        void DestroyScene(const State state);
        void SetupScene(const State state, const std::shared_ptr<Scene>& scene);

    private:
        void Load(State state);

    private:
        std::mutex m_Mutex;
        State m_State, m_NextState, m_LastState, m_DeleteScene, m_LoadingState;
        std::shared_ptr<Scene> m_Scenes[static_cast<int>(State::MAX_STATES)];
        bool m_UserInputEnabled;
        bool m_StateLoaded[static_cast<int>(State::MAX_STATES)];
        int m_DeleteSceneCounter;
    };
} // namespace LucreApp
