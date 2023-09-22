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
#include "VKdevice.h"
#include "VKcore.h"

namespace GfxRenderEngine
{
    class VK_BloomRenderPass
    {

    public:

        struct Attachment
        {
            VkImageView         m_ImageView;
            VkFormat            m_Format;
            VkExtent2D          m_Extent;
            VkAttachmentLoadOp  m_LoadOp;
            VkAttachmentStoreOp m_StoreOp;
            VkImageLayout       m_InitialLayout;
            VkImageLayout       m_FinalLayout;
            VkImageLayout       m_SubpassLayout;
        };

    public:

        VK_BloomRenderPass(uint numberOfFramebuffers);
        ~VK_BloomRenderPass();

        VK_BloomRenderPass(const VK_BloomRenderPass &) = delete;
        VK_BloomRenderPass& operator=(const VK_BloomRenderPass &) = delete;

        VkImageView GetImageView(uint index) const { return m_Attachments[index].m_ImageView; }
        VkFormat GetFormat(uint index) const { return m_Attachments[index].m_Format; }
        VkFramebuffer GetFramebuffer(uint index) const { return m_Framebuffers[index]; }
        VkRenderPass GetRenderPass() const { return m_RenderPass; }
        VkExtent2D GetExtent() const { return m_RenderPassExtent; }

        VK_BloomRenderPass& AddAttachment(Attachment const& attachment);
        void Build();

    private:

        void CreateFramebuffers();

    private:

        uint m_NumberOfFramebuffers;
        uint m_NumberOfSubpasses;

        std::shared_ptr<VK_Device> m_Device;
        VkExtent2D m_RenderPassExtent;

        std::vector<Attachment> m_Attachments;
        std::vector<VkFramebuffer> m_Framebuffers;
        VkRenderPass m_RenderPass;

    };
}
