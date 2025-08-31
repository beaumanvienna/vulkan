/* Engine Copyright (c) 2023 Engine Development Team
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

#include "systems/VKdeferredShading.h"

#include "VKswapChain.h"

namespace GfxRenderEngine
{
    VK_RenderSystemDeferredShading::VK_RenderSystemDeferredShading(
        VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& lightingDescriptorSetLayouts,
        const VkDescriptorSet* lightingDescriptorSet, const VkDescriptorSet* shadowMapDescriptorSet)
    {
        CreateLightingPipelineLayout(lightingDescriptorSetLayouts);
        m_LightingDescriptorSets = lightingDescriptorSet;
        m_ShadowMapDescriptorSets = shadowMapDescriptorSet;
        CreateLightingPipeline(renderPass);

        // IBL pipeline
        CreateLightingPipelineLayoutIBL(lightingDescriptorSetLayouts);
        CreateLightingPipelineIBL(renderPass);
    }

    VK_RenderSystemDeferredShading::~VK_RenderSystemDeferredShading()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_LightingPipelineLayout, nullptr);
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_LightingPipelineLayoutIBL, nullptr);
    }

    void
    VK_RenderSystemDeferredShading::CreateLightingPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPipelineLayoutCreateInfo lightingPipelineLayoutInfo{};
        lightingPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        lightingPipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        lightingPipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        lightingPipelineLayoutInfo.pushConstantRangeCount = 0;
        auto result = vkCreatePipelineLayout(VK_Core::m_Device->Device(), &lightingPipelineLayoutInfo, nullptr,
                                             &m_LightingPipelineLayout);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemDeferredShading::CreateLightingPipeline(VkRenderPass renderPass)
    {
        CORE_ASSERT(m_LightingPipelineLayout != nullptr, "pipeline layout is null");

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_LightingPipelineLayout;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.subpass = static_cast<uint>(VK_RenderPass::SubPasses3D::SUBPASS_LIGHTING);
        pipelineConfig.m_BindingDescriptions.clear(); // this pipeline is not using vertices
        pipelineConfig.m_AttributeDescriptions.clear();

        // create a pipeline
        m_LightingPipeline = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/deferredShading.vert.spv",
                                                           "bin-int/deferredShading.frag.spv", pipelineConfig);
    }

    void VK_RenderSystemDeferredShading::LightingPass(const VK_FrameInfo& frameInfo, VkDescriptorSet* lightingDescriptorSet)
    {
        m_LightingPipeline->Bind(frameInfo.m_CommandBuffer);

        auto descriptorSets = std::to_array<VkDescriptorSet>(
            {frameInfo.m_GlobalDescriptorSet,
             lightingDescriptorSet ? *lightingDescriptorSet : m_LightingDescriptorSets[frameInfo.m_FrameIndex],
             m_ShadowMapDescriptorSets[frameInfo.m_FrameIndex]});

        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_LightingPipelineLayout, // VkPipelineLayout layout
                                0,                        // uint32_t         firstSet
                                descriptorSets.size(),    // uint32_t         descriptorSetCount
                                descriptorSets.data(),    // VkDescriptorSet* pDescriptorSets
                                0,                        // uint32_t         dynamicOffsetCount
                                nullptr                   // const uint32_t*  pDynamicOffsets
        );

        vkCmdDraw(frameInfo.m_CommandBuffer,
                  3, // vertexCount
                  1, // instanceCount
                  0, // firstVertex
                  0  // firstInstance
        );
    }

    // IBL
    void
    VK_RenderSystemDeferredShading::CreateLightingPipelineLayoutIBL(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantsIBL);

        VkPipelineLayoutCreateInfo lightingPipelineLayoutInfo{};
        lightingPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        lightingPipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        lightingPipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        lightingPipelineLayoutInfo.pushConstantRangeCount = 1;
        lightingPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        auto result = vkCreatePipelineLayout(VK_Core::m_Device->Device(), &lightingPipelineLayoutInfo, nullptr,
                                             &m_LightingPipelineLayoutIBL);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create IBL pipeline layout!");
        }
    }

    void VK_RenderSystemDeferredShading::CreateLightingPipelineIBL(VkRenderPass renderPass)
    {
        CORE_ASSERT(m_LightingPipelineLayoutIBL != nullptr, "IBL pipeline layout is null");

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_LightingPipelineLayoutIBL;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.subpass = static_cast<uint>(VK_RenderPass::SubPasses3D::SUBPASS_LIGHTING);
        pipelineConfig.m_BindingDescriptions.clear(); // this pipeline is not using vertices
        pipelineConfig.m_AttributeDescriptions.clear();

        // create a pipeline
        m_LightingPipelineIBL = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/deferredShading.vert.spv",
                                                              "bin-int/deferredShadingIBL.frag.spv", pipelineConfig);
    }

    void VK_RenderSystemDeferredShading::LightingPassIBL(const VK_FrameInfo& frameInfo, float uMaxPrefilterMip,
                                                         std::shared_ptr<ResourceDescriptor> const& resourceDescriptorIBL,
                                                         VkDescriptorSet* lightingDescriptorSet)
    {
        // bind pipeline
        m_LightingPipelineIBL->Bind(frameInfo.m_CommandBuffer);

        // push constants
        VK_PushConstantsIBL push{};
        push.m_Values0 = glm::vec4(uMaxPrefilterMip /*number of mips - 1*/, m_Exposure, 0.0f, 0.0f);
        push.m_Values1 = glm::ivec4(static_cast<int>(m_ShaderSettings0.to_ulong()), 0.0f, 0.0f, 0.0f);
        vkCmdPushConstants(frameInfo.m_CommandBuffer, m_LightingPipelineLayoutIBL, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(VK_PushConstantsIBL), &push);

        // bind descriptor sets
        VK_ResourceDescriptor* VK_resourceDescriptorIBL = static_cast<VK_ResourceDescriptor*>(resourceDescriptorIBL.get());
        VkDescriptorSet globalDescriptorSet = frameInfo.m_GlobalDescriptorSet;
        VkDescriptorSet lightingSet =
            lightingDescriptorSet ? *lightingDescriptorSet : m_LightingDescriptorSets[frameInfo.m_FrameIndex];
        VkDescriptorSet shadowMapDescriptorSet = m_ShadowMapDescriptorSets[frameInfo.m_FrameIndex];
        VkDescriptorSet descriptorSetIBL = VK_resourceDescriptorIBL->GetDescriptorSet();
        auto descriptorSets =
            std::to_array<VkDescriptorSet>({globalDescriptorSet, lightingSet, shadowMapDescriptorSet, descriptorSetIBL});

        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_LightingPipelineLayoutIBL, // VkPipelineLayout layout
                                0,                           // uint32_t         firstSet
                                descriptorSets.size(),       // uint32_t         descriptorSetCount
                                descriptorSets.data(),       // VkDescriptorSet* pDescriptorSets
                                0,                           // uint32_t         dynamicOffsetCount
                                nullptr                      // const uint32_t*  pDynamicOffsets
        );

        vkCmdDraw(frameInfo.m_CommandBuffer,
                  3, // vertexCount
                  1, // instanceCount
                  0, // firstVertex
                  0  // firstInstance
        );
    }
} // namespace GfxRenderEngine
