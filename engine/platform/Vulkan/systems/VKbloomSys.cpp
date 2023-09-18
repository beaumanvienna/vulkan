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

#include "systems/VKbloomSys.h"
#include "VKswapChain.h"

namespace GfxRenderEngine
{
    VK_RenderSystemBloom::VK_RenderSystemBloom
    (
        VkRenderPass renderPass,
        std::vector<VkDescriptorSetLayout>& bloomDescriptorSetLayout,
        const VkDescriptorSet* bloomDescriptorSet
    )
    {
        CreateBloomPipelinesLayout(bloomDescriptorSetLayout);
        m_BloomDescriptorSets = bloomDescriptorSet;
        CreateBloomPipelines(renderPass);
    }

    VK_RenderSystemBloom::~VK_RenderSystemBloom()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_BloomPipelineLayout, nullptr);
    }

    // up & down share the same layout
    void VK_RenderSystemBloom::CreateBloomPipelinesLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataBloom);

        VkPipelineLayoutCreateInfo bloomPipelineLayoutInfo{};
        bloomPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        bloomPipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayout.size());
        bloomPipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
        bloomPipelineLayoutInfo.pushConstantRangeCount = 1;
        bloomPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(VK_Core::m_Device->Device(), &bloomPipelineLayoutInfo, nullptr, &m_BloomPipelineLayout) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemBloom::CreateBloomPipelines(VkRenderPass renderPass)
    {
        ASSERT(m_BloomPipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_BloomPipelineLayout;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.subpass = static_cast<uint>(VK_RenderPass::SubPassesPostProcessing::SUBPASS_BLOOM);
        pipelineConfig.m_BindingDescriptions.clear();   // this pipeline is not using vertices
        pipelineConfig.m_AttributeDescriptions.clear();

        // create pipelines
        m_BloomPipelineUp = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin-int/bloomUp.vert.spv",
            "bin-int/bloomUp.frag.spv",
            pipelineConfig
        );
        m_BloomPipelineDown = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin-int/bloomDown.vert.spv",
            "bin-int/bloomDown.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystemBloom::RenderBloom(const VK_FrameInfo& frameInfo)
    {
        { // common
            std::vector<VkDescriptorSet> descriptorSets =
            {
                frameInfo.m_GlobalDescriptorSet,
                m_BloomDescriptorSets[frameInfo.m_FrameIndex]
            };

            vkCmdBindDescriptorSets
            (
                frameInfo.m_CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_BloomPipelineLayout,  // VkPipelineLayout layout
                0,                          // uint32_t         firstSet
                descriptorSets.size(),      // uint32_t         descriptorSetCount
                descriptorSets.data(),      // VkDescriptorSet* pDescriptorSets
                0,                          // uint32_t         dynamicOffsetCount
                nullptr                     // const uint32_t*  pDynamicOffsets
            );
        }

        // down
        {
            m_BloomPipelineDown->Bind(frameInfo.m_CommandBuffer);

            vkCmdDraw
            (
                frameInfo.m_CommandBuffer,
                3,      // vertexCount
                1,      // instanceCount
                0,      // firstVertex
                0       // firstInstance
            );
        }

        // up
        {
            m_BloomPipelineUp->Bind(frameInfo.m_CommandBuffer);

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
}
