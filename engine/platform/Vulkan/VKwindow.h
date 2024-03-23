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

#include <memory>

#include "engine.h"
#include "platform/window.h"

#include "VKcore.h"
#include "VKswapChain.h"
#include "VKmodel.h"
#include "VKrenderer.h"

#define GLFW_INCLUDE_VULKAN
#include "glfw/include/GLFW/glfw3.h"

namespace GfxRenderEngine
{

    class VK_Window : public Window
    {

    public:
        VK_Window(const WindowProperties& props);
        virtual ~VK_Window() override;

        VK_Window(const VK_Window&) = delete;
        VK_Window& operator=(const VK_Window&) = delete;

        bool InitGLFW();
        virtual void Shutdown() override;
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
        VkExtent2D GetExtent()
        {
            return {static_cast<uint>(m_WindowProperties.m_Width), static_cast<uint>(m_WindowProperties.m_Height)};
        }
        void* GetBackendWindow() const override { return (void*)m_Window; }

        void OnUpdate() override;
        uint GetWidth() const override { return m_WindowProperties.m_Width; }
        uint GetHeight() const override { return m_WindowProperties.m_Height; }
        uint GetDesktopWidth() const override { return m_DesktopWidth; }
        uint GetDesktopHeight() const override { return m_DesktopHeight; }

        void SetEventCallback(const EventCallbackFunction& callback) override;
        void ToggleFullscreen() override;
        bool IsFullscreen() override { return m_IsFullscreen; }
        bool IsOK() const override { return m_OK; }
        void SetWindowAspectRatio() override;
        void SetWindowAspectRatio(int numer, int denom) override;
        float GetWindowAspectRatio() const override { return m_WindowProperties.m_AspectRatio; }
        double GetTime() const override { return glfwGetTime(); }

        static void OnError(int errorCode, const char* description);

        void EnableMousePointer() override;
        void DisableMousePointer() override;
        virtual void AllowCursor() override;
        virtual void DisallowCursor() override;

        bool WasResized() const { return m_WindowProperties.m_FramebufferResized; }
        void ResetWindowResizedFlag() { m_WindowProperties.m_FramebufferResized = false; }

    private:
        void CreateWindow();

    private:
        GLFWwindow* m_Window;

        struct WindowData
        {
            std::string m_Title;
            int m_Width;
            int m_Height;
            float m_AspectRatio;
            // int m_VSync;
            EventCallbackFunction m_EventCallback;
            double m_MousePosX;
            double m_MousePosY;
            bool m_FramebufferResized;
            bool m_ToggleCmd;
            VK_Window* m_Window;
        };

        static bool m_GLFWIsInitialized;
        //
        bool m_OK;

        WindowData m_WindowProperties;

        uint m_RefreshRate;
        bool m_IsFullscreen;

        int m_WindowedWidth, m_WindowedHeight;
        int m_WindowPositionX, m_WindowPositionY;
        bool m_AllowCursor;
        uint m_DesktopWidth, m_DesktopHeight;
    };
} // namespace GfxRenderEngine
