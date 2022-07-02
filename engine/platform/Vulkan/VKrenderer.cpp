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
#include "auxiliary/file.h"

#include "VKrenderer.h"
#include "VKwindow.h"
#include "VKshader.h"

namespace GfxRenderEngine
{
    std::shared_ptr<Texture> gTextureSpritesheet;
    std::shared_ptr<Texture> gTextureFontAtlas;

    std::unique_ptr<VK_DescriptorPool> VK_Renderer::m_DescriptorPool;

    VK_Renderer::VK_Renderer(VK_Window* window, std::shared_ptr<VK_Device> device)
        : m_Window{window}, m_Device{device},
          m_CurrentImageIndex{0},
          m_CurrentFrameIndex{0},
          m_FrameInProgress{false}
    {
        CompileShaders();
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
        static constexpr uint POOL_SIZE = 1000;
        m_DescriptorPool = 
            VK_DescriptorPool::Builder()
            .SetMaxSets(VK_SwapChain::MAX_FRAMES_IN_FLIGHT * POOL_SIZE)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SwapChain::MAX_FRAMES_IN_FLIGHT * 50)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SwapChain::MAX_FRAMES_IN_FLIGHT * 900)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SwapChain::MAX_FRAMES_IN_FLIGHT * 50)
            .Build();

        std::unique_ptr<VK_DescriptorSetLayout> globalDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // spritesheet
                    .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // font atlas
                    .Build();

        std::unique_ptr<VK_DescriptorSetLayout> diffuseDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // color map
                    .Build();

        std::unique_ptr<VK_DescriptorSetLayout> diffuseNormalDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // color map
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // normal map
                    .Build();

        std::unique_ptr<VK_DescriptorSetLayout> diffuseNormalRoughnessMetallicDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // color map
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // normal map
                    .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // roughness metallic map
                    .Build();

        m_LightingDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT) // g buffer position input attachment
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT) // g buffer normal input attachment
                    .AddBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT) // g buffer color input attachment
                    .AddBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT) // g buffer material input attachment
                    .Build();

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsDefaultDiffuse =
        {
            globalDescriptorSetLayout->GetDescriptorSetLayout()
        };

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsDiffuse =
        {
            globalDescriptorSetLayout->GetDescriptorSetLayout(),
            diffuseDescriptorSetLayout->GetDescriptorSetLayout()
        };

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsDiffuseNormal =
        {
            globalDescriptorSetLayout->GetDescriptorSetLayout(),
            diffuseNormalDescriptorSetLayout->GetDescriptorSetLayout()
        };

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsDiffuseNormalRoughnessMetallic =
        {
            globalDescriptorSetLayout->GetDescriptorSetLayout(),
            diffuseNormalRoughnessMetallicDescriptorSetLayout->GetDescriptorSetLayout()
        };

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsLighting =
        {
            globalDescriptorSetLayout->GetDescriptorSetLayout(),
            m_LightingDescriptorSetLayout->GetDescriptorSetLayout()
        };

        size_t fileSize;
        auto data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/atlas/atlas.png", IDB_ATLAS, "PNG");
        auto textureSpritesheet = std::make_shared<VK_Texture>(Engine::m_TextureSlotManager);
        textureSpritesheet->Init(data, fileSize);
        textureSpritesheet->m_FileName = "spritesheet";

        gTextureSpritesheet = textureSpritesheet; // copy from VK_Texture to Texture
        VkDescriptorImageInfo imageInfo0 {};
        imageInfo0.sampler     = textureSpritesheet->m_Sampler;
        imageInfo0.imageView   = textureSpritesheet->m_TextureView;
        imageInfo0.imageLayout = textureSpritesheet->m_ImageLayout;

        data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/atlas/fontAtlas.png", IDB_FONTS_RETRO, "PNG");
        auto textureFontAtlas = std::make_shared<VK_Texture>(Engine::m_TextureSlotManager);
        textureFontAtlas->Init(data, fileSize);
        textureFontAtlas->m_FileName = "font atlas";

        gTextureFontAtlas = textureFontAtlas; // copy from VK_Texture to Texture
        VkDescriptorImageInfo imageInfo1 {};
        imageInfo1.sampler     = textureFontAtlas->m_Sampler;
        imageInfo1.imageView   = textureFontAtlas->m_TextureView;
        imageInfo1.imageLayout = textureFontAtlas->m_ImageLayout;

        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo = m_UniformBuffers[i]->DescriptorInfo();
            VK_DescriptorWriter(*globalDescriptorSetLayout, *m_DescriptorPool)
                .WriteBuffer(0, &bufferInfo)
                .WriteImage(1, &imageInfo0)
                .WriteImage(2, &imageInfo1)
                .Build(m_GlobalDescriptorSets[i]);
        }

        m_PointLightSystem                              = std::make_unique<VK_PointLightSystem>(m_Device, m_SwapChain->GetRenderPass(), *globalDescriptorSetLayout);
        //m_RenderSystemDefaultDiffuseMap                 = std::make_unique<VK_RenderSystemDefaultDiffuseMap>(m_SwapChain->GetRenderPass(), descriptorSetLayoutsDiffuse);

        m_RenderSystemPbrNoMap                          = std::make_unique<VK_RenderSystemPbrNoMap>(m_SwapChain->GetRenderPass(), *globalDescriptorSetLayout);
        m_RenderSystemPbrDiffuse                        = std::make_unique<VK_RenderSystemPbrDiffuse>(m_SwapChain->GetRenderPass(), descriptorSetLayoutsDiffuse);
        m_RenderSystemPbrDiffuseNormal                  = std::make_unique<VK_RenderSystemPbrDiffuseNormal>(m_SwapChain->GetRenderPass(), descriptorSetLayoutsDiffuseNormal);
        m_RenderSystemPbrDiffuseNormalRoughnessMetallic = std::make_unique<VK_RenderSystemPbrDiffuseNormalRoughnessMetallic>(m_SwapChain->GetRenderPass(), descriptorSetLayoutsDiffuseNormalRoughnessMetallic);

        CreateLightingDescriptorSets();

        m_RenderSystemDeferredRendering                 = std::make_unique<VK_RenderSystemDeferredRendering>
        (
            m_SwapChain->GetRenderPass(),
            descriptorSetLayoutsLighting,
            m_LightingDescriptorSets.data()
        );

        //m_Imgui = Imgui::Create(m_SwapChain->GetRenderPass(), static_cast<uint>(m_SwapChain->ImageCount()));
    }
    
    void VK_Renderer::CreateLightingDescriptorSets()
    {
        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorImageInfo imageInfoGBufferPositionInputAttachment {};
            imageInfoGBufferPositionInputAttachment.imageView   = m_SwapChain->GetImageViewGBufferPosition(i);
            imageInfoGBufferPositionInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo imageInfoGBufferNormalInputAttachment {};
            imageInfoGBufferNormalInputAttachment.imageView   = m_SwapChain->GetImageViewGBufferNormal(i);
            imageInfoGBufferNormalInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo imageInfoGBufferColorInputAttachment {};
            imageInfoGBufferColorInputAttachment.imageView   = m_SwapChain->GetImageViewGBufferColor(i);
            imageInfoGBufferColorInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo imageInfoGBufferMaterialInputAttachment {};
            imageInfoGBufferMaterialInputAttachment.imageView   = m_SwapChain->GetImageViewGBufferMaterial(i);
            imageInfoGBufferMaterialInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VK_DescriptorWriter(*m_LightingDescriptorSetLayout, *m_DescriptorPool)
                .WriteImage(0, &imageInfoGBufferPositionInputAttachment)
                .WriteImage(1, &imageInfoGBufferNormalInputAttachment)
                .WriteImage(2, &imageInfoGBufferColorInputAttachment)
                .WriteImage(3, &imageInfoGBufferMaterialInputAttachment)
                .Build(m_LightingDescriptorSets[i]);
        }
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

        // create the swapchain
        if (m_SwapChain == nullptr)
        {
            m_SwapChain = std::make_unique<VK_SwapChain>(m_Device, extent);
        }
        else
        {
            std::shared_ptr<VK_SwapChain> oldSwapChain = std::move(m_SwapChain);
            m_SwapChain = std::make_unique<VK_SwapChain>(m_Device, extent, oldSwapChain);
            CreateLightingDescriptorSets();
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

        std::array<VkClearValue, VK_SwapChain::NUMBER_OF_ATTACHMENTS> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        clearValues[2].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[3].color = {0.5f, 0.5f, 0.1f, 1.0f};
        clearValues[4].color = {0.5f, 0.1f, 0.5f, 1.0f};
        clearValues[5].color = {0.5f, 0.7f, 0.2f, 1.0f};
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

    void VK_Renderer::BeginFrame(Camera* camera, entt::registry& registry)
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

            BeginSwapChainRenderPass(m_CurrentCommandBuffer);
        }
    }

    void VK_Renderer::UpdateTransformCache(entt::registry& registry, TreeNode& node, const glm::mat4& parentMat4, bool parentDirtyFlag)
    {
        entt::entity gameObject = node.GetGameObject();
        auto& transform = registry.get<TransformComponent>(gameObject);
        bool dirtyFlag = transform.GetDirtyFlag() || parentDirtyFlag;

        if (dirtyFlag)
        {
            transform.SetDirtyFlag();
            auto& mat4 = transform.GetMat4();
            glm::mat4 cleanMat4 = parentMat4*mat4;
            transform.SetMat4(cleanMat4);
            for (uint index = 0; index < node.Children(); index++)
            {
                UpdateTransformCache(registry, node.GetChild(index), cleanMat4, true);
            }
        }
        else
        {
            auto& mat4 = transform.GetMat4();
            for (uint index = 0; index < node.Children(); index++)
            {
                UpdateTransformCache(registry, node.GetChild(index), mat4, false);
            }

        }
    }

    void VK_Renderer::Submit(entt::registry& registry, TreeNode& sceneHierarchy)
    {
        if (m_CurrentCommandBuffer)
        {
            UpdateTransformCache(registry, sceneHierarchy, glm::mat4(1.0f), false);

            // sprites
//            m_RenderSystemDefaultDiffuseMap->RenderEntities(m_FrameInfo, registry);
//
//            // 3D objects
            m_RenderSystemPbrNoMap->RenderEntities(m_FrameInfo, registry);
            m_RenderSystemPbrDiffuse->RenderEntities(m_FrameInfo, registry);
            m_RenderSystemPbrDiffuseNormal->RenderEntities(m_FrameInfo, registry);
            m_RenderSystemPbrDiffuseNormalRoughnessMetallic->RenderEntities(m_FrameInfo, registry);
//
//            m_PointLightSystem->Render(m_FrameInfo, registry);
        }
    }

    void VK_Renderer::Submit(std::shared_ptr<ParticleSystem>& particleSystem)
    {
        if (m_CurrentCommandBuffer)
        {
            //m_RenderSystemDefaultDiffuseMap->DrawParticles(m_FrameInfo, particleSystem);
        }
    }

    void VK_Renderer::LightingPass()
    {
        if (m_CurrentCommandBuffer)
        {
            m_RenderSystemDeferredRendering->LightingPass(m_FrameInfo);
        }
    }

    void VK_Renderer::NextSubpass()
    {
        if (m_CurrentCommandBuffer)
        {
            vkCmdNextSubpass(m_CurrentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
        }
    }

    void VK_Renderer::SubmitGUI(entt::registry& registry)
    {
        if (m_CurrentCommandBuffer)
        {
            //m_RenderSystemDefaultDiffuseMap->RenderEntities(m_FrameInfo, registry);
        }
    }

    void VK_Renderer::EndScene()
    {
        if (m_CurrentCommandBuffer)
        {
            //m_Imgui->NewFrame();
            //m_Imgui->Run();
            //m_Imgui->Render(m_CurrentCommandBuffer);

            EndSwapChainRenderPass(m_CurrentCommandBuffer);
            EndFrame();
        }
    }

    int VK_Renderer::GetFrameIndex() const
    {
        ASSERT(m_FrameInProgress);
        return m_CurrentFrameIndex;
    }

    void VK_Renderer::CompileShaders()
    {
        if (!EngineCore::FileExists("bin"))
        {
            LOG_CORE_WARN("creating bin directory for spirv files");
            EngineCore::CreateDirectory("bin");
        }
        std::vector<std::string> shaderFilenames = 
        {
            "pointLight.vert",
            "pointLight.frag",
            "defaultDiffuseMap.vert",
            "defaultDiffuseMap.frag",
            "pbrNoMap.vert",
            "pbrNoMap.frag",
            "pbrDiffuse.vert",
            "pbrDiffuse.frag",
            "pbrDiffuseNormal.vert",
            "pbrDiffuseNormal.frag",
            "pbrDiffuseNormalRoughnessMetallic.vert",
            "pbrDiffuseNormalRoughnessMetallic.frag",
            "deferredRendering.vert",
            "deferredRendering.frag"
        };

        for (auto& filename : shaderFilenames)
        {
            std::string spirvFilename = std::string("bin/") + filename + std::string(".spv");
            if (!EngineCore::FileExists(spirvFilename))
            {
                std::string name = std::string("engine/platform/Vulkan/shaders/") + filename;
                VK_Shader shader{name, spirvFilename};
            }
        }
    }

}
