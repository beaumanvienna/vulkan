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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   This file is based on 
   github.com/blurrypiano/littleVulkanEngine/blob/master/littleVulkanEngine/imguiDemo/lve_imgui.cpp
   License: MIT */

#pragma once

#include "VKcore.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

// This whole class is only necessary right now because it needs to manage the descriptor pool
// because we haven't set one up anywhere else in the application, and we manage the
// example state, otherwise all the functions could just be static helper functions if you prefered
namespace GfxRenderEngine
{

    static void VKCheckResult(VkResult err)
    {
        if (err == 0) return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0) abort();
    }

    class VK_Imgui
    {
        public:
            VK_Imgui(VkRenderPass renderPass, uint32_t imageCount);
            ~VK_Imgui();

            void NewFrame();
            void Render(VkCommandBuffer commandBuffer);
            void Run();

        public:

            bool m_ShowDemoWindow = false;
            bool m_ShowAnotherWindow = false;
            ImVec4 m_ClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        private:

            // We haven't yet covered descriptor pools in the tutorial series
            // so I'm just going to create one for just imgui and store it here for now.
            // maybe its preferred to have a separate descriptor pool for imgui anyway,
            // I haven't looked into imgui best practices at all.
            VkDescriptorPool descriptorPool;
    };
}
