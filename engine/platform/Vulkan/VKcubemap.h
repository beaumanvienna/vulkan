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
#include "renderer/cubemap.h"

#include "VKdevice.h"

namespace GfxRenderEngine
{
    class VK_Cubemap : public Cubemap
    {

    public:
        VK_Cubemap(bool nearestFilter = false);
        virtual ~VK_Cubemap();

        virtual bool Init(const std::vector<std::string>& fileNames, bool sRGB, bool flip = false) override;
        virtual int GetWidth() const override { return m_Width; }
        virtual int GetHeight() const override { return m_Height; }

        const VkDescriptorImageInfo GetDescriptorImageInfo() const { return m_DescriptorImageInfo; }

    private:
        bool Create();
        void CreateImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                          VkDeviceMemory& bufferMemory);
        void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

    private:
        static constexpr int NUMBER_OF_CUBEMAP_IMAGES = 6;

        std::vector<std::string> m_FileNames{};
        int m_Width, m_Height, m_BytesPerPixel;
        uint m_MipLevels;
        bool m_NearestFilter, m_sRGB;

        VkFormat m_ImageFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VkImage m_CubemapImage{nullptr};
        VkDeviceMemory m_CubemapImageMemory{nullptr};
        VkImageLayout m_ImageLayout{VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageView m_ImageView{nullptr};
        VkSampler m_Sampler{nullptr};

        VkDescriptorImageInfo m_DescriptorImageInfo{};
    };
} // namespace GfxRenderEngine
