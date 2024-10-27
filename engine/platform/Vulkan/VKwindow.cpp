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

#include <memory>

#include "coreSettings.h"
#include "events/applicationEvent.h"
#include "events/mouseEvent.h"
#include "events/keyEvent.h"
#include "resources/resources.h"
#include "stb_image.h"

#include "VKwindow.h"

namespace GfxRenderEngine
{

    bool VK_Window::m_GLFWIsInitialized = false;

    VK_Window::VK_Window(const WindowProperties& props) : m_OK(false), m_IsFullscreen(false), m_AllowCursor(false)
    {
        m_WindowProperties.m_Title = props.m_Title;
        m_WindowProperties.m_Width = props.m_Width;
        m_WindowProperties.m_Height = props.m_Height;
        m_WindowProperties.m_AspectRatio = static_cast<float>(props.m_Width) / static_cast<float>(props.m_Height);

        m_WindowProperties.m_MousePosX = 0.0f;
        m_WindowProperties.m_MousePosY = 0.0f;
        m_WindowProperties.m_FramebufferResized = false;
        m_WindowProperties.m_ToggleCmd = false;
        m_WindowProperties.m_Window = this;

        if (!m_GLFWIsInitialized)
        {
            // init glfw
            m_GLFWIsInitialized = InitGLFW();
        }
        if (m_GLFWIsInitialized)
        {

            CreateWindow();

            uint extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

            LOG_CORE_INFO("{0}  extensions supported", extensionCount);

            if (m_Window)
            {
                m_OK = true;
            }
            else
            {
                LOG_APP_WARN("Houston, we have problem: (m_Window) failed");
            }
        }

        // set app icon
        GLFWimage icon;
        size_t fileSize;
        const uchar* data =
            (const uchar*)ResourceSystem::GetDataPointer(fileSize, "/images/images/I_Vulkan.png", IDB_VULKAN, "PNG");
        icon.pixels = stbi_load_from_memory(data, fileSize, &icon.width, &icon.height, 0, 4); // rgba channels
        if (icon.pixels)
        {
            glfwSetWindowIcon(m_Window, 1, &icon);
            stbi_image_free(icon.pixels);
        }
        else
        {
            LOG_CORE_WARN("Could not load app icon");
        }
    }

    VK_Window::~VK_Window()
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void VK_Window::Shutdown()
    {
        VK_Core::m_Device->Shutdown();
        VK_Core::m_Device->WaitIdle();
    }

    void VK_Window::ToggleFullscreen()
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        if (m_IsFullscreen)
        {
            m_WindowProperties.m_Width = m_WindowedWidth;
            m_WindowProperties.m_Height = m_WindowedHeight;
            m_WindowProperties.m_AspectRatio = static_cast<float>(m_WindowedWidth) / static_cast<float>(m_WindowedHeight);

            glfwSetWindowMonitor(m_Window, nullptr, m_WindowPositionX, m_WindowPositionY, m_WindowedWidth, m_WindowedHeight,
                                 videoMode->refreshRate);
        }
        else
        {
            m_WindowedWidth = m_WindowProperties.m_Width;
            m_WindowedHeight = m_WindowProperties.m_Height;
            m_WindowProperties.m_AspectRatio =
                static_cast<float>(m_WindowProperties.m_Width) / static_cast<float>(m_WindowProperties.m_Height);
            glfwGetWindowPos(m_Window, &m_WindowPositionX, &m_WindowPositionY);

            glfwSetWindowMonitor(m_Window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
        }
        m_IsFullscreen = !m_IsFullscreen;
    }

    void VK_Window::OnUpdate()
    {
        if (glfwWindowShouldClose(m_Window))
        {
            m_OK = false;
        }
        else
        {
            glfwPollEvents();
        }
    }

    void VK_Window::OnError(int errorCode, const char* description)
    {
        std::cout << "GLEW error, code: :" << std::to_string(errorCode) << ", description: " << description << std::endl;
    }

