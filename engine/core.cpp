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

#include <csignal>
#include <chrono>
#include <thread>

#include "core.h"
#include "engine.h"
#include "auxiliary/file.h"
#include "auxiliary/instrumentation.h"
#include "events/applicationEvent.h"
#include "events/mouseEvent.h"
#include "events/keyEvent.h"
#include "scene/nativeScript.h"

namespace GfxRenderEngine
{
    Engine* Engine::m_Engine = nullptr;
    SettingsManager Engine::m_SettingsManager;

    Engine::Engine(const std::string& configFilePath)
        : m_ConfigFilePath(configFilePath), m_DisableMousePointerTimer(2500), m_Running(false), m_Paused(false),
          m_Timestep{0ms}, m_GraphicsContextInitialized(false)
    {
#ifdef _MSC_VER
        m_HomeDir = "";
#else
        m_HomeDir = getenv("HOME");
#endif

        if (m_HomeDir == "")
        {
            auto path = std::filesystem::current_path();
            m_HomeDir = path.string();
        }

        EngineCore::AddSlash(m_HomeDir);

        m_Engine = this;

        m_DisableMousePointerTimer.SetEventCallback(
            [](uint interval, void* parameters)
            {
                uint returnValue = 0;
                Engine::m_Engine->DisableMousePointer();
                return returnValue;
            });
    }

    Engine::~Engine() {}

    bool Engine::Start()
    {
        InitSettings();

        // signal handling
        signal(SIGINT, SignalHandler);

        // create main window
        std::string title = "Vulkan Engine v" ENGINE_VERSION;
        WindowProperties windowProperties(title);
        m_Window = Window::Create(windowProperties);
        if (!m_Window->IsOK())
        {
            LOG_CORE_CRITICAL("Could not create main window");
            return false;
        }
        m_Window->SetEventCallback([this](Event& event) { return this->OnEvent(event); });
        m_GraphicsContext = GraphicsContext::Create(m_Window.get(), m_PoolPrimary, m_PoolSecondary);

        // init audio
        m_Audio = Audio::Create();
        m_Audio->Start();
#ifdef PULSEAUDIO
        Sound::SetCallback([this](const LibPAmanager::Event& event) { AudioCallback((int)event.GetType()); });
#endif

        // init controller
        if (!m_Controller.Start())
        {
            LOG_CORE_CRITICAL("Could not create controller");
            return false;
        }
        else
        {
            m_Controller.SetEventCallback([this](Event& event) { return this->OnEvent(event); });
        }

        m_Running = true;

        return true;
    }

    void Engine::Shutdown(bool switchOffComputer)
    {
        m_Window->Shutdown();
        m_Running = false;
    }

    void Engine::Quit()
    {
        // save settings
        m_CoreSettings.m_EngineVersion = ENGINE_VERSION;
        m_CoreSettings.m_EnableFullscreen = IsFullscreen();
        m_SettingsManager.SaveToFile();
    }

    void Engine::WaitInitialized()
    {
        m_Window->OnUpdate();

        auto time = GetTime();
        m_Timestep = time - m_TimeLastFrame;
        m_TimeLastFrame = time;

        if (!m_Window->IsOK())
        {
            Shutdown();
        }

        if (!m_GraphicsContextInitialized)
        {
            m_GraphicsContextInitialized = m_GraphicsContext->Init();
        }
    }

    void Engine::OnUpdate()
    {
        m_Window->OnUpdate();

        auto time = GetTime();
        m_Timestep = time - m_TimeLastFrame;
        m_TimeLastFrame = time;

        if (!m_Window->IsOK())
        {
            Shutdown();
        }
        m_Controller.OnUpdate();

        if (m_EventQueue.size())
        {
            for (auto& event : m_EventQueue)
            {
                OnEvent(*event.get());
            }
            m_EventQueue.clear();
        }
        m_StartTime = GetTime();
    }

    void Engine::PostRender() { m_GraphicsContext->LimitFrameRate(m_StartTime); }

    void Engine::SignalHandler(int signal)
    {
        static bool sigIntReceived{false};
        if ((signal == SIGINT) && sigIntReceived)
        {
            LOG_CORE_INFO("force shudown");
            // force shudown
            exit(1);
        }
        if (signal == SIGINT)
        {
            sigIntReceived = true;
            LOG_CORE_INFO("Received signal SIGINT, exiting");
            Engine::m_Engine->Shutdown();
        }
    }

    void Engine::QueueEvent(std::unique_ptr<Event>& event) { m_EventQueue.push_back(std::move(event)); }

