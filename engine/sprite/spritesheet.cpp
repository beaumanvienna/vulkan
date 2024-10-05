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

#include "engine.h"
#include "core.h"
#include "sprite/spritesheet.h"
#include "resources/atlas/atlas.cpp"
#include "resources/resources.h"

namespace GfxRenderEngine
{
    SpriteSheet::SpriteSheet() : m_Rows{0}, m_Columns{0}
    {
        m_TextureAtlas = Engine::m_Engine->GetRenderer()->GetTextureAtlas();
    }

    SpriteSheet::~SpriteSheet() {}

    void SpriteSheet::AddSpritesheet()
    {
        m_Texture = m_TextureAtlas;
        m_SpriteTable.reserve(m_SpriteTable.size() + atlas.num_images);
        for (int i = 0; i < atlas.num_images; ++i)
        {
            bool rotated = images[i].rotation;
            m_SpriteTable.emplace_back(images[i].u1, images[i].v1, images[i].u2, images[i].v2, images[i].w, images[i].h,
                                       m_Texture, images[i].name, 1.0f, rotated);
        }
    }

    bool SpriteSheet::AddSpritesheet(const char* path /* GNU */, int resourceID /* MSVC */,
                                     const std::string& resourceClass /* MSVC */)
    {
        m_Texture = Texture::Create();
        size_t fileSize;
        const uchar* data = (const uchar*)ResourceSystem::GetDataPointer(fileSize, path, resourceID, resourceClass);
        bool ok = m_Texture->Init(data, fileSize, Texture::USE_SRGB);
        if (ok)
        {
            AddSpritesheet();
        }
        return ok;
    }

    bool SpriteSheet::AddSpritesheet(const std::string& fileName)
    {
        m_Texture = Texture::Create();
        bool ok = m_Texture->Init(fileName, Texture::USE_SRGB);
        if (ok)
        {
            AddSpritesheet();
        }
        return ok;
    }

    // from existing sprite
    bool SpriteSheet::AddSpritesheetTile(const Sprite& originalSprite, const std::string& mapName, uint rows, uint columns,
                                         uint spacing, const float scale)
    {
        m_Texture = originalSprite.m_Texture;
        m_Rows = rows;
        m_Columns = columns;

        int tileWidth = (originalSprite.GetWidth() - spacing * (columns - 1)) / columns;
        int tileHeight = (originalSprite.GetHeight() - spacing * (rows - 1)) / rows;

        float tileWidthNormalized = static_cast<float>(tileWidth) / originalSprite.m_Texture->GetWidth();
        float tileHeightNormalized = static_cast<float>(tileHeight) / originalSprite.m_Texture->GetHeight();

        float advanceX = static_cast<float>(tileWidth + spacing) / originalSprite.m_Texture->GetWidth();
        float advanceY = static_cast<float>(tileHeight + spacing) / originalSprite.m_Texture->GetHeight();

        float currentY = originalSprite.m_Pos1Y;
        m_SpriteTable.reserve(m_SpriteTable.size() + rows * columns);
        for (uint row = 0; row < rows; ++row)
        {
            float currentX = originalSprite.m_Pos1X;
            for (uint column = 0; column < columns; ++column)
            {
                std::string name = mapName + "_" + std::to_string(row) + "_" + std::to_string(column);
                bool rotated = false;
                float u1 = currentX;
                float v1 = currentY;
                float u2 = currentX + tileWidthNormalized;
                float v2 = currentY - tileHeightNormalized;

                m_SpriteTable.emplace_back(u1, v1, u2, v2, tileWidth, tileHeight, m_Texture, name, scale, rotated);
                currentX += advanceX;
            }
            currentY -= advanceY;
        }
        return true;
    }

    // from file on disk
    bool SpriteSheet::AddSpritesheetTile(const std::string& fileName, const std::string& mapName, uint rows, uint columns,
                                         uint spacing, const float scale)
    {
        m_Texture = Texture::Create();
        m_Rows = rows;
        m_Columns = columns;
        bool ok = m_Texture->Init(fileName, Texture::USE_SRGB);
        if (ok)
        {
            AddSpritesheetTile(mapName, rows, columns, spacing, scale);
        }
        else
        {
            LOG_CORE_CRITICAL("Couldn't load {0}", fileName);
        }
        return ok;
    }

