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

#include "VKrenderSystem.h"
#include "VKmodel.h"

namespace GfxRenderEngine
{
    VK_RenderSystem::VK_RenderSystem(std::shared_ptr<VK_Device> device, VkRenderPass renderPass)
        : m_Device(device)
    {
        CreatePipelineLayout();
        CreatePipeline(renderPass);
    }

    VK_RenderSystem::~VK_RenderSystem()
    {
        vkDestroyPipelineLayout(m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystem::CreatePipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystem::CreatePipeline(VkRenderPass renderPass)
    {
        ASSERT(m_PipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>
        (
            m_Device,
            "bin/simpleShader.vert.spv",
            "bin/simpleShader.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystem::RenderEntities(const VK_FrameInfo& frameInfo, std::vector<Entity>& entities)
    {
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        auto viewProjectionMatrix = frameInfo.m_Camera.GetViewProjectionMatrix();
        for (auto& entity : entities)
        {
            VK_SimplePushConstantData push{};
            push.m_Transform = viewProjectionMatrix * entity.m_Transform.Mat4();
            push.m_NormalMatrix  = entity.m_Transform.NormalMatrix();
            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_SimplePushConstantData),
                &push);
            static_cast<VK_Model*>(entity.m_Model.get())->Bind(frameInfo.m_CommandBuffer);
            static_cast<VK_Model*>(entity.m_Model.get())->Draw(frameInfo.m_CommandBuffer);
        }
    }
}
