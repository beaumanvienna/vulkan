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

#include "VKcore.h"
#include "VKswapChain.h"
#include "VKmodel.h"

#include "systems/VKpbrDiffuseSys.h"

namespace GfxRenderEngine
{
    VK_RenderSystemPbrDiffuse::VK_RenderSystemPbrDiffuse(VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        CreatePipelineLayout(descriptorSetLayouts);
        CreatePipeline(renderPass);
    }

    VK_RenderSystemPbrDiffuse::~VK_RenderSystemPbrDiffuse()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystemPbrDiffuse::CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataPbrDiffuse);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(VK_Core::m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemPbrDiffuse::CreatePipeline(VkRenderPass renderPass)
    {
        ASSERT(m_PipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin/pbrDiffuse.vert.spv",
            "bin/pbrDiffuse.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystemPbrDiffuse::RenderEntities(const VK_FrameInfo& frameInfo, entt::registry& registry)
    {
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        auto view = registry.view<MeshComponent, TransformComponent, PbrDiffuseComponent>();
        for (auto entity : view)
        {
            auto& pbrDiffuseComponent = view.get<PbrDiffuseComponent>(entity);
            VkDescriptorSet localDescriptorSet = pbrDiffuseComponent.m_DescriptorSet[frameInfo.m_FrameIndex];
            std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet, localDescriptorSet};
            vkCmdBindDescriptorSets
            (
                frameInfo.m_CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_PipelineLayout,
                0,
                2,
                descriptorSets.data(),
                0,
                nullptr
            );
            auto& transform = view.get<TransformComponent>(entity);
            VK_PushConstantDataPbrDiffuse push{};
            push.m_ModelMatrix  = transform.GetMat4();
            push.m_NormalMatrix = transform.GetNormalMatrix();
            push.m_NormalMatrix[3].x = pbrDiffuseComponent.m_Roughness;
            push.m_NormalMatrix[3].y = pbrDiffuseComponent.m_Metallic;
            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataPbrDiffuse),
                &push);

            auto& mesh = view.get<MeshComponent>(entity);
            if (mesh.m_Enabled)
            {
                static_cast<VK_Model*>(mesh.m_Model.get())->Bind(frameInfo.m_CommandBuffer);
                static_cast<VK_Model*>(mesh.m_Model.get())->Draw(frameInfo.m_CommandBuffer);
            }
        }
    }
}
