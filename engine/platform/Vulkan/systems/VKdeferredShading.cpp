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
#include "systems/pushConstantData.h"

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
    }

    VK_RenderSystemDeferredShading::~VK_RenderSystemDeferredShading()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_LightingPipelineLayout, nullptr);
    }

    void
    VK_RenderSystemDeferredShading::CreateLightingPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataGeneric);

        VkPipelineLayoutCreateInfo lightingPipelineLayoutInfo{};
        lightingPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        lightingPipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        lightingPipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        lightingPipelineLayoutInfo.pushConstantRangeCount = 1;
        lightingPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
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

    void VK_RenderSystemDeferredShading::LightingPass(const VK_FrameInfo& frameInfo)
    {
        m_LightingPipeline->Bind(frameInfo.m_CommandBuffer);

        std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet,
                                                       m_LightingDescriptorSets[frameInfo.m_FrameIndex],
                                                       m_ShadowMapDescriptorSets[frameInfo.m_FrameIndex]};

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
} // namespace GfxRenderEngine
