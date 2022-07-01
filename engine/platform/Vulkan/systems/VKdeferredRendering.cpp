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

#include "systems/VKdeferredRendering.h"
#include "VKswapChain.h"

namespace GfxRenderEngine
{
    VK_RenderSystemDeferredRendering::VK_RenderSystemDeferredRendering
    (
        VkRenderPass renderPass,
        std::vector<VkDescriptorSetLayout>& geometryDescriptorSetLayouts,
        std::vector<VkDescriptorSetLayout>& ligthingDescriptorSetLayouts,
        const VkDescriptorSet* lightingDescriptorSet)
    {
        CreateGeometryPipelineLayout(geometryDescriptorSetLayouts);
        CreateLightingPipelineLayout(ligthingDescriptorSetLayouts);
        m_LightingDescriptorSets = lightingDescriptorSet;
        CreateGeometryPipeline(renderPass);
        CreateLightingPipeline(renderPass);
    }

    VK_RenderSystemDeferredRendering::~VK_RenderSystemDeferredRendering()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_GeometryPipelineLayout, nullptr);
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_LightingPipelineLayout, nullptr);
    }

    void VK_RenderSystemDeferredRendering::CreateGeometryPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataDeferredRendering);

        VkPipelineLayoutCreateInfo geometryPipelineLayoutInfo{};
        geometryPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        geometryPipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        geometryPipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        geometryPipelineLayoutInfo.pushConstantRangeCount = 1;
        geometryPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(VK_Core::m_Device->Device(), &geometryPipelineLayoutInfo, nullptr, &m_GeometryPipelineLayout) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemDeferredRendering::CreateLightingPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {

        VkPipelineLayoutCreateInfo lightingPipelineLayoutInfo{};
        lightingPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        lightingPipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        lightingPipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        lightingPipelineLayoutInfo.pushConstantRangeCount = 0;
        lightingPipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(VK_Core::m_Device->Device(), &lightingPipelineLayoutInfo, nullptr, &m_LightingPipelineLayout) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemDeferredRendering::CreateGeometryPipeline(VkRenderPass renderPass)
    {
        ASSERT(m_GeometryPipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_GeometryPipelineLayout;
        pipelineConfig.subpass = VK_SwapChain::SUBPASS_GEOMETRY;
        pipelineConfig.colorBlendAttachment.blendEnable = VK_FALSE;

        int attachmentCount = 3; // g buffer position, g buffer normal, g buffer color
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
        blendAttachments.resize(attachmentCount);
        for(int i=0; i<attachmentCount; i++)
        {
            blendAttachments[i] = pipelineConfig.colorBlendAttachment;
        }

        VK_Pipeline::SetColorBlendState(pipelineConfig, attachmentCount, blendAttachments.data());

        // create a pipeline
        m_GeometryPipeline = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin/gBuffer.vert.spv",
            "bin/gBuffer.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystemDeferredRendering::CreateLightingPipeline(VkRenderPass renderPass)
    {
        ASSERT(m_LightingPipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_LightingPipelineLayout;
        pipelineConfig.subpass = VK_SwapChain::SUBPASS_LIGHTING;

        // create a pipeline
        m_LightingPipeline = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin/deferredRendering.vert.spv",
            "bin/deferredRendering.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystemDeferredRendering::RenderEntities(const VK_FrameInfo& frameInfo, entt::registry& registry)
    {
        m_GeometryPipeline->Bind(frameInfo.m_CommandBuffer);

        auto view = registry.view<MeshComponent, TransformComponent, PbrDiffuseNormalRoughnessMetallicTag>();
        for (auto entity : view)
        {

            auto& transform = view.get<TransformComponent>(entity);
            auto& mesh = view.get<MeshComponent>(entity);
            if (mesh.m_Enabled)
            {
                static_cast<VK_Model*>(mesh.m_Model.get())->Bind(frameInfo.m_CommandBuffer);
                static_cast<VK_Model*>(mesh.m_Model.get())->DrawDiffuseNormalRoughnessMetallicMap(frameInfo, transform, m_GeometryPipelineLayout);
            }
        }
    }

    void VK_RenderSystemDeferredRendering::LightingPass(const VK_FrameInfo& frameInfo)
    {
        m_LightingPipeline->Bind(frameInfo.m_CommandBuffer);

        vkCmdBindDescriptorSets
        (
            frameInfo.m_CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_LightingPipelineLayout,                           // VkPipelineLayout layout
            0,                                                  // uint32_t         firstSet
            1,                                                  // uint32_t         descriptorSetCount
            &m_LightingDescriptorSets[frameInfo.m_FrameIndex],    // VkDescriptorSet* pDescriptorSets
            0,                                                  // uint32_t         dynamicOffsetCount
            nullptr                                             // const uint32_t*  pDynamicOffsets
        );

        vkCmdDraw
        (
            frameInfo.m_CommandBuffer,
            3,      // vertexCount
            1,      // instanceCount
            0,      // firstVertex
            0       // firstInstance
        );
    }
}
