/* Engine Copyright (c) 2022 Engine Development Team 
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

#pragma once

#include <vulkan/vulkan.h>

#include "engine.h"
#include "renderer/texture.h"

#include "VKdevice.h"
#include "VKtextureSlotManager.h"

namespace GfxRenderEngine
{
    class VK_Texture: public Texture
    {

    public:
        VK_Texture(std::shared_ptr<TextureSlotManager> textureSlotManager, bool nearestFilter = false);
        VK_Texture(std::shared_ptr<TextureSlotManager> textureSlotManager, uint ID, int internalFormat, int dataFormat, int type);
        ~VK_Texture();

        virtual bool Init(const uint width, const uint height, const void* data) override;
        virtual bool Init(const std::string& fileName, bool flip = true) override;
        virtual bool Init(const unsigned char* data, int length) override;
        virtual bool Init(const uint width, const uint height, const uint rendererID) override;
        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual int GetWidth() const override { return m_Width; }
        virtual int GetHeight() const override { return m_Height; }
        virtual uint GetTextureSlot() const override { return m_TextureSlot; }
        virtual void Resize(uint width, uint height) override;
        virtual void Blit(uint x, uint y, uint width, uint height, uint bytesPerPixel, const void* data) override;
        virtual void Blit(uint x, uint y, uint width, uint height, int dataFormat, int type, const void* data) override;
        virtual void SetFilename(const std::string& filename) override { m_FileName = filename;}

    private:

        bool Create();
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                          VkMemoryPropertyFlags properties, VkBuffer& buffer,
                          VkDeviceMemory& bufferMemory);
        void CreateImage(VkFormat format, VkImageTiling tiling,
                         VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void GenerateMipmaps();

    private:

        std::string m_FileName;
        uint m_RendererID;
        uchar* m_LocalBuffer;
        int m_Width, m_Height, m_BytesPerPixel;
        int m_TextureSlot;
        uint m_MipLevels;
        std::shared_ptr<TextureSlotManager> m_TextureSlotManager;

        int m_InternalFormat, m_DataFormat;
        int m_Type;

        VkImage m_TextureImage;
        VkDeviceMemory m_TextureImageMemory;
        VkFormat m_ImageFormat;

        bool m_NearestFilter;
    public:

        VkSampler m_Sampler;
        VkImageView m_TextureView;
        VkImageLayout m_ImageLayout;

    };
}
