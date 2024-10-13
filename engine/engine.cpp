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

#include <chrono>
#include <thread>

#include "core.h"
#include "engine.h"
#include "application.h"

using Profiler = GfxRenderEngine::Instrumentation::Profiler;
// global logger for the engine and application
std::unique_ptr<GfxRenderEngine::Log> g_Logger;
// non-owning pointer to the global profiler
std::unique_ptr<Profiler> g_Profiler;

int engine(int argc, char* argv[])
{
    g_Logger = std::make_unique<GfxRenderEngine::Log>();
    g_Profiler = std::make_unique<Profiler>("RunTime", "profiling (open with chrome tracing).json");

    std::unique_ptr<GfxRenderEngine::Engine> engine;
    std::unique_ptr<GfxRenderEngine::Application> application;

    {
        PROFILE_SCOPE("engine startup");
        engine = std::make_unique<GfxRenderEngine::Engine>("./");

        if (!engine->Start())
        {
            return -1;
        }
        while (!engine->IsInitialized())
        {
            // poll window events
            // to prevent desktop message
            // "app not responding"
            // while shaders compile
            engine->WaitInitialized();
            std::this_thread::sleep_for(16ms);
        }
    }

    {
        PROFILE_SCOPE("application startup");
        application = GfxRenderEngine::Application::Create();
        if (!application->Start())
        {
            return -1;
        }
        engine->SetAppEventCallback([&](Event& event) { application->OnEvent(event); });
    }

    LOG_CORE_INFO("entering main application");
    while (engine->IsRunning())
    {
        {
            {
                PROFILE_SCOPE("engine->OnUpdate()");
                engine->OnUpdate();
            }
            if (!engine->IsPaused())
            {
                {
                    PROFILE_SCOPE("application->OnUpdate()");
                    ZoneScopedN("application->OnUpdate");
                    application->OnUpdate(engine->GetTimestep());
                    engine->RunScripts(application.get());
                }
                {
                    ZoneScopedN("engine->PostRender()");
                    engine->PostRender();
                }
            }
            else
            {
                std::this_thread::sleep_for(16ms);
            }
        }
        FrameMark;
    }

    application->Shutdown();
    engine->Shutdown();
    application.reset();
    engine->Quit();
    engine.reset();
    g_Profiler.reset();
    g_Logger.reset();

#ifdef DEBUG
    std::cout << "leaving main" << std::endl;
#endif
    return 0;
}
