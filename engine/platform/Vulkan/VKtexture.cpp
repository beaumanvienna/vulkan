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
        if (m_TextureSlot > -1)
        {
            //Engine::m_TextureSlotManager->RemoveTextureSlot(m_TextureSlot);
        }
        std::cout << "VK_Texture::~VK_Texture() not implemented" << std::endl;
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

    bool VK_Texture::Create()
    {
        LOG_CORE_CRITICAL("m_RendererID: {0}", m_RendererID);
        LOG_CORE_CRITICAL("m_FileName: {0}", m_FileName);
        LOG_CORE_CRITICAL("m_LocalBuffer: {0}", (unsigned long long) m_LocalBuffer);
        LOG_CORE_CRITICAL("m_Width: {0}, m_Height: {1}, m_BytesPerPixel: {2}", m_Width, m_Height, m_BytesPerPixel);
        LOG_CORE_CRITICAL("m_TextureSlot: {0}", m_TextureSlot);
        LOG_CORE_CRITICAL("m_InternalFormat: {0}, m_DataFormat: {1}",  m_InternalFormat, m_DataFormat);
        LOG_CORE_CRITICAL("m_Type: {0}", m_Type);
        
        //free local buffer
        stbi_image_free(m_LocalBuffer);
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
