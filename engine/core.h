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

#include <memory>
#include <string>
#include <chrono>

#include "engine.h"
#include "application.h"
#include "events/event.h"
#include "settings/settings.h"
#include "coreSettings.h"
#include "auxiliary/timestep.h"
#include "platform/SDL/controller.h"
#include "platform/SDL/timer.h"
#include "platform/window.h"
#include "layer/layerStack.h"
#include "renderer/graphicsContext.h"
#include "auxiliary/threadPool.h"
#include "renderer/renderer.h"
#include "renderer/model.h"
#include "audio/audio.h"

namespace GfxRenderEngine
{
    class Engine
    {
    public:
        static constexpr bool SWITCH_OFF_COMPUTER = true;

    public:
        Engine(const std::string& configFilePath);
        ~Engine();

        bool Start();
        void WaitInitialized();
        void WaitIdle() const;
        void OnUpdate();
        void OnEvent(Event& event);
        void PostRender();
        void QueueEvent(std::unique_ptr<Event>& event);
        void ResetDescriptorPools();
        void Shutdown(bool switchOffComputer = false);
        void Quit();

        void InitSettings();
        void ApplyAppSettings();
        bool IsPaused() const { return m_Paused; }
        bool IsInitialized() const { return m_GraphicsContextInitialized; }
        bool IsRunning() const { return m_Running; }
        void RunScripts(Application* application);

        std::string& GetHomeDirectory() { return m_HomeDir; }
        std::string GetConfigFilePath() const { return m_ConfigFilePath; }
        std::chrono::time_point<std::chrono::high_resolution_clock> GetTime() const;
        double GetTimeDouble() const { return m_Window->GetTime(); }

        std::shared_ptr<Window> GetWindow() const { return m_Window; }
        void* GetBackendWindow() const { return m_Window->GetBackendWindow(); }
        float GetWindowScale() const { return m_Window->GetWindowAspectRatio(); }
        float GetWindowAspectRatio() const { return m_Window->GetWindowAspectRatio(); }
        uint GetContextWidth() const { return m_GraphicsContext->GetContextWidth(); }
        uint GetContextHeight() const { return m_GraphicsContext->GetContextHeight(); }
        float GetWindowWidth() const { return static_cast<float>(m_Window->GetWidth()); }
        float GetWindowHeight() const { return static_cast<float>(m_Window->GetHeight()); }
        float GetDesktopWidth() const { return static_cast<float>(m_Window->GetDesktopWidth()); }
        float GetDesktopHeight() const { return static_cast<float>(m_Window->GetDesktopHeight()); }

        std::shared_ptr<Model> LoadModel(const Builder&);
        std::shared_ptr<Model> LoadModel(const TerrainBuilder&);
        std::shared_ptr<Model> LoadModel(const GltfBuilder&);
        std::shared_ptr<Model> LoadModel(const Model::ModelData&);
        std::shared_ptr<Model> LoadModel(const FbxBuilder&);
        std::shared_ptr<Model> LoadModel(const UFbxBuilder&);
        bool IsFullscreen() const { return m_Window->IsFullscreen(); }

        void EnableMousePointer() { m_Window->EnableMousePointer(); }
        void DisableMousePointer() { m_Window->DisableMousePointer(); }
        void AllowCursor() { m_Window->AllowCursor(); }
        void DisallowCursor() { m_Window->DisallowCursor(); }

        void PlaySound(std::string filename) { m_Audio->PlaySound(filename); }
        void PlaySound(const char* path, int resourceID, const std::string& resourceClass)
        {
            m_Audio->PlaySound(path, resourceID, resourceClass);
        }

        Renderer* GetRenderer() const { return m_GraphicsContext->GetRenderer(); }
        bool MultiThreadingSupport() const { return m_GraphicsContext->MultiThreadingSupport(); }
        void SetAppEventCallback(EventCallbackFunction eventCallback);

        void PushLayer(Layer* layer) { m_LayerStack.PushLayer(layer); }
        void PopLayer(Layer* layer) { m_LayerStack.PopLayer(layer); }
        void PushOverlay(Layer* overlay) { m_LayerStack.PushOverlay(overlay); }
        void PopOverlay(Layer* overlay) { m_LayerStack.PopOverlay(overlay); }

        void ToggleDebugWindow(const GenericCallback& callback = nullptr) { m_GraphicsContext->ToggleDebugWindow(callback); }

        Timestep GetTimestep() const { return m_Timestep; }
        void ToggleFullscreen();

    public:
        static Engine* m_Engine;
        static SettingsManager m_SettingsManager;
        CoreSettings m_CoreSettings{&m_SettingsManager};
        ThreadPool m_PoolPrimary;
        ThreadPool m_PoolSecondary;

    private:
        static void SignalHandler(int signal);
        void AudioCallback(int eventType);

    private:
        std::string m_HomeDir;
        std::string m_ConfigFilePath;
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<GraphicsContext> m_GraphicsContext;
        std::shared_ptr<Audio> m_Audio;
        Controller m_Controller;
        Timer m_DisableMousePointerTimer;
        EventCallbackFunction m_AppEventCallback;
        LayerStack m_LayerStack;

        Timestep m_Timestep;
        Chrono::TimePoint m_TimeLastFrame;
        Chrono::TimePoint m_StartTime;

        bool m_Running, m_Paused, m_GraphicsContextInitialized;
        std::vector<std::unique_ptr<Event>> m_EventQueue;
    };
} // namespace GfxRenderEngine
