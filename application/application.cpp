/* Engine Copyright (c) 2022 Engine Development Team
   https://github.com/beaumanvienna/gfxRenderEngine

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

#include "core.h"
#include "engine.h"
#include "application.h"

std::shared_ptr<Application> Application::m_Instance;

Application::Application()
{
}

bool Application::Start()
{
    std::thread consoleInputHandler(ConsoleInputHandler);
    consoleInputHandler.detach();

    auto window = Engine::m_Engine->GetWindow();
    window->SetWindowAspectRatio();
    window->DisallowCursor();

    return true;
}

void Application::Shutdown()
{
}

void Application::OnUpdate()
{
}

std::shared_ptr<Application> Application::Create()
{
    if (!m_Instance)
    {
        m_Instance = std::make_shared<Application>();
    }
    return m_Instance;
}

void Application::ConsoleInputHandler()
{
    while (true)
    {
        LOG_APP_INFO("press enter to exit");
        getchar(); // block until enter is pressed
        Engine::m_Engine->Shutdown();
        break;
    }

}
