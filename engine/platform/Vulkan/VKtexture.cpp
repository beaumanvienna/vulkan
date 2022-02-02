/* Engine Copyright (c) 2021 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

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

namespace GfxRenderEngine
{

    VK_Texture::VK_Texture()
        : m_FileName(""), m_RendererID(0), m_LocalBuffer(nullptr), m_Type(0),
        m_Width(0), m_Height(0), m_BytesPerPixel(0), m_InternalFormat(0), m_DataFormat(0)
    {
        m_TextureSlot = -1;
    }

    VK_Texture::~VK_Texture()
    {
        auto device = VK_Core::m_Device->Device();
        if (m_TextureSlot > -1)
        {
            //Engine::m_TextureSlotManager->RemoveTextureSlot(m_TextureSlot);
        }
        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);
    }

    VK_Texture::VK_Texture(uint ID, int internalFormat, int dataFormat, int type)
    {
        //m_TextureSlot = Engine::m_TextureSlotManager->GetTextureSlot();
        m_RendererID = ID;
        m_InternalFormat = internalFormat;
        m_DataFormat = dataFormat;
        m_Type = type;
    }

    // create texture from raw memory
    bool VK_Texture::Init(const uint width, const uint height, const void* data)
    {
        bool ok = false;
        m_FileName = "raw memory";
        m_LocalBuffer = (uchar*)data;

        if(m_LocalBuffer)
        {
            ok = true;
            m_Width = width;
            m_Height = height;
            LOG_CORE_CRITICAL("not implemented");
        }
        return ok;
    }

    // create texture from file on disk
    bool VK_Texture::Init(const std::string& fileName)
    {
        bool ok = false;
        int channels_in_file;
        stbi_set_flip_vertically_on_load(true);
        m_FileName = fileName;
        m_LocalBuffer = stbi_load(m_FileName.c_str(), &m_Width, &m_Height, &m_BytesPerPixel, 4);

        if(m_LocalBuffer)
        {
            ok = Create();
        }
        else
        {
            std::cout << "Texture: Couldn't load file " << m_FileName << std::endl;
        }
        return ok;
    }

    // create texture from file in memory
    bool VK_Texture::Init(const unsigned char* data, int length)
    {
        bool ok = false;
        int channels_in_file;
        stbi_set_flip_vertically_on_load(true);
        m_FileName = "file in memory";
        m_LocalBuffer = stbi_load_from_memory(data, length, &m_Width, &m_Height, &m_BytesPerPixel, 4);

        if(m_LocalBuffer)
        {
            ok = Create();
        }
        else
        {
            std::cout << "Texture: Couldn't load file " << m_FileName << std::endl;
        }
        return ok;
    }

    // create texture from framebuffer attachment
    bool VK_Texture::Init(const uint width, const uint height, const uint rendererID)
    {
        LOG_CORE_CRITICAL("not implemented");
        return true;
    }

    void VK_Texture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = VK_Core::m_Device->BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
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

        VK_Core::m_Device->EndSingleTimeCommands(commandBuffer);
    }

    void VK_Texture::CreateImage
    (
        uint width, uint height, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
        VkImage& image, VkDeviceMemory& imageMemory
    )
    {
        auto device = VK_Core::m_Device->Device();

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        {
            auto result = vkCreateImage(device, &imageInfo, nullptr, &image);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to create image!");
            }
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VK_Core::m_Device->FindMemoryType(memRequirements.memoryTypeBits, properties);

        {
            auto result = vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory);
            if (result != VK_SUCCESS)
            {
                LOG_CORE_CRITICAL("failed to allocate image memory!");
            }
        }

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    void VK_Texture::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        auto device = VK_Core::m_Device->Device();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VK_Core::m_Device->FindMemoryType(memRequirements.memoryTypeBits, properties);

        auto result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
        if (result != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
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
        CreateBuffer
        (
            imageSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        void* data;

        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, m_LocalBuffer, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        stbi_image_free(m_LocalBuffer);

        CreateImage
        (
            m_Width,
            m_Height,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            textureImage,
            textureImageMemory
        );

        TransitionImageLayout
        (
            textureImage,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );

        VK_Core::m_Device->CopyBufferToImage
        (
            stagingBuffer,
            textureImage, 
            static_cast<uint>(m_Width), 
            static_cast<uint>(m_Height),
            1
        );

        TransitionImageLayout
        (
            textureImage,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        return true;
    }

    void VK_Texture::Bind() const
    {
        LOG_CORE_CRITICAL("not implemented");
    }

    void VK_Texture::Unbind() const
    {
        LOG_CORE_CRITICAL("not implemented");
    }

    void VK_Texture::Blit(uint x, uint y, uint width, uint height, uint bytesPerPixel, const void* data)
    {
        LOG_CORE_CRITICAL("not implemented");
    }

    void VK_Texture::Blit(uint x, uint y, uint width, uint height, int dataFormat, int type, const void* data)
    {
        LOG_CORE_CRITICAL("not implemented");
    }

    void VK_Texture::Resize(uint width, uint height)
    {
        LOG_CORE_CRITICAL("not implemented");
    }
}
