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

#include "engine.h"
#include "core.h"
#include "resources/resources.h"

#include "VKrenderer.h"
#include "VKwindow.h"

namespace GfxRenderEngine
{
    std::shared_ptr<Texture> gTextureBloodIsland;
    std::shared_ptr<Texture> gTextureWalkway;

    VK_Renderer::VK_Renderer(VK_Window* window, std::shared_ptr<VK_Device> device)
        : m_Window{window}, m_Device{device},
          m_CurrentImageIndex{0},
          m_CurrentFrameIndex{0},
          m_FrameInProgress{false}
    {
        RecreateSwapChain();
        CreateCommandBuffers();

        for (uint i = 0; i < m_UniformBuffers.size(); i++)
        {
            m_UniformBuffers[i] = std::make_unique<VK_Buffer>
            (
                *m_Device, sizeof(GlobalUniformBuffer),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                m_Device->properties.limits.minUniformBufferOffsetAlignment
            );
            m_UniformBuffers[i]->Map();
        }

        // create a global pool for desciptor sets
        m_DescriptorPool = 
            VK_DescriptorPool::Builder()
            .SetMaxSets(VK_SwapChain::MAX_FRAMES_IN_FLIGHT * 4)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SwapChain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SwapChain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SwapChain::MAX_FRAMES_IN_FLIGHT)
            .Build();

        std::unique_ptr<VK_DescriptorSetLayout> globalDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts =
        {
            globalDescriptorSetLayout->GetDescriptorSetLayout()
        };

    #warning "fix me"
    
    size_t fileSize;
    const uchar* data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/images/Blood_Island.png", IDB_BLOOD_ISLAND, "PNG");
    auto textureBloodIsland = std::make_shared<VK_Texture>();
    textureBloodIsland->Init(data, fileSize);
    
    gTextureBloodIsland = textureBloodIsland; // copy from VK_Texture to Texture
    VkDescriptorImageInfo imageInfo0 {};
    imageInfo0.sampler     = textureBloodIsland->m_Sampler;
    imageInfo0.imageView   = textureBloodIsland->m_TextureView;
    imageInfo0.imageLayout = textureBloodIsland->m_ImageLayout;

    data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/images/walkway.png", IDB_WALKWAY, "PNG");
    auto textureWalkway = std::make_shared<VK_Texture>();
    textureWalkway->Init(data, fileSize);

