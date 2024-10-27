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

#include "systems/VKdebugRenderSys.h"

namespace GfxRenderEngine
{
    VK_RenderSystemDebug::VK_RenderSystemDebug(VkRenderPass renderPass,
                                               std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                                               const VkDescriptorSet* shadowMapDescriptorSet)
    {
        CreatePipelineLayout(descriptorSetLayouts);
        m_ShadowMapDescriptorSets = shadowMapDescriptorSet;
        CreatePipeline(renderPass);
    }

    VK_RenderSystemDebug::~VK_RenderSystemDebug()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystemDebug::CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

        auto result = vkCreatePipelineLayout(VK_Core::m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemDebug::CreatePipeline(VkRenderPass renderPass)
    {
        CORE_ASSERT(m_PipelineLayout != nullptr, "pipeline layout is null");

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        pipelineConfig.subpass = static_cast<uint>(VK_RenderPass::SubPasses3D::SUBPASS_TRANSPARENCY);

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/debug.vert.spv", "bin-int/debug.frag.spv",
                                                   pipelineConfig);
    }

    void VK_RenderSystemDebug::RenderEntities(const VK_FrameInfo& frameInfo, bool showDebugShadowMap)
    {
        if (!showDebugShadowMap)
            return;
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        std::vector<VkDescriptorSet> localDescriptorSet = {m_ShadowMapDescriptorSets[frameInfo.m_FrameIndex]};

        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer,       // VkCommandBuffer        commandBuffer
                                VK_PIPELINE_BIND_POINT_GRAPHICS, // VkPipelineBindPoint    pipelineBindPoint
                                m_PipelineLayout,                // VkPipelineLayout       layout
                                0,                               // uint32_t               firstSet
                                localDescriptorSet.size(),       // uint32_t               descriptorSetCount
                                localDescriptorSet.data(),       // const VkDescriptorSet* pDescriptorSets
                                0,                               // uint32_t               dynamicOffsetCount
                                nullptr                          // const uint32_t*        pDynamicOffsets
        );

        // vertices actually generated in the shader
        int vertexCount = 6;
        vkCmdDraw(frameInfo.m_CommandBuffer, // VkCommandBuffer commandBuffer
                  vertexCount,               // uint32_t        vertexCount
                  1,                         // uint32_t        instanceCount
                  0,                         // uint32_t        firstVertex
                  0                          // uint32_t        firstInstance
        );
    }
} // namespace GfxRenderEngine
