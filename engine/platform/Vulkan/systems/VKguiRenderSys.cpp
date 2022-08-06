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

#include "systems/VKguiRenderSys.h"
#include "renderer/model.h"

namespace GfxRenderEngine
{
    VK_RenderSystemGUIRenderer::VK_RenderSystemGUIRenderer(VkRenderPass renderPass, VK_DescriptorSetLayout& globalDescriptorSetLayout)
    {
        CreatePipelineLayout(globalDescriptorSetLayout.GetDescriptorSetLayout());
        CreatePipeline(renderPass);
        CreateVertexBuffer();
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
        if (vkCreatePipelineLayout(VK_Core::m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemGUIRenderer::CreatePipeline(VkRenderPass renderPass)
    {
        ASSERT(m_PipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        pipelineConfig.subpass = (uint)VK_SwapChain::SubPassesGUI::SUBPASS_GUI;

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin/guiShader.vert.spv",
            "bin/guiShader.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystemGUIRenderer::RenderEntities(const VK_FrameInfo& frameInfo, entt::registry& registry, const glm::mat4& viewProjectionMatrix)
    {
        vkCmdBindDescriptorSets
        (
            frameInfo.m_CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout,
            0,
            1,
            &frameInfo.m_GlobalDescriptorSet,
            0,
            nullptr
        );
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        auto view = registry.view<MeshComponent, TransformComponent, GuiRendererComponent>();
        for (auto entity : view)
        {
            auto& mesh = view.get<MeshComponent>(entity);
            if (mesh.m_Enabled)
            {
                auto& transform = view.get<TransformComponent>(entity);
                VK_PushConstantDataGUIRenderer push{};
                push.m_MVP  = viewProjectionMatrix * transform.GetMat4();

                vkCmdPushConstants(
                    frameInfo.m_CommandBuffer,
                    m_PipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(VK_PushConstantDataGUIRenderer),
                    &push);

                static_cast<VK_Model*>(mesh.m_Model.get())->Bind(frameInfo.m_CommandBuffer);
                static_cast<VK_Model*>(mesh.m_Model.get())->Draw(frameInfo.m_CommandBuffer);
            }
        }
    }

    void VK_RenderSystemGUIRenderer::CreateVertexBuffer()
    {
        Vertex vertices[m_VertexCount];
        uint vertexSize = sizeof(Vertex);
        VkDeviceSize bufferSize = vertexSize * m_VertexCount;

        VK_Buffer stagingBuffer
        {
            *VK_Core::m_Device, vertexSize, m_VertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.Map();

        vertices[0].m_Position = {0.0f, 0.0f, 0.0f};
        vertices[0].m_UV =  {0.0f, 0.0f};
        vertices[1].m_Position = {0.0f, 0.0f, 0.0f};
        vertices[1].m_UV =  {0.0f, 0.0f};
        vertices[2].m_Position = {0.0f, 0.0f, 0.0f};
        vertices[2].m_UV =  {0.0f, 0.0f};
        vertices[3].m_Position = {0.0f, 0.0f, 0.0f};
        vertices[3].m_UV =  {0.0f, 0.0f};

        stagingBuffer.WriteToBuffer((void*) vertices);

        m_VertexBuffer = std::make_unique<VK_Buffer>
        (
            *VK_Core::m_Device, vertexSize, m_VertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        VK_Core::m_Device->CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), bufferSize);
    }

    void VK_RenderSystemGUIRenderer::RenderSprite(const VK_FrameInfo& frameInfo, Sprite* sprite, const glm::mat4& viewProjectionMatrix)
    {
        vkCmdBindDescriptorSets
        (
            frameInfo.m_CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout,
            0,
            1,
            &frameInfo.m_GlobalDescriptorSet,
            0,
            nullptr
        );
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        VK_PushConstantDataGUIRenderer push{};
        push.m_MVP  = viewProjectionMatrix;
        push.m_UV[0]  = glm::vec2{sprite->m_Pos1X, sprite->m_Pos1Y};
        push.m_UV[1]  = glm::vec2{sprite->m_Pos2X, sprite->m_Pos2Y};
        
        vkCmdPushConstants(
            frameInfo.m_CommandBuffer,
            m_PipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(VK_PushConstantDataGUIRenderer),
            &push);

        VkDeviceSize offsets[] = {0};
        VkBuffer buffer = m_VertexBuffer->GetBuffer();
        vkCmdBindVertexBuffers(frameInfo.m_CommandBuffer, 0, 1, &buffer, offsets);

        vkCmdDraw
            (
                frameInfo.m_CommandBuffer,      // VkCommandBuffer commandBuffer
                m_VertexCount,                  // uint32_t        vertexCount
                1,                              // uint32_t        instanceCount
                0,                              // uint32_t        firstVertex
                0                               // uint32_t        firstInstance
            );

    }
}
