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

#include "VKcore.h"
#include "VKswapChain.h"
#include "VKrenderPass.h"
#include "VKmodel.h"

#include "systems/VKspriteRenderSys.h"
#include "systems/pushConstantData.h"

namespace GfxRenderEngine
{
    VK_RenderSystemSpriteRenderer::VK_RenderSystemSpriteRenderer(VkRenderPass renderPass,
                                                                 std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        CreatePipelineLayout(descriptorSetLayouts);
        CreatePipeline(renderPass);
    }

    VK_RenderSystemSpriteRenderer::~VK_RenderSystemSpriteRenderer()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystemSpriteRenderer::CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataGeneric);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        auto result = vkCreatePipelineLayout(VK_Core::m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemSpriteRenderer::CreatePipeline(VkRenderPass renderPass)
    {
        CORE_ASSERT(m_PipelineLayout != nullptr, "pipeline layout is null");

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        pipelineConfig.subpass = static_cast<uint>(VK_RenderPass::SubPasses3D::SUBPASS_TRANSPARENCY);

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/spriteRenderer.vert.spv",
                                                   "bin-int/spriteRenderer.frag.spv", pipelineConfig);
    }

    void VK_RenderSystemSpriteRenderer::RenderEntities(const VK_FrameInfo& frameInfo, Registry& registry)
    {
        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1,
                                &frameInfo.m_GlobalDescriptorSet, 0, nullptr);
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        auto view = registry.view<MeshComponent, TransformComponent, SpriteRendererComponent>();
        for (auto entity : view)
        {
            auto& spriteRendererComponent = view.get<SpriteRendererComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);
            VK_PushConstantDataGeneric push{};
            push.m_ModelMatrix = transform.GetMat4Local();
            push.m_NormalMatrix = transform.GetNormalMatrix();
            push.m_NormalMatrix[3].x = spriteRendererComponent.m_Roughness;
            push.m_NormalMatrix[3].y = spriteRendererComponent.m_Metallic;
            vkCmdPushConstants(frameInfo.m_CommandBuffer, m_PipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(VK_PushConstantDataGeneric), &push);

            auto& mesh = view.get<MeshComponent>(entity);
            if (mesh.m_Enabled)
            {
                static_cast<VK_Model*>(mesh.m_Model.get())->Bind(frameInfo.m_CommandBuffer);
                static_cast<VK_Model*>(mesh.m_Model.get())->Draw(frameInfo.m_CommandBuffer);
            }
        }
    }

    void VK_RenderSystemSpriteRenderer::DrawParticles(const VK_FrameInfo& frameInfo, ParticleSystem* particleSystem)
    {
        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1,
                                &frameInfo.m_GlobalDescriptorSet, 0, nullptr);

        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        for (auto& particle : particleSystem->m_ParticlePool)
        {
            if (!particle.m_Enabled)
            {
                continue;
            }

            auto& transform = particleSystem->m_Registry.get<TransformComponent>(particle.m_Entity);
            VK_PushConstantDataGeneric push{};
            push.m_ModelMatrix = transform.GetMat4Local();
            push.m_NormalMatrix = transform.GetNormalMatrix();
            vkCmdPushConstants(frameInfo.m_CommandBuffer, m_PipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(VK_PushConstantDataGeneric), &push);

            auto& mesh = particleSystem->m_Registry.get<MeshComponent>(particle.m_SpriteEntity);
            static_cast<VK_Model*>(mesh.m_Model.get())->Bind(frameInfo.m_CommandBuffer);
            static_cast<VK_Model*>(mesh.m_Model.get())->Draw(frameInfo.m_CommandBuffer);
        }
    }
} // namespace GfxRenderEngine
