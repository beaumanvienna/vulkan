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
#include "renderer/texture.h"

#include "VKdevice.h"

namespace GfxRenderEngine
{
    class VK_Texture : public Texture
    {

    public:
        VK_Texture(bool nearestFilter = false);
        virtual ~VK_Texture();

        virtual bool Init(const uint width, const uint height, bool sRGB, const void* data, int minFilter,
                          int magFilter) override;
        virtual bool Init(const std::string& fileName, bool sRGB, bool flip = true) override;
        virtual bool Init(const unsigned char* data, int length, bool sRGB) override;
        virtual int GetWidth() const override { return m_Width; }
        virtual int GetHeight() const override { return m_Height; }
        virtual void Resize(uint width, uint height) override;
        virtual void Blit(uint x, uint y, uint width, uint height, uint bytesPerPixel, const void* data) override;
        virtual void Blit(uint x, uint y, uint width, uint height, int dataFormat, int type, const void* data) override;
        virtual void SetFilename(const std::string& filename) override { m_FileName = filename; }

        const VkDescriptorImageInfo& GetDescriptorImageInfo() const { return m_DescriptorImageInfo; }

    private:
        bool Create();
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                          VkDeviceMemory& bufferMemory);
        void CreateImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void GenerateMipmaps();

        VkFilter SetFilter(int minMagFilter);
        VkFilter SetFilterMip(int minFilter);

    private:
        std::string m_FileName;
        uchar* m_LocalBuffer;
        int m_Width, m_Height, m_BytesPerPixel;
        uint m_MipLevels;

        bool m_sRGB;
        VkFilter m_MinFilter;
        VkFilter m_MagFilter;
        VkFilter m_MinFilterMip;

        VkFormat m_ImageFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VkImage m_TextureImage{nullptr};
        VkDeviceMemory m_TextureImageMemory{nullptr};
        VkImageLayout m_ImageLayout{VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageView m_ImageView{nullptr};
        VkSampler m_Sampler{nullptr};

        VkDescriptorImageInfo m_DescriptorImageInfo{};

    private:
        static constexpr int TEXTURE_FILTER_NEAREST = 9728;
        static constexpr int TEXTURE_FILTER_LINEAR = 9729;
        static constexpr int TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST = 9984;
        static constexpr int TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST = 9985;
        static constexpr int TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR = 9986;
        static constexpr int TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR = 9987;
    };
} // namespace GfxRenderEngine
