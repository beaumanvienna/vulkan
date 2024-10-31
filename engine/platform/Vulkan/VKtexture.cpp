/* Engine Copyright (c) 2024 Engine Development Team
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

#include "core.h"
#include "stb_image.h"

#include "VKcore.h"
#include "VKtexture.h"

namespace GfxRenderEngine
{

    VK_Texture::VK_Texture(bool nearestFilter)
        : m_FileName(""), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BytesPerPixel(0), m_MipLevels(0), m_sRGB(false)
    {
        nearestFilter ? m_MinFilter = VK_FILTER_NEAREST : m_MinFilter = VK_FILTER_LINEAR;
        nearestFilter ? m_MagFilter = VK_FILTER_NEAREST : m_MagFilter = VK_FILTER_LINEAR;
        m_MinFilterMip = VK_FILTER_LINEAR;
    }

    VK_Texture::~VK_Texture()
    {
        auto device = VK_Core::m_Device->Device();

        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
        vkDestroyImage(device, m_TextureImage, nullptr);
        vkDestroyImageView(device, m_ImageView, nullptr);
        vkDestroySampler(device, m_Sampler, nullptr);
        vkFreeMemory(device, m_TextureImageMemory, nullptr);
    }

    // create texture from raw memory
    bool VK_Texture::Init(const uint width, const uint height, bool sRGB, const void* data, int minFilter, int magFilter)
    {
        ZoneScopedNC("VK_Texture::Init", 0xffff00);
        bool ok = false;
        m_FileName = "raw memory";
        m_sRGB = sRGB;
        m_LocalBuffer = (uchar*)data;
        m_MinFilter = SetFilter(minFilter);
        m_MagFilter = SetFilter(magFilter);
        m_MinFilterMip = SetFilterMip(minFilter);

        if (m_LocalBuffer)
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

        if (m_LocalBuffer)
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

        if (m_LocalBuffer)
        {
            ok = Create();
            stbi_image_free(m_LocalBuffer);
        }
        else
        {
            LOG_CORE_CRITICAL("Texture: Couldn't load file {0}", m_FileName);
        }
        return ok;
    }

    void VK_Texture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = VK_Core::m_Device->BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_TextureImage;
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

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }

        VK_Core::m_Device->EndSingleTimeCommands(commandBuffer);
    }

    void VK_Texture::CreateImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties)
    {
        auto device = VK_Core::m_Device->Device();
        m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

        m_ImageFormat = format;
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_Width;
        imageInfo.extent.height = m_Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = m_MipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        {
            auto result = vkCreateImage(device, &imageInfo, nullptr, &m_TextureImage);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create image!");
            }
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, m_TextureImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VK_Core::m_Device->FindMemoryType(memRequirements.memoryTypeBits, properties);

        {
            auto result = vkAllocateMemory(device, &allocInfo, nullptr, &m_TextureImageMemory);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to allocate image memory in 'void "
                                  "VK_Texture::CreateImage'");
            }
        }
        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkBindImageMemory(device, m_TextureImage, m_TextureImageMemory, 0);
        }
    }

    void VK_Texture::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                  VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        auto device = VK_Core::m_Device->Device();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        {
            auto result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create buffer!");
            }
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VK_Core::m_Device->FindMemoryType(memRequirements.memoryTypeBits, properties);

        {
            auto result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to allocate buffer memory!");
            }
        }
        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkBindBufferMemory(device, buffer, bufferMemory, 0);
        }
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

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                     stagingBufferMemory);

        void* data;
        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        }
        memcpy(data, m_LocalBuffer, static_cast<size_t>(imageSize));
        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkUnmapMemory(device, stagingBufferMemory);
        }

        VkFormat format = m_sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        CreateImage(format, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VK_Core::m_Device->CopyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint>(m_Width),
                                             static_cast<uint>(m_Height), 1 /*layerCount*/
        );

        GenerateMipmaps();

        m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }

        // Create a texture sampler
        // In Vulkan, textures are accessed by samplers
        // This separates sampling information from texture data.
        // This means you could have multiple sampler objects for the same
        // texture with different settings Note: Similar to the samplers
        // available with OpenGL 3.3
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = m_MagFilter;
        samplerCreateInfo.minFilter = m_MinFilter;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = static_cast<float>(m_MipLevels);
        samplerCreateInfo.maxAnisotropy = 4.0;
        samplerCreateInfo.anisotropyEnable = VK_TRUE;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        {
            auto result = vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_Sampler);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create sampler!");
            }
        }

        // Create image view
        // Textures are not directly accessed by shaders and
        // are abstracted by image views.
        // Image views contain additional
        // information and sub resource ranges
        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = m_ImageFormat;
        view.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
        // A subresource range describes the set of mip levels (and array layers) that can be accessed through this image
        // view It's possible to create multiple image views for a single image referring to different (and/or overlapping)
        // ranges of the image
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.baseMipLevel = 0;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 1;
        // Linear tiling usually won't support mip maps
        // Only set mip map count if optimal tiling is used
        view.subresourceRange.levelCount = m_MipLevels;
        // The view will be based on the texture's image
        view.image = m_TextureImage;

        {
            auto result = vkCreateImageView(device, &view, nullptr, &m_ImageView);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create image view!");
            }
        }

        m_DescriptorImageInfo.sampler = m_Sampler;
        m_DescriptorImageInfo.imageView = m_ImageView;
        m_DescriptorImageInfo.imageLayout = m_ImageLayout;

        return true;
    }

    void VK_Texture::Blit(uint x, uint y, uint width, uint height, uint bytesPerPixel, const void* data)
    {
        LOG_CORE_CRITICAL("not implemented void VK_Texture::Blit(uint x, uint y, uint width, uint height, uint "
                          "bytesPerPixel, const void* data)");
    }

    void VK_Texture::Blit(uint x, uint y, uint width, uint height, int dataFormat, int type, const void* data)
    {
        LOG_CORE_CRITICAL("not implemented void VK_Texture::Blit(uint x, uint y, uint width, uint height, int dataFormat, "
                          "int type, const void* data)");
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

        VkCommandBuffer commandBuffer = VK_Core::m_Device->BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = m_TextureImage;
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

            {
                std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                     nullptr, 0, nullptr, 1, &barrier);
            }

            VkImageBlit blit{};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            {
                std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
                vkCmdBlitImage(commandBuffer, m_TextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_TextureImage,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, m_MinFilterMip);
            }

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            {
                std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                     0, nullptr, 0, nullptr, 1, &barrier);
            }

            if (mipWidth > 1)
            {
                mipWidth /= 2;
            }
            if (mipHeight > 1)
            {
                mipHeight /= 2;
            }
        }

        barrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                                 nullptr, 0, nullptr, 1, &barrier);
        }

        VK_Core::m_Device->EndSingleTimeCommands(commandBuffer);
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
            case TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            {
                break;
            }
            case TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            {
                break;
            }
                {
                    filter = VK_FILTER_NEAREST;
                    break;
                }
        }
        return filter;
    }
} // namespace GfxRenderEngine