    // from file in memory
    bool SpriteSheet::AddSpritesheetTile(const char* path /* GNU */, int resourceID /* MSVC */,
                                         const std::string& resourceClass /* MSVC */, const std::string& mapName, uint rows,
                                         uint columns, uint spacing, const float scale)
    {
        m_Texture = Texture::Create();
        m_Rows = rows;
        m_Columns = columns;
        size_t fileSize;
        const uchar* data = (const uchar*)ResourceSystem::GetDataPointer(fileSize, path, resourceID, resourceClass);
        bool ok = m_Texture->Init(data, fileSize, Texture::USE_SRGB);
        if (ok)
        {
            AddSpritesheetTile(mapName, rows, columns, spacing, scale);
        }
        else
        {
#ifdef _WIN32
            LOG_CORE_CRITICAL("Couldn't load resource from resourceID: {0}, resourceClass: {1}", resourceID, resourceClass);
#else
            LOG_CORE_CRITICAL("Couldn't load resource from path: {0}", path);
#endif
        }
        return ok;
    }

    // internal
    void SpriteSheet::AddSpritesheetTile(const std::string& mapName, uint rows, uint columns, uint spacing,
                                         const float scale)
    {
        int tileWidth = (m_Texture->GetWidth() - spacing * (columns - 1)) / columns;
        int tileHeight = (m_Texture->GetHeight() - spacing * (rows - 1)) / rows;

        float tileWidthNormalized = static_cast<float>(tileWidth) / m_Texture->GetWidth();
        float tileHeightNormalized = static_cast<float>(tileHeight) / m_Texture->GetHeight();

        float advanceX = static_cast<float>(tileWidth + spacing) / m_Texture->GetWidth();
        float advanceY = static_cast<float>(tileHeight + spacing) / m_Texture->GetHeight();

        float currentY = 0.0f;
        m_SpriteTable.reserve(m_SpriteTable.size() + rows * columns);
        for (uint row = 0; row < rows; ++row)
        {
            float currentX = 0.0f;
            for (uint column = 0; column < columns; ++column)
            {
                std::string name = mapName + "_" + std::to_string(row) + "_" + std::to_string(column);
                bool rotated = false;
                float u1 = currentX;
                float v1 = currentY;
                float u2 = currentX + tileWidthNormalized;
                float v2 = currentY + tileHeightNormalized;

                m_SpriteTable.emplace_back(u1, v1, u2, v2, tileWidth, tileHeight, m_Texture, name, scale, rotated);
                currentX += advanceX;
            }
            currentY += advanceY;
        }
    }

    void SpriteSheet::ListSprites()
    {
        uint i = 0;

        for (auto sprite : m_SpriteTable)
        {
            LOG_CORE_INFO("Found sprite, name: {0}, index: {1}", sprite.GetName(), std::to_string(i));
            ++i;
        }
    }

    Sprite& SpriteSheet::GetSprite(uint index) { return m_SpriteTable[index]; }

