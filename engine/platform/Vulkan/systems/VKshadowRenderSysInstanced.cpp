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
#include "VKmodel.h"
#include "VKswapChain.h"
#include "VKshadowMap.h"

#include "systems/VKshadowRenderSysInstanced.h"
#include "systems/pushConstantData.h"

namespace GfxRenderEngine
{
    VK_RenderSystemShadowInstanced::VK_RenderSystemShadowInstanced(VkRenderPass renderPass0, VkRenderPass renderPass1,
                                                                   std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        CreatePipelineLayout(descriptorSetLayouts);
        CreatePipeline(m_Pipeline0, renderPass0);
        CreatePipeline(m_Pipeline1, renderPass1);
    }

    VK_RenderSystemShadowInstanced::~VK_RenderSystemShadowInstanced()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystemShadowInstanced::CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
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

    void VK_RenderSystemShadowInstanced::CreatePipeline(std::unique_ptr<VK_Pipeline>& pipeline, VkRenderPass renderPass)
    {
        CORE_ASSERT(m_PipelineLayout != nullptr, "pipeline layout is null");

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        pipelineConfig.subpass = static_cast<uint>(VK_ShadowMap::SubPassesShadow::SUBPASS_SHADOW);

        pipelineConfig.rasterizationInfo.depthBiasEnable = VK_TRUE;
        pipelineConfig.rasterizationInfo.depthBiasConstantFactor = 8.0f; // Optional
        pipelineConfig.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
        pipelineConfig.rasterizationInfo.depthBiasSlopeFactor = 3.0f;    // Optional

        // create a pipeline
        pipeline = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/shadowShaderInstanced.vert.spv",
                                                 "bin-int/shadowShaderInstanced.frag.spv", pipelineConfig);
    }

    void VK_RenderSystemShadowInstanced::RenderEntities(const VK_FrameInfo& frameInfo, Registry& registry,
                                                        DirectionalLightComponent* directionalLight, int renderpass,
                                                        const VkDescriptorSet& shadowDescriptorSet)
    {

        if (directionalLight->m_RenderPass == 0)
        {
            m_Pipeline0->Bind(frameInfo.m_CommandBuffer);
        }
        else
        {
            m_Pipeline1->Bind(frameInfo.m_CommandBuffer);
        }

        auto meshView = registry.Get().view<MeshComponent, TransformComponent, InstanceTag>(
            entt::exclude<SkeletalAnimationTag, GrassTag>);
        for (auto entity : meshView)
        {
            auto& mesh = meshView.get<MeshComponent>(entity);
            if (mesh.m_Enabled)
            {
                static_cast<VK_Model*>(mesh.m_Model.get())->Bind(frameInfo.m_CommandBuffer);
                static_cast<VK_Model*>(mesh.m_Model.get())
                    ->DrawShadowInstanced(frameInfo, m_PipelineLayout, shadowDescriptorSet);
            }
        }
    }
} // namespace GfxRenderEngine
