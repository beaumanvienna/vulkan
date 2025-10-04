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

#include "VKcore.h"
#include "VKstorageImage.h"

namespace GfxRenderEngine
{
    VK_StorageImage::StorageImageID VK_StorageImage::m_GlobalStorageImageIDCounter = 0;
    std::mutex VK_StorageImage::m_Mutex;

    VK_StorageImage::VK_StorageImage() : m_Width{0}, m_Height{0}
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_StorageImageID = m_GlobalStorageImageIDCounter;
            ++m_GlobalStorageImageIDCounter;
        }
    }

    bool VK_StorageImage::Init(const uint width, const uint height)
    {
        m_Width = width;
        m_Height = height;
        return CreateImage(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    bool VK_StorageImage::CreateImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                      VkMemoryPropertyFlags properties)
    {
        auto device = VK_Core::m_Device->Device();

        // create the image
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_Width;
        imageInfo.extent.height = m_Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format; // must support STORAGE_IMAGE
        imageInfo.tiling = tiling; // good for GPU access
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        {
            auto result = vkCreateImage(device, &imageInfo, nullptr, &m_StorageImage);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create image!");
                return false;
            }
        }

        // allocate and bind memory
        VkMemoryRequirements memReq;
        vkGetImageMemoryRequirements(device, m_StorageImage, &memReq);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = VK_Core::m_Device->FindMemoryType(memReq.memoryTypeBits, properties);
        {
            auto result = vkAllocateMemory(device, &allocInfo, nullptr, &m_StorageImageMemory);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to allocate memory!");
                return false;
            }
        }

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkBindImageMemory(device, m_StorageImage, m_StorageImageMemory, 0);
        }

        // create image view
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_StorageImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        {
            auto result = vkCreateImageView(device, &viewInfo, nullptr, &m_StorageImageView);
            if (result != VK_SUCCESS)
            {
                VK_Core::m_Device->PrintError(result);
                LOG_CORE_CRITICAL("failed to create image view!");
                return false;
            }
        }

        VkCommandBuffer commandBuffer = VK_Core::m_Device->BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_StorageImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.srcAccessMask = 0; // since oldLayout = UNDEFINED
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;

        {
            std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                                 0, nullptr, 0, nullptr, 1, &barrier);
        }

        VK_Core::m_Device->EndSingleTimeCommands(commandBuffer);

        m_StorageImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        m_DescriptorImageInfo.imageView = m_StorageImageView;
        m_DescriptorImageInfo.imageLayout = m_StorageImageLayout;

        return true;
    }

    void VK_StorageImage::Destroy()
    {
        auto device = VK_Core::m_Device->Device();

        std::lock_guard<std::mutex> guard(VK_Core::m_Device->m_DeviceAccessMutex);
        vkDestroyImageView(device, m_StorageImageView, nullptr);
        vkFreeMemory(device, m_StorageImageMemory, nullptr);
        vkDestroyImage(device, m_StorageImage, nullptr);

        m_StorageImageFormat = VK_FORMAT_UNDEFINED;
        m_StorageImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_StorageImage = nullptr;
        m_StorageImageView = nullptr;
        m_StorageImageMemory = nullptr;
    }

    void VK_StorageImage::Resize(uint width, uint height)
    {
        if ((width == m_Width) && (height == m_Height))
        {
            return;
        }
        Destroy();
        Init(width, height);
    }

    VK_StorageImage::~VK_StorageImage() { Destroy(); }
} // namespace GfxRenderEngine
