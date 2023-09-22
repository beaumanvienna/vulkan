/* Engine Copyright (c) 2023 Engine Development Team 
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

#include "engine.h"

#include "VKbloomRenderPass.h"

namespace GfxRenderEngine
{

    VK_BloomRenderPass::VK_BloomRenderPass(uint numberOfFramebuffers)
        : m_NumberOfFramebuffers{numberOfFramebuffers},
          m_NumberOfSubpasses{1} // just one for now
    {
        m_Device = VK_Core::m_Device;
    }

    VK_BloomRenderPass::~VK_BloomRenderPass()
    {
        for (auto framebuffer : m_Framebuffers)
        {
            vkDestroyFramebuffer(m_Device->Device(), framebuffer, nullptr);
        }
        vkDestroyRenderPass(m_Device->Device(), m_RenderPass, nullptr);
    }

    VK_BloomRenderPass& VK_BloomRenderPass::AddAttachment(Attachment const& attachment)
    {
        if (!m_Attachments.size()) // first attachment defines extend
        {
            m_RenderPassExtent = attachment.m_Extent;
        }
        m_Attachments.push_back(attachment);
        return *this;
    }

    void VK_BloomRenderPass::CreateFramebuffers()
    {
        m_Framebuffers.resize(m_NumberOfFramebuffers);
        for (uint framebufferIndex = 0; framebufferIndex < m_NumberOfFramebuffers; ++framebufferIndex)
        {
            uint numberOfAttachments = m_Attachments.size();

            std::vector<VkImageView> attachments;

            for (uint attachmentIndex = 0; attachmentIndex < numberOfAttachments; ++attachmentIndex)
            {
                attachments.push_back(m_Attachments[attachmentIndex].m_ImageView);
            }

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = numberOfAttachments;
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_RenderPassExtent.width;
            framebufferInfo.height = m_RenderPassExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(
                    m_Device->Device(),
                    &framebufferInfo,
                    nullptr,
                    &m_Framebuffers[framebufferIndex]) != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create framebuffer!");
            }
        }
    }

    void VK_BloomRenderPass::Build()
    {
        CreateFramebuffers();

        uint numberOfAttachments = m_Attachments.size();

        std::vector<VkAttachmentDescription> attachmentDescriptions;
        std::vector<VkAttachmentReference> attachmentReferences;

        for (uint attachmentIndex = 0; attachmentIndex < numberOfAttachments; ++attachmentIndex)
        {
            VkAttachmentDescription attachmentDescr = {};
            attachmentDescr.format = m_Attachments[attachmentIndex].m_Format;
            attachmentDescr.samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescr.loadOp = m_Attachments[attachmentIndex].m_LoadOp;
            attachmentDescr.storeOp = m_Attachments[attachmentIndex].m_StoreOp;
            attachmentDescr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescr.initialLayout = m_Attachments[attachmentIndex].m_InitialLayout;
            attachmentDescr.finalLayout = m_Attachments[attachmentIndex].m_FinalLayout;

            attachmentDescriptions.push_back(attachmentDescr);

            VkAttachmentReference attachmentRef = {};
            attachmentRef.attachment = attachmentIndex;
            attachmentRef.layout = m_Attachments[attachmentIndex].m_SubpassLayout;

            attachmentReferences.push_back(attachmentRef);
        }

        std::vector<VkSubpassDescription> subpasses;
        for (uint subpassIndex; subpassIndex < m_NumberOfSubpasses; ++subpassIndex)
        {
            VkSubpassDescription subpass = {};
            subpass.flags = 0;
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.inputAttachmentCount = 0;
            subpass.pInputAttachments = nullptr;
            subpass.colorAttachmentCount = numberOfAttachments;
            subpass.pColorAttachments = attachmentReferences.data();
            subpass.pResolveAttachments = nullptr;
            subpass.pDepthStencilAttachment = nullptr;
            subpass.preserveAttachmentCount = 0;
            subpass.pPreserveAttachments = nullptr;

            subpasses.push_back(subpass);
        }

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = numberOfAttachments;
        renderPassInfo.pAttachments = attachmentDescriptions.data();
        renderPassInfo.subpassCount = m_NumberOfSubpasses;
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = 0;
        renderPassInfo.pDependencies = nullptr;

        if (vkCreateRenderPass(m_Device->Device(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create render pass!");
        }
    }
}
