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

#include <thread>

#include "gameState.h"

#include "core.h"
#include "scenes/splashScene.h"
#include "scenes/mainScene.h"
#include "scenes/cutScene.h"
#include "scenes/beachScene.h"
#include "scenes/settingsScene.h"

namespace LucreApp
{

    GameState::GameState()
        : m_State{State::SPLASH}, m_NextState{State::SPLASH}, m_LastState{State::SPLASH},
          m_InputIdle{false}, m_UserInputEnabled{false}
    {
        memset(m_StateLoaded, 0, static_cast<int>(State::MAX_STATES) * sizeof(bool));
    }

    void GameState::Start()
    {
        // the splash and cutscene are loaded upfront
        Load(State::SPLASH);
        Load(State::CUTSCENE);
        Load(State::SETTINGS);

        SetState(State::SPLASH);
        SetNextState(State::MAIN);
    }

    void GameState::Stop()
    {
        GetScene().Stop();
    }

    std::string GameState::StateToString(State state) const
    {
        std::string str;
        switch(state)
        {
            case State::SPLASH:
            {
                str = "State::SPLASH";
                break;
            }
            case State::CUTSCENE:
            {
                str = "State::CUTSCENE";
                break;
            }
            case State::MAIN:
            {
                str = "State::MAIN";
                break;
            }
            case State::BEACH:
            {
                str = "State::BEACH";
                break;
            }
            case State::SETTINGS:
            {
                str = "State::SETTINGS";
                break;
            }
            default:
                str = "state not found";
                break;
        }
        return str;
    }

    Scene* GameState::OnUpdate()
    {
        switch(m_State)
        {
            case State::SPLASH:
            {
                if (GetScene().IsFinished() && IsLoaded(GetNextState()))
                {
                    SetState(GetNextState());
                }
                break;
            }
            case State::CUTSCENE:
            {
                if (GetScene().IsFinished() && IsLoaded(GetNextState()))
                {
                    SetState(GetNextState());
                }
                break;
            }
            case State::MAIN:
            {
                if (GetScene().IsFinished())
                {
                    // game over
                    Engine::m_Engine->Shutdown();
                }
                break;
            }
            case State::BEACH:
            {
                if (GetScene().IsFinished())
                {
                    // game over
                    Engine::m_Engine->Shutdown();
                }
                break;
            }
            case State::SETTINGS:
            {
                if (GetScene().IsFinished())
                {
                    SetState(m_LastState);
                }
                break;
            }
        }
        return m_Scenes[m_State].get();
    }

    void GameState::SetState(State state)
    {
        m_LastState = m_State;
        m_State = state;
        GetScene().SetRunning();
        GetScene().OnResize();
    }

    void GameState::SetNextState(State state)
    {
        m_NextState = state;
        if (!IsLoaded(state)) Load(state);
    }

    Scene& GameState::GetScene()
    {
        auto& scene_ptr = m_Scenes[m_State];
        return *scene_ptr;
    }

    Scene& GameState::GetScene(State state)
    {
        auto& scene_ptr = m_Scenes[state];
        return *scene_ptr;
    }

    Scene& GameState::GetNextScene()
    {
        auto& scene_ptr = m_Scenes[m_NextState];
        return *scene_ptr;
    }

    void GameState::Load(GameState::State state)
    {
        if (m_Scenes[state]) return;
        switch(state)
        {
            case State::SPLASH:
            {
                m_Scenes[state] = std::make_unique<SplashScene>("splash.scene", "application/lucre/sceneDescriptions/splash.scene");
                m_Scenes[state]->Start();
                SetLoaded(state);
                break;
            }
            case State::MAIN:
            {
                std::thread loadMainSceneThread([=]()
                {
                    m_Scenes[state] = std::make_unique<MainScene>("main.scene", "application/lucre/sceneDescriptions/main.scene");
                    m_Scenes[state]->Load();
                    m_Scenes[state]->Start();
                    SetLoaded(state);
                });
                loadMainSceneThread.detach();
                break;
            }
            case State::BEACH:
            {
                std::thread loadBeachSceneThread([=]()
                {
                    m_Scenes[state] = std::make_unique<BeachScene>("beach.scene", "application/lucre/sceneDescriptions/beach.scene");
                    m_Scenes[state]->Load();
                    m_Scenes[state]->Start();
                    SetLoaded(state);
                });
                loadBeachSceneThread.detach();
                break;
            }
            case State::CUTSCENE:
            {
                m_Scenes[state] = std::make_unique<CutScene>("cutScene.scene", "application/lucre/sceneDescriptions/cutScene.scene");
                m_Scenes[state]->Start();
                SetLoaded(state);
                break;
            }
            case State::SETTINGS:
            {
                m_Scenes[state] = std::make_unique<SettingsScene>("settings.scene", "application/lucre/sceneDescriptions/settings.scene");
                m_Scenes[state]->Start();
                SetLoaded(state);
                break;
            }
            default:
                LOG_APP_CRITICAL("GameState::Load invalid state");
                break;
        }
    }

    bool GameState::IsLoaded(State state)
    {
        return m_StateLoaded[static_cast<int>(state)];
    }

    void GameState::SetLoaded(State state, bool isLoaded)
    {
        m_StateLoaded[static_cast<int>(state)] = isLoaded;
    }

    void GameState::EnableUserInput(bool enable)
    {
        m_UserInputEnabled = enable;
    }
}
