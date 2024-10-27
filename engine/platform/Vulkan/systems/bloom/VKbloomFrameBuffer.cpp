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

#include "systems/bloom/VKbloomFrameBuffer.h"

namespace GfxRenderEngine
{

    VK_BloomFrameBuffer::VK_BloomFrameBuffer(VK_Attachments::Attachment const& attachment, VkRenderPass const& renderPass)
        : m_Attachment{attachment}
    {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &m_Attachment.m_ImageView;
        framebufferInfo.width = m_Attachment.m_Extent.width;
        framebufferInfo.height = m_Attachment.m_Extent.height;
        framebufferInfo.layers = 1;

        auto result = vkCreateFramebuffer(VK_Core::m_Device->Device(), &framebufferInfo, nullptr, &m_Framebuffer);
        if (result != VK_SUCCESS)
        {
            VK_Core::m_Device->PrintError(result);
            LOG_CORE_CRITICAL("failed to create framebuffer!");
        }
    }

    VK_BloomFrameBuffer::~VK_BloomFrameBuffer()
    {
        vkDestroyFramebuffer(VK_Core::m_Device->Device(), m_Framebuffer, nullptr);
    }
} // namespace GfxRenderEngine
