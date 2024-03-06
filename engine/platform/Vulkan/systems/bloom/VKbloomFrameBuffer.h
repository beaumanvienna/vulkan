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

#pragma once

#include <vulkan/vulkan.h>

#include "engine.h"
#include "VKcore.h"

#include "systems/bloom/VKbloomAttachments.h"

namespace GfxRenderEngine
{
    class VK_BloomFrameBuffer
    {

    public:
        VK_BloomFrameBuffer(VK_Attachments::Attachment const& attachment, VkRenderPass const& renderPass);
        ~VK_BloomFrameBuffer();

        VK_BloomFrameBuffer(const VK_BloomFrameBuffer&) = delete;
        VK_BloomFrameBuffer& operator=(const VK_BloomFrameBuffer&) = delete;

        VkFramebuffer Get() const { return m_Framebuffer; }
        VkExtent2D GetExtent() const { return m_Attachment.m_Extent; }

    private:
        VK_Attachments::Attachment m_Attachment;
        VkFramebuffer m_Framebuffer;
    };
} // namespace GfxRenderEngine
