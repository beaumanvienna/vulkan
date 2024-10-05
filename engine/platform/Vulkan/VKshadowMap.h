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

#pragma once

#include <vulkan/vulkan.h>

#include "engine.h"
#include "VKdevice.h"
#include "VKcore.h"

namespace GfxRenderEngine
{
    class VK_ShadowMap
    {

    public:
        enum class SubPassesShadow
        {
            SUBPASS_SHADOW = 0,
            NUMBER_OF_SUBPASSES
        };

        enum class ShadowRenderTargets
        {
            ATTACHMENT_DEPTH = 0,
            NUMBER_OF_ATTACHMENTS
        };

    public:
        VK_ShadowMap(int width);
        ~VK_ShadowMap();

        VK_ShadowMap(const VK_ShadowMap&) = delete;
        VK_ShadowMap& operator=(const VK_ShadowMap&) = delete;

        VkFramebuffer GetShadowFrameBuffer() { return m_ShadowFramebuffer; }
        VkRenderPass GetShadowRenderPass() { return m_ShadowRenderPass; }
        VkExtent2D GetShadowMapExtent() { return m_ShadowMapExtent; }
        const VkDescriptorImageInfo& GetDescriptorImageInfo() const { return m_DescriptorImageInfo; }

    private:
        void CreateShadowDepthResources();
        void CreateShadowRenderPass();
        void CreateShadowFramebuffer();

    private:
        VkFormat m_DepthFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VK_Device* m_Device;

        VkExtent2D m_ShadowMapExtent{};
        VkFramebuffer m_ShadowFramebuffer{nullptr};
        VkRenderPass m_ShadowRenderPass{nullptr};

        VkImage m_ShadowDepthImage{nullptr};
        VkImageLayout m_ImageLayout{};
        VkImageView m_ShadowDepthImageView{nullptr};
        VkDeviceMemory m_ShadowDepthImageMemory{nullptr};
        VkSampler m_ShadowDepthSampler{nullptr};

        VkDescriptorImageInfo m_DescriptorImageInfo{};
    };
} // namespace GfxRenderEngine
