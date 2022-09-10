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

#include "systems/VKcubemapRenderSys.h"

namespace GfxRenderEngine
{
    VK_RenderSystemCubemap::VK_RenderSystemCubemap(VkRenderPass renderPass, VK_DescriptorSetLayout& descriptorSetLayout)
    {
        CreatePipelineLayout(descriptorSetLayout.GetDescriptorSetLayout());
        CreatePipeline(renderPass);
    }

    VK_RenderSystemCubemap::~VK_RenderSystemCubemap()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystemCubemap::CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(VK_PushConstantDataCubemap);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};

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

    void VK_RenderSystemCubemap::CreatePipeline(VkRenderPass renderPass)
    {
        ASSERT(m_PipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        pipelineConfig.subpass = (uint)VK_SwapChain::SubPasses::SUBPASS_TRANSPARENCY;

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>
        (
            VK_Core::m_Device,
            "bin/skybox.vert.spv",
            "bin/skybox.frag.spv",
            pipelineConfig
        );
    }

    void VK_RenderSystemCubemap::RenderEntities(const VK_FrameInfo& frameInfo, entt::registry& registry)
    {
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);

        auto view = registry.view<MeshComponent, TransformComponent, CubemapComponent>();
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);

            VK_PushConstantDataCubemap push{};

            push.m_ModelMatrix  = transform.GetMat4();
            push.m_NormalMatrix = transform.GetNormalMatrix();

            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataCubemap),
                &push);

            auto& mesh = view.get<MeshComponent>(entity);
            if (mesh.m_Enabled)
            {
                static_cast<VK_Model*>(mesh.m_Model.get())->Bind(frameInfo.m_CommandBuffer);
                static_cast<VK_Model*>(mesh.m_Model.get())->DrawCubemap(frameInfo, m_PipelineLayout);
            }
        }
    }
}
