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
        : m_Window{window}, m_FrameDuration{16.667ms}, m_VSyncIsWorking{10}, m_Initialized{false}
    {
        // create a device
        VK_Core::m_Device = std::make_shared<VK_Device>(window, threadPoolPrimary, threadPoolSecondary);
        m_Renderer = std::make_shared<VK_Renderer>(m_Window);
        m_Initialized = m_Renderer->Init();
    }

    bool VK_Context::Init()
    {
        if (!m_Initialized)
        {
            m_Initialized = m_Renderer->Init();
        }
        else
        {
            LOG_CORE_WARN("VK_Context: already initialized");
        }
        return m_Initialized;
    }

    void VK_Context::SetVSync(int interval) {}

    void VK_Context::SwapBuffers()
    {
        auto diffTime = Engine::m_Engine->GetTime() - m_StartTime;
        auto sleepTime = m_FrameDuration - diffTime - 150us;
        if (sleepTime < 0s)
            sleepTime = 0s;

        // here ends the frame
        if (m_VSyncIsWorking)
        {
            if (diffTime < (m_FrameDuration / 2))
            {
                // time difference too short
                m_VSyncIsWorking--;
            }
        }
        else
        {
            std::this_thread::sleep_for(sleepTime);
        }
        // here starts the new frame
        m_StartTime = Engine::m_Engine->GetTime();
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const Builder& builder)
    {
        ASSERT(VK_Core::m_Device != nullptr);
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const TerrainBuilder& builder)
    {
        ASSERT(VK_Core::m_Device != nullptr);
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const GltfBuilder& builder)
    {
        ASSERT(VK_Core::m_Device != nullptr);
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const FastgltfBuilder& builder)
    {
        ASSERT(VK_Core::m_Device != nullptr);
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const FbxBuilder& builder)
    {
        ASSERT(VK_Core::m_Device != nullptr);
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const UFbxBuilder& builder)
    {
        ASSERT(VK_Core::m_Device != nullptr);
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

    bool VK_Context::MultiThreadingSupport() const { return VK_Core::m_Device->MultiThreadingSupport(); }
} // namespace GfxRenderEngine
