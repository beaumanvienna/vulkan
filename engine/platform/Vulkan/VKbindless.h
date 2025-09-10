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
    class VK_Bindless
    {
    public:
        VK_Bindless();
        ~VK_Bindless();

        // Not copyable or movable
        VK_Bindless(const VK_Bindless&) = delete;
        VK_Bindless& operator=(const VK_Bindless&) = delete;
        VK_Bindless(VK_Bindless&&) = delete;
        VK_Bindless& operator=(VK_Bindless&&) = delete;

        uint AddTexture(Texture* texture);
        void UpdateBindlessDescriptorSets();

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_BindlessTextureSetLayout; }
        VkDescriptorSet GetDescriptorSet() const { return m_BindlessSetTextures; }
        uint GetTextureCount() const { return m_NextBindlessIndex; }
        uint GetMaxDescriptors() const { return MAX_DESCRIPTOR; }

    private:
        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSet();

    private:
        constexpr static int MAX_DESCRIPTOR = 16384;
        uint m_NextBindlessIndex;
        VkDescriptorSetLayout m_BindlessTextureSetLayout;
        VkDescriptorPool m_DescriptorPoolTextures;
        VkDescriptorSet m_BindlessSetTextures;
        std::mutex m_Mutex; // protect shared data

        // Map texture ID (e.g., from your asset manager) to bindless index
        std::unordered_map<uint, uint> m_TextureIndexMap;
        std::vector<Texture*> m_PendingUpdates;
    };
} // namespace GfxRenderEngine
