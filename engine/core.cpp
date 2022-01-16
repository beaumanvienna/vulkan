/* Engine Copyright (c) 2021 Engine Development Team
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

#include <csignal>

#include "core.h"
#include "engine.h"
#include "instrumentation.h"

Engine* Engine::m_Engine = nullptr;

Engine::Engine(const std::string& configFilePath) :
            m_ConfigFilePath(configFilePath),
            m_Running(false)
{
    m_Engine = this;
}

Engine::~Engine()
{
}

bool Engine::Start()
{
    // init logger
    if (!Log::Init())
    {
        std::cout << "Could not initialize logger" << std::endl;
    }

    //signal handling
    signal(SIGINT, SignalHandler);

    glfwInit();
    
    // create main window
    std::string title = "Vulkan Engine v" ENGINE_VERSION;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(800, 600, title.c_str(), nullptr, nullptr);

    uint extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    LOG_CORE_INFO("{0}  extensions supported", extensionCount);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()); //populate buffer
    for (auto& extension : extensions)
    {
        LOG_CORE_INFO("extension found: {0}", extension.extensionName);
    }

    m_Running = true;

    return true;
}

void Engine::Shutdown()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    m_Running = false;
}

void Engine::Quit()
{
}

void Engine::OnUpdate()
{
    if (glfwWindowShouldClose(m_Window))
    {
        Shutdown();
    }
    else
    {
        glfwPollEvents();
    }
}

void Engine::OnRender()
{
}

void Engine::SignalHandler(int signal)
{
    if (signal == SIGINT)
    {
        LOG_CORE_INFO("Received signal SIGINT, exiting");
        exit(0);
    }
}
