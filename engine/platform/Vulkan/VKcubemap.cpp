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
#include "VKcubemap.h"

namespace GfxRenderEngine
{

    VK_Cubemap::VK_Cubemap(bool nearestFilter)
        : m_NearestFilter{nearestFilter}, m_MipLevels{1}, m_Width{0}, m_Height{0}, m_BytesPerPixel{0}, m_sRGB{false}
    {
    }

    VK_Cubemap::~VK_Cubemap()
    {
        auto device = VK_Core::m_Device->Device();

        vkDestroyImage(device, m_CubemapImage, nullptr);
        vkDestroyImageView(device, m_ImageView, nullptr);
        vkDestroySampler(device, m_Sampler, nullptr);
        vkFreeMemory(device, m_CubemapImageMemory, nullptr);
    }

    // create texture from files on disk
    bool VK_Cubemap::Init(const std::vector<std::string>& fileNames, bool sRGB, bool flip /*= true*/)
    {
        stbi_set_flip_vertically_on_load(flip);
        m_FileNames = fileNames;
        m_sRGB = sRGB;
        return Create();
    }

    void VK_Cubemap::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = VK_Core::m_Device->BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_CubemapImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_MipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = NUMBER_OF_CUBEMAP_IMAGES;

        VkPipelineStageFlags sourceStage = 0;
        VkPipelineStageFlags destinationStage = 0;

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
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VK_Core::m_Device->EndSingleTimeCommands(commandBuffer);
        m_ImageLayout = newLayout;
    }

    void VK_Cubemap::CreateImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties)
    {
        auto device = VK_Core::m_Device->Device();

        m_ImageFormat = format;
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_Width;
        imageInfo.extent.height = m_Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = m_MipLevels;
        imageInfo.arrayLayers = NUMBER_OF_CUBEMAP_IMAGES;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        {
            auto result = vkCreateImage(device, &imageInfo, nullptr, &m_CubemapImage);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create image!");
            }
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, m_CubemapImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VK_Core::m_Device->FindMemoryType(memRequirements.memoryTypeBits, properties);

        {
            auto result = vkAllocateMemory(device, &allocInfo, nullptr, &m_CubemapImageMemory);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to allocate image memory in 'void VK_Cubemap::CreateImage'");
            }
        }

        vkBindImageMemory(device, m_CubemapImage, m_CubemapImageMemory, 0);
    }

    void VK_Cubemap::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
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
                CORE_HARD_STOP("failed to create buffer!");
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

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    bool VK_Cubemap::Create()
    {
        auto device = VK_Core::m_Device->Device();

        VkDeviceSize layerSize;
        VkDeviceSize imageSize;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        void* data;
        uint64 memAddress;
        stbi_uc* pixels;

        for (int i = 0; i < NUMBER_OF_CUBEMAP_IMAGES; i++)
        {
            // load all faces
            pixels = stbi_load(m_FileNames[i].c_str(), &m_Width, &m_Height, &m_BytesPerPixel, 4); // 4 == STBI_rgb_alpha
            if (!pixels)
            {
                LOG_CORE_CRITICAL("Texture: Couldn't load file {0}", m_FileNames[i]);
                return false;
            }
            if (i == 0)
            {
                layerSize = m_Width * m_Height * 4;
                imageSize = layerSize * NUMBER_OF_CUBEMAP_IMAGES;

                CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                             stagingBufferMemory);
                vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
                memAddress = reinterpret_cast<uint64>(data);
            }
            memcpy(reinterpret_cast<void*>(memAddress), static_cast<void*>(pixels), static_cast<size_t>(layerSize));
            stbi_image_free(pixels);
            memAddress += layerSize;
        }
        vkUnmapMemory(device, stagingBufferMemory);

        VkFormat format = m_sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        CreateImage(format,                                                       /*VkFormat format                 */
                    VK_IMAGE_TILING_OPTIMAL,                                      /*VkImageTiling tiling            */
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, /*VkImageUsageFlags usage         */
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT                           /*VkMemoryPropertyFlags properties*/
        );

        TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VK_Core::m_Device->CopyBufferToImage(stagingBuffer,               /*VkBuffer buffer*/
                                             m_CubemapImage,              /*VkImage image  */
                                             static_cast<uint>(m_Width),  /*uint width     */
                                             static_cast<uint>(m_Height), /*uint height    */
                                             NUMBER_OF_CUBEMAP_IMAGES     /*uint layerCount*/
        );

        TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        // Create a texture sampler
        // In Vulkan, textures are accessed by samplers
        // This separates sampling information from texture data.
        // This means you could have multiple sampler objects for the same texture with different settings
        // Note: Similar to the samplers available with OpenGL 3.3
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        if (m_NearestFilter)
        {
            samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
            samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
        }
        else
        {
            samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
            samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        }
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
        // Images are not directly accessed by shaders and
        // are abstracted by image views.
        // Image views contain additional
        // information and sub resource ranges
        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        view.format = m_ImageFormat;
        view.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
        // A subresource range describes the set of mip levels (and array layers) that can be accessed through this image
        // view It's possible to create multiple image views for a single image referring to different (and/or overlapping)
        // ranges of the image
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.baseMipLevel = 0;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = NUMBER_OF_CUBEMAP_IMAGES;
        // Linear tiling usually won't support mip maps
        // Only set mip map count if optimal tiling is used
        view.subresourceRange.levelCount = m_MipLevels;
        // The view will be based on the texture's image
        view.image = m_CubemapImage;

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
} // namespace GfxRenderEngine
