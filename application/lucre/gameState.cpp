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
#include "scenes/beachScene.h"
#include "scenes/cutScene.h"
#include "scenes/dessertScene.h"
#include "scenes/mainScene.h"
#include "scenes/nightScene.h"
#include "scenes/settingsScene.h"
#include "scenes/splashScene.h"
#include "scenes/terrainScene.h"
#include "scenes/island2Scene.h"
#include "scenes/volcanoScene.h"
#include "scenes/reserved0Scene.h"

namespace LucreApp
{

    GameState::GameState()
        : m_State{State::SPLASH}, m_NextState{State::SPLASH}, m_LastState{State::SPLASH}, m_UserInputEnabled{false},
          m_DeleteScene{State::NULL_STATE}, m_LoadingState{State::NULL_STATE}
    {
        memset(m_StateLoaded, false, static_cast<int>(State::MAX_STATES) * sizeof(bool));
    }

    void GameState::Start()
    {
        // the splash, cutscene, and settings scene are loaded upfront
        Load(State::SPLASH);
        Load(State::CUTSCENE);
        Load(State::SETTINGS);

        SetState(State::SPLASH);
        SetNextState(State::VOLCANO);
    }

    void GameState::Stop()
    {
        GetScene()->Stop();
        DestroyScene(m_State);
    }

