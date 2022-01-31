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

#include "core.h"
#include "scene/scene.h"

#include "systems/VKpointLightSystem.h"
#include "VKswapChain.h"
#include "VKmodel.h"

namespace GfxRenderEngine
{

    struct PointLightPushConstants
    {
        glm::vec4 m_Position;
        glm::vec4 m_Color;
        float m_Radius;
    };

    VK_PointLightSystem::VK_PointLightSystem(std::shared_ptr<VK_Device> device, VkRenderPass renderPass, VK_DescriptorSetLayout& globalDescriptorSetLayout)
        : m_Device(device)
    {
        CreatePipelineLayout(globalDescriptorSetLayout.GetDescriptorSetLayout());
        CreatePipeline(renderPass);
    }

    VK_PointLightSystem::~VK_PointLightSystem()
    {
        vkDestroyPipelineLayout(m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_PointLightSystem::CreatePipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalDescriptorSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_PointLightSystem::CreatePipeline(VkRenderPass renderPass)
    {
        ASSERT(m_PipelineLayout != nullptr);

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.m_BindingDescriptions.clear();
        pipelineConfig.m_AttributeDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>
        (
            m_Device,
            "bin/pointLight.vert.spv",
            "bin/pointLight.frag.spv",
            pipelineConfig
        );
    }

    void VK_PointLightSystem::Render(const VK_FrameInfo& frameInfo, entt::registry& registry)
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

        vkCmdDraw(frameInfo.m_CommandBuffer, 6, 1, 0, 0);

        auto view = registry.view<PointLightComponent, TransformComponent>();
        for (auto entity : view)
        {
            auto& transform  = view.get<TransformComponent>(entity);
            auto& pointLight = view.get<PointLightComponent>(entity);

            PointLightPushConstants push{};
            push.m_Position = glm::vec4(transform.m_Translation, 1.f);
            push.m_Color = glm::vec4(pointLight.m_Color, pointLight.m_LightIntensity);
            push.m_Radius = pointLight.m_Radius;

            vkCmdPushConstants
            (
                frameInfo.m_CommandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push
            );

            vkCmdDraw(frameInfo.m_CommandBuffer, 6, 1, 0, 0);
        }
    }

    void VK_PointLightSystem::Update(const VK_FrameInfo& frameInfo, GlobalUniformBuffer& ubo, entt::registry& registry)
    {
        int lightIndex = 0;
        auto view = registry.view<PointLightComponent, TransformComponent>();
        for (auto entity : view)
        {
            auto& transform  = view.get<TransformComponent>(entity);
            auto& pointLight = view.get<PointLightComponent>(entity);

            ASSERT(lightIndex < MAX_LIGHTS);

            // copy light to ubo
            ubo.m_PointLights[lightIndex].m_Position = glm::vec4(transform.m_Translation, 1.f);
            ubo.m_PointLights[lightIndex].m_Color = glm::vec4(pointLight.m_Color, pointLight.m_LightIntensity);

            lightIndex++;
        }

        ubo.m_NumberOfActiveLights = lightIndex;
    }
}
