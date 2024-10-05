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

   This file is based on
   github.com/blurrypiano/littleVulkanEngine/blob/master/littleVulkanEngine/imguiDemo/lve_imgui.cpp
   License: MIT */

#include "core.h"

#include "platform/Vulkan/imguiEngine/imgui.h"
#include "platform/Vulkan/imguiEngine/VKimgui.h"
#include "platform/Vulkan/imguiEngine/imguiNull.h"

namespace GfxRenderEngine
{
    std::unique_ptr<Imgui> Imgui::m_Imgui;
    std::unique_ptr<Imgui> Imgui::m_ImguiNull;
    bool Imgui::m_ImguiDebugWindowEnabled{false};
    GenericCallback Imgui::m_Callback;

    Imgui* Imgui::Create(VkRenderPass renderPass, uint imageCount)
    {
        m_Imgui = std::make_unique<VK_Imgui>(renderPass, imageCount);
        m_ImguiNull = std::make_unique<ImguiNull>();
        return m_ImguiNull.get();
    }

    void Imgui::Destroy()
    {
        if (m_Imgui)
        {
            m_Imgui.reset();
        }
        if (m_ImguiNull)
        {
            m_ImguiNull.reset();
        }
    }

    Imgui* Imgui::ToggleDebugWindow(const GenericCallback& callback)
    {
        m_Callback = callback;
        m_ImguiDebugWindowEnabled = !m_ImguiDebugWindowEnabled;
        if (m_ImguiDebugWindowEnabled)
        {
            return m_Imgui.get();
        }
        else
        {
            return m_ImguiNull.get();
        }
    }
} // namespace GfxRenderEngine
