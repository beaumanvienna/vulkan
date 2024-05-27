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

#include <memory>

#include <vulkan/vulkan.h>

namespace GfxRenderEngine
{
    class VK_Attachments
    {

    public:
        struct Attachment
        {
            VkImageView m_ImageView;
            VkFormat m_Format;
            VkExtent2D m_Extent;
            VkAttachmentLoadOp m_LoadOp;
            VkAttachmentStoreOp m_StoreOp;
            VkImageLayout m_InitialLayout;
            VkImageLayout m_FinalLayout;
            VkImageLayout m_SubpassLayout;
        };

    public:
        void Add(Attachment const& attachment);
        Attachment& operator[](uint index) { return m_Attachments[index]; }

    private:
        std::vector<Attachment> m_Attachments;
    };
} // namespace GfxRenderEngine
