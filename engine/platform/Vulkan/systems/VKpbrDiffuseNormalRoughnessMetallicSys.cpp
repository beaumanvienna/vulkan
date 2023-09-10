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

#include "systems/VKpbrDiffuseNormalRoughnessMetallicSys.h"

namespace GfxRenderEngine
{
    VK_RenderSystemPbrDiffuseNormalRoughnessMetallic::VK_RenderSystemPbrDiffuseNormalRoughnessMetallic(VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        CreatePipelineLayout(descriptorSetLayouts);
        CreatePipeline(renderPass);
    }

    VK_RenderSystemPbrDiffuseNormalRoughnessMetallic::~VK_RenderSystemPbrDiffuseNormalRoughnessMetallic()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystemPbrDiffuseNormalRoughnessMetallic::CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataPbrDiffuseNormalRoughnessMetallic);

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

    void VK_RenderSystemPbrDiffuseNormalRoughnessMetallic::CreatePipeline(VkRenderPass renderPass)
    {
        ASSERT(m_PipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        pipelineConfig.subpass = static_cast<uint>(VK_RenderPass::SubPasses::SUBPASS_GEOMETRY);

        // g buffer position, g buffer normal, g buffer color, g buffer material
        // no blending
        auto attachmentCount = (int)VK_RenderPass::NUMBER_OF_GBUFFER_ATTACHMENTS;
        pipelineConfig.colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState blAttachments[] =
        {
            pipelineConfig.colorBlendAttachment,
            pipelineConfig.colorBlendAttachment,
            pipelineConfig.colorBlendAttachment,
            pipelineConfig.colorBlendAttachment
        };
        VK_Pipeline::SetColorBlendState(pipelineConfig, attachmentCount, blAttachments);

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin-int/pbrDiffuseNormalRoughnessMetallic.vert.spv",
            "bin-int/pbrDiffuseNormalRoughnessMetallic.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystemPbrDiffuseNormalRoughnessMetallic::RenderEntities(const VK_FrameInfo& frameInfo, entt::registry& registry)
    {
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        auto view = registry.view<MeshComponent, TransformComponent, PbrDiffuseNormalRoughnessMetallicTag>();
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            auto& mesh = view.get<MeshComponent>(entity);
            if (mesh.m_Enabled)
            {
                static_cast<VK_Model*>(mesh.m_Model.get())->Bind(frameInfo.m_CommandBuffer);
                static_cast<VK_Model*>(mesh.m_Model.get())->DrawDiffuseNormalRoughnessMetallicMap(frameInfo, transform, m_PipelineLayout);
            }
        }
    }
}
