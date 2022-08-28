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
#include "VKmodel.h"

#include "systems/VKdeferredRendering.h"
#include "VKswapChain.h"

namespace GfxRenderEngine
{
    VK_RenderSystemDeferredRendering::VK_RenderSystemDeferredRendering
    (
        VkRenderPass renderPass,
        std::vector<VkDescriptorSetLayout>& ligthingDescriptorSetLayouts,
        const VkDescriptorSet* lightingDescriptorSet)
    {
        CreateLightingPipelineLayout(ligthingDescriptorSetLayouts);
        m_LightingDescriptorSets = lightingDescriptorSet;
        CreateLightingPipeline(renderPass);
    }

    VK_RenderSystemDeferredRendering::~VK_RenderSystemDeferredRendering()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_LightingPipelineLayout, nullptr);
    }

    void VK_RenderSystemDeferredRendering::CreateLightingPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataDeferredRendering);

        VkPipelineLayoutCreateInfo lightingPipelineLayoutInfo{};
        lightingPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        lightingPipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        lightingPipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        lightingPipelineLayoutInfo.pushConstantRangeCount = 1;
        lightingPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(VK_Core::m_Device->Device(), &lightingPipelineLayoutInfo, nullptr, &m_LightingPipelineLayout) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemDeferredRendering::CreateLightingPipeline(VkRenderPass renderPass)
    {
        ASSERT(m_LightingPipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_LightingPipelineLayout;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.subpass = (uint)VK_SwapChain::SubPasses::SUBPASS_LIGHTING;

        // create a pipeline
        m_LightingPipeline = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin/deferredRendering.vert.spv",
            "bin/deferredRendering.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystemDeferredRendering::LightingPass(const VK_FrameInfo& frameInfo, uint currentImageIndex)
    {
        m_LightingPipeline->Bind(frameInfo.m_CommandBuffer);

        std::vector<VkDescriptorSet> descriptorSets = 
        {
            frameInfo.m_GlobalDescriptorSet,
            m_LightingDescriptorSets[currentImageIndex]
        };

        vkCmdBindDescriptorSets
        (
            frameInfo.m_CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_LightingPipelineLayout,   // VkPipelineLayout layout
            0,                          // uint32_t         firstSet
            2,                          // uint32_t         descriptorSetCount
            descriptorSets.data(),      // VkDescriptorSet* pDescriptorSets
            0,                          // uint32_t         dynamicOffsetCount
            nullptr                     // const uint32_t*  pDynamicOffsets
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