    std::string GameState::StateToString(State state) const
    {
        std::string str;
        switch (state)
        {
            case State::NULL_STATE:
            {
                str = "State::NULL_STATE";
                break;
            }
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
            case State::NIGHT:
            {
                str = "State::NIGHT";
                break;
            }
            case State::DESSERT:
            {
                str = "State::DESSERT";
                break;
            }
            case State::TERRAIN:
            {
                str = "State::TERRAIN";
                break;
            }
            case State::ISLAND_2:
            {
                str = "State::ISLAND_2";
                break;
            }
            case State::VOLCANO:
            {
                str = "State::VOLCANO";
                break;
            }
            case State::RESERVED0:
            {
                str = "State::RESERVED0";
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
        switch (m_State)
        {
            case State::SPLASH:
            {
                if (GetScene()->IsFinished() && IsLoaded(GetNextState()))
                {
                    SetState(GetNextState());
                }
                break;
            }
            case State::CUTSCENE:
            {
                if (GetScene()->IsFinished())
                {
                    if (IsLoaded(GetNextState()))
                    {
                        SetState(GetNextState());
                    }
                }
                DeleteScene();
                LoadNextState();
                break;
            }
            case State::SETTINGS:
            {
                if (GetScene()->IsFinished())
                {
                    SetState(m_LastState);
                }
                break;
            }
            case State::MAIN:
            case State::BEACH:
            case State::NIGHT:
            case State::DESSERT:
            case State::TERRAIN:
            case State::ISLAND_2:
            case State::VOLCANO:
            case State::RESERVED0:
            case State::NULL_STATE:
            case State::MAX_STATES:
            {
                break;
            }
        }
        return GetScene(m_State);
    }

    void GameState::SetState(State state)
    {
        m_LastState = m_State;
        m_State = state;
        GetScene()->SetRunning();
        GetScene()->OnResize();
        PrepareDeleteScene();
    }

    void GameState::SetNextState(State state)
    {
        m_NextState = state;
        if (!IsLoaded(state) && m_DeleteScene == State::NULL_STATE)
        {
            Load(state);
        }
    }

    void GameState::LoadNextState()
    {
        if (m_LoadingState != State::NULL_STATE)
        {
            return;
        }
        if (!IsLoaded(m_NextState))
        {
            if (m_DeleteScene == State::NULL_STATE)
            {
                Load(m_NextState);
            }
        }
    }

    void GameState::PrepareDeleteScene()
    {
        // last scene must be a game level
        if (static_cast<int>(m_LastState) > static_cast<int>(State::CUTSCENE))
        {
            // current scene must be cut scene
            if (static_cast<int>(m_State) == static_cast<int>(State::CUTSCENE))
            {
                LOG_APP_INFO("deleting scene {0}", StateToString(m_LastState));
                m_DeleteScene = m_LastState;
                m_DeleteSceneCounter = 5;
            }
        }
    }

    void GameState::DeleteScene()
    {
        if (IsLoaded(m_DeleteScene))
        {
            if (m_DeleteSceneCounter > 0)
            {
                // a delay to wait a few frames
                // so the GPU is using it no longer
                m_DeleteSceneCounter--;
            }
            else
            {
                DestroyScene(m_DeleteScene);
            }
        }
    }

    void GameState::Load(GameState::State state)
    {
        CORE_ASSERT(!IsLoaded(state), "!IsLoaded(state)");
        if (m_LoadingState != State::NULL_STATE)
        {
            return;
        }
        m_LoadingState = state;
        switch (state)
        {
            case State::SPLASH:
            {
                auto scenePtr =
                    std::make_shared<SplashScene>("splash.scene", "application/lucre/sceneDescriptions/splash.scene");
                SetupScene(state, scenePtr);
                GetScene(state)->Start();
                SetLoaded(state);
                break;
            }
            case State::MAIN:
            {
                auto lambda = [this, state]()
                {
                    auto scenePtr =
                        std::make_shared<MainScene>("main.json", "application/lucre/sceneDescriptions/main.json");
                    SetupScene(state, scenePtr);
                    GetScene(state)->Load();
                    GetScene(state)->Start();
                    SetLoaded(state);
                };
                ThreadPool& threadPool = Engine::m_Engine->m_PoolPrimary;
                std::future<void> future = threadPool.SubmitTask(lambda);
                break;
            }
            case State::BEACH:
            {
                auto lambda = [this, state]()
                {
                    auto scenePtr =
                        std::make_shared<BeachScene>("beach.json", "application/lucre/sceneDescriptions/beach.json");
                    SetupScene(state, scenePtr);
                    GetScene(state)->Load();
                    GetScene(state)->Start();
                    SetLoaded(state);
                };
                ThreadPool& threadPool = Engine::m_Engine->m_PoolPrimary;
                std::future<void> future = threadPool.SubmitTask(lambda);
                break;
            }
            case State::NIGHT:
            {
                auto lambda = [this, state]()
                {
                    auto scenePtr =
                        std::make_shared<NightScene>("night.json", "application/lucre/sceneDescriptions/night.json");
                    SetupScene(state, scenePtr);
                    GetScene(state)->Load();
                    GetScene(state)->Start();
                    SetLoaded(state);
                };
                ThreadPool& threadPool = Engine::m_Engine->m_PoolPrimary;
                std::future<void> future = threadPool.SubmitTask(lambda);
                break;
            }
            case State::DESSERT:
            {
                auto lambda = [this, state]()
                {
                    auto scenePtr =
                        std::make_shared<DessertScene>("dessert.json", "application/lucre/sceneDescriptions/dessert.json");
                    SetupScene(state, scenePtr);
                    GetScene(state)->Load();
                    GetScene(state)->Start();
                    SetLoaded(state);
                };
                ThreadPool& threadPool = Engine::m_Engine->m_PoolPrimary;
                std::future<void> future = threadPool.SubmitTask(lambda);
                break;
            }
            case State::TERRAIN:
            {
                auto lambda = [this, state]()
                {
                    ZoneScopedN("loadTerrainScene");
                    auto scenePtr =
                        std::make_shared<TerrainScene>("terrain.json", "application/lucre/sceneDescriptions/terrain.json");
                    SetupScene(state, scenePtr);
                    GetScene(state)->Load();
                    GetScene(state)->Start();
                    SetLoaded(state);
                };
                ThreadPool& threadPool = Engine::m_Engine->m_PoolPrimary;
                std::future<void> future = threadPool.SubmitTask(lambda);
                break;
            }
            case State::ISLAND_2:
            {
                auto lambda = [this, state]()
                {
                    ZoneScopedN("loadIsland_2");
                    auto scenePtr =
                        std::make_shared<Island2Scene>("island2.json", "application/lucre/sceneDescriptions/island2.json");
                    SetupScene(state, scenePtr);
                    GetScene(state)->Load();
                    GetScene(state)->Start();
                    SetLoaded(state);
                };
                ThreadPool& threadPool = Engine::m_Engine->m_PoolPrimary;
                std::future<void> future = threadPool.SubmitTask(lambda);
                break;
            }
            case State::VOLCANO:
            {
                auto lambda = [this, state]()
                {
                    ZoneScopedN("loadVolcano");
                    auto scenePtr =
                        std::make_shared<VolcanoScene>("volcano.json", "application/lucre/sceneDescriptions/volcano.json");
                    SetupScene(state, scenePtr);
                    GetScene(state)->Load();
                    GetScene(state)->Start();
                    SetLoaded(state);
                };
                ThreadPool& threadPool = Engine::m_Engine->m_PoolPrimary;
                std::future<void> future = threadPool.SubmitTask(lambda);
                break;
            }
            case State::RESERVED0:
            {
                auto lambda = [this, state]()
                {
                    ZoneScopedN("loadResreved0");
                    auto scenePtr = std::make_shared<Reserved0Scene>("reserved0.json",
                                                                     "application/lucre/sceneDescriptions/reserved0.json");
                    SetupScene(state, scenePtr);
                    GetScene(state)->Load();
                    GetScene(state)->Start();
                    SetLoaded(state);
                };
                ThreadPool& threadPool = Engine::m_Engine->m_PoolPrimary;
                std::future<void> future = threadPool.SubmitTask(lambda);
                break;
            }
            case State::CUTSCENE:
            {
                auto scenePtr =
                    std::make_shared<CutScene>("cutScene.scene", "application/lucre/sceneDescriptions/cutScene.scene");
                SetupScene(state, scenePtr);
                GetScene(state)->Start();
                SetLoaded(state);
                break;
            }
            case State::SETTINGS:
            {
                auto scenePtr =
                    std::make_shared<SettingsScene>("settings.scene", "application/lucre/sceneDescriptions/settings.scene");
                SetupScene(state, scenePtr);
                GetScene(state)->Start();
                SetLoaded(state);
                break;
            }
            default:
                LOG_APP_CRITICAL("GameState::Load invalid state");
                break;
        }
    }

    void GameState::EnableUserInput(bool enable) { m_UserInputEnabled = enable; }

    Scene* GameState::GetScene() { return GetScene(m_State); }

    Scene* GameState::GetNextScene() { return GetScene(m_NextState); }

    Scene* GameState::GetScene(State state)
    {
        std::lock_guard lock(m_Mutex);
        auto scene_ptr = m_Scenes[static_cast<int>(state)].get();
        return scene_ptr;
    }

    bool GameState::IsLoaded(State state)
    {
        std::lock_guard lock(m_Mutex);
        return m_StateLoaded[static_cast<int>(state)];
    }

    void GameState::SetLoaded(State state, bool isLoaded)
    {
        std::lock_guard lock(m_Mutex);
        m_StateLoaded[static_cast<int>(state)] = isLoaded;
        m_LoadingState = State::NULL_STATE;
    }

    void GameState::SetupScene(const State state, const std::shared_ptr<Scene>& scene)
    {
        std::lock_guard lock(m_Mutex);
        m_Scenes[static_cast<int>(state)] = scene;
    }

    void GameState::DestroyScene(const State state)
    {
        std::lock_guard lock(m_Mutex);
        Engine::m_Engine->WaitIdle();
        m_StateLoaded[static_cast<int>(state)] = false;
        m_Scenes[static_cast<int>(state)] = nullptr;
        m_DeleteScene = State::NULL_STATE;
        Engine::m_Engine->ResetDescriptorPools();
    }
} // namespace LucreApp
