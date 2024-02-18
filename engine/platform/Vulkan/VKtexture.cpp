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

#include <string>

#include "stb_image.h"
#include "core.h"

#include "VKcore.h"
#include "VKtexture.h"
#include "VKbuffer.h"

namespace GfxRenderEngine
{

    VK_Texture::VK_Texture(bool nearestFilter)
        : m_FileName(""), m_RendererID(0), m_LocalBuffer(nullptr), m_Type(0),
          m_Width(0), m_Height(0), m_BytesPerPixel(0), m_InternalFormat(0),
          m_DataFormat(0), m_MipLevels(0), m_sRGB(false)
    {
        nearestFilter ? m_MinFilter = VK_FILTER_NEAREST : m_MinFilter = VK_FILTER_LINEAR;
        nearestFilter ? m_MagFilter = VK_FILTER_NEAREST : m_MagFilter = VK_FILTER_LINEAR;
        m_MinFilterMip = VK_FILTER_LINEAR;
    }

    VK_Texture::~VK_Texture()
    {
        auto device = VK_Core::m_Device->Device();

        VK_Core::m_Device->DestroySampler(m_Sampler);
        VK_Core::m_Device->DestroyImage(m_TextureImage);
    }

    VK_Texture::VK_Texture(uint ID, int internalFormat, int dataFormat, int type)
        : m_RendererID{ID}, m_InternalFormat{internalFormat},
          m_DataFormat{dataFormat}, m_Type{type}, m_sRGB{false}
    {
    }

    // create texture from raw memory
    bool VK_Texture::Init(const uint width, const uint height, bool sRGB, const void* data, int minFilter, int magFilter)
    {
        bool ok = false;
        m_FileName = "raw memory";
        m_sRGB = sRGB;
        m_LocalBuffer = (uchar*)data;
        m_MinFilter = SetFilter(minFilter);
        m_MagFilter = SetFilter(magFilter);
        m_MinFilterMip = SetFilterMip(minFilter);

        if(m_LocalBuffer)
        {
            m_Width = width;
            m_Height = height;
            m_BytesPerPixel = 4;
            ok = Create();
        }
        return ok;
    }

    // create texture from file on disk
    bool VK_Texture::Init(const std::string& fileName, bool sRGB, bool flip)
    {
        bool ok = false;
        stbi_set_flip_vertically_on_load(flip);
        m_FileName = fileName;
        m_sRGB = sRGB;
        m_LocalBuffer = stbi_load(m_FileName.c_str(), &m_Width, &m_Height, &m_BytesPerPixel, 4);

        if(m_LocalBuffer)
        {
            ok = Create();
            stbi_image_free(m_LocalBuffer);
        }
        else
        {
            LOG_CORE_CRITICAL("Texture: Couldn't load file {0}", fileName);
        }
        return ok;
    }

    // create texture from file in memory
    bool VK_Texture::Init(const unsigned char* data, int length, bool sRGB)
    {
        bool ok = false;
        stbi_set_flip_vertically_on_load(true);
        m_FileName = "file in memory";
        m_sRGB = sRGB;
        m_LocalBuffer = stbi_load_from_memory(data, length, &m_Width, &m_Height, &m_BytesPerPixel, 4);

        if(m_LocalBuffer)
        {
            ok = Create();
            stbi_image_free(m_LocalBuffer);
        }
        else
        {
            std::cout << "Texture: Couldn't load file " << m_FileName << std::endl;
        }
        return ok;
    }

