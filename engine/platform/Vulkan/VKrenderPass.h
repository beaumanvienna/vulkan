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

        static constexpr int NUMBER_OF_GBUFFER_ATTACHMENTS =
            (int)RenderTargets3D::NUMBER_OF_ATTACHMENTS - (int)RenderTargets3D::ATTACHMENT_GBUFFER_POSITION;
        static constexpr int NUMBER_OF_POSTPROCESSING_INPUT_ATTACHMENTS =
            (int)RenderTargetsPostProcessing::NUMBER_OF_ATTACHMENTS -
            (int)RenderTargetsPostProcessing::INPUT_ATTACHMENT_3DPASS_COLOR;
        static constexpr int NUMBER_OF_POSTPROCESSING_OUPUT_ATTACHMENTS =
            (int)RenderTargetsPostProcessing::INPUT_ATTACHMENT_3DPASS_COLOR; // 1st input attachment == number of output
                                                                             // attachments

    public:
        VK_RenderPass(VK_SwapChain* swapChain);
        ~VK_RenderPass();

        VK_RenderPass(const VK_RenderPass&) = delete;
        VK_RenderPass& operator=(const VK_RenderPass&) = delete;

        VkImageView GetImageViewColorAttachment() { return m_ColorAttachmentView; }
        VkImageView GetImageViewGBufferPosition() { return m_GBufferPositionView; }
        VkImageView GetImageViewGBufferNormal() { return m_GBufferNormalView; }
        VkImageView GetImageViewGBufferColor() { return m_GBufferColorView; }
        VkImageView GetImageViewGBufferMaterial() { return m_GBufferMaterialView; }
        VkImageView GetImageViewGBufferEmission() { return m_GBufferEmissionView; }

        VkImage GetImageEmission() const { return m_GBufferEmissionImage; }
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
        VK_Device* m_Device;
        VK_SwapChain* m_SwapChain;     // constructor initialized
        VkExtent2D m_RenderPassExtent; // constructor initialized

        VkFormat m_DepthFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VkFormat m_BufferPositionFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VkFormat m_BufferNormalFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VkFormat m_BufferColorFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VkFormat m_BufferMaterialFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VkFormat m_BufferEmissionFormat{VkFormat::VK_FORMAT_UNDEFINED};

        VkImage m_DepthImage{nullptr};
        VkImage m_ColorAttachmentImage{nullptr};
        VkImage m_GBufferPositionImage{nullptr};
        VkImage m_GBufferNormalImage{nullptr};
        VkImage m_GBufferColorImage{nullptr};
        VkImage m_GBufferMaterialImage{nullptr};
        VkImage m_GBufferEmissionImage{nullptr};

        VkImageView m_DepthImageView{nullptr};
        VkImageView m_ColorAttachmentView{nullptr};
        VkImageView m_GBufferPositionView{nullptr};
        VkImageView m_GBufferNormalView{nullptr};
        VkImageView m_GBufferColorView{nullptr};
        VkImageView m_GBufferMaterialView{nullptr};
        VkImageView m_GBufferEmissionView{nullptr};

        VkDeviceMemory m_DepthImageMemory{nullptr};
        VkDeviceMemory m_ColorAttachmentImageMemory{nullptr};
        VkDeviceMemory m_GBufferPositionImageMemory{nullptr};
        VkDeviceMemory m_GBufferNormalImageMemory{nullptr};
        VkDeviceMemory m_GBufferColorImageMemory{nullptr};
        VkDeviceMemory m_GBufferMaterialImageMemory{nullptr};
        VkDeviceMemory m_GBufferEmissionImageMemory{nullptr};

        std::vector<VkFramebuffer> m_3DFramebuffers;
        std::vector<VkFramebuffer> m_PostProcessingFramebuffers;
        std::vector<VkFramebuffer> m_GUIFramebuffers;

        VkRenderPass m_3DRenderPass{nullptr};
        VkRenderPass m_PostProcessingRenderPass{nullptr};
        VkRenderPass m_GUIRenderPass{nullptr};
    };
} // namespace GfxRenderEngine
