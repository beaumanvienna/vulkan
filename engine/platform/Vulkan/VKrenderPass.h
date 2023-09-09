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
    class VK_RenderPass
    {

    public:

        enum class SubPasses
        {
            SUBPASS_GEOMETRY = 0,
            SUBPASS_LIGHTING,
            SUBPASS_TRANSPARENCY,
            NUMBER_OF_SUBPASSES
        };

        enum class RenderTargets
        {
            ATTACHMENT_BACKBUFFER = 0,
            ATTACHMENT_DEPTH,
            ATTACHMENT_GBUFFER_POSITION,
            ATTACHMENT_GBUFFER_NORMAL,
            ATTACHMENT_GBUFFER_COLOR,
            ATTACHMENT_GBUFFER_MATERIAL,
            NUMBER_OF_ATTACHMENTS
        };

        enum class SubPassesGUI
        {
            SUBPASS_GUI = 0,
            NUMBER_OF_SUBPASSES
        };

        enum class RenderTargetsGUI
        {
            ATTACHMENT_BACKBUFFER = 0,
            NUMBER_OF_ATTACHMENTS
        };

        static constexpr int NUMBER_OF_GBUFFER_ATTACHMENTS = (int)RenderTargets::NUMBER_OF_ATTACHMENTS - (int)RenderTargets::ATTACHMENT_GBUFFER_POSITION;

    public:

        VK_RenderPass(VkExtent2D windowExtent, VK_SwapChain* swapChain);
        ~VK_RenderPass();

        VK_RenderPass(const VK_RenderPass &) = delete;
        VK_RenderPass& operator=(const VK_RenderPass &) = delete;

        VkImageView GetImageViewGBufferPosition() { return m_GBufferPositionView; }
        VkImageView GetImageViewGBufferNormal() { return m_GBufferNormalView; }
        VkImageView GetImageViewGBufferColor() { return m_GBufferColorView; }
        VkImageView GetImageViewGBufferMaterial() { return m_GBufferMaterialView; }

        VkFramebuffer Get3DFrameBuffer(int index) { return m_3DFramebuffers[index]; }
        VkFramebuffer GetGUIFrameBuffer(int index) { return m_GUIFramebuffers[index]; }

        VkRenderPass Get3DRenderPass() { return m_3DRenderPass; }
        VkRenderPass GetGUIRenderPass() { return m_GUIRenderPass; }

        VkExtent2D GetExtent() { return m_RenderPassExtent; }

    private:

        void CreateDepthResources();

        void Create3DRenderPass();
        void CreateGUIRenderPass();

        void Create3DFramebuffers();
        void CreateGUIFramebuffers();

        void CreateGBufferImages();
        void CreateGBufferImageViews();
        void DestroyGBuffers();

    private:

        std::shared_ptr<VK_Device> m_Device;
        VK_SwapChain* m_SwapChain;
        VkExtent2D m_RenderPassExtent;

        VkFormat m_DepthFormat;
        VkFormat m_BufferPositionFormat;
        VkFormat m_BufferNormalFormat;
        VkFormat m_BufferColorFormat;
        VkFormat m_BufferMaterialFormat;

        VkImage m_DepthImage;
        VkImage m_GBufferPositionImage;
        VkImage m_GBufferNormalImage;
        VkImage m_GBufferColorImage;
        VkImage m_GBufferMaterialImage;

        VkImageView m_DepthImageView;
        VkImageView m_GBufferPositionView;
        VkImageView m_GBufferNormalView;
        VkImageView m_GBufferColorView;
        VkImageView m_GBufferMaterialView;
    
        VkDeviceMemory m_DepthImageMemory;
        VkDeviceMemory m_GBufferPositionImageMemory;
        VkDeviceMemory m_GBufferNormalImageMemory;
        VkDeviceMemory m_GBufferColorImageMemory;
        VkDeviceMemory m_GBufferMaterialImageMemory;

        std::vector<VkFramebuffer> m_3DFramebuffers;
        std::vector<VkFramebuffer> m_GUIFramebuffers;

        VkRenderPass m_3DRenderPass;
        VkRenderPass m_GUIRenderPass;
    };
}
