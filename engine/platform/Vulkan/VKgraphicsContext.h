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
        VK_Context(VK_Window* window, ThreadPool& threadPoolPrimary, ThreadPool& threadPoolSecondary);
        virtual ~VK_Context() override;

        virtual bool Init() override;
        virtual void SetVSync(int interval) override;
        virtual void LimitFrameRate(Chrono::TimePoint) override;
        virtual bool IsInitialized() const override { return m_Initialized; }

        virtual Renderer* GetRenderer() const override { return m_Renderer.get(); }
        virtual std::shared_ptr<Model> LoadModel(const Builder& builder) override;
        virtual std::shared_ptr<Model> LoadModel(const TerrainBuilder& builder) override;
        virtual std::shared_ptr<Model> LoadModel(const GltfBuilder& builder) override;
        virtual std::shared_ptr<Model> LoadModel(const Model::ModelData& modelData) override;
        virtual std::shared_ptr<Model> LoadModel(const FbxBuilder& builder) override;
        virtual std::shared_ptr<Model> LoadModel(const UFbxBuilder& builder) override;
        virtual void ToggleDebugWindow(const GenericCallback& callback = nullptr) override
        {
            m_Renderer->ToggleDebugWindow(callback);
        }

        virtual uint GetContextWidth() const override { return m_Renderer->GetContextWidth(); }
        virtual uint GetContextHeight() const override { return m_Renderer->GetContextHeight(); }
        virtual bool MultiThreadingSupport() const override;
        virtual void WaitIdle() const override;
        virtual void ResetDescriptorPool(ThreadPool& threadPool) override;

    private:
        bool m_Initialized;

        VK_Window* m_Window;
        std::unique_ptr<VK_Device> m_Device;
        std::unique_ptr<VK_Renderer> m_Renderer;

        Chrono::TimePoint m_StartTime;

        // *** m_MinFrameDuration ***
        // The main thread must use at least m_MinFrameDuration of CPU time.
        // If the app is using less, LimitFrameRate() pads the remainder via sleep().
        // Without the frame limiter, vkQueuesubmit() would pad the remainder,
        // and we don't want that:
        // vkQueuesubmit() is blocking the acces mutex and thus background operations
        // on the queue (such as resource loading) are blocked as well.
        Chrono::Duration m_MinFrameDuration{16.0ms};
    };
} // namespace GfxRenderEngine
