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

#include "core.h"
#include "systems/VKguiRenderSys.h"
#include "renderer/model.h"
#include "transform/matrix.h"

namespace GfxRenderEngine
{
    VK_RenderSystemGUIRenderer::VK_RenderSystemGUIRenderer(VkRenderPass renderPass,
                                                           VK_DescriptorSetLayout& globalDescriptorSetLayout)
    {
        CreatePipelineLayout(globalDescriptorSetLayout.GetDescriptorSetLayout());
        CreatePipeline(renderPass);
    }

    VK_RenderSystemGUIRenderer::~VK_RenderSystemGUIRenderer()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystemGUIRenderer::CreatePipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataGUIRenderer);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalDescriptorSetLayout};

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

    void VK_RenderSystemGUIRenderer::CreatePipeline(VkRenderPass renderPass)
    {
        CORE_ASSERT(m_PipelineLayout != nullptr, "pipeline layout is null");

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.m_BindingDescriptions.clear();
        pipelineConfig.m_AttributeDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        pipelineConfig.subpass = static_cast<uint>(VK_RenderPass::SubPassesGUI::SUBPASS_GUI);

        // create pipelines
        m_Pipeline = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/guiShader.vert.spv",
                                                   "bin-int/guiShader.frag.spv", pipelineConfig);

        m_Pipeline2 = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/guiShader2.vert.spv",
                                                    "bin-int/guiShader2.frag.spv", pipelineConfig);
    }

    // uses guiShader
    void VK_RenderSystemGUIRenderer::RenderSprite(const VK_FrameInfo& frameInfo, const Sprite& sprite,
                                                  const glm::mat4& modelViewProjectionMatrix)
    {
        // this function takes in a sprite and transformation matrix to be applied to the normalized device coordinates
        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1,
                                &frameInfo.m_GlobalDescriptorSet, 0, nullptr);
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        VK_PushConstantDataGUIRenderer push{};
        push.m_MVP = modelViewProjectionMatrix;

        push.m_UV[0] = {sprite.m_Pos1X, sprite.m_Pos1Y};
        push.m_UV[1] = {sprite.m_Pos2X, sprite.m_Pos2Y};

        vkCmdPushConstants(frameInfo.m_CommandBuffer, m_PipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(VK_PushConstantDataGUIRenderer), &push);

        vkCmdDraw(frameInfo.m_CommandBuffer, // VkCommandBuffer commandBuffer
                  m_VertexCount,             // uint32_t        vertexCount
                  1,                         // uint32_t        instanceCount
                  0,                         // uint32_t        firstVertex
                  0                          // uint32_t        firstInstance
        );
    }

    // uses guiShader2
    void VK_RenderSystemGUIRenderer::RenderSprite(const VK_FrameInfo& frameInfo, const Sprite& sprite,
                                                  const glm::mat4& position, const glm::vec4& color, const float textureID)
    {
        // this function takes in a sprite, four 2D positions, and a color

        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1,
                                &frameInfo.m_GlobalDescriptorSet, 0, nullptr);
        m_Pipeline2->Bind(frameInfo.m_CommandBuffer);

        VK_PushConstantDataGUIRenderer2 push{};
        push.m_Mat4 = position;
        push.m_Mat4[2][0] = color.r;
        push.m_Mat4[3][0] = color.g;
        push.m_Mat4[2][1] = color.b;
        push.m_Mat4[3][1] = color.a;
        push.m_Mat4[2][2] = Engine::m_Engine->GetWindowWidth();
        push.m_Mat4[2][3] = Engine::m_Engine->GetWindowHeight();
        push.m_Mat4[3][2] = textureID;

        push.m_UV[0] = {sprite.m_Pos1X, sprite.m_Pos1Y};
        push.m_UV[1] = {sprite.m_Pos2X, sprite.m_Pos2Y};

        vkCmdPushConstants(frameInfo.m_CommandBuffer, m_PipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(VK_PushConstantDataGUIRenderer), &push);

        vkCmdDraw(frameInfo.m_CommandBuffer, // VkCommandBuffer commandBuffer
                  m_VertexCount,             // uint32_t        vertexCount
                  1,                         // uint32_t        instanceCount
                  0,                         // uint32_t        firstVertex
                  0                          // uint32_t        firstInstance
        );
    }
} // namespace GfxRenderEngine
