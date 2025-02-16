/* Engine Copyright (c) 2024 Engine Development Team
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
#include "VKcore.h"
#include "VKswapChain.h"
#include "VKinstanceBuffer.h"
#include "VKrenderPass.h"
#include "VKmodel.h"

#include "water/VKwater1Sys.h"

namespace GfxRenderEngine
{
    VK_RenderSystemWater1::VK_RenderSystemWater1(VkRenderPass renderPass,
                                                 std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        CreatePipelineLayout(descriptorSetLayouts);
        CreatePipeline(renderPass);
    }

    VK_RenderSystemWater1::~VK_RenderSystemWater1()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystemWater1::CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantWater1);

        // create desciptor set layout
        m_WaterTextureLayout =
            VK_DescriptorSetLayout::Builder()
                .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // duDvMap
                .Build();

        // create texture
        std::string path("resources/images/waterDUDV.png");
        m_WaterTexture = std::make_unique<VK_Texture>();
        m_WaterTexture->Init(path, Texture::USE_UNORM);

        // create descriptor set
        auto& imageInfo0 = m_WaterTexture->GetDescriptorImageInfo();
        VK_DescriptorWriter(*m_WaterTextureLayout).WriteImage(0, imageInfo0).Build(m_WaterTextureDescriptorSet);

        std::vector<VkDescriptorSetLayout> localDescriptorSetLayouts = descriptorSetLayouts;
        localDescriptorSetLayouts.push_back(m_WaterTextureLayout->GetDescriptorSetLayout());

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint>(localDescriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = localDescriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        auto result = vkCreatePipelineLayout(VK_Core::m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemWater1::CreatePipeline(VkRenderPass renderPass)
    {
        CORE_ASSERT(m_PipelineLayout != nullptr, "no pipeline layout");

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.m_BindingDescriptions.clear();
        pipelineConfig.m_AttributeDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        pipelineConfig.subpass = static_cast<uint>(VK_RenderPass::SubPasses3D::SUBPASS_TRANSPARENCY);

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/water1.vert.spv", "bin-int/water1.frag.spv",
                                                   pipelineConfig);
    }

    void VK_RenderSystemWater1::RenderEntities(const VK_FrameInfo& frameInfo, Registry& registry,
                                               VkDescriptorSet& refractionReflectionDescriptorSet)
    {
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        auto view = registry.Get().view<Water1Component, TransformComponent>();
        for (auto mainInstance : view)
        {
            auto& transform = view.get<TransformComponent>(mainInstance);
            auto& water1Component = view.get<Water1Component>(mainInstance);

            { // push constants
                auto& scale = water1Component.m_Scale;
                auto& translation = water1Component.m_Translation;
                glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);

                glm::mat4 modelMatrix = glm::scale( // Scale first
                    translationMatrix,              // Translate second
                    scale);
                static constexpr float STATIC_MOVE_FACTOR = 0.05f;
                m_MoveFactor += STATIC_MOVE_FACTOR * Engine::m_Engine->GetTimestep();
                VK_PushConstantWater1 pushConstantWater1 = {.m_ModelMatrix = transform.GetMat4Global() * modelMatrix,
                                                            .m_Values = glm::vec4(m_MoveFactor, 0.0f, 0.0f, 0.0f)};
                vkCmdPushConstants(frameInfo.m_CommandBuffer, m_PipelineLayout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   sizeof(VK_PushConstantWater1), &pushConstantWater1);
            }

            { // bind descriptor sets
                std::array<VkDescriptorSet, 3> descriptorSets = {
                    frameInfo.m_GlobalDescriptorSet, refractionReflectionDescriptorSet, m_WaterTextureDescriptorSet};
                vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer,       // VkCommandBuffer        commandBuffer,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS, // VkPipelineBindPoint    pipelineBindPoint,
                                        m_PipelineLayout,                // VkPipelineLayout       layout,
                                        0,                               // uint32_t               firstSet,
                                        descriptorSets.size(),           // uint32_t               descriptorSetCount,
                                        descriptorSets.data(),           // const VkDescriptorSet* pDescriptorSets,
                                        0,                               // uint32_t               dynamicOffsetCount,
                                        nullptr                          // const uint32_t*        pDynamicOffsets);
                );
            }

            {                                        // draw call
                vkCmdDraw(frameInfo.m_CommandBuffer, // VkCommandBuffer commandBuffer
                          VERTEX_COUNT_QUAD,         // uint32_t        vertexCount
                          1,                         // uint32_t        instanceCount
                          0,                         // uint32_t        firstVertex
                          0                          // uint32_t        firstInstance
                );
            }
        }
    }
} // namespace GfxRenderEngine
