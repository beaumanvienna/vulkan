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

#pragma once

#include <chrono>

#include "engine.h"
#include "renderer/graphicsContext.h"

#include "VKwindow.h"
#include "VKdevice.h"
#include "VKrenderer.h"

namespace GfxRenderEngine
{
    class VK_Context : public GraphicsContext
    {

    public:

        VK_Context(VK_Window* window);
        ~VK_Context() override {}

        virtual bool Init() override;
        virtual void SetVSync(int interval) override;
        virtual void SwapBuffers() override;
        virtual bool IsInitialized() const override { return m_Initialized; }

        virtual std::shared_ptr<Renderer> GetRenderer() const override { return m_Renderer; }
        virtual std::shared_ptr<Model> LoadModel(const Builder& builder) override;
        virtual void ToggleDebugWindow(const GenericCallback& callback = nullptr) override { m_Renderer->ToggleDebugWindow(callback);}

        virtual uint GetContextWidth() const override { return m_Renderer->GetContextWidth(); }
        virtual uint GetContextHeight() const override { return m_Renderer->GetContextHeight(); }

    private:

        bool m_Initialized;

        VK_Window* m_Window;
        std::shared_ptr<VK_Renderer> m_Renderer;

        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
        std::chrono::duration<float, std::chrono::seconds::period> m_FrameDuration;
        int m_VSyncIsWorking;

    };
}
