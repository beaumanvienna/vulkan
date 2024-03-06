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

#include <functional>
#include <memory>

#include "engine.h"
#include "events/event.h"
#include "renderer/model.h"
#include "renderer/renderer.h"

namespace GfxRenderEngine
{

    typedef std::function<void(Event&)> EventCallbackFunction;

    struct WindowProperties
    {
        std::string m_Title;
        int m_Width;
        int m_Height;
        int m_VSync;

        WindowProperties(const std::string& title = "", const bool vsync = 1 /*true*/,
                        const int width = -1, const int height = -1)
            : m_Title(title), m_VSync(vsync), m_Width(width), m_Height(height)
        {
        }
    };

    class Window
    {

    public:

        Window() {}
        virtual ~Window() {}

        virtual void Shutdown() = 0;
        virtual void* GetBackendWindow() const = 0;
        virtual bool  IsOK() const = 0;
        virtual float GetWindowAspectRatio() const = 0;
        virtual void  SetWindowAspectRatio() = 0;
        virtual void  SetWindowAspectRatio(int numer, int denom) = 0;
        virtual void  OnUpdate() = 0;
        virtual uint  GetWidth() const = 0;
        virtual uint  GetHeight() const = 0;
        virtual uint  GetDesktopWidth() const = 0;
        virtual uint  GetDesktopHeight() const = 0;
        virtual double GetTime() const = 0;
        virtual void SetEventCallback(const EventCallbackFunction& callback) = 0;
        //virtual void SetVSync(int interval) = 0;
        virtual void ToggleFullscreen() = 0;
        virtual bool IsFullscreen() = 0;

        virtual void EnableMousePointer() = 0;
        virtual void DisableMousePointer() = 0;
        virtual void AllowCursor() = 0;
        virtual void DisallowCursor() = 0;

        static std::unique_ptr<Window> Create(const WindowProperties& props);

    protected:

    private:

    };
}
