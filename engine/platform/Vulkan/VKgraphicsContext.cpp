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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The code in this file is based on and inspired by the project
   https://github.com/TheCherno/Hazel. The license of this prject can
   be found under https://github.com/TheCherno/Hazel/blob/master/LICENSE
   */

#include "core.h"

#include "VKcore.h"
#include "VKgraphicsContext.h"

namespace GfxRenderEngine
{
    VK_Context::VK_Context(VK_Window* window, ThreadPool& threadPoolPrimary, ThreadPool& threadPoolSecondary)
        : m_Window{window}, m_Initialized{false}
    {
        // create a device
        m_Device = std::make_unique<VK_Device>(window);
        VK_Core::m_Device = m_Device.get();
        m_Device->LoadPool(threadPoolPrimary, threadPoolSecondary);
        m_Renderer = std::make_unique<VK_Renderer>(m_Window);
        m_Initialized = m_Renderer->Init();
    }

    VK_Context::~VK_Context() {}

    bool VK_Context::Init()
    {
        if (!m_Initialized)
        {
            m_Initialized = m_Renderer->Init();
        }
        return m_Initialized;
    }

    void VK_Context::SetVSync(int interval) {}

    void VK_Context::LimitFrameRate(Chrono::TimePoint startTime)
    {
        ZoneScopedN("LimitFrameRate");
        auto diffTime = Engine::m_Engine->GetTime() - startTime;
        auto sleepTime = m_MinFrameDuration - diffTime;

        if (sleepTime > 0s)
        {
#ifdef MACOSX
            static constexpr Chrono::Duration MACOS_MAX_SLEEP_TIME = 10ms;
            if (sleepTime > MACOS_MAX_SLEEP_TIME)
            {
                sleepTime = MACOS_MAX_SLEEP_TIME;
            }
#endif
            std::this_thread::sleep_for(sleepTime);
        }
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const Builder& builder)
    {
        CORE_ASSERT(VK_Core::m_Device != nullptr, "device is null");
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const TerrainBuilder& builder)
    {
        CORE_ASSERT(VK_Core::m_Device != nullptr, "device is null");
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const GltfBuilder& builder)
    {
        CORE_ASSERT(VK_Core::m_Device != nullptr, "device is null");
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const Model::ModelData& modelData)
    {
        auto model = std::make_shared<VK_Model>(modelData);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const FbxBuilder& builder)
    {
        CORE_ASSERT(VK_Core::m_Device != nullptr, "device is null");
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const UFbxBuilder& builder)
    {
        CORE_ASSERT(VK_Core::m_Device != nullptr, "device is null");
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    bool VK_Context::MultiThreadingSupport() const { return VK_Core::m_Device->MultiThreadingSupport(); }

    void VK_Context::WaitIdle() const { VK_Core::m_Device->WaitIdle(); };

    void VK_Context::ResetDescriptorPool(ThreadPool& threadPool) { VK_Core::m_Device->ResetPool(threadPool); };
} // namespace GfxRenderEngine
