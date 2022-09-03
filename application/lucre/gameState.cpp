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
#include "scenes/settingsScene.h"

namespace LucreApp
{

    GameState::GameState()
        : m_State{State::SPLASH}, m_InputIdle{false},
          m_UserInputEnabled{false},
          m_MainSceneLoaded{false}
    {
    }

    void GameState::Start()
    {
        Load(State::SPLASH);
        Load(State::MAIN);
        Load(State::SETTINGS);

        SetState(State::SPLASH);
    }

    void GameState::Stop()
    {
        GetScene().Stop();
    }

    Scene* GameState::OnUpdate()
    {
        switch(m_State)
        {
            case State::SPLASH:
            {
                if (GetScene().IsFinished() && m_MainSceneLoaded)
                {
                    SetState(State::MAIN);
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
            case State::SETTINGS:
            {
                if (GetScene().IsFinished())
                {
                    SetState(State::MAIN);
                }
                break;
            }
        }
        return m_Scenes[m_State].get();
    }

    void GameState::SetState(State state)
    {
        m_State = state;
        GetScene().SetRunning();
        GetScene().OnResize();
    }

    Scene& GameState::GetScene()
    {
        auto& scene_ptr = m_Scenes[m_State];
        return *scene_ptr;
    }

    void GameState::Load(GameState::State state)
    {
        switch(state)
        {
            case State::SPLASH:
            {
                m_Scenes.emplace(State::SPLASH, std::make_unique<SplashScene>("splash.scene", "application/lucre/sceneDescriptions/splash.scene"));
                m_Scenes[State::SPLASH]->Start();
                break;
            }
            case State::MAIN:
            {
                std::thread loadMainSceneThread([this]()
                {
                    m_Scenes.emplace(State::MAIN, std::make_unique<MainScene>("main.scene", "application/lucre/sceneDescriptions/main.scene"));
                    m_Scenes[State::MAIN]->Load();
                    m_Scenes[State::MAIN]->Start();
                    m_MainSceneLoaded = true;
                });
                loadMainSceneThread.detach();
                break;
            }
            case State::SETTINGS:
            {
                m_Scenes.emplace(State::SETTINGS, std::make_unique<SettingsScene>("settings.scene", "application/lucre/sceneDescriptions/settings.scene"));
                m_Scenes[State::SETTINGS]->Start();
                break;
            }
            default:
                LOG_APP_CRITICAL("GameState::Load invalid state");
                break;
        }
    }

    void GameState::EnableUserInput(bool enable)
    {
        m_UserInputEnabled = enable;
    }
}
