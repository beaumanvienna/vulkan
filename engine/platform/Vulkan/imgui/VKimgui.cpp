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

#include "core.h"

#include "imgui/VKimgui.h"

namespace GfxRenderEngine
{

    VK_Imgui::VK_Imgui(VkRenderPass renderPass, uint imageCount)
    {
        // set up a descriptor pool stored on this instance, see header for more comments on this.
        VkDescriptorPoolSize pool_sizes[] = 
        {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        auto result = vkCreateDescriptorPool(VK_Core::m_Device->Device(), &pool_info, nullptr, &descriptorPool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up descriptor pool for imgui");
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        // Initialize imgui for vulkan
        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Engine::m_Engine->GetBackendWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = VK_Core::m_Device->GetInstance();
        init_info.PhysicalDevice = VK_Core::m_Device->PhysicalDevice();
        init_info.Device = VK_Core::m_Device->Device();
        init_info.QueueFamily = VK_Core::m_Device->GetGraphicsQueueFamily();
        init_info.Queue = VK_Core::m_Device->GraphicsQueue();

        // pipeline cache is a potential future optimization, ignoring for now
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        // todo, I should probably get around to integrating a memory allocator library such as Vulkan
        // memory allocator (VMA) sooner than later. We don't want to have to update adding an allocator
        // in a ton of locations.
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.MinImageCount = 2;
        init_info.ImageCount = imageCount;
        init_info.CheckVkResultFn = VKCheckResult;
        ImGui_ImplVulkan_Init(&init_info, renderPass);

        // upload fonts, this is done by recording and submitting a one time use command buffer
        // which can be done easily bye using some existing helper functions on the lve device object
        auto commandBuffer = VK_Core::m_Device->BeginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        VK_Core::m_Device->EndSingleTimeCommands(commandBuffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    VK_Imgui::~VK_Imgui()
    {
        vkDestroyDescriptorPool(VK_Core::m_Device->Device(), descriptorPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
        //ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void VK_Imgui::NewFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // this tells imgui that we're done setting up the current frame,
    // then gets the draw data from imgui and uses it to record to the provided
    // command buffer the necessary draw commands
    void VK_Imgui::Render(VkCommandBuffer commandBuffer)
    {
        ImGui::Render();
        ImDrawData *drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, commandBuffer);
    }

    void VK_Imgui::Run()
    {
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can
        // browse its code to learn more about Dear ImGui!).
        if (m_ShowDemoWindow) ImGui::ShowDemoWindow(&m_ShowDemoWindow);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named
        // window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Vulkan Engine Debug Window");  // Create a window called "Hello, world!" and append into it.

            //ImGui::Text("This is some useful text.");  // Display some text (you can use a format strings too)
            //ImGui::Checkbox(
            //    "Demo Window",
            //    &m_ShowDemoWindow);  // Edit bools storing our window open/close state
            //ImGui::Checkbox("Another Window", &m_ShowAnotherWindow);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color",
                            (float *)&m_ClearColor);  // Edit 3 floats representing a color

            if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true// when edited/activated)
            {
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text
            (
                "Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate
            );
            ImGui::End();
        }

        // 3. Show another simple window.
        if (m_ShowAnotherWindow)
        {
            ImGui::Begin     // Pass a pointer to our bool variable (the window will have a
            (                // closing button that will clear the bool when clicked)
                "Another Window",
                &m_ShowAnotherWindow
            );  

            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me")) m_ShowAnotherWindow = false;
            ImGui::End();
        }
    }
}