    void VK_Window::SetEventCallback(const EventCallbackFunction& callback)
    {
        m_WindowProperties.m_EventCallback = callback;
        glfwSetWindowUserPointer(m_Window, &m_WindowProperties);

        glfwSetErrorCallback([](int errorCode, const char* description) { VK_Window::OnError(errorCode, description); });

        glfwSetKeyCallback(m_Window,
                           [](GLFWwindow* window, int key, int scancode, int action, int modes)
                           {
                               WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
                               EventCallbackFunction OnEvent = windowProperties.m_EventCallback;

                               switch (action)
                               {
                                   case GLFW_PRESS:
                                   {
                                       KeyPressedEvent event(key);
                                       OnEvent(event);
                                       break;
                                   }
                                   case GLFW_RELEASE:
                                   {
                                       KeyReleasedEvent event(key);
                                       OnEvent(event);
                                       break;
                                   }
                                   case GLFW_REPEAT:
                                       break;
                               }
                           });

        glfwSetWindowFocusCallback(m_Window,
                                   [](GLFWwindow* window, int focused)
                                   {
                                       WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
                                       if (windowProperties.m_ToggleCmd)
                                       {
                                           windowProperties.m_ToggleCmd = false;
                                           windowProperties.m_Window->ToggleFullscreen();
                                       }
                                   });

        glfwSetFramebufferSizeCallback(m_Window,
                                       [](GLFWwindow* window, int width, int height)
                                       {
                                           WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
                                           EventCallbackFunction OnEvent = windowProperties.m_EventCallback;

                                           windowProperties.m_Width = width;
                                           windowProperties.m_Height = height;
                                           windowProperties.m_AspectRatio =
                                               static_cast<float>(width) / static_cast<float>(height);
                                           windowProperties.m_FramebufferResized = true;

                                           WindowResizeEvent event(width, height);
                                           OnEvent(event);
                                       });

        glfwSetWindowIconifyCallback(m_Window,
                                     [](GLFWwindow* window, int iconified)
                                     {
                                         int width, height;
                                         WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
                                         EventCallbackFunction OnEvent = windowProperties.m_EventCallback;

                                         if (iconified)
                                         {
                                             width = height = 0;
                                         }
                                         else
                                         {
                                             glfwGetWindowSize(window, &width, &height);
                                         }

                                         windowProperties.m_Width = width;
                                         windowProperties.m_Height = height;

                                         WindowResizeEvent event(width, height);
                                         OnEvent(event);
                                     });

        glfwSetMouseButtonCallback(m_Window,
                                   [](GLFWwindow* window, int button, int action, int mods)
                                   {
                                       WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
                                       EventCallbackFunction OnEvent = windowProperties.m_EventCallback;

                                       switch (action)
                                       {
                                           case GLFW_PRESS:
                                           {
                                               MouseButtonPressedEvent event(button, windowProperties.m_MousePosX,
                                                                             windowProperties.m_MousePosY);
                                               OnEvent(event);
                                               break;
                                           }
                                           case GLFW_RELEASE:
                                           {
                                               MouseButtonReleasedEvent event(button);
                                               OnEvent(event);
                                               break;
                                           }
                                       }
                                   });

        glfwSetCursorPosCallback(m_Window,
                                 [](GLFWwindow* window, double xpos, double ypos)
                                 {
                                     WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
                                     EventCallbackFunction OnEvent = windowProperties.m_EventCallback;

                                     windowProperties.m_MousePosX = xpos;
                                     windowProperties.m_MousePosY = ypos;

                                     MouseMovedEvent event(xpos, ypos);
                                     OnEvent(event);
                                 });

        glfwSetScrollCallback(m_Window,
                              [](GLFWwindow* window, double xoffset, double yoffset)
                              {
                                  WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
                                  EventCallbackFunction OnEvent = windowProperties.m_EventCallback;

                                  MouseScrolledEvent event(xoffset, yoffset);
                                  OnEvent(event);
                              });
    }

