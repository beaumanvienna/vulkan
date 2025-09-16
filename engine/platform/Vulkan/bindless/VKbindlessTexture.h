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

#include "VKtexture.h"

namespace GfxRenderEngine
{
    class VK_BindlessTexture
    {
    public:
        VK_BindlessTexture();
        ~VK_BindlessTexture();

        // Not copyable or movable
        VK_BindlessTexture(const VK_BindlessTexture&) = delete;
        VK_BindlessTexture& operator=(const VK_BindlessTexture&) = delete;
        VK_BindlessTexture(VK_BindlessTexture&&) = delete;
        VK_BindlessTexture& operator=(VK_BindlessTexture&&) = delete;

        Texture::BindlessTextureID AddTexture(Texture* texture);
        void UpdateBindlessDescriptorSets();

        [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_BindlessTextureSetLayout; }
        [[nodiscard]] VkDescriptorSet GetDescriptorSet() const { return m_BindlessSetTextures; }
        [[nodiscard]] Texture::BindlessTextureID GetTextureCount() const { return m_NextBindlessIndex; }
        [[nodiscard]] Texture::BindlessTextureID GetMaxDescriptors() const { return MAX_DESCRIPTOR; }

    private:
        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSet();

    private:
        constexpr static Texture::BindlessTextureID MAX_DESCRIPTOR = 16384u;
        constexpr static Texture::BindlessTextureID BINDLESS_ID_TEXTURE_ATLAS = 0u;
        Texture::BindlessTextureID m_NextBindlessIndex;
        VkDescriptorSetLayout m_BindlessTextureSetLayout;
        VkDescriptorPool m_DescriptorPoolTextures;
        VkDescriptorSet m_BindlessSetTextures;
        std::mutex m_TableAccessMutex; // protect shared data

        // Map texture ID to bindless index
        constexpr static size_t TEXTURE_ID_2_BINDLESS_ID_PREALLOC = 4096u;
        std::unordered_map<Texture::TextureID, Texture::BindlessTextureID> m_TextureID2BindlessID;
        constexpr static size_t PENDING_UPDATES_PREALLOC = 256u;
        std::vector<Texture*> m_PendingUpdates;
    };
} // namespace GfxRenderEngine