    gTextureWalkway = textureWalkway; // copy from VK_Texture to Texture
    VkDescriptorImageInfo imageInfo1 {};
    imageInfo1.sampler     = textureWalkway->m_Sampler;
    imageInfo1.imageView   = textureWalkway->m_TextureView;
    imageInfo1.imageLayout = textureWalkway->m_ImageLayout;

        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo = m_UniformBuffers[i]->DescriptorInfo();
            VK_DescriptorWriter(*globalDescriptorSetLayout, *m_DescriptorPool)
                .WriteBuffer(0, &bufferInfo)
                .WriteImage(1, &imageInfo0)
                .WriteImage(2, &imageInfo1)
                .Build(m_GlobalDescriptorSets[i]);
        }

        m_RenderSystem = std::make_unique<VK_RenderSystem>(m_SwapChain->GetRenderPass(), descriptorSetLayouts);
        m_PointLightSystem = std::make_unique<VK_PointLightSystem>(m_Device, m_SwapChain->GetRenderPass(), *globalDescriptorSetLayout);
        m_Imgui = std::make_unique<VK_Imgui>(m_SwapChain->GetRenderPass(), static_cast<uint>(m_SwapChain->ImageCount()));
    }

    VK_Renderer::~VK_Renderer()
    {
        FreeCommandBuffers();
    }

    void VK_Renderer::RecreateSwapChain()
    {
        auto extent = m_Window->GetExtend();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = m_Window->GetExtend();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_Device->Device());

        // create the swapchain and pipeline
        if (m_SwapChain == nullptr)
        {
            m_SwapChain = std::make_unique<VK_SwapChain>(m_Device, extent);
        }
        else
        {
            std::shared_ptr<VK_SwapChain> oldSwapChain = std::move(m_SwapChain);
            m_SwapChain = std::make_unique<VK_SwapChain>(m_Device, extent, oldSwapChain);

            if (!oldSwapChain->CompareSwapFormats(*m_SwapChain.get()))
            {
                LOG_CORE_CRITICAL("swap chain image or depth format has changed");
            }
        }

    }

    void VK_Renderer::CreateCommandBuffers()
    {
        m_CommandBuffers.resize(VK_SwapChain::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = m_Device->GetCommandPool();
        allocateInfo.commandBufferCount = static_cast<uint>(m_CommandBuffers.size());

        if (vkAllocateCommandBuffers(m_Device->Device(), &allocateInfo, m_CommandBuffers.data()) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to allocate command buffers");
        }
    }

    void VK_Renderer::FreeCommandBuffers()
    {
        vkFreeCommandBuffers
        (
            m_Device->Device(),
            m_Device->GetCommandPool(),
            static_cast<uint>(m_CommandBuffers.size()),
            m_CommandBuffers.data()
        );
        m_CommandBuffers.clear();
    }

    VkCommandBuffer VK_Renderer::GetCurrentCommandBuffer() const
    {
        ASSERT(m_FrameInProgress);
        return m_CommandBuffers[m_CurrentFrameIndex];
    }

    VkCommandBuffer VK_Renderer::BeginFrame()
    {
        ASSERT(!m_FrameInProgress);

        auto result = m_SwapChain->AcquireNextImage(&m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LOG_CORE_CRITICAL("failed to acquire next swap chain image");
        }

        m_FrameInProgress = true;

        auto commandBuffer = GetCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to begin recording command buffer!");
        }

        return commandBuffer;

    }

    void VK_Renderer::EndFrame()
    {
        ASSERT(m_FrameInProgress);

        auto commandBuffer = GetCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("recording of command buffer failed");
        }
        auto result = m_SwapChain->SubmitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window->WasResized())
        {
            m_Window->ResetWindowResizedFlag();
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            LOG_CORE_WARN("failed to present swap chain image");
        }
        m_FrameInProgress = false;
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % VK_SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void VK_Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        ASSERT(m_FrameInProgress);
        ASSERT(commandBuffer == GetCurrentCommandBuffer());

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_SwapChain->GetSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    }

    void VK_Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        ASSERT(m_FrameInProgress);
        ASSERT(commandBuffer == GetCurrentCommandBuffer());

        vkCmdEndRenderPass(commandBuffer);
    }

    void VK_Renderer::BeginScene(Camera* camera, entt::registry& registry)
    {
        m_Camera = camera;
        if (m_CurrentCommandBuffer = BeginFrame())
        {
            m_FrameInfo = {m_CurrentFrameIndex, 0.0f, m_CurrentCommandBuffer, m_Camera, m_GlobalDescriptorSets[m_CurrentFrameIndex]};

            GlobalUniformBuffer ubo{};
            ubo.m_Projection = m_Camera->GetProjectionMatrix();
            ubo.m_View = m_Camera->GetViewMatrix();
            ubo.m_AmbientLightColor = {1.0f, 1.0f, 1.0f, 0.02f};
            m_PointLightSystem->Update(m_FrameInfo, ubo, registry);
            m_UniformBuffers[m_CurrentFrameIndex]->WriteToBuffer(&ubo);
            m_UniformBuffers[m_CurrentFrameIndex]->Flush();

            m_Imgui->NewFrame();

            BeginSwapChainRenderPass(m_CurrentCommandBuffer);
        }
    }

    void VK_Renderer::Submit(entt::registry& registry)
    {
        if (m_CurrentCommandBuffer)
        {
            m_RenderSystem->RenderEntities(m_FrameInfo, registry);
            m_PointLightSystem->Render(m_FrameInfo, registry);

            m_Imgui->Run();
            m_Imgui->Render(m_CurrentCommandBuffer);
        }
    }

    void VK_Renderer::EndScene()
    {
        if (m_CurrentCommandBuffer)
        {
            EndSwapChainRenderPass(m_CurrentCommandBuffer);
            EndFrame();
        }
    }

    int VK_Renderer::GetFrameIndex() const
    {
        ASSERT(m_FrameInProgress);
        return m_CurrentFrameIndex;
    }

}