    bool VK_Window::InitGLFW()
    {

        // init glfw
        if (!glfwInit())
        {
            LOG_CORE_CRITICAL("glfwInit() failed");
            return false;
        }

        return true;
    }

    void VK_Window::EnableMousePointer()
    {
        if (m_AllowCursor)
        {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    void VK_Window::DisableMousePointer() { glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); }

    void VK_Window::AllowCursor() { m_AllowCursor = true; }
    void VK_Window::DisallowCursor()
    {
        m_AllowCursor = false;
        DisableMousePointer();
    }

    void VK_Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
    {
        auto result = glfwCreateWindowSurface(instance, m_Window, nullptr, surface);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("Could not create window surface");
        }
    }

    void VK_Window::CreateWindow()
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        m_RefreshRate = videoMode->refreshRate;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // make window invisible before it gets created
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        int monitorX, monitorY;
        glfwGetMonitorPos(monitor, &monitorX, &monitorY);
        m_DesktopWidth = videoMode->width;
        m_DesktopHeight = videoMode->height;
        m_WindowedWidth = videoMode->width / 2.5f;
        m_WindowedHeight = m_WindowedWidth;
        m_WindowPositionX = (videoMode->width - m_WindowedWidth) / 2;
        m_WindowPositionY = (videoMode->height - m_WindowedHeight) / 2;

        if (CoreSettings::m_EnableFullscreen)
        {
#ifdef _WIN32
            m_WindowProperties.m_Width = videoMode->width;
            m_WindowProperties.m_Height = videoMode->height;
            m_WindowProperties.m_AspectRatio =
                static_cast<float>(m_WindowProperties.m_Width) / static_cast<float>(m_WindowProperties.m_Height);
            m_Window = glfwCreateWindow(m_WindowProperties.m_Width, m_WindowProperties.m_Height,
                                        m_WindowProperties.m_Title.c_str(), monitor, nullptr);
            m_IsFullscreen = true;
#else
            // go to windowed mode first and then toggle to fullscreen in 'window focused' callback
            m_IsFullscreen = false;
            m_WindowProperties.m_Width = m_WindowedWidth;
            m_WindowProperties.m_Height = m_WindowedHeight;
            m_WindowProperties.m_AspectRatio = static_cast<float>(m_WindowedWidth) / static_cast<float>(m_WindowedHeight);
            m_WindowProperties.m_ToggleCmd = CoreSettings::m_EnableFullscreen;

            m_Window = glfwCreateWindow(m_WindowProperties.m_Width, m_WindowProperties.m_Height,
                                        m_WindowProperties.m_Title.c_str(), nullptr, nullptr);
#endif
        }
        else
        {
            m_WindowProperties.m_Width = m_WindowedWidth;
            m_WindowProperties.m_Height = m_WindowedHeight;
            m_WindowProperties.m_AspectRatio = static_cast<float>(m_WindowedWidth) / static_cast<float>(m_WindowedHeight);

            m_Window = glfwCreateWindow(m_WindowProperties.m_Width, m_WindowProperties.m_Height,
                                        m_WindowProperties.m_Title.c_str(), nullptr, nullptr);
            m_IsFullscreen = false;
        }

        // center window
        glfwSetWindowPos(m_Window, monitorX + m_WindowPositionX, monitorY + m_WindowPositionY);

        // make the window visible
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwShowWindow(m_Window);
    }

    void VK_Window::SetWindowAspectRatio()
    {
        // set aspect ratio to current ratio
        int numer = m_WindowProperties.m_Width;
        int denom = m_WindowProperties.m_Height;
        glfwSetWindowAspectRatio(m_Window, numer, denom);
    }

    void VK_Window::SetWindowAspectRatio(int numer, int denom) { glfwSetWindowAspectRatio(m_Window, numer, denom); }
} // namespace GfxRenderEngine