    bool SpriteSheet::AddSpritesheetRow(const Sprite& originalSprite, uint frames, const float scaleX, const float scaleY)
    {
        m_Rows = 1;
        m_Columns = frames;
        bool ok = true;

        m_Texture = originalSprite.m_Texture;
        bool rotated = originalSprite.IsRotated();

        int tileWidth = originalSprite.GetWidth() / frames;
        int tileHeight = originalSprite.GetHeight();

        float tileWidthNormalized = static_cast<float>(tileWidth) / m_Texture->GetWidth();
        m_SpriteTable.reserve(m_SpriteTable.size() + frames);
        if (rotated)
        {
            float advanceY = tileWidthNormalized;

            float currentY = originalSprite.m_Pos1Y - tileWidthNormalized;
            for (uint row = 0; row < frames; ++row)
            {
                std::string name = originalSprite.GetName() + "_" + std::to_string(row);
                float u1 = originalSprite.m_Pos1X;
                float v1 = currentY;
                float u2 = originalSprite.m_Pos2X;
                float v2 = currentY + tileWidthNormalized;
                m_SpriteTable.emplace_back(u1, v2, u2, v1, tileHeight, tileWidth, m_Texture, name, scaleX, scaleY, rotated);
                currentY -= advanceY;
            }
        }
        else
        {
            float advanceX = tileWidthNormalized;

            float currentX = originalSprite.m_Pos1X;
            for (uint column = 0; column < frames; ++column)
            {
                std::string name = originalSprite.GetName() + "_" + std::to_string(column);
                float u1 = currentX;
                float v1 = originalSprite.m_Pos1Y;
                float u2 = currentX + tileWidthNormalized;
                float v2 = originalSprite.m_Pos2Y;
                m_SpriteTable.emplace_back(u1, v1, u2, v2, tileWidth, tileHeight, m_Texture, name, scaleX, scaleY);
                currentX += advanceX;
            }
        }

        return ok;
    }

    bool SpriteSheet::AddSpritesheetRow(const Sprite& originalSprite, uint frames, const float scale)
    {
        return AddSpritesheetRow(originalSprite, frames, scale, scale);
    }

    bool SpriteSheet::AddSpritesheetRow(const std::string& fileName, uint frames, const float scaleX, const float scaleY)
    {
        m_Texture = Texture::Create();
        bool ok = m_Texture->Init(fileName, Texture::USE_SRGB);

        Sprite originalSprite{0.0f,      1.0f,     1.0f, 0.0f, m_Texture->GetWidth(), m_Texture->GetHeight(),
                              m_Texture, fileName, 1.0f, 1.0f};

        AddSpritesheetRow(originalSprite, frames, scaleX, scaleY);

        return ok;
    }

    bool SpriteSheet::AddSpritesheetRow(const std::string& fileName, uint frames, const float scale)
    {
        m_Texture = Texture::Create();
        bool ok = m_Texture->Init(fileName, Texture::USE_SRGB);

        Sprite originalSprite{0.0f,      1.0f,     1.0f, 0.0f, m_Texture->GetWidth(), m_Texture->GetHeight(),
                              m_Texture, fileName, scale};

        AddSpritesheetRow(originalSprite, frames);

        return ok;
    }

    bool SpriteSheet::AddSpritesheetRow(const char* path /* GNU */, int resourceID /* MSVC */,
                                        const std::string& resourceClass /* MSVC */, uint frames, const float scaleX,
                                        const float scaleY)
    {
        m_Texture = Texture::Create();
        size_t fileSize;
        const uchar* data = (const uchar*)ResourceSystem::GetDataPointer(fileSize, path, resourceID, resourceClass);
        bool ok = m_Texture->Init(data, fileSize, Texture::USE_SRGB);

        Sprite originalSprite{
            0.0f, 1.0f, 1.0f, 0.0f, m_Texture->GetWidth(), m_Texture->GetHeight(), m_Texture, std::string(path), 1.0f, 1.0f};

        AddSpritesheetRow(originalSprite, frames, scaleX, scaleY);

        return ok;
    }

    bool SpriteSheet::AddSpritesheetRow(const char* path /* GNU */, int resourceID /* MSVC */,
                                        const std::string& resourceClass /* MSVC */, uint frames, const float scale)
    {
        m_Texture = Texture::Create();
        size_t fileSize;
        const uchar* data = (const uchar*)ResourceSystem::GetDataPointer(fileSize, path, resourceID, resourceClass);
        bool ok = m_Texture->Init(data, fileSize, Texture::USE_SRGB);

        Sprite originalSprite{
            0.0f, 1.0f, 1.0f, 0.0f, m_Texture->GetWidth(), m_Texture->GetHeight(), m_Texture, std::string(path), scale};

        AddSpritesheetRow(originalSprite, frames);

        return ok;
    }

    void SpriteSheet::SetScale(const float scale)
    {
        for (auto& sprite : m_SpriteTable)
        {
            sprite.SetScale(scale);
        }
    }
} // namespace GfxRenderEngine
