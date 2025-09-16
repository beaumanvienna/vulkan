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

#include "engine.h"

#include "VKstorageImage.h"

namespace GfxRenderEngine
{
    class VK_BindlessImage
    {
    public:
        VK_BindlessImage();
        ~VK_BindlessImage();

        // Not copyable or movable
        VK_BindlessImage(const VK_BindlessImage&) = delete;
        VK_BindlessImage& operator=(const VK_BindlessImage&) = delete;
        VK_BindlessImage(VK_BindlessImage&&) = delete;
        VK_BindlessImage& operator=(VK_BindlessImage&&) = delete;

        StorageImage::BindlessImageID AddImage(StorageImage* storageImage);
        void UpdateBindlessDescriptorSets();

        [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_BindlessImageSetLayout; }
        [[nodiscard]] VkDescriptorSet GetDescriptorSet() const { return m_BindlessSetImages; }
        [[nodiscard]] StorageImage::BindlessImageID GetImageCount() const { return m_NextBindlessIndex; }
        [[nodiscard]] StorageImage::BindlessImageID GetMaxDescriptors() const { return MAX_DESCRIPTOR; }

    private:
        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSet();

    private:
        constexpr static StorageImage::BindlessImageID MAX_DESCRIPTOR = 16384u;
        constexpr static StorageImage::BindlessImageID BINDLESS_ID_TEXTURE_ATLAS = 0u;
        StorageImage::BindlessImageID m_NextBindlessIndex;
        VkDescriptorSetLayout m_BindlessImageSetLayout;
        VkDescriptorPool m_DescriptorPoolImages;
        VkDescriptorSet m_BindlessSetImages;
        std::mutex m_TableAccessMutex; // protect shared data

        // Map texture ID to bindless index
        constexpr static size_t TEXTURE_ID_2_BINDLESS_ID_PREALLOC = 4096u;
        std::unordered_map<StorageImage::StorageImageID, StorageImage::BindlessImageID> m_ImageID2BindlessImageID;
        constexpr static size_t PENDING_UPDATES_PREALLOC = 256u;
        std::vector<StorageImage*> m_PendingUpdates;
    };
} // namespace GfxRenderEngine
