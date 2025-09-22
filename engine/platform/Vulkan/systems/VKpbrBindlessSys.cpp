/* Engine Copyright (c) 2025 Engine Development Team
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
#include "VKinstanceBuffer.h"
#include "VKrenderPass.h"
#include "VKmodel.h"

#include "systems/VKpbrBindlessSys.h"

namespace GfxRenderEngine
{
    VK_RenderSystemPbrBindless::VK_RenderSystemPbrBindless(VkRenderPass renderPass,
                                                           std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        CreatePipelineLayout(descriptorSetLayouts);
        CreatePipeline(renderPass);
    }

    VK_RenderSystemPbrBindless::~VK_RenderSystemPbrBindless()
    {
        vkDestroyPipelineLayout(VK_Core::m_Device->Device(), m_PipelineLayout, nullptr);
    }

    void VK_RenderSystemPbrBindless::CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange0{};
        pushConstantRange0.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange0.offset = 0;
        pushConstantRange0.size = sizeof(DrawCallInfo);

        auto pushConstantRanges = std::to_array({pushConstantRange0});

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
        pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
        auto result = vkCreatePipelineLayout(VK_Core::m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create pipeline layout!");
        }
    }

    void VK_RenderSystemPbrBindless::CreatePipeline(VkRenderPass renderPass)
    {
        CORE_ASSERT(m_PipelineLayout != nullptr, "no pipeline layout");

        PipelineConfigInfo pipelineConfig{};

        VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);

        // we don’t bind vertex buffers, instead we are using BDA
        pipelineConfig.m_BindingDescriptions.clear();
        pipelineConfig.m_AttributeDescriptions.clear();

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        pipelineConfig.subpass = static_cast<uint>(VK_RenderPass::SubPasses3D::SUBPASS_GEOMETRY);

        // g buffer position, g buffer normal, g buffer color, g buffer material, g buffer emission
        // no blending
        auto attachmentCount = (int)VK_RenderPass::NUMBER_OF_GBUFFER_ATTACHMENTS;
        pipelineConfig.colorBlendAttachment.blendEnable = VK_FALSE;

        std::array<VkPipelineColorBlendAttachmentState,
                   static_cast<uint>(VK_RenderPass::RenderTargets3D::NUMBER_OF_ATTACHMENTS)>
            blAttachments;
        for (uint i = 0; i < static_cast<uint>(VK_RenderPass::RenderTargets3D::NUMBER_OF_ATTACHMENTS); ++i)
        {
            blAttachments[i] = pipelineConfig.colorBlendAttachment;
        }
        VK_Pipeline::SetColorBlendState(pipelineConfig, attachmentCount, blAttachments.data());

        // create a pipeline
        m_Pipeline = std::make_unique<VK_Pipeline>(VK_Core::m_Device, "bin-int/pbrBindless.vert.spv",
                                                   "bin-int/pbrBindless.frag.spv", pipelineConfig);
    }

    void VK_RenderSystemPbrBindless::SetVertexCtrl(VertexCtrl const& vertexCtrl)
    {
        m_DrawCallInfo.m_VertexCtrl = vertexCtrl;
    }

    void VK_RenderSystemPbrBindless::RenderEntities(const VK_FrameInfo& frameInfo, Registry& registry,
                                                    VK_BindlessTexture* bindlessTexture, VK_BindlessImage* bindlessImage)
    {
        bindlessTexture->UpdateBindlessDescriptorSets();
        bindlessImage->UpdateBindlessDescriptorSets();
        m_Pipeline->Bind(frameInfo.m_CommandBuffer);
        { // bind descriptor sets
            auto descriptorSets = std::to_array(
                {frameInfo.m_GlobalDescriptorSet, bindlessTexture->GetDescriptorSet(), bindlessImage->GetDescriptorSet()});
            vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer,       // VkCommandBuffer        commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS, // VkPipelineBindPoint    pipelineBindPoint,
                                    m_PipelineLayout,                // VkPipelineLayout       layout,
                                    0,                               // uint32_t               firstSet,
                                    descriptorSets.size(),           // uint32_t               descriptorSetCount,
                                    descriptorSets.data(),           // const VkDescriptorSet* pDescriptorSets,
                                    0,                               // uint32_t               dynamicOffsetCount,
                                    nullptr                          // const uint32_t*        pDynamicOffsets
            );
        }

        auto view = registry.Get().view<MeshComponent, TransformComponent, PbrBindlessMaterialTag, InstanceTag>(
            entt::exclude<SkeletalAnimationTag, GrassTag, Grass2Tag>);
        for (auto mainInstance : view)
        {
            auto& mesh = view.get<MeshComponent>(mainInstance);
            // update instance buffer on the GPU
            InstanceTag& instanced = view.get<InstanceTag>(mainInstance);
            VK_InstanceBuffer* instanceBuffer = static_cast<VK_InstanceBuffer*>(instanced.m_InstanceBuffer.get());
            instanceBuffer->Update();

            if (mesh.m_Enabled)
            {
                auto model = static_cast<VK_Model*>(mesh.m_Model.get());
                m_DrawCallInfo.m_MeshBufferDeviceAddress = model->GetMeshBufferDeviceAddress();
                model->DrawPbrBindless(frameInfo, m_PipelineLayout, m_DrawCallInfo);
            }
        }
    }
} // namespace GfxRenderEngine
