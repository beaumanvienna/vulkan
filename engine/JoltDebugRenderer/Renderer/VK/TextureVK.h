// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

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

#pragma once

#include <JoltDebugRenderer/Renderer/Texture.h>

#include <vulkan/vulkan.h>

namespace JPH
{

    class RendererVK;

    class TextureVK : public Texture
    {
    public:
        /// Constructor, called by Renderer::CreateTextureVK
        TextureVK(RendererVK* inRenderer, const Surface* inSurface);  // Create a normal TextureVK
        TextureVK(RendererVK* inRenderer, int inWidth, int inHeight); // Create a render target (depth only)
        virtual ~TextureVK() override;

        /// Bind texture to the pixel shader
        virtual void Bind() const override;

        VkImageView GetImageView() const { return mImageView; }

    private:
        void CreateImageViewAndDescriptorSet(VkFormat inFormat, VkImageAspectFlags inAspectFlags, VkSampler inSampler);
        void TransitionImageLayout(VkCommandBuffer inCommandBuffer, VkImage inImage, VkFormat inFormat,
                                   VkImageLayout inOldLayout, VkImageLayout inNewLayout);

        RendererVK* mRenderer;
        VkImage mImage = VK_NULL_HANDLE;
        VkDeviceMemory mImageMemory = VK_NULL_HANDLE;
        VkImageView mImageView = VK_NULL_HANDLE;
        VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
    };
} // namespace JPH