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

#include "core.h"
#include "engine.h"
#include "resources/resources.h"
#include "auxiliary/file.h"

#include "shadowMapping.h"
#include "VKrenderer.h"
#include "VKwindow.h"
#include "VKshader.h"

namespace GfxRenderEngine
{
    std::shared_ptr<Texture> gTextureSpritesheet;
    std::shared_ptr<Texture> gTextureFontAtlas;

    std::unique_ptr<VK_DescriptorPool> VK_Renderer::m_DescriptorPool;

    VK_Renderer::VK_Renderer(VK_Window* window)
        : m_Window{window}, m_FrameCounter{0},
          m_CurrentImageIndex{0}, m_AmbientLightIntensity{0.0f},
          m_CurrentFrameIndex{0}, m_ShowDebugShadowMap{false},
          m_FrameInProgress{false}, m_ShadersCompiled{false},
          m_Device{VK_Core::m_Device}
    {
        CompileShaders();  // runs in a parallel thread and sets m_ShadersCompiled
    }

    bool VK_Renderer::Init()
    {
        if (!m_ShadersCompiled) return m_ShadersCompiled;

        RecreateSwapChain();
        RecreateRenderpass();
        RecreateShadowMaps();
        CreateCommandBuffers();

        for (uint i = 0; i < m_ShadowUniformBuffers0.size(); i++)
        {
            m_ShadowUniformBuffers0[i] = std::make_unique<VK_Buffer>
            (
                *m_Device, sizeof(ShadowUniformBuffer),
                1,                                      // uint instanceCount
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                m_Device->m_Properties.limits.minUniformBufferOffsetAlignment
            );
            m_ShadowUniformBuffers0[i]->Map();
        }

        for (uint i = 0; i < m_ShadowUniformBuffers1.size(); i++)
        {
            m_ShadowUniformBuffers1[i] = std::make_unique<VK_Buffer>
            (
                *m_Device, sizeof(ShadowUniformBuffer),
                1,                                      // uint instanceCount
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                m_Device->m_Properties.limits.minUniformBufferOffsetAlignment
            );
            m_ShadowUniformBuffers1[i]->Map();
        }

        for (uint i = 0; i < m_UniformBuffers.size(); i++)
        {
            m_UniformBuffers[i] = std::make_unique<VK_Buffer>
            (
                *m_Device, sizeof(GlobalUniformBuffer),
                1,                                      // uint instanceCount
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                m_Device->m_Properties.limits.minUniformBufferOffsetAlignment
            );
            m_UniformBuffers[i]->Map();
        }

        // create a global pool for desciptor sets
        static constexpr uint POOL_SIZE = 10000;
        m_DescriptorPool = 
            VK_DescriptorPool::Builder()
            .SetMaxSets(VK_SwapChain::MAX_FRAMES_IN_FLIGHT * POOL_SIZE)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SwapChain::MAX_FRAMES_IN_FLIGHT * 50)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SwapChain::MAX_FRAMES_IN_FLIGHT * 7500)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SwapChain::MAX_FRAMES_IN_FLIGHT * 2450)
            .Build();

        std::unique_ptr<VK_DescriptorSetLayout> shadowUniformBufferDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        m_ShadowMapDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        std::unique_ptr<VK_DescriptorSetLayout> globalDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // spritesheet
                    .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // font atlas
                    .AddBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        std::unique_ptr<VK_DescriptorSetLayout> diffuseDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // color map
                    .Build();

        std::unique_ptr<VK_DescriptorSetLayout> emissiveDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // emissive map
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
                    .AddBinding(4, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT) // g buffer emissive input attachment
                    .Build();

        m_PostProcessingDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT) // color input attachment
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT) // g buffer emissive input attachment
                    .Build();

        std::unique_ptr<VK_DescriptorSetLayout> cubemapDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // cubemap
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

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsEmissiveTexture =
        {
            globalDescriptorSetLayout->GetDescriptorSetLayout(),
            emissiveDescriptorSetLayout->GetDescriptorSetLayout()
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
            m_LightingDescriptorSetLayout->GetDescriptorSetLayout(),
            m_ShadowMapDescriptorSetLayout->GetDescriptorSetLayout()
        };

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsPostProcessing =
        {
            globalDescriptorSetLayout->GetDescriptorSetLayout(),
            m_PostProcessingDescriptorSetLayout->GetDescriptorSetLayout(),
        };

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsCubemap =
        {
            globalDescriptorSetLayout->GetDescriptorSetLayout(),
            cubemapDescriptorSetLayout->GetDescriptorSetLayout()
        };

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsShadow =
        {
            shadowUniformBufferDescriptorSetLayout->GetDescriptorSetLayout()
        };

        std::vector<VkDescriptorSetLayout> descriptorSetLayoutsDebug =
        {
            m_ShadowMapDescriptorSetLayout->GetDescriptorSetLayout()
        };

        size_t fileSize;
        auto data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/atlas/atlas.png", IDB_ATLAS, "PNG");
        auto textureSpritesheet = std::make_shared<VK_Texture>(true);
        textureSpritesheet->Init(data, fileSize, Texture::USE_SRGB);
        textureSpritesheet->SetFilename("spritesheet");

        gTextureSpritesheet = textureSpritesheet; // copy from VK_Texture to Texture
        VkDescriptorImageInfo imageInfo0 = textureSpritesheet->GetDescriptorImageInfo();

        data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/atlas/fontAtlas.png", IDB_FONTS_RETRO, "PNG");
        auto textureFontAtlas = std::make_shared<VK_Texture>(true);
        textureFontAtlas->Init(data, fileSize, Texture::USE_SRGB);
        textureFontAtlas->SetFilename("font atlas");

        gTextureFontAtlas = textureFontAtlas; // copy from VK_Texture to Texture
        VkDescriptorImageInfo imageInfo1 = textureFontAtlas->GetDescriptorImageInfo();

        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo shadowUBObufferInfo = m_ShadowUniformBuffers0[i]->DescriptorInfo();
            VK_DescriptorWriter(*shadowUniformBufferDescriptorSetLayout, *m_DescriptorPool)
                .WriteBuffer(0, shadowUBObufferInfo)
                .Build(m_ShadowDescriptorSets0[i]);
        }

        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo shadowUBObufferInfo = m_ShadowUniformBuffers1[i]->DescriptorInfo();
            VK_DescriptorWriter(*shadowUniformBufferDescriptorSetLayout, *m_DescriptorPool)
                .WriteBuffer(0, shadowUBObufferInfo)
                .Build(m_ShadowDescriptorSets1[i]);
        }

        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo = m_UniformBuffers[i]->DescriptorInfo();
            VK_DescriptorWriter(*globalDescriptorSetLayout, *m_DescriptorPool)
                .WriteBuffer(0, bufferInfo)
                .WriteImage(1, imageInfo0)
                .WriteImage(2, imageInfo1)
                .Build(m_GlobalDescriptorSets[i]);
        }

        m_RenderSystemShadow                            = std::make_unique<VK_RenderSystemShadow>
                                                          (
                                                              m_ShadowMap[ShadowMaps::HIGH_RES]->GetShadowRenderPass(),
                                                              m_ShadowMap[ShadowMaps::LOW_RES]->GetShadowRenderPass(),
                                                              descriptorSetLayoutsShadow
                                                          );
        m_LightSystem                                   = std::make_unique<VK_LightSystem>(m_Device, m_RenderPass->Get3DRenderPass(), *globalDescriptorSetLayout);
        m_RenderSystemSpriteRenderer                    = std::make_unique<VK_RenderSystemSpriteRenderer>(m_RenderPass->Get3DRenderPass(), descriptorSetLayoutsDiffuse);
        m_RenderSystemSpriteRenderer2D                  = std::make_unique<VK_RenderSystemSpriteRenderer2D>(m_RenderPass->GetGUIRenderPass(), *globalDescriptorSetLayout);
        m_RenderSystemGUIRenderer                       = std::make_unique<VK_RenderSystemGUIRenderer>(m_RenderPass->GetGUIRenderPass(), *globalDescriptorSetLayout);
        m_RenderSystemCubemap                           = std::make_unique<VK_RenderSystemCubemap>(m_RenderPass->Get3DRenderPass(), descriptorSetLayoutsCubemap);

        m_RenderSystemPbrNoMap                          = std::make_unique<VK_RenderSystemPbrNoMap>(m_RenderPass->Get3DRenderPass(), *globalDescriptorSetLayout);
        m_RenderSystemPbrEmissive                       = std::make_unique<VK_RenderSystemPbrEmissive>(m_RenderPass->Get3DRenderPass(), *globalDescriptorSetLayout);
        m_RenderSystemPbrDiffuse                        = std::make_unique<VK_RenderSystemPbrDiffuse>(m_RenderPass->Get3DRenderPass(), descriptorSetLayoutsDiffuse);
        m_RenderSystemPbrDiffuseNormal                  = std::make_unique<VK_RenderSystemPbrDiffuseNormal>(m_RenderPass->Get3DRenderPass(), descriptorSetLayoutsDiffuseNormal);
        m_RenderSystemPbrEmissiveTexture                = std::make_unique<VK_RenderSystemPbrEmissiveTexture>(m_RenderPass->Get3DRenderPass(), descriptorSetLayoutsEmissiveTexture);
        m_RenderSystemPbrDiffuseNormalRoughnessMetallic = std::make_unique<VK_RenderSystemPbrDiffuseNormalRoughnessMetallic>(m_RenderPass->Get3DRenderPass(), descriptorSetLayoutsDiffuseNormalRoughnessMetallic);

        CreateShadowMapDescriptorSets();
        CreateLightingDescriptorSets();
        CreatePostProcessingDescriptorSets();

        m_RenderSystemDeferredShading                 = std::make_unique<VK_RenderSystemDeferredShading>
        (
            m_RenderPass->Get3DRenderPass(),
            descriptorSetLayoutsLighting,
            m_LightingDescriptorSets.data(),
            m_ShadowMapDescriptorSets.data()
        );

        m_RenderSystemPostProcessing                 = std::make_unique<VK_RenderSystemPostProcessing>
        (
            m_RenderPass->GetPostProcessingRenderPass(),
            descriptorSetLayoutsPostProcessing,
            m_PostProcessingDescriptorSets.data()
        );

        m_RenderSystemDebug                             = std::make_unique<VK_RenderSystemDebug>
        (
            m_RenderPass->Get3DRenderPass(),
            descriptorSetLayoutsDebug,
            m_ShadowMapDescriptorSets.data()
        );

        m_Imgui = Imgui::Create(m_RenderPass->GetGUIRenderPass(), static_cast<uint>(m_SwapChain->ImageCount()));
        return m_ShadersCompiled;
    }

    void VK_Renderer::CreateShadowMapDescriptorSets()
    {
        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorImageInfo shadowMapInfo0 = m_ShadowMap[ShadowMaps::HIGH_RES]->GetDescriptorImageInfo();
            VkDescriptorImageInfo shadowMapInfo1 = m_ShadowMap[ShadowMaps::LOW_RES]->GetDescriptorImageInfo();

            VkDescriptorBufferInfo shadowUBObufferInfo0 = m_ShadowUniformBuffers0[i]->DescriptorInfo();
            VkDescriptorBufferInfo shadowUBObufferInfo1 = m_ShadowUniformBuffers1[i]->DescriptorInfo();

            VK_DescriptorWriter(*m_ShadowMapDescriptorSetLayout, *m_DescriptorPool)
                .WriteImage(0, shadowMapInfo0)
                .WriteImage(1, shadowMapInfo1)
                .WriteBuffer(2, shadowUBObufferInfo0)
                .WriteBuffer(3, shadowUBObufferInfo1)
                .Build(m_ShadowMapDescriptorSets[i]);
        }
    }

    void VK_Renderer::CreateLightingDescriptorSets()
    {
        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorImageInfo imageInfoGBufferPositionInputAttachment {};
            imageInfoGBufferPositionInputAttachment.imageView   = m_RenderPass->GetImageViewGBufferPosition();
            imageInfoGBufferPositionInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo imageInfoGBufferNormalInputAttachment {};
            imageInfoGBufferNormalInputAttachment.imageView   = m_RenderPass->GetImageViewGBufferNormal();
            imageInfoGBufferNormalInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo imageInfoGBufferColorInputAttachment {};
            imageInfoGBufferColorInputAttachment.imageView   = m_RenderPass->GetImageViewGBufferColor();
            imageInfoGBufferColorInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo imageInfoGBufferMaterialInputAttachment {};
            imageInfoGBufferMaterialInputAttachment.imageView   = m_RenderPass->GetImageViewGBufferMaterial();
            imageInfoGBufferMaterialInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo imageInfoGBufferEmissionInputAttachment {};
            imageInfoGBufferEmissionInputAttachment.imageView   = m_RenderPass->GetImageViewGBufferEmission();
            imageInfoGBufferEmissionInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VK_DescriptorWriter(*m_LightingDescriptorSetLayout, *m_DescriptorPool)
                .WriteImage(0, imageInfoGBufferPositionInputAttachment)
                .WriteImage(1, imageInfoGBufferNormalInputAttachment)
                .WriteImage(2, imageInfoGBufferColorInputAttachment)
                .WriteImage(3, imageInfoGBufferMaterialInputAttachment)
                .WriteImage(4, imageInfoGBufferEmissionInputAttachment)
                .Build(m_LightingDescriptorSets[i]);
        }
    }

    void VK_Renderer::CreatePostProcessingDescriptorSets()
    {
        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorImageInfo imageInfoColorInputAttachment {};
            imageInfoColorInputAttachment.imageView   = m_RenderPass->GetImageViewColorAttachment();
            imageInfoColorInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkDescriptorImageInfo imageInfoGBufferEmissionInputAttachment {};
            imageInfoGBufferEmissionInputAttachment.imageView   = m_RenderPass->GetImageViewGBufferEmission();
            imageInfoGBufferEmissionInputAttachment.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VK_DescriptorWriter(*m_PostProcessingDescriptorSetLayout, *m_DescriptorPool)
                .WriteImage(0, imageInfoColorInputAttachment)
                .WriteImage(1, imageInfoGBufferEmissionInputAttachment)
                .Build(m_PostProcessingDescriptorSets[i]);
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
            m_SwapChain = std::make_unique<VK_SwapChain>(extent);
        }
        else
        {
            LOG_CORE_INFO("recreating swapchain at frame {0}", m_FrameCounter);
            std::shared_ptr<VK_SwapChain> oldSwapChain = std::move(m_SwapChain);
            m_SwapChain = std::make_unique<VK_SwapChain>(extent, oldSwapChain);
            if (!oldSwapChain->CompareSwapFormats(*m_SwapChain.get()))
            {
                LOG_CORE_CRITICAL("swap chain image or depth format has changed");
            }
        }
    }

    void VK_Renderer::RecreateRenderpass()
    {
        auto extent = m_Window->GetExtend();
        m_RenderPass = std::make_unique<VK_RenderPass>(extent, m_SwapChain.get());
    }

    void VK_Renderer::RecreateShadowMaps()
    {
        // create shadow maps
        m_ShadowMap[ShadowMaps::HIGH_RES] = std::make_unique<VK_ShadowMap>(SHADOW_MAP_HIGH_RES);
        m_ShadowMap[ShadowMaps::LOW_RES]  = std::make_unique<VK_ShadowMap>(SHADOW_MAP_LOW_RES);
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
            RecreateRenderpass();
            CreateLightingDescriptorSets();
            CreatePostProcessingDescriptorSets();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LOG_CORE_CRITICAL("failed to acquire next swap chain image");
        }

        m_FrameInProgress = true;
        m_FrameCounter++;

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
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            m_Window->ResetWindowResizedFlag();
            RecreateSwapChain();
            RecreateRenderpass();
            CreateLightingDescriptorSets();
            CreatePostProcessingDescriptorSets();
        }
        else if (result != VK_SUCCESS)
        {
            LOG_CORE_WARN("failed to present swap chain image");
        }
        m_FrameInProgress = false;
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % VK_SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void VK_Renderer::BeginShadowRenderPass0(VkCommandBuffer commandBuffer)
    {
        ASSERT(m_FrameInProgress);
        ASSERT(commandBuffer == GetCurrentCommandBuffer());

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_ShadowMap[ShadowMaps::HIGH_RES]->GetShadowRenderPass();
        renderPassInfo.framebuffer = m_ShadowMap[ShadowMaps::HIGH_RES]->GetShadowFrameBuffer();

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_ShadowMap[ShadowMaps::HIGH_RES]->GetShadowMapExtent();

        std::array<VkClearValue, static_cast<uint>(VK_ShadowMap::ShadowRenderTargets::NUMBER_OF_ATTACHMENTS)> clearValues{};
        clearValues[0].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_ShadowMap[ShadowMaps::HIGH_RES]->GetShadowMapExtent().width);
        viewport.height = static_cast<float>(m_ShadowMap[ShadowMaps::HIGH_RES]->GetShadowMapExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_ShadowMap[ShadowMaps::HIGH_RES]->GetShadowMapExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    }

    void VK_Renderer::BeginShadowRenderPass1(VkCommandBuffer commandBuffer)
    {
        ASSERT(m_FrameInProgress);
        ASSERT(commandBuffer == GetCurrentCommandBuffer());

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_ShadowMap[ShadowMaps::LOW_RES]->GetShadowRenderPass();
        renderPassInfo.framebuffer = m_ShadowMap[ShadowMaps::LOW_RES]->GetShadowFrameBuffer();

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_ShadowMap[ShadowMaps::LOW_RES]->GetShadowMapExtent();

        std::array<VkClearValue, static_cast<uint>(VK_ShadowMap::ShadowRenderTargets::NUMBER_OF_ATTACHMENTS)> clearValues{};
        clearValues[0].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_ShadowMap[ShadowMaps::LOW_RES]->GetShadowMapExtent().width);
        viewport.height = static_cast<float>(m_ShadowMap[ShadowMaps::LOW_RES]->GetShadowMapExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_ShadowMap[ShadowMaps::LOW_RES]->GetShadowMapExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void VK_Renderer::SubmitShadows(entt::registry& registry, const std::vector<DirectionalLightComponent*>& directionalLights)
    {
        // this function supports one directional light 
        // with a high-resolution and
        // with a low-resolution component
        // --> either both or none must be provided
        if (directionalLights.size() == 2)
        {
            {
                ShadowUniformBuffer ubo{};
                ubo.m_Projection = directionalLights[0]->m_LightView->GetProjectionMatrix();
                ubo.m_View = directionalLights[0]->m_LightView->GetViewMatrix();
                m_ShadowUniformBuffers0[m_CurrentFrameIndex]->WriteToBuffer(&ubo);
                m_ShadowUniformBuffers0[m_CurrentFrameIndex]->Flush();
            }
            {
                ShadowUniformBuffer ubo{};
                ubo.m_Projection = directionalLights[1]->m_LightView->GetProjectionMatrix();
                ubo.m_View = directionalLights[1]->m_LightView->GetViewMatrix();
                m_ShadowUniformBuffers1[m_CurrentFrameIndex]->WriteToBuffer(&ubo);
                m_ShadowUniformBuffers1[m_CurrentFrameIndex]->Flush();
            }

            BeginShadowRenderPass0(m_CurrentCommandBuffer);
            m_RenderSystemShadow->RenderEntities
            (
                m_FrameInfo,
                registry,
                directionalLights[0],
                0 /* shadow pass 0*/,
                m_ShadowDescriptorSets0[m_CurrentFrameIndex]
            );
            EndRenderPass(m_CurrentCommandBuffer);

            BeginShadowRenderPass1(m_CurrentCommandBuffer);
            m_RenderSystemShadow->RenderEntities
            (
                m_FrameInfo,
                registry,
                directionalLights[1],
                1 /* shadow pass 1*/,
                m_ShadowDescriptorSets1[m_CurrentFrameIndex]
            );
            EndRenderPass(m_CurrentCommandBuffer);
        }
        else
        {
            // we still have to clear the shadow map depth buffers
            // because the lighting shader expects values
            BeginShadowRenderPass0(m_CurrentCommandBuffer);
            EndRenderPass(m_CurrentCommandBuffer);
            BeginShadowRenderPass1(m_CurrentCommandBuffer);
            EndRenderPass(m_CurrentCommandBuffer);
        }
    }

    void VK_Renderer::Begin3DRenderPass(VkCommandBuffer commandBuffer)
    {
        ASSERT(m_FrameInProgress);
        ASSERT(commandBuffer == GetCurrentCommandBuffer());

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass->Get3DRenderPass();
        renderPassInfo.framebuffer = m_RenderPass->Get3DFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

        std::array<VkClearValue, static_cast<uint>(VK_RenderPass::RenderTargets3D::NUMBER_OF_ATTACHMENTS)> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        clearValues[2].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[3].color = {0.5f, 0.5f, 0.1f, 1.0f};
        clearValues[4].color = {0.5f, 0.1f, 0.5f, 1.0f};
        clearValues[5].color = {0.5f, 0.7f, 0.2f, 1.0f};
        clearValues[6].color = {0.0f, 0.0f, 0.0f, 1.0f};
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

    void VK_Renderer::BeginPostProcessingRenderPass(VkCommandBuffer commandBuffer)
    {
        ASSERT(m_FrameInProgress);
        ASSERT(commandBuffer == GetCurrentCommandBuffer());

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass->GetPostProcessingRenderPass();
        renderPassInfo.framebuffer = m_RenderPass->GetPostProcessingFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

        std::array<VkClearValue, static_cast<uint>(VK_RenderPass::RenderTargetsPostProcessing::NUMBER_OF_ATTACHMENTS)> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
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

    void VK_Renderer::BeginGUIRenderPass(VkCommandBuffer commandBuffer)
    {
        ASSERT(m_FrameInProgress);
        ASSERT(commandBuffer == GetCurrentCommandBuffer());

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass->GetGUIRenderPass();
        renderPassInfo.framebuffer = m_RenderPass->GetGUIFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();
        renderPassInfo.clearValueCount = 0;
        renderPassInfo.pClearValues = nullptr;

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

    void VK_Renderer::EndRenderPass(VkCommandBuffer commandBuffer)
    {
        ASSERT(m_FrameInProgress);
        ASSERT(commandBuffer == GetCurrentCommandBuffer());

        vkCmdEndRenderPass(commandBuffer);
    }

    void VK_Renderer::BeginFrame(Camera* camera)
    {
        if (m_CurrentCommandBuffer = BeginFrame())
        {
            m_FrameInfo =
            {
                m_CurrentFrameIndex,
                m_CurrentImageIndex,
                0.0f, /* m_FrameTime */
                m_CurrentCommandBuffer, 
                camera,
                m_GlobalDescriptorSets[m_CurrentFrameIndex]
            };
        }
    }

    void VK_Renderer::Renderpass3D(entt::registry& registry)
    {
        if (m_CurrentCommandBuffer)
        {
            GlobalUniformBuffer ubo{};
            ubo.m_Projection = m_FrameInfo.m_Camera->GetProjectionMatrix();
            ubo.m_View = m_FrameInfo.m_Camera->GetViewMatrix();
            ubo.m_AmbientLightColor = {1.0f, 1.0f, 1.0f, m_AmbientLightIntensity};
            m_LightSystem->Update(m_FrameInfo, ubo, registry);
            m_UniformBuffers[m_CurrentFrameIndex]->WriteToBuffer(&ubo);
            m_UniformBuffers[m_CurrentFrameIndex]->Flush();

            Begin3DRenderPass(m_CurrentCommandBuffer);
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

            // 3D objects
            m_RenderSystemPbrNoMap->RenderEntities(m_FrameInfo, registry);
            m_RenderSystemPbrDiffuse->RenderEntities(m_FrameInfo, registry);
            m_RenderSystemPbrDiffuseNormal->RenderEntities(m_FrameInfo, registry);
            m_RenderSystemPbrDiffuseNormalRoughnessMetallic->RenderEntities(m_FrameInfo, registry);

            // the emissive pipelines need to go last
            // their do not write to the depth buffer
            m_RenderSystemPbrEmissive->RenderEntities(m_FrameInfo, registry);
            m_RenderSystemPbrEmissiveTexture->RenderEntities(m_FrameInfo, registry);
        }
    }

    void VK_Renderer::LightingPass()
    {
        if (m_CurrentCommandBuffer)
        {
            m_RenderSystemDeferredShading->LightingPass(m_FrameInfo);
        }
    }

    void VK_Renderer::TransparencyPass(entt::registry& registry, ParticleSystem* particleSystem)
    {
        if (m_CurrentCommandBuffer)
        {
            // sprites
            m_RenderSystemCubemap->RenderEntities(m_FrameInfo, registry);
            m_RenderSystemSpriteRenderer->RenderEntities(m_FrameInfo, registry);
            if (particleSystem) m_RenderSystemSpriteRenderer->DrawParticles(m_FrameInfo, particleSystem);
            m_LightSystem->Render(m_FrameInfo, registry);
            m_RenderSystemDebug->RenderEntities(m_FrameInfo, m_ShowDebugShadowMap);
        }
    }

    void VK_Renderer::PostProcessingRenderpass()
    {
        if (m_CurrentCommandBuffer)
        {
            EndRenderPass(m_CurrentCommandBuffer); // end 3D renderpass
            BeginPostProcessingRenderPass(m_CurrentCommandBuffer);
            m_RenderSystemPostProcessing->PostProcessingPass(m_FrameInfo);
        }
    }

    void VK_Renderer::NextSubpass()
    {
        if (m_CurrentCommandBuffer)
        {
            vkCmdNextSubpass(m_CurrentCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
        }
    }

    void VK_Renderer::GUIRenderpass(Camera* camera)
    {
        if (m_CurrentCommandBuffer)
        {
            EndRenderPass(m_CurrentCommandBuffer); // end post processing renderpass
            BeginGUIRenderPass(m_CurrentCommandBuffer);

            // set up orthogonal camera
            m_GUIViewProjectionMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix();
        }
    }

    void VK_Renderer::Submit2D(Camera* camera, entt::registry& registry)
    {
        if (m_CurrentCommandBuffer)
        {
            m_RenderSystemSpriteRenderer2D->RenderEntities(m_FrameInfo, registry, camera);
        }
    }

    void VK_Renderer::EndScene()
    {
        if (m_CurrentCommandBuffer)
        {
            // built-in editor GUI runs last
            m_Imgui->NewFrame();
            m_Imgui->Run();
            m_Imgui->Render(m_CurrentCommandBuffer);

            EndRenderPass(m_CurrentCommandBuffer); // end GUI render pass
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
        
        std::thread shaderCompileThread([this]()
        {
            if (!EngineCore::FileExists("bin-int"))
            {
                LOG_CORE_WARN("creating bin directory for spirv files");
                EngineCore::CreateDirectory("bin-int");
            }
            std::vector<std::string> shaderFilenames = 
            {
                // 2D
                "spriteRenderer.vert",
                "spriteRenderer.frag",
                "spriteRenderer2D.frag",
                "spriteRenderer2D.vert",
                "guiShader.frag",
                "guiShader.vert",
                "guiShader2.frag",
                "guiShader2.vert",
                // 3D
                "pointLight.vert",
                "pointLight.frag",
                "pbrNoMap.vert",
                "pbrNoMap.frag",
                "pbrDiffuse.vert",
                "pbrDiffuse.frag",
                "pbrDiffuseNormal.vert",
                "pbrDiffuseNormal.frag",
                "pbrDiffuseNormalRoughnessMetallic.vert",
                "pbrDiffuseNormalRoughnessMetallic.frag",
                "deferredShading.vert",
                "deferredShading.frag",
                "skybox.vert",
                "skybox.frag",
                "shadowShader.vert",
                "shadowShader.frag",
                "debug.vert",
                "debug.frag",
                "pbrEmissive.vert",
                "pbrEmissive.frag",
                "pbrEmissiveTexture.vert",
                "pbrEmissiveTexture.frag",
                "postprocessing.vert",
                "postprocessing.frag"
            };
    
            for (auto& filename : shaderFilenames)
            {
                std::string spirvFilename = std::string("bin-int/") + filename + std::string(".spv");
                if (!EngineCore::FileExists(spirvFilename))
                {
                    std::string name = std::string("engine/platform/Vulkan/shaders/") + filename;
                    VK_Shader shader{name, spirvFilename};
                }
            }
            m_ShadersCompiled = true;
        });
        shaderCompileThread.detach();
    }

    void VK_Renderer::DrawWithTransform(const Sprite& sprite, const glm::mat4& transform)
    {
        m_RenderSystemGUIRenderer->RenderSprite(m_FrameInfo, sprite, m_GUIViewProjectionMatrix * transform);
    }

    void VK_Renderer::Draw(const Sprite& sprite, const glm::mat4& position, const glm::vec4& color, const float textureID)
    {
        m_RenderSystemGUIRenderer->RenderSprite(m_FrameInfo, sprite, position, color, textureID);
    }
}