    void Engine::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);

        // log events
        // if (event.GetCategoryFlags() & EventCategoryApplication) LOG_CORE_INFO(event);
        // if (event.GetCategoryFlags() & EventCategoryInput)       LOG_CORE_INFO(event);
        // if (event.GetCategoryFlags() & EventCategoryMouse)       LOG_CORE_INFO(event);
        // if (event.GetCategoryFlags() & EventCategoryController)  LOG_CORE_INFO(event);
        // if (event.GetCategoryFlags() & EventCategoryJoystick)    LOG_CORE_INFO(event);

        // dispatch to Engine
        // dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent event)
        //    {
        //        Shutdown();
        //        return true;
        //    }
        //);

        dispatcher.Dispatch<WindowResizeEvent>(
            [this](WindowResizeEvent l_Event)
            {
                // RenderCommand::SetScissor(0, 0, event.GetWidth(), event.GetHeight());
                // m_WindowScale = GetWindowWidth() / GetContextWidth();
                if ((l_Event.GetWidth() == 0) || (l_Event.GetHeight() == 0))
                {
                    LOG_CORE_INFO("application paused");
                    m_Paused = true;
                }
                else
                {
                    m_Paused = false;
                }
                return false;
            });

        dispatcher.Dispatch<KeyPressedEvent>(
            [this](KeyPressedEvent l_Event)
            {
                switch (l_Event.GetKeyCode())
                {
                    case ENGINE_KEY_F:
                        LOG_CORE_INFO("toggle fullscreen at frame {0}", GetRenderer()->GetFrameCounter());
                        ToggleFullscreen();
                        break;
                }
                return false;
            });

        dispatcher.Dispatch<MouseMovedEvent>(
            [this](MouseMovedEvent l_Event)
            {
                m_Window->EnableMousePointer();
                m_DisableMousePointerTimer.Stop();
                m_DisableMousePointerTimer.Start();
                return true;
            });

        // dispatch to application layers
        if (!event.IsHandled())
        {
            for (auto layerIterator = m_LayerStack.end(); layerIterator != m_LayerStack.begin();)
            {
                layerIterator--;
                (*layerIterator)->OnEvent(event);

                if (event.IsHandled())
                    break;
            }
        }
        // dispatch to application
        if (!event.IsHandled() && m_GraphicsContextInitialized)
        {
            m_AppEventCallback(event);
        }
    }

    void Engine::ToggleFullscreen() { m_Window->ToggleFullscreen(); }

    void Engine::AudioCallback(int eventType)
    {
#ifdef PULSEAUDIO
        switch (eventType)
        {

            case LibPAmanager::Event::OUTPUT_DEVICE_CHANGED:
            {
                LOG_CORE_INFO("current audio output device: {0}", Sound::GetDefaultOutputDevice());
                break;
            }
            case LibPAmanager::Event::OUTPUT_DEVICE_LIST_CHANGED:
            {
                auto outputDeviceList = Sound::GetOutputDeviceList();
                for (auto device : outputDeviceList)
                {
                    LOG_CORE_INFO("list all audio output devices: {0}", device);
                }
                break;
            }
            case LibPAmanager::Event::OUTPUT_DEVICE_VOLUME_CHANGED:
            {
                auto volume = Sound::GetDesktopVolume();
                // user code goes here
                LOG_CORE_INFO("output volume changed to: {0}", volume);
                break;
            }
        }
#endif
    }

    void Engine::InitSettings()
    {
        m_CoreSettings.InitDefaults();
        m_CoreSettings.RegisterSettings();

        // load external configuration
        m_ConfigFilePath = GetHomeDirectory() + m_ConfigFilePath;
        std::string configFile = /*m_ConfigFilePath + */ "engine.cfg";

        m_SettingsManager.SetFilepath(configFile);
        m_SettingsManager.LoadFromFile();

        if (m_CoreSettings.m_EngineVersion != ENGINE_VERSION)
        {
            LOG_CORE_INFO("Welcome to engine version {0} (gfxRenderEngine)!", ENGINE_VERSION);
        }
        else
        {
            LOG_CORE_INFO("Starting engine (gfxRenderEngine) v" ENGINE_VERSION);
        }
    }

    void Engine::ApplyAppSettings() { m_SettingsManager.ApplySettings(); }

    void Engine::SetAppEventCallback(EventCallbackFunction eventCallback) { m_AppEventCallback = eventCallback; }

    std::chrono::time_point<std::chrono::high_resolution_clock> Engine::GetTime() const
    {
        return std::chrono::high_resolution_clock::now();
    }

    void Engine::RunScripts(Application* application)
    {
        auto currentScene = application->GetScene();
        auto& registry = currentScene->GetRegistry();

        auto view = registry.view<ScriptComponent>();
        for (auto entity : view)
        {
            auto& scriptComponent = view.get<ScriptComponent>(entity);

            if (scriptComponent.m_Script)
            {
                scriptComponent.m_Script->OnUpdate(m_Timestep);
            }
            else
            {
                static std::vector<uint> log;
                for (uint currentLog : log)
                {
                    if (currentLog == static_cast<uint>(entity))
                    {
                        return;
                    }
                }
                log.push_back(static_cast<uint>(entity));
                LOG_CORE_WARN("no script loaded for game object {0}, '{1}'", static_cast<uint>(entity),
                              currentScene->GetDictionary().GetName(entity));
            }
        }
    }

    void Engine::WaitIdle() const { m_GraphicsContext->WaitIdle(); }
    void Engine::ResetDescriptorPools()
    {
        m_GraphicsContext->ResetDescriptorPool(m_PoolPrimary);
        m_GraphicsContext->ResetDescriptorPool(m_PoolSecondary);
    }

    std::shared_ptr<Model> Engine::LoadModel(const Builder& builder) { return m_GraphicsContext->LoadModel(builder); }
    std::shared_ptr<Model> Engine::LoadModel(const TerrainBuilder& builder) { return m_GraphicsContext->LoadModel(builder); }
    std::shared_ptr<Model> Engine::LoadModel(const GltfBuilder& builder) { return m_GraphicsContext->LoadModel(builder); }
    std::shared_ptr<Model> Engine::LoadModel(const Model::ModelData& modelData)
    {
        return m_GraphicsContext->LoadModel(modelData);
    }
    std::shared_ptr<Model> Engine::LoadModel(const FbxBuilder& builder) { return m_GraphicsContext->LoadModel(builder); }
    std::shared_ptr<Model> Engine::LoadModel(const UFbxBuilder& builder) { return m_GraphicsContext->LoadModel(builder); }
} // namespace GfxRenderEngine