    void VK_Texture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = VK_Core::m_Device->BeginSingleTimeCommands(QueueTypes::GRAPHICS);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = VK_Core::m_Device->GetImageSlot(m_TextureImage).vkImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_MipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } 
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            LOG_APP_CRITICAL("unsupported layout transition!");
            return;
        }

        vkCmdPipelineBarrier
        (
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        VK_Core::m_Device->EndSingleTimeCommands(commandBuffer, QueueTypes::GRAPHICS);
    }

    bool VK_Texture::Create()
    {
        auto device = VK_Core::m_Device->Device();

        VkDeviceSize imageSize = m_Width * m_Height * 4;

        if (!m_LocalBuffer)
        {
            LOG_CORE_CRITICAL("failed to load texture image!");
            return false;
        }

        std::unique_ptr<VK_Buffer> stagingBuffer = std::make_unique<VK_Buffer>(imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryFlagBits::HOST_ACCESS_RANDOM);

        memcpy(stagingBuffer->GetMappedMemory(), m_LocalBuffer, static_cast<size_t>(imageSize));

        m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
        m_ImageFormat = static_cast<VkFormat>(m_sRGB ? Format::R8G8B8A8_SRGB : Format::R8G8B8A8_UNORM);
        
        m_TextureImage = VK_Core::m_Device->CreateImage({
            .format = static_cast<Format>(m_ImageFormat),
            .size = { static_cast<uint>(m_Width), static_cast<uint>(m_Height), 1 },
            .mipLevelCount = m_MipLevels,
            .usage = ImageUsageFlagBits::TRANSFER_SRC | ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::SHADER_SAMPLED,
            .memoryFlags = MemoryFlagBits::DEDICATED_MEMORY
            });

        TransitionImageLayout
        (
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );

        VK_Core::m_Device->CopyBufferToImage
        (
            stagingBuffer->GetBuffer(),
            VK_Core::m_Device->GetImageSlot(m_TextureImage).vkImage,
            static_cast<uint>(m_Width), 
            static_cast<uint>(m_Height),
            1 /*layerCount*/
        );

        GenerateMipmaps();

        m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Create a texture sampler
        // In Vulkan, textures are accessed by samplers
        // This separates sampling information from texture data.
        // This means you could have multiple sampler objects for the same texture with different settings
        // Note: Similar to the samplers available with OpenGL 3.3

        m_Sampler = VK_Core::m_Device->CreateSampler({
            .magnificationFilter = static_cast<Filter>(m_MagFilter),
            .minificationFilter = static_cast<Filter>(m_MinFilter),
            .addressModeU = SamplerAddressMode::REPEAT,
            .addressModeV = SamplerAddressMode::REPEAT,
            .addressModeW = SamplerAddressMode::REPEAT,
            .mipLodBias = 0.0f,
            .enableAnisotropy = true,
            .maxAnisotropy = 4.0f,
            .maxLod = static_cast<float>(m_MipLevels),
            .borderColor = BorderColor::FLOAT_OPAQUE_WHITE
            });

        m_DescriptorImageInfo.sampler     = VK_Core::m_Device->GetSamplerSlot(m_Sampler).vkSampler;
        m_DescriptorImageInfo.imageView   = VK_Core::m_Device->GetImageViewSlot(m_TextureImage.defaultView()).vkImageView;
        m_DescriptorImageInfo.imageLayout = m_ImageLayout;

        return true;
    }

    void VK_Texture::Blit(uint x, uint y, uint width, uint height, uint bytesPerPixel, const void* data)
    {
        LOG_CORE_CRITICAL("not implemented void VK_Texture::Blit(uint x, uint y, uint width, uint height, uint bytesPerPixel, const void* data)");
    }

    void VK_Texture::Blit(uint x, uint y, uint width, uint height, int dataFormat, int type, const void* data)
    {
        LOG_CORE_CRITICAL("not implemented void VK_Texture::Blit(uint x, uint y, uint width, uint height, int dataFormat, int type, const void* data)");
    }

    void VK_Texture::Resize(uint width, uint height)
    {
        LOG_CORE_CRITICAL("not implemented void VK_Texture::Resize(uint width, uint height)");
    }

    void VK_Texture::GenerateMipmaps()
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(VK_Core::m_Device->PhysicalDevice(), m_ImageFormat, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            LOG_CORE_WARN("texture image format does not support linear blitting!");
            return;
        }

        VkCommandBuffer commandBuffer = VK_Core::m_Device->BeginSingleTimeCommands(QueueTypes::GRAPHICS);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = VK_Core::m_Device->GetImageSlot(m_TextureImage).vkImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = m_Width;
        int32_t mipHeight = m_Height;

        for (uint i = 1; i < m_MipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer, VK_Core::m_Device->GetImageSlot(m_TextureImage).vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_Core::m_Device->GetImageSlot(m_TextureImage).vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, m_MinFilterMip);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VK_Core::m_Device->EndSingleTimeCommands(commandBuffer, QueueTypes::GRAPHICS);

    }
    
    VkFilter VK_Texture::SetFilter(int minMagFilter)
    {
        VkFilter filter = VK_FILTER_LINEAR;
        switch (minMagFilter)
        {
            case TEXTURE_FILTER_NEAREST:
            case TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            case TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            {
                filter = VK_FILTER_NEAREST;
                break;
            }
        }
        return filter;
    }

    VkFilter VK_Texture::SetFilterMip(int minFilter)
    {
        VkFilter filter = VK_FILTER_LINEAR;
        switch (minFilter)
        {
            case TEXTURE_FILTER_NEAREST:
            case TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: { break; }
            case TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST: { break; }
            {
                filter = VK_FILTER_NEAREST;
                break;
            }
        }
        return filter;
    }
}
