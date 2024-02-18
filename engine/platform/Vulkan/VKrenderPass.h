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
#include "VKswapChain.h"

#include "engine.h"
#include "VKcore.h"

namespace GfxRenderEngine
{
    class VK_RenderPass
    {

    public:

        enum class SubPasses3D
        {
            SUBPASS_GEOMETRY = 0,
            SUBPASS_LIGHTING,
            SUBPASS_TRANSPARENCY,
            NUMBER_OF_SUBPASSES
        };

        enum class RenderTargets3D
        {
            ATTACHMENT_COLOR = 0,
            ATTACHMENT_DEPTH,
            ATTACHMENT_GBUFFER_POSITION,
            ATTACHMENT_GBUFFER_NORMAL,
            ATTACHMENT_GBUFFER_COLOR,
            ATTACHMENT_GBUFFER_MATERIAL,
            ATTACHMENT_GBUFFER_EMISSION,
            NUMBER_OF_ATTACHMENTS
        };

        enum class SubPassesPostProcessing
        {
            SUBPASS_BLOOM = 0,
            NUMBER_OF_SUBPASSES
        };

        enum class RenderTargetsPostProcessing
        {
            ATTACHMENT_COLOR = 0,
            INPUT_ATTACHMENT_3DPASS_COLOR,
            INPUT_ATTACHMENT_GBUFFER_EMISSION,
            NUMBER_OF_ATTACHMENTS
        };

        enum class SubPassesGUI
        {
            SUBPASS_GUI = 0,
            NUMBER_OF_SUBPASSES
        };

        enum class RenderTargetsGUI
        {
            ATTACHMENT_COLOR = 0,
            NUMBER_OF_ATTACHMENTS
        };

        static constexpr int NUMBER_OF_GBUFFER_ATTACHMENTS = (int)RenderTargets3D::NUMBER_OF_ATTACHMENTS - (int)RenderTargets3D::ATTACHMENT_GBUFFER_POSITION;
        static constexpr int NUMBER_OF_POSTPROCESSING_INPUT_ATTACHMENTS = (int)RenderTargetsPostProcessing::NUMBER_OF_ATTACHMENTS - (int)RenderTargetsPostProcessing::INPUT_ATTACHMENT_3DPASS_COLOR;
        static constexpr int NUMBER_OF_POSTPROCESSING_OUPUT_ATTACHMENTS = (int)RenderTargetsPostProcessing::INPUT_ATTACHMENT_3DPASS_COLOR; // 1st input attachment == number of output attachments

    public:

        VK_RenderPass(VK_SwapChain* swapChain);
        ~VK_RenderPass();

        VK_RenderPass(const VK_RenderPass &) = delete;
        VK_RenderPass& operator=(const VK_RenderPass &) = delete;

        VkImageView GetImageViewColorAttachment() { return m_Device->GetImageViewSlot(m_ColorAttachmentImage.defaultView()).vkImageView; }
        VkImageView GetImageViewGBufferPosition() { return m_Device->GetImageViewSlot(m_GBufferPositionImage.defaultView()).vkImageView; }
        VkImageView GetImageViewGBufferNormal() { return m_Device->GetImageViewSlot(m_GBufferNormalImage.defaultView()).vkImageView; }
        VkImageView GetImageViewGBufferColor() { return m_Device->GetImageViewSlot(m_GBufferColorImage.defaultView()).vkImageView; }
        VkImageView GetImageViewGBufferMaterial() { return m_Device->GetImageViewSlot(m_GBufferMaterialImage.defaultView()).vkImageView; }
        VkImageView GetImageViewGBufferEmission() { return m_Device->GetImageViewSlot(m_GBufferEmissionImage.defaultView()).vkImageView; }

        VkImage GetImageEmission() const { return m_Device->GetImageSlot(m_GBufferEmissionImage).vkImage; }
        VkFormat GetFormatEmission() const { return m_BufferEmissionFormat; }

        VkFramebuffer Get3DFrameBuffer(int index) { return m_3DFramebuffers[index]; }
        VkFramebuffer GetPostProcessingFrameBuffer(int index) { return m_PostProcessingFramebuffers[index]; }
        VkFramebuffer GetGUIFrameBuffer(int index) { return m_GUIFramebuffers[index]; }

        VkRenderPass Get3DRenderPass() { return m_3DRenderPass; }
        VkRenderPass GetPostProcessingRenderPass() { return m_PostProcessingRenderPass; }
        VkRenderPass GetGUIRenderPass() { return m_GUIRenderPass; }

        VkExtent2D GetExtent() const { return m_RenderPassExtent; }

    private:

        void CreateColorAttachmentResources();
        void CreateDepthResources();

        void Create3DRenderPass();
        void CreatePostProcessingRenderPass();
        void CreateGUIRenderPass();

        void Create3DFramebuffers();
        void CreatePostProcessingFramebuffers();
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
        VkFormat m_BufferEmissionFormat;

        ImageId m_DepthImage;
        ImageId m_ColorAttachmentImage;
        ImageId m_GBufferPositionImage;
        ImageId m_GBufferNormalImage;
        ImageId m_GBufferColorImage;
        ImageId m_GBufferMaterialImage;
        ImageId m_GBufferEmissionImage;

        ImageViewId m_GBufferEmissionImageView;

        /*VkImage m_DepthImage;
        VkImage m_ColorAttachmentImage;
        VkImage m_GBufferPositionImage;
        VkImage m_GBufferNormalImage;
        VkImage m_GBufferColorImage;
        VkImage m_GBufferMaterialImage;
        VkImage m_GBufferEmissionImage;

        VkImageView m_DepthImageView;
        VkImageView m_ColorAttachmentView;
        VkImageView m_GBufferPositionView;
        VkImageView m_GBufferNormalView;
        VkImageView m_GBufferColorView;
        VkImageView m_GBufferMaterialView;
        VkImageView m_GBufferEmissionView;

        VkDeviceMemory m_DepthImageMemory;
        VkDeviceMemory m_ColorAttachmentImageMemory;
        VkDeviceMemory m_GBufferPositionImageMemory;
        VkDeviceMemory m_GBufferNormalImageMemory;
        VkDeviceMemory m_GBufferColorImageMemory;
        VkDeviceMemory m_GBufferMaterialImageMemory;
        VkDeviceMemory m_GBufferEmissionImageMemory;*/

        std::vector<VkFramebuffer> m_3DFramebuffers;
        std::vector<VkFramebuffer> m_PostProcessingFramebuffers;
        std::vector<VkFramebuffer> m_GUIFramebuffers;

        VkRenderPass m_3DRenderPass;
        VkRenderPass m_PostProcessingRenderPass;
        VkRenderPass m_GUIRenderPass;
    };
}
